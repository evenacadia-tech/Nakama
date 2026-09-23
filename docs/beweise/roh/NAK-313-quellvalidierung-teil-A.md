# NAK-313 — Quellvalidierung Teil A (lesender Opus-Agent, HEAD 43549a0e)

**Ticket:** NAK-313 · Planschritt S25l „Tiefenaudit 3 abarbeiten, Teil 5: State und
Verträge" · Etappe 0 „Quellvalidierung", **Teil A** (State und Revisionen)
**HEAD:** `43549a0eab4d810cbe9e148e7c8962bc47190042` (Zweig `master`)
**Datum:** 23.09.2026 · **Modell:** Opus 5.5 (lesend, Effort max)
**IDs (6):** T3-02-02, T3-02-04, T3-02-05 (W10); T3-02-01, T3-02-03, T3-03-03 (W11). Dazu
die zwei Regelfragen dieses Teils: die Duplikatregel für doppelte Eigenschaftsnamen im
binären State (T3-02-05) und der persistente Revisionsbereich (T3-02-03).
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen
`:362-366` und `:380`; Skeptikerzeilen Phase 10 `:505-509` und `:514`; Bündel B07 `:77`,
B10 `:80`, B11 `:81`; Kartenentwurf PF11-14 `:302-307`; Arbeitsübersicht W10 `:813`,
W11 `:814`) und die Rohberichte unter `docs/audits/2026-09-15-tiefenaudit/roh/`:
`phase-02-zustand.md`, `phase-03-vertrag.md` (nur T3-03-03), `phase-10-skeptiker.md`,
`phase-11-synthese.md`, `phase-16-c-persistenz.md`, `phase-16-f-gegenpruefung-c.md`,
`phase-16-abdeckung.md`.
**Gate:** Schritt S25l in `docs/plan/plan.json:366-371` (Feld `text` in `:369`);
Registerzeilen `docs/offene-punkte.md:317` (NAK-313), `:261` (NAK-252), `:276` (NAK-267),
`:292` (NAK-283).
**Muster:** `docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md` (Trennung Technik und Produkt).

**Gate-Text wörtlich** (`docs/plan/plan.json:369`, Feld `text`):

> „ANGELEGT 18.09.2026 (Register NAK-313; Arbeitsübersicht Phase 16 W10, W11, W30, W31,
> W33). INHALT: 16 Befunde an State und Verträgen — T3-02-02, T3-02-04, T3-02-05 (Writer
> macht grenzgroßen State unlesbar, Panel-Abbau verliert Label/Paar-ID, doppelte
> ValueTree-Properties), T3-02-01, T3-02-03, T3-03-03 (Revisionsbereich, Überlauf,
> ACK-Revision 0), T3-03-01, T3-03-02, T3-03-04 bis T3-03-09 (JSON- und
> FlatBuffers-Vertrag an jedem Produkteingang: null-Startsample, 1.0/1e0, lockere Syntax,
> Option/null/Text, UTF-8-Längen, zusätzliche FlatBuffer-Slots, v2-Integerüberlauf,
> doppelte Schlüssel), T3-03-10 (binary_telemetry), T3-14-04 (Exportkollision in derselben
> Sekunde). GRENZE: Schemas sind Verträge — versionieren, Altstände laden, unbekannte
> Felder erhalten, Save und Load gemeinsam, Discriminator/Zieladresse/Revision/Capability
> nicht additiv; Fixtures bytegleich mit .gitattributes; Cross-Language-Gleichheit
> C++/Rust; keine stillen Rundungen. Produktentscheide als Karten nur, wo der Vertrag
> schweigt (Duplikatregel, Archiv- oder Überschreibregel beim Export). BEWEIS:
> docs/beweise/NAK-313.md mit Matrix, identischen Positiv-/Negativvektoren durch echte C++-
> und Rust-Reader (A5, A8, A11, A12, B2, B3c, B10, B13, A4), vollem Kanon GRÜN und
> Laufzeit-Arm."

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon,
kein Skript, kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend (`status`,
`rev-parse`, `log`, `show`, `diff`, `blame`, `ls-files`). Jede Zeilennummer unten ist am
HEAD `43549a0e` selbst nachgelesen, keine aus dem Audit übernommen; wo eine Auditzeile
gewandert ist, steht die neue Zahl. Sicherheitsbefunde (T3-12-\*) sind nicht Gegenstand;
Legacy-Bezeichner (`EQ-Copilot`, `EqCop*`, `Eqcp`) sind kein Befund; der Designteil ist
geparkt (12.09.2026) — unten steht kein Oberflächenvorschlag, nur der nötige Handgriff oder
die ehrliche Zustandsmeldung. Die JUCE-Zeilen stammen aus dem Bauartefakt
`eq-copilot/build/_deps/juce-src/` (siehe „Nicht geprüft").

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs* (23.09.2026,
19:11:44 +0200) und *unmittelbar vor dem Schreiben dieser Datei* (19:38:39 +0200)
identisch: die zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` sowie
die drei Auftragsdateien `docs/beweise/roh/NAK-313-quellvalidierung-teil-A-auftrag.txt`,
`…-teil-B-auftrag.txt` und `…-teil-C-auftrag.txt`, sonst nichts. HEAD beide Male
`43549a0e`. Der HEAD ist während des Laufs **nicht** gewandert; ein Basisabgleich der
Zeilennummern war deshalb nicht nötig. Diese Datei ist die einzige Schreibaktion des
Agenten. `…-teil-B.md` und `…-teil-C.md` gehören den Parallelagenten und wurden nicht
angefasst. *Nach dem Schreiben* (19:46:19 +0200, HEAD weiter `43549a0e`) zeigt
`git status --short` zusätzlich zu dieser Datei `…-teil-C.md` des Parallelagenten, sonst
unverändert.

**Stand des Audits gegen den HEAD.** Phase 02 prüfte `3f2619c74d39…` (15.09.2026,
20:00), Phase 03 `8f7c6f5fe934…` (20:51). Die Phase-16-Pakete (`aff2d818`, 18.09.)
brachten zu W10 und W11 keinen frischen Lauf (`BEFUNDE.md:813`: „Altbasis/NAK-252";
`:814`: „Altbasis; C positive Revision/Retry/Undo frisch, jedoch keine Maximalzahlen").
NAK-283 (`3606b0e9`, 12.09.2026, „Revisionsraender (F11)") ist Vorfahr von `3f2619c7`
(`git merge-base --is-ancestor`): seine F11-Reparaturen lagen dem Audit schon vor. Seit
`3f2619c7` änderten 24 Commits die Pfade dieses Teils (NAK-309, NAK-311, NAK-312);
**NAK-310 änderte keinen** (`git log 3f2619c7..HEAD` über State, Editor, Prozessor, IPC,
Broker-Coordinator, Schemas, Fixtures). Für die sechs IDs entscheidend:

- `git diff --stat 8f7c6f5f HEAD -- eq-copilot/plugin/core/ipc/controlclient/
  broker/src/vertrag.rs broker/src/coordinator/` ist **leer**. T3-03-03 und die
  Brokerhälfte von T3-02-03 stehen am HEAD wörtlich wie im Audit.
- `eq-copilot/plugin/src/prozessor/Analyse.cpp` und
  `eq-copilot/plugin/tests/Sonde014IntentTest.cpp` stehen nicht in `git diff --stat
  3f2619c7 HEAD`: T3-02-01 ist unverändert; die Zeilen stammen von `84226d46` (09.09.).
- `eq-copilot/plugin/state/NakamaState.cpp` ist stark verändert (+588/−375, vor allem
  NAK-312 Etappe 7b `f6d80cdf`, Kind `RetainedMainProject`). Die tragenden Zeilen von
  T3-02-02 und T3-02-05 stammen laut `git blame` unverändert von `2f3ee24e` (27.08.); die
  Label-Zeile hat `f6d80cdf` nur verschoben (Auditstand `:821` gegen HEAD `:1098`, gleicher
  Wortlaut). Die Revisionsleser tragen seit `f6d80cdf` den Kindnamen im Grund, sonst
  unverändert.
- `eq-copilot/plugin/src/PluginEditor.cpp`: NAK-312 Etappe 4 (`553b70ae`) setzte die
  Lebendprüfung in `uebernehmen()`; die Kürzungszeilen stammen unverändert von `31c41244`
  (18.08.).
- `eq-copilot/plugin/src/prozessor/State.cpp`: `setzeBindung` hat `f6d80cdf` im Innern
  geändert (der Rollenwechsel löscht nichts mehr); Grenzprüfung und Bindungserzeugung
  stammen unverändert von `048dae92` (09.09.).

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-02-02 (W10) | BESTÄTIGT; dazu: der Vertragstext verspricht genau den fehlenden Schutz, und dieselbe Lücke gilt für die Gesamtgrenze | DEFEKT | medium (Audit: critical) | B2 `StateMigrationTestMain.cpp` neben `:2454-2461`; Zusagezeile `passt` in `NakamaState.cpp:1216-1230` | nein |
| T3-02-04 (W10) | **PRÄZISIERT**: Kürzung von Label und Paarname und der Wegfall eines Paarnamens tragen; die „ungefragte leere Main-Bindung" ist über das Panel nicht erreichbar | DEFEKT | high (Audit: critical) | B15 `ShotTestMain.cpp` `panelLauf` `:373-448` (M-37 mit geladenem langem Label); Zusagezeile die neue Unverändert-Prüfung in `uebernehmen()` `PluginEditor.cpp:802-818` | nein |
| T3-02-05 (W10) | BESTÄTIGT; dazu: auch die Reihenfolge ändert sich, und bekannte Felder werden nach „letzter gewinnt" ausgelegt | LÜCKE, Technikregel | low (Audit: medium) | B2 neben `:2336-2342`; Zusagezeile die neue Duplikaterkennung vor bzw. nach dem JUCE-Leser in `lade` | nein |
| T3-02-01 (W11) | BESTÄTIGT; dazu: Stufe 2 (kein Bedienweg ruft den Wrapper), Nebenwirkungen liegen vor dem Überlauf | DEFEKT | low (Audit: high) | B29 `Sonde014AssistentTest.cpp` neben `:589-602`; Zusagezeile die neue Vorprüfung in `Analyse.cpp:1297-1301` | nein |
| T3-02-03 (W11) | BESTÄTIGT; dazu: der Broker verwirft die ganze Nachricht, drei Obergrenzen leben nebeneinander, Etappe 7b trägt dieselben Felder auch in `RetainedMainProject` | DEFEKT mit Regelbedarf | medium (Audit: high) | B27/B29/B2 (Leser), B7 (Sonde), A4/A5 (Draht); Zusagezeile die neue Obergrenze im Revisionsleser | nein — Technikregel (Regelfrage 2) |
| T3-03-03 (W11) | BESTÄTIGT; dazu: die 0 reist in einen Konflikt-Wiederholungskopf, ein Erfolgs-ACK verändert den Produktzustand; die eigene Gegenstelle sendet so ein ACK nie | DEFEKT | medium (Audit: high) | B10 `IpcTestMain.cpp` neben `:5982-5986`; Zusagezeile die Grenzprüfung in `commandAckArtLesen` `Intern.h:206-217` | nein |

---

## 1. T3-02-02 — der eigene Writer macht einen angenommenen grenzgroßen State unlesbar (W10)

*Befundzeile: `BEFUNDE.md:363` · Skeptikerzeile Phase 10: `:506` · Bündel B10: `:80` ·
Arbeitsübersicht W10: `:813` · Rohbeleg: `roh/phase-02-zustand.md:40-64` · Register
NAK-252: `docs/offene-punkte.md:261`*

### a) Behauptung des Audits

Ein Common mit 65.536 verschiedenen Eigenschaften ohne `label` passiert alle
Eingangsgrenzen und lädt schreibbar; der Writer ergänzt `label` zu 65.537, und der eigene
nächste Load ignoriert den ganzen State (`BEFUNDE.md:363`).

### b) Quellkette am HEAD

**Station 1 — die Grenzen des Byte-Vorlesers.**
`eq-copilot/plugin/state/NakamaState.cpp:91-95`:

```
constexpr size_t kMaxStateBytes = 16u * 1024u * 1024u;
constexpr int kMaxStateTiefe = 64;
constexpr int kMaxVariantenTiefe = 64;
constexpr int kMaxEintraegeJeSammlung = 65536;
constexpr int kMaxEintraegeGesamt = 262144;
```

Die Sammlungsgrenze sitzt in `zaehler`, `:193-203`:

```
    bool zaehler (int& aus)
    {
        std::int64_t wert = 0;
        if (! komprimierteZahl (wert) || wert < 0 || wert > kMaxEintraegeJeSammlung)
            return false;
        if (eintraegeGesamt > kMaxEintraegeGesamt - static_cast<int> (wert))
            return false;
```

Sie gilt je Eigenschaftsliste eines Knotens, `baum`, `:287-289`
(`int eigenschaften = 0; if (! zaehler (eigenschaften)) return false;`). **Zahlenrand:**
65.536 Eigenschaften passieren (`wert > 65536` ist falsch), 65.537 fallen. Der Auditbaum
(Root mit `schema` und einem Kind, Common mit vier Pflichtfeldern und 65.532 unbekannten
`int32`) zählt 1 + 1 + 65.536 + 0 = 65.538 Einträge insgesamt, weit unter 262.144.

**Station 2 — die Annahme.** `lade`, `:2236-2240` und `:2274-2284`:

```
    const auto bytePruefung = pruefeValueTreeBytes (daten, laenge);
    if (bytePruefung == BytePruefung::ungueltig)
        return LadeErgebnis::ignoriert;
```
```
        if (! leseSchema2 (v, bundle, z, grund))
            return nurLesen (grund, v);
        if (! hatWriterHeadroom (z, bundle))
            return nurLesen ("state leaves no bounded headroom for a losslessly reloadable save", v);
        z.herkunft = Herkunft::schema2Geladen;
        aus = z;
        return LadeErgebnis::geladen;
```

`label` darf fehlen, `:1612-1614`
(`if (common.hasProperty (kLabel) && ! label.isString()) { … }`); legacy mit `insert` ist
nach Kind-Matrix und Messpositionsregel erlaubt (`:1603-1610`, `:1631-1645`).

**Station 3 — der Headroom prüft nur Bytes.** `hatWriterHeadroom`, `:1194-1195`
(Kommentar) und `:1216-1230`:

```
/** Beweist beim Laden, dass jede heute ueber die Produkt-API erreichbare
    Aenderung wieder einen State <= 16 MiB schreibt. …
```
```
    auto passt = [] (const Zustand& kandidat)
    {
        try
        {
            juce::MemoryBlock bytes;
            juce::MemoryOutputStream strom (bytes, false);
            synchronisiert (kandidat).writeToStream (strom);
            strom.flush();
            return bytes.getSize() <= kMaxStateBytes;
        }
```

Der Kandidat trägt sogar drei zusätzliche Common-Eigenschaften (`:1235-1238`: Label mit
120 × U+10FFFF, Paarname mit 60 × U+10FFFF, Bindung `ffff…`), also 65.539 — geprüft wird
nur, ob die Bytes unter 16 MiB bleiben. Der Kommentar `:1194-1195` sagt das ehrlich
(„<= 16 MiB").

**Station 4 — Save ergänzt `label` bedingungslos.** `synchronisiert`, `:1098-1102`:

```
    common.setProperty (kLabel, z.common.label, nullptr);
    if (z.common.pairId.isNotEmpty()) common.setProperty (kPairId, z.common.pairId, nullptr);
    else                              common.removeProperty (kPairId, nullptr);
```

`speichere` (`:2290-2301`) und `EqCopilotProcessor::getStateInformation`
(`eq-copilot/plugin/src/prozessor/State.cpp:69-113`) schreiben ohne erneute Prüfung.
Ergebnis: 65.537 Common-Eigenschaften.

**Station 5 — der nächste Load ignoriert.** `:196` fällt, `pruefe` liefert `ungueltig`
(`:118-119`), `lade` gibt `LadeErgebnis::ignoriert` (`:2237-2238`). Gen,
`State.cpp:147-148`:

```
    if (ergebnis == nakama::state::LadeErgebnis::ignoriert)
        return;   // fremder Baumtyp / Muell: Zustand bleibt (wie seit 0.1)
```

Probeeq ebenso, `eq-copilot/plugin/sonde/SondeProcessor.cpp:1264-1265`. **Wirkung:** die
neue Instanz behält ihren frischen Zustand; das nächste Speichern schreibt diesen über die
Projektbytes — Instanzkennung, Klasse, Messposition, Label, alle unbekannten Eigenschaften
und bei Main der Bestand sind danach endgültig fort. Nicht read-only, sondern verworfen.

**Eigene Ergänzung — dieselbe Lücke an der Gesamtgrenze.** `kMaxEintraegeGesamt`
(`:95`, geprüft in `:198`) hat dieselbe Asymmetrie: ein geladener Baum knapp unter 262.144
Einträgen, dem der Writer Einträge hinzufügt (Label; bei `active_probe` das Kind `Dsp`
(`:1151-1185`); bei Main die Bestandsarrays über die Produkt-API), wird ebenso vom eigenen
Leser verworfen. Hergeleitet, nicht gemessen.

**Erreichbarkeit.** Hostbytes (Stufe 1), aber nur mit einem fremden oder von Hand gebauten
State: kein Produktweg schreibt unbekannte Eigenschaften, und der größte legitime Bestand
(256 Intents, je 256 Schutzangaben und Beziehungen, 64 Mitglieder und Passagen) liegt bei
wenigen tausend Einträgen.

### c) Zusage wörtlich

- `eq-copilot/schemas/state/nakama-state-v2.md:302-307`: „Für einen **schreibbaren**
  geladenen State baut der Leser zusätzlich den größten mit den heutigen API-Grenzen
  erreichbaren Folgezustand über demselben additiven Baum. Passt dieser nicht vollständig
  unter 16 MiB, bleibt der Eingang read-only und damit bytegleich. Dieser konkrete Headroom
  deckt die maximalen heutigen Userfelder und verhindert, dass `Save(Load(x))` einen State
  erzeugt, den derselbe Leser beim nächsten Start wegen seiner eigenen Grenze ablehnt."
- `docs/beweise/NAK-283.md:607` (M-11): „**Der eigene Reader nimmt jeden selbst erzeugten
  Stand an, und ein abgewiesener Handgriff verändert die gespeicherten Bytes nicht.**" Die
  Zeile steht im Kontext des Revisionsrandes; der Satz selbst ist allgemein.
- `CLAUDE.md:194-196`: „**State bleibt verlustfrei.** Unbekannte Major-Versionen oder
  unzulässige Matrizen werden read-only mit Originalbytes gehalten; jede persistente
  Änderung meldet dem Host Dirty-State."
- Register NAK-252, `docs/offene-punkte.md:261`: „Zu widerlegender Verdacht: ein Baum direkt
  an einer Collectiongrenze könnte nach dem Nachtragen einer optionalen Label-Property vom
  eigenen Reader abgewiesen werden. … Wer es angeht: Grenzfixture bauen, Read/Write/Read
  fahren."

**Irreführender Vertragstext.** Der letzte Satz von `nakama-state-v2.md:305-307` verspricht
Schutz gegen *jede* eigene Grenze; `:304` und der Code prüfen nur 16 MiB. Dazu:
`nakama-state-v2.md:58-59` („Der Byte-Riegel (§5) erlaubt höchstens 64 `ValueTree`-Knoten
im ganzen Baum") und `:297-298` („höchstens 64 `ValueTree`-Knoten inklusive Wurzel")
beschreiben eine **Knotenzahl**; der Code begrenzt die **Tiefe**
(`NakamaState.cpp:92`, `:284`: `if (tiefe >= kMaxStateTiefe || …)`), und nur die
Wurzelkinder werden gezählt (`:1563-1575`). Der Test `zuTieferBaum`
(`eq-copilot/plugin/tests/StateMigrationTestMain.cpp:243-254`) baut eine Kette und kann
beides nicht unterscheiden.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B2 `StateMigrationTestMain.cpp:2454-2461` (Bein `tools/beweise.ps1:693`) | 65.536 Eigenschaften passieren den Riegel (Ergebnis `nurLesen`), 65.537 werden verworfen | Der Baum aus `baumMitEigenschaftszahl` (`:349-363`) hat keinen Common, alle Namen heißen `x`, kein Kind: nie schreibbar, kein Save→Load |
| B2 `:2463-2470` | 262.144 gegen 262.145 Gesamteinträge | dieselbe Bauart, nie schreibbar |
| B2 `:2473-2540` | der Byte-Headroom an der 16-MiB-Grenze | misst Bytes, nicht Einträge |
| NAK-312 M-121 (`docs/beweise/NAK-312.md:2415`) | Abstand des Legacy-Bestands zur 16-MiB-Grenze | nur Bytes; die Zeile sagt selbst „T3-02-02 bleibt S25l (§47.4)" |
| A12 (`tools/beweise.ps1:576`), `eq-copilot/fixtures/state/MANIFEST.json:386-437` | zehn Schema-2-Goldens bytegleich | kein Golden an einer Sammlungs- oder Gesamtgrenze |

Die Behauptungszeile B2 (`tools/beweise.ps1:693`) sagt nichts über Sammlungsgrenzen.

### e) Urteil: BESTÄTIGT

Alle fünf Stationen am HEAD gelesen; die Kette schließt. Gegenüber dem Auditstand sind nur
Zeilen gewandert (`:85`→`:94`, `:821`→`:1098`, `:1116`→`:1224`, `:1503`→`:1612`); keine
der drei Folgetickets hat den Mechanismus berührt. Drei Ergänzungen: (1) Der Vertragstext
`nakama-state-v2.md:305-307` verspricht den fehlenden Schutz — der Fix muss Code und Text
zugleich ändern. (2) Die Gesamtgrenze trägt dieselbe Lücke. (3) Die Knotengrenze des
Vertragstexts gibt es im Code nicht. Die Byteanzahlen des Audits (917.605 und 917.615)
habe ich nicht nachgerechnet; für das Urteil trägt die Eigenschaftszahl.

### f) Kategorie und Schwere

**DEFEKT** gegen `nakama-state-v2.md:305-307` und den allgemeinen Satz von M-11. **Schwere
medium** (Audit: critical): der Verlust ist vollständig und still, aber nur ein fremder oder
von Hand gebauter State erreicht die Grenze.

### g) Kleinster Fix

**Ort:** `hatWriterHeadroom`, `NakamaState.cpp:1202-1359`, dort `passt` (`:1216-1230`).

Der Kandidat wird zusätzlich durch denselben allokationsfreien Byte-Vorleser geführt, der
später die gespeicherten Bytes beurteilt (`pruefeValueTreeBytes`, `:311-316`), und muss
`verlustfrei` ergeben. Damit sind Tiefe, Sammlungs- und Gesamtgrenze und die Markerregel in
einer Zeile Logik abgedeckt, ohne neue Grenzkonstante. Fällt ein Kandidat, führt der
vorhandene Weg in read-only mit Originalbytes (`:2280-2281`). Keine neue State-Version und
keine Formatänderung: eine bisher schreibbar angenommene Randeingabe wird read-only und
bleibt bytegleich. Kein eingefrorenes Fixture von 0,9 MB; der Grenzbaum entsteht im Test
aus Rohbytes. Der Vertragstext zieht mit: `nakama-state-v2.md:302-307` nennt den ganzen
Byte-Riegel statt nur 16 MiB, `:58-59` und `:297-298` sprechen von Tiefe 64 (oder der Code
bekommt eine echte Knotengrenze — das kleinere ist der Text).

**Mitzuprüfende Beziehung:** speichern↔laden. Ein angenommener Stand lädt nach
`Save(Load(x))` wieder schreibbar; ein abgewiesener bleibt read-only und kommt bytegleich
zurück. Gegenfall: der Stand eins unter der Grenze bleibt schreibbar und läuft rund.

### h) Ort des Rotbeweises

**Bein B2** (`EqCopStateMigrationTest`, `eq-copilot/plugin/CMakeLists.txt:789-791`,
Behauptungszeile `tools/beweise.ps1:693`), `StateMigrationTestMain.cpp` neben `:2454-2461`.

Aufbau aus Rohbytes (Muster `baumMitRohVariante`, `:256`): Root `schema = 2`, genau ein
Common mit `schema = 1`, `instance_id`, `plugin_kind = legacy`,
`measurement_position = insert` und 65.532 **verschieden** benannten `int32`-Eigenschaften,
ohne `label`. Assertionen:

- `lade` liefert `nurLesen` mit dem Headroom-Grund, `speichere` gibt die Eingabebytes
  bytegleich zurück — **heute rot**: `geladen`, Save mit 65.537, Reload `ignoriert`.
- Gegenfall mit 65.531 unbekannten Eigenschaften (65.535 ohne Label): `geladen`, Save mit
  65.536, Reload `geladen`, zweites Save bytegleich.
- Gesamtgrenze entsprechend (Baum knapp unter 262.144 Einträgen, Writer fügt hinzu).

Die Zeile, die die Zusage trägt, ist `passt` (`NakamaState.cpp:1216-1230`): eine Mutation,
die dort auf den reinen Größenvergleich (`:1224`) zurückgeht, muss den ersten Fall reißen.

---

## 2. T3-02-04 — das unveränderte Schließen des Messpunktpanels kürzt geladene Texte (W10)

*Befundzeile: `BEFUNDE.md:365` · Skeptikerzeile Phase 10: `:508` · Bündel B07: `:77` ·
Arbeitsübersicht W10: `:813` · Rohbeleg: `roh/phase-02-zustand.md:81-92`*

### a) Behauptung des Audits

Unverändertes Schließen des Messpunktpanels kürzt ein geladenes `label` auf 120 und
`pair_id` auf 60 Zeichen und kann dabei ungefragt eine leere Main-Projektbindung erzeugen;
Dirty und nächster Save tragen die gekürzten Werte (`BEFUNDE.md:365`).

### b) Quellkette am HEAD

**Station 1 — der Leser bewahrt lange Texte.** `NakamaState.cpp:1612-1620`: `label` muss
nur eine Zeichenkette sein, `pair_id` bei Präsenz eine nichtleere; keine Längengrenze. Die
Schema-1-Migration übernimmt beide bytegleich (`:2205-2206`).

**Station 2 — das Panel übernimmt die vollen Texte.** `PluginEditor.cpp:779-780`:

```
            initFeld (labelFeld, proz.holeLabel());
            initFeld (paarFeld, proz.holePaarId());
```

`initFeld` (`:767-778`) setzt keine Eingabegrenze; die einzige im Editor ist
`sourcesLabelFeld.setInputRestrictions (120);` (`:198`, Main-Fläche).

**Station 3 — jedes Schließen und jeder Fokusverlust übernimmt.** `:784`
`~MesspunktPanel() override { uebernehmen(); }` und `:819-820`:

```
        void comboBoxChanged (juce::ComboBox*) override { paarSichtbarkeit(); uebernehmen(); }
        void textEditorFocusLost (juce::TextEditor&) override { uebernehmen(); }
```

**Station 4 — die Kürzung.** `uebernehmen()`, `:802-818`:

```
        void uebernehmen()
        {
            auto* ed = editor.getComponent();
            if (ed == nullptr)
                return;
            const auto id = rolleWahl.getSelectedId();
            const juce::String rolle = id == 2 ? "hub" : id == 3 ? "pre" : id == 4 ? "post" : "sensor";
            const bool paar = (id == 3 || id == 4);
            const auto label = labelFeld.getText().substring (0, 120);
            const auto paarId = paar ? paarFeld.getText().substring (0, 60) : juce::String();
#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
            if (auto& marke = testzugang::messpunktMarkeFuerTest(); marke && ! marke())
                return;   // R-312-12: gezählt, kein Zugriff
#endif
            if (ed->processor.setzeBindung (rolle, label, paarId) && geaendert)
                geaendert();   // Kopfzeile (Rolle/Name) sofort nachziehen
        }
```

**Station 5 — `setzeBindung` nimmt die gekürzten Werte als Änderung.**
`State.cpp:316-317` lässt 120/60 durch (`if (lbl.length() > 120 || p.length() > 60)
return false;`); `:332-336` übernimmt Rolle, Label und Paarnamen; `:343-344`:

```
        if (neu == zustand.common)
            return false;   // keine Aenderung: kein Dirty, kein Reconnect-Geflacker
```

vergleicht mit dem **gekürzten** Wert und findet eine Änderung; `:345`
`zustand.common = neu;`, danach Publikation und Reconnect (`:390-393`) und in
`werteSourcesPublikationAus` (`eq-copilot/plugin/src/prozessor/Ipc.cpp:1499-1500`)
`meldeHostDirty(); v3StateRevision.fetch_add (1);`. Das nächste Speichern schreibt den
gekürzten Wert (`NakamaState.cpp:1098-1100`).

**Station 6 — der Paarname fällt auch ganz weg.** Steht ein geladener legacy+insert-Stand
(Rolle `sensor`) mit `pair_id` im Panel, ist `paar` falsch (`:809`), `paarId` leer
(`:811`), und `setzeBindung` entfernt den Paarnamen samt Dirty. Der Leser erlaubt `pair_id`
für jede Klasse (`NakamaState.cpp:1615-1620`).

**Station 7 — die „leere Main-Bindung" ist über das Panel nicht erreichbar.**
`setzeBindung` erzeugt die Bindung vor dem Vergleich, `State.cpp:340-342`
(`if (klasse == nakama::state::Klasse::main && neu.projectBindingId.isEmpty())
neu.projectBindingId = juce::String (uuidHex32());`). Das Panel sendet `hub` aber nur,
wenn die Auswahl beim Öffnen `hub` war oder der User „Sammelpunkt (Master)" wählt
(`PluginEditor.cpp:754`, `:808`, `:819`). Das erste ist ausgeschlossen: der Knopf ist in
jeder `hub`-Rolle unsichtbar — `:955-958`:

```
bool EqCopilotEditor::istMainFlaeche() const
{
    return processor.holeRolle() == "hub" && ! processor.stateNurLesen();
}
```

und `:981-987`, `messpunktKnopf` in der Legacy-Liste mit `c->setVisible (! sollMain);`;
der Konstruktor ruft `wechsleFlaecheWennNoetig()` sofort (`:239`). `zeigeMesspunkt`
(`:704`) hat keinen anderen Aufrufer als den Knopf (`:165`). Dieselbe Sichtbarkeitsregel
stand schon am Auditstand (`git show 3f2619c7`: Legacy-Liste `:840-844`,
`istMainFlaeche` `:828-831`). Die Bindung entsteht also nur an einer ausdrücklichen
Rollenwahl — genau dem Akt, den `State.cpp:337-339` und der Lebenslauf (`:364-375`)
vorsehen.

**Was NAK-312 geändert hat.** `553b70ae` (Etappe 4, R-312-2) setzte die Lebendprüfung
(`:804-806`) und den Zugriff über `ed->processor` (`:816`). Stirbt der Editor vor dem Panel,
passiert seither nichts — auch keine Kürzung. Beim gewöhnlichen Schließen mit lebendem
Editor, beim Fokuswechsel und beim Rollenwechsel bleibt der Befund bestehen.

**Zahlenränder und Unicode.** `juce::String::length()` und `substring` zählen Codepoints;
die Grenzen gelten in Codepoints (`NakamaState.cpp:1200-1201`), eine UTF-8-Folge wird nie
zerteilt. 120 Codepoints bleiben unverändert, 121 werden gekürzt; beim Paarnamen 60
gegen 61. Eine Graphem-Folge (Grundzeichen mit Kombinationszeichen, ZWJ-Emoji, Flagge)
kann an Stelle 120 zerteilt werden (hergeleitet). Dieselbe Kürzung trifft ohne
Eingabegrenze auch getippte Überlänge: wer 130 Zeichen tippt, bekommt still 120.

**Erreichbarkeit: Stufe 1**, sobald ein geladener Stand ein Label über 120 oder einen
Paarnamen über 60 Codepoints trägt — aus einem Altprojekt, einer Migration oder einem
fremden Werkzeug. Ob echte Altprojekte solche Labels tragen, ist aus diesem Repo nicht
bestimmbar (siehe „Nicht geprüft").

### c) Zusage wörtlich

- `nakama-state-v2.md:315`: „Jede persistente Änderung aus dem Plugin heraus
  (`setzeBindung` mit echter Änderung, `neueSensorId`, später: bestätigte Bindung) meldet
  `updateHostDisplay (…)` … **Laden und Migration melden nicht** … Ohne echte Änderung:
  keine Meldung, kein Reconnect."
- `nakama-state-v2.md:20-21`: `label` „User-Wort, untrusted, nie interpretiert (auch leer;
  eigener Writer höchstens 120 Zeichen)", `pair_id` „NUR wenn nicht leer (fehlt = kein Paar;
  eigener Writer höchstens 60 Zeichen)" — die Grenze gilt der eigenen Eingabe, nicht
  geladenen Werten.
- `nakama-state-v2.md:268`: „`label` bytegleich; `pair_id` bytegleich, leer ⇒ Eigenschaft
  fehlt." (Migration)
- `docs/beweise/NAK-312.md:848` (M-37): „**Keine Mutation und keine Dirty-Meldung: der
  Frühausstieg wegen fehlender Änderung bleibt, wo er ist**".
- Behauptungszeile B15, `tools/beweise.ps1:752`: „schliesst es, solange der Editor lebt,
  wird eine Aenderung mit genau einer Host-Dirty-Meldung uebernommen, ohne Aenderung mit
  keiner."
- `docs/plugin-wissen.md:1460-1461`: „der normale Handgriff meldet genau ein Host-Dirty
  (M-36), ohne Änderung keines (M-37)."
- `CLAUDE.md:194-196` (oben zitiert).
- Zur Bindung: `nakama-state-v2.md:22` „`project_binding_id` … wird NIE still erzeugt
  (§32.2)" — über das Panel **nicht** gebrochen (Station 7).

**Irreführender Kommentar:** `PluginEditor.cpp:793-795`: „Übernimmt NUR bei echter
Änderung — setzeBindung löst einen Reconnect aus, und der wäre bei jedem Öffnen/Schließen
oder Fokuswechsel ohne Änderung reines Verbindungs-Geflacker." Die „echte Änderung"
entscheidet `setzeBindung` gegen die schon gekürzte Kopie; bei langem Label ist jedes
Schließen eine Änderung.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B15 `ShotTestMain.cpp:480-485` (312/M-37), Bein `tools/beweise.ps1:752` | Panel öffnen, nichts ändern, schließen: 0 Dirty, Label gleich | frischer Prozessor mit Standardlabel (`:388`, `:391`); nie ein geladenes Label über 120 |
| B15 `:472-478` (312/M-36) | echte Änderung → genau ein Dirty | gewollter Weg |
| B2 `StateMigrationTestMain.cpp:2795` | `setzeBindung` ohne Änderung meldet nichts | API-Aufruf, kein Panel |
| B2 `:2809-2815` | die API weist 121/61 ab | zeigt genau die Grenze, die das Panel durch Vorkürzen umgeht |

Kein Fixture trägt ein Label über 120 oder einen Paarnamen über 60 Codepoints (die
Schema-1-Rollentabelle `:181-186` hat kurze Texte; `MANIFEST.json:386-437`).

### e) Urteil: PRÄZISIERT

Kürzung von Label (121 → 120) und Paarname (61 → 60), der Wegfall eines Paarnamens bei
Rolle `sensor` und die ungefragte Dirty-Meldung samt Reconnect tragen am HEAD. **Die
„ungefragt erzeugte leere Main-Projektbindung" trägt über das Panel nicht:** der Knopf ist
in jeder `hub`-Rolle unsichtbar, heute wie am Auditstand; sie ist eine Eigenschaft der
API `setzeBindung` (`State.cpp:340-342`), die das Produkt nur nach einer ausdrücklichen
Rollenwahl erreicht. NAK-312 Etappe 4 (`553b70ae`) hat nur den Abbauweg nach dem Tod des
Editors entschärft. Zusätzlich gefunden: Fokusverlust und Rollenwechsel lösen dieselbe
Kürzung aus, und getippte Überlänge wird still gekürzt.

### f) Kategorie und Schwere

**DEFEKT** gegen `nakama-state-v2.md:315`, M-37 (`NAK-312.md:848`) und `CLAUDE.md:194`.
**Schwere high** (Audit: critical): ein gewöhnlicher Handgriff (Öffnen und Schließen, ein
Klick ins Feld) verliert still Text des Users, meldet Dirty und verbindet neu; Bedingung ist
ein geladener langer Text.

### g) Kleinster Fix

**Ort:** `uebernehmen()`, `PluginEditor.cpp:802-818`, Konstruktor `:728-783`, und
`setzeBindung`, `State.cpp:316`.

1. Das Panel merkt sich beim Öffnen Rolle, Label und Paarnamen. `uebernehmen()` kehrt nach
   der Lebendprüfung ohne Aufruf zurück, wenn keiner der drei Werte geändert wurde.
2. Bei einer Änderung reist nur das Geänderte aus dem Panel; unveränderte Felder kommen aus
   dem aktuellen Prozessorzustand, nicht aus der Panelkopie. Kein `substring` mehr: die
   Felder bekommen Eingabegrenzen 120/60 wie `:198`.
3. `State.cpp:316` prüft die Grenzen nur für geänderte Werte. Sonst scheitert eine reine
   Rollenwahl an einem unverändert geladenen langen Label — ein toter Handgriff.
4. Der Kommentar `:793-795` sagt danach, was gilt.

Keine neue State-Version, keine Schemaänderung: `nakama-state-v2.md:20-21` sagt schon, dass
die Grenzen dem eigenen Writer gelten. Der Headroom-Kandidat deckt ein erhaltenes langes
Label schon (`NakamaState.cpp:1235`, `laenger`).

**Mitzuprüfende Beziehungen:** öffnen↔schließen (M-36 bleibt: eine Änderung gibt genau ein
Dirty; M-37 gilt dann auch mit langem Label), speichern↔laden (langes Label laden,
schließen, speichern: bytegleich), R-312-2 (die Lebendprüfung bleibt die erste Zeile),
Karte U57 bleibt unberührt (`docs/plan/fragen.json:64-72`; nach dem Ende des Editors wird
weiterhin nichts übernommen).

### h) Ort des Rotbeweises

**Bein B15** (`EqCopShot`, `eq-copilot/plugin/CMakeLists.txt:686-688`, Behauptungszeile
`tools/beweise.ps1:752`), `ShotTestMain.cpp`, neue Variante von `panelLauf` (`:373-448`):
vor dem Editor `setStateInformation` mit einem legacy+pre-Stand, Label 121 Codepoints,
Paarname 61; öffnen, nichts ändern, schließen (Folge `panelVorEditor`,
`aendern = false`). Assertionen:

- `holeLabel()` und `holePaarId()` bytegleich zum geladenen Original, 0 Host-Dirty,
  `getStateInformation` vorher und nachher bytegleich — **heute rot** (120/60, Dirty 1);
- zweiter Fall legacy+insert mit Paarnamen: der Paarname bleibt;
- Gegenfall wie M-36: eine echte Änderung wird mit genau einem Dirty übernommen.

Dazu in **B2** neben `StateMigrationTestMain.cpp:2809-2815`: eine Rollenwahl mit
unverändert geladenem 121er-Label gelingt und lässt das Label stehen; ein **geändertes**
121er-Label wird weiter abgewiesen. Die Zeile, die die Zusage trägt, ist die neue
Unverändert-Prüfung in `uebernehmen()`: entfernt man sie, muss der B15-Fall reißen.

---

## 3. T3-02-05 — doppelte Eigenschaftsnamen im binären State (W10)

*Befundzeile: `BEFUNDE.md:366` · Skeptikerzeile Phase 10: `:509` · Bündel B10: `:80` ·
Kartenentwurf PF11-14: `:302-307` · Arbeitsübersicht W10: `:813` · Rohbeleg:
`roh/phase-02-zustand.md:94-100`*

### a) Behauptung des Audits

Doppelte Eigenschaftsnamen im binären Host-State passieren den Byte-Riegel, JUCE reduziert
sie still auf den letzten Wert, der nächste Save enthält nur diesen; eine Regel für solche
mehrdeutigen Fremdbytes fehlt (`BEFUNDE.md:366`).

### b) Quellkette am HEAD

**Station 1 — der Byte-Vorleser prüft Namen einzeln.** `NakamaState.cpp:287-292`:

```
        int eigenschaften = 0;
        if (! zaehler (eigenschaften))
            return false;
        for (int i = 0; i < eigenschaften; ++i)
            if (! utf8CString (false) || ! variante (0))
                return false;
```

`utf8CString` (`:167-191`) prüft Länge, NUL und UTF-8, führt aber keine Menge gelesener
Namen.

**Station 2 — JUCE setzt, statt anzuhängen.**
`eq-copilot/build/_deps/juce-src/modules/juce_data_structures/values/juce_ValueTree.cpp:1064-1072`:

```
    for (int i = 0; i < numProps; ++i)
    {
        auto name = input.readString();

        if (name.isNotEmpty())
            v.object->properties.set (name, var::readFromStream (input));
```

und `…/juce_core/containers/juce_NamedValueSet.cpp:195-207`: `set` ersetzt den Wert eines
vorhandenen Namens (`*v = newValue;`) und hängt nur Neues an. Der erste Name behält seine
Position, der letzte Wert gewinnt.

**Station 3 — Leser und Writer sehen nur noch einen Wert.** `leseSchema2` liest den Baum
aus JUCE; `synchronisiert` editiert eine Kopie dieses Baums (`:1085`). Save gibt nur den
letzten Wert an der Position des ersten zurück: `future=A, x=1, future=B` wird zu
`future=B, x=1`.

**Eigene Ergänzungen.** (1) Auch **bekannte** Eigenschaften können doppelt stehen und
werden nach „letzter gewinnt" ausgelegt: Root `schema` 3 dann 2 lädt schreibbar und
speichert nur 2; Common `plugin_kind` erst `legacy`, dann `main` lädt als Main. (2) Für den
verwandten Fall, dass JUCE Bytes nicht verlustfrei zurückschreiben kann, gibt es schon einen
Weg: Marker 9 und unbekannte Marker setzen `nichtVerlustfrei` (`:267-276`), `pruefe`
liefert für eine bekannte Wurzel `bekannteWurzelNichtVerlustfrei` (`:120-121`), `lade` hält
dann read-only mit Originalbytes (`:2239-2240`). (3) Der Grenztest `baumMitEigenschaftszahl`
(`StateMigrationTestMain.cpp:349-363`) schreibt 65.536 Eigenschaften, die **alle** `x`
heißen — er stützt sich nebenbei darauf, dass Duplikate heute passieren.

**Erreichbarkeit.** Nur fremde, beschädigte oder von Hand gebaute Bytes; JUCEs Writer
erzeugt keine doppelten Namen, und kein Produktweg schreibt Rohbytes.

### c) Zusage wörtlich

**Die Quellen schweigen zu doppelten Eigenschaftsnamen ausdrücklich** (gesucht in
`nakama-state-v2.md`, Entwurf §53.8 `docs/FL-Nakama-Sonden-Design-Entwurf.md:4052-4085`,
`CLAUDE.md`). Sie legen die Antwort aber fest:

- `CLAUDE.md:194-196` (oben zitiert).
- `nakama-state-v2.md:10`: „**unbekannte Eigenschaften überleben einen Roundtrip** (der
  Leser hält den Baum und schreibt ihn zurück, statt ihn aus Feldern neu zu bauen)".
- `nakama-state-v2.md:152`: „Eine **unbekannte Eigenschaft** in einem bekannten Kind
  desselben Majors ⇒ additiv, wird erhalten."
- `nakama-state-v2.md:307-311`: „Marker, die JUCE 8 nicht byteverlustfrei zurückschreiben
  kann (`undefined`/Marker 9 oder ein zukünftiger Marker), machen eine bekannte
  `NakamaState`-/`EqCopilotState`-Wurzel dagegen **read-only**; die Originalbytes gehen
  unverändert an den Host zurück."
- Für JSON lehnt das Produkt doppelte Schlüssel ab: `nakama-state-v2.md:277` („Textstufe
  doppelter Schlüssel") und `:282`, `eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:515-518`
  („Doppelter Schluessel ist eine Ablehnung, keine "letzter gewinnt"-Regel.").

### d) Heutige Abdeckung

B2 prüft zwei Common-**Kinder** (`StateMigrationTestMain.cpp:2340`, „Common doppelt"),
ein doppeltes Bestandskind (`:2642`) und doppelte JSON-Schlüssel im Kanonleser
(`:929-934`) — nie zwei gleichnamige Eigenschaften in einem Knoten. Die read-only-Fälle
(`:2336-2342`) entstehen über die JUCE-API, die gar keine doppelten Namen bauen kann. Kein
Fixture. Die Karte PF11-14 steht nicht in `docs/plan/fragen.json` (Suche nach
„PF11-14", „T3-02-05", „T3-03-09": kein Treffer).

### e) Urteil: BESTÄTIGT

Die Kette schließt am HEAD; gegenüber dem Audit sind nur Zeilen gewandert (`:281` →
`:287-292`, `:2117` → `:2236-2240`). Ergänzt: die Reihenfolge der Eigenschaften ändert sich
mit, bekannte Felder werden nach „letzter gewinnt" ausgelegt, und der Marker-Weg ist das
fertige Vorbild für die Regel.

### f) Kategorie und Schwere

**LÜCKE, Technikregel** (Begründung unter „Regelfragen", Frage 1). Die Sätze, die ein
Duplikat verletzt (`nakama-state-v2.md:10`, `:152`), setzen eindeutige Namen voraus; der
Fall selbst ist nirgends genannt. Die Antwort folgt aber zwingend aus `CLAUDE.md:194-196`
und `nakama-state-v2.md:307-311`. **Schwere low** (Audit: medium): nur fremde oder
beschädigte Bytes erreichen den Weg, kein Produktwriter erzeugt sie, keine Audiowirkung.

### g) Kleinster Fix

**Ort:** `lade`, `NakamaState.cpp:2221-2288`, und `ValueTreeByteRiegel`, `:104-309`.

Ein doppelter Name in einem Knoten führt bei bekannter Wurzel in read-only mit
Originalbytes, auf demselben Weg wie Marker 9; bei fremder Wurzel bleibt es bei `ignoriert`
(`:2287`). Allokationsfreie Form: der Byte-Riegel zählt alle Eigenschaftseinträge; nach
`readFromData` (`:2245`) zählt `lade` `getNumProperties()` über den ganzen Baum. Ist die
JUCE-Zahl kleiner, hat JUCE Namen zusammengelegt (leere Namen und leere Typen lehnt der
Riegel schon ab, `:171`, `:284`). Die Technik kann auch eine Namensmenge je Knoten führen;
die Zählung braucht keinen Speicher.

Keine neue State-Version. Der Vertragstext nimmt den Fall in die read-only-Liste
`nakama-state-v2.md:286` und in den Markerabsatz `:307-311` auf. Ein kleines
Negativ-Golden (ein legacy-Common mit `future` zweimal) entsteht über
`EqCopStateMigrationTest --schreibe-goldens` und wird im Erzeuger registriert
(`tools/eq-copilot/erzeuge_state_fixtures.py:1090-1092`, Bein A12); `fixtures/state/**`
steht laut `CLAUDE.md` unter `-text`. `baumMitEigenschaftszahl` bekommt verschiedene Namen,
damit der Grenztest nur die Grenze misst.

**Mitzuprüfende Beziehung:** speichern↔laden — read-only, Save gibt die Originalbytes
zurück, Reload wieder read-only; Gegenfall mit eindeutigen Namen lädt schreibbar und läuft
bytegleich rund.

### h) Ort des Rotbeweises

**Bein B2**, `StateMigrationTestMain.cpp` neben den read-only-Fällen `:2336-2342`.
Rohbyte-Baum (Muster `:256`): gültiger legacy-Common mit `future = "A"` und danach
`future = "B"`. Assertionen: `lade` liefert `nurLesen` mit einem Grund, der das Duplikat
nennt; `speichere` gibt die Eingabebytes bytegleich zurück — **heute rot** (`geladen`, Save
nur mit `B`). Zweiter Fall mit doppeltem bekanntem Feld (Root `schema` 3, dann 2): read-only.
Gegenfall `future_a`/`future_b`: `geladen`, Roundtrip bytegleich. Mutation an der Zeile, die
die Zusage trägt: die neue Duplikaterkennung in `lade` entfernen → der erste Fall reißt.

---

## 4. T3-02-01 — `assistentVersuchStarten` erhöht die Assistentenrevision ohne Obergrenze (W11)

*Befundzeile: `BEFUNDE.md:362` · Skeptikerzeile Phase 10: `:505` · Bündel B11: `:81` ·
Arbeitsübersicht W11: `:814` · Rohbeleg: `roh/phase-02-zustand.md:30-38`*

### a) Behauptung des Audits

Neben den in NAK-283 (F11) geschlossenen Stellen erhöht `assistentVersuchStarten` die
geladene Assistentenrevision ohne Obergrenze; bei `int64max` läuft sie nach erfolgreichem
Experimentbeginn über (`BEFUNDE.md:362`).

### b) Quellkette am HEAD

**Station 1 — der Leser nimmt `int64max` an.** `NakamaState.cpp:2033-2038`:

```
            if (! (revision.isInt() || revision.isInt64())
                || static_cast<juce::int64> (revision) < 1)
            {
                grund = bestandName + ".assistant_step_v1 revision must be at least 1";
                return false;
            }
```

Zuweisung `:2089`. Keine Obergrenze.

**Station 2 — der Wrapper.** `eq-copilot/plugin/src/prozessor/Analyse.cpp:1291-1319`:

```
bool EqCopilotProcessor::assistentVersuchStarten (const juce::String& passageId)
{
    …
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (! zustand.assistent.gesetzt || ! zustand.assistent.offen)
            return false;
    }
    if (! beginneVersuch (passageId))
        return false;
    juce::String versuchId;
    {
        std::lock_guard<std::mutex> l (versuchMutex);
        versuchId = versuchIdAktiv;
    }
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.assistent.experimentId == versuchId)
            return true;
        zustand.assistent.experimentId = versuchId;
        zustand.assistent.revision += 1;
        veraendert = true;
    }
    return assistentAenderungMelden (veraendert);
}
```

**Station 3 — die Nebenwirkungen liegen vor dem Überlauf.** `beginneVersuch` prüft
Main/read-only (`:798`), Passage, laufenden Versuch, Fenster und Pegel und sendet dann,
`:925-931`:

```
    if (! sendeVersuchP0 (json))
        return false;
    ruecknahme.behalten = true;      // ab hier traegt der offene Versuch den Gain
    std::lock_guard<std::mutex> l (versuchMutex);
    versuchIdAktiv = versuchId;
    versuchPassageId = passageId;
    return true;
```

`experiment_begin` liegt dann im P0-Register (`:777-788`), der Versuchsslot ist belegt.
Eine Prüfung erst bei `:1315` käme zu spät.

**Station 4 — die Folgen.** `:1315` ist bei `int64max` vorzeichenbehafteter Überlauf (UB).
`assistentAenderungMelden` (`:1117-1136`) meldet Dirty, erhöht `v3StateRevision` und
sendet den Schritt; `v3AssistantStepJson` schreibt die Revision auf den Draht
(`Ipc.cpp:674`), der Broker verwirft sie (Schema `minimum: 1`,
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1714`). Nach Speichern und Laden fällt der
Stand am Leser (Station 1) in read-only — sofern MSVC umklappt, wie die Mutationsspalte von
NAK-283 M-10 es dokumentiert („der MSVC-Lauf erzeugt `-9223372036854775808`",
`NAK-283.md:606`). Nicht nachgemessen.

**Zum Vergleich: die bewachten Bibliothekswege.** `assistentenrevisionHeben`,
`NakamaState.cpp:2805-2814` (`if (a.revision >= std::numeric_limits<juce::int64>::max())
{ grund = "assistant revision would overflow"; return false; }`), gerufen in `:2831`,
`:2879`, `:2984`. Der Wrapper umgeht sie.

**Erreichbarkeit: Stufe 2.** Kein Bedienelement ruft `assistentVersuchStarten`: außer der
Deklaration (`eq-copilot/plugin/src/PluginProcessor.h:364`) und dem Rumpf gibt es unter
`eq-copilot/plugin/src` keinen Aufrufer. Dazu braucht es einen fremden oder gebauten State
mit offenem Schritt an `int64max`; durch Bedienung ist die Zahl nicht erreichbar.

### c) Zusage wörtlich

- `docs/beweise/NAK-283.md:606` (M-10): „**Kein Überlauf, kein unlesbarer Stand.** Die
  Änderung wird abgewiesen (`veraendert = false`, Grund gesetzt), `a.revision` bleibt
  `int64max`; `a.revision += 1` wird nie auf `int64max` ausgeführt". Die Ereignisspalte
  nennt die drei Bibliothekswege; der fettgedruckte Satz ist allgemein.
- `NAK-283.md:607` (M-11), allgemeiner Satz (oben zitiert).
- `CLAUDE.md:194-196`.
- `docs/beweise/SONDE-014.md:883` (M-88): „Die Revisionsordnung ist total: eine ältere
  Revision überschreibt nach der Koaleszierung nie eine jüngere." Ein Umklappen auf den
  kleinsten Wert macht den jüngsten Schritt zum ältesten.

**Irreführende Kommentare:** `NakamaState.cpp:2796` („`a.revision += 1` stand an drei
Stellen OHNE Schranke") — es gibt eine vierte, `Analyse.cpp:1315`. `PluginProcessor.h:330-332`
(„Jede Aenderung geht denselben Weg: Automat unter dem Bindungsschloss → Host-Dirty →
Wire.") — das Anhängen der Versuchskennung umgeht den Automaten.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B27 `Sonde014IntentTest.cpp:1488-1550` (M-10), Bein `tools/beweise.ps1:773` | die drei Bibliothekswege an `int64max` | der Prozessorwrapper fehlt |
| B2 `StateMigrationTestMain.cpp:2964-3044` (M-11) | Save↔Load am Rand mit `assistentUeberspringen` (`:3009`) | anderer Handgriff |
| B29 `Sonde014AssistentTest.cpp:589-602` (M-62), Bein `:774` | ohne erfüllte Voraussetzungen startet kein Versuch | `beginneVersuch` scheitert dort |
| B23 `Sonde013PassageStateTest.cpp:1170-1186` (C5) | ein **erfolgreicher** `beginneVersuch` | kein Assistent, keine Randrevision |

Kein Fixture trägt eine Assistentenrevision am Rand (NAK-267 A-3 führt
`assistant_step_v1` weiter ohne Fixture, `docs/offene-punkte.md:276`).

### e) Urteil: BESTÄTIGT

Die Kette schließt; alle Zeilen stammen unverändert von `84226d46` (09.09.), weder NAK-283
(älter als die Auditbasis) noch NAK-310 oder NAK-312 haben sie berührt. Ergänzt: Stufe 2,
und die externen Nebenwirkungen liegen vor dem Überlauf.

### f) Kategorie und Schwere

**DEFEKT** gegen den allgemeinen Satz von M-11 und den fettgedruckten Satz von M-10.
**Schwere low** (Audit: high): derselbe Auslöser wie F11, den NAK-283 als P3 führte
(`NAK-283.md:61`, Register `docs/offene-punkte.md:292`: „zwei P3 (F11, F12)"), hier
zusätzlich nur über die API erreichbar; das Ergebnis ist read-only mit Originalbytes und ein
verwaister Versuch.

### g) Kleinster Fix

**Ort:** `assistentVersuchStarten`, `Analyse.cpp:1291-1319`, und eine neue öffentliche
State-Funktion in `NakamaState.cpp`.

Die Obergrenze wird im ersten Sperrblock (`:1297-1301`) geprüft, **bevor** `beginneVersuch`
etwas sendet. Das Anhängen der Versuchskennung samt Inkrement läuft über eine öffentliche
Bibliotheksfunktion, die `assistentenrevisionHeben` (`:2805-2814`) benutzt: prüfen statt
zurückrollen (`NAK-283.md:1704-1708`, Feinheit 12). Die Grenze ist die aus Regelfrage 2
(2^53−1, falls angenommen; sonst `int64max`). Keine State-Version, kein Fixture nötig.

**Mitzuprüfende Beziehungen:** senden↔bestätigen (abgewiesen heißt: kein
`experiment_begin`, kein belegter Slot), speichern↔laden (die Bytes bleiben gleich).

### h) Ort des Rotbeweises

**Bein B29** (`EqCopSonde014AssistentTest`, `eq-copilot/plugin/CMakeLists.txt:535-538`,
Behauptungszeile `tools/beweise.ps1:774`), `Sonde014AssistentTest.cpp` neben M-62
(`:589-602`). Aufbau nach dem Muster C5 (`Sonde013PassageStateTest.cpp:1170-1186`: Main mit
Bindung, Quellenfixture, `prepareToPlay`, Passage, gebundenes Fenster, eingefrorener Pegel)
auf einem State-Mutanten mit offenem Schritt an der Grenze (Bauweise wie M-11,
`StateMigrationTestMain.cpp:2982-2993`). Assertionen:

- `assistentVersuchStarten` liefert `false`, `letzterVersuchP0FuerTest()` bleibt leer (kein
  `experiment_begin`), die Revision bleibt an der Grenze, 0 Host-Dirty;
- Save → Load in eine neue Instanz: nicht read-only;
- Gegenfall Grenze − 1: der Versuch startet, Revision = Grenze, genau ein Dirty.

Mutation an der Zeile, die die Zusage trägt: die neue Vorprüfung entfernen → der Versuch
wird gesendet und die Revision läuft über → rot.

---

## 5. T3-02-03 — State und Draht meinen verschiedene Revisionsbereiche (W11)

*Befundzeile: `BEFUNDE.md:364` · Skeptikerzeile Phase 10: `:507` · Bündel B11: `:81` ·
Arbeitsübersicht W11: `:814` · Rohbeleg: `roh/phase-02-zustand.md:66-79`*

### a) Behauptung des Audits

Persistente Revisionen sind `int64`, der v3-Draht erlaubt höchstens 2^53−1: gültig
geladene Gen-Revisionen ab 9007199254740992 verwirft der Broker, Probeeq meldet solche
`state_revision`-Werte als 9007199254740991 (`BEFUNDE.md:364`).

### b) Quellkette am HEAD

**Gen — drei Felder, ein Weg.**

1. *Leser ohne Obergrenze.* `intent_revision_v1` `NakamaState.cpp:1971-1985` (≥ 1), die
   Revision je `source_intents_v1`-Eintrag `:1819-1829` (≥ 1), die Assistentenrevision
   `:2033-2038` (≥ 1). Seit `f6d80cdf` liest `:1780` aus `MainProject` **oder**
   `RetainedMainProject` (`const auto& mainProject = bestandsknoten;`): dieselben drei
   Felder ruhen jetzt auch in `legacy`.
2. *Sender ohne Grenze.* `Ipc.cpp:583-587`:

   ```
       std::string aus = "{\"type\":\"intent_update\",\"adresse\":"
                       + nakama::ipc::adresseAlsJson (h.adresse)
                       + ",\"session_epoch\":\"" + h.adresse.sessionEpoch + "\""
                       + ",\"bestand_revision\":" + std::to_string ((long long) revision)
                       + ",\"vollstaendig\":" + (vollstaendig ? "true" : "false");
   ```

   `:601` `aus += ",\"revision\":" + std::to_string ((long long) s.revision);` und `:674`
   `+ ",\"revision\":" + std::to_string ((long long) schritt.revision)`. Gesendet wird nur
   in `main` (`:555`, `:659`), nach jedem Verbindungsaufbau (`:1059` →
   `sendeIntentVollbestand`, `:809-825`). `sendeP1` prüft nur die Größe
   (`eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:402-410`).
3. *Broker verwirft die ganze Nachricht, bevor er parst.*
   `broker/src/coordinator/schema.rs:422-423`
   (`crate::vertrag::textriegel_bytes(payload).ok()?;`), `broker/src/vertrag.rs:243-246`:

   ```
        let zu_gross = ganz.len() > 16
            || (ganz.len() == 16 && ganz.parse::<u64>().unwrap_or(u64::MAX) > SICHERE_GANZZAHL);
        if zu_gross {
            return Err(format!("Ganzzahl ausserhalb 2^53-1: {lit}"));
   ```

   mit `SICHERE_GANZZAHL = 9_007_199_254_740_991` (`:127`). `intent_update`:
   `broker/src/coordinator/intent.rs:323-326` → `IntentAbweisung::Vertrag`;
   `assistant_step_update`: `broker/src/coordinator/assistent.rs:90`.
4. *Wirkung.* Die Vollständigkeitsmarke kommt nie an. Der Broker bleibt geschlossen
   (`intent.rs:136-138`, `intent.is_some_and(|b| b.vollstaendig)`): er rechnet nichts
   Falsches, er rechnet für diese Sitzung gar nicht mehr. Das Plugin erfährt davon nichts
   (P1 ohne Quittung).

**Probeeq — ein Feld, still gekappt.**

1. *Leser:* `Dsp.state_revision` `NakamaState.cpp:1412-1417` (≥ 0, keine Obergrenze).
2. *Ladestart:* `SondeProcessor.cpp:1287` `(std::uint64_t) zustand.stateRevision`; der
   Transaktionskern nimmt bis `kHoechsteRevision`
   (`eq-copilot/plugin/state/NakamaTransaktion.h:55`,
   `(std::uint64_t) std::numeric_limits<juce::int64>::max()`; geprüft
   `NakamaTransaktion.cpp:247`) und hält Commits dort an (`:528-530`,
   `"revision_erschoepft"`).
3. *Bericht:* `SondeProcessor.cpp:1397` `s.stateRevision = transaktion->revision();`.
4. *Draht:* `eq-copilot/plugin/core/ipc/controlclient/Intern.h:381-384`:

   ```
   inline std::uint64_t jsonSafe (std::uint64_t wert) noexcept
   {
       return std::min (wert, kJsonSafeModulus - 1);
   }
   ```

   angewandt in `stateReportJson` (`:443`) und `heartbeatAlsJson`
   (`controlclient/Vertrag.cpp:163`). Jede Revision ab 2^53−1 erscheint als
   9007199254740991; verschiedene Stände sind am Broker nicht mehr unterscheidbar
   (gespeichert `broker/src/coordinator/liveness.rs:556`, `:620`; benutzt für den
   Konfliktvergleich `broker/src/coordinator/befehl.rs:341-344`).
5. *Lokal bleibt die ungekappte Zahl:* `Verbindung.cpp:865`
   (`stateReportRevisionEingereiht.store (status.stateRevision);`) und `:1010-1012`; der
   Kommentar `:1004` „JETZT kennt der Broker diesen Stand" stimmt über 2^53−1 nicht.

**Eigene Ergänzung — vier Grenzen leben nebeneinander.** Leser: `int64` (Gen) bzw. ≥ 0
(Dsp); Mutatoren: Halt bei `int64max` (`bestandsrevisionHeben` `NakamaState.cpp:2335-2342`,
`:2390-2392`, `assistentenrevisionHeben` `:2805-2814`, Transaktionskern
`NakamaTransaktion.h:55`) und ein ungeschützter Weg (`Analyse.cpp:1315`, T3-02-01); Draht:
2^53−1 mit Ablehnung (Broker) bzw. Kappung (`Intern.h:383`); der Broker sättigt seine
eigene Sitzungsrevision bei 2^53−1 (`befehl.rs:588-591`, `.saturating_add(1)
.min(9_007_199_254_740_991)`). Gens eigene `v3StateRevision` ist flüchtig und beginnt bei 0
(`eq-copilot/plugin/src/PluginProcessor.h:1440`); sie erreicht 2^53 nicht.

**Zahlenränder.** 9007199254740991 reist; 9007199254740992 (2^53) lässt bei Gen die ganze
Nachricht fallen und wird bei Probeeq zu 9007199254740991; `int64max`
(9223372036854775807) ebenso. Kein Weg setzt eine Revision auf 0 zurück (das tut nur der
ACK-Leser, T3-03-03).

**Erreichbarkeit: Stufe 1 über Hostbytes**, aber nur mit fremdem oder gebautem State:
2^53 Änderungen sind durch Bedienung nicht erreichbar.

### c) Zusage wörtlich

- `docs/beweise/SONDE-014.md:882` (M-86): „Main meldet seinen **vollständigen**
  Intent-Bestand — **auch einen leeren** — mit einer **Vollständigkeitsmarke samt
  Revision**, **bevor** der Broker eine Hypothese oder ein Proposal rechnet."
- `SONDE-014.md:883` (M-88): „Main meldet nach Verbindungsaufbau und nach jedem
  Brokerneustart **Intent-Bestand und aktuellen Schritt**, bevor der Broker rechnet (M-86)."
- State-Vertrag: `nakama-state-v2.md:29` (`intent_revision_v1 int64 optional; … ab 1`),
  `:184` (`revision` `int64` ≥ 1 je Intent), `:187` („`int64` ≥ 1 · Revision des **ganzen**
  Bestands, steigt bei jeder persistenten Änderung genau einmal"), `:38`
  (`state_revision int64 PFLICHT; ≥ 0, steigt mit jeder committeten Transaktion, sinkt nie`).
  Form und Revisionsbereich von `assistant_step_v1` stehen im Vertragstext **nicht**
  (nur genannt in `:68` und `:103`).
- v3-Vertrag: `eq-copilot/schemas/v3/README.md:62-66`: „**Parser-sichere
  Dezimalpraezision:** mathematische Ganzzahlen nur innerhalb ±(2^53−1) …". Die Felder
  selbst tragen nur Untergrenzen (`eq-ipc-v3.schema.json:931`, `:1671-1675`, `:1632`,
  `:1714`, `:2240`); „Warum kein `maximum` je Feld" steht in `README.md:54-57`.
- Gate S25l, `docs/plan/plan.json:369`: „keine stillen Rundungen". Regel der
  Arbeitsübersicht W11 (`BEFUNDE.md:814`, Auditvorschlag, kein Userentscheid): „keine stille
  Rundung oder Rücksetzung auf 0".
- `CLAUDE.md:178-181`: „**Schemas sind Verträge.** … Für v3 sind Discriminator,
  Zieladresse, Revision und Capability nicht additiv erweiterbar."
- Der Entwurf schweigt zum Zahlenbereich der Revisionen (Suche nach „2^53", „Ganzzahl",
  „int64", „Überlauf" in `docs/FL-Nakama-Sonden-Design-Entwurf.md`: kein Treffer zu
  Revisionen). `docs/beweise/NAK-310.md` enthält keine Zeile zu `state_revision` oder zur
  ACK-Revision (einziger Treffer `:253-256`, das Zitat von `CLAUDE.md:178-181`).
- `CLAUDE.md:189-190` (NaN-Ehrlichkeit) betrifft nicht-endliche Werte und trägt den Fall
  nicht.

**Irreführende Texte:** `Ipc.cpp:543-546` („Die Zahlen entstehen mit `juce::String` und
nicht mit `std::to_string`") — `:586`, `:601`, `:674` benutzen `std::to_string` (für
Ganzzahlen folgenlos, aber falsch beschrieben). `Intern.h:51` nennt die Konstante
`kJsonSafeModulus`, `:383` kappt damit, `Verbindung.cpp:870` rechnet Modulo. `Verbindung.cpp:1004`
(oben).

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was nicht |
|---|---|---|
| B27 `Sonde014IntentTest.cpp:1321-1550` (M-07 bis M-10) | Bibliothekswege an `int64max` | keinen Draht |
| B2 `StateMigrationTestMain.cpp:2964-3044` (M-11) | Persistenz bei `int64max` | keinen Draht |
| B10 `IpcTestMain.cpp` | `state_revision` nur als 7 (`:4436`, `:6059`) | keine Kappung, keinen Rand |
| A5/A8 v3-Korpus | Revisionswerte nur 0, 4, 12, 13 (Suche über `eq-copilot/fixtures/v3`) | `ungueltig/zahl-ueber-2hoch53.json` betrifft `heartbeat.sequence` (`fixtures/v3/MANIFEST.json:4224-4229`) |
| A4 (`tools/beweise.ps1:512-518`, `cargo test` über `broker/Cargo.toml`) | Textriegel allgemein | kein vom Plugin gebautes `intent_update` mit 2^53 |
| B7 `TransactionTestMain.cpp` | Transaktionskern | `revision_erschoepft` und `kHoechsteRevision` in keinem Test gefunden |

Kein State-Golden trägt eine Revision am Rand: `dsp-v2-voll.bin` trägt 12 und im Undo 11
(`StateMigrationTestMain.cpp:1523`, `:1533`), `main-intent-v1.bin` kleine Werte
(`:1360-1376`).

### e) Urteil: BESTÄTIGT

Alle Zeilen stehen wie im Audit (`Ipc.cpp` von `7e2780aa`, 09.09.; `Intern.h` von
`053ff7f8`, 09.09.; `vertrag.rs` von `863caca9`, 21.08.). Ergänzt: (1) der Broker verwirft
die **ganze** Nachricht, nicht nur das Feld, und bleibt geschlossen; (2) vier Grenzen leben
nebeneinander; (3) NAK-312 Etappe 7b (`f6d80cdf`) trägt die drei Gen-Felder zusätzlich im
ruhenden Bestand; (4) die Form von `assistant_step_v1` fehlt im Vertragstext.

### f) Kategorie und Schwere

**DEFEKT mit Regelbedarf**: M-86 und M-88 sind für einen vom eigenen Leser angenommenen
Stand nicht erfüllbar, und die Kappung der Sonde ist eine stille Rundung gegen das Gate. Die
Reparatur braucht eine Regel für den Bereich (Regelfrage 2: Technik). **Schwere medium**
(Audit: high): nur gebaute Stände erreichen den Rand; Gen bleibt geschlossen, die Sonde
meldet aber eine falsche Zahl.

### g) Kleinster Fix

Nach der Regel aus Regelfrage 2 (ein Bereich, Obergrenze 2^53−1):

1. **Leser** (`NakamaState.cpp:1412-1417`, `:1819-1829`, `:1971-1985`, `:2033-2038`, dazu
   die Undo-Revision `:1480-1482`): über 2^53−1 → read-only mit Originalbytes, der Grund
   nennt das Feld.
2. **Mutatoren** halten bei 2^53−1 (`:2335-2342`, `:2390-2392`, `:2805-2814`,
   `NakamaTransaktion.h:55`, dazu `Analyse.cpp:1315` über T3-02-01).
3. **Sender** ohne Kappung: `jsonSafe` fällt für `state_revision` (`Intern.h:443`,
   `Vertrag.cpp:163`); der Bereich garantiert die Grenze. Ein Wert darüber wäre ein
   Programmfehler und würde nicht gesendet, nie gekappt.
4. **Headroom-Kandidat** auf die neue Grenze (`NakamaState.cpp:1275`, `:1297`, `:1304`,
   `:1319`, `:1331`).
5. **Vertragstext** `nakama-state-v2.md:29`, `:38`, `:184`, `:187` mit der Grenze; Form und
   Revisionsbereich von `assistant_step_v1` nachtragen.
6. **NAK-283 M-10 und M-11** werden umgeschrieben: ihr Rand wandert von `int64max` auf
   2^53−1, und ein Stand bei `int64max` lädt künftig read-only. Das ist eine dokumentierte
   Matrixänderung im Manifest von NAK-313, keine stille Testkorrektur.

Keine neue State-Version (Begründung unter Regelfrage 2). Neue Vektoren: v3-Fixtures mit
2^53−1 (gültig) und 2^53 (ungültig) je Revisionsfeld für A5/A8, dieselben durch den C++-Leser
(B10) und den Rust-Leser (A4).

**Mitzuprüfende Beziehungen:** speichern↔laden, senden↔bestätigen
(`state_report` gegen ACK-Revision und Konfliktvergleich).

### h) Ort des Rotbeweises

- **B27** (`EqCopSonde014IntentTest`, `CMakeLists.txt:888-890`, `tools/beweise.ps1:773`)
  neben M-07 bis M-12: ein Stand mit `intent_revision_v1 = 2^53` lädt read-only — **heute
  rot** (`geladen`); mit 2^53−1 lädt er, und `v3IntentUpdateFuerTest (true)`
  (`PluginProcessor.h:366-367`) trägt 9007199254740991 und passiert
  `nakama::vertrag::textriegelBytes`. Dasselbe für die Intent-Revision und in **B29** für den
  Assistenten.
- **B7** (Transaktionskern): `Dsp.state_revision = 2^53` lädt read-only; bei 2^53−1 weist ein
  Commit mit `revision_erschoepft` ab, und `state_report` trägt die Revision unverändert.
- **A4**: derselbe Vektor durch `intent_update_json_grund_fuer_test`
  (`broker/src/coordinator/intent.rs:742`).

Mutation an der Zeile, die die Zusage trägt: die neue Obergrenze im Leser (etwa bei
`NakamaState.cpp:1979-1984`) entfernen → der Stand lädt schreibbar, die erzeugte Nachricht
fällt am C++- und am Rust-Textriegel → rot.

---

## 6. T3-03-03 — `command_ack` ohne Rohzahlgrenze; Überlauf wird Revision 0 (W11)

*Befundzeile: `BEFUNDE.md:380` · Skeptikerzeile Phase 10: `:514` · Bündel B11: `:81` ·
Arbeitsübersicht W11: `:814` · Rohbeleg: `roh/phase-03-vertrag.md:22`, `:111`, `:164`*

### a) Behauptung des Audits

Am `command_ack`-Eingang fehlt die v3-Rohzahlgrenze: ein Erfolgs-ACK mit Revision
9007199254740992 wird angenommen, noch größere Ziffernfolgen werden über den
`stoull`-Überlauf zu Revision 0, bleiben gültiges Erfolgs-ACK, und `inFlightAck` gibt den
Auftrag frei (`BEFUNDE.md:380`).

### b) Quellkette am HEAD

**Station 1 — das ACK geht vor jedem Produktrückruf ins Register.**
`controlclient/Verbindung.cpp:832-842`:

```
            const std::string antwort (reinterpret_cast<const char*> (e.payload),
                                       e.payloadLaenge);
            inFlightAck (antwort);
            // Nach `stop()` wird kein Callback mehr gerufen (`B-CC-10`).
            if (! sollAbbrechen (generation))
            {
                if (beiVersionierterAntwort)
                    beiVersionierterAntwort (antwort, e.kopf.schemaMinor);
```

**Station 2 — die Syntaxwache kennt kein Maximum.** `Intern.h:81-91`
(`nichtnegativeJsonGanzzahl`: „0" oder Ziffern ohne führende Null) und `:206-217`:

```
inline CommandAckArt commandAckArtLesen (const std::string& text, std::string& commandId,
                                  std::uint64_t* revisionAus = nullptr)
{
    std::vector<JsonFeld> felder;
    std::string typ, ergebnis, revision;
    if (! flachesJsonObjekt (text, felder)
        || ! jsonText (felder, "type", typ) || typ != "command_ack"
        || ! jsonText (felder, "command_id", commandId) || ! istHex32 (commandId)
        || ! jsonText (felder, "ergebnis", ergebnis)
        || ! jsonLiteral (felder, "state_revision", revision)
        || ! nichtnegativeJsonGanzzahl (revision))
        return CommandAckArt::keinAck;
```

**Station 3 — der Überlauf wird 0.** `:237-250`:

```
    // SONDE-014 WN-01: der Broker nennt im ACK die Revision, die er KENNT.
    // Genau sie traegt der frische Kopf einer Wiederholung - keine geratene
    // und keine lokal hochgezaehlte.
    if (revisionAus != nullptr)
    {
        try
        {
            *revisionAus = std::stoull (revision);
        }
        catch (...)
        {
            *revisionAus = 0;
        }
    }
```

Die Art hängt davon nicht ab (`:252-297`). **Zahlenränder:** 9007199254740991 korrekt;
9007199254740992 (2^53) angenommen; 18446744073709551615 (2^64−1) angenommen mit
`UINT64_MAX`; ab 18446744073709551616 (2^64) wirft `stoull`, Revision 0.

**Station 4 — Freigabe und Erfolg.** `Nachrichten.cpp:297-303` liest mit
`&brokerRevision`; bei `angewandt` entfernt `:362-363` den Eintrag
(`if (! wiederholt) inFlight.erase (eintrag);`), `:383-384` ruft
`hookAuftragAbgeschlossen`, `:395-397` zählt `++zustand.inFlightErfolg`.

**Eigene Ergänzung 1 — die 0 reist weiter.** Bei `konflikt` baut `:332-337` den Auftrag
mit `brokerRevision` neu (`hookKonfliktWiederholung (commandId, eintrag->json,
brokerRevision)`, höchstens drei Mal, `Laufzeit.h:213`). Der Hook
(`eq-copilot/plugin/src/PluginProcessor.cpp:442-450` → `Ipc.cpp:779-794` →
`controlclient/Vertrag.cpp:73-131`) ersetzt `base_revision` im gesendeten Urteil durch 0
(bzw. durch 9007199254740992, das dann der Textriegel der Gegenstelle abweist).

**Eigene Ergänzung 2 — ein Erfolgs-ACK verändert den Produktzustand.** `Ipc.cpp:1063-1078`:
`commandAckHaeltVertrag` (`Vertrag.cpp:49-60`, derselbe Leser) liefert `erfolgreich`, der
Quellenbefehl wandert nach `bestaetigteSourcesCommands` und wird beim nächsten Abholen
angewandt (etwa `State.cpp:97-109`) — Mitgliedschaft, Dirty, Revision.

**Eigene Ergänzung 3 — die eigene Gegenstelle sendet so ein ACK nie.**
`broker/src/coordinator/befehl.rs:60-63` prüft jedes ACK vor dem Senden gegen denselben
Vertrag (`v3_nachricht_lesen(&payload, "command_ack").is_some().then_some(payload)`); die
Revisionen, die der Broker nennt, stammen aus textgeprüften Berichten oder sind bei 2^53−1
gesättigt (`:588-591`). Erreichbar ist der Weg nur über eine nicht vertragstreue
Gegenstelle.

### c) Zusage wörtlich

- `eq-copilot/schemas/v3/README.md:62-66` (Regel 2, oben) und `:50-52`: „Das ist kein
  Randfall der Fehlermeldung, sondern ein **umgeklappter Wert**: `sequence`,
  `state_revision`, `transport_epoch` und `base_revision` sind Monotonie- und
  Revisionsfelder."
- Entwurf §33.1, `docs/FL-Nakama-Sonden-Design-Entwurf.md:2040-2042`: „Discriminator,
  Zieladresse, Revision, Capability und sicherheitsrelevante Felder sind **nicht** additiv
  und erzwingen bei unbekannter Bedeutung Ablehnung."
- `CLAUDE.md:178-181` (oben).
- `docs/plugin-wissen.md:491-495`: „Erst `command_ack.ergebnis = angewandt` oder
  `idempotent_wiederholt` beendet den Auftrag erfolgreich" — setzt ein vertragstreues ACK
  voraus.
- Testname als Zusage: `IpcTestMain.cpp:6025` „nur_schemafestes_command_ack_gibt_inflight_frei".
- W11 (`BEFUNDE.md:814`, Auditvorschlag): „keine stille Rundung oder Rücksetzung auf 0".

**Irreführende Kommentare:** `Intern.h:237-239` („keine geratene") — bei Überlauf ist die 0
geraten; `Intern.h:276` („der Leser ist genau so streng wie der Vertrag") — für die
Rohzahlgrenze gilt das nicht.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was nicht |
|---|---|---|
| B10 `IpcTestMain.cpp:5966-6026`, Bein `tools/beweise.ps1:738` (`CMakeLists.txt:977-978`) | drei Brüche: führende Null, falscher Hash-Typ, fehlender Hash (`:5982-5986`) | keinen Überbereich |
| Testserver `V3TestServer.h:705-709` | `state_revision` fest `"7"` oder `"01"` | keinen Randwert |
| B3c (`tools/beweise.ps1:702`) | Korpus über `Schema::pruefe` und Textriegel | nicht den Produktleser (NAK-267 A-2, `docs/offene-punkte.md:276`) |
| A5/A8 | `command_ack`-Fixtures mit Revision 12/13 | kein Randwert |
| A4 | Brokerwriter | Test mit Überbereich im ACK nicht gefunden |

### e) Urteil: BESTÄTIGT

Die Zeilen stammen unverändert von `053ff7f8` und `c447fc4e` (09.09.); `controlclient/`
hat seit der Auditbasis keinen Diff. Ergänzt: die 0 reist in einen Wiederholungskopf, ein
Erfolgs-ACK verändert dauerhaft den Produktzustand, und die eigene Gegenstelle erzeugt ein
solches ACK nicht.

### f) Kategorie und Schwere

**DEFEKT** gegen Regel 2 des v3-Textriegels und Entwurf §33.1. **Schwere medium** (Audit:
high): die Folge ist ein falscher Erfolg mit dauerhafter Wirkung, aber nur eine nicht
vertragstreue Gegenstelle löst sie aus.

### g) Kleinster Fix

**Ort:** `commandAckArtLesen`, `Intern.h:206-297`.

`state_revision` wird nur als nichtnegative JSON-Ganzzahl bis 2^53−1 angenommen: Grenze am
Literal vor jeder Umwandlung (Länge und lexikalischer Vergleich; `kJsonSafeModulus` steht in
`:51` schon bereit), dann eine Umwandlung ohne Ausnahmefall, unabhängig davon, ob der
Aufrufer die Revision wissen will. Jeder Fehlschlag heißt `keinAck` — der `catch`-Zweig mit
0 entfällt. `commandAckHaeltVertrag` erbt das. Kein neuer Abhängigkeitspfeil nach
`plugin/vertrag/` (der ControlClient bindet `NakamaVertrag.h` heute nicht ein), keine
Schemaänderung. Mit Teil B im selben Änderungssatz: T3-03-02 (Zahlenformen `1.0`/`1e0`)
betrifft genau diese Funktion. Die Kommentare `:237-239` und `:276` ziehen mit.

**Mitzuprüfende Beziehung:** senden↔bestätigen — ein abgewiesenes ACK lässt den Auftrag im
Register, nach dem Reconnect wird er unter derselben `command_id` wiederholt.

### h) Ort des Rotbeweises

**Bein B10** (`EqCopIpcTest`), `IpcTestMain.cpp` neben `:5982-5986`, mit zwei neuen Brüchen
im Testserver (`V3TestServer.h:705-709`: Revision `9007199254740992` und
`18446744073709551616`), je mit `ackArt` 1 (`angewandt`) und 3 (`konflikt`). Assertionen:
Auftrag bleibt offen (`inFlight == 1`, `inFlightErfolg == 0`,
`inFlightEndgueltigOhneErfolg == 0`), bei `konflikt` keine Wiederholung
(`inFlightWiederholungen == 0`, kein Kopf mit `base_revision` 0); nach dem Reconnect mit
gültigem ACK frei — **heute rot**. Dazu direkt: `commandAckHaeltVertrag` mit 2^53 falsch,
mit 2^53−1 wahr. Für die Sprachgleichheit dieselben Vektoren als v3-Fixtures
(`command_ack` mit 2^53−1 gültig, mit 2^53 und 2^64 ungültig) durch A5/A8 und A4. Mutation
an der Zeile, die die Zusage trägt: die neue Grenze in `commandAckArtLesen` entfernen → rot;
den `catch`-Zweig auf 0 zurückstellen → der 2^64-Fall reißt.

---

## Gemeinsame Ursachen

**Wurzel A — der Leser nimmt mehr an, als Schreiber oder Draht zurückgeben können.**
T3-02-02 (Sammlungsgrenze gegen Writer-Zusatz), T3-02-05 (Duplikate, die JUCE nicht
zurückschreiben kann), T3-02-03 und T3-02-01 (Revisionen, die der Draht nicht trägt bzw. das
Inkrement nicht verträgt). Ein Grundsatz trägt alle vier: **angenommen heißt vollständig und
verlustfrei rückschreibbar — sonst read-only mit Originalbytes.** Die Änderungssätze
bleiben trotzdem getrennt:

- **Satz „Annahme = Rückschreibbarkeit"** (T3-02-02, T3-02-05): dieselbe Funktionsfamilie
  (`lade`, `ValueTreeByteRiegel`, `hatWriterHeadroom`), dieselbe Testbauart (Rohbytes in
  B2), derselbe Vertragsabschnitt (`nakama-state-v2.md` §5). Ein Satz.
- **Satz „ein Revisionsbereich"** (T3-02-03, T3-02-01, T3-03-03): eine Regel, drei Orte —
  State (Leser, Mutatoren, `Analyse.cpp:1315`), Sender (`Intern.h:443`, `Vertrag.cpp:163`),
  ACK-Leser (`Intern.h:206-297`). Regel und Vertragstext zuerst, dann die drei Orte; die
  Matrixumschrift von NAK-283 M-10/M-11 gehört in denselben Satz.

**Wurzel B — Rückschreiben ohne Handgriff.** T3-02-04 allein: das Panel schreibt seine
Kopie aller Felder zurück, ob geändert oder nicht. Eigener Satz in `PluginEditor.cpp` und
`State.cpp:316`, eigene Beine (B15, B2).

**Nur die Datei geteilt:** T3-02-02 und T3-02-01 liegen in `NakamaState.cpp`, aber in
verschiedenen Funktionen; T3-02-04 teilt `State.cpp` (`setzeBindung`) mit dem ruhenden
Bestand aus NAK-312 7b, nicht aber dessen Logik.

**Berührung mit Teil B.**

- *T3-02-03 ↔ T3-03-02 und T3-03-08.* T3-03-02 (`BEFUNDE.md:379`): Produktleser nehmen
  `1.0`/`1e0` nicht an; der ACK-Leser ist einer davon (`Intern.h:81-91` verlangt reinen
  Zifferntext). Eine Grenze für T3-03-03 muss nach derselben Zahlnormalisierung greifen, die
  Teil B für T3-03-02 festlegt (`9007199254740992.0` liegt ebenfalls darüber) — **ein**
  Zahlenleser im ACK, ein Änderungssatz. T3-03-08 (`:385`, v2-Überlauf vor der Grenze in
  `PipeClient.cpp`) folgt demselben Satz „Grenze am Literal vor jeder Akkumulation", liegt
  aber in anderem Code; getrennt.
- *T3-02-05 ↔ T3-03-09.* T3-03-09 (`:386`): JSON-Duplikate werden je Eingang verschieden
  behandelt. Ein Grundsatz für beide — **ein doppelter Name wählt nie still einen Wert** —,
  mit mediumgerechter Folge: auf dem Draht ablehnen (nichts Persistentes steht auf dem
  Spiel, das Produkt tut es schon in `IpcVerbindung.cpp:515-518`), im State read-only mit
  Originalbytes (Ablehnung hieße dort `ignoriert` und damit Datenverlust beim nächsten
  Speichern).

**Berührung mit NAK-312 Etappe 4 am Panel-Destruktor.** Der Fix zu T3-02-04 sitzt in
derselben Funktion wie die Lebendprüfung R-312-2 (`PluginEditor.cpp:804-806`) und die
Testmarke (`:812-815`). Die Lebendprüfung bleibt die erste Zeile; die Unverändert-Prüfung
kommt danach und vor jedem Prozessorzugriff. M-35 bis M-37 (`NAK-312.md:846-848`) bleiben
gültig, M-37 gilt danach auch mit langem Label. Karte U57 (`docs/plan/fragen.json:64-72`)
bleibt offen und unberührt: nach dem Ende des Editors wird weiterhin nichts übernommen.

**Registerfolgen (für den Dirigenten).** T3-02-02 entscheidet den Verdacht NAK-252
(`docs/offene-punkte.md:261`) als Defekt. T3-03-03 verlangt keine Verdrahtung von
`Schema::pruefe` im Produktclient; der Entscheid zu NAK-267 A-2 („Härtung, kein Defekt",
`:276`) bleibt damit unberührt.

**Irreführende Texte, die in dieselben Änderungssätze gehören:**
`PluginEditor.cpp:793-795` (T3-02-04); `nakama-state-v2.md:58-59`, `:297-298`, `:305-307`
(T3-02-02); `NakamaState.cpp:2796`, `PluginProcessor.h:330-332` (T3-02-01);
`Ipc.cpp:543-546`, `Intern.h:51`, `Verbindung.cpp:1004` (T3-02-03); `Intern.h:237-239`,
`:276` (T3-03-03); `docs/plugin-wissen.md:1460-1461` und die Behauptungszeile B15
`tools/beweise.ps1:752` („ohne Aenderung mit keiner"), die heute mehr behaupten, als M-37
misst.

---

## Regelfragen

### 1. Duplikatregel für mehrdeutige Fremdbytes im binären State (T3-02-05)

**Quellenlage.** Keine Quelle nennt doppelte Eigenschaftsnamen im binären State. Vier Sätze
legen die Antwort fest: `CLAUDE.md:194-196` („State bleibt verlustfrei … read-only mit
Originalbytes"), `nakama-state-v2.md:10` (unbekannte Eigenschaften überleben den
Roundtrip), `:152` (unbekannte Eigenschaft wird erhalten) und `:307-311` (was JUCE nicht
byteverlustfrei zurückschreiben kann, macht eine bekannte Wurzel read-only). Der
Kartenentwurf PF11-14 (`BEFUNDE.md:302-307`) nennt als Default schon „binären State vor
mutierbarer Übernahme abweisen beziehungsweise Originalbytes read-only erhalten, niemals
still einen Wert verlieren. Technik legt den kompatiblen Weg fest." — ein Auditvorschlag,
kein Entscheid.

**Die drei Wege im Vergleich.**

- *Verlustfrei erhalten und schreibbar laden:* mit JUCEs `ValueTree` nicht möglich — er
  kann zwei gleichnamige Eigenschaften nicht halten; es bräuchte einen eigenen Baum.
  Unverhältnismäßig.
- *Vor mutierbarem Laden abweisen* im heutigen Sinn (`ignoriert`): die Instanz behält ihren
  frischen Zustand, das nächste Speichern überschreibt die Projektbytes — Datenverlust,
  gegen `CLAUDE.md:194`.
- *Read-only mit Originalbytes* bei bekannter Wurzel: verlustfrei, audio-neutral, der
  Editor nennt den Grund (`nakama-state-v2.md:292`). Genau der Weg des Markerfalls.

**Technik oder Produkt: Technik.** Der User erzeugt solche Bytes nie; kein Produktweg
schreibt sie. Von den drei Wegen hält nur einer das feste Gesetz „State bleibt
verlustfrei", und seine sichtbare Folge (ein gesperrter Messpunkt mit ehrlichem Grund) ist
dieselbe wie bei jedem anderen unlesbaren Stand, etwa einem unbekannten Major — ein
Verhalten, das es heute schon gibt. Eine Karte hätte nur einen Weg, der das Gesetz einhält;
der andere wäre „deine Daten werden still verändert". Das ist keine echte Wahl.
**Kein Kartenentwurf.** Die Regel für den Dirigenten: *Ein doppelter Eigenschaftsname in
einem Knoten macht eine bekannte Wurzel read-only mit Originalbytes; eine fremde Wurzel
bleibt ignoriert.* Für JSON auf dem Draht (T3-03-09, Teil B) folgt aus demselben Grundsatz
die Ablehnung.

### 2. Persistenter Revisionsbereich (T3-02-03)

**Was heute festgeschrieben ist.**

- *State-Vertrag:* `int64` — `intent_revision_v1` ≥ 1 (`nakama-state-v2.md:29`, `:187`),
  Revision je Intent ≥ 1 (`:184`), `Dsp.state_revision` ≥ 0 (`:38`); für
  `assistant_step_v1` steht kein Bereich im Text (nur `:68`, `:103`).
- *Leser:* keine Obergrenze (`NakamaState.cpp:1416-1417`, `:1824-1829`, `:1979-1984`,
  `:2033-2038`).
- *Mutatoren:* Halt bei `int64max` (`:2335-2342`, `:2390-2392`, `:2805-2814`;
  `NakamaTransaktion.h:55`); ein ungeschützter Weg (`Analyse.cpp:1315`).
- *NAK-283 M-10/M-11* (`NAK-283.md:606-607`): `int64max` ist der angenommene, rundlaufende
  Rand; B2 prüft das (`StateMigrationTestMain.cpp:2999-3042`).
- *Fixtures:* kein State-Golden am Rand; v3-Korpus nur mit kleinen Revisionen.
- *Draht:* Untergrenzen im Schema, Obergrenze ±(2^53−1) im Textriegel
  (`README.md:62-66`; C++ `eq-copilot/plugin/vertrag/NakamaVertrag.h:50-51`; Rust
  `broker/src/vertrag.rs:126-127`, `:243-246`).

**Was gelebt wird.** Gen sendet `int64` ungekappt, der Broker verwirft die Nachricht; die
Sonde kappt still auf 2^53−1; der ACK-Leser nimmt jede Ziffernfolge und macht aus dem
Überlauf 0; der Broker sättigt seine eigene Revision bei 2^53−1.

**Vorschlag (Technik).** Ein Bereich für alle persistenten Revisionen und ihre Drahtform:
Untergrenze wie heute (0 bzw. 1), **Obergrenze 2^53−1**. Der Leser hält einen Stand darüber
read-only mit Originalbytes; die Mutatoren halten an der Grenze (Halt, keine Sättigung,
kein Umklappen, keine 0); die Sender geben die Zahl unverändert aus; die Empfänger lehnen
einen Wert darüber als Vertragsbruch ab. Die Alternative — `int64` behalten und
verlustfrei übertragen, etwa als Zeichenkette — änderte die Drahtform eines nicht
additiven Revisionsfeldes (`CLAUDE.md:180-181`, Entwurf `:2040-2042`) und bräuchte ein
neues v3-Major. Unverhältnismäßig.

**Neue State-Version oder Migration: nein.** (1) Kein Produktweg hat im Gebrauch je eine
Revision über 2^53−1 geschrieben: jede Änderung zählt +1, und alles Höhere stammt aus einem
vorher geladenen fremden Stand. (2) Die Verengung schiebt solche Stände nach read-only mit
Originalbytes — verlustfrei, dieselbe Behandlung wie ein unbekanntes Major. (3) Der
Vertragstext bekommt die Grenze als datierte Präzisierung, wie `nakama-state-v2.md:152` es
für die Kind-Matrix vormacht. „Schemas sind Verträge" bleibt gewahrt: Altstände aus normalem
Gebrauch laden unverändert, unbekannte Felder bleiben unberührt, Save und Load werden
gemeinsam geprüft. **Folge, die ausdrücklich zu beschließen ist:** NAK-283 M-10 und M-11
wechseln ihren Rand von `int64max` auf 2^53−1; ein Stand bei `int64max` lädt danach
read-only statt schreibbar. Das ist eine Matrixänderung mit Beleg, kein stilles Umbiegen
eines grünen Tests.

**Technik oder Produkt: Technik.** Kein User erreicht 2^53 Revisionen (rund 9 × 10^15
Änderungen); betroffen sind nur fremde oder gebaute Stände, und im Gebrauch verhält sich das
Produkt unter jedem Weg gleich. Die Regel W11 („keine stille Rundung oder Rücksetzung auf
0") ist mit dem Vorschlag erfüllt. **Kein Kartenentwurf.**

---

## Reihenfolge

Maßstab ist die Regel der Arbeitsübersicht (`BEFUNDE.md:796`): Zustandsverlust vor
Dokumentationshärtung; dazu, was keine Regel braucht, vor dem, was eine braucht.

**Etappe 1 — T3-02-04 (Editor, keine Regel nötig).** Höchste Schwere, gewöhnlicher
Handgriff, kleiner Satz (`PluginEditor.cpp` `uebernehmen`, Feldgrenzen, Kommentar;
`State.cpp:316`). Beine B15 und B2. Berührt keine Datei der Etappen 2 und 3 außer
`State.cpp`, deren Stelle dort niemand sonst ändert — kann parallel zu Etappe 2 laufen.

**Etappe 2 — Regel „Annahme = Rückschreibbarkeit" und Duplikatregel festschreiben;
T3-02-02 und T3-02-05.** Ein Änderungssatz in `NakamaState.cpp` (`hatWriterHeadroom`,
Byte-Riegel, `lade`) mit Vertragstext §5 (Headroom-Satz, Tiefe statt Knoten,
Duplikatfall). Beine B2 und A12 (kleines Negativ-Golden).

**Etappe 3 — Regel „ein Revisionsbereich" festschreiben; T3-02-03, T3-02-01, T3-03-03.**
Innere Folge: (i) Vertragstext und Grenzkonstante samt Matrixumschrift von NAK-283 M-10/M-11;
(ii) State: Leser, Mutatoren, Transaktionskern, `Analyse.cpp:1315` (T3-02-01);
(iii) Sender ohne Kappung; (iv) ACK-Leser (T3-03-03) **gemeinsam mit Teil B T3-03-02** in
`commandAckArtLesen`. Beine B2, B27, B29, B7, B10, A4, A5/A8 (neue v3-Vektoren), B3c. Nach
Etappe 2, weil beide `NakamaState.cpp` ändern und der Headroom-Kandidat hier nur noch seine
Grenzwerte tauscht.

**Sofort machbar, unabhängig von der Folge:** keine eigene Etappe für die irreführenden
Kommentare — sie gehören in den Satz, der ihre Stelle repariert.

---

## Nicht geprüft

- **Kein Lauf.** Der Auftrag verbot Bau, Test, Kanon, Skript und Fehlerinjektion. Alles
  oben ist Quellenlektüre. Die Byteanzahlen des Audits zu T3-02-02 (917.605 und 917.615)
  habe ich nicht nachgerechnet; das Urteil stützt sich auf die Eigenschaftszahl.
- **Das Überlaufergebnis von T3-02-01** stammt aus der Mutationsspalte von NAK-283 M-10
  (`NAK-283.md:606`); vorzeichenbehafteter Überlauf ist in C++ undefiniert, ich habe kein
  Ergebnis gemessen.
- **Der lokale JUCE-Stand** unter `eq-copilot/build/_deps/juce-src/` ist ein Bauartefakt; dass
  er dem gepinnten Stand entspricht, habe ich nicht geprüft. Die zitierten JUCE-Zeilen sind
  „der Stand, der hier liegt".
- **Ob echte Altprojekte Labels über 120 oder Paarnamen über 60 Codepoints tragen**, ist aus
  diesem Repo nicht bestimmbar: die 120er-Kürzung im Panel steht schon im Einzugscommit
  `31c41244` (18.08.2026); die Vorgeschichte im FL-Studio-Repo habe ich nicht gelesen.
- **Laden bei offenem Messpunktpanel:** schließt das Panel nach einem Hostladevorgang mit
  seiner alten Rollenwahl, schriebe es die alte Rolle über den frisch geladenen Stand
  (hergeleitet aus `:757-759` und `:802-818`). Ob FL einen State laden kann, während das
  Popover offen ist, habe ich nicht geprüft. Der Fix aus 2 g) (nur Geändertes, auf den
  aktuellen Stand angewandt) deckt den Fall mit ab.
- **Ob FL beim Schließen des Fensters den Fokus vorher abgibt** (Karte U57), nicht geprüft.
- **Die Reaktion des Brokers auf ein verworfenes `intent_update`** über
  `IntentAbweisung::Vertrag` hinaus (etwa eine Fehlermeldung an das Plugin) habe ich nicht
  verfolgt.
- **Der Verlauf nach einem `konflikt`-ACK mit 2^53** (Wiederholung mit einer
  `base_revision`, die die Gegenstelle abweist) ist nur bis `kKonfliktWiederholungenMax = 3`
  (`Laufzeit.h:213`) gelesen, nicht bis zum Endzustand des Registers.
- **Die nicht vertragstreue Gegenstelle** zu T3-03-03 berührt die Pipe-Vertrauensgrenze;
  das ist Sicherheitsarbeit (T3-12-\*) und nicht Gegenstand.
- **Teil B und Teil C** sind nicht validiert; ihre IDs stehen oben nur als Berührung aus den
  Befundzeilen.
- **Der Entwurf** ist nur an den Stellen zu Revision, ACK, State und Migration gelesen
  (§33.1, §33.5, `:3042-3065`, §53.8), nicht vollständig; `docs/beweise/NAK-310.md` nur nach
  Revision und ACK durchsucht.
- **`bestandsknotenFuer`** (Kandidat für die Klasse `main` aus einem Legacy-Stand) habe ich
  nicht im Einzelnen gelesen; für die Aussagen zu T3-02-02 trägt das nicht, weil `passt` in
  jedem Fall nur Bytes prüft.
- **Kein Dokuriegel gelaufen.** `tools/plan/dokuriegel.py` ist ein Skript; diese Datei ist
  ein Rohbeleg unter `docs/beweise/roh/`, kein Plandokument.

---

FERTIG Teil A, 5 bestätigt, 1 präzisiert, 0 widerlegt.
