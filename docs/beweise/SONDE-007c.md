# Beweismanifest — SONDE-007c «Suna-Ziel stilllegen»
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-28 offen -->
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-29 nachgearbeitet -->

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

**Zusätzlich geändert in der Nacharbeit Runde 1 (29.08.2026)** — Einzelheiten in
[§6](#nacharbeit-1):

- `eq-copilot/cmake/NakamaIdentitaet.cmake` — der Riegel fällt jetzt wirklich
  auf die *Anwesenheit* der Marke (`stillgelegt-NOTFOUND` als einziger
  Durchlass-Fall); zweite Meldung für eine vorhandene, aber unlesbare Marke;
  leere `am`/`entscheid` erscheinen nicht mehr als `…-NOTFOUND` im Text.
- `eq-copilot/install/Install-Nakama.ps1` — `$null -ne $_.stillgelegt` durch
  die Anwesenheitsfrage ersetzt; eine vorhandene, nicht objektförmige Marke
  bricht ab, statt das Ziel als aktiv zu zählen.
- `eq-copilot/plugin/tests/IdentityTestMain.cpp` — neue Hilfsfunktion
  `hatStilllegungsmarke()` (`DynamicObject::hasProperty`, weil JUCE JSON-`null`
  und „Schlüssel fehlt" auf dasselbe leere `var` abbildet); neue Prüfung
  „jede vorhandene Stilllegungsmarke ist ein lesbares Objekt".
- `eq-copilot/schemas/installer/nakama-installer-v1.md` — der Absatz nach der
  Feldtabelle sagt statt `3 vs 3` die Beziehung *aktiv ↔ Artefakt*.
- `tools/eq-copilot/pruefe_installer_manifest.py` — neue Hilfsfunktion
  `_brauchbare_ids()`; `r_stillgelegte_benannt` validiert IDs vor `sorted`/`set`;
  sieben neue Gegenproben.
- `tools/eq-copilot/pruefe_installer_gegenpfad.py` — neuer Block `[3d]`: eine
  unlesbare Marke sperrt den Installer, auch mit geschmuggeltem
  Artefakteintrag.

`eq-copilot/identity/plugin-identities-v1.json` ist in dieser Runde **nicht**
angefasst worden (`git diff` leer); alle Gegenproben liefen gegen Kopien unter
`%TEMP%`.

---


> **Verlauf ausgegliedert (30.08.2026, NAK-100 / Dirigent §3.5):** Nacharbeitsrunden, Dirigentenstände und
> ältere Kanon-Rohläufe stehen byteweise unverändert in `SONDE-007c-verlauf.md` (append-only, kein Prüfgegenstand;
> jeder Abschnitt gilt zu dem Stand, den er nennt). Hier bleiben Urteilsmarken, Behauptungen, Belege, der
> jüngste Kanon-Lauf und der jüngste Dirigentenstand. Stand der Ausgliederung: `dc7ce7d`.

## 1. Ticket-Behauptungen

| # | Behauptung | Befehl | Ergebnis | Rohausgabe |
|---|---|---|---|---|
| 1 | **`NakamaSuna_VST3` entsteht nicht mehr.** Der Configure kennt das Ziel nicht; die Solution enthält keinen Suna-Eintrag; der Runner baut es nicht | `cmake -S eq-copilot -B eq-copilot/build` · `grep -ci suna EqCopilotSuite.sln` | ☑ 0 Treffer | [↓ C1](#c1) |
| 2 | **Der Identitätsleser ist fail-closed.** Wer den Aufruf wieder einsetzt, bekommt keinen stillen Bau, sondern einen FATAL_ERROR mit Datum und Entscheid — der Riegel hängt am Manifest, nicht an der gelöschten Zeile. **Runde 1:** jetzt mit dem **echten** Aufruf gemessen (Probe-Configure gegen die echte Identitätsdatei), nicht mehr nur mit einer Typ-Mikroprobe | `cmake -S <probe> -B <probe>/build` mit `nakama_identitaet_lesen(passive-probe PROBE)` | ☑ **gefallen**, Exit 1 | [↓ C2a](#c2a) |
| 3 | **Die Stilllegung ist am Bauskript gemessen, nicht behauptet.** `EqCopIdentityTest` misst: kein `nakama_sonde_ziel(NakamaSuna`, kein eigener `juce_add_plugin(NakamaSuna`, und der Grund steht an Ort und Stelle | `EqCopIdentityTest.exe` | ☑ 115/115 | Kanon-Lauf, B1 |
| 4 | **Die Kennung ist NICHT gelöscht.** Derselbe Test misst, dass `plugin_code`, `bundle`, `produktname` und beide CIDs von `passive-probe` vollständig dastehen und die Stilllegung Datum und Entscheid nennt | `EqCopIdentityTest.exe` | ☑ | Kanon-Lauf, B1 |
| 5 | **Die harte Drei ist weg — überall.** Der Test misst die Beziehung „aktive Kennungen ↔ gebaute Ziele" und „stillgelegte ↔ nicht gebaute", nicht eine Zahl. **Runde 1:** der Satz war unwahr, solange `nakama-installer-v1.md:91-94` weiter „A17 zählt `3 vs 3`" behauptete — dieser Absatz trägt jetzt dieselbe Beziehung wie §2.3 | `EqCopIdentityTest.exe` · `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | ☑ `2 aktiv, 1 stillgelegt` · `2 vs 2` · `1 vs 1` · A17 meldet `2 vs 2 aktiv (3 Kennungen gesamt)` | Kanon-Lauf, B1 · [↓ C2b](#c2b) |
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
| 18 | **Die „keine zweite Identität"-Regel misst jetzt, was sie seit dem 23.08. behauptet.** Der Manifestkopf sagt „WEDER Produktnamen NOCH Viercodes NOCH Class-IDs" — gemessen wurden bis heute nur zwei der drei Hälften. Beim Verschärfen fiel **sofort ein Treffer**: mein eigener Stilllegungstext hatte „Nakama Probeeq" beiläufig festgeschrieben. Entfernt; zwei eigene Gegenproben brechen die neue Hälfte einzeln | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | ☑ 45/45 (Runde 1: 52/52) · ☑ **gefallen** an Produkt- und Bundlename einzeln | [↓ C7](#c7) |

**Neu in der Nacharbeit Runde 1 (29.08.2026):**

| # | Behauptung | Befehl | Ergebnis | Rohausgabe |
|---|---|---|---|---|
| 19 | **Jede vorhandene Stilllegungsmarke sperrt — unabhängig von ihrem Inhalt.** Der Riegel fällt auf die *Anwesenheit*: `OBJECT` ergibt die Meldung mit Datum und Entscheid, jeder andere Typ die Meldung „unlesbar"; **kein** Typ lässt weiterbauen. Der Kommentar behauptete das seit dem 28.08., der Code maß es nicht | 6 Probe-Configures, je ein `stillgelegt`-Typ | ☑ NULL · STRING · ARRAY · NUMBER · BOOLEAN je `Exit 1` „unlesbar" · OBJECT `Exit 1` „STILLGELEGT" | [↓ C2c](#c2c) |
| 20 | **Ohne den Fix war das Loch fünf von sechs Typen groß — vorgeführt.** Dieselben sechs Läufe gegen das Modul von `043b48f`: NULL, STRING, ARRAY, NUMBER und BOOLEAN kamen mit `Exit 0` durch und lieferten die Identität des stillgelegten Ziels aus | dieselben Probe-Configures, Modul aus `git show 043b48f:` | ☑ **5× DURCHGELASSEN** (`Exit 0`), nur OBJECT gesperrt | [↓ C2c](#c2c) |
| 21 | **Alle vier Leser der Marke sperren fail-closed** — *für A17 erst seit Runde 2 gemessen.* Identitätsleser (CMake), Installer, `EqCopIdentityTest` und A17 klassifizieren nach der **Anwesenheit** der Marke und verlangen ihre **Objektform**; ein Ziel mit Marke wird bei keinem der vier je wieder „aktiv". Den **Feldinhalt** misst nicht jeder gleich streng: A17 verlangt im Kanon `am` und `entscheid` als nichtleere Zeichenketten (`r_stilllegungsmarke_lesbar`), `EqCopIdentityTest` verlangt Nichtleere nach `toString()`; `Install-Nakama.ps1` und `NakamaIdentitaet.cmake` prüfen den Inhalt nicht. Vor Runde 1 gab es drei verschiedene Antworten. **Berichtigt am 29.08.2026 (Runde 2, Befund P1):** der Beleg dieser Zeile maß nur den Installer (A18 `[3d]`) und `EqCopIdentityTest` — für A17 stand die Aussage ohne Rohausgabe da, **und sie war falsch**; siehe Behauptung 23. **Auf das Gemessene begrenzt am 30.08.2026 (Runde 4, NAK-89):** der frühere Satz „ein kaputter Inhalt ist überall ein harter Fehler" behauptete für alle vier Leser eine Inhaltsprüfung, die nur zwei von ihnen führen — er ist gefallen. Einordnung „Lücke, kein Defekt" und die Regeln dieser Runde: Konvergenzentscheid des Dirigenten am Ende dieses Manifests | A18 Block `[3d]` · `EqCopIdentityTest.exe` gegen eine gespiegelte `null`-Marke · **Runde 2:** `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | ☑ A18 `87 ok, 0 Fehler` (5 neue Prüfungen in `[3d]`) · ☑ Test wird rot: `[1 unlesbar]`, Zählung bleibt korrekt `2 aktiv, 1 stillgelegt` · ☑ **A17 seit Runde 2** `94 ok, 0 Fehler`, Regel `r_stilllegungsmarke_lesbar` | [↓ C2d](#c2d) · [↓ C2e](#c2e) · [↓ C2f](#c2f) |
| 22 | **A17 lehnt eine kaputte Stilllegungs-ID ab, statt am eigenen `set()` zu sterben.** Sechs neue Gegenproben plus der Fall gemischter Typen; **ohne** den Fix stirbt dieselbe Mutation an `TypeError` — vorgeführt gegen den Stand `043b48f` | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` | ☑ `52 ok, 0 Fehler` · ☑ **ohne Fix** `TypeError: unhashable type: 'list'` bzw. `'<' not supported between instances of 'int' and 'str'` | [↓ C2b](#c2b) |

**Neu in der Nacharbeit Runde 2 (29.08.2026):**

| # | Behauptung | Befehl | Ergebnis | Rohausgabe |
|---|---|---|---|---|
| 23 | **A17 weist eine unlesbare Stilllegungsmarke jetzt ab — vorher ließ A17 sie mit `Exit 0` durch.** Neue Regel `r_stilllegungsmarke_lesbar`: der Wert von `stillgelegt` muss ein Objekt mit `am` und `entscheid` als nichtleere Zeichenketten sein. Die beiden Feldnamen sind an den anderen Lesern gemessen, nicht erfunden (`NakamaIdentitaet.cmake:153-154`, `IdentityTestMain.cpp:388-391`) | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` · dieselben Marken-Varianten gegen `git show 05dbbb1:…` | ☑ `94 ok, 0 Fehler`, jede Variante rot · ☑ **ohne Fix**: `null`, String, Array, Zahl, Boolean, `{}` und leeres `am` — **alle sieben grün**, alle 14 Regeln bestanden | [↓ C2f](#c2f) |
| 24 | **Eine kaputte Marke macht ein Ziel trotzdem nie wieder „aktiv".** Die Mengeneinteilung fällt weiter auf die *Anwesenheit* — das ist die gemeinsame Regel aller vier Leser; die *Lesbarkeit* misst seit dieser Runde die eigene Regel daneben. Acht Varianten messen beide Hälften einzeln | dieselbe A17-Rohausgabe, Block `[3]` | ☑ je Variante zwei Zeilen: „fällt, wenn die Stilllegungsmarke … ist" **und** „und das Ziel bleibt trotzdem stillgelegt" | [↓ C2f](#c2f) |
| 25 | **Die vier Pflichtfelder je Stilllegungseintrag werden typstreng geprüft.** `seit`, `warum`, `umgang_mit_altbestand` und `kennung_bleibt` müssen nichtleere Zeichenketten sein. Der Typ ist am echten Manifest gemessen, weil Vertrag §2.3 die Felder fordert, aber keinen Typ nennt. Vorher stand dort `str(e.get(feld, "")).strip()` — und `str(None)` sind vier nichtleere Zeichen | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` · 4 Felder × 6 Werte, dieselben gegen `05dbbb1` | ☑ 24 neue Gegenproben, alle rot · ☑ **ohne Fix**: `null`, `[]` und `{}` in **jedem** der vier Felder grün — nur `""` und `"   "` fielen | [↓ C2g](#c2g) |
| 26 | **Der Installer bricht bei unbrauchbarem Pflichtfeld ab, statt eine Zeile mit leerem Datum zu drucken.** Der Riegel steht vor `Melde-StillgelegteAltlasten` und gilt in allen drei Betriebsarten; A18 fährt ihn im neuen Block `[3e]` | `py -3.13 tools/eq-copilot/pruefe_installer_gegenpfad.py` | ☑ `127 ok, 0 Fehler`, 40 neue Prüfungen · ☑ **ohne Fix** (dasselbe Bein gegen `git show 05dbbb1:…Install-Nakama.ps1`): `87 ok, 40 Fehler`, **alle 40 aus `[3e]`**, jeder Lauf `Exit 0` — und die Meldung lautete wörtlich `ok      Nakama Suna : stillgelegt seit , nicht installiert` | [↓ C2h](#c2h) |

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

> ⚠️ **Nacharbeit Runde 1 (29.08.2026):** Dieser Block C2 ist **Vorarbeit**, kein
> Beweis für Behauptung 2. Der T2-Prüfer hat das zu Recht beanstandet: er
> enthält eine `string(JSON … TYPE)`-Mikroprobe und einen kopierten
> Quellausschnitt, aber **keinen einzigen Aufruf** von
> `nakama_identitaet_lesen`. Was dort behauptet wurde, misst er nicht. Der
> echte Aufruf steht jetzt in [C2a](#c2a); der abgedruckte Quellausschnitt
> zeigt außerdem den **alten**, inzwischen ersetzten Vergleich gegen `OBJECT`.

<a id="c2a"></a>
### C2a · Der echte Aufruf fällt (Behauptung 2)

Ein Probe-Configure bindet **das Modul aus dem Repo** ein und ruft die echte
Funktion mit der echten Identitätsdatei auf — kein Nachbau, keine Mikroprobe.
Das Probeprojekt (`<probe>/CMakeLists.txt`):

```cmake
cmake_minimum_required(VERSION 3.22)
project(NakamaRiegelProbe NONE)
include("C:/Users/phili/Projekte/Nakama/eq-copilot/cmake/NakamaIdentitaet.cmake")
nakama_identitaet_lesen(passive-probe PROBE)
message(STATUS "DURCHGELASSEN: PROBE_PRODUKTNAME=${PROBE_PRODUKTNAME}")
```

**Befehl:** `cmake -S <probe> -B <probe>/build`

```
-- Building for: Visual Studio 17 2022
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- Configuring incomplete, errors occurred!
CMake Error at C:/Users/phili/Projekte/Nakama/eq-copilot/cmake/NakamaIdentitaet.cmake:168 (message):
  S9b/SONDE-007c: Ziel 'passive-probe' ist seit 2026-08-28 STILLGELEGT und
  wird nicht gebaut.

  Entscheid: design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md

  Seine Kennung bleibt im Identitaetsmanifest reserviert und gesperrt
  (NAK-30) -

  das ist kein Freibrief, sie wieder zu bauen.  Soll das Ziel zurueckkommen,
  gehoert

  der Weg dorthin in ein eigenes Ticket samt Abnahme, nicht in diese Zeile.
Call Stack (most recent call first):
  CMakeLists.txt:4 (nakama_identitaet_lesen)


EXITCODE: 1
```

Die Zeile `DURCHGELASSEN` erscheint nicht — der Aufruf kommt nicht zurück.

<a id="c2b"></a>
### C2b · A17: Typprüfung der Stilllegungs-IDs (Behauptungen 5, 22)

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py`

```
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
  ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird
  ok      faellt, wenn ein stillgelegtes Ziel nirgends benannt ist
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine leere Liste
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist ein Objekt
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine leere Zeichenkette
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist nur Leerraum
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine Zahl
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id fehlt ganz
  ok      faellt kontrolliert bei gemischten ziel_id-Typen in einer Liste
  ok      faellt, wenn ein aktives Ziel still aus der Auslieferung faellt
…
52 ok, 0 Fehler
exit=0
```

Der gemeldete Vergleich lautet `2 vs 2 aktiv (3 Kennungen gesamt)` — genau das,
was der Vertrag nach der Korrektur von `nakama-installer-v1.md:91-94` sagt.
Vorher stand dort „A17 zählt `3 vs 3`".

**Gegenprobe — dieselben Mutationen gegen den Stand `043b48f`.** Beide Regeln
werden aus derselben Datei geladen, einmal aus `git show 043b48f:…`, einmal aus
dem Arbeitsstand:

**Befehl:** `py -3.13 <gegenprobe>.py <a17_vor_fix.py> tools/eq-copilot/pruefe_installer_manifest.py .`

```
=== Mutation: ziel_id = [] ==============================
  VOR  dem Fix (043b48f): ABSTURZ TypeError: unhashable type: 'list'
        if len(benannt) != len(set(benannt)):
                               ~~~^^^^^^^^^
    TypeError: unhashable type: 'list'

  NACH dem Fix: (False, "stillgelegte_ziele[0]: ziel_id ist keine nichtleere Zeichenkette (list: []); benannt [] != stillgelegt ['passive-probe']")
=== Mutation: gemischte Typen (str + int) ==============================
  VOR  dem Fix (043b48f): ABSTURZ TypeError: '<' not supported between instances of 'int' and 'str'
        if sorted(x for x in benannt if x is not None) != stillgelegt_ids:
           ~~~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    TypeError: '<' not supported between instances of 'int' and 'str'

  NACH dem Fix: (False, 'stillgelegte_ziele[1]: ziel_id ist keine nichtleere Zeichenkette (int: 7)')
```

Beide vom Prüfer genannten Absturzstellen sind reproduziert: `set()` bei einer
Liste und `sorted()` bei gemischten Typen. Nach dem Fix ist beides ein
Regelbefund im Klartext.

<a id="c2c"></a>
### C2c · Jede vorhandene Marke sperrt — und wie groß das Loch war (Behauptungen 19, 20)

Sechs Probe-Configures, je einer pro `stillgelegt`-Typ, jeweils gegen eine
**Kopie** der Identitätsdatei unter `%TEMP%` (die echte Datei bleibt
unverändert; ihr `git diff` ist leer). Das Modul ist bytegleich kopiert,
einmal vom Arbeitsstand (`nach`), einmal aus `git show 043b48f:` (`vor`).

**Befehl:** je `cmake -S <probe> -B <probe>/build`

```
  Modul nach  stillgelegt-Typ NULL         Exit=1  FATAL - unlesbare Marke, Typ NULL, erwartet OBJECT
  Modul nach  stillgelegt-Typ STRING       Exit=1  FATAL - unlesbare Marke, Typ STRING, erwartet OBJECT
  Modul nach  stillgelegt-Typ ARRAY        Exit=1  FATAL - unlesbare Marke, Typ ARRAY, erwartet OBJECT
  Modul nach  stillgelegt-Typ NUMBER       Exit=1  FATAL - unlesbare Marke, Typ NUMBER, erwartet OBJECT
  Modul nach  stillgelegt-Typ BOOLEAN      Exit=1  FATAL - unlesbare Marke, Typ BOOLEAN, erwartet OBJECT
  Modul nach  stillgelegt-Typ OBJECT-leer  Exit=1  FATAL - STILLGELEGT-Meldung
  Modul vor   stillgelegt-Typ NULL         Exit=0  DURCHGELASSEN - Ziel waere gebaut worden
  Modul vor   stillgelegt-Typ STRING       Exit=0  DURCHGELASSEN - Ziel waere gebaut worden
  Modul vor   stillgelegt-Typ ARRAY        Exit=0  DURCHGELASSEN - Ziel waere gebaut worden
  Modul vor   stillgelegt-Typ NUMBER       Exit=0  DURCHGELASSEN - Ziel waere gebaut worden
  Modul vor   stillgelegt-Typ BOOLEAN      Exit=0  DURCHGELASSEN - Ziel waere gebaut worden
  Modul vor   stillgelegt-Typ OBJECT-leer  Exit=1  FATAL - STILLGELEGT-Meldung
```

Der `vor`-Block ist der Befund P1 Nr. 1 in Zahlen: **fünf von sechs Typen kamen
durch**, und der Configure gab dabei die Identität des stillgelegten Ziels aus
(`DURCHGELASSEN: PROBE_PRODUKTNAME=Nakama Suna PROBE_PLUGINCODE=NkPr`).

**Was das `cmake -P`-Verhalten dahinter ist** (gemessen, nicht angenommen):

```
-- a: TYPE=[OBJECT] ERR=[NOTFOUND]
-- b: TYPE=[NULL] ERR=[NOTFOUND]
-- c: TYPE=[STRING] ERR=[NOTFOUND]
-- d: TYPE=[ARRAY] ERR=[NOTFOUND]
-- e: TYPE=[NUMBER] ERR=[NOTFOUND]
-- f: TYPE=[BOOLEAN] ERR=[NOTFOUND]
-- g: TYPE=[g-NOTFOUND] ERR=[member 'g' not found]
```

Ein **vorhandener** Schlüssel liefert nie `-NOTFOUND`. Deshalb ist
`stillgelegt-NOTFOUND` der eine benennbare Durchlass-Fall, und der Riegel prüft
seither auf ihn statt auf `OBJECT`.

**Nachtrag aus dem T1-Selbstaudit derselben Runde:** bei einem *leeren*
Marken-Objekt lautete die Meldung „ist seit `stillgelegt-am-NOTFOUND`
STILLGELEGT" — fail-closed, aber ein Text, der eine CMake-Interna als Datum
ausgibt. Behoben; jetzt:

```
S9b/SONDE-007c: Ziel 'passive-probe' ist seit <im Manifest nicht angegeben>
Entscheid: <im Manifest nicht angegeben>
EXITCODE: 1
```

Der echte Fall bleibt unverändert (`ist seit 2026-08-28 … Entscheid:
design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md`). Dass die Marke
Datum **und** Entscheid trägt, misst weiterhin `EqCopIdentityTest`.

<a id="c2d"></a>
### C2d · Der Installer klassifiziert wie alle anderen (Behauptung 21)

Neuer Block `[3d]` in A18 — gegen den **echten** Installer in der Sandbox:

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_installer_gegenpfad.py`

```
[3d] Eine unlesbare Stilllegungsmarke sperrt, statt 'aktiv' zu heissen
  ok      Marke vom Typ null bricht ab und benennt sich  [Exit 1]
  ok      Marke vom Typ String bricht ab und benennt sich  [Exit 1]
  ok      Marke vom Typ Array bricht ab und benennt sich  [Exit 1]
  ok      Marke vom Typ Zahl bricht ab und benennt sich  [Exit 1]
  ok      auch MIT geschmuggeltem Artefakteintrag sperrt die Marke zuerst  [Exit 1]
…
87 ok, 0 Fehler
exit=0
```

**Gegenprobe:** dasselbe Bein, derselbe Sandbox-Aufbau, nur der Installer auf
den Stand `043b48f` zurückgedreht (Repo-Spiegel unter `%TEMP%`, in dem
ausschließlich `Install-Nakama.ps1` aus `git show` stammt):

```
[3d] Eine unlesbare Stilllegungsmarke sperrt, statt 'aktiv' zu heissen
  FEHLER  Marke vom Typ null bricht ab und benennt sich  [Exit 1]
  FEHLER  Marke vom Typ String bricht ab und benennt sich  [Exit 0]
  FEHLER  Marke vom Typ Array bricht ab und benennt sich  [Exit 0]
  FEHLER  Marke vom Typ Zahl bricht ab und benennt sich  [Exit 0]
  FEHLER  auch MIT geschmuggeltem Artefakteintrag sperrt die Marke zuerst  [Exit 1]
EXITCODE: 2
```

Drei der fünf Fälle liefen **grün durch**; die zwei, die abbrachen, benannten
die Marke nicht. In einer Sandbox ohne festgeschriebene Hashes zeigt derselbe
Vorstand, *welche* falsche Erklärung er stattdessen gab:

```
  Marke=null    Exit=1  ABBRUCH: Das Manifest muss jedes NICHT stillgelegte Identitaetsziel genau einmal als VST3-Artefakt enthalten.
```

Das ist die Klassifikation aus dem Befund: `"stillgelegt": null` galt als
**aktiv**. Der Grund liegt in der Sprache und ist gemessen:

```
a: anwesend=False altTest=False typ=<null> isPSCustomObject=False
b: anwesend=True altTest=False typ=<null> isPSCustomObject=False
c: anwesend=True altTest=True typ=String isPSCustomObject=False
d: anwesend=True altTest=True typ=Object[] isPSCustomObject=False
e: anwesend=True altTest=True typ=PSCustomObject isPSCustomObject=True
```

(`a` = keine Marke, `b` = `null`, `c` = String, `d` = Array, `e` = Objekt.)

Zeile `b` ist der Fall: die Property **ist da**, aber `$null -ne $_.stillgelegt`
(`altTest`) sagt „nein" — genau wie bei der fehlenden Property in Zeile `a`.

<a id="c2e"></a>
### C2e · `EqCopIdentityTest` sperrt dieselbe Marke (Behauptung 21)

`finde()` sucht zuerst relativ zum Arbeitsverzeichnis. Der Test läuft deshalb
zweimal gegen einen CWD-Spiegel unter `%TEMP%`, der **nur** den Identitätsordner
enthält — einmal unverändert, einmal mit `stillgelegt: null`. Alles andere
findet er über den Pfad der Programmdatei im echten Repo. Der Spiegel erzeugt
in beiden Läufen dieselben fünf sachfremden Fehler (die Goldens liegen neben der
gespiegelten Datei); der **Unterschied** ist der Beweis:

```
=== CWD-Spiegel 'echt' : Exit=1 ===
   ok      mindestens ein Ziel ist nicht stillgelegt  [2 aktiv, 1 stillgelegt]
   ok      jede vorhandene Stilllegungsmarke ist ein lesbares Objekt  [0 unlesbar]
   ok      passive-probe: und ist dort als stillgelegt markiert
   IDENTITY-TEST FEHLGESCHLAGEN - 99 Pruefungen ok, 5 Fehler
=== CWD-Spiegel 'null' : Exit=1 ===
   ok      mindestens ein Ziel ist nicht stillgelegt  [2 aktiv, 1 stillgelegt]
   FEHLER  jede vorhandene Stilllegungsmarke ist ein lesbares Objekt  [1 unlesbar]
   FEHLER  passive-probe: und ist dort als stillgelegt markiert
   IDENTITY-TEST FEHLGESCHLAGEN - 96 Pruefungen ok, 8 Fehler
```

Zwei Dinge stehen darin: die neue Prüfung wird rot, **und** die Zählung bleibt
`2 aktiv, 1 stillgelegt`. Genau das war vorher unmöglich — mit
`ziele[i]["stillgelegt"].isObject()` hätte dieselbe Datei `3 aktiv, 0
stillgelegt` ergeben, weil JUCE JSON-`null` auf dasselbe leere `var` abbildet
wie einen fehlenden Schlüssel (`juce_JSON.cpp`, `case 'n': … return {};`).
Diese letzte Aussage ist an der JUCE-Quelle **gelesen**, nicht gefahren — die
gemessene Hälfte ist die korrekte Zählung oben.

Auf dem echten Repo-Stand (ohne Spiegel) läuft der Test vollständig grün:

```
  ok      mindestens ein Ziel ist nicht stillgelegt  [2 aktiv, 1 stillgelegt]
  ok      jede vorhandene Stilllegungsmarke ist ein lesbares Objekt  [0 unlesbar]
  ok      jedes AKTIVE Ziel im Manifest hat hier eine Zeile  [2 vs 2]
  ok      jedes STILLGELEGTE Ziel im Manifest hat hier eine Zeile  [1 vs 1]

IDENTITY-TEST OK - 116 Pruefungen ok, 0 Fehler
EXITCODE: 0
```

<a id="c5"></a>
### C5 · Keine Altlast auf dieser Maschine (Behauptung 15)

**Befehl:** `ls "C:/Program Files/Common Files/VST3/" | grep -i nakama`

```
(kein Treffer — weder "Nakama Suna.vst3" noch "Nakama Probeeq.vst3")
```

Aus diesem Projekt liegen dort `EQ-Copilot.vst3` (der Bau vom 16.08.2026,
`74D86BD5…`), die beiden Wegwerf-Messgeräte `EqCop-Aux-Spike.vst3` und
`EqCop-Host-Probe.vst3` sowie ein Datenordner `eq-copilot` — kein Bundle. Die Stilllegung trifft
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

<a id="c2f"></a>
### C2f · A17 ließ eine unlesbare Stilllegungsmarke durch (Behauptungen 21, 23, 24)

Die Probe ruft **alle** Strukturregeln des jeweiligen Moduls gegen eine tiefe
Kopie der echten Identitätsdatei auf und mutiert nur den Wert von
`stillgelegt`. Die Datei auf Platte bleibt unberührt — `git diff` auf
`eq-copilot/identity/plugin-identities-v1.json` ist in dieser Runde leer.

**Vorher** — `git show 05dbbb1:tools/eq-copilot/pruefe_installer_manifest.py`:

```
== P1: unlesbare Stilllegungsmarke in der IDENTITAET ==
  stillgelegt = null           -> GRUEN (Exit 0)
  stillgelegt = String         -> GRUEN (Exit 0)
  stillgelegt = Array          -> GRUEN (Exit 0)
  stillgelegt = Zahl           -> GRUEN (Exit 0)
  stillgelegt = Boolean        -> GRUEN (Exit 0)
  stillgelegt = leeres Objekt  -> GRUEN (Exit 0)
  stillgelegt = am leer        -> GRUEN (Exit 0)
```

Vierzehn Regeln, sieben kaputte Marken, kein einziger Befund. A17 hätte in
jedem dieser Fälle mit `Exit 0` beglaubigt, was CMake-Leser,
`EqCopIdentityTest` und Installer hart ablehnen.

**Nachher** — Arbeitsstand:

```
== P1: unlesbare Stilllegungsmarke in der IDENTITAET ==
  stillgelegt = null           -> ROT  ["r_stilllegungsmarke_lesbar: 'passive-probe': Stilllegungsmarke ist kein Objekt (NoneType: None)"]
  stillgelegt = String         -> ROT  ["r_stilllegungsmarke_lesbar: 'passive-probe': Stilllegungsmarke ist kein Objekt (str: 'x')"]
  stillgelegt = Array          -> ROT  ["r_stilllegungsmarke_lesbar: 'passive-probe': Stilllegungsmarke ist kein Objekt (list: [])"]
  stillgelegt = Zahl           -> ROT  ["r_stilllegungsmarke_lesbar: 'passive-probe': Stilllegungsmarke ist kein Objekt (int: 7)"]
  stillgelegt = Boolean        -> ROT  ["r_stilllegungsmarke_lesbar: 'passive-probe': Stilllegungsmarke ist kein Objekt (bool: True)"]
  stillgelegt = leeres Objekt  -> ROT  ["r_stilllegungsmarke_lesbar: 'passive-probe': `stillgelegt.am` ist keine nichtleere Zeichenkette (NoneType: None); 'passive-probe': `stillgelegt.entscheid` ist keine nichtleere Zeichenkette (NoneType: None)"]
  stillgelegt = am leer        -> ROT  ["r_stilllegungsmarke_lesbar: 'passive-probe': `stillgelegt.am` ist keine nichtleere Zeichenkette (str: ''); 'passive-probe': `stillgelegt.entscheid` ist keine nichtleere Zeichenkette (NoneType: None)"]
```

Dasselbe im Kanon-Bein selbst, gekürzt auf die neuen Zeilen:

```
[1] Struktur - eine Identitaet, ein Ort

  ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`

[2] Gegenprobe - dieselben Regeln an verdorbener Eingabe
  ok      faellt an verdorbener Eingabe: jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt, wenn die Stilllegungsmarke null ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke null ist
  ok      faellt, wenn die Stilllegungsmarke eine Zeichenkette ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke eine Zeichenkette ist
  ok      faellt, wenn die Stilllegungsmarke ein leeres Array ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein leeres Array ist
  ok      faellt, wenn die Stilllegungsmarke eine Zahl ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke eine Zahl ist
  ok      faellt, wenn die Stilllegungsmarke ein Boolean ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Boolean ist
  ok      faellt, wenn die Stilllegungsmarke ein leeres Objekt ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein leeres Objekt ist
  ok      faellt, wenn die Stilllegungsmarke ein Objekt mit leerem `am` ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Objekt mit leerem `am` ist
  ok      faellt, wenn die Stilllegungsmarke ein Objekt ohne `entscheid` ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Objekt ohne `entscheid` ist

94 ok, 0 Fehler
```

<a id="c2g"></a>
### C2g · `str(None)` ist `"None"` — die Pflichtfelder waren ungeprüft (Behauptung 25)

Dieselbe Probe, andere Seite: mutiert wird der Manifesteintrag unter
`stillgelegte_ziele`, nicht die Identität.

**Vorher** — `git show 05dbbb1:…`:

```
== P2: Pflichtfelder im MANIFEST-Block stillgelegte_ziele ==
  seit                   = null   -> GRUEN (Exit 0)
  seit                   = []     -> GRUEN (Exit 0)
  seit                   = {}     -> GRUEN (Exit 0)
  seit                   = ""     -> ROT  ['passive-probe': seit fehlt]
  seit                   = "   "  -> ROT  ['passive-probe': seit fehlt]
  warum                  = null   -> GRUEN (Exit 0)
  warum                  = []     -> GRUEN (Exit 0)
  warum                  = {}     -> GRUEN (Exit 0)
  warum                  = ""     -> ROT  ['passive-probe': warum fehlt]
  warum                  = "   "  -> ROT  ['passive-probe': warum fehlt]
  umgang_mit_altbestand  = null   -> GRUEN (Exit 0)
  umgang_mit_altbestand  = []     -> GRUEN (Exit 0)
  umgang_mit_altbestand  = {}     -> GRUEN (Exit 0)
  umgang_mit_altbestand  = ""     -> ROT  ['passive-probe': umgang_mit_altbestand fehlt]
  umgang_mit_altbestand  = "   "  -> ROT  ['passive-probe': umgang_mit_altbestand fehlt]
  kennung_bleibt         = null   -> GRUEN (Exit 0)
  kennung_bleibt         = []     -> GRUEN (Exit 0)
  kennung_bleibt         = {}     -> GRUEN (Exit 0)
  kennung_bleibt         = ""     -> ROT  ['passive-probe': kennung_bleibt fehlt]
  kennung_bleibt         = "   "  -> ROT  ['passive-probe': kennung_bleibt fehlt]
```

Drei von fünf Werten je Feld bestanden. Der Grund steht in einer einzigen
Zeile: `str(e.get(feld, "")).strip()` konvertiert **vor** der Prüfung, und
`str(None)` sind vier nichtleere Zeichen — `str([])` und `str({})` je zwei.

**Nachher** — Arbeitsstand:

```
== P2: Pflichtfelder im MANIFEST-Block stillgelegte_ziele ==
  seit                   = null   -> ROT  ['passive-probe': seit ist keine nichtleere Zeichenkette (NoneType: None)]
  seit                   = []     -> ROT  ['passive-probe': seit ist keine nichtleere Zeichenkette (list: [])]
  seit                   = {}     -> ROT  ['passive-probe': seit ist keine nichtleere Zeichenkette (dict: {})]
  seit                   = ""     -> ROT  ['passive-probe': seit ist keine nichtleere Zeichenkette (str: '')]
  seit                   = "   "  -> ROT  ['passive-probe': seit ist keine nichtleere Zeichenkette (str: '   ')]
  warum                  = null   -> ROT  ['passive-probe': warum ist keine nichtleere Zeichenkette (NoneType: None)]
  warum                  = []     -> ROT  ['passive-probe': warum ist keine nichtleere Zeichenkette (list: [])]
  warum                  = {}     -> ROT  ['passive-probe': warum ist keine nichtleere Zeichenkette (dict: {})]
  warum                  = ""     -> ROT  ['passive-probe': warum ist keine nichtleere Zeichenkette (str: '')]
  warum                  = "   "  -> ROT  ['passive-probe': warum ist keine nichtleere Zeichenkette (str: '   ')]
  umgang_mit_altbestand  = null   -> ROT  ['passive-probe': umgang_mit_altbestand ist keine nichtleere Zeichenkette (NoneType: None)]
  umgang_mit_altbestand  = []     -> ROT  ['passive-probe': umgang_mit_altbestand ist keine nichtleere Zeichenkette (list: [])]
  umgang_mit_altbestand  = {}     -> ROT  ['passive-probe': umgang_mit_altbestand ist keine nichtleere Zeichenkette (dict: {})]
  umgang_mit_altbestand  = ""     -> ROT  ['passive-probe': umgang_mit_altbestand ist keine nichtleere Zeichenkette (str: '')]
  umgang_mit_altbestand  = "   "  -> ROT  ['passive-probe': umgang_mit_altbestand ist keine nichtleere Zeichenkette (str: '   ')]
  kennung_bleibt         = null   -> ROT  ['passive-probe': kennung_bleibt ist keine nichtleere Zeichenkette (NoneType: None)]
  kennung_bleibt         = []     -> ROT  ['passive-probe': kennung_bleibt ist keine nichtleere Zeichenkette (list: [])]
  kennung_bleibt         = {}     -> ROT  ['passive-probe': kennung_bleibt ist keine nichtleere Zeichenkette (dict: {})]
  kennung_bleibt         = ""     -> ROT  ['passive-probe': kennung_bleibt ist keine nichtleere Zeichenkette (str: '')]
  kennung_bleibt         = "   "  -> ROT  ['passive-probe': kennung_bleibt ist keine nichtleere Zeichenkette (str: '   ')]
```

Im Kanon-Bein, Block `[3]`:

```
  ok      faellt, wenn `seit` null ist
  ok      faellt, wenn `seit` ein leeres Array ist
  ok      faellt, wenn `seit` ein leeres Objekt ist
  ok      faellt, wenn `seit` leer ist
  ok      faellt, wenn `seit` nur Leerraum ist
  ok      faellt, wenn `seit` eine Zahl ist
  ok      faellt, wenn `warum` null ist
  ok      faellt, wenn `warum` ein leeres Array ist
  ok      faellt, wenn `warum` ein leeres Objekt ist
  ok      faellt, wenn `warum` leer ist
  ok      faellt, wenn `warum` nur Leerraum ist
  ok      faellt, wenn `warum` eine Zahl ist
  ok      faellt, wenn `umgang_mit_altbestand` null ist
  ok      faellt, wenn `umgang_mit_altbestand` ein leeres Array ist
  ok      faellt, wenn `umgang_mit_altbestand` ein leeres Objekt ist
  ok      faellt, wenn `umgang_mit_altbestand` leer ist
  ok      faellt, wenn `umgang_mit_altbestand` nur Leerraum ist
  ok      faellt, wenn `umgang_mit_altbestand` eine Zahl ist
  ok      faellt, wenn `kennung_bleibt` null ist
  ok      faellt, wenn `kennung_bleibt` ein leeres Array ist
  ok      faellt, wenn `kennung_bleibt` ein leeres Objekt ist
  ok      faellt, wenn `kennung_bleibt` leer ist
  ok      faellt, wenn `kennung_bleibt` nur Leerraum ist
  ok      faellt, wenn `kennung_bleibt` eine Zahl ist

94 ok, 0 Fehler
```

<a id="c2h"></a>
### C2h · Der Installer druckte „stillgelegt seit " ohne Datum (Behauptung 26)

Gefahren wird das **neue** Kanon-Bein A18 zweimal: einmal in einem
Scratch-Baum, in dem nur `eq-copilot/install/Install-Nakama.ps1` durch
`git show 05dbbb1:…` ersetzt ist (bytegleich geprüft), einmal mit dem
Arbeitsstand. Beide Scratch-Läufe tragen eine zusätzliche Diagnosezeile
`ROHZEILE:`, die nur dort im Skript steht und die betroffene Ausgabezeile des
Installers zeigt; im Repo steht sie nicht.

**Vorher** — alter Installer, neues Bein:

```
[3e] Ein unbrauchbares Pflichtfeld bricht ab, statt Luecken zu drucken
    ROHZEILE: ['  ok      Nakama Suna : stillgelegt seit , nicht installiert']
  FEHLER  `seit` als null bricht ab und benennt Feld und Typ  [Exit 0]
  FEHLER  und keine Altlastzeile mit leerem Datum (seit = null)
  FEHLER  `seit` als Array bricht ab und benennt Feld und Typ  [Exit 0]
  FEHLER  und keine Altlastzeile mit leerem Datum (seit = Array)
  FEHLER  `seit` als Objekt bricht ab und benennt Feld und Typ  [Exit 0]
  FEHLER  und keine Altlastzeile mit leerem Datum (seit = Objekt)
  ... (40 Pruefungen = 4 Pflichtfelder x 5 Werte x 2 Zusagen)

87 ok, 40 Fehler
```

Die Zeile `ok      Nakama Suna : stillgelegt seit , nicht installiert` ist der
Schaden in einem Satz: eine `ok`-Meldung, die ein Datum ankündigt und dann
eine Lücke zeigt. Alle 40 Fehler dieses Laufs stammen aus `[3e]`, die übrigen
Blöcke bleiben grün.

**Nachher** — Arbeitsstand:

```
[3e] Ein unbrauchbares Pflichtfeld bricht ab, statt Luecken zu drucken
    ROHZEILE: ["ABBRUCH: Stillgelegtes Ziel 'passive-probe': Pflichtfeld 'seit' ist keine nichtleere Zeichenkette (Typ null). Eine Stilllegung ohne brauchbares Datum, ohne Grund oder ohne Umgang mit dem Altbestand ist keine - repariere den Eintrag in eq-copilot/install/nakama-installer-v1.json (Vertrag nakama-installer-v1.md, Abschnitt 2.3), entferne ihn nicht."]
  ok      `seit` als null bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = null)
  ok      `seit` als Array bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = Array)
  ok      `seit` als Objekt bricht ab und benennt Feld und Typ  [Exit 1]
  ok      und keine Altlastzeile mit leerem Datum (seit = Objekt)
  ... (40 Pruefungen = 4 Pflichtfelder x 5 Werte x 2 Zusagen)

127 ok, 0 Fehler
```

Das Bein zieht die Ausgabe vor dem Vergleich flach und prüft seit dem
T1-Selbstaudit auch das **Ende** der Meldung. Grund: im ersten Wurf stand im
Abbruchtext ein Backtick vor `nakama-installer-v1.md` — in einer
PowerShell-Doppelquote ist der Backtick das Escape-Zeichen, Backtick plus `n`
also ein Zeilenumbruch. Der Riegel griff, aber sein Satz war ab „(Vertrag"
zerhackt. Eine Prüfung, die nur den Anfang einer Meldung liest, sieht genau
diesen Schaden nicht.

---

<a id="nacharbeit-1"></a>
## Kanon-Lauf - SONDE-007c NAK-94 vorgezogen - Abschluss

**Stand dieses Abschnitts:** `9b4bb4d` — Momentaufnahme dieses Laufs (Kopf-Tabelle des Laufs); Positionen ohne eigene Angabe sind an diesen Commit gebunden.

**Lauf:** 2026-08-29 20:25 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-007c-9b4bb4d.md](roh/SONDE-007c-9b4bb4d.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-29 20:25:21 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 9b4bb4d NAK-94: Beweisabschnitt in SONDE-007c und Registernachtrag |
| Commit (voll) | 9b4bb4d30b672dc2b13a923596735a4a3851e016 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-007c-9b4bb4d.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,67 s | [A2](roh/SONDE-007c-9b4bb4d.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,56 s | [A3](roh/SONDE-007c-9b4bb4d.md#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,07 s | [A4](roh/SONDE-007c-9b4bb4d.md#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,12 s | [A4b](roh/SONDE-007c-9b4bb4d.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,49 s | [A5](roh/SONDE-007c-9b4bb4d.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [A6](roh/SONDE-007c-9b4bb4d.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,15 s | [A7](roh/SONDE-007c-9b4bb4d.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,19 s | [A8](roh/SONDE-007c-9b4bb4d.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [A9](roh/SONDE-007c-9b4bb4d.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,37 s | [A10](roh/SONDE-007c-9b4bb4d.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [A11](roh/SONDE-007c-9b4bb4d.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,26 s | [A12](roh/SONDE-007c-9b4bb4d.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,23 s | [A13](roh/SONDE-007c-9b4bb4d.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. Gemessen wird kein vorhandenes Artefakt: das Bein loescht vor jeder Messung Objekte, Tlogs und Lib des Kernverzeichnisses und laesst NakamaKern vollstaendig neu uebersetzen und linken (dabei laeuft ueber ZERO_CHECK auch das Configure samt K2/K2b/K2c mit); ob eine fruehere Lib veraltet war, ist damit keine Frage mehr. Ist der Neubau nicht moeglich oder schlaegt er fehl, ist das Exit 3 und kein Urteil; ohne Neubau (--nur-messen) gibt es kein gruenes Frische-Urteil. Zusaetzlich gemessen: keine Datei aus plugin/**, die der Compiler tatsaechlich gelesen hat - erzwungene Includes und vorkompilierte Koepfe eingeschlossen -, traegt das Token JucePlugin_ ausserhalb von NakamaKernRiegel.h; jede vom Compiler gelesene Datei stammt aus einem erlaubten, aus dem Bau abgeleiteten Ort, wobei juce_audio_plugin_client, generierte JuceLibraryCode-Header und alles Unbekannte ROT sind; und der JUCE-Baum ist der gepinnte Tag plus genau der benannte Nakama-VST3-Patch, ohne fremde Aenderung und ohne unverfolgte Datei. Die eigenen Wachen des Beins (Configure-Stamps, vier Schalterklassen beidseitig, TU-Mengen, Linkfrische, lastbuildstate) belegen nur noch, WOMIT gebaut wurde; die AdditionalOptions-Klasse prueft dabei ausdruecklich nur Enthaltensein. Ausdruecklich nicht behauptet: der Inhalt der Toolchain- und SDK-Header ausserhalb des Repos (nur ihre Herkunft aus den abgeleiteten Wurzeln wird geprueft, kein Fingerprint), ein Compilerwechsel innerhalb derselben lastbuildstate-Kennung, und die uebrigen ClCompile-Elemente der Projektdatei (Warnstufe, Optimierung, Laufzeitbibliothek und die anderen) - sie sind durch den Neubau gegenstandslos, aber nicht einzeln nachgebildet. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 5,75 s | [A14](roh/SONDE-007c-9b4bb4d.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. Zum Auslieferungsstand sagt dieser Kanon-Lauf NICHT, dass der Bau dem festgeschriebenen Paket gleicht (nach einem Relink tut er das erwartungsgemaess nicht) - hart gemessen wird hier nur, dass jedes festgeschriebene Artefakt UEBERHAUPT vorliegt und sein Ordner-Hash bildbar ist; eine Hash-Abweichung erscheint als Hinweis mit beiden Kurz-Hashes, und [4b] berichtet ohne Urteil, ob der installierte Stand aus install-ergebnis.json dem Manifest entspricht. Hart verglichen wird der Hash nur mit --release (Auslieferungsschritt, hier nicht aufgerufen) und in Install-Nakama.ps1 Riegel 2 (NAK-94). | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,90 s | [A17](roh/SONDE-007c-9b4bb4d.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 37,14 s | [A18](roh/SONDE-007c-9b4bb4d.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/SONDE-007c-9b4bb4d.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,32 s | [A20](roh/SONDE-007c-9b4bb4d.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,35 s | [A21](roh/SONDE-007c-9b4bb4d.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,37 s | [A22](roh/SONDE-007c-9b4bb4d.md#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,12 s | [A16](roh/SONDE-007c-9b4bb4d.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,14 s | [B1](roh/SONDE-007c-9b4bb4d.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,44 s | [B2](roh/SONDE-007c-9b4bb4d.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,12 s | [B3](roh/SONDE-007c-9b4bb4d.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,16 s | [B3b](roh/SONDE-007c-9b4bb4d.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,27 s | [B3c](roh/SONDE-007c-9b4bb4d.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,50 s | [B4](roh/SONDE-007c-9b4bb4d.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,21 s | [B9](roh/SONDE-007c-9b4bb4d.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,35 s | [B5](roh/SONDE-007c-9b4bb4d.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 32,79 s | [B10](roh/SONDE-007c-9b4bb4d.md#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,23 s | [B8](roh/SONDE-007c-9b4bb4d.md#b8) |


## Dirigentenstand NAK-94 — 2026-08-30 11:17 (Sitzung 054eedac): Prüfer 14 PASS — NAK-94 geschlossen

**Stand dieses Abschnitts:** `6cd244d`

**Nacharbeit 13:** Opus/max `nakama-s8r18-nak94r13-e27974c-bau` (gemeinsam mit S8 Runde 18); Pflichtmenge der Fuzz-Verbraucher als fallende Wache; Kanon GRÜN 32/32 auf `0e7a60e` (Roh-Datei `docs/beweise/roh/SONDE-007a-0e7a60e.md`, Bein A17: 120 ok / 0 Fehler).
**Prüfer 14:** Codex high `01a051f2-0ce6-7771-ba00-efd937b10420`, lesend über `git diff da62dec...6cd244d`, HEAD vor/nach identisch — **PASS** (kein Defekt, keine Lücke, keine Härtung), wörtlich (`@ 6cd244d`):

> Der Vorprüferbefund ist geschlossen: Das Entfernen von `_fuzz_erzeuger` wird namentlich erkannt, Fixture-Bytes und Installer-Pfade blieben unverändert, und zusätzliche Einzelbyteproben endeten kontrolliert ohne Traceback. Der vollständige A17-Lauf war in der aktuellen Sandbox mangels beschreibbarem TEMP nicht erneut ausführbar; die eingecheckte Rohausgabe belegt jedoch den unveränderten Messcode mit 120 ok und 0 Fehlern.

**Urteil des Dirigenten:** NAK-94 ist **geschlossen** — Bereich `da62dec...6cd244d`, Prüfer 1–14 (Codex `gpt-5.6-sol`, high), Nacharbeiten 1–13, Wegwechsel W1 (Writer-Fixtures aus der A18-Sandbox), W3 (zentraler Fänger + Byte-Kipp-Fuzz `[3c]`). Zusagen, die seither für A17 gelten: `[4]` im Kanon weich, hart nur `--release` und Installer-Riegel 2; fehlendes Artefakt / nicht bildbarer Ordner-Hash beidseitig Fehler; `[4b]` ohne Urteil und ohne Abbruch, Statussperre vor der Eintragsliste; Writer-Fixtures und deklarierte Mutanten, Pflichtmenge aus `MANIFEST.json`; je Zusage genau ein diskriminierender Bruch; jede gelesene JSON-Datei strukturell geprüft, jede unerwartete Ausnahme kontrolliert — gemessen durch den Fuzz über die Pflichtverbraucher (A17-Kanonpfad, `--hashen`, Erzeuger-`pruefen()`). Das Installer-Manifest wurde **nie** neu gehasht (`git diff --stat da62dec 6cd244d -- eq-copilot/install/` leer). Die Urteilsmarke von S9b `SONDE-007c` bleibt unverändert (`T2 NEEDS_WORK 2026-08-29 nachgearbeitet`): NAK-89 ist weiter offen und Gegenstand der S9b-Nacharbeit.

---

## Konvergenzentscheid des Dirigenten — 2026-08-30 (Sitzung 3e24ab41): NAK-89 ist Lücke, nicht Defekt

**Stand:** `ab09f71`. Gate-Text S9b (Bauaufteilung S9b-Zeile, wörtlich im Kopf): „Kanon grün ohne A15, A17 mit zwei Bundles, `pluginval` SUCCESS an beiden, keine fremde CID" — er verlangt nicht, dass alle vier Leser der Stilllegungsmarke den Feldinhalt gleich streng prüfen. Kein Test bricht (Kanon GRÜN 32/32 auf `26811a0`, A17 95 ok). An der Quelle: (1) `IdentityTestMain.cpp` prüft `am`/`entscheid` über `toString().isNotEmpty()`, `Install-Nakama.ps1` die Objektform, `NakamaIdentitaet.cmake` setzt die Werte ungeprüft in die Abbruchmeldung, A17 `r_stilllegungsmarke_lesbar` verlangt nichtleere Zeichenketten — Behauptung 21 („ein kaputter Inhalt ist überall ein harter Fehler") geht über die Messung hinaus. (2) `eq-copilot/schemas/installer/nakama-installer-v1.md` §2.3 nennt für `seit`, `warum`, `umgang_mit_altbestand`, `kennung_bleibt` keinen Typ; A17 und Installer erzwingen nichtleere Zeichenketten. (3) Die A17-Behauptung in `tools/beweise.ps1` sagt seit NAK-94/NAK-100 „jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke" — Teil (3) ist geschlossen.

**Einordnung: LÜCKE** in allen drei Teilen. **Regeln (Technik, Dirigent):** (1) Behauptung 21 im lebenden Kopf wird auf das Gemessene begrenzt: alle vier Leser sperren fail-closed auf Anwesenheit und Objektform der Marke; den Feldinhalt (`am`, `entscheid` als nichtleere Zeichenketten) misst A17 im Kanon, `EqCopIdentityTest` misst Nichtleere nach `toString()`. Keine Angleichung der vier Leser — von keiner Zusage verlangt; die Marke ist repo-eigen, und A17 hält sie im Kanon. (2) §2.3 des Vertrags nennt für die vier Pflichtfelder den Typ so, wie A17 ihn misst. (3) nichts. Runde 4 ist genau diese zwei Textstellen (Prüfliste E, Nachweis per `git grep`), Kanon auf dem committeten Stand, danach ein frischer Prüfer nach Vorlage A.
