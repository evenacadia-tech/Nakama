//! Audit-only component probe. Product code is a path dependency, unchanged.
//! Wire fixtures are from the frozen checkout. Hello and heartbeat fields are
//! assembled from current product serializers: this is not C++ execution.
use eqcop_broker::coordinator::{Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::{Arc, Mutex};

fn hex(n: usize) -> String { format!("{n:032x}") }
fn address(n: usize) -> Adresse {
    Adresse { logon_sid: "S-1-5-21-1-2-3-1001".into(), project_binding_id: hex(0x11),
        session_epoch: hex(0x22), instance_id: hex(n), runtime_nonce: hex(0x40+n) }
}
fn bytes(v: &Value) -> Vec<u8> { serde_json::to_vec(v).unwrap() }
fn fixture(name: &str) -> Value {
    let path = std::path::Path::new("C:/na-audit16-aff2d818/eq-copilot/fixtures/v3/gueltig")
        .join(format!("{name}.json"));
    serde_json::from_slice(&std::fs::read(path).unwrap()).unwrap()
}
fn capabilities() -> Value {
    json!({"host_context_presence":"supported", "project_time_samples":"supported",
        "sample_accurate_automation":"unsupported", "presentation_latency":"unsupported",
        "aux_compare_pre":"unsupported", "aux_priority_sidechain":"unsupported",
        "contribution_aux":"unsupported", "float64_processing":"unsupported",
        "binary_telemetry":"unsupported", "remote_control":"unsupported"})
}
fn hello(c: &Coordinator, link: &str, a: &Adresse, kind: &str) {
    let h=HelloControl { typ:"hello".into(), connection_kind:"control".into(), protocol:3,
        plugin_version:"audit".into(), plugin_kind:kind.into(), adresse:a.clone(),
        host:Some(HostAngabe {pid:4711,name:Some("FL Studio".into()),version:None}),
        audio:AudioLage {samplerate:48000.0,block_size:512,channels:2} };
    assert!(c.control_hello_registrieren(link,&h).angenommen);
}
fn heartbeat(c: &Coordinator, link: &str, a: &Adresse, runtime: Option<Value>) {
    let mut v=json!({"type":"heartbeat", "adresse":a, "sequence":1, "state_revision":0,
        "capabilities":capabilities(),
        "zaehler":{"frames_dropped":0,"parse_errors":0,"queue_overflows":0}});
    if let Some(r)=runtime { v["runtime"]=r; }
    // First heartbeat after Gen has confirmed a neutral rebuild (NAK-180).
    // Use its real wire ingress instead of the direct resync test helper.
    if link=="main" { v["intervention_state_unknown"]=json!(false); }
    let ack=c.p0(link,&bytes(&v)).expect("valid heartbeat accepted by product P0 parser");
    let ack:Value=serde_json::from_slice(&ack).unwrap();
    assert_eq!(ack["type"],"heartbeat_ack");
}
fn intent(c: &Coordinator, a:&Adresse, revision:u64) {
    c.p1("main",&bytes(&json!({"type":"intent_update","adresse":a,
        "session_epoch":a.session_epoch,"vollstaendig":true,"bestand_revision":revision})));
    assert_eq!(c.intent_sicht(&a.project_binding_id,&a.session_epoch).revision,revision as i64);
}
fn evidence(a:&Adresse,n:usize,pos:i64) -> Vec<u8> {
    let mut v=fixture("evidence-snapshot-mit-ereignissen-und-stereo");
    v["adresse"]=serde_json::to_value(a).unwrap(); v["evidence_id"]=json!(hex(0x1000+n));
    v["transport"]["sequence"]=json!(n+1); v["transport"]["project_sample_start"]=json!(pos);
    for path in ["/baender/werte","/verteilung/p10/werte","/verteilung/p50/werte","/verteilung/p95/werte"] {
        if let Some(Value::Array(values))=v.pointer_mut(path) {
            for i in 98..102 { let old=values[i].as_i64().unwrap_or(0); values[i]=json!(old+90); }
        }
    }
    bytes(&v)
}
fn series(c:&Coordinator,link:&str,a:&Adresse,base:usize,count:usize) {
    for i in 0..count { c.p1(link,&evidence(a,base+i,44_108_200+i as i64*512)); }
}
fn setup(main_runtime:bool,main_evidence:bool) -> (Arc<Coordinator>,Adresse,Adresse) {
    let c=Arc::new(Coordinator::mit_uhr(Arc::new(ManualClock::default()),hex(0xbeef)));
    let m=address(1); let p=address(2);
    hello(&c,"main",&m,"main");
    heartbeat(&c,"main",&m,main_runtime.then(||json!({"messpunkt":"insert","betrieb":"active","host_mixer_index":1})));
    hello(&c,"probe",&p,"active_probe");
    heartbeat(&c,"probe",&p,Some(json!({"messpunkt":"post","betrieb":"active","host_mixer_index":3})));
    intent(&c,&m,0);
    if main_evidence { series(&c,"main",&m,0,13); }
    series(&c,"probe",&p,100,11);
    assert_eq!(c.evidenz_historie(&p.instance_id).len(),11,"probe evidence truly accepted");
    (c,m,p)
}
fn session_command(c:&Coordinator,p:&Adresse,command:&str,id:usize) {
    let ack=c.p0("main",&bytes(&json!({"type":"session_command","command":command,
        "command_id":hex(id),"ziel":p,"session_epoch":p.session_epoch}))).unwrap();
    let v:Value=serde_json::from_slice(&ack).unwrap();
    assert_eq!(v["ergebnis"],"angewandt","{v}");
}

#[test]
fn a01_current_gen_messages_never_supply_master_evidence() {
    let (c,m,_p)=setup(false,false);
    let findings=c.befunde_sicht(&m.project_binding_id,&m.session_epoch);
    assert!(c.evidenz_historie(&m.instance_id).is_empty());
    assert_eq!(findings.len(),1);
    assert!(!findings[0].beobachtung.gueltig);
    println!("A01_CURRENT {:?}",findings);
    let (fixed,mf,_)=setup(true,true);
    let positive=fixed.befunde_sicht(&mf.project_binding_id,&mf.session_epoch);
    assert!(!positive.is_empty());
    assert!(positive.iter().any(|b|b.beobachtung.gueltig),"additional explicit master evidence changes output");
    println!("A01_COUNTER {:?}",positive);
}

#[test]
fn f01_unbind_during_calculation_retains_old_membership_until_new_evidence() {
    let (c,m,p)=setup(false,false);
    let mh=m.clone(); let ph=p.clone(); let weak=Arc::downgrade(&c);
    c.rechen_test_haken_setzen(Box::new(move || {
        let c=weak.upgrade().unwrap(); session_command(&c,&ph,"unbind_probe",0x900);
        assert!(!c.befunde_sicht(&mh.project_binding_id,&mh.session_epoch).is_empty());
    }));
    c.p1("probe",&evidence(&p,111,44_108_200+11*512));
    let delayed=c.befunde_sicht(&m.project_binding_id,&m.session_epoch);
    assert_eq!(delayed.len(),1);
    assert!(delayed[0].evidence_ids.contains(&hex(0x1000+111)));
    println!("F01_DELAYED {:?}",delayed);
    c.p1("probe",&evidence(&p,112,44_108_200+12*512));
    let healed=c.befunde_sicht(&m.project_binding_id,&m.session_epoch);
    assert!(healed.is_empty());
    println!("F01_NEXT_EVIDENCE findings={}",healed.len());
}

#[test]
fn f01_empty_calculation_clears_stale_after_intent_revision() {
    let (c,m,p)=setup(false,false);
    session_command(&c,&p,"unbind_probe",0x901);
    let at_hook=Arc::new(Mutex::new(Vec::new())); let slot=Arc::clone(&at_hook);
    let mh=m.clone(); let weak=Arc::downgrade(&c);
    c.rechen_test_haken_setzen(Box::new(move || {
        let c=weak.upgrade().unwrap(); intent(&c,&mh,1);
        *slot.lock().unwrap()=c.befunde_sicht(&mh.project_binding_id,&mh.session_epoch);
    }));
    c.p1("probe",&evidence(&p,111,44_108_200+11*512));
    let interim=at_hook.lock().unwrap();
    assert_eq!(interim.len(),1);
    assert_eq!(interim[0].zustand,eqcop_broker::coordinator::Befundzustand::Stale);
    let final_state=c.befunde_sicht(&m.project_binding_id,&m.session_epoch);
    assert!(final_state.is_empty());
    println!("F01_EMPTY interim_stale={} final_findings={}",interim.len(),final_state.len());
}

#[test]
fn f15_02_reload_position_with_and_without_mixer_context() {
    for next_position in ["pre","post"] {
    for mixer in [None,Some(3)] {
        let c=Coordinator::mit_uhr(Arc::new(ManualClock::default()),hex(0xbeef));
        let m=address(1); let p=address(2);
        hello(&c,"main",&m,"main"); heartbeat(&c,"main",&m,None);
        hello(&c,"probe",&p,"active_probe");
        let mut runtime=json!({"messpunkt":"pre","betrieb":"active"});
        if let Some(index)=mixer {runtime["host_mixer_index"]=json!(index);}
        heartbeat(&c,"probe",&p,Some(runtime.clone()));
        intent(&c,&m,0);
        series(&c,"probe",&p,100,3);
        assert_eq!(c.evidenz_historie(&p.instance_id).iter().filter(|e|e.ausschlussgrund.is_none()).count(),3);
        c.control_ende("probe");
        hello(&c,"probe-new",&p,"active_probe");
        runtime["messpunkt"]=json!(next_position);
        heartbeat(&c,"probe-new",&p,Some(runtime));
        let history=c.evidenz_historie(&p.instance_id);
        let valid=history.iter().filter(|e|e.ausschlussgrund.is_none()).count();
        println!("F15_02 pre->{next_position} mixer={mixer:?} valid={valid} reasons={:?}",history.iter().map(|e|e.ausschlussgrund.as_deref()).collect::<Vec<_>>());
        assert_eq!(valid,if mixer.is_some(){0}else{3});
    }
    }
}
