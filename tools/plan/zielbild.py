#!/usr/bin/env python3
"""Zielbild — so arbeitet Nakama, wenn der heutige Plan fertig ist.

WARUM ES DIESES WERKZEUG GIBT (User 23.09.2026): der Planstand sagt, WIE WEIT
der Bau ist. Dem User als Projektleiter fehlte das Bild, WOHIN er fuehrt —
„die theorethische funktionsbeschreibung wie nakama funktionieren wuerde
,wenn es jetzt mit dem aktuellen plan und den bestehenden entscheidungen
fertig waere … ein sich stets aktualisierendes standbild".

Getrennt wie beim Planstand (docs/plan/LIES-MICH.md):

  Text         docs/ZIELBILD.md — autoriert, in Alltagssprache, wie der Text
               in docs/plan/plan.json. Nie gerechnet.
  Aktualitaet  gerechnet. Der Fussblock ZIELBILD-ABGLEICH haelt fest, welche
               Entscheidquellen eingearbeitet sind; `pruefen` vergleicht ihn
               mit dem Repo und nennt jede neue oder geaenderte Quelle beim
               Namen.

Entscheidquellen (CLAUDE.md: verbindlich ist nur Datum plus User-Zitat):

  docs/plan/fragen.json   beantwortete, offene und geparkte Karten
  design/abnahmen/*.md    datierte Abnahmen
  docs/offene-punkte.md   Registerzeilen, deren Klasse mit „Produkt" beginnt
                          oder es enthaelt (Produktfrage, Produktentscheid …)
  docs/plan/plan.json     Planschritte (Kennung, Phase, leitungsname)

🔑 Fail-closed an zwei Stellen. Erstens macht jede neue oder geaenderte Quelle
das Zielbild VERALTET, bis jemand sie eingearbeitet und `abgleichen` gefahren
hat. Zweitens verweigert `abgleichen`, solange eine offene oder geparkte Karte
im Text fehlt oder eine Planmarke ins Leere zeigt: Vergessen fuehrt zu
„veraltet", nie zu einem still falschen Bild.

Planmarken im Text: eine eigene Zeile `<!-- plan: P4–P5 · S31b -->` direkt
unter einer Ueberschrift. `anzeige` ersetzt sie durch den gerechneten Stand
dieser Phasen oder Schritte aus docs/PLAN-STAND.md (dieselben Zeichen wie der
Plan-Tab: ✓ fertig · ◐ laeuft · ○ offen).

Aufruf vom Repo-Root:
  py -3.13 tools/plan/zielbild.py pruefen
  py -3.13 tools/plan/zielbild.py abgleichen
  py -3.13 tools/plan/zielbild.py anzeige [--breite N] [--farbe] [--kurz]
Exit 0 = aktuell bzw. abgeglichen · 4 = veraltet bzw. Text unvollstaendig ·
2 = Werkzeugfehler (Quelle unlesbar, Fussblock kaputt). Nicht blockierend wie
die Beine A32/A33; laeuft in Sekunden, ohne Bau. Proben:
`py -3.13 tools/plan/zielbild_test.py`.
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import pathlib
import re
import sys

WURZEL_STANDARD = pathlib.Path(__file__).resolve().parents[2]

ZIELBILD = "docs/ZIELBILD.md"
FRAGEN = "docs/plan/fragen.json"
PLAN = "docs/plan/plan.json"
PLANSTAND = "docs/PLAN-STAND.md"
REGISTER = "docs/offene-punkte.md"
ABNAHMEN = "design/abnahmen"

EXIT_OK, EXIT_WERKZEUG, EXIT_VERALTET = 0, 2, 4

BLOCK_RE = re.compile(r"<!--\s*ZIELBILD-ABGLEICH\b(.*?)-->", re.S)
# Eine Planmarke als eigene Zeile wird zur Zeile „Plan: …"; am Ende einer Listenzeile
# wird sie kurz („✓ P3") an den Text gehaengt.
PLANMARKE_RE = re.compile(r"^\s*<!--\s*plan:\s*(.*?)\s*-->\s*$")
PLANMARKE_INLINE_RE = re.compile(r"\s*<!--\s*plan:\s*(.*?)\s*-->")
KOMMENTAR_RE = re.compile(r"<!--.*?-->", re.S)
# Dieselbe Zeilenform, aus der tools/dirigent/cockpit.ps1 den Schrittstatus liest.
PLANSTAND_ZEILE_RE = re.compile(
    r"^- ([■▣□]) \*\*(.+?)\*\*.*\((abgenommen|gebaut|offen)(?: ·|\))")
REGISTER_ID_RE = re.compile(r"^~*(NAK-\d+)~*$")
KLASSE_RE = re.compile(r"\*\*\[([^\]]+)\]\*\*")
FETT_RE = re.compile(r"\*\*(.+?)\*\*")
KARTE_RE = re.compile(r"\bU\d+(?:\.\d+)?\b")

LINIENBREITE_HOECHSTENS = 96   # Lesbarkeit: breite Fenster brechen trotzdem hier um

BLOCK_KOPF = ("ZIELBILD-ABGLEICH — geschrieben von "
              "`py -3.13 tools/plan/zielbild.py abgleichen`, nie von Hand.")


class Werkzeugfehler(Exception):
    """Eine Quelle ist unlesbar oder der Fussblock kaputt: Exit 2."""


# ---------------------------------------------------------------------------
# Quellen lesen
# ---------------------------------------------------------------------------

def kurzhash(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()[:6]


def lies_text(wurzel: pathlib.Path, rel: str) -> str:
    try:
        return (wurzel / rel).read_bytes().decode("utf-8-sig")
    except (OSError, UnicodeDecodeError) as e:
        raise Werkzeugfehler(f"{rel} nicht lesbar ({e})") from e


def lies_json(wurzel: pathlib.Path, rel: str) -> dict:
    try:
        return json.loads(lies_text(wurzel, rel))
    except json.JSONDecodeError as e:
        raise Werkzeugfehler(f"{rel} ist kein gueltiges JSON ({e})") from e


def einzeilig(text: str, laenge: int = 90) -> str:
    text = re.sub(r"\s+", " ", str(text or "")).strip()
    return text if len(text) <= laenge else text[: laenge - 1].rstrip() + "…"


def karten_lesen(wurzel: pathlib.Path) -> dict:
    """Karten mit Titel. Beantwortete tragen einen Hash ueber Datum und Wahl,
    damit eine erneut beantwortete Karte als Aenderung auffaellt."""
    daten = lies_json(wurzel, FRAGEN)
    beantwortet, offen, geparkt = {}, {}, {}
    for kennung, eintrag in (daten.get("beantwortet") or {}).items():
        eintrag = eintrag if isinstance(eintrag, dict) else {}
        karte = eintrag.get("karte") if isinstance(eintrag.get("karte"), dict) else {}
        titel = (karte.get("titel") or eintrag.get("frage") or eintrag.get("ergebnis")
                 or eintrag.get("wahl") or "")
        schluessel = f"{eintrag.get('datum', '')}|{eintrag.get('wahl', '')}|{eintrag.get('text', '')}"
        beantwortet[str(kennung)] = (kurzhash(schluessel), einzeilig(titel))
    for liste, ziel in (("offen", offen), ("geparkt", geparkt)):
        eintraege = daten.get(liste) or []
        if isinstance(eintraege, dict):
            eintraege = list(eintraege.values())
        for karte in eintraege:
            if isinstance(karte, dict) and karte.get("id"):
                ziel[str(karte["id"])] = einzeilig(karte.get("titel") or karte.get("frage") or "")
    return {"beantwortet": beantwortet, "offen": offen, "geparkt": geparkt}


def abnahmen_lesen(wurzel: pathlib.Path) -> dict:
    ordner = wurzel / ABNAHMEN
    if not ordner.is_dir():
        raise Werkzeugfehler(f"{ABNAHMEN}/ fehlt")
    namen = sorted(p.name for p in ordner.glob("*.md") if p.name != "LIES-MICH.md")
    return {kurzhash(name): name for name in namen}


def register_lesen(wurzel: pathlib.Path) -> dict:
    """Registerzeilen, deren Klasse „Produkt" enthaelt: Kennung -> (Hash der
    Klassen, Kurztitel). Steht eine Kennung in zwei Tabellen, zaehlen beide."""
    klassen: dict[str, list[str]] = {}
    titel: dict[str, str] = {}
    for zeile in lies_text(wurzel, REGISTER).splitlines():
        if not zeile.startswith("|"):
            continue
        zellen = [z.strip() for z in re.split(r"(?<!\\)\|", zeile)[1:-1]]
        if not zellen:
            continue
        m = REGISTER_ID_RE.match(zellen[0])
        if not m:
            continue
        rest = " | ".join(zellen[1:])
        k = KLASSE_RE.search(rest)
        if not k or "Produkt" not in k.group(1):
            continue
        kennung = m.group(1)
        klassen.setdefault(kennung, []).append(k.group(1).strip())
        f = FETT_RE.search(rest[k.end():])
        titel.setdefault(kennung, einzeilig(f.group(1) if f else k.group(1)))
    return {kennung: (kurzhash("|".join(sorted(liste))), titel[kennung])
            for kennung, liste in klassen.items()}


def plan_lesen(wurzel: pathlib.Path) -> tuple[dict, dict]:
    """Planschritte: Kennung -> (Hash ueber Phase und leitungsname, Name) und
    Phase -> Liste der Schrittkennungen."""
    daten = lies_json(wurzel, PLAN)
    schritte, phasen = {}, {}
    for phase in daten.get("phasen") or []:
        name = str(phase.get("phase") or "?")
        phasen[name] = []
        for schritt in phase.get("schritte") or []:
            kennung = str(schritt.get("id") or "?")
            leitung = str(schritt.get("leitungsname") or "")
            schritte[kennung] = (kurzhash(f"{name}|{leitung}"), einzeilig(leitung))
            phasen[name].append(kennung)
    return schritte, phasen


def planstand_lesen(wurzel: pathlib.Path) -> dict:
    """Schrittstatus aus dem gerechneten Blatt; fehlt es, ist nichts bekannt."""
    try:
        text = lies_text(wurzel, PLANSTAND)
    except Werkzeugfehler:
        return {}
    status = {}
    for zeile in text.splitlines():
        m = PLANSTAND_ZEILE_RE.match(zeile)
        if m:
            status[m.group(2)] = m.group(3)
    return status


def quellen_lesen(wurzel: pathlib.Path) -> dict:
    schritte, phasen = plan_lesen(wurzel)
    return {"karten": karten_lesen(wurzel), "abnahmen": abnahmen_lesen(wurzel),
            "register": register_lesen(wurzel), "schritte": schritte, "phasen": phasen}


# ---------------------------------------------------------------------------
# Fussblock
# ---------------------------------------------------------------------------

def kennung_kodieren(kennung: str) -> str:
    """Kennungen wie „Termin A" tragen Leerzeichen; im Fussblock trennt das Leerzeichen."""
    return kennung.replace("%", "%25").replace(" ", "%20").replace("=", "%3D")


def kennung_dekodieren(kodiert: str) -> str:
    return kodiert.replace("%3D", "=").replace("%20", " ").replace("%25", "%")


def paare(eintraege: dict) -> str:
    return " ".join(f"{kennung_kodieren(k)}={v[0]}" for k, v in sorted(eintraege.items()))


def liste(kennungen) -> str:
    return " ".join(kennung_kodieren(k) for k in sorted(kennungen))


def block_bauen(quellen: dict, nl: str) -> str:
    karten = quellen["karten"]
    zeilen = [
        "<!-- " + BLOCK_KOPF,
        "stand: " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M"),
        "karten-beantwortet: " + paare(karten["beantwortet"]),
        "karten-offen: " + liste(karten["offen"]),
        "karten-geparkt: " + liste(karten["geparkt"]),
        "abnahmen: " + liste(quellen["abnahmen"]),
        "register-produkt: " + paare(quellen["register"]),
        "planschritte: " + paare(quellen["schritte"]),
        "-->",
    ]
    return nl.join(zeilen)


def block_lesen(text: str) -> dict | None:
    """Der gespeicherte Abgleich oder None, wenn es noch keinen gibt."""
    treffer = BLOCK_RE.findall(text)
    if not treffer:
        return None
    if len(treffer) > 1:
        raise Werkzeugfehler(f"{ZIELBILD} traegt {len(treffer)} Abgleich-Bloecke statt einem")
    felder = {}
    for zeile in treffer[0].splitlines():
        if ":" in zeile and not zeile.strip().startswith("ZIELBILD-ABGLEICH"):
            schluessel, _, wert = zeile.partition(":")
            felder[schluessel.strip()] = wert.strip()
    pflicht = ("stand", "karten-beantwortet", "karten-offen", "karten-geparkt",
               "abnahmen", "register-produkt", "planschritte")
    fehlend = [f for f in pflicht if f not in felder]
    if fehlend:
        raise Werkzeugfehler(f"Abgleich-Block unvollstaendig, es fehlt: {', '.join(fehlend)}")

    def als_paare(wert: str) -> dict:
        ergebnis = {}
        for teil in wert.split():
            kennung, gleich, h = teil.partition("=")
            if not gleich:
                raise Werkzeugfehler(f"Abgleich-Block: unlesbarer Eintrag {teil!r}")
            ergebnis[kennung_dekodieren(kennung)] = h
        return ergebnis

    def als_menge(wert: str) -> set:
        return {kennung_dekodieren(teil) for teil in wert.split()}

    return {
        "stand": felder["stand"],
        "beantwortet": als_paare(felder["karten-beantwortet"]),
        "offen": als_menge(felder["karten-offen"]),
        "geparkt": als_menge(felder["karten-geparkt"]),
        "abnahmen": als_menge(felder["abnahmen"]),
        "register": als_paare(felder["register-produkt"]),
        "schritte": als_paare(felder["planschritte"]),
    }


# ---------------------------------------------------------------------------
# Pruefen
# ---------------------------------------------------------------------------

def strich(text: str) -> str:
    """Phasennamen tragen einen Halbgeviertstrich; ein getippter Bindestrich zaehlt gleich."""
    return text.replace("–", "-").replace("—", "-").strip()


def marken_zerlegen(inhalt: str) -> list[str]:
    return [t.strip() for t in re.split(r"[·,]", inhalt) if t.strip()]


def planmarken(text: str) -> list[str]:
    """Alle Planmarken, ob eigene Zeile oder am Zeilenende."""
    marken = []
    for m in PLANMARKE_INLINE_RE.finditer(text):
        marken += marken_zerlegen(m.group(1))
    return marken


def textbefunde(text: str, quellen: dict) -> list[str]:
    """Was am autorierten Text selbst fehlt — unabhaengig vom Abgleich."""
    befunde = []
    phasen = {strich(p) for p in quellen["phasen"]}
    schritte = {strich(s) for s in quellen["schritte"]}
    for marke in planmarken(text):
        if strich(marke) not in phasen and strich(marke) not in schritte:
            befunde.append(f"Planmarke „{marke}\" zeigt auf keine Phase und keinen Schritt in {PLAN}")
    lesetext = KOMMENTAR_RE.sub(" ", text)
    genannt = set(KARTE_RE.findall(lesetext))
    for art in ("offen", "geparkt"):
        for kennung, titel in sorted(quellen["karten"][art].items()):
            if kennung not in genannt:
                befunde.append(f"{art}e Karte {kennung} fehlt im Text ({titel})")
    return befunde


def unterschiede(gespeichert: dict | None, quellen: dict) -> list[str]:
    if gespeichert is None:
        return ["noch nie abgeglichen — der Fussblock ZIELBILD-ABGLEICH fehlt"]
    karten = quellen["karten"]
    liste = []
    for kennung, (h, titel) in sorted(karten["beantwortet"].items()):
        if kennung not in gespeichert["beantwortet"]:
            liste.append(f"Karte {kennung} beantwortet: {titel}")
        elif gespeichert["beantwortet"][kennung] != h:
            liste.append(f"Karte {kennung} neu beantwortet: {titel}")
    for kennung, titel in sorted(karten["offen"].items()):
        if kennung not in gespeichert["offen"]:
            liste.append(f"Karte {kennung} neu offen: {titel}")
    for kennung, titel in sorted(karten["geparkt"].items()):
        if kennung not in gespeichert["geparkt"]:
            liste.append(f"Karte {kennung} neu geparkt: {titel}")
    alle_karten = set(karten["beantwortet"]) | set(karten["offen"]) | set(karten["geparkt"])
    for kennung in sorted((gespeichert["offen"] | gespeichert["geparkt"]) - alle_karten):
        liste.append(f"Karte {kennung} ist aus fragen.json verschwunden")
    for h, name in sorted(quellen["abnahmen"].items(), key=lambda e: e[1]):
        if h not in gespeichert["abnahmen"]:
            liste.append(f"Abnahme neu: {name.removesuffix('.md')}")
    weg = gespeichert["abnahmen"] - set(quellen["abnahmen"])
    if weg:
        liste.append(f"{len(weg)} Abnahme(n) entfernt oder umbenannt")
    for kennung, (h, titel) in sorted(quellen["register"].items(), key=lambda e: int(e[0][4:])):
        if kennung not in gespeichert["register"]:
            liste.append(f"Registerzeile {kennung} neu: {titel}")
        elif gespeichert["register"][kennung] != h:
            liste.append(f"Registerzeile {kennung} Klasse geändert: {titel}")
    for kennung in sorted(set(gespeichert["register"]) - set(quellen["register"])):
        liste.append(f"Registerzeile {kennung} ist keine Produktzeile mehr oder fehlt")
    for kennung, (h, name) in quellen["schritte"].items():
        if kennung not in gespeichert["schritte"]:
            liste.append(f"Planschritt {kennung} neu: {name}")
        elif gespeichert["schritte"][kennung] != h:
            liste.append(f"Planschritt {kennung} geändert: {name}")
    for kennung in sorted(set(gespeichert["schritte"]) - set(quellen["schritte"])):
        liste.append(f"Planschritt {kennung} entfernt")
    return liste


def pruefung(wurzel: pathlib.Path) -> dict:
    text = lies_text(wurzel, ZIELBILD)
    quellen = quellen_lesen(wurzel)
    gespeichert = block_lesen(text)
    return {
        "text": text,
        "quellen": quellen,
        "stand": gespeichert["stand"] if gespeichert else "",
        "neu": unterschiede(gespeichert, quellen),
        "text_befunde": textbefunde(text, quellen),
    }


def stand_deutsch(stand: str) -> str:
    m = re.match(r"(\d{4})-(\d{2})-(\d{2})(?:\s+(\d{2}:\d{2}))?", stand or "")
    if not m:
        return stand or "ohne Datum"
    return f"{m.group(3)}.{m.group(2)}.{m.group(1)}" + (f" {m.group(4)}" if m.group(4) else "")


def befehl_pruefen(wurzel: pathlib.Path) -> int:
    erg = pruefung(wurzel)
    offen = erg["neu"] + erg["text_befunde"]
    if not offen:
        print(f"ZIELBILD AKTUELL · Stand {stand_deutsch(erg['stand'])} · alle Entscheidquellen eingearbeitet")
        return EXIT_OK
    seit = f"seit dem Abgleich vom {stand_deutsch(erg['stand'])}" if erg["stand"] else "(noch nie abgeglichen)"
    print(f"ZIELBILD VERALTET · {len(offen)} Punkt(e) {seit}:")
    for punkt in offen:
        print(f"  - {punkt}")
    print("Nachziehen: die Punkte in docs/ZIELBILD.md einarbeiten, dann "
          "`py -3.13 tools/plan/zielbild.py abgleichen`.")
    return EXIT_VERALTET


def befehl_abgleichen(wurzel: pathlib.Path) -> int:
    erg = pruefung(wurzel)
    if erg["text_befunde"]:
        print("ABGLEICH VERWEIGERT · der Text ist unvollständig:")
        for punkt in erg["text_befunde"]:
            print(f"  - {punkt}")
        return EXIT_VERALTET
    if not erg["neu"]:
        print(f"bereits abgeglichen · Stand {stand_deutsch(erg['stand'])} · nichts geschrieben")
        return EXIT_OK
    text = erg["text"]
    nl = "\r\n" if "\r\n" in text else "\n"
    rumpf = BLOCK_RE.sub("", text).rstrip()
    neu = rumpf + nl + nl + block_bauen(erg["quellen"], nl) + nl
    (wurzel / ZIELBILD).write_bytes(neu.encode("utf-8"))
    print(f"abgeglichen · {len(erg['neu'])} Punkt(e) als eingearbeitet vermerkt:")
    for punkt in erg["neu"]:
        print(f"  - {punkt}")
    return EXIT_OK


# ---------------------------------------------------------------------------
# Anzeige fuer das Terminal (Plan-Tab, Taste Z)
# ---------------------------------------------------------------------------

FETT_AN, FETT_AUS = "\x01", "\x02"


class Stil:
    def __init__(self, farbe: bool):
        e = "\x1b["
        self.farbe = farbe
        self.fett = e + "1m" if farbe else ""
        self.duenn = e + "2m" if farbe else ""
        self.titel = e + "1;36m" if farbe else ""
        self.gruen = e + "32m" if farbe else ""
        self.gelb = e + "33m" if farbe else ""
        self.warn = e + "1;33m" if farbe else ""
        self.rot = e + "1;31m" if farbe else ""
        self.aus = e + "0m" if farbe else ""


SCHUTZ = " "   # geschuetztes Leerzeichen: Zeichen und Wort bleiben beim Umbruch zusammen


def inline(text: str) -> str:
    text = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", text)
    text = re.sub(r"`([^`]+)`", r"\1", text)
    text = re.sub(r"\*\*(.+?)\*\*", FETT_AN + r"\1" + FETT_AUS, text)
    text = re.sub(r"(?<![\w*])\*(?![\s*])(.+?)(?<![\s*])\*(?![\w*])", r"\1", text)
    return re.sub(r"([✓◐○]) ", r"\1" + SCHUTZ, text)


def sichtbar(text: str) -> int:
    return len(text) - text.count(FETT_AN) - text.count(FETT_AUS)


def umbrechen(text: str, breite: int, erste: str = "", folge: str = "") -> list[str]:
    zeilen, aktuell, laenge, leer = [], erste, sichtbar(erste), True
    for wort in (w for w in re.split(r"[ \t\r\n]+", text) if w):
        wl = sichtbar(wort)
        if not leer and laenge + 1 + wl > breite:
            zeilen.append(aktuell)
            aktuell, laenge, leer = folge, sichtbar(folge), True
        if leer:
            aktuell, laenge, leer = aktuell + wort, laenge + wl, False
        else:
            aktuell, laenge = aktuell + " " + wort, laenge + 1 + wl
    zeilen.append(aktuell)
    return zeilen


def einfaerben(zeilen: list[str], stil: Stil, basis: str = "") -> list[str]:
    """Fett-Marken in ANSI; der Grundstil der Zeile wird nach „fett aus"
    wiederhergestellt, und ein Fettlauf traegt ueber den Zeilenumbruch."""
    fertig, fett = [], False
    for zeile in zeilen:
        teile = [basis + (stil.fett if fett else "")]
        for zeichen in zeile:
            if zeichen == FETT_AN:
                fett = True
                teile.append(stil.fett)
            elif zeichen == FETT_AUS:
                fett = False
                teile.append(stil.aus + basis)
            else:
                teile.append(zeichen)
        if stil.farbe and (basis or fett or FETT_AN in zeile):
            teile.append(stil.aus)
        fertig.append("".join(teile))
    return fertig


def symbole_faerben(zeile: str, stil: Stil) -> str:
    """Nach dem Umbruch: ✓ gruen, ◐ gelb — Farbe erst hier, damit der Umbruch nur
    sichtbare Zeichen zaehlt. SGR 39 setzt nur die Vordergrundfarbe zurueck, fett und
    duenn der Zeile bleiben stehen."""
    if not stil.farbe:
        return zeile
    zurueck = "\x1b[39m"
    return zeile.replace("✓", f"{stil.gruen}✓{zurueck}").replace("◐", f"{stil.gelb}◐{zurueck}")


def marken_kurz(marken: list[str], phasen: dict, status: dict) -> str:
    """Kurzform fuer das Zeilenende: nur Zeichen und Name, ohne Farbe (der Umbruch
    zaehlt Zeichen, Steuersequenzen darin wuerden ihn verfaelschen)."""
    teile = []
    phasen_n = {strich(p): (p, s) for p, s in phasen.items()}
    status_n = {strich(k): (k, v) for k, v in status.items()}
    for marke in marken:
        if strich(marke) in phasen_n:
            name, schritte = phasen_n[strich(marke)]
            werte = [status.get(s, "offen") for s in schritte]
            if schritte and all(w == "abgenommen" for w in werte):
                zeichen = "✓"
            elif any(w in ("abgenommen", "gebaut") for w in werte):
                zeichen = "◐"
            else:
                zeichen = "○"
            teile.append(f"{zeichen} {name}")
        elif strich(marke) in status_n:
            name, wert = status_n[strich(marke)]
            zeichen = {"abgenommen": "✓", "gebaut": "◐"}.get(wert, "○")
            teile.append(f"{zeichen} {name}")
        else:
            teile.append(f"? {marke}")
    return " ".join(teile)


def marken_stand(marken: list[str], phasen: dict, status: dict) -> str:
    """Langform fuer die Zeile „Plan: …", ohne Farbe (die kommt nach dem Umbruch)."""
    teile = []
    phasen_n = {strich(p): (p, s) for p, s in phasen.items()}
    status_n = {strich(k): (k, v) for k, v in status.items()}
    for marke in marken:
        if strich(marke) in phasen_n:
            name, schritte = phasen_n[strich(marke)]
            werte = [status.get(s, "offen") for s in schritte]
            fertig = sum(1 for w in werte if w == "abgenommen")
            gebaut = sum(1 for w in werte if w == "gebaut")
            if schritte and fertig == len(schritte):
                teile.append(f"✓ {name} fertig")
            elif fertig or gebaut:
                teile.append(f"◐ {name} läuft ({fertig} von {len(schritte)} abgenommen)")
            else:
                teile.append(f"○ {name} offen")
        elif strich(marke) in status_n:
            name, wert = status_n[strich(marke)]
            zeichen = {"abgenommen": "✓", "gebaut": "◐"}.get(wert, "○")
            teile.append(f"{zeichen} {name} {wert}")
        else:
            teile.append(f"? {marke} unbekannt")
    return " · ".join(teile)


def rendern(text: str, breite: int, stil: Stil, phasen: dict, status: dict) -> list[str]:
    breite = max(30, min(breite, LINIENBREITE_HOECHSTENS))
    zeilen: list[str] = []
    absatz: list[str] = []
    art = ""  # "absatz" | "zitat" | "punkt" | "nummer"
    einzug = ""

    def leeren():
        nonlocal absatz, art, einzug
        if not absatz:
            return
        roh = PLANMARKE_INLINE_RE.sub(
            lambda m: " · " + marken_kurz(marken_zerlegen(m.group(1)), phasen, status),
            " ".join(absatz))
        inhalt = inline(roh)
        if art == "zitat":
            roh = umbrechen(inhalt, breite, "│ ", "│ ")
            zeilen.extend(einfaerben(roh, stil, stil.duenn))
        elif art in ("punkt", "nummer"):
            kopf, folge = einzug, " " * sichtbar(einzug)
            zeilen.extend(einfaerben(umbrechen(inhalt, breite, kopf, folge), stil))
        else:
            zeilen.extend(einfaerben(umbrechen(inhalt, breite), stil))
        absatz, art, einzug = [], "", ""

    def leerzeile():
        if zeilen and zeilen[-1] != "":
            zeilen.append("")

    im_kommentar = False
    for roh in text.splitlines():
        zeile = roh.rstrip()
        if im_kommentar:
            if "-->" in zeile:
                im_kommentar = False
            continue
        marke = PLANMARKE_RE.match(zeile)
        if marke:
            leeren()
            text = marken_stand(marken_zerlegen(marke.group(1)), phasen, status)
            for i, teil in enumerate(umbrechen(re.sub(r"([✓◐○?]) ", r"\1" + SCHUTZ, text), breite,
                                               "Plan: ", " " * 6)):
                if i == 0:
                    teil = f"{stil.duenn}Plan:{stil.aus}" + teil[len("Plan:"):]
                zeilen.append(teil)
            continue
        if zeile.lstrip().startswith("<!--"):
            leeren()
            if "-->" not in zeile:
                im_kommentar = True
            continue
        if not zeile.strip():
            leeren()
            leerzeile()
            continue
        kopf = re.match(r"^(#{1,3})\s+(.*)$", zeile)
        if kopf:
            leeren()
            stufe, inhalt = len(kopf.group(1)), inline(kopf.group(2)).replace(FETT_AN, "").replace(FETT_AUS, "")
            if stufe == 1:
                zeilen.extend(stil.titel + z + stil.aus for z in umbrechen(inhalt, breite))
            elif stufe == 2:
                leerzeile()
                zeilen.extend(stil.titel + z + stil.aus for z in umbrechen(inhalt, breite))
                zeilen.append(stil.duenn + "─" * min(breite, max(8, len(inhalt))) + stil.aus)
            else:
                zeilen.extend(stil.fett + z + stil.aus for z in umbrechen(inhalt, breite))
            continue
        zitat = re.match(r"^\s*>\s?(.*)$", zeile)
        if zitat:
            if art != "zitat":
                leeren()
                art = "zitat"
            if not zitat.group(1).strip():
                # Leere Zitatzeile trennt zwei Absaetze desselben Kastens.
                if absatz:
                    leeren()
                    zeilen.append(stil.duenn + "│" + stil.aus)
                art = "zitat"
                continue
            absatz.append(zitat.group(1))
            continue
        punkt = re.match(r"^(\s*)[-*]\s+(.*)$", zeile)
        nummer = re.match(r"^(\s*)(\d+)\.\s+(.*)$", zeile)
        if punkt:
            leeren()
            tiefe = len(punkt.group(1)) // 2
            art, einzug = "punkt", "  " * tiefe + ("• " if tiefe == 0 else "◦ ")
            absatz = [punkt.group(2)]
            continue
        if nummer:
            leeren()
            tiefe = len(nummer.group(1)) // 2
            art, einzug = "nummer", "  " * tiefe + f"{nummer.group(2)}. "
            absatz = [nummer.group(3)]
            continue
        if art in ("punkt", "nummer") and roh.startswith(" "):
            absatz.append(zeile.strip())
            continue
        if art and art != "absatz":
            leeren()
        art = "absatz"
        absatz.append(zeile.strip())
    leeren()
    while zeilen and zeilen[-1] == "":
        zeilen.pop()
    return [symbole_faerben(z.replace(SCHUTZ, " "), stil) for z in zeilen]


def statuszeilen(erg: dict | None, fehler: str, stil: Stil, breite: int) -> list[str]:
    breite = max(30, min(breite, LINIENBREITE_HOECHSTENS))
    kopf = "ZIELBILD · wie Nakama arbeitet, wenn der heutige Plan fertig ist"

    def gefaerbt(text: str, farbe: str, erste: str = "", folge: str = "") -> list[str]:
        return [f"{farbe}{z}{stil.aus}" for z in umbrechen(text, breite, erste, folge)]

    if fehler:
        return gefaerbt(f"{kopf} · Aktualität nicht prüfbar · {fehler}", stil.rot)
    offen = erg["neu"] + erg["text_befunde"]
    if not offen:
        return gefaerbt(f"{kopf} · Stand {stand_deutsch(erg['stand'])} · alle Entscheide eingearbeitet",
                        stil.duenn)
    seit = (f"seit dem Stand vom {stand_deutsch(erg['stand'])} " if erg["stand"]
            else "(noch nie abgeglichen) ")
    zeilen = gefaerbt(f"{kopf} · ! {len(offen)} Punkt(e) {seit}noch nicht eingearbeitet — "
                      f"Nachzug im nächsten Abschlussfenster", stil.warn)
    for punkt in offen[:6]:
        zeilen += gefaerbt(punkt, stil.gelb, "  - ", "    ")
    if len(offen) > 6:
        zeilen.append(f"{stil.gelb}  … und {len(offen) - 6} weitere (zielbild.py pruefen){stil.aus}")
    return zeilen


def befehl_anzeige(wurzel: pathlib.Path, breite: int, farbe: bool, kurz: bool) -> int:
    stil = Stil(farbe)
    try:
        erg = pruefung(wurzel)
        fehler = ""
    except Werkzeugfehler as e:
        erg, fehler = None, str(e)
    if kurz:
        # Eine Zeile fuer den Plan-Tab; das Wort „Zielbild" setzt der Tab selbst davor.
        if fehler:
            print(f"Aktualität nicht prüfbar · {fehler}")
            return EXIT_WERKZEUG
        offen = len(erg["neu"]) + len(erg["text_befunde"])
        if not erg["stand"]:
            print("noch nie abgeglichen")
            return EXIT_VERALTET
        if offen:
            print(f"{offen} Punkt(e) noch nicht eingearbeitet · Nachzug im nächsten Abschlussfenster")
            return EXIT_VERALTET
        print(f"aktuell, Stand {stand_deutsch(erg['stand'])}")
        return EXIT_OK
    if erg is None:
        try:
            text = lies_text(wurzel, ZIELBILD)
        except Werkzeugfehler as e:
            print(f"Zielbild nicht lesbar · {e}")
            return EXIT_WERKZEUG
        phasen = {}
    else:
        text, phasen = erg["text"], erg["quellen"]["phasen"]
    breite = breite if breite > 0 else LINIENBREITE_HOECHSTENS
    zeilen = statuszeilen(erg, fehler, stil, breite) + [""]
    zeilen += rendern(text, breite - 1, stil, phasen, planstand_lesen(wurzel))
    sys.stdout.write("\n".join(zeilen) + "\n")
    if fehler:
        return EXIT_WERKZEUG
    return EXIT_VERALTET if (erg["neu"] or erg["text_befunde"]) else EXIT_OK


def main(argv: list[str] | None = None) -> int:
    for strom in (sys.stdout, sys.stderr):
        try:
            strom.reconfigure(encoding="utf-8")
        except (AttributeError, ValueError):
            pass
    parser = argparse.ArgumentParser(description="Zielbild pruefen, abgleichen, anzeigen")
    parser.add_argument("befehl", choices=("pruefen", "abgleichen", "anzeige"))
    parser.add_argument("--breite", type=int, default=0, help="Fensterbreite in Zeichen (0 = Standard)")
    parser.add_argument("--farbe", action="store_true", help="ANSI-Farben fuer das Terminal")
    parser.add_argument("--kurz", action="store_true", help="nur eine Zeile Aktualitaet (Plan-Tab)")
    parser.add_argument("--wurzel", type=pathlib.Path, default=WURZEL_STANDARD,
                        help="Repo-Wurzel (fuer Proben)")
    args = parser.parse_args(argv)
    wurzel = args.wurzel.resolve()
    try:
        if args.befehl == "pruefen":
            return befehl_pruefen(wurzel)
        if args.befehl == "abgleichen":
            return befehl_abgleichen(wurzel)
        return befehl_anzeige(wurzel, args.breite, args.farbe, args.kurz)
    except Werkzeugfehler as e:
        print(f"ZIELBILD NICHT PRÜFBAR · {e}", file=sys.stderr)
        return EXIT_WERKZEUG


if __name__ == "__main__":
    sys.exit(main())
