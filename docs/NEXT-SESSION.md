# NEXT-SESSION — Einstieg für die nächste Runde

> **WORKSPACE-UMZUG 18.08.2026:** Alles lebt jetzt HIER
> (`Projekte\Nakama`) — eq-copilot/, broker/ (eigenständig,
> eqcop-broker.exe), tools/. FL-Repo-SHAs in älteren Einträgen unten
> gehören zur FL-Studio-Historie (bis `7964777`).

> Stand: 2026-08-21 (nach S3, S3b und S5). Diese Datei ist der Schnellstart;
> Tiefe in `docs/design-stand.md` und `docs/geschmacksprofil.md`.
> **🔨 BAUENTSCHEIDUNG ERTEILT (User, 20.08.): der Sondenkern wird gebaut.**
> **S0 bis S3, S3b UND S5 sind gebaut** (Beweis-Runner + Basislinie ·
> Aux-/PDC-Messgerät · Identität eingefroren — alles 20.08.; JUCE-Bridge-Patch +
> Termin-B-Messgerät 21.08., beide mit T2-PASS; v3-Vertragsbaum 21.08., **T2 noch
> offen**). P0 steht an den **zwei FL-Terminen des Users**, P1 läuft daran vorbei
> weiter. Einstieg: „DER EINE NÄCHSTE SCHRITT".
> Kanon: **14/14 grün**.
> Sessionplan, Gates und Prüfstufen in `docs/bauaufteilung-sonden.md`.
> Die Design-Spur läuft parallel weiter und blockiert nichts.
> **Erledigt am 20.08.:** /freshen gelaufen (+ eigenes Playbook);
> Sondenentwurf auf **Fassung 0.4** (Opus-Prüfbericht komplett
> eingearbeitet, §0.3 Produktarchitektur, §0.4 Arbeitsmodell +
> Design-Spuren) — technisch baureif, **Bauentscheidung weiter offen**
> (erste Welle: SONDE-004 zuerst); Design-Projekt
> `Projekte\Nakama-Design` mit Fundament angelegt (Arbeitsplan,
> /interview-Skill, Prototyp-Schleuse, UX-Arbeitskern/Wissensbasis).
> Der User macht die Plugin-UI dort mit Opus; Prisma-Arbeit bleibt HIER.

## Visueller Nordstern — vor jeder Designarbeit lesen

Der User hat am 19.08. das langfristige Zielbild **Nakama als
Hörkompass** freigegeben. Verbindlicher Zielvertrag und
Fünf-Keyframe-Storyboard:
`docs/visuelles-zielbild-hoerkompass.md`.

Kurzform: Der Mix bleibt unsichtbar; nur dort, wo er sich selbst im Weg
steht, bricht das klare Prisma das Licht. Gesund = leeres Glas. Nur
belastbare Befunde erzeugen eine lokale Schlieren-Messung. Das Prisma
wendet die betroffene Fläche zum User; Klick öffnet Ursache, Hörbeweis und
genau einen nächsten Versuch; nach erfolgreicher Prüfung klärt sich das
Glas wieder. **Keine neue Grundmetapher und kein Dauer-Visualizer.**

**Produktarchitektur (User-Entscheid 20.08.):** Die Prisma-App ist eine
EIGENSTÄNDIGE Begleit-App NEBEN Master-Plugin + Sonden — Addition, nie
Master-Hub. Das Master-Plugin (Nakama Main) bekommt eine konventionellere
Arbeits-UI mit den Sonden; der Hörkompass-Zielvertrag gilt der Prisma-App.
Verbindlich festgehalten in `docs/FL-Nakama-Sonden-Design-Entwurf.md`
§0.3 (Fassung 0.4, mit eingearbeitetem Prüfbericht). Die konventionelle
Plugin-UI-Spur arbeitet seit 20.08. im eigenen, kontextreinen Projekt
`C:\Users\phili\Projekte\Nakama-Design` (Opus-Sessions; Assettruhe;
Abnahmen bindend) — von dort nichts hierher spiegeln, bis der User eine
Abnahme bringt.

Dieser strategische Lock ersetzt NICHT den unmittelbar freigegebenen
Technikschritt unten. NAK-16 muss dem Zielbild dienen, darf es aber nicht
durch einen neuen Effekt oder eine neue Metapher umdeuten.

## 🔎 FÜR EINEN CODE-REVIEW DIESER SESSION — hier anfangen

In dieser Runde sind **drei** Tickets entstanden. `SONDE-003` und `S3b` sind
bereits durch den `evaluator`-Frischkontextprüfer gelaufen — in **3** bzw. **4
Runden**, beide am Ende **PASS**. `SONDE-005a` ist **noch nicht** durch T2. Wer
jetzt reviewt, sollte wissen, was dort schon gejagt wurde, um nicht dieselben
Wege doppelt zu gehen.

### Was zu reviewen ist

| Ticket | Diff | Beweismanifest | T2 |
|---|---|---|---|
| `SONDE-003` — JUCE-Bridge-Patch | `git diff 0ba87cc..e9c6fa0` | `docs/beweise/SONDE-003.md` | ✅ PASS (3 Runden) |
| `S3b` — Termin-B-Messgerät | `git diff e9c6fa0..22d23d7` | `docs/beweise/SONDE-003b.md` | ✅ PASS (4 Runden) |
| `SONDE-005a` — v3-Vertragsbaum | `git diff 5299037..HEAD` | `docs/beweise/SONDE-005a.md` | **offen** |

Zwischen `22d23d7` und `5299037` liegen reine Doku-Commits — **kein Code**.

Die Manifeste tragen zu **jeder** Behauptung die rohe Ausgabe, dazu §6 mit allen
Befunden und ihrem Ausgang. Der Gate-Text steht jeweils im Kopf **im Wortlaut**
aus `docs/FL-Nakama-Sonden-Design-Entwurf.md` — nicht als meine Zusammenfassung.

### Die riskantesten Flächen, in dieser Reihenfolge

1. **`third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`** — ein Patch am
   gevendorten JUCE. Neun Ankerstellen. Die Kernfrage für einen Reviewer:
   **ändert er irgendwo Verhalten statt nur zu beobachten?** JUCEs eigener
   Parameterweg steht unverändert daneben; das Gate `eq-copilot/cmake/NakamaBruecke.cmake`
   bricht bei fremdem Quellstand ab.
2. **`eq-copilot/plugin/hostbridge/NakamaHostBridge.h`** — läuft im Audiothread.
   Vorallokiert, lock-frei, 0 Allokationen (gemessen). Zwei Verhältnisse sind
   Vertrag und waren beide schon einmal falsch: der **Rückfallwert überlebt den
   Ringüberlauf** (eigene `Letztwert`-Tabelle), und die **Zähler beschreiben, was
   der Host geliefert hat** — nicht, was in unsere Struktur passte.
3. **`eq-copilot/plugin/hostprobe/`** — Wegwerf-Messgerät, aber mit Seqlock
   zwischen Audio- und Nachrichtenthread und einem Editor, dessen Höhe aus dem
   Inhalt gerechnet wird.
4. **`eq-copilot/schemas/v3/` + die beiden Engines** (`plugin/vertrag/`,
   `broker/src/vertrag.rs`) — zwei handgeschriebene JSON-Schema-Engines, die
   IDENTISCH klassifizieren müssen. Die Kernfrage für einen Reviewer:
   **gibt es eine Eingabe, bei der die beiden auseinanderlaufen und der Korpus
   es nicht sieht?** Der Riegel dagegen ist, dass beide ein Schema mit einem
   nicht implementierten Schlüsselwort ABLEHNEN statt es zu ignorieren; wer
   den aushebelt, hebelt den ganzen Vertrag aus. Auslegung: `schemas/v3/README.md`.

### Was T2 bereits gefunden hat (alles gefixt, Riegel fällt nachweislich)

Nur damit klar ist, welche Fehlerklassen schon durchgekämmt sind:

* §53.7 war nicht eingelöst — der Überlauf verwarf genau den Vertrags-Rückfallwert
  (gemessen 0.511 statt 0.777, und zwar mit `true` gemeldet).
* Eine Regression aus der eigenen Nacharbeit: NaN als Punkt 513 wurde stiller
  Rückfallwert, während `unplausibleWerte` 0 meldete.
* Der Editor schnitt 49 px ab — ausgerechnet die Automationszeilen; ich hatte
  das Gerät nie gerendert.
* Eine Prüfung, die durch eine Längenklausel **0 Fälle** verglich und deshalb
  unbedingt grün war.
* Ein Zähler, der Blöcke statt Änderungen zählte (eine Hostmeldung wäre als
  sechsstellige Zahl erschienen).
* Dazu: `.gitattributes` hätte den Patch beim Commit kaputtnormalisiert, und der
  Baustand-Scan des Beweis-Runners sah fünf Quellorte nicht.

### Was ausdrücklich NICHT bewiesen ist

Kein Reviewer muss das erst herausfinden — es steht so in beiden Manifesten §0:

* **Was FL tatsächlich antwortet.** Ob ein `ProcessContext` anliegt, welche
  Validity-Bits gesetzt sind, ob Presentation-Latency überhaupt gemeldet wird,
  ob samplegenaue Automation ankommt — headless nicht beweisbar. Das ist
  **Termin B**.
* **Seek gegen Smart Disable.** Ein Vorwärtssprung ist aus den Daten allein
  nicht trennbar. Steht in Anzeige, Klickliste und JSON als Grenze.
* **Tearing-Freiheit von `ereignisseLesen`.** Der nebenläufige Test ist ein
  Rauchtest; der Wiederholpfad wird praktisch nie betreten. Auch das steht so im
  Quelltext und im Manifest.
* **Kein Processor des Produkts implementiert die `Senke`.** Das ist Schnittgrenze,
  keine Lücke — `EqCopilotProcessor` bleibt in `SONDE-003` unangetastet, damit der
  Patch am Produkt nachweislich folgenlos ist. Der Verbraucher kommt in
  `SONDE-008/009` zusammen mit **NAK-24**.

### Alles nachfahren

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/review.md -Titel 'Review'
eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe <ziel.png>
```

Der zweite Befehl rendert zusätzlich die beiden Bildbeweise neu — sie liegen in
`eq-copilot/docs/bilder/` und sind gegen ein unabhängiges Rendering
SHA-256-identisch geprüft worden.

---

## ▶ DER EINE NÄCHSTE SCHRITT — 👤 **User-Termin B in FL**, dann S4

**S0–S3 und S3b sind gebaut** (Beweis-Runner · Aux-/PDC-Messgerät · Identität
eingefroren · JUCE-Bridge-Patch · Termin-B-Messgerät). Der Bau steht damit an einer Stelle, an der er **nur noch
durch Messungen aus FL** weitergeht: P0 endet mit dem Capabilityreport, und der
braucht beide User-Termine.

### Was beim User liegt — zwei Termine, beide in FL

| Termin | Klickliste | Dauer | Was zurückkommt |
|---|---|---|---|
| **A** — Aux-Layout, Kanalreihenfolge, Recall, PDC-Impulse | `eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md` | ~20 min | zwei JSON aus `%APPDATA%\evenacadia\nakama\spike\` |
| **B** — Live/Stop/Seek/Loop-Straddle/Render/Smart Disable/Automation | `eq-copilot/docs/FL-TERMIN-B-HOSTZEIT.md` | ~25 min | eine JSON aus `%APPDATA%\evenacadia\nakama\spike\` + optional `nakama-altprojekt.flp` (§54 Punkt 2) |

Termin B ist zugleich der **einzige** Weg, die Hostbrücke im echten Host zu
sehen: ob FL einen `ProcessContext` anlegt, welche Validity-Flags gesetzt sind,
ob überhaupt Presentation-Latency gemeldet wird und wie sich ein Loop-Straddle
innerhalb eines Blocks verhält. Headless ist all das nicht beweisbar — und wird
deshalb auch nirgends behauptet (`docs/beweise/SONDE-003.md` §0 sagt es
ausdrücklich).

✅ **Termin B hat sein Messgerät (S3b, gebaut 21.08.).** `EqCopHostProbe` ist das
erste Ziel, das die Hostbrücke wirklich benutzt: sein Processor ist eine
`Senke` und zeigt live an, was FL liefert — Context-Anwesenheit,
Gültigkeitsbits als **immer / manchmal / nie**, Zeitsprünge für Seek, Loop und
Smart Disable (mit Fehlalarm-Riegel gegen normales Spielen und gegen
Stop/Play), Offline-Render, float/double, Presentation-Latency und vor allem
**wie viele Automationspunkte je Block ankommen**. Selbsttest 53/53,
`pluginval` Strenge 8 SUCCESS, Kanon 7/7.

Damit liegen **beide Termine beim User** — mehr ist an P0 ohne seine Messwerte
nicht zu bauen. `S4` und das Gate `G0` hängen daran.

### ✅ S5 UND S6 sind gebaut und T2-geprüft (21.08.) — der v3-Vertrag steht

Die Fläche, die nicht an den FL-Terminen hängt, ist fertig: **`SONDE-005a`**
(JSON) und **`SONDE-005b`** (FlatBuffers) liegen in `eq-copilot/schemas/v3/`.
Manifeste: `docs/beweise/SONDE-005a.md` und `docs/beweise/SONDE-005b.md`.

- **17 der 25 Nachrichtenfamilien** aus §33.3 sind definiert, die restlichen
  **8 namentlich reserviert** mit Eigentümerticket und vom Parser abgelehnt.
- **Beide Bandgitter** als eingefrorene Zahlen (221 + 64); von vier denkbaren
  IEC-Konventionen trifft nur eine die 221 aus §33.2.
- **`capabilities` ist der Satz aus §53.6, wörtlich und vollständig** — alle
  zehn. Vorher standen dort sieben selbst gebaute Namen.
- **153 JSON-Fixtures + 40 Binärfixtures**, beide mit **handgeschriebenem**
  Manifest, gefahren von je drei bzw. zwei Prüfbeinen.
- **Codegen-Drift ist 0**: `flatc` auf einen **Commit** gepinnt (nicht auf
  einen Tag — der Upstream führt für 25.12.19 deren zwei), Compiler,
  C++-Header und Rust-Crate aus derselben Quelle, Neugenerierung bytegleich.
- Kanon **7/7 → 14/14**, `pluginval` Strenge 8 SUCCESS.

**T2 ist gelaufen — in zwei Runden, beide NEEDS_WORK, beide mit echten
Blockern.** Was sie gefunden haben, steht vollständig in
`docs/beweise/SONDE-005a.md` §6 (24 + 12 Befunde mit Quelle und Ausgang).
Die zwei Sätze, die ein neuer Leser mitnehmen sollte:

> **1. `C++/Rust validieren identisch` war zweimal falsch, und beide Male lag
> es an JUCEs Zahlenleser.** `parseNumber` akkumuliert ohne Bereichsprüfung
> (`18446744073709552016` kam als **400** an), und `readDoubleValue` läuft im
> Exponenten über (`1e4294967296` kam als **1.0** an). Der Vertrag hat
> deshalb jetzt eine Stufe VOR dem Parser — den **Textriegel**, acht Regeln
> auf dem Rohtext.
>
> **2. Ein Riegel darf nie die Bibliothek befragen, gegen deren Verhalten er
> schützt.** Die Ganzzahlregel war aus dem Literal gerechnet und hat
> gehalten; die Endlichkeitsregel war an `getDoubleValue()` delegiert und hat
> nicht gehalten. Beide rechnen jetzt aus dem Literal.

Dazu eine strukturelle Lehre: die Falltabelle des Riegels lag als **drei
handgepflegte Kopien** in den drei Beinen — gezählt 31, 32 und 33 Fälle,
während das Manifest ihre Gleichheit behauptete. Sie ist jetzt **eine
gelesene Datei** (`fixtures/v3/TEXTRIEGEL-FAELLE.json`, 59 Fälle).

**Offen an S5/S6:** der **Schließungsvorbehalt aus §65** — das Ticket gilt
erst nach der Capabilityentscheidung aus S4 als geschlossen. Die Form des
Vertrags kann die Hostantwort nicht mehr ändern; was fehlt, sind **Werte**.
Ausnahme mit Ticket: **NAK-27** (`offline_render_detection` ist keine
§53.6-Capability — eine elfte wäre ein Major-Schritt, keine Minor-Erweiterung).

### Die nächste baubare Fläche ohne FL-Termine

**S7 (`SONDE-006`)** — State-Schema 2, fester Parameterbestand, reine
Schema-1-Migration. Dort wird auch die RFC-8785-Kanonisierung für
`state_hash` erstmals gebraucht (in S5 bewusst NICHT vorgebaut), und dort
gehört der Herkunftstag aus §33.4 hin.

Danach **S8 (`SONDE-007a`)** — der gemeinsame Kern ohne `JucePlugin_*`-
Konstanten (NAK-23b); laut `bauaufteilung-sonden.md` der Umbau, den der
Entwurf unterschätzt.

Drei offene Punkte aus den T2-Runden hängen an späteren Tickets und stehen
mit ID im Offen-Set: **NAK-27** (Capabilitysatz), **NAK-28**
(`subscribe_session` ohne Gegenpfad), **NAK-29** (bedingte Feldpflichten im
Transportstempel → `SONDE-009`).

### Danach: S4 → Gate G0

**S4** wertet beide Termine zu einem **Capabilityreport mit Rohmessdaten** aus
(kein Bit wird aus einer Versionsnummer vermutet, Entwurf §53.6). **G0** ist dann
das erste Phasengate: `/c-review` gegen den Bridge-Patch + **Codex als zweites
Modell**, mit Falsifikationsauftrag gegen die harten Gates **1** und **5**.

### Was S3 hinterlassen hat (21.08., `1e91d54`)

- **`third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`** — neun
  anker-genaue Stellen im gevendorten Wrapper. Er **nimmt nichts weg**: JUCEs
  eigener Parameterweg (nur letzter Punkt) steht unverändert daneben, die
  Brücke beobachtet zusätzlich. Ohne `NAKAMA_HOST_BRIDGE` ist der Wrapper
  wortgleich JUCE — der Fallbackweg wird in **jedem** Bau mitkompiliert
  (`EqCopAuxSpike_VST3` hat das Define nicht).
- **`eq-copilot/cmake/NakamaBruecke.cmake`** — Quellhash-Gate, alle drei Zweige
  vorgeführt statt behauptet: unberührt ⇒ patchen **und nachmessen** · gepatcht
  ⇒ No-Op · fremd ⇒ Bauabbruch mit dem **gemessenen** Hash. Setzt FetchContent
  die Datei zurück, heilt der nächste Configure von selbst.
- **`eq-copilot/plugin/hostbridge/NakamaHostBridge.h`** — die Gegenseite,
  bewusst JUCE- und SDK-frei: die riskante Abbildung liegt damit in UNSEREM Repo
  und ist headless prüfbar, während der Patch am Fremdcode dünner Klebstoff
  bleibt und bei jedem JUCE-Update billig neu zu beweisen ist.
- **`EqCopHostContextTest`** — **91 Prüfungen**. Die Kontext-Abbildung wird
  gegen den **echten** `Steinberg::Vst::ProcessContext` instanziiert, nicht gegen
  einen Nachbau; ein falsch abgeschriebenes Flag oder Feld (SDK-Schreibweise
  `continousTimeSamples`, ohne „u") würde die Kompilierung brechen.
- **T2 brauchte DREI Runden, und das war den Aufwand wert.** Runde 1 und 2 waren
  NEEDS_WORK: erst war Entwurf §53.7 nicht eingelöst (der Überlauf verwarf genau
  den Rückfallwert, den der Vertrag überleben lässt — gemessen 0.511 statt
  0.777, und zwar mit `true` gemeldet, also ununterscheidbar von einem gültigen
  Wert), dann baute meine eigene Nacharbeit eine Regression ein (der
  Überlauf-Ausstieg sprang über die Plausibilitätsprüfungen, ein NaN als Punkt
  513 wurde stiller Rückfallwert bei `unplausibleWerte == 0`). Beides gefixt,
  beide Riegel nachweislich zum Fallen gebracht. **Lehre für die nächsten
  Tickets:** ein Puffer, der beim Überlauf hinten abschneidet, darf nie die
  Quelle eines Wertes sein, den ein Vertrag überleben lässt — und ein Zähler
  muss beschreiben, was der HOST geliefert hat, nicht was in unsere Struktur
  passte.
- **Der Kanon wächst auf 6/6** — `EqCopHostContextTest` war als „geplant (ab P2)"
  vorgesehen und ist ab jetzt Pflicht.

**Zwei Riegel, die vorher fehlten und nebenbei nachgezogen wurden:** der
Baustand-Scan des Runners sah `hostbridge/`, `spike/`, `probe/`, `cmake/` und
den Patch nicht (hätte veraltete Binärdateien als beglaubigt durchgelassen), und
`.gitattributes` hätte den Patch beim Commit kaputtnormalisiert — JUCE speichert
den Wrapper mit **CRLF**, der Patch trägt es in 149 von 163 Zeilen.

### Was beim Bauen gilt

- **1 Session = 1 Ticket + Beweismanifest + Frischkontext-Prüfer.** Code ohne
  Manifest heißt *offen*, nicht *fast fertig*.
- **T1** Selbstaudit je Commit · **T2** `evaluator`-Subagent je Ticket (sieht
  nur Diff + Manifest + Gate-Text aus dem Entwurf) · **T3** adversariale Runde
  je Phasengate in eigener Session (`/c-review`, `/rust-review`,
  `/security-review` + **Codex als zweites Modell**), mit
  **Falsifikationsauftrag** statt Prüfliste.
- Jeder T3-Befund wird **gegen die Quelldatei verifiziert**, bevor gehandelt
  wird (~25 % AI-Auditbefunde waren hier falsch), und verschwindet nie still:
  gefixt, NAK-Zeile oder protokollierte Widerlegung.
- Beweislauf: `pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/<Ticket>.md -Anhaengen`.

---


## Parallel offen — Design-Spur (User-Aktion, blockiert den Bau NICHT)

**NAK-16 PROBE-STILL IST GEBAUT UND BESTANDEN (19.08., Commits cba6bd6 +
602b20a). Offen: der User SIEHT SICH DAS LEBENDE BLATT AN —
`eq-copilot/design/prisma/stmap-probe.html` doppelklicken (Testmuster)
und mit `?bg=unicorn` (lebender Nexus, braucht Internet).** Optik-Urteil
gehört ihm: Dispersion-Stärke (`?dispersion=0.035` Vorgabe), Glanz-Rig
(zwei Kanten + Front-Streifen, Dark-Field), TIR-Zonen (12 % der
Glasfläche zeigen ehrlich nur Glanz statt Hintergrund).

Was bewiesen ist (`renders/stmap/messung.json` + probe-live-a/b.png +
probe-live-unicorn.png): **74,2 % der Glaspixel ändern sich mit dem
Hintergrund-Stand** (Erfolgsmaßstab der Freigabe — Ähnlichkeit zum alten
Frame bleibt als Metrik VERBOTEN); Konsistenz außerhalb 0/255;
Direktsicht-Beweis p99 0,019 px; B nicht Fresnel-gewichtet; 64 Samples
genügen; Half-Float trägt; RGBA16F-Upload ok. Fallen + Details:
`docs/design-stand.md` (NAK-16-Abschnitt); verbindliche Pipeline
weiterhin `docs/research/2026-08-19-stmap-live-refraktion.md`.

**Lichtwelt-Workflow steht (19.08. nachmittags):** Der User designt die
Ebene 0 SELBST in Unicorn („Fläche mit Raumtiefe, Licht von weit her");
Orientierung: `docs/spielregeln-skizze.html` (4 Tafeln, auch als
privates Artifact) + `prisma/vorlage-lichtwelt.html` (Referenzbild) +
`prisma/vorlage-tiefenkarte.html` (Parallax-Depth-Map, ?horizont=).
Das Probe-Blatt hat ein **Lade-Panel** (unten rechts): Projekt-ID oder
Unicorn-JSON-Export direkt laden (Blob-URL, kein Publish nötig) —
Technik aus dem evenacadia-site-Editor übernommen, SDK v2.2.10 lokal
gevendort (`design/vendor/`), Mount/Destroy serialisiert; alle drei
Wege headless bewiesen. Front-Kicker aus dem Glanz-Pass ist RAUS
(User-Befund: statisches Band = eingebackene fremde Beleuchtung).
Sonifold-Ingredients (WGSL) portiere ich bei Bedarf pro Effekt nach
GLSL.

**72er-SEQUENZ GEBAUT (19.08. abends): das Prisma DREHT im Probe-Blatt.**
Im Panel „DREHKARTEN LADEN …" →
`renders/stmap/dreh-karten.bin.gz` wählen (17,4 MB: 72 ST-Maps halbe
Auflösung + Silhouetten-Crop + Glanz-WebPs, gzip); danach Ziel „Prisma"
→ Ziehen dreht (Trägheit + Einrasten auf 58,3°), Mausrad schubst.
Glanz-Rig wird PRO FRAME am evaluierten Mesh gerechnet (Silhouetten-
Ecken + Weißbrand-Riegel — das mitdrehende Rig war falsch, die
Spiegelbedingung braucht die feste Kamera). **Gemessene Eigenschaft,
kein Bug:** in den Edge-on-Zonen (±30° um die Kanten-Pose, 3×/Umdrehung)
ist der TIR-Anteil 99 % → das Glas verdunkelt beim Wenden und klart an
den Rastposen (12 %) auf — ehrliche Physik ohne Umgebungs-Spiegelung;
ob das als Geste trägt, ist User-Urteil (Eskalation wäre eine
Reflexions-Karte). Regenerieren: `--nur-stmap --dreh 72` (5 min) +
`--nur-glanz --dreh 72` (13 min) + `konvertiere-stmap.py --dreh 72`;
EXRs sind gitignored, Paket + Glanz-WebPs versioniert.

**Danach (braucht ggf. frischen Freigabe-Marker):**
Integration ins Prototyp-/Schlieren-Blatt: Ebene 0 braucht ÜBERSTAND
(Ziel-UVs bis u=1,5; Probe nutzt 1920×1280 hinter 768×1024-Glas),
getContext-Patch (preserveDrawingBuffer) VOR dem Unicorn-SDK-Load
mitnehmen; Offsets sind Glas-Canvas-UV → affin in Hintergrund-UV.

Maschinen-Fakten Sequenz: Kamera = `KAM` im Blatt (lens 65,
−1,65/−3,53/0,95 → Ziel 0/0/0,8 — Code im Blatt schlägt ältere
Notizen), 768×1024; `--weiter` überspringt existierende Frames; alte
Beauty-Frames: ~75 s/Frame bei 320 Samples.

Danach weiter offen (User-Urteile): Boden-Kontaktebene an/aus (Taste B) ·
bandrelative Fahrt (Tasten 2/3) · Unicorn-Bindungen der
Drei-String-Szene (User-seitig).

Was die Messung ergab (Commit 7148248, Details `docs/design-stand.md`):

1. **lineIdx-Verdacht WIDERLEGT** — gemessen (1/60-s-Paar: nur
   Subpixel-Schimmer, 1 Pixel Vollausschlag; Noise-Floor exakt 0) und aus
   dem Code bewiesen (sichtbare Punkte enden bei 0,30·spacing, Identität
   kippt erst bei 0,5·spacing). Den notierten Shader-Umbau NICHT machen.
2. **Wahre Ursachen behoben:** stehender Prüfton (→ Fahrt, `?fahrt=0` oder
   `?ptonHz=` stellt still, Taste F) und Bin-Treppe (→ Sub-Bin-Interpolation
   in Blatt UND Brücke, befundOrt beider Seiten bitgleich verifiziert).

Offen daran: die Persistenz eines Wanderers ist ehrlich niedrig (~23 %) —
der Wirbel ist deshalb kompakt statt feldhoch. Wenn der User mehr
Vertikale will, ist das eine Datenfrage (langsamere Fahrt, längerer
Aufenthalt), keine Zeichenfrage.

**KREATIV-SCHLEUSE:** Der aktuelle Marker `.claude/kreativ-freigabe.md`
enthält getrennt (a) den engen NAK-16-Bau-Scope und (b) den strategischen
Hörkompass-Lock. Der Nordstern ist keine pauschale Baufreigabe; nach Ablauf
der 24 h oder für zusätzlichen Scope erneut mit User-Wortlaut schreiben.

## Was steht (18.08., alles gemessen und committet)

- **Instrument kalibriert:** Deflection 100 → Lücke 195 %, engster Abstand
  77 %; linear; Spiegelsymmetrie 0,0 px; Mittellinie steht (719,5 bei 720,0).
- **Shader versioniert:** `prisma/schlieren-probe-field.frag` — ab jetzt die
  Quelle, Unicorn ist nur die Anzeige. Faktor 0,76 IST die Kalibrierung.
- **Punktgröße** `min(1,6·fwidth, 0,30·Abstand)` — beide Reinformen sind
  gescheitert (Vollfläche bei klein, unsichtbar bei 4K bzw. auf der
  Prismenfläche).
- **Brücke** sendet 0–1 (am Szenen-Export belegt, nicht 0–100),
  `?projekt=ilYX64xlvU811imBKfxV&band=0`.
- **Unicorn-Fallen dokumentiert** in `docs/design-stand.md`: Regler von Hand
  verschieben löst die Bindung; falsch gebundene Variable ist still.
- Szenen-Sicherung: `eq-copilot/design/unicorn/schlieren-probe-field-szene.json`

## Historischer Stand vor dem Hörkompass-Lock (P01 im Glas — verworfen)

Dieser Abschnitt ist nur Verlaufsbeleg. Die damalige Suche nach einem
Flächen-Inhalt ist durch `docs/visuelles-zielbild-hoerkompass.md` beendet.
Material-Urteil ist da (18.08.): **klar** vorn, rauchig Reserve, Frost raus
(„zumindest auf den Bildern" — Anmutungs-Endabnahme am lebenden Objekt).
Kandidaten aus der User-autorisierten Richtung: Klangkörper-Silhouette ·
Balance-Ring · P01-Perkolation im Glas (einzige abgenommene Ästhetik).
Vision kommt vom User — Referenzen erfragen, treu übersetzen.

**Sicht-Probe LEBT (18.08.): P01-Perkolation im Glasvolumen** —
Film-Sandwich in der Prototyp-Seite (Beauty → additive Punkt-Ebene →
additiver Glas-Deckel), bewiesene Kameraprojektion
(`pruefe-projektion.html`), Songschleife, Bandwelten Amber/Magenta/Eis
(FL-Repo 6b5d4eb). Der User schaut sie an, um seine Vorstellung zu formen.

**Denk-Leinwand steht (18.08.):** `eq-copilot/design/prisma/
prisma-prototyp.html` — drehbares Prisma aus 60 vorgerenderten
Cycles-Frames (klar, 120°-Periode, `renders/dreh/`); Ziehen/Einrasten,
Tasten 1–3, `?winkel=` friert ein. Bewusst ohne Daten-/Effektebene.

Werkzeug steht: `eq-copilot/design/prisma/prisma-material-still.py`
(Blender 5.1 headless, Cycles CPU — nie GPU, Arc-Landmine; CLI: --variante
--samples --breite --hoehe --energie --kamera --ziel --nur-strahl).
Cycles-Grenzen (Spot-Kugelnormierung, Fächer-Sampelbarkeit, MNEE nur eine
Fläche) stehen in `design-stand.md`, Abschnitt AKTIVE RICHTUNG.

## Warum Prisma (Kurzform)

User-autorisiertes Konzept: 3 Flächen = Tiefen/Mitten/Höhen, Drehen =
Selektion, kompakter Dauerbegleiter statt Analyzer-Fenster. Gesund =
leeres Glas; nur belastbare Befunde brechen lokal das Prüffeld. Klick auf
einen Befund öffnet Ursache und Hörbeweis. Der Newton-Bündelmodus bleibt
als möglicher Gesamtblick erhalten, seine Geste ist noch offen.
Darstellung: lebender Hintergrund → ST-Map-Live-Refraktion → lokale
Schlieren-Befundebene → Glanz/Kanten. Details: `design-stand.md` und
`docs/visuelles-zielbild-hoerkompass.md`.

## Nicht verhandelbar (aus dieser Session gelernt)

- **KREATIV-SCHLEUSE:** Nie wieder losbauen ohne mit dem User
  ausgearbeitete Idee. Hook blockt Write/Edit unter `eq-copilot/design/`
  ohne frische `.claude/kreativ-freigabe.md` (Marker gilt 24 h — für die
  nächste Session neu schreiben, mit dem, was der User freigibt!).
- **Geschmacksprofil lesen** (`docs/geschmacksprofil.md`) vor jeder
  visuellen Entscheidung: die drei Gesetze + 3a–3c + Meta-Erkenntnis
  (Vision kommt vom User; Claudes freie Bilderfindung = Mode-Collapse;
  P01-Punktästhetik ist als Ästhetik abgenommen, als rohe Dauerwolke aber
  ausdrücklich verworfen).

## Infrastruktur, die steht

- `eq-copilot/design/unicorn-bruecke.html` — Unicorn-Szene + Musikdaten
  (JSON-Backups in `design/unicorn/`). Taste T = Tafel; die Tafel prüft
  seit 19.08. `bindingCount` und prangert definierte-aber-UNGEBUNDENE
  Variablen an. `?hintergrund=<ID>` legt eine zweite Szene als Ebene 0
  dahinter — Vorgabe „Winter Nexus (Remix) 3" `DAGhm8NIQHdX9GhNYmP9`,
  liegt auch live hinter `prisma-schlieren.html` (`?hintergrund=0` für
  Messläufe).
- `eq-copilot/design/proben/` — 4 Kalibrier-Proben + `render-probe.mjs`
  (generisch: Blatt, Name, Query, Größe, Wartezeit; Einzellauf, SwiftShader).
- 30-s-Songschleife (`songLage`/`musikDb`) — in jedem Blatt identisch
  portiert; deterministisch, `?zeit=` friert ein.
- Geparkt, unangetastet: Tiefenfeld + Bauplan 2.0 + Kunstwerk-Studie
  (drei Welten) — nie ungefragt reaktivieren.

## Offene kleine Punkte

- NAK-11: Unicorn-Lizenz („Legend"-Plan für Offline-JSON) vor
  Plugin-Auslieferung klären — Design-Phase unkritisch.
- ~~Nakama-Repo hat kein Git-Remote~~ **Erledigt 19.08.:** privates
  Remote `https://github.com/evenacadia-tech/Nakama` angelegt und
  gepusht (User arbeitet wechselnd Desktop/Laptop). Auf dem Laptop
  einmalig: `git clone`, dann in `eq-copilot/design/vendor-build/`
  `npm ci` + `npx playwright install chromium` (node_modules sind
  bewusst nicht im Repo); Broker per `cargo build --release`,
  Golden-WAVs per `py -3.13 tools/eq-copilot/erzeuge_fixtures.py
  --nur-wav`. Ab jetzt: nach Commits pushen, vor Arbeitsbeginn pullen.
