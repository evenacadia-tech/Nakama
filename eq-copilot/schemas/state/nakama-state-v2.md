# State-Schema 2 — `NakamaState` (SONDE-006)

- **Stand:** 2026-08-22 · **Ticket:** `SONDE-006` (S7) · **Vertragstext:** Entwurf §53.8, §33.5, §32.1/32.2, §53.5, §67
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
│     label                string   User-Wort, untrusted, nie interpretiert (auch leer)
│     pair_id              string   NUR wenn nicht leer (fehlt = kein Paar)
│     project_binding_id   string   NUR wenn bestätigt gebunden (hex32); wird NIE still erzeugt (§32.2)
├── MainProject                           schema = 1  (int)   nur plugin_kind = main (Pflicht dort)
│     (heute keine weiteren Eigenschaften; Intent/Mitgliedschaft/Passage/AssistantStep/Outbox ergänzen P3/P5 ADDITIV)
├── Parameters                            schema = 1  (int)   nur plugin_kind = active_probe (Pflicht dort)
│     109 Eigenschaften mit den IDs aus nakama-parameter-v1.json, in Vertragsreihenfolge
│     bool → bool · float → double (bit-exakt) · enum → string (Enumwort)
├── Dsp                                   NAME RESERVIERT — Inhalt definiert SONDE-015 (Revision, Schutz, bestätigter DspState, Undo-Ring)
└── Pairing                               NAME RESERVIERT — Inhalt definiert SONDE-016 (Ziel-IDs, DPAPI-Blobs; nie Klartext, nie im Hash)
```

Reihenfolge beim Schreiben: für Stände, die **dieser Schreiber** erzeugt (frisch, migriert), Kinder Common, MainProject, Parameters und Eigenschaften in der Reihenfolge oben. Ein **geladener** Stand behält seine eigene Reihenfolge (der Schreiber editiert eine Kopie des gehaltenen Baums in place — unbekannte Eigenschaften eingeschlossen); fehlt einem geladenen Common das `label`, bekommt es beim Speichern eines. Damit ist `speichere(lade(x)) == x` **bytegleich** für jeden Stand, den dieser Schreiber selbst geschrieben hat (Goldens, Roundtrip-Test), und die Schema-1-Migration ist **deterministisch** (Golden-fähig).

### 2.1 Kind-Matrix (§53.8: „Unzulässige Ziel-/Kindkombinationen werden nicht teilweise interpretiert")

| `plugin_kind` | Common | MainProject | Parameters | Dsp | Pairing |
|---|---|---|---|---|---|
| `main` | Pflicht | **Pflicht** | verboten | verboten | ab SONDE-016 (heute: nicht lesbar ⇒ read-only) |
| `legacy` | Pflicht | verboten | verboten | verboten | verboten |
| `passive_probe` | Pflicht | verboten | verboten | verboten | verboten |
| `active_probe` | Pflicht | verboten | **Pflicht** | ab SONDE-015 (heute: nicht lesbar ⇒ read-only) | ab SONDE-016 (heute: nicht lesbar ⇒ read-only) |

Ein Kind mit unbekanntem Namen ⇒ read-only (ein neues Kind ist eine Root-Versionierung, keine Minor-Erweiterung). Eine **unbekannte Eigenschaft** in einem bekannten Kind desselben Majors ⇒ additiv, wird erhalten.

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

- DTO-Form: `{"dsp_schema_version":1,"parameters":{<id>:<Wert>,…}}` — genau die 109 IDs, physikalische Werte.
- Validierung **vor** dem Hash (Reihenfolge fest, erster Fehler zählt): Textstufe doppelter Schlüssel → Typ/Struktur → `dsp_schema_version` → unbekannter Schlüssel → fehlender Schlüssel → Typ je Parameter → nichtendlich → Bereich/Enum.
- Kanon: **RFC 8785** (Schlüssel nach UTF-16-Code-Units sortiert, rekursiv; Zahlen nach ECMA-262 `Number::toString`; Strings mit `\b \t \n \f \r \" \\` und sonst `\u00xx` klein; kein Whitespace; UTF-8).
- `state_hash` = SHA-256-Hex (64 Kleinbuchstaben) des kanonischen UTF-8 — das Format, das v3 `state_report.state_hash` verlangt.
- Drei Beine gegen dieselben Fixtures (`fixtures/state/jcs/`, `fixtures/state/dto/`): C++ (`NakamaKanon`), Python (`rfc8785` 0.1.4, Trail of Bits), Rust (`serde_json_canonicalizer` 0.3.x + `sha2`). Die Zahlenvektoren aus RFC 8785 Anhang B stehen mit dem **vom RFC gedruckten** Erwartungstext im Erzeuger — die Python-Referenz wird damit selbst gegen den RFC gemessen, nicht nur gegen sich.
- **Der Kanon liest JSON selbst** (`kanon::lies`, `std::from_chars`), nicht über `juce::JSON`. Gemessen am Korpus (22.08.): JUCEs `readDoubleValue` liest `5e-324` und `0.000000000000000000000000001` als **0** (Subnormal-Flush, 18-Stellen-Puffer) und verweigert den leeren Schlüssel `""` (v3-Textriegel Regel 7 dokumentiert das für den IPC-Pfad). Für einen Hash, den drei Sprachen auf demselben Text bilden, darf der Zahlenleser nicht „ungefähr" sein — *ein Riegel befragt nie die Bibliothek, gegen die er schützt.* Der eigene Leser lehnt außerdem doppelte Schlüssel, einsame Surrogate, rohe Steuerzeichen, NUL und nicht darstellbare Zahlen (`1e999`) als Fehler ab statt still zu runden.

## 5 · Unbekanntes Major, read-only (§53.8, §33.5)

Tritt ein, wenn: Root-`schema` ≠ 2 (oder `EqCopilotState` mit `schema` ≠ 1) · `Common` fehlt oder `Common.schema` ≠ 1 · unbekanntes Enumwort · Klasse nicht im Bundle · Kind-Matrix verletzt · unbekanntes Kind · `Dsp`/`Pairing` vorhanden, solange dieser Build sie nicht liest.

Verhalten: **audio-neutral** (Passthrough wie immer) · **read-only** (`setzeBindung`/`neueSensorId` werden verweigert, kein Host-Dirty) · `getStateInformation` liefert die **Originalbytes unverändert** zurück (nie ein Teilstate) · keine Pipe-Anmeldung (es gibt keine vertrauenswürdige Identität zu melden) · Editor zeigt den Zustand (Anzeige-Pflicht „Capability-Degradation", §0.4).

Fremder Baumtyp oder Müllbytes: Zustand bleibt wie vor dem Aufruf (heutiges Verhalten, IdentityTest „Muellbytes aendern den Zustand nicht").

## 6 · Host-Dirty (§32.2, §67 Punkt 4)

Jede persistente Änderung aus dem Plugin heraus (`setzeBindung` mit echter Änderung, `neueSensorId`, später: bestätigte Bindung) meldet `updateHostDisplay (ChangeDetails().withNonParameterStateChanged (true))` — der JUCE-VST3-Wrapper setzt daraus `IComponentHandler2::setDirty (true)`. **Laden und Migration melden nicht** (ein Projekt, das der User nicht speichert, bleibt Schema 1 und migriert beim nächsten Laden wieder — nichts geht verloren). Ohne echte Änderung: keine Meldung, kein Reconnect.

## 7 · Duplicate (§32.1 „Duplikate werden sichtbar aufgelöst")

Zwei Instanzen aus denselben Bytes tragen dieselbe `instance_id` (Absicht: der State IST der Messpunkt) und verschiedene `runtime_nonce`. Der Broker meldet die zweite lebende Verbindung (`heartbeat_ack{konflikt}`), der User löst sichtbar auf (`neueSensorId` ⇒ neue ID, Label/Paar bleiben, Host-Dirty).

## 8 · Was NICHT in diesem Schema liegt

Undo-Ring, `state_revision`, bestätigter DspState, Schutzgrenzen (→ `Dsp`, SONDE-015) · Pairingsecret (→ `Pairing`, SONDE-016, DPAPI) · Evidenz/Findings/Experimente (→ SQLite, SONDE-011) · `runtime_nonce`, `session_epoch`, `broker_epoch`, `transport_epoch` (flüchtig, §32.1).
