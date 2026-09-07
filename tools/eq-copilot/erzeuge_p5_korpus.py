"""Erzeugt den P5-Evaluationskorpus (SONDE-014 M-64 bis M-70, R2).

WAS DIESER KORPUS IST — UND WORIN ER SICH VON P4 UNTERSCHEIDET
--------------------------------------------------------------
Der P4-Korpus ist ein **Register**: er haelt fest, welches Bein einen Fall
misst und welche Aussage Nakama darueber traegt. Das genuegt fuer P4, weil
dort jede Klasse von einem eigenen Golden gemessen wird.

Fuer P5 genuegt es NICHT, und die Matrixpruefung 1 hat genau das als Defekt D2
gefunden: „der Korpus mass Hygiene, nicht die Unterscheidung". Regel **R2**
zieht daraus den Schluss — der P5-Korpus ist eine **KETTE**:

    Korpusdatei (Wahrheit)
      → `broker/tests/sonde014_p5_korpus.rs` faehrt die Sitzung durch `p1`,
        also durch DENSELBEN Produktpfad wie im Betrieb
      → `hypothese_verdrahtung.rs` rechnet
      → das Bein schreibt die TATSAECHLICH ausgegebene Hypothese als Ergebnis
      → `pruefe_p5_korpus.py` haelt Ergebnis gegen Wahrheit

Eine falsche starke Produktbehauptung aendert die Angaben in dieser Datei
NICHT — sie faellt am Vergleich. Das ist der Unterschied zwischen einem
Register und einer Kette.

WAS DIESE DATEI TROTZDEM LEISTET
---------------------------------
Sie ist der **Hygieneriegel** nach dem Muster von **A25**: der Erzeuger baut
jede Datei bytegleich neu, und `--pruefen` vergleicht den committeten Bestand
gegen die Neuerzeugung. Eine handgepflegte Liste driftet gegen die Beine, und
der Drift faellt niemandem auf, weil beide Seiten fuer sich gruen sind.

Zusaetzlich prueft der Erzeuger, dass jeder genannte Bezeichner WOERTLICH in
seiner Quelldatei steht (NAK-182 R2) — ein geloeschter oder umbenannter Fall
faellt hier und nicht erst dem Leser auf.

DIE FUENF SESSIONS (M-65)
--------------------------
1. `wahrer_kandidat`      — eine Quelle draengt wirklich; starke Aussage erlaubt
2. `korrelierter_distraktor` — zwei Quellen laufen gleich, nur eine ist die
                            Ursache; der Distraktor darf ALTERNATIVE sein,
                            nie Ursache (NAK-190, SONDE-013 M-85 Ziel 1)
3. `parent_duplikat`      — zwei Sonden auf demselben Mixerkanal; keine von
                            beiden traegt eine starke Aussage (M-22)
4. `verschobene_passage`  — die Quelle misst ein anderes Zeitfenster (M-23)
5. `zu_kurze_passage`     — zu wenig unabhaengige Evidenzfenster (M-23, R1)
6. `daten_reichen_nicht`  — kein Kandidat; die Enthaltung ist das GEWUENSCHTE
                            Ergebnis, nicht ein Fehlschlag (M-27)

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_p5_korpus.py
    py -3.13 tools/eq-copilot/erzeuge_p5_korpus.py --pruefen

Exitcodes: 0 gruen · 2 Abweichung.
"""

from __future__ import annotations

import hashlib
import json
import pathlib
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
KORPUS = WURZEL / "eq-copilot" / "fixtures" / "p5-korpus"

BROKER = "broker/tests/"

# ── Die geschlossenen Mengen ─────────────────────────────────────────────
#
# Sie reisen im MANIFEST mit; der Pruefer liest sie VON DORT, damit es sie nur
# einmal gibt (NAK-182 R1a, hier fuer P5 uebernommen).

# Die sieben Ursachenklassen aus Entwurf §8, wortgleich mit
# `$defs/ursachenklasse`.
URSACHENKLASSEN = [
    "quelle_resonanz",
    "zwei_quellen_konkurrenz",
    "effektkette_pre_post",
    "summe_auf_master",
    "peak_aus_transient",
    "stereo_aus_quelle_oder_kette",
    "daten_reichen_nicht",
]

# Die drei Sicherheitsstufen.
SICHERHEITEN = ["hoch", "mittel", "unklar"]

# Wahrheiten, auf denen KEINE starke Aussage zulaessig ist. Eine starke
# Behauptung auf einer von ihnen ist `falsche_starke` und rot.
KEINE_STARKE = [
    "distraktor",
    "parent_duplikat",
    "verschobene_passage",
    "zu_kurze_passage",
    "keine_ursache",
]

# Die acht Ausschlussgruende aus M-87.
AUSSCHLUSSGRUENDE = [
    "coverage_fehlt",
    "alignment_falsch",
    "passage_unvergleichbar",
    "passage_zu_kurz",
    "intent_veto_geschuetzt",
    "intent_veto_verschmolzen",
    "capability_fehlt",
    "evidenz_zurueckgenommen",
]

# ── Das Bandfenster, in dem die Sessions ihre Anomalie tragen ────────────
#
# Dieselben Baender wie die Beine der Etappen C bis F; das Gitter ist
# eingefroren, und eine zweite Zahl waere eine zweite Wahrheit.
ANOMALIEBAND_VON = 98
ANOMALIEBAND_BIS = 102

# Das Bein, das die Kette faehrt. Sein Bezeichner steht woertlich in der
# Datei — der Riegel unten prueft das.
KETTENBEIN = {
    "datei": BROKER + "sonde014_p5_korpus.rs",
    "bezeichner": "korpus_kette_laeuft_durch_den_produktpfad",
}


def sitzung(
    kennung: str,
    wahrheit: str,
    ursachenklasse: str,
    quellen: list[dict],
    *,
    master_db: float = 9.0,
    master_fenster: int = 12,
    erwartete_sicherheit: str,
    erwarteter_ausschluss: str | None = None,
    distraktor_ist_alternative: bool = False,
    hinweis: str = "",
) -> dict:
    """Eine synthetische Sitzung.

    `wahrheit` ist die Konstruktion des Falls, nicht die Ausgabe des Produkts.
    `erwartete_sicherheit` ist die HOECHSTE Sicherheit, die der Fall tragen
    darf — der Pruefer misst gegen sie, und eine hoehere ist `falsche_starke`.
    """
    if ursachenklasse not in URSACHENKLASSEN:
        raise SystemExit(f"{kennung}: unbekannte Ursachenklasse {ursachenklasse!r}")
    if erwartete_sicherheit not in SICHERHEITEN:
        raise SystemExit(f"{kennung}: unbekannte Sicherheit {erwartete_sicherheit!r}")
    if erwarteter_ausschluss is not None and erwarteter_ausschluss not in AUSSCHLUSSGRUENDE:
        raise SystemExit(f"{kennung}: unbekannter Ausschlussgrund {erwarteter_ausschluss!r}")
    return {
        "kennung": kennung,
        "wahrheit": wahrheit,
        "ursachenklasse": ursachenklasse,
        "master": {"anhebung_db": master_db, "fenster": master_fenster},
        "quellen": quellen,
        "erwartet": {
            "sicherheit_hoechstens": erwartete_sicherheit,
            "ausschlussgrund": erwarteter_ausschluss,
            "distraktor_ist_alternative": distraktor_ist_alternative,
        },
        "hinweis": hinweis,
    }


def quelle(
    instanz: int,
    *,
    anhebung_db: float,
    fenster: int = 12,
    mixer: int | None = 3,
    versatz_fenster: int = 0,
    wahre_ursache: bool = False,
    distraktor: bool = False,
) -> dict:
    """Eine Quelle der Sitzung.

    `versatz_fenster` verschiebt ihre Projektzeit gegen den Master — so
    entsteht die verschobene Passage aus M-23, ohne eine zweite Zeitachse zu
    erfinden. `mixer = None` heisst „Routing unbekannt" (M-22).
    """
    return {
        "instanz": instanz,
        "anhebung_db": anhebung_db,
        "fenster": fenster,
        "mixer": mixer,
        "versatz_fenster": versatz_fenster,
        "wahre_ursache": wahre_ursache,
        # NR-14 (Nacharbeit 1, 07.09.2026): die Distraktorquelle steht
        # AUSDRUECKLICH im Korpus. Der NAK-190-Nachweis gilt nur, wenn eine
        # `alternatives`-ID auf einen existenten Befund GENAU dieser Quelle
        # aufloest; "irgendeine andere" waere kein Nachweis.
        "distraktor": distraktor,
    }


SITZUNGEN: list[dict] = [
    sitzung(
        "wahrer_kandidat",
        wahrheit="wahre_ursache",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, anhebung_db=9.0, wahre_ursache=True)],
        erwartete_sicherheit="hoch",
        hinweis=("Eine Quelle mit bekanntem Routing, zwoelf zusammenhaengenden "
                 "Fenstern und derselben Bandanhebung wie der Master. Der Fall, "
                 "auf dem eine starke Aussage ERLAUBT ist — ohne ihn waere jeder "
                 "Riegel unten trivial erfuellt."),
    ),
    sitzung(
        "korrelierter_distraktor",
        wahrheit="distraktor",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, anhebung_db=9.0, wahre_ursache=True),
            quelle(3, anhebung_db=9.0, mixer=4, distraktor=True),
        ],
        erwartete_sicherheit="mittel",
        distraktor_ist_alternative=True,
        hinweis=("Zwei Quellen laufen im selben Band gleich. Der Distraktor darf "
                 "als ALTERNATIVE erscheinen (NAK-190, SONDE-013 M-85 Ziel 1), "
                 "aber nie als Ursache mit eigener starker Aussage. Hoechstens "
                 "MITTEL, und zwar fuer BEIDE: im Material ist hier nichts, was "
                 "sie trennt — wer fuehrt, entscheidet sonst nur der "
                 "Gleichstandsschluessel. Eine starke Aussage auf diesem Platz "
                 "waere richtig oder falsch je nach Lage der Kennungen, und "
                 "genau diese Muenzwurf-Behauptung verbietet §36.4 Satz 1. "
                 "Die Erwartung steht bewusst NICHT auf `hoch`: sonst maesse "
                 "der Fall, ob die wahre Ursache die kleinere Kennung hat."),
    ),
    sitzung(
        "parent_duplikat",
        wahrheit="parent_duplikat",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, anhebung_db=9.0, mixer=7, wahre_ursache=True),
            quelle(3, anhebung_db=9.0, mixer=7),
        ],
        erwartete_sicherheit="mittel",
        hinweis=("Zwei Sonden auf DEMSELBEN Mixerkanal messen dasselbe Signal. "
                 "Keine von beiden traegt eine starke Aussage (M-22)."),
    ),
    sitzung(
        "verschobene_passage",
        wahrheit="verschobene_passage",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, anhebung_db=9.0, versatz_fenster=6, wahre_ursache=True)],
        erwartete_sicherheit="unklar",
        erwarteter_ausschluss="alignment_falsch",
        hinweis=("Die Quelle misst ein um sechs Fenster verschobenes "
                 "Zeitfenster. Das Alignment reisst, und der Kandidat verlaesst "
                 "das Ranking MIT Grund (M-20, M-87)."),
    ),
    sitzung(
        "zu_kurze_passage",
        wahrheit="zu_kurze_passage",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, anhebung_db=9.0, fenster=3, wahre_ursache=True)],
        master_fenster=3,
        erwartete_sicherheit="mittel",
        hinweis=("Drei unabhaengige Fenster — unter `GATE_MINDEST_FENSTER`. "
                 "Der Fall, den D1 aufgedeckt hat: die vier relativen Gates "
                 "bestehen ihn alle, weil `ueberdeckung` auf das kuerzere "
                 "Fenster normiert. Nur die absolute Zahl faengt ihn."),
    ),
    sitzung(
        "daten_reichen_nicht",
        wahrheit="keine_ursache",
        ursachenklasse="daten_reichen_nicht",
        quellen=[],
        erwartete_sicherheit="unklar",
        hinweis=("Kein Kandidat. Die Enthaltung ist das GEWUENSCHTE Ergebnis "
                 "und zaehlt nicht als Fehlschlag (M-27, §49.4: 'Ein "
                 "konservatives unsicher ist besser als eine ueberzeugende "
                 "falsche Ursache.')."),
    ),
]


def _kanonisch(daten: dict) -> bytes:
    """LF, kein BOM, feste Schluesselordnung — wie jeder andere Erzeuger."""
    text = json.dumps(daten, ensure_ascii=False, indent=2, sort_keys=True)
    return (text + "\n").encode("utf-8")


def baue() -> dict[str, bytes]:
    kennungen = [s["kennung"] for s in SITZUNGEN]
    if len(set(kennungen)) != len(kennungen):
        raise SystemExit("doppelte Sitzungskennung")

    # NAK-182 R2: der Bezeichner des Kettenbeins steht WOERTLICH in seiner
    # Datei. Ohne diesen Riegel waere der Name ein Etikett.
    ziel = WURZEL / KETTENBEIN["datei"]
    if not ziel.is_file():
        raise SystemExit(f"Kettenbein fehlt: {KETTENBEIN['datei']}")
    if KETTENBEIN["bezeichner"] not in ziel.read_text(encoding="utf-8", errors="replace"):
        raise SystemExit(
            f"Bezeichner {KETTENBEIN['bezeichner']!r} steht nicht in "
            f"{KETTENBEIN['datei']}"
        )

    dateien: dict[str, bytes] = {
        "sitzungen.json": _kanonisch({"sitzungen": SITZUNGEN}),
    }
    manifest = {
        "$id": "evenacadia.nakama.p5korpus.v1",
        "titel": "P5-Evaluationskorpus (SONDE-014 M-64 bis M-70, R2)",
        "zweck": (
            "Eine KETTE, kein Register: jede Sitzung laeuft als Evidenzbestand "
            "durch den Produktpfad, und gelesen wird die TATSAECHLICH "
            "ausgegebene Hypothese gegen die Wahrheit des Falls. Eine falsche "
            "starke Produktbehauptung aendert diese Datei nicht — sie faellt "
            "am Vergleich."
        ),
        "erzeuger": "tools/eq-copilot/erzeuge_p5_korpus.py",
        "kettenbein": KETTENBEIN,
        "pruefer": "tools/eq-copilot/pruefe_p5_korpus.py",
        "ergebnisdatei": "eq-copilot/build/p5-korpus-ergebnis.json",
        "bandfenster": {"von": ANOMALIEBAND_VON, "bis": ANOMALIEBAND_BIS},
        "mengen": {
            "ursachenklassen": URSACHENKLASSEN,
            "sicherheiten": SICHERHEITEN,
            "keine_starke": KEINE_STARKE,
            "ausschlussgruende": AUSSCHLUSSGRUENDE,
        },
        "dateien": {
            name: {
                "sha256": hashlib.sha256(inhalt).hexdigest(),
                "bytes": len(inhalt),
            }
            for name, inhalt in sorted(dateien.items())
        },
        "sitzungen_gesamt": len(SITZUNGEN),
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

    vorhanden = {p.name for p in KORPUS.glob("*.json")}
    for verwaist in sorted(vorhanden - set(dateien)):
        abweichungen.append(f"verwaist: {verwaist}")

    print(f"P5-Korpus: {len(dateien)} Dateien, {len(SITZUNGEN)} Sitzungen")
    if abweichungen:
        for a in abweichungen:
            print(f"  ROT: {a}")
        return 2
    print("bytegleich" if pruefen else "geschrieben")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
