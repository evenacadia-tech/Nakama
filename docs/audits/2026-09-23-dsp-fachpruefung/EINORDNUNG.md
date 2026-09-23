# DSP-Fachprüfung — Einordnung des Dirigenten (23.09.2026)

**Gegenstand:** `BEFUNDE.md` (Nebensession `nakama-1f`, 25 Befunde DSP-01
bis DSP-25, Nachbauten unter `skripte/`). **Aufnahme:** Register NAK-379,
Commit `3da624e0` (23.09.2026, 18:13 Uhr; 101 Dateien hashgleich aus der
Arbeitskopie `C:\Users\phili\nk-dsp` übernommen). **Quellvalidierung** (Muster
NAK-246): drei frische, lesende Opus-5.5-Threads, Effort max, `dontAsk`, im
sichtbaren Checkout, HEAD `3da624e0` vor und nach jedem Lauf gleich, kein Bau,
kein Test, Skripte gelesen und nicht ausgeführt, eigene Nachrechnungen
außerhalb des Repos:

| Teil | Befunde | Thread | Bericht | Ergebnis |
|---|---|---|---|---|
| A | DSP-01 bis DSP-08 | `2f962554` | `docs/beweise/roh/NAK-379-dsp-validierung-teil-A.md` | 6 bestätigt, 2 präzisiert, 0 widerlegt |
| B | DSP-09 bis DSP-16 | `379b24d5` | `docs/beweise/roh/NAK-379-dsp-validierung-teil-B.md` | 4 bestätigt, 4 präzisiert, 0 widerlegt |
| C | DSP-17 bis DSP-25 | `a5cfb743` | `docs/beweise/roh/NAK-379-dsp-validierung-teil-C.md` | 4 bestätigt, 5 präzisiert, 0 widerlegt |

Aufträge: `docs/beweise/roh/NAK-379-dsp-validierung-teil-{A,B,C}-auftrag.txt`.
Start 18:13:41 Uhr, alle drei `FERTIG` bis 18:43 Uhr.

**Maßstab der Einordnung:** Defekt nur, wenn ein Satz des Entwurfs (Fassung
0.5), einer datierten Abnahme, einer Manifestregel, eines Vertrags oder einer
CLAUDE.md-Invariante bricht — am Wortlaut, den der Validierer zitiert.
Schweigen alle: Lücke (fachlich angreifbar), Härtung (wünschenswert) oder
Produktfrage (nur der User entscheidet, Karte). „Ruhend" = heute ohne
Aufrufer oder UI-Weg, am Code belegt. Die Klassenvorschläge der Validierer
sind Vorschläge; die Einordnung unten ist die des Dirigenten.

## 1. Einordnung je Befund

| ID | Validierung | Klasse (Dirigent) | heute erreichbar | Ort |
|---|---|---|---|---|
| DSP-01 | PRÄZISIERT (Spitze steht 0,13–0,15 s; 10–20 dB nur mit Q 24, sonst 8–10 dB; Maß an einem Prüfling verallgemeinert) | Lücke (mittel, hörbar) + Doku | ja | NAK-336 (d) erweitert; Bau in S25p (NAK-340, Etappe 0 „Kriteriensprünge"); „bis 8,0" in Entwurf §57.3, NAK-336 (d), NAK-337 berichtigen |
| DSP-02 | PRÄZISIERT (bis ~17 dB bei dynamischen Absenkungen im Bass; SVF-Weg läuft für jedes dynamische Band) | Lücke (hörbar) | ja | NAK-382 → S25p, Golden „SVF-Bell Q 24→6, ±12 dB, 50 Hz und 1 kHz, Maß R-311-13" |
| DSP-03 | BESTÄTIGT | **Defekt (ruhend)** — Entwurf §41.2 `:2730` „auf ±6 dB begrenzt", `:2735-2736` gemeinsame Absenkung | nein | NAK-381 → S25r (vor S29–31); `SONDE-015.md:1773` berichtigen (NAK-385) |
| DSP-04 | BESTÄTIGT | Lücke + **Produktfrage** | ja | Karte U61 (NAK-384) |
| DSP-05 | BESTÄTIGT | Lücke (Information; U45 „sind Technik") | ja | Zahl in NAK-331 nachgetragen; Karte U62 nennt den Boden |
| DSP-06 | BESTÄTIGT | Spezifikationslücke + **Produktfrage** + Doku (`DspFilter.h:220-223`) | ja | Karte U62 (NAK-384) |
| DSP-07 | BESTÄTIGT | **Produktfrage** (mit Tiefenaudit-3 O-04/F10) | ja | Karte U63 (NAK-384) |
| DSP-08 | BESTÄTIGT | **Produktfrage** + Doku (Vertrag `:109`, SONDE-015 R3 Feinheit 2 und 4) | ja | Karte U64 (NAK-384); Doku NAK-385 |
| DSP-09 | PRÄZISIERT | entschiedene Produktfrage (U46), Doku-Nachtrag | ja | NAK-332 ergänzt |
| DSP-10 | BESTÄTIGT | Lücke (Beweis) + Doku (Prüfungsname, Runnerbehauptung B6) | entfällt | NAK-383 → S26–28, Prüfauftrag G6 |
| DSP-11 | PRÄZISIERT (nur die Hörmatrix-Blende bricht §41.2 „5–20 ms"; übrige Übergänge ohne Zusage) | **Defekt (ruhend)** für die Hörmatrix-Blende; Härtung für EQ-Schalter, Bypass, Rampen; Doku R2 F1 | Hörmatrix nein, Rest ja | Hörmatrix-Teil NAK-381 → S25r; Rest NAK-383 → S26–28 |
| DSP-12 | BESTÄTIGT | Härtung (Benennung) + **Produktfrage** | ja | Karte U62 (NAK-384) |
| DSP-13 | BESTÄTIGT | Härtung (niedrig) + Doku (R9 F5) | ja, Folge in FL unbelegt | NAK-383 → S26–28 |
| DSP-14 | BESTÄTIGT | Härtung (ruhend) + Widerspruch Entwurf `:2975` gegen `eq-ipc-v3.schema.json:1973` | nein | NAK-381 (Güteformel vor S29–31); Widerspruchszeile NAK-385; Schemabereich beim nächsten v3-Vertragsschritt |
| DSP-15 | PRÄZISIERT | Härtung (niedrig) + Doku | ja | NAK-383 → S26–28 |
| DSP-16 | PRÄZISIERT | Doku (Entwurf `:2985`, M-30 „normalisierte Energie" gegen Code (L±R)/2; gebaute Wahl bleibt) | entfällt | NAK-385 |
| DSP-17 | PRÄZISIERT (Wirkung in der Richtung vertauscht: eine Spur mit Höhen **und** Bass rutscht hinter eine basslose Spur mit weniger Höhenenergie) | **Defekt** — Entwurf `:2285-2286` Einheiten + Lücke (Vertrag ohne Größe) | Rechnung ja, sichtbar nur mittelbar | NAK-380 → S25q |
| DSP-18 | BESTÄTIGT | **Defekt (ruhend)** — Entwurf `:1117`, `:2730` | nein | NAK-381 → S25r |
| DSP-19 | PRÄZISIERT (Guardrail-Schutz nur, solange keine gemessene Guardrail reißt; `wiederholbarkeit` des Ursachenfinders hat dieselbe Konstruktion) | **Defekt (ruhend)** — Entwurf `:2893-2894`, M-45 | nein | NAK-316 (S25o) ergänzt |
| DSP-20 | PRÄZISIERT (96 kHz zusätzlich 200–375 Hz, 192 kHz alles unter ≈ 750 Hz) | Lücke | ja | NAK-380 → S25q |
| DSP-21 | BESTÄTIGT | Lücke (Textwirkung M1) | ja | NAK-380 → S25q |
| DSP-22 | PRÄZISIERT | **Defekt am Wortlaut** Entwurf §39.1 `:2618-2619` (ruhend) + Lücke (absolute Schwelle) | Rechnung ja, Anzeige nein | NAK-380 → S25q |
| DSP-23 | BESTÄTIGT | **Defekt (ruhend)** — Entwurf `:2686-2687` | nein | NAK-380 → S25q, NAK-177 ergänzt |
| DSP-24 | BESTÄTIGT | Härtung (±0,1 LU eingehalten) | ja, unhörbar | NAK-199 ergänzt |
| DSP-25 | PRÄZISIERT (Klasse) | **Defekt (ruhend, gering)** — Entwurf `:2597-2598`, Tech 3342 §3.1 | nein | NAK-380 → S25q |

§4 (Textwidersprüche, nicht gegengeprüft): Doku-Zeile NAK-385, je Punkt beim
nächsten Entwurfs-Nachzug zu prüfen; die 3-dB-Grenzen sind NAK-368/NAK-369.
§5 (Prüfregeln für Goldens): Bauinhalt von NAK-383. §6 (Ergänzungsquellen):
Zeiger für `wissen/`, keine Produktentscheide.

## 2. Entscheide (Technik, Dirigent)

- **E-379-1 — Planschritt S25q, Ticket NAK-380 (Messkern):** DSP-17, DSP-22,
  DSP-23, DSP-25 (Defekte) und DSP-20, DSP-21 (Lücken im selben Code), ein
  Ticket mit Matrix vor Code, Verträge versioniert. Eingereiht **direkt nach
  S25l (NAK-313)** und vor S25m: DSP-17 verfälscht heute die Rechnung des
  Ursachenfinders (Vorrang externer Audit, User-Wort 10.09.2026); NAK-313
  bleibt „Als Nächstes", weil es einen State-Verlust trägt (T3-02-02).
- **E-379-2 — Planschritt S25r, Ticket NAK-381 (Hörmatrix und Vergleich):**
  DSP-03, DSP-11 (Hörmatrix-Blende), DSP-18 und DSP-14 — ruhende Defekte
  desselben Entwurfsabschnitts §41, zu schließen **vor der Verdrahtung der
  Hörmatrix und des Vorher/Nachher (S29–31)**; eingereiht am Ende von P4–P5.
- **E-379-3 — Rampenkomplex nach S25p (NAK-340):** DSP-01 als Erweiterung
  von NAK-336 (d), DSP-02 als NAK-382; die Rampen-Goldens bekommen dort eine
  physikalische Referenz (Querbezug DSP-10) und eine Länge in Millisekunden
  (DSP-11-Rest bleibt in S26–28).
- **E-379-4 — Beweislücken nach S26–28 als NAK-383:** DSP-10, DSP-11 (übrige
  Übergänge), DSP-13, DSP-15 und die Prüfregeln §5.
- **E-379-5 — Produktfragen als Karten U61 bis U64 (NAK-384):** Mono-Bass
  (DSP-04), dynamisches Band (DSP-06, DSP-12, mit DSP-05 als Information),
  Güte und Flanken (DSP-07), Mix-Regler (DSP-08); Dringlichkeit „vor der
  Abnahme der Klangregelung (S26–28)", gestellt über `/fragen`.
- **E-379-6 — Doku und Widersprüche als NAK-385:** §4 des Berichts, DSP-16,
  Vertragstext `mix` (`nakama-parameter-v2.json:109`), Entwurf `:2975` gegen
  Schema `:1973` (DSP-14), SONDE-015 R2 F1, `:1773`, `:1592`, `:1594`,
  Runnerbehauptung B6 (`tools/beweise.ps1:723`) und Prüfungsname
  `…_unter_minus_100_dbfs` (DSP-10); Abnahmeblatt vom 24.08.2026 `:56` als
  überholter Claude-Text vermerkt, nicht umgeschrieben.
- **E-379-7 — Nachträge in bestehende Zeilen:** NAK-177 (DSP-23), NAK-199
  (DSP-24), NAK-316 (DSP-19), NAK-331 (DSP-05), NAK-332 (DSP-09), NAK-336
  (DSP-01).
- **Nicht übernommen:** nichts — kein Befund widerlegt. Berichtigt gegenüber
  dem Bericht: Wirkungsrichtung von DSP-17 (Validierung C), Reichweite von
  DSP-01 („für Millisekunden" → 0,13–0,15 s; 10–20 dB nur bei Q 24),
  DSP-11 nur für die Hörmatrix ein Defekt, DSP-20 mit 96/192-kHz-Lücken.

Die Skripte unter `skripte/` sind Belege der Nebensession, keine Prüfbeine;
ihre Aussagen gelten bis zum Rotbeweis der jeweiligen Tickets als validierte
Rechnung, nicht als Messung am Binary (`BEFUNDE.md` §7).
