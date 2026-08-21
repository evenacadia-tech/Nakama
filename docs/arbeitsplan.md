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

> **⚠ Stand 2026-08-21: diese Abnahme fehlt.** Es liegt keine
> `abnahmen/*designvertrag*.md` vor; es gab nie ein `/interview designvertrag`.
> Phase 0.5 und Phase 1 (Main) wurden **ohne sie** gearbeitet — zulässig, weil
> beides in `werkzeug/` stattfand und nicht in `prototyp/`. Die Schleuse ist
> damit weiterhin **zu** (geprüft: exit 2). Wer die erste Prototyp-Datei
> anlegen will, holt Phase 0 vorher nach. Wer nur weiter Wireframes baut,
> arbeitet in `werkzeug/` und ist nicht betroffen.
>
> Was dabei bis heute fehlt und Phase 2 beurteilbar machen würde:
> Erfolgskriterien, Gefühlsattribute, Antireferenzen, Grad der kreativen
> Autorenschaft.

## Phase 0.5 — Formfaktor und Größe (VOR jedem Wireframe)

Eine Anordnung ohne bekannte Fläche ist wertlos. Für alle drei Oberflächen
zuerst festlegen: **Seitenverhältnis, Größe, und ob fest / in Stufen / frei
ziehbar.** Beurteilt wird ausschließlich bei **1:1** auf dem Zielbildschirm,
über einem nachgestellten Host — ein skaliertes Bild taugt dafür nicht.

Werkzeug: `werkzeug/formfaktor.html`. Es misst, ab welcher Größe Inhalt
abgeschnitten wird, und zeigt den Preis in Prozent des Bildschirms.

**Ergebnis:** drei abgenommene Formate. Tote Fläche gilt als Fehler.
Hintergrund: `abnahmen/2026-08-20-korrektur-formfaktor-zuerst.md`.

## Phase 1 — Wireframes (Graustufen, keine Optik)

**Reihenfolge: Main zuerst** (dort steckt die eigentliche
Informationsarchitektur — der teuerste Fehler wird hier billig
verhindert), dann Active-Editor, dann Passive-Kachel.

**Je Oberfläche:** zuerst die **Aufgabenliste nach Häufigkeit**
(`docs/DESIGN-GESETZE.md`, verbindliche Folge 1 — ohne sie kein Wireframe),
dann dominante Leserichtung, Gruppierung, Gewichtung, Haupt-/Leer-/Lade-/
Fehlerzustand — als graues lebendes Blatt mit Zustands-Schaltern. Varianten
unterscheiden sich in der **Disclosure-Mechanik**, nicht in der
Kastenanordnung. Vorher `/interview struktur` für die betroffene Oberfläche.

**Stand:** Main ist auf Wireframe-Ebene **vollständig abgenommen**
(2026-08-20): Größe 760×430 · Disclosure-Mechanik Overview + Detail · Karte
trägt jede Quelle · Aufteilung teilt den Überschuss · Vorhören markiert die
betroffene Zeile · Zustände nach dem Ausnahme-Prinzip. Der Pflichtinhalt der
Arbeitsfläche ist gelöst, aber **handwerklich** — dazu gibt es keine Abnahme
und es wird keine geben (`abnahmen/2026-08-20-karte-alle-quellen.md`).
Aktueller Entwurf: `werkzeug/zustaende.html`.

Als Nächstes: **Active-Probe-Editor** (700×420), dann **Passive-Kachel**
(260×84). Auftrag, Aufgabenliste-Entwurf, vorgerechnete Klemme und die
Stolperfallen stehen in **`docs/sondenplan.md`** — vor der ersten Zeile lesen.

**Was Main auf Wireframe-Ebene noch fehlt** (kein Hindernis für die Sonden,
aber vor Phase 2 zu klären): Begründung, Alternativursachen und billigster
nächster Beweisschritt am Befund; „keine Änderung empfohlen" als Ergebnis;
Suche/Pinning/Fokusgruppen; die musikalischen Rollen; Name und Rolle einer
Quelle in der Karte. Vollständige Liste mit Belegstellen:
`abnahmen/2026-08-21-codereview.md`.

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
