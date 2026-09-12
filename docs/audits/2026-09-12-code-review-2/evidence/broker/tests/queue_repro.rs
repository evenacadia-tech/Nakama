// Compile the original production module; no queue algorithm is copied here.
#![allow(dead_code)]
use std::collections::{HashMap, VecDeque};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Condvar, Mutex};
use std::time::Duration;
use eqcop_broker::transport::server_v3::CAP_WRITER;
use eqcop_broker::transport::v3::Familie;
use eqcop_broker::transport::warteschlange::{IngressErgebnis, IngressWarteschlange};

#[path = "C:/Users/phili/Projekte/Nakama/broker/src/transport/server_v3/queues.rs"]
mod original_queue;

#[test]
fn snapshot_nach_ruecknahme_muss_auch_danach_entnommen_werden() {
    let queue = original_queue::Ausgang::neu();
    let alt = queue.snapshot_einreihen_mit_antwort(
        "session_snapshot", 1, b"old snapshot".to_vec()).unwrap();
    let ruecknahme = queue.snapshot_einreihen_mit_antwort(
        "evidence_invalidate", 2, b"invalidation".to_vec()).unwrap();
    let neu = queue.snapshot_einreihen_mit_antwort(
        "session_snapshot", 3, b"new snapshot".to_vec()).unwrap();
    assert!(!alt.recv_timeout(Duration::from_millis(100)).unwrap());

    let zuerst = queue.entnehmen().unwrap();
    let danach = queue.entnehmen().unwrap();
    println!("Queue publication order: ({}, {}), ({}, {})",
        zuerst.marke, String::from_utf8_lossy(&zuerst.frame),
        danach.marke, String::from_utf8_lossy(&danach.frame));
    zuerst.geschrieben.unwrap().send(true).unwrap();
    danach.geschrieben.unwrap().send(true).unwrap();
    assert!(ruecknahme.recv_timeout(Duration::from_millis(100)).unwrap());
    assert!(neu.recv_timeout(Duration::from_millis(100)).unwrap());
    assert_eq!((zuerst.marke, danach.marke), (2, 3),
        "Die alte Ruecknahme darf nicht hinter den danach gebildeten Vollstand wandern");
}
