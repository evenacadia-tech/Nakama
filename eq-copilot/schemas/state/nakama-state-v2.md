# State-Schema 2 — `NakamaState` (SONDE-006)

- **Stand:** 2026-09-10 · **Ticket:** `SONDE-006` (S7), Kind `Dsp` definiert in `SONDE-015` (S26–28) · **Vertragstext:** Entwurf §53.8, §33.5, §32.1/32.2, §53.5, §67
- **Leser/Schreiber:** `eq-copilot/plugin/state/NakamaState.*` (C++, JUCE-core, keine `JucePlugin_*`-Konstante — S8-tauglich)
- **Beweis:** `EqCopStateMigrationTest` (Kanon B2), Goldens in `eq-copilot/fixtures/state/`, Python-Bein `tools/eq-copilot/erzeuge_state_fixtures.py --pruefen` (Kanon A12)
- **Was das hier ist:** das Dateiformat, das in FL-Projekten reist. Wie die VST3-Identität (SONDE-001) ist es ab jetzt ein **Vertrag**: eine Änderung an Root-Major, Kind-Major oder Kind-Matrix ist eine Versionierung mit Beleg, kein Edit.

## 1 · Träger

Der Host-State ist ein **JUCE-`ValueTree` im Binärformat** (`ValueTree::writeToStream`), wie schon Schema 1. Gründe: bit-exakte `double`-Werte, kompakt, und **unbekannte Eigenschaften überleben einen Roundtrip** (der Leser hält den Baum und schreibt ihn zurück, statt ihn aus Feldern neu zu bauen). Kein JSON im Host-State — JSON (RFC 8785) ist die Form des DSP-DTO für den `state_hash` (§4).

## 2 · Form

```
NakamaState                               schema = 2  (int)
├── Common                                schema = 1  (int)   PFLICHT
│     instance_id          string   bytegleich aus Schema-1 sensor_id; frisch: 32 Hex-Kleinbuchstaben (juce::Uuid)
│     plugin_kind          string   main | passive_probe | active_probe | legacy          (§32.2, v3 plugin_kind)
│     measurement_position string   insert | pre | post | post_fader_contribution         (§32.2, v3 measurement_position)
│     label                string   User-Wort, untrusted, nie interpretiert (auch leer; eigener Writer höchstens 120 Zeichen)
│     pair_id              string   NUR wenn nicht leer (fehlt = kein Paar; eigener Writer höchstens 60 Zeichen)
│     project_binding_id   string   NUR wenn bestätigt gebunden (hex32); wird NIE still erzeugt (§32.2)
├── MainProject                           schema = 1  (int)   nur plugin_kind = main (Pflicht dort)
│     confirmed_members_v1   array  optional; flach [instance_id, label, ...], höchstens 64 Paare
│     manual_passages_v1     array  optional; flach [passage_id, label, projekt_start, projekt_ende, ...], höchstens 64 Vierergruppen (SONDE-013 M-69)
│     source_intents_v1      array  optional; flach [quelle_id, passage_id|"", rolle, revision, herkunft, konfidenz, ...], höchstens 256 Sechsergruppen (SONDE-014 M-02)
│     intent_protections_v1  array  optional; flach [quelle_id, eigenschaft, band_von, band_bis, ...], höchstens 256 Vierergruppen (SONDE-014 M-03)
│     intent_relations_v1    array  optional; flach [quelle_a, quelle_b, art, ...], höchstens 256 Dreiergruppen (SONDE-014 M-06)
│     intent_revision_v1     int64  optional; Revision des GANZEN Intent-Bestands, ab 1 (SONDE-014 M-86)
├── Parameters                            schema = 1  (int)   nur plugin_kind = active_probe (Pflicht dort)
│     dsp_schema_version   int      OPTIONAL; fehlt = Layout v1 (109 Kennungen), 2 = Layout v2 (112 Host-Parameter);
│                                   jede andere Zahl ⇒ read-only (unbekanntes Layout-Major)
│     109 bzw. 112 Eigenschaften mit den IDs aus nakama-parameter-v1.json bzw. nakama-parameter-v2.json,
│     in Vertragsreihenfolge · bool → bool · float → double (bit-exakt) · enum → string (Enumwort)
├── Dsp                                   schema = 1  (int)   nur plugin_kind = active_probe, OPTIONAL (SONDE-015)
│     state_revision       int64    PFLICHT; ≥ 0, steigt mit jeder committeten Transaktion, sinkt nie
│     occupied_v1          array    optional; flach [bool × 8] in Slotreihenfolge; fehlt = kein Slot belegt
│     schutz_zonen_v1      array    optional; flach [id, low_hz, high_hz, enabled, …], höchstens 8 Vierergruppen,
│                                   streng aufsteigend nach id
│     undo_ring_v1         array    optional; höchstens 32 Einträge, jeder Eintrag ein VERSCHACHTELTES Array
│                                   [art, slot, revision, 120 Werte …, 4·z Zonenwerte …] mit z = 0…8
│     undo_cursor          int      optional; 0 = am jüngsten Eintrag, n = n Schritte zurückgenommen; ≤ Ringlänge
└── Pairing                               NAME RESERVIERT — Inhalt definiert SONDE-016 (Ziel-IDs, DPAPI-Blobs; nie Klartext, nie im Hash)
```

### 2.0 Das Kind `Dsp` (SONDE-015)

**Was es trägt und was nicht.** `Dsp` hält `state_revision`, die acht
`occupied`, die Schutz-Zonen, den Undo-Ring und seinen Cursor. Es hält **keine
zweite Kopie der Parameterwerte**: die 112 Host-Parameter stehen in
`Parameters`, und der bestätigte DspState ist die Vereinigung aus beiden
Kindern. Zwei Kopien derselben Werte können auseinanderlaufen, und §33.5
verbietet ausdrücklich eine zweite Wahrheit — ein Leser müsste bei Widerspruch
raten.

**Flache Arrays, keine Kindknoten.** Der Byte-Riegel (§5) erlaubt höchstens 64
`ValueTree`-Knoten im ganzen Baum. Acht Zonen und 32 Undo-Einträge als Knoten
wären allein 40. `Dsp` führt seine Listen deshalb als flache bzw.
verschachtelte Variantenarrays, wie `MainProject` es seit SONDE-012 tut.

**Wann `Dsp` überhaupt geschrieben wird.** Nur, wenn es etwas zu sagen gibt:
`state_revision ≠ 0` **oder** mindestens ein `occupied` **oder** mindestens
eine Zone **oder** mindestens ein Undo-Eintrag. Ein frischer, unberührter Stand
schreibt das Kind **nicht** und bleibt damit für einen Build lesbar, der `Dsp`
noch nicht kennt. Dieselbe Regel wie beim nie gesetzten
`MainProject.assistant_step_v1`: „noch nie benutzt" und „mit leeren Feldern
benutzt" wären in den Bytes sonst dasselbe.

**Der Undo-Eintrag ist ein voller Schnappschuss, kein Delta.** Jeder Eintrag
trägt einen Kopf (`art` als Wort aus der geschlossenen Menge `apply`, `revert`,
`neutralisieren`, `remove`, `preset_laden`, `gestus`; die betroffene Slot-ID
oder `-1`; die Revision, die er zurückgibt) und danach den vollständigen
DTO-Zustand: 120 Werte in Vertragsreihenfolge und die Zonenliste als
Vierergruppen. Seine Länge ist damit `123 + 4·z`; jede andere Länge ist ein
Fehler. Grund für den Schnappschuss: ein Deltaformat braucht je Handlungsart
einen eigenen Decoder, und ein falscher Decoder erzeugt lautlos einen halben
Zustand. `undo` und `redo` erzeugen **keinen** Eintrag — sie bewegen den
`undo_cursor` im selben Ring.

**Größe, gemessen statt geschätzt.** 32 Einträge à höchstens `123 + 32` = 155
Werte sind 4.960 Einträge — weit unter den 65.536 je Sammlung und den 262.144
im gesamten Baum, und weit unter 16 MiB.

Reihenfolge beim Schreiben: für Stände, die **dieser Schreiber** erzeugt (frisch, migriert), Kinder Common, MainProject, Parameters, Dsp und Eigenschaften in der Reihenfolge oben. Ein **geladener** Stand behält seine eigene Reihenfolge (der Schreiber editiert eine Kopie des gehaltenen Baums in place — unbekannte Eigenschaften eingeschlossen); fehlt einem geladenen Common das `label`, bekommt es beim Speichern eines. Damit ist `speichere(lade(x)) == x` **bytegleich** für jeden Stand, den dieser Schreiber selbst geschrieben hat (Goldens, Roundtrip-Test), und die Schema-1-Migration ist **deterministisch** (Golden-fähig).

### 2.1 Kind-Matrix (§53.8: „Unzulässige Ziel-/Kindkombinationen werden nicht teilweise interpretiert")

| `plugin_kind` | Common | MainProject | Parameters | Dsp | Pairing |
|---|---|---|---|---|---|
| `main` | Pflicht | **Pflicht** | verboten | verboten | ab SONDE-016 (heute: nicht lesbar ⇒ read-only) |
| `legacy` | Pflicht | verboten | verboten | verboten | verboten |
| `passive_probe` | Pflicht | verboten | verboten | verboten | verboten |
| `active_probe` | Pflicht | verboten | **Pflicht** | **optional** (seit SONDE-015 gelesen) | ab SONDE-016 (heute: nicht lesbar ⇒ read-only) |

Ein Kind mit unbekanntem Namen ⇒ read-only (ein neues Kind ist eine Root-Versionierung, keine Minor-Erweiterung). Eine **unbekannte Eigenschaft** in einem bekannten Kind desselben Majors ⇒ additiv, wird erhalten.

`MainProject.confirmed_members_v1` ist seit SONDE-012 die additive, persistente
Mitgliedschaftswahrheit. Jedes Paar besteht aus einer eindeutigen hex32-`instance_id`
und dem zugehörigen User-Label (String, auch leer, höchstens 120 Unicode-Codepoints).
Die Reihenfolge auf der Leitung ist `instance_id`-aufsteigend und trägt keine
Mixer- oder Verbindungsreihenfolge. Fehlt die Eigenschaft, ist die bestätigte
Menge leer. Runtime-Nonce, PID, Control-Liveness, Frische, Descriptor, Hostname,
Frames, Lautheit und Fehlergründe sind ausdrücklich **kein** Host-State.

`MainProject.manual_passages_v1` ist seit SONDE-013 (M-25, M-69) der
Projektintent der vom User markierten Passagen: Vierergruppen aus hex32-`passage_id`,
User-Label (höchstens 120 Codepoints), `projekt_start` und `projekt_ende` als
`int64` mit `0 <= start < ende`, höchstens 64 Einträge, sortiert nach Start.
Fingerprint, aktives Quellenset, Abdeckung und Transportepoche sind
**Messergebnisse** und bleiben im Store — sonst behauptete ein Projekt nach dem
Löschen der Datenbank weiter, es gebe Evidenz.

> **Nachgetragen am 06.09.2026 (SONDE-014, Nebenbefund N-02).** Der Zustandsbaum
> oben führte `manual_passages_v1` bis zu diesem Ticket **nicht**, obwohl
> `NakamaState.cpp` es seit SONDE-013 schreibt und liest. Der Vertragstext war
> gegenüber dem Code unvollständig; die Zeile steht jetzt an ihrem Platz.

### 2.1.1 Der musikalische Intent (SONDE-014, S23–25)

Vier weitere additive Eigenschaften im bekannten Kind `MainProject` — kein
neues Kind, keine Root-Versionierung. Entwurf §33.5 weist `SourceIntent` dem
`MainProjectState` zu; die Produktzusage aus Antwort U22 (06.09.2026) ist die
**Anzahl der Rollen: genau fünf**.

| Eigenschaft | Form | Regeln |
|---|---|---|
| `source_intents_v1` | flaches Array aus Sechsergruppen | `quelle_id` hex32; `passage_id` hex32 **oder leer** (leer = globaler Scope); `rolle` aus der geschlossenen Menge `fuehrt`, `traegt`, `begleitet`, `geschuetzt`, `verschmolzen`; `revision` `int64` ≥ 1; `herkunft` aus `user`, `template`, `inferred`; `konfidenz` endlich in `[0,1]`. Genau **ein** Eintrag je (Quelle, Scope). Höchstens 256 Gruppen, sortiert nach Quelle, dann Scope. |
| `intent_protections_v1` | flaches Array aus Vierergruppen | `quelle_id` hex32; `eigenschaft` aus `attack`, `breite`, `ausklang`, `band`; bei `band` ein halboffenes Intervall `[band_von, band_bis)` im 221-Band-Evidenzgitter (`0 <= von < bis <= 221`), sonst **beide `-1`** — `0` ist ein gültiger Bandindex und taugt deshalb nicht als „kein Intervall". Höchstens 256 Gruppen. |
| `intent_relations_v1` | flaches Array aus Dreiergruppen | `quelle_a` und `quelle_b` hex32 und **verschieden**; `art` aus `fuehrt_vor`, `darf_verschmelzen`, `gleichrangig`. Genau eine Beziehung je geordnetem Paar. Der `fuehrt_vor`-Teilgraph ist **zyklenfrei**: ein Zyklus macht den Stand read-only (§37.4). Höchstens 256 Gruppen. |
| `intent_revision_v1` | `int64` ≥ 1 | Revision des **ganzen** Bestands, steigt bei jeder persistenten Änderung genau einmal. Fehlt sie, ist der Bestand nie beschrieben worden; **Inhalt ohne Revision** ist ein ungültiger Stand, weil die Vollständigkeitsmarke aus SONDE-014 M-86 dann keine Zahl hätte. |

Die §37.1-Belegung (`prominence`, Funktionstag, Veto-Kennzeichen) wird aus der
Rolle **abgeleitet** und **nie** getrennt gespeichert: `fuehrt` → (`foreground`,
`lead`), `traegt` → (`middle`, `foundation`), `begleitet` → (`background`,
`texture`), `geschuetzt` → (`middle`, leerer Tag, Veto `schutz`),
`verschmolzen` → (`middle`, leerer Tag, Veto `verschmolzen`). Die Abbildung ist
total und injektiv; zwei Wahrheiten für dieselbe Aussage wären ein Verstoß
gegen §33.5.

Eine leere Liste bedeutet **Eigenschaft weg**, nicht leeres Array — sonst
unterschieden sich ein Projekt ohne Intent und eines, dessen letzter Eintrag
gelöscht wurde, in den Bytes. Golden: `fixtures/state/schema2/main-intent-v1.bin`,
vom Writer erzeugt (Kanon **A12**, geschrieben von
`EqCopStateMigrationTest --schreibe-goldens`).

### 2.2 Messposition je Klasse

| `plugin_kind` | erlaubte `measurement_position` |
|---|---|
| `main` | `insert` |
| `legacy` | `insert`, `pre`, `post` |
| `passive_probe` | `insert`, `pre`, `post` |
| `active_probe` | `insert`, `pre`, `post` |

**`post_fader_contribution` ist heute für KEINE Klasse erlaubt** (geändert
24.08.2026, G1-Nacharbeit zu Befund §4.2). Bis dahin stand in der
`passive_probe`-Zeile zusätzlich `post_fader_contribution`, ausdrücklich als
„Vorschlag für SONDE-007b" — und `NakamaState.cpp` setzte genau diesen
Vorschlag durch (`case Klasse::passive_probe: return true;`). Damit konnte eine
gewöhnliche Standard-Insertinstanz sich dauerhaft als exakter
Mastersummenbeitrag bezeichnen: **Gate 7** aus Entwurf §49.2 Nr. 7 im Wortlaut,
gefahren vom Gate-Lauf G1.

Zwei gemessene Dinge tragen die Rücknahme, keine Meinung:

1. Der Vorschlag war **nie angenommen**. In diesem Projekt existiert ein
   Entscheid nur mit Datum und Wortlaut des Users (`CLAUDE.md`); alles andere
   heißt Vorschlag und bindet nichts. Ein Vorschlag, den Code durchsetzt, ist
   die teuerste Sorte Notiz.
2. Die Capability `contribution_aux` steht im eingefrorenen Capabilityreport
   (`identity/host-capabilities-fl-v1.json`) auf **`unsupported`**, Rohfeld
   „nicht gemessen — kein Gerät misst die Main-Aux-Busse des Receivers". Der in
   §53.6 dafür festgelegte Fallback lautet wörtlich „nur Assoziation statt
   exakter Attribution". Und kein gebautes Bundle hat den Bus: `SondeProcessor`
   deklariert genau einen Stereo-Ein- und einen Stereo-Ausgang.

Der Wortschatz bleibt: `post_fader_contribution` steht weiter im v3-Vertrag
(`eq-ipc-v3.schema.json`, Zweig `probe_descriptor_beitrag`) und in
`Messposition`. Ein Vertrag muss die Position **ausdrücken** können, bevor ein
Wirt sie tragen kann — verboten ist nicht das Wort, sondern die **Behauptung**
einer Instanz, die den Bus nachweislich nicht hat.

Ein Altprojekt verliert dadurch nichts: ein State mit dieser Position fällt auf
**read-only mit den Originalbytes** (§5), sichtbar im Editor. Er behält seinen
Stand, er darf ihn nur nicht mehr behaupten.

Die Frage „**welche** Klasse darf die Position führen, wenn es den Bus gibt?"
bleibt **offen** und ist eine Produktfrage — der Entwurf beantwortet sie nicht
(§32.2 ordnet Positionen ihren Aussageklassen zu und benennt in :1610 den
Receiver, sagt aber nicht, welche Produktklasse die Position trägt). Bis zu
einer Antwort gilt oben die fail-closed-Vorgabe. Eigentümer: `SONDE-011`.

### 2.3 Bundle → erlaubte Klassen

Der Leser bekommt die Klassenmenge seines Bundles mit (S8: drei Bundles, ein Kern): `Eqcp` ⇒ {`main`, `legacy`} · `NkPr` ⇒ {`passive_probe`} · `NkAc` ⇒ {`active_probe`}. Ein State mit fremder Klasse ⇒ read-only — er gehört einem anderen Bundle (Copy/Paste zwischen Bundles, Preset-Browser).

## 3 · Migration Schema 1 → 2 (§32.2, §53.8 — rein, deterministisch)

Quelle: `EqCopilotState{schema=1, sensor_id, role, label, pair_id}` (Goldens `fixtures/identity/state-schema1-*.bin`).

| Schema 1 `role` | Schema 2 `plugin_kind` + `measurement_position` | v2-`hello`-Rolle (bis SONDE-010) |
|---|---|---|
| `hub` | `main` + `insert` (+ leeres `MainProject`) | `hub` |
| `sensor` | `legacy` + `insert` | `sensor` |
| `pre` | `legacy` + `pre` | `pre` |
| `post` | `legacy` + `post` | `post` |
| fehlt | wie `sensor` (der Schema-1-Leser las eine fehlende Rolle seit 0.1 als `sensor`) | `sensor` |
| unbekanntes Wort | **nicht migrierbar** ⇒ read-only, Originalbytes bewahrt | — |

- `sensor_id` → `instance_id` **bytegleich** (auch wenn es kein hex32 ist — ein altes Projekt behält seinen Messpunkt; die v3-Adresse `adresse.instance_id` verlangt hex32, die Abbildung ist Sache von SONDE-010 — **NAK-40**).
- `label` bytegleich; `pair_id` bytegleich, leer ⇒ Eigenschaft fehlt.
- `project_binding_id` wird **nicht** erfunden (§32.2: das führende Main bietet einen sichtbaren Join an und schreibt die Bindung erst mit Host-Dirty — P3).
- Leere `sensor_id` in Schema 1 ⇒ neue `juce::Uuid` (wie der Schema-1-Leser seit 0.1) — das ist die einzige nicht-deterministische Stelle, und sie tritt nur bei einem kaputten Altstate ein; die Goldens haben feste IDs.
- `runtime_nonce` entsteht bei jedem Laden neu und ist **nie** Teil des States (§32.2: `instance_nonce` war nie Projekt-State).
- Die Migration der Migration ist die Identität (Schema 2 rein ⇒ Schema 2 gleich raus).

## 4 · DSP-DTO und `state_hash` (§53.8)

- DTO-Form seit SONDE-015: `{"dsp_schema_version":2,"parameters":{<id>:<Wert>,…},"schutz_zonen":[…]}` — genau die 120 IDs aus `nakama-parameter-v2.json`, physikalische Werte, dazu die Zonenliste (0 bis 8 Einträge, streng aufsteigend nach `id`). Genau **drei** Wurzelschlüssel; RFC 8785 sortiert sie nach UTF-16-Code-Units zu `dsp_schema_version` < `parameters` < `schutz_zonen`. Layout v1 (`dsp_schema_version:1`, zwei Wurzelschlüssel, 109 IDs) bleibt als **Verlauf** in `nakama-parameter-v1.json` beschrieben; der heutige DTO-Leser nimmt es nicht mehr an.
- Validierung **vor** dem Hash (Reihenfolge fest, erster Fehler zählt): Textstufe doppelter Schlüssel → Typ/Struktur → `dsp_schema_version` → unbekannter Schlüssel → fehlender Schlüssel → Typ je Parameter → nichtendlich → Bereich/Enum → Zonenregeln (Anzahl, Typ, Bereich, doppelte `id`, Sortierung).
- Was **nicht** im DTO steht und deshalb nicht im Hash: `state_revision`, der Undo-Ring, der `undo_cursor` und der aus dem Programm **abgeleitete** Auto-Gain-Betrag. Sie sind Zustand über den Zustand, nicht der Zustand selbst.
- Kanon: **RFC 8785** (Schlüssel nach UTF-16-Code-Units sortiert, rekursiv; Zahlen nach ECMA-262 `Number::toString`; Strings mit `\b \t \n \f \r \" \\` und sonst `\u00xx` klein; kein Whitespace; UTF-8).
- `state_hash` = SHA-256-Hex (64 Kleinbuchstaben) des kanonischen UTF-8 — das Format, das v3 `state_report.state_hash` verlangt.
- Drei Beine gegen dieselben Fixtures (`fixtures/state/jcs/`, `fixtures/state/dto/`): C++ (`NakamaKanon`), Python (`rfc8785` 0.1.4, Trail of Bits), Rust (`serde_json_canonicalizer` 0.3.x + `sha2`). Die Zahlenvektoren aus RFC 8785 Anhang B stehen mit dem **vom RFC gedruckten** Erwartungstext im Erzeuger — die Python-Referenz wird damit selbst gegen den RFC gemessen, nicht nur gegen sich.
- **Der Kanon liest JSON selbst** (`kanon::lies`, `std::from_chars`), nicht über `juce::JSON`. Gemessen am Korpus (22.08.): JUCEs `readDoubleValue` liest `5e-324` und `0.000000000000000000000000001` als **0** (Subnormal-Flush, 18-Stellen-Puffer) und verweigert den leeren Schlüssel `""` (v3-Textriegel Regel 7 dokumentiert das für den IPC-Pfad). Für einen Hash, den drei Sprachen auf demselben Text bilden, darf der Zahlenleser nicht „ungefähr" sein — *ein Riegel befragt nie die Bibliothek, gegen die er schützt.* Der eigene Leser lehnt außerdem doppelte Schlüssel, einsame Surrogate, rohe Steuerzeichen, NUL und nicht darstellbare Zahlen (`1e999`) als Fehler ab statt still zu runden.

## 5 · Unbekanntes Major, read-only (§53.8, §33.5)

Tritt ein, wenn: Root-`schema` ≠ 2 (oder `EqCopilotState` mit `schema` ≠ 1) · `Common` fehlt oder `Common.schema` ≠ 1 · unbekanntes Enumwort · Klasse nicht im Bundle · Kind-Matrix verletzt · unbekanntes Kind · `Parameters.dsp_schema_version` ist vorhanden und weder 1 noch 2 (unbekanntes Layout-Major) · `Dsp.schema` ≠ 1 oder sein Inhalt verletzt eine Regel aus §2.0 · `Pairing` vorhanden, solange dieser Build es nicht liest.

Seit SONDE-015 ist `Dsp` für `active_probe` **kein** read-only-Grund mehr; für jede andere Klasse bleibt es einer (Kind-Matrix). Ein Build, der `Dsp` noch nicht liest, hält einen Stand mit diesem Kind weiterhin read-only mit Originalbytes — genau dafür schreibt der Writer das Kind nur, wenn es etwas trägt (§2.0).

Verhalten: **audio-neutral** (Passthrough wie immer) · **read-only** (`setzeBindung`/`neueSensorId` werden verweigert, kein Host-Dirty) · `getStateInformation` liefert die **Originalbytes unverändert** zurück (nie ein Teilstate) · keine Pipe-Anmeldung (es gibt keine vertrauenswürdige Identität zu melden) · Editor zeigt den Zustand (Anzeige-Pflicht „Capability-Degradation", §0.4).

Fremder Baumtyp oder Müllbytes: Zustand bleibt wie vor dem Aufruf (heutiges Verhalten, IdentityTest „Muellbytes aendern den Zustand nicht").

Vor dem JUCE-Leser prüft ein allokationsfreier Byte-Riegel genau einen
vollständigen `ValueTree`: höchstens 16 MiB, höchstens 64 `ValueTree`-Knoten
inklusive Wurzel sowie 63 ineinander geschachtelte Variantenarrays plus
Skalarblatt, höchstens 65.536 Einträge je
Sammlung sowie 262.144 Einträge im gesamten Baum. Abgeschnittene Streams,
Suffixbytes und deklarierte Längen außerhalb des vorhandenen Puffers gelten als
Müllbytes. Für einen **schreibbaren** geladenen State baut der Leser zusätzlich
den größten mit den heutigen API-Grenzen erreichbaren Folgezustand über demselben
additiven Baum. Passt dieser nicht vollständig unter 16 MiB, bleibt der Eingang
read-only und damit bytegleich. Dieser konkrete Headroom deckt die maximalen
heutigen Userfelder und verhindert, dass `Save(Load(x))` einen State erzeugt, den
derselbe Leser beim nächsten Start wegen seiner eigenen Grenze ablehnt. Marker, die JUCE
8 nicht byteverlustfrei zurückschreiben kann
(`undefined`/Marker 9 oder ein zukünftiger Marker), machen eine bekannte
`NakamaState`-/`EqCopilotState`-Wurzel dagegen **read-only**; die Originalbytes
gehen unverändert an den Host zurück.

## 6 · Host-Dirty (§32.2, §67 Punkt 4)

Jede persistente Änderung aus dem Plugin heraus (`setzeBindung` mit echter Änderung, `neueSensorId`, später: bestätigte Bindung) meldet `updateHostDisplay (ChangeDetails().withNonParameterStateChanged (true))` — der JUCE-VST3-Wrapper setzt daraus `IComponentHandler2::setDirty (true)`. **Laden und Migration melden nicht** (ein Projekt, das der User nicht speichert, bleibt Schema 1 und migriert beim nächsten Laden wieder — nichts geht verloren). Ohne echte Änderung: keine Meldung, kein Reconnect.

## 7 · Duplicate (§32.1 „Duplikate werden sichtbar aufgelöst")

Zwei Instanzen aus denselben Bytes tragen dieselbe `instance_id` (Absicht: der State IST der Messpunkt) und verschiedene `runtime_nonce`. Der Broker meldet die zweite lebende Verbindung (`heartbeat_ack{konflikt}`), der User löst sichtbar auf (`neueSensorId` ⇒ neue ID, Label/Paar bleiben, Host-Dirty).

## 8 · Was NICHT in diesem Schema liegt

Pairingsecret (→ `Pairing`, SONDE-016, DPAPI) · Evidenz/Findings/Experimente (→ SQLite, SONDE-011) · `runtime_nonce`, `session_epoch`, `broker_epoch`, `transport_epoch` (flüchtig, §32.1) · der abgeleitete Auto-Gain-Betrag (folgt aus dem bestätigten Programm, SONDE-015 R4) · die transiente Hörmatrixauswahl Dry/Processed/Delta/Candidate (nach Laden immer Processed, SONDE-015 R10) · das **Preset** (→ `nakama-preset-v1.json`): es trägt den Klanginhalt ohne Identität, ohne `eq_enabled` und ohne Undo-Ring und ist kein Projektzustand.
