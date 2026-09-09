//! PRE/POST-Paare: wie stark die Aussage ueber ein Paar sein darf.
//!
//! Zwei Sensoren bilden ein Paar, wenn beide dieselbe Paarkennung melden. Wie
//! stark die Aussage ueber dieses Paar ist, haengt an der Ausrichtung ihrer
//! Projektfenster — und die Abstufung ist bewusst konservativ:
//!
//! * **Ausgerichtet** nur bei deckungsgleichen Fenstern derselben Epoche.
//! * **Herabgestuft** bei Spruengen oder Teilueberdeckung; ohne Ueberlappung
//!   bleibt es unklar.
//! * **Hoechstens wahrscheinlich** ueber Prozessgrenzen hinweg — zwei
//!   Prozesse teilen keine Transportuhr.
//! * **Unklar MIT GRUND** bei einem v1-Partner, der die noetigen Felder gar
//!   nicht sendet. Ein stilles „unklar" waere hier eine Auskunft weniger.
//!
//! ⚠️ Die Fensterrechnung saettigt an den i64-Raendern, statt ueberzulaufen:
//! ein Ueberlauf ergaebe eine schoene Zahl ohne Bedeutung.
//!
//! Daneben stehen hier die zwei Sperrgruende, die dieselbe Sensormenge lesen:
//! die laufende Hoermarkierung und ein aktives Aggregat.

use crate::register::SensorEintrag;
use serde::Serialize;
use std::collections::HashMap;

/// PRE/POST-Paar-Auswertung (Plan §5.7): bei JEDER Unsicherheit wird
/// herabgestuft — »ausgerichtet« ist eine Fenster-Aussage, keine samplegenaue
/// Kausalitätsbehauptung (PDC-Schätzung existiert in M2 nicht).
#[derive(Debug, Clone, Serialize)]
pub struct PaarStatus {
    pub pair_id: String,
    pub pre_sensor_id: Option<String>,
    pub post_sensor_id: Option<String>,
    /// "vollstaendig" | "unvollstaendig"
    pub status: String,
    /// "ausgerichtet" | "wahrscheinlich" | "unklar"
    pub timing: String,
    /// Warum diese Stufe — nie leer.
    pub grund: String,
}

pub(crate) fn hoermarkierungs_sperrgrund(sensoren: &[SensorEintrag]) -> Option<String> {
    sensoren.iter().find(|s| s.hoermarkierung).map(|s| {
        let name = if s.label.is_empty() { &s.sensor_id } else { &s.label };
        if s.markierung_unaufloesbar {
            format!(
                "Hör-Markierung an »{name}« ist nach zu vielen unbestätigten Instanzen nicht mehr eindeutig zuordenbar"
            )
        } else if !s.verbunden {
            format!(
                "Hör-Markierung an »{name}« wurde vor der Trennung nicht als beendet bestätigt"
            )
        } else if s.stale {
            format!(
                "Hör-Markierung an »{name}« meldet sich nicht mehr — ihr Ende ist nicht bestätigt"
            )
        } else {
            format!(
                "Hör-Markierung an mindestens einer Instanz von »{name}« ist aktiv oder ihr Ende nicht bestätigt — fremde Messung ist pausiert"
            )
        }
    })
}

pub(crate) fn aggregat_sperrgrund(sensoren: &[SensorEintrag]) -> Option<String> {
    hoermarkierungs_sperrgrund(sensoren).or_else(|| {
        sensoren
            .iter()
            .find(|s| s.messung_gesperrt_durch_hoermarkierung)
            .map(|sensor| {
                format!(
                    "Messung von »{}« kann Hör-Markierung enthalten — bitte neu messen",
                    sensor.label
                )
            })
    })
}

pub fn paare_auswerten(sensoren: &[SensorEintrag]) -> Vec<PaarStatus> {
    let mut nach_paar: HashMap<&str, (Vec<&SensorEintrag>, Vec<&SensorEintrag>)> = HashMap::new();
    for s in sensoren {
        let Some(pair) = s.pair_id.as_deref().filter(|p| !p.is_empty()) else {
            continue;
        };
        let eintrag = nach_paar.entry(pair).or_default();
        match s.role.as_str() {
            "pre" => eintrag.0.push(s),
            "post" => eintrag.1.push(s),
            _ => {}
        }
    }

    let mut ergebnis: Vec<PaarStatus> = Vec::new();
    for (pair_id, (pres, posts)) in nach_paar {
        if pres.is_empty() && posts.is_empty() {
            continue;
        }
        let mut p = PaarStatus {
            pair_id: pair_id.to_string(),
            pre_sensor_id: pres.first().map(|s| s.sensor_id.clone()),
            post_sensor_id: posts.first().map(|s| s.sensor_id.clone()),
            status: "unvollstaendig".into(),
            timing: "unklar".into(),
            grund: String::new(),
        };

        if pres.len() > 1 || posts.len() > 1 {
            let rolle = if pres.len() > 1 { "VORHER" } else { "NACHHER" };
            p.grund = format!(
                "mehrere {rolle}-Messpunkte teilen dieselbe Paar-Kennung — bitte eindeutig machen"
            );
            ergebnis.push(p);
            continue;
        }
        let (pre, post) = match (pres.first(), posts.first()) {
            (Some(a), Some(b)) => (*a, *b),
            (Some(_), None) => {
                p.grund = "der NACHHER-Messpunkt fehlt".into();
                ergebnis.push(p);
                continue;
            }
            (None, Some(_)) => {
                p.grund = "der VORHER-Messpunkt fehlt".into();
                ergebnis.push(p);
                continue;
            }
            (None, None) => unreachable!(),
        };
        p.status = "vollstaendig".into();

        // Harte Ausschlüsse → unklar.
        let unklar = |grund: String, p: &mut PaarStatus| {
            p.timing = "unklar".into();
            p.grund = grund;
        };
        if let Some(grund) = hoermarkierungs_sperrgrund(sensoren) {
            unklar(grund, &mut p);
            ergebnis.push(p);
            continue;
        }
        if pre.messung_gesperrt_durch_hoermarkierung || post.messung_gesperrt_durch_hoermarkierung {
            unklar(
                "Messung nach Hör-Markierung gesperrt — betroffene Messpunkte bitte neu messen"
                    .into(),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        }
        if !pre.verbunden || !post.verbunden {
            let wer = if !pre.verbunden { "VORHER" } else { "NACHHER" };
            unklar(format!("{wer}-Messpunkt ist getrennt"), &mut p);
            ergebnis.push(p);
            continue;
        }
        if pre.stale || post.stale {
            let wer = if pre.stale { "VORHER" } else { "NACHHER" };
            unklar(
                format!("{wer}-Messpunkt sendet nicht mehr (letztes Lebenszeichen zu alt)"),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        }
        if pre.samplerate != post.samplerate {
            unklar(
                "die beiden Messpunkte laufen mit verschiedenen Samplerates".into(),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        }
        let (Some(mp), Some(mq)) = (pre.messung.as_ref(), post.messung.as_ref()) else {
            let wer = if pre.messung.is_none() { pre } else { post };
            unklar(
                if wer.protokoll_version <= 1 {
                    format!("»{}« liefert keine Messdaten (altes Plugin, v1)", wer.label)
                } else {
                    format!("»{}« hat noch keinen Messstand gemeldet", wer.label)
                },
                &mut p,
            );
            ergebnis.push(p);
            continue;
        };
        if mp.zustand != "messbereit" || mq.zustand != "messbereit" {
            let wer = if mp.zustand != "messbereit" {
                pre
            } else {
                post
            };
            unklar(
                format!("»{}« sammelt noch — Messung nicht belastbar", wer.label),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        }
        let (Some(fp), Some(fq)) = (mp.projekt_fenster, mq.projekt_fenster) else {
            unklar(
                "keine Projektzeit-Information — Messung lief ohne Transport?".into(),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        };
        let len_p = fp.bis_samples.saturating_sub(fp.von_samples);
        let len_q = fq.bis_samples.saturating_sub(fq.von_samples);
        if len_p <= 0 || len_q <= 0 {
            unklar(
                "Messung lief ohne laufenden Transport — keine Projektzeit-Zuordnung".into(),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        }
        // Projektzeiten sind volle i64-Vertragswerte. Zwei gueltige Fenster an
        // entgegengesetzten Zahlenraendern duerfen weder im Debug-Build paniken
        // noch im Release-Build umbrechen und dadurch als deckungsgleich gelten.
        let overlap = fp
            .bis_samples
            .min(fq.bis_samples)
            .saturating_sub(fp.von_samples.max(fq.von_samples));
        if overlap <= 0 {
            unklar(
                "die Messfenster überlappen nicht — vermutlich verschiedene Passagen".into(),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        }

        // Herabstufungen → wahrscheinlich (Plan §5.7 wörtlich: »pre/post wahrscheinlich«).
        let kuerzer = len_p.min(len_q);
        let mut wahrscheinlich: Option<String> = None;
        if fp.spruenge > 0 || fq.spruenge > 0 {
            wahrscheinlich = Some(
                "Loop-/Seek-Sprünge im Messfenster — Passagen nicht sicher deckungsgleich".into(),
            );
        } else if (overlap as f64) < 0.8 * kuerzer as f64 {
            wahrscheinlich = Some("die Messfenster decken sich nur teilweise".into());
        } else if pre.host_pid != post.host_pid {
            wahrscheinlich = Some(
                "die Messpunkte laufen in verschiedenen Prozessen (Bridge oder zweites FL?)".into(),
            );
        } else {
            let (a, g) = (mp.aktiv_s.min(mq.aktiv_s), mp.aktiv_s.max(mq.aktiv_s));
            if g > 0.0 && (g - a) > 0.1 * g {
                wahrscheinlich = Some("die aktive Messzeit unterscheidet sich deutlich (Smart Disable oder Stille auf einem Punkt?)".into());
            }
        }
        match wahrscheinlich {
            Some(grund) => {
                p.timing = "wahrscheinlich".into();
                p.grund = grund;
            }
            None => {
                p.timing = "ausgerichtet".into();
                p.grund = "Messfenster deckungsgleich und ohne Sprünge".into();
            }
        }
        ergebnis.push(p);
    }
    ergebnis.sort_by(|a, b| a.pair_id.cmp(&b.pair_id));
    ergebnis
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testhilfe::*;
    use crate::{jetzt_ms, Register};

    // ── Paar-Auswertung (Plan §5.7: Herabstufung bei jeder Unsicherheit) ──

    #[test]
    fn paar_ausgerichtet_bei_deckungsgleichen_fenstern() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare.len(), 1);
        assert_eq!(paare[0].status, "vollstaendig");
        assert_eq!(paare[0].timing, "ausgerichtet", "Grund: {}", paare[0].grund);
    }

    #[test]
    fn paar_wird_bei_spruengen_herabgestuft() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 480000, 2)), 10.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "wahrscheinlich");
        assert!(
            paare[0].grund.contains("Sprünge"),
            "Grund war: {}",
            paare[0].grund
        );
    }

    #[test]
    fn paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar() {
        let mut r = Register::default();
        paar_basis(&mut r);
        // 50 % Überlappung → wahrscheinlich.
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 100_000, 0)), 10.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(50_000, 150_000, 0)), 10.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "wahrscheinlich");
        // Keine Überlappung → unklar.
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(500_000, 600_000, 0)), 10.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "unklar");
        assert!(paare[0].grund.contains("überlappen nicht"));
    }

    #[test]
    fn paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(i64::MIN, i64::MIN + 4096, 0)), 1.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(i64::MAX - 4096, i64::MAX, 0)), 1.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "unklar");
        assert!(paare[0].grund.contains("überlappen nicht"));
    }

    #[test]
    fn paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund() {
        let mut r = Register::default();
        r.sensor_verbinden(
            &hello("s-pre", "pre", "PIANO VORHER", Some("paar-1"), 1),
            "n-p",
        );
        let mut alt = hello("s-post", "post", "PIANO NACHHER", Some("paar-1"), 1);
        alt.protocol_version = 1;
        r.sensor_verbinden(&alt, "n-q");
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "unklar");
        assert!(
            paare[0].grund.contains("v1"),
            "Grund war: {}",
            paare[0].grund
        );
    }

    #[test]
    fn paar_ohne_partner_ist_unvollstaendig() {
        let mut r = Register::default();
        r.sensor_verbinden(
            &hello("s-pre", "pre", "PIANO VORHER", Some("paar-1"), 1),
            "n-p",
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare.len(), 1);
        assert_eq!(paare[0].status, "unvollstaendig");
        assert!(paare[0].grund.contains("NACHHER"));
    }

    #[test]
    fn paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-pre", "pre", "V", Some("paar-1"), 111), "n-p");
        r.sensor_verbinden(&hello("s-post", "post", "N", Some("paar-1"), 222), "n-q");
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "wahrscheinlich");
        assert!(paare[0].grund.contains("Prozessen"));
    }
}
