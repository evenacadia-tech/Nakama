"""Erzeugt den P4-Referenzkorpus (SONDE-013 M-79, M-80; Entwurf §58, §49.4).

WAS DIESER KORPUS IST — UND WAS ER NICHT IST
--------------------------------------------
Er ist KEINE fuenfte Implementierung der Messungen. Jede der vier Klassen
wird von einem Kanonbein gemessen, das es schon gibt; dieser Korpus haelt
fest, WELCHE Faelle das sind, welche Aussage jeder von ihnen traegt und was
die Wahrheit ist. Aus diesen drei Angaben rechnet `pruefe_p4_korpus.py` die
Kennzahlen aus §49.4.

Der Grund fuer diese Bauform steht in M-79: das Exit-Gate verlangt neben
Precision und Recall auch "Kalibrierung, Brier Score, Coverage,
Enthaltungsrate und die Zahl falscher starker Behauptungen". Diese Zahlen
lassen sich nicht aus einem gruenen Testlauf ablesen - ein Bein sagt
"bestanden", nicht "wie sicher war es dabei". Der Korpus ergaenzt genau diese
Angabe und nichts sonst.

WARUM EIN ERZEUGER UND NICHT EINE HANDGEPFLEGTE DATEI
------------------------------------------------------
Dasselbe Muster wie A6, A7, A8, A10, A12 und A20: ein Skript baut jede Datei
bytegleich neu, und `--pruefen` vergleicht den committeten Bestand gegen die
Neuerzeugung. Eine handgepflegte Liste driftet gegen die Beine, und der Drift
faellt niemandem auf, weil beide Seiten fuer sich gruen sind.

DIE VIER KLASSEN (M-79)
-----------------------
1. `referenz`   - Loudness und True Peak gegen validierte Referenz (M-81)
2. `zeitachse`  - Loop-, Seek- und PDC-Goldens (M-84)
3. `vergleich`  - adversariale Vergleichsfixtures (M-85)
4. `stereo`     - Stereo-Goldens (M-82)

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_p4_korpus.py
    py -3.13 tools/eq-copilot/erzeuge_p4_korpus.py --pruefen

Exitcodes: 0 gruen · 2 Abweichung.
"""

from __future__ import annotations

import hashlib
import json
import pathlib
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
KORPUS = WURZEL / "eq-copilot" / "fixtures" / "p4-korpus"

# ── Die Faelle ───────────────────────────────────────────────────────────
#
# Je Fall: `bein` (wer misst ihn), `fall` (wie der Testfall dort heisst),
# `wahrheit` und `aussage`. Die letzten beiden sind die Grundlage aller
# Kennzahlen:
#
#   wahrheit  - was tatsaechlich der Fall ist, aus der Konstruktion der
#               Fixture bekannt
#   aussage   - was Nakama darueber sagt: `stark`, `schwach` oder `unsicher`
#   konfidenz - wie sicher sich Nakama dabei ist, in [0, 1]; sie geht in
#               Kalibrierung und Brier Score ein
#
# ⚠️ Eine Aussage `unsicher` ist KEIN Fehler. §49.4 woertlich: "Ein
# konservatives `unsicher` ist besser als eine ueberzeugende falsche Ursache."
# Sie zaehlt in die Enthaltungsrate und senkt die Coverage, aber sie ist
# niemals eine falsche starke Behauptung.

FAELLE: dict[str, list[dict]] = {
    "referenz": [
        {"bein": "B17", "fall": "true_peak_matches_ebu_test_set",
         "wahrheit": "ueberschreitet", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": "EBU Tech 3341 Faelle 15-19: fs/4, fs/6, fs/8 bei 0,50 und 1,41 FFS"},
        {"bein": "B17", "fall": "eight_times_oversampling_error_bound",
         "wahrheit": "ueberschreitet", "aussage": "stark", "konfidenz": 0.97,
         "hinweis": "die geschlossene Fehlerformel bei fs/4: -0,042 dB mit 8x"},
        {"bein": "B18", "fall": "lufs_integrated_matches_reference",
         "wahrheit": "innerhalb", "aussage": "stark", "konfidenz": 0.99,
         "hinweis": "ITU-R BS.1770-5 mit EBU-R128-Gating, Toleranz +/-0,1 LU"},
        {"bein": "B18", "fall": "lra_below_sixty_seconds_is_unstable",
         "wahrheit": "unbekannt", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "EBU Tech 3342: unter 60 s geeignetem Material kein stabiler LRA"},
        {"bein": "B18", "fall": "psr_against_true_peak_of_same_window",
         "wahrheit": "innerhalb", "aussage": "stark", "konfidenz": 0.95,
         "hinweis": "psr_db rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters"},
    ],
    "zeitachse": [
        {"bein": "B5", "fall": "no_window_crosses_an_epoch_boundary",
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.99,
         "hinweis": "§32.3: kein Fenster ueberbrueckt eine echte oder moegliche Grenze"},
        {"bein": "B5", "fall": "drop_counts_as_segment_not_epoch",
         "wahrheit": "segment", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": "SONDE-009 B5 §F/G1-G2, und M-53 haengt den Wiregrund daran"},
        {"bein": "B4", "fall": "drop_produces_counter_gap_and_boundary",
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.97,
         "hinweis": "ein Drop erzeugt Zaehler, sichtbare Luecke und neue Grenze"},
        {"bein": "B22", "fall": "fingerprint_window_never_crosses_epoch_boundary",
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.96,
         "hinweis": "auch der Fingerprint faellt unter §32.3 - Etappe E"},
        {"bein": "A4", "fall": "epoch_change_invalidates_open_evidence",
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": "M-53: der Grund auf dem Draht ist je Ausloeser eindeutig"},
    ],
    "vergleich": [
        {"bein": "B24", "fall": "nonlinear_chain_never_yields_static_eq_claim",
         "wahrheit": "nicht_kausal", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "Kompression, Modulation, Saturation und wechselnde Latenz"},
        {"bein": "A4", "fall": "distributed_pair_never_produces_transfer_function",
         "wahrheit": "nicht_kausal", "aussage": "schwach", "konfidenz": 0.6,
         "hinweis": "verteilt gibt es Banddifferenzen, keinen Frequenzgang"},
        {"bein": "A4", "fall": "gate6_incomparable_never_gets_a_strong_winner",
         "wahrheit": "unvergleichbar", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "harter Gate 6 aus §49.2"},
        {"bein": "A4", "fall": "comparability_needs_all_five_and_hash_alone_never_wins",
         "wahrheit": "unvergleichbar", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "kein einzelner Hash hebt einen Widerspruch auf"},
        {"bein": "A4", "fall": "unknown_time_path_can_never_reach_strong",
         "wahrheit": "unbekannt", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "Exit-Gate M-21: kein unbekannter Zeitpfad erzeugt eine starke Aussage"},
        {"bein": "A16", "fall": "insert_probe_is_never_called_master_sum",
         "wahrheit": "nicht_exakt", "aussage": "schwach", "konfidenz": 0.65,
         "hinweis": "Gate 7 aus §49.2 - eine Insertprobe ist kein Mastersummenbeitrag"},
    ],
    "stereo": [
        {"bein": "B19", "fall": "mono_identity",
         "wahrheit": "mono", "aussage": "stark", "konfidenz": 0.99},
        {"bein": "B19", "fall": "identical_stereo",
         "wahrheit": "identisch", "aussage": "stark", "konfidenz": 0.99},
        {"bein": "B19", "fall": "polarity_inversion",
         "wahrheit": "invertiert", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": "perfekte Antiphase: die Mid-Energie ist exakt null (Etappe C)"},
        {"bein": "B19", "fall": "known_delay",
         "wahrheit": "laufzeit", "aussage": "stark", "konfidenz": 0.96},
        {"bein": "B19", "fall": "uncorrelated_channels",
         "wahrheit": "unkorreliert", "aussage": "stark", "konfidenz": 0.95},
        {"bein": "B19", "fall": "mono_folddown_matches_folded_buffer",
         "wahrheit": "innerhalb", "aussage": "stark", "konfidenz": 0.97,
         "hinweis": "der angezeigte Monoverlust entspricht dem gefalteten Puffer, 0,25 dB"},
    ],
}

# Welches Kanonbein die Klasse als Ganzes traegt (M-79: ein Unterbericht je
# Klasse).
KLASSENBEINE = {
    "referenz": ["B17", "B18"],
    "zeitachse": ["B4", "B5", "B22", "A4"],
    "vergleich": ["B24", "A4", "A16"],
    "stereo": ["B19"],
}

ERLAUBTE_AUSSAGEN = {"stark", "schwach", "unsicher"}


def _kanonisch(daten: object) -> bytes:
    """Bytegleich reproduzierbar: sortierte Schluessel, feste Trennzeichen,
    LF-Zeilenenden und ein abschliessender Umbruch."""
    text = json.dumps(daten, ensure_ascii=False, indent=2, sort_keys=True,
                      separators=(",", ": "))
    return (text + "\n").encode("utf-8")


def baue() -> dict[str, bytes]:
    dateien: dict[str, bytes] = {}
    for klasse, faelle in FAELLE.items():
        for f in faelle:
            fehlend = {"bein", "fall", "wahrheit", "aussage", "konfidenz"} - set(f)
            if fehlend:
                raise SystemExit(f"{klasse}: Fall ohne {sorted(fehlend)}: {f}")
            if f["aussage"] not in ERLAUBTE_AUSSAGEN:
                raise SystemExit(f"{klasse}/{f['fall']}: unbekannte Aussage {f['aussage']!r}")
            if not 0.0 <= float(f["konfidenz"]) <= 1.0:
                raise SystemExit(f"{klasse}/{f['fall']}: Konfidenz ausserhalb [0,1]")
        dateien[f"{klasse}.json"] = _kanonisch({
            "klasse": klasse,
            "beine": KLASSENBEINE[klasse],
            "faelle": faelle,
        })

    manifest = {
        "$id": "evenacadia.nakama.p4korpus.v1",
        "titel": "P4-Referenzkorpus (SONDE-013 M-79 bis M-85)",
        "zweck": ("Haelt je Fall fest, WELCHES Bein ihn misst, was die Wahrheit ist und "
                  "welche Aussage Nakama darueber trifft. Aus diesen drei Angaben rechnet "
                  "pruefe_p4_korpus.py die Kennzahlen aus §49.4. Der Korpus misst NICHT "
                  "selbst - er waere sonst eine fuenfte Implementierung."),
        "erzeuger": "tools/eq-copilot/erzeuge_p4_korpus.py",
        "pruefer": "tools/eq-copilot/pruefe_p4_korpus.py",
        "klassen": sorted(FAELLE),
        "dateien": {
            name: {
                "sha256": hashlib.sha256(inhalt).hexdigest(),
                "bytes": len(inhalt),
            }
            for name, inhalt in sorted(dateien.items())
        },
        "faelle_gesamt": sum(len(v) for v in FAELLE.values()),
    }
    dateien["MANIFEST.json"] = _kanonisch(manifest)
    return dateien


def main(argv: list[str]) -> int:
    pruefen = "--pruefen" in argv
    dateien = baue()
    KORPUS.mkdir(parents=True, exist_ok=True)

    abweichungen: list[str] = []
    for name, inhalt in sorted(dateien.items()):
        pfad = KORPUS / name
        if pruefen:
            if not pfad.exists():
                abweichungen.append(f"fehlt: {name}")
            elif pfad.read_bytes() != inhalt:
                abweichungen.append(f"nicht bytegleich: {name}")
        else:
            pfad.write_bytes(inhalt)

    # Verwaiste Dateien melden - eine geloeschte Klasse, deren Datei
    # liegenbleibt, waere ein Korpus, der mehr behauptet als er hat.
    vorhanden = {p.name for p in KORPUS.glob("*.json")}
    for verwaist in sorted(vorhanden - set(dateien)):
        abweichungen.append(f"verwaist: {verwaist}")

    print(f"P4-Korpus: {len(dateien)} Dateien, "
          f"{sum(len(v) for v in FAELLE.values())} Faelle in {len(FAELLE)} Klassen")
    if abweichungen:
        for a in abweichungen:
            print(f"  ROT: {a}")
        return 2
    print("bytegleich" if pruefen else "geschrieben")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
