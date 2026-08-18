#!/bin/bash
# SessionStart hook: Nakama-Systemkarte + Live-Stand injizieren.
# Der statische Teil ist ein BEWUSSTES Duplikat der CLAUDE.md-Invarianten —
# Fade-Resilienz: bei langen Sessions und nach Compaction verblasst der
# Anfangskontext; dieser Hook feuert auch bei source=compact. Nie zu einem
# Pointer zusammenkürzen.
# Der dynamische Teil liest den Live-Stand aus dem eigenen Git (Code ist
# Wahrheit — statische Doku würde hier veralten).

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"

cat <<'KARTE'
=== Nakama-Systemkarte (Langform: CLAUDE.md + docs/plugin-wissen.md + docs/design-stand.md) ===

WAS: Transparentes Berater-VST3 für FL Studio (Win 11, JUCE 8/C++20, CMake)
     + EIGENSTÄNDIGER Rust-Broker (Named Pipe, broker/ — seit 18.08. aus
     der Hub-App herausgelöst; eqcop-broker.exe). Produktname NAKAMA;
     Code/Bundle/Pipes/Schemas tragen aus Kompatibilität den Legacy-Namen
     EQ-Copilot (EqCop*) — das ist KEIN Umbenennungs-Task.

GRUNDGESETZ (nicht verhandelbar):
  Nakama BERÄT NUR. Setzt keine EQ-Parameter, schreibt keine Automation,
  verändert kein Audio (Passthrough sampleidentisch, 0 Latenz, kein Tail).
  Audiothread: keine Sperren, Allokationen, Datei-/Pipe-/Netz-Zugriffe,
  kein Logging. Überlast verwirft Analyseframes, nie Audio.
  Einzige dokumentierte Ausnahme (0.3.0): die Hör-Markierung färbt auf
  bewussten Klick das MONITORSIGNAL (streng verriegelt: Echtzeit bewiesen ∧
  Editor offen ∧ Transport ∧ !isNonRealtime) — Render bleibt bitidentisch.

ALLES LEBT HIER (eigenständiger Workspace seit 18.08.2026 — Code, Wissen,
Design in EINEM Repo; Historie davor: FL-Studio-Repo bis 7964777):
  eq-copilot/plugin/src/      AnalyseEngine · PluginProcessor · PluginEditor ·
                              PipeClient · Diagnose · HoerMarkierung · AssetKit
  eq-copilot/schemas/         eq-ipc v2 · eq-measurement · eq-report ·
                              eq-snapshot v3 · eq-aggregat — versionierte
                              Verträge; neue Felder ⇒ ERST Versionierung
  eq-copilot/design/          Musterblätter + tokens.json + Renders + Fonts
  eq-copilot/docs/            Baupläne, M0–M3a-Befunde, FL-TESTANLEITUNG
  broker/                     EIGENSTÄNDIGER Broker (Pipe-Server, Paare,
                              Profilbindung, Aggregat; eqcop-broker.exe,
                              Hub-App kennt ihn nicht mehr)
  FL-EQ-Copilot-Recherche.md  kanonischer Produkt-/Umsetzungsplan (Wurzel)

BEWEIS-KANON (nie "fertig" ohne die betroffenen Beweise, alle headless):
  EqCopGoldenTest (Fixtures, zuletzt 239/239) · EqCopNullTest (bitgleicher
  Passthrough) · EqCopMarkierungTest (30/30, Hör-Markierung inkl.
  Render-bitidentisch) · pluginval --strictness-level 8 · EqCopShot
  (Offscreen-PNG mit echter 20-s-Messung) · EqCopPaintBench (paint()-
  Kosten) · EqCopPipeProbe (NUR gegen eigenen Probe-Pipenamen ...m2probe —
  nie gegen den Produktions-Broker) · cargo test (in broker/, 36 Tests).
  Befund-Standard: "ausgeführt und gesehen", nie "sollte funktionieren".

DESIGN-ARBEITSMODELL (Stand 18.08. — PRISMA-Richtung):
  User-autorisiertes PRISMA (3 Flächen = Tiefen/Mitten/Höhen, Drehen =
  Selektion, Klick = Bündeln/Newton), Film-Compositing (Blender-5.1-Cycles
  auf CPU! + Ebenen-Sandwich mit live Daten-Ebene). ABGENOMMEN: das
  Prisma-OBJEKT („sieht top aus", klar > rauchig, Frost raus) + der
  drehbare Prototyp (eq-copilot/design/prisma/). VERWORFEN 18.08.: rohe
  Energie-Punktwolke als Dauerinhalt („Wasserfall aus Pixeln" — Profi mit
  20 Spuren liest daran NICHTS ab). OFFEN: Befund-Verkörperung (1 Befund =
  1 präzises Objekt im Glas; WAS es ist, kommt vom User + Referenzen).
  Einstieg: docs/NEXT-SESSION.md + docs/design-stand.md. GEPARKT, nie
  ungefragt reaktivieren: Tiefenfeld, Bauplan 2.0, Kunstwerk-Studie.
  eq-copilot/design/nakama-spectral-field-vorentwurf.html = CODEX-BESITZ.

DESIGN-VERFASSUNG (Geschmacksprofil — 4 Proben, teuer bezahlt 17.08.):
  1 SCHÄRFE: Detail aus Auflösung in echte Punkte/Struktur; Glow/Bloom/
    Weichzeichner = "billig" (Unity-Baukasten-Effekte verboten).
  2 HANDSCHRIFT: lieber einfach mit persönlichem Stil + Liebe zum Detail
    als komplex-generisch.
  3 SICHTBARER DENKVORGANG mit Distanz: laut→hell = Tautologie. Verschärft:
    NOTWENDIG statt konstruiert ("kein Grund, dass es da ist" = Todesurteil)
    und ENTDECKT statt entworfen (Phänomen/Emergenz schlägt Designer-Objekt).
  META: Claudes freie Bilderfindung = Mode-Collapse ("wie algorithmisch
    komponierte Musik — objektiv ok, aber egal"). Vision + Referenzen
    kommen vom USER; Claude übersetzt treu, verdatet ehrlich, verfeinert
    unter seinem Auge. Einzige abgenommene Ästhetik: P01-Perkolation
    (scharfe Punktwolke, Bandfarben Amber/Magenta/Eis).
  Volltext + wörtliche Urteile: docs/geschmacksprofil.md — vor JEDER
  Design-Entscheidung lesen. KREATIV-SCHLEUSE: erst ausarbeiten, Marker
  (.claude/kreativ-freigabe.md, 24 h), dann bauen — Hook erzwingt es.

ANTI-CONFLATION (jede Zeile war ein echter teurer Irrweg):
  - Pixel-Beweis ≠ Schönheits-Beweis (E0–E5 war 18/18 bewiesen und hässlich;
    Optik-Abnahme macht der USER am lebenden Blatt)
  - VERWORFEN, nie neu vorschlagen: 3D-Papier-Shader-Modell (E0–E5) ·
    Tusche-Einzelmarken-Piktogramme (Borsten/Siegel = Deko). Tragfähig ist,
    was der Graph SELBST mit Licht/Tiefe erzählt (Position/Breite/Stärke).
  - Musterblatt-Beispieldaten ≠ Plugin-Daten (HTML-Demo-Werte nie übernehmen)
  - ltasReferenzDb ≠ Sollkurve (reine 8192er-Messachse; es gibt KEINEN
    globalen Zielkorridor — Befunde messen gegen ihre eigene Schulterlinie)
  - paint()-FPS ≠ Datenkadenz (M3a: 4-Hz-Snapshots sahen aus wie Render-Lag;
    paint() war mit 2,4 ms unschuldig — erst messen, dann optimieren)
  - Editor-Anzeigezustand ≠ Analysezustand (Editor hält NUR Anzeige;
    Materialbewegung lebt auf der Snapshot-Kopie, nie in der Engine)
  - Legacy-Name ≠ Rename-Task (EqCop*/Schema-/Pipe-Namen bleiben)
  - UAC-Install = User-Klick (Install-EQ-Copilot.ps1 als Admin; Rollback
    liegt bereit; nie automatisch installieren)
KARTE

echo ""
echo "=== Live-Stand (git in diesem Workspace — Code ist Wahrheit) ==="
git -C "$NAK" log --oneline -6 2>/dev/null \
  || echo "(Nakama-Repo nicht lesbar — Pfad prüfen: $NAK)"
DIRTY=$(git -C "$NAK" status --porcelain 2>/dev/null)
if [ -n "$DIRTY" ]; then
  echo ""
  echo "-- Uncommitted (evtl. parallele Codex-Session — eigene Edits sofort committen, nie --amend):"
  echo "$DIRTY" | head -10 | sed 's/^/  /'
  N=$(echo "$DIRTY" | wc -l | tr -d ' ')
  [ "$N" -gt 10 ] && echo "  … ($N gesamt)"
fi
