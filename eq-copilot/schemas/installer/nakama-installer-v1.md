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

| Datei | Rolle |
|---|---|
| `eq-copilot/install/nakama-installer-v1.json` | die Auslieferung selbst (versioniert) |
| `eq-copilot/install/Install-Nakama.ps1` | der einzige Ausführende (versioniert) |
| `tools/eq-copilot/pruefe_installer_manifest.py` | Kanon-Bein **A17** + Release-Schritt `--hashen` |
| `eq-copilot/install/install-ergebnis.json` | Ergebnis des letzten Laufs (Maschinenartefakt, nicht versioniert) |
| `eq-copilot/install/rueckweg/` | die gesicherten Vorgängerdateien (Maschinenartefakt) |

---

## 1. Die eine Regel: keine zweite Identität

Das Manifest trägt **weder Produktnamen noch Viercodes noch Class-IDs**. Es
nennt nur `ziel_id` aus `eq-copilot/identity/plugin-identities-v1.json`; Skript
und Prüfbein schlagen alles Übrige dort nach.

Das ist dieselbe Regel, die S9 Abschnitt 1 im Bauskript durchgesetzt hat
(NAK-52). Ein Installer ist die andere Hälfte derselben Gefahr: ein Paket, das
Bundlenamen selbst führte, könnte eine Datei an einen Pfad legen, dessen Name
nirgends eingefroren ist — und FL lädt Bundles über den Pfad.

Weil ein Dateipfad den Bundlenamen zwangsläufig enthält, ist die Regel
zweiteilig, und beide Hälften sind nötig:

1. **Nachrechnen statt lesen.** Der Quellpfad eines VST3-Artefakts ist
   vollständig bestimmt durch `cmake_ziel` und den Bundlenamen der
   Identitätsdatei:
   `eq-copilot/build/plugin/{cmake_ziel}_artefacts/Release/VST3/{bundle}/Contents/x86_64-win/{bundle}`.
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
| `artefakte[].art` | `vst3` oder `broker` |
| `artefakte[].ziel_id` | nur bei `vst3`: ID aus der Identitätsdatei |
| `artefakte[].cmake_ziel` | nur bei `vst3`: das CMake-Ziel, aus dem der Pfad entsteht |
| `artefakte[].quelle` | repo-relativer Pfad des gebauten Artefakts |
| `artefakte[].sha256` | `null` oder SHA-256 in Großbuchstaben |
| `hashes_erzeugt_am` | UTC-Zeitpunkt des `--hashen`-Laufs |
| `rueckweg.*` | siehe §5 |

**Jedes Ziel der Identitätsdatei hat genau einen `vst3`-Eintrag.** A17 zählt
`3 vs 3`. Ein viertes Ziel im Identitätsmanifest ohne Installer-Eintrag bringt
das Bein zum Sprechen, statt still ungemessen zu bleiben — dieselbe Zeile wie
in `EqCopIdentityTest`.

## 3. `sha256: null` heißt **nicht ausliefer-bar**

Ein Artefakt ohne festgeschriebenen Hash wurde nie gegen einen Bau eingefroren.
`Install-Nakama.ps1` bricht dann ab, **bevor** es irgendetwas anfasst. Kein
Vorgabewert, keine „nimm einfach, was da liegt"-Kulanz — dieselbe Regel wie im
Identitätsleser (`cmake/NakamaIdentitaet.cmake`), aus demselben Grund: eine
Auslieferung, die nirgends eingefroren ist, kann niemand nachprüfen.

`null` ist deshalb kein Mangel, sondern der ehrliche Normalfall **zwischen**
zwei Releases. A17 meldet ihn als Hinweis, nicht als Fehler, und sagt dazu, dass
das Paket in diesem Zustand nicht installierbar ist.

Gefüllt wird ausschließlich per
`py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen`, unmittelbar
nach einem grünen Kanon-Lauf. Fehlt dabei auch nur ein Artefakt, schreibt der
Schritt gar nichts: ein halb gefülltes Manifest wäre eine Auslieferung, die nur
zur Hälfte eingefroren ist.

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

1. Vor jedem Tausch wandert die **vorgefundene** Datei nach
   `eq-copilot/install/rueckweg/<stempel>-<kennung>.bak`.
2. Pfad, Hash davor, Hash danach und Sicherungsdatei landen in
   `install-ergebnis.json`.
3. `Install-Nakama.ps1 -Rueckweg` liest genau das: gesicherte Datei
   zurückkopieren — oder **entfernen**, wenn vorher keine da war (die neuen
   Bundles waren nie installiert).

Ein Installer, der seinen Rückweg aus einer Liste historischer Bundles zöge,
könnte nur Stände zurückgeben, die vorher jemand aufgeschrieben hat.

### 5.1 Der NAK-41-Riegel

Ein FL-Projekt, das dieser Bau als `NakamaState{schema=2}` gespeichert hat,
verliert im 16.08.-Build **still** seine Messpunkt-Identität: jener Build kennt
nur `EqCopilotState{schema=1}`, sieht einen fremden Baumtyp und ignoriert ihn —
ohne Fehlermeldung.

Der Rückweg schlägt den Hash der gesicherten Datei in `rueckweg.bekannte_staende`
nach und vergleicht deren `state_schema` mit dem `state_schema` des Ziels in der
Identitätsdatei. Ist es kleiner **oder unbekannt**, verweigert das Skript und
verlangt `-Erzwingen` — sichtbar, mit Aufzählung der betroffenen Bundles.

**Unbekannt zählt wie älter.** Schweigen ist hier kein Freibrief: eine Datei, die
niemand einordnen kann, ist genau der Fall, in dem der stille Verlust passiert.

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
