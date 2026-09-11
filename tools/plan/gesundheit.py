#!/usr/bin/env python3
"""Gesundheit — misst Codebase und Kontext gegen Schwellen und meldet, was reisst.

Aufruf vom Workspace-Root:

    py -3.13 tools/plan/gesundheit.py [--clippy] [--json] [--selbsttest]

Exitcode 0 = keine Grenze gerissen · 4 = mindestens eine Grenze gerissen ·
2 = das Werkzeug selbst ist kaputt (Messort fehlt, cargo clippy bricht ab,
Selbsttest rot) ·
3 = eine ausdruecklich angeforderte Voraussetzung fehlt (nur bei --clippy ohne
cargo). Nur Standardbibliothek.

WARUM ES DIESES SKRIPT GIBT (User 08.09.2026, Register NAK-223): „ich moechte
dass die codebase selbst regelmaessig auf solche probleme ueberprueft und
verbessert wird. das projekt muss nachhaltig funktionieren und wartbar bleiben.
das geht nur wenn die codebase regelmaessig in perfektem shape bleibt. nicht
immer nur dann wenn hart an der grenze ist. … ich will nicht dass deine eine
kleine nebentaetigkeit ist, sondern bestandteil vom plan . da ich kein coder
bin, muss ich den code fuer di KI so wartbar wie moeglich halten."

Pflege war bis dahin eine Handmessung im Abschlussfenster des Dirigenten
(`Measure-Object -Line` auf die groessten Quelldateien). Eine Handmessung
misst, woran jemand denkt. Dieses Skript misst dasselbe jedes Mal gleich, mit
Schwellen, die aus einer Messung stammen und nicht aus einem Gefuehl.

WAS EINE SCHWELLE HIER BEDEUTET
-------------------------------
  ZIEL   ist ein Wunsch. Wird er verfehlt, sagt die Tabelle es — der Exitcode
         bleibt 0. Ein Ziel darf jahrelang offen sein.
  GRENZE reisst. Reisst eine, endet der Lauf mit Exit 4, und im Plan gehoert
         ein datierter Registerpunkt dazu, nie stilles Nachbessern.

Beide Woerter stehen so in der Tabelle. Ein Maass, das immer gruen ist, waere
ein totes Element (CLAUDE.md); deshalb sind die Grenzen fuer clippy und die
Kommentar-Bezeichner RATSCHEN auf dem gemessenen Iststand vom 09.09.2026 —
neue Funde reissen sofort, gesenkte Zahlen werden nachgezogen.

HERKUNFT JEDER SCHWELLE (Stand 09.09.2026, Basis-SHA 274d3ff8)
--------------------------------------------------------------
  Zeilen je Quelldatei   Grenze 2 000 / Ziel 1 500 — gesetzt durch Plan S25b
                         und CLAUDE.md („bis es gebaut ist, von Hand: keine
                         Quelldatei ueber 2 000 Zeilen"). HAELT seit NAK-225
                         (S25d, 09.09.2026): keine Quelldatei liegt mehr ueber
                         der Grenze, die Pflegeticket-Zuordnung ist leer. Der
                         naechste Treffer traegt „OHNE PFLEGETICKET" und ist
                         damit sichtbar neue Schuld; ein Eintrag, dessen Datei
                         die Grenze nicht mehr reisst, beendet den Lauf als
                         WERKZEUGFEHLER.
  Funktionen ueber 200   Massstab 200 Zeilen aus Plan S25b; Grenze fuer die
                         ANZAHL ist die Ratsche 28 = Iststand, Ziel 0.
  Clippy-Fundstellen     Grenze 91 = Iststand (Ratsche), Ziel 0.
  dead_code ohne Aufrufer Grenze 0 = Iststand 0.
  Kommentar-Bezeichner   Grenze 30 = Iststand (Ratsche), Ziel 0.
  Kontextflaechen        woertlich aus docs/context-hygiene-playbook.md und
                         Dirigenten-Skill Paragraph 3.5; alle fuenf halten
                         heute.

Seit NAK-225 (S25d, 09.09.2026) ist KEIN Maass mehr rot: die Zeilengrenze war
das letzte, und die drei Dateien, die sie rissen, sind geteilt. Jede Rotmeldung
ab hier ist echte Verschlechterung — es gibt keinen bekannten Rest mehr, hinter
dem sie sich verstecken koennte.

WAS GEMESSEN WIRD — CODEBASE
----------------------------
Quellorte: `broker/src`, `eq-copilot/plugin/src`, `eq-copilot/plugin/core`;
Endungen `.rs`, `.cpp`, `.h`, `.hpp`. Ausgenommen `broker/src/generiert/` —
das ist Codegen aus dem `.fbs` und wird von Bein A9 bytegleich gehalten; seine
Zeilen sind kein Wartungsaufwand (Ticketnachtrag 08.09.2026).

  (1) ZEILEN JE QUELLDATEI. Zeilenzahl = Anzahl `\\n` plus eins, wenn die Datei
      nicht mit einem Umbruch endet; die leere Datei hat 0. CRLF und LF zaehlen
      gleich, ein UTF-8-BOM wird verworfen statt mitgezaehlt.

  (2) FUNKTIONEN UEBER 200 ZEILEN. Heuristik ueber Klammertiefe auf einem
      Text, in dem Kommentare, Strings, Rohstrings und Zeichenliterale durch
      Leerzeichen ersetzt sind (Zeilenstruktur bleibt erhalten). Ein Block
      `{...}` gilt als Funktion, wenn der Text zwischen dem letzten `;`/`{`/`}`
      und der oeffnenden Klammer eine Signatur ist:

        Rust  — enthaelt `fn NAME` und eine geschlossene Parameterliste.
        C++   — endet auf `)` plus erlaubte Qualifikatoren (`const`,
                `noexcept`, `override`, `final`, `-> typ`, Initialisiererliste)
                und beginnt nicht mit `if`/`for`/`while`/`switch`/`catch`/
                `else`/`do`.

      Laenge = Zeile der schliessenden Klammer minus Zeile des Signaturanfangs
      plus eins. FEHLERKLASSEN, ehrlich benannt und nicht behoben:
        * C++-Lambdas werden erkannt und BEWUSST NICHT gezaehlt (Vortext endet
          auf `]` vor der Parameterliste) — ihre Zeilen zaehlen zur
          umgebenden Funktion. Die Zahl der uebersprungenen Lambdas steht im
          Bericht.
        * Ein funktionsartiges Makro `MAKRO(x) { … }` in C++ ist von einer
          Funktionsdefinition textuell nicht unterscheidbar und wird gezaehlt.
        * Rust-Closures (`|x| { … }`) sind keine Kandidaten (kein `fn`).
        * Ein Praeprozessorzweig mit unbalancierten Klammern verschiebt die
          Tiefenzaehlung; im gemessenen Bestand kommt keiner vor.
        * `macro_rules!`-Rumpfe sind keine Kandidaten (kein `fn`).

  (3) CLIPPY-FUNDSTELLEN (nur mit `--clippy`, weil der Lauf baut). Gefahren
      wird `cargo clippy --manifest-path broker/Cargo.toml --all-targets
      --message-format=json -- --cap-lints warn`. `--cap-lints warn` ist
      NOETIG: `broker/Cargo.toml` setzt `undocumented_unsafe_blocks = "deny"`,
      der Lauf braeche nach 26 Fehlern ab und meldete nur einen Teil der
      Funde (gemessen 09.09.2026: 36 gemeldet statt 91). Gezaehlt werden
      EINDEUTIGE Fundstellen ueber (Datei, Zeile, Spalte, Lint-Code) — ohne
      Dedup zaehlte dieselbe Diagnose fuer `lib` und `lib test` doppelt.
      Ein Exitcode != 0 des Prozesses ist ein WERKZEUGFEHLER (Exit 2), auch
      wenn der Strom schon Diagnosen trug — ein Compilerfehler schreibt selbst
      eine, und die Teilmessung eines abgebrochenen Laufs waere keine
      Warnungszahl. „Nicht messbar" bleibt allein dem Lauf OHNE `--clippy` und
      der Umgebung ohne cargo.

  (4) AUFRUFERLOSE `#[allow(dead_code)]`-HELFER. Zum Attribut wird der erste
      folgende Bezeichner gesucht (`fn`, `struct`, `enum`, `trait`, `type`,
      `const`, `static`, `union`, `mod` — auf derselben oder einer der
      naechsten Zeilen, Attribute und Kommentare uebersprungen; steht direkt
      hinter dem Attribut ein Typname, gilt dieser). Qualifizierer zaehlen
      nicht als Name: `pub`, `pub(crate)`, `unsafe`, `async`, `extern "C"`
      und `default` stehen vor dem Schluesselwort, `const fn name` und
      `static mut NAME` zwischen Schluesselwort und Bezeichner. `const NAME`
      bleibt davon unterschieden. Befund, wenn der Name
      ausserhalb seiner Definitionszeile nirgends im Crate vorkommt. Ein
      Attribut ohne erkennbaren Bezeichner wird gezaehlt und benannt, nicht
      verschwiegen. FEHLERKLASSEN: ein in `#[cfg_attr(…, allow(dead_code))]`
      verpacktes Attribut wird NICHT gesehen (die Form beginnt nicht mit
      `#[allow`), und ein Typ, der nur noch in `impl Foo` vorkommt, gilt als
      verwendet.

  (5) KOMMENTARE MIT NICHT EXISTENTEN BEZEICHNERN. Kandidat ist eine
      Backtick-Spanne in einem Kommentar der Quellorte, die genau wie ein
      Bezeichner aussieht (`[A-Za-z_][A-Za-z0-9_]*`, mindestens 4 Zeichen),
      kein Schluesselwort und kein gewoehnliches Wort aus der Stoppliste ist.
      Befund, wenn der Name in KEINER Rust- oder C++-Quelle des Repos
      ausserhalb von Kommentaren als Wort vorkommt. Heuristik: ein Kommentar
      darf einen Namen aus einem anderen Repo, aus FL Studio oder aus einem
      Schema nennen — solche Faelle sind die Fehlerrate, die im Manifest steht.

WAS GEMESSEN WIRD — KONTEXT
---------------------------
Bytes von `MEMORY.md` (Ziel 22 KB, Grenze 24,4 KB — darueber schneidet der
Harness still ab), Root-`CLAUDE.md` (Grenze 20 KB) und
`.claude/skills/dirigent/SKILL.md` (Grenze 24 KB); Index-Zeilen in MEMORY.md
ueber 250 Zeichen; Memory-Dateien ohne Link im Index. Alle Werte woertlich aus
`docs/context-hygiene-playbook.md` und Dirigenten-Skill Paragraph 3.5.

Der Memory-Ordner ist RECHNERLOKAL (`~/.claude/projects/<slug>/memory/`, Slug
aus dem absoluten Repo-Pfad). Fehlt er — anderer Rechner, andere Installation —
melden diese drei Maasse „nicht messbar" und sind nie rot. Ein Werkzeug, das
auf dem Laptop rot wird, weil dort ein Ordner anders heisst, misst den Rechner
und nicht das Projekt.

PRUEFSCHALTER (fuer Rotbeweise, nie im Kanon)
---------------------------------------------
  --wurzel <pfad>      misst einen anderen Baum (synthetische Verletzung).
  --memory <pfad>      misst einen anderen Memory-Ordner.
  --einspeisen M=N     setzt den Istwert eines Maasses auf N. Jede Zeile, die
                       so entstand, traegt in Tabelle und JSON die Marke
                       EINGESPEIST — sie ist nie eine Messung.
  --selbsttest         faehrt die eingebauten Faelle und misst sonst nichts.
"""
from __future__ import annotations

import argparse
import json
import pathlib
import re
import shutil
import subprocess
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]

# ------------------------------------------------------------------ Messorte

QUELLORTE = ("broker/src", "eq-copilot/plugin/src", "eq-copilot/plugin/core")
QUELLENDUNGEN = (".rs", ".cpp", ".h", ".hpp")
# Codegen aus dem .fbs. Bein A9 haelt ihn bytegleich zur Neuerzeugung; wer ihn
# von Hand kuerzte, braeche A9. Seine Zeilen sind deshalb kein Wartungsaufwand.
AUSGENOMMEN = ("broker/src/generiert",)

# ------------------------------------------------------------------ Schwellen

ZEILEN_GRENZE = 2000
ZEILEN_ZIEL = 1500
FUNKTION_GRENZE = 200

# Ratschen auf dem Iststand. Sie halten heute und reissen bei der ERSTEN
# Verschlechterung. Sinkt eine Zahl durch ein Pflegeticket, wird die Ratsche im
# selben Ticket nachgezogen — sonst deckt sie stillschweigend wieder Platz fuer
# neue Funde. Eine Ratsche wird nur GESENKT, nie erhoeht.
#
# Stand 09.09.2026 nach NAK-224 (S25c, Aufteilung hypothese.rs, server_v3/mod.rs
# und lib.rs): Kommentar-Bezeichner von 35 auf 32 gesenkt — drei Kommentare der
# drei Ticketdateien nannten Bezeichner, die es nicht gibt, und sind auf den
# wahren Namen gesetzt. Funktionen ueber 200 Zeilen und Clippy sind unveraendert:
# S25c teilt Dateien und zerlegt bewusst keine Funktion (NAK-235 ist ein eigener
# Schritt).
#
# Stand 09.09.2026 nach NAK-225 (S25d, Aufteilung PluginProcessor.cpp,
# FeatureEngine.h, ControlClient.cpp): Kommentar-Bezeichner von 32 auf 30
# gesenkt — zwei Kommentare der Ticketdateien nannten entfallene Namen
# (`assistentSchritt`, `commandIdVorgabe`) ohne Historienmarker im eigenen
# Umfeld und tragen ihn jetzt. Funktionen ueber 200 Zeilen und Clippy wieder
# unveraendert, aus demselben Grund. Die Zeilengrenze reisst seit S25d KEINE
# Datei mehr; die Pflegeticket-Zuordnung ist leer.
#
# Warum die Anzahl langer Funktionen eine Ratsche ist und keine 0: 200 Zeilen
# je Funktion ist der Massstab (Plan S25b), aber die 28 heutigen Treffer haben
# kein Pflegeticket. Eine 0 stuende dauerhaft rot und saehe nach Normalzustand
# aus; die Ratsche schlaegt bei der 29. an. Die 28 stehen als Nebenbefund im
# Manifest NAK-223 Paragraph 4.
FUNKTION_ANZAHL_GRENZE = 28
CLIPPY_GRENZE = 91
CLIPPY_ZIEL = 0
DEADCODE_GRENZE = 0
KOMMENTAR_GRENZE = 30
KOMMENTAR_ZIEL = 0

# Woertlich aus docs/context-hygiene-playbook.md und Dirigenten-Skill 3.5.
MEMORY_ZIEL = 22528       # 22 KB Arbeitsziel
MEMORY_GRENZE = 24986     # 24,4 KB — darueber schneidet der Harness still ab
CLAUDEMD_GRENZE = 20480   # 20 KB (seit 12.09.2026; vorher 24 KB)
SKILL_GRENZE = 24576      # 24 KB (seit 12.09.2026; vorher 36 KB)
INDEXZEILE_GRENZE = 250
SKILL_PFAD = ".claude/skills/dirigent/SKILL.md"

# Wer die 2 000er-Grenze heute reisst, hat ein Pflegeticket. Die Zuordnung
# steht hier, damit die Trefferliste einen Treffer OHNE Ticket sofort sichtbar
# macht — das ist der Unterschied zwischen bekannter Schuld und neuer.
#
# NAK-224 (S25c, 09.09.2026) hat `broker/src/coordinator/hypothese.rs` und
# `broker/src/transport/server_v3/mod.rs` aufgeteilt; beide liegen nicht mehr
# ueber der Grenze und stehen deshalb nicht mehr hier. Ein Eintrag fuer eine
# Datei, die die Grenze nicht mehr reisst, waere eine Schuld, die es nicht gibt.
#
# NAK-225 (S25d, 09.09.2026) hat `PluginProcessor.cpp`, `FeatureEngine.h` und
# `ControlClient.cpp` aufgeteilt. Damit reisst KEINE Datei die Grenze mehr, und
# die Zuordnung ist leer. Leer heisst hier nicht „vergessen": `veraltete_tickets`
# unten sorgt dafuer, dass ein Eintrag, dessen Datei die Grenze nicht mehr
# reisst, den Lauf als WERKZEUGFEHLER beendet — die Liste kann also gar nicht
# still veralten. Der naechste Treffer ueber der Grenze traegt
# „OHNE PFLEGETICKET" und ist damit sichtbar neue Schuld.
PFLEGETICKETS: dict[str, str] = {}

TREFFER_ZEIGEN = 25   # laengere Listen werden gekappt, mit Restzahl

# ------------------------------------------------------------------ Maskierung

# Die exakte Form eines Zeichenliterals in Rust und C++: genau ein Zeichen oder
# eine Escape-Folge zwischen zwei Hochkommas. Was nicht so aussieht, ist eine
# Rust-Lebensdauer (`'a`) oder ein C++-Ziffernstrenner (`1'000`).
ZEICHENLITERAL = re.compile(
    r"'(?:\\(?:x[0-9a-fA-F]{1,2}|u\{[0-9a-fA-F]{1,6}\}|u[0-9a-fA-F]{4}"
    r"|U[0-9a-fA-F]{8}|[0-7]{1,3}|.)|[^\\'\n])'")


def maskiere(text: str, rust: bool):
    """Ersetzt Kommentare und Literale durch Leerzeichen; Zeilenstruktur bleibt.

    Liefert (maskierter_text, kommentar_spannen). Eine Spanne ist (start, ende)
    im ORIGINALTEXT und deckt den Kommentarinhalt ohne die Einleitung ab.

    Warum maskiert wird: eine Klammer in `"}"` oder in einem Kommentar wuerde
    die Tiefenzaehlung der Funktionsmessung verschieben, und ein Backtick in
    einem String waere ein Scheinkandidat der Kommentarmessung. Die Zeichen
    werden durch Leerzeichen ersetzt statt entfernt, damit jede Position und
    jede Zeilennummer im maskierten Text dieselbe bleibt wie im Original.
    """
    n = len(text)
    aus = list(text)
    spannen = []
    i = 0

    def leeren(a: int, b: int) -> None:
        for k in range(a, min(b, n)):
            if aus[k] != "\n":
                aus[k] = " "

    while i < n:
        c = text[i]

        # --- Zeilenkommentar ------------------------------------------------
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            if j < 0:
                j = n
            spannen.append((i + 2, j))
            leeren(i, j)
            i = j
            continue

        # --- Blockkommentar (Rust: verschachtelbar) -------------------------
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = i + 2
            tiefe = 1
            while j < n:
                if rust and text.startswith("/*", j):
                    tiefe += 1
                    j += 2
                    continue
                if text.startswith("*/", j):
                    tiefe -= 1
                    j += 2
                    if tiefe == 0:
                        break
                    continue
                j += 1
            spannen.append((i + 2, max(i + 2, j - 2)))
            leeren(i, j)
            i = j
            continue

        # --- Rust-Rohstring r"…", r#"…"#, br##"…"## -------------------------
        if rust and c in "rb":
            m = re.match(r'(?:b?r)(#*)"', text[i:i + 40])
            if m and (c == "r" or text.startswith("br", i)):
                # Kein Rohstring, wenn davor ein Bezeichnerzeichen steht
                # (`for` endet auf `r`, `foo_r"` gibt es nicht).
                if i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_"):
                    rauten = m.group(1)
                    ende_marke = '"' + rauten
                    start = i + m.end()
                    j = text.find(ende_marke, start)
                    j = n if j < 0 else j + len(ende_marke)
                    leeren(i, j)
                    i = j
                    continue

        # --- C++-Rohstring R"delim(…)delim" ---------------------------------
        if not rust and c == "R" and i + 1 < n and text[i + 1] == '"':
            m = re.match(r'R"([^()\\ ]{0,16})\(', text[i:])
            if m and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")):
                ende_marke = ")" + m.group(1) + '"'
                j = text.find(ende_marke, i + m.end())
                j = n if j < 0 else j + len(ende_marke)
                leeren(i, j)
                i = j
                continue

        # --- gewoehnlicher String (laeuft ueber Zeilen) ----------------------
        if c == '"':
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == '"':
                    j += 1
                    break
                j += 1
            leeren(i, j)
            i = j
            continue

        # --- Zeichenliteral --------------------------------------------------
        # Ein `'` ist nicht immer ein Literal: in Rust beginnt es eine
        # Lebensdauer (`'a`), in C++ trennt es Ziffern (`1'000`). Ein Fenster
        # ("Schlusszeichen in den naechsten N Zeichen") reicht NICHT: in
        # `struct S<'a> { x: &'a str }` faende es das zweite `'a` und
        # maskierte die Klammer dazwischen weg. Deshalb entscheidet die exakte
        # Form eines Literals — trifft sie nicht, ist das `'` gewoehnlicher
        # Text und die Zaehlung bleibt heil.
        if c == "'":
            m = ZEICHENLITERAL.match(text, i)
            if m:
                leeren(i, m.end())
                i = m.end()
                continue
            i += 1
            continue

        i += 1

    return "".join(aus), spannen


# ------------------------------------------------------- Maass 1: Dateizeilen


def veraltete_tickets(ueber_grenze: list[str], tickets: dict[str, str]) -> list[str]:
    """Pflegeticket-Eintraege, deren Datei die Grenze nicht mehr reisst.

    Die Zuordnung oben behauptet: wer die Grenze reisst, hat ein Ticket. Bleibt
    ein Eintrag stehen, nachdem die Datei geteilt wurde, behauptet sie eine
    Schuld, die es nicht gibt — und der naechste Leser glaubt sie. Ein solcher
    Eintrag ist deshalb ein WERKZEUGfehler, kein Befund: nicht die Codebase ist
    schlechter geworden, das Mass stimmt nicht mehr.
    """
    ueber = set(ueber_grenze)
    return sorted(p for p in tickets if p not in ueber)


def zeilen_zaehlen(text: str) -> int:
    """Zeilen einer Datei. Eine letzte Zeile ohne Schlussumbruch zaehlt mit."""
    if not text:
        return 0
    return text.count("\n") + (0 if text.endswith("\n") else 1)


def lies_text(pfad: pathlib.Path) -> str:
    """Liest eine Quelldatei; ein UTF-8-BOM wird verworfen, nicht mitgezaehlt."""
    roh = pfad.read_bytes()
    return roh.decode("utf-8-sig", errors="replace")


def sammle_quellen(wurzel: pathlib.Path):
    """Alle Quelldateien der Messorte, sortiert, ohne die Ausnahmen.

    Ein FEHLENDER Messort ist ein Werkzeugfehler, kein leeres Ergebnis. Das
    Kontext-Hygiene-Playbook nennt die Klasse beim Namen: „Ein Prüfkommando auf
    einen verschobenen Pfad liefert still 0 und lässt Veraltetes verifiziert
    aussehen." Wird `eq-copilot/plugin/core` umbenannt, faellt ohne diese
    Wache ein Drittel der Codebase unbemerkt aus der Messung — und das Werkzeug
    meldete gruen.
    """
    fehlend = [ort for ort in QUELLORTE if not (wurzel / ort).is_dir()]
    if fehlend:
        raise RuntimeError(
            "Messort fehlt: " + ", ".join(fehlend)
            + f" (unter {wurzel}). Umbenannt oder verschoben? "
              "Die Ortsliste QUELLORTE gehoert nachgezogen.")
    treffer = []
    for ort in QUELLORTE:
        basis = wurzel / ort
        for pfad in basis.rglob("*"):
            if not pfad.is_file() or pfad.suffix not in QUELLENDUNGEN:
                continue
            rel = pfad.relative_to(wurzel).as_posix()
            if any(rel.startswith(a + "/") or rel == a for a in AUSGENOMMEN):
                continue
            treffer.append((rel, pfad))
    return sorted(treffer)


# -------------------------------------------------- Maass 2: lange Funktionen

RUST_FN = re.compile(r"\bfn\s+([A-Za-z_][A-Za-z0-9_]*)")
CPP_KOPF = re.compile(r"^\s*(if|for|while|switch|catch|else|do|return)\b")
CPP_SCHWANZ = re.compile(
    r"\)\s*"
    r"(?:const\s*)?(?:volatile\s*)?(?:&{1,2}\s*)?"
    r"(?:noexcept\s*(?:\([^()]*\)\s*)?)?"
    r"(?:const\s*)?(?:override\s*)?(?:final\s*)?"
    r"(?:->[^;{]*)?"
    r"(?::[^;{]*)?"
    r"$"
)
CPP_LAMBDA = re.compile(r"\][^;{]*\)\s*(?:mutable\s*)?(?:noexcept\s*)?(?:->[^;{]*)?$")


def ist_signatur(vortext: str, rust: bool):
    """Urteil ueber den Text vor einer oeffnenden Klammer.

    Liefert 'fn' (Funktionsdefinition), 'lambda' (bewusst nicht gezaehlt) oder
    None. Der Vortext ist alles zwischen dem letzten `;`/`{`/`}` und der
    Klammer, mit maskierten Kommentaren und Literalen.
    """
    s = vortext.strip()
    if not s:
        return None
    if rust:
        if not RUST_FN.search(s):
            return None
        if ")" not in s:
            return None
        return "fn"
    if CPP_KOPF.match(s):
        return None
    if CPP_LAMBDA.search(s):
        return "lambda"
    if not CPP_SCHWANZ.search(s):
        return None
    # `class X : public Y {` endet ebenfalls auf eine Doppelpunkt-Liste, hat
    # aber keine Parameterliste vor dem Doppelpunkt.
    if re.match(r"^(class|struct|union|enum|namespace)\b", s) and "(" not in s:
        return None
    return "fn"


def zeile_von(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1


def finde_lange_funktionen(rel: str, text: str, rust: bool, grenze: int):
    """Funktionen ueber der Grenze. Liefert (treffer, zahl_uebersprungener_lambdas)."""
    maskiert, _ = maskiere(text, rust)
    treffer = []
    lambdas = 0
    stapel = []
    letzter_trenner = 0
    for m in re.finditer(r"[{};]", maskiert):
        z = m.group(0)
        p = m.start()
        if z == "{":
            vortext = maskiert[letzter_trenner:p]
            art = ist_signatur(vortext, rust)
            if art == "lambda":
                lambdas += 1
            anfang = letzter_trenner + (len(vortext) - len(vortext.lstrip()))
            stapel.append((art, anfang))
            letzter_trenner = p + 1
        elif z == "}":
            letzter_trenner = p + 1
            if not stapel:
                continue
            art, anfang = stapel.pop()
            if art != "fn":
                continue
            von = zeile_von(maskiert, anfang)
            bis = zeile_von(maskiert, p)
            laenge = bis - von + 1
            if laenge > grenze:
                name = "(unbenannt)"
                kopf = maskiert[anfang:anfang + 400]
                mm = RUST_FN.search(kopf) if rust else re.search(
                    r"([A-Za-z_][A-Za-z0-9_:~]*)\s*\(", kopf)
                if mm:
                    name = mm.group(1)
                treffer.append((rel, von, laenge, name))
        else:
            letzter_trenner = p + 1
    return treffer, lambdas


# ----------------------------------------------------------- Maass 3: clippy


def miss_clippy(wurzel: pathlib.Path):
    """Eindeutige clippy-Fundstellen des Brokers. Liefert (zahl, treffer, fehler).

    `fehler` traegt ausschliesslich den Umgebungsgrund „cargo nicht gefunden";
    nur dann ist das Mass NICHT MESSBAR. Jeder WERKZEUGfehler — fehlender
    Messort, abgebrochener Prozess — wirft `RuntimeError` und endet im
    Hauptlauf als „Werkzeugfehler" mit Exit 2, genau wie ein fehlender
    Quellort (Selbstaudit 092db4de).
    """
    manifest = wurzel / "broker" / "Cargo.toml"
    if not manifest.is_file():
        # Derselbe Fall wie ein verschobener Quellort: wer --clippy anfordert
        # und den Messort nicht findet, hat ein kaputtes Werkzeug vor sich,
        # keine Umgebung ohne cargo. NICHT MESSBAR waere hier ein stilles Null.
        raise RuntimeError(f"Messort fuer clippy fehlt: {manifest}")
    if shutil.which("cargo") is None:
        return None, [], "cargo nicht gefunden"
    befehl = [
        "cargo", "clippy",
        "--manifest-path", str(manifest),
        "--all-targets",
        "--message-format=json",
        "--color", "never",
        "--", "--cap-lints", "warn",
    ]
    lauf = subprocess.run(befehl, cwd=str(wurzel), capture_output=True, text=True,
                          encoding="utf-8", errors="replace")
    # Der Exitcode wird VOR den Diagnosen gelesen und ohne jede Bedingung.
    # Ein abgebrochener Lauf hat nicht zu Ende gemessen — auch dann nicht, wenn
    # der Strom schon Diagnosen trug: ein Compilerfehler (E0308, Exit 101)
    # schreibt selbst eine. Wer erst zaehlt und den Exitcode nur bei LEERER
    # Fundmenge ansieht, reicht eine Teilmessung als gueltige Warnungszahl
    # durch und der Gesamtlauf endet mit 0 (Erstpruefung 09.09.2026, Defekt 2).
    if lauf.returncode != 0:
        kurz = (lauf.stderr or "").strip().splitlines()
        raise RuntimeError(f"cargo clippy Exit {lauf.returncode} - "
                           + (kurz[-1] if kurz else "ohne Ausgabe auf stderr"))
    fund = {}
    for zeile in (lauf.stdout or "").splitlines():
        zeile = zeile.strip()
        if not zeile.startswith("{"):
            continue
        try:
            o = json.loads(zeile)
        except json.JSONDecodeError:
            continue
        if o.get("reason") != "compiler-message":
            continue
        m = o.get("message") or {}
        if m.get("level") not in ("warning", "error"):
            continue
        code = (m.get("code") or {}).get("code")
        if not code:
            # Sammelzeilen wie „generated 36 warnings" tragen keinen Code.
            continue
        haupt = [s for s in (m.get("spans") or []) if s.get("is_primary")]
        if haupt:
            s = haupt[0]
            ort = (s.get("file_name", "?").replace("\\", "/"),
                   s.get("line_start", 0), s.get("column_start", 0))
        else:
            ort = ("?", 0, 0)
        fund[(ort, code)] = m.get("message", "")
    treffer = sorted(f"{o[0]}:{o[1]}:{o[2]} {c}" for (o, c) in fund)
    return len(fund), treffer, ""


# -------------------------------------------------------- Maass 4: dead_code

DEADCODE_ATTR = re.compile(r"#\[\s*allow\s*\(([^)]*)\)\s*\]")
# Der Bezeichner steht NICHT immer hinter dem ersten Schluesselwort, aber er
# steht IMMER unmittelbar hinter dem ELEMENTschluesselwort. Die Grammatik ist
# `[Qualifizierer]* Schluesselwort [mut]? Name`: Qualifizierer (`pub`,
# `pub(crate)`, `pub(in …)`, `const`, `unsafe`, `async`, `default`,
# `extern "C"`) stehen nur DAVOR und werden nur uebersprungen, wenn ein
# Schluesselwort folgt; `mut` nur unmittelbar nach `static`. Wer stumpf hinter
# dem ERSTEN Treffer liest, erfasst bei `pub const fn helfer` das Wort `fn` als
# Namen (Erstpruefung 09.09.2026, Defekt 1). Wer die Wiederholung stattdessen
# HINTER das Schluesselwort legt, ueberspringt bei `struct union where` den
# echten Namen `union` und liest die fremde `where`-Klausel als Bezeichner
# (Wiederpruefung 1, 09.09.2026). Beide Male gilt danach jede andere Definition
# im Crate als Verwendung, und der aufruferlose Helfer bleibt strukturell
# ungemeldet — das Maass ist stumm. Das erste Wort nach dem Schluesselwort ist
# deshalb ausnahmslos der Name, auch wenn es `union`, `default` oder `where`
# heisst. `const` ist Qualifizierer, solange ein Schluesselwort folgt
# (`const fn name`), sonst selbst das Schluesselwort (`const NAME: T`); das
# Zurueckfallen erledigt die Wiederholung durch Backtracking.
SCHLUESSELWORT = r"fn|struct|enum|trait|type|const|static|union|mod"
QUALIFIZIERER = (r"pub\b(?:\s*\([^)\n]*\))?|const\b|unsafe\b|async\b"
                 r"|default\b|extern\b(?:\s*\"[^\"\n]*\")?")
ELEMENT = re.compile(rf"\b(?:(?:{QUALIFIZIERER})\s+)*"
                     rf"(?:static\s+mut|{SCHLUESSELWORT})\b"
                     r"\s+([A-Za-z_][A-Za-z0-9_]*)")
WORT = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")


def finde_dead_code(quellen, texte):
    """`#[allow(dead_code)]`-Helfer ohne jede Verwendung im Crate."""
    treffer = []
    # Wortmenge des Crates, Definitionszeilen bleiben drin — sie werden beim
    # Vergleich einzeln abgezogen.
    zeilen_index = {}
    for rel, _ in quellen:
        if not rel.endswith(".rs"):
            continue
        maskiert, _ = maskiere(texte[rel], True)
        zeilen_index[rel] = maskiert.splitlines()

    for rel, _ in quellen:
        if not rel.endswith(".rs"):
            continue
        zeilen = zeilen_index[rel]
        for nr, zeile in enumerate(zeilen, start=1):
            for m in DEADCODE_ATTR.finditer(zeile):
                if "dead_code" not in [t.strip() for t in m.group(1).split(",")]:
                    continue
                # Bezeichner: erst hinter dem Attribut auf derselben Zeile,
                # dann in den naechsten Zeilen (Attribute/Leerzeilen ueberspringen).
                rest = zeile[m.end():]
                name, def_zeile = None, nr
                mm = ELEMENT.search(rest)
                if mm:
                    name = mm.group(1)
                else:
                    mw = WORT.search(rest)
                    if mw:
                        name = mw.group(0)
                if name is None:
                    for versatz, folge in enumerate(zeilen[nr:nr + 6], start=1):
                        s = folge.strip()
                        if not s or s.startswith("#["):
                            continue
                        mm = ELEMENT.search(folge)
                        if mm:
                            name = mm.group(1)
                            def_zeile = nr + versatz
                        break
                if name is None:
                    treffer.append((rel, nr, "(kein Bezeichner erkannt)"))
                    continue
                # Die DEFINITIONSZEILE wird ausgenommen, nicht die Attributzeile:
                # der Name steht im Regel nicht neben dem Attribut, sondern eine
                # Zeile darunter — wer die falsche Zeile ausnimmt, findet den
                # Namen dort und meldet nie einen Befund.
                anderswo = False
                for arel, azeilen in zeilen_index.items():
                    for anr, azeile in enumerate(azeilen, start=1):
                        if arel == rel and anr in (nr, def_zeile):
                            continue
                        if name in azeile and name in WORT.findall(azeile):
                            anderswo = True
                            break
                    if anderswo:
                        break
                if not anderswo:
                    treffer.append((rel, nr, name))
    return sorted(treffer)


# ------------------------------------------- Maass 5: Kommentar-Bezeichner

BEZEICHNER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
BACKTICK = re.compile(r"`([^`\n]{1,80})`")

# Woerter, die wie ein Bezeichner aussehen, in einem deutschen oder englischen
# Kommentar aber gewoehnliche Sprache sind. Ohne diese Liste meldete das Maass
# jeden hervorgehobenen Begriff als fehlenden Bezeichner.
STOPPWORTE = frozenset("""
None Some true false null nullptr void auto const static struct class enum union
public private protected virtual override final inline extern template typename
namespace using return break continue while else match impl trait where unsafe
async await move self Self super crate pub let mut type dyn ref box loop
sizeof alignof constexpr noexcept explicit friend operator delete goto
bool char short long float double signed unsigned size_t uint8_t uint16_t
uint32_t uint64_t int8_t int16_t int32_t int64_t usize isize
String Vec Option Result Box Arc Rc Mutex RwLock HashMap BTreeMap VecDeque
true_ false_ TODO FIXME NOTE HACK XXX
oder aber nicht nur noch schon dann wenn dass sich auch eine einen einem
eines diese dieser dieses jeder jede jedes kein keine keinen alle alles
nach vor beim zum zur ohne durch gegen ueber unter zwischen
""".split())


# Umfeldausnahme, nach dem Muster von tools/plan/dokuriegel.py. Ein Kommentar
# DARF einen Bezeichner nennen, den es nicht mehr gibt — genau das ist die
# Historie eines Befunds („`TAIL_SCHRITT_JE_TICK` ist ENTFALLEN"), und der
# Nakama-Kanon verlangt sie. Ein solcher Verweis wird nicht gezaehlt; gezaehlt
# wird der Kommentar, der einen fehlenden Namen als GEGENWART behauptet.
# Gemessen 09.09.2026: ohne diese Ausnahme sind rund vier von fuenf Treffern
# markierte Historie.
HISTORIE = re.compile(
    r"entfall|entfaell|entfiel|gestrichen|umbenannt|hiess|historisch|Verlauf"
    r"|ehemals|frueher|zurueckgebaut|geloescht|entfernt|nicht mehr|stand bis"
    r"|gab es|haette|waere|wuerde|Runde\s*\d|bis zur Runde|seit dem Rueckbau",
    re.IGNORECASE)
HISTORIE_FENSTER = 200


def finde_kommentar_bezeichner(quellen, texte, repo_woerter):
    """Backtick-Bezeichner in Kommentaren, die im Repo nirgends vorkommen.

    Liefert (treffer, zahl_markierter_historie). Ein Treffer, dessen Umfeld
    (200 Zeichen davor bis 200 danach, im selben Kommentar) einen
    Historienmarker traegt, wird gezaehlt und benannt — aber nicht als Befund.
    """
    treffer = []
    historie = 0
    for rel, _ in quellen:
        rust = rel.endswith(".rs")
        text = texte[rel]
        _, spannen = maskiere(text, rust)
        for a, b in spannen:
            stueck = text[a:b]
            for m in BACKTICK.finditer(stueck):
                name = m.group(1).strip()
                if len(name) < 4 or not BEZEICHNER.match(name):
                    continue
                if name in STOPPWORTE or name in repo_woerter:
                    continue
                umfeld = text[max(a, a + m.start() - HISTORIE_FENSTER):
                              min(b, a + m.end() + HISTORIE_FENSTER)]
                if HISTORIE.search(umfeld):
                    historie += 1
                    continue
                treffer.append((rel, zeile_von(text, a + m.start()), name))
    # Dedup: derselbe Name in derselben Datei zaehlt einmal je Zeile.
    return sorted(set(treffer)), historie


def ohne_kommentare(text: str, spannen) -> str:
    """Originaltext mit geleerten Kommentaren — Literale bleiben stehen.

    Fuer die Klammerzaehlung muessen Strings weg; fuer die Existenzpruefung
    von Maass 5 muessen sie BLEIBEN. Ein Protokoll- oder Schemafeld
    (`event_log`, `assistant_step_update`) existiert im Code nur als
    Stringliteral; gemessen am maskierten Text meldete das Maass am 09.09.2026
    409 Treffer, von denen fast alle genau diese Klasse waren.
    """
    aus = list(text)
    for a, b in spannen:
        for k in range(a, min(b, len(aus))):
            if aus[k] != "\n":
                aus[k] = " "
    return "".join(aus)


# Nachweisbasis von Maass 5. Quellen UND Vertraege: ein Kommentar darf auf ein
# Schemafeld (`.json`, `.fbs`), ein Bau- oder Testziel (`CMakeLists.txt`,
# `Cargo.toml`) oder ein Werkzeugskript verweisen — das existiert, nur nicht in
# C++ oder Rust. Ohne diese Erweiterung meldete das Maass am 09.09.2026 107
# Treffer, deren Mehrheit genau diese Klasse war.
#
# `docs/` und `.md` bleiben BEWUSST draussen: Manifeste zitieren Bezeichner
# fortlaufend, und ein Kommentarverweis, den nur noch ein altes Manifest kennt,
# ist der Fund, den dieses Maass sucht — nicht sein Gegenbeweis.
NACHWEIS_ENDUNGEN = (".rs", ".cpp", ".h", ".hpp", ".py", ".json", ".fbs",
                     ".toml", ".ps1", ".cmake", ".txt", ".patch")
NACHWEIS_AUS = ("build", "target", ".git", "node_modules", "docs", "wissen")
NACHWEIS_MAXBYTES = 4 * 1024 * 1024


def sammle_repo_woerter(wurzel: pathlib.Path):
    """Alle Woerter der Quellen und Vertraege des Repos ausserhalb von
    Kommentaren — Bezeichner, Stringinhalte, Schluessel, Ziel- und Dateinamen.
    Basis der Existenzpruefung von Maass 5."""
    woerter = set()
    for pfad in sorted(wurzel.rglob("*")):
        if not pfad.is_file():
            continue
        teile = pfad.relative_to(wurzel).parts
        if any(t in NACHWEIS_AUS for t in teile[:-1]):
            continue
        # Der Dateiname selbst ist ein Bezeichner: ein Rust-Testziel heisst wie
        # seine Datei (`--test store_crash_matrix`), ein Modul wie sein Ordner.
        woerter.update(WORT.findall(pfad.stem))
        if pfad.suffix not in NACHWEIS_ENDUNGEN:
            continue
        try:
            if pfad.stat().st_size > NACHWEIS_MAXBYTES:
                continue
            text = lies_text(pfad)
        except OSError:
            continue
        if pfad.suffix in QUELLENDUNGEN:
            _, spannen = maskiere(text, pfad.suffix == ".rs")
            text = ohne_kommentare(text, spannen)
        woerter.update(WORT.findall(text))
    return woerter


# ------------------------------------------------------------------- Kontext


def memory_ordner(wurzel: pathlib.Path):
    """Rechnerlokaler Memory-Ordner. Slug = absoluter Repo-Pfad, `:`/`\\`/`/` zu `-`."""
    roh = str(wurzel)
    slug = roh.replace(":", "-").replace("\\", "-").replace("/", "-")
    return pathlib.Path.home() / ".claude" / "projects" / slug / "memory"


def miss_kontext(wurzel: pathlib.Path, memory: pathlib.Path):
    """Liefert die Kontextmaasse als Liste von Dictionaries."""
    ergebnis = []

    def bytes_von(pfad: pathlib.Path):
        try:
            return len(pfad.read_bytes())
        except OSError:
            return None

    mem_md = memory / "MEMORY.md"
    n = bytes_von(mem_md)
    ergebnis.append(dict(name="MEMORY.md (Bytes)", ist=n, ziel=MEMORY_ZIEL,
                         grenze=MEMORY_GRENZE, treffer=[],
                         nicht_messbar=None if n is not None else str(mem_md)))

    n = bytes_von(wurzel / "CLAUDE.md")
    ergebnis.append(dict(name="CLAUDE.md (Bytes)", ist=n, ziel=None,
                         grenze=CLAUDEMD_GRENZE, treffer=[],
                         nicht_messbar=None if n is not None else "CLAUDE.md"))

    n = bytes_von(wurzel / SKILL_PFAD)
    ergebnis.append(dict(name="Dirigenten-Skill (Bytes)", ist=n, ziel=None,
                         grenze=SKILL_GRENZE, treffer=[],
                         nicht_messbar=None if n is not None else SKILL_PFAD))

    lange, waisen, messbar = [], [], mem_md.is_file()
    if messbar:
        inhalt = mem_md.read_text(encoding="utf-8", errors="replace")
        for nr, zeile in enumerate(inhalt.splitlines(), start=1):
            if len(zeile) > INDEXZEILE_GRENZE:
                lange.append((nr, len(zeile)))
        for datei in sorted(memory.glob("*.md")):
            if datei.name == "MEMORY.md":
                continue
            slug = datei.stem
            if f"({datei.name})" in inhalt or f"[[{slug}]]" in inhalt:
                continue
            waisen.append(datei.name)

    ergebnis.append(dict(name="MEMORY.md: Zeilen ueber 250 Zeichen",
                         ist=len(lange) if messbar else None, ziel=0, grenze=0,
                         treffer=[f"Zeile {nr}: {ln} Zeichen" for nr, ln in lange],
                         nicht_messbar=None if messbar else str(mem_md)))
    ergebnis.append(dict(name="Memory-Dateien ohne Index-Link",
                         ist=len(waisen) if messbar else None, ziel=0, grenze=0,
                         treffer=waisen,
                         nicht_messbar=None if messbar else str(memory)))
    return ergebnis


# ------------------------------------------------------------------- Bericht


def bewerte(ist, ziel, grenze):
    """Ein Wort fuer eine Zahl. `None` heisst nicht messbar, nie rot."""
    if ist is None:
        return "NICHT MESSBAR"
    if grenze is not None and ist > grenze:
        return "GRENZE"
    if ziel is not None and ist > ziel:
        return "ZIEL"
    return "OK"


def zahl(x):
    return "-" if x is None else str(x)


def tabelle(maasse):
    kopf = ("Maass", "Ist", "Ziel", "Grenze", "Status")
    zeilen = [(m["name"], zahl(m["ist"]), zahl(m["ziel"]), zahl(m["grenze"]),
               m["status"] + (" [EINGESPEIST]" if m.get("eingespeist") else ""))
              for m in maasse]
    breiten = [max(len(kopf[i]), *(len(z[i]) for z in zeilen)) for i in range(5)]
    trenn = "  ".join("-" * b for b in breiten)
    aus = ["  ".join(kopf[i].ljust(breiten[i]) for i in range(5)), trenn]
    for z in zeilen:
        aus.append("  ".join(z[i].ljust(breiten[i]) for i in range(5)))
    return aus


def drucke_bericht(maasse, wurzel, memory, ueberschrieben):
    print("NAKAMA-GESUNDHEIT")
    print(f"Wurzel : {wurzel}" + ("  (UEBERSCHRIEBEN)" if ueberschrieben else ""))
    print(f"Memory : {memory}")
    # Ein eingespeister Wert steht zwar mit seiner Marke in der Zeile, aber die
    # Urteilszeile unten spraeche sonst von „allen Grenzen" und meinte eine
    # Messung, die es nicht gab. Der Kopf sagt es deshalb zuerst.
    gesetzt = [m["name"] for m in maasse if m.get("eingespeist")]
    if gesetzt:
        print("")
        print("ACHTUNG: dieser Bericht ist KEINE vollstaendige Messung - "
              f"{len(gesetzt)} Maass(e) per --einspeisen gesetzt: "
              + ", ".join(gesetzt))
    print("")
    for z in tabelle(maasse):
        print(z)
    print("")
    for m in maasse:
        if m["status"] == "NICHT MESSBAR":
            print(f"nicht messbar: {m['name']} - {m.get('nicht_messbar', 'Quelle fehlt')}")
    gerissen = [m for m in maasse if m["status"] == "GRENZE"]
    verfehlt = [m for m in maasse if m["status"] == "ZIEL"]
    for m in gerissen + verfehlt:
        if not m.get("treffer"):
            continue
        wort = "GRENZE gerissen" if m["status"] == "GRENZE" else "Ziel verfehlt"
        print("")
        print(f"{wort}: {m['name']} ({m['ist']} von hoechstens {zahl(m['grenze'])})")
        for t in m["treffer"][:TREFFER_ZEIGEN]:
            print(f"  {t}")
        rest = len(m["treffer"]) - TREFFER_ZEIGEN
        if rest > 0:
            print(f"  ... und {rest} weitere")
    for m in maasse:
        for h in m.get("hinweise", []):
            print(f"Hinweis: {h}")
    print("")
    if gerissen:
        namen = ", ".join(m["name"] for m in gerissen)
        print(f"URTEIL: GRENZE GERISSEN - {len(gerissen)} Maass(e): {namen}")
    else:
        print(f"URTEIL: alle Grenzen gehalten ({len(verfehlt)} Ziel(e) verfehlt)")


# ------------------------------------------------------------------- Messung


def miss(wurzel: pathlib.Path, memory: pathlib.Path, mit_clippy: bool):
    quellen = sammle_quellen(wurzel)
    if not quellen:
        raise RuntimeError(f"kein Messort gefunden unter {wurzel} ({', '.join(QUELLORTE)})")
    texte = {rel: lies_text(p) for rel, p in quellen}

    maasse = []

    # (1)+(2) Dateizeilen ----------------------------------------------------
    ueber_grenze, ueber_ziel = [], []
    for rel, _ in quellen:
        n = zeilen_zaehlen(texte[rel])
        if n > ZEILEN_GRENZE:
            ticket = PFLEGETICKETS.get(rel, "OHNE PFLEGETICKET")
            ueber_grenze.append((n, f"{rel}: {n} Zeilen [{ticket}]"))
        elif n > ZEILEN_ZIEL:
            ueber_ziel.append((n, f"{rel}: {n} Zeilen"))
    ohne_ticket = sum(1 for _, t in ueber_grenze if "OHNE PFLEGETICKET" in t)
    veraltet = veraltete_tickets([rel for rel, _ in quellen
                                  if zeilen_zaehlen(texte[rel]) > ZEILEN_GRENZE],
                                 PFLEGETICKETS)
    if veraltet:
        raise RuntimeError(
            "Pflegeticket-Zuordnung veraltet - diese Datei(en) reissen die "
            "Grenze nicht mehr und duerfen keine Schuld mehr tragen: "
            + ", ".join(veraltet))
    maasse.append(dict(
        name=f"Quelldateien ueber {ZEILEN_GRENZE} Zeilen", ist=len(ueber_grenze),
        ziel=0, grenze=0,
        treffer=[t for _, t in sorted(ueber_grenze, reverse=True)],
        hinweise=([] if ohne_ticket == 0 else
                  [f"{ohne_ticket} Datei(en) ueber der Grenze OHNE Pflegeticket - "
                   f"neuer Registerpunkt noetig"])))
    maasse.append(dict(
        name=f"Quelldateien ueber Ziel {ZEILEN_ZIEL} Zeilen",
        ist=len(ueber_ziel), ziel=0, grenze=None,
        treffer=[t for _, t in sorted(ueber_ziel, reverse=True)]))

    # (3) lange Funktionen ---------------------------------------------------
    lang, lambdas = [], 0
    for rel, _ in quellen:
        t, l = finde_lange_funktionen(rel, texte[rel], rel.endswith(".rs"), FUNKTION_GRENZE)
        lang.extend(t)
        lambdas += l
    lang.sort(key=lambda x: (-x[2], x[0], x[1]))
    maasse.append(dict(
        name=f"Funktionen ueber {FUNKTION_GRENZE} Zeilen", ist=len(lang),
        ziel=0, grenze=FUNKTION_ANZAHL_GRENZE,
        treffer=[f"{r}:{z} {n} - {ln} Zeilen" for r, z, ln, n in lang],
        hinweise=[f"{lambdas} C++-Lambda(s) bewusst nicht als eigene Funktion gezaehlt"]
        if lambdas else []))

    # (4) clippy -------------------------------------------------------------
    if mit_clippy:
        n, treffer, fehler = miss_clippy(wurzel)
        maasse.append(dict(name="Clippy-Fundstellen (broker)", ist=n,
                           ziel=CLIPPY_ZIEL, grenze=CLIPPY_GRENZE,
                           treffer=treffer, nicht_messbar=fehler or None))
    else:
        maasse.append(dict(name="Clippy-Fundstellen (broker)", ist=None,
                           ziel=CLIPPY_ZIEL, grenze=CLIPPY_GRENZE, treffer=[],
                           nicht_messbar="nicht angefordert (Schalter --clippy)"))

    # (5) dead_code ----------------------------------------------------------
    tot = finde_dead_code(quellen, texte)
    maasse.append(dict(name="allow(dead_code) ohne Aufrufer", ist=len(tot),
                       ziel=0, grenze=DEADCODE_GRENZE,
                       treffer=[f"{r}:{z} {n}" for r, z, n in tot]))

    # (6) Kommentar-Bezeichner ----------------------------------------------
    repo_woerter = sammle_repo_woerter(wurzel)
    komm, historie = finde_kommentar_bezeichner(quellen, texte, repo_woerter)
    maasse.append(dict(name="Kommentar-Bezeichner ohne Code", ist=len(komm),
                       ziel=KOMMENTAR_ZIEL, grenze=KOMMENTAR_GRENZE,
                       treffer=[f"{r}:{z} `{n}`" for r, z, n in komm],
                       hinweise=[f"{historie} weitere(r) Verweis(e) auf fehlende "
                                 f"Bezeichner sind als Historie markiert und nicht "
                                 f"gezaehlt"] if historie else []))

    maasse.extend(miss_kontext(wurzel, memory))
    return maasse


# ------------------------------------------------------------------ Selbsttest


def selbsttest() -> int:
    """Faelle im Speicher durch dieselben Funktionen. Jede Erwartung mit Gegenteil."""
    faelle = []

    def pruefe(name, ist, soll):
        faelle.append((name, ist == soll, f"ist={ist!r} soll={soll!r}"))

    # --- Zeilenzaehlung, alle Zahlenraender --------------------------------
    pruefe("leere Datei = 0", zeilen_zaehlen(""), 0)
    pruefe("eine Zeile ohne Umbruch = 1", zeilen_zaehlen("a"), 1)
    pruefe("eine Zeile mit Umbruch = 1", zeilen_zaehlen("a\n"), 1)
    pruefe("zwei Zeilen LF", zeilen_zaehlen("a\nb\n"), 2)
    pruefe("zwei Zeilen ohne Schlussumbruch", zeilen_zaehlen("a\nb"), 2)
    pruefe("zwei Zeilen CRLF", zeilen_zaehlen("a\r\nb\r\n"), 2)
    pruefe("nur Umbruch = 1", zeilen_zaehlen("\n"), 1)

    # --- Grenze: exakt 2 000 haelt, 2 001 reisst ---------------------------
    pruefe("2000 Zeilen halten", bewerte(zeilen_zaehlen("x\n" * 2000) > ZEILEN_GRENZE and 1 or 0,
                                        0, 0), "OK")
    pruefe("2001 Zeilen reissen", bewerte(zeilen_zaehlen("x\n" * 2001) > ZEILEN_GRENZE and 1 or 0,
                                          0, 0), "GRENZE")

    # --- BOM wird verworfen -------------------------------------------------
    pruefe("BOM zaehlt nicht als Zeile",
           zeilen_zaehlen("﻿a\nb\n".encode("utf-8").decode("utf-8-sig")), 2)

    # --- Pflegeticket-Zuordnung: veraltet = Werkzeugfehler ------------------
    # NAK-225 (S25d, 09.09.2026): die Zuordnung ist seit dem Schnitt leer. Leer
    # darf nicht heissen „unbeobachtet", also faellt hier beides — der Eintrag,
    # der ins Leere zeigt, UND die Datei, die kein Ticket hat.
    pruefe("leere Zuordnung ist nie veraltet", veraltete_tickets(["a.cpp"], {}), [])
    pruefe("Eintrag zu einer Datei ueber der Grenze ist aktuell",
           veraltete_tickets(["a.cpp"], {"a.cpp": "NAK-1"}), [])
    pruefe("Eintrag zu einer geteilten Datei ist veraltet",
           veraltete_tickets([], {"a.cpp": "NAK-1"}), ["a.cpp"])
    pruefe("zwei veraltete Eintraege kommen sortiert",
           veraltete_tickets(["c.cpp"], {"b.cpp": "NAK-1", "a.cpp": "NAK-2"}),
           ["a.cpp", "b.cpp"])
    pruefe("Datei ueber der Grenze ohne Eintrag ist kein Werkzeugfehler",
           veraltete_tickets(["a.cpp", "b.cpp"], {"a.cpp": "NAK-1"}), [])

    # --- Bewertung: Ziel ist kein Rot --------------------------------------
    pruefe("Ziel verfehlt heisst ZIEL", bewerte(5, 0, 10), "ZIEL")
    pruefe("Grenze gehalten heisst OK", bewerte(0, 0, 10), "OK")
    pruefe("Grenze gerissen heisst GRENZE", bewerte(11, 0, 10), "GRENZE")
    pruefe("Grenze exakt getroffen reisst nicht", bewerte(10, None, 10), "OK")
    pruefe("Grenze getroffen, Ziel verfehlt heisst ZIEL", bewerte(10, 0, 10), "ZIEL")
    pruefe("nicht messbar ist nie rot", bewerte(None, 0, 0), "NICHT MESSBAR")
    pruefe("Ratsche 91 haelt", bewerte(91, CLIPPY_ZIEL, CLIPPY_GRENZE), "ZIEL")
    pruefe("Ratsche 92 reisst", bewerte(92, CLIPPY_ZIEL, CLIPPY_GRENZE), "GRENZE")

    # --- Maskierung ---------------------------------------------------------
    m, _ = maskiere('let a = "}{";\n', True)
    pruefe("Klammern im String verschwinden", "{" in m or "}" in m, False)
    m, _ = maskiere("// } {\nlet a = 1;\n", True)
    pruefe("Klammern im Zeilenkommentar verschwinden", "{" in m or "}" in m, False)
    m, _ = maskiere("/* } */ let a = 1;\n", False)
    pruefe("Klammern im Blockkommentar verschwinden", "}" in m, False)
    m, _ = maskiere("/* /* } */ */ let a = 1;\n", True)
    pruefe("Rust-Blockkommentar verschachtelt", "}" in m, False)
    m, _ = maskiere('let a = r#"}{"#;\n', True)
    pruefe("Rust-Rohstring maskiert", "{" in m or "}" in m, False)
    m, _ = maskiere('auto a = R"x(})x";\n', False)
    pruefe("C++-Rohstring maskiert", "}" in m, False)
    m, _ = maskiere("struct S<'a> { x: &'a str }\n", True)
    pruefe("Lebensdauer bricht die Maskierung nicht", m.count("{"), 1)
    m, _ = maskiere("int a = 1'000; if (a) { }\n", False)
    pruefe("Ziffernstrenner bricht die Maskierung nicht", m.count("{"), 1)
    m, _ = maskiere("char c = '}';\nint x;\n", False)
    pruefe("Zeichenliteral maskiert", "}" in m, False)
    m, _ = maskiere("let a = 1;\nlet b = 2;\n", True)
    pruefe("Maskierung erhaelt Zeilenzahl", m.count("\n"), 2)
    _, sp = maskiere("// hallo `Foo`\n", True)
    pruefe("Kommentarspanne gefunden", len(sp), 1)

    # --- lange Funktionen: exakt 200 haelt, 201 reisst -----------------------
    rumpf = "    let x = 1;\n" * 198
    genau200 = "fn f() {\n" + rumpf + "}\n"
    t, _ = finde_lange_funktionen("t.rs", genau200, True, 200)
    pruefe("Rust-Funktion mit exakt 200 Zeilen haelt", len(t), 0)
    ueber201 = "fn f() {\n" + rumpf + "    let y = 2;\n" + "}\n"
    t, _ = finde_lange_funktionen("t.rs", ueber201, True, 200)
    pruefe("Rust-Funktion mit 201 Zeilen reisst", len(t), 1)
    pruefe("gemeldete Laenge stimmt", t[0][2] if t else 0, 201)
    pruefe("gemeldeter Name stimmt", t[0][3] if t else "", "f")

    cpp200 = "void f() {\n" + ("    int x = 1;\n" * 198) + "}\n"
    t, _ = finde_lange_funktionen("t.cpp", cpp200, False, 200)
    pruefe("C++-Funktion mit exakt 200 Zeilen haelt", len(t), 0)
    cpp201 = "void f() {\n" + ("    int x = 1;\n" * 199) + "}\n"
    t, _ = finde_lange_funktionen("t.cpp", cpp201, False, 200)
    pruefe("C++-Funktion mit 201 Zeilen reisst", len(t), 1)

    ifblock = "void f() {\n  if (a) {\n" + ("    int x = 1;\n" * 260) + "  }\n}\n"
    t, _ = finde_lange_funktionen("t.cpp", ifblock, False, 200)
    pruefe("if-Block ist keine eigene Funktion", len(t), 1)
    lam = "void f() {\n  auto g = [](int x) {\n" + ("    int y = 1;\n" * 260) + "  };\n}\n"
    t, l = finde_lange_funktionen("t.cpp", lam, False, 200)
    pruefe("Lambda zaehlt nicht als Funktion", len(t), 1)
    pruefe("Lambda wird gezaehlt und gemeldet", l, 1)
    ctor = "X::X(int a) : f_(a) {\n" + ("  int y = 1;\n" * 260) + "}\n"
    t, _ = finde_lange_funktionen("t.cpp", ctor, False, 200)
    pruefe("Initialisiererliste ist eine Funktion", len(t), 1)
    trait = "trait T {\n    fn f(&self) -> u32;\n}\n"
    t, _ = finde_lange_funktionen("t.rs", trait, True, 0)
    pruefe("Trait-Methode ohne Rumpf ist keine Funktion", len(t), 0)
    cls = "class X {\n" + ("  int a;\n" * 260) + "};\n"
    t, _ = finde_lange_funktionen("t.cpp", cls, False, 200)
    pruefe("Klassenrumpf ist keine Funktion", len(t), 0)

    # --- dead_code ----------------------------------------------------------
    q = [("a.rs", None), ("b.rs", None)]
    mit = {"a.rs": "#[allow(dead_code)]\nfn helfer() {}\n", "b.rs": "fn x() { helfer(); }\n"}
    pruefe("dead_code mit Aufrufer ist kein Befund", len(finde_dead_code(q, mit)), 0)
    ohne = {"a.rs": "#[allow(dead_code)]\nfn helfer() {}\n", "b.rs": "fn x() { }\n"}
    d = finde_dead_code(q, ohne)
    pruefe("dead_code ohne Aufrufer ist ein Befund", len(d), 1)
    pruefe("dead_code meldet den Namen", d[0][2] if d else "", "helfer")
    andere = {"a.rs": "#[allow(unused)]\nfn helfer() {}\n", "b.rs": "fn x() { }\n"}
    pruefe("anderes allow ist kein Befund", len(finde_dead_code(q, andere)), 0)
    inline = {"a.rs": "struct P(#[allow(dead_code)] Platz);\n", "b.rs": "fn x(p: Platz) {}\n"}
    pruefe("inline-Attribut mit verwendetem Typ ist kein Befund",
           len(finde_dead_code(q, inline)), 0)

    # Namenswahl: der Name ist das erste Wort hinter dem ELEMENTschluesselwort,
    # nicht das naechste Wort und nicht das uebernaechste. Sonst erfasst
    # `pub const fn helfer` das Wort `fn` (Erstpruefung 09.09.2026, Defekt 1)
    # oder `struct union where` das Wort `where` (Wiederpruefung 1, 09.09.2026);
    # beide Male gilt jede andere Definition im Crate als Verwendung und kein
    # Helfer wird je gemeldet. Jeder Fall einmal ohne und einmal mit Verwendung
    # - die Erwartung mit ihrem Gegenteil.
    for quelle, erwartet, gebrauch in (
            ("pub const fn helfer() {}", "helfer", "helfer();"),
            ("pub(crate) unsafe fn helfer() {}", "helfer", "unsafe { helfer(); }"),
            ("pub async fn helfer() {}", "helfer", "helfer();"),
            ("unsafe fn helfer() {}", "helfer", "unsafe { helfer(); }"),
            ('pub const extern "C" fn helfer() {}', "helfer", "helfer();"),
            ("pub const HELFER: u8 = 1;", "HELFER", "let a = HELFER;"),
            ("static mut ZAEHLER: u8 = 0;", "ZAEHLER", "unsafe { ZAEHLER = 1; }"),
            ("pub unsafe trait Helfer {}", "Helfer", "fn y<T: Helfer>() {}"),
            # Ein Wort, das zugleich Schluesselwort und gueltiger Bezeichner
            # ist, an Namensposition: uebersprungen werden duerfen nur echte
            # Qualifizierer VOR dem Schluesselwort.
            ("struct union where u8: Copy;", "union", "let a: union;"),
            ("union Foo { a: u8 }", "Foo", "let f: Foo;"),
            ("fn default() -> u8 { 1 }", "default", "let a = default();"),
            ("default fn helfer() {}", "helfer", "helfer();"),
            # `const` ist Qualifizierer, solange ein Schluesselwort folgt,
            # sonst selbst das Schluesselwort.
            ("const fn name() {}", "name", "name();"),
            ("const NAME: u8 = 1;", "NAME", "let a = NAME;"),
    ):
        kopf = "#[allow(dead_code)]\n" + quelle + "\n"
        d = finde_dead_code(q, {"a.rs": kopf, "b.rs": "fn x() { }\n"})
        pruefe(f"dead_code `{quelle}` ohne Verwendung ist ein Befund", len(d), 1)
        pruefe(f"dead_code `{quelle}` meldet den Namen", d[0][2] if d else "", erwartet)
        pruefe(f"dead_code `{quelle}` mit Verwendung ist kein Befund",
               len(finde_dead_code(q, {"a.rs": kopf,
                                       "b.rs": "fn x() { " + gebrauch + " }\n"})), 0)

    # Reproduktion der Wiederpruefung 1 woertlich: Wird beim gueltigen Typnamen
    # `union` stattdessen `where` erfasst, zaehlt die FREMDE `where`-Klausel in
    # b.rs als Verwendung und der Befund verschwindet ganz - der Gesamtlauf
    # kippte damit von Exit 4 auf 0, ohne dass eine Zeile daneben auffiele.
    fremd = {"a.rs": "#[allow(dead_code)]\nstruct union where u8: Copy;\n",
             "b.rs": "fn x<T>() where T: Copy {}\n"}
    d = finde_dead_code(q, fremd)
    pruefe("dead_code `struct union where` bleibt trotz fremder where-Klausel "
           "ein Befund", len(d), 1)
    pruefe("dead_code `struct union where` meldet den Namen neben der fremden "
           "where-Klausel", d[0][2] if d else "", "union")

    # --- Clippy: abgebrochener Prozess ist ein Werkzeugfehler ---------------
    # Ohne jede Cargo-Ausfuehrung: ersetzt wird nur der Prozessaufruf. Ein
    # Exitcode != 0 ist ein Werkzeugfehler, EGAL ob schon Diagnosen gelesen
    # wurden - sonst erscheint die Teilmessung eines abgebrochenen Laufs als
    # gueltige Warnungszahl (Erstpruefung 09.09.2026, Defekt 2).
    import tempfile

    def clippy_probe(wurzel, rc, diagnosen, fehlerstrom=""):
        """miss_clippy mit ersetztem Prozess. Liefert Ergebnis ODER Fehlertext."""
        antwort = subprocess.CompletedProcess(
            [], rc, "\n".join(json.dumps(d) for d in diagnosen), fehlerstrom)
        echt_run, echt_which = subprocess.run, shutil.which
        subprocess.run = lambda *a, **k: antwort
        shutil.which = lambda _: "cargo"
        try:
            return miss_clippy(wurzel)
        except RuntimeError as e:
            return f"WERKZEUGFEHLER: {e}"
        finally:
            subprocess.run, shutil.which = echt_run, echt_which

    def diagnose(stufe, code):
        return dict(reason="compiler-message",
                    message=dict(level=stufe, code=dict(code=code),
                                 message="x", spans=[]))

    def clippy_zahl(x):
        return x[0] if isinstance(x, tuple) else x

    with tempfile.TemporaryDirectory() as tmp:
        w = pathlib.Path(tmp)
        (w / "broker").mkdir(parents=True)
        (w / "broker" / "Cargo.toml").write_text("[package]\n", encoding="utf-8")

        mit_diag = clippy_probe(w, 101, [diagnose("error", "E0308")])
        pruefe("clippy: Abbruch MIT Diagnosen ist ein Werkzeugfehler",
               str(mit_diag).startswith("WERKZEUGFEHLER: cargo clippy Exit 101"), True)
        pruefe("clippy: Abbruch MIT Diagnosen liefert keine Zahl",
               isinstance(mit_diag, tuple), False)
        ohne_diag = clippy_probe(w, 101, [], "error: could not compile eqcop-broker")
        pruefe("clippy: Abbruch OHNE Diagnosen ist ein Werkzeugfehler",
               ohne_diag,
               "WERKZEUGFEHLER: cargo clippy Exit 101 - "
               "error: could not compile eqcop-broker")
        pruefe("clippy: Abbruch ohne stderr sagt genau das",
               clippy_probe(w, 101, []),
               "WERKZEUGFEHLER: cargo clippy Exit 101 - ohne Ausgabe auf stderr")
        drei = clippy_probe(w, 0, [diagnose("warning", f"clippy::l{i}") for i in range(3)])
        pruefe("clippy: sauberer Lauf mit 3 Warnungen zaehlt 3", clippy_zahl(drei), 3)
        pruefe("clippy: sauberer Lauf ohne Diagnosen zaehlt 0",
               clippy_zahl(clippy_probe(w, 0, [])), 0)

        # Der Gesamtlauf erbt den Fehler: miss() reicht ihn durch, main() macht
        # daraus „Werkzeugfehler" und Exit 2 - derselbe Weg wie beim fehlenden
        # Messort, nie 0 und nie 4.
        for ort in QUELLORTE:
            (w / ort).mkdir(parents=True)
        (w / QUELLORTE[0] / "a.rs").write_text("fn a() {}\n", encoding="utf-8")
        kein_memory = w / "kein-memory"
        antwort = subprocess.CompletedProcess([], 101, "", "error: linker failed")
        echt_run, echt_which = subprocess.run, shutil.which
        subprocess.run = lambda *a, **k: antwort
        shutil.which = lambda _: "cargo"
        try:
            miss(w, kein_memory, True)
            geerbt = "(nicht geworfen)"
        except RuntimeError as e:
            geerbt = str(e)
        finally:
            subprocess.run, shutil.which = echt_run, echt_which
        pruefe("clippy: Gesamtlauf erbt den Werkzeugfehler (Weg zu Exit 2)",
               geerbt, "cargo clippy Exit 101 - error: linker failed")
        pruefe("clippy: ohne Schalter bleibt derselbe Baum unberuehrt",
               [m.get("nicht_messbar") for m in miss(w, kein_memory, False)
                if m["name"].startswith("Clippy")],
               ["nicht angefordert (Schalter --clippy)"])

    with tempfile.TemporaryDirectory() as tmp:
        w = pathlib.Path(tmp)
        pruefe("clippy: fehlender Messort ist ein Werkzeugfehler",
               str(clippy_probe(w, 0, [])).startswith(
                   "WERKZEUGFEHLER: Messort fuer clippy fehlt"), True)
        # Gegenteil: fehlendes cargo ist Umgebung, kein Werkzeugfehler.
        (w / "broker").mkdir(parents=True)
        (w / "broker" / "Cargo.toml").write_text("[package]\n", encoding="utf-8")
        echt_which = shutil.which
        shutil.which = lambda _: None
        try:
            umgebung = miss_clippy(w)
        finally:
            shutil.which = echt_which
        pruefe("clippy: fehlendes cargo bleibt NICHT MESSBAR",
               umgebung, (None, [], "cargo nicht gefunden"))

    # --- Kommentar-Bezeichner ----------------------------------------------
    q2 = [("a.rs", None)]
    t2 = {"a.rs": "// siehe `Vorhanden`\nfn Vorhanden() {}\n"}
    pruefe("existenter Bezeichner ist kein Befund",
           len(finde_kommentar_bezeichner(q2, t2, {"Vorhanden"})[0]), 0)
    t3 = {"a.rs": "// siehe `Verschwunden`\nfn x() {}\n"}
    k, h = finde_kommentar_bezeichner(q2, t3, {"x"})
    pruefe("fehlender Bezeichner ist ein Befund", len(k), 1)
    pruefe("Befund nennt den Namen", k[0][2] if k else "", "Verschwunden")
    pruefe("ohne Historienmarker wird nichts ausgenommen", h, 0)
    t4 = {"a.rs": "// `abc`\nfn x() {}\n"}
    pruefe("zu kurzer Kandidat wird uebergangen",
           len(finde_kommentar_bezeichner(q2, t4, {"x"})[0]), 0)
    t5 = {"a.rs": "// `nicht`\nfn x() {}\n"}
    pruefe("Stoppwort wird uebergangen",
           len(finde_kommentar_bezeichner(q2, t5, {"x"})[0]), 0)
    t6 = {"a.rs": 'let s = "`Verschwunden`";\nfn x() {}\n'}
    pruefe("Backtick im String ist kein Kandidat",
           len(finde_kommentar_bezeichner(q2, t6, {"x"})[0]), 0)

    # Umfeldausnahme: derselbe fehlende Name, einmal als Gegenwart und einmal
    # ausdruecklich als Historie behauptet.
    t7 = {"a.rs": "// `Verschwunden` ist ENTFALLEN, Runde 2 zog ihn zurueck\nfn x() {}\n"}
    k7, h7 = finde_kommentar_bezeichner(q2, t7, {"x"})
    pruefe("markierte Historie ist kein Befund", len(k7), 0)
    pruefe("markierte Historie wird gezaehlt", h7, 1)
    t8 = {"a.rs": "// ruft `Verschwunden` bei jedem Tick auf\nfn x() {}\n"}
    pruefe("dieselbe Luecke als Gegenwart ist ein Befund",
           len(finde_kommentar_bezeichner(q2, t8, {"x"})[0]), 1)

    # --- Index-Zeilen: 250 haelt, 251 reisst --------------------------------
    pruefe("Indexzeile mit 250 Zeichen haelt", len("x" * 250) > INDEXZEILE_GRENZE, False)
    pruefe("Indexzeile mit 251 Zeichen reisst", len("x" * 251) > INDEXZEILE_GRENZE, True)

    # --- Fehlender Messort ist ein Werkzeugfehler, kein leeres Ergebnis -----
    # Die Wache laeuft gegen einen temporaeren Baum: mit allen drei Orten
    # liefert sie eine Dateiliste, ohne einen davon wirft sie.
    import tempfile
    with tempfile.TemporaryDirectory() as tmp:
        w = pathlib.Path(tmp)
        for ort in QUELLORTE:
            (w / ort).mkdir(parents=True)
        (w / QUELLORTE[0] / "a.rs").write_text("fn a() {}\n", encoding="utf-8")
        pruefe("vollstaendige Messorte liefern Dateien", len(sammle_quellen(w)), 1)
        # Einen Ort umbenennen - genau der Fall „Pfad verschoben".
        (w / QUELLORTE[2]).rename(w / QUELLORTE[2].replace("core", "kern"))
        geworfen = False
        try:
            sammle_quellen(w)
        except RuntimeError as e:
            geworfen = "Messort fehlt" in str(e)
        pruefe("fehlender Messort wirft statt still 0 zu messen", geworfen, True)

    # --- Eingespeister Wert wird im Kopf angesagt ---------------------------
    # Die Marke an der Zeile allein genuegt nicht: die Urteilszeile spraeche
    # sonst von „allen Grenzen" und meinte eine Messung, die es nicht gab.
    import io
    import contextlib
    probe = [dict(name="Probe", ist=1, ziel=0, grenze=9, status="ZIEL",
                  treffer=[], eingespeist=True)]
    puffer = io.StringIO()
    with contextlib.redirect_stdout(puffer):
        drucke_bericht(probe, pathlib.Path("."), pathlib.Path("."), False)
    pruefe("eingespeister Wert wird im Kopf angesagt",
           "KEINE vollstaendige Messung" in puffer.getvalue(), True)
    probe[0]["eingespeist"] = False
    puffer = io.StringIO()
    with contextlib.redirect_stdout(puffer):
        drucke_bericht(probe, pathlib.Path("."), pathlib.Path("."), False)
    pruefe("ohne Einspeisung keine Warnzeile",
           "KEINE vollstaendige Messung" in puffer.getvalue(), False)

    rot = [(n, t) for n, ok, t in faelle if not ok]
    for n, ok, t in faelle:
        print(("  ok   " if ok else "  ROT  ") + n + ("" if ok else "   " + t))
    print("")
    print(f"Selbsttest: {len(faelle) - len(rot)}/{len(faelle)} bestanden")
    return 0 if not rot else 2


# ------------------------------------------------------------------- Hauptlauf


def main() -> int:
    p = argparse.ArgumentParser(add_help=True, description=(
        "Misst Codebase- und Kontext-Gesundheit gegen die Schwellen aus Plan "
        "S25b und dem Kontext-Hygiene-Playbook."))
    p.add_argument("--clippy", action="store_true",
                   help="cargo clippy mitmessen (baut; ohne Schalter nicht messbar)")
    p.add_argument("--json", action="store_true", help="Ergebnis als JSON")
    p.add_argument("--selbsttest", action="store_true",
                   help="nur die eingebauten Faelle fahren")
    p.add_argument("--mit-selbsttest", action="store_true", dest="mit_selbsttest",
                   help="Selbsttest voranstellen, dann messen (Form des Kanon-Beins)")
    p.add_argument("--wurzel", help="Pruefschalter: anderen Baum messen")
    p.add_argument("--memory", help="Pruefschalter: anderen Memory-Ordner messen")
    p.add_argument("--einspeisen", action="append", default=[], metavar="MAASS=ZAHL",
                   help="Pruefschalter: Istwert setzen (Teilname des Maasses)")
    a = p.parse_args()

    if a.selbsttest:
        return selbsttest()

    if a.mit_selbsttest:
        # Ein Mass, dessen Werkzeug nicht mehr misst, ist gefaehrlicher als gar
        # keines: es meldete jede Verschlechterung als „alles in Ordnung". Der
        # Selbsttest laeuft deshalb VOR jeder Messung, und sein Fehlschlag
        # beendet den Lauf mit 2 (Werkzeugfehler) statt mit 4 (Befund).
        print("SELBSTTEST")
        if selbsttest() != 0:
            print("")
            print("ABBRUCH: Selbsttest rot - es wird nicht gemessen.", file=sys.stderr)
            return 2
        print("")

    wurzel = pathlib.Path(a.wurzel).resolve() if a.wurzel else WURZEL
    memory = pathlib.Path(a.memory).resolve() if a.memory else memory_ordner(wurzel)

    if a.clippy and shutil.which("cargo") is None:
        print("Voraussetzung fehlt: --clippy angefordert, cargo nicht gefunden",
              file=sys.stderr)
        return 3

    try:
        maasse = miss(wurzel, memory, a.clippy)
    except RuntimeError as e:
        print(f"Werkzeugfehler: {e}", file=sys.stderr)
        return 2

    for setzung in a.einspeisen:
        if "=" not in setzung:
            print(f"Werkzeugfehler: --einspeisen braucht MAASS=ZAHL, nicht {setzung!r}",
                  file=sys.stderr)
            return 2
        teil, wert = setzung.split("=", 1)
        getroffen = [m for m in maasse if teil.lower() in m["name"].lower()]
        if len(getroffen) != 1:
            print(f"Werkzeugfehler: {teil!r} trifft {len(getroffen)} Maass(e)",
                  file=sys.stderr)
            return 2
        getroffen[0]["ist"] = int(wert)
        getroffen[0]["eingespeist"] = True
        getroffen[0]["treffer"] = ["(eingespeist - keine Messung)"]

    for m in maasse:
        m["status"] = bewerte(m["ist"], m.get("ziel"), m.get("grenze"))

    if a.json:
        print(json.dumps({
            "wurzel": str(wurzel),
            "memory": str(memory),
            "clippy_gemessen": bool(a.clippy),
            "maasse": maasse,
        }, indent=2, ensure_ascii=True))
    else:
        drucke_bericht(maasse, wurzel, memory, bool(a.wurzel))

    return 4 if any(m["status"] == "GRENZE" for m in maasse) else 0


if __name__ == "__main__":
    sys.exit(main())
