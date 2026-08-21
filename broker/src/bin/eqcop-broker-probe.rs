// Prozessgrenzen-Probe des EQ-Copilot-Brokers: fährt den ECHTEN Server
// (echte Nur-dieser-User-ACL) für n Sekunden, damit die C++-Konsolen-Probe
// des Plugins (EqCopPipeProbe.exe) Ende-zu-Ende dagegen spricht — dieselben
// Server-/Register-Funktionen, die auch eqcop-broker.exe (main.rs) fährt
// (bis 18.08.2026 rief sie die Tauri-Hub-App in run() an — herausgelöst).
//
//   eqcop-broker-probe.exe [sekunden] [pipe-name]
//
// Ohne pipe-name läuft die Probe auf einem EIGENEN Namen (.m2probe) — nie auf
// dem Produktionsnamen: dort lauscht womöglich der Produktions-Broker (bis 18.08. von der Hub-App gestartet), und zwei
// Broker auf einem Namen stählen sich still die Clients (M2-Fund; der
// Produktions-Broker verweigert das inzwischen per FIRST_PIPE_INSTANCE).
// Druckt am Ende den Brokerstatus als JSON; Exit 0 nur bei sauberem Lauf.
use eqcop_broker as eq_copilot;

fn main() {
    let sekunden: u64 = std::env::args()
        .nth(1)
        .and_then(|a| a.parse().ok())
        .unwrap_or(10);
    let pipe_name = std::env::args()
        .nth(2)
        .unwrap_or_else(|| r"\\.\pipe\evenacadia.eq-copilot.m2probe".to_string());
    eprintln!("Broker läuft auf {pipe_name} — warte {sekunden} s auf Sensoren …");
    match eq_copilot::probe_lauf(&pipe_name, sekunden) {
        Ok(status_json) => {
            println!("{status_json}");
            std::process::exit(0);
        }
        Err(e) => {
            eprintln!("Broker-Probe fehlgeschlagen: {e}");
            std::process::exit(1);
        }
    }
}
