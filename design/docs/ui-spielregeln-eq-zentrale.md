# UI-Spielregeln der EQ-Zentrale — was jedes Design erfüllen muss

- **Stand:** 2026-08-23 · **Autor:** Claude (Technik-Vorgaben, keine Gestaltung)
- **Zweck:** Der User designt Gen Seite 1 + Seite 2 parallel zur
  Implementierung in Figma (sein Wort 23.08.: „das ist ja ein prozess kein
  ‚mal eben schnell machen'"). Dieses Blatt sind die **Spielregeln, die das
  Design erfüllen muss, um umsetzbar zu sein** — was die Technik liefert,
  was sie verlangt, was sie (noch) nicht kann.
- **Quellen (alle im Repo, vom Workspace-Root):** Entwurf
  `docs/FL-Nakama-Sonden-Design-Entwurf.md` (§44, §53.8, Erratum (n)) ·
  Parameterbestand `eq-copilot/schemas/state/nakama-parameter-v1.json` ·
  Capability-Report `eq-copilot/identity/host-capabilities-fl-v1.json` ·
  Entscheid-Register in `CLAUDE.md` · Wortlaut der Entscheide:
  `design/abnahmen/2026-08-23-gen-eq-zentrale.md` und
  `design/abnahmen/2026-08-23-interview-struktur.md` (21 Festlegungen).
  Jede Regel trägt ihre Quelle; **(Vorschlag)** / **(Annahme)** =
  Claude-Setzung, noch ohne User-Wort. Zahlen sind Startwerte
  (Errata (f)), änderbar — aber nur versioniert, nie stillschweigend.
  **Für frische Sessions:** dieses Blatt gilt nur zusammen mit dem Repo —
  im Nakama-Workspace öffnen, nicht als loser Text weitergeben.

## 1. Rahmen

1. **Ein Fenster je Instanz.** Seite 1 ↔ Seite 2 ist ein Umschalten im
   selben Plugin-Editor — VST3 kennt kein zweites Fenster je Instanz.
   Übergänge sind Seitenwechsel, keine Popups mit Eigenleben.
2. **Produktsprache Englisch** (Register 21.08.); dunkel verbindlich,
   hell geplant (Figma-Depot 22.08.).
3. **Größen:** 760×430 gilt für **beide** Seiten — Entscheid 24.08.: „da es
   sich um 2 flächen der selben app handelt müssen die selbstverständlich
   gleich groß sein" (die Abnahme vom 20.08. galt der Übersicht; die
   Größenfrage aus NAK-65 ist damit beantwortet). Freie Skalierung ist
   technisch da (der heutige Editor skaliert frei bei festem Verhältnis),
   aber als Produktentscheid offen.
4. **Working Design:** Probeeq-Figma ist die Arbeitsgrundlage der
   EQ-Fläche (User 23.08.); Handling-Referenzen: Smooth Operator Pro,
   Pro-Q — Bedienlogik studieren erlaubt, die Optik bleibt Nakama.

## 2. Was eine EQ-Spur an Bedienbarem hat (Datenmodell)

5. **Je Sonde: 8 Band-Slots** mit je 13 Parametern — enabled · type ·
   freq · gain · Q · channel_mode (Stereo/L/R/M/S) | dyn_enabled ·
   threshold · range · attack · hold · release · sidechain_source
   (§53.8; im ersten Release bleibt die Sidechain-Quelle unsichtbar,
   Entscheid 24.08.; Default `none`). Dazu
   **globale Regler je Spur:** Hard-Bypass · Input-Trim ±24 dB ·
   Output-Trim ±24 dB · Width 0–2 · Mono-Bass 0–500 Hz
   (`nakama-parameter-v1.json`, 109 Hostparameter, IDs eingefroren).
6. **Bereiche — für Hand UND Advisor-Drafts dieselben:** 20 Hz–20 kHz
   (Laufzeit-Deckel 0,45·Samplerate), ±12 dB Gain, Q 0,15–24 (§44.2).
   **Das alte Remote-Limit (±3 dB, Q 0,4–2) ist entfallen** (Entscheid
   23.08., U14 beantwortet; Regel 44): ein Draft ist im EQ frei
   nachjustierbar, eine Sonderbegrenzung des Vorschlagswegs gibt es
   nicht mehr.
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
    sichtbares eigenes Undo je Fläche ist Pflicht. Auf Seite 1 öffnet ein
    kleines Verlaufssymbol den echten Revisionsverlauf bestätigter Änderungen;
    auf Seite 2 wirkt das kleine Undo-Symbol je gewählter Sonde. Ein bloßes
    `UNDO · n` ohne Verlauf kommt nicht zurück (Entscheid 24.08.).

## 5. Durchschalten und Mengen

19. **Der Sonden-Durchschalter sitzt AUF Seite 2** — nie über Seite 1
    (Entscheid 23.08.: „1 klick oder 3 klicks dazwischen liegen welten").
20. **Mausrad wechselt die Sonde, solange nichts markiert ist** (User
    23.08.); ein markiertes Element besitzt das Rad (Band-Feinjustage,
    Pro-Q-Konvention) **(Angenommen, unbestätigt)**.
21. **Mengen:** Regelfall einstellige Quellenzahl, sichtbar bis 16,
    Vertrag bis 32 (Startwerte 21.08.). Der Durchschalter muss 1–16
    ohne Menü tragen; die **Reihenfolge** ist die FL-Mixer-Reihenfolge,
    und der Master ist keine Rad-Position, sondern ein eigenes UI-Element
    (Regel 33).
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

## 9. Nachträge — Interview `struktur` Runden 2–3 (23.08., Abnahme
`../abnahmen/2026-08-23-interview-struktur.md`)

32. **Mix (Dry/Wet je Sonde) ist beschlossen, steht aber NOCH NICHT im
    Parameterbestand** — `nakama-parameter-v1.json` kennt global nur
    Bypass/Trims/Width/Mono-Bass. Umsetzung als versionierte Erweiterung
    mit neuer ID (NAK-64 Punkt 4). Sichtbar auf der Rückfallfläche UND
    („ganz wichtig") in Gen. Bis der Vertrag ihn trägt, darf kein Blatt
    ihn als vorhanden zeigen, wohl aber als geplanten Platz.
33. **Durchschalt-Reihenfolge = FL-Mixer-Reihenfolge; Master als eigenes
    UI-Element** neben dem Rad-Kreis (Entscheid).
34. **Rückfallfläche der Sonde** (Entscheid + Annahme): Performance
    zuerst — die Fläche bleibt minimal und kostet geschlossen nichts.
    Inhalt: CONNECTED/DISCONNECTED · EQ-Mode-Punkt grün/rot · aktuell
    veränderte EQ-Werte (Anzeige) · Bypass (bedienbar) · Mix (bedienbar).
    Prüfstein für jeden weiteren Inhalt: „was könnte den User veranlassen,
    Probeeq zu öffnen statt gleich Gen?" Finaler Zuschnitt erst, wenn der
    User Gens Je-Sonde-Anzeige gesehen hat (offen).
35. **Master-EQ-Umfang: das Rad nicht neu erfinden** — Vorbild Pro-Q /
    Smooth Operator Pro, Machbarkeit bewertet die Technik. Die sechs
    Filtertypen des Vertrags (bell, low/high_shelf, notch, low/high_cut)
    decken „alle gängigen filter"; SOPs Resonanzpeak-Fokus ist spätere
    Ausbaustufe.

**Runde 3 (Button für Button am Working Design):**

36. **Durchschalter je Sonde: nur der Bus-Name** — kein Status, kein
    Vorschlags-Hinweis im Schalter selbst (Entscheid).
37. **Draft-Kasten-Zustandsautomat** (Entscheid „stimmt so"): HOLD TO
    AUDITION = Halten (Preview, Regel 10) · APPLY = erster Klick
    (temporär, Lease läuft) und derselbe Button wechselt in den
    Bestätigen-Zustand mit sichtbarer Restzeit · REJECT bleibt daneben
    und bricht ab · Ablauf ohne zweiten Klick = automatischer Revert
    (Regel 11).
38. **Live-Spektrum hinter der Kurve: ja** — aus der Sonden-Telemetrie;
    bei zwei aktiven Spuren NUR das Spektrum der gerade bearbeiteten
    Spur (Entscheid). Kadenz-Ehrlichkeit (Regel 25) gilt unverändert.
39. **PROTECTED-Zonen: harte Sperre für alle Nakama-Bedienwege**
    (Entscheid) — auch eigene Bänder lassen sich nicht in die Zone
    ziehen, bis sie gelöst ist. Anlegen durch den User; Gen darf eine
    Zone aus einem Befund VORSCHLAGEN, angelegt wird sie erst per
    User-Klick. Technisch: „User-Schutz" ist im Fernweg bereits ein
    Ablehnungsgrund; neu ist die Sperre für die eigene Hand.
    ⚠️ Ehrliche Grenze: FLs Host-Automation schreibt an der Sperre
    vorbei (Regel 17) — die Zone zeigt die Verletzung, verhindern kann
    sie sie dort nicht. Persistenz/Vertragsort: NAK-64 Punkt 5.

**Runde 4 (die letzten Buttons):**

40. **EQ-Mode und Bypass sind zwei Dinge** (Entscheid „ja korrekt so"):
    der EQ-Punkt grün/rot ist der Grundschalter „diese Sonde darf
    klingen" (Lebenslauf-/Mode-Zustand, kein Hostparameter), Bypass ist
    der schnelle Hörvergleich (`v1.global.bypass`, automatisierbar).
    Rückfallfläche präzisiert: CONNECTED/DISCONNECTED · Bypass · bei EQ
    on die veränderten Werte + der Mode der Probe (Lesart: Messposition
    PRE/INSERT/POST — Annahme).
41. **Undo ist ein kleines Symbol auf dem Display** — kein großer
    Button; ein Klick = letzter Schritt zurück (Annahme: kein Menü).
    **Reichweite: je Sonde** (Entscheid) — es nimmt nur auf der gerade
    gewählten Spur zurück; jede Instanz führt ihren eigenen
    Revisions-Ring, eine App-weite Kette gibt es nicht.
42. **Der Mix-Knob sitzt unten rechts und ist zweistufig** (Entscheid):
    er gilt der ganzen App, ein Klick schaltet auf „nur die gewählte
    Spur" um — jede Sonde hat ihren eigenen Dry/Wet. Technische Lesart
    (Annahme, Umsetzung NAK-64 Punkt 4): zwei Schichten, global × je
    Spur; der wirksame Mix einer Sonde ist das Produkt beider.
43. **Die GLOBAL-Sektion ist einklappbar** (Entscheid): eingeklappt als
    Standard, ein kleiner Pfeil klappt sie aus. Inhalt unverändert die
    vier Werte aus Regel 5.
44. **Draft-Kasten-Wortlaut** (Entscheid): oben **„DRAFT"**, darunter
    die Klick/Hold-Elemente. **Limit-Angabe und Limit entfallen** („das
    kann im eq dann ja angepasst werden von selbst") — siehe Regel 6;
    U14 damit beantwortet, Register 23.08.

**Nachtrag 24.08. (Rückmeldung auf den ersten Figma-Umbau, Wortlaut
`../abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md`):**

45. **Beide Seiten gleich groß:** 760×430 gilt für Seite 1 UND Seite 2
    (Entscheid; Wortlaut in Regel 3).
46. **Keine toten Elemente** (Entscheid: „die schlimmste ui/ux sünde sind
    sinnlose tote elemente"): jedes sichtbare Element bedient einen
    Handgriff oder meldet ehrlich einen Zustand. Folgen für die
    EQ-Zentrale: EIN Link-Zustand statt CONNECTED·PAIRED — im Fehlerfall
    benennt der Text den gefallenen Abschnitt (BROKER OFFLINE ≠
    PROBE OFFLINE); kein Dauer-„LIVE" (nur Ausnahmen sprechen, Abnahme
    20.08.); der Seitenwechsel ist ein Tab-Paar OVERVIEW · EQ CENTER an
    identischer Position auf beiden Seiten; Aufklapp-Pfeile sitzen IM
    Bedienelement, nie frei daneben. Material/Schale (Glas, Grain,
    Lichtsaum) ist Design-Identität, kein totes Element.

## 10. Offen (Kennungen)

| Was | Wo |
|---|---|
| ~~Sidechain-Quelle je Band~~ — beantwortet 24.08.: im ersten Release unsichtbar, Parameter bleibt mit Default `none` gespeichert | **U5 / NAK-33 geschlossen** |
| ~~Größe Seite 2~~ (beantwortet 24.08.: 760×430, Regel 45) · Figma-Flächen · Rückfallfläche der Sonde | **NAK-65** |
| ~~Undo-Form auf Gen Seite 1~~ — beantwortet 24.08.: kleines Symbol öffnet echten Revisionsverlauf | **U2.8 geschlossen** |
| Mix/Dry-Wet: versionierte Parameter-Erweiterung, zweistufig global↔Spur (beschlossen, ungebaut) | Plan S26–28/S29–31 (`SONDE-015`/`016`) · Regeln 32/42 |
| ~~Umschnitt Bauaufteilung/P6–P7~~ — **gefahren 23.08.** (`e3dbad4`): S28b Gen-Master-EQ, S31b EQ-Zentrale-UI, Schutz-Zonen und Mix in S26–31 eingeplant | NAK-64 geschlossen |
