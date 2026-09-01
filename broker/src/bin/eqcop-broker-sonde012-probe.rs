//! SONDE-012 L06: echter Coordinator auf einem ausschliesslichen Probe-Namen.

use eqcop_broker::coordinator::{Coordinator, MonotonicClock};
use eqcop_broker::transport::bootstrap::neue_kennung;
use eqcop_broker::transport::pipetoken::{ist_probe_pipename, PROBE_PRAEFIX};
use eqcop_broker::transport::server_v3::{v3_server_starten_mit_epoch_und_sender, V3Sender};
use std::io::{BufRead, Write};
use std::sync::Arc;
use std::time::{Duration, Instant};

struct ProbeClock(Instant);

impl MonotonicClock for ProbeClock {
    fn jetzt(&self) -> Duration {
        self.0.elapsed()
    }
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("Aufruf: eqcop-broker-sonde012-probe <pipe-name> [sekunden]");
        std::process::exit(2);
    }
    let pipe = args[1].clone();
    if !ist_probe_pipename(&pipe) {
        eprintln!("VERWEIGERT: {pipe} liegt nicht im Probe-Namensraum {PROBE_PRAEFIX}");
        std::process::exit(3);
    }
    let sekunden = args
        .get(2)
        .and_then(|s| s.parse::<u64>().ok())
        .unwrap_or(90);
    let epoch = neue_kennung();
    let coordinator = Arc::new(Coordinator::mit_uhr(
        Arc::new(ProbeClock(Instant::now())),
        epoch.clone(),
    ));
    let sender = V3Sender::neu();
    coordinator.session_push_setzen(Arc::new(sender.clone()));
    let mut griff = match v3_server_starten_mit_epoch_und_sender(
        &pipe,
        coordinator,
        "sonde012-latency".into(),
        epoch,
        sender,
    ) {
        Ok(griff) => griff,
        Err(fehler) => {
            eprintln!("Listener konnte nicht starten: {fehler}");
            std::process::exit(3);
        }
    };

    println!("BEREIT {pipe}");
    let _ = std::io::stdout().flush();
    let (tx, rx) = std::sync::mpsc::channel();
    std::thread::spawn(move || {
        let mut zeile = String::new();
        let stdin = std::io::stdin();
        while stdin.lock().read_line(&mut zeile).unwrap_or(0) > 0 {
            if zeile.trim() == "STOP" {
                let _ = tx.send(());
                return;
            }
            zeile.clear();
        }
        let _ = tx.send(());
    });
    let ende = Instant::now() + Duration::from_secs(sekunden);
    while Instant::now() < ende && rx.recv_timeout(Duration::from_millis(50)).is_err() {}
    griff.stoppen();
}
