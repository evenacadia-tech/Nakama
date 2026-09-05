# unit-14 — vertrag/NakamaTelemetrie.cpp, vertrag/NakamaTelemetrie.h, vertrag/NakamaUtf8.h, vertrag/NakamaVertrag.cpp, vertrag/NakamaVertrag.h
Gelesen: 55 von 55 Einheiten vollständig · Zeilen: 2121 (alle fünf Dateien wurden zusätzlich ganz gelesen: 725 + 142 + 74 + 1187 + 149 = 2277)

Bedrohungsmodell LOCAL_UNPRIVILEGED — vorab an der Quelle geklärt, weil es jede
Erreichbarkeitszeile trägt:

* `vertrag::textriegel/textriegelBytes` hat drei Aufrufer außerhalb der Tests:
  `src/SourcesModel.cpp:459` (`session_snapshot`), `:813` (`evidence_invalidate`)
  und `state/NakamaParameter.cpp:197` (`ausDtoText` — das wiederum hat heute
  keinen Produktionsaufrufer, nur Header und Tests).
* `vertrag::Schema` und `vertrag::Verletzung` haben KEINEN Produktionsaufrufer
  (`rg "Verletzung|vertrag::Schema" eq-copilot/plugin` trifft nur `tests/`).
  Alles ab `NakamaVertrag.cpp:684` (Ladelauf, Prüfwerk, `pruefeWert`,
  `diskriminator*`, `pointerSegment`, `gleich`) ist damit heute nicht
  angreifbar; Beobachtungen dort bleiben unten LOW oder ungemeldet.
* `telemetrie::lese/pruefe` ist produktiv: `src/SourcesModel.cpp:959`
  ← `src/PluginProcessor.cpp:1277` (`v3Frame`) ← `TelemetryClient`-Thread.
* Die Bytes müssen von einem Peer kommen, der
  `ipc::namedPipeServerAuthentisieren` besteht (Pfad + SHA-256 + Authenticode,
  `core/ipc/IpcVerbindung.h:105-144`, `src/PluginProcessor.cpp:73-77`). Ein
  reiner Pipenamen-Besetzer liefert also kein Byte; die realistische feindliche
  Quelle ist ein kompromittierter oder fehlerhafter Broker bzw. Inhalt, den der
  echte Broker von einem dritten lokalen Probe-Client aggregiert. Dieser
  Vorbehalt steht in jeder Befundzeile.

## Befunde

- [MEDIUM] vertrag/NakamaVertrag.cpp:487 · bounds/logik (Rechenaufwand) · Der Ausschnitts-Helfer `teil()` des Textriegels baut seinen String Zeichen für Zeichen mit `s += juce::String::charToString(z[k])` und ist damit Θ(k²) in der Tokenlänge; ein einziges JSON-Token von Rahmengröße hängt den Control-Thread für zig Sekunden auf. · Auslöser: ein v3-Control-Frame (Rahmengrenze `core/ipc/WireEnvelope.h:38` = 262144 Bytes) mit EINEM Token von rund 262 000 Zeichen — eine Zahl (`1111…1`), ein alphabetisches Literal (`aaaa…a`) oder ein Exponent aus lauter Nullen (`1e000…05`). · Wirkung: JUCE reallokiert bei jedem Anhängen exakt auf die neue Länge (`juce_String.cpp:747-763` → `:296-299` → `:190-209`, `jmax(allocatedNumBytes, numBytes)`, kein geometrisches Wachstum), und `getByteOffsetOfEnd()` (`:551-554`) läuft davor jedes Mal bis zum NUL; also rund 2·k Heapallokationen und k²/2 kopierte Bytes, bei k = 262 000 etwa 3,4·10^10 Byte je Aufruf. `teil()` wird für dasselbe Token zweimal gerufen (`:621` für `ganz`, `:668` für `lit`; bei führender Null zusätzlich `:629`), für ein Literal einmal (`:604`). Dieselbe Form in `zahlPruefen`: `:340-341` (`ohneNull = ohneNull.substring(1)` je Nullziffer) und `:403-404` — jedes `substring` ist eine volle Kopie. Der Riegel weist das Token danach ab; bezahlt ist der Aufwand trotzdem, und er ist wiederholbar. Kein Speicherfehler, kein Audioeinfluss — der Control-Thread steht, Quellen- und Telemetrieanzeige frieren ein, und `stop()` kann diesen Thread währenddessen nur ablösen statt joinen (`core/ipc/ControlClient.h:264-309`). · Erreichbar: ja, mit Peer-Vorbehalt (Kette: Broker → `ControlClient`-Thread → `PluginProcessor::v3Antwort` `src/PluginProcessor.cpp:1233` → `SourcesModel::uebernehmeSessionSnapshot` `src/SourcesModel.cpp:459` bzw. `uebernehmeEvidenzruecknahme` `:813` → `textriegelBytes` `NakamaVertrag.cpp:469` → `textriegel` `:474`). · Beleg: NakamaVertrag.cpp:487-493, 604, 621, 629, 668, 340-341, 403-404; juce_String.cpp:190-209, 296-299, 551-554, 747-763. · Register: neu

- [MEDIUM] vertrag/NakamaVertrag.cpp:474 · logik/caller-contract · Der Textriegel, der laut eigenem Vertrag „den ROHTEXT prüft, BEVOR ihn ein Parser sieht", hat keine Regel gegen Verschachtelungstiefe — beide Produktionsaufrufer geben den Text unmittelbar danach an `juce::JSON::parse`, dessen Parser ohne Tiefenschranke rekursiert. · Auslöser: ein Control-Frame, dessen Nutzlast nur aus öffnenden Klammern besteht (`[[[[…`); der Riegel läuft dort über `NakamaVertrag.cpp:675` (`++i`) durch und meldet „sauber", weil keine der neun Regeln (`NakamaVertrag.h:66-84`) Struktur oder Tiefe betrifft. Rund 10 KB Nutzlast reichen. · Wirkung: `juce_JSON.cpp:202/290/336` ruft `parseArray → parseAny → parseArray` ohne Tiefenzähler; bei 1 MiB Threadstack und 100-200 Byte je Rahmen kippt das nach wenigen tausend Ebenen in STATUS_STACK_OVERFLOW. Das ist kein fangbarer C++-Fehler — der Hostprozess stirbt mit ungesichertem Projekt. Der Gegensatz ist im Repo belegt: der EIGENE Leser des Projekts kappt bei 64 (`state/NakamaKanon.cpp:396,424`, `NakamaKanon.h:83`), der `juce::JSON`-Weg nicht; NAK-78 führt denselben Gegensatz nur für den Binärpfad (`state/NakamaState.cpp:353`). · Erreichbar: ja, mit Peer-Vorbehalt; Kette identisch bis `textriegelBytes`, der Absturz fällt eine Zeile später bei `src/SourcesModel.cpp:466` bzw. `:820`. · Beleg: NakamaVertrag.h:56-101 (neun Regeln, keine Tiefe), NakamaVertrag.cpp:474-680 (kein Tiefenzähler), NakamaVertrag.cpp:675; juce_JSON.cpp:202, 209-210, 290, 324, 336, 352. · Register: neu (NAK-78 nennt nur den Binärpfad)

- [MEDIUM] vertrag/NakamaTelemetrie.cpp:621 · logik (Ressourcenerschöpfung) · `pruefe` meldet `eintraege_zu_viele` und läuft danach trotzdem über JEDEN Eintrag; bei aliasierten FlatBuffers-`uoffset`s kaufen vier Eingabebytes eine volle Eintragsprüfung samt rund 50 Verstoßobjekten. · Auslöser: ein FeatureBatch von 262144 Byte, dessen `eintraege`-Vektor rund 65 000 Offsets auf DIESELBE, maximal fehlerhafte `QuellenEintrag`-Tabelle zeigt (der Verifier erlaubt Aliasing, und rund 5 Tabellenbesuche je Eintrag bleiben weit unter `max_tables` = 10^6). · Wirkung: rund 3,5·10^6 `Verstoss`-Objekte mit je zwei `juce::String` in `out` (`:566`), grob 400-500 MB flüchtiger Heap und Millionen Allokationen aus 256 KiB Eingabe (Verstärkung rund 1800×), danach `kanonisch` (`:549-558`) mit ebenso vielen `std::set`-Einfügungen. Die Klassifikation bleibt korrekt, es ist reine Erschöpfung. · Erreichbar: ja — und hier hat sich die Lage seit der Registerzeile geändert: NAK-116 hielt fest „`pruefe` hat heute keinen Produktionsaufrufer"; seit SONDE-012/013 gibt es ihn (`src/SourcesModel.cpp:959` ← `src/PluginProcessor.cpp:1277` `v3Frame` ← `TelemetryClient`-Thread), Peer-Vorbehalt wie oben. Der Broker hat dieselbe Auslassung (`broker/src/telemetrie.rs:368-392`), die Verstoßmenge bleibt also sprachgleich. · Beleg: NakamaTelemetrie.cpp:621-624 (Grenze wird nur gemeldet), 631-645 (Schleife läuft trotzdem), 549-558 (`kanonisch`), 133-136 (`hinzu`). · Register: NAK-150 (Ursprung NAK-116 `RESEXHAUST-003` / `G2-RESEXHAUST-001`) — nicht neu; neu ist nur die Erreichbarkeit

- [LOW] vertrag/NakamaVertrag.cpp:560 · logik/protokoll · Der Riegel akzeptiert zwei Escape-Formen, die das Rust-Bein ablehnt — genau die Sorte Beinunterschied, gegen die er laut `NakamaVertrag.h:66-84` gebaut ist. · Auslöser: (a) ein unbekanntes Escape in einer Zeichenkette, etwa das Paar Backslash+q in `"a\qb"` — `:555-561` überspringt Backslash plus Folgezeichen ungeprüft, und JUCEs Parser nimmt es über `default: break;` (`juce_JSON.cpp:186`) als `q` an, während serde_json und Python das Dokument verwerfen; (b) ein am Dokumentende abgeschnittenes u-Escape: die Bedingung `j + 5 < n` (`:512`) greift dann nicht, der Riegel fällt in denselben Zweig `:560` und meldet „sauber" (JUCE wirft danach zwar in `parseHexDigit`, das Urteil fällt aber an einem anderen Ort und mit anderer Wirkung — `ungueltig` gegen `ignoriert` in `SourcesModel`). · Wirkung: kein Speicherfehler; das C++-Bein ist an dieser Stelle das nachgiebigere, ein Feldwert, den der Broker verworfen hätte, kann bis in die Anzeige laufen. · Erreichbar: ja, dieselbe Kette und derselbe Peer-Vorbehalt. · Beleg: NakamaVertrag.cpp:510-513, 555-561; juce_JSON.cpp:157-193. · Register: neu

Zwei ältere Registerpunkte auf dieser Fläche sind an der Quelle geschlossen und
werden deshalb nicht erneut gemeldet: NAK-78 Punkt 3 („unbewachtes
`static_cast<int>(laenge)` im Textriegel") — heute deckt
`laenge > kMaxDokumentBytes` (`NakamaVertrag.cpp:444`, Konstante
`NakamaVertrag.h:29`) den Cast in `:469-470` ab, und das rohe NUL fällt in
`:459`. NAK-75 („`start_ppq`/`end_ppq` in beiden Lesern ungeprüft") — heute
geprüft in `NakamaTelemetrie.cpp:237-243` und in `broker/src/telemetrie.rs:812-825`.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren, Allokationen, IO und Logging** — hält, weil
   nicht berührt. Keine meiner Einheiten wird aus `processBlock` gerufen; der
   einzige produktive Weg ist Pipe-Thread → `PluginProcessor::v3Antwort`
   bzw. `v3Frame` (`src/PluginProcessor.cpp:1233, 1273`) → `SourcesModel` →
   `NakamaVertrag.cpp:474` bzw. `NakamaTelemetrie.cpp:650`.
   `core/ipc/ControlClient.h:14` hält ausdrücklich fest, dass dieser Client nie
   im Audiothread läuft. Die freien Allokationen in
   `NakamaTelemetrie.cpp:133-136` und `NakamaVertrag.cpp:487-493` sind dort
   deshalb zulässig — sie sind Ursache von Befund 1 und 3, nicht von Gate 1.
2. **Passthrough bitidentisch / Hörmarker** — nicht berührt. Keine Einheit
   schreibt in einen Hostpuffer oder liest `src/HoerMarkierung.h`.
3. **Gate 5 — kein Telemetrie- oder Steuerframe steuert samplegenauen Gain** —
   hält. `lese` (`NakamaTelemetrie.cpp:650-723`) erzeugt ausschließlich
   eigentümerhaltende Kopien: `->str()` für die vier Tokens (`:670-673`),
   Skalare für den Rest (`:674-718`); kein generierter FlatBuffers-Zeiger
   verlässt den Leser, wie `NakamaTelemetrie.h:90-94` zusagt. Die Zielstruktur
   `Empfangsframe` (`NakamaTelemetrie.h:95-132`) trägt weder Gain- noch
   Sampleoffsetfeld, und jedes Zahlenfeld hat ein eigenes Präsenzbit
   (`:112-131`) — es entsteht also auch keine erfundene 0.
4. **NaN-Ehrlichkeit** — hält. Nichtendliches wird verriegelt statt saniert:
   `sample_rate` `NakamaTelemetrie.cpp:200-203`, die fünfzehn optionalen
   Kennzahlen `:447-470`, `werte_f32` `:375-385`, Band-Stereo `:416-420`,
   Headroom `:537-539`; die Beziehungsregeln `:472-528` prüfen jeweils erst
   `isfinite`. Auf der Textseite rechnet `zahlPruefen` Betrag und
   Ganzzahligkeit ausschließlich lexikalisch (`NakamaVertrag.cpp:325-430`,
   Grenze |x| < 1e308 in `:362-366`), und `NaN`/`Infinity` fallen als
   alphabetische Literale (`:600-610`). Nyquist-Kappung liegt außerhalb dieser
   Einheiten.
5. **State bleibt verlustfrei** — nicht berührt. `textriegelBytes` ist im
   DTO-Weg nur ein Bytetor (`state/NakamaParameter.cpp:197`) und hat dort heute
   keinen Produktionsaufrufer; Originalbytes und Read-only-Haltung liegen in
   `state/NakamaState.cpp`.

Zusätzlich geprüft und ohne Befund: `NakamaUtf8.h:11-73` — die Fortsetzungs-
und Längenprüfungen sind an jeder der sieben Verzweigungen korrekt
(`i >= laenge` für zwei Byte, `i + 1 >= laenge` für drei, `i + 2 >= laenge` für
vier; C0/C1, F5..FF und nackte Fortsetzungsbytes fallen in `:70`), `i` bleibt
echt kleiner als `laenge`, und ein Überlauf von `i + 2` ist bei realen
Puffergrößen ausgeschlossen. Ebenso `NakamaTelemetrie.cpp:118-131` (`istHex32`
prüft `size() != 32` VOR dem 32-Byte-Lauf), `:344-358` (Bitmap: `Get(size()-1)`
nur im Zweig `size() == soll` mit `soll >= 1`; Shiftweite `anzahl % 8` liegt
dort in 1..7), `:409-412` (`bitmap->Get(i/8u)` ist mit `i / 8u < bitmap->size()`
bewacht) und `:603-619` (der UTF-8-Nachschlag läuft VOR `pruefeAdresse`,
deshalb kann `:166` keinen uninitialisierten Zähler lesen — `sidLaenge` ist
zudem mit 0 vorbelegt). Alle in `NakamaTelemetrie.cpp` ohne Nullprüfung
dereferenzierten Felder (`eintraege`, `quelle`, `frame`, `transport`,
`baender`, `gueltig_bitmap`) sind im Schema `required`
(`eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs:122-126, 204,
240-241, 323-324, 342`) und damit vom Verifier garantiert.

## Ledger

vertrag/NakamaTelemetrie.cpp:133-136 · clean
vertrag/NakamaTelemetrie.cpp:147-153 · clean
vertrag/NakamaTelemetrie.cpp:155-179 · clean
vertrag/NakamaTelemetrie.cpp:181-287 · clean
vertrag/NakamaTelemetrie.cpp:289-386 · clean
vertrag/NakamaTelemetrie.cpp:388-430 · clean
vertrag/NakamaTelemetrie.cpp:432-547 · clean
vertrag/NakamaTelemetrie.cpp:549-558 · Befund 3 (Nebenort von NAK-150)
vertrag/NakamaTelemetrie.cpp:564-648 · Befund 3 (NAK-150)
vertrag/NakamaTelemetrie.cpp:650-723 · clean
vertrag/NakamaTelemetrie.h:file-scope-1 · clean
vertrag/NakamaTelemetrie.h:34-142 · clean
vertrag/NakamaUtf8.h:file-scope-1 · clean
vertrag/NakamaUtf8.h:6-74 · clean
vertrag/NakamaVertrag.cpp:file-scope-1 · clean
vertrag/NakamaVertrag.cpp:31-37 · clean
vertrag/NakamaVertrag.cpp:39-42 · clean
vertrag/NakamaVertrag.cpp:44-47 · clean
vertrag/NakamaVertrag.cpp:49-61 · clean
vertrag/NakamaVertrag.cpp:63-67 · clean
vertrag/NakamaVertrag.cpp:78-91 · clean
vertrag/NakamaVertrag.cpp:93-101 · clean
vertrag/NakamaVertrag.cpp:103-114 · clean
vertrag/NakamaVertrag.cpp:128-137 · clean
vertrag/NakamaVertrag.cpp:145-145 · clean
vertrag/NakamaVertrag.cpp:147-152 · clean
vertrag/NakamaVertrag.cpp:154-158 · clean
vertrag/NakamaVertrag.cpp:160-164 · clean
vertrag/NakamaVertrag.cpp:166-170 · clean
vertrag/NakamaVertrag.cpp:172-175 · clean
vertrag/NakamaVertrag.cpp:181-230 · clean
vertrag/NakamaVertrag.cpp:232-250 · clean
vertrag/NakamaVertrag.cpp:256-259 · clean
vertrag/NakamaVertrag.cpp:261-271 · clean
vertrag/NakamaVertrag.cpp:278-285 · clean
vertrag/NakamaVertrag.cpp:287-290 · clean
vertrag/NakamaVertrag.cpp:292-295 · clean
vertrag/NakamaVertrag.cpp:320-431 · Befund 1 (Nebenort :340-341 und :403-404)
vertrag/NakamaVertrag.cpp:435-472 · clean
vertrag/NakamaVertrag.cpp:474-614 · Befund 1, Befund 2, Befund 4
vertrag/NakamaVertrag.cpp:615-680 · Befund 1 (Nebenort :668)
vertrag/NakamaVertrag.cpp:699-720 · clean
vertrag/NakamaVertrag.cpp:722-762 · clean
vertrag/NakamaVertrag.cpp:764-832 · clean
vertrag/NakamaVertrag.cpp:836-848 · clean
vertrag/NakamaVertrag.cpp:857-876 · clean
vertrag/NakamaVertrag.cpp:file-scope-2 · clean
vertrag/NakamaVertrag.cpp:888-905 · clean
vertrag/NakamaVertrag.cpp:907-932 · clean
vertrag/NakamaVertrag.cpp:934-969 · clean
vertrag/NakamaVertrag.cpp:971-1115 · clean
vertrag/NakamaVertrag.cpp:1116-1169 · clean
vertrag/NakamaVertrag.cpp:1173-1183 · clean
vertrag/NakamaVertrag.h:file-scope-1 · clean
vertrag/NakamaVertrag.h:23-149 · Befund 2 (Vertragstext :56-101)

(Die Zuweisung schreibt die Einheiten-IDs mit Backslash-Trenner; hier stehen
sie mit Schrägstrich, Reihenfolge und Zeilenbereiche sind unverändert.)

## Nicht gelesen

keine

## Außerhalb meiner Einheiten

- eq-copilot/plugin/src/SourcesModel.cpp:466 und :820 — hier steht der
  `juce::JSON::parse`-Aufruf, an dem der fehlende Tiefenriegel aus Befund 2
  tatsächlich kippt. Unbewertet.
- eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:20-27 — `Verstoss::operator<`
  ist `noexcept`, legt je Vergleich vier `std::string`-Kopien an und ist der
  `std::set`-Komparator in `kanonisch` (`:551-553`); eine gescheiterte
  Allokation endet dort in `std::terminate()`. Die Einheit beginnt erst bei
  Zeile 133. Unbewertet. Dieselbe Form in meiner Fläche
  (`NakamaVertrag.cpp:261-271`) ist heute testeigen und deshalb nicht als
  Befund geführt.
- eq-copilot/plugin/state/NakamaParameter.cpp:186 — `ausDtoText` hat heute
  keinen Produktionsaufrufer (nur Header `:79` und Tests), obwohl es der
  einzige Weg wäre, auf dem ein manipulierter State-Blob den Textriegel
  erreicht. Unbewertet.
- eq-copilot/plugin/state/NakamaKanon.cpp:396 und :424 — der eigene JSON-Leser
  des Projekts kappt bei Tiefe 64 und wird auf dem Sources-Weg nicht benutzt;
  Referenz für Befund 2. Unbewertet.
