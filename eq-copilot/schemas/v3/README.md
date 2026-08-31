# v3-Vertragsbaum (SONDE-005a)

Dieses Verzeichnis ist der **Vertrag**, nicht die Implementierung. Drei
Sprachen lesen dieselben Dateien; wo sie sich uneinig sind, ist eine von ihnen
falsch — nie der Vertrag.

| Datei | Was |
|---|---|
| `eq-ipc-v3.schema.json` | Die 17 definierten JSON-Nachrichtenfamilien (P0/P1) |
| `reservierte-nachrichten-v1.json` | Die 9 reservierten Familiennamen, reservierte Feldnamen + ihr Eigentuemerticket |
| `quantisierung-v1.json` | Kodierung der Bandwerte + 61 Testvektoren |
| `bandgitter/nakama_1_24_oct_30_18k_v1.json` | 221 Baender (Evidenz, 1–4 Hz) |
| `bandgitter/nakama_log64_v1.json` | 64 Gruppen (Live, 10 Hz) |

## Wer diesen Vertrag prueft

| Bein | Wo | Prueft |
|---|---|---|
| Referenz | `tools/eq-copilot/pruefe_v3_vertrag.py` (`jsonschema` 4.26, draft 2020-12) | Urteil |
| C++ | `EqCopSchemaTest` (`plugin/vertrag/`) | Urteil **und** Verletzungsmenge |
| Rust | `broker/tests/contract_cross_language.rs` | Urteil **und** Verletzungsmenge |

Alle drei messen gegen **dasselbe** `eq-copilot/fixtures/v3/MANIFEST.json`.
Das Manifest ist die dritte Partei: stimmen C++ und Rust je mit ihm ueberein,
stimmen sie transitiv miteinander ueberein — ohne dass zwei Ausgaben
gegeneinander diffiert werden muessen.

Das Referenzbein prueft nur das **Urteil**, nicht die Verletzungsmenge: eine
echte JSON-Schema-Implementierung meldet bei `oneOf` die Fehler **aller**
Zweige, waehrend die beiden eigenen Engines ueber den Discriminator in genau
**einen** Zweig absteigen (siehe unten). Beide Verhalten sind korrekt; nur die
Fehlerlisten sind nicht vergleichbar. Das steht hier, damit niemand spaeter
versucht, sie doch gleichzuziehen.

## Der Textriegel — die Stufe VOR dem Parser

Seit T2-Runde 1 (21.08.) hat der Vertrag eine Stufe, die kein JSON-Schema
ausdruecken kann: eine Pruefung des **Rohtexts**, bevor ihn ein Parser sieht.

Der Anlass war gemessen, nicht befuerchtet. JUCEs `parseNumber`
(`juce_JSON.cpp`) akkumuliert `intValue * 10 + digit` in einem `int64` **ohne
Bereichspruefung** und nimmt fuehrende Nullen an. Konkret:

| Eingabe | C++ (JUCE) | Rust (serde_json) | Python |
|---|---|---|---|
| `18446744073709552016` | liest **400** | 1.8446744e19 | exakt |
| `091` | liest **91** | Parsefehler | Parsefehler |
| `10000000000000000000` | ungueltig (Minimum) | gueltig | gueltig |

Das ist kein Randfall der Fehlermeldung, sondern ein **umgeklappter Wert**:
`sequence`, `state_revision`, `transport_epoch` und `base_revision` sind
Monotonie- und Revisionsfelder.

**Warum kein `maximum` je Feld:** der Wert ist beim Ankommen bereits
verfaelscht. Ein `maximum: 400` wuerde auf der C++-Seite anstandslos passieren
und auf der Rust-Seite fallen. Der einzige Ort, an dem alle drei Beine
dasselbe sehen koennen, ist der Text.

**Neun Regeln**, jede gegen eine **gemessene** Abweichung:

1. **Keine fuehrende Null** — RFC 8259 verbietet sie, JUCE nimmt sie an.
2. **Parser-sichere Dezimalpraezision:** mathematische Ganzzahlen nur innerhalb
   ±(2^53−1), unabhaengig davon, ob sie als `5`, `5.0`, `5e0` oder mit
   verschobenen Endnullen geschrieben sind. Nichtganzzahlige Werte tragen
   hoechstens 15 signifikante Dezimalziffern. Andernfalls kann binary64 den
   Nachkommateil verlieren und ein `type: integer` faelschlich passieren.
3. **Zahlen betragsmaessig unter 1e308, und ein `e` braucht Ziffern.**
4. **Genau vier ASCII-Hexziffern** in einem `\u`-Escape. Vorher hatten die drei
   Beine drei Hex-Grammatiken: Pythons `int(roh, 16)` nahm `+123`, `` 12 ``,
   `0x1f`, `1_23` und arabisch-indische Ziffern, Rusts `from_str_radix` das
   Vorzeichen, die C++-Handschleife nichts davon.
5. **Kein NUL-Escape in einer Zeichenkette** — `juce::String` ist
   nullterminiert und bricht dort im Parser ab, waehrend `serde_json` und
   Python das Dokument annehmen.
6. **Keine einsamen Surrogate** — hier lehnen *beide eigenen* Engines ab und
   nur das Referenzbein nimmt an; die Regel zieht es nach.
7. **Kein leerer Objektschluessel** — JUCE lehnt ihn im Parser ab; in einem
   **additiven** Objekt (`zaehler`, `konfidenz`, `verteilung`) haette
   `serde_json` ihn dagegen angenommen, weil `additionalProperties: true` ihn
   nicht auffaengt.
8. **Nur die alphabetischen JSON-Literale `true`, `false` und `null`.** Jedes
   andere ASCII-Buchstabentoken ausserhalb einer Zeichenkette wird abgelehnt,
   einschliesslich Pythons nicht-standardisierter Erweiterungen `NaN`,
   `Infinity` und `-Infinity`.
9. **Auf Byteebene: höchstens 16 MiB, kein BOM, kein rohes NUL, gültiges
   UTF-8.** Der produktive Pipe-Framer ist mit 256 KiB enger; 16 MiB begrenzen
   zusätzlich direkte Datei-/DTO-Aufrufer in allen drei Sprachen. Bei einem BOM
   streift JUCE es und parst weiter, rohes NUL beendet dort die Prüfung am
   gültigen Präfix, und bei kaputtem UTF-8 liefen alle drei auseinander
   (ungefangene Ausnahme · Panik beim Lesen · stille Ersetzung).

Auch der **Ziffernbegriff** ist jetzt derselbe: `str.isdigit()` ist fuer
arabisch-indische Ziffern und Hochzahlen wahr, `is_ascii_digit` und
`c >= '0' && c <= '9'` nicht. Bei `{"w": 0٢}` meldete das Referenzbein deshalb
eine fuehrende Null und die anderen beiden nicht.

### Die teuerste Lehre dieses Vertrags

Regel 3 war in der ersten Fassung **delegiert** — die drei Beine fragten
`float(lit)`, `lit.parse::<f64>()` bzw. `getDoubleValue()`. Auf der C++-Seite
ist das **genau der Leser, gegen dessen Ueberlauf der Riegel schuetzen soll**.
`juce_CharacterFunctions.h` akkumuliert den Exponenten in einem `int` ohne
Schranke und prueft erst **danach** gegen `max_exponent10`; ein zweiter Zweig
(`extraExponent`) prueft gar nicht und schreibt immer genau drei
Exponentziffern. Gemessen in T2-Runde 2:

| Eingabe | C++ (JUCE) | Rust | Python |
|---|---|---|---|
| `1e4294967296` | liest **1.0** | inf | inf |
| `1` + 1017 Nullen + `.0` | liest **1e17** | inf | inf |
| `1e` | liest **1.0** | Parsefehler | Parsefehler |

> **Ein Riegel darf nie die Bibliothek befragen, gegen deren Verhalten er
> schuetzt.**

Regel 2 pruefte anfangs nur Literale ohne Punkt und Exponent. Damit passierten
mathematisch identische Formen wie `9007199254740992.0`,
`90071992547409920e-1` und `0.9007199254740992e16` den Riegel. Sie werden nun
exakt als `Ziffern * 10^Skala` normalisiert: bei negativer Skala ist der Wert
nur dann ganzzahlig, wenn die abzuschneidenden Stellen ausschliesslich Nullen
sind; erst danach erfolgt der Laengen-/Lexikvergleich gegen 2^53−1. Regel 3
rechnet den Dezimalexponenten ebenfalls aus Vorkommastellen, fuehrenden Nullen
und explizitem Exponenten. Ein zweiter gemessener Bypass waren lexikalisch
nichtganzzahlige Werte wie `9007199254740991.1`: der Nachkommateil ging beim
binary64-Lesen verloren, sodass die Schema-Engines ihn als Integer sahen;
Rust und Python rundeten dabei im Produktionspfad sogar auf verschiedene
Ganzzahlen. Darum kappt Regel 2 echte Brueche auf 15 signifikante
Dezimalziffern; fuehrende und wertgleiche abschliessende Nullen zaehlen dabei
nicht. Beide Regeln verwenden nur kleine ganze Zahlen und keine
Gleitkommaoperation.

Diese 15-Ziffern-Sicherung gehoert zum **v3-Schemaweg**, dessen drei Engines
den Zahlenwert vor der Typpruefung als binary64 lesen. Der DSP-DTO-/JCS-Weg
nutzt dieselben Byte-, Grammatik-, Endlichkeits- und Ganzzahlregeln, besitzt
danach aber einen eigenen korrekt gerundeten Zahlenleser mit feldgenauer
Typ-/Bereichspruefung. Dort bleibt deshalb z. B. der vertragliche Wert
`0.7071067811865476` zulaessig; eine globale Kappung wuerde echte DSP-Daten
veraendern, ohne den Schema-Bug zu beheben.

### Die Falltabelle ist eine Datei, keine drei Kopien

Die Faelle stehen in **`../../fixtures/v3/TEXTRIEGEL-FAELLE.json`** und werden
von allen drei Beinen **gelesen**. Vorher trug jedes Bein eine eigene Kopie —
gezaehlt **31, 32 und 33** Faelle —, waehrend das Beweismanifest „dieselbe
31-Faelle-Tabelle" behauptete (T2-Runde 2, Befund BF-5). Drei handgepflegte
Kopien driften; eine gelesene Datei kann es nicht.

Der Text steht dort **hex-kodiert**: die Tabelle enthaelt NUL-Escapes, rohe
Steuerzeichen, kaputtes UTF-8 und ein BOM — Inhalte, an denen ein JSON-Leser
oder eine Zwischenschicht sich verschluckt. `zeigetext` ist nur fuer Menschen
da und wird von keinem Bein gelesen.

Der Riegel laeuft ueber **jedes** Fixture, nicht nur ueber die, die an ihm
fallen sollen — sonst waere seine zweite Haelfte („laesst alles andere durch")
ungeprueft.

Fixtures, die an dieser Stufe fallen, tragen im Manifest
`textriegel_lehnt_ab: true` und **keine** Verletzungsmenge: sie erreichen das
Schema nie, und eine erfundene Menge waere eine Luege ueber den Ort der
Ablehnung.

## Die Engine-Teilmenge

Beide eigenen Engines implementieren **genau** diese Schluesselwoerter:

```
$ref (nur lokal, "#/$defs/<name>")   type   const   enum
required   properties   additionalProperties (nur true|false)   maxProperties
minimum   maximum   exclusiveMinimum   exclusiveMaximum
minLength   maxLength   pattern (nur aus der Tabelle unten)
items   minItems   maxItems
oneOf (nur mit x-nakama-discriminator)
```

Reine Anmerkungen, die uebergangen werden: `$schema`, `$id`, `title`,
`description`, `$comment`, `x-nakama-discriminator` (ausser an seinem
`oneOf`).

**Ein Schluesselwort ausserhalb dieser Liste bricht den Ladevorgang** — die
Engine verweigert das Schema, statt es zu ignorieren. Das ist die wichtigste
Eigenschaft dieses Entwurfs: JSON Schema **ignoriert unbekannte
Schluesselwoerter absichtlich**. Traegt jemand spaeter ein `multipleOf` ein,
wuerde das Referenzbein es durchsetzen und die beiden eigenen Engines wuerden
es stillschweigend uebergehen — die Einschraenkung verschwaende auf zwei von
drei Seiten, und nur ein Fixture, das zufaellig genau darauf zielt, haette es
gefunden. So faellt es beim ersten Lauf auf.

Seit T2-Runde 1 prueft der Ladelauf zwei Dinge mehr, beide aus demselben
Grund und beide gegen einen gemessenen Fall:

* **Der WERTTYP jedes Schluesselworts.** Bis dahin sah die Pruefung nur
  Namen. Gemessen: `"maxLength": 5.0` wurde vom Rust-Bein still verworfen
  (`as_u64()` auf einer Float-Zahl ist `None`) und vom C++-Bein durchgesetzt
  (`static_cast<int>`) — dieselbe Fehlerklasse wie ein unbekanntes
  Schluesselwort, nur eine Ebene tiefer. Laengen und Anzahlen muessen
  **ganzzahlig und nicht negativ** sein, `type` ist String **oder**
  nicht-leeres String-Array, `required` ein String-Array, `enum`/`oneOf`
  nicht leer.
* **Dass jedes `$ref` ein ZIEL hat.** Bis dahin wurde nur das Praefix
  `#/$defs/` geprueft; `aufloesen()` fiel bei fehlendem Ziel auf den
  Originalknoten zurueck, und damit blieb der ganze Teilbaum **still
  ungeprueft**. Das ist dieselbe Klasse wie das `multipleOf`-Argument oben,
  nur schlimmer: es verschwindet nicht *eine* Einschraenkung, sondern *jede*
  des Zieles.

### `type`

`integer` folgt draft 2020-12: **jede Zahl ohne Nachkommateil** ist ein
Integer, auch `1.0`. `number` schliesst Integer ein. `null` ist ein eigener
Typ, nicht „fehlt". Faellt `type`, wird **nicht** in den Wert abgestiegen —
sonst haengen an einem falschen Typ lawinenartig Folgefehler, und die beiden
Engines muessten sich ueber deren Reihenfolge einigen.

### `pattern` — geschlossene Tabelle

Regex ist zwischen Python `re`, C++ und Rust nicht in jeder Ecke gleich.
Deshalb gibt es keine allgemeine Regex-Auswertung, sondern eine **Tabelle
bekannter Muster**; ein unbekanntes Muster bricht den Ladevorgang wie ein
unbekanntes Schluesselwort.

| Muster | Bedeutung |
|---|---|
| `^[0-9a-f]{32}$` | genau 32 Kleinbuchstaben-Hexziffern |

`pruefe_v3_vertrag.py` prueft, dass jedes im Schema vorkommende `pattern` in
dieser Tabelle steht.

### `oneOf` und der Discriminator

`oneOf` ist nur zusammen mit `x-nakama-discriminator` erlaubt. Der Wert ist
entweder der Name einer direkten Eigenschaft oder ein RFC-6901-Pointer mit
fuehrendem Slash durch verschachtelte Objekte (einschliesslich `~0`/`~1`-
Escapes). Jeder Zweig muss am Ziel ein `const` festlegen. Der
Discriminatorwert darf String **oder Boolean** sein; genau diese beiden Typen
werden fuer die Transportunionen (`time_basis`, `bounds_valid` und die beiden
Validity-Bits) gebraucht. Die Engine liest den Wert aus der Instanz, waehlt
den einen passenden Zweig und steigt nur dort ab. Passt keiner, fehlt ein
Pointersegment oder hat der Wert einen anderen Typ, ist das **eine**
Verletzung mit Schluessel `oneOf`.

Zwei Feinheiten, beide von der Cross-Language-Pruefung erzwungen:

* **Ein Zweig darf selbst eine diskriminierte Union sein.** `hello` ist genau
  das: es haelt keine eigenen `properties`, sondern zwei Varianten, die beide
  `type: "hello"` festlegen und sich erst in `connection_kind` unterscheiden.
  Die Suche nach dem `const` steigt deshalb durch geschachtelte `oneOf`
  hindurch und nimmt den Wert nur an, wenn **alle** Untervarianten denselben
  festlegen — sonst waere die Zuordnung mehrdeutig. Ohne diesen Abstieg faende
  der aeussere Discriminator die Familie ueberhaupt nicht.
* **Ist die Instanz gar kein Objekt**, zeigt die Verletzung auf die Instanz
  selbst (`""` an der Wurzel) statt auf `/<disc>`. Ein `/type` an einem
  blossen String waere ein Pfad, den es dort nicht gibt.
* **Der gewaehlte Zweig ersetzt nicht seine Geschwisterregeln.** Nach der
  Zweigpruefung werden `required`, `properties`, Grenzen und weitere
  diskriminierte Unionen desselben Knotens ebenfalls ausgewertet. Sonst
  wuerde etwa die innere `bounds_valid`-Union die gemeinsamen Cycle-Regeln
  verschlucken.

Damit sind zwei Dinge erreicht: die Verletzungsmenge ist endlich und
deterministisch, und ein **reservierter oder unbekannter Discriminator wird
abgelehnt** statt gegen alle Zweige gehalten (Entwurf §33.1: „Discriminator,
Zieladresse, Revision, Capability und sicherheitsrelevante Felder sind nicht
additiv und erzwingen bei unbekannter Bedeutung Ablehnung").

Eine echte JSON-Schema-Implementierung kommt beim **Urteil** zum selben
Ergebnis, weil genau ein Zweig den passenden `const` traegt.

### Verletzungen

Eine Verletzung ist ein Tripel:

```
{ "instanz": "/pfad/zum/wert", "schema": "#/$defs/x/properties/y/type", "schluessel": "type" }
```

* `instanz` ist ein JSON-Pointer in die geprueften Daten; `""` ist die Wurzel.
* `schema` ist der **aufgeloeste** Pfad — ueber `$ref` hinweg, damit beide
  Engines denselben Text bilden.
* Fehlende Pflichteigenschaften erzeugen **je eine** Verletzung mit
  `schema` = `.../required/<name>` und `instanz` = Pfad des ELTERNobjekts
  (ein fehlender Wert hat keinen eigenen Pfad).
* Unbekannte Eigenschaften erzeugen je eine Verletzung mit
  `schluessel` = `additionalProperties` und `instanz` = Pfad der Eigenschaft.

Die Menge wird kanonisch sortiert (`instanz`, dann `schema`, dann
`schluessel`), damit ein Vergleich nicht von der Auswertungsreihenfolge
abhaengt. Beide Engines melden **alle** Verletzungen des gewaehlten Zweiges,
nicht nur die erste.

## Eine gemessene Abweichung zwischen den Beinen

Der Korpus hat sie gefunden, und sie bleibt hier stehen, statt weggeraeumt zu
werden:

**Ein JSON-Dokument, dessen Wurzel weder Objekt noch Array ist** — etwa der
blosse String `"heartbeat"` — wird von den drei Beinen an verschiedenen
Stellen abgelehnt. `juce::JSON::parse` folgt RFC 4627 (2006) und verlangt `{`
oder `[` am Anfang; `serde_json` und Pythons `json` folgen RFC 8259 (2014) und
nehmen jeden Wert als vollstaendiges Dokument an. Das C++-Bein lehnt also
schon im **Parser** ab, die anderen beiden erst am **Schema**.

Fuer das Produkt ist das kein Riss: beide Wege sind eine Ablehnung, und ein
v3-Payload ist per Rahmen immer ein Objekt mit Discriminator. Vergleichbar
sind aber nur die Urteile, nicht die Verletzungsmengen. Das Manifest
kennzeichnet solche Fixtures mit `wurzel_skalar: true`, und das C++-Bein
prueft dort ausdruecklich, dass der Parser zurueckweist.

## Additiv oder strikt

Entwurf §33.1 verlangt beide Politiken und Contracttests dafuer.

* **Strikt** (`additionalProperties: false`) ist die Vorgabe. Alles, was
  Discriminator, Zieladresse, Revision, Capability oder Sicherheit traegt,
  ist strikt.
* **Additiv** (`additionalProperties: true` **plus** `maxProperties`) ist die
  Ausnahme und heute genau dreimal vergeben: `zaehler`, `konfidenz` und
  `verteilung`. Ohne `maxProperties` waere „erweiterbar" gleichbedeutend mit
  „beliebig gross" — ein unbegrenzter Eingang.

`maxProperties` ist jeweils die Zahl der **deklarierten** Eigenschaften plus
acht — also acht unbekannte Eigenschaften Luft, an jedem der drei Objekte
gleich viel. T2-Runde 1 hat gemessen, dass `konfidenz` sich nicht daran hielt
(6 deklariert, aber `maxProperties: 10` — das waren nur vier); das Schema ist
nachgezogen, und `pruefe_v3_vertrag.py` rechnet die Regel jetzt nach, statt
sie nur hier zu behaupten.

## Capabilities — und warum sie keine eigene Version tragen

`capabilities` ist seit T2-Runde 1 der Satz aus Entwurf §53.6, **woertlich und
vollstaendig**: alle zehn Namen, keiner umbenannt, keiner zusammengelegt.
Vorher standen dort sieben — zwei still umbenannt, fuenf fehlend, zwei selbst
erfunden. Der Schaden daran ist nicht die Zahl, sondern dass §53.6 jeder
Capability einen **eigenen Beweis und einen eigenen Fallback** gibt: die drei
Aux-Capabilities haben drei verschiedene („kein lokales Audio-Delta" / „keine
dynamische Aktuation" / „nur Assoziation statt exakter Attribution"). Ein
zusammengelegtes `discrete_aux_buses` haette genau die Unterscheidung
geloescht, fuer die sie getrennt sind.

Entwurf §33.3 verlangt in jeder steuernden Nachricht „Schema-/
Capability-Version". Der `steuerkopf` traegt `schema_major`/`schema_minor` und
**kein** eigenes Capabilityfeld — mit Absicht: der Capabilitysatz ist
geschlossen (`additionalProperties: false`) und alle zehn sind Pflicht. Damit
ist jede Aenderung an ihm ein Major-Schritt, und `schema_major` IST seine
Version. Ein zweiter Zaehler daneben koennte nur auseinanderlaufen.

Was daraus folgt, und was der Schliessungsvorbehalt weiter unten meint: kommt
aus den FL-Terminen eine elfte Faehigkeit, ist das **keine** Minor-Erweiterung
(NAK-27 haelt den ersten solchen Fall fest). Was S4 liefert, sind **Werte**;
die Namen stehen jetzt.

## Was hier NICHT gebaut ist

Damit niemand danach sucht:

| Fehlt | Eigentuemer | Warum nicht hier |
|---|---|---|
| 16-Byte-Envelope + CRC32C-Parser | `SONDE-010` | Der Entwurf gibt den Parser ausdruecklich diesem Ticket (§65). Fixtures ohne Implementierung waeren toter Ballast. |
| FlatBuffers-`FeatureBatch`, `flatc`-Pinning, Codegen-Drift | `SONDE-005b` (S6) | Zweite Haelfte desselben Entwurfstickets, eigener Sessionschnitt. |
| RFC-8785-Kanonisierung und `state_hash` | `SONDE-006` (S7) | Wird dort erstmals verbraucht. Die ES6-Zahlenserialisierung ist eine eigene Beweisflaeche und gehoert nicht als Anhaengsel hierher. |
| Die 9 reservierten Nachrichtenfamilien und reservierten Feldnamen | siehe `reservierte-nachrichten-v1.json` | Ihre Nutzlasten sind ihren spaeteren Eigentuemertickets zugewiesen; bis dahin lehnt der aktive strikte Vertrag sie ab. |
| `evidence_snapshot.ereignisse` | `SONDE-013` (P4, §39.1) | §33.2 zaehlt „Ereignisse" als Inhalt des Snapshots auf. Der `DynamicsEvent`-Strom entsteht aber erst mit dem Dynamik-/Experimentkern; ihn hier zu erfinden waere der Vorgriff, den Bauaufteilung §6.2 verbietet. Der **Feldname** ist in `reservierte-nachrichten-v1.json` verbrannt, damit ihn kein anderes Ticket belegt, und `additionalProperties: false` lehnt ihn heute ab. |
| Die Domain-Objekte aus §34.1 | `SONDE-012` / `SONDE-014` | §65 nennt „v3-**Domain**-/JSON-/FlatBuffers-Schemas". Von den elf Kernobjekten aus §34.1 ist hier genau eines als Schema da (`probe_descriptor`), weil nur es ueber IPC laeuft. `Evidence`, `Passage`, `SourceIntent`, `Finding`, `AssistantStep` und `FeatureFrame` sind Store-/Domainobjekte; sie entstehen mit ihren Tickets. Der Sessionschnitt (`bauaufteilung-sonden.md` §3) gibt `SONDE-005a` ausdruecklich nur „v3-JSON-Schemas + Bandgitter + Fixtures". |
| Der Herkunftstag `host\|local_ui\|remote_transaction\|state_restore` (§33.4) | `SONDE-006` (S7) | Er beschreibt, WOHER eine Parameteraenderung kam, und lebt im Plugin-State, nicht in einer IPC-Nachricht. |

## Schliessungsvorbehalt

Entwurf §65: „`SONDE-005` darf parallel zu den Hostmessungen **vorbereitet**,
aber erst nach deren Capabilityentscheidung **geschlossen** werden."

Der Vertrag ist deshalb so gebaut, dass die Hostantwort seine **Form** nicht
mehr aendern kann: `capability_wert` kennt `supported` und `unsupported` und
sonst nichts (§54: „Kein `unknown, spaeter pruefen` darf P1 passieren"), und
jedes hostabhaengige Feld haengt an einem eigenen Gueltigkeitsbit — seit
T2-Runde 1 auch `continuous_time_samples` und die beiden
Presentation-Latencies, die vorher keins hatten. Was S4 liefert, sind
**Werte**, keine Felder.

**Berichtigt in T2-Runde 1:** hier stand, ein spaeter dazukommendes
Capabilitybit waere „eine Minor-Erweiterung mit eigenem Fixture". Das ist
**gemessen falsch** — `capabilities` ist strikt und alle zehn sind Pflicht,
ein elftes Bit bricht also jeden bestehenden Consumer. Der Satz ist ersetzt
durch den Abschnitt „Capabilities — und warum sie keine eigene Version
tragen" weiter oben; der erste konkrete Fall steht als **NAK-27** im
Offen-Set. Die Lehre daraus ist allgemeiner als dieser eine Satz: eine
Vertragsdatei darf ueber ihre eigene Erweiterbarkeit nur behaupten, was ein
Fixture belegt.
