# NAK-283 Erstprüfung 3 — Validierung des einen Befundes (Heilungstakt über Sitzungen)

**Stand:** Branch `master`, HEAD `34a127e5`; Etappenstand `30a0b1bf`, Basis
`a754a992`, Produktcommit `6ce14429`. `git diff --stat 30a0b1bf HEAD --
broker/` ist **leer** — jedes Rust-Zitat dieser Datei gilt unverändert für HEAD.
`git status --short` zeigt nur die zwei fremden untracked Ordner
(`briefing-hub/`, `nimbalyst-local/`). Nur gelesen; einzige Schreibaktion ist
diese Datei.

**Befund (Codex Astra, Erstprüfung 3, Thread `01a09799`), wörtlich:**

> [P2] Begrenze den Heilungstakt auf die überholten Sitzungen —
> broker/src/coordinator/hypothese_verdrahtung.rs:108-109. DEFEKT bei zwei
> Sitzungen mit Vollständigkeitsmarke auf getrennten Host-PIDs: Am Rechenhaken
> verliert S1 wie in M-16/Fenster 1 ein unbenutztes Masterfenster
> (`Ueberholt`), während S2 wie in M-15 eine Intentänderung erhält
> (`Veraltet`). S2s veröffentlichter Befund wird stale; das laufende Ergebnis
> wird noch vor dem Cache-Eintrag verworfen. Die sitzungsübergreifende
> ODER-Verknüpfung startet hier jedoch eine Heilung, deren `aufnahmen_sammeln`
> erneut **alle** Sitzungen aufnimmt: Auch S2 wird ohne neue Evidenz frisch
> gerechnet und verliert stale. Das verletzt R-283-7 (docs/beweise/NAK-283.md
> §31): außerhalb der Cache-Persistenz-Naht „stale bis neues Material, kein
> Nachrechnen". BASIS hatte diesen Folgeaufruf nicht. Der Heilungsbedarf muss
> deshalb sitzungsbezogen bleiben. Lesender Pfadnachweis:
> `git show 30a0b1bf:broker/src/coordinator/hypothese_verdrahtung.rs`,
> insbesondere 107–175, 208–231 und 551–568.

---

## 1. Der Ablauf am Zielstand `30a0b1bf`

### 1.1 Ist `befunde_neu_bilden` ein einziger globaler bool im `Stand`? — **WAHR**

`broker/src/coordinator/zustand.rs:359-363`:

```
:359    /// Der Evidenzbestand hat sich geaendert, und die Befunde stehen noch auf
:360    /// dem alten Stand. Dasselbe Muster wie `paare_neu_bilden`: gesetzt UNTER
:361    /// dem Standlock, eingeloest ausserhalb — die Rechnung nimmt sich ihr
:362    /// eigenes Lock und darf den Sessiongraphen nicht anhalten.
:363    pub(super) befunde_neu_bilden: bool,
```

Ein einzelnes `bool`, **nicht** nach `SessionKey` geschlüsselt — anders als
`stand.befunde` (`:358`, `HashMap<SessionKey, …>`) oder `dirty_sessions`.
Initialisierung `zustand.rs:603`. Sämtliche Berührungen im Produkt
(`git grep -n "befunde_neu_bilden" 30a0b1bf -- broker/src`): fünf Setzer
(`hypothese_verdrahtung.rs:554`, `:565`, `:671`; `intent.rs:626`;
`invalidierung_verdrahtung.rs:279`), zwei Leser/Löscher
(`hypothese_verdrahtung.rs:173`, `:204`), Feld und Default in `zustand.rs`.
Keine dieser Stellen trägt eine Sitzung.

Die Zeile ist **identisch mit dem BASIS-Stand**: `git diff a754a992..30a0b1bf
-- broker/src/coordinator/zustand.rs` ändert dort nur
`befund_schreibversuche_unterlassen` (+6 Zeilen, Feld und Default);
`befunde_neu_bilden` steht an beiden Ständen auf `:363`.

### 1.2 Ist der Rückgabewert von `hypothesen_rechnen` ein ODER über alle Sitzungen? — **WAHR**

`hypothese_verdrahtung.rs:141-157`:

```
:141        let mut ueberholt = false;
:142        for (session, befunde, eingangsmenge) in ergebnisse {
:143            let eintragung = self.befunde_eintragen(&session, befunde, &eingangsmenge);
:144            ueberholt |= eintragung.ueberholt;
…
:157        ueberholt
```

`ueberholt` ist ein `bool` über die ganze Schleife; welche Sitzung ihn gesetzt
hat, verlässt die Funktion nicht. `Eintragung` (`:77-86`) trägt die Sitzung
ebenfalls nicht.

### 1.3 Nimmt `aufnahmen_sammeln` alle Sitzungen mit Vollständigkeitsmarke auf? — **WAHR**

`hypothese_verdrahtung.rs:195-212` (Sitzungsmenge) und `:229-231` (der einzige
Riegel):

```
:195    fn aufnahmen_sammeln(&self) -> Vec<(SessionKey, Aufnahme, BTreeSet<String>)> {
:196        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
…
:204        stand.befunde_neu_bilden = false;
…
:208        let mut sitzungen: BTreeSet<(String, String)> = BTreeSet::new();
:209        for key in stand.evidenz.keys() {
:210            sitzungen.insert((key.project_binding_id.clone(), key.session_epoch.clone()));
:211        }
…
:229            let intent = stand.intent.get(&session).cloned();
:230            if !super::intent::darf_gerechnet_werden(intent.as_ref()) {
:231                continue;
:232            }
```

Die Menge entsteht aus **allen** Evidenzschlüsseln des Stands; gefiltert wird
nur mit NR-01 (Vollständigkeitsmarke, `:230`), mit „kein `main`-Client"
(`:383-385`) und in `hypothesen_rechnen` mit „keine Aufnahme"
(`:122-124`). Es gibt **keinen** Parameter und **keinen** Merker, der die
Sitzungsmenge eines Laufs einschränkt. Die Funktion hat genau zwei Aufrufer,
beide ohne Sitzung: `:121` und (über `hypothesen_rechnen`) `:175`.

Zusatzbefund zur Frist: `:204` löscht den globalen Merker **vor** dem Sammeln.
Ein Merker, den eine andere Sitzung gesetzt hat (etwa `intent.rs:626` für S2),
wird damit von **jedem** Rechenlauf eingelöst, auch wenn dieser Lauf aus einem
anderen Anlass startet. Der Kommentar `:197-203` benennt genau diese Kopplung
als die beabsichtigte Wirkung der Zeile.

### 1.4 Löst `hypothesen_bilden` nach einem `Ueberholt`-Verwurf in S1 einen Lauf aus, der S2 mitrechnet? — **WAHR**

`hypothese_verdrahtung.rs:107-111`:

```
:107    pub(super) fn hypothesen_bilden(&self) {
:108        if self.hypothesen_rechnen() {
:109            self.hypothesen_bei_bedarf_bilden();
:110        }
:111    }
```

und `:169-179`:

```
:169    pub(super) fn hypothesen_bei_bedarf_bilden(&self) {
:170        loop {
:171            let noetig = {
:172                let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
:173                std::mem::take(&mut stand.befunde_neu_bilden)
:174            };
:175            if !noetig || !self.hypothesen_rechnen() {
:176                return;
:177            }
:178        }
:179    }
```

Ablauf im beschriebenen Fall: S1 fällt in `befunde_eintragen` auf
`Rueckschreibung::Ueberholt` (`:560-570`, Setzer `:565`, Rückgabe
`ueberholt: true`); S2 fällt auf `Rueckschreibung::Veraltet` (`:553-559`,
Setzer `:554`, Rückgabe `ueberholt: false`). Das ODER aus `:144` ist `true`;
`:109` ruft den Takt; `:173` nimmt den Merker (den beide Zweige gesetzt haben);
`:175` ruft `hypothesen_rechnen` → `aufnahmen_sammeln` → **S1 und S2**.

Der Riegel, der S1 auf `Ueberholt` statt `Veraltet` führt, steht in
`ergebnis_ist_noch_gueltig` (`:1183-1217`): die Prüfungen (a) Generation
(`:1194-1204`) und (b) verwendete `evidence_id` (`:1205-1211`) laufen **vor**
dem Mengenvergleich (`:1213-1215`). Ein zurückgenommenes **unbenutztes**
Masterfenster ändert `gueltige_evidenz_ids_locked` (`:1227-1240`), lässt aber
Generation und verwendete IDs unberührt — genau der Ausgang `Ueberholt`. Eine
gestiegene Intentgeneration fällt schon an (a) — genau der Ausgang `Veraltet`.
Die Bühnenannahme des Prüfers ist am Code also erreichbar.

### 1.5 Wird der `Veraltet`-Verwurf in S2 durch die Neurechnung als READY eingetragen, obwohl S2 kein neues Material bekam? — **WAHR** (mit einer Präzisierung)

Drei Belege:

1. **Frisch gerechnet ist nie `stale`.**
   `broker/src/coordinator/hypothese/befund.rs:278-281`:

   ```
   :278    // M-29: die EINE Abbildung im Datenweg. Frisch gerechnet ist ein Befund
   :279    // nie veraltet — `stale` entsteht erst, wenn sich unter ihm etwas aendert
   :280    // (Intent-Revision gestiegen, Belege zurueckgenommen).
   :281    let zustand = zustand_aus_sicherheit(klasse, false);
   ```

   `hypothese/klassenwahl.rs:112-122` liefert bei `veraltet == false` je nach
   Sicherheitsklasse `ReadyToSend` oder `MoreData`. Alle drei Produktaufrufer
   übergeben `false` (`befund.rs:281`, `enthaltung.rs:146`, `:191`,
   `rechnung.rs:297`). **Präzisierung:** „READY" gilt nur bei
   `Sicherheitsklasse::Hoch`; sonst `more_data`. Entscheidend und in jedem Fall
   wahr ist: der Zustand `stale` ist nach der Neurechnung fort.

2. **Der neue Eintrag ersetzt den alten.** `hypothese_verdrahtung.rs:572-585`:
   der Gleichheitsvergleich `alt.map(Vec::as_slice) == Some(befunde.as_slice())`
   trifft nicht zu, weil der Standeintrag `zustand == Stale` trägt
   (`befunde_veralten_locked` `:1248-1267`, Setzer `:1263`) und das neue
   Ergebnis nicht; also `stand.befunde.insert(...)` (`:583`).

3. **Der Riegel lässt die Neurechnung passieren.** `:1194-1204` vergleicht die
   Generation des frisch gerechneten Befunds gegen `stand.intent[...].generation`
   — beide sind jetzt die neue Generation; `:1213-1215` vergleicht die
   Eingangsmenge, die sich für S2 nicht geändert hat. Ergebnis `Gueltig`.

Neues Material für S2 ist an keiner Stelle dieses Wegs verlangt.

### 1.6 Braucht die Neurechnung von S2 die Vollständigkeitsmarke (NR-01)? — **WAHR, und sie ist nach der Intentänderung weiter gesetzt**

Geprüft wird an **einer** Stelle, `hypothese_verdrahtung.rs:229-232` (zitiert in
1.3), über `intent.rs:136-138`:

```
:136    pub fn darf_gerechnet_werden(intent: Option<&IntentBestand>) -> bool {
:137        intent.is_some_and(|b| b.vollstaendig)
:138    }
```

Gelesen wird der **aktuelle** Bestand (`stand.intent.get(&session)`), also der
der neuen Generation. Dass die Marke die Intentänderung überlebt, steht in
`intent.rs:526-548`: ein Vollbericht setzt `vollstaendig: true` (`:534`), ein
Teilupdate klont den bestehenden Bestand und behält das Feld (`:540-547`); die
Generation steigt in beiden Fällen (`:526`, `:536`, `:546`).

Was `intent.rs:620-630` beim Eintreffen der Marke setzt:

```
:622            let veraltet =
:623                Coordinator::befunde_veralten_locked(&mut stand, &session, naechste_generation);
:624            // Und die Sitzung wird als neu zu rechnen gefuehrt: beim naechsten
:625            // Material ruht die Aussage auf dem vollstaendigen Bestand.
:626            stand.befunde_neu_bilden = true;
:627            (session, veraltet)
```

Also: veröffentlichte Befunde sichtbar `stale` (`:622-623`) **und** der globale
Merker gesetzt (`:626`). Der Kommentar `:624-625` plant die Neurechnung
ausdrücklich ein — „beim naechsten Material". Die Zeile ist **BASIS-gleich**:
`git diff --stat a754a992..30a0b1bf -- broker/src/coordinator/intent.rs` ist
leer; `intent.rs:626` steht an beiden Ständen auf derselben Zeile.

---

## 2. Vergleich mit dem BASIS-Stand `a754a992`

### 2.1 Rechnete der Materialtakt am BASIS ebenfalls ALLE Sitzungen? — **WAHR**

`git show a754a992:broker/src/coordinator/hypothese_verdrahtung.rs:72-106` ist
der Rechenlauf selbst:

```
:72     pub(super) fn hypothesen_bilden(&self) {
:73         let aufnahmen = self.aufnahmen_sammeln();
:74         if aufnahmen.is_empty() {
:75             return;
:76         }
…
:92         for (session, befunde) in ergebnisse {
:93             let geaendert = self.befunde_eintragen(&session, befunde);
```

und `aufnahmen_sammeln` am BASIS (`:130-138`) bildet die Sitzungsmenge
wortgleich aus `stand.evidenz.keys()`, mit demselben NR-01-Riegel. Der Aufrufer
ist `broker/src/coordinator/evidenz.rs:232` (`self.hypothesen_bilden();`) —
**ohne Sitzungsargument**. `git diff --stat a754a992..30a0b1bf --
broker/src/coordinator/evidenz.rs` ist leer: der Materialtakt ist an beiden
Ständen bytegleich und an beiden Ständen sitzungsübergreifend.

Damit gilt: **auch am BASIS wurde S2 ohne eigenes Material gerechnet, sobald S1
Material bekam.**

### 2.2 Wurde ein `stale`-Befund von S2 am BASIS beim nächsten Materialeingang von S1 neu gerechnet und der `stale`-Zustand verloren? — **WAHR**

`ergebnis_ist_noch_gueltig` am BASIS (`:961-997`) prüft (a) Generation
(`:969-979`) und (b) verwendete IDs (`:980-996`) — beide bestehen für die
frische Rechnung unter der aktuellen Generation. `befunde_eintragen`
(`:443-477`) trägt ein (`:475`). Und `broker/src/coordinator/hypothese/**` ist
zwischen BASIS und Ziel **bytegleich** (`git diff --stat a754a992..30a0b1bf --
broker/src/coordinator/hypothese/` ist leer), also gilt `befund.rs:278-281`
(„Frisch gerechnet ist ein Befund nie veraltet") am BASIS genauso.

Der `stale`-Zustand aus einer Intentänderung hielt am BASIS folglich **bis zum
nächsten Rechenlauf, gleich welche Sitzung ihn auslöste**, nicht bis zum
nächsten Material der eigenen Sitzung.

### 2.3 Gab es am BASIS bereits Läufe ohne Materialeingang, und rechneten sie alle Sitzungen? — **WAHR**

`git show a754a992:broker/src/coordinator/invalidierung_verdrahtung.rs:115-128`
(Storefehler-Rückweg):

```
:115            Err(()) => {
:116                // ZUERST der Rueckweg des Ausloesers: …
:119                rueckweg(self);
:120                self.invalidierung_ruecknehmen(wirkung);
…
:126                self.hypothesen_bei_bedarf_bilden();
```

(am Zielstand dieselbe Stelle, `:115-128` mit dem Aufruf auf `:126`).
`invalidierung_ruecknehmen` setzt dort den Merker (`:258` am BASIS, `:279` am
Ziel), und `hypothesen_bei_bedarf_bilden` (BASIS `:109-117`) rief
`hypothesen_bilden` (`:115`) — also wieder **alle** Sitzungen. Ein Lauf ohne
Materialeingang, der jede Sitzung mit Marke frisch rechnet, existierte damit
bereits am BASIS.

Verschärfend am BASIS: `aufnahmen_sammeln` löschte den Merker **nicht** (der
`grep` nach `befunde_neu_bilden` am BASIS zeigt nur die Setzer `:464`,
`intent.rs:626`, `invalidierung_verdrahtung.rs:258` und den einen Leser `:112`).
Ein von `intent.rs:626` für S2 gesetzter Merker blieb deshalb stehen, bis
irgendein Storefehler-Rückweg — auch der einer **anderen** Sitzung — ihn
einlöste und alle Sitzungen rechnete.

### 2.4 Die Aufrufer, beide Stände

| Anlass | BASIS `a754a992` | Ziel `30a0b1bf` |
|---|---|---|
| Materialeingang | `evidenz.rs:232` → `hypothesen_bilden` (`:72`) = Rechenlauf über alle Sitzungen | `evidenz.rs:232` → `hypothesen_bilden` (`:107`) → `hypothesen_rechnen` (`:120`), Rechenlauf über alle Sitzungen |
| Verwurf nach R-283-1 (Fenster 1 oder 2) | kein Aufruf; nur Merker `:464` | **neu:** `:108-109` ruft `hypothesen_bei_bedarf_bilden` (`:169`), Schleife über weitere Läufe |
| Storefehler-Rückweg der Invalidierung | `invalidierung_verdrahtung.rs:118` → `hypothesen_bei_bedarf_bilden` (`:109`) → `hypothesen_bilden` (`:115`) | `invalidierung_verdrahtung.rs:126` → `hypothesen_bei_bedarf_bilden` (`:169`) |
| Merker beim Sammeln gelöscht | nein | **neu:** `:204` |

---

## 3. Was die Fachquellen über die DAUER des `stale`-Zustands sagen

### 3.1 SONDE-014 M-10 (`docs/beweise/SONDE-014.md:724`)

> **Vorschläge werden stale, nie still umgerechnet.** Jede auf Intent basierende
> Empfehlung referenziert die verwendete **Intent-Revision**; steigt die
> Revision, geht jedes abhängige Proposal und jeder abhängige Befund sichtbar in
> `STALE`, ohne dass Zahlen nachgerechnet werden.

Keine Dauer, kein Sitzungsbezug. Die Zeile bindet den **Veraltungsschritt**
(„steigt die Revision → geht in STALE, ohne Nachrechnen"), nicht die Zeit
danach.

### 3.2 SONDE-014 M-24 (`:743`)

> **Entfernen eines Evidenzobjekts invalidiert abhängige Hypothesen
> deterministisch.** Reihenfolge unter **einem** Lock: Umfang auflösen … → jede
> Hypothese, deren `evidence_ids` eine davon enthält, terminal invalidieren →
> Projektion in `findings` → Outbox. „Deterministisch" heißt: dieselbe Rücknahme
> in anderer Reihenfolge trifft dieselbe Menge.

„terminal" beschreibt den Ausgang **dieser** Rücknahme (die Hypothese bleibt
nicht gültig zurück), nicht eine Sperre gegen spätere Rechnungen. Keine Dauer,
kein Sitzungsbezug.

### 3.3 SONDE-014 M-29 (`:2332`)

> `sonde014_befund.rs::hoehere_intent_revision_macht_den_befund_stale` — der
> Zustand wechselt, und Rang, Beobachtung, Evidenz-IDs und `finding_id` stehen
> **unverändert** (§37.3: „ohne dass Zahlen nachgerechnet werden")

Eine Rotbeweiszeile über den Veraltungsschritt. Keine Dauer.

### 3.4 Entwurf §37.3 (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2463-2465`)

> Intent verändert nur Ranking, Schutz und erlaubte Tests. Er verändert keine
> Messwerte. Jede darauf basierende Empfehlung referenziert die verwendete
> Intent-Revision. Ändert der User die Rolle, werden Vorschläge stale, nicht
> still umgerechnet.

Keine Dauer, kein Sitzungsbezug, kein Wort „Material".

### 3.5 Der einzige Text mit „bis neues Material": ein Codekommentar

`broker/src/coordinator/invalidierung_verdrahtung.rs:204-211`:

```
:204        // ⚠️ Und genau deshalb wird hier NICHT `befunde_neu_bilden` gesetzt.
:205        // „Terminal" heisst: diese Ruecknahme laesst die Hypothese nicht
:206        // gueltig zurueck. Eine Neurechnung unmittelbar danach rechnete sie
:207        // aus den VERBLIEBENEN Belegen frisch auf und haette den Zustand
:208        // `stale` in derselben Bewegung wieder weggeraeumt — die
:209        // Invalidierung waere ein Flackern gewesen, kein Zustand. Neu
:210        // gerechnet wird erst, wenn NEUES Material ankommt; dann ruht die
:211        // Aussage auch wirklich auf neuem Material.
```

Das ist die Herkunft der Formel „stale bis neues Material". **Zwei Einschränkungen
an der Quelle:** (a) es ist ein Kommentar im Code, keine Matrix-, Vertrags- oder
Entwurfszeile; (b) er ist **nicht sitzungsbezogen** — „wenn NEUES Material
ankommt" bedeutet im Produkt `evidenz.rs:232`, und dieser Takt rechnet seit
jeher alle Sitzungen (2.1). Der Kommentar ist zwischen BASIS und Ziel
unverändert (BASIS `:195-202`).

`broker/src/coordinator/intent.rs:592-600`:

```
:592            // 🔑 SONDE-014 Etappe D (§37.3, M-29): eine gestiegene
:593            // Bestandsrevision macht jeden Befund darunter SICHTBAR `stale`.
:594            //
:595            // §37.3 woertlich: „Steigt sie, geht der Befund sichtbar in
:596            // `stale`, ohne dass Zahlen nachgerechnet werden." Genau das
:597            // passiert hier — kein Neurechnen, nur der Zustand. Der Grund ist
:598            // derselbe wie bei der Evidenzruecknahme: eine Aussage, die unter
:599            // einer alten Absicht entstanden ist, darf nicht weiter als
:600            // handelbar dastehen, nur weil niemand sie angefasst hat.
```

„kein Neurechnen, nur der Zustand" bezieht sich ausdrücklich auf **diese
Stelle** („passiert hier"), also auf den Veraltungsschritt. Die Fortsetzung
desselben Blocks, `:624-626` (zitiert in 1.6), plant die Neurechnung
ausdrücklich ein.

### 3.6 Der bestehende Test misst den Schritt, nicht die Dauer

`broker/tests/sonde014_befund.rs:317-358`, `hoehere_intent_revision_macht_den_befund_stale`:

- `:320-323` Bühne mit zwölf Fenstern, Ausgangszustand `ReadyToSend`;
- `:326-340` **ein** `intent_update` mit `bestand_revision + 1`;
- `:342-354` unmittelbar danach: Zustand `Stale`, `rang`, `beobachtung`,
  `evidence_ids` und `finding_id` unverändert;
- `:356-357` der Snapshot trägt `"stale"`.

Danach endet der Fall. Es folgt **kein** weiterer Beleg, **kein** zweiter
Rechenlauf, **keine** zweite Sitzung. Gemessen ist ausschließlich der
Veraltungsschritt; die **Dauer** des `stale`-Zustands ist an keiner Stelle
gemessen. (Derselbe Testkörper existiert am BASIS unverändert —
`broker/tests/` ist von der Etappe nur in `sonde013_verdrahtung.rs` und
`sonde014_verdrahtung.rs` berührt.)

---

## 4. Scope: liegt das Verhalten im Diff dieser Etappe?

### 4.1 Ist der Aufruf `:109` neu gegenüber BASIS? — **WAHR**

`git diff a754a992..30a0b1bf -- broker/src/coordinator/hypothese_verdrahtung.rs`,
erster Hunk der Auslöser:

```
     pub(super) fn hypothesen_bilden(&self) {
+        if self.hypothesen_rechnen() {
+            self.hypothesen_bei_bedarf_bilden();
+        }
+    }
```

Ebenfalls neu im selben Diff: die Schleifenform von
`hypothesen_bei_bedarf_bilden` (`:169-179`, BASIS: ein einmaliger Aufruf
`:109-117`), die drei Ausgänge `Rueckschreibung` (`:60-74`), das `Eintragung`-
Paar (`:76-86`), die Eingangsmenge und `stand.befunde_neu_bilden = false`
(`:204`).

### 4.2 Liegt „Rechenlauf umfasst alle Sitzungen" im Diff? — **FALSCH, vorbestehend**

Im selben Diff stehen die tragenden Zeilen als **Kontext**, nicht als Änderung:

```
         let mut sitzungen: BTreeSet<(String, String)> = BTreeSet::new();
         for key in stand.evidenz.keys() {
             sitzungen.insert((key.project_binding_id.clone(), key.session_epoch.clone()));
         }
```

Geändert sind in `aufnahmen_sammeln` nur die Signatur (Eingangsmenge), der
`mut`-Guard und die Zeile `:204`. Die Sitzungsmenge eines Laufs, der
NR-01-Riegel und die Abwesenheit jedes Sitzungsfilters sind unverändert seit
dem BASIS.

### 4.3 Die kritisierte Zeile trägt bereits einen gemessenen Rotbeweis

`docs/beweise/NAK-283.md:3606` (§29.2, Obergrenze zu M-16):

> \| M-16 (Obergrenze) \| derselbe Fall, Zählzeile von Fenster 1 \| A4 \|
> MUT-16c, `hypothese_verdrahtung.rs:108-110`: nach dem Heilungstakt läuft eine
> zweite Rechnung → zwei statt einer Neurechnung (Abweichung 13) \|
> `NAK-283-rot-M-16c-etappe-3.txt` \| 1× Exit 101 / Exit 0 \|

Gemessen ist damit an genau diesen Zeilen die **Anzahl** der Läufe in einer
Bühne mit **einer** Sitzung („genau eine Neurechnung"). Die **Reichweite** des
Takts über mehrere Sitzungen misst keine Zeile der Matrix.

---

## 5. Teilbehauptungen des Befundes, je Urteil

| Teilbehauptung | Urteil | Beleg |
|---|---|---|
| Fundstelle `hypothese_verdrahtung.rs:108-109` | **wahr** | `:108` `if self.hypothesen_rechnen() {`, `:109` `self.hypothesen_bei_bedarf_bilden();` |
| Zwei Sitzungen mit Marke sind am Code erreichbar | **wahr** | Sitzungsmenge aus allen Evidenzschlüsseln `:208-211`; Riegel nur NR-01 `:230` |
| S1 verliert ein unbenutztes Masterfenster und fällt auf `Ueberholt` | **wahr** | `:1213-1215` Mengenvergleich nach (a) und (b); `:1227-1240` Mengenbildung; Ausgang `:560-570` |
| S2 erhält eine Intentänderung und fällt auf `Veraltet` | **wahr** | `:1194-1204` Generationsvergleich zuerst; Ausgang `:553-559` |
| S2s veröffentlichter Befund wird `stale` | **wahr** | `intent.rs:622-623` → `befunde_veralten_locked` `:1248-1267`, Setzer `:1263` |
| Die ODER-Verknüpfung startet die Heilung | **wahr** | `:141`, `:144`, `:157`, ausgewertet an `:108` |
| Die Heilung nimmt erneut **alle** Sitzungen auf | **wahr** | `:175` → `:121` → `:195-232`, kein Sitzungsfilter |
| S2 wird ohne neue Evidenz frisch gerechnet und verliert `stale` | **wahr** | `befund.rs:278-281` (nie veraltet), `:572-585` (Ersetzung), `:1213-1216` (Riegel passiert) |
| „BASIS hatte diesen Folgeaufruf nicht" | **wahr** | BASIS `:72-106` ohne Folgeaufruf; Diff 4.1 |
| Implizit: der Lauf über alle Sitzungen sei Teil dieses Bruchs | **falsch** | vorbestehend, 4.2 und 2.1 — am BASIS rechnete jeder Materialtakt und jeder Storefehler-Takt alle Sitzungen |
| Implizit: „stale bis neues Material" sei am BASIS sitzungsscharf gewesen | **falsch** | 2.2 und 2.3 — am BASIS heilte Material **jeder** Sitzung den `stale`-Befund von S2, und der Storefehler-Rückweg ebenso |
| „Das verletzt R-283-7" | **wahr, aber nur gegen R-283-7** | siehe §6 — gebrochen ist ausschließlich der zweite Satz von R-283-7, unter sitzungsscharfer Lesart von „neues Material" |

---

## 6. Welcher Satz bricht

**Genau einer.** `docs/beweise/NAK-283.md:3916` (§31, Zeile „§29.7 Punkt 2 —
Fenster-2-Heilung"), letzter Teil von R-283-7, wörtlich:

> Außerhalb der Naht bleiben SONDE-014 M-24 und §37.3 unverändert: `stale` bis
> neues Material, kein Nachrechnen.

Der beschriebene Ablauf rechnet S2 außerhalb der in derselben Regel definierten
Naht („Innerhalb der Rückschreibenaht (zwischen Cache-Eintrag und Persistenz)")
neu — S2s Verwurf liegt in **Fenster 1**, vor dem Cache-Eintrag —, ohne dass S2
neues Material bekam.

**Ausdrücklich festgehalten:** Diese Norm galt am BASIS nie sitzungsscharf. Es
gibt am BASIS keinen Zustand, in dem ein `stale`-Befund von S2 auf Material von
S2 gewartet hätte: jeder Materialeingang irgendeiner Sitzung (2.1, 2.2) und
jeder Storefehler-Rückweg irgendeiner Sitzung (2.3) rechnete ihn frisch.
„Neues Material" in R-283-7 ist damit — gemessen am Verhalten, das die Regel
schützen wollte — „der nächste Rechenlauf", und der war schon immer
sitzungsübergreifend.

**Was NICHT bricht, je mit Zitat:**

- **Gate** (`:8`): „je Befund gilt die in §4 wörtlich zitierte Zusage
  beziehungsweise die Regel R-283-1 bis R-283-6 (§2), gemessen durch die
  Matrixzeilen M-01 bis M-75 (§5) mit Rotbeweis an der Zusagezeile". Der Gate-Satz
  nennt **R-283-1 bis R-283-6**; R-283-7 ist eine spätere Dirigentenregel aus
  §31 (12.09.2026) und steht nicht im Gate.
- **R-283-1** (`:111`): „ist Evidenz hinzugekommen oder verworfen, wird das
  Ergebnis verworfen und **neu gerechnet**, nie über ein jüngeres Ergebnis
  geschrieben." Der Ablauf tut genau das.
- **M-13** (`:616`): die Zusage betrifft „Die Frischeprüfung reicht bis zur
  Persistenz und Publikation, nicht nur bis zum Cache"; über die Sitzungsmenge
  eines Takts sagt die Zeile nichts.
- **M-16** (`:620`): „**Jeder Verwurf zieht genau eine Neurechnung nach sich.** …
  Reihenfolge: Verwurf → Flag → Takt → Eintrag. Frist: ein Takt, **ohne dass neue
  Evidenz eintreffen muss**." Die Zeile **verlangt** den Takt aus `:109` und
  begrenzt ihn nicht auf eine Sitzung.
- **M-14** (`:618`) und **M-15** (`:619`): „Der bestehende Riegel bleibt: A wird
  verworfen, `befunde_neu_bilden` wird gesetzt". Beides geschieht (`:553-559`).
  Beide Zeilen sagen nichts darüber, wer den Merker einlöst.
- **SONDE-014 M-10, M-24, M-29 und Entwurf §37.3**: keine Dauer, kein
  Sitzungsbezug (§3).

---

## 7. Die schließende Regel — zwei Fassungen, je mit Umfang

Ohne Empfehlung; die Einordnung trifft der Dirigent.

### (a) Codefassung „Heilungstakt sitzungsbezogen"

Betroffene Stellen am Stand `30a0b1bf`:

1. **Merker.** `zustand.rs:363` `befunde_neu_bilden: bool` → eine Menge je
   Sitzung (`BTreeSet<SessionKey>` oder `HashSet<SessionKey>`); Default
   `zustand.rs:603` mitziehen.
2. **Setzer, alle fünf.** `hypothese_verdrahtung.rs:554` (Veraltet) und `:565`
   (Ueberholt) — die Sitzung liegt als Parameter `session` von
   `befunde_eintragen` vor (`:528`); `:671` in `befund_persistieren` — ebenfalls
   Parameter (`:636`); `intent.rs:626` — `session` liegt im Block vor (`:627`);
   `invalidierung_verdrahtung.rs:279` in `invalidierung_ruecknehmen` — die
   Signatur (`:250`) trägt **keine** Sitzung, sie ist aber aus
   `wirkung.zurueck` (`ClientKey::session()`) oder `wirkung.ziele`
   (`SnapshotZiel`) ableitbar; alternativ nimmt die Funktion die Sitzung als
   Parameter.
3. **Leser und Löscher.** `:173` (`std::mem::take` in
   `hypothesen_bei_bedarf_bilden`) und `:204` (Löschung in `aufnahmen_sammeln`)
   werden mengenwertig. **Das genügt nicht allein:** `aufnahmen_sammeln`
   (`:195-232`) bildet die Sitzungsmenge unabhängig vom Merker
   (`:208-211`). Ohne einen zusätzlichen Sitzungsfilter — als Parameter der
   Funktion oder als zweiter Einstieg — rechnete der Heilungstakt weiterhin alle
   Sitzungen. Der Filter darf nur für den Heilungstakt gelten, nicht für den
   Materialtakt.
4. **Rückgabe und Weitergabe.** `hypothesen_rechnen` (`:120-158`) gibt heute
   `bool` zurück (`:141`, `:144`, `:157`); es müsste die Menge der überholten
   Sitzungen liefern, und `hypothesen_bilden` (`:107-111`) sie an den Takt
   übergeben. `Eintragung` (`:76-86`) bleibt sonst unverändert.
5. **Tests.** `broker/tests/sonde014_verdrahtung.rs` — M-13 Fenster 1
   (`:786-791`, „genau eine Neurechnung") und M-16 beide Fenster
   (`:1128-1132` und Fenster 2 ab `:1134`); `broker/tests/sonde013_verdrahtung.rs:3685-3690`
   (M-22, Storefehler-Rückweg). Alle drei zählen über den Rechenhaken, **wie oft**
   gerechnet wurde, und fahren **eine** Sitzung; sie bleiben grün, messen die neue
   Zusage aber nicht. Für den Bruch, den der Prüfer beschreibt, bräuchte es einen
   neuen Fall mit **zwei** Sitzungen.

**Bliebe der Materialtakt aus `evidenz.rs` sitzungsübergreifend?** Ja — er
bliebe unangetastet: `evidenz.rs:232` ruft `hypothesen_bilden()` ohne Sitzung,
und `aufnahmen_sammeln` nähme für diesen Weg weiter alle Sitzungen auf. Damit
bleibt auch die Folge bestehen, dass Material in S1 den `stale`-Befund von S2
heilt — genau wie am BASIS. Diese Fassung verkürzt also **nur den zusätzlichen
Heilungstakt**, nicht die Dauer des `stale`-Zustands. Wollte man auch die Dauer
sitzungsscharf machen, müsste der Materialtakt ebenfalls auf die Sitzung des
eingegangenen Belegs eingeschränkt werden — das wäre eine Verhaltensänderung
gegenüber dem BASIS und beträfe die Bühnen von M-13 bis M-16 zusätzlich.

### (b) Regelfassung „R-283-7 präzisiert"

Wortlaut der Fassung: der Rechenlauf umfasst wie bisher alle Sitzungen mit
Vollständigkeitsmarke; „bis neues Material" heißt bis zum nächsten Rechenlauf,
den Material oder ein Verwurf nach R-283-1 auslöst.

**Umfang gegenüber dem BASIS — genau eine Neuerung:** Ein **Verwurf nach
R-283-1 ohne Materialeingang** löst einen zusätzlichen Lauf über alle Sitzungen
mit Marke aus (`:108-109`, Schleife `:169-179`). Alles andere ist
BASIS-Verhalten: der Lauf über alle Sitzungen (2.1, 4.2), die Heilung eines
`stale`-Befunds durch Material einer fremden Sitzung (2.2) und der Lauf über
alle Sitzungen aus einem Rückweg ohne Material (2.3, Storefehler).

Zwei Nebenwirkungen derselben Etappe gehören der Vollständigkeit halber dazu,
beide **verringern** Läufe oder halten sie konstant und fügen keine neue
Sitzung hinzu:

- `:204` löscht den Merker beim Sammeln. Am BASIS blieb ein von `intent.rs:626`
  gesetzter Merker über jeden Rechenlauf hinweg stehen und trieb den nächsten
  `hypothesen_bei_bedarf_bilden`-Aufruf; am Ziel ist er nach jedem Lauf
  eingelöst.
- `hypothesen_bei_bedarf_bilden` ist eine Schleife statt eines einmaligen
  Aufrufs (`:169-179` gegen BASIS `:109-117`). Sie wiederholt nur, solange ein
  Verwurf den Merker erneut setzt; jeder Durchlauf verlangt eine echte
  Standänderung zwischen Sammeln und Rückschreiben.

Produktwirkung dieser Fassung gegenüber dem BASIS: der `stale`-Zustand eines
Befunds, dessen Sitzung kein eigenes Material bekommt, endet unter Umständen
**früher** — im Fenster eines Verwurfs statt erst beim nächsten Materialeingang
irgendeiner Sitzung. Der Endzustand ist in beiden Fällen derselbe, weil der
nächste Materialtakt ihn ohnehin frisch gerechnet hätte (2.2).

---

## 8. Scope-Beweis: gelesene Quellen

Nur gelesen; kein Bau, kein Test, keine Quelländerung. Einzige Schreibaktion ist
diese Datei.

- `broker/src/coordinator/hypothese_verdrahtung.rs` — am Ziel `30a0b1bf`
  vollständig in vier Abschnitten (`:1-260`, `:260-700`, `:700-900`,
  `:1090-1290`); am BASIS `a754a992` `:55-145`, `:425-500`, `:955-1005`
- `broker/src/coordinator/zustand.rs` — Ziel `:350-370`; Diff BASIS→Ziel
  vollständig
- `broker/src/coordinator/intent.rs` — Ziel `:100-160`, `:520-560`, `:560-660`;
  Trefferzeilen zu `vollstaendig`
- `broker/src/coordinator/invalidierung_verdrahtung.rs` — Ziel `:100-135`,
  `:190-300`; BASIS über `git grep` (`:195`, `:258`) und den Diff
- `broker/src/coordinator/evidenz.rs` — Ziel `:210-240`; Diff BASIS→Ziel leer
- `broker/src/coordinator/hypothese/klassenwahl.rs` — `:106-122`
- `broker/src/coordinator/hypothese/befund.rs` — `:266-281`
- `broker/tests/sonde014_befund.rs` — `:295-394`
- `broker/tests/sonde014_verdrahtung.rs` — `:760-800`, `:1120-1140`
- `broker/tests/sonde013_verdrahtung.rs` — `:3675-3695`
- `docs/beweise/NAK-283.md` — `:1-20` (Kopf, Gate), `:105-120` (§2, R-283-1 bis
  R-283-6), `:605-625` (§5.1-Ende, §5.2 M-13 bis M-21), `:3599-3606` (§29.2), `:930-950` (§6.4),
  `:3835-3850` (§29.7), `:3903`, `:3916`, `:3918` (§31)
- `docs/beweise/SONDE-014.md` — `:724` (M-10), `:743` (M-24), `:2332` (M-29)
- `docs/FL-Nakama-Sonden-Design-Entwurf.md` — `:2455-2485` (§37.3)
- `docs/beweise/roh/NAK-283-erstpruefung-2-validierung-1.md` — vollständig
  (Formatmuster)
- Git nur lesend: `git rev-parse HEAD`, `git status --short`, `git show
  <sha>:<datei>`, `git grep -n … <sha> -- broker/`, `git diff [--stat]
  a754a992..30a0b1bf -- …`, `git diff --stat 30a0b1bf HEAD -- broker/`,
  `git ls-files --eol`, `git check-attr -a`
