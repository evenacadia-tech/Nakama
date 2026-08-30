# Vertrag: Installer-Manifest `nakama.installer/v1`

> **Was das hier ist:** die Beschreibung EINER Auslieferung — welche Artefakte
> zusammengehören, wohin sie kommen, wie ihre Echtheit geprüft wird und wie der
> Rückweg aussieht. Wie die VST3-Identität (SONDE-001) und das State-Schema
> (SONDE-006) ist es ein **Vertrag**: eine Änderung an Schemaname, Feldbedeutung
> oder Riegel ist eine Versionierung mit Beleg, kein Edit.
>
> Ticket `SONDE-007b` (S9, Abschnitt 3). Quelle des Auftrags:
> `docs/FL-Nakama-Sonden-Design-Entwurf.md` §55 (P1-Lieferumfang) — *„Installer-Manifest
> für alle drei Bundles plus Broker mit Hash-/Signaturprüfung, Repair/Uninstall
> und v2-Kompatibilitätslistener"* — und §53.9 (Installationspfad und
> Betriebssystemschutz sind maßgeblich).
>
> **Nachtrag S9b `SONDE-007c`, 28.08.2026:** Der zitierte §55-Wortlaut sagt
> „alle drei Bundles"; seit dem User-Entscheid vom 28.08.2026
> (`design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md`) sind es **zwei**
> — Nakama Gen und Nakama Probeeq. Nakama Suna ist stillgelegt: die Kennung
> bleibt in der Identitätsdatei gesperrt, das Bundle wird nicht mehr gebaut und
> nicht mehr ausgeliefert. Der Vertrag bekommt dafür das Feld
> `stillgelegte_ziele` (§2.3). Das Zitat bleibt im Wortlaut stehen, weil es
> zitiert ist; der Stand steht hier.

| Datei | Rolle |
|---|---|
| `eq-copilot/install/nakama-installer-v1.json` | die Auslieferung selbst (versioniert) |
| `eq-copilot/install/Install-Nakama.ps1` | der einzige Ausführende (versioniert) |
| `eq-copilot/install/NakamaOrdnerHash.ps1` | der Ordner-Hash für PowerShell — **eine** Funktion, zwei Aufrufer (versioniert) |
| `tools/eq-copilot/pruefe_installer_manifest.py` | Kanon-Bein **A17** + Auslieferungsprüfung `--release` + Release-Schritt `--hashen` + die Python-Hälfte des Ordner-Hashes |
| `tools/eq-copilot/pruefe_installer_gegenpfad.py` | Kanon-Bein **A18** — fährt den Gegenpfad in einer Sandbox (§5, installiert nichts) |
| `eq-copilot/install/install-ergebnis.json` | Ergebnis des letzten Laufs (Maschinenartefakt, nicht versioniert) |
| `eq-copilot/install/rueckweg/` | die gesicherten Vorgängerstände (Maschinenartefakt) |

---

## 1. Die eine Regel: keine zweite Identität

Das Manifest trägt **weder Produktnamen noch Viercodes noch Class-IDs**. Es
nennt nur `ziel_id` aus `eq-copilot/identity/plugin-identities-v1.json`; Skript
und Prüfbein schlagen alles Übrige dort nach.

> **Nachtrag 28.08.2026 (S9b `SONDE-007c`):** Dieser Satz stand seit dem 23.08.
> hier, aber `A17` maß nur zwei seiner drei Hälften — Viercodes und Class-IDs.
> **Produkt- und Bundlenamen prüfte niemand.** Aufgefallen ist es beim
> Schreiben des Stilllegungsblocks (§2.3): dessen Fließtext hätte den
> Bundlenamen beiläufig ein zweites Mal festgeschrieben, und keine Regel hätte
> das gesehen. `r_keine_identitaetsliterale` deckt sie jetzt mit ab, mit zwei
> eigenen Gegenproben. **Zwei Ausnahmen, beide begründet:** `quelle` (dort
> steckt der Bundlename zwangsläufig im Pfad — genau deshalb rechnet
> `r_quellpfade_nachgerechnet` ihn nach) und `hersteller.name` (er ist
> Bestandteil der von §4 festgelegten geschützten Pfade und geht, anders als
> `hersteller.code`, in keine Class-ID ein).

Das ist dieselbe Regel, die S9 Abschnitt 1 im Bauskript durchgesetzt hat
(NAK-52). Ein Installer ist die andere Hälfte derselben Gefahr: ein Paket, das
Bundlenamen selbst führte, könnte eine Datei an einen Pfad legen, dessen Name
nirgends eingefroren ist — und FL lädt Bundles über den Pfad.

Weil ein Dateipfad den Bundlenamen zwangsläufig enthält, ist die Regel
zweiteilig, und beide Hälften sind nötig:

1. **Nachrechnen statt lesen.** Der Quellpfad eines VST3-Artefakts ist
   vollständig bestimmt durch `cmake_ziel` und den Bundlenamen der
   Identitätsdatei:
   `eq-copilot/build/plugin/{cmake_ziel}_artefacts/Release/VST3/{bundle}`.
   A17 bildet ihn und vergleicht.
2. **Keine Codes, keine CIDs.** Außerhalb der `quelle`-Felder darf kein
   Viercode und keine Class-ID vorkommen.

Einzeln wäre jede Hälfte löchrig; zusammen ist die Aussage dicht.

## 2. Felder

| Feld | Bedeutung |
|---|---|
| `schema` | `nakama.installer/v1` — fremder Wert ⇒ das Skript verweigert |
| `identitaetsquelle` | Pfad (repo-relativ) auf die eingefrorene Identitätsdatei |
| `ziele.vst3_verzeichnis` | fest `C:/Program Files/Common Files/VST3` — FL scannt VST3 nur dort (Termine A/B, 22.08.2026) |
| `ziele.broker_verzeichnis` | unter `C:/Program Files/` — **Sicherheitsentscheid**, siehe §4 |
| `signatur.verfahren` | `sha256-manifest` |
| `signatur.authenticode_thumbprint` | `null` = wird nicht geprüft; dann ist `warum_null` Pflicht |
| `artefakte[].art` | **genau** `vst3` oder `broker` — ein dritter Wert ist ein Fehler, kein Zukunftsfeld (§2.2) |
| `artefakte[].ziel_id` | nur bei `vst3`: ID aus der Identitätsdatei |
| `artefakte[].cmake_ziel` | nur bei `vst3`: das CMake-Ziel, aus dem der Pfad entsteht |
| `artefakte[].quelle` | repo-relativer Pfad des gebauten Artefakts. Bei `vst3` der **Bundle-Ordner**, bei `broker` die Datei (§2.1) |
| `artefakte[].sha256` | `null` oder SHA-256 in Großbuchstaben. Bei `vst3` der **Ordner-Hash** nach §2.1, bei `broker` der Dateihash |
| `stillgelegte_ziele[]` | Ziele der Identitätsdatei, die **nicht mehr** ausgeliefert werden (§2.3). Pflichtfelder je Eintrag: `ziel_id`, `seit`, `warum`, `umgang_mit_altbestand`, `kennung_bleibt`; die letzten vier je eine **nichtleere Zeichenkette** (Typ je Feld in §2.3) |
| `hashes_erzeugt_am` | UTC-Zeitpunkt des `--hashen`-Laufs |
| `rueckweg.*` | siehe §5 |

**Jedes *aktive* Ziel der Identitätsdatei hat genau einen `vst3`-Eintrag, jedes
stillgelegte keinen.** „Aktiv" heißt: ohne das Feld `stillgelegt` (§2.3). A17
meldet den Vergleich als `<Artefakte> vs <aktive> aktiv (<Kennungen> gesamt)`
und misst die Stilllegung in **beide** Richtungen — keine feste Zahl, sondern
die Beziehung. Ein neues Ziel im Identitätsmanifest ohne Installer-Eintrag
bringt das Bein deshalb zum Sprechen, statt still ungemessen zu bleiben —
dieselbe Zeile wie in `EqCopIdentityTest`.

> **Warum hier keine Zahl mehr steht** (Nacharbeit Runde 1, 29.08.2026,
> T2-Befund P1 Nr. 3): Bis dahin stand an dieser Stelle „A17 zählt `3 vs 3`".
> Das war seit der Stilllegung von `passive-probe` (S9b/SONDE-007c) falsch —
> A17 misst `2 vs 2 aktiv (3 Kennungen gesamt)` — und widersprach §2.3
> derselben Datei. Eine Zahl im Vertrag veraltet bei jeder Änderung des
> Lieferumfangs; die Beziehung nicht.

### 2.1 Die Auslieferungseinheit ist der Bundle-ORDNER

> Entschieden am 23.08.2026 in der S9-Nacharbeit (T2-Befund T2-5). Eine
> **Technikentscheidung** dieses Tickets, kein User-Entscheid.

Bis dahin adressierte die Pfadformel die innere Binärdatei
(`…/{bundle}/Contents/x86_64-win/{bundle}`). Ein VST3-Bundle ist unter Windows
aber ein **Ordner**: neben der Binärdatei liegt `Contents/Resources/moduleinfo.json`.
Gemessen an den gebauten Bundles — je zwei Dateien, 1 090 B und 5 598 208 B —
und am installierten Stand: dessen `moduleinfo.json` sagt `"Version": "0.1.0"`,
der heutige Bau `"0.3.0"`.

Die alte Formel hatte vier Folgen, alle aus derselben Wurzel:

- eine **Erstinstallation** von Suna/Probeeq erzeugte ein Bundle **ohne**
  `moduleinfo.json`,
- beim Main-Bundle bliebe das `moduleinfo.json` vom 16.08. liegen — Metadaten
  „0.1.0" neben einer 0.3.0-Binärdatei,
- `-Pruefen` meldete dazu `aktuell`, weil es nur die Binärdatei hasht,
- `-Rueckweg` einer Erstinstallation entfernte die Datei und ließ die selbst
  angelegten Ordner als leeres `.vst3`-Gehäuse stehen.

**Der Ordner ist damit die Einheit** — beim Hashen, beim Sichern, beim
Tauschen, beim Prüfen und beim Rückweg. Was ein Host von einem Bundle liest,
ist der Ordner; was ausgeliefert und verifiziert wird, muss dasselbe sein.

#### Ordner-Hash v1 — die kanonische Form

Ein SHA-256 über einen Ordner ist keine Selbstverständlichkeit; er braucht eine
Definition, an die sich **zwei Implementierungen** halten (Python schreibt ihn
mit `--hashen`, PowerShell prüft ihn beim Installieren). Die Form ist:

```
Für jede DATEI unterhalb des Ordners, rekursiv (Verzeichnisse selbst zählen nicht):
    relpfad  := Pfad relativ zum Ordner, Trennzeichen '/', kein führendes './'
    zeile    := <SHA-256 der Datei, 64 Hex GROSS> <ein Leerzeichen> <relpfad> <LF>

Alle Zeilen aufsteigend nach relpfad sortiert, aneinandergehängt, UTF-8;
Ordner-Hash := SHA-256 über diesen Bytestrom, 64 Hex GROSS.
```

Drei Festlegungen, jede mit ihrem Grund:

- **Nur Dateien, keine Verzeichniseinträge.** Ein leeres Verzeichnis trägt
  nichts, was ein Host lädt — und zwei Implementierungen müssten sich sonst
  darauf einigen, ob `Contents/` mitzählt. Dateien allein ist eindeutig.
  Ein Ordner ohne jede Datei ergibt damit den SHA-256 des leeren Bytestroms;
  `--hashen` verweigert ihn trotzdem ausdrücklich, weil ein Bundle ohne Datei
  keine Auslieferung ist.
- **Relative Pfade müssen ASCII sein — sonst Abbruch, nicht irgendein Hash.**
  PowerShell sortiert Zeichenketten ordinal nach UTF-16-Code-Einheiten, Python
  byteweise nach UTF-8; oberhalb der BMP laufen diese Ordnungen auseinander.
  Statt zu hoffen, dass der Fall nie eintritt, wird der Bereich so weit
  eingeengt, dass zwei Implementierungen **nicht** uneins sein können — und die
  Einengung wird erzwungen. Beide Hälften brechen ab, wenn ein Pfad sie verlässt.
- **Ein Reparse-Punkt (Junction, Symlink) im Ordner bricht ab.** Er ist
  dieselbe Falle wie ein Nicht-ASCII-Pfad, nur unauffälliger: PowerShells
  `-Recurse` steigt in ein Verzeichnis-Reparse **nicht** hinab, Pythons
  `rglob` schon — dieselbe Definition ergäbe zwei Hashes. VST3-Bundles
  enthalten keine; stünde je einer drin, sagen beide Hälften es, statt
  verschiedene Zahlen zu liefern. Auch versteckte Dateien zählen mit
  (PowerShell braucht dafür `-Force`), damit „unsichtbar" nicht „nicht
  ausgeliefert" bedeutet.

Dass beide Implementierungen wirklich denselben Wert bilden, misst **A17
selbst** an einem synthetischen Ordner (verschachtelt, mit Leerzeichen im
Namen, mit Groß-/Kleinschreibung an der Sortiergrenze) — nicht an einem
gebauten Bundle, damit die Kreuzprobe auch ohne Bau läuft. Fehlt `pwsh`, ist
das ein **Fehler** des Beins und keine stille Auslassung: eine Kreuzprobe, die
nicht lief, hat nichts bewiesen.

### 2.2 `art` ist eine geschlossene Menge

`vst3` oder `broker`, nichts sonst. Der Grund ist der Zielpfad: `Ziel-Pfad()`
im Skript behandelt alles, was nicht `vst3` ist, im **Broker-Zweig** — ein
Tippfehler beim Erweitern legte die Datei also unter `ziele.broker_verzeichnis`,
in genau den Pfad, den §4 schützt, weil er ab `SONDE-010` ein Spawn-Ziel wird.
A17 hat dafür eine eigene Regel; sie fiel vorher niemandem auf, weil `_vst3()`
auf `art == "vst3"` filtert und die Broker-Regel nur `broker` zählt — ein
drittes Wort fällt durch beide Siebe (gemessen: 0 von 12 Regeln sahen es).

### 2.3 Stillgelegte Ziele (S9b `SONDE-007c`, 28.08.2026)

Ein Ziel kann seine **Kennung behalten** und trotzdem aus der Auslieferung
verschwinden. Das sind zwei verschiedene Aussagen, und sie dürfen nicht
zusammenfallen:

- Die **Kennung** (Viercode, beide Class-IDs, Bundlename) bleibt in
  `identity/plugin-identities-v1.json` eingefroren und gesperrt. Löschte man
  sie, wäre `NkPr` wieder frei — und ein späteres Ziel könnte still die
  Class-ID eines Bundles erben, das einmal in FL-Projekten stand.
- Die **Auslieferung** endet: kein CMake-Ziel, kein Bundle, kein
  Artefakteintrag, kein Kanon-Bein.

Die Identitätsdatei trägt dafür das Feld `stillgelegt` am Ziel; dieses Manifest
trägt den Gegenpart `stillgelegte_ziele`. **A17 misst beide Richtungen:**

| Regel | Wogegen sie schützt |
|---|---|
| jedes Ziel **ohne** `stillgelegt` hat genau einen `vst3`-Eintrag | ein aktives Ziel fällt aus der Auslieferung |
| jedes Ziel **mit** `stillgelegt` steht genau einmal in `stillgelegte_ziele` | eine Stilllegung geschieht **still** |
| ein stillgelegtes Ziel hat **keinen** Artefakteintrag | ein stillgelegtes Bundle wird doch ausgeliefert |
| jeder Eintrag trägt `seit`, `warum`, `umgang_mit_altbestand`, `kennung_bleibt` — je eine nichtleere Zeichenkette | „stillgelegt" ohne Grund und Datum |

Ohne die zweite Zeile bliebe die erste zahnlos: wer nur den Artefakteintrag
löscht, ließe Soll- und Ist-Menge **gemeinsam** schrumpfen, und der
Mengenvergleich bliebe grün.

**Typ je Pflichtfeld.** Genau das, was A17 misst — nicht strenger, nicht laxer:

| Feld | Typ |
|---|---|
| `seit` | nichtleere Zeichenkette |
| `warum` | nichtleere Zeichenkette |
| `umgang_mit_altbestand` | nichtleere Zeichenkette |
| `kennung_bleibt` | nichtleere Zeichenkette — ein Satz über die gesperrte Kennung, **kein** Boolean |

> **Nachtrag 30.08.2026 (S9b `SONDE-007c`, Runde 4, NAK-89):** Diese vier Felder standen bis heute ohne Typ da, während A17 seit Runde 2 (29.08.2026) je Feld eine nichtleere Zeichenkette erzwingt — der Leser war strenger als sein Vertrag. Kein Versionsschritt: die Angabe schreibt den Typ nieder, den `eq-copilot/install/nakama-installer-v1.json` schon führt und A17 schon misst; Feldbedeutung und Riegel bleiben unverändert. Gemessen von `r_stillgelegte_benannt` — `tools/eq-copilot/pruefe_installer_manifest.py:476` (Feldliste `EINTRAG_PFLICHTFELDER`) und `:520-524` (`isinstance(wert, str) and wert.strip()`), Stand `36560b0`. Die eigene Gegenprobe ebenda (`:1008-1014`) fährt je Feld `null`, `[]`, `{}`, `""`, reinen Leerraum und eine Zahl einzeln gegen die Regel.

#### Altbestand: melden, nicht löschen

`umgang_mit_altbestand: "melden-nicht-loeschen"`. Findet
`Install-Nakama.ps1` das Bundle eines stillgelegten Ziels im
Installationsverzeichnis, meldet es **ALTLAST** mit vollem Pfad und dem
Handgriff zum Entfernen — im Installationslauf und in `-Pruefen`. Es entfernt
nichts. Zwei Gründe:

1. **Der Gegenpfad bliebe halb.** Journal und `artefakte` stehen 1:1
   zueinander (§5). Eine Löschung wäre ein vierter Akt, dessen Rückweg das
   Bundle wiederherstellen müsste — sonst verlöre der Rückweg einen Stand,
   statt ihn zurückzugeben. Das ist ein eigener Änderungssatz, kein Nebenzug.
2. **`Common Files\VST3` gehört nicht diesem Installer.** Dort liegt jeder
   Hersteller. Ein Verzeichnis auszuräumen, das man mitbenutzt, ist genau die
   Art unbestellter Verarbeitung, die das Grundgesetz ausschließt.

Ausgeführt statt zugesagt: A18 legt das stillgelegte Bundle in der Sandbox
wirklich hin, misst die Meldung — und misst, dass das Bundle danach
**bytegleich** noch da ist (Block `[3b]`).

**Warum das v1 bleibt:** aus demselben Grund wie in §5.2 — jedes `sha256`
steht auf `null`, es wurde unter v1 nie etwas ausgeliefert, also gibt es kein
altes Manifest, das nach der alten Feldbedeutung gelesen werden müsste. Das
Feld ist additiv, seine Pflicht greift nur, wenn ein Ziel überhaupt
stillgelegt ist.

---

## 3. `sha256: null` heißt **nicht ausliefer-bar**

Ein Artefakt ohne festgeschriebenen Hash wurde nie gegen einen Bau eingefroren.
`Install-Nakama.ps1` bricht dann ab, **bevor** es irgendetwas anfasst. Kein
Vorgabewert, keine „nimm einfach, was da liegt"-Kulanz — dieselbe Regel wie im
Identitätsleser (`cmake/NakamaIdentitaet.cmake`), aus demselben Grund: eine
Auslieferung, die nirgends eingefroren ist, kann niemand nachprüfen.

`null` ist deshalb kein Mangel, sondern der ehrliche Normalfall **zwischen**
zwei Releases. A17 meldet ihn im Kanon als Hinweis, nicht als Fehler, und sagt
dazu, dass das Paket in diesem Zustand nicht installierbar ist; unter
`--release` ist derselbe Zustand ein Fehler, denn dort wird ausgeliefert.

Gefüllt wird ausschließlich per
`py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen`, unmittelbar
nach einem grünen Kanon-Lauf. Fehlt dabei auch nur ein Artefakt, schreibt der
Schritt gar nichts: ein halb gefülltes Manifest wäre eine Auslieferung, die nur
zur Hälfte eingefroren ist.

### 3.1 Drei Härtegrade für denselben Hash (NAK-94, 29.08.2026)

Ein grüner Kanon-Lauf sagt seit NAK-94 **nicht** mehr, dass der gebaute Stand
dem festgeschriebenen gleicht. Der Grund ist mechanisch: der Kanon baut den
Kern vor jeder Messung neu, der Linker erzeugt dabei andere Bundlebytes auch
ohne jede Quelltextänderung — ein Riegel, der danach immer fällt, unterscheidet
nichts mehr. Dieselbe Zahl beantwortet je nach Aufrufer eine andere Frage:

| Aufrufer | Frage | Abweichung |
|---|---|---|
| A17 im Kanon (ohne Flag) | welcher Bau liegt gerade da? | Hinweis mit beiden Kurz-Hashes |
| A17 `--release` | frieren wir genau diesen Stand ein? | **Fehler**, Exit 2 |
| `Install-Nakama.ps1` Riegel 2 | darf das kopiert werden? | **Abbruch**, bevor irgendetwas kopiert wird |

Nicht weich wird in **keinem** Modus: ein festgeschriebenes Artefakt, das gar
nicht vorliegt, und ein Ordner-Hash, der sich nicht bilden lässt. Beides kann
ein Relink nicht verursachen.

Zusätzlich berichtet A17 in `[4b]`, ob der **installierte** Stand aus
`install-ergebnis.json` dem heutigen Manifest entspricht. Dieser Block urteilt
nie: Installieren ist ein bewusster Admin-Handgriff des Users, kein Bestandteil
eines Kanonlaufs.

## 4. Warum der Broker unter `Program Files` liegt

§53.5: *„Ausschließlich ein positiv klassifiziertes Main mit geöffnetem Editor
darf den installierten Broker starten."* Der Broker ist damit ein **Spawn-Ziel**.

Ein per-User-Pfad (`%LOCALAPPDATA%`) ist ohne Rechteerhöhung beschreibbar — jeder
Nutzerprozess könnte die Datei austauschen, die ein Plugin später startet. Der
Hash-Riegel des Manifests schützt die *Installation*, nicht den späteren Start.
Deshalb dieselbe geschützte Ebene wie die Bundles; §53.9 nennt Installationspfad
und Betriebssystemschutz ausdrücklich als maßgeblich.

Das ist eine Technikentscheidung dieses Tickets, kein User-Entscheid.

## 5. Gegenpfad: installieren ↔ Rückweg

Hausinvariante: beide Hälften im selben Änderungssatz. Strategie
`vor-dem-tausch-sichern`:

1. Vor jedem Tausch wandert der **vorgefundene** Stand nach
   `eq-copilot/install/rueckweg/<stempel>-<kennung>` — bei `vst3` als
   **Ordnerkopie** (Suffix `.bundle`), bei `broker` als Datei (Suffix `.bak`).
2. Pfad, Hash davor, Hash danach, Sicherung, `art` und die Liste der vom
   Skript **selbst angelegten Verzeichnisse** landen in `install-ergebnis.json`.
3. `Install-Nakama.ps1 -Rueckweg` liest genau das: gesicherten Stand
   zurückstellen — oder **entfernen**, wenn vorher keiner da war (Nakama
   Probeeq war nie installiert). Ein stillgelegtes Ziel steht in keinem
   Journal und wird vom Rückweg deshalb nie angefasst — es wurde ja auch
   nicht installiert (§2.3).

Ein Installer, der seinen Rückweg aus einer Liste historischer Bundles zöge,
könnte nur Stände zurückgeben, die vorher jemand aufgeschrieben hat.

**Der Rückweg stellt den Vorzustand her, auch wenn der Vorzustand „kein
Ordner" war** (T2-5 (d), 23.08.2026). Drei Punkte, die das tragen:

- Bei `vst3` wird der **ganze Bundle-Ordner** entfernt, nicht eine Datei
  darin — sonst bliebe ein leeres `.vst3`-Gehäuse in `Common Files\VST3`
  stehen, das jeder Scanner findet und das nichts enthält.
- `erzeugte_ordner` verzeichnet jedes Verzeichnis, das der Installationslauf
  **selbst** angelegt hat (etwa `Program Files\evenacadia\Nakama` für den
  Broker). Der Rückweg entfernt sie in umgekehrter Reihenfolge — **nur wenn
  sie leer sind**. Zwei Bedingungen, keine davon entbehrlich: *selbst
  angelegt* schützt vorgefundene Verzeichnisse (`Common Files\VST3` steht
  praktisch immer schon da und wird darum nie verzeichnet), *leer* schützt
  davor, fremde Dateien mitzunehmen, die inzwischen dort gelandet sind. Das
  Aufräumen darf nie mehr wegnehmen, als es angelegt hat.

**Ein Tausch ersetzt, er mischt nicht.** Ein vorgefundener Bundle-Ordner wird
vor dem Kopieren **entfernt**. `Copy-Item` über einen bestehenden Ordner würde
zusammenführen — eine Datei, die der neue Bau nicht mehr enthält, überlebte
still im installierten Bundle. Genau das ist die Form von T2-5 (b).

### 5.0 Der Gegenpfad wird gefahren, nicht zugesagt

Bis zur S9-Nacharbeit war die zweite Hälfte dieser Invariante **geschrieben,
aber nie ausgeführt** — der T2-Prüfer sagte das selbst (Manifest §5.9: *„Nur
gelesen und gegen den Vertrag gemessen. Weder installiert noch
zurückgerollt."*). Beim ersten wirklichen Lauf fielen zwei echte Fehler
heraus, die dem Lesen entgangen waren: ein Nachschlagen mit leerem Schlüssel
(`$hashtable[$null]` wirft in PowerShell), das den Rückweg **ab der zweiten
Installation** sterben ließ, und ein verweigerter Rückweg, der `status: ERROR`
in die Ergebnisdatei schrieb und damit genau die Quelle zerstörte, aus der die
angebotene Wiederholung mit `-Erzwingen` hätte lesen müssen.

Deshalb läuft der Gegenpfad ab jetzt im Kanon mit (**A18**): das *echte*
`Install-Nakama.ps1` in einer Sandbox unter `%TEMP%`, deren Manifest
`ziele.*` dorthin zeigt. Es wird nichts installiert, es braucht keine
Rechteerhöhung, und das Skript braucht keinen Testschalter. Genau **eine**
Zeile unterscheidet die Sandbox-Kopie vom Original — Riegel 3 („Ruhe",
verweigert solange FL läuft, was in einer Sandbox ohne Host keinen
Gegenstand hat). A18 prüft vorher, dass diese Zeile im Original steht, und
druckt die Ersetzung als Diff mit: ein Riegel darf nicht unbemerkt
verschwinden, nur weil eine Probe ihn umgeht.

### 5.1 Der NAK-41-Riegel

Ein FL-Projekt, das dieser Bau als `NakamaState{schema=2}` gespeichert hat,
verliert im 16.08.-Build **still** seine Messpunkt-Identität: jener Build kennt
nur `EqCopilotState{schema=1}`, sieht einen fremden Baumtyp und ignoriert ihn —
ohne Fehlermeldung.

Der Rückweg schlägt den Hash des gesicherten Standes in `rueckweg.bekannte_staende`
nach und vergleicht dessen `state_schema` mit dem `state_schema` des Ziels in der
Identitätsdatei. Ist es kleiner **oder unbekannt**, verweigert das Skript und
verlangt `-Erzwingen` — sichtbar, mit Aufzählung der betroffenen Bundles.

**Unbekannt zählt wie älter.** Schweigen ist hier kein Freibrief: ein Stand, den
niemand einordnen kann, ist genau der Fall, in dem der stille Verlust passiert.

**Zwei Hasharten, ein Nachschlagen** (23.08.2026, Folge von §2.1). Ein
`bekannte_staende`-Eintrag trägt seit dem Umzug auf den Ordner das Feld
`hash_art`:

| `hash_art` | was der `sha256` beschreibt |
|---|---|
| `ordner` | Ordner-Hash des ganzen Bundles nach §2.1 |
| `datei-innen` | Dateihash **nur** der inneren Binärdatei `Contents/x86_64-win/{bundle}` |

Der Rückweg bildet von einem gesicherten `vst3`-Stand **beide** Hashes und
schlägt beide nach. Das ist keine Kulanz, sondern die einzige ehrliche Form:
die zwei historischen Einträge (Bau vom 16.08. und der Stand davor) wurden
gegen die **Binärdatei** eingefroren, lange bevor der Ordner die Einheit war.
Sie bleiben damit gültig und benennen weiter dieselben Builds. Ein neuer
Eintrag wird als `ordner` geführt. Fällt ein Stand durch beide Nachschlagungen,
gilt er als unbekannt — also als älter.

## 5.2 Warum das v1 bleibt und kein Versionsschritt ist

Hausregel: **Schemas sind Verträge** — eine Änderung an Schemaname,
Feldbedeutung oder Riegel ist eine Versionierung mit Beleg, kein Edit. Die
Änderung vom 23.08. ändert Feldbedeutungen (`quelle`, `sha256`) und trägt ein
neues Feld (`hash_art`). Sie bleibt trotzdem **in v1**, und zwar aus einem
Grund, der nachprüfbar ist statt bequem:

**Unter v1 wurde nie etwas ausgeliefert.** Jedes `sha256` steht auf `null` und
`hashes_erzeugt_am` ebenfalls — und genau dieser Zustand heißt laut §3 „nicht
ausliefer-bar", das Skript bricht darin ab, bevor es irgendetwas anfasst. Es
gibt also kein Paket, keine `install-ergebnis.json` und keinen installierten
Stand, der nach der alten Feldbedeutung gelesen werden müsste. Der Zweck einer
Versionierung — *alte Stände laden weiter* — hat hier **kein Subjekt**.

Alle drei Leser (dieser Vertrag, `A17`, `Install-Nakama.ps1`) liegen in
diesem Repo und ändern sich im selben Änderungssatz. Ein `v2` müsste einen
`v1`-Zweig mitführen, den kein Manifest je genommen haben kann: totes
Migrationsgerüst, das behauptet, einen Fall zu behandeln, den es nicht gibt —
dieselbe Art Lüge, gegen die die Hausregel eigentlich gerichtet ist.

**Wo die Grenze verläuft:** Sobald ein `--hashen`-Lauf gefüllte Hashes und
`hashes_erzeugt_am` **committet**, ist v1 ausgeliefert. Von da an ist jede
Änderung an der Pfadformel, an der Bedeutung von `sha256` oder an §2.1 ein
**Versionsschritt** (`nakama.installer/v2`) mit beiden Ständen nebeneinander.
Das ist kein Vorsatz, sondern an einem Feld ablesbar: `hashes_erzeugt_am`
sagt, ob dieser Satz schon gilt.

## 6. Was dieser Vertrag ausdrücklich NICHT abdeckt

- **Repair** über die Diagnose hinaus. `-Pruefen` vergleicht den installierten
  Stand gegen das Manifest und benennt `aktuell` / `ABWEICHEND` /
  `nicht installiert`; ein eigener Reparaturdienst ist P9.
- **Update-Kanal, atomarer Austausch, Signaturkette** — Distribution, P9.
- **v2-Kompatibilitätslistener** (steht im P1-Lieferumfang): gehört zum Broker
  und kommt mit `SONDE-010`. Hier steht nichts darüber, weil es dazu nichts zu
  sagen gibt.
- **Echte Signaturprüfung.** Es gibt kein Codesigning-Zertifikat für evenacadia.
  Trägt `authenticode_thumbprint` einmal einen Wert, prüft das Skript zusätzlich
  Authenticode. Bis dahin sagt es beim Lauf ausdrücklich, dass es *nicht* prüft —
  statt eine Prüfung zu behaupten, die es nicht gibt.
- **Die Installation selbst.** Sie bleibt ein Klick des Users mit
  Rechteerhöhung (NAK-32); keine Automatik fährt dieses Skript.
