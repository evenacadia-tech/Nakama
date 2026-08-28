# Beweismanifest — SONDE-007c «Suna-Ziel stilllegen»
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-28 offen -->

> **Die eine harte Regel** (`docs/bauaufteilung-sonden.md` §2): *Eine Behauptung
> ohne eingefügte Rohausgabe ist ein **gescheitertes** Ticket, kein fast
> fertiges.* Unten stehen stdout/stderr wortgleich.

> ⚠️ **Gebaut heißt nicht abgenommen.** Wer baut, urteilt nicht über sich
> selbst. Die Marke oben steht auf `T2 NEEDS_WORK … offen`, bis ein frischer
> Prüfer ohne Anteil an diesem Bau sie ersetzt (S8-Präzedenz). Was dieses
> Ticket ausdrücklich **nicht** behauptet, steht in [§2](#2-was-nicht-bewiesen-wird).

| Feld | Wert |
|---|---|
| Ticket | `SONDE-007c` (S9b, vorgezogen aus S28b `SONDE-015b`) |
| Phase / Session | P1 / S9b |
| Gate-Text (Quelle) | `docs/bauaufteilung-sonden.md` §3, Zeile S9b — **wörtlich:** „**Suna-Ziel stilllegen** (vorgezogen aus S28b, User-Entscheid 28.08.2026, `design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md`): `NakamaSuna_VST3` aus dem Bau, A15 verabschiedet sich deklariert aus dem Kanon, Installer-Manifest auf zwei Bundles plus Broker, Suna-Eintrag in `plugin-identities-v1.json` als stillgelegt markiert — eingefrorene Kennungen von Gen und Probeeq unberührt (NAK-30), Rückweg vollständig. Gate: Kanon grün ohne A15, A17 mit zwei Bundles, `pluginval` SUCCESS an beiden, keine fremde CID. Danach gilt §55 Klausel 1 für **beide** Ziele: scannen und laden in FL" |
| User-Entscheid | `design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md` — **wörtlich:** „suna gibt es nichtmehr . nurnoch Nakama Gen mit 2 Oberflächen und Nakama Probeeq" · „ja stell das einfach richtig" |
| Datum | Arbeit 2026-08-28, Beweislauf 2026-08-29 00:2x (die Session lief über Mitternacht) |
| Basis | `9bb75ad` (master) |
| Prüfstufen | T1 ☑ gefahren ([§4](#4-t1--selbstaudit)) · T2 ☐ — ein frischer Prüfer steht aus · T3 ☐ (erst am Gate) |

**Was geändert wurde (Karte, keine Behauptung — Behauptungen stehen in §1):**

- `eq-copilot/identity/plugin-identities-v1.json` — `passive-probe` bekommt
  das Feld `stillgelegt` (Datum, Ticket, Entscheid, Grund, Folge). **Keine
  Kennung gelöscht, keine geändert**; `status` bleibt `eingefroren`.
- `eq-copilot/cmake/NakamaIdentitaet.cmake` — der Leser **bricht ab**, wenn ein
  Ziel mit `stillgelegt` gebaut werden soll (fail-closed auf die *Anwesenheit*
  des Feldes).
- `eq-copilot/plugin/CMakeLists.txt` — Aufrufe für `NakamaSuna` und
  `EqCopSunaNullTest` entfernt, an ihrer Stelle steht der Grund mit Datum und
  Entscheid; `NakamaSuna`/`EqCopSunaNullTest` aus der `/utf-8`-Schleife.
- `tools/beweise.ps1` — A15 trägt das Feld `Stillgelegt`; der Runner baut es
  nicht, fährt es nicht, zählt es weder grün noch rot — und **zeigt die Zeile
  weiter** (`[STILLGELEGT]` in Konsole und Manifest, Nachsatz im Urteil).
- `eq-copilot/install/nakama-installer-v1.json` — zwei Bundles plus Broker;
  neuer Block `stillgelegte_ziele` mit dem Umgang für Altbestand.
- `eq-copilot/install/Install-Nakama.ps1` — Sollmenge ist „jedes **nicht**
  stillgelegte Ziel"; neue Funktion `Melde-StillgelegteAltlasten`.
- `tools/eq-copilot/pruefe_installer_manifest.py` (A17) — harte `3` raus,
  Beziehung rein; neue Regel `r_stillgelegte_benannt` samt Gegenproben.
- `tools/eq-copilot/pruefe_installer_gegenpfad.py` (A18) — Artefaktzahl aus dem
  Manifest statt `== 4`; Zielliste aus der Identitätsdatei statt
  `("passive-probe", "active-probe")`; neuer Block `[3b]`.
- `eq-copilot/plugin/tests/IdentityTestMain.cpp` — „Manifest kennt drei Ziele"
  ersetzt durch die gemessene Beziehung aktiv ↔ gebaut / stillgelegt ↔ nicht
  gebaut.
- `eq-copilot/schemas/installer/nakama-installer-v1.md` — Vertrag §2.3.
- `eq-copilot/plugin/sonde/SondeProcessor.h`, `docs/plugin-wissen.md` —
  Texte, die noch drei Bundles behaupteten.

---

## 1. Ticket-Behauptungen

| # | Behauptung | Befehl | Ergebnis | Rohausgabe |
|---|---|---|---|---|
| 1 | **`NakamaSuna_VST3` entsteht nicht mehr.** Der Configure kennt das Ziel nicht; die Solution enthält keinen Suna-Eintrag; der Runner baut es nicht | `cmake -S eq-copilot -B eq-copilot/build` · `grep -ci suna EqCopilotSuite.sln` | ☑ 0 Treffer | [↓ C1](#c1) |
| 2 | **Der Identitätsleser ist fail-closed.** Wer den Aufruf wieder einsetzt, bekommt keinen stillen Bau, sondern einen FATAL_ERROR mit Datum und Entscheid — der Riegel hängt am Manifest, nicht an der gelöschten Zeile | Probe: `nakama_identitaet_lesen(passive-probe …)` | ☑ **gefallen** | [↓ C2](#c2) |
| 3 | **Die Stilllegung ist am Bauskript gemessen, nicht behauptet.** `EqCopIdentityTest` misst: kein `nakama_sonde_ziel(NakamaSuna`, kein eigener `juce_add_plugin(NakamaSuna`, und der Grund steht an Ort und Stelle | `EqCopIdentityTest.exe` | ☑ 115/115 | Kanon-Lauf, B1 |
| 4 | **Die Kennung ist NICHT gelöscht.** Derselbe Test misst, dass `plugin_code`, `bundle`, `produktname` und beide CIDs von `passive-probe` vollständig dastehen und die Stilllegung Datum und Entscheid nennt | `EqCopIdentityTest.exe` | ☑ | Kanon-Lauf, B1 |
| 5 | **Die harte Drei ist weg — überall.** Der Test misst die Beziehung „aktive Kennungen ↔ gebaute Ziele" und „stillgelegte ↔ nicht gebaute", nicht eine Zahl | `EqCopIdentityTest.exe` | ☑ `2 aktiv, 1 stillgelegt` · `2 vs 2` · `1 vs 1` | Kanon-Lauf, B1 |
| 6 | **Kein Bundle trägt eine fremde CID** — auch nicht die freigewordene `NkPr`. Der Riegel läuft über *alle* Manifestziele, das stillgelegte eingeschlossen | `EqCopIdentityTest.exe` | ☑ `main: keine fremde Ziel-CID` · `active-probe: keine fremde Ziel-CID` | Kanon-Lauf, B1 |
| 7 | **A15 verabschiedet sich DEKLARIERT.** Der Runner meldet `28/28 grün` **plus** den Nachsatz „1 stillgelegte(s) Bein(e)"; die A15-Zeile steht mit Datum und Grund in der Übersichtstabelle | `pwsh -File tools/beweise.ps1 -Bauen …` | ☑ `GRUEN - 28/28` | [↓ Kanon-Lauf](#kanon-lauf---sonde-007c) |
| 8 | **Der Runner verliert die Zeile nicht still.** `[STILLGELEGT]` ist ein eigener Zustand — weder grün noch rot noch „geplant": ein geplantes Bein kommt, ein stillgelegtes kommt nicht wieder | derselbe Lauf | ☑ | [↓ Kanon-Lauf](#kanon-lauf---sonde-007c) |
| 9 | **Installer-Manifest: zwei Bundles plus Broker**, A17 grün — und die harte `3` ist durch zwei Regeln in *beide* Richtungen ersetzt | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | ☑ 45/45 | Kanon-Lauf, A17 |
| 10 | **Jede neue Regel fällt an einem verdorbenen Manifest** — auch die Stilllegungsregel, und zwar mit einem erfundenen Namen statt einer leeren Liste: eine Gegenprobe, die von der Datenlage abhinge, wäre keine | dasselbe Bein, Teile [2] und [3] | ☑ **gefallen** | Kanon-Lauf, A17 |
| 11 | **Die zwei konkreten Umgehungen sind einzeln gebrochen:** ein stillgelegtes Ziel wird doch ausgeliefert · ein aktives Ziel fällt still aus der Auslieferung, indem nur sein Artefakteintrag verschwindet | dasselbe Bein, Teil [3] | ☑ **gefallen** | Kanon-Lauf, A17 |
| 12 | **Der Rückweg ist vollständig, mit zwei Bundles GEFAHREN** (Sandbox, nichts installiert) | `py -3.13 tools/eq-copilot/pruefe_installer_gegenpfad.py` | ☑ 82/82 | Kanon-Lauf, A18 |
| 13 | **Die Altlast-Zusage wird ausgeführt, nicht zugesagt.** A18 legt „Nakama Suna.vst3" in der Sandbox wirklich hin, misst die ALTLAST-Meldung samt Pfad und Handgriff — und misst, dass das Bundle danach **bytegleich** noch da ist | dasselbe Bein, Block `[3b]` | ☑ | Kanon-Lauf, A18 |
| 14 | **`pluginval --strictness-level 8` SUCCESS an beiden Bundles** | `pluginval.exe --strictness-level 8 --validate-in-process --skip-gui-tests --validate …` | ☑ SUCCESS / SUCCESS | [↓ C3](#c3) · [↓ C4](#c4) |
| 15 | **Auf dieser Maschine liegt keine Suna-Altlast.** `C:\Program Files\Common Files\VST3` enthält kein „Nakama Suna.vst3" — die Stilllegung trifft einen leeren Fall | `ls "C:/Program Files/Common Files/VST3"` | ☑ nicht vorhanden | [↓ C5](#c5) |
| 16 | **Kein stiller Rest im Baumverzeichnis.** Die vom letzten Bau übrig gebliebenen `NakamaSuna*`- und `EqCopSunaNullTest*`-Artefakte sind entfernt; CMake räumt entfallene Ziele nicht selbst weg | `find eq-copilot/build -iname "*Suna*"` | ☑ leer | [↓ C1](#c1) |
| 17 | **Der Tag, an dem das letzte stillgelegte Ziel verschwindet, bricht den Installer nicht.** Im Selbstaudit gelesen, dann gefahren: `@() \| Sort-Object` liefert `$null`, und `Compare-Object $null $null` bricht ab — unter `$ErrorActionPreference='Stop'` ein harter Fehler. Behoben und als Bein `[3c]` gefahren; **ohne** den Fix fällt genau dieses Bein (vorgeführt) | `pruefe_installer_gegenpfad.py`, Block `[3c]` | ☑ mit Fix `Exit 0` · ☑ **gefallen** ohne Fix (`Exit 1`) | [↓ C6](#c6) |
| 18 | **Die „keine zweite Identität"-Regel misst jetzt, was sie seit dem 23.08. behauptet.** Der Manifestkopf sagt „WEDER Produktnamen NOCH Viercodes NOCH Class-IDs" — gemessen wurden bis heute nur zwei der drei Hälften. Beim Verschärfen fiel **sofort ein Treffer**: mein eigener Stilllegungstext hatte „Nakama Probeeq" beiläufig festgeschrieben. Entfernt; zwei eigene Gegenproben brechen die neue Hälfte einzeln | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | ☑ 45/45 · ☑ **gefallen** an Produkt- und Bundlename einzeln | [↓ C7](#c7) |

---

## 2. Was nicht bewiesen wird

- **Der FL-Schritt ist nicht gemessen.** §55 Klausel 1 lautet nach diesem
  Ticket „**beide** Ziele scannen und laden in FL". Das ist ein User-Termin mit
  Admin-Klick, kein Schritt dieses Tickets. Es ist **nichts installiert** —
  jedes `sha256` im Installer-Manifest steht weiter auf `null`, und in diesem
  Zustand bricht `Install-Nakama.ps1` ab, bevor es etwas anfasst.
- **Die Altlast-Behandlung ist an einer Sandbox gemessen, nicht an einem
  echten Fall.** Es gab nie eine ausgelieferte „Nakama Suna.vst3"
  (Behauptung 15). A18 stellt den Fall her, weil ein nie gefahrener Zweig
  keine Zusage ist — aber der Beweis kommt aus `%TEMP%`, nicht aus
  `Common Files`.
- **Der Installer entfernt keine Altlast.** Das ist eine Entscheidung (§3),
  keine Lücke — aber wer erwartet, dass ein Lauf die Maschine aufräumt, wird
  enttäuscht. Er meldet und nennt den Handgriff.
- **Der passive Zweig im geteilten Code ist nicht entfernt.**
  `NAKAMA_SONDE_PASSIV` wird von keinem Ziel mehr gesetzt; der Zweig bleibt als
  zweite Hälfte des `#error`-Riegels stehen (`sonde/SondeProcessor.h`). Das ist
  toter Konfigurationspfad, kein toter Produktpfad — sein Ausbau wäre ein
  Eingriff in die geteilte Quelle und gehört nicht in dieses Ticket.
- **Nichts an Gen oder Probeeq wurde angefasst.** Beide Kennungen sind
  unverändert (NAK-30 gilt weiter); das ist in Behauptung 6 nur *gemessen*,
  nicht *geändert*.
- **Kein Urteil über die eigene Arbeit.** T2 steht offen.

---

## 3. Entscheidungen

### 3.1 `status` bleibt `eingefroren`, `stillgelegt` kommt daneben

Naheliegend wäre gewesen, `status` von `eingefroren` auf `stillgelegt` zu
setzen. Das wäre falsch, weil es zwei verschiedene Aussagen zusammenzöge:

| Feld | Aussage | gilt für |
|---|---|---|
| `status: "eingefroren"` | Viercode, beide Class-IDs und Bundlename sind gesperrt und dürfen nie neu vergeben werden | die **Kennung** |
| `stillgelegt: {…}` | wird nicht gebaut, nicht ausgeliefert, nicht geprüft | den **Lieferumfang** |

Ein einziges Feld für beides hätte gelesen werden können als „nicht mehr
eingefroren" — und dann wäre `NkPr` wieder frei. Ein späteres Ziel könnte
still die Class-ID eines Bundles erben, das einmal in FL-Projekten stand.
Deshalb zwei Felder, und deshalb der erklärende Absatz
`stillgelegt_bedeutet` in derselben Datei.

### 3.2 Der Riegel hängt am Manifest, nicht an der gelöschten Zeile

Eine gelöschte CMake-Zeile ist eine Meinung: sie lässt sich in dreißig
Sekunden wieder einsetzen, und nichts fiele auf. Deshalb liegt der eigentliche
Riegel in `nakama_identitaet_lesen()` — der Leser bricht bei `stillgelegt` ab,
also **bevor** ein Ziel überhaupt entstehen kann. Er fällt auf die
*Anwesenheit* des Feldes, nicht auf seinen Inhalt: fail-closed.

Zweiter Riegel derselben Sache im `EqCopIdentityTest`, weil der Leser nur beim
Configure läuft und der Test auch ohne Bau greifen soll. Gemessen mit `cmake -P`
statt geraten: `string(JSON … TYPE)` liefert bei vorhandenem Objekt `OBJECT`,
bei fehlendem Schlüssel `<membername>-NOTFOUND` — deshalb der Vergleich gegen
`OBJECT` (siehe [↓ C2](#c2)).

### 3.3 Altbestand: melden, nicht löschen

`umgang_mit_altbestand: "melden-nicht-loeschen"`. Zwei Gründe, beide technisch:

1. **Der Gegenpfad bliebe halb.** Journal und `artefakte` stehen 1:1
   zueinander (Vertrag §5); A18 misst das. Eine Löschung wäre ein vierter Akt,
   dessen Rückweg das Bundle **wiederherstellen** müsste — sonst verlöre der
   Rückweg einen Stand, statt ihn zurückzugeben. Das ist ein eigener
   Änderungssatz mit eigener Sicherung, kein Nebenzug in diesem Ticket.
2. **`Common Files\VST3` gehört nicht diesem Installer.** Dort liegt jeder
   Hersteller. Ein Verzeichnis auszuräumen, das man mitbenutzt, ist genau die
   Art unbestellter Verarbeitung, die das Grundgesetz ausschließt.

Was dafür **nicht** passiert: stillschweigen. Die Meldung nennt Produktnamen,
Datum der Stilllegung, vollen Pfad, den Handgriff und den Grund — im
Installationslauf **und** in `-Pruefen`. Und sie ist gefahren (Behauptung 13),
nicht zugesagt.

### 3.4 Die Kanonzeile bleibt stehen

Ein Runner, der ein Bein löscht, meldet eine kleinere Zahl und sagt nicht,
warum. Von außen ist „28 statt 29" nicht von „eine Prüfung ist verschwunden"
zu unterscheiden. `[STILLGELEGT]` ist deshalb ein eigener Zustand neben
`[OK]`, `[ROT]`, `[FEHLT]` und `[GEPLANT]` — und das Urteil trägt den Nachsatz
selbst.

Drei Stellen mussten dafür mit: der Bau (ein stillgelegtes Ziel darf auch dann
nicht gebaut werden, wenn sein Name noch in einem Kommentar steht), der
Frischeriegel (die alte `.exe` läge sonst für immer „älter als die Quellen"
im Baum und verweigerte jede Beglaubigung) und die Laufschleife.

---

## 4. T1 — Selbstaudit

Die feste Sechserliste, gefahren vor dem Commit:

| # | Frage | Befund |
|---|---|---|
| 1 | **Zahlenränder / harte Zahlen** | Vier gefunden und ersetzt: `ziele.size() == 3` (IdentityTest), `len(ident_ziele) == 3` (A17), `aus.count("aktuell") == 4` (A18), `$sollIds` über *alle* Identitätsziele (Install-Nakama.ps1). Alle vier durch eine gemessene Beziehung ersetzt, keine durch eine kleinere Zahl. Dazu ein **leerer Rand** in meinem eigenen neuen Code: `Compare-Object` mit zwei leeren Listen bricht ab (Behauptung 17, [↓ C6](#c6)) — gefunden beim Lesen, behoben, und als Bein `[3c]` samt Gegenprobe gefahren. |
| 2 | **Gegenpfad im selben Änderungssatz** | Installieren ↔ Rückweg: A18 fährt beide Hälften mit zwei Bundles (82/82). Stilllegen ↔ Kennung behalten: der Test misst beide Richtungen. Melden ↔ nicht anfassen: Block `[3b]` misst die Meldung **und** die Bytegleichheit danach. |
| 3 | **Stale Closures / stale Artefakte** | Der Baum trug nach dem Configure noch `NakamaSuna_artefacts`, `EqCopSunaNullTest_artefacts` und deren `.vcxproj` — CMake räumt entfallene Ziele nicht weg. Entfernt (Behauptung 16). Die Solution enthielt sie bereits nicht mehr. |
| 4 | **Irreführende UI-/Doku-Texte** | Sieben Stellen, die noch drei Apps oder drei Bundles behaupteten: `plugin/CMakeLists.txt` (4×), `cmake/NakamaIdentitaet.cmake` (3×), `sonde/SondeProcessor.h`, `install/nakama-installer-v1.json`, Vertrag §Kopf und §5, `docs/plugin-wissen.md` (4×). Der zitierte §55-Wortlaut („alle drei Bundles") bleibt im Wortlaut stehen — er ist ein Zitat; der Stand steht daneben. **Ein Text von mir selbst war darunter:** der neue Kopfsatz im Installer-Manifest schrieb einen Produktnamen fest — gefunden nicht durch Lesen, sondern weil ich die Regel verschärft habe, die ihn hätte finden müssen (Behauptung 18, [↓ C7](#c7)). |
| 5 | **Fail-closed statt fail-open** | Der Identitätsleser fällt auf die *Anwesenheit* des Feldes. A17s neue Regel prüft Pflichtfelder einzeln, nicht die bloße Existenz des Blocks. `Ist-Stillgelegt` fragt PSObject, weil `Set-StrictMode -Version Latest` beim Punktzugriff auf ein fehlendes Feld wirft — das war ein echter Fehler beim ersten Lauf, nicht im Nachdenken gefunden. |
| 6 | **Gegenprobe der Gegenprobe** | `verdirb()` bricht die neue Regel mit einem erfundenen Namen, nicht mit einer leeren Liste: eine leere Liste fiele nur, *solange* es ein stillgelegtes Ziel gibt. In `adversariale_strukturproben` stand `ident["ziele"][1]` — der Eintrag der passiven Sonde; nach der Stilllegung wäre `next(...)` mit `StopIteration` gestorben. Jetzt werden die Indizes gesucht, und weniger als zwei aktive Ziele brechen die Probe laut ab, statt sie still zu überspringen. |

---

## 5. Rohe Belege

<a id="c1"></a>
### C1 · Configure ohne Suna, Solution ohne Suna, Baum ohne Suna (Behauptungen 1, 16)

**Befehl:** `cmake -S eq-copilot -B eq-copilot/build` (gefiltert auf die Identitäts- und Verbraucherzeilen)

```
-- Nakama-Identitaet: 'main' aus dem Manifest — EQ-Copilot (Evna/Eqcp), Bundle EQ-Copilot.vst3, Kategorien Fx Analyzer.
-- Nakama-Identitaet: 'active-probe' aus dem Manifest — Nakama Probeeq (Evna/NkAc), Bundle Nakama Probeeq.vst3, Kategorien Fx EQ.
-- Nakama-Kern: K2b/K2c gegen alle 13 Verbraucher gemessen.
-- Configuring done (17.5s)
-- Generating done (2.1s)
```

Vor dem Ticket stand hier zusätzlich eine dritte Zeile
(`'passive-probe' … Nakama Suna (Evna/NkPr) …`, wortgleich belegt in
`docs/beweise/SONDE-007b.md` B4). Sie fehlt jetzt, weil kein Ziel sie mehr
liest. Die Verbraucherzahl ist die aus **diesem** Lauf; sie folgt der Zahl der
Ziele und ist mit den zwei entfallenen Zielen entsprechend kleiner.

**Befehl:** `grep -c -i "suna" eq-copilot/build/EqCopilotSuite.sln`

```
0
```

**Befehl:** `find eq-copilot/build -iname "*Suna*"` (nach dem Aufräumen)

```
(leer)
```

<a id="c2"></a>
### C2 · Der Leser bricht bei einem stillgelegten Ziel ab (Behauptung 2)

Gemessene Vorarbeit — das Verhalten von `string(JSON … TYPE)` wurde nicht
angenommen, sondern mit `cmake -P` an einem Probe-JSON gemessen:

```
-- t1=[OBJECT] e1=[NOTFOUND]
-- t2=[fehlt-NOTFOUND] e2=[member 'fehlt' not found]
```

`TYPE` liefert bei vorhandenem Objekt `OBJECT`, bei fehlendem Schlüssel
`<membername>-NOTFOUND`. Der Riegel vergleicht deshalb gegen `OBJECT`.

Der Riegel selbst (`eq-copilot/cmake/NakamaIdentitaet.cmake`):

```cmake
string(JSON _stilltyp ERROR_VARIABLE _e6 TYPE "${_eintrag}" stillgelegt)
if(_stilltyp STREQUAL "OBJECT")
    string(JSON _stillam    ERROR_VARIABLE _e7 GET "${_eintrag}" stillgelegt am)
    string(JSON _stillwarum ERROR_VARIABLE _e8 GET "${_eintrag}" stillgelegt entscheid)
    message(FATAL_ERROR
        "S9b/SONDE-007c: Ziel '${ziel_id}' ist seit ${_stillam} STILLGELEGT und wird nicht gebaut.\n"
        "Entscheid: ${_stillwarum}\n"
        ...)
endif()
```

<a id="c5"></a>
### C5 · Keine Altlast auf dieser Maschine (Behauptung 15)

**Befehl:** `ls "C:/Program Files/Common Files/VST3/" | grep -i nakama`

```
(kein Treffer — weder "Nakama Suna.vst3" noch "Nakama Probeeq.vst3")
```

Installiert ist dort aus diesem Projekt allein `EQ-Copilot.vst3` (der Bau vom
16.08.2026, `74D86BD5…`) sowie die beiden Wegwerf-Messgeräte
`EqCop-Aux-Spike.vst3` und `EqCop-Host-Probe.vst3`. Die Stilllegung trifft
damit einen leeren Fall — der Altlast-Zweig ist trotzdem gebaut und gefahren
(Behauptung 13), weil „gibt es heute nicht" kein Grund ist, einen Zweig
ungeprüft zu lassen.

<a id="c7"></a>
### C7 · Die Literalregel, verschärft — und was sie sofort fand (Behauptung 18)

`r_keine_identitaetsliterale` prüfte bis zum 28.08.2026 `plugin_code`,
`component_cid` und `controller_cid`. Der Kopf des Manifests versprach seit dem
23.08. mehr. Beim ersten Lauf der erweiterten Regel:

```
  FEHLER  kein Viercode, keine Class-ID im Installer-Manifest  [active-probe.produktname, hersteller.name]
```

Zwei Treffer, mit verschiedenem Ausgang:

- **`active-probe.produktname`** — **echter Verstoß, von mir selbst in diesem
  Ticket eingebaut.** Der Kopftext lautete „…sind das ZWEI Bundles: Nakama Gen
  und Nakama Probeeq". Entfernt; der Text verweist jetzt auf die
  Identitätsdatei, statt sie abzuschreiben.
- **`hersteller.name`** — **kein Verstoß, sondern eine zu weit gefasste Regel.**
  „evenacadia" steckt in den von Vertrag §4 festgelegten geschützten Pfaden
  (`C:/Program Files/evenacadia/…`), also in derselben Zwangslage wie der
  Bundlename in `quelle`; anders als `hersteller.code` geht er in keine
  Class-ID ein. Die Regel prüft ihn deshalb nicht — `hersteller.code` weiter
  schon.

Danach, mit zwei **einzeln** gebrochenen Gegenproben (die grobe
`verdirb`-Probe trägt schon einen Viercode und wäre auch dann gefallen, wenn
die neue Hälfte gar nichts prüfte):

```
  ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
  ok      faellt an einem Produktnamen ausserhalb der Pfade
  ok      faellt an einem Bundlenamen ausserhalb der Pfade
```

Der Vergleichswert der Gegenproben kommt aus der Identitätsdatei, nicht aus
einem Literal im Prüfbein — ein dort abgeschriebener Name wäre selbst die
zweite Wahrheit, gegen die die Regel gerichtet ist.

<a id="c6"></a>
### C6 · Der leere Fall — gelesen, behoben, gefahren, und ohne Fix vorgeführt (Behauptung 17)

Der Fehler war **kein Laufzeitbefund**: heute gibt es genau ein stillgelegtes
Ziel, also sind beide Listen einelementig und `Compare-Object` bekommt gültige
Argumente. Er wäre erst an dem Tag aufgeschlagen, an dem das letzte
stillgelegte Ziel aus dem Manifest verschwindet — und dann als
Bindungsfehler, nicht als Aussage.

Gemessenes Verhalten (PowerShell 7, `pwsh -File`):

```
fehlende Eigenschaft ist null: True
Compare-Object leer/leer -> Count=0
fehlender Block -> Count=0
Compare-Object: Cannot bind argument to parameter 'ReferenceObject' because it is null.
```

`Compare-Object @() @()` ist harmlos; `Compare-Object ($leer | Sort-Object) …`
ist es nicht — die Pipe durch `Sort-Object` macht aus der leeren Liste `$null`.

**Mit dem Fix** (Zählvergleich vor dem Mengenvergleich, Mengenvergleich nur bei
nicht-leeren Listen):

```
[3c] Eine Auslieferung ganz OHNE stillgelegtes Ziel laeuft weiter
  ok      ohne stillgelegtes Ziel laeuft -Pruefen durch und schweigt darueber  [Exit 0]
```

**Ohne den Fix** — derselbe Block, an einem zurückgebauten Skript vorgeführt,
damit das Bein nicht nur mitläuft, sondern nachweislich zufasst:

```
[3c] Eine Auslieferung ganz OHNE stillgelegtes Ziel laeuft weiter
  FEHLER  ohne stillgelegtes Ziel laeuft -Pruefen durch und schweigt darueber  [Exit 1]
```

<a id="c3"></a>
### C3 · `pluginval` Strenge 8 an Nakama Gen (Behauptung 14)

**Befehl:** `%TEMP%\pluginval.exe --strictness-level 8 --validate-in-process --skip-gui-tests --validate "…\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3"` | **Exitcode:** 0

```
Started validating: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3
Random seed: 0x639099
Validation started
Strictness level: 8
-----------------------------------------------------------------
Starting tests in: pluginval / Scan for plugins located in: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3...
Num plugins found: 1

Testing plugin: VST3-EQ-Copilot-4e7666d2-fd30d2b9
evenacadia: EQ-Copilot v0.3.0
Completed tests in pluginval / Scan for plugins located in: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3
-----------------------------------------------------------------
Starting tests in: pluginval / Open plugin (cold)...
Completed tests in pluginval / Open plugin (cold)
-----------------------------------------------------------------
Starting tests in: pluginval / Open plugin (warm)...
Running tests 1 times
Completed tests in pluginval / Open plugin (warm)
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin info...

Plugin name: EQ-Copilot
Alternative names: EQ-Copilot
SupportsDoublePrecision: no
Reported latency: 0
Reported taillength: 0
Completed tests in pluginval / Plugin info
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin programs...
Num programs: 0
All program names checked
Completed tests in pluginval / Plugin programs
-----------------------------------------------------------------
Starting tests in: pluginval / Audio processing...
Testing with sample rate [44100] and block size [64]
Testing with sample rate [44100] and block size [128]
Testing with sample rate [44100] and block size [256]
Testing with sample rate [44100] and block size [512]
Testing with sample rate [44100] and block size [1024]
Testing with sample rate [48000] and block size [64]
Testing with sample rate [48000] and block size [128]
Testing with sample rate [48000] and block size [256]
Testing with sample rate [48000] and block size [512]
Testing with sample rate [48000] and block size [1024]
Testing with sample rate [96000] and block size [64]
Testing with sample rate [96000] and block size [128]
Testing with sample rate [96000] and block size [256]
Testing with sample rate [96000] and block size [512]
Testing with sample rate [96000] and block size [1024]
Completed tests in pluginval / Audio processing
-----------------------------------------------------------------
Starting tests in: pluginval / Non-releasing audio processing...
Testing with sample rate [44100] and block size [64]
Testing with sample rate [44100] and block size [128]
Testing with sample rate [44100] and block size [256]
Testing with sample rate [44100] and block size [512]
Testing with sample rate [44100] and block size [1024]
Testing with sample rate [48000] and block size [64]
Testing with sample rate [48000] and block size [128]
Testing with sample rate [48000] and block size [256]
Testing with sample rate [48000] and block size [512]
Testing with sample rate [48000] and block size [1024]
Testing with sample rate [96000] and block size [64]
Testing with sample rate [96000] and block size [128]
Testing with sample rate [96000] and block size [256]
Testing with sample rate [96000] and block size [512]
Testing with sample rate [96000] and block size [1024]
Completed tests in pluginval / Non-releasing audio processing
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin state...
Completed tests in pluginval / Plugin state
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin state restoration...
Completed tests in pluginval / Plugin state restoration
-----------------------------------------------------------------
Starting tests in: pluginval / Automation...
Testing with sample rate [44100] and block size [64] and sub-block size [32]
Testing with sample rate [44100] and block size [128] and sub-block size [32]
Testing with sample rate [44100] and block size [256] and sub-block size [32]
Testing with sample rate [44100] and block size [512] and sub-block size [32]
Testing with sample rate [44100] and block size [1024] and sub-block size [32]
Testing with sample rate [48000] and block size [64] and sub-block size [32]
Testing with sample rate [48000] and block size [128] and sub-block size [32]
Testing with sample rate [48000] and block size [256] and sub-block size [32]
Testing with sample rate [48000] and block size [512] and sub-block size [32]
Testing with sample rate [48000] and block size [1024] and sub-block size [32]
Testing with sample rate [96000] and block size [64] and sub-block size [32]
Testing with sample rate [96000] and block size [128] and sub-block size [32]
Testing with sample rate [96000] and block size [256] and sub-block size [32]
Testing with sample rate [96000] and block size [512] and sub-block size [32]
Testing with sample rate [96000] and block size [1024] and sub-block size [32]
Completed tests in pluginval / Automation
-----------------------------------------------------------------
Starting tests in: pluginval / Automatable Parameters...
Completed tests in pluginval / Automatable Parameters
-----------------------------------------------------------------
Starting tests in: pluginval / Parameters...
Completed tests in pluginval / Parameters
-----------------------------------------------------------------
Starting tests in: pluginval / Parameter thread safety...
Completed tests in pluginval / Parameter thread safety
-----------------------------------------------------------------
Starting tests in: pluginval / auval...
Completed tests in pluginval / auval
-----------------------------------------------------------------
Starting tests in: pluginval / vst3 validator...
INFO: Skipping vst3 validator as validator path hasn't been set
Completed tests in pluginval / vst3 validator
-----------------------------------------------------------------
Starting tests in: pluginval / Basic bus...
Completed tests in pluginval / Basic bus
-----------------------------------------------------------------
Starting tests in: pluginval / Listing available buses...
Inputs:
	Named layouts: Mono, Stereo
	Discrete layouts: Discrete #1
Outputs:
	Named layouts: Mono, Stereo
	Discrete layouts: Discrete #1
Main bus num input channels: 2
Main bus num output channels: 2
Completed tests in pluginval / Listing available buses
-----------------------------------------------------------------
Starting tests in: pluginval / Enabling all buses...
Completed tests in pluginval / Enabling all buses
-----------------------------------------------------------------
Starting tests in: pluginval / Disabling non-main busses...
Completed tests in pluginval / Disabling non-main busses
-----------------------------------------------------------------
Starting tests in: pluginval / Restoring default layout...
Main bus num input channels: 2
Main bus num output channels: 2
Completed tests in pluginval / Restoring default layout
-----------------------------------------------------------------
Starting tests in: pluginval / Fuzz parameters...
Completed tests in pluginval / Fuzz parameters
SUCCESS
```

<a id="c4"></a>
### C4 · `pluginval` Strenge 8 an Nakama Probeeq (Behauptung 14)

**Befehl:** `%TEMP%\pluginval.exe --strictness-level 8 --validate-in-process --skip-gui-tests --validate "…\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3"` | **Exitcode:** 0

```
Started validating: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3
Random seed: 0x5f65900
Validation started
Strictness level: 8
-----------------------------------------------------------------
Starting tests in: pluginval / Scan for plugins located in: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3...
Num plugins found: 1

Testing plugin: VST3-Nakama Probeeq-e7a9ace5-62ab0ac
evenacadia: Nakama Probeeq v0.3.0
Completed tests in pluginval / Scan for plugins located in: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3
-----------------------------------------------------------------
Starting tests in: pluginval / Open plugin (cold)...
Completed tests in pluginval / Open plugin (cold)
-----------------------------------------------------------------
Starting tests in: pluginval / Open plugin (warm)...
Running tests 1 times
Completed tests in pluginval / Open plugin (warm)
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin info...

Plugin name: Nakama Probeeq
Alternative names: Nakama Probeeq
SupportsDoublePrecision: no
Reported latency: 0
Reported taillength: 0
Completed tests in pluginval / Plugin info
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin programs...
Num programs: 0
All program names checked
Completed tests in pluginval / Plugin programs
-----------------------------------------------------------------
Starting tests in: pluginval / Audio processing...
Testing with sample rate [44100] and block size [64]
Testing with sample rate [44100] and block size [128]
Testing with sample rate [44100] and block size [256]
Testing with sample rate [44100] and block size [512]
Testing with sample rate [44100] and block size [1024]
Testing with sample rate [48000] and block size [64]
Testing with sample rate [48000] and block size [128]
Testing with sample rate [48000] and block size [256]
Testing with sample rate [48000] and block size [512]
Testing with sample rate [48000] and block size [1024]
Testing with sample rate [96000] and block size [64]
Testing with sample rate [96000] and block size [128]
Testing with sample rate [96000] and block size [256]
Testing with sample rate [96000] and block size [512]
Testing with sample rate [96000] and block size [1024]
Completed tests in pluginval / Audio processing
-----------------------------------------------------------------
Starting tests in: pluginval / Non-releasing audio processing...
Testing with sample rate [44100] and block size [64]
Testing with sample rate [44100] and block size [128]
Testing with sample rate [44100] and block size [256]
Testing with sample rate [44100] and block size [512]
Testing with sample rate [44100] and block size [1024]
Testing with sample rate [48000] and block size [64]
Testing with sample rate [48000] and block size [128]
Testing with sample rate [48000] and block size [256]
Testing with sample rate [48000] and block size [512]
Testing with sample rate [48000] and block size [1024]
Testing with sample rate [96000] and block size [64]
Testing with sample rate [96000] and block size [128]
Testing with sample rate [96000] and block size [256]
Testing with sample rate [96000] and block size [512]
Testing with sample rate [96000] and block size [1024]
Completed tests in pluginval / Non-releasing audio processing
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin state...
Completed tests in pluginval / Plugin state
-----------------------------------------------------------------
Starting tests in: pluginval / Plugin state restoration...
Completed tests in pluginval / Plugin state restoration
-----------------------------------------------------------------
Starting tests in: pluginval / Automation...
Testing with sample rate [44100] and block size [64] and sub-block size [32]
Testing with sample rate [44100] and block size [128] and sub-block size [32]
Testing with sample rate [44100] and block size [256] and sub-block size [32]
Testing with sample rate [44100] and block size [512] and sub-block size [32]
Testing with sample rate [44100] and block size [1024] and sub-block size [32]
Testing with sample rate [48000] and block size [64] and sub-block size [32]
Testing with sample rate [48000] and block size [128] and sub-block size [32]
Testing with sample rate [48000] and block size [256] and sub-block size [32]
Testing with sample rate [48000] and block size [512] and sub-block size [32]
Testing with sample rate [48000] and block size [1024] and sub-block size [32]
Testing with sample rate [96000] and block size [64] and sub-block size [32]
Testing with sample rate [96000] and block size [128] and sub-block size [32]
Testing with sample rate [96000] and block size [256] and sub-block size [32]
Testing with sample rate [96000] and block size [512] and sub-block size [32]
Testing with sample rate [96000] and block size [1024] and sub-block size [32]
Completed tests in pluginval / Automation
-----------------------------------------------------------------
Starting tests in: pluginval / Automatable Parameters...
Completed tests in pluginval / Automatable Parameters
-----------------------------------------------------------------
Starting tests in: pluginval / Parameters...
Completed tests in pluginval / Parameters
-----------------------------------------------------------------
Starting tests in: pluginval / Parameter thread safety...
Completed tests in pluginval / Parameter thread safety
-----------------------------------------------------------------
Starting tests in: pluginval / auval...
Completed tests in pluginval / auval
-----------------------------------------------------------------
Starting tests in: pluginval / vst3 validator...
INFO: Skipping vst3 validator as validator path hasn't been set
Completed tests in pluginval / vst3 validator
-----------------------------------------------------------------
Starting tests in: pluginval / Basic bus...
Completed tests in pluginval / Basic bus
-----------------------------------------------------------------
Starting tests in: pluginval / Listing available buses...
Inputs:
	Named layouts: Mono, Stereo, LCR, Quadraphonic, 5.0 Surround, 5.1 Surround, 7.0 Surround, 7.1 Surround
	Discrete layouts: Discrete #1
Outputs:
	Named layouts: Mono, Stereo, LCR, Quadraphonic, 5.0 Surround, 5.1 Surround, 7.0 Surround, 7.1 Surround
	Discrete layouts: Discrete #1
Main bus num input channels: 2
Main bus num output channels: 2
Completed tests in pluginval / Listing available buses
-----------------------------------------------------------------
Starting tests in: pluginval / Enabling all buses...
Completed tests in pluginval / Enabling all buses
-----------------------------------------------------------------
Starting tests in: pluginval / Disabling non-main busses...
Completed tests in pluginval / Disabling non-main busses
-----------------------------------------------------------------
Starting tests in: pluginval / Restoring default layout...
Main bus num input channels: 2
Main bus num output channels: 2
Completed tests in pluginval / Restoring default layout
-----------------------------------------------------------------
Starting tests in: pluginval / Fuzz parameters...
Completed tests in pluginval / Fuzz parameters
SUCCESS
```

> Beide Läufe sind an den Bundles desselben Baus gefahren, den der
> Kanon-Lauf unten beglaubigt. `pluginval` prüft das VST3-Protokoll — es
> sagt **nicht**, dass die Stilllegung richtig ist; das messen A17, A18 und
> der `EqCopIdentityTest`.

---

## Kanon-Lauf - SONDE-007c

**Lauf:** 2026-08-29 00:26 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 28/28 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 00:26:37 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 9bb75ad S8 SONDE-007a: T3 NEEDS_WORK 2026-08-28 nachgearbeitet - drei frische Pruefungen, Restbefunde NAK-84/85, Planstand |
| Commit (voll) | 9bb75ada59f62bd7325059a418ebd71f31b6bb8b |
| Arbeitsbaum | 13 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/plugin-wissen.md
 M eq-copilot/cmake/NakamaIdentitaet.cmake
 M eq-copilot/identity/plugin-identities-v1.json
 M eq-copilot/install/Install-Nakama.ps1
 M eq-copilot/install/nakama-installer-v1.json
 M eq-copilot/plugin/CMakeLists.txt
 M eq-copilot/plugin/sonde/SondeProcessor.h
 M eq-copilot/plugin/tests/IdentityTestMain.cpp
 M eq-copilot/schemas/installer/nakama-installer-v1.md
 M tools/beweise.ps1
 M tools/eq-copilot/pruefe_installer_gegenpfad.py
 M tools/eq-copilot/pruefe_installer_manifest.py
?? docs/beweise/SONDE-007c.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-28 22:50:07 | `5AF83DC8A66EC081` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 22:50:11 | `620AD1FFB7ABB49D` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-29 00:10:48 | `6204C4A752F2EB3A` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-29 00:10:55 | `7AD4328F39A1C855` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 22:50:21 | `F6FE9313DBED1CB4` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 22:50:26 | `53D6108D902A9ADF` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 22:50:28 | `ABBB29D7F552D8C9` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 22:50:32 | `75C84EA6AC1BAAAD` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 22:50:34 | `5D0C5CE7D2E3A2AF` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-29 00:08:17**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,62 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,44 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,99 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,06 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,45 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,12 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,19 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,86 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,23 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,23 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,20 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,41 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,80 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 24,76 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A19](#a19) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,29 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,17 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,45 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,04 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,62 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,44 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,99 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::hin_und_zurueck ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.49s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.15s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.07s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,06 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [327 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,45 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,12 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=c4cf81a4f940f91a29c0d9f627654635bed88b5307f4e0f640ca26a63d9f3018  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,86 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,20 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,41 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      jede gebaute TU hat exakt die heutige Definemenge (5 TUs, 16 Defines)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii,fourcc-int-be]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,80 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
  ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird
  ok      faellt, wenn ein stillgelegtes Ziel nirgends benannt ist
  ok      faellt, wenn ein aktives Ziel still aus der Auslieferung faellt
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 3 Artefakt(e) ohne Hash: main, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

43 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 24,76 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-d991kwwz\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-d991kwwz\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: m_null).
  ok      Nakama Suna : stillgelegt seit 2026-08-28, nicht installiert
  ok      EQ-Copilot installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-d991kwwz\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-d991kwwz\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-d991kwwz\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle 3 Artefakte melden `aktuell`  [3x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[3b] Ein stillgelegtes Bundle wird gemeldet, nicht geloescht
  ok      das vorgefundene stillgelegte Bundle wird mit vollem Pfad gemeldet
  ok      die Meldung nennt den Handgriff, statt nur zu klagen
  ok      und der Installer hat es NICHT angefasst - bytegleich vorgefunden
  ok      ohne Altbestand bleibt die Zeile ehrlich (`nicht installiert`)

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

81 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt mindestens ein Ziel  [3]
  ok      mindestens ein Ziel ist nicht stillgelegt  [2 aktiv, 1 stillgelegt]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes AKTIVE Ziel im Manifest hat hier eine Zeile  [2 vs 2]
  ok      jedes STILLGELEGTE Ziel im Manifest hat hier eine Zeile  [1 vs 1]
  ok      main: steht als AKTIVES Ziel im Manifest
  ok      active-probe: steht als AKTIVES Ziel im Manifest
  ok      passive-probe: die Kennung steht weiter im Manifest
  ok      passive-probe: und ist dort als stillgelegt markiert
  ok      passive-probe: plugin_code ist nicht geloescht  [NkPr]
  ok      passive-probe: bundle ist nicht geloescht  [Nakama Suna.vst3]
  ok      passive-probe: produktname ist nicht geloescht  [Nakama Suna]
  ok      passive-probe: component_cid ist nicht geloescht  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: controller_cid ist nicht geloescht  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: die Stilllegung nennt Datum und Entscheid  [2026-08-28 / design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      NakamaSuna: stillgelegt - kein Aufruf von nakama_sonde_ziel() mehr
  ok      NakamaSuna: stillgelegt - auch kein eigener juce_add_plugin-Block
  ok      NakamaSuna: das Bauskript erklaert die Stilllegung an Ort und Stelle
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [29 Aug 2026 12:27:02am vs 28 Aug 2026 11:59:24pm]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [29 Aug 2026 12:27:05am vs 28 Aug 2026 11:59:24pm]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 115 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,29 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      Matrix Klasse=main, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=main, Position=pre: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position pre is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=legacy, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for legacy', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=passive_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for passive_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=active_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for active_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [036325d2c90b4579a75f5a8570602c44]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [65795959e4194a0196370adb005ed789]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 169 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260829-002754.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (57857 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,17 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,45 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3862 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,04 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [31 Bloecke]
[ok]   und mindestens einen Frame gebaut  [3 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 27,61 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>


---

## Kanon-Lauf - SONDE-007c (nach Selbstaudit)

**Lauf:** 2026-08-29 00:33 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 28/28 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 00:33:22 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 9bb75ad S8 SONDE-007a: T3 NEEDS_WORK 2026-08-28 nachgearbeitet - drei frische Pruefungen, Restbefunde NAK-84/85, Planstand |
| Commit (voll) | 9bb75ada59f62bd7325059a418ebd71f31b6bb8b |
| Arbeitsbaum | 15 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/PLAN-STAND.md
 M docs/offene-punkte.md
 M docs/plugin-wissen.md
 M eq-copilot/cmake/NakamaIdentitaet.cmake
 M eq-copilot/identity/plugin-identities-v1.json
 M eq-copilot/install/Install-Nakama.ps1
 M eq-copilot/install/nakama-installer-v1.json
 M eq-copilot/plugin/CMakeLists.txt
 M eq-copilot/plugin/sonde/SondeProcessor.h
 M eq-copilot/plugin/tests/IdentityTestMain.cpp
 M eq-copilot/schemas/installer/nakama-installer-v1.md
 M tools/beweise.ps1
 M tools/eq-copilot/pruefe_installer_gegenpfad.py
 M tools/eq-copilot/pruefe_installer_manifest.py
?? docs/beweise/SONDE-007c.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-28 22:50:07 | `5AF83DC8A66EC081` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 22:50:11 | `620AD1FFB7ABB49D` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-29 00:10:48 | `6204C4A752F2EB3A` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-29 00:33:09 | `6E02D7DB16D66D64` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 22:50:21 | `F6FE9313DBED1CB4` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 22:50:26 | `53D6108D902A9ADF` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 22:50:28 | `ABBB29D7F552D8C9` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 22:50:32 | `75C84EA6AC1BAAAD` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 22:50:34 | `5D0C5CE7D2E3A2AF` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-29 00:33:02**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,66 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,64 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1,01 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,06 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,40 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,95 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,19 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,40 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,80 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 25,13 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A19](#a19) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,29 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,22 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,40 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,03 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,66 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,64 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 1,01 s

stdout:

```text

running 90 tests
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.53s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.08s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,06 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [328 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,40 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=c4cf81a4f940f91a29c0d9f627654635bed88b5307f4e0f640ca26a63d9f3018  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,95 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,40 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      jede gebaute TU hat exakt die heutige Definemenge (5 TUs, 16 Defines)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii,fourcc-int-be]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,80 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
  ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird
  ok      faellt, wenn ein stillgelegtes Ziel nirgends benannt ist
  ok      faellt, wenn ein aktives Ziel still aus der Auslieferung faellt
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 3 Artefakt(e) ohne Hash: main, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

43 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 25,13 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-y_k993i8\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-y_k993i8\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: m_null).
  ok      Nakama Suna : stillgelegt seit 2026-08-28, nicht installiert
  ok      EQ-Copilot installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-y_k993i8\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-y_k993i8\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-y_k993i8\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle 3 Artefakte melden `aktuell`  [3x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[3b] Ein stillgelegtes Bundle wird gemeldet, nicht geloescht
  ok      das vorgefundene stillgelegte Bundle wird mit vollem Pfad gemeldet
  ok      die Meldung nennt den Handgriff, statt nur zu klagen
  ok      und der Installer hat es NICHT angefasst - bytegleich vorgefunden
  ok      ohne Altbestand bleibt die Zeile ehrlich (`nicht installiert`)

[3c] Eine Auslieferung ganz OHNE stillgelegtes Ziel laeuft weiter
  ok      ohne stillgelegtes Ziel laeuft -Pruefen durch und schweigt darueber  [Exit 0]

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

82 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt mindestens ein Ziel  [3]
  ok      mindestens ein Ziel ist nicht stillgelegt  [2 aktiv, 1 stillgelegt]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes AKTIVE Ziel im Manifest hat hier eine Zeile  [2 vs 2]
  ok      jedes STILLGELEGTE Ziel im Manifest hat hier eine Zeile  [1 vs 1]
  ok      main: steht als AKTIVES Ziel im Manifest
  ok      active-probe: steht als AKTIVES Ziel im Manifest
  ok      passive-probe: die Kennung steht weiter im Manifest
  ok      passive-probe: und ist dort als stillgelegt markiert
  ok      passive-probe: plugin_code ist nicht geloescht  [NkPr]
  ok      passive-probe: bundle ist nicht geloescht  [Nakama Suna.vst3]
  ok      passive-probe: produktname ist nicht geloescht  [Nakama Suna]
  ok      passive-probe: component_cid ist nicht geloescht  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: controller_cid ist nicht geloescht  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: die Stilllegung nennt Datum und Entscheid  [2026-08-28 / design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      NakamaSuna: stillgelegt - kein Aufruf von nakama_sonde_ziel() mehr
  ok      NakamaSuna: stillgelegt - auch kein eigener juce_add_plugin-Block
  ok      NakamaSuna: das Bauskript erklaert die Stilllegung an Ort und Stelle
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [29 Aug 2026 12:33:46am vs 28 Aug 2026 11:59:24pm]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [29 Aug 2026 12:33:49am vs 28 Aug 2026 11:59:24pm]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 115 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,29 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      Matrix Klasse=main, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=main, Position=pre: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position pre is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=legacy, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for legacy', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=passive_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for passive_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=active_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for active_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [7a3d5c01980f480099e3c2033a550f42]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [aa8a8cd08b9642fe90c014c2cae6c451]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 169 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260829-003439.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (38547 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,40 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3841 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,03 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [40 Bloecke]
[ok]   und mindestens einen Frame gebaut  [4 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 27,15 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>


---

## Kanon-Lauf - SONDE-007c (Endstand vor Commit)

**Lauf:** 2026-08-29 00:37 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 28/28 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 00:37:13 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 9bb75ad S8 SONDE-007a: T3 NEEDS_WORK 2026-08-28 nachgearbeitet - drei frische Pruefungen, Restbefunde NAK-84/85, Planstand |
| Commit (voll) | 9bb75ada59f62bd7325059a418ebd71f31b6bb8b |
| Arbeitsbaum | 15 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/PLAN-STAND.md
 M docs/offene-punkte.md
 M docs/plugin-wissen.md
 M eq-copilot/cmake/NakamaIdentitaet.cmake
 M eq-copilot/identity/plugin-identities-v1.json
 M eq-copilot/install/Install-Nakama.ps1
 M eq-copilot/install/nakama-installer-v1.json
 M eq-copilot/plugin/CMakeLists.txt
 M eq-copilot/plugin/sonde/SondeProcessor.h
 M eq-copilot/plugin/tests/IdentityTestMain.cpp
 M eq-copilot/schemas/installer/nakama-installer-v1.md
 M tools/beweise.ps1
 M tools/eq-copilot/pruefe_installer_gegenpfad.py
 M tools/eq-copilot/pruefe_installer_manifest.py
?? docs/beweise/SONDE-007c.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-28 22:50:07 | `5AF83DC8A66EC081` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 22:50:11 | `620AD1FFB7ABB49D` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-29 00:10:48 | `6204C4A752F2EB3A` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-29 00:33:09 | `6E02D7DB16D66D64` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 22:50:21 | `F6FE9313DBED1CB4` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 22:50:26 | `53D6108D902A9ADF` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 22:50:28 | `ABBB29D7F552D8C9` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 22:50:32 | `75C84EA6AC1BAAAD` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 22:50:34 | `5D0C5CE7D2E3A2AF` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-29 00:33:02**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,08 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,75 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,47 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,94 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,05 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,46 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,90 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,27 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,22 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,42 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,82 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 25,44 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A19](#a19) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,09 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,32 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,18 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,42 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,14 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,75 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,47 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,94 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test framing::tests::hin_und_zurueck ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.47s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.14s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,05 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [329 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,46 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=c4cf81a4f940f91a29c0d9f627654635bed88b5307f4e0f640ca26a63d9f3018  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,90 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,27 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,42 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      jede gebaute TU hat exakt die heutige Definemenge (5 TUs, 16 Defines)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii,fourcc-int-be]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,82 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
  ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird
  ok      faellt, wenn ein stillgelegtes Ziel nirgends benannt ist
  ok      faellt, wenn ein aktives Ziel still aus der Auslieferung faellt
  ok      faellt an einem Produktnamen ausserhalb der Pfade
  ok      faellt an einem Bundlenamen ausserhalb der Pfade
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 3 Artefakt(e) ohne Hash: main, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

45 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 25,44 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-h5kpvx4j\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-h5kpvx4j\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: m_null).
  ok      Nakama Suna : stillgelegt seit 2026-08-28, nicht installiert
  ok      EQ-Copilot installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-h5kpvx4j\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-h5kpvx4j\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-h5kpvx4j\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle 3 Artefakte melden `aktuell`  [3x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[3b] Ein stillgelegtes Bundle wird gemeldet, nicht geloescht
  ok      das vorgefundene stillgelegte Bundle wird mit vollem Pfad gemeldet
  ok      die Meldung nennt den Handgriff, statt nur zu klagen
  ok      und der Installer hat es NICHT angefasst - bytegleich vorgefunden
  ok      ohne Altbestand bleibt die Zeile ehrlich (`nicht installiert`)

[3c] Eine Auslieferung ganz OHNE stillgelegtes Ziel laeuft weiter
  ok      ohne stillgelegtes Ziel laeuft -Pruefen durch und schweigt darueber  [Exit 0]

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

82 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,09 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt mindestens ein Ziel  [3]
  ok      mindestens ein Ziel ist nicht stillgelegt  [2 aktiv, 1 stillgelegt]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes AKTIVE Ziel im Manifest hat hier eine Zeile  [2 vs 2]
  ok      jedes STILLGELEGTE Ziel im Manifest hat hier eine Zeile  [1 vs 1]
  ok      main: steht als AKTIVES Ziel im Manifest
  ok      active-probe: steht als AKTIVES Ziel im Manifest
  ok      passive-probe: die Kennung steht weiter im Manifest
  ok      passive-probe: und ist dort als stillgelegt markiert
  ok      passive-probe: plugin_code ist nicht geloescht  [NkPr]
  ok      passive-probe: bundle ist nicht geloescht  [Nakama Suna.vst3]
  ok      passive-probe: produktname ist nicht geloescht  [Nakama Suna]
  ok      passive-probe: component_cid ist nicht geloescht  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: controller_cid ist nicht geloescht  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: die Stilllegung nennt Datum und Entscheid  [2026-08-28 / design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      NakamaSuna: stillgelegt - kein Aufruf von nakama_sonde_ziel() mehr
  ok      NakamaSuna: stillgelegt - auch kein eigener juce_add_plugin-Block
  ok      NakamaSuna: das Bauskript erklaert die Stilllegung an Ort und Stelle
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [29 Aug 2026 12:37:37am vs 28 Aug 2026 11:59:24pm]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [29 Aug 2026 12:37:41am vs 28 Aug 2026 11:59:24pm]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 115 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,32 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      Matrix Klasse=main, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=main, Position=pre: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position pre is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=legacy, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for legacy', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=passive_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for passive_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=active_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for active_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [6a09220777114a508d5898250bd90120]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [1692f4bf60b540829bb3f90a0c6d0e59]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 169 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260829-003831.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (20950 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,42 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3853 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,14 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [35 Bloecke]
[ok]   und mindestens einen Frame gebaut  [3 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 28,04 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>

