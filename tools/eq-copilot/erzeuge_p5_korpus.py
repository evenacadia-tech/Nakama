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
    # 🔑 NAK-212 N-34 (07.09.2026): die Menge waechst um GENAU ZWEI Werte und
    # bleibt geschlossen (M-67). KEIN Wert `kein_zusammenhang` — eine Quelle
    # ohne belegten Zusammenhang ist entweder `keine_ursache` (sie draengt
    # nicht) oder `distraktor` (sie draengt, ist aber nicht kausal); ein
    # dritter Name fuer dieselben zwei Lagen waere eine Wahrheit ohne Fall.
    "gegenbeleg",
    "routing_unbekannt",
]

# Die vollstaendige Wahrheitsmenge: `keine_starke` plus die eine Wahrheit, auf
# der eine starke Aussage ZULAESSIG ist. Ohne sie waere jeder Riegel trivial.
WAHRHEITEN = KEINE_STARKE + ["wahre_ursache"]

# Die zehn Ausschlussgruende aus M-87. Der fuenfte Spiegel der geschlossenen
# Menge: er riegelt jede Sitzungserwartung gegen sie und schreibt sie als
# `mengen.ausschlussgruende` in das Korpus-MANIFEST. NAK-213 (Fassung 4)
# haengt die letzten zwei AN DAS ENDE - dieselbe Reihenfolge wie in Vertrag,
# Rust und C++, weil `wire()`/`aus_wire()` ueber den Index gehen.
AUSSCHLUSSGRUENDE = [
    "coverage_fehlt",
    "alignment_falsch",
    "passage_unvergleichbar",
    "passage_zu_kurz",
    "intent_veto_geschuetzt",
    "intent_veto_verschmolzen",
    "capability_fehlt",
    "evidenz_zurueckgenommen",
    "screening_ueberboten",
    "master_duplikat",
]

# Die drei Zusammenhangskomponenten aus E1 — die geschlossene Menge, aus der
# eine Sitzung ihre Vorbedingungen nennen darf (NAK-212 Nacharbeit 1, NR-04).
# Sie tragen dieselben Namen wie die Rangkomponenten im Snapshot, weil das
# Kettenbein sie genau dort liest.
ZUSAMMENHANGSKOMPONENTEN = ["uplift", "koinzidenz", "wiederholbarkeit"]

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
    master_reihe_db: list[float] | None = None,
    master_onsets: list[float] | None = None,
    passage: dict | None = None,
    vorbedingung: list[str] | None = None,
    erwartete_sicherheit: str,
    erwarteter_ausschluss: str | None = None,
    distraktor_ist_alternative: bool = False,
    luecke: dict | None = None,
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
    if wahrheit not in WAHRHEITEN:
        raise SystemExit(f"{kennung}: unbekannte Wahrheit {wahrheit!r}")
    if luecke is not None and luecke.get("wirkung") not in ("ausgenommen", "benannt"):
        raise SystemExit(f"{kennung}: Luecke ohne gueltige `wirkung`")
    if master_reihe_db is not None and len(master_reihe_db) != master_fenster:
        raise SystemExit(f"{kennung}: Masterreihe passt nicht zur Fensterzahl")
    # 🔑 NAK-212 Nacharbeit 1, NR-04: eine Zusage der Form „NUR X trennt"
    # braucht den Nachweis, dass alles Uebrige wirklich gegeben ist. Die
    # genannten Zusammenhangskomponenten muessen im Lauf positiv sein, sonst
    # faellt der Fall an einer anderen Regel und misst etwas anderes.
    for k in vorbedingung or []:
        if k not in ZUSAMMENHANGSKOMPONENTEN:
            raise SystemExit(f"{kennung}: unbekannte Vorbedingung {k!r}")
    return {
        "kennung": kennung,
        "wahrheit": wahrheit,
        "ursachenklasse": ursachenklasse,
        # 🔑 NAK-212 E7: die Reihe steht IMMER ausgeschrieben in der Datei.
        # `anhebung_db` ist nur die Kurzform des Erzeugers; eine Kurzform in
        # der Fixture liesse sich von zwei Lesern verschieden aufloesen.
        "master": {
            "anhebung_db": master_db,
            "fenster": master_fenster,
            "reihe_db": master_reihe_db
            if master_reihe_db is not None
            else [master_db] * master_fenster,
            "onsets": master_onsets,
        },
        "passage": passage,
        "vorbedingung": vorbedingung,
        "luecke": luecke,
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
    anhebung_db: float = 9.0,
    fenster: int = 12,
    mixer: int | None = 3,
    versatz_fenster: int = 0,
    reihe_db: list[float] | None = None,
    onsets: list[float] | None = None,
    wahre_ursache: bool = False,
    distraktor: bool = False,
) -> dict:
    """Eine Quelle der Sitzung.

    `versatz_fenster` verschiebt ihre Projektzeit gegen den Master — so
    entsteht die verschobene Passage aus M-23, ohne eine zweite Zeitachse zu
    erfinden. `mixer = None` heisst „Routing unbekannt" (M-22).
    """
    if reihe_db is not None and len(reihe_db) != fenster:
        raise SystemExit(f"Instanz {instanz}: Reihe passt nicht zur Fensterzahl")
    if onsets is not None and len(onsets) != fenster:
        raise SystemExit(f"Instanz {instanz}: Onsetreihe passt nicht zur Fensterzahl")
    return {
        "instanz": instanz,
        "anhebung_db": anhebung_db,
        "fenster": fenster,
        "mixer": mixer,
        "versatz_fenster": versatz_fenster,
        # 🔑 NAK-212 E7 — der Grund, warum es diese Felder gibt:
        # mit KONSTANTER Anhebung liegt jedes Fenster auf oder ueber dem
        # eigenen Median, die Vergleichsmenge „ohne die Quelle" bleibt leer,
        # und der bedingte Uplift ist nach M-19 nicht messbar. Im ganzen
        # bisherigen Korpus war deshalb KEINE Zusammenhangskomponente von null
        # verschieden — auch nicht bei der wahren Ursache. Ohne diese Form ist
        # R1 keine erfuellbare Regel und die Sicherheit `hoch` ein totes
        # Element im Datenweg.
        "reihe_db": reihe_db if reihe_db is not None else [anhebung_db] * fenster,
        "onsets": onsets,
        "wahre_ursache": wahre_ursache,
        # NR-14 (Nacharbeit 1, 07.09.2026): die Distraktorquelle steht
        # AUSDRUECKLICH im Korpus. Der NAK-190-Nachweis gilt nur, wenn eine
        # `alternatives`-ID auf einen existenten Befund GENAU dieser Quelle
        # aufloest; "irgendeine andere" waere kein Nachweis.
        "distraktor": distraktor,
    }


# ── Die Pegel- und Onsetreihen (NAK-212 E7) ─────────────────────────────
#
# ⚠️ Das LETZTE Fenster ist in `WECHSEL` immer laut. `masteranomalie` liest
# das juengste Masterfenster INNERHALB der Passage beziehungsweise das
# juengste ueberhaupt; waere es leise, faende sie eine andere Bandgruppe, und
# der Befund zeigte auf ein Band, in dem die Quelle nichts tut.
WECHSEL = [0.0, 9.0] * 6
GEGEN = [9.0, 0.0] * 6
# Fuer den Gegenbeleg braucht der Master eine Reihe, die in BEIDEN Mengen
# misst — sonst waere seine Spanne null und der Uplift nicht normierbar.
WECHSEL_MITTE = [4.0, 12.0] * 6
ONSETS = [1.0, 2.5, 4.0, 2.5] * 3
ONSETS_GEGEN = [4.0, 2.5, 1.0, 2.5] * 3
# ── Die beiden Randwertfaelle (7 und 8 Fenster) ─────────────────────────
#
# 🔑 **NAK-212 Nacharbeit 1, NR-04.** `WECHSEL7` lief gegen `WECHSEL`
# GEGENlaeufig: die Quelle war genau dann laut, wenn der Master leise war,
# der bedingte Uplift also negativ. Der Fall blieb schon an R1/R2 `mittel`
# und mass NICHT die zugesagte Gegenprobe „nur die Fensterzahl trennt".
#
# Gleichlaeufig UND messbar geht bei beiden Laengen nur in der Phase „laut
# zuerst": bei `[0,9,0,9,0,9,0]` (sieben Werte, VIER leise) faellt der Median
# auf den leisen Wert, `k >= schwelle` steckt alle Fenster in `mit`, `ohne`
# bleibt leer, und M-19 misst gar nichts. Bei `[9,0,9,0,9,0,9]` liegt der
# Median auf dem lauten Wert, der Split ist 4 zu 3, und der Uplift ist
# maximal positiv — genau wie bei acht Fenstern mit dem Split 4 zu 4.
#
# Der Master der beiden Sitzungen laeuft deshalb in derselben Phase. Sein
# LETZTES Fenster bleibt laut (`masteranomalie` liest genau dieses), was das
# Alternieren einmal am Ende bricht — hinter dem letzten Kandidatenfenster,
# also ausserhalb jeder gemessenen Groesse.
WECHSEL_RANDWERT = [9.0, 0.0] * 5 + [9.0, 9.0]
WECHSEL8 = [9.0, 0.0] * 4
WECHSEL7 = [9.0, 0.0, 9.0, 0.0, 9.0, 0.0, 9.0]

SITZUNGEN: list[dict] = [
    sitzung(
        "wahrer_kandidat",
        wahrheit="wahre_ursache",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, anhebung_db=9.0, wahre_ursache=True)],
        erwartete_sicherheit="mittel",
        hinweis=("Eine Quelle mit bekanntem Routing, zwoelf zusammenhaengenden "
                 "Fenstern und KONSTANTER Bandanhebung. "
                 "🔑 **NAK-212 R1/E7 (07.09.2026): die Erwartung sinkt von "
                 "`hoch` auf `mittel`.** Sie IST die wahre Ursache — aber es "
                 "gibt kein Fenster OHNE sie, also ist der bedingte Uplift "
                 "nach M-19 nicht messbar, und die Onsetreihe ist konstant. "
                 "Ohne einen einzigen Zusammenhangsbeleg traegt der Fall keine "
                 "starke Aussage. Das ist die vom Gate-Text gewollte "
                 "Verschaerfung — die Faelle, auf denen `hoch` ERLAUBT ist, "
                 "heissen seit NAK-212 `zusammenhang_uplift`, "
                 "`zusammenhang_koinzidenz`, `getrennt_durch_zusammenhang`, "
                 "`g5_fenster_genau_acht` und `passage_traegt`."),
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
    # ═════════════════════════════════════════════════════════════════════
    # NAK-212 (07.09.2026): ZUSAMMENHANG — die Faelle, auf denen `hoch`
    # ueberhaupt erreichbar ist
    # ═════════════════════════════════════════════════════════════════════
    #
    # Ohne sie waere R1 keine erfuellbare Regel: im Bestandskorpus ist keine
    # einzige Zusammenhangskomponente von null verschieden, auch nicht bei
    # `wahrer_kandidat`. Ein Produkt, das NIE `hoch` sagt, erfuellt jeden
    # Riegel trivial — und die Kalibrierung maesse nichts.
    sitzung(
        "zusammenhang_uplift",
        wahrheit="wahre_ursache",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=WECHSEL, wahre_ursache=True)],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="hoch",
        hinweis=("Quelle und Master sind in denselben Fenstern laut und sonst "
                 "leise. Damit teilt `upliftreihe` die Masterfenster in eine "
                 "Menge MIT und eine OHNE aktive Quelle — der bedingte Uplift "
                 "aus M-19 ist messbar und positiv. Das LETZTE Fenster ist "
                 "laut, damit `masteranomalie` dasselbe Band findet."),
    ),
    sitzung(
        "zusammenhang_koinzidenz",
        wahrheit="wahre_ursache",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, anhebung_db=9.0, onsets=ONSETS, wahre_ursache=True)],
        master_onsets=ONSETS,
        erwartete_sicherheit="hoch",
        hinweis=("Konstanter Pegel, aber die ONSETS laufen gleich. R1 verlangt "
                 "MINDESTENS EINE Komponente; ohne diesen Fall waere die Regel "
                 "von `beide noetig` nicht zu unterscheiden — und `hoch` in "
                 "jeder Sitzung ohne Onsetereignisse unerreichbar."),
    ),
    sitzung(
        "gegenbeleg_uplift",
        wahrheit="gegenbeleg",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=GEGEN)],
        master_reihe_db=WECHSEL_MITTE,
        erwartete_sicherheit="mittel",
        hinweis=("Die Quelle ist laut, WENN der Master leise ist. Bis NAK-212 "
                 "kostete das exakt so viel wie `keine Angabe` — nichts —, und "
                 "der zweiseitige Bootstrap belohnte die Stabilitaet des "
                 "Gegenlaufs sogar (G-D5: Rang 0,4358)."),
    ),
    sitzung(
        "gegenbeleg_koinzidenz",
        wahrheit="gegenbeleg",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, anhebung_db=9.0, onsets=ONSETS_GEGEN)],
        master_onsets=ONSETS,
        erwartete_sicherheit="mittel",
        hinweis="Dasselbe in der Onsetspur: eine gegenlaeufige Spur ist ein Gegenbeleg (R2).",
    ),
    sitzung(
        "routing_unbekannt",
        wahrheit="routing_unbekannt",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=WECHSEL, mixer=None)],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="mittel",
        hinweis=("`mixer: null` heisst `Routing unbekannt` (M-22) — die "
                 "Routingqualitaet ist 0 und die Aussage gedeckelt. Der Fall "
                 "war im Korpus nie vertreten, obwohl der Erzeuger ihn "
                 "vorsieht (G5-Befund E-H3): `Routing unbekannt` war nur im "
                 "Rustbein gemessen, nie in der Kalibrierung."),
    ),
    sitzung(
        "getrennt_durch_zusammenhang",
        wahrheit="wahre_ursache",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, reihe_db=WECHSEL, wahre_ursache=True),
            quelle(3, anhebung_db=9.0, mixer=4, distraktor=True),
        ],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="hoch",
        distraktor_ist_alternative=True,
        hinweis=("Nur EINE der beiden Quellen traegt einen belegten Uplift. "
                 "Der Abstand ist gross genug, dass sich auch die "
                 "quantisierten GESAMTRAENGE unterscheiden — beide Bedingungen "
                 "aus E6 halten, und genau EIN Befund darf stark sein (M-21)."),
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
    # ═════════════════════════════════════════════════════════════════════
    # G5-GEGENBEISPIELE (Phasengate G5, adversarialer Lauf 07.09.2026,
    # uebernommen mit NAK-212 R6)
    # ═════════════════════════════════════════════════════════════════════
    #
    # Sie stehen NICHT als Produktzusage hier, sondern als Angriff: jede
    # Sitzung ist so gebaut, dass der zu pruefende Riegel wirklich unter Last
    # steht und nicht ein trivialer Vorriegel (zu wenig Fenster, Routing
    # unbekannt) vorher greift. Drei von ihnen tragen eine GEDRUCKTE LUECKE
    # (`luecke`) — sie liefern bewusst weiter eine falsche starke Behauptung,
    # weil erst NAK-213 sie schliesst. Der Pruefer nimmt sie aus den
    # Kennzahlen und meldet sie als offene Luecke: nicht gruen, nicht rot,
    # BENANNT (Muster NAK-190).
    sitzung(
        "g5_distraktor_mehr_fenster",
        wahrheit="distraktor",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, reihe_db=WECHSEL, wahre_ursache=True),
            quelle(3, reihe_db=WECHSEL + WECHSEL[:4], fenster=16, mixer=4, distraktor=True),
        ],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="mittel",
        distraktor_ist_alternative=True,
        hinweis=("G5 Fall 1: derselbe Verlauf, aber der Distraktor traegt MEHR "
                 "Fenster. `gemeinsame_reihen` iteriert ueber die Kandidaten- "
                 "fenster und ueberspringt jedes ohne Masterpartner — die vier "
                 "ueberzaehligen gehen in keine der drei Zusammenhangsgroessen "
                 "ein. Materialmenge trennt nicht (R3)."),
    ),
    sitzung(
        "g5_distraktor_lauter",
        wahrheit="distraktor",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, reihe_db=WECHSEL, wahre_ursache=True),
            quelle(3, reihe_db=[x + 0.1 if x > 0 else 0.1 for x in WECHSEL],
                   mixer=4, distraktor=True),
        ],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="mittel",
        distraktor_ist_alternative=True,
        hinweis=("G5 Fall 7: die kleinste im Gitter darstellbare Trennung "
                 "(0,1 dB) liegt weit ueber RANG_QUANTUM und trennt die "
                 "GESAMTRAENGE. Die Zusammenhangskomponenten trennt sie nicht "
                 "— die Bandpassung sagt, WO eine Quelle Energie hat, nicht OB "
                 "sie die Ursache ist (G-H3)."),
    ),
    sitzung(
        "g5_distraktor_allein_im_rennen",
        wahrheit="distraktor",
        ursachenklasse="quelle_resonanz",
        quellen=[
            quelle(2, reihe_db=WECHSEL, versatz_fenster=6, wahre_ursache=True),
            quelle(3, reihe_db=WECHSEL, mixer=4, distraktor=True),
        ],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="mittel",
        erwarteter_ausschluss="alignment_falsch",
        hinweis=("G5 Fall 2: der wahre Verursacher faellt am Alignment. Die "
                 "Sitzung traegt bewusst eine Pegelreihe, damit der Distraktor "
                 "einen belegten Zusammenhang hat und die Zusage wirklich "
                 "beisst — R1 wuerde ihn sonst schon senken. "
                 "🔑 **NAK-213 R2:** `alignment_falsch` ist einer der fuenf "
                 "MESSgruende; der einzige Ueberlebende ist deshalb nicht "
                 "`getrennt`, sondern hoechstens `mittel`. Bis NAK-213 war "
                 "diese Sitzung als Luecke AUSGENOMMEN — sie zaehlt wieder in "
                 "allen Kennzahlen mit."),
    ),
    sitzung(
        "g5_unbeteiligte_quelle",
        wahrheit="keine_ursache",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, anhebung_db=0.0)],
        erwartete_sicherheit="mittel",
        hinweis=("G5 eigene Angriffsidee 1: die Quelle traegt im Befundband "
                 "GAR KEINE Anhebung. Uplift, Koinzidenz und Wiederholbarkeit "
                 "sind null — es gibt keinen Beleg fuer einen Zusammenhang. "
                 "⚠️ Die Erwartung ist `mittel`, NICHT `unklar`: die Quelle "
                 "passiert alle fuenf Gates, also ueberlebt ein Kandidat, und "
                 "`unklar` ist im Datenweg ausschliesslich die Klasse von "
                 "`enthaltung()`."),
    ),
    sitzung(
        "g5_sonde_auf_masterkanal",
        wahrheit="parent_duplikat",
        ursachenklasse="daten_reichen_nicht",
        quellen=[quelle(2, reihe_db=WECHSEL, mixer=1)],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="unklar",
        erwarteter_ausschluss="master_duplikat",
        hinweis=("G5 eigene Angriffsidee 2: die Sonde misst denselben "
                 "Mixerkanal wie der Master. Mit Pegelreihe ist ihr Uplift "
                 "gegen den Master perfekt belegt — R1 greift nicht, und die "
                 "Zusage bleibt scharf. "
                 "🔑 **NAK-213 R3:** sie faellt mit `master_duplikat` aus dem "
                 "Ranking; kein Kandidat ueberlebt, und die Sitzung enthaelt "
                 "sich mit `daten_reichen_nicht`/`unklar`. "
                 "⚠️ Kanal **1** und nicht 0: der Vertrag laesst "
                 "`host_mixer_index` erst ab 1 zu (`minimum: 1`, „Ausserhalb "
                 "gilt der Hostwert als nicht geliefert“), und nur der "
                 "permissive Testsetter liess die 0 durch. Die Kanaele 1 und 2 "
                 "sind im uebrigen Korpus frei."),
    ),
    sitzung(
        "g5_parent_partner_faellt_aus",
        wahrheit="parent_duplikat",
        ursachenklasse="quelle_resonanz",
        quellen=[
            quelle(2, reihe_db=WECHSEL, mixer=7, wahre_ursache=True),
            quelle(3, reihe_db=WECHSEL, mixer=7, versatz_fenster=6),
        ],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="mittel",
        erwarteter_ausschluss="alignment_falsch",
        hinweis=("G5 Fall 3: der Partner desselben Mixerkanals faellt am "
                 "Alignment. Die Duplikatmarke haelt trotzdem — `ids` und "
                 "`ist_parent` lesen ALLE Kandidaten, nicht nur die "
                 "Ueberlebenden."),
    ),
    sitzung(
        "g5_parent_partner_zu_wenig_fenster",
        wahrheit="parent_duplikat",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, reihe_db=WECHSEL, mixer=7, wahre_ursache=True),
            quelle(3, reihe_db=[0.0, 9.0], fenster=2, mixer=7),
        ],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="mittel",
        hinweis="G5 Fall 3: der Partner traegt zwei Fenster — zu wenig fuer eine starke Aussage.",
    ),
    sitzung(
        "g5_parent_partner_ohne_evidenz",
        wahrheit="parent_duplikat",
        ursachenklasse="quelle_resonanz",
        quellen=[
            quelle(2, reihe_db=WECHSEL, mixer=7, wahre_ursache=True),
            quelle(3, reihe_db=[], fenster=0, mixer=7),
        ],
        master_reihe_db=WECHSEL,
        erwartete_sicherheit="mittel",
        erwarteter_ausschluss="evidenz_zurueckgenommen",
        hinweis=("G5 Fall 3: der Partner ist angemeldet, traegt denselben "
                 "Mixerkanal, hat aber nie gesendet. "
                 "🔑 **NAK-213 R2/R3:** er wird Kandidat mit LEERER "
                 "Fensterfolge, faellt mit `evidenz_zurueckgenommen` und steht "
                 "als Ausschluss im Befund; zugleich setzt er die Duplikatmarke "
                 "des Partners, weil die Kanaltafel aus den CLIENTS entsteht. "
                 "Beide Wirkungen sind unabhaengig und werden im Kettenbein "
                 "einzeln gemessen (K-17, K-29)."),
    ),
    sitzung(
        "g5_fenster_genau_acht",
        wahrheit="wahre_ursache",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=WECHSEL8, onsets=ONSETS[:8], fenster=8,
                        wahre_ursache=True)],
        master_reihe_db=WECHSEL_RANDWERT,
        master_onsets=ONSETS,
        vorbedingung=["uplift", "koinzidenz"],
        erwartete_sicherheit="hoch",
        hinweis=("G5 Fall 6: der Randwert. GATE_MINDEST_FENSTER = 8, die "
                 "Klassenwahl prueft `fenster < 8` — acht sind genug (M-23: "
                 "`mindestens acht`). Die Sitzung traegt eine Pegelreihe, "
                 "sonst fiele sie an R1 und der Randwert waere nicht mehr "
                 "gemessen. Das Alignment haelt, weil `paarueberdeckung` auf "
                 "die KUERZERE Seite normiert (8 von 8). "
                 "🔑 **NAK-212 Nacharbeit 1, NR-04:** Reihe und Master "
                 "laufen jetzt in der Phase `laut zuerst`, und die Onsets "
                 "kommen dazu — dieselbe Konstruktion wie in "
                 "`g5_fenster_sieben`, damit zwischen beiden wirklich NUR "
                 "die Fensterzahl steht. Uplift und Koinzidenz sind "
                 "Vorbedingung und werden im Kettenbein gemessen."),
    ),
    sitzung(
        "g5_fenster_sieben",
        wahrheit="zu_kurze_passage",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=WECHSEL7, onsets=ONSETS[:7], fenster=7,
                        wahre_ursache=True)],
        master_reihe_db=WECHSEL_RANDWERT,
        master_onsets=ONSETS,
        vorbedingung=["uplift", "koinzidenz"],
        erwartete_sicherheit="mittel",
        hinweis=("G5 Fall 6, Gegenprobe: ein Fenster unter dem Randwert. Ohne "
                 "diese Sitzung waere `< 8` von `<= 8` nicht zu unterscheiden. "
                 "🔑 **NAK-212 Nacharbeit 1, NR-04:** die Reihe lief bis "
                 "hierher GEGEN den Master — die Quelle war laut, wenn er "
                 "leise war, der bedingte Uplift also negativ. Der Fall blieb "
                 "schon wegen R1/R2 `mittel`, unabhaengig von der "
                 "Fensterzahl, und mass die zugesagte Gegenprobe nicht. Jetzt "
                 "traegt er denselben positiven, gleichlaeufigen Zusammenhang "
                 "wie `g5_fenster_genau_acht`; `mittel` folgt allein aus "
                 "`fenster < GATE_MINDEST_FENSTER`, und Uplift wie Koinzidenz "
                 "sind Vorbedingung."),
    ),
    sitzung(
        "g5_kandidatendeckel",
        wahrheit="distraktor",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, anhebung_db=8.0, wahre_ursache=True),
            quelle(3, anhebung_db=9.0, mixer=4, distraktor=True),
            quelle(4, anhebung_db=9.1, mixer=5, distraktor=True),
            quelle(5, anhebung_db=9.2, mixer=6, distraktor=True),
            quelle(6, anhebung_db=9.3, mixer=8, distraktor=True),
            quelle(7, anhebung_db=9.4, mixer=9, distraktor=True),
        ],
        erwartete_sicherheit="mittel",
        erwarteter_ausschluss="screening_ueberboten",
        distraktor_ist_alternative=True,
        hinweis=("G5 Fall 2 / M-18: sechs Kandidaten, der wahre Verursacher ist "
                 "der leiseste. Die SICHERHEIT ist seit R3 korrekt `mittel` "
                 "(alle fuenf sind ungetrennt). "
                 "🔑 **NAK-213 R1:** der Deckel schneidet VOR Stufe B, und der "
                 "abgeschnittene Sechste traegt `screening_ueberboten` — die "
                 "fehlende Spur, die M-87 verbietet, ist geschlossen. Die sechs "
                 "Quellen tragen konstante Anhebungen 8,0 bis 9,4 dB und keine "
                 "eigene Onsetreihe; ihre Screeningraenge sind also paarweise "
                 "verschieden, der Fuenfte ist vom Sechsten getrennt, und E2 "
                 "greift hier nicht."),
    ),
    sitzung(
        "g5_gleichstand_verursacher_fuehrt",
        wahrheit="distraktor",
        ursachenklasse="zwei_quellen_konkurrenz",
        quellen=[
            quelle(2, anhebung_db=9.1, wahre_ursache=True),
            quelle(3, anhebung_db=9.0, mixer=4, distraktor=True),
        ],
        erwartete_sicherheit="mittel",
        distraktor_ist_alternative=True,
        hinweis=("G5 Fall 7, Gegenprobe: der Verursacher fuehrt um 0,1 dB. Im "
                 "G5-Lauf galt das als ZULAESSIGE starke Aussage; seit R3 ist "
                 "es keine — die Trennung war reine Bandpassung, und beide "
                 "fallen auf `mittel`. Das ist die vom Gate-Text gewollte "
                 "Verschaerfung."),
    ),
    # ═════════════════════════════════════════════════════════════════════
    # PASSAGE (NAK-212 R6/G-L1): der Passagenriegel stand im Korpus NIE unter
    # Last — `sitzung_fahren` legte keine Passage an, und `gate()` Schritt 4
    # lief im ganzen Lauf nicht. `verschobene_passage` faellt am
    # Master-Alignment, nicht an der Passage.
    # ═════════════════════════════════════════════════════════════════════
    sitzung(
        "passage_traegt",
        wahrheit="wahre_ursache",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=WECHSEL, wahre_ursache=True)],
        master_reihe_db=WECHSEL,
        passage={"von_offset": 0, "bis_offset": 6144},
        erwartete_sicherheit="hoch",
        hinweis=("Zwoelf Fensterlaengen, alle Fenster vollstaendig innerhalb. "
                 "Der Kontrollfall — ohne ihn waere jede Passagenzeile trivial "
                 "erfuellt."),
    ),
    sitzung(
        "passage_zu_kurz_echt",
        wahrheit="zu_kurze_passage",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=WECHSEL, wahre_ursache=True)],
        master_reihe_db=WECHSEL,
        passage={"von_offset": 0, "bis_offset": 3584},
        erwartete_sicherheit="unklar",
        erwarteter_ausschluss="passage_zu_kurz",
        hinweis=("Sieben Fensterlaengen: das Passagenmaterial ist 7 < 8, jeder "
                 "Kandidat faellt mit Grund, und das Ergebnis ist die "
                 "Enthaltung."),
    ),
    sitzung(
        "passage_teilweise_innerhalb",
        wahrheit="zu_kurze_passage",
        ursachenklasse="quelle_resonanz",
        quellen=[quelle(2, reihe_db=WECHSEL, wahre_ursache=True)],
        master_reihe_db=WECHSEL,
        passage={"von_offset": 256, "bis_offset": 4352},
        erwartete_sicherheit="unklar",
        erwarteter_ausschluss="passage_zu_kurz",
        hinweis=("Acht Fensterlaengen, um einen HALBEN Fensterschritt versetzt. "
                 "Vollstaendig innerhalb liegen die Fenster 1..7 — Material 7 "
                 "< 8. Nach der alten Beruehrungsregel waeren es 0..8, also 9 "
                 "≥ 8, und der Fall hielte (G5-Befund E-D3). Gate 3 und Gate "
                 "4a halten in beiden Faellen; NUR das Materialgate trennt."),
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
