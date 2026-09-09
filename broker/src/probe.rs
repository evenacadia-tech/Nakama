//! Die drei Aufrufe, die von aussen in den Broker hineinreichen.
//!
//! `probe_lauf` faehrt einen kurzen Testlauf auf einem UEBERGEBENEN
//! Pipenamen — nie auf der Produktions-Pipe (CLAUDE.md, „Bauen und
//! beweisen"). `profil_binden` und `aggregat_schreiben` sind die beiden
//! Schreibwege, die der Host anstoesst; beide geben ihren Fehler als Text
//! zurueck, statt ihn zu verschlucken.

use crate::lebenslauf::{broker_version, BrokerStatus, BROKER};
use crate::paar::{aggregat_sperrgrund, paare_auswerten};
use crate::register::Register;
use crate::sitzung::sessions_bilden;
use crate::zeit::jetzt_ms;
use crate::{aggregat, bindung, protokoll, server};
use std::sync::atomic::Ordering;
use std::sync::{Arc, Mutex};

/// Prozessgrenzen-Probe: fährt einen ECHTEN Server (eigene ACL, eigenes
/// Register) auf einem FREI WÄHLBAREN Pipenamen — ohne den globalen OnceLock.
/// So kollidiert die Ende-zu-Ende-Probe nie mit dem Broker einer laufenden (bis 18.08.: Hub-App-gestarteten)
/// Hub-App (M2-Fund: zwei Broker auf einem Namen stehlen sich still Clients;
/// die erste Instanz trägt deshalb jetzt FILE_FLAG_FIRST_PIPE_INSTANCE).
/// Liefert am Ende den Status als JSON.
#[cfg(windows)]
pub fn probe_lauf(pipe_name: &str, sekunden: u64) -> Result<String, String> {
    let register = Arc::new(Mutex::new(Register::default()));
    let session_token = uuid::Uuid::new_v4().to_string();
    let griff = server::server_starten(
        pipe_name,
        register.clone(),
        broker_version(),
        session_token.clone(),
    )?;
    std::thread::sleep(std::time::Duration::from_secs(sekunden));
    let status = {
        let r = register
            .lock()
            .map_err(|_| "Register-Mutex vergiftet".to_string())?;
        let sensoren = r.sensoren_snapshot(jetzt_ms());
        let sessions = sessions_bilden(&sensoren);
        let paare = paare_auswerten(&sensoren);
        BrokerStatus {
            laeuft: true,
            broker_version: broker_version(),
            protokoll_version: protokoll::PROTOKOLL_VERSION,
            pipe_name: pipe_name.to_string(),
            session_token_kurz: session_token.chars().take(8).collect(),
            gestartet_ms: 0,
            pakete_verworfen: r.pakete_verworfen,
            fehler: r.fehler.clone(),
            sensoren,
            sessions,
            paare,
        }
    };
    drop(griff);
    serde_json::to_string_pretty(&status).map_err(|e| format!("Status serialisieren: {e}"))
}

/// Profilbindung setzen/lösen + persistieren. Fehler beim Schreiben werden
/// zurückgegeben UND im Register sichtbar gemerkt — nie still (Plan §8.3).
pub fn profil_binden(sensor_id: &str, profil_id: Option<String>) -> Result<(), String> {
    let Some(Ok(lauf)) = BROKER.get() else {
        return Err("Broker läuft nicht".to_string());
    };
    if lauf.beendet.load(Ordering::SeqCst) {
        return Err("Broker läuft nicht".to_string());
    }
    let mut register = lauf.register.lock().expect("Register-Mutex");
    register.profil_binden(sensor_id, profil_id);
    if let Some(pfad) = &lauf.bindungen_pfad {
        if let Err(e) = bindung::bindungen_schreiben(pfad, &register.bindungen) {
            register.fehler_merken(format!("Profilbindungen schreiben: {e}"));
            return Err(e);
        }
    }
    Ok(())
}

/// Zeitausgerichteter Aggregat-Snapshot aller (gefilterten) Sensoren →
/// eine Datei unter %LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\.
/// Liefert den Pfad der geschriebenen Datei.
pub fn aggregat_schreiben(
    filter_profil: Option<String>,
    filter_pid: Option<u32>,
) -> Result<String, String> {
    let Some(Ok(lauf)) = BROKER.get() else {
        return Err("Broker läuft nicht".to_string());
    };
    if lauf.beendet.load(Ordering::SeqCst) {
        return Err("Broker läuft nicht".to_string());
    }
    let (sensoren, token_kurz) = {
        let register = lauf.register.lock().expect("Register-Mutex");
        (
            register.sensoren_snapshot(jetzt_ms()),
            lauf.session_token.chars().take(8).collect::<String>(),
        )
    };
    if let Some(grund) = aggregat_sperrgrund(&sensoren) {
        return Err(format!("Aggregat gesperrt: {grund}"));
    }
    let dokument = aggregat::aggregat_bauen(
        &sensoren,
        filter_profil.as_deref(),
        filter_pid,
        &broker_version(),
        &token_kurz,
        jetzt_ms(),
    );
    let pfad = aggregat::aggregat_schreiben(&dokument)?;
    Ok(pfad.display().to_string())
}
