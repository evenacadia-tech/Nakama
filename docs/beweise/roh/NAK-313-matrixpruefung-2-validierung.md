# NAK-313 — Validierung der Matrixprüfung 2 (W1, W2)

| Merkmal | Wert |
|---|---|
| Ticket | NAK-313, Planschritt S25l, Etappe 1 (Matrixrunde 2 von 3) |
| HEAD | `cd9c6c1c27ab249e4d61f2400272e931c30b4018` zu Beginn (24.09.2026, 01:24:47 Uhr) und vor dem Schreiben (01:41:43 Uhr), beide Werte aus `git rev-parse HEAD` und `date -Iseconds`; `git status --short` beide Male nur `?? briefing-hub/` und `?? nimbalyst-local/` |
| Datum | 24.09.2026 |
| Modell | Claude Opus 5.5 (`claude-opus-5-5`), frischer lesender Validierer; Effort nach der Startvorgabe in §14 max (in der Sitzung nicht messbar) |
| Gegenstand | Rohurteil `docs/beweise/roh/NAK-313-matrixpruefung-2-urteil.md` (Codex `gpt-6-astra` xhigh, Thread `01a0d07d-c2a3-7451-9d02-2c2647170abf`, URTEIL NEEDS_WORK): W1 = D12 offen (M-72, §7.3, §8.6), W2 = M-101 (c/d) gebrochen |
| Prüfform | Quellenlektüre und Handrechnung: Manifest `docs/beweise/NAK-313.md` am HEAD (die zitierten Zeilen 1 bis 3330 sind an `35c84a16`, `25e41849` und HEAD gleich: die beiden Folgecommits ändern im Kopf eine Zeile und hängen §13 und §14 an), Code am HEAD. Kein Bau, kein Lauf, kein Skript, kein Codex, kein FL Studio. |
| Arbeitsbaum | Produktcode seit `43549a0e` unverändert: `git diff --stat 43549a0e HEAD -- eq-copilot broker tools` leer (gemessen 01:41 Uhr). Einzige Schreibaktion dieser Sitzung: diese Datei. |

Zeilenangaben ohne Dateinamen mit führendem Doppelpunkt (`:1133`) meinen `docs/beweise/NAK-313.md`.

---

## Teil 1 — W1: D12 offen (M-72, §7.3, §8.6)

### a) Beanstandete Stellen, wörtlich

§7.3, Etappe 6 (`:1501-1507`; der Prüfer nennt `:1503-1504`):

> **Etappe 6:** Tor-Einträge für `rust_bootstrap` (M-98 bis M-102; M-104 mit `fassung` `v2`, gültig in `vertrag` und `produkt`) und die aus Etappe 5 verschobenen Bootstrap-Negativen (`3.5` und `1.5` an `schema` und `9007199254740992` an `textriegel`, jeweils in beiden Objekten; `NaN`, `Infinity`, `-Infinity`, `1e999` mit `produkt` `parser` — `serde_json` lehnt sie vor der Protokollwahl ab — und `vertrag` `textriegel`, Abweichung der Art `stufe`, §8.6; M-72, M-73, M-96); …

M-72 (`:1133`), Zusagespalte (Auszug) und Spalte „Reihenfolge und Frist“:

> `3.5` → `KeinHello`; … `3.5` ist hier ein Modultest ohne Stufenvergleich, sein Tabelleneintrag (Stufe `schema`) entsteht mit dem Tor in Etappe 6 (D12)

> Protokollwahl über den Ganzzahlhelfer statt `as_u64` (`bootstrap.rs:242`); typisierte Felder über eine Normalisierung integraler Zahlen im geprüften Wert vor `from_value`

§8.6, Tabelleneinträge (`:2596-2599`):

> `rust_bootstrap`: M-98 bis M-102 und M-104 (die v2-Hellos mit `fassung` `v2`), dazu die aus Etappe 5 verschobenen Negativen (`3.5` und `1.5` an `schema`, `9007199254740992` an `textriegel`, …

Zum Vergleich der Entscheid, auf den sich die Verschiebung stützt, E-313-11 (`:3070-3072`): „Vektoren verschieben, Tor nicht vorziehen: die Bootstrap-Einträge `1.5` (`schema`) und `9007199254740992` (`textriegel`) wandern von Etappe 5 nach Etappe 6“. `3.5` steht dort nicht; vor der Runde trug M-72 für `3.5` nur „`3.5` → `KeinHello`“ ohne Stufe (Fassung an `9a1ae133`). Die Stufe `schema` für `3.5` ist eine Aussage der Matrixnacharbeit 1.

### b) Nachmessung an der Quelle

Die Protokollwahl heute (`broker/src/transport/bootstrap.rs:230-250`):

```rust
    // Die Protokollentscheidung faellt VOR jeder Feldpruefung: v2 traegt
    // `protocol_version`, v3 traegt `protocol`. Ein v2-Hello darf hier nie
    // an v3-Regeln scheitern und umgekehrt.
    if let Some(pv) = obj.get("protocol_version").and_then(|v| v.as_u64()) {
        if pv <= crate::protokoll::PROTOKOLL_VERSION as u64 {
            return Ok((Bootstrap::V2 { roh: roh.to_string() }, ende));
        }
        return Err(BootstrapFehler::KeinHello(format!(
            "protocol_version {pv} ist keine v2-Version"
        )));
    }

    match obj.get("protocol").and_then(|v| v.as_u64()) {
        Some(3) => {}
        Some(p) => {
            return Err(BootstrapFehler::KeinHello(format!(
                "protocol {p} ist keine v3-Version"
            )))
        }
        None => return Err(BootstrapFehler::KeinHello("weder protocol noch protocol_version".into())),
    }
```

Der Parselauf heute (`bootstrap.rs:219-224`), die typisierte Übernahme (`bootstrap.rs:252-256`), das Zielfeld (`bootstrap.rs:106`):

```rust
    let roh = std::str::from_utf8(&daten[4..ende]).map_err(|_| BootstrapFehler::KeinUtf8)?;
    let wert: serde_json::Value = serde_json::from_str(roh)
        .map_err(|e| BootstrapFehler::KeinJson(e.to_string()))?;
```
```rust
        Some("control") => {
            let h: HelloControl = serde_json::from_str(roh)
                .map_err(|e| BootstrapFehler::KeinHello(e.to_string()))?;
```
```rust
    pub protocol: u32,
```

Textriegel und Schemaprüfung am Tor gibt es im Code nicht; sie sind Plan (§8.6, Schritte 2, 5 und 6). Die Engine, die Schritt 6 nutzen soll, liest heute nur P0/P1 (`broker/src/coordinator/schema.rs:422-426`):

```rust
pub(super) fn v3_nachricht_lesen_beliebig_mit_minor(payload: &[u8], schema_minor: u8) -> Option<Value> {
    crate::vertrag::textriegel_bytes(payload).ok()?;
    let wert: Value = serde_json::from_slice(payload).ok()?;
    v3_schema(schema_minor)?.gueltig(&wert).then_some(wert)
}
```

Der Plan, gegen den der Prüfer misst:

- Helfer, §8.5 (`:2231-2233`): „`pub fn ganzzahl (wert: &Value, min: i64, max: i64) -> Option<i64>` in `broker/src/vertrag.rs`: `as_i64`, sonst `as_u64` im Bereich, sonst `as_f64` endlich mit `fract() == 0.0` im Bereich; sonst `None`, nie 0.“
- Ort am Bootstrap, §8.5 (`:2264-2268`): „Bootstrap: `protocol` über den Helfer (`bootstrap.rs:242`); vor `from_value` ersetzt eine Normalisierung jede endliche, ganzzahlige Gleitkommazahl im Bereich ±(2^53−1) im geprüften Wert durch ihre Ganzzahl, damit `u32`-Felder `256.0` lesen; `protocol_version` des v2-Weges bleibt, wie er ist.“
- Reihenfolge am Tor, §8.6 (`:2469-2476`): „(4) `type` und Protokollwahl aus dem Wert; ein v2-Hello verlässt das Tor als `V2 { roh }`, das gehaltene Textriegelurteil und das v3-Schema spielen für ihn keine Rolle. (5) Für v3 wird das gehaltene Urteil angewandt: `KeinHello ("textriegel: …")`. (6) Hello-Prüfung gegen die Definition `hello_control` beziehungsweise `hello_telemetry` mit derselben Engine wie `schema.rs`: `KeinHello ("schema: …")` mit dem JSON-Zeiger der ersten Verletzung (M-99, M-105).“
- Stufenerkennung `rust_bootstrap`, §7.2 (`:1449`): „`KeinHello ("schema: …")` → `schema`; jeder andere `KeinHello` → `feldregel`“.

Der Vertrag: `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:855` `"protocol": { "type": "integer", "const": 3 },` (ebenso `hello_telemetry`, `eq-ipc-v3.schema.json:872`). A5 prüft mit `jsonschema.Draft202012Validator` (`tools/eq-copilot/pruefe_v3_vertrag.py:531`); `3.5` verletzt dort `type` und `const` an `/protocol` (gelesen, nicht gelaufen). Die Rust-Engine urteilt ebenso (`broker/src/vertrag.rs:805-808`: `"integer" => wert.as_f64().map(|f| f.is_finite() && f.fract() == 0.0)`).

Textriegel für `3.5`, lexikalisch nachgerechnet (`broker/src/vertrag.rs:240-320`): Bruch „5“ vorhanden, also nicht der Ganzzahlzweig (`vertrag.rs:242`); `dez` = (1 − 0 − 1) + 0 = 0 (`vertrag.rs:271`), innerhalb; Skala −1, die abgeschnittene Ziffer „5“ ist keine Null, also keine Ganzzahl (`vertrag.rs:291-303`); 2 signifikante Ziffern ≤ 15 (`vertrag.rs:311-317`) → `Ok`. `3.5` passiert Schritt 5; ein Hello, das Schritt 6 erreichte, fiele dort.

Kette nach Plan für `{"protocol":3.5, …}`: Schritt 3, strenger Lauf, gelingt → Schritt 4, `ganzzahl(3.5, …)`: `as_i64` und `as_u64` liefern für eine Gleitkommazahl `None`, `as_f64` liefert 3.5 mit `fract()` 0,5 → `None` → None-Zweig `bootstrap.rs:249` → `KeinHello ("weder protocol noch protocol_version")` → nach `:1449` Stufe `feldregel`. Schritte 5 und 6 laufen nicht. A4 vergleicht `produkt` (`:1418-1421`): erwartet `{ungueltig, schema}`, gemessen `{ungueltig, feldregel}` → der Tabellenfall ist rot, obwohl der Code dem Plan folgt. Die Selbstprüfung des Erzeugers lässt den Eintrag durch, weil beide Objekte gleich sind (`:1360-1367`); sie prüft „die Handschrift auf Widerspruchsfreiheit, nicht auf Wahrheit“ (`:1367-1368`).

**Reproduktion: stimmt in allen Gliedern** (Helfer liefert `None`, Protokollwahl endet mit `KeinHello`, §8.6 lässt die Wahl vor Schritt 5 und 6, §7.2 ordnet das `feldregel` zu, der A4-Fall scheitert). Genauer als der Bericht:

1. Der Bruch sitzt an der Stufe, die die Nacharbeit dem hinzugefügten Eintrag `3.5` gab, nicht an der Verschiebung selbst. E-313-11 nennt `3.5` nicht; die dort genannten Einträge sind in Etappe 6 erreichbar, wenn ihr Feld nach der Protokollwahl gelesen wird (Punkt 2).
2. Für `1.5` und `9007199254740992` nennt das Manifest am Bootstrap kein Feld (`:1494`, `:1503-1504`, `:2269-2270`, `:2597-2599`). In `protocol` hätten beide denselben Fehler: `1.5` endet wie `3.5` im None-Zweig; `9007199254740992` endet je nach Helferbereich im None-Zweig oder in `Some(p)` → „protocol … ist keine v3-Version“ (`bootstrap.rs:244-248`), beides `feldregel`. In `audio.block_size` (`eq-ipc-v3.schema.json:164`), `audio.channels` (`eq-ipc-v3.schema.json:165`) oder `host.pid` (`eq-ipc-v3.schema.json:886`) passiert Schritt 4 mit `protocol` 3; `1.5` fällt dann in Schritt 6 an `type` (`schema` in beiden Objekten), `9007199254740992` in Schritt 5 im Ganzzahlzweig des Riegels (`vertrag.rs:242-247`, „Ganzzahl ausserhalb 2^53-1“; `textriegel` in beiden Objekten, A5 ebenso über `pruefe_v3_vertrag.py:182-184`).
3. Der Grund, den der None-Zweig für `3.5` meldet, lautet „weder protocol noch protocol_version“, obwohl `protocol` vorhanden ist; für die Stufenerkennung (`:1449`) ist der Wortlaut ohne Belang.

### c) Welcher Satz bricht

- §7.2, Zuordnung (`:1418-1421`): „**Produktbeine (B10, B13, A4b, A4) vergleichen `produkt`** (Urteil, Stufe, bei eigenem Urteil `gueltig` den Wert, Wirkung)“ — zusammen mit der Stufenerkennung (`:1449`) und der Reihenfolge §8.6 (`:2469-2476`) ergibt der Plan für `3.5` die Stufe `feldregel`; die Einträge `:1133`, `:1503-1504`, `:2597-2598` verlangen `schema`.
- R-313-13 (`:3034-3042`): „jede Abweichung (zustandsabhängige Feldregel wie der Sequenzvergleich des v2-Clients, Stufenunterschiede wie `textriegel` gegen `feldregel` am flachen ACK-Leser) ist ausgewiesen, begründet und gezählt“ — `3.5` trägt einen Stufenunterschied `feldregel` gegen `schema`, der nicht ausgewiesen ist.
- R-313-14 (`:3043-3046`): „Jeder neue Eintrag hat am Ende seiner Etappe ein Bein, dessen Bauziel die gemessene Funktion übersetzt …, und einen Leser, der die verlangte Stufe schon hat“ — die Kette von `rust_bootstrap` hat ab Etappe 6 `schema` (`:1449`), ein Wert in `protocol` erreicht sie aber nicht.
- §7.1, Stand der Spezifikation (`:1371-1375`): „Abweichungen der Art `stufe` tragen … (… Bootstrap vor Etappe 6) und ab Etappe 6 die Hellos, an denen schon der strenge Lauf scheitert (`KeinJson`, §8.6)“ — die Fallklasse „`protocol` scheitert an der Protokollwahl“ fehlt.

Nicht gebrochen: R-313-7 (`:180-194`; die dort genannten Vektoren `null`, NUL-Escape und `1e-308` tragen `protocol` 3 und erreichen ihre Stufen), E-313-11 (Tor nicht vorgezogen, `3.5` dort nicht genannt), R-313-6 und NAK-310 M-77 (ein Wert je Hello), die Invarianten aus `CLAUDE.md` (kein Audiopfad, keine State- oder Vertragsversion, Identität unberührt).

### d) Urteil und Einordnung

**PRÄZISIERT** (Reproduktion vollständig richtig; Ort und Reichweite genauer, siehe b, Punkte 1 bis 3). **DEFEKT**: D12 ist für den Eintrag `3.5` nicht geschlossen, und die geänderten Sätze M-72, §7.3 und §8.6 widersprechen §7.2, §8.6 und R-313-13 (Frage 1 und Frage 2 des Auftrags).

Ausschlüsse (`docs/beweise/roh/NAK-313-matrixpruefung-2-auftrag.txt:31`): keiner greift. Es geht um die inhaltliche Zusage eines geänderten Tabelleneintrags (keine Prosa, kein Zeilenverweis), nicht um U65, Etappe 8 oder B-1 bis B-8, nicht um Sicherheit am Tor (reine Stufenzuordnung der Vertragsprüfung, NAK-310 unberührt), nicht um Sabotage an Prüfwerkzeugen, nicht um `tools/dirigent/pruefliste.md`, nicht um Kosmetik. Die Stellen liegen im Prüfbereich: §12.1 (`:3163`, `:3172`), §12.2 (`:3256`).

### e) Schließende Regel

Ein `rust_bootstrap`-Eintrag trägt in `produkt` die Stufe, an der `bootstrap_lesen` nach der Reihenfolge von §8.6 für genau sein Feld ablehnt: ein `protocol`, das die Protokollwahl (Schritt 4, Ganzzahlhelfer) nicht als 3 liest, endet an `feldregel` und steht mit ausgewiesener, gezählter Abweichung der Art `stufe` gegen `vertrag` `schema` (Muster M-60, `:1121`); Einträge, die in beiden Objekten `schema` oder `textriegel` tragen sollen, liegen in einem Feld, das erst nach Schritt 4 gelesen wird.

Folgestellen mit derselben Annahme: M-72 (`:1133`, Zusage- und Rotbeweisspalte); §7.3 (`:1494-1497`, `:1501-1507`); §8.5 (`:2268-2272`, Feld der Bootstrap-Negativen); §8.6 (`:2469-2480`, ein Satz zu Schritt 4, und `:2596-2601`); §7.1 (`:1371-1375`, Stand der Abweichungen); M-73 (`:1134`, Klammer „am Bootstrap erst mit dem Tor in Etappe 6“ ohne Feld); die Kopfzähler `anzahl_abweichungen` und `anzahl_je_eingang` (§7.1, `:1338-1339`). Nicht betroffen: M-96 (`:1157`; `NaN`, `Infinity`, `-Infinity`, `1e999` scheitern vor Schritt 4 am strengen Lauf, ihre Abweichung ist ausgewiesen), M-98 bis M-102 und M-104 (tragen `protocol` 3 beziehungsweise `protocol_version` 2).

---

## Teil 1 — W2: M-101 (c/d) gebrochen

### a) Beanstandete Zeile, wörtlich

M-101 (`:1186`), Zusagespalte:

> **(a) `KeinHello ("textriegel: Zahl ausserhalb +/-1e308: …")` (`broker/src/vertrag.rs:271-272`; Regel 3, Untergrenze), Stufe `textriegel`; (b), (c), (d) `V3Control`; (e) `KeinHello ("schema: …")`, Stufe `schema` (`maximum` 768000). Die Grenze ist die des C++-Writers: `audioGueltig` (`eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:231-260`) liefert für (a) und (e) false, für (b), (c), (d) true. `1e-300` bleibt nur benannt (NAK-387 Beobachtung 1)**

Vektoren: (a) `1e-308`, (b) `1e-307`, (c) `1e-300`, (d) `48000`, (e) `768000.5`. Rotbeweisspalte (Auszug):

> (a): **heute rot** …; nach dem Bau: das gehaltene Textriegelurteil nicht anwenden → angenommen → rot. (e): Urteil **Regressionswache** …, Stufe **heute rot** …; Einzelbruch: die Schemaprüfung überspringen → wieder der Feldgrund → Stufenprüfung rot. (b): **Regressionswache**; Einzelbruch: die Dezimalgrenze des Riegels um eins enger (`dez <= -307`) → `1e-307` fällt → rot. (c), (d): **Regressionswache**, getragen von denselben Brüchen. C++-Writer: **Regressionswache** (die Grenzen stehen heute in `Vertrag.cpp:251-252`); Einzelbruch: `samplerate <= 768000.0` weglassen → `768000.5` true → rot

Der Satz „(c), (d): **Regressionswache**, getragen von denselben Brüchen.“ ist neu in `35c84a16`; vorher stand „(b) bis (e): **Regressionswache**“ ohne Bruch.

### b) Nachmessung an der Quelle

Rust-Riegel, Ganzzahlzweig und Betragsgrenze (`broker/src/vertrag.rs:240-249`, `vertrag.rs:270-274`; `DEZ_GRENZE` = 308, `vertrag.rs:130`):

```rust
fn zahl_pruefen(ganz: &str, bruch: &str, exp_ziffern: &str, exp_negativ: bool,
                lit: &str, schema_ganzzahl_sichern: bool) -> Result<(), String> {
    if bruch.is_empty() && exp_ziffern.is_empty() {
        let zu_gross = ganz.len() > 16
            || (ganz.len() == 16 && ganz.parse::<u64>().unwrap_or(u64::MAX) > SICHERE_GANZZAHL);
        if zu_gross {
            return Err(format!("Ganzzahl ausserhalb 2^53-1: {lit}"));
        }
        return Ok(());
    }
```
```rust
    let fuehrende = (alle.len() - signifikant.len()) as i64;
    let dez = (ganz.len() as i64 - fuehrende - 1) + exp;
    if dez >= DEZ_GRENZE || dez <= -DEZ_GRENZE {
        return Err(format!("Zahl ausserhalb +/-1e{DEZ_GRENZE}: {}", kurz(lit)));
    }
```

Zweite Wand am Tor (`broker/src/transport/bootstrap.rs:265-272`), bleibt nach §8.6 Schritt 7 (`:2477-2478`):

```rust
            if !(h.audio.samplerate.is_finite()
                && h.audio.samplerate > 0.0
                && h.audio.samplerate <= 768_000.0
                && (1..=65_536).contains(&h.audio.block_size)
                && h.audio.channels <= 64)
            {
                return Err(BootstrapFehler::KeinHello("audio ausserhalb des Vertrags".into()));
            }
```

Schema (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:163`): `"samplerate": { "type": "number", "exclusiveMinimum": 0, "maximum": 768000 },`; Rust-Engine `"number" => wert.is_number(),` (`broker/src/vertrag.rs:802`), Grenzen `vertrag.rs:917-925`.

C++-Writer (`eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:251-254`):

```cpp
        return std::isfinite (samplerate) && samplerate > 0.0 && samplerate <= 768000.0
            && nakama::wire::wireZahl (samplerate, verworfen)
            && blockSize >= 1 && blockSize <= 65536
            && channels >= 0 && channels <= 64;
```

`audioGueltig` rechnet die Riegelgrenze über `nakama::wire::wireZahl` (`eq-copilot/plugin/core/ipc/WireZahl.h:195-197`, `WireZahl.h:209-211`), nicht über `zahlPruefen` in `eq-copilot/plugin/vertrag/NakamaVertrag.cpp` (Ganzzahlzweig `NakamaVertrag.cpp:328-338`, Betragsgrenze `NakamaVertrag.cpp:364-369`; das ist der C++-Riegel des Vertragswegs B3c/B16 und das C++-Bein von M-121):

```cpp
    const bool ganzzahlig = (betrag == std::floor (betrag));
    if (ganzzahlig && betrag > kSichereGanzzahl)
        return false;
```
```cpp
    const int dez = n - 1;
    if (dez >= kDezGrenze || dez <= -kDezGrenze)
        return false;
```

Die „C++-Obergrenze“ der Zeile ist `samplerate <= 768000.0` in `controlclient/Vertrag.cpp:251`, nicht in `NakamaVertrag.cpp`. Bestehende Writerwache N-18d (`eq-copilot/plugin/tests/SchemaTestMain.cpp:256-265`): `1e-308` und `5e-324` false, `48000.0` und `44100.5` true.

Nachgerechnet:

- `1e-300`: Exponent vorhanden, also nicht der Ganzzahlzweig; `dez` = (1 − 0 − 1) + (−300) = −300; `dez <= -308` falsch → passiert. Unter `dez <= -307`: −300 ≤ −307 falsch → passiert weiter.
- `48000`: weder Bruch noch Exponent → Ganzzahlzweig `vertrag.rs:242-248`, 5 Stellen ≤ 16 → `Ok`, bevor `dez` gerechnet wird.
- `1e-307` zur Gegenprobe: `dez` = −307; unter `dez <= -307` wahr → fällt. Der Bruch trägt (b).

| Genannter Bruch | (c) `1e-300` am Tor | (d) `48000` am Tor | (c), (d) am Writer |
|---|---|---|---|
| (a): Textriegelurteil nicht anwenden | `V3Control` (passiert den Riegel ohnehin) | `V3Control` | nicht betroffen |
| (e): Schemaprüfung überspringen | `V3Control` (zweite Wand: endlich, größer 0, bis 768 000) | `V3Control` | nicht betroffen |
| (b): `dez <= -307` im Riegel | `V3Control` (`dez` −300) | `V3Control` (Ganzzahlzweig) | `wireZahl` nicht betroffen |
| Writer: `samplerate <= 768000.0` weglassen | nicht betroffen | nicht betroffen | beide weiter true |

**Reproduktion: stimmt vollständig.** Keiner der vier genannten Brüche ändert die Beobachtung von (c) oder (d), weder am Tor noch am Writer.

### c) Welcher Satz bricht

- R-313-11 (`:3022-3027`): „… eine Mutation, die die Beobachtung nachgerechnet nicht ändert, ist kein Rotbeweis.“
- R-313-12 (`:3028-3033`): „… jede Wache hat ihren Einzelbruch dort, wo sie nach dem Bau allein trägt.“
- Etikettendefinition §6 (`:903-905`): „**Regressionswache**: heute grün und hält, was der Bau nicht ändern darf; sie wird einmal absichtlich gebrochen und ist kein Beleg“; §8.1 (`:1710-1711`): „Für **heute nicht messbar** und **Regressionswache** ist die Mutation nach dem Bau der Beleg.“

Nicht gebrochen: die Zusage selbst. Tor und Writer nehmen `1e-300` und `48000` an, R-313-7 Satz 1 („die Samplerategrenze des Bootstraps ist die des C++-Writers“) gilt für beide Werte; es fehlt nur der Beleg.

### d) Urteil und Einordnung

**BESTÄTIGT. DEFEKT**: Der in der Runde neu geschriebene Satz der Zeile M-101 widerspricht R-313-11 und R-313-12 (Frage 2 des Auftrags; M-101 steht in §12.2 als geändert, `:3285`).

Ausschlüsse: keiner greift. Der verlangte Einzelbruch ist nach §8.1 der Beleg der Wache selbst, kein Sabotage-Szenario gegen ein Prüfwerkzeug; es geht um die inhaltliche Rotbeweisaussage einer geänderten Zeile, nicht um Prosa oder Zeilenverweise.

### e) Schließende Regel

Jede Zusagehälfte von M-101, die als Regressionswache geführt wird, nennt je Seite (Tor `bootstrap_lesen`, C++-Writer `audioGueltig`) einen Einzelbruch, dessen Kette Mutation → Vektor → Beobachtung am Code nachgerechnet die Beobachtung genau dieses Werts ändert; ein Wert ohne eigene Zusage (hier `1e-300`, NAK-387 Beobachtung 1) steht als benannt ohne Etikett.

Folgestellen derselben Annahme „ein Bruch trägt mehrere Werte oder beide Seiten“, vom Prüfer nicht erhoben:

- M-101, Writerhälfte zu (b): Der Bruch „die Dezimalgrenze des Riegels um eins enger (`dez <= -307`)“ nennt keine Sprache. `audioGueltig` rechnet über `WireZahl.h:210`, nicht über `vertrag.rs:272`; für den Writer trägt der Bruch nur, wenn er dort gesetzt wird (`wireZahl(1e-307)`: `n` = −306, `dez` = −307 → false → `audioGueltig` false → der neue B3c-Fall „`1e-307` true“ rot).
- M-101, Writerhälfte zu (a): `1e-308` false steht in N-18d (`SchemaTestMain.cpp:256-259`), in M-101 ohne Bruch. Wirksam wäre `WireZahl.h:210` `<=` → `<`: `dez` = −308 passiert, `audioGueltig(1e-308)` wird true → N-18d rot.
- Unverändert richtig: §6.7 (`:1302`, „1e-300 nur benannt M-101“), §8.6 (`:2480-2484`, „`1e-300` passiert beide und bleibt NAK-387s Beobachtung 1“), M-121 (`:1206`, eigener Bruch je Sprache an der Riegelgrenze).

---

## Teil 2 — Entscheidungsgrundlagen

### 1. Zu W1: Reihenfolge am Tor, Stufe von `protocol` 3.5, Empfehlung

**1.1 Reihenfolge nach §8.6 gegen den Code**

| Schritt nach §8.6 (`:2458-2478`) | heute im Code | Stelle |
|---|---|---|
| 1 Länge, Präfix, UTF-8 | vorhanden | `bootstrap.rs:205-219` |
| 2 Textriegelurteil rechnen und halten | fehlt | Plan |
| 3 ein strenger Lauf, Fehler `KeinJson` | `serde_json::from_str`, zweiter Lauf in der Übernahme | `bootstrap.rs:220-224`, `bootstrap.rs:254`, `bootstrap.rs:276` |
| 4 `type` und Protokollwahl aus dem Wert | `type` `bootstrap.rs:226-228`, v2-Weiche `bootstrap.rs:233-240`, v3-Wahl `bootstrap.rs:242-250` (heute `as_u64`, nach Plan der Helfer) | `bootstrap.rs:226-250` |
| 5 gehaltenes Textriegelurteil für v3 | fehlt | Plan |
| 6 Hello-Prüfung am v3-Schema | fehlt | Plan |
| 7 Normalisierung, typisierte Übernahme, zweite Wand | Übernahme aus dem Rohtext, zweite Wand vorhanden | `bootstrap.rs:252-272` |

Die Protokollwahl steht damit nach dem Parselauf und vor Textriegel und Schema, im Code wie im Plan.

**1.2 Welche Stufe `protocol` 3.5 erreicht**

- **(a) Protokollwahl vor dem Schema mit Ganzzahlhelfer (Plan):** Schritt 4 → `None` → `KeinHello ("weder protocol noch protocol_version")` (`bootstrap.rs:249`) → `produkt` `feldregel`; `vertrag` `schema`. Abweichung der Art `stufe`, auszuweisen.
- **(b) Protokollwahl nach der Schemaprüfung:** Schritt 4 wird zur reinen v2-Weiche, danach Schritt 5 (`3.5` passiert, siehe Teil 1 b) und Schritt 6 (`/protocol` verletzt `type` und `const`, `eq-ipc-v3.schema.json:855`) → `KeinHello ("schema: …")` → `produkt` `schema` gleich `vertrag`. Der v2-Schutz („Ein v2-Hello darf hier nie an v3-Regeln scheitern und umgekehrt“, `bootstrap.rs:231-232`; R-313-7 „v2-Hellos scheitern nie an v3-Regeln“) hält nur, wenn die Weiche am **Vorhandensein** von `protocol_version` entscheidet, nicht am Erfolg von `as_u64` (`bootstrap.rs:233`). Sonst fiele ein v2-gültiges `"protocol_version":2.0` (v2-Schema `eq-copilot/schemas/eq-ipc.schema.json:24`: `{ "type": "integer", "const": 2 }`; `2.0` ist unter draft 2020-12 eine Ganzzahl) in die v3-Kette und scheiterte am v3-Schema (`hello_control` verlangt `protocol` und ist `additionalProperties: false`, `eq-ipc-v3.schema.json:850-851`). Heute endet dieses Hello am None-Zweig `bootstrap.rs:249`, also ohne v3-Regel.
- **(c) Protokollwahl auf dem Rohwert mit dem Helfer:** ergibt 3 den v3-Weg; ein nicht ganzzahliger Wert muss einen der beiden Wege nehmen. Ablehnen in Schritt 4 ist Form (a) (`feldregel`), Weitergeben an Schritt 5 und 6 ist Form (b) (`schema`). Eine dritte Form gibt es nicht. v2 wird ohnehin nicht über `protocol` 2 gewählt, sondern über den Schlüssel `protocol_version` (`bootstrap.rs:233`; v2-Schema `eq-ipc.schema.json:20`, `eq-ipc.schema.json:24`).

**1.3 R-313-7, E-313-11, A5**

R-313-7 (`:180-194`) hält in (a) und in (b), in (b) nur mit der Weiche nach Vorhandensein: `null` (M-99), NUL-Escape (M-100) und `1e-308` (M-101 a) tragen `protocol` 3, passieren Schritt 4 in jeder Form und fallen an `schema` beziehungsweise `textriegel`; die Samplerategrenze bleibt die zweite Wand. E-313-11 (`:3070-3074`) hält in jeder Form, denn keine zieht das Tor vor Etappe 6. Die Wahl entscheidet allein R-313-13: in (a) muss die Abweichung ausgewiesen und gezählt werden, in (b) entsteht keine. A5 ist die Vertragsreferenz und urteilt in jeder Form gleich: `3.5` ist ungültig an `schema`, `/protocol` verletzt `type` und `const` (`eq-ipc-v3.schema.json:855`).

**1.4 Produktwirkung**

Keine Produktwirkung. Der eigene Writer sendet `"protocol":3` (`eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:145`). Ein fremdes `3.5` wird in jeder Form abgelehnt. Die Ablehnung geht als `bootstrap: {e:?}` an den Client (`broker/src/transport/server_v3/verbindung.rs:265-270`), und Gen ordnet sie nach Etappe 6 unabhängig vom Grundtext als `incompatible` ein (Beobachtung B-8, `:1670-1673`; Teilwörter in `eq-copilot/plugin/src/SourcesModel.cpp:339-345`). Die Entscheidung ist rein technisch.

**1.5 Empfehlung: Weg 1, die Abweichung ausweisen**

Der Tabelleneintrag `3.5` in `protocol` bekommt `produkt` `{ungueltig, feldregel}`, `vertrag` `{ungueltig, schema}` und `abweichung` `{stufe, „Protokollwahl vor Textriegel und Schema (§8.6 Schritt 4) über den Ganzzahlhelfer, bootstrap.rs:242-250“}` (der Grund nennt Produktregel und Codestelle, `:1352`). Das entspricht der Torreihenfolge in §8.6, die auch der Prüfer beibehält (Vorschlag in `urteil.md:12`), und dem Muster M-60 für denselben Wert am C++-Welcome (`:1121`: „Stufen für `4`, `3.5`: `produkt` `feldregel`, `vertrag` `schema` … — Abweichung der Art `stufe`“). Der Korpus hält damit genau das, wofür R-313-13 die Form A geschaffen hat. Zusätzlich werden `1.5` und `9007199254740992` in ein Feld nach der Protokollwahl gelegt, etwa `audio.block_size`.

Der Vorschlag im Auftrag „samplerate 1.5 am Schema“ trägt nicht. `1.5` ist als Samplerate überall gültig: Schema `eq-ipc-v3.schema.json:163` (`number`, größer 0, bis 768 000), zweite Wand `bootstrap.rs:265-269` und Writer (`audioGueltig`: endlich, größer 0, bis 768 000, `wireZahl` liefert „1.5“ mit `dez` 0). Der Eintrag wäre in beiden Objekten `gueltig`. Ein Vektor, der in beiden Ketten `schema` erreicht, ist eine Nichtganzzahl in einem Ganzzahlfeld nach Schritt 4, etwa `audio.block_size` `1.5`. Das ist bereits der `1.5`-Eintrag aus E-313-11, sobald sein Feld feststeht.

Nicht empfohlen ist Form (b). Sie ändert die Torreihenfolge in §8.6 und den Kommentar `bootstrap.rs:230-232`, verlangt eine neue Regel für die v2-Weiche und verschiebt die Gründe für Hellos ohne Protokollschlüssel oder mit `protocol` ungleich 3 von `feldregel` zu `schema`. Keine Regel verlangt das, und für das Produkt ändert sich nichts.

| Stelle | Weg 1: Abweichung ausweisen (empfohlen) | Weg 2: anderer Tabellenvektor, `3.5` nur Modultest |
|---|---|---|
| M-72 `:1133` | „sein Tabelleneintrag (`produkt` `feldregel` aus der Protokollwahl, `vertrag` `schema`, Abweichung der Art `stufe`) entsteht mit dem Tor in Etappe 6“; Rotbeweis des Eintrags: im Helfer `fract() == 0` weglassen → Schritt 4 liest 3 → Schritt 5 passt → Schritt 6 verletzt `/protocol` → `KeinHello ("schema: …")` → Stufe `schema` statt `feldregel` → Tabellenfall rot (unabhängig von der Normalisierung) | kein Tabelleneintrag für `3.5`; der Modultest braucht ab Etappe 6 eine Grund- oder Stufenbeobachtung (1.6), sonst hat seine Wache keinen wirksamen Einzelbruch |
| M-73 `:1134` | Klammer „am Bootstrap erst mit dem Tor in Etappe 6“ um das Feld der Negativen ergänzen | ebenso |
| M-96 `:1157` | unberührt | unberührt |
| §7.3 `:1501-1507` | `3.5` in `protocol` mit `feldregel` gegen `schema` (Abweichung); `1.5` an `schema` und `9007199254740992` an `textriegel` in einem Feld nach der Protokollwahl, jeweils in beiden Objekten | `3.5` aus der Liste; `1.5` und `9007199254740992` mit Feld nach der Protokollwahl |
| §7.1 `:1371-1375` | Stand der Abweichungen um „am Bootstrap ab Etappe 6 ein `protocol`, das die Protokollwahl nicht als 3 liest“ ergänzen | unverändert |
| §8.5 `:2268-2272` | Feld der Negativen nennen; die Modultests der Etappe 5 bleiben | ebenso |
| §8.6 `:2469-2480` | ein Satz zu Schritt 4: ein `protocol`, das der Helfer nicht als 3 liest, endet dort (`feldregel`); Reihenfolge unverändert | ebenso |
| §8.6 `:2596-2601` | wie §7.3 | wie §7.3 |
| Zählprüfung | `anzahl_abweichungen` Art `stufe` plus 1; `anzahl_je_eingang` für `rust_bootstrap` unverändert | `anzahl_je_eingang` für `rust_bootstrap` minus 1; `anzahl_abweichungen` unverändert |
| Produktcode | wie geplant | wie geplant |

**1.6 Bedingung für den Modultest `3.5` in M-72 (beide Wege, vom Prüfer nicht erhoben)**

Etappe 5: Der Einzelbruch „im Helfer `fract() == 0` weglassen → `3.5` wird 3 → `V3Control` → rot“ (`:1133`) trägt nur, wenn die Normalisierung vor `from_value` denselben Helfer ruft. Prüft sie die Ganzzahligkeit selbst, bleibt `protocol` die Gleitkommazahl 3,5, `from_value` scheitert am Zielfeld `protocol: u32` (`bootstrap.rs:106`), das Urteil bleibt `KeinHello`, und der Bruch ist kein Rotbeweis. §8.5 (`:2264-2268`) legt das nicht fest.

Ab Etappe 6 tragen zwei Wände das Urteil `KeinHello`: Schritt 4 (Helfer) und Schritt 6 (Schema an `/protocol`). Kein einzelner Bruch kippt das Urteil. Wirksam ist nur ein Bruch an Stufe oder Grund. Das spricht für Weg 1: Der Tabelleneintrag vergleicht die Stufe ohnehin.

### 2. Zu W2: M-101 vollständig, wirksame Einzelbrüche, Trennung

**2.1 Die Zusagehälften von M-101**

| Wert | Seite | Etikett laut Zeile | genannter Bruch | trägt |
|---|---|---|---|---|
| (a) `1e-308` | Tor | heute rot | Textriegelurteil nicht anwenden → Schema und zweite Wand nehmen an → rot | ja |
| (a) `1e-308` | Writer | Wache, Sammelsatz „C++-Writer“ | nur `samplerate <= 768000.0` weglassen | nein, `wireZahl` lehnt weiter ab |
| (b) `1e-307` | Tor | Wache | `dez <= -307` in `vertrag.rs:272` → `dez` −307 fällt → rot | ja |
| (b) `1e-307` | Writer | Wache | derselbe Satz ohne Sprachangabe | nur, wenn an `WireZahl.h:210` gesetzt |
| (c) `1e-300` | Tor und Writer | Wache, „getragen von denselben Brüchen“ | keiner trägt (Teil 1 b) | nein |
| (d) `48000` | Tor und Writer | Wache, „getragen von denselben Brüchen“ | keiner trägt (Teil 1 b) | nein |
| (e) `768000.5` | Tor | Urteil Wache, Stufe heute rot | Schemaprüfung überspringen → zweite Wand, `feldregel` → Stufenprüfung rot | ja, für den Tabellenfall |
| (e) `768000.5` | Writer | Wache | `samplerate <= 768000.0` weglassen → true → rot | ja |

**2.2 Kandidaten, nachgerechnet nach R-313-11**

- **Ganzzahlzweig entfernen** (`vertrag.rs:242-249`): kein Rotbeweis. `48000` läuft dann durch den allgemeinen Weg: kein Exponent, also `exp` = 0; `dez` = (5 − 0 − 1) + 0 = 4, innerhalb; Skala 0, 5 Stellen, also keine Ganzzahl über der Grenze (`vertrag.rs:280-290`); 2 signifikante Ziffern → `Ok` → `V3Control` bleibt. Am Writer ebenso: ohne den Ganzzahlzweig (`WireZahl.h:195-197`) läuft `48000` über den gedeckelten Weg, `zerlegen` liefert Ziffern „48“ mit `n` = 5, `dez` = 4, der Rückweg ergibt 48000 ≤ 2^53−1 (`WireZahl.h:216-222`) → `wireZahl` true.
- **Exponentuntergrenze auf −300** (`vertrag.rs:272`, genau `dez <= -300`; `dez <= -301` ließe −300 passieren): `1e-300` fällt mit `dez` −300 an Schritt 5 → `KeinHello ("textriegel: Zahl ausserhalb +/-1e308: 1e-300")` → (c) rot. Mit rot werden (b) und der M-121-Fall „`1e-307` gültig“ im Rust-Bein. Wirksam, aber das ist die Riegelgrenze von M-121 und (b).
- **Samplerate-Untergrenze der zweiten Wand auf `1e-299`** (`bootstrap.rs:266`, `> 0.0` → `> 1e-299`): `1e-300` passiert Schritt 5 (`dez` −300) und Schritt 6 (größer 0, bis 768 000) und fällt in Schritt 7 → `KeinHello ("audio ausserhalb des Vertrags")` → (c) rot. `audioGueltig(1e-300)` bleibt true, der Bruch verletzt also genau „Grenze des Tors gleich Grenze des Writers“. (b) wird mit rot. Das Writergegenstück ist `Vertrag.cpp:251` `samplerate > 0.0` → `> 1e-299`: `audioGueltig(1e-300)` false → der neue B3c-Fall „`1e-300` true“ rot; der Fall `1e-307` wird mit rot.
- **Obergrenze in der falschen Einheit, 768 statt 768 000** (für (d)): Am Tor `bootstrap.rs:267` `<= 768_000.0` → `<= 768.0`. `48000` passiert Schritt 5 (Ganzzahlzweig) und Schritt 6 (bis 768 000) und fällt in Schritt 7 → `KeinHello ("audio ausserhalb des Vertrags")` → (d) rot. (b) und (c) bleiben unter 768 und angenommen, (a) fällt weiter am Riegel, (e) weiter am Schema. Unter den Werten von M-101 wird damit nur (d) rot. Am Writer `Vertrag.cpp:251` `samplerate <= 768000.0` → `<= 768.0`: `audioGueltig(48000.0)` false → N-18d `SchemaTestMain.cpp:262-263` rot (mit rot wird `SchemaTestMain.cpp:264-265`, `44100.5`); `1e-307` und `1e-300` bleiben true. Dieser Bruch ist für (d) exklusiv und hängt nur an Code, der heute steht und nach §8.6 Schritt 7 bleibt.
- **Typregel `number` der Rust-Engine** (für (d) am Tor, zweite Wahl): `vertrag.rs:802` `wert.is_number()` → `wert.is_f64()`. `48000` ist eine Ganzzahl-`Number` → Verletzung an `/audio/samplerate` `type` in Schritt 6 → `KeinHello ("schema: …")` → rot; die Gleitkommawerte (b), (c), (e) bleiben unberührt. Die Kette setzt voraus, dass der strenge Lauf aus Etappe 4 Ganzzahlliterale als Ganzzahl-`Number` hält wie heute `serde_json::from_str` (`bootstrap.rs:220`). Dieser Parser ist noch nicht gebaut; deshalb nur zweite Wahl.

**2.3 Trennung**

- **M-121** (`:1206`) trägt die Riegelgrenze als Vertragstext und als vier Fälle der Falltabelle direkt an der Grenze (`1e-307` gültig; `1e-308`, `2e-308`, `9.9e-308` ungültig), je Sprache mit eigenem Bruch. Keine Hälfte von M-101 gehört dorthin: M-101 misst nicht den Riegel, sondern die Gleichheit von Tor und Writer (R-313-7 Satz 1) je Wert.
- **M-101 (a), (b)** liegen an der unteren Kante, weil die Untergrenze beider Seiten nach Etappe 6 der Riegel ist (Rust `vertrag.rs:272`, Writer `WireZahl.h:210`). Sie gehören zu M-101, brauchen aber je Seite ihren Bruch (Teil 1 e).
- **M-101 (c) `1e-300`** ist keine Kante. Es ist NAK-387 Beobachtung 1 (`docs/offene-punkte.md:601`: „`audio_lage.samplerate` im Bootstrap hat keine sinnvolle Untergrenze (`1e-300` passiert den Textriegel)“; Quelle Teil B `docs/beweise/roh/NAK-313-quellvalidierung-teil-B.md:776-778`, „hier nur benannt“). Jede monotone Verschärfung, die `1e-300` ablehnt, lehnt auch `1e-307` ab, weil `1e-307` kleiner ist, und macht (b) rot. (c) hat damit keinen Ort, „wo sie nach dem Bau allein trägt“ (R-313-12).
- **M-101 (d) `48000`** ist der gewöhnliche Wert. Er steckt auch in jedem gültigen Control-Hello der Bootstraptests (`bootstrap.rs:460`, `bootstrap.rs:473`) und in N-18d. Einen exklusiven Bruch hat er nur über die Einheitenprobe an der Obergrenze.

**2.4 Empfehlung**

(d) bleibt Wache mit dem Einheitenbruch an beiden Seiten (`bootstrap.rs:267` und `Vertrag.cpp:251`, je `<= 768.0`), mit der Kette aus 2.2. (c) verliert das Etikett: „(c)“ entfällt aus „(b), (c), (d) `V3Control`“ und aus dem Writersatz, „`1e-300` bleibt nur benannt (NAK-387 Beobachtung 1)“ bleibt. Der Vektor darf als Tabelleneintrag `gueltig`/`gueltig` stehen bleiben; ihn tragen Tabellenlauf und Zählprüfung (`:1421-1424`). Behält der Dirigent (c) als Wache, sind die Brüche `bootstrap.rs:266` und `Vertrag.cpp:251`, je `> 1e-299`, wirksam; die Zeile sagt dann, dass beide auch (b) rot machen. Die Writerhälften von (a) und (b) bekommen ihren Bruch an `WireZahl.h:210` (Teil 1 e).

### 3. Umfang des Prüfers und die dreizehn geschlossenen Befunde

- **Nichts außerhalb der zwei Fragen.** W1 betrifft D12 aus der Befundliste (`docs/beweise/roh/NAK-313-matrixpruefung-2-auftrag.txt:23`, Frage 1) und die dabei geänderten Sätze M-72, §7.3 und §8.6 (Frage 2). W2 betrifft die in der Durchsicht geänderte Zeile M-101 (§12.2 `:3285`, Frage 2). Der Bericht sagt „keine zusätzlichen Lücken oder Härtungen erhoben“ (`docs/beweise/roh/NAK-313-matrixpruefung-2-urteil.md:7`); das trifft zu.
- **Die dreizehn.** Der Bericht enthält zu ihnen genau einen Satzteil: „die übrigen dreizehn Befunde geschlossen“ (`urteil.md:7`), ohne Vorbehalt. Es fehlen die verlangten Zeilen je Befund („je Befund eine Zeile „D<n>: geschlossen/offen — Beleg““), die Absätze „geprüft / nicht geprüft“ und die Schlusszeile „FERTIG Matrixprüfung 2, …“ (`auftrag.txt:42-43`). Die erste Zeile hat nicht die Form „URTEIL: NEEDS_WORK — …“ (`auftrag.txt:38-41`). Die Schließung von D1 bis D11, D13 und D14 ist damit behauptet, aber nicht je Befund belegt. §14 übernimmt sie als „13 von 14 Befunden geschlossen“ (`:3368-3370`). Beide HEAD-Werte und die Angabe „Ausschließlich statisch geprüft, keine Testläufe; der angeforderte Produktdiff ist leer“ stehen im Bericht (`urteil.md:7`); der leere Produktdiff deckt sich mit meiner Messung am HEAD. Eigene Wiederprüfung der dreizehn: nicht gemacht (Auftrag).

---

## Übersicht

| Befund | Urteil | Einordnung | schließende Regel in einem Satz | Folgestellen |
|---|---|---|---|---|
| W1 — D12, `protocol` 3.5 am Bootstrap (M-72, §7.3, §8.6) | PRÄZISIERT | DEFEKT | Ein `rust_bootstrap`-Eintrag trägt in `produkt` die Stufe, an der `bootstrap_lesen` nach §8.6 für sein Feld ablehnt: ein `protocol`, das die Protokollwahl nicht als 3 liest, endet an `feldregel` mit ausgewiesener, gezählter Abweichung `stufe` gegen `vertrag` `schema` (Muster M-60), und Einträge mit `schema` oder `textriegel` in beiden Objekten liegen in einem Feld nach Schritt 4. | M-72 `:1133`; §7.3 `:1494-1497`, `:1501-1507`; §8.5 `:2268-2272`; §8.6 `:2469-2480`, `:2596-2601`; §7.1 `:1371-1375`, `:1338-1339`; M-73 `:1134`; Bedingung des Modultests 1.6 |
| W2 — M-101 (c) `1e-300` und (d) `48000` ohne wirksamen Einzelbruch | BESTÄTIGT | DEFEKT | Jede Zusagehälfte von M-101, die als Regressionswache geführt wird, nennt je Seite (Tor, C++-Writer) einen Einzelbruch, dessen Kette am Code nachgerechnet die Beobachtung genau dieses Werts ändert, und ein Wert ohne eigene Zusage steht als benannt ohne Etikett. | M-101 `:1186` Writerhälften zu (a) und (b) (`WireZahl.h:210`); unverändert richtig §6.7 `:1302`, §8.6 `:2480-2484`, M-121 `:1206` |

## Nicht geprüft

- Kein Bau, kein Testlauf, kein `cargo`, kein A5 und kein `jsonschema`-Lauf. Alle Ketten sind von Hand aus dem Quelltext gerechnet. Das Verhalten von `serde_json` (`as_i64` und `as_u64` auf einer Gleitkommazahl `None`, `from_value` in `u32` aus einer Gleitkommazahl ein Fehler, Ganzzahlliterale als Ganzzahl-`Number`, `1e999` ein Parsefehler) stammt aus der Crate-Semantik, nicht aus einem Lauf.
- Der geplante Code (strenger Lauf, Helfer `ganzzahl`, Normalisierung, Torschritte 2, 5 und 6) existiert nicht. Die Ketten gelten gegen den Plantext §8.5 und §8.6 zusammen mit dem heutigen Code.
- Keine eigene Wiederprüfung von D1 bis D11, D13 und D14; keine weiteren Matrixzeilen außer den genannten Folgestellen.
- Der Telemetry-Hello-Weg (`bootstrap.rs:275-284`) ist nicht gesondert verfolgt; §8.6 gibt ihm dieselbe Reihenfolge, und das Schema trägt dort dasselbe `protocol` (`eq-ipc-v3.schema.json:872`).
- `tools/plan/dokuriegel.py` ist über diese Datei nicht gelaufen (kein Skript nach Auftrag).

FERTIG Validierung Matrixprüfung 2, 1 bestätigt, 1 präzisiert, 0 widerlegt
