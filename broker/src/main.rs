//! Eigenständiger EQ-Copilot-Broker (Nakama) — ersetzt den in der
//! Plugin-Hub-App eingebetteten Start (Herauslösung 18.08.2026).
//!
//!   eqcop-broker.exe [--bindungen <pfad>]
//!
//! Ohne Argument liegen die persistenten Profilbindungen unter
//! %APPDATA%\evenacadia\nakama\eq-copilot-bindungen.json (die Datei wird
//! beim ersten Binden angelegt). Der Broker bedient den v2-Legacy- und den
//! SID-gebundenen v3-Endpunkt und beendet sich nach der festgelegten Idlefrist
//! ohne Clients selbst. FIRST_PIPE_INSTANCE verhindert einen zweiten Broker
//! auf den Produktionsnamen.

use std::path::PathBuf;

fn standard_bindungen() -> Option<PathBuf> {
    std::env::var_os("APPDATA").map(|a| {
        PathBuf::from(a)
            .join("evenacadia")
            .join("nakama")
            .join("eq-copilot-bindungen.json")
    })
}

fn main() {
    let mut args = std::env::args().skip(1);
    let mut bindungen = standard_bindungen();
    while let Some(a) = args.next() {
        match a.as_str() {
            "--bindungen" => match args.next() {
                Some(p) => bindungen = Some(PathBuf::from(p)),
                None => {
                    eprintln!("--bindungen braucht einen Pfad");
                    std::process::exit(2);
                }
            },
            unbekannt => {
                eprintln!("Unbekanntes Argument: {unbekannt}");
                eprintln!("Aufruf: eqcop-broker.exe [--bindungen <pfad>]");
                std::process::exit(2);
            }
        }
    }
    if let Some(p) = &bindungen {
        if let Some(eltern) = p.parent() {
            if let Err(e) = std::fs::create_dir_all(eltern) {
                eprintln!("Bindungs-Ordner {}: {e}", eltern.display());
            }
        }
    }
    match eqcop_broker::broker_starten(bindungen.clone()) {
        Ok(()) => {
            println!(
                "EQ-Copilot-Broker läuft auf {} (Bindungen: {})",
                eqcop_broker::PIPE_NAME,
                bindungen
                    .as_deref()
                    .map(|p| p.display().to_string())
                    .unwrap_or_else(|| "keine — nur im Speicher".into())
            );
            let fataler_listenerfehler = loop {
                std::thread::sleep(std::time::Duration::from_millis(250));
                if eqcop_broker::broker_hat_fatalen_v3_listenerfehler() {
                    eprintln!("Fataler v3-Listenerfehler; Broker stoppt fail-closed");
                    break true;
                }
                if eqcop_broker::broker_soll_idle_enden() {
                    println!("EQ-Copilot-Broker beendet sich nach lokaler Idlefrist selbst");
                    break false;
                }
            };
            eqcop_broker::broker_geordnet_stoppen();
            if fataler_listenerfehler {
                std::process::exit(1);
            }
        }
        Err(e) => {
            eprintln!("Broker-Start fehlgeschlagen: {e}");
            std::process::exit(1);
        }
    }
}
