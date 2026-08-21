# Arbeitsplan — womit konkret angefangen wird

Der Fahrplan dieses Projekts. Eine Phase endet erst mit einer Abnahme in
`abnahmen/`; keine Phase wird übersprungen, keine vorgezogen. Methodik je
Phase: `CLAUDE-UX-UI-ARBEITSKERN.md` (Stufen 1–7); Hintergründe:
`UX-UI-WISSENSBASIS-FUER-CLAUDE.md`.

> **Stand 2026-08-21, nach dem Designvertrag:** Der User gestaltet das finale
> Design in **Figma**; seine Stände liegen in `assets/figma/` und sind die
> Vorgabe („Figma ist Quelle; Repo setzt um"). Damit ändert sich der Plan:
> Phase 0 ist erledigt, die Stufen, die eine eigene Stilsuche oder eine
> Varianten-Befragung vorsahen, **entfallen** (sie bleiben unten stehen, als
> Verlauf gekennzeichnet), und die Arbeit heißt jetzt **Übersetzen** —
> Phase 1b. **HIER weiter: Phase 1b, Schritt Gen.**
>
> **Stand 2026-08-21, Sessionende (Suna-Runde):** Der Gen-Stand in der Truhe
> ist die *alte* Sprache; die Familie spricht seit dem markierten Probeeq-Stand
> anders. User-Wort: **die nächste Session entwirft Nakama Gen in Figma, in der
> Probeeq-Sprache, von vorne** — wie Suna in Studie 03 (Figma-Node 6039:2,
> Vorgehen in `abnahmen/2026-08-21-suna-auftrag-figma.md`). Ein früherer
> Codex-Versuch ist kein Referenzmaterial. Erst danach beginnt das Übersetzen
> in `prototyp/`, gegen den dann exportierten Stand.

## Phase 0 — Designvertrag ✓ (erledigt 2026-08-21)

**Einstieg war:** `/interview designvertrag` führen — noch nichts bauen.
Danach die Assettruhe sichten und laut zusammenfassen, was dort liegt.

**Ergebnis:** `abnahmen/JJJJ-MM-TT-designvertrag.md` mit: wichtigster
Nutzerjob, 1–3 Erfolgskriterien, 2–3 Gefühlsattributen (mit
Konsequenzen), Referenzen + Antireferenzen (je mit Grund), technischen
Grenzen (Fenstergrößen, Skalierung), Grad der kreativen Autorenschaft.

**Gate:** Ohne diese Abnahme blockt die Schleuse jeden Prototyp-Write.

> **✓ Erledigt 2026-08-21:** `abnahmen/2026-08-21-designvertrag.md`, aus dem
> Kontext-Interview des Users (zwanzig Fragen, Wortlaut je Urteil). Die
> Schleuse vor `prototyp/` ist damit **offen** (gemessen: Write und Bash
> nach `prototyp/` exit 0, vorher exit 2). Von der Liste oben fehlen
> **Erfolgskriterien und Gefühlsattribute in Worten** — das Interview hat
> sie nicht gefragt, der User hat stattdessen Bilder geliefert. Sie stehen
> im Vertrag als offen und werden beim User erfragt, wenn eine Abnahme sie
> braucht; sie werden nicht aus dem Bild gelesen. Die Truhe ist gesichtet:
> `assets/LIES-MICH.md`.

## Phase 0.5 — Formfaktor und Größe ✓ (erledigt 2026-08-20)

Eine Anordnung ohne bekannte Fläche ist wertlos. Für alle drei Oberflächen
zuerst festlegen: **Seitenverhältnis, Größe, und ob fest / in Stufen / frei
ziehbar.** Beurteilt wird ausschließlich bei **1:1** auf dem Zielbildschirm,
über einem nachgestellten Host — ein skaliertes Bild taugt dafür nicht.

Werkzeug: `werkzeug/formfaktor.html`. Es misst, ab welcher Größe Inhalt
abgeschnitten wird, und zeigt den Preis in Prozent des Bildschirms.

**Ergebnis:** drei abgenommene Formate — **Gen 760×430 · Probeeq 700×420 ·
Suna 260×84** (`abnahmen/2026-08-20-groessen-alle-drei.md`). Tote Fläche
gilt als Fehler. Hintergrund: `abnahmen/2026-08-20-korrektur-formfaktor-zuerst.md`.
Die Figma-Stände vom 2026-08-21 sind auf genau diesen Maßen gezeichnet
(vierfach exportiert, gemessen in `assets/figma/LIES-MICH.md`).

## Phase 1 — Wireframes (Main ✓ · Editor und Kachel entfallen seit 2026-08-21)

**Reihenfolge war: Main zuerst** (dort steckt die eigentliche
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
Letzter Entwurf: `werkzeug/zustaende.html`. **Diese Abnahmen gelten weiter**
— der Figma-Stand von Gen wird gegen sie gelesen (Phase 1b).

> **Entfallen seit 2026-08-21 — Editor und Kachel als eigener Entwurfsschritt.**
> Der Figma-Stand von Probeeq zeigt die Anordnung (Bänder als Griffe auf der
> Kurve, eine Bandzeile, Draft-Kasten, Statuszeile); sie wird nicht mehr hier
> entworfen. Was aus dieser Phase bleibt und in Phase 1b gebraucht wird:
> die **Messbank** `werkzeug/sonde-messung.html` (was der Inhalt auf 700×420
> kostet — die Klemme gilt für den Stand genauso), der Auftrag und die
> Stolperfallen in `docs/sondenplan.md`, und die **Aufgabenliste nach
> Häufigkeit** als Prüfliste statt Vorbedingung: jedes Element des Stands
> muss sagen, welchen Handgriff es bedient; ein Handgriff ohne Ort ist ein
> offener Punkt für den User. Der Stand zu diesem Zeitpunkt: Schritt 2
> (natürliche Höhen messen) war erledigt, **19 nachprüfbare Angaben, 10
> davon falsch**, berichtigt in `docs/sondenplan.md`; Schritt 1
> (Aufgabenliste) war Entwurf und nicht abgenommen — das bleibt so.
>
> **Was Main auf Wireframe-Ebene noch fehlt** (vollständige Liste mit
> Belegstellen: `abnahmen/2026-08-21-codereview.md`): Begründung,
> Alternativursachen und billigster nächster Beweisschritt am Befund; „keine
> Änderung empfohlen" als Ergebnis; Suche/Pinning/Fokusgruppen; die
> musikalischen Rollen; Name und Rolle einer Quelle in der Karte. Der
> Gen-Stand zeigt davon Begründung (`LIKELY CAUSE`) und Beweisschritt
> (`SMALLEST TEST`) und die Namen in der Karte; der Rest wandert als offene
> Frage in Phase 1b.

## Phase 1b — Figma-Stände übersetzen (HIER weiter, seit 2026-08-21)

Je Stand ein lebendes Blatt in `prototyp/` (Schleuse offen), **1:1 auf der
abgenommenen Größe**, aus dem Stand gebaut — nicht neu angeordnet, nicht
„verbessert". Die erste Entscheidung davor, mit dem User: ein Blatt pro
Oberfläche oder ein Blatt mit Tabs (`prototyp/LIES-MICH.md`).

**Reihenfolge: Gen → Probeeq → Suna.** Gen zuerst, weil dort die Abnahmen
vom 2026-08-20 liegen, gegen die der Stand gelesen werden muss; Suna zuletzt,
weil noch kein Stand in der Truhe vorliegt (seit 2026-08-21 ein Figma-Entwurf
von Claude, nicht abgenommen — `abnahmen/2026-08-21-suna-auftrag-figma.md`).

**Je Stand, in dieser Folge:**

1. **Lesen, nicht raten.** Den Stand Element für Element gegen
   `docs/oberflaechen-spezifikation.md` und die Abnahmen halten. Drei Listen
   entstehen: *deckt sich* · *weicht ab* (Stand sagt etwas anderes als eine
   Abnahme — dem User vorlegen, nicht entscheiden) · *fehlt im Bild*
   (Pflichtinhalt ohne Ort — offene Frage an den User). Für Gen ist der
   Anfang gemacht: `Nakama Designausarbeitungen selfmade/LIES-MICH.md`.
2. **Blatt bauen, 1:1.** Schrift, Abstände, Farben aus dem Stand (Export ist
   4×, also durch vier). Demo-Daten realistisch und als Demo gekennzeichnet.
   Playwright-Screenshot neben den Stand legen — erst wenn beides dasselbe
   zeigt, ist der Schönfall übersetzt.
3. **Zustände durchschaltbar.** Alle Zustände der Spezifikation als
   Schalter im Bedien-Panel: Quelle frisch / veraltet / getrennt, Messung
   läuft / unvollständig, degradiert, Draft da / weg, Vorhören gehalten,
   angewandt, Bypass, Pairing, verbunden / getrennt. Jeder Zustand, den der
   Stand nicht zeigt, wird aus seinen Zeichen abgeleitet und dem User als
   Frage vorgelegt — eine je Runde.
4. **Grenzfälle.** 16 Quellen (Belastungsprobe; Regelfall bleibt
   einstellig), alles getrennt, acht aktive Bänder mit Dynamik, Bypass an,
   vom Gen getrennt, Schutzbereich getroffen, langer Quellenname, 75 %-Stufe.
   Was bei 16 bricht, trägt nicht — der Preis steht sichtbar auf dem Blatt.
5. **Abnahme je Stand** in `abnahmen/`: was der User am Blatt beurteilt hat,
   wörtlich. Ein neuer Figma-Stand danach ist neues User-Wort und beginnt bei 1.

**Ergebnis:** drei übersetzte, je einzeln abgenommene Blätter — und eine
Liste offener Fragen an Figma, statt eigener Antworten darauf.

## Phase 2 — Visuelle Richtung (Stilprobe) — ENTFALLEN seit 2026-08-21

> Die Richtung kommt aus Figma; es gibt keine Stilprobe und keine
> Richtungs-Varianten mehr. Bleibt als Verlauf stehen.

*War:* An EINER Oberfläche (Active-Editor — die reichste vertraute
Konventionsfläche: EQ-Kurve, Regler, Werte) werden 2–3 Richtungs-
Varianten NEBENEINANDER gebaut, jede mit benannter Hypothese („Variante A
löst X besser, weil …"). Vorher `/interview richtung`. Ergebnis: eine
abgenommene Richtung (+ was aus den Verlierern mitgenommen wird).

## Phase 3 — UI-System und Ausrollen (Quelle ist jetzt Figma)

Design-Tokens (Farb-/Typo-/Spacing-Rollen) und Kernkomponenten (Panel,
Regler, Wertfeld, Kurvenfeld, Statuszeile, Befundkarte) — **aus den
Figma-Ständen gelesen**, nicht aus einer hier abgenommenen Richtung — dann
auf alle drei Blätter angewendet, damit die eine Identität auch im Code eine
ist. Editor-Panel in jedem Blatt: Tokens live verstellbar.

**Ergebnis:** drei gestaltete Blätter auf einem Tokensatz, je einzeln
abgenommen.

## Phase 4 — Zustände und Bewegung

Die Zustandsmatrix je Blatt entsteht bereits in Phase 1b, Schritt 3. Hier
kommt dazu: Motion nur wo sie Zustand/Wirkung zeigt, Intensität
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
- Ein neuer Figma-Stand ersetzt den älteren: Datei datiert ablegen,
  `assets/figma/LIES-MICH.md` nachziehen, Übersetzung bei Schritt 1 neu
  beginnen. Das ist kein Rückschritt — „Ein Design entwickelt sich".
