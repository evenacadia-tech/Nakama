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
#
# NAK-182 R1a: es gibt ZWEI geschlossene Mengen von Wahrheiten, und der
# Unterschied ist der ganze Punkt. Auf `KEINE_BEHAUPTUNG` ist JEDE Aussage
# falsch, auch die schwache (Gate 6, Exit-Gate Satz 2, M-83 Satz 3: keine
# falsche statische EQ-Behauptung, auch keine vorsichtige). Auf den zwei
# Werten, die nur in `KEINE_STARKE` stehen, traegt die SCHWACHE Aussage die
# Wahrheit sehr wohl: ein verteiltes Paar liefert Banddifferenzen, nur keinen
# Frequenzgang; eine Insertprobe liefert einen Beitrag, nur keinen exakten
# Mastersummenbeitrag. Beide Mengen stehen hier und reisen im MANIFEST mit -
# der Pruefer liest sie VON DORT, damit es sie nur einmal gibt.
#
# NAK-182 R2: jeder Fall traegt `quelle` mit Datei und woertlichem
# Bezeichner. Ohne sie war das Feld `fall` ein Etikett: von 22 Namen standen
# 8 woertlich in einem Test, und ein geloeschter oder umbenannter Fall waere
# niemandem aufgefallen.

PLUGIN = "eq-copilot/plugin/tests/"
BROKER = "broker/tests/"


def q(datei: str, bezeichner: str) -> dict:
    """Die Quelle eines Falls: Pfad ab Repo-Wurzel plus woertlicher Bezeichner."""
    return {"datei": datei, "bezeichner": bezeichner}


FAELLE: dict[str, list[dict]] = {
    "referenz": [
        {"bein": "B17", "fall": "true_peak_matches_ebu_test_set",
         "quelle": q(PLUGIN + "Sonde013TruePeakGoldenTest.cpp",
                     "true_peak_matches_ebu_test_set"),
         "wahrheit": "ueberschreitet", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": "EBU Tech 3341 Faelle 15-19: fs/4, fs/6, fs/8 bei 0,50 und 1,41 FFS"},
        # NAK-283 N01 (13.09.2026): die transienten Faelle als EIGENER Eintrag
        # (Manifest §8.1 Feinheit 15) - eine andere Referenz (im Test gerechnet
        # und validiert statt analytisch) und eine zweite Toleranz. Die
        # Konfidenz liegt deshalb eine Stufe unter 15-19.
        {"bein": "B17", "fall": "true_peak_matches_ebu_transient_set",
         "quelle": q(PLUGIN + "Sonde013TruePeakGoldenTest.cpp",
                     "true_peak_matches_ebu_transient_set"),
         "wahrheit": "ueberschreitet", "aussage": "stark", "konfidenz": 0.96,
         "hinweis": ("EBU Tech 3341 Faelle 20-23: transient, vier Abtastversaetze, "
                     "±0,1 dB gegen validierte Referenz plus Normtoleranz +0,2/−0,4 dBTP")},
        {"bein": "B17", "fall": "eight_times_oversampling_error_bound",
         "quelle": q(PLUGIN + "Sonde013TruePeakGoldenTest.cpp",
                     "eight_times_oversampling_error_bound"),
         "wahrheit": "ueberschreitet", "aussage": "stark", "konfidenz": 0.97,
         "hinweis": "die geschlossene Fehlerformel bei fs/4: -0,042 dB mit 8x"},
        {"bein": "B9", "fall": "lufs_integrated_matches_reference",
         "quelle": q(PLUGIN + "LoudnessGoldenTestMain.cpp",
                     "lufs_integrated_matches_reference"),
         "wahrheit": "innerhalb", "aussage": "stark", "konfidenz": 0.99,
         "hinweis": ("Toleranz +/-0,1 LU gegen die AUSGEBAUTE Vektorrechnung - die "
                     "zweite Implementierung, nicht ein externes Testset. M-81 sagt "
                     "zusaetzlich 'auf Standard-Testmaterial'; das ist fuer True Peak "
                     "gedeckt (B17, EBU Tech 3341), fuer LUFS-I nicht.")},
        {"bein": "B18", "fall": "lra_below_sixty_seconds_is_not_a_number",
         "quelle": q(PLUGIN + "Sonde013DynamicsTest.cpp",
                     "lra_below_sixty_seconds_is_not_a_number"),
         "wahrheit": "unbekannt", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "EBU Tech 3342: unter 60 s geeignetem Material kein stabiler LRA"},
        {"bein": "B18", "fall": "plr_psr_definitions_and_labels",
         "quelle": q(PLUGIN + "Sonde013DynamicsTest.cpp",
                     "plr_psr_definitions_and_labels"),
         "wahrheit": "innerhalb", "aussage": "stark", "konfidenz": 0.95,
         "hinweis": "psr_db rechnet gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters"},
    ],
    "zeitachse": [
        {"bein": "B5", "fall": "no_window_crosses_an_epoch_boundary",
         "quelle": q(PLUGIN + "AnalysisGoldenTestMain.cpp",
                     "no_window_crosses_an_epoch_boundary"),
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.99,
         "hinweis": "§32.3: kein Fenster ueberbrueckt eine echte oder moegliche Grenze"},
        {"bein": "B5", "fall": "drop_counts_as_segment_not_epoch",
         "quelle": q(PLUGIN + "AnalysisGoldenTestMain.cpp",
                     "drop_counts_as_segment_not_epoch"),
         "wahrheit": "segment", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": "SONDE-009 B5 §F/G1-G2, und M-53 haengt den Wiregrund daran"},
        {"bein": "B5", "fall": "impulse_time_is_stable_over_block_sizes_and_sample_rates",
         "quelle": q(PLUGIN + "AnalysisGoldenTestMain.cpp",
                     "impulse_time_is_stable_over_block_sizes_and_sample_rates"),
         "wahrheit": "zeitstabil", "aussage": "stark", "konfidenz": 0.97,
         "hinweis": ("NAK-182: der gemeldete stromSample eines bekannten Impulses ist "
                     "ueber fuenf Blockgroessen je Samplerate identisch, bei fuenf "
                     "Sampleraten, und liegt innerhalb einer Fensterlaenge (4096) vor "
                     "t0. Die Auswahl kennt t0 nicht - gemessen wird das staerkste "
                     "Flussereignis des Laufs.")},
        {"bein": "B4", "fall": "drop_produces_counter_gap_and_boundary",
         "quelle": q(PLUGIN + "QueueStressTestMain.cpp",
                     "drop_produces_counter_gap_and_boundary"),
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.97,
         "hinweis": "ein Drop erzeugt Zaehler, sichtbare Luecke und neue Grenze"},
        {"bein": "B22", "fall": "fingerprint_window_never_crosses_epoch_boundary",
         "quelle": q(PLUGIN + "Sonde013FingerprintGoldenTest.cpp",
                     "fingerprint_window_never_crosses_epoch_boundary"),
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.96,
         "hinweis": "auch der Fingerprint faellt unter §32.3 - Etappe E"},
        {"bein": "A4", "fall": "epoch_change_invalidates_open_evidence",
         "quelle": q(BROKER + "sonde013_taint.rs",
                     "epoch_change_invalidates_open_evidence"),
         "wahrheit": "getrennt", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": "M-53: der Grund auf dem Draht ist je Ausloeser eindeutig"},
    ],
    "vergleich": [
        {"bein": "B24", "fall": "nonlinear_chain_never_yields_static_eq_claim",
         "quelle": q(PLUGIN + "Sonde013PrePostGoldenTest.cpp",
                     "nonlinear_chain_never_yields_static_eq_claim"),
         "wahrheit": "zeitvariabel", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": ("Kompression, Modulation, Saturation und wechselnde Latenz - "
                     "ZEITVARIABLE Ketten DESSELBEN Quellsignals. Die Kohaerenz faellt, "
                     "und deshalb entsteht keine statische EQ-Behauptung.")},
        {"bein": "B16", "fall": "focused_evidence_wire_carries_0p01_db_steps",
         "quelle": q(PLUGIN + "Sonde013EventWireTest.cpp",
                     "focused_evidence_wire_carries_0p01_db_steps"),
         "wahrheit": "wiedergefunden", "aussage": "stark", "konfidenz": 0.96,
         "hinweis": ("NAK-182 M-83 Satz 2, C++-Haelfte: der echte Encoder schreibt "
                     "q_db_0p01_i16, aus seinem eigenen Wire-Text kommt der Gain je "
                     "Band mit Bit innerhalb +/-0,1 dB zurueck, und eine Leiter aus 20 "
                     "Stufen zu 0,01 dB ergibt 20 streng monotone Ganzzahlen im "
                     "Abstand 1.")},
        {"bein": "A4", "fall": "dekodierte_evidenz_findet_gain_mit_intervall",
         "quelle": q(BROKER + "sonde013_verdrahtung.rs",
                     "dekodierte_evidenz_findet_gain_mit_intervall"),
         "wahrheit": "wiedergefunden", "aussage": "stark", "konfidenz": 0.95,
         "hinweis": ("NAK-182 M-83 Satz 2, Rust-Haelfte: Wirepfad -> "
                     "perzentil_dekodieren -> resultatmessung -> achsen. Bandweise "
                     "+/-0,1 dB aus der Produktmessung, und das Bootstrap-Intervall "
                     "enthaelt den Gain.")},
        {"bein": "A4", "fall": "distributed_pair_never_produces_transfer_function",
         "quelle": q(BROKER + "sonde013_prepost.rs",
                     "distributed_pair_never_produces_transfer_function"),
         "wahrheit": "nicht_kausal", "aussage": "schwach", "konfidenz": 0.6,
         "hinweis": "verteilt gibt es Banddifferenzen, keinen Frequenzgang"},
        {"bein": "A4", "fall": "gate6_incomparable_never_gets_a_strong_winner",
         "quelle": q(BROKER + "sonde013_experiment.rs",
                     "gate6_incomparable_never_gets_a_strong_winner"),
         "wahrheit": "unvergleichbar", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "harter Gate 6 aus §49.2"},
        {"bein": "A4", "fall": "comparability_needs_all_five_and_hash_alone_never_wins",
         "quelle": q(BROKER + "sonde013_passage.rs",
                     "comparability_needs_all_five_and_hash_alone_never_wins"),
         "wahrheit": "unvergleichbar", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "kein einzelner Hash hebt einen Widerspruch auf"},
        {"bein": "A4", "fall": "unknown_time_path_can_never_reach_strong",
         "quelle": q(BROKER + "sonde013_prepost.rs",
                     "unknown_time_path_can_never_reach_strong"),
         "wahrheit": "unbekannt", "aussage": "unsicher", "konfidenz": 0.5,
         "hinweis": "Exit-Gate M-21: kein unbekannter Zeitpfad erzeugt eine starke Aussage"},
        {"bein": "A16", "fall": "insert_probe_is_never_called_master_sum",
         "quelle": q(PLUGIN + "SondeNullTestMain.cpp",
                     "insert_probe_is_never_called_master_sum"),
         "wahrheit": "nicht_exakt", "aussage": "schwach", "konfidenz": 0.65,
         "hinweis": "Gate 7 aus §49.2 - eine Insertprobe ist kein Mastersummenbeitrag"},
    ],
    "stereo": [
        {"bein": "B19", "fall": "sweep_mono_identity",
         "quelle": q(PLUGIN + "Sonde013StereoGoldenTest.cpp", "sweep_mono_identity"),
         "wahrheit": "mono", "aussage": "stark", "konfidenz": 0.99,
         "hinweis": ("NAK-182: ueber die 18 Blockgroessen des Sweeps. Die tiefe "
                     "Einzelmessung bei 512 steht daneben (Abschnitt mono_identity).")},
        {"bein": "B19", "fall": "sweep_identical_stereo",
         "quelle": q(PLUGIN + "Sonde013StereoGoldenTest.cpp", "sweep_identical_stereo"),
         "wahrheit": "identisch", "aussage": "stark", "konfidenz": 0.99,
         "hinweis": "breitbandiges Material, auf beiden Kanaelen bitgleich"},
        {"bein": "B19", "fall": "sweep_polarity_inversion",
         "quelle": q(PLUGIN + "Sonde013StereoGoldenTest.cpp", "sweep_polarity_inversion"),
         "wahrheit": "invertiert", "aussage": "stark", "konfidenz": 0.98,
         "hinweis": ("perfekte Antiphase: Korrelation -1 bei Kohaerenz 1, und die "
                     "Monosumme laeuft an die Vertragsgrenze")},
        {"bein": "B19", "fall": "sweep_known_delay",
         "quelle": q(PLUGIN + "Sonde013StereoGoldenTest.cpp", "sweep_known_delay"),
         "wahrheit": "laufzeit", "aussage": "stark", "konfidenz": 0.96,
         "hinweis": "acht Samples Verzoegerung; die Phase folgt +2*pi*f*tau"},
        {"bein": "B19", "fall": "sweep_uncorrelated_channels",
         "quelle": q(PLUGIN + "Sonde013StereoGoldenTest.cpp", "sweep_uncorrelated_channels"),
         "wahrheit": "unkorreliert", "aussage": "stark", "konfidenz": 0.95,
         "hinweis": "zwei unabhaengige Rauschstroeme"},
        {"bein": "B19", "fall": "sweep_folddown_within_0p25db",
         "quelle": q(PLUGIN + "Sonde013StereoGoldenTest.cpp", "sweep_folddown_within_0p25db"),
         "wahrheit": "innerhalb", "aussage": "stark", "konfidenz": 0.97,
         "hinweis": "der angezeigte Monoverlust gegen den gefalteten Puffer, 0,25 dB"},
    ],
}

# Welches Kanonbein die Klasse als Ganzes traegt (M-79: ein Unterbericht je
# Klasse).
KLASSENBEINE = {
    "referenz": ["B17", "B18", "B9"],
    "zeitachse": ["B4", "B5", "B22", "A4"],
    "vergleich": ["B24", "B16", "A4", "A16"],
    "stereo": ["B19"],
}

# ── Die zwei geschlossenen Mengen (NAK-182 R1a) ──────────────────────────
#
# Sie stehen HIER und reisen im MANIFEST mit; `pruefe_p4_korpus.py` liest sie
# von dort. Eine Kopie im Pruefer waere die zweite Wahrheit, an der C6 hing.
KEINE_BEHAUPTUNG = ("unbekannt", "unvergleichbar", "zeitvariabel")
KEINE_STARKE = KEINE_BEHAUPTUNG + ("nicht_kausal", "nicht_exakt")
SICHERE_WAHRHEITEN = ("ueberschreitet", "innerhalb", "getrennt", "segment", "mono",
                      "identisch", "invertiert", "laufzeit", "unkorreliert",
                      "zeitstabil", "wiedergefunden")
ERLAUBTE_WAHRHEITEN = KEINE_STARKE + SICHERE_WAHRHEITEN

# Was P4 ausdruecklich NICHT misst. A26 druckt jede Zeile im Kopf - eine
# Luecke, die still fehlt, ist keine ausgewiesene Luecke (Entscheid G4 §8).
NICHT_GEMESSEN = [
    {"zusage": "M-85 Ziel 1 (korrelierter, nicht kausaler Distraktor)",
     "grund": ("setzt Ursachen und Alternativen voraus und ist damit "
               "P5-Lieferumfang (Entwurf §59)"),
     "ticket": "NAK-190"},
]

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
            fehlend = {"bein", "fall", "quelle", "wahrheit", "aussage",
                       "konfidenz"} - set(f)
            if fehlend:
                raise SystemExit(f"{klasse}: Fall ohne {sorted(fehlend)}: {f}")
            if f["aussage"] not in ERLAUBTE_AUSSAGEN:
                raise SystemExit(f"{klasse}/{f['fall']}: unbekannte Aussage {f['aussage']!r}")
            # NAK-182 R1a: eine Wahrheit ausserhalb der Menge ist rot. Ohne
            # diesen Riegel entstuende beim naechsten Fall eine dritte
            # Schreibweise, und `_passt` fiele still auf "passt" zurueck.
            if f["wahrheit"] not in ERLAUBTE_WAHRHEITEN:
                raise SystemExit(f"{klasse}/{f['fall']}: unbekannte Wahrheit "
                                 f"{f['wahrheit']!r}; erlaubt: "
                                 f"{sorted(ERLAUBTE_WAHRHEITEN)}")
            if not 0.0 <= float(f["konfidenz"]) <= 1.0:
                raise SystemExit(f"{klasse}/{f['fall']}: Konfidenz ausserhalb [0,1]")
            # NAK-182 R2: die Quelle muss existieren und den Bezeichner
            # woertlich tragen. Der Erzeuger prueft es hier, damit ein
            # falscher Verweis gar nicht erst in den Korpus kommt; der
            # Pruefer misst dasselbe am committeten Bestand.
            quelle = f["quelle"]
            if set(quelle) != {"datei", "bezeichner"}:
                raise SystemExit(f"{klasse}/{f['fall']}: `quelle` braucht genau "
                                 f"`datei` und `bezeichner`, hat {sorted(quelle)}")
            ziel = WURZEL / quelle["datei"]
            if not ziel.is_file():
                raise SystemExit(f"{klasse}/{f['fall']}: Quelldatei fehlt: "
                                 f"{quelle['datei']}")
            if quelle["bezeichner"] not in ziel.read_text(encoding="utf-8",
                                                          errors="replace"):
                raise SystemExit(f"{klasse}/{f['fall']}: Bezeichner "
                                 f"{quelle['bezeichner']!r} steht nicht in "
                                 f"{quelle['datei']}")
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
        # NAK-182 R1a: die zwei geschlossenen Mengen reisen MIT. Der Pruefer
        # liest sie von hier - eine Quelle, keine Kopie im Pruefskript.
        "mengen": {
            "keine_behauptung": list(KEINE_BEHAUPTUNG),
            "keine_starke": list(KEINE_STARKE),
            "erlaubte_wahrheiten": sorted(ERLAUBTE_WAHRHEITEN),
            "erlaubte_aussagen": sorted(ERLAUBTE_AUSSAGEN),
        },
        # NAK-182 R6: was P4 ausdruecklich nicht misst. A26 druckt es.
        "nicht_gemessen": NICHT_GEMESSEN,
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
