//! Real v3 listener on named probe pipes, with a timed Senke wrapper.
//! The wrapper delegates every payload to the original Coordinator. It only
//! holds one completed call to expose a legal interleaving deterministically.
use crate::components::*;
use eqcop_broker::coordinator::{Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::HelloControl;
use eqcop_broker::transport::server_v3::{Senke, ControlAnmeldung, v3_server_starten};
use eqcop_broker::transport::v3::{envelope_schreiben,envelope_pruefen,Familie};
use serde_json::{json,Value};
use std::sync::{Arc,Mutex,Condvar};
use std::sync::atomic::{AtomicBool,AtomicU64,Ordering::SeqCst};
use std::time::{Duration,Instant};
use windows_sys::Win32::Foundation::{HANDLE,INVALID_HANDLE_VALUE,CloseHandle,GENERIC_READ,GENERIC_WRITE};
use windows_sys::Win32::Storage::FileSystem::{CreateFileW,WriteFile,ReadFile,OPEN_EXISTING,SECURITY_SQOS_PRESENT,SECURITY_IDENTIFICATION};
use windows_sys::Win32::System::Pipes::PeekNamedPipe;

struct Client { h:HANDLE }
impl Client {
    fn new(pipe:&str)->Self {
        assert!(eqcop_broker::transport::pipetoken::ist_probe_pipename(pipe));
        let wide:Vec<u16>=pipe.encode_utf16().chain(Some(0)).collect();
        let end=Instant::now()+Duration::from_secs(3);
        loop {
            // Owned synchronous probe handle; same flags as repo Testclient.
            let h=unsafe{CreateFileW(wide.as_ptr(),GENERIC_READ|GENERIC_WRITE,0,std::ptr::null(),
                OPEN_EXISTING,SECURITY_SQOS_PRESENT|SECURITY_IDENTIFICATION,std::ptr::null_mut())};
            if h!=INVALID_HANDLE_VALUE{return Self{h}}
            assert!(Instant::now()<end,"probe connect timeout"); std::thread::sleep(Duration::from_millis(5));
        }
    }
    fn write(&self,data:&[u8]) {
        let mut offset=0;
        while offset<data.len() {
            let mut n=0;
            let ok=unsafe{WriteFile(self.h,data[offset..].as_ptr(),(data.len()-offset) as u32,&mut n,std::ptr::null_mut())};
            assert!(ok!=0 && n>0,"probe write failed"); offset+=n as usize;
        }
    }
    fn available(&self)->Option<u32> {
        let mut n=0;
        let ok=unsafe{PeekNamedPipe(self.h,std::ptr::null_mut(),0,std::ptr::null_mut(),&mut n,std::ptr::null_mut())};
        (ok!=0).then_some(n)
    }
    fn frame(&self)->Value {
        let end=Instant::now()+Duration::from_secs(3); let mut raw=Vec::new();
        loop {
            if let Ok(f)=envelope_pruefen(&raw) { return serde_json::from_slice(&f.payload).unwrap() }
            assert!(Instant::now()<end,"frame timeout, bytes={}",raw.len());
            let n=self.available().expect("peer closed before frame");
            if n==0 {std::thread::sleep(Duration::from_millis(2));continue}
            let mut data=vec![0;n as usize]; let mut read=0;
            assert_ne!(unsafe{ReadFile(self.h,data.as_mut_ptr(),n,&mut read,std::ptr::null_mut())},0);
            raw.extend_from_slice(&data[..read as usize]);
        }
    }
}
impl Drop for Client { fn drop(&mut self) {unsafe{CloseHandle(self.h)};} }
fn prefixed(v:Value)->Vec<u8> {
    let text=serde_json::to_vec(&v).unwrap(); let mut bytes=(text.len() as u32).to_le_bytes().to_vec(); bytes.extend(text);bytes
}
fn framed(f:Familie,body:&[u8])->Vec<u8> {envelope_schreiben(f,if f==Familie::P2 {2}else{5},body).unwrap()}
fn control(pipe:&str,h:&HelloControl)->(Client,Value) {
    let c=Client::new(pipe); c.write(&prefixed(serde_json::to_value(h).unwrap()));let w=c.frame();
    assert_eq!(w["type"],"welcome");(c,w)
}
fn telemetry(pipe:&str,h:&HelloControl,w:&Value)->Client {
    let c=Client::new(pipe);c.write(&prefixed(json!({"type":"hello","connection_kind":"telemetry",
        "protocol":3,"plugin_version":"audit16","adresse":h.adresse,"link_id":w["link_id"],"challenge":w["challenge"]})));
    assert_eq!(c.frame()["type"],"welcome");c
}
fn wait(label:&str,ms:u64,condition:impl Fn()->bool) {
    let end=Instant::now()+Duration::from_millis(ms);
    while !condition(){assert!(Instant::now()<end,"timeout: {label}");std::thread::sleep(Duration::from_millis(2));}
}
struct Gate { state:Mutex<(bool,bool)>, cv:Condvar }
impl Gate {
    fn new()->Self {Self{state:Mutex::new((false,false)),cv:Condvar::new()}}
    fn hold(&self) {
        let mut s=self.state.lock().unwrap(); s.0=true; self.cv.notify_all();
        let end=Instant::now()+Duration::from_millis(1600);
        while !s.1 { let remain=end.saturating_duration_since(Instant::now());
            assert!(!remain.is_zero(),"audit gate timeout"); s=self.cv.wait_timeout(s,remain).unwrap().0; }
    }
    fn entered(&self)->bool {self.state.lock().unwrap().0}
    fn release(&self) {self.state.lock().unwrap().1=true;self.cv.notify_all();}
}
struct Sink {
    c:Arc<Coordinator>, family:Familie, gate:Gate, hold_next:AtomicBool,
    p1:AtomicU64,p2:AtomicU64,coupled:AtomicU64,decoupled:AtomicU64,closed:AtomicU64,
}
impl Sink {
    fn new(c:Arc<Coordinator>,f:Familie)->Self {Self{c,family:f,gate:Gate::new(),hold_next:AtomicBool::new(true),
        p1:AtomicU64::new(0),p2:AtomicU64::new(0),coupled:AtomicU64::new(0),decoupled:AtomicU64::new(0),closed:AtomicU64::new(0)}}
}
impl Senke for Sink {
    fn control_verbunden(&self,l:&str,h:&HelloControl)->ControlAnmeldung {Senke::control_verbunden(&*self.c,l,h)}
    fn control_schliesst(&self,l:&str){Senke::control_schliesst(&*self.c,l);self.closed.fetch_add(1,SeqCst);}
    fn control_getrennt(&self,l:&str){Senke::control_getrennt(&*self.c,l)}
    fn telemetrie_gekoppelt(&self,l:&str){Senke::telemetrie_gekoppelt(&*self.c,l);self.coupled.fetch_add(1,SeqCst);}
    fn telemetrie_getrennt(&self,l:&str){Senke::telemetrie_getrennt(&*self.c,l);self.decoupled.fetch_add(1,SeqCst);}
    fn p0(&self,l:&str,p:&[u8])->Option<Vec<u8>> {Senke::p0(&*self.c,l,p)}
    fn p1(&self,l:&str,p:&[u8]) {Senke::p1(&*self.c,l,p);self.p1.fetch_add(1,SeqCst);
        if self.family==Familie::P1 && self.hold_next.swap(false,SeqCst){self.gate.hold();}}
    fn p2(&self,l:&str,p:&[u8]) {Senke::p2(&*self.c,l,p);self.p2.fetch_add(1,SeqCst);
        if self.family==Familie::P2 && self.hold_next.swap(false,SeqCst){self.gate.hold();}}
    fn abgewiesen(&self,_g:&str){}
}
fn late_old_telemetry_close() {
    let pipe=r"\\.\pipe\evenacadia.nakama.v3.probe.audit16-b-f13-aff2d818";
    let c=Arc::new(Coordinator::mit_uhr(Arc::new(ManualClock::default()),hex(999)));
    let sink=Arc::new(Sink::new(c.clone(),Familie::P2));
    let server=v3_server_starten(pipe,sink.clone(),"audit16".into()).unwrap();
    let h=hello(2,20,200,"active_probe");let (control,w)=control(pipe,&h);
    control.write(&framed(Familie::P0,&heartbeat(&h.adresse,1)));assert_eq!(control.frame()["type"],"heartbeat_ack");
    let t1=telemetry(pipe,&h,&w);t1.write(&framed(Familie::P2,&batch(&h.adresse)));
    wait("old P2 held",500,||sink.gate.entered());assert_eq!(c.p2_live_frames(),1);
    drop(t1);
    // The real client waits 500 ms after its local close; the server may still
    // be joining the old consumer for up to 2000 ms. This is not a 0 ms trick.
    std::thread::sleep(Duration::from_millis(550));
    let t2=telemetry(pipe,&h,&w);
    assert_eq!(sink.coupled.load(SeqCst),2);assert_eq!(sink.decoupled.load(SeqCst),0);
    let coupled_before=c.telemetrie_kopplungen();
    sink.gate.release();wait("old callback delivered",800,||sink.decoupled.load(SeqCst)==1);
    let coupled_after=c.telemetrie_kopplungen();
    for _ in 0..3 {t2.write(&framed(Familie::P2,&batch(&h.adresse)));}
    wait("new frames reached sink",800,||sink.p2.load(SeqCst)==4);
    assert_eq!(c.p2_live_frames(),1);assert_eq!(coupled_after.0,0);
    control.write(&framed(Familie::P0,&heartbeat(&h.adresse,2)));assert_eq!(control.frame()["type"],"heartbeat_ack");
    assert!(t2.available().is_some());
    emit("F13-real-listener-late-old-close",json!({"pipe":pipe,"coupled_callbacks":sink.coupled.load(SeqCst),
        "old_close_callbacks":sink.decoupled.load(SeqCst),"coordinator_coupled_before":coupled_before,
        "coordinator_coupled_after":coupled_after,"transport_p2_calls":sink.p2.load(SeqCst),
        "coordinator_p2_frames":c.p2_live_frames(),"control_heartbeat_after":"heartbeat_ack","telemetry2_still_open":true}));
    drop(t2);wait("t2 cleanup",1200,||sink.decoupled.load(SeqCst)==2);
    let t3=telemetry(pipe,&h,&w);t3.write(&framed(Familie::P2,&batch(&h.adresse)));
    wait("clean reconnect restores",800,||c.p2_live_frames()==2);
    emit("F13-clean-reconnect-countercase",json!({"coordinator_p2_frames":c.p2_live_frames(),"couplings":c.telemetrie_kopplungen()}));
    drop(t3);drop(control);drop(server);
}
fn evidence(a:&eqcop_broker::transport::bootstrap::Adresse,n:usize)->Vec<u8> {
    let root=std::env::var("AUDIT16_SOURCE").unwrap();
    let file=std::path::Path::new(&root).join("eq-copilot/fixtures/v3/gueltig/evidence-snapshot-mit-ereignissen-und-stereo.json");
    let mut v:Value=serde_json::from_slice(&std::fs::read(file).unwrap()).unwrap();
    v["adresse"]=serde_json::to_value(a).unwrap();v["evidence_id"]=json!(hex(1000+n));v["transport"]["sequence"]=json!(n+1);
    serde_json::to_vec(&v).unwrap()
}
fn p1_close_loss() {
    let pipe=r"\\.\pipe\evenacadia.nakama.v3.probe.audit16-b-f15-aff2d818";
    let c=Arc::new(Coordinator::mit_uhr(Arc::new(ManualClock::default()),hex(999)));
    let sink=Arc::new(Sink::new(c.clone(),Familie::P1));
    let server=v3_server_starten(pipe,sink.clone(),"audit16".into()).unwrap();
    let h=hello(2,20,200,"active_probe");let (client,_w)=control(pipe,&h);
    client.write(&framed(Familie::P0,&heartbeat(&h.adresse,1)));client.frame();
    client.write(&framed(Familie::P1,&evidence(&h.adresse,0)));
    wait("P1 held after real coordinator",500,||sink.gate.entered());
    assert_eq!(c.evidenz_historie(&h.adresse.instance_id).len(),1);
    for n in 1..4 {client.write(&framed(Familie::P1,&evidence(&h.adresse,n)));}
    wait("three queued P1 frames read by server",500,||server.statistik.ingress_hoechststand.load(SeqCst)>=3);
    let high=server.statistik.ingress_hoechststand.load(SeqCst);drop(client);
    wait("semantic control cleanup",500,||sink.closed.load(SeqCst)==1);
    sink.gate.release();std::thread::sleep(Duration::from_millis(100));
    assert_eq!(sink.p1.load(SeqCst),1);
    let (next,_)=control(pipe,&h);next.write(&framed(Familie::P0,&heartbeat(&h.adresse,2)));next.frame();
    let before=c.evidenz_historie(&h.adresse.instance_id).iter().map(|e|e.evidence_id.clone()).collect::<Vec<_>>();
    assert!((1..4).all(|n|!before.contains(&hex(1000+n))));
    next.write(&framed(Familie::P1,&evidence(&h.adresse,4)));
    wait("later evidence accepted",500,||c.evidenz_historie(&h.adresse.instance_id).iter().any(|e|e.evidence_id==hex(1004)));
    emit("F15-real-listener-written-P1-loss",json!({"pipe":pipe,"successfully_written_evidence_frames":4,
        "server_ingress_high_watermark":high,"first_link_p1_calls":1,"before_reconnect_new_evidence":before,
        "history_after_new_evidence":c.evidenz_historie(&h.adresse.instance_id).iter().map(|e|e.evidence_id.clone()).collect::<Vec<_>>(),
        "ingress_overflow":server.statistik.ingress_p1_ueberlauf_trennt.load(SeqCst)}));
    drop(next);drop(server);
}
pub fn run(){late_old_telemetry_close();p1_close_loss();}
