# Design-Stand — an welchem Prototypen wir arbeiten

> **Stand: 2026-08-17.** Lebendes Dokument — nach jedem Design-Commit im
> FL-Studio-Repo hier nachziehen. Bei Widerspruch gewinnt der Code
> (`eq-copilot/design/…`) und der Bauplan dort.

## Die zwei Ebenen

1. **Zielvertrag (verbindlich):** `eq-copilot/docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md`
   — „Bauplan 2.0" für die neue VST3-Hauptansicht. Nakama = bildschirmfüllendes
   Spectral Field: Membran-Kurve, textfreie Symbolkreise mit Dringlichkeitsring,
   überlagerbare Befunde (kein Top-Drei-Limit), manuelles Befundarchiv,
   umschaltbare Farbpakete, 仲間-Bildmarke als Pfadgeometrie, Schriften
   Nakama Phase + Manrope (eingebettet). Kern-Grammatik §6.6: **jede sichtbare
   Schicht hat genau eine Messbedeutung**; Nebel und Licht sind zwei Enden
   EINER Evidenzachse (`nebel_anteil + licht_anteil = 1`). Es gibt keinen
   globalen Zielkorridor — `ltasReferenzDb` ist Messachse, keine Sollkurve.
   Der Umbau der produktiven JUCE-Ansicht ist **noch nicht erfolgt**.

2. **Live-Design-Fläche (hier passiert die Arbeit):**
   `eq-copilot/design/nakama-spectral-field-claude.html` — das
   **Claude-Musterblatt** im Browser. Design-Schleife = Sekunden (Regler
   ziehen), Plugin-Schleife = Minuten (Kompilieren + UAC + FL-Neustart);
   deshalb bleibt die Design-Phase im Musterblatt, bis der Design-Freeze
   fällt (Transfer-Vertrag im 3D-Effekte-Plan §5.1).
   `nakama-spectral-field-vorentwurf.html` ist **Codex-Besitz** — nie
   editieren (PreToolUse-Guard blockt das).

## Aktueller Mechanismus: das TIEFENFELD

Nach zwei Verwürfen gilt: **EIN Mechanismus** — die Abweichung vom
Bleistift-Plan wird Relief; der Graph selbst erzählt die Befunde.

- **Nur noch dunkle Welten** (User-Entscheid 17.08.: „Weiß fliegt raus"):
  `tusche-dunkel` = Standard, `nebel-nacht` als zweite; Hell-Paletten gelöscht.
- Auf dunklem Blatt erzählt **Licht statt Schatten**: `tiefenLicht`
  (Lichtsäule unter dem Plan, so tief wie der Befund, 1:1 px) · `talLicht`
  (Lichtkante unter der Plan-Lippe) · `strichSchein` (Lichthof statt
  Schlagschatten).
- Vokabular: **Strich** = Ist (hungert im Tal) · **Bleistift** = Soll ·
  **Fahne** = Abweichung · **Befund-Nebel = WO** (Fog über dem Band,
  Dicke = Schwere, wogt nur live) · **Pool** = Zeitgedächtnis.
- **Chips = Befund-DATEN** (Resonanz · Mulm · Loch · Härte; Tasten 1–4,
  `?aus=resonanz,…`) — Kurve, Fahne, Nebel und Licht antworten gemeinsam.
- Kalibrier-Panel: 10 Regler (u. a. `nebel`, `feld`, `schatten`) +
  **Farbwahl je Graph-Element** (10 Picker, pro Welt gespeichert; das
  Kopier-JSON trägt `farben` mit). `?kali=auf` · `?zeit=` friert ein.
- Renders: `node eq-copilot/design/render-blatt.mjs` → deterministischer
  `vorentwurf-renders/n4-*.png`-Satz (semantische Zustände + die drei
  vertraglichen Referenzgrößen); `… pruefe` fährt sechs Semantik-/
  Gegenpfadchecks (Nebel↔Licht, Leerzustand, Resize-Klassen, Stop↔Start).
  Ältere `t3-*`-Serien bleiben als Verlaufsbelege liegen.

## Nächster Schritt (liegt beim User)

Musterblatt im Browser öffnen — der Nebel atmet nur live, Renders sind
Standbilder. Wenn es sitzt: **„Werte kopieren"-JSON in den Chat = Design-
Freeze** → wird 1:1 JUCE-Voreinstellung → dann Umbau der produktiven
Ansicht nach Bauplan §13 (Reihenfolge verbindlich, 12 Schritte, beginnend
mit freiem Resize ohne 750:520-Ratio).

## Verworfene Richtungen — nie neu vorschlagen (teuer bezahlt)

1. **3D-Papier-Shader-Modell (E0–E5, 16.08.):** three.js-Bühne, Brand/Riss/
   Feuchte-Felder, selektives Bloom. War vollständig gebaut und formal
   bewiesen (Null-Beweis 0,16 %, Regler-Beweis 18/18) — User: „sieht sehr
   schlecht aus". Lehre: **Pixel-Beweise messen Änderung, nie Schönheit.**
   Prozedurale Material-Simulation ist an dieser Aufgabe **zweimal**
   gescheitert. Verlaufsbeleg: `eq-copilot/docs/NAKAMA-PAPIER-3D-EFFEKTE-PLAN.md`
   (Status ⛔ VERWORFEN).
2. **Tusche-Einzelmarken (17.08.):** Bruch-Borsten („zerissenes Stroh"),
   Siegel („Kochhut") — Einzelmarken-Piktogramme lesen sich als Deko.
   Tragfähig ist, was der Graph selbst mit Licht/Tiefe erzählt und dabei
   Position/Breite/Stärke abbildet (FabFilter-Referenz: Tiefe als Mittel).

## Produktions-UI heute (zum Vergleich)

Das installierte Plugin (0.3.0, M3a) trägt noch die **Material-Kit-Front**:
warme Metallfläche + schwarzes Analyseglas + Ember-Akzent, eigenständige
Vektorfront aus `plugin/src/EqCopilotAssetKit.h` (Vertrag:
`design/ASSET-KIT.md`), Fensterformat 750:520. Diese Front wird durch das
Spectral Field **ersetzt**, sobald der Design-Freeze steht.
