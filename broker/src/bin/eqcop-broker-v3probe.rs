//! SONDE-010 — v3-Listener als eigenes Probe-Programm.
//!
//! Er oeffnet AUSSCHLIESSLICH den uebergebenen Probe-Pipenamen, nie die
//! Produktions-Pipe (CLAUDE.md, "Bauen und beweisen"). Damit kann das
//! Lastbein den echten Rust-Transport gegen die echten C++-Clients fahren —
//! zwei Sprachen, ein Draht.
//!
//! Aufruf:
//!   eqcop-broker-v3probe <pipe-name> [sekunden]
//!
//! Er meldet `BEREIT <pipe-name>` auf stdout, sobald der Listener steht, und
//! beendet sich nach `sekunden` oder sobald eine Zeile `STOP` auf stdin
//! ankommt. Zum Schluss schreibt er einen JSON-Bericht — der wandert roh ins
//! Beweismanifest.

use std::io::{BufRead, Write};
use std::sync::atomic::Ordering;
use std::sync::Arc;
use std::time::{Duration, Instant};

use eqcop_broker::transport::server_v3::{v3_server_starten, ZaehlSenke};

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("Aufruf: eqcop-broker-v3probe <pipe-name> [sekunden]");
        std::process::exit(2);
    }
    let pipe = args[1].clone();
    if pipe.contains("evenacadia.eq-copilot.v1") {
        eprintln!(
            "VERWEIGERT: {pipe} ist die Produktions-Pipe. Der Probe-Broker faehrt nur \
             ueber einen Probe-Namen."
        );
        std::process::exit(3);
    }
    let sekunden: u64 = args.get(2).and_then(|s| s.parse().ok()).unwrap_or(30);

    let senke = Arc::new(ZaehlSenke::default());
    let mut griff = match v3_server_starten(&pipe, senke.clone(), "v3probe".into()) {
        Ok(g) => g,
        Err(e) => {
            eprintln!("Listener konnte nicht starten: {e}");
            std::process::exit(3);
        }
    };

    println!("BEREIT {pipe}");
    let _ = std::io::stdout().flush();

    let (sender, empfaenger) = std::sync::mpsc::channel::<()>();
    std::thread::spawn(move || {
        let eingabe = std::io::stdin();
        let mut zeile = String::new();
        while eingabe.lock().read_line(&mut zeile).unwrap_or(0) > 0 {
            if zeile.trim() == "STOP" {
                let _ = sender.send(());
                return;
            }
            zeile.clear();
        }
        let _ = sender.send(());
    });

    let ende = Instant::now() + Duration::from_secs(sekunden);
    while Instant::now() < ende {
        if empfaenger.recv_timeout(Duration::from_millis(50)).is_ok() {
            break;
        }
    }

    let s = &griff.statistik;
    let bericht = serde_json::json!({
        "pipe": pipe,
        "angenommen": s.angenommen.load(Ordering::SeqCst),
        "control_verbindungen": senke.control_verbindungen.load(Ordering::SeqCst),
        "telemetrie_verbindungen": senke.telemetrie_verbindungen.load(Ordering::SeqCst),
        "p0": senke.p0.load(Ordering::SeqCst),
        "p0_beantwortet": senke.p0_beantwortet.load(Ordering::SeqCst),
        "p1": senke.p1.load(Ordering::SeqCst),
        "p2": senke.p2.load(Ordering::SeqCst),
        "abweisungen": senke.abweisungen.load(Ordering::SeqCst),
        "letzter_grund": senke.letzter_grund.lock().map(|g| g.clone()).unwrap_or_default(),
        "geschlossen_bootstrap": s.geschlossen_bootstrap.load(Ordering::SeqCst),
        "geschlossen_envelope": s.geschlossen_envelope.load(Ordering::SeqCst),
        "geschlossen_rate": s.geschlossen_rate.load(Ordering::SeqCst),
        "geschlossen_p0_ueberlauf": s.geschlossen_p0_ueberlauf.load(Ordering::SeqCst),
        "ingress_p2_verworfen": s.ingress_p2_verworfen.load(Ordering::SeqCst),
        "ingress_p1_verworfen": s.ingress_p1_verworfen.load(Ordering::SeqCst),
    });
    println!("{}", serde_json::to_string(&bericht).unwrap_or_default());
    let _ = std::io::stdout().flush();
    griff.stoppen();
}
