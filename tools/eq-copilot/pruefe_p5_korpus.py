"""Das Sammelbein des P5-Exit-Gates (SONDE-014 M-64 bis M-70, M-31, R2).

WAS DIESES BEIN MISST
---------------------
Entwurf §59 Lieferumfang Punkt 6 und §36.4 verlangen fuer P5 **Precision und
Recall je Ursachenklasse**, dazu Kalibrierung, Brier Score, Coverage und
Enthaltung. Dieses Bein rechnet sie — aber es rechnet sie NICHT aus einer
Deklaration.

Es ist das dritte Glied einer **Kette** (Regel R2):

    `eq-copilot/fixtures/p5-korpus/`   die WAHRHEIT jedes Falls
      → `broker/tests/sonde014_p5_korpus.rs` faehrt jede Sitzung durch `p1`
      → `eq-copilot/build/p5-korpus-ergebnis.json`  was das PRODUKT gesagt hat
      → dieses Bein haelt beides gegeneinander

Die Matrixpruefung 1 hat als Defekt **D2** gefunden, dass der P4-Weg das nicht
leistet: dort serialisiert der Erzeuger deklarierte Werte, und eine falsche
starke Produktbehauptung aendert sie nicht. Hier aendert sie das Ergebnis, und
das Ergebnis faellt.

DIE VORAUSSETZUNG IST EINE MESSUNG, KEINE ANNAHME
--------------------------------------------------
Ohne frische Ergebnisdatei meldet dieses Bein **Exit 3 (Voraussetzung fehlt)**
statt gruen. Ein Sammelbein, das ein veraltetes Artefakt als frisch bezeugt,
ist genau der Fehler, den `tools/dirigent/pruefliste.md` Abschnitt D benennt.

DIE SCHWELLE IST AUSGABE, NICHT EINGABE (M-31, M-66)
-----------------------------------------------------
Die Abbildung hoch/mittel/unklar → handelbar wird hier **gemessen**, nicht
gesetzt: das Bein sucht die Schwelle, bei der Precision und Recall ihre Riegel
halten, und druckt sie. Wer sie als Konstante hineingaebe, bekaeme ein Bein,
das seine eigene Eingabe bestaetigt.

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_p5_korpus.py                (Sammelbein)
    py -3.13 tools/eq-copilot/pruefe_p5_korpus.py --selbsttest   (Selbsttest)

Exitcodes: 0 gruen · 2 Befund · 3 Voraussetzung fehlt.
"""

from __future__ import annotations

import json
import pathlib
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
KORPUS = WURZEL / "eq-copilot" / "fixtures" / "p5-korpus"
ERGEBNIS = WURZEL / "eq-copilot" / "build" / "p5-korpus-ergebnis.json"
# Die Quellen, deren Aenderung das Ergebnis veralten laesst.
QUELLEN = [
    WURZEL / "broker" / "src" / "coordinator" / "hypothese.rs",
    WURZEL / "broker" / "src" / "coordinator" / "hypothese_verdrahtung.rs",
    WURZEL / "broker" / "tests" / "sonde014_p5_korpus.rs",
]

HOCH, MITTEL, UNKLAR = "hoch", "mittel", "unklar"
ORDNUNG = {UNKLAR: 0, MITTEL: 1, HOCH: 2}

# ⚠️ EINE ANNAHME DER MESSUNG, KEINE PRODUKTKONSTANTE.
#
# §59 verlangt Brier Score und Kalibrierung. Beide brauchen eine
# Wahrscheinlichkeitsvorhersage — und das Produkt hat bewusst keine:
# `Befundkonfidenz` traegt `klasse` UND `score`, und `hypothese.rs`:495–497
# haelt ausdruecklich fest, dass die Klasse NIE aus dem Score gerundet wird
# (M-15/M-20). Der `score` ist `rang.rang()`, das quantisierte Mittel der
# sechs Rangkomponenten — eine RANGZAHL, keine Trefferwahrscheinlichkeit.
#
# Brier ueber den Rang zu rechnen waere ein Kategoriefehler und ergaebe eine
# Zahl, die schlecht aussieht, ohne etwas ueber die Kalibrierung zu sagen.
# Deshalb ist die Vorhersage hier die KLASSE, und diese Tabelle ist der
# Nennwert, gegen den ihre empirische Trefferquote gehalten wird. Sie lebt
# im Messwerkzeug und wird nie ins Produkt zurueckgelesen — genau das waere
# die von M-15 verbotene Rundung.
KLASSENNENNWERT = {HOCH: 0.9, MITTEL: 0.6, UNKLAR: 0.3}


# ═════════════════════════════════════════════════════════════════════════
# Die Kennzahlen — als eigene Funktionen, damit der Selbsttest sie fuettert
# ═════════════════════════════════════════════════════════════════════════


def kennzahlen(faelle: list[dict]) -> dict:
    """Precision, Recall, Brier, Kalibrierung, Coverage, Enthaltung.

    Ein `fall` traegt: `wahrheit`, `sicherheit_hoechstens`, `ist_wahre_ursache`,
    `sicherheit` (die AUSGEGEBENE), `score` und `ist_enthaltung`.

    ⚠️ Eine ENTHALTUNG ist kein Fehlschlag (M-27, §49.4: „Ein konservatives
    `unsicher` ist besser als eine ueberzeugende falsche Ursache."). Sie senkt
    die Coverage und hebt die Enthaltungsrate — sie ist nie eine falsche
    starke Behauptung.
    """
    gesamt = len(faelle)
    stark = [f for f in faelle if f["sicherheit"] == HOCH]
    enthaltungen = [f for f in faelle if f["ist_enthaltung"]]

    # FALSCHE STARKE: zwei Wege, und beide sind derselbe Fehler.
    #
    #   1. eine starke Aussage ueber eine Quelle, die NICHT die wahre Ursache
    #      ist — das ist der Distraktor aus M-21;
    #   2. eine starke Aussage in einer Sitzung, die hoechstens `mittel`
    #      tragen darf — das ist das Parent-Duplikat aus M-22 und die
    #      verschobene oder zu kurze Passage aus M-23.
    falsche_starke = [
        f for f in stark
        if not f["ist_wahre_ursache"]
        or ORDNUNG[f["sicherheit_hoechstens"]] < ORDNUNG[HOCH]
    ]
    # FALSCHE SCHWACHE: eine Aussage `mittel`, wo der Fall gar keine traegt.
    falsche_schwache = [
        f for f in faelle
        if f["sicherheit"] == MITTEL
        and ORDNUNG[f["sicherheit_hoechstens"]] < ORDNUNG[MITTEL]
    ]

    richtige_starke = len(stark) - len(falsche_starke)
    # Zaehler und Nenner lesen DIESELBE Menge — die Lehre aus NAK-182 C6.
    moegliche_starke = len([
        f for f in faelle
        if f["ist_wahre_ursache"] and ORDNUNG[f["sicherheit_hoechstens"]] >= ORDNUNG[HOCH]
    ])
    precision = richtige_starke / len(stark) if stark else 1.0
    recall = richtige_starke / moegliche_starke if moegliche_starke else 1.0

    # Brier und Kalibrierung NUR ueber die Behauptungen. Eine Enthaltung ist
    # keine Wahrscheinlichkeitsvorhersage; sie mit 0,5 hineinzuziehen
    # bestrafte die Vorsicht, die §49.4 verlangt.
    #
    # Vorhergesagt wird die KLASSE ueber `KLASSENNENNWERT`, nicht der `score`
    # — der ist die Rangzahl und keine Wahrscheinlichkeit (siehe dort).
    behauptungen = [f for f in faelle if not f["ist_enthaltung"]]
    n = len(behauptungen)
    treffer = [1.0 if _passt(f) else 0.0 for f in behauptungen]
    nennwerte = [KLASSENNENNWERT[f["sicherheit"]] for f in behauptungen]
    brier = (sum((p - t) ** 2 for p, t in zip(nennwerte, treffer)) / n) if n else 0.0
    mittlere = (sum(nennwerte) / n) if n else 0.0
    trefferquote = (sum(treffer) / n) if n else 0.0
    # Die Zuverlaessigkeitstafel: je Klasse ihr Nennwert gegen die GEMESSENE
    # Trefferquote. Sie ist annahmefrei lesbar — „von dem, was das Produkt
    # `hoch` nannte, waren X % richtig" — und macht sichtbar, wo eine
    # Fehlkalibrierung sitzt, statt sie in einer Gesamtzahl zu mitteln.
    zuverlaessigkeit = {}
    for stufe in (HOCH, MITTEL, UNKLAR):
        dieser = [(p, t) for p, t in zip(nennwerte, treffer)
                  if abs(p - KLASSENNENNWERT[stufe]) < 1e-12]
        if dieser:
            zuverlaessigkeit[stufe] = {
                "n": len(dieser),
                "nennwert": KLASSENNENNWERT[stufe],
                "trefferquote": sum(t for _, t in dieser) / len(dieser),
            }
    return {
        "zuverlaessigkeit": zuverlaessigkeit,
        # Der Rang bleibt sichtbar (§42.4), aber getrennt von der Kalibrierung.
        "rangmittel": (sum(float(f["score"]) for f in behauptungen) / n) if n else 0.0,
        "faelle": gesamt,
        "starke_behauptungen": len(stark),
        "falsche_starke": len(falsche_starke),
        "falsche_schwache": len(falsche_schwache),
        "enthaltungen": len(enthaltungen),
        "enthaltungsrate": len(enthaltungen) / gesamt if gesamt else 0.0,
        "coverage": (gesamt - len(enthaltungen)) / gesamt if gesamt else 0.0,
        "precision": precision,
        "recall": recall,
        "brier": brier,
        "kalibrierung": abs(mittlere - trefferquote),
        "namen_falscher_starker": [f.get("kennung", "?") for f in falsche_starke],
        "namen_falscher_schwacher": [f.get("kennung", "?") for f in falsche_schwache],
    }


def _passt(fall: dict) -> bool:
    """Traegt die Wahrheit die ausgegebene Sicherheit?"""
    if ORDNUNG[fall["sicherheit"]] > ORDNUNG[fall["sicherheit_hoechstens"]]:
        return False
    if fall["sicherheit"] == HOCH and not fall["ist_wahre_ursache"]:
        return False
    return True


def riegel(k: dict, wo: str) -> list[str]:
    """Die vier Riegel aus M-64 — als EIGENE Funktion.

    NAK-182 MP1-6: `precision > 1` und `recall > 1` sind ueber den Korpusweg
    strukturell unerreichbar. Eine Wache ohne ausfuehrbaren Negativtest ist
    keine gemessene Zusage — deshalb steht die Auswertung hier, wird von
    `main` auf die ECHTEN Kennzahlen jeder Ursachenklasse angewandt und vom
    Selbsttest mit synthetischen Dicts gefuettert. Ein Riegel, der nur im
    Selbsttest existierte, waere die zweite Wahrheit.
    """
    befunde: list[str] = []
    if k["falsche_starke"] > 0:
        befunde.append(f"{wo}: {k['falsche_starke']} falsche starke Behauptung(en): "
                       f"{', '.join(k['namen_falscher_starker'])}")
    if k["falsche_schwache"] > 0:
        befunde.append(f"{wo}: {k['falsche_schwache']} falsche schwache Behauptung(en): "
                       f"{', '.join(k['namen_falscher_schwacher'])}")
    if not 0.0 <= k["precision"] <= 1.0:
        befunde.append(f"{wo}: precision {k['precision']} liegt ausserhalb [0,1]")
    if not 0.0 <= k["recall"] <= 1.0:
        befunde.append(f"{wo}: recall {k['recall']} liegt ausserhalb [0,1]")
    if not 0.0 <= k["brier"] <= 1.0:
        befunde.append(f"{wo}: brier {k['brier']} liegt ausserhalb [0,1]")
    if not 0.0 <= k["kalibrierung"] <= 1.0:
        befunde.append(f"{wo}: kalibrierung {k['kalibrierung']} liegt ausserhalb [0,1]")
    return befunde


def schwelle_suchen(faelle: list[dict]) -> dict:
    """M-31/M-66: die Schwelle ist AUSGABE, nicht Eingabe.

    Gesucht wird die niedrigste Sicherheitsstufe, ab der `handelbar` gilt und
    die Riegel halten. Sie wird gemessen, nicht gesetzt — ein Bein, dem man
    die Schwelle hineingibt, bestaetigt seine eigene Eingabe.
    """
    ergebnis = {}
    for stufe in (HOCH, MITTEL, UNKLAR):
        handelbar = [f for f in faelle if ORDNUNG[f["sicherheit"]] >= ORDNUNG[stufe]]
        falsch = [f for f in handelbar if not _passt(f)]
        ergebnis[stufe] = {
            "handelbar": len(handelbar),
            "davon_falsch": len(falsch),
            "haelt": len(falsch) == 0,
        }
    gewaehlt = next((s for s in (UNKLAR, MITTEL, HOCH) if ergebnis[s]["haelt"]), None)
    return {"je_stufe": ergebnis, "niedrigste_haltende": gewaehlt}


def produktschwelle(faelle: list[dict]) -> list[str]:
    """M-31: die handelbare Schwelle des PRODUKTS, an der Kette gelesen.

    M-31 faellt, wenn die Schwelle „als Literal im Editor" steht. Das Produkt
    trifft die Entscheidung an genau EINER Stelle — `zustand_aus_sicherheit`
    in `hypothese.rs`:611–622 — und die Kette liest sie hier zurueck:
    handelbar (`ready_to_send`) ist genau `hoch`, nichts darunter.

    ⚠️ Geprueft wird eine AEQUIVALENZ, nicht eine Richtung. Nur „kein
    handelnder Befund unter `hoch`" zu pruefen liesse ein Produkt durch, das
    ueberhaupt nichts mehr handelt; nur „jeder `hoch` handelt" liesse eines
    durch, das auch `mittel` handelt. Beide Richtungen sind Rueckschritte.
    Ein veralteter Befund ist die benannte Ausnahme: er traegt `stale` und
    handelt nicht, egal wie sicher er war.
    """
    frisch = [f for f in faelle if f["zustand"] != "stale"]
    zu_locker = [f for f in frisch if f["handelbar"] and f["sicherheit"] != HOCH]
    zu_streng = [f for f in frisch if f["sicherheit"] == HOCH and not f["handelbar"]]
    befunde: list[str] = []
    if zu_locker:
        befunde.append(
            f"{len(zu_locker)} handelnde(r) Befund(e) unter `hoch`: "
            + ", ".join(f"{f.get('kennung', '?')}/{f['sicherheit']}" for f in zu_locker)
            + " — die Schwelle waere geraten, nicht kalibriert (M-31)"
        )
    if zu_streng:
        befunde.append(
            f"{len(zu_streng)} Befund(e) mit `hoch` handeln nicht: "
            + ", ".join(f.get("kennung", "?") for f in zu_streng)
            + " — dann traegt `hoch` seine Zusage nicht mehr (M-31)"
        )
    return befunde


def p4_luecke(nicht_gemessen: list[dict], messende: list[str]) -> tuple[list[str], str]:
    """M-70: die gedruckte Luecke des P4-Korpus gegen ihre P5-Messung.

    NAK-190 hat im P4-Manifest eine Zeile `nicht_gemessen` hinterlassen: der
    korrelierte, nicht kausale Distraktor (SONDE-013 M-85 Ziel 1) war dort
    nicht messbar. M-70 sagt, sie werde **erst dann entfernt, wenn der
    P5-Korpus den Fall wirklich misst**; ein Entfernen ohne Messung ist ein
    Rueckschritt hinter NAK-182.

    Der Riegel haelt genau diese Kopplung — in beide Richtungen:
    verschwindet die Zeile, ohne dass eine P5-Sitzung den Fall misst, ist das
    rot. Steht sie noch, ist das nie rot; die Zeile ist dann hoechstens
    ueberfaellig, und das sagt die Ausgabe.
    """
    offen = [e for e in nicht_gemessen if e.get("ticket") == "NAK-190"]
    if not offen and not messende:
        return ([
            "die Luecke NAK-190 ist aus dem P4-Manifest verschwunden, ohne dass "
            "eine P5-Sitzung den korrelierten Distraktor misst — ein Rueckschritt "
            "hinter NAK-182 (M-70)"
        ], "entfernt, ungemessen")
    if not offen:
        return ([], f"entfernt und gemessen durch {', '.join(messende)}")
    if messende:
        return ([], f"gedruckt, inzwischen gemessen durch {', '.join(messende)} — "
                    "sie darf jetzt entfernt werden")
    return ([], "gedruckt und noch ungemessen")


# ═════════════════════════════════════════════════════════════════════════
# Der Weg ueber die Kette
# ═════════════════════════════════════════════════════════════════════════


def _voraussetzung() -> tuple[dict, dict, list[str]]:
    """Korpus und Ergebnis lesen; Frische pruefen."""
    fehlt: list[str] = []
    for pfad in (KORPUS / "MANIFEST.json", KORPUS / "sitzungen.json"):
        if not pfad.is_file():
            fehlt.append(f"Korpusdatei fehlt: {pfad.relative_to(WURZEL)}")
    if not ERGEBNIS.is_file():
        fehlt.append(
            f"Ergebnisdatei fehlt: {ERGEBNIS.relative_to(WURZEL)} — sie entsteht in "
            "`cargo test --test sonde014_p5_korpus` (Bein A4)"
        )
    if fehlt:
        return {}, {}, fehlt
    stand = ERGEBNIS.stat().st_mtime
    for quelle in QUELLEN + [KORPUS / "sitzungen.json"]:
        if quelle.is_file() and quelle.stat().st_mtime > stand:
            fehlt.append(
                f"Ergebnis ist AELTER als {quelle.relative_to(WURZEL)} — der Lauf "
                "misst ein veraltetes Artefakt"
            )
    if fehlt:
        return {}, {}, fehlt
    manifest = json.loads((KORPUS / "MANIFEST.json").read_text(encoding="utf-8"))
    sitzungen = json.loads((KORPUS / "sitzungen.json").read_text(encoding="utf-8"))
    ergebnis = json.loads(ERGEBNIS.read_text(encoding="utf-8"))
    return {"manifest": manifest, "sitzungen": sitzungen}, ergebnis, []


def _faelle_bilden(korpus: dict, ergebnis: dict) -> tuple[list[dict], list[str], list[str]]:
    """Ein `fall` je AUSGEGEBENEM Befund, gehalten gegen die Wahrheit.

    Dritter Rueckgabewert: die Sitzungen, die den korrelierten Distraktor
    **wirklich** als Alternative gemessen haben (M-70). „Wirklich" heisst:
    der Fall sagt es zu UND der Lauf hat mindestens eine Alternative
    ausgegeben — eine Zusage ohne Ausgabe misst nichts.
    """
    befunde_je_sitzung = {e["kennung"]: e["befunde"] for e in ergebnis["ergebnisse"]}
    faelle: list[dict] = []
    probleme: list[str] = []
    messende: list[str] = []
    for s in korpus["sitzungen"]["sitzungen"]:
        kennung = s["kennung"]
        if kennung not in befunde_je_sitzung:
            probleme.append(f"{kennung}: kein Ergebnis im Lauf")
            continue
        wahre = {
            f"{q['instanz']:032x}" for q in s["quellen"] if q.get("wahre_ursache")
        }
        erwartet = s["erwartet"]
        gefundene_gruende: set[str] = set()
        gefundene_alternativen = 0
        for b in befunde_je_sitzung[kennung]:
            quelle = b["candidate_source"]
            gefundene_gruende |= {a["grund"] for a in b.get("ausschluesse", [])}
            gefundene_alternativen += len(b.get("alternatives", []))
            faelle.append({
                "kennung": kennung,
                "ursachenklasse": b["ursachenklasse"],
                "wahrheit": s["wahrheit"],
                "sicherheit_hoechstens": erwartet["sicherheit_hoechstens"],
                # ⚠️ Eine Enthaltung hat KEINE wahre Ursache und ist trotzdem
                # richtig — deshalb zaehlt sie hier als „passend".
                "ist_wahre_ursache": (quelle in wahre) or b["ursachenklasse"] == "daten_reichen_nicht",
                "sicherheit": b["confidence_class"],
                "score": float(b["confidence_score"]),
                "ist_enthaltung": b["ursachenklasse"] == "daten_reichen_nicht",
                # Was das PRODUKT selbst entschieden hat (M-31).
                "zustand": b["zustand"],
                "handelbar": b["zustand"] == "ready_to_send",
            })
        # Die Zusagen, die AM FALL haengen und nicht an einer Kennzahl.
        if erwartet.get("ausschlussgrund"):
            if erwartet["ausschlussgrund"] not in gefundene_gruende:
                probleme.append(
                    f"{kennung}: erwarteter Ausschlussgrund "
                    f"{erwartet['ausschlussgrund']!r} fehlt — gefunden: "
                    f"{sorted(gefundene_gruende) or 'keiner'}"
                )
        if erwartet.get("distraktor_ist_alternative"):
            if gefundene_alternativen == 0:
                probleme.append(
                    f"{kennung}: der Distraktor erscheint nicht als Alternative "
                    "(NAK-190, SONDE-013 M-85 Ziel 1)"
                )
            else:
                messende.append(kennung)
    return faelle, probleme, messende


def main(argv: list[str]) -> int:
    if "--selbsttest" in argv:
        return selbsttest()

    korpus, ergebnis, fehlt = _voraussetzung()
    if fehlt:
        print("P5-Korpus: VORAUSSETZUNG FEHLT")
        for f in fehlt:
            print(f"  {f}")
        return 3

    faelle, probleme, messende = _faelle_bilden(korpus, ergebnis)
    print(f"P5-Korpus: {len(korpus['sitzungen']['sitzungen'])} Sitzungen, "
          f"{len(faelle)} ausgegebene Befunde")

    # ── Kennzahlen JE URSACHENKLASSE (M-64) ─────────────────────────────
    #
    # ⚠️ Nicht ueber alle gemittelt: M-64 verlangt sie ausdruecklich „pro
    # Ursachenklasse — den sieben Klassen aus §8, nicht den vier Messklassen
    # des P4-Korpus".
    befunde: list[str] = list(probleme)
    klassen = sorted({f["ursachenklasse"] for f in faelle})
    for klasse in klassen:
        k = kennzahlen([f for f in faelle if f["ursachenklasse"] == klasse])
        print(f"  {klasse}: n={k['faelle']} precision={k['precision']:.3f} "
              f"recall={k['recall']:.3f} brier={k['brier']:.3f} "
              f"kalibrierung={k['kalibrierung']:.3f} coverage={k['coverage']:.3f} "
              f"enthaltung={k['enthaltungsrate']:.3f} "
              f"falsche_starke={k['falsche_starke']}")
        befunde += riegel(k, klasse)

    gesamt = kennzahlen(faelle)
    print(f"  GESAMT: n={gesamt['faelle']} precision={gesamt['precision']:.3f} "
          f"recall={gesamt['recall']:.3f} brier={gesamt['brier']:.3f} "
          f"kalibrierung={gesamt['kalibrierung']:.3f} "
          f"coverage={gesamt['coverage']:.3f} "
          f"enthaltung={gesamt['enthaltungsrate']:.3f} "
          f"rangmittel={gesamt['rangmittel']:.3f}")
    befunde += riegel(gesamt, "gesamt")

    # ── Zuverlaessigkeit je Sicherheitsklasse (§59 Kalibrierung) ────────
    for stufe, z in gesamt["zuverlaessigkeit"].items():
        print(f"  ZUVERLAESSIGKEIT {stufe}: n={z['n']} nennwert={z['nennwert']:.2f} "
              f"gemessene Trefferquote={z['trefferquote']:.3f}")

    # ── Die Schwelle als AUSGABE (M-31, M-66) ───────────────────────────
    schwelle = schwelle_suchen(faelle)
    for stufe, wert in schwelle["je_stufe"].items():
        print(f"  Schwelle {stufe}: handelbar={wert['handelbar']} "
              f"davon_falsch={wert['davon_falsch']} haelt={wert['haelt']}")
    print(f"  SCHWELLE (Ausgabe, M-31): niedrigste haltende Stufe = "
          f"{schwelle['niedrigste_haltende']}")
    if schwelle["niedrigste_haltende"] is None:
        befunde.append("keine Sicherheitsstufe haelt die Riegel — die Schwelle "
                       "aus M-31 ist nicht bestimmbar")

    # ── Die Schwelle, die das PRODUKT wirklich faehrt (M-31) ────────────
    handelnde = [f for f in faelle if f["handelbar"]]
    print(f"  PRODUKTSCHWELLE (M-31): {len(handelnde)} von {len(faelle)} Befunden "
          f"handelbar, Sicherheiten "
          f"{sorted({f['sicherheit'] for f in handelnde}) or ['keine']}")
    befunde += produktschwelle(faelle)

    # ── Die gedruckte Luecke des P4-Korpus (M-70) ───────────────────────
    p4 = WURZEL / "eq-copilot" / "fixtures" / "p4-korpus" / "MANIFEST.json"
    p4_eintraege = (
        json.loads(p4.read_text(encoding="utf-8")).get("nicht_gemessen", [])
        if p4.is_file() else []
    )
    p4_befunde, p4_stand = p4_luecke(p4_eintraege, messende)
    print(f"  P4-LUECKE NAK-190 (M-70): {p4_stand}")
    befunde += p4_befunde

    # ── Die Kalibrierung von GATE_MINDEST_FENSTER (M-23, E-12) ──────────
    #
    # Der Startwert 8 gilt, solange die Sitzungen „zu kurze Passage" und
    # „verschobene Passage" keine starke Aussage tragen. Traegt eine von
    # ihnen sie, muesste der Wert steigen — und mit ihm die metrics_version
    # (Entscheid E-12).
    kurz = [f for f in faelle if f["wahrheit"] in ("zu_kurze_passage", "verschobene_passage")]
    kurz_stark = [f for f in kurz if f["sicherheit"] == HOCH]
    print(f"  GATE_MINDEST_FENSTER: {len(kurz)} Faelle aus den zwei Passagensitzungen, "
          f"{len(kurz_stark)} davon stark")
    if kurz_stark:
        befunde.append("eine kurze oder verschobene Passage traegt eine starke "
                       "Aussage — GATE_MINDEST_FENSTER muss steigen (E-12)")
    else:
        print("  GATE_MINDEST_FENSTER: der Startwert 8 haelt — kein Versionsschritt "
              "noetig (E-12 Fall 3)")

    if befunde:
        for b in befunde:
            print(f"  ROT: {b}")
        return 2
    print("P5-Korpus: gruen")
    return 0


# ═════════════════════════════════════════════════════════════════════════
# Der Selbsttest (M-68, Muster A27)
# ═════════════════════════════════════════════════════════════════════════


def selbsttest() -> int:
    """Ohne Repo-Fixture: synthetische Faelle durch DIESELBEN Funktionen.

    ⚠️ Jede Erwartung laeuft mit ihrem GEGENTEIL. Ohne das Gegenteil waere der
    Selbsttest auch dann gruen, wenn ein Riegel immer rot meldete — genau der
    Fehler, den NAK-182 an `precision > 1` gefunden hat.
    """
    fehler = 0

    def pruefe(ok: bool, was: str) -> None:
        nonlocal fehler
        print(("  ok      " if ok else "  FEHLER  ") + was)
        if not ok:
            fehler += 1

    def fall(sicherheit: str, hoechstens: str, wahre: bool, score: float,
             enthaltung: bool = False, kennung: str = "x",
             zustand: str | None = None) -> dict:
        # Ohne ausdruecklichen Zustand faehrt der synthetische Fall genau die
        # Abbildung, die `zustand_aus_sicherheit` im Produkt faehrt.
        echt = zustand or ("ready_to_send" if sicherheit == HOCH else "more_data")
        return {
            "kennung": kennung,
            "ursachenklasse": "quelle_resonanz",
            "wahrheit": "wahre_ursache",
            "sicherheit_hoechstens": hoechstens,
            "ist_wahre_ursache": wahre,
            "sicherheit": sicherheit,
            "score": score,
            "ist_enthaltung": enthaltung,
            "zustand": echt,
            "handelbar": echt == "ready_to_send",
        }

    print("P5-Korpus Selbsttest (M-68)")

    # ── falsche_starke: beide Wege, jeder mit Gegenteil ─────────────────
    k = kennzahlen([fall(HOCH, HOCH, False, 0.9, kennung="fremd")])
    pruefe(k["falsche_starke"] == 1, "stark auf einer fremden Quelle ist falsch")
    pruefe(riegel(k, "x") != [], "und der Riegel meldet sie")
    k = kennzahlen([fall(HOCH, HOCH, True, 0.9)])
    pruefe(k["falsche_starke"] == 0, "GEGENTEIL: stark auf der wahren Ursache ist richtig")
    pruefe(riegel(k, "x") == [], "und der Riegel schweigt")

    k = kennzahlen([fall(HOCH, MITTEL, True, 0.9, kennung="zuhoch")])
    pruefe(k["falsche_starke"] == 1, "stark, wo hoechstens mittel erlaubt ist, ist falsch")
    k = kennzahlen([fall(MITTEL, MITTEL, True, 0.5)])
    pruefe(k["falsche_starke"] == 0, "GEGENTEIL: mittel, wo mittel erlaubt ist, passt")

    # ── falsche_schwache ────────────────────────────────────────────────
    k = kennzahlen([fall(MITTEL, UNKLAR, True, 0.5, kennung="schwach")])
    pruefe(k["falsche_schwache"] == 1, "mittel, wo nur unklar erlaubt ist, ist falsch")
    k = kennzahlen([fall(UNKLAR, UNKLAR, True, 0.2, enthaltung=True)])
    pruefe(k["falsche_schwache"] == 0, "GEGENTEIL: unklar, wo unklar erlaubt ist, passt")

    # ── Enthaltung ist KEIN Fehlschlag ──────────────────────────────────
    k = kennzahlen([fall(UNKLAR, UNKLAR, True, 0.1, enthaltung=True),
                    fall(HOCH, HOCH, True, 0.9)])
    pruefe(k["enthaltungen"] == 1 and k["falsche_starke"] == 0,
           "eine Enthaltung ist keine falsche Behauptung")
    pruefe(abs(k["enthaltungsrate"] - 0.5) < 1e-9 and abs(k["coverage"] - 0.5) < 1e-9,
           "sie senkt die Coverage und hebt die Enthaltungsrate")
    nur_behauptung = kennzahlen([fall(HOCH, HOCH, True, 0.9)])
    pruefe(abs(k["brier"] - nur_behauptung["brier"]) < 1e-9,
           "und sie geht NICHT in den Brier Score ein")
    mit_zweiter = kennzahlen([fall(HOCH, HOCH, True, 0.9),
                              fall(UNKLAR, UNKLAR, True, 0.1, kennung="behauptet")])
    pruefe(abs(mit_zweiter["brier"] - nur_behauptung["brier"]) > 1e-9,
           "GEGENTEIL: eine echte zweite Behauptung veraendert ihn sehr wohl")

    # ── precision und recall in ihren Grenzen, mit Gegenteil ────────────
    k = kennzahlen([fall(HOCH, HOCH, True, 0.9), fall(HOCH, HOCH, False, 0.9)])
    pruefe(abs(k["precision"] - 0.5) < 1e-9, "precision zaehlt beide starken")
    pruefe(riegel(k, "x") != [], "und der Riegel meldet die falsche")
    k = kennzahlen([])
    pruefe(k["precision"] == 1.0 and k["recall"] == 1.0,
           "ein leerer Satz hat weder falsche noch fehlende Behauptungen")
    pruefe(riegel(k, "x") == [], "und meldet nichts")
    # ⚠️ Der Riegel MUSS fallen koennen — synthetisch gefuettert.
    pruefe(riegel({"falsche_starke": 0, "falsche_schwache": 0, "precision": 1.5,
                   "recall": 1.0, "brier": 0.0, "kalibrierung": 0.0,
                   "namen_falscher_starker": [], "namen_falscher_schwacher": []}, "x") != [],
           "precision > 1 faellt (der Riegel hat einen Weg zu fallen)")
    pruefe(riegel({"falsche_starke": 0, "falsche_schwache": 0, "precision": 1.0,
                   "recall": 1.5, "brier": 0.0, "kalibrierung": 0.0,
                   "namen_falscher_starker": [], "namen_falscher_schwacher": []}, "x") != [],
           "recall > 1 ebenso")
    pruefe(riegel({"falsche_starke": 0, "falsche_schwache": 0, "precision": 1.0,
                   "recall": 1.0, "brier": 2.0, "kalibrierung": 0.0,
                   "namen_falscher_starker": [], "namen_falscher_schwacher": []}, "x") != [],
           "brier > 1 ebenso")
    pruefe(riegel({"falsche_starke": 0, "falsche_schwache": 0, "precision": 1.0,
                   "recall": 1.0, "brier": 0.0, "kalibrierung": 0.0,
                   "namen_falscher_starker": [], "namen_falscher_schwacher": []}, "x") == [],
           "GEGENTEIL: gueltige Kennzahlen melden nichts")

    # ── Kalibrierung und Brier rechnen wirklich ─────────────────────────
    #
    # ⚠️ Vorhergesagt wird die KLASSE ueber `KLASSENNENNWERT`, nicht der Rang:
    # ein richtiges `hoch` liegt (1 − 0,9)² = 0,01 daneben, ein falsches
    # (0 − 0,9)² = 0,81. Der `score` darf das Ergebnis NICHT bewegen.
    k = kennzahlen([fall(HOCH, HOCH, True, 1.0)])
    pruefe(abs(k["brier"] - 0.01) < 1e-9 and abs(k["kalibrierung"] - 0.1) < 1e-9,
           "eine richtige starke Behauptung liegt genau ihren Nennwert daneben")
    k = kennzahlen([fall(HOCH, HOCH, False, 1.0, kennung="daneben")])
    pruefe(abs(k["brier"] - 0.81) < 1e-9 and abs(k["kalibrierung"] - 0.9) < 1e-9,
           "GEGENTEIL: eine falsche starke Behauptung traegt den vollen Fehler")
    # Derselbe Fall, nur mit anderem Rang — das Ergebnis darf sich nicht ruehren.
    k2 = kennzahlen([fall(HOCH, HOCH, False, 0.02, kennung="daneben")])
    pruefe(abs(k2["brier"] - k["brier"]) < 1e-12,
           "der Rang bewegt den Brier nicht (er ist keine Wahrscheinlichkeit, M-15)")
    pruefe(abs(k2["rangmittel"] - 0.02) < 1e-12 and abs(k["rangmittel"] - 1.0) < 1e-12,
           "GEGENTEIL: er steht als `rangmittel` trotzdem sichtbar da (§42.4)")

    # ── Die Zuverlaessigkeitstafel je Klasse ────────────────────────────
    k = kennzahlen([fall(HOCH, HOCH, True, 0.9), fall(HOCH, HOCH, False, 0.9,
                                                      kennung="fremd"),
                    fall(MITTEL, MITTEL, True, 0.5)])
    pruefe(k["zuverlaessigkeit"][HOCH]["n"] == 2
           and abs(k["zuverlaessigkeit"][HOCH]["trefferquote"] - 0.5) < 1e-9,
           "die Tafel zeigt je Klasse die GEMESSENE Trefferquote")
    pruefe(abs(k["zuverlaessigkeit"][MITTEL]["trefferquote"] - 1.0) < 1e-9,
           "und mittelt sie nicht mit der anderen Klasse zusammen")
    pruefe(UNKLAR not in k["zuverlaessigkeit"],
           "GEGENTEIL: eine Klasse ohne Faelle steht nicht mit erfundener Quote da")

    # ── Die Schwelle ist Ausgabe ────────────────────────────────────────
    s = schwelle_suchen([fall(HOCH, HOCH, True, 0.9), fall(MITTEL, UNKLAR, True, 0.5,
                                                           kennung="schwach")])
    pruefe(s["niedrigste_haltende"] == HOCH,
           "die Schwelle wird GESUCHT: nur `hoch` haelt hier")
    s = schwelle_suchen([fall(HOCH, HOCH, True, 0.9), fall(MITTEL, MITTEL, True, 0.5)])
    pruefe(s["niedrigste_haltende"] == UNKLAR,
           "GEGENTEIL: haelt jede Stufe, ist die niedrigste die Antwort")

    # ── Die Schwelle des Produkts, beide Richtungen (M-31) ──────────────
    pruefe(produktschwelle([fall(HOCH, HOCH, True, 0.9),
                            fall(MITTEL, MITTEL, True, 0.5)]) == [],
           "handelbar genau bei `hoch` meldet nichts")
    pruefe(produktschwelle([fall(MITTEL, MITTEL, True, 0.5, kennung="locker",
                                 zustand="ready_to_send")]) != [],
           "ein handelnder `mittel`-Befund faellt (Schwelle geraten statt kalibriert)")
    pruefe(produktschwelle([fall(HOCH, HOCH, True, 0.9, kennung="streng",
                                 zustand="more_data")]) != [],
           "GEGENTEIL: ein `hoch`, das nicht handelt, faellt ebenso")
    pruefe(produktschwelle([fall(HOCH, HOCH, True, 0.9, kennung="alt",
                                 zustand="stale")]) == [],
           "ein veralteter Befund ist die benannte Ausnahme und faellt nicht")

    # ── Die gedruckte P4-Luecke, beide Richtungen (M-70) ────────────────
    zeile = [{"ticket": "NAK-190", "zusage": "M-85 Ziel 1"}]
    pruefe(p4_luecke([], [])[0] != [],
           "die Luecke verschwindet ohne P5-Messung: rot (M-70)")
    pruefe(p4_luecke([], ["korrelierter_distraktor"])[0] == [],
           "GEGENTEIL: verschwunden UND gemessen ist in Ordnung")
    pruefe(p4_luecke(zeile, [])[0] == [] and "ungemessen" in p4_luecke(zeile, [])[1],
           "gedruckt und ungemessen ist nie rot, nur benannt")
    pruefe(p4_luecke(zeile, ["korrelierter_distraktor"])[0] == []
           and "entfernt werden" in p4_luecke(zeile, ["korrelierter_distraktor"])[1],
           "gedruckt und inzwischen gemessen: die Zeile darf weg")
    pruefe(p4_luecke([{"ticket": "NAK-999"}], [])[0] != [],
           "ein fremdes Ticket haelt die Luecke NAK-190 nicht offen")

    print(f"P5-Korpus Selbsttest: {'gruen' if fehler == 0 else f'{fehler} Fehler'}")
    return 0 if fehler == 0 else 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
