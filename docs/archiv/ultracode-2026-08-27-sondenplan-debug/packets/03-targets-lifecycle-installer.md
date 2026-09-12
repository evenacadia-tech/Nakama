# Paket 03 — Targets, Lifecycle und Installer

Stand: 2026-08-27, Codeprüfung plus scoped Abschlussbehebung. Geändert wurden
die Broker-Persistenz in `aggregat.rs`/`bindung.rs` samt gemeinsamer neuer
`persistenz.rs`, `Install-Nakama.ps1`, Installer-Manifest, A17, A18 und dieses
Paketartefakt. Installerverhalten lief ausschließlich in der kanonischen
`%TEMP%`-Sandbox mit einer Kopie des echten Skripts; weder Installation noch
Deinstallation auf dem System fanden statt.

## Ergebnis in einem Satz

Der heutige **Vor-Umschnitt mit drei Bundles** ist in CMake, gebauter Identität,
Bundle-State-Matrix, Neutralität und dem derzeit rein verbindenden Pipe-Pfad
konsistent. Das Ausgangsaudit fand acht Installer-/Rollback-Gruppen. Nach der
abschließenden Integration sind die **acht Codegruppen geschlossen**; die vier
zuletzt offenen Restgruppen besitzen jetzt jeweils eine explizite
adversariale Gegenprobe. Offen bleiben nur die unten getrennt benannten
Produkt-/Auslieferungsbelege. Zusätzlich sind Bindungs- und
Aggregatsnapshot-Persistenz jetzt haltbar, wiederholbar und kollisionsfest.
Die Produktentscheidung vom 27.08. ist damit nicht als implementiert behauptet;
der geprüfte Stand bleibt `EQ-Copilot.vst3`, `Nakama Suna.vst3` und
`Nakama Probeeq.vst3`.

## Abschluss der vier letzten Codereste

### R03-A — geschlossen: geschützte Transaktionsautorität

`install-ergebnis.json` ist nur noch Diagnose. Der erhöhte Rückweg liest
ausschließlich den atomaren Spiegel
`C:/Program Files/evenacadia/.nakama-installer/aktive-transaktion.json`, dessen
Pfad aus der fest verdrahteten Broker-Produktpolicy abgeleitet wird. Der Spiegel
friert Transaktions-ID, Vorhashes und NAK-41-`bekannte_staende` beim
Installationsmoment ein. Auch die tatsächlichen Sicherungsbytes liegen direkt
unter dem geschützten `backups/<transaktions-id>/`-Baum; das Repo-`rueckweg`
wird weder geschrieben noch als Restorequelle gelesen.

Belegt in A18:

- manipuliertes öffentliches Journal ist keine Autorität;
- öffentliches Journal ohne geschützten Anker verweigert;
- manipuliertes Backup plus passend manipulierter öffentlicher Vorhash
  verweigert vor Zielmutation;
- nachträglich ergänzte `bekannte_staende` umgehen NAK-41 nicht.

### R03-B — geschlossen: harter Abbruch in Hin- und Rückweg

`VORBEREITET`, `KOMPENSATION`, `ERROR_TEILSTAND` und `RUECKWEG_AKTIV`
unterscheiden die gefährlichen Fenster. Nur ein geschützt belegter,
`mutation_begonnen` und noch nicht abgeschlossener Eintrag darf einen sonst
unbekannten Zielhash als eigenen Teilstand behandeln. Der inverse Pfad
journalisiert `rollback_abgeschlossen` nach jedem Ziel. Ein neuer
Installationslauf verweigert alle vier offenen Stati, statt den Recovery-Plan
zu überschreiben. `OK` und vollständig kompensierte Endstände werden erst nach
vollständiger neuer Sicherung explizit superseded; alte Backup-Generationen
werden danach begrenzt bereinigt.

A18 baut sowohl ein partiell kopiertes erstes Bundle unter `VORBEREITET` als
auch einen partiellen Restore unter `RUECKWEG_AKTIV`; beide Läufe enden mit
Exit 0 und bytegleichem Vorzustand.

### R03-C — geschlossen: kanonische Ziel- und Quellpolicy

Der Runtime-Ausführer bindet VST3 und Broker exakt an die per Windows-
Known-Folder abgeleiteten Produktziele. A17 normalisiert mit Windows-Semantik,
verlangt das exakte Brokerziel, beschränkt `cmake_ziel` auf ein einzelnes
CMake-Segment und containert auch den mutierenden `--hashen`-Pfad. Die früher
grünen Mutationen `Program Files/../Temp` und
`cmake_ziel=../../../../outside` fallen nun explizit.

### R03-D — geschlossen: Identitäts- und State-Vertrag

A17 verlangt genau die drei heutigen, kollisionsfreien Bundleblätter und
liest `kRootSchema` aus `NakamaState.cpp`; Identity-`state_schema` muss
typstreng `int` und exakt `2` sein. Der Ausführer wiederholt Kollision und
Schema-2-Riegel zur Laufzeit. Bundlekollision sowie `1`, Text und fehlendes
Schema besitzen einzelne negative Gegenproben.

### Abschlussbelege

- `py -3.13 -m py_compile` für A17/A18 — Exit 0;
- `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` — **38 ok,
  0 Fehler**, plus ehrlicher Hinweis auf vier Null-Hashes;
- `py -3.13 tools/eq-copilot/pruefe_installer_gegenpfad.py` — **78 ok,
  0 Fehler**, ausschließlich `%TEMP%`;
- `cargo clippy --all-targets -- -D warnings` im Broker — Exit 0;
- `cargo test bindung::tests` — **5 ok**, und
  `cargo test aggregat::tests` — **7 ok**; die beiden Parallelproben liefen
  zusätzlich je zehnmal grün;
- voller `cargo test` auf dem abschließend integrierten Brokerstand —
  **88 Unit- plus 8 Cross-Language-Tests grün**;
- echter `Install-Nakama.ps1 -Pruefen`-Pfad — erwarteter Exit 1 vor Mutation,
  weil alle vier Release-Hashes weiterhin ehrlich `null` sind;
- `git diff --check` für die scoped Implementierung — keine Fehler.

## Nachtrag: haltbare Broker-Dateien und Installer-Crashvertrag

### B03-01 — geschlossen: gemeinsamer persistenter Broker-Write

`broker/src/persistenz.rs:31-175` legt Tempdateien mit `create_new`, PID plus
UUID im Zielordner an, schreibt vollständig und ruft `sync_all` vor der
Veröffentlichung. Bindungen verwenden atomaren Replace; Windows nutzt
`MoveFileExW(MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)` mit eng
begrenztem Retry auf konkurrierende Sharing-Fehler. Snapshots verwenden einen
No-Replace-Pfad; eine Namenskollision liefert ein neues UUID-Suffix und kann
kein vorhandenes Snapshotbyte überschreiben.

Belegt sind wiederholtes Ersetzen, zwölf parallele Bindungsschreiber, zwei und
zwölf Snapshots derselben Millisekunde sowie fremde alte Temp-Restdateien
(`bindung.rs:95-164`, `aggregat.rs:334-414`). Nach jedem Paralleltest bleibt
keine eigene Tempdatei zurück.

### B03-02 — geschlossen: haltbare Installer-Journale

`Schreibe-JsonAtomar` nutzt `FileMode.CreateNew`, UTF-8 ohne BOM,
`FileStream.Flush(true)` und `File.Replace` im selben Ordner
(`Install-Nakama.ps1:139-178`). Der mutationsfähige Lock liegt ebenfalls im
geschützten Transaktionsbaum und serialisiert auch Starts aus verschiedenen
Repo-Kopien (`:492-508`); `-Pruefen` erzeugt keinen autoritativen Repo-Lock.
A18 belegt zweite Veröffentlichung, fehlende Temp-Reste, den gehaltenen
geschützten Lock und die Trennung zwischen Repo-Diagnose und geschütztem
Anker/Backupbaum.

### Verbleibend, aber keine offenen Codekandidaten dieses Pakets

- vier Null-Artefakthashes und kein Codesigning-Zertifikat: bewusst noch nicht
  auslieferbar;
- Repair, Uninstall und v2-Listener: weiterhin eigener §55-/P9-Lieferumfang;
- kein frischer Drei-Bundle-Scan/Load in FL Studio: Gate 1 bleibt extern offen;
- der 27.08.-Umschnitt ist nicht als bereits implementiert behauptet.

## Zwischenrevalidierung vor den Abschlussfixes (historischer Stand)

### Status der acht Ausgangsbefunde

| Befund | Aktueller Status | Beleg am Integrationsstand |
|---|---|---|
| P03-01 Journal als freier Rollback-Plan | **geschlossen für den ursprünglichen Journal-Angriff** | Schema/Status und vollständige Neuableitung von Art, ID, Ziel und Installationshash aus Manifest/Identität (`Install-Nakama.ps1:380-415`); Sicherung und Löschordner sind auf feste Wurzeln bzw. den exakt ableitbaren Ziel-Elternordner begrenzt (`:417-456`). A18 verwirft fremdes Ziel, fremden Löschpfad und Sicherung außerhalb der Wurzel. Die noch unauthentisierte Manifest-/Paketgrenze steht separat unter Produktresten. |
| P03-02 ungeprüfte Sicherung | **teilweise offen** | Reiner Backup-Drift, innerer VST3-Hash und Restore-Nachhash werden jetzt geprüft (`:417-438`, `:535-545`); A18 belegt den Abbruch vor Mutation. Hash und Backup sind jedoch weiterhin gemeinsam durch das Benutzer-Journal autorisiert. Die gefahrene Gegenprobe ersetzte eine Broker-`.bak`, aktualisierte nur `vorher_sha256` im Journal und erhielt `RUECKWEG OK`; der Broker-Zielstand enthielt danach exakt die manipulierten Bytes. |
| P03-03 Teilinstallation ohne Journal/Kompensation | **teilweise offen** | Alle Sicherungen entstehen vor der ersten Zielmutation, `VORBEREITET` wird haltbar geschrieben und gefangene Fehler werden rückwärts kompensiert (`:655-716`, `:718-823`). A18 belegt einen Fehler nach drei Tauschakten mit `ERROR_RUECKGEROLLT` und bytegleichem Vorzustand. Ein harter Prozessabbruch mitten in `Kopiere-Stand` bleibt festgefahren: Zielhash ist dann weder Vor- noch Installationshash und der Rückweg bricht trotz `mutation_begonnen=true` ab (`:458-467`). Die Temp-Probe ließ den Teilstand reproduzierbar bestehen. |
| P03-04 fehlendes Pfad-Containment / `--hashen`-Bypass | **teilweise offen** | Der Ausführer kanonisiert Repoquellen, Zielblätter und Sicherungen und verweigert Reparsepunkte (`:98-136`, `:270-294`, `:582-608`). `--hashen` führt jetzt die Regeln aus (`pruefe_installer_manifest.py:526-541`). Die Regeln selbst akzeptieren aber weiterhin Brokerziel `C:/Program Files/../Temp/Nakama` (`:263-274`) und ein durch `cmake_ziel=../../../../outside` konsistent konstruiertes Quell-Traversal (`:196-214`). Beide aktuellen In-Memory-Proben blieben ohne fallende Regel. |
| P03-05 offene Identitätsinvarianten | **teilweise offen** | Der Ausführer verwirft doppelte IDs, unsichere/doppelte Bundles, nichtpositive bzw. nichtganzzahlige `state_schema` und nicht eindeutige aufgelöste Ziele (`Install-Nakama.ps1:211-229`, `:260-268`, `:369-378`). A17 akzeptiert weiterhin eine Bundle-Zielkollision sowie fehlendes, textuelles oder auf `1` abgesenktes Identity-`state_schema`. Vor allem ist die positive Zahl nicht an das tatsächliche `kRootSchema = 2` gebunden (`plugin/state/NakamaState.cpp:32`); damit kann NAK-41 trotz Schema-2-Writer abgeschwächt werden. |
| P03-06 fehlende Serialisierung | **geschlossen** | Exklusiver `FileStream` mit `FileShare.None`/`DeleteOnClose`, atomarer Tempdatei-Write und UUID-Sicherungsnamen (`Install-Nakama.ps1:138-147`, `:168-179`, `:664-680`). |
| P03-07 Rollback überschreibt Zieldrift | **geschlossen für einen vollständigen Installationsstand** | Vollvorprüfung akzeptiert nur den protokollierten Installationshash oder einen bereits wiederhergestellten Vorzustand (`:458-467`). A18 verändert ein spätes Ziel und belegt, dass kein früheres Ziel mutiert wird. Der Hard-Crash-Teilstand bleibt P03-03, nicht eine Wiederöffnung dieses Drift-Riegels. |
| P03-08 Authenticode auf Bundle-Ordner / nur 64 Stellen | **Codepfad geschlossen, noch nicht als Produkt gefahren** | 40- und 64-stellige Eingaben werden akzeptiert; bei VST3 wird die innere Binärdatei statt des Ordners geprüft (`Install-Nakama.ps1:610-630`, `pruefe_installer_manifest.py:67`, `:277-285`). Da das Manifest bewusst keinen Thumbprint trägt, fehlt noch ein echter signierter End-to-End-Beleg. |

### Weiterhin bestätigte Codefehler und sichere Reproduktionen

#### R03-A — Hoch: Sicherung und Erwartungshash haben dieselbe ungeschützte Autorität

**Status:** bestätigt · **Confidence:** hoch

Die neue Prüfung bildet den Sicherungshash korrekt, vergleicht ihn aber nur
mit `e.vorher_sha256` aus derselben benutzerbeschreibbaren Ergebnisdatei
(`Install-Nakama.ps1:417-438`). Anders als `e.sha256` ist der Vorzustand an
keinen unabhängigen, authentisierten Vertrag gebunden. Ein Angreifer kann daher
Sicherung **und** Journal konsistent ändern. Der Broker besitzt keinen NAK-41-
Zweig; der erhöhte Rückweg kopiert diese Bytes nach dem Ziel und bestätigt sie
gegen den ebenfalls geänderten Journalhash (`:529-545`).

Gefahrene Temp-Reproduktion:

```text
[A] self-authenticated backup + journal
install 0 OK
rollback_exit 0 accepted=True target_is_malicious=True
signal RUECKWEG OK
```

**Sichere Behebung/Residual:** Journal plus Backups in eine nur erhöht
beschreibbare Ablage verschieben oder den Vorzustand authentisieren. Zusätzlich
unmittelbar vor dem Kopieren erneut hashen bzw. über einen bereits validierten
Handle arbeiten; die heutige Vollvorprüfung und das spätere Kopieren
(`:425-438` gegenüber `:529-545`) lassen sonst noch ein TOCTOU-Fenster.

#### R03-B — Hoch: `VORBEREITET` ist bei halbem oder entferntem Ziel nicht rückwegfähig

**Status:** bestätigt · **Confidence:** hoch

Das Journal setzt `mutation_begonnen` ausdrücklich vor dem gefährlichen
Fenster (`Install-Nakama.ps1:728-740`) und der Kommentar verspricht einen
Rückweg nach hartem Prozessende (`:655-658`). Die Rückwegvalidierung benutzt
dieses Flag jedoch nicht. Sie akzeptiert nur Vorhash, Sollhash oder Abwesenheit
bei zuvor abwesendem Ziel (`:458-467`). Stirbt der Prozess nach Entfernen des
alten Ziels oder während des rekursiven Kopierens, ist der Hash keiner dieser
Werte; der vorhandene, bereits gesicherte Vorzustand wird nicht restauriert.

Gefahrene Temp-Reproduktion mit einem authentisch erzeugten Journal, danach
nur auf den exakten Crash-Zwischenzustand `VORBEREITET`, erstes
`mutation_begonnen=true`, `mutation_abgeschlossen=false` gesetzt:

```text
[B] VORBEREITET journal + partial first copy
install 0 OK
rollback_exit 1 rejected=True partial_persists=True
signal ABBRUCH: Ziel ... wurde seit der Installation veraendert ...
```

**Sichere Behebung/Residual:** Ziel zunächst neben dem Live-Pfad vollständig
stagen und erst dann atomar tauschen. Alternativ darf ein authentisiertes
`VORBEREITET`/`ERROR_TEILSTAND` bei `mutation_begonnen &&
!mutation_abgeschlossen` den belegten Vorzustand restaurieren bzw. ein zuvor
nicht vorhandenes Teilziel entfernen. Ohne die Behebung von R03-A wäre diese
Ausnahme selbst wieder eine Angriffsfläche.

#### R03-C — Mittel: A17 adelt weiterhin lexikalisch korrekte Traversalpfade

**Status:** bestätigt · **Confidence:** hoch

`--hashen` umgeht A17 nicht mehr, aber die zwei zugrunde liegenden Regeln sind
weiter lexikalisch. Aktuelle reine In-Memory-Gegenprobe:

```text
broker-target-traversal ACCEPTED
source-traversal ACCEPTED
```

Der Runtime-Riegel verhindert das Quell-Traversal, nicht aber eine aus dem
Manifest kanonisierte Brokerbasis außerhalb des behaupteten Program-Files-
Baums. A17 muss `cmake_ziel` als einzelnes erlaubtes Segment validieren und
Windowspfade vor dem Vergleich kanonisieren; dieselben Mutationen gehören in
die Gegenprobe von A17.

#### R03-D — Mittel: Release-Validator und State-Writer teilen keinen Identitätsvertrag

**Status:** bestätigt · **Confidence:** hoch

Aktuelle reine In-Memory-Gegenprobe gegen **alle** A17-Regeln:

```text
bundle-collision ACCEPTED
identity-state-schema-1 ACCEPTED
identity-state-schema-'kaputt' ACCEPTED
identity-state-schema-None ACCEPTED
```

Der Ausführer fängt Kollision und falschen Typ inzwischen ab, aber erst beim
Installerlauf. Der Wert `1` bleibt auch dort legal, obwohl der Writer
`NakamaState schema=2` erzeugt (`NakamaState.cpp:32`,
`IdentityTestMain.cpp:646-650`). Eine einzige maschinenlesbare Schema-Konstante
oder ein Validatorvergleich gegen den State-Vertrag muss diese Drift vor dem
Release schließen.

### Echte Produktreste, nicht als Codefehler gezählt

- **Paket noch nicht auslieferbar:** Alle vier Artefakthashes und der
  Authenticode-Thumbprint sind bewusst `null`. A17 meldet dies ehrlich; der
  Installer verweigert die Installation. Der korrigierte Signatur-Codepfad ist
  deshalb noch kein gefahrenes Signatur-End-to-End.
- **Paket-/Manifest-Provenienz fehlt:** Skript und Manifest stammen weiterhin
  aus der benutzerbeschreibbaren Vor-Auslieferungsstruktur. Journal und
  Backups liegen nun geschützt und die Produktzielpolicy ist fest verdrahtet;
  eine vollständige Distributions-Vertrauenskette braucht trotzdem ein
  authentisiertes Paket/Manifest. Das ist ein Release-/Produktrest, nicht die
  wieder geschlossene beliebige Journal-/Backupquelle.
- **§55 ist bewusst unvollständig:** Repair, Uninstall und v2-Listener sind
  weiterhin vertagt. Ein sicherer Install/Rückweg ersetzt diese Produktpfade
  nicht.
- **Host-/Gate-Beleg offen:** Kein aktueller Lauf hat alle drei Bundles frisch
  in FL Studio gescannt und geladen. Sandbox-Grün ist kein Gate-1-PASS.
- **Vor-Umschnitt bleibt Vor-Umschnitt:** Die Entscheidung vom 27.08. ist nicht
  als bereits in CMake/Identität umgesetzt zu werten; drei Bundles sind für
  diesen geprüften Stand intentional.

### Zwischenbelege vor den Abschlussfixes

- `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py` — **31 ok,
  0 Fehler**, plus ehrlicher Hinweis auf vier Null-Hashes;
- `py -3.13 tools/eq-copilot/pruefe_installer_gegenpfad.py` — **42 ok,
  0 Fehler**, ausschließlich in `%TEMP%`;
- sechs reine A17-In-Memory-Mutationen — Bundlekollision, zwei Traversals und
  drei ungültige/abweichende Identity-Schemawerte wurden unerwartet akzeptiert;
- zwei zusätzliche Temp-Sandbox-Gegenpfade — selbstauthentisierte
  Broker-Sicherung wurde akzeptiert; partieller `VORBEREITET`-Stand blieb nach
  Rückwegabbruch bestehen.

## Scope und Norm

Gelesen wurden:

- `eq-copilot/CMakeLists.txt`, `plugin/CMakeLists.txt`,
  `cmake/NakamaKern.cmake`, `cmake/NakamaIdentitaet.cmake`;
- `identity/plugin-identities-v1.json`, `plugin/sonde/**`,
  `plugin/state/NakamaLebenslauf.*`, die State-Implementierung sowie die
  relevanten Processor-/Editor-/Factory-/Pipe-Caller;
- Identity-, Lifecycle-, State-Migration- und beide Sonden-Nulltests;
- `install/nakama-installer-v1.json`, `Install-Nakama.ps1`,
  `NakamaOrdnerHash.ps1`, den Installer-Vertrag und A17/A18.

Bewertet wurde gegen Entwurf §53.3–53.6
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:3509-3627`), §55 einschließlich
Exit/Rollback (`:3816-3840`), SONDE-007 (`:4078`), Gate 1 und Gate 7. Besonders
maßgeblich sind: kein `JucePlugin_*` im Kern (`:3577-3584`), neutral bis zur
positiven Klassifikation und Spawn nur für Main mit offenem Editor
(`:3594-3606`) sowie keine Neuinstallation bei gescheitertem Manifest oder
gescheiterter Migration (`:3839-3840`).

## Bestätigte Befunde des Ausgangsaudits (historischer Vor-Fix-Stand)

### P03-01 — Kritisch: `install-ergebnis.json` ist ein ungeprüfter privilegierter Rollback-Plan

**Status:** bestätigt · **Confidence:** hoch

`Install-Nakama.ps1` legt Ergebnis und Sicherungen unter dem vom normalen
Benutzer beschreibbaren Repo ab (`:82-84`, `:438-460`). `-Rueckweg` prüft an
der Ergebnisdatei nur `status == OK` (`:258-264`) und glaubt anschließend
unverändert:

- `art` und `ziel` für rekursives Löschen bzw. Ersetzen (`:319-337`),
- `gesichert` als Kopierquelle nach einem bloßen `Join-Path` (`:96`, `:329-330`),
- `erzeugte_ordner` als Liste zu löschender Verzeichnisse (`:346-355`).

Es gibt weder Ergebnis-Schema noch Signatur/MAC, kanonische Pfade oder eine
Containment-Prüfung gegen VST3-, Broker- und Sicherungswurzel. Ein Eintrag mit
`gesichert=false`, `art=vst3` und einem frei gesetzten `ziel` erreicht direkt
`Remove-Item -Recurse`; ein Eintrag ohne `ziel_id` umgeht zugleich den
NAK-41-Zweig als vermeintlicher Broker (`:285-288`). Die heute noch leeren
Release-Hashes schützen diesen Weg nicht, weil der Rückweg vor dem
Vollständigkeitsriegel abzweigt (`:257` gegenüber `:375`).

**Sichere Reproduktion/Testlücke:** A18 in seiner `%TEMP%`-Sandbox um eine
kopierte Ergebnisdatei mit einem zweiten Sentinel-Verzeichnis außerhalb der
deklarierten Zielwurzel erweitern; ausschließlich den Sentinel adressieren.
Der heutige A18-Lauf prüft nur einen wohlgeformten Erfolgsstand und NAK-41
(`tools/eq-copilot/pruefe_installer_gegenpfad.py:187-239`), keine manipulierte
Ergebnisdatei.

### P03-02 — Kritisch: Rollback-Sicherungen werden weder vor noch nach dem Restore verifiziert

**Status:** bestätigt · **Confidence:** hoch

Beim Installieren wird der Hash des Vorzustands als `vorher_sha256` notiert
(`Install-Nakama.ps1:447-460`, `:479-488`). Beim Rückweg dient jedoch nur dieser
gespeicherte Text der NAK-41-Einstufung (`:285-305`); die Bytes unter
`gesichert` werden nie gegen ihn gehasht. Anschließend kopiert der erhöhte Lauf
die Sicherung nach Program Files (`:329-330`) und schreibt ohne Zielhash
`RUECKWEG OK` (`:362-371`). Das gilt besonders direkt für die Broker-`.bak`,
weil sie keine `ziel_id` und damit keine State-Warnung hat.

Damit kann eine nach der Installation im Benutzer-Repo ausgetauschte `.bak`
oder `.bundle` als privilegierter Broker/Plugin-Stand restauriert werden. Die
vorhandene Hashangabe ist vorhanden, aber am inversen Pfad wirkungslos.

**Sichere Reproduktion/Testlücke:** A18-Sandbox erfolgreich installieren,
danach ausschließlich die Sandbox-Sicherung byteweise ändern und
`-Rueckweg` fahren. Erwartung eines Riegels: Abbruch vor Zielmutation. Der
heutige Test verändert keine Sicherung und prüft keinen Restore-Nachhash.

### P03-03 — Hoch: Eine Teilinstallation besitzt kein haltbares Journal und keinen automatischen Gegenpfad

**Status:** bestätigt · **Confidence:** hoch

Nach vollständiger Quellenprüfung mutiert der Installer die vier Ziele
seriell (`Install-Nakama.ps1:383-402`, `:445-490`). Pro Artefakt wird gesichert,
das vorhandene Ziel vollständig entfernt, kopiert und nachgehasht
(`:453-475`). Die einzige Ergebnisdatei mit allen Rückwegseinträgen entsteht
erst **nach** der gesamten Schleife (`:492-497`).

Scheitert Artefakt 2–4:

- hat Artefakt 1 bereits den neuen Stand;
- ein eigener Posthash-Fehler ruft `Abbruch`, das nur `{status, grund, zeit}`
  schreibt und die bis dahin aufgebauten Einträge verwirft (`:102-125`);
- ein von `$ErrorActionPreference='Stop'` ausgelöster Copy-/Remove-Fehler wird
  überhaupt nicht gefangen (`:77`, kein `try/catch`) und kann sogar eine alte
  `OK`-Ergebnisdatei stehenlassen;
- `-Rueckweg` akzeptiert nur `status == OK` (`:262-264`).

Der Lauf kann somit eine gemischte Installation hinterlassen, obwohl sein
eigener Rückweg nicht mehr weiß, was bereits getauscht wurde. Das widerspricht
dem gekoppelten Install/Rollback-Lebenszyklus und §55 Rollback.

**Sichere Reproduktion/Testlücke:** In der bestehenden A18-Sandbox beim
zweiten Ziel nach der ersten erfolgreichen Kopie einen deterministischen
Copy- oder Posthash-Fehler injizieren; anschließend Ziel 1 und
`install-ergebnis.json` prüfen und `-Rueckweg` versuchen. A18 deckt nur den
vollständig grünen Vier-Artefakt-Pfad ab.

### P03-04 — Hoch: Pfad-Containment fehlt; A17 akzeptiert Traversal und `--hashen` umgeht A17 vollständig

**Status:** bestätigt · **Confidence:** hoch

Der Ausführer löst repo-relative Quellen mit einfachem `Join-Path` auf
(`Install-Nakama.ps1:96`, `:386`) und bildet Ziele direkt aus Manifest- bzw.
Identitätsstrings (`:173-180`). `GetFullPath`/`Resolve-Path` plus Prüfung gegen
eine erlaubte Wurzel fehlt. Der Validator ist an zwei Stellen ebenfalls offen:

- der Broker gilt schon durch den Stringpräfix
  `C:/Program Files/` als geschützt
  (`tools/eq-copilot/pruefe_installer_manifest.py:262-273`);
- der Quellpfad wird aus dem ebenfalls ungeprüften `cmake_ziel` nachgerechnet
  (`:195-213`). Traversal kann daher auf beiden Seiten konsistent aussehen.

Zusätzlich springt `--hashen` direkt in `hashen(manifest)` und führt keine der
`REGELN` aus (`:516-530`). Dort wird `WURZEL / a["quelle"]` ohne
Containment gehasht und anschließend das Manifest geschrieben (`:397-435`).

Die rein speicherinterne Mutationsprobe dieser Session ergab:

```text
TRAVERSAL_TARGET_RULE= (True, 'C:/Program Files/Common Files/VST3 | C:/Program Files/../Temp/Nakama')
TRAVERSAL_SOURCE_RULE= (True, '')
```

Damit kann ein fehlerhaftes oder ausgetauschtes Release-Manifest außerhalb
des Repos hashen und der erhöhte Ausführer außerhalb der vertraglichen
Installationswurzeln schreiben/löschen.

**Sichere Reproduktion/Testlücke:** Die obige In-Memory-Mutation als A17-Fall
festschreiben; zusätzlich in A18 ausschließlich Temp-Wurzeln verwenden und
vor jeder Mutation kanonische Containment-Abbrüche erwarten. `--hashen` darf
erst nach denselben Regeln laufen.

### P03-05 — Hoch: Installationskritische Identitätsinvarianten sind nicht geschlossen

**Status:** bestätigt · **Confidence:** hoch

Die drei aktuellen Einträge sind korrekt und verschieden
(`identity/plugin-identities-v1.json:32-72`). Die Leser beweisen aber nicht,
dass dies so bleibt:

1. `bundle` muss weder pfadsicher noch über alle Ziele eindeutig sein.
   `NakamaIdentitaet.cmake` verlangt nur „nicht leer“ und Viercodes
   (`:118-161`); der Identity-Test zählt drei Einträge und drei handschriftliche
   Zielzeilen (`plugin/tests/IdentityTestMain.cpp:261-303`), prüft aber keine
   eindeutigen aufgelösten Installationspfade. A17 reduziert die Identitäten
   sogar per Dictionary nach ID und vergleicht nur IDs
   (`pruefe_installer_manifest.py:150-176`). Der Installer leitet dagegen
   genau aus `ident.bundle` sein Ziel ab (`Install-Nakama.ps1:173-180`) und
   würde zwei Artefakte am selben Ziel nacheinander überschreiben.
2. `state_schema` steuert den NAK-41-Riegel (`Install-Nakama.ps1:295-303`),
   wird aber weder vom CMake-Leser noch vom Identity-Test oder A17 auf Typ,
   Bereich und Gleichheit mit dem wirklichen State-Vertrag geprüft. A17 prüft
   nur `state_schema` der historischen `bekannte_staende`
   (`pruefe_installer_manifest.py:296-311`).

Speicherinterne Gegenproben bestätigten die Lücke: Zwei Identitäten mit
demselben Bundle und passend nachgerechneter Quelle ergaben
`COLLISION_FAILED_RULES=[]`; `state_schema` mit `1`, `"kaputt"` oder `null`
ergab jeweils ebenfalls keine fallende A17-Regel. Ein Zielkollisions-Release
installiert still das letzte Bundle; ein falsches Schema kann NAK-41 umgehen
oder den erhöhten Rückweg erst mitten im Lauf mit Typfehler abbrechen.

**Sichere Reproduktion/Testlücke:** reine Mutationsfälle für doppelte
aufgelöste Zielpfade, `..`/Separatoren im Bundle und ungültige
`state_schema`-Werte. Keine echte Installation nötig.

### P03-06 — Mittel: Installieren und Rückweg sind nicht gegen Parallelbetrieb serialisiert

**Status:** bestätigt · **Confidence:** hoch

Das Skript besitzt weder benannten Mutex noch Dateilock. Sicherungsnamen sind
nur auf Sekunden genau (`Install-Nakama.ps1:442`, `:455-460`), und
`Schreibe-Ergebnis` überschreibt die gemeinsame JSON-Datei direkt per
`Set-Content`, nicht über eine atomar umbenannte Tempdatei (`:98-100`). Zwei
gleichzeitige Installationen, oder Installation gegen Rückweg, können daher
dieselben Ziele und bei Start in derselben Sekunde dieselben Sicherungen
löschen/ersetzen; zuletzt schreibt ein nicht notwendig zum Zielstand passendes
Ergebnis.

**Sichere Reproduktion/Testlücke:** Zwei A18-Sandboxläufe mit einer Barriere
vor `Kopiere-Stand` parallel starten und anschließend Zielhash,
Sicherungshashes und JSON-Zuordnung prüfen. Der vorhandene Gegenpfadtest fährt
nur seriell.

### P03-07 — Mittel: Rollback überschreibt Drift ohne Konfliktprüfung

**Status:** bestätigt · **Confidence:** hoch

Der Installationsstand wird als `e.sha256` protokolliert
(`Install-Nakama.ps1:479-488`). `-Rueckweg` bildet vor Restore/Entfernen jedoch
keinen Hash von `e.ziel` und vergleicht ihn nie mit `e.sha256`
(`:319-358`). Wurde das Ziel nach dem protokollierten Lauf repariert,
aktualisiert oder manuell geändert, zerstört ein alter Rückweg diesen neueren
Stand ohne Warnung. `-Erzwingen` bezieht sich ausschließlich auf NAK-41, nicht
auf Ziel-Drift.

**Sichere Reproduktion/Testlücke:** Nach erfolgreicher A18-Sandboxinstallation
eine zusätzliche Zieldatei bzw. einen neuen Brokerhash erzeugen und Rückweg
anfordern. Erwartung: Konfliktabbruch oder ein eigener expliziter Force-Pfad.

### P03-08 — Mittel, derzeit dormant: Der angekündigte Authenticode-Zweig kann kein VST3-Bundle prüfen

**Status:** bestätigt · **Confidence:** hoch

Für VST3 ist `plan.Quelle` absichtlich der Bundle-**Ordner**
(`Install-Nakama.ps1:383-400`). Sobald ein Thumbprint gesetzt ist, ruft der
Installer für jedes Planobjekt `Get-AuthenticodeSignature -LiteralPath
$p.Quelle` auf (`:404-412`). Das Cmdlet unterstützt keine Verzeichnisse. Die
read-only Probe am aktuellen gebauten `EQ-Copilot.vst3` ergab:

```text
The Get-AuthenticodeSignature cmdlet does not support directories.
Supply a path to a file and retry.
```

Unabhängig davon verlangt A17 für `authenticode_thumbprint` `HEX64`
(`pruefe_installer_manifest.py:66`, `:276-284`). Die lokale Gegenprobe an der
gültig signierten Microsoft-Datei `$PSHOME/pwsh.exe` lieferte einen
40-stelligen `SignerCertificate.Thumbprint`. Der angekündigte Übergang von
`null` zu echter Prüfung ist daher auf Validator- **und** Ausführerseite
blockiert.

Das Feld ist heute bewusst `null` (`install/nakama-installer-v1.json:19-22`),
also behauptet der aktuelle Lauf keine Signaturprüfung. Der Defekt ist dormant,
aber vor dem ersten signierten Paket zwingend.

**Sichere Reproduktion/Testlücke:** Die zwei ausgeführten read-only Proben als
Validatorfall übernehmen; für VST3 explizit festlegen, welche signierte Datei
im Bundle geprüft wird und wie der Ordner-/Manifesthash authentisiert ist.

## Intentional, bekannt oder nicht als neuer Defekt gewertet

- **Repair/Uninstall/v2-Listener fehlen absichtlich.** §55 fordert sie
  (`Entwurf:3830-3831`), der lokale Vertrag vertagt Repair über Diagnose,
  Update/atomaren Tausch/Signaturkette und den v2-Listener ausdrücklich
  (`schemas/installer/nakama-installer-v1.md:305-319`); das Manifest wiederholt
  dies (`install/nakama-installer-v1.json:93-97`). Einen Uninstall-Schalter gibt
  es nicht. Klassifikation: **intentionaler Scope-Entscheid, aber §55-Exit damit
  nicht erfüllt**, nicht „bereits implementiert“.
- **Noch nicht auslieferbar:** Alle vier `sha256` und
  `hashes_erzeugt_am` sind `null` (`nakama-installer-v1.json:25-58`); der normale
  Installationspfad bricht korrekt vor der ersten Mutation ab
  (`Install-Nakama.ps1:375-381`). Klassifikation: **intentional**. Dies nimmt
  P03-01/P03-02 nicht zurück, weil `-Rueckweg` vorher abzweigt.
- **FL-Exit fehlt weiterhin:** kein aktueller Lauf hat alle drei Bundles in FL
  gescannt und geladen. `docs/PLAN-STAND.md:89` führt G1 weiter als
  `NEEDS_WORK`; die Nacharbeit ist kein frischer Gate-PASS. Klassifikation:
  **bekannter offener externer Beleg**, nicht durch Codeinspektion geschlossen.
- **Gate 7:** Die aktuelle State-Matrix lehnt
  `post_fader_contribution` für alle vier Klassen ab, solange
  `contribution_aux` unsupported ist; der aktuelle State-Test belegt 16/16.
  Das schließt den damals gefundenen konkreten Pfad, ersetzt aber keinen neuen
  adversarialen Gate-Lauf. Klassifikation des alten konkreten Befunds:
  **stale/geschlossen**, Gate insgesamt nicht neu abgenommen.
- **27.08.-Produktentscheid:** nicht in CMake/Identität hineininterpretiert.
  Der aktuelle Code baut weiterhin drei getrennte Bundles. Klassifikation:
  **bewusster Vor-Umschnitt**, kein Ziel-/Bundlefehler des heutigen Auftrags.

## Widerlegte Bruchhypothesen im aktuellen Stand

- **Identitätsleck im gemeinsamen Kern:** widerlegt. `NakamaKern` umfasst die
  deklarierte kleine Quellmenge, K1/K2/K3 fanden keine `JucePlugin_*`- oder
  Zielidentität; die drei gebauten Module tragen jeweils nur ihre eigene CID.
- **Aktuelle Ziel-/Bundle-Verwechslung:** widerlegt. CMake liest `main`,
  `passive-probe`, `active-probe` über die dünnen Zielschichten
  (`plugin/CMakeLists.txt:32-47`, `:181-223`); die gebauten Module stimmen mit
  den drei aktuellen Manifestzeilen überein. P03-05 betrifft den fehlenden
  Riegel gegen künftige Kollision, nicht einen heutigen Tausch.
- **Unklassifiziert/legacy/read-only färbt Audio:** widerlegt. Die
  Hörmarkierung verlangt zusätzlich das atomare positive Main-Urteil,
  gültiges Playing, Echtzeit und Editor/Testweg
  (`plugin/src/PluginProcessor.cpp:286-307`). Lifecycle-Test und Audiovergleich
  blieben vor Klassifikation, bei legacy und nach read-only bitgleich.
- **Falsche Lifecycle-Übergänge:** widerlegt. Schema-1
  `sensor|pre|post -> legacy`, `hub -> main`, bestätigtes Schema-2-Main ->
  main, read-only -> unclassified, feste Sondenklasse erst nach gültigem State;
  die Implementierung steht in `plugin/state/NakamaLebenslauf.cpp:45-114`.
  `ignoriert` lässt Zustand und Klassifikation absichtlich unverändert, weil
  kein Restore stattgefunden hat (`:47-53`).
- **Scanner-/Probe-/Render-Spawn heute:** widerlegt. Main startet zwar seinen
  Worker und den Connect-Client bereits im Konstruktor
  (`PluginProcessor.cpp:34-50`), aber `PipeClient` tut nur
  `CreateFileW(... OPEN_EXISTING)` (`PipeClient.cpp:178-195`). Im gesamten
  Pluginpfad existiert kein `CreateProcess`/`ShellExecute`/Spawn-Caller.
  Sonden besitzen gar keinen PipeClient und `darfBrokerStarten()` bleibt immer
  falsch. Das ist der ausdrücklich erlaubte connect-only-Vorstand; der
  Autorisierungshaken ist erst für SONDE-010.
- **State-Bundle-Matrix-Lücke:** widerlegt für den aktuellen Vertrag. Fresh,
  Recall, fremdes Major/read-only, additive Felder, alle 109 Parameter sowie
  alle 16 Klasse×Messposition-Kombinationen liefen grün. Suna schreibt
  `passive_probe` ohne `Parameters`, Probeeq `active_probe` mit genau einem
  `Parameters`-Kind (`plugin/sonde/SondeProcessor.cpp:6-25`).

## Laufende Belege dieser Session

Alle Läufe erfolgten vom Workspace-Root und ohne Installation:

- `python tools/eq-copilot/pruefe_kern_identitaetsfrei.py` — **26 ok, 0 Fehler**;
- `EqCopIdentityTest.exe` — **114 ok, 0 Fehler**;
- `EqCopLebenslaufTest.exe` — **61 ok, 0 Fehler**;
- `EqCopSunaNullTest.exe` — **62 ok, 0 Fehler**;
- `EqCopProbeeqNullTest.exe` — **62 ok, 0 Fehler**;
- `EqCopStateMigrationTest.exe` — **132 ok, 0 Fehler**;
- damaliger `python tools/eq-copilot/pruefe_installer_manifest.py` — **31 ok, 0 Fehler**,
  dabei ehrlich vier nicht auslieferbare Null-Hashes;
- damaliger Ausgangslauf
  `python tools/eq-copilot/pruefe_installer_gegenpfad.py` — **27 ok,
  0 Fehler**, ausschließlich `%TEMP%`-Sandbox; der aktuelle erweiterte Lauf
  steht oben mit **78 ok, 0 Fehler**;
- read-only Authenticode-Proben — VST3-Ordner reproduzierbar abgelehnt,
  gültig signiertes `pwsh.exe` mit 40-stelligem Thumbprint;
- reine In-Memory-Mutationen — Zielkollision, Traversal und ungültige
  `state_schema` passieren die jeweils genannten A17-Regeln.

Die fünf gefahrenen C++-Binaries sind vom 24.08. 15:40 und damit neuer als die
späteste einschlägige Produkt-/Testquelle (24.08. 15:39:58). Es wurde kein
Rebuild ausgeführt, weil dieses Paket nur lesend arbeiten durfte.

## Empfohlene Reihenfolge für eine spätere Behebung

1. Rollback-Ergebnis als validiertes, authentisiertes und auf feste Wurzeln
   begrenztes Format behandeln; Sicherung vor und Ziel nach Restore hashen.
2. Haltbares per-Artefakt-Journal plus automatische Kompensation und exklusiven
   Installer-Lock bauen; Ergebnis atomar veröffentlichen.
3. Alle Pfade kanonisieren/containern; dieselben Regeln **vor** `--hashen` und
   zur Laufzeit anwenden.
4. Aufgelöste Zielpfade und `state_schema` im Identitäts-/Installervertrag
   vollständig validieren.
5. Authenticode-Einheit und Thumbprintformat festlegen, danach erst das Feld
   von `null` lösen.

**Residualrisiko danach:** Auch ein Fix aller acht Kandidaten wäre noch kein
§55-PASS. Solange Manifest/Skript nicht authentisiert sind, Repair/Uninstall
und v2-Listener fehlen und die drei Bundles nicht frisch in FL gescannt und
geladen wurden, bleibt das Paket Vor-Auslieferung. Diese Punkte dürfen nicht
durch grüne Sandbox- oder Headless-Tests ersetzt werden.
