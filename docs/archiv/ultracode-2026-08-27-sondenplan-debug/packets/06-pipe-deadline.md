# Paket 06 — PipeClient: absolute Frame-Deadline

Stand: 27.08.2026. Scope dieses Pakets waren ausschliesslich
`eq-copilot/plugin/src/PipeClient.{h,cpp}` und der bereits im laufenden Audit
angelegte `PipeClientLifecycleTestMain.cpp`.

## Geschlossener Fund

Der fruehere 5-s-Timeout galt pro `ioGenau()`-Aufruf und wurde innerhalb der
Schleife nach jedem Teiltransfer erneut voll angesetzt. Kopf und Body hatten
zudem je ein eigenes Budget. Ein drip-feedender Peer konnte einen Frame damit
beliebig lange offenhalten.

`sende()` und `empfange()` bilden nun unmittelbar vor dem ersten Frame-I/O je
eine absolute `steady_clock`-Deadline. Dieselbe Deadline wird fuer den
4-Byte-Kopf, den Body und jede Wiederholung nach einem partiellen
Read/Write weitergereicht. Vor der Submission, nach dem Erwerb des
`handleMutex`, vor dem Wait und nach jeder Completion wird nur noch die
verbleibende Zeit beziehungsweise die absolute Frist bewertet. Die auf volle
Win32-Millisekunden aufgerundete Wartezeit kann deshalb auch eine finale
Completion nach der Deadline nicht versehentlich akzeptieren.

Das Defaultbudget bleibt 5000 ms. Fuer schnelle Gegenproben nimmt der
Konstruktor optional `std::chrono::milliseconds`; bestehende Aufrufe mit zwei,
drei oder vier Argumenten bleiben quellkompatibel. Werte kleiner als 1 ms und
groesser als das groesste endliche Win32-Wait werden sicher begrenzt, bevor die
Deadline addiert wird.

Die bestehende Abbruch- und Handle-Disziplin blieb erhalten:

- Submission, externes `CancelIoEx` und Close werden weiter ueber dasselbe
  `handleMutex` serialisiert.
- Stop/Reconnect invalidieren weiter die Verbindungsgeneration.
- Nach Timeout wird eine ausstehende stacklokale `OVERLAPPED`-Operation
  abgebrochen und ihre Completion abgewartet, bevor die Struktur endet.
- Ein Abbruch schreibt keinen irrefuehrenden Timeoutfehler in den sichtbaren
  Zustand; der Aufrufer prueft weiterhin die Generation.

## Regressionstest

`eineDeadlineFuerDenGanzenFrame()` verwendet einen eindeutigen Probe-Pipenamen
und ein 320-ms-Testbudget. Der Fake-Peer sendet die vier Kopfbytes in vier
60-ms-Teilstuecken und wartet danach weitere 120 ms auf den Body. Jeder
Einzeltransfer und jede einzelne Kopf-/Body-Phase bleibt unter 320 ms; nur der
vollstaendige Frame braucht rund 360 ms. Ein erneuertes Teil- oder Phasenbudget
wuerde den Frame daher akzeptieren, die neue gemeinsame Frist trennt nach rund
320 ms mit `Zeitlimit`.

## Frische Belege

- CMake Release-Build `EqCopPipeClientTest`: erfolgreich.
- `EqCopPipeClientTest.exe`: alle Lifecycle-/Protokollfaelle gruen, neuer
  Deadline-Fall bei 327 ms, insgesamt **0 Fehler**.
- Derselbe Test lief zuvor fuenfmal hintereinander gruen; Deadline-Fall jeweils
  325–330 ms.
- CMake Release-Build `EqCopPipeProbe`: erfolgreich und belegt den unveraendert
  funktionierenden Vier-Argument-Aufruf mit dem 5000-ms-Default.
- `git diff --check` fuer die drei Scope-Dateien: keine Whitespacefehler
  (nur die bestehende Git-Zeilenendenwarnung).

## Restrisiko

Die Deadline gilt absichtlich pro einzelnem Protokollframe, nicht fuer eine
gesamte Hello/Welcome- oder Heartbeat/ACK-Runde. Die Regression prueft den
Windows-Named-Pipe-Pfad deterministisch mit grossem Timingabstand; harte
Echtzeitgarantien gegen einen fuer mehr als die Prozess-Scheduling-Latenz
vollstaendig angehaltenen Clientthread kann ein Wanduhrtimeout naturgemaess
nicht geben.
