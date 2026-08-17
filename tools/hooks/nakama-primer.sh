#!/bin/bash
# SessionStart hook: Nakama-Systemkarte + Live-Stand injizieren.
# Der statische Teil ist ein BEWUSSTES Duplikat der CLAUDE.md-Invarianten —
# Fade-Resilienz: bei langen Sessions und nach Compaction verblasst der
# Anfangskontext; dieser Hook feuert auch bei source=compact. Nie zu einem
# Pointer zusammenkürzen.
# Der dynamische Teil liest den Live-Stand aus dem FL-Studio-Git (Code ist
# Wahrheit — statische Doku würde hier veralten).

FL="$HOME/FL-Studio"

cat <<'KARTE'
=== Nakama-Systemkarte (Langform: CLAUDE.md + docs/plugin-wissen.md + docs/design-stand.md) ===

WAS: Transparentes Berater-VST3 für FL Studio (Win 11, JUCE 8/C++20, CMake)
     + Rust-Broker (Named Pipe) in der Tauri-Hub-App. Produktname NAKAMA;
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

CODE LEBT IM FL-STUDIO-REPO (dieses Workspace = nur Kontext/Wissen/Handoffs):
  eq-copilot/plugin/src/      AnalyseEngine · PluginProcessor · PluginEditor ·
                              PipeClient · Diagnose · HoerMarkierung · AssetKit
  eq-copilot/schemas/         eq-ipc v2 · eq-measurement · eq-report ·
                              eq-snapshot v3 · eq-aggregat — versionierte
                              Verträge; neue Felder ⇒ ERST Versionierung
  eq-copilot/design/          Musterblätter + tokens.json + Renders + Fonts
  eq-copilot/docs/            Baupläne, M0–M3a-Befunde, FL-TESTANLEITUNG
  plugin-hub-app/src-tauri/src/eq_copilot/   Broker (Pipe-Server, Paare,
                              Profilbindung, Aggregat)
  FL-EQ-Copilot-Recherche.md  kanonischer Produkt-/Umsetzungsplan (Wurzel)

BEWEIS-KANON (nie "fertig" ohne die betroffenen Beweise, alle headless):
  EqCopGoldenTest (Fixtures, zuletzt 239/239) · EqCopNullTest (bitgleicher
  Passthrough) · EqCopMarkierungTest (30/30, Hör-Markierung inkl.
  Render-bitidentisch) · pluginval --strictness-level 8 · EqCopShot
  (Offscreen-PNG mit echter 20-s-Messung) · EqCopPaintBench (paint()-
  Kosten) · EqCopPipeProbe (NUR gegen eigenen Probe-Pipenamen ...m2probe —
  nie gegen den Produktions-Broker) · cargo test eq_copilot.
  Befund-Standard: "ausgeführt und gesehen", nie "sollte funktionieren".

DESIGN-ARBEITSMODELL:
  Design-Phase lebt im Browser-Musterblatt (Sekunden-Schleife), NICHT im
  Plugin (Minuten/UAC/FL-Neustart). Erst "Werte kopieren"-JSON vom User
  = Design-Freeze, DANN JUCE-Umbau nach Bauplan 2.0.
  - eq-copilot/design/nakama-spectral-field-claude.html  = Claudes Blatt
  - eq-copilot/design/nakama-spectral-field-vorentwurf.html = CODEX-BESITZ,
    nie editieren
  - Verbindlicher Zielvertrag: eq-copilot/docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md

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
echo "=== Live-Stand (git im FL-Studio-Repo — Code ist Wahrheit) ==="
git -C "$FL" log --oneline -6 -- eq-copilot plugin-hub-app/src-tauri/src/eq_copilot 2>/dev/null \
  || echo "(FL-Studio-Repo nicht lesbar — Pfad prüfen: $FL)"
DIRTY=$(git -C "$FL" status --porcelain -- eq-copilot plugin-hub-app 2>/dev/null)
if [ -n "$DIRTY" ]; then
  echo ""
  echo "-- Uncommitted im FL-Studio-Repo (evtl. fremde Parallel-Session — eigene Edits sofort committen, nie --amend):"
  echo "$DIRTY" | head -10 | sed 's/^/  /'
  N=$(echo "$DIRTY" | wc -l | tr -d ' ')
  [ "$N" -gt 10 ] && echo "  … ($N gesamt)"
fi
