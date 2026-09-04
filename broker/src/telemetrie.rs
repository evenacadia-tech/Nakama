//! Rust-Bein des FlatBuffers-Telemetrievertrags (SONDE-005b).
//!
//! Gegenstueck zu `eq-copilot/plugin/vertrag/NakamaTelemetrie.*`. Beide lesen
//! denselben Binaerkorpus und muessen ihn IDENTISCH klassifizieren — Urteil
//! und vollstaendige Verstossmenge —, gemessen gegen dasselbe
//! handgeschriebene `eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json`.
//!
//! ## Warum ein Leser noetig ist, obwohl es ein Schema gibt
//!
//! FlatBuffers prueft beim Verifizieren die STRUKTUR: Offsets zeigen in den
//! Puffer, `required`-Felder sind da, Vektorlaengen passen zum Puffer. Es
//! prueft NICHT:
//!
//! * ob ein Enumwert im deklarierten Bereich liegt (ein `ubyte` 99 kommt
//!   unbeanstandet durch `Verify` und erst der Leser sieht es),
//! * ob ein Bitflagfeld nur bekannte Bits traegt,
//! * irgendeine Beziehung zwischen zwei Feldern.
//!
//! Genau dort liegen aber die Regeln aus Entwurf §33.1: hoechstens ein Frame
//! je Quelle, Encoding passt zur Nutzlast, Bandzahl folgt aus dem Gitter,
//! Bitmap ist `ceil(n/8)` Bytes. Ein Vertrag, der sie nur in seinen
//! Kommentaren traegt, ist keiner.

use crate::generiert::nakama_telemetry_v1_generated::nakama::v_3 as fb;
use crate::transport::bootstrap::Adresse;
use std::collections::BTreeSet;

/// Ein einzelner Vertragsverstoss.
///
/// `pfad` ist ein JSON-Pointer in den Batch (`""` ist der Batch selbst),
/// `regel` einer der Namen aus der geschlossenen Liste in
/// `eq-copilot/schemas/v3/flatbuffers/README.md`. Beide Beine bilden
/// denselben Text; die Menge wird kanonisch sortiert, damit der Vergleich
/// nicht von der Auswertungsreihenfolge abhaengt.
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct Verstoss {
    pub pfad: String,
    pub regel: String,
}

/// Vollstaendig aus einem verifizierten FeatureBatch gelesene Broker-Sicht.
/// Der rohe Puffer bleibt die Wire-Wahrheit; diese kleine Sicht dient nur der
/// Quelladressierung sowie Alter-/Fenster-/Lautheitsprojektion.
#[derive(Debug, Clone, PartialEq)]
pub struct Quellframe {
    pub adresse: Adresse,
    pub sequence: u64,
    pub sample_count: u32,
    pub sample_rate: f64,
    pub lufs_i_paar: Option<(f32, f32)>,
    pub lufs_i_status: Option<u8>,
    /// NAK-68: ueber wie viel Audio DIESER Rahmen integriert wurde, in
    /// Samples je Kanal. `None` heisst "der Erzeuger sagt es nicht" und darf
    /// NICHT als 0 gelesen werden; ein gesendetes 0 ist ein Senderfehler und
    /// faellt vorher an `integration_samples_null`. Ohne dieses Feld kann ein
    /// Empfaenger "leise" nicht von "kurz gemessen" unterscheiden.
    pub integration_samples: Option<u32>,
}

/// Ergebnis der Broker-Eingangspruefung. Ein ausschliesslich kaputtes
/// LUFS-I-Paar entwertet nicht die uebrigen Framefelder: die drei optionalen
/// Lautheitsfelder werden aus einer Kopie des Puffers entfernt und genau diese
/// wieder gegen denselben Leser geprueft. Andere Verstoesse bleiben eine
/// vollstaendige Ablehnung vor Mutation.
#[derive(Debug, Clone, PartialEq)]
pub struct BrokerBatch {
    pub payload: Vec<u8>,
    pub frames: Vec<Quellframe>,
    pub lautheit_ungueltige_instance_ids: Vec<String>,
    pub verstoesse: Vec<Verstoss>,
}

impl Verstoss {
    fn neu(pfad: &str, regel: &str) -> Self {
        Verstoss {
            pfad: pfad.to_string(),
            regel: regel.to_string(),
        }
    }
}

/// Obergrenze der Eintraege je Batch.
///
/// Entwurf §53.9 nennt „bis zu 32 Sonden"; §33.1 sagt, dass Broker→Main
/// „typisch vier bis fuenf" buendelt. 32 ist damit die Systemgrenze, nicht
/// die Erwartung — ein Batch darueber ist ein Fehler, kein Ausreisser.
pub const MAX_EINTRAEGE: usize = 32;

/// Bandzahlen der beiden eingefrorenen Gitter (`schemas/v3/bandgitter/`).
pub const BAENDER_FEIN: usize = 221;
pub const BAENDER_GROB: usize = 64;

/// Plausibilitaetsgrenzen der Bandwerte, als bereits skalierte Traegerwerte.
///
/// Quelle ist `schemas/v3/quantisierung-v1.json`, Feld `plausibler_bereich_db`
/// — NICHT `bereich_db`, das ist der Traegerumfang (+/-32767). Der Unterschied
/// war ein T2-Runde-3-Befund: README und Beweismanifest nannten `bereich_db`
/// als Quelle, waehrend hier etwas ganz anderes durchgesetzt wurde.
///
/// `contract_cross_language` vergleicht diese vier Zahlen bei JEDEM Lauf mit
/// der Vertragsdatei; die C++-Seite tut dasselbe. Damit ist die Quellenangabe
/// nicht mehr eine Behauptung, sondern eine Pruefung.
pub const Q_0P1_MIN: i16 = -1440; // -144.0 dB * 10
pub const Q_0P1_MAX: i16 = 240; //   24.0 dB * 10
pub const Q_0P01_MIN: i16 = -14400; // -144.00 dB * 100
pub const Q_0P01_MAX: i16 = 2400; //   24.00 dB * 100

fn ist_hex32(s: &str) -> bool {
    s.len() == 32
        && s.bytes()
            .all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(&b))
}

// -------------------------------------------------------------- Strukturriegel

/// C++' Regel „May not point to itself" — auf der Rust-Seite nachgezogen.
///
/// T2-Runde 4, Blocker BL-A: der FlatBuffers-Verifier ist in den beiden
/// Sprachen NICHT gleich stark, und diesmal ist DIESE Seite die schwächere.
/// Gemessen an den gepinnten 25.12.19, beide aus demselben Commit:
///
/// * C++ `verifier.h:265-269`, `VerifyOffset`:
///   `// May not point to itself.` → `if (!Check(o != 0)) return 0;`
/// * Rust `verifier.rs:515-522`, `ForwardsUOffset::run_verifier`:
///   `let next_pos = offset.saturating_add(pos);` — **kein `o != 0`**.
///
/// Ein `uoffset` von 0 zeigt auf die eigene Zelle. Rust folgt dem, liest dort
/// eine vtable der Länge 0 und damit eine Tabelle OHNE JEDES FELD — also lauter
/// Defaults, die semantisch unauffällig sind.
///
/// Die Richtung ist die gefährliche: der Broker sitzt ZWISCHEN Sonde und Main.
/// Er würde so einen Batch als vollständig gültig durchreichen, und das Plugin
/// verwürfe ihn danach — die Validierung wäre keine eine Wahrheit mehr.
///
/// `VerifierOptions` kennt dafür keine Option; es gibt keinen
/// Konfigurationsausweg. Der Riegel läuft deshalb hier, NACH dem Verifier
/// (der die Grenzen schon gesichert hat) und über die von `flatc` ERZEUGTEN
/// `VT_*`-Konstanten — keine abgeschriebene Zahl, die beim nächsten Feld
/// veraltet. Dass die Liste unten VOLLSTÄNDIG ist, hält
/// `tools/eq-copilot/pruefe_fbs_feldids.py` (Pruefung 7) gegen das `.fbs`:
/// ein neues Offsetfeld ohne Riegelzeile wird dort rot, nicht erst hier.
///
/// ## Gemessen, nicht behauptet — die KLASSE, nicht ein Fall
///
/// Über alle neun gültigen Fixtures, jede 4-byte-ausgerichtete Zelle ab
/// Offset 4 einzeln auf 0 gesetzt (Zellen je Fixture = Größe/4 − 1, zusammen
/// 6215 Puffer). Beide Beine über denselben Korpus, verglichen wurde die
/// VOLLSTÄNDIGE Verstoßmenge, nicht nur gültig/ungültig:
///
/// | Stand | Puffer, die auseinanderlaufen |
/// |---|---:|
/// | ohne diesen Riegel | **143** von 6215 |
/// | mit diesem Riegel  | **0** von 6215 |
///
/// ## Was auf dem Weg WIEDER RAUSGEFLOGEN ist
///
/// Die erste Fassung trug zusätzlich eine Plausibilitätsprüfung der vtable
/// (`num_bytes() >= 4`). Sie schloss dieselben 143 Lücken — und riss **61 neue
/// auf**, in der Gegenrichtung: `alle-validity-bits`, Offset 332, ist der
/// Tabellenkopf des `Transportstempel`, Offset 296 überschreibt seine
/// `vtable_len`; beides macht die Tabelle feldlos. `Transportstempel`,
/// `Schleife` und `AbgeleiteteGrenzen` tragen **kein** `required`-Feld — für
/// C++ ist eine feldlose Tabelle dort strukturell in Ordnung: er liest lauter
/// Defaults und meldet deren semantische Folgen (`zeitbasis|enum_unbekannt`,
/// `sample_rate|sample_rate_bereich`,
/// `process_context_present|context_bit_fehlt`). Rust meldete `verifier`.
///
/// 🔑 **Ein Riegel, der STRENGER ist als das Bein, das er spiegelt, bricht
/// denselben Vertrag wie einer, der schwächer ist.** Verlangt ist dieselbe
/// Verstoßmenge — nicht dasselbe Vorzeichen des Urteils.
///
/// ## Warum die Vektorelemente keinen eigenen Riegel brauchen
///
/// `eintraege:[QuellenEintrag]` ist ein Vektor von Offsets; ein Element auf 0
/// wäre derselbe Selbstbezug. Er braucht hier trotzdem nichts: `QuellenEintrag`
/// trägt zwei `required`-Felder, und eine feldlose Tabelle fällt deshalb schon
/// in BEIDEN Verifiern. Das ist keine Herleitung, sondern Teil derselben
/// Messung — unter den 6215 bleibt kein Fall übrig.
fn offset_nicht_null(tab: &::flatbuffers::Table, slot: ::flatbuffers::VOffsetT) -> bool {
    let vo = tab.vtable().get(slot);
    if vo == 0 {
        return true; // Feld nicht vorhanden — das ist kein Selbstbezug.
    }
    let pos = tab.loc() + vo as usize;
    let buf = tab.buf();
    if pos + 4 > buf.len() {
        return false;
    }
    u32::from_le_bytes([buf[pos], buf[pos + 1], buf[pos + 2], buf[pos + 3]]) != 0
}

/// Alle 16 Offsetfelder des Vertrags, Tabelle für Tabelle.
///
/// Der gemeldete Verstoß ist absichtlich derselbe wie auf der C++-Seite
/// (`""` / `verifier`): dort fällt der Puffer im Verifier, hier gleich danach.
fn strukturriegel(batch: &fb::FeatureBatch) -> bool {
    if !offset_nicht_null(&batch._tab, fb::FeatureBatch::VT_EINTRAEGE) {
        return false;
    }

    for eintrag in batch.eintraege().iter() {
        if !offset_nicht_null(&eintrag._tab, fb::QuellenEintrag::VT_QUELLE)
            || !offset_nicht_null(&eintrag._tab, fb::QuellenEintrag::VT_FRAME)
        {
            return false;
        }

        let a = eintrag.quelle();
        for slot in [
            fb::Adresse::VT_LOGON_SID,
            fb::Adresse::VT_PROJECT_BINDING_ID,
            fb::Adresse::VT_SESSION_EPOCH,
            fb::Adresse::VT_INSTANCE_ID,
            fb::Adresse::VT_RUNTIME_NONCE,
        ] {
            if !offset_nicht_null(&a._tab, slot) {
                return false;
            }
        }

        let f = eintrag.frame();
        if !offset_nicht_null(&f._tab, fb::Frame::VT_TRANSPORT)
            || !offset_nicht_null(&f._tab, fb::Frame::VT_BAENDER)
            || !offset_nicht_null(&f._tab, fb::Frame::VT_BAND_STEREO)
            // SONDE-013 M-03: `headroom` ist das dritte Offsetfeld des Frames.
            // `Headroomverteilung` trägt selbst nur Skalare — hier endet der
            // Ast, wie bei `AbgeleiteteGrenzen` weiter unten.
            || !offset_nicht_null(&f._tab, fb::Frame::VT_HEADROOM)
        {
            return false;
        }

        // Reihenfolge ist hier tragend: erst die Zelle prüfen, dann ihr folgen.
        let tr = f.transport();
        if !offset_nicht_null(&tr._tab, fb::Transportstempel::VT_SCHLEIFE) {
            return false;
        }
        if let Some(s) = tr.schleife() {
            if !offset_nicht_null(&s._tab, fb::Schleife::VT_ABGELEITETE_GRENZEN) {
                return false;
            }
            // `AbgeleiteteGrenzen` trägt selbst kein Offsetfeld — hier endet der Ast.
        }

        let b = f.baender();
        for slot in [
            fb::Bandwerte::VT_WERTE_I16,
            fb::Bandwerte::VT_WERTE_F32,
            fb::Bandwerte::VT_GUELTIG_BITMAP,
        ] {
            if !offset_nicht_null(&b._tab, slot) {
                return false;
            }
        }
        if let Some(stereo) = f.band_stereo() {
            for slot in [
                fb::Bandwerte::VT_WERTE_I16,
                fb::Bandwerte::VT_WERTE_F32,
                fb::Bandwerte::VT_GUELTIG_BITMAP,
            ] {
                if !offset_nicht_null(&stereo._tab, slot) {
                    return false;
                }
            }
        }
    }
    true
}

/// Prueft einen rohen FeatureBatch-Puffer.
///
/// Gibt die kanonisch sortierte, doppelfreie Verstossmenge zurueck; leer
/// heisst gueltig.
pub fn pruefe(puffer: &[u8]) -> Vec<Verstoss> {
    let mut out: Vec<Verstoss> = Vec::new();

    // 1. Die Dateikennung steht an Offset 4 und ist das Erste, was ein Leser
    //    sehen kann. Ein fremder Puffer faellt hier, VOR jedem Feldzugriff.
    //
    //    Der Laengenriegel davor ist kein Zierrat: T2-Runde 3 hat gemessen,
    //    dass `feature_batch_buffer_has_identifier` bei 0..7 Bytes PANICKT
    //    (`assertion failed: data.len() >= SIZE_UOFFSET + FILE_IDENTIFIER_LENGTH`),
    //    waehrend die C++-Seite sauber `dateikennung` meldete. Ein Panic im
    //    Broker beendet den Thread, der die Pipe bedient - auch das ist keine
    //    Klassifikation.
    if puffer.len() < 8 || !fb::feature_batch_buffer_has_identifier(puffer) {
        out.push(Verstoss::neu("", "dateikennung"));
        return kanonisch(out);
    }

    // 2. Der Verifier. Ohne ihn ist jeder Feldzugriff auf einen manipulierten
    //    Puffer undefiniert - das ist der eine Punkt, an dem FlatBuffers
    //    wirklich hart ist, und er wird benutzt.
    let batch = match fb::root_as_feature_batch(puffer) {
        Ok(b) => b,
        Err(_) => {
            out.push(Verstoss::neu("", "verifier"));
            return kanonisch(out);
        }
    };

    // 3. Der Strukturriegel — die Haelfte des Verifiers, die DIESER Seite
    //    fehlt (T2-Runde 4). Siehe `strukturriegel` fuer die Messung.
    if !strukturriegel(&batch) {
        out.push(Verstoss::neu("", "verifier"));
        return kanonisch(out);
    }

    // `eintraege` ist im .fbs `required`: der Verifier hat seine Anwesenheit
    // bereits garantiert, und flatc erzeugt deshalb einen Accessor OHNE
    // Option.
    let eintraege = batch.eintraege();

    if eintraege.is_empty() {
        out.push(Verstoss::neu("/eintraege", "eintraege_leer"));
    }
    if eintraege.len() > MAX_EINTRAEGE {
        out.push(Verstoss::neu("/eintraege", "eintraege_zu_viele"));
    }

    // 4. §33.1: "Ein Batch traegt nie mehrere Frames derselben Quelle."
    //    Das kann kein Schema ausdruecken, und es ist genau der Grund, warum
    //    das Format keine zweite Wrapper-Ebene braucht.
    let mut gesehen: BTreeSet<String> = BTreeSet::new();

    for (i, eintrag) in eintraege.iter().enumerate() {
        let p = format!("/eintraege/{i}");

        // Beide `required` — siehe oben.
        let a = eintrag.quelle();
        pruefe_adresse(&a, &format!("{p}/quelle"), &mut out);
        if !gesehen.insert(a.instance_id().to_string()) {
            out.push(Verstoss::neu(
                &format!("{p}/quelle/instance_id"),
                "quelle_doppelt",
            ));
        }

        pruefe_frame(&eintrag.frame(), &format!("{p}/frame"), &mut out);
    }

    kanonisch(out)
}

pub fn gueltig(puffer: &[u8]) -> bool {
    pruefe(puffer).is_empty()
}

/// Prueft und liest genau die Information, die der Broker fuer den passiven
/// Quellen-Slice benoetigt. Der Sonde->Broker-Sonderfall mit genau einem
/// Quellframe wird im Coordinator gegen den gekoppelten Link erzwungen.
pub fn fuer_broker(puffer: &[u8]) -> Result<BrokerBatch, Vec<Verstoss>> {
    let verstoesse = pruefe(puffer);
    if verstoesse.is_empty() {
        let Some(frames) = quellframes_lesen(puffer) else {
            return Err(vec![Verstoss::neu("", "verifier")]);
        };
        return Ok(BrokerBatch {
            payload: puffer.to_vec(),
            frames,
            lautheit_ungueltige_instance_ids: Vec::new(),
            verstoesse: Vec::new(),
        });
    }

    let Some(indices) = ausschliesslich_lautheitsverstoesse(&verstoesse) else {
        return Err(verstoesse);
    };
    let batch = fb::root_as_feature_batch(puffer).map_err(|_| verstoesse.clone())?;
    let eintraege = batch.eintraege();
    // Auf der Eingangsseite ist ein Batch genau ein Quellframe. Mehrere
    // Eintraege hier zu reparieren waere gefaehrlich, weil FlatBuffers
    // Vtables zwischen Tabellen teilen darf. Der Coordinator lehnt denselben
    // Fall vor jeder Mutation mit einem klaren Brokergrund ab.
    if eintraege.len() != 1 || indices.iter().any(|index| *index != 0) {
        return Err(verstoesse);
    }
    let instance_id = eintraege.get(0).quelle().instance_id().to_owned();
    let frame_loc = eintraege.get(0).frame()._tab.loc();
    let Some(bereinigt) = lautheitsfelder_entfernen(puffer, frame_loc) else {
        return Err(verstoesse);
    };
    if !pruefe(&bereinigt).is_empty() {
        return Err(verstoesse);
    }
    let Some(frames) = quellframes_lesen(&bereinigt) else {
        return Err(verstoesse);
    };
    Ok(BrokerBatch {
        payload: bereinigt,
        frames,
        lautheit_ungueltige_instance_ids: vec![instance_id],
        verstoesse,
    })
}

fn ausschliesslich_lautheitsverstoesse(verstoesse: &[Verstoss]) -> Option<BTreeSet<usize>> {
    let mut indices = BTreeSet::new();
    for verstoss in verstoesse {
        let lautheit = match verstoss.regel.as_str() {
            "lufs_i_paar" | "lufs_i_status" | "lufs_i_status_mit_paar" => true,
            "nicht_endlich" => {
                verstoss.pfad.ends_with("/lufs_i")
                    || verstoss.pfad.ends_with("/lufs_i_unsicherheit_lu")
            }
            _ => false,
        };
        if !lautheit {
            return None;
        }
        let mut teile = verstoss.pfad.split('/');
        if teile.next() != Some("") || teile.next() != Some("eintraege") {
            return None;
        }
        let index = teile.next()?.parse::<usize>().ok()?;
        indices.insert(index);
    }
    (!indices.is_empty()).then_some(indices)
}

fn lautheitsfelder_entfernen(puffer: &[u8], frame_loc: usize) -> Option<Vec<u8>> {
    if frame_loc.checked_add(4)? > puffer.len() {
        return None;
    }
    let vtable_abstand = i32::from_le_bytes([
        puffer[frame_loc],
        puffer[frame_loc + 1],
        puffer[frame_loc + 2],
        puffer[frame_loc + 3],
    ]);
    if vtable_abstand <= 0 {
        return None;
    }
    let vtable_loc = frame_loc.checked_sub(vtable_abstand as usize)?;
    if vtable_loc.checked_add(2)? > puffer.len() {
        return None;
    }
    let vtable_len = u16::from_le_bytes([puffer[vtable_loc], puffer[vtable_loc + 1]]) as usize;
    let mut aus = puffer.to_vec();
    for slot in [
        fb::Frame::VT_LUFS_I,
        fb::Frame::VT_LUFS_I_UNSICHERHEIT_LU,
        fb::Frame::VT_LUFS_I_STATUS,
    ] {
        let slot = slot as usize;
        if slot + 2 <= vtable_len {
            let pos = vtable_loc.checked_add(slot)?;
            if pos.checked_add(2)? > aus.len() {
                return None;
            }
            aus[pos] = 0;
            aus[pos + 1] = 0;
        }
    }
    Some(aus)
}

fn quellframes_lesen(puffer: &[u8]) -> Option<Vec<Quellframe>> {
    let batch = fb::root_as_feature_batch(puffer).ok()?;
    let mut frames = Vec::with_capacity(batch.eintraege().len());
    for eintrag in batch.eintraege().iter() {
        let quelle = eintrag.quelle();
        let frame = eintrag.frame();
        let transport = frame.transport();
        let lufs_i_paar = match (frame.lufs_i(), frame.lufs_i_unsicherheit_lu()) {
            (Some(lufs_i), Some(unsicherheit))
                if lufs_i.is_finite() && unsicherheit.is_finite() =>
            {
                Some((lufs_i, unsicherheit))
            }
            _ => None,
        };
        frames.push(Quellframe {
            adresse: Adresse {
                logon_sid: quelle.logon_sid().to_owned(),
                project_binding_id: quelle.project_binding_id().to_owned(),
                session_epoch: quelle.session_epoch().to_owned(),
                instance_id: quelle.instance_id().to_owned(),
                runtime_nonce: quelle.runtime_nonce().to_owned(),
            },
            sequence: transport.sequence(),
            sample_count: transport.sample_count(),
            sample_rate: transport.sample_rate(),
            lufs_i_paar,
            lufs_i_status: frame.lufs_i_status(),
            integration_samples: frame.integration_samples(),
        });
    }
    Some(frames)
}

fn kanonisch(v: Vec<Verstoss>) -> Vec<Verstoss> {
    let menge: BTreeSet<Verstoss> = v.into_iter().collect();
    menge.into_iter().collect()
}

fn pruefe_adresse(a: &fb::Adresse, p: &str, out: &mut Vec<Verstoss>) {
    // Die SID wird nie geparst, nur verglichen (§32.1) - geprueft wird
    // deshalb nur, dass sie ueberhaupt da ist und nicht ins Uferlose waechst.
    let sid = a.logon_sid();
    if sid.is_empty() || sid.chars().count() > 184 {
        out.push(Verstoss::neu(&format!("{p}/logon_sid"), "sid_laenge"));
    }
    for (name, wert) in [
        ("project_binding_id", a.project_binding_id()),
        ("session_epoch", a.session_epoch()),
        ("instance_id", a.instance_id()),
        ("runtime_nonce", a.runtime_nonce()),
    ] {
        if !ist_hex32(wert) {
            out.push(Verstoss::neu(&format!("{p}/{name}"), "hex32"));
        }
    }
}

fn pruefe_frame(f: &fb::Frame, p: &str, out: &mut Vec<Verstoss>) {
    if f.metrics_version() < 1 {
        out.push(Verstoss::neu(
            &format!("{p}/metrics_version"),
            "metrics_version",
        ));
    }

    pruefe_transport(&f.transport(), &format!("{p}/transport"), out);
    pruefe_baender(&f.baender(), &format!("{p}/baender"), out);
    if let Some(stereo) = f.band_stereo() {
        pruefe_band_stereo(&stereo, &format!("{p}/band_stereo"), out);
    }

    // Optionale Kennzahlen: ein nicht messbarer Wert wird WEGGELASSEN, nicht
    // als NaN gesendet (quantisierung-v1.json: Nichtendliches wird beim
    // Erzeugen zu Wert 0 mit gueltig=false). Ein NaN auf der Leitung ist
    // deshalb ein Senderfehler und wird abgelehnt, nicht sanitisiert.
    for (name, wert) in [
        ("aktivitaet", f.aktivitaet()),
        ("lufs_s", f.lufs_s()),
        ("peak_db", f.peak_db()),
        ("crest_db", f.crest_db()),
        ("psr_db", f.psr_db()),
        ("breite", f.breite()),
        ("korrelation", f.korrelation()),
        ("lufs_i", f.lufs_i()),
        ("lufs_i_unsicherheit_lu", f.lufs_i_unsicherheit_lu()),
        // SONDE-013 M-07: jede NEUE Metrik erbt denselben NaN-Riegel. Sie
        // stehen in derselben Liste und nicht in einer zweiten daneben —
        // eine zweite Liste waere der Ort, an dem ein spaeteres Feld
        // vergessen wird.
        ("lufs_m", f.lufs_m()),
        ("true_peak_db", f.true_peak_db()),
        ("true_peak_passage_db", f.true_peak_passage_db()),
        ("plr_db", f.plr_db()),
        ("lra_lu", f.lra_lu()),
        ("crest_kurz_db", f.crest_kurz_db()),
    ] {
        if let Some(x) = wert {
            if !x.is_finite() {
                out.push(Verstoss::neu(&format!("{p}/{name}"), "nicht_endlich"));
            }
        }
    }
    if let Some(k) = f.korrelation() {
        if k.is_finite() && !(-1.0..=1.0).contains(&k) {
            out.push(Verstoss::neu(
                &format!("{p}/korrelation"),
                "korrelation_bereich",
            ));
        }
    }
    if let Some(b) = f.breite() {
        if b.is_finite() && b < 0.0 {
            out.push(Verstoss::neu(&format!("{p}/breite"), "breite_negativ"));
        }
    }

    let lufs_i = f.lufs_i();
    let unsicherheit = f.lufs_i_unsicherheit_lu();
    let lufs_i_paar = matches!((lufs_i, unsicherheit),
        (Some(l), Some(u)) if l.is_finite() && u.is_finite());
    if lufs_i.is_some() != unsicherheit.is_some()
        || (lufs_i.is_some() && unsicherheit.is_some() && !lufs_i_paar)
    {
        out.push(Verstoss::neu(p, "lufs_i_paar"));
    }

    if let Some(status) = f.lufs_i_status() {
        if status != 1 && status != 2 {
            out.push(Verstoss::neu(
                &format!("{p}/lufs_i_status"),
                "lufs_i_status",
            ));
        }
        if lufs_i_paar {
            out.push(Verstoss::neu(
                &format!("{p}/lufs_i_status"),
                "lufs_i_status_mit_paar",
            ));
        }
    }

    // NAK-68 (SONDE-013): ein ANWESENDES integration_samples von 0 behauptet
    // einen Rahmen ohne Audio - das ist ein Senderfehler, kein duenner Frame.
    // ABWESENHEIT ist dagegen erlaubt und heisst "der Erzeuger sagt es nicht";
    // sie darf vom Leser nicht als 0 gelesen werden. Dieselbe Regel steht in
    // NakamaTelemetrie.cpp; ein Fixture misst sie in beiden Sprachen.
    if let Some(0) = f.integration_samples() {
        out.push(Verstoss::neu(
            &format!("{p}/integration_samples"),
            "integration_samples_null",
        ));
    }

    // ── SONDE-013 M-02 bis M-04: die Grenzen der neuen Metriken ───────────
    //
    // Jede dieser vier Regeln beschreibt einen Zustand, den ein RICHTIGER
    // Erzeuger nie herstellt. Sie stehen hier und nicht in der Anzeige, weil
    // ein Wert, den der Empfaenger erst beim Zeichnen als unmoeglich erkennt,
    // vorher schon gespeichert wurde.

    // Eine negative Loudness Range gibt es nicht: LRA ist P95 minus P10 ueber
    // derselben Verteilung, also nie kleiner als null (EBU Tech 3342).
    if let Some(lra) = f.lra_lu() {
        if lra.is_finite() && lra < 0.0 {
            out.push(Verstoss::neu(&format!("{p}/lra_lu"), "lra_negativ"));
        }
    }

    // PLR ist Passagen-True-Peak MINUS LUFS-I. Ohne das integrierte
    // Lautheitspaar gibt es den Bezugspunkt nicht — die Zahl waere gegen
    // etwas gerechnet, das der Frame nicht mitschickt (§39.1, E-A02).
    if f.plr_db().is_some() && !lufs_i_paar {
        out.push(Verstoss::neu(&format!("{p}/plr_db"), "plr_ohne_lufs_i"));
    }

    // Der True Peak liegt ZWISCHEN den Samples und ist deshalb nie KLEINER
    // als der Sample-Peak desselben Rahmens. Ein Erzeuger, bei dem er es
    // ist, hat die zwei vertauscht — und genau diese Verwechslung ist der
    // Fehler, gegen den M-02 schuetzt.
    if let (Some(tp), Some(sp)) = (f.true_peak_db(), f.peak_db()) {
        // 0,01 dB Spielraum fuer die float32-Rundung der beiden Wege.
        if tp.is_finite() && sp.is_finite() && tp < sp - 0.01 {
            out.push(Verstoss::neu(
                &format!("{p}/true_peak_db"),
                "true_peak_unter_sample_peak",
            ));
        }
    }

    // Die Headroomverteilung ist als GANZES optional; ist sie da, muss sie
    // geordnet und belegt sein. Ein leeres Fenster hiesse "Verteilung ueber
    // nichts", eine verdrehte Ordnung waere keine Verteilung.
    if let Some(h) = f.headroom() {
        for (name, wert) in [
            ("p10_db", h.p10_db()),
            ("p50_db", h.p50_db()),
            ("p95_db", h.p95_db()),
        ] {
            if !wert.is_finite() {
                out.push(Verstoss::neu(
                    &format!("{p}/headroom/{name}"),
                    "nicht_endlich",
                ));
            }
        }
        if h.fenster() == 0 {
            out.push(Verstoss::neu(
                &format!("{p}/headroom/fenster"),
                "headroom_fenster_null",
            ));
        }
        if h.p10_db().is_finite() && h.p50_db().is_finite() && h.p95_db().is_finite()
            && !(h.p10_db() <= h.p50_db() && h.p50_db() <= h.p95_db())
        {
            out.push(Verstoss::neu(
                &format!("{p}/headroom"),
                "headroom_unsortiert",
            ));
        }
    }
}

fn pruefe_transport(t: &fb::Transportstempel, p: &str, out: &mut Vec<Verstoss>) {
    // §32.3: "Die Wrapper-Bridge liefert deshalb process_context_present und
    // unabhaengige Validity-Bits; ohne Bridge gilt Projektzeit als unbewiesen."
    // Das Feld ist ein Optional und kein bool mit Default: sonst waere "der
    // Sender hat es weggelassen" ununterscheidbar von "der Host hat keinen
    // Context angelegt" — und das sind zwei verschiedene Konfidenzaussagen.
    if t.process_context_present().is_none() {
        out.push(Verstoss::neu(
            &format!("{p}/process_context_present"),
            "context_bit_fehlt",
        ));
    }
    if t.zeitbasis() == fb::Zeitbasis::unbekannt {
        out.push(Verstoss::neu(&format!("{p}/zeitbasis"), "enum_unbekannt"));
    }
    if fb::Zeitbasis::ENUM_VALUES
        .iter()
        .all(|v| *v != t.zeitbasis())
    {
        out.push(Verstoss::neu(&format!("{p}/zeitbasis"), "enum_unbekannt"));
    }
    if t.sample_count() > 1_048_576 {
        out.push(Verstoss::neu(
            &format!("{p}/sample_count"),
            "sample_count_bereich",
        ));
    }
    let sr = t.sample_rate();
    if !sr.is_finite() {
        out.push(Verstoss::neu(&format!("{p}/sample_rate"), "nicht_endlich"));
    } else if sr <= 0.0 || sr > 768_000.0 {
        out.push(Verstoss::neu(
            &format!("{p}/sample_rate"),
            "sample_rate_bereich",
        ));
    }

    // Ein gesetztes Bit ausserhalb der sieben bekannten. FlatBuffers prueft
    // das nicht; `ANY` ist die von flatc erzeugte Maske aller deklarierten.
    if t.gueltigkeit().bits() & !fb::Gueltigkeit::all().bits() != 0 {
        out.push(Verstoss::neu(
            &format!("{p}/gueltigkeit"),
            "validity_unbekanntes_bit",
        ));
    }

    let projektzeit = t.gueltigkeit().contains(fb::Gueltigkeit::project_time);
    match t.zeitbasis() {
        fb::Zeitbasis::project_samples => {
            if !projektzeit {
                out.push(Verstoss::neu(
                    &format!("{p}/gueltigkeit"),
                    "project_time_bit_fehlt",
                ));
            }
            if t.project_sample_start().is_none() {
                out.push(Verstoss::neu(
                    &format!("{p}/project_sample_start"),
                    "project_sample_start_fehlt",
                ));
            }
        }
        fb::Zeitbasis::local_monotonic => {
            if projektzeit {
                out.push(Verstoss::neu(
                    &format!("{p}/gueltigkeit"),
                    "local_project_time_bit",
                ));
            }
            if t.project_sample_start().is_some() {
                out.push(Verstoss::neu(
                    &format!("{p}/project_sample_start"),
                    "local_project_sample_start",
                ));
            }
        }
        _ => {}
    }

    if let Some(s) = t.schleife() {
        // 🔑 G1-Befund §4.3, geschlossen am 24.08.2026. `start_ppq` und
        // `end_ppq` waren die zwei von vier Fliesskomma-Traegern des Vertrags,
        // die in BEIDEN handgeschriebenen Lesern ungeprueft blieben — und
        // genau deshalb sah es kein Bein: der Kreuzsprachtest vergleicht die
        // beiden Leser miteinander, und sie waren sich einig. Er stimmte
        // ueberein, auf der falschen Antwort. Ein Vergleich zweier Spiegel
        // findet keine gemeinsame Auslassung.
        for (name, wert) in [("start_ppq", s.start_ppq()), ("end_ppq", s.end_ppq())] {
            if let Some(x) = wert {
                if !x.is_finite() {
                    out.push(Verstoss::neu(
                        &format!("{p}/schleife/{name}"),
                        "nicht_endlich",
                    ));
                }
            }
        }
        // Dieselbe Frage wie `grenzen_verdreht` eine Ebene tiefer, nur in der
        // Zeitbasis des Hosts. `bounds_valid` ist die Bedingung: ohne es
        // behauptet §32.3 ueber die Grenzen nichts, und ein Vergleich waere
        // eine erfundene Zusage.
        if s.bounds_valid() {
            if s.start_ppq().is_none() {
                out.push(Verstoss::neu(
                    &format!("{p}/schleife/start_ppq"),
                    "cycle_start_ppq_fehlt",
                ));
            }
            if s.end_ppq().is_none() {
                out.push(Verstoss::neu(
                    &format!("{p}/schleife/end_ppq"),
                    "cycle_end_ppq_fehlt",
                ));
            }
            if let (Some(a), Some(b)) = (s.start_ppq(), s.end_ppq()) {
                if a.is_finite() && b.is_finite() && b < a {
                    out.push(Verstoss::neu(&format!("{p}/schleife"), "ppq_verdreht"));
                }
            }
        }
        if let Some(g) = s.abgeleitete_grenzen() {
            let pg = format!("{p}/schleife/abgeleitete_grenzen");
            if g.herleitung() == fb::Herleitung::unbekannt
                || fb::Herleitung::ENUM_VALUES
                    .iter()
                    .all(|v| *v != g.herleitung())
            {
                out.push(Verstoss::neu(&format!("{pg}/herleitung"), "enum_unbekannt"));
            }
            if g.ende() < g.start() {
                out.push(Verstoss::neu(&pg, "grenzen_verdreht"));
            }
            if !s.bounds_valid() && g.herleitung() == fb::Herleitung::validated_block_mapping {
                out.push(Verstoss::neu(
                    &format!("{pg}/herleitung"),
                    "validated_mapping_ohne_bounds",
                ));
            }
        }
    }

    if t.gueltigkeit().contains(fb::Gueltigkeit::cycle_bounds) {
        let schleife = t.schleife();
        if schleife.as_ref().is_none_or(|s| s.start_ppq().is_none()) {
            out.push(Verstoss::neu(
                &format!("{p}/schleife/start_ppq"),
                "cycle_bounds_start_ppq_fehlt",
            ));
        }
        if schleife.as_ref().is_none_or(|s| s.end_ppq().is_none()) {
            out.push(Verstoss::neu(
                &format!("{p}/schleife/end_ppq"),
                "cycle_bounds_end_ppq_fehlt",
            ));
        }
    }
    if t.gueltigkeit().contains(fb::Gueltigkeit::continuous_time)
        && t.continuous_time_samples().is_none()
    {
        out.push(Verstoss::neu(
            &format!("{p}/continuous_time_samples"),
            "continuous_time_samples_fehlt",
        ));
    }
}

fn pruefe_baender(b: &fb::Bandwerte, p: &str, out: &mut Vec<Verstoss>) {
    let gitter = b.gitter();
    let encoding = b.encoding();

    let gitter_ok =
        gitter != fb::Bandgitter::unbekannt && fb::Bandgitter::ENUM_VALUES.contains(&gitter);
    let encoding_ok = encoding != fb::BandEncoding::unbekannt
        && fb::BandEncoding::ENUM_VALUES.contains(&encoding);
    if !gitter_ok {
        out.push(Verstoss::neu(&format!("{p}/gitter"), "enum_unbekannt"));
    }
    if !encoding_ok {
        out.push(Verstoss::neu(&format!("{p}/encoding"), "enum_unbekannt"));
    }

    let i16er = b.werte_i16();
    let f32er = b.werte_f32();

    // Genau EIN Traeger. Beide gesetzt hiesse zwei Wahrheiten ueber dieselbe
    // Messung; keiner gesetzt hiesse ein Bandsatz ohne Baender.
    match (i16er.is_some(), f32er.is_some()) {
        (true, true) => out.push(Verstoss::neu(p, "zwei_traeger")),
        (false, false) => out.push(Verstoss::neu(p, "kein_traeger")),
        _ => {}
    }

    // Und der EINE muss zur Kodierung passen. §33.1: "Empfaenger raten die
    // Skalierung nie aus dem Nachrichtentyp" - sie steht im Batch, und wenn
    // sie nicht zur Nutzlast passt, ist der Batch falsch, nicht auslegbar.
    if encoding_ok {
        let erwartet_i16 = matches!(
            encoding,
            fb::BandEncoding::q_db_0p1_i16 | fb::BandEncoding::q_db_0p01_i16
        );
        if erwartet_i16 && i16er.is_none() && f32er.is_some() {
            out.push(Verstoss::neu(p, "encoding_passt_nicht"));
        }
        if !erwartet_i16 && f32er.is_none() && i16er.is_some() {
            out.push(Verstoss::neu(p, "encoding_passt_nicht"));
        }
    }

    let anzahl = i16er.map(|v| v.len()).or_else(|| f32er.map(|v| v.len()));
    let Some(anzahl) = anzahl else { return };

    // Die Bandzahl FOLGT aus dem Gitter - die beiden Gitter sind als Zahlen
    // eingefroren (schemas/v3/bandgitter/), und ein Bandsatz, der sich nicht
    // an sie haelt, misst etwas anderes als er behauptet.
    if gitter_ok {
        let soll = match gitter {
            fb::Bandgitter::nakama_1_24_oct_30_18k_v1 => BAENDER_FEIN,
            _ => BAENDER_GROB,
        };
        if anzahl != soll {
            out.push(Verstoss::neu(p, "bandzahl"));
        }
    }

    // Bitmap: ceil(n/8) Bytes, LSB-first. Die Fuellbits des letzten Bytes
    // MUESSEN 0 sein - sonst erzeugen zwei Sender fuer dieselbe Messung zwei
    // verschiedene Puffer, und ein Bytevergleich waere keine Aussage mehr.
    // `gueltig_bitmap` ist `required` — Anwesenheit garantiert der Verifier.
    let bm = b.gueltig_bitmap();
    let soll = anzahl.div_ceil(8);
    if bm.len() != soll {
        out.push(Verstoss::neu(
            &format!("{p}/gueltig_bitmap"),
            "bitmap_laenge",
        ));
    } else if anzahl % 8 != 0 && !bm.is_empty() {
        let genutzt = (1u8 << (anzahl % 8)) - 1;
        if bm.get(bm.len() - 1) & !genutzt != 0 {
            out.push(Verstoss::neu(
                &format!("{p}/gueltig_bitmap"),
                "bitmap_fuellbits",
            ));
        }
    }

    if let Some(v) = i16er {
        if encoding_ok {
            let (min, max) = match encoding {
                fb::BandEncoding::q_db_0p01_i16 => (Q_0P01_MIN, Q_0P01_MAX),
                _ => (Q_0P1_MIN, Q_0P1_MAX),
            };
            for (i, w) in v.iter().enumerate() {
                if w < min || w > max {
                    out.push(Verstoss::neu(
                        &format!("{p}/werte_i16/{i}"),
                        "bandwert_bereich",
                    ));
                    break; // ein benannter Fall reicht; die Menge bleibt endlich
                }
            }
        }
    }
    if let Some(v) = f32er {
        for (i, w) in v.iter().enumerate() {
            if !w.is_finite() {
                out.push(Verstoss::neu(
                    &format!("{p}/werte_f32/{i}"),
                    "nicht_endlich",
                ));
                break;
            }
        }
    }
}

fn pruefe_band_stereo(b: &fb::Bandwerte, p: &str, out: &mut Vec<Verstoss>) {
    if b.gitter() != fb::Bandgitter::nakama_log64_v1 {
        out.push(Verstoss::neu(&format!("{p}/gitter"), "band_stereo_gitter"));
    }
    if b.encoding() != fb::BandEncoding::float32 {
        out.push(Verstoss::neu(
            &format!("{p}/encoding"),
            "band_stereo_encoding",
        ));
    }
    if b.werte_i16().is_some() {
        out.push(Verstoss::neu(
            &format!("{p}/werte_i16"),
            "band_stereo_werte_i16",
        ));
    }

    let werte = b.werte_f32();
    if werte.as_ref().map_or(0, |w| w.len()) != BAENDER_GROB {
        out.push(Verstoss::neu(
            &format!("{p}/werte_f32"),
            "band_stereo_bandzahl",
        ));
    }
    let bitmap = b.gueltig_bitmap();
    if bitmap.len() != 8 {
        out.push(Verstoss::neu(
            &format!("{p}/gueltig_bitmap"),
            "band_stereo_bitmap_laenge",
        ));
    }
    if let Some(werte) = werte {
        for (i, wert) in werte.iter().enumerate() {
            let gueltig = i / 8 < bitmap.len() && bitmap.get(i / 8) & (1 << (i % 8)) != 0;
            if !gueltig {
                continue;
            }
            if !wert.is_finite() {
                out.push(Verstoss::neu(
                    &format!("{p}/werte_f32/{i}"),
                    "nicht_endlich",
                ));
                break;
            }
            if !(0.0..=1.0).contains(&wert) {
                out.push(Verstoss::neu(
                    &format!("{p}/werte_f32/{i}"),
                    "band_stereo_bereich",
                ));
                break;
            }
        }
    }
    if b.saturated() {
        out.push(Verstoss::neu(
            &format!("{p}/saturated"),
            "band_stereo_saturated",
        ));
    }
}
