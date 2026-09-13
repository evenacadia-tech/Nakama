# NAK-286 — Validierung der Codex-Matrixprüfung 1, Teil C (lesend)

**Ticket:** NAK-286 (Plan S25e, Laufzeit-Arm), Etappe 1 — Verhaltensmatrix und
Bauplan in `docs/beweise/NAK-286.md`.
**HEAD:** `a4c7128b` (Zweig `master`).
**Datum:** 14.09.2026 laut Auftrag des Dirigenten; geschrieben am 13.09.2026 um
23:5x Uhr Ortszeit (die Sitzung läuft in den 14.09. hinein).
**Umfang:** Befunde 6, 7, 8, 11, 12, 14 der Rohdatei
`docs/beweise/roh/NAK-286-matrixpruefung-1-0523c07.txt` (Diagnose-Briefkasten im
Plugin und im Broker).
**Vorgehen:** nur gelesen. Nichts gebaut, kein Test, kein Skript, keine
Installation; Git nur lesend (`git diff --stat`, `git ls-files --eol`); geändert
wurde ausschließlich diese Datei. Die fremden untracked Ordner `briefing-hub/`
und `nimbalyst-local/` blieben unberührt.

**Zeilenversatz: null.** Das Urteil zitiert `docs/beweise/NAK-286.md` zum Stand
`0523c079`. `git diff 0523c079..HEAD --numstat -- docs/beweise/NAK-286.md`
meldet `15 1` — eine ersetzte Zeile in der Kopftabelle (Zeile 10, „Etappe") und
14 angehängte Zeilen ab der alten letzten Zeile 929 (§9). Vor §9 verschiebt sich
keine Zeile; jede Zeilenangabe des Prüfers gilt an HEAD unverändert. Unten steht
jeweils die HEAD-Zeile.

---

## Befund 6 — „Erhalte die Einmal-Zusage nach dem Kennungs-Wrap"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:547-547 — DEFEKT M-51/F-13/T-2: Der
vorgeschriebene Test verlangt nach 257 verschiedenen Kennungen ausdrücklich eine
zweite Antwort auf die älteste Kennung. Das widerspricht R-286-1 (Zeile 350),
wonach jede Instanz jede Anfragekennung genau einmal beantwortet. Die Begründung,
ein einzelner Runnerlauf benötige weniger als 256 Kennungen und wiederhole selbst
keine, begrenzt weder die Prozessorlebensdauer noch die Gültigkeit erneut
vorgelegter Anfragen. Der Rotbeweis muss auch nach dem Wrap eine zweite Antwort
erkennen, statt sie als Sollverhalten festzuschreiben.`

**Gegenstand — Regel R-286-1, `docs/beweise/NAK-286.md:350`:**

> „Jede Instanz (Gen, Probeeq, Broker) **beantwortet jede Anfragekennung genau
> einmal** und löscht die Anfrage nie — die Anfrage räumt der Anfragende
> (Runner) ab."

**Gegenstand — Matrixzeile M-51, `docs/beweise/NAK-286.md:547`** (Zustand,
Zusage, Rotbeweis):

> Zustand: „257 verschiedene gültige Kennungen nacheinander, danach die
> zweitjüngste und die älteste erneut" · Zusage: „die 256 jüngsten beantworteten
> Kennungen bekommen nie eine zweite Antwort; **die verdrängte älteste bekommt
> eine zweite**; Ring fest 256 Einträge, nach dem Anlauf keine Allokation je
> Takt" · Rotbeweis: „Ringgröße 1 → zweitjüngste Kennung erneut beantwortet →
> rot" · Quelle: „R-286-1 „genau einmal"".

**Gegenstand — F-13, `docs/beweise/NAK-286.md:369`:**

> „Je Instanz **256 zuletzt beantwortete Kennungen, FIFO, fester Speicher**;
> höchstens drei Schreibversuche je Kennung in drei aufeinanderfolgenden Takten;
> danach aufgegeben, Fehlerzähler plus 1 (nur Testzugang) (§7.1 T-2)."

**Gegenstand — T-2, `docs/beweise/NAK-286.md:874`:**

> „… Ring 256 (ein Abschlusslauf stellt weit weniger Anfragen: Snapshot eine,
> U40 drei Stellen mit je höchstens acht, dazu Fenster und Bereitschaft); drei
> Schreibversuche; Takt 1 Hz. **Eine verdrängte Kennung würde erneut beantwortet
> (M-51); der Runner wiederholt nie eine Kennung.**"

**Quelle an HEAD — Kennung ist 8 Bit? Nein.** Die Anfragekennung ist nach F-2
(`:353`) „genau 32 Zeichen aus `0-9a-f`", nach T-2 (`:874`) „128 Bit, Runner
`[guid]::NewGuid().ToString('N')`". Es gibt **keinen** 8-Bit-Zähler und keinen
Ring von 256 Kennungswerten: 256 ist allein die Kapazität des Merkspeichers über
einem 128-Bit-Raum. „Wrap" heißt hier nicht Zahlenüberlauf, sondern Verdrängung
aus dem FIFO. Der Prüfer nennt das richtig, nur der Kurzname „Wrap" könnte in
der Nacharbeit missverstanden werden.

**Quelle an HEAD — gibt es heute schon eine Kennungs- oder Sequenzlogik im
Plugin?** Ja, zwei, und beide taugen als Muster:

`eq-copilot/plugin/src/prozessor/Ipc.cpp:783-791`:

```cpp
    // Nichts erfinden: nur ein Auftrag, der wirklich unter DIESER Kennung
    // steht und wirklich ein `user_verdict` ist, wird wiederholt. Beides sind
    // Wachen gegen eine falsche Verdrahtung, keine zweite Zuordnung - die
    // leistet das In-Flight-Register, das den Eintrag ueber seine
    // `command_id` findet und genau dessen Text uebergibt.
    const auto kennung = commandId.toStdString();
```

Das In-Flight-Register ordnet über die `command_id` zu und ist **nicht**
verdrängend — es ist die bestehende „genau einmal je Kennung"-Stelle des
Produkts. Daneben führt `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:110-114`
eine monoton wachsende `interventionsSequenz`
(`interventionsSequenz.fetch_add (1, …) + 1`), und `Analyse.cpp:366` nennt die
„SEQUENZLUECKE" als tragendes Mittel. Beide bestehenden Wege binden Eindeutigkeit
also entweder an ein vollständiges Register oder an eine monotone Nummer, nie an
einen verdrängenden Ring.

**Realistische Lebensdauer.** Der Prüfer bestreitet T-2s Rechnung, nicht ihre
Zahl — zu Recht, und die Quellen zeigen ein geteiltes Bild:

- *Plugin:* `snapshot-runde01.json` trägt `frischer_start`
  (`:792`), M-59 (`:560`) beendet und startet das Diagnose-FL vor diesem
  Szenario neu. Je Runnerlauf lebt der Prozessor also nur über
  `snapshot-runde01` (eine Anfrage) und `u40-aktivitaetsgate` (drei Stellen ×
  höchstens acht = 24) — rund 25 Kennungen. **Im Runnerbetrieb ist der Wrap für
  das Plugin heute praktisch unerreichbar.**
- *Broker:* der Runner startet und beendet ihn nie (M-57, `:558`; T-10, `:882`);
  er endet nur nach `BROKER_IDLE_ENDE_MS = 60_000`
  (`broker/src/lebenslauf.rs:43`) **ohne Clients**. Mit einem angemeldeten Gen
  `main` lebt er über beliebig viele Läufe. Dort summieren sich die Kennungen.
- *Produkt:* R-286-1 ist eine Produktzusage ohne Runnerbezug. Ein Gen oder
  Probeeq in einer echten FL-Sitzung lebt Stunden; der Takt ist 1 Hz (F-12,
  `:368`), die Anfragedatei darf jede Sekunde eine neue Kennung tragen. 257
  Kennungen sind dann 257 Sekunden.

**Status: BESTÄTIGT.** M-51 schreibt in der Spalte *Zusage* wörtlich ein
Verhalten fest („die verdrängte älteste bekommt eine zweite"), das R-286-1
wörtlich verbietet („genau einmal"), und nennt R-286-1 dabei selbst als Quelle.
Das ist ein Widerspruch in derselben Zeile, nicht nur eine Auslegungsfrage. T-2
begründet ihn mit der Anfragezahl **eines Laufs** und mit einer Zusage über den
**Runner** („der Runner wiederholt nie eine Kennung") — beides bindet die
Instanz nicht, deren Lebensdauer und deren Anfragequelle die Regel gerade nicht
einschränkt. Der Rotbeweis („Ringgröße 1") misst die Ringgröße korrekt, aber
keine der beiden Aussagen zur Einmaligkeit nach der Verdrängung; nach dem
Leitsatz (`:345`) fällt er damit neben der strittigen Zusage.
Einschränkung gegenüber dem Prüfer: die Erreichbarkeit im heutigen Runnerpfad
ist für das Plugin gering (siehe oben) — der Defekt liegt im Regelwiderspruch
und in der Broker-/Produktlebensdauer, nicht in einem drohenden Fehlverhalten
des Abschlusslaufs.

**Einordnung (Vorschlag): DEFEKT** (Matrixzeile widerspricht Regel R-286-1
wörtlich und ist in sich widersprüchlich; der Rotbeweis trägt die strittige
Zusage nicht).

**Schließende Änderung (Vorschlag).** Der Widerspruch ist nur an einer von zwei
Stellen auflösbar, und die Regel gehört dem Dirigenten:

1. *Regel präzisieren (empfohlen, kleinster Eingriff):* R-286-1 bekommt den
   Geltungsrahmen, den F-13 ohnehin baut — „genau einmal, solange die Kennung im
   Kennungsring steht (fest 256, FIFO); eine verdrängte Kennung ist für die
   Instanz eine neue Kennung". M-51 behält dann sein Sollverhalten, bekommt aber
   **zusätzlich** einen Rotbeweis an der Einmal-Zusage: Mutation „Ring wird vor
   dem Schreiben nicht gefragt" → die zweitjüngste (noch im Ring stehende)
   Kennung bekommt eine zweite Antwort → rot. Damit fällt je Zusage genau ein
   Beweis: Ringgröße (bestehende Mutation) und Einmaligkeit (neue Mutation).
2. *Zusage halten:* der Merkspeicher wird so bemessen, dass eine Verdrängung
   innerhalb einer Instanzlebensdauer nicht auftreten kann — dann muss die
   Grenze aus einer gemessenen Obergrenze der Anfragen je Lebensdauer folgen,
   nicht aus der Anfragezahl eines Laufs, und M-51 misst statt der Verdrängung,
   dass die Grenze nie erreicht wird. Ohne eine solche Messung ist das eine
   Zusage ohne Beweisweg.

Zusätzlich in beiden Fällen: T-2 (`:874`) ersetzt „ein Abschlusslauf stellt weit
weniger Anfragen" durch eine Aussage über die **Instanzlebensdauer** (Broker
ohne Neustart, Plugin über mehrere Läufe hinweg), und der Kurzname „Wrap" wird
als Verdrängung im FIFO benannt, damit ihn niemand als Zahlenüberlauf liest.

---

## Befund 7 — „Beantworte auch die zehnte Instanz mit gleicher Kennung"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:354-354 — DEFEKT F-3/M-33: M-33
berücksichtigt ausdrücklich mehrere Instanzen mit gleicher persistenter
Instanzkennung im selben Prozess. Bei zehn solchen Instanzen derselben Rolle
stehen hier aber nur neun Antwortnamen zur Verfügung: der Grundname und `.2` bis
`.9`. Die zehnte Instanz gibt ihre gültige Anfrage auf und verletzt damit
R-286-1 „Jede Instanz … beantwortet jede Anfragekennung genau einmal". T-3
begründet lediglich das Hashpräfix, nicht diesen Deckel. Die Namensstrategie und
ihr Test müssen diesen bereits zugelassenen Kollisionsfall ohne verlorene
Instanzantwort behandeln.`

**Gegenstand — F-3, `docs/beweise/NAK-286.md:354`:**

> „**F-3 Antwortname** `<anfrage_id>.<rolle>.<pid>.<instanz16>.json`: `rolle` ∈
> {`gen`, `probeeq`, `broker`}; `pid` dezimal; `instanz16` = die ersten 16
> Hex-Zeichen von SHA-256 über die UTF-8-Bytes der Instanzkennung (Plugin:
> `zustand.common.instanceId`; Broker: `session_token`,
> `broker/src/lebenslauf.rs:106`). **Existiert der Name schon, folgen `.2` bis
> `.9` vor `.json`; danach gilt die Kennung als aufgegeben (§7.1 T-3).**"

**Gegenstand — M-33, `docs/beweise/NAK-286.md:514`:**

> Zustand: „zwei Probeeq-Instanzen in einem Prozess; (a) verschiedene
> Instanzkennungen; (b) **gleiche Kennung (Kennungskonflikt)**" · Zusage: „(a)
> zwei Dateien mit verschiedenem `instanz16`; (b) zwei Dateien, die zweite mit
> Suffix `.2`; keine Datei wird je ersetzt" · Quelle: „R-286-1 („Jede Instanz …
> beantwortet")".

**Gegenstand — T-3, `docs/beweise/NAK-286.md:875`** (die Begründung, auf die F-3
für den Deckel verweist):

> „**T-3 Dateiname mit Hashpräfix.** `instanceId` ist bei Altständen bytegleich
> der frühere `sensor_id`-Text (`eq-copilot/plugin/state/NakamaState.h:52`) und
> kann beliebige Zeichen tragen; der Name nimmt deshalb die ersten 16
> Hex-Zeichen von SHA-256 der Kennung, der Kopf die Kennung selbst. …"

T-3 begründet **nur** den Hash. Über die Zahl der Suffixe, über das Aufgeben
nach dem neunten Namen und über die Folge für R-286-1 steht dort nichts. Der
Verweis „(§7.1 T-3)" in F-3 zeigt an dieser Stelle ins Leere.

**Quelle an HEAD — wie entsteht die persistente Instanzkennung, und wann
kollidiert sie?**

`eq-copilot/plugin/state/NakamaState.h:52`:

```cpp
    juce::String instanceId;                         ///< bytegleich aus sensor_id; frisch: juce::Uuid
```

Sie wird beim Serialisieren in den Hoststate geschrieben
(`eq-copilot/plugin/state/NakamaState.cpp:706`, `:818`, Schlüssel `kInstanceId`)
und beim Laden zurückgelesen (`:1482`, `c.instanceId = id.toString();`). Frisch
erzeugt wird sie nur an zwei Stellen: `NakamaState.cpp:2041-2044`
(`Zustand frisch (…)`) und `eq-copilot/plugin/src/PluginProcessor.cpp:243`
(`zustand = nakama::state::frisch (juce::Uuid().toString());`, eine Zeile über
`instanceNonce`). **Folge: jede Instanz, die ihren State aus einem gespeicherten
Stand lädt, trägt dessen Kennung.** Ein kopierter Mixer-Slot, ein dupliziertes
Plugin, eine kopierte Projektdatei und ein aus derselben Vorlage gebautes
Projekt liefern deshalb echte Kollisionen. Aufgelöst wird eine Kollision heute
nur von Hand: `eq-copilot/plugin/src/prozessor/Ipc.cpp:1195-1198`

```cpp
// Sichtbare Antwort auf einen Kennungs-Konflikt (Plan §8.4): DIESE Instanz
// bekommt eine frische persistente ID und meldet sich neu an. Der Host
// speichert sie mit dem nächsten Projekt-Save (getStateInformation) — dafür
// MUSS er die Änderung kennen: Host-Dirty (Vertrag nakama-state-v2.md §6).
```

und der zugehörige Weg `neueSensorId()`
(`eq-copilot/plugin/src/prozessor/State.cpp:54-67`) hängt am Editorknopf
(`eq-copilot/plugin/src/PluginEditor.cpp:591`). **Probeeq hat keinen Editor**
(`hasEditor()` false, `eq-copilot/plugin/sonde/SondeProcessor.h:156-157`, in
M-30 selbst zitiert) — eine kollidierende Probeeq-Kennung bleibt also bestehen.
Genau der Fall, den M-33 (b) anlegt.

**Zehn Instanzen — realistisch?** Ja. Das Produkt ist auf viele Probeeq-Instanzen
ausgelegt (`CLAUDE.md`, Wahrheitskern: die zweite Oberfläche „bedient die
Probeeq-Instanzen zentral"), und der natürliche Handgriff dafür — einen fertig
eingestellten Slot auf weitere Busse kopieren — vervielfältigt genau die
persistente Kennung. Ein Deckel bei neun ist damit kein Randfall, sondern eine
Grenze im erwarteten Gebrauch.

**Quelle an HEAD — es gibt bereits eine per-Instanz eindeutige Kennung.** Beide
Prozessoren führen eine flüchtige Laufzeit-Kennung, die nie im State steht:

`eq-copilot/plugin/src/PluginProcessor.h:1347-1350`:

```cpp
    // Flüchtige Verbindungs-ID pro Prozessor-Lebenszeit (v2, Plan §8.2;
    // Entwurf §32.1 `runtime_nonce`) — nach dem Konstruktor unveränderlich,
    // daher ohne Mutex lesbar. Nie Teil des States.
    juce::String instanceNonce;
```

gesetzt in `eq-copilot/plugin/src/PluginProcessor.cpp:243`
(`instanceNonce = juce::Uuid().toString();`), als hex32 auf die Leitung gegeben
(`eq-copilot/plugin/src/prozessor/Ipc.cpp:472`). Probeeq hat dieselbe Größe als
`const std::string v3RuntimeNonce` (`eq-copilot/plugin/sonde/SondeProcessor.h:450`),
im Konstruktor aus `uuidHex32()` (`SondeProcessor.cpp:138`). Der Broker benutzt
für F-3 ohnehin schon sein prozesseindeutiges `session_token`.

**Status: BESTÄTIGT.** Die Rechnung des Prüfers stimmt: Grundname plus `.2` bis
`.9` sind neun Namen; die zehnte Instanz mit derselben persistenten Kennung im
selben Prozess (gleiche `pid`, gleiches `instanz16`) findet keinen freien Namen
und gibt nach F-3 „die Kennung als aufgegeben" — sie beantwortet die gültige
Anfrage gar nicht, während R-286-1 „jede Instanz … beantwortet jede
Anfragekennung genau einmal" verlangt. Der Deckel ist von keiner Zusage
begründet: T-3 trägt nur den Hash. M-33 misst zudem nur zwei Instanzen und kann
den Deckel deshalb nie treffen. Zusatz, den der Prüfer nicht nennt: F-3 löst das
Problem mit einem Suffixleiter, obwohl das Produkt mit `runtime_nonce` schon eine
kollisionsfreie Laufzeitidentität führt — die Suffixleiter ist damit nicht nur zu
kurz, sie ist unnötig.
Einordnende Nuance: der **Messweg** leidet nicht darunter, denn M-56 (`:557`)
wertet zwei Antworten derselben Rolle und PID mit verschiedener `instanz_id`
ohnehin als „mehrdeutig" und verfehlt den Schritt. Betroffen ist die
Produktzusage R-286-1, nicht die U40-Messung.

**Einordnung (Vorschlag): DEFEKT** (F-3 verletzt R-286-1 im bereits von M-33
zugelassenen Kollisionsfall; der begründende Verweis §7.1 T-3 trägt die Zusage
nicht).

**Schließende Änderung (Vorschlag).** F-3 nimmt die vorhandene, per Instanz
eindeutige Laufzeitkennung in den Namen auf, statt Kollisionen zu zählen:
`<anfrage_id>.<rolle>.<pid>.<instanz16>.<nonce8>.json`, mit `nonce8` = die ersten
8 Hex-Zeichen der Laufzeit-Nonce (Gen `instanceNonce`,
`PluginProcessor.cpp:243`; Probeeq `v3RuntimeNonce`, `SondeProcessor.cpp:138`;
Broker `session_token`, `lebenslauf.rs:106`). Damit können zwei **lebende**
Instanzen im selben Prozess denselben Namen nicht mehr treffen; die Suffixleiter
bleibt nur noch als Riegel gegen eine liegengebliebene Datei eines früheren
Laufs und darf dann nicht mehr aufgeben, sondern verlängert den Nonce-Anteil.
Der Umschlag trägt die persistente `instanz_id` unverändert (F-4, M-56 bleiben
gültig); die Nonce steht zusätzlich im Kopf, damit die Auswahl im Runner beide
unterscheiden kann. M-33 bekommt eine dritte Lage: „zehn Probeeq-Instanzen mit
gleicher persistenter Kennung → zehn Dateien, keine aufgegebene Kennung", und
der Rotbeweis fällt an genau dieser Zusage (Mutation: Nonce aus dem Namen
entfernt → zehnte Instanz ohne Antwort → rot). T-3 wird um die Begründung der
Namenszusammensetzung ergänzt oder F-3 verweist auf einen neuen Technikentscheid.

---

## Befund 8 — „Wähle einen Writer mit den zugesagten Dateisemantiken"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:746-746 — DEFEKT §5.6/F-8: Der
vorgeschriebene unveränderte Helfer `persistenz::atomar_neu` erfüllt die Matrix
nicht: `broker/src/persistenz.rs:31–33` ruft `create_dir_all` auf, und
`TempPfad::drop` löscht bei einem Umbenennungsfehler die Temp-Datei (`:19–22`,
`:158–165`). F-1/M-43 untersagen Ordneranlagen, F-9 verlangt einen Löschzähler
von null, und M-53 verlangt ausdrücklich eine liegenbleibende Temp-Datei. Der
Bauplan muss einen kompatiblen Schreibweg innerhalb der zugelassenen Pfade
vorsehen; eine Attrappe darf nicht andere Semantik als der tatsächlich
verwendete Helfer beweisen.`

**Gegenstand — Bauplan §5.6 Punkt 1, `docs/beweise/NAK-286.md:746`:**

> „**Modul Briefkasten** (`broker/src/`): Griff mit Thread `nakama-briefkasten`,
> Stoppflag und Weckbedingung (Takt 1 s); Takt nach §2.1 F-2, F-8 bis F-11 und
> §2.2 F-13, F-14; Anfrageprüfung; Kennungsring; Umschlag mit `aggregat_bauen`
> ohne Filter und Sperre aus `aggregat_sperrgrund` (F-7); **Schreiben über
> `persistenz::atomar_neu`**; Fassaden für Dateisystem und Uhr; Tests unter
> `#[cfg(test)]`."

Das Wort „unverändert" steht dort nicht wörtlich; es folgt aus den Ticketpfaden
der Etappe 3 (`:739-742`), die nur `broker/src/briefkasten.rs`,
`broker/src/lib.rs`, `broker/src/lebenslauf.rs` und eine neue Datei unter
`broker/tests/` nennen — `broker/src/persistenz.rs` darf die Etappe **nicht
anfassen**. Der Prüfer paraphrasiert also richtig, nur nicht wörtlich.

**Gegenstand — F-8, `docs/beweise/NAK-286.md:359`:**

> „**F-8 Schreiben.** In `<name>.tmp-<pid>` im Antwortordner, schließen, Größe
> größer 0 prüfen, dann ohne Ersetzen umbenennen (Broker:
> `persistenz::atomar_neu`, `broker/src/persistenz.rs:158-166`; Plugin: gleiche
> Semantik). Nie direkt in den Zielnamen."

**Gegenstand — F-1, `docs/beweise/NAK-286.md:352`** (Schluss) und **T-4,
`:876`:**

> F-1: „**Keine Instanz legt einen dieser Ordner an**; der Runner legt
> `diagnose\` und `antwort\` an (§7.1 T-4)."
> T-4: „**Keine Ordneranlage durch Instanzen.** Sonst hinterließe eine einzige
> liegengebliebene Anfrage auf jedem Rechner dauerhaft Ordner; der Anfragende
> besitzt den Briefkasten (R-286-1)."

**Gegenstand — F-9, `docs/beweise/NAK-286.md:360`:**

> „**F-9 Genau einmal, nie löschen.** … Instanzen löschen weder `anfrage.json`
> noch eine Antwort (Zähler `loeschen` der Fassade bleibt in jedem Fall 0)."

**Gegenstand — M-43, `docs/beweise/NAK-286.md:529`** (Rust-Zusage) und **M-41,
`:527`:**

> M-43 Zusage: „dieselben Zusagen wie im Plugin: … höchstens drei
> Schreibversuche; **keine Ordneranlage**; nie gelöscht".
> M-41 Zusage: „600 Existenzprüfungen, 0 Leseöffnungen, 0 Schreibvorgänge,
> **0 Ordneranlagen, 0 Löschungen**".

**Gegenstand — M-53, `docs/beweise/NAK-286.md:549`:**

> Zusage: „es entsteht keine `.json` der Länge 0; **die Temp-Datei bleibt als
> `.tmp-<pid>` liegen**; …"

**Quelle an HEAD — `broker/src/persistenz.rs:19-23`:**

```rust
impl Drop for TempPfad {
    fn drop(&mut self) {
        let _ = std::fs::remove_file(&self.pfad);
    }
}
```

**`broker/src/persistenz.rs:31-46`:**

```rust
fn temp_anlegen(ziel: &Path) -> Result<(TempPfad, File), String> {
    let eltern = elternordner(ziel);
    std::fs::create_dir_all(eltern).map_err(|e| format!("{} anlegen: {e}", eltern.display()))?;
    let dateiname = ziel
        .file_name()
        .ok_or_else(|| format!("{} hat keinen Dateinamen", ziel.display()))?;

    for _ in 0..TEMP_VERSUCHE {
        let mut temp_name = OsString::from(".");
        temp_name.push(dateiname);
        temp_name.push(format!(
            ".tmp-{}-{}",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ));
```

**`broker/src/persistenz.rs:158-166`:**

```rust
pub(crate) fn atomar_neu(ziel: &Path, daten: &[u8]) -> Result<bool, String> {
    let temp = temp_schreiben(ziel, daten)?;

    #[cfg(windows)]
    match verschieben(&temp.pfad, ziel, false) {
        Ok(()) => Ok(true),
        Err(e) if ist_bereits_vorhanden(&e) => Ok(false),
        Err(e) => Err(format!("{} atomar veröffentlichen: {e}", ziel.display())),
    }
```

**Status: BESTÄTIGT**, und der Widerspruch ist größer, als das Urteil sagt. Fünf
gemessene Abweichungen zwischen dem vorgeschriebenen Helfer und den Zusagen:

1. **Ordneranlage.** `temp_anlegen` ruft `create_dir_all` bei **jedem** Schreiben
   (`:33`), also auch dann, wenn `antwort\` fehlt. F-1 (`:352`), T-4 (`:876`),
   M-29 (`:509`, „keine Ordneranlage … Ordneranlagen größer 0 → rot"), M-41
   (`:527`) und M-43 (`:529`) verbieten genau das. Der Weg über `atomar_neu`
   macht M-29 (a) („`antwort\` fehlt") sogar grün, statt drei Versuche zu zählen.
2. **Löschung.** `TempPfad::drop` (`:19-22`) entfernt die Temp-Datei in jedem
   Rückweg, in dem das Umbenennen nicht erfolgreich war — genau der Fall, den
   M-53 (`:549`) als „die Temp-Datei bleibt als `.tmp-<pid>` liegen" zusagt, und
   der Fall, in dem F-9 (`:360`) den Löschzähler bei 0 verlangt. (Nach einem
   **erfolgreichen** Umbenennen ist der Pfad weg, `remove_file` scheitert
   folgenlos — dort ist der Drop harmlos.)
3. **Temp-Name.** `persistenz` bildet `.<dateiname>.tmp-<pid>-<uuid>` (`:39-45`),
   F-8 sagt `<name>.tmp-<pid>` zu. M-55 (`:556`) lässt den Runner beim Aufräumen
   nur „`<32 hex>.<rolle>.<pid>.<16 hex>[.<n>].json` und `.tmp-*`" abräumen und
   M-53 lässt ihn „`.tmp-*`" ignorieren — der tatsächlich entstehende Name
   beginnt mit einem Punkt und **endet** auf `.tmp-…`; beide Namensmuster
   greifen nicht.
4. **Größenprüfung.** F-8 verlangt „schließen, Größe größer 0 prüfen, dann …
   umbenennen". `temp_schreiben` (`:60-70`) schreibt, `sync_all`, `drop` — es
   prüft keine Größe. Die Zusage aus M-53 („es entsteht keine `.json` der Länge
   0") ruht damit allein auf der Annahme, dass `write_all` alles geschrieben hat.
5. **Messbarkeit.** F-14 (`:370`) verlangt eine Dateisystem-Fassade, die
   „Existenzprüfungen, Leseöffnungen, gelesene Bytes, Schreibvorgänge,
   Umbenennungen, Ordneranlagen und Löschungen" zählt, und M-41/M-43 werten
   genau diese Zähler. `persistenz` ruft `std::fs` und `MoveFileExW` direkt
   (`:33`, `:47`, `:104`, `:21`) — **an der Fassade vorbei**. Die Zähler der
   Zeilen M-41 und M-43 könnten den einzigen Schreibweg des Moduls also gar
   nicht sehen; der Beweisweg trägt die Zusage nicht.

Dazu die Sperre, die der Prüfer im letzten Satz nur andeutet: `persistenz.rs`
steht **nicht** in den Ticketpfaden der Etappe 3 (`:739-742`), der Helfer ist
also nicht änderbar; und er ist nicht frei: `aggregat_schreiben`
(`broker/src/aggregat.rs:158-174`) ist heute sein einziger `atomar_neu`-Nutzer
und **braucht** `create_dir_all`, weil der Snapshotordner fehlen darf. Eine
Änderung an `persistenz` wäre also zugleich eine Verhaltensänderung an einem
bestehenden Weg.

**Einordnung (Vorschlag): DEFEKT** (der Bauplan schreibt einen Helfer vor, dessen
gemessene Semantik vier Matrixzusagen widerspricht, und dessen Aufrufe die
zugesagte Messfassade umgehen; die Ticketpfade lassen den Helfer nicht
korrigieren).

**Schließende Änderung (Vorschlag).** Der Briefkasten bekommt seinen **eigenen**
Schreibweg in `broker/src/briefkasten.rs` — innerhalb der bestehenden
Ticketpfade, ohne `persistenz.rs` anzufassen und ohne den Weg von
`aggregat_schreiben` zu berühren. Er läuft vollständig über die
Dateisystem-Fassade aus F-14 und leistet in dieser Reihenfolge:
(1) `antwort\` **nicht** anlegen; existiert der Ordner nicht, zählt das als
fehlgeschlagener Schreibversuch nach F-13 (drei Takte, dann aufgeben);
(2) Temp-Datei exakt als `<name>.tmp-<pid>` exklusiv anlegen (`create_new`), so
wie F-8, M-53 und das Aufräummuster in M-55 sie benennen;
(3) schreiben, `sync_all`, schließen, **Größe > 0 lesen**;
(4) ohne Ersetzen umbenennen (auf Windows `MoveFileExW` ohne
`MOVEFILE_REPLACE_EXISTING`, wie `persistenz::verschieben` es vormacht);
(5) **kein** `Drop`-Aufräumer: scheitert das Umbenennen, bleibt die Temp-Datei
liegen (M-53) und der Löschzähler bleibt 0 (F-9).
F-8 (`:359`) wird entsprechend umformuliert (Verweis auf den eigenen Weg statt
auf `persistenz::atomar_neu`; der Helfer bleibt nur noch als Vorlage genannt),
§5.6 Punkt 1 (`:746`) ebenso, und M-41/M-43 nennen ausdrücklich, dass alle
Dateizugriffe des Moduls durch die Fassade laufen — sonst zählt der Test nichts.
Die C++-Hälfte („Plugin: gleiche Semantik") ist davon nicht betroffen, muss aber
denselben Wortlaut bekommen, damit `tools/dirigent/pruefliste.md` A („Dieselbe
Regel in allen Sprachen") hält.

---

## Befund 11 — „Lass den Lebensdauer-Rotbeweis die Join-Frist verletzen"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:531-531 — DEFEKT M-45: Beim in F-12
festgelegten 1-s-Takt kann ein Thread auch ohne Weckruf beim nächsten regulären
Aufwachen sein Stoppflag erkennen und innerhalb der zugesagten 2 Sekunden enden.
Die Mutation „Stopp ohne Weckruf, nur Schlaf" erzeugt deshalb nicht zuverlässig
den behaupteten Fristverstoß; Schedulerverzögerungen wären kein Beweis für den
fehlenden Weckruf. Der Rotbeweis muss deterministisch das fehlende
Stoppen/Joinen oder Zugriffe nach dem Lebensdauerende erkennen, damit die
Beziehung starten↔stoppen tatsächlich abgesichert wird
([CLAUDE.md:84–86](../../CLAUDE.md#L84-L86)).`

**Gegenstand — M-45, `docs/beweise/NAK-286.md:531`:**

> Zusage: „Stoppflag und Weckbedingung beenden den Thread binnen 2 s; nach dem
> Stopp keine Existenzprüfung und keine Antwort mehr; im Brokerlauf startet der
> Griff nach beiden Servern und wird in `broker_geordnet_stoppen` als erster
> Teil gestoppt, vor Supervisor, v2, v3 und Store" · Frist: „Join-Frist 2 s;
> Reihenfolge über eine gemeinsame Stoppfunktion" · Test:
> „`griff_stoppt_binnen_zwei_sekunden` und `stoppreihenfolge_briefkasten_zuerst`
> … A4" · Rotbeweis: „(1) **Stopp ohne Weckruf, nur Schlaf → Join länger als 2 s
> → rot**; (2) Briefkasten nach dem Store gestoppt → Protokoll rot".

**Gegenstand — F-12, `docs/beweise/NAK-286.md:368`** (Broker-Hälfte):

> „Broker: Thread `nakama-briefkasten` mit Stoppflag und Weckbedingung (**Takt
> 1 s**), Start in `broker_starten` nach beiden Servern
> (`broker/src/lebenslauf.rs:145-162`), Stopp als erster Teil von
> `broker_geordnet_stoppen` (vor `:229-234`), **Join-Frist 2 s**."

**Gegenstand — Regel der Matrix selbst, `docs/beweise/NAK-286.md:463`:**

> „Nebenläufigkeits- und Lebensdauerbeweise fallen an einem **deterministisch
> erzwungenen Interleaving** (Testhaken, Barriere, Kanarienwert), **nie an einer
> Wahrscheinlichkeit**; Zeitbeweise an einer Testuhr."

**Quelle an HEAD — das bestehende Muster ist genau die kritisierte Bauart.**
`broker/src/lebenslauf.rs:167-177` (Supervisor-Thread):

```rust
            let supervisor_join = std::thread::Builder::new()
                .name("nakama-coordinator-tick".into())
                .spawn(move || {
                    while !stop_fuer_thread.load(Ordering::SeqCst) {
                        std::thread::sleep(Duration::from_millis(100));
```

und `broker/src/lebenslauf.rs:72-79`:

```rust
impl Drop for BrokerSupervisor {
    fn drop(&mut self) {
        self.stop.store(true, Ordering::SeqCst);
        if let Some(join) = self.join.take() {
            let _ = join.join();
        }
    }
}
```

Der einzige Lebenszyklus-Thread des Brokers arbeitet also mit **Flag plus
Schlaf, ohne Weckruf** und ohne Frist — er kommt mit 100 ms Takt davon. Andere
Griffe joinen ebenso unbefristet (`broker/src/store/writer.rs:128`,
`broker/src/transport/server_v3/griff.rs:327`, `:330`, `:391`,
`broker/src/server.rs:275`). Eine **Weckbedingung** gibt es im Broker als Muster
(`Condvar` in `broker/src/store/handle.rs:14-20`,
`broker/src/transport/server_v3/queues.rs:12-19`), aber keine bestehende
Lebensdauerprüfung gegen eine Frist. Die Zusage „binnen 2 s" ist damit neu und
hat kein Vorbild, an dem sie sich messen könnte.

Die Reihenfolgeangaben des Bauplans stimmen dagegen: `broker_geordnet_stoppen`
beginnt an `broker/src/lebenslauf.rs:219`, endet an `:257` (§5.6 Punkt 3, `:748`
nennt `:219-257`), und die Reihenfolge Supervisor → v2 → v3 → Store steht dort
wörtlich (`:227-255`).

**Status: BESTÄTIGT.** Die Rechnung: der Takt schläft 1 s (F-12), die Frist ist
2 s. Ein Thread nach dem Muster `while !stop { sleep(1s); arbeiten }` erkennt das
Flag spätestens 1 s plus eine Taktdauer nach dem Setzen. Solange ein Takt (eine
Existenzprüfung über die Fassade) deutlich unter 1 s bleibt — und das ist der
Normalfall, M-41 fährt 600 solche Takte —, endet der mutierte Thread **innerhalb**
der 2 s. Die Mutation (1) erzeugt den Fristverstoß also nicht; sie erzeugt ihn
nur, wenn der Scheduler oder ein langsamer Takt zufällig über 1 s hinausläuft.
Damit hängt der Rotbeweis an einer Wahrscheinlichkeit — was Zeile 463 der Matrix
für Lebensdauerbeweise ausdrücklich verbietet. Die Zeile widerspricht sich
selbst.

Zwei Beobachtungen über das Urteil hinaus:

- M-45 trägt **drei** Zusagen (Frist, „nach dem Stopp keine Existenzprüfung und
  keine Antwort mehr", Reihenfolge), aber nur **zwei** Mutationen. Die
  Zusage „nach dem Stopp keine Existenzprüfung und keine Antwort mehr" — die
  einzige der drei, die deterministisch messbar wäre — hat gar keinen
  Rotbeweis. Zeile B der Selbstprüfung (`:845`) behauptet dagegen: „nach dem
  Stopp keine Lieferung: M-45".
- Die Frist von 2 s ist auf einer echten Uhr gemessen; Zeile 463 verlangt für
  Zeitbeweise eine Testuhr. Ein Join mit einer Wanduhr-Frist von 2 s ist auf
  einem ausgelasteten Kanonrechner zudem eine mögliche Flake-Quelle in A4.

**Einordnung (Vorschlag): DEFEKT** (der Rotbeweis kann die Zusage nicht tragen
und verletzt die Beweisregel der Matrix selbst; eine der drei Zusagen der Zeile
hat gar keine Mutation).

**Schließende Änderung (Vorschlag).** M-45 wird in drei Zusagen mit je einem
deterministischen Beweis zerlegt:

1. **Stopp weckt.** Der Takt wartet über eine `Condvar` mit `wait_timeout(1 s)`
   (Muster `broker/src/store/handle.rs:14-20`); der Test setzt eine **Testuhr**
   beziehungsweise einen Testhaken, der den Takt an einer Barriere anhält, und
   misst nicht Sekunden, sondern die Zahl der Warteschritte: nach dem Stopp
   kehrt der Thread ohne einen weiteren Taktschlaf zurück. Mutation: `notify`
   im Stoppweg entfernt → der Thread braucht einen vollen Wartelauf → der
   Zähler „Wartedurchläufe nach dem Stoppsignal" ist 1 statt 0 → rot,
   unabhängig vom Scheduler.
2. **Nach dem Stopp keine Lieferung.** Anfrage liegt bereit, Griff wird
   gestoppt, danach werden zehn Takte manuell ausgelöst: 0 Existenzprüfungen,
   0 Antworten, Kanarienwert unberührt. Mutation: Stoppflag im Takt nicht
   geprüft (oder Join fehlt und der Thread läuft weiter) → Zähler größer 0 →
   rot. Das ist zugleich der von `CLAUDE.md:84-86` verlangte Gegenpfad
   starten↔stoppen.
3. **Reihenfolge.** bleibt wie gebaut (Protokoll der gemeinsamen Stoppfunktion,
   Mutation 2 der heutigen Zeile).

Die Wanduhr-Frist von 2 s bleibt als **Zusage** in F-12 stehen (sie beschreibt
das Produkt), wird aber nicht mehr zum Rotbeweis gemacht; gemessen wird sie
höchstens als Rohzeile.

---

## Befund 12 — „Injiziere den Leerdateifehler vor dem ersten Schreibbyte"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:549-549 — DEFEKT M-53: Die
Fehlerinjektion erfolgt ausdrücklich nach dem Schreiben der Temp-Datei. Wird
lediglich auf direktes Schreiben in den Zielnamen mutiert, ist zu diesem
Zeitpunkt auch die Zieldatei bereits beschrieben und nicht null Bytes groß. Der
angekündigte Rotbeweis kann daher höchstens am fehlenden Temp-Artefakt
scheitern, nicht an der zugesagten Vermeidung leerer Antworten. Für diesen
Beweis braucht es einen Fehlerpunkt nach Anlegen beziehungsweise Trunkieren,
aber vor dem ersten Payload-Byte, und eine Prüfung der sichtbaren
Antwortdatei.`

**Gegenstand — M-53, `docs/beweise/NAK-286.md:549`** (vollständig):

> Zustand: „**Fehlerinjektion nach dem Schreiben der Temp-Datei, vor dem
> Umbenennen**; der Runner sieht `.tmp-*`, eine `.json` mit 0 Bytes und eine mit
> 17 MiB" · Ereignis: „Takt; Auswahl im Runner" · Zusage: „**es entsteht keine
> `.json` der Länge 0**; die Temp-Datei bleibt als `.tmp-<pid>` liegen; der
> Runner ignoriert `.tmp-*` und verwirft `.json` unter 2 Bytes oder über 16 MiB
> als unbrauchbar (verfehlt, nie gewertet)" · Reihenfolge: „Umbenennen erst nach
> der Größenprüfung" · Rotbeweis: „**direktes Schreiben in den Zielnamen → die
> Injektion hinterlässt eine `.json` mit 0 Bytes → rot**" · Quelle: „Auftrag
> Etappe 1 („Dateigrößen …, leere Antwortdatei"); §2.1 F-8" · Etappe: „2, 3, 4".

**Gegenstand — F-8, `docs/beweise/NAK-286.md:359`** (die Zusage, die die Zeile
trägt): „In `<name>.tmp-<pid>` … schließen, Größe größer 0 prüfen, dann ohne
Ersetzen umbenennen … **Nie direkt in den Zielnamen.**"

**Status: BESTÄTIGT.** Der Ablauf der Zeile, Schritt für Schritt durchgespielt:

- *Grüner Weg:* Temp anlegen → Payload schreiben → schließen → Größe prüfen →
  umbenennen. Der Injektionspunkt liegt zwischen „schreiben" und „umbenennen".
- *Mutation „direktes Schreiben in den Zielnamen":* es gibt keine Temp-Datei und
  kein Umbenennen mehr. Der Injektionspunkt, so wie die Zeile ihn definiert
  („nach dem Schreiben"), liegt jetzt **nach** dem vollständigen Schreiben der
  Zieldatei. Die sichtbare `.json` trägt den ganzen Payload und ist nicht
  0 Bytes groß. Die behauptete Beobachtung („die Injektion hinterlässt eine
  `.json` mit 0 Bytes") tritt nicht ein.
- Rot würde der Fall trotzdem — aber an der **anderen** Zusage derselben Zeile,
  „die Temp-Datei bleibt als `.tmp-<pid>` liegen": unter der Mutation entsteht
  gar keine Temp-Datei. Genau das verbietet der Leitsatz aus `:345`: „je
  Matrixzeile fällt der Rotbeweis an der Zeile, die die Zusage trägt", nicht an
  einem Nebeneffekt.

Der Prüfer hat damit in beiden Teilen recht. Zwei Ergänzungen:

- Die Zusage „es entsteht keine `.json` der Länge 0" ist im grünen Weg **gar
  nicht erzeugbar**: solange über Temp und Umbenennen geschrieben wird, kann
  eine 0-Byte-`.json` per Konstruktion nicht entstehen. Die Zeile misst also
  eine Eigenschaft, die aus dem Aufbau folgt, statt aus einem Fehlerfall — der
  Fehlerfall, der sie brechen könnte (ein Schreiber, der die Zieldatei anlegt
  oder trunkiert und dann scheitert), wird von der Zeile nicht hergestellt.
- Die zweite Zusage kollidiert zusätzlich mit dem in §5.6 vorgeschriebenen
  Helfer (Befund 8): `TempPfad::drop` (`broker/src/persistenz.rs:19-22`) löscht
  die Temp-Datei bei einem Umbenennungsfehler, sodass sie auch im **grünen**
  Lauf nicht liegen bliebe. M-53 ist damit an zwei Enden defekt: der Rotbeweis
  trifft die falsche Zusage, und die getroffene Zusage hält der vorgeschriebene
  Schreibweg nicht.

**Einordnung (Vorschlag): DEFEKT** (der Beweisweg kann die Zusage der Zeile nicht
tragen; die Zeile fällt unter der Mutation an einem Nebeneffekt).

**Schließende Änderung (Vorschlag).** M-53 wird in zwei Lagen geteilt, jede mit
ihrem eigenen Injektionspunkt und ihrer eigenen Beobachtung:

- **(a) Keine leere Antwortdatei sichtbar.** Injektionspunkt der Fassade:
  **nach dem Anlegen/Trunkieren des Schreibziels, vor dem ersten Payload-Byte**
  („Schreibfehler beim ersten `write`"). Beobachtet wird der **Antwortordner**:
  nach dem Takt existiert keine Datei mit dem Muster
  `<32 hex>.<rolle>.<pid>.<16 hex>[.<n>].json` — weder leer noch teilweise.
  Rotbeweis: Mutation „direkt in den Zielnamen schreiben" → der Injektionspunkt
  liegt jetzt im Zielnamen → eine sichtbare `.json` mit 0 Bytes → rot, und zwar
  genau an dieser Zusage.
- **(b) Fehlgeschlagenes Umbenennen hinterlässt die Temp-Datei.**
  Injektionspunkt wie heute (nach dem Schreiben, vor dem Umbenennen).
  Beobachtet wird: genau eine Datei `<name>.tmp-<pid>` liegt noch da,
  Löschzähler der Fassade 0, keine `.json`. Rotbeweis: Mutation „Aufräumer
  löscht die Temp-Datei im Fehlerfall" (das heutige `Drop`-Verhalten von
  `persistenz::atomar_neu`) → 0 Temp-Dateien, Löschzähler 1 → rot. Diese Lage
  ist zugleich die Regressionswache gegen den in Befund 8 beschriebenen
  Rückfall auf `persistenz::atomar_neu`.
- Die Runner-Hälfte („ignoriert `.tmp-*`, verwirft `.json` unter 2 Bytes oder
  über 16 MiB") bleibt als dritte Lage in Etappe 4 (Selbsttest `antwortgroessen`,
  A35) unverändert; sie hängt nicht an der Fehlerinjektion und braucht nur die
  drei vorbereiteten Dateien.

---

## Befund 14 — „Halte Broker-Startfehler ohne Anfrage aus dem Register"

**Wörtlich (Urteil):** `[P2] … NAK-286.md:766-768 — DEFEKT §5.6/R-286-2: Bei
fehlendem `LOCALAPPDATA` soll der neue Briefkasten auch ohne Anfrage einen
Registerfehler hinzufügen. `fehler_merken` verändert jedoch die Fehlerliste
(`broker/src/register.rs:438–443`), die `broker_status` nach außen liefert
(`broker/src/lebenslauf.rs:364–375`). Damit entsteht genau die nach R-286-2
untersagte messbare Änderung ohne Anfragedatei. Die Neutralitätsprüfung M-41
deckt diesen Startpfad bislang nicht ab; Bauplan und Test müssen auch hier das
bisherige beobachtbare Brokerverhalten erhalten.`

**Gegenstand — Bauplan §5.6 Risiken (b), `docs/beweise/NAK-286.md:766-768`:**

> „(b) Fehlt `LOCALAPPDATA`, startet der Briefkasten nicht; **der Grund steht wie
> andere Startfehler im Register** (`fehler_merken`,
> `broker/src/lebenslauf.rs:131`), der Broker läuft weiter — kein stiller
> Ausfall."

**Gegenstand — Regel R-286-2, `docs/beweise/NAK-286.md:366`:**

> „**Ohne Anfragedatei ändert der Briefkasten nichts Messbares** — kein
> Host-Parameter, kein State-Feld, kein Feld auf der Leitung, keine
> Identitätsänderung, kein Zugriff auf dem Audio-Thread; …"

**Gegenstand — M-41, `docs/beweise/NAK-286.md:527`:**

> Zustand: „**Briefkasten-Griff mit Temp-Wurzel, Fassade und Testuhr**; keine
> Anfrage" · Ereignis: „600 Takte" · Zusage: „600 Existenzprüfungen, 0
> Leseöffnungen, 0 Schreibvorgänge, 0 Ordneranlagen, 0 Löschungen" · Rotbeweis:
> „Existenzprüfung entfernt, der Takt öffnet die Datei direkt → Lesezähler
> größer 0 → rot".

**Quelle an HEAD — `broker/src/register.rs:438-444`:**

```rust
    pub fn fehler_merken(&mut self, text: String) {
        // Begrenzte Liste — sichtbar, aber kein unbegrenztes Wachstum (Plan §8.3).
        if self.fehler.len() >= 50 {
            self.fehler.remove(0);
        }
        self.fehler.push(text);
    }
```

**`broker/src/lebenslauf.rs:357-372`** (Auszug):

```rust
pub fn broker_status() -> BrokerStatus {
    match BROKER.get() {
        Some(Ok(lauf)) if !lauf.beendet.load(Ordering::SeqCst) => {
            let register = lauf.register.lock().expect("Register-Mutex");
            …
                pakete_verworfen: register.pakete_verworfen,
                fehler: register.fehler.clone(),
```

**Quelle an HEAD — das Vorbild, das der Bauplan zitiert,
`broker/src/lebenslauf.rs:128-131`:**

```rust
            if let Some(pfad) = &bindungen_pfad {
                match bindung::bindungen_laden(pfad) {
                    Ok(b) => register.bindungen = b,
                    Err(e) => register.fehler_merken(format!("Profilbindungen: {e}")),
```

Die Zeilenangabe des Bauplans (`lebenslauf.rs:131`) stimmt exakt; das Muster
„Startfehler ohne Client sichtbar merken" existiert also bereits.

**Wie weit reicht „nach außen" heute?** Hier ist der Prüfer ungenauer als die
Quelle:

- Die v2- und v3-Leitung trägt die Fehlerliste **nicht**. `register.fehler` wird
  im ganzen Baum nur an vier Stellen gelesen: `lebenslauf.rs:372`
  (`broker_status`), `probe.rs:54` (Statusausgabe eines **Probelaufs** mit
  eigenem Register), und zwei Testzusicherungen (`server.rs:1589`, `:1627`).
- `broker_status()` hat im ganzen Repo **keinen Aufrufer**: `broker/src/main.rs`
  ruft `broker_starten`, `broker_hat_fatalen_v3_listenerfehler`,
  `broker_soll_idle_enden` und `broker_geordnet_stoppen`, aber nie
  `broker_status` (Suche über `*.rs`, `*.cpp`, `*.h`, `*.ps1`, `*.py`: nur die
  Definition und ein Kommentar in `lebenslauf.rs:13`).
- Der Fall selbst ist zudem eng: `LOCALAPPDATA` fehlt im Benutzerkontext
  praktisch nie; der Broker liest es heute schon so
  (`broker/src/aggregat.rs:139-142`, in F-1 zitiert).

**Status: TEILWEISE BESTÄTIGT.**

*Bestätigt:* Der Bauplan sieht in §5.6 (b) tatsächlich vor, ohne Anfragedatei
eine beobachtbare Zustandsänderung im Broker vorzunehmen — eine Zeile in der
Registerfehlerliste, die über die öffentliche Bibliotheksfunktion
`broker_status()` (und, mit derselben Struktur, über die Probe-Statusausgabe)
lesbar wird. Das steht im Spannungsverhältnis zum ersten Satz von R-286-2
(„ändert der Briefkasten nichts Messbares"). Ebenfalls bestätigt: **M-41 deckt
diesen Pfad nicht ab.** M-41 setzt einen bereits laufenden Griff mit Temp-Wurzel
voraus und zählt Fassadenaufrufe über 600 Takte; über den **Startpfad** und über
fehlendes `LOCALAPPDATA` sagt keine Matrixzeile etwas — „LOCALAPPDATA" kommt in
§4 an keiner Stelle vor (Suche über die ganze Datei: nur Gate, §0, §2.1 F-1,
§2.3 F-18, M-05, M-12, M-21, M-38, M-39, M-60, §5.6 (b), T-6). Für das Plugin
fehlt der Gegenpart erst recht: was geschieht, wenn
`juce::File::windowsLocalAppData` leer ist, steht nirgends.

*Nicht bestätigt in dieser Schärfe:* „Damit entsteht genau die nach R-286-2
untersagte messbare Änderung" trifft die Aufzählung der Regel nicht. R-286-2
verbietet namentlich Host-Parameter, State-Feld, **Feld auf der Leitung**,
Identitätsänderung und Audio-Thread-Zugriff. Die Registerfehlerliste ist keines
davon: sie geht nicht auf die Leitung, und `broker_status` wird heute von keinem
Produktweg gerufen. Die Änderung fällt unter den **allgemeinen** Satz („nichts
Messbares"), nicht unter die Aufzählung — deshalb ist es keine wörtliche
Regelverletzung, sondern eine Regel, die den Fall nicht entscheidet. Auch die
Zurechnung ist genauer zu fassen: die Zeile wird von `broker_starten`
geschrieben, nicht vom Briefkasten-Takt; der Briefkasten existiert in diesem
Fall gar nicht.

**Einordnung (Vorschlag): LÜCKE** (Matrix und Regeln schweigen zum Startpfad
ohne `LOCALAPPDATA` — in beiden Sprachen; der Bauplan entscheidet den Fall in
einem Risikoabsatz statt in einer Regel oder Matrixzeile). Sollte der Dirigent
den allgemeinen Satz von R-286-2 als bindend lesen, wird daraus ein DEFEKT des
Bauplanabsatzes; die schließende Änderung ist in beiden Fällen dieselbe.

**Schließende Änderung (Vorschlag).**

1. **Regel entscheiden.** R-286-2 bekommt einen Satz zum Startpfad, weil der
   allgemeine Satz ihn heute weder erlaubt noch verbietet — Vorschlag:
   „Startfehler des Briefkastens ändern nichts, was ein anderer Weg lesen kann;
   sie werden erst in der ersten Antwort sichtbar (`gruende`) und, wo eine
   Instanz überhaupt protokolliert, im Log — nie im Register, nie im Status."
   Das hält R-286-2 dicht und lässt „kein stiller Ausfall" trotzdem gelten.
2. **Bauplan §5.6 (b)** (`:766-768`) wird darauf umgeschrieben: fehlt
   `LOCALAPPDATA`, startet der Briefkasten nicht, merkt sich den Grund
   **instanzlokal** (Testzähler und Grundtext) und ändert das Register nicht;
   der Broker läuft unverändert weiter. `fehler_merken` bleibt den bestehenden
   Startfehlern vorbehalten.
3. **Matrix.** Zwei neue Zeilen, je Sprache eine, in derselben Runde — sonst
   bleibt der Fall wieder unbelegt:
   - Broker: „`LOCALAPPDATA` fehlt (Fassade meldet die Variable als nicht
     gesetzt) → kein Thread gestartet (Startzähler 0), Registerfehlerliste vor
     und nach `broker_starten` **bytegleich**, `broker_status().fehler`
     unverändert, Broker läuft". Rotbeweis: Mutation „Grund über
     `fehler_merken` eintragen" → Listenvergleich rot. Bein A4, Etappe 3.
   - Plugin: „`windowsLocalAppData` liefert keinen gültigen Ordner → kein Takt
     ausgelöst, 0 Fassadenaufrufe, State und Parameter bytegleich (wie M-37)".
     Rotbeweis: Mutation „Wurzel ungeprüft benutzt" → Existenzprüfungen größer 0
     → rot. Bein B30, Etappe 2.
4. **M-41** (`:527`) bekommt den Zusatz, dass die Zeile den **laufenden** Griff
   misst, und verweist für den Startpfad auf die neue Zeile — damit niemand die
   Neutralität für belegt hält, die M-41 nicht misst.

---

## Gemeinsame Ursachen

Die Vermutung des Dirigenten trifft in beiden Hälften zu, mit einer Verschiebung
bei der zweiten Gruppe.

**Ursache 1 — Identität und Einmaligkeit hängen an zu kleinen, verdrängenden
Behältern (Befunde 6 und 7).** Zweimal löst dieselbe Denkfigur dasselbe Problem
falsch: Eindeutigkeit wird durch einen **endlichen, überlaufenden Speicher**
hergestellt (256 Ringplätze für Kennungen, neun Namensplätze für Instanzen), und
in beiden Fällen ist der Überlauf als Sollverhalten festgeschrieben, obwohl
R-286-1 keinen Überlauf kennt. Beide Male steht im Produkt bereits das richtige
Mittel bereit und wird nicht benutzt: ein vollständiges In-Flight-Register
beziehungsweise eine monotone Sequenz (`Ipc.cpp:783-791`,
`Hostbruecke.cpp:110-114`) für die Kennung, und die flüchtige, per Instanz
eindeutige `runtime_nonce` (`PluginProcessor.cpp:243`, `SondeProcessor.cpp:138`)
für den Namen. Ein einziger Auftrag „Eindeutigkeit nicht über verdrängende
Behälter, sondern über die vorhandenen eindeutigen Größen; wo eine Grenze
bleibt, gehört sie in die Regel, nicht in die Zusage" schließt beide Befunde.

**Ursache 2 — der Schreibweg im Broker ist geliehen, nicht gebaut (Befunde 8, 12
und teils 14).** F-8 und §5.6 borgen `persistenz::atomar_neu`, dessen Semantik
für einen anderen Zweck gebaut ist (`aggregat_schreiben`: Ordner darf fehlen,
Temp darf verschwinden) und die vier Zusagen der Matrix verletzt; M-53 baut
seinen Rotbeweis auf dieser geliehenen Semantik auf und trifft deshalb den
falschen Nebeneffekt; und der Ordner- und Pfadteil derselben Leihe erzeugt in
§5.6 (b) den Startfehlerfall, der Befund 14 auslöst. Der gemeinsame Auftrag ist
ein **eigener, vollständig über die F-14-Fassade laufender Schreibweg im
Briefkastenmodul** mit den fünf Schritten aus Befund 8; M-53 und die neuen
Startpfadzeilen aus Befund 14 hängen sich daran.

Befund 14 gehört nur zur Hälfte in diese Gruppe: sein zweiter Teil — Matrix und
Regeln schweigen zum Startpfad — ist eine eigenständige Lücke, die auch im
Plugin klafft und dort noch niemand gemeldet hat.

Befund 11 hat mit beiden nichts zu tun; er ist ein Beweiswegfehler der
Lebenszykluszeile und steht für sich.

---

## Übersicht

| Befund | Kurzname | Status | Einordnung (Vorschlag) | Schließende Änderung (Kurz) |
|---|---|---|---|---|
| 6 | Einmal-Zusage nach dem Kennungs-Wrap (M-51/F-13/T-2, `:547`) | BESTÄTIGT | DEFEKT | R-286-1 bekommt den Geltungsrahmen des Rings, **oder** die Grenze wird aus einer gemessenen Obergrenze je Instanzlebensdauer hergeleitet; M-51 bekommt zusätzlich einen Rotbeweis an der Einmal-Zusage (Ring nicht gefragt → zweite Antwort auf eine noch gemerkte Kennung); T-2 argumentiert über Instanzlebensdauer statt über einen Lauf |
| 7 | Zehnte Instanz mit gleicher Kennung (F-3/M-33, `:354`) | BESTÄTIGT | DEFEKT | Antwortname nimmt die vorhandene Laufzeit-Nonce auf (`<anfrage_id>.<rolle>.<pid>.<instanz16>.<nonce8>.json`); Suffixleiter gibt nie auf; M-33 bekommt die Lage „zehn Instanzen, zehn Dateien" mit Rotbeweis an dieser Zusage; T-3 begründet die Namenszusammensetzung |
| 8 | Writer mit den zugesagten Dateisemantiken (§5.6/F-8, `:746`) | BESTÄTIGT (fünf Abweichungen statt zwei) | DEFEKT | eigener Schreibweg in `briefkasten.rs` **über die F-14-Fassade**: keine Ordneranlage, Temp exakt `<name>.tmp-<pid>`, Größenprüfung, Umbenennen ohne Ersetzen, kein Drop-Aufräumer; F-8 und §5.6 Punkt 1 entsprechend umschreiben; `persistenz.rs` bleibt unberührt |
| 11 | Lebensdauer-Rotbeweis verletzt die Join-Frist (M-45, `:531`) | BESTÄTIGT | DEFEKT | M-45 in drei Zusagen mit je einem deterministischen Beweis zerlegen: Weckruf über Zähler „Wartedurchläufe nach dem Stoppsignal" statt über Sekunden; eigene Lage „nach dem Stopp keine Existenzprüfung, keine Antwort"; Reihenfolge bleibt; die 2-s-Frist bleibt Zusage, wird kein Rotbeweis |
| 12 | Leerdateifehler vor dem ersten Schreibbyte (M-53, `:549`) | BESTÄTIGT | DEFEKT | M-53 in zwei Lagen teilen: (a) Injektion nach Anlegen/Trunkieren, vor dem ersten Payload-Byte, Beobachtung am **sichtbaren** Antwortordner; (b) Injektion vor dem Umbenennen, Beobachtung „Temp bleibt liegen, Löschzähler 0" mit dem Drop-Verhalten als Mutation; Runner-Hälfte bleibt |
| 14 | Broker-Startfehler ohne Anfrage (§5.6/R-286-2, `:766-768`) | TEILWEISE BESTÄTIGT (Registerzeile und M-41-Lücke ja; „Feld auf der Leitung" nein — `broker_status` hat heute keinen Aufrufer, die Liste geht nicht auf die Leitung) | LÜCKE (DEFEKT, falls der Dirigent den allgemeinen Satz von R-286-2 als bindend liest) | R-286-2 entscheidet den Startpfad ausdrücklich (Grund erst in der Antwort, nie im Register); §5.6 (b) darauf umschreiben; zwei neue Matrixzeilen (Broker A4, Plugin B30) für fehlendes `LOCALAPPDATA` mit Rotbeweis; M-41 stellt klar, dass es nur den laufenden Griff misst |
