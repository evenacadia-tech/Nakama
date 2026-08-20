# Arbeitsplan — womit konkret angefangen wird

Der Fahrplan dieses Projekts. Eine Phase endet erst mit einer Abnahme in
`abnahmen/`; keine Phase wird übersprungen, keine vorgezogen. Methodik je
Phase: `CLAUDE-UX-UI-ARBEITSKERN.md` (Stufen 1–7); Hintergründe:
`UX-UI-WISSENSBASIS-FUER-CLAUDE.md`.

## Phase 0 — Designvertrag (HIER anfangen)

**Einstieg:** `/interview designvertrag` führen — noch nichts bauen.
Danach die Assettruhe sichten und laut zusammenfassen, was dort liegt
(oder dass sie noch leer ist und was hineingehört).

**Ergebnis:** `abnahmen/JJJJ-MM-TT-designvertrag.md` mit: wichtigster
Nutzerjob, 1–3 Erfolgskriterien, 2–3 Gefühlsattributen (mit
Konsequenzen), Referenzen + Antireferenzen (je mit Grund), technischen
Grenzen (Fenstergrößen, Skalierung), Grad der kreativen Autorenschaft.

**Gate:** Ohne diese Abnahme blockt die Schleuse jeden Prototyp-Write.

## Phase 1 — Wireframes (Graustufen, keine Optik)

**Reihenfolge: Main zuerst** (dort steckt die eigentliche
Informationsarchitektur — der teuerste Fehler wird hier billig
verhindert), dann Active-Editor, dann Passive-Kachel.

**Je Oberfläche:** dominante Leserichtung, Gruppierung, Gewichtung,
Haupt-/Leer-/Lade-/Fehlerzustand — als graues lebendes Blatt mit
Zustands-Schaltern. Vorher `/interview struktur` für die betroffene
Oberfläche.

**Ergebnis:** drei abgenommene Wireframes. Was im Graubild nicht
funktioniert, wird nicht mit Farbe gerettet.

## Phase 2 — Visuelle Richtung (Stilprobe)

**An EINER Oberfläche** (Active-Editor — die reichste vertraute
Konventionsfläche: EQ-Kurve, Regler, Werte) werden 2–3 Richtungs-
Varianten NEBENEINANDER gebaut, jede mit benannter Hypothese („Variante A
löst X besser, weil …"). Vorher `/interview richtung`.

**Ergebnis:** eine abgenommene Richtung (+ was aus den Verlierern
mitgenommen wird).

## Phase 3 — UI-System und Ausrollen

Design-Tokens (Farb-/Typo-/Spacing-Rollen) und Kernkomponenten (Panel,
Regler, Wertfeld, Kurvenfeld, Statuszeile, Befundkarte) aus der
abgenommenen Richtung — dann auf alle drei Blätter anwenden.
Editor-Panel in jedem Blatt: Tokens live verstellbar.

**Ergebnis:** drei gestaltete Blätter, je einzeln abgenommen.

## Phase 4 — Zustände und Bewegung

Volle Zustandsmatrix je Blatt (Arbeitskern Schritt 3) sichtbar +
durchschaltbar; Motion nur wo sie Zustand/Wirkung zeigt, Intensität
proportional. Vorher `/interview zustaende`.

**Ergebnis:** abgenommene Zustands- und Motion-Fassung.

## Phase 5 — Übergabepaket

Eine Spezifikation für die Plugin-Umsetzung: Tokens als Werte,
Komponentenliste mit Zuständen, Maße/Abstände, Assets (exportfertig),
Timing-Angaben, Abnahmekriterien. Der Prototyp bleibt als lebende
Referenz bestehen.

## Ständige Regeln

- Jeder gezeigte Stand: Playwright-Screenshot als Beleg.
- Jede Abnahme sofort nach `abnahmen/`.
- `/interview` jederzeit zusätzlich, wenn eine User-Äußerung eine
  Richtungsänderung andeutet oder eine Vorstellung unklar ist — fragen
  ist billig, Fehlbau nicht.
