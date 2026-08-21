# v3-Vertragsbaum (SONDE-005a)

Dieses Verzeichnis ist der **Vertrag**, nicht die Implementierung. Drei
Sprachen lesen dieselben Dateien; wo sie sich uneinig sind, ist eine von ihnen
falsch — nie der Vertrag.

| Datei | Was |
|---|---|
| `eq-ipc-v3.schema.json` | Die 17 definierten JSON-Nachrichtenfamilien (P0/P1) |
| `reservierte-nachrichten-v1.json` | Die 8 reservierten Namen + ihr Eigentuemerticket |
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
der Name einer Eigenschaft; jeder Zweig muss fuer sie ein `const` festlegen.
Die Engine liest den Wert aus der Instanz, waehlt den einen passenden Zweig
und steigt nur dort ab. Passt keiner — oder fehlt die Eigenschaft, oder ist
sie kein String — ist das **eine** Verletzung mit Schluessel `oneOf`.

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

## Additiv oder strikt

Entwurf §33.1 verlangt beide Politiken und Contracttests dafuer.

* **Strikt** (`additionalProperties: false`) ist die Vorgabe. Alles, was
  Discriminator, Zieladresse, Revision, Capability oder Sicherheit traegt,
  ist strikt.
* **Additiv** (`additionalProperties: true` **plus** `maxProperties`) ist die
  Ausnahme und heute genau dreimal vergeben: `zaehler`, `konfidenz` und
  `verteilung`. Ohne `maxProperties` waere „erweiterbar" gleichbedeutend mit
  „beliebig gross" — ein unbegrenzter Eingang.

`maxProperties` ist jeweils die Zahl der deklarierten Eigenschaften plus
acht.

## Was hier NICHT gebaut ist

Damit niemand danach sucht:

| Fehlt | Eigentuemer | Warum nicht hier |
|---|---|---|
| 16-Byte-Envelope + CRC32C-Parser | `SONDE-010` | Der Entwurf gibt den Parser ausdruecklich diesem Ticket (§65). Fixtures ohne Implementierung waeren toter Ballast. |
| FlatBuffers-`FeatureBatch`, `flatc`-Pinning, Codegen-Drift | `SONDE-005b` (S6) | Zweite Haelfte desselben Entwurfstickets, eigener Sessionschnitt. |
| RFC-8785-Kanonisierung und `state_hash` | `SONDE-006` (S7) | Wird dort erstmals verbraucht. Die ES6-Zahlenserialisierung ist eine eigene Beweisflaeche und gehoert nicht als Anhaengsel hierher. |
| Die 8 reservierten Nachrichtenfamilien | siehe `reservierte-nachrichten-v1.json` | Ihre Nutzlasten (Proposal, DSP-DTO, Experiment) sind erst ab P4 entschieden. |

## Schliessungsvorbehalt

Entwurf §65: „`SONDE-005` darf parallel zu den Hostmessungen **vorbereitet**,
aber erst nach deren Capabilityentscheidung **geschlossen** werden."

Der Vertrag ist deshalb so gebaut, dass die Hostantwort seine **Form** nicht
mehr aendern kann: `capability_wert` kennt `supported` und `unsupported` und
sonst nichts (§54: „Kein `unknown, spaeter pruefen` darf P1 passieren"), und
jedes hostabhaengige Feld haengt an einem eigenen Gueltigkeitsbit. Was S4
liefert, sind **Werte**, keine Felder. Nachzuziehen bleibt nach S4 einzig:
ob der Capability-Satz in `capabilities` vollstaendig ist — kommt aus den
Terminen eine Faehigkeit dazu, die hier fehlt, ist das eine Minor-Erweiterung
mit eigenem Fixture.
