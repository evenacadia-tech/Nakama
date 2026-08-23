# UI-Spielregeln der EQ-Zentrale — was jedes Design erfüllen muss

- **Stand:** 2026-08-23 · **Autor:** Claude (Technik-Vorgaben, keine Gestaltung)
- **Zweck:** Der User designt Gen Seite 1 + Seite 2 parallel zur
  Implementierung in Figma (sein Wort 23.08.: „das ist ja ein prozess kein
  ‚mal eben schnell machen'"). Dieses Blatt sind die **Spielregeln, die das
  Design erfüllen muss, um umsetzbar zu sein** — was die Technik liefert,
  was sie verlangt, was sie (noch) nicht kann.
- **Quellen:** Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md`
  (§44, §53.8, Erratum (n)) · Parameterbestand
  `eq-copilot/schemas/state/nakama-parameter-v1.json` · Capability-Report
  `eq-copilot/identity/host-capabilities-fl-v1.json` · Register/Abnahmen
  23.08. Jede Regel trägt ihre Quelle; **(Vorschlag)** = Claude-Setzung,
  noch ohne User-Wort. Zahlen sind Startwerte (Errata (f)), änderbar —
  aber nur versioniert, nie stillschweigend.

## 1. Rahmen

1. **Ein Fenster je Instanz.** Seite 1 ↔ Seite 2 ist ein Umschalten im
   selben Plugin-Editor — VST3 kennt kein zweites Fenster je Instanz.
   Übergänge sind Seitenwechsel, keine Popups mit Eigenleben.
2. **Produktsprache Englisch** (Register 21.08.); dunkel verbindlich,
   hell geplant (Figma-Depot 22.08.).
3. **Größen:** 760×430 ist für die Übersicht abgenommen (20.08.); die
   Größe von Seite 2 ist **offen** (NAK-65). Freie Skalierung ist technisch
   da (der heutige Editor skaliert frei bei festem Verhältnis), aber als
   Produktentscheid offen.
4. **Working Design:** Probeeq-Figma ist die Arbeitsgrundlage der
   EQ-Fläche (User 23.08.); Handling-Referenzen: Smooth Operator Pro,
   Pro-Q — Bedienlogik studieren erlaubt, die Optik bleibt Nakama.

## 2. Was eine EQ-Spur an Bedienbarem hat (Datenmodell)

5. **Je Sonde: 8 Band-Slots** mit je 13 Parametern — enabled · type ·
   freq · gain · Q · channel_mode (Stereo/L/R/M/S) | dyn_enabled ·
   threshold · range · attack · hold · release · sidechain_source
   (§53.8; Sichtbarkeit der Sidechain-Quelle offen: U5/NAK-33). Dazu
   **globale Regler je Spur:** Hard-Bypass · Input-Trim ±24 dB ·
   Output-Trim ±24 dB · Width 0–2 · Mono-Bass 0–500 Hz
   (`nakama-parameter-v1.json`, 109 Hostparameter, IDs eingefroren).
6. **Bereiche manuell:** 20 Hz–20 kHz (Laufzeit-Deckel 0,45·Samplerate),
   ±12 dB Gain, Q 0,15–24 (§44.2). **Advisor-Vorschläge:** ±3 dB, Q 0,4–2
   (§44.2) — ob das Cap künftig NUR den Advisor bindet, ist Frage **U14**.
7. **Der erste Kern ist minimalphasig, 0 Samples Latenz** — kein
   Linear-Phase-Schalter, kein Lookahead im ersten Design (§44.2).
   Dynamik-Reduktion: Standard 1,5 dB, Hard-Cap 3 dB (Startwerte).
8. **Typ-, Kanalmodus- und Dyn-an/aus-Wechsel sind „topologisch":** sie
   wechseln am Blockrand per Crossfade zwischen zwei vorbereiteten
   Programmen (§44.2) und werden im ersten Release nicht samplegenau
   automatisiert. Design darf sie nicht als stufenloses Morphing
   versprechen.
9. **Master-EQ in Gen:** gleicher Parametersatz wie eine Sonde als
   **(Vorschlag)**-Startannahme; was „vollwertig wie andere am Markt"
   darüber hinaus heißt, klärt das Interview (offen).

## 3. Die Anwenden-Geste (Entscheid 23.08., technisch gedeckt)

10. **Halten = hören:** technisch `preview_begin/renew/end`. Eine Preview
    läuft **nur** bei gültigem, laufendem „spielt" und **nie** bei
    Aufnahme oder unbekanntem Aufnahmezustand — das blockiert **sichtbar**
    (Entwurf: „unbekannter Record-State blockiert, damit kein flüchtiger
    Zustand versehentlich aufgenommen wird"). Das Design braucht diesen
    Verweigerungszustand: Transport steht / Aufnahme läuft.
11. **1. Klick = angewendet auf Probe:** Apply mit befristetem Lease,
    Rückfall nach **10 s** (User-Startwert 23.08.). Stop, Lease-Ablauf,
    Sidechain-/Prozessorfehler oder Transport-Epochenwechsel blenden zum
    gespeicherten Zustand zurück (Entwurf §Preview). Ob die Restzeit
    sichtbar tickt, ist Designfreiheit — die Zahl ist technisch vorhanden.
12. **2. Klick = permanent** (bestätigte Revision). Jede Anwenden-Stelle
    braucht den erreichbaren **Rückweg** in derselben Ansicht
    (Gegenpfade-Invariante).
13. **Gespeichert und gerendert wird nur der bestätigte Stand** —
    gehaltene Previews und nicht bestätigte Erst-Klicks landen nie im
    Projektfile und nie im Export **(Vorschlag, Folge des Grundgesetzes
    „nichts Ungefragtes")**.
14. **Die Geste wohnt an drei Orten** (User 23.08.): am Befund (Seite 1),
    als Sprungziel-Element im Sonden-EQ (Seite 2), und der Sprung-Button
    daneben wechselt direkt in die betroffene Sonden-Ansicht.

## 4. Fernbedienung heißt: die Wahrheit wohnt in der Sonde

15. **Seite 2 ist Fernbedienung.** Parameter- und Zustands-Hoheit liegt in
    der Sonden-Instanz (Grundgesetz 23.08.). Die gezeigte Kurve ist der
    **bestätigte** Zustand der Sonde; ein gesendeter, noch unbestätigter
    Befehl ist als „unterwegs" unterscheidbar (`command_ack`/
    `state_report` existieren dafür) **(Vorschlag zur Darstellungspflicht)**.
16. **Verbindung weg = ehrlich zeigen:** Ohne Broker/Sonde friert die
    Sonde auf dem bestätigten Klang ein. Seite 2 zeigt die Spur dann als
    nicht erreichbar mit gesperrten Reglern — **keine editierbare
    Geisterkurve** (§Lease/Revert; Grundgesetz).
17. **FL-Automation schreibt an der Sonde vorbei an Gen** und gewinnt
    sichtbar; eine dadurch abweichende Revision macht den Gen-Entwurf
    stale (Entwurf, wörtlich). Regler dürfen sich also „von selbst"
    bewegen; das Design darf keine Exklusivität der Maus suggerieren.
18. **Undo ist unser eigenes:** Host-Undo sieht Ferneingriffe nicht
    zuverlässig (Entwurf §44); es gibt den eigenen Revisions-Ring. Ein
    sichtbares eigenes Undo je Fläche ist Pflicht; seine Form ist offen
    (U2.8-Folgefrage: Verlauf oder Zähler).

## 5. Durchschalten und Mengen

19. **Der Sonden-Durchschalter sitzt AUF Seite 2** — nie über Seite 1
    (Entscheid 23.08.: „1 klick oder 3 klicks dazwischen liegen welten").
20. **Mausrad wechselt die Sonde, solange nichts markiert ist** (User
    23.08.); ein markiertes Element besitzt das Rad (Band-Feinjustage,
    Pro-Q-Konvention) **(Angenommen, unbestätigt)**.
21. **Mengen:** Regelfall einstellige Quellenzahl, sichtbar bis 16,
    Vertrag bis 32 (Startwerte 21.08.). Der Durchschalter muss 1–16
    ohne Menü tragen; die **Reihenfolge** der Sonden ist offen (Interview).
22. **Zwei Spuren in EINEM Graph** (Entscheid 23.08.): Standard EINE
    Spur; optional eine frei wählbare zweite; Kurven farblich
    unterscheidbar; bei zwei aktiven wählt der User das
    **Bearbeitungsziel** — es muss jederzeit eindeutig ablesbar sein,
    welche Kurve die Maus trifft.

## 6. Ehrlichkeits-Pflichten (gelten für jede Fassung)

23. **Sichtbar in jeder Fassung:** Frische/stale je Spur · Konfidenz ·
    Capability-Degradation (Bauaufteilung §0.4/§1.4; die konkrete
    Zählerliste steht in NAK-57).
24. **Zwei verschiedene Löcher, zwei Zeichen:** „der Host ist gesprungen"
    (Epoche) und „unsere Analyse hat eine Lücke" (Segment) dürfen in
    keiner Anzeige verschmelzen (§32.3, NAK-57).
25. **Analyzer-Kadenz ist Datenkadenz:** Leichtpfad ~20 Hz, Schwerlauf
    alle 250 ms. Malen darf flüssiger sein als messen, aber nichts
    suggerieren, was die Daten nicht haben (Invariante paint-FPS ≠
    Datenkadenz).
26. **Drei Mess-Abgriffe existieren** in der aktiven Sonde: `pre_nakama`,
    `post_committed`, `post_candidate` (§44.2). Welche der Analyzer
    zeigt, muss beschriftet sein; Candidate wird nie als Normalzustand
    dargestellt.

## 7. Host-Realitäten (FL Studio, gemessen 22.08.)

27. **Keine samplegenaue Automation:** FL legt nie mehr als einen Punkt
    je Block und zerteilt Blöcke bis auf 1 Sample (Capability-Report).
    Feine Automation ist Blockrampen-Realität — kein UI-Versprechen
    „sample-genau".
28. **Nur Float-Audio** (kein Double-Callback in FL) · Blöcke 1–4096
    Samples · Presentation-Latency ungeklärt (NAK-43). Nichts davon darf
    eine UI-Aussage werden, bevor das Bit `supported` ist.
29. **Hören nur mit gültigem „spielt"** — steht der Transport, hört man
    weder Markierung noch Preview (U10-Entscheid, §Preview). Das Design
    zeigt den Grund, statt still zu schweigen.

## 8. Für spätere Design-Stufen vorgemerkt

30. **A/B/Compare:** Der DSP-Entwurf führt Committed- und Candidate-Pfad
    getrennt (§44.2); Blind-A/B verbirgt die Identität bis zum Urteil
    (§43). Eine spätere Compare-UI muss diese Verdeckung darstellen
    können.
31. **„Effekt solo + Wirkung je Instrument"** (User-Vision 23.08.):
    Ausbaustufen laut Abnahme — erst Attribution über die vorhandene
    Sonden-Telemetrie + Sprung zur Sonde mit vorgeladener Frequenz;
    echtes Bus-Audio-Streaming nach Gen ist geparkt.

## 9. Offen (Kennungen)

| Was | Wo |
|---|---|
| ±3-dB-Cap: bindet es künftig nur den Advisor? | `docs/plan/fragen.json` **U14** |
| Sidechain-Quelle je Band sichtbar/Untermenü/unsichtbar | **U5** / NAK-33 |
| Größe Seite 2 · Figma-Flächen · Rückfallfläche der Sonde | **NAK-65** |
| Undo-Form (Verlauf oder Zähler) | U2.8-Folgefrage |
| Sonden-Reihenfolge beim Durchschalten · Master-EQ-Umfang · Überladen-Grenze | Interview `struktur`, Runde 2 |
| Umschnitt Bauaufteilung/P6–P7 auf den neuen Schnitt | **NAK-64** |
