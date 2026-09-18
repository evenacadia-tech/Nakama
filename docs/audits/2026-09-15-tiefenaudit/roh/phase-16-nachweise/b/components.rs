//! Audit-only driver. Calls the unmodified broker crate, never reimplements it.
use eqcop_broker::coordinator::{Coordinator, ManualClock, SessionPush};
use eqcop_broker::generiert::nakama_telemetry_v1_generated::nakama::v_3 as fb;
use eqcop_broker::store::{StoreKonfiguration, StoreWriter};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use flatbuffers::FlatBufferBuilder;
use serde_json::{json, Value};
use std::sync::{Arc, Mutex};

pub fn hex(n: usize) -> String { format!("{n:032x}") }
pub fn hello(session: usize, instance: usize, nonce: usize, kind: &str) -> HelloControl {
    HelloControl { typ: "hello".into(), connection_kind: "control".into(), protocol: 3,
        plugin_version: "audit16".into(), plugin_kind: kind.into(),
        adresse: Adresse { logon_sid: "S-1-5-21-1-2-3-1001".into(), project_binding_id: hex(1),
            session_epoch: hex(session), instance_id: hex(instance), runtime_nonce: hex(nonce) },
        host: Some(HostAngabe { pid: 16016, name: Some("FL Studio".into()), version: None }),
        audio: AudioLage { samplerate: 48000., block_size: 512, channels: 2 } }
}
pub fn heartbeat(a: &Adresse, seq: u64) -> Vec<u8> {
    let mut value=json!({"type":"heartbeat", "adresse":a, "sequence":seq,
        "state_revision":0, "intervention_state_unknown":false,
        "capabilities":{
            "host_context_presence":"supported", "project_time_samples":"supported",
            "sample_accurate_automation":"supported", "presentation_latency":"supported",
            "aux_compare_pre":"unsupported", "aux_priority_sidechain":"unsupported",
            "contribution_aux":"unsupported", "float64_processing":"supported",
            "binary_telemetry":"supported", "remote_control":"supported"},
        "zaehler":{"frames_dropped":0,"parse_errors":0,"queue_overflows":0}});
    // This driver's probe has instance 20. Real Main heartbeats omit runtime.
    if a.instance_id==hex(20) { value["runtime"]=json!({"messpunkt":"insert","betrieb":"active"}); }
    serde_json::to_vec(&value).unwrap()
}
pub fn beat(c: &Coordinator, link: &str, a: &Adresse, seq: u64) -> Value {
    serde_json::from_slice(&Senke::p0(c, link, &heartbeat(a,seq)).expect("heartbeat_ack")).unwrap()
}
pub fn register(c: &Coordinator, link: &str, h: &HelloControl) {
    let r=Senke::control_verbunden(c,link,h); assert!(r.angenommen,"{r:?}");
}
pub fn subscribe(c: &Coordinator, link: &str, a: &Adresse) {
    Senke::p1(c,link,&serde_json::to_vec(&json!({"type":"subscribe_session",
        "adresse":a,"session_epoch":a.session_epoch})).unwrap());
}
pub fn batch(a: &Adresse) -> Vec<u8> {
    let mut b=FlatBufferBuilder::new();
    let sid=b.create_string(&a.logon_sid); let project=b.create_string(&a.project_binding_id);
    let session=b.create_string(&a.session_epoch); let instance=b.create_string(&a.instance_id);
    let nonce=b.create_string(&a.runtime_nonce);
    let addr=fb::Adresse::create(&mut b,&fb::AdresseArgs { logon_sid:Some(sid),
        project_binding_id:Some(project), session_epoch:Some(session), instance_id:Some(instance), runtime_nonce:Some(nonce) });
    let values=b.create_vector(&[0i16;64]); let bitmap=b.create_vector(&[255u8;8]);
    let bands=fb::Bandwerte::create(&mut b,&fb::BandwerteArgs { gitter:fb::Bandgitter::nakama_log64_v1,
        encoding:fb::BandEncoding::q_db_0p1_i16,werte_i16:Some(values),gueltig_bitmap:Some(bitmap),..Default::default() });
    let transport=fb::Transportstempel::create(&mut b,&fb::TransportstempelArgs { transport_epoch:1,
        continuity_segment:1,sequence:1,zeitbasis:fb::Zeitbasis::local_monotonic,
        sample_count:512,sample_rate:48000.,process_context_present:Some(false),..Default::default() });
    let frame=fb::Frame::create(&mut b,&fb::FrameArgs { transport:Some(transport),baender:Some(bands),
        metrics_version:1,aktivitaet:Some(1.),lufs_s:Some(-18.),..Default::default() });
    let entry=fb::QuellenEintrag::create(&mut b,&fb::QuellenEintragArgs {quelle:Some(addr),frame:Some(frame)});
    let entries=b.create_vector(&[entry]);
    let root=fb::FeatureBatch::create(&mut b,&fb::FeatureBatchArgs {eintraege:Some(entries)});
    fb::finish_feature_batch_buffer(&mut b,root); b.finished_data().to_vec()
}
#[derive(Default)]
pub struct Push { pub snapshots:Mutex<Vec<Value>>, pub frames:Mutex<usize> }
impl SessionPush for Push {
    fn snapshot_schreiben(&self,_link:&str,_key:&str,_order:i64,p:&[u8])->bool {
        self.snapshots.lock().unwrap().push(serde_json::from_slice(p).unwrap()); true
    }
    fn messframe_schreiben(&self,_link:&str,_instance:&str,_p:&[u8])->bool {
        *self.frames.lock().unwrap()+=1; true
    }
}
pub fn emit(name:&str,v:Value) { println!("{}",json!({"case":name,"observed":v})); }

fn store_rounds(degraded:bool, runtime:bool) {
    let root=std::env::var("AUDIT16_DATA").expect("AUDIT16_DATA isolated build dir");
    let dir=std::path::Path::new(&root).join(if degraded {"degraded-db-as-directory"} else {"healthy"});
    std::fs::create_dir_all(&dir).unwrap();
    let path=if degraded {dir.clone()} else {dir.join(if runtime {"runtime.sqlite3"}else{"store.sqlite3"})};
    let mut writer=StoreWriter::starten(StoreKonfiguration::fuer_pfad(path.clone()));
    assert_eq!(writer.ist_degradiert(),degraded);
    let clock=Arc::new(ManualClock::default());
    let c=Coordinator::mit_store(clock.clone(),hex(999),&writer);
    let push=Arc::new(Push::default()); c.session_push_setzen(push.clone());
    let m=hello(2,10,100,"main"); let p=hello(2,20,200,"active_probe");
    let mut cycles=Vec::new(); let mut main="m0".to_owned(); let mut probe="p0".to_owned();
    register(&c,&main,&m); register(&c,&probe,&p); Senke::telemetrie_gekoppelt(&c,&probe);
    let mut lock=None;
    if runtime {
        beat(&c,&main,&m.adresse,1);beat(&c,&probe,&p.adresse,1);subscribe(&c,&main,&m.adresse);
        Senke::p2(&c,&probe,&batch(&p.adresse));assert_eq!(c.p2_live_frames(),1);
        push.snapshots.lock().unwrap().clear();
        let conn=rusqlite::Connection::open(path).unwrap();conn.execute_batch("BEGIN IMMEDIATE").unwrap();
        lock=Some(conn);
    }
    for n in 0..5 {
        clock.vor(1000);
        let ma=beat(&c,&main,&m.adresse,n+2);
        if let Some(conn)=lock.take(){assert!(writer.ist_degradiert());conn.execute_batch("ROLLBACK").unwrap();}
        let pa=beat(&c,&probe,&p.adresse,n+2);
        subscribe(&c,&main,&m.adresse); Senke::p2(&c,&probe,&batch(&p.adresse));
        let dead=c.liveness_tick();
        cycles.push(json!({"round":n,"close":dead,"refusals":c.store_verweigerungen(),
            "p2":c.p2_live_frames(),"snapshots":push.snapshots.lock().unwrap().len(),"main_ack":ma,"probe_ack":pa}));
        if degraded || runtime {
            assert_eq!(dead.len(),2); assert_eq!(c.p2_live_frames(),if runtime {1}else{0});
            Senke::control_schliesst(&c,&main); Senke::control_getrennt(&c,&main);
            Senke::control_schliesst(&c,&probe); Senke::telemetrie_getrennt(&c,&probe); Senke::control_getrennt(&c,&probe);
            main=format!("m{}",n+1); probe=format!("p{}",n+1);
            register(&c,&main,&m); register(&c,&probe,&p); Senke::telemetrie_gekoppelt(&c,&probe);
        } else { assert!(dead.is_empty()); assert_eq!(c.p2_live_frames(),n+1); }
    }
    if degraded || runtime { assert!(push.snapshots.lock().unwrap().is_empty()); }
    emit(if degraded {"T3-15-12-start-degraded"} else if runtime {"T3-15-12-runtime-degraded"} else {"T3-15-12-healthy-countercase"},
        json!({"degraded":writer.ist_degradiert(),"routing":c.routing_bereit(),"cycles":cycles}));
    writer.stoppen();
}

fn command(c:&Coordinator,link:&str,target:&Adresse,id:usize)->Value {
    let p=serde_json::to_vec(&json!({"type":"session_command","command":"confirm_join",
        "command_id":hex(id),"ziel":target,"session_epoch":target.session_epoch})).unwrap();
    serde_json::from_slice(&Senke::p0(c,link,&p).expect("command_ack")).unwrap()
}
fn replay_after_eviction() {
    let clock=Arc::new(ManualClock::default()); let c=Coordinator::mit_uhr(clock.clone(),hex(999));
    let m=hello(2,10,100,"main"); let p=hello(2,20,200,"active_probe");
    register(&c,"main",&m); beat(&c,"main",&m.adresse,1); register(&c,"probe",&p); beat(&c,"probe",&p.adresse,1);
    Senke::control_schliesst(&c,"main"); Senke::control_getrennt(&c,"main");
    clock.vor(500); register(&c,"fast",&m);
    let quick=command(&c,"fast",&p.adresse,700); assert_eq!(quick["ergebnis"],"angewandt");
    Senke::control_schliesst(&c,"fast"); Senke::control_getrennt(&c,"fast");
    clock.vor(10001); beat(&c,"probe",&p.adresse,2); c.liveness_tick();
    assert_eq!(c.client_anzahl(),1);
    register(&c,"after-eviction",&m);
    let early=command(&c,"after-eviction",&p.adresse,701);
    assert_eq!(early["ergebnis"],"abgelehnt"); assert_eq!(early["code"],"unauthorized");
    beat(&c,"after-eviction",&m.adresse,3);
    let late=command(&c,"after-eviction",&p.adresse,702); assert_eq!(late["ergebnis"],"angewandt");
    emit("F17-replay-eviction",json!({"before_eviction":quick,"before_first_heartbeat":early,"after_heartbeat":late}));
}
fn two_mains() {
    let clock=Arc::new(ManualClock::default()); let c=Coordinator::mit_uhr(clock.clone(),hex(999));
    let a=hello(2,10,100,"main"); let b=hello(3,11,101,"main"); let p=hello(1,20,200,"active_probe");
    register(&c,"main-a",&a); beat(&c,"main-a",&a.adresse,1);
    register(&c,"main-b",&b); beat(&c,"main-b",&b.adresse,1);
    register(&c,"probe",&p); beat(&c,"probe",&p.adresse,1);
    let before=json!({"main_a":String::from_utf8(c.session_snapshot_json(&hex(1),&hex(2))).unwrap(),
        "main_b":String::from_utf8(c.session_snapshot_json(&hex(1),&hex(3))).unwrap(),
        "unbound":format!("{:?}",c.modell_sicht(&hex(1),&hex(1)))});
    Senke::control_schliesst(&c,"main-b"); Senke::control_getrennt(&c,"main-b");
    let immediate=c.liveness_tick();
    clock.vor(10001); beat(&c,"main-a",&a.adresse,2); beat(&c,"probe",&p.adresse,2);
    let after=c.liveness_tick();
    assert!(after.contains(&"probe".to_owned()));
    Senke::control_schliesst(&c,"probe"); Senke::control_getrennt(&c,"probe");
    register(&c,"probe-again",&p); beat(&c,"probe-again",&p.adresse,3);
    let view=c.modell_sicht(&hex(1),&hex(2)); assert!(view.clients.iter().any(|x|x.adresse.instance_id==p.adresse.instance_id && x.bestaetigt));
    clock.vor(10001);beat(&c,"main-a",&a.adresse,4);beat(&c,"probe-again",&p.adresse,4);c.liveness_tick();
    let final_view=c.modell_sicht(&hex(1),&hex(2));assert!(!final_view.beitritt_bestaetigung_noetig);
    emit("F16-two-mains",json!({"before":before,"immediate_close":immediate,"after_10s_close":after,"recovered":format!("{view:?}"),
        "after_unbound_tombstone_eviction":format!("{final_view:?}")}));
}
fn collisions() {
    let clock=Arc::new(ManualClock::default()); let c=Coordinator::mit_uhr(clock.clone(),hex(999));
    for n in 0..512 {
        let a=hello(10+n,20,1000+n*2,"active_probe"); let b=hello(10+n,20,1001+n*2,"active_probe");
        register(&c,"old",&a); register(&c,"new",&b);
        beat(&c,"old",&a.adresse,1);
        Senke::control_schliesst(&c,"old"); Senke::control_getrennt(&c,"old");
        Senke::control_schliesst(&c,"new"); Senke::control_getrennt(&c,"new");
        clock.vor(10001); c.liveness_tick();
    }
    let other=hello(2000,2000,2000,"main");
    let rejected=Senke::control_verbunden(&c,"unrelated",&other);
    assert!(!rejected.angenommen); assert_eq!(rejected.grund.as_deref(),Some("alias_quarantaene_deckel"));
    emit("F18-collision-lifetime",json!({"collisions":512,"active_clients":c.client_anzahl(),"unrelated_result":format!("{rejected:?}"),"routing":c.routing_bereit()}));
}
fn persistent_collisions() {
    let root=std::env::var("AUDIT16_DATA").unwrap();let db=std::path::Path::new(&root).join("guard-lifetime.sqlite3");
    let mut writer=StoreWriter::starten(StoreKonfiguration::fuer_pfad(&db));assert!(!writer.ist_degradiert());
    let mut counts=Vec::new();
    for n in 0..3 {
        let clock=Arc::new(ManualClock::default());let c=Coordinator::mit_store(clock.clone(),hex(999+n),&writer);
        let a=hello(100+n,20,300+n*2,"active_probe");let b=hello(100+n,20,301+n*2,"active_probe");
        register(&c,"old",&a);register(&c,"new",&b);beat(&c,"old",&a.adresse,1);
        Senke::control_schliesst(&c,"old");Senke::control_getrennt(&c,"old");
        Senke::control_schliesst(&c,"new");Senke::control_getrennt(&c,"new");clock.vor(10001);c.liveness_tick();
        assert_eq!(c.client_anzahl(),0);
        let count:i64=rusqlite::Connection::open(&db).unwrap().query_row("SELECT COUNT(*) FROM conflict_guards",[],|r|r.get(0)).unwrap();
        counts.push(count);assert_eq!(count,2*(n as i64+1));
    }
    writer.stoppen();
    let mut restored=StoreWriter::starten(StoreKonfiguration::fuer_pfad(&db));
    let c=Coordinator::mit_store(Arc::new(ManualClock::default()),hex(9999),&restored);
    let other=hello(2000,2000,2000,"main");let outcome=Senke::control_verbunden(&c,"other",&other);
    assert!(outcome.angenommen);
    emit("F18-persistent-guard-lifetime",json!({"guards_after_closed_sessions":counts,"unrelated_after_broker_restart":format!("{outcome:?}"),"routing":c.routing_bereit()}));
    restored.stoppen();
}
pub fn run() { store_rounds(false,false); store_rounds(true,false); store_rounds(false,true); replay_after_eviction(); two_mains(); collisions(); persistent_collisions(); }
