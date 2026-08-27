---
id: w-2026-08-27-installer-als-privilegierte-transaktion
typ: wissenseintrag
status: aktiv
datum: 2026-08-27
aktualisiert: 2026-08-27
hauptbereich: engineering
weitere_bereiche: [fl-studio]
pruefstand: mehrfach-belegt
---

# Ein Installer ist eine privilegierte Transaktion

## Menschliches Warum

Ein Update darf ein bestehendes Plugin weder verlieren noch durch unbemerkte
fremde Bytes ersetzen. Weil die Installation mit erhöhten Rechten arbeitet,
ist selbst ein korrektes Kopieren gefährlich, wenn Pfad, Journal oder
Rollback-Anweisung zuvor von einem normalen Benutzer austauschbar waren.

Die Kausalkette lautet:

> Weil ein Produzent sein funktionierendes Plugin einer erhöhten Installation
> anvertraut, muss Nakama Eingabe, Ziel, Sicherung und Rückweg als eine
> geschützte Transaktion beweisen, damit Fehler oder lokale Manipulation weder
> Fremdcode installieren noch den letzten guten Stand zerstören.

## Belegtes Wissen

Windows-Pfade sind keine bloßen Zeichenketten. Reparse Points können das
übliche Öffnungsverhalten umlenken; `FILE_FLAG_OPEN_REPARSE_POINT` öffnet den
Punkt selbst statt seinem Ziel zu folgen. `GetFinalPathNameByHandleW` liefert
den vollständig aufgelösten Pfad eines bereits geöffneten Handles. Eine
Containment-Prüfung nur an normalisiertem Text beweist deshalb nicht, dass das
später geöffnete Objekt noch innerhalb des erlaubten Ziels liegt.

Microsoft empfiehlt für atomare Aktualisierung einer einzelnen Datei, zuerst
eine neue Datei zu schreiben und anschließend `ReplaceFile` zu verwenden.
Diese API ersetzt genau eine Datei, hat dokumentierte Teilfehlerzustände und
verlangt, dass Original, Ersatz und Backup auf demselben Volume liegen. Sie
macht aus mehreren Bundle-Dateien noch keine gemeinsame Transaktion.

Authenticode prüft ebenfalls ein konkretes Objekt: `WINTRUST_FILE_INFO` erhält
vollständigen Dateipfad oder Dateihandle. Ein `.vst3`-Verzeichnis ist daher
nicht selbst die Signatur-Einheit; das Manifest muss die tatsächlich zu
prüfende Binärdatei benennen. Microsoft empfiehlt außerdem Zeitstempel, damit
eine gültige Signatur nach Zertifikatsablauf weiter prüfbar bleibt.

## Fehlerlexikon und Gegenbeweis

| Fehlerquelle | Gefährlicher Pfad | Erforderlicher Gegenbeweis |
|---|---|---|
| Journal im benutzerbeschreibbaren Checkout | erhöhter Prozess folgt fremdem Rückweg | geschützter Transaktionsanker mit eigener ACL |
| textuelle Pfadpräfix-Prüfung | `..`, Alias oder Reparse Point verlässt Ziel | kanonische Windows-Semantik plus Handle-/Reparse-Prüfung |
| Preflight lange vor Mutation | Ziel wechselt zwischen Prüfen und Schreiben | unmittelbar vor Mutation erneut prüfen; Objektidentität binden |
| Backup ohne Hash | beschädigte Sicherung gilt als Rückweg | Hash nach Schreiben und vor Restore prüfen |
| mehrere Bundleziele kollidieren | eine Quelle überschreibt die andere | eindeutige kanonische Zielmenge vor erster Mutation |
| Crash ohne durable Phase | Recovery kennt letzte vollständige Aktion nicht | Phase vor/nach jeder Mutation flushen und Killpunkte testen |
| parallele Installer | Journale und Backups verschränken sich | geschützte exklusive Sperre über Preflight bis Abschluss |
| Rollback überschreibt Benutzerdrift | neuerer Fremdstand geht verloren | aktuellen Zielhash gegen Transaktionsnachbild prüfen |
| Signatur auf Verzeichnis prüfen | Prüfung ist technisch nicht das signierte Objekt | konkrete Binärdatei und erwartete Zertifikatsidentität |
| Zertifikat ohne Zeitstempelregel | gültiges Release wird nach Ablauf unprüfbar | Signatur-, Ketten-, Identitäts- und Zeitstempelpolitik |
| strukturell fremdes `.vst3` | Installation beginnt, Rückweg kann Form nicht erhalten | Bundleform und Wiederherstellbarkeit vor Mutation beweisen |
| Reparatur/Uninstall getrennt gedacht | nur Happy-Path-Install besitzt Wahrheit | Install, Resume, Rollback, Repair und Uninstall als Zustandsmaschine |

## Bedeutung für Nakama

Die Debugging-Session fand diese Klasse in großer Breite: Das
Rollback-Journal lag zunächst unter einer benutzerbeschreibbaren Repo-Wurzel;
Backup-Bytes wurden nicht verifiziert; Teilinstallationen hatten keinen
dauerhaften Kompensationspfad; Zielcontainment, Kollisionen, exklusive Sperre
und driftbewusster Rollback waren unvollständig. Die Signaturprüfung behandelte
ein VST3-Verzeichnis wie eine Datei und erwartete nicht dieselbe
Thumbprintform wie Windows.

Die Reparatur verlegte Journal und Backups in einen geschützten Anker, bindet
Hashes, Crashphasen, kanonische Zielprüfung, Reparse-Schutz und Cross-Repo-Lock
zusammen und verweigert fremde oder unvollständige Bundles vor der ersten
Mutation. Die konkrete signierte Binärdatei und Zertifikatsidentität werden als
Manifestvertrag geprüft.

Für die spätere Release-Sonde folgt: Kill-Tests müssen jede Dateigrenze sowie
Resume, Rollback, Repair und Uninstall abdecken. Der Beweis bleibt derzeit
begrenzt: reale erhöhte Installation, echte Release-Signaturen, FL-Studio-
Scan/Load und Stromverlust während Flush wurden in der Debugging-Session nicht
ausgeführt; leere Release-Hashes sind kein auslieferbares Manifest.

## Grenzen, Widersprüche und offene Fragen

`ReplaceFile` löst den atomaren Ersatz einer Datei, nicht den Commit eines
mehrteiligen VST3-Bundles. Der mehrstufige Transaktions- und
Kompensationsvertrag ist daher eine Nakama-Ableitung. Welche Zertifikatskette
und Zeitstempelpolitik für Releases gilt, bleibt eine spätere verbindliche
Releaseentscheidung.

## Quellen

- Titel: *Reparse Point Operations*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/fileio/reparse-point-operations
- Titel: *GetFinalPathNameByHandleW function*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfinalpathnamebyhandlew
- Titel: *ReplaceFile function*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-replacefilew
- Titel: *WINTRUST_FILE_INFO structure*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/api/wintrust/ns-wintrust-wintrust_file_info
- Titel: *Time Stamping Authenticode Signatures*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/seccrypto/time-stamping-authenticode-signatures
- Eingang: `web-recherche`
- geprüft: Microsoft-Win32-Referenz sowie Nakamas Installer-, Rollback- und
  Debugging-Beweise

## Verknüpfungen

- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/03-targets-lifecycle-installer.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/03-targets-lifecycle-installer.md)
- [`../../docs/FL-Nakama-Sonden-Design-Entwurf.md`](../../docs/FL-Nakama-Sonden-Design-Entwurf.md)
- [`../../eq-copilot/install/`](../../eq-copilot/install/)
- [`2026-08-27-crashfester-store-und-outbox.md`](2026-08-27-crashfester-store-und-outbox.md)
