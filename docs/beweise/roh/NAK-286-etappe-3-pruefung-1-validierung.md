# NAK-286 Etappe 3 — Quellvalidierung des Befunds der Erstprüfung 1 (lesender Opus-Agent, 14.09.2026, HEAD `03a91fe6`)

Befund (Codex `gpt-6-astra` max, Thread `01a0a191-3276-7e00-9574-69fb26e7e224`, Rohdatei `NAK-286-etappe-3-pruefung-1-03a91fe6.txt`): „[P2] Weise JSON-Arrays als Anfrage zurück — broker/src/briefkasten.rs:112-113. DEFEKT gegen R-286-1/F-2."

## URTEIL: BESTÄTIGT

Der Rust-Leser liest die Anfrage über ein von serde abgeleitetes `Deserialize` ohne jede Wurzeltyp-Prüfung; `serde_json::de::deserialize_struct` leitet bei `[` in `visit_seq`, und der abgeleitete Struct-Visitor füllt `format`/`anfrage_id` positionell — `deny_unknown_fields` greift nur im Map-Zweig. Der C++-Leser verlangt explizit `{`. Damit behandeln beide Hälften denselben Inhalt verschieden.

### 1. Rust-Leser: kein Objekt-Zwang

`broker/src/briefkasten.rs:99-117`

```rust
 99  #[derive(Deserialize)]
100  #[serde(deny_unknown_fields)]
101  struct AnfrageDokument {
102      format: String,
103      anfrage_id: String,
104  }
...
108  fn anfrage_lesen(daten: &[u8]) -> Option<Kennung> {
109      if daten.starts_with(&[0xEF, 0xBB, 0xBF]) {
110          return None;
111      }
112      let dokument: AnfrageDokument = serde_json::from_slice(daten).ok()?;
113      if dokument.format != ANFRAGE_FORMAT || !kennung_gueltig(dokument.anfrage_id.as_bytes()) {
```

Geprüft: keine `Value::Object`-Prüfung, kein eigener Visitor, kein Blick auf das erste Nicht-Leerraum-Byte. Die einzige Vorprüfung ist die BOM-Prüfung (Zeile 109). Einziger Aufrufer: `briefkasten.rs:668` (`anfrage_lesen(&puffer[..gelesen]).map_or(Lesung::Fremd, Lesung::Kennung)`). Der Doc-Kommentar direkt darüber behauptet die Objektform, ohne sie zu erzwingen:

```
106  /// F-2: UTF-8 ohne BOM, ein JSON-Objekt mit genau den Schluesseln `format` und
107  /// `anfrage_id`, je einmal. Alles andere ist fremd geformt.
```

### 2. Vertrag

**F-2**, `docs/beweise/NAK-286.md:355` (wörtlich, tragender Teil):

> **F-2 Anfrageformat** (neuer Vertrag `nakama.diagnose.anfrage.v1`, Schemadatei im neuen Ordner `diagnose/` unter `eq-copilot/schemas/`): UTF-8 ohne BOM, **ein JSON-Objekt mit genau den Schlüsseln `format` (Wert `nakama.diagnose.anfrage.v1`) und `anfrage_id` (genau 32 Zeichen aus `0-9a-f`)**. […] Alles andere ist fremd geformt.

**M-27** (§4.3, C++-Hälfte), `docs/beweise/NAK-286.md:544` — Varianten und Zusage:

> Anfragevarianten: (a) 1 025 Bytes; (b) kein JSON; (c) zusätzlicher Schlüssel; (d) `format` falsch; (e) Kennung mit Großbuchstaben, mit 31 oder 33 Zeichen, mit `..`, `\`, `/` oder NUL; (f) `anfrage.json` ist ein Verzeichnis; (g) leere Datei; (h) UTF-8 mit BOM | … | keine Antwort; kein Logeintrag …

Rotbeweisspalte M-27:

> (1) Größenprüfung entfernt → (a) liest mehr als 1 024 Bytes → rot; (2) Kennungsmuster auf „nicht leer" gelockert → (e) erzeugt eine Antwort → rot

**M-43** (§4.4, Rust-Hälfte), `docs/beweise/NAK-286.md:572` — Zusage:

> dieselben Zusagen wie im Plugin: genau eine Antwort je Kennung, solange sie im Ring steht (Kennungsring F-13, §10.2 P-1); **Fremdes ignoriert ohne Wiederlesen und ohne Log**; gesperrt → nächster Takt; …

Rotbeweisspalte M-43:

> (1) Ringabfrage entfernt → für die schon beantwortete Kennung zählt die Fassade Anlegeversuche größer 0 … → rot; (2) Umbenennen mit Ersetzen (`MOVEFILE_REPLACE_EXISTING` wie `broker/src/persistenz.rs:98-101`) → der SHA-256 der Markendatei ändert sich → rot

Prüfmittelspalte M-43 (`:572`) und Prüfbarkeitszeile (`:813`) verankern die Zusage im Rust-Testfall `kennungsregeln_wie_plugin` — „dieselbe Falltabelle wie der C++-Fall", Bein A4.

**Schema** `eq-copilot/schemas/diagnose/nakama-diagnose-anfrage-v1.schema.json:6-12` — die Objektform ist normativ:

```json
  "type": "object",
  "required": ["format", "anfrage_id"],
  "additionalProperties": false,
  "properties": {
    "format": { "const": "nakama.diagnose.anfrage.v1" },
    "anfrage_id": { "type": "string", "pattern": "^[0-9a-f]{32}$", ... }
  }
```

Ein JSON-Array scheitert hier an `"type": "object"`.

### 3. C++-Leser: lehnt ein Array ab

`eq-copilot/plugin/core/diagnose/Briefkasten.cpp:185-188` — die Objektform wird erzwungen:

```cpp
185      leer();
186      if (i >= n || d[i] != '{')
187          return false;
188      ++i;
```

und geschlossen wird sie ebenso (`:236-242`):

```cpp
236      leer();
237      if (i >= n || d[i] != '}')
238          return false;
239      ++i;
240      leer();
241      if (i != n || ! habeFormat || ! habeKennung)
242          return false;
```

Die Schlüssel werden namentlich gelesen (`:210`, `:216`), unbekannte Schlüssel fallen über den `else`-Zweig `:223-226`. `["nakama.diagnose.anfrage.v1","000…0"]` scheitert bereits an `:186`. Aufrufer: `Briefkasten.cpp:654`.

### 4. serde-Verhalten (serde_json 1.0.151 laut `broker/Cargo.lock:311-312`)

`C:\Users\phili\.cargo\registry\src\index.crates.io-1949cf8c6b5b557f\serde_json-1.0.151\src\de.rs:1836-1847`:

```rust
1836         let value = match peek {
1837             b'[' => {
1838                 check_recursion! {
1839                     self.eat_char();
1840                     let ret = visitor.visit_seq(SeqAccess::new(self));
1841                 }
1842
1843                 match (ret, self.end_seq()) {
1844                     (Ok(ret), Ok(())) => Ok(ret),
1845                     (Err(err), _) | (_, Err(err)) => Err(err),
1846                 }
1847             }
```

`deserialize_struct` hat also einen vollwertigen Sequenzzweig; der abgeleitete Struct-Visitor belegt darin die Felder in Deklarationsreihenfolge (`format`, dann `anfrage_id`). `deny_unknown_fields` wirkt ausschließlich auf den Map-Zweig (`:1848-1858`) und ist im Sequenzzweig wirkungslos. Die Array-Form durchläuft damit Zeile 112 erfolgreich, `format` und `kennung_gueltig` (32 Nullen sind gültiges Hex) passieren Zeile 113 — die Anfrage wird beantwortet.

### 5. Tests: kein Fall mit Nicht-Objekt-Wurzel

`broker/src/briefkasten.rs:1339-1344` — `anfrage_und_zeitstempel_wie_plugin` prüft nur Gutfall, Leerraum und doppelten Schlüssel:

```rust
1341         assert_eq!(anfrage_lesen(anfrage_text(&k).as_bytes()).map(|x| kennung_text(&x)), Some(k.clone()));
1342         assert!(anfrage_lesen(format!(" \r\n\t{} \n", anfrage_text(&k)).as_bytes()).is_some(), "Leerraum ist erlaubt");
1343         let doppelt = format!("{{\"format\":\"{ANFRAGE_FORMAT}\",\"anfrage_id\":\"{k}\",\"anfrage_id\":\"{k}\"}}");
1344         assert!(anfrage_lesen(doppelt.as_bytes()).is_none(), "ein doppelter Schluessel ist fremd");
```

M-27-Falltabelle in `kennungsregeln_wie_plugin`, `broker/src/briefkasten.rs:1629-1644` — die Variantenliste:

```rust
1630             ("(a) 1025 Bytes", Some(gross), 0),
1631             ("(b) kein JSON", Some(b"das ist kein JSON".to_vec()), 1),
1632             ("(c) zusaetzlicher Schluessel", …"x":1}"…, 1),
1633             ("(d) format falsch", …"nakama.diagnose.anfrage.v2"…, 1),
1634-1640       ("(e) Grossbuchstaben" / "31 Zeichen" / "33 Zeichen" / "mit .." / "mit Backslash" / "mit /" / "mit NUL", …),
1641             ("(f) Verzeichnis", None, 0),
1642             ("(g) leere Datei", Some(Vec::new()), 1),
1643             ("(h) UTF-8 mit BOM", …),
```

Keine Wurzel-Variante: `(b)` ist `"das ist kein JSON"` — überhaupt kein gültiges JSON, kein Array, keine Zahl, kein String-Literal, kein `null`. Die Falltabelle steht im Modul `mod tests` ab `briefkasten.rs:946`; der Integrationstest `broker/tests/briefkasten.rs` (vom Agenten als nicht gefunden gemeldet; laut `git ls-files` vorhanden — Dirigent) trägt keine Anfragevarianten. Die C++-Falltabelle `eq-copilot/plugin/tests/BriefkastenTestMain.cpp:1172-1187` ist zeichengleich und hat dieselbe Lücke — dort deckt sie der handgeschriebene Leser ab, in Rust nicht.

### Was genau bricht

- **Vertragssatz:** F-2, `docs/beweise/NAK-286.md:355` — „ein JSON-Objekt mit genau den Schlüsseln `format` … und `anfrage_id`"; dazu `nakama-diagnose-anfrage-v1.schema.json:6` `"type": "object"` und R-286-1 „Eine unlesbare, zu große … oder fremd geformte Anfrage wird ignoriert, nicht beantwortet, nicht geloggt" (zitiert in M-27, `:544`).
- **Matrixzeile:** **M-43** (§4.4, `:572`), Zusage „dieselben Zusagen wie im Plugin … Fremdes ignoriert ohne Wiederlesen und ohne Log", Prüfmittel `kennungsregeln_wie_plugin` / Bein A4, Pflicht aus `tools/dirigent/pruefliste.md` A „Dieselbe Regel in allen Sprachen". M-27 (§4.3, C++/B30) bleibt am Code intakt — der C++-Leser lehnt ab —, teilt aber die Lücke in der Falltabelle.

### Sprachvergleich

Für denselben Byteinhalt `["nakama.diagnose.anfrage.v1","00000000000000000000000000000000"]` liefert der C++-Leser `false` (Ablehnung an `Briefkasten.cpp:186`), während der Rust-Leser eine gültige Kennung zurückgibt und der Broker antwortet — dieselbe Regel ist in den beiden Sprachen nicht dieselbe.
