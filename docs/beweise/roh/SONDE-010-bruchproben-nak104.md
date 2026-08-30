# Rohausgabe der Bruchproben — SONDE-010 NAK-104 (2026-08-30)

Jeder der vier Fixe wurde einmal absichtlich zurückgenommen; hier steht die rote
Ausgabe. **Alle vier Proben laufen gegen denselben committeten Stand `b4fe522`**
— je Probe wird genau EIN Fix zurückgenommen, gebaut, gefahren und sofort wieder
hergestellt. Deshalb trägt jede Ausgabe dieselbe Grundzahl (B10 207 Prüfungen),
und deshalb ist an jeder Ausgabe ablesbar, dass die drei ANDEREN Proben grün
bleiben: die Tests treffen wirklich je ihren eigenen Defekt.

Die Brüche liefen ausschließlich am Arbeitsbaum; committet ist allein der
gefixte Stand.

---

## B1 · `A-P2-03`/`A-P2-04` — die alte feste Schranke `kSlots * 2`

**Bruch:** in `P2Schleuse::veroeffentlichen` (`IpcQueues.h`) nach dem
Kollisionszähler wieder abbrechen, sobald `p + 1 - schreib >= kSlots * 2` ist —
genau die Fassung von `c72d51e`.

**Befehl:** `EqCopIpcTest.exe`

```text
  ok      Kollisionsloecher lassen den Verbraucher wirklich zurueckfallen (schreib - boden waechst)  [6 Positionen Rueckstand nach 5 Runden]
  ok      der Verbraucher ist zwischen JEDEM Versuch weitergerueckt — kein statisch freier dritter Platz  [6 Spruenge]
  FEHLER  und der Erzeuger bekommt TROTZDEM einen Platz — garantiert, nicht begrenzt versucht  [1 verworfen, 6 Loecher in dieser Veroeffentlichung]
  FEHLER  der neueste Frame liegt danach wirklich da, hinter allen Loechern  [64 Bytes]
== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  ok      Verbindung steht, und der Peer liest kein einziges Byte
  ok      ein vor dem Verbindungsende empfangener ACK geht nicht verloren — auch nach einem Write-Zeitlimit  [1 ACKs, empfangen=1, Pipe-Schreiben: Zeitlimit ueberschritten]
  ok      der neue Lauf steht NACHGEWIESEN — welcome geprueft, nicht nur ein Versuch gezaehlt
  ok      der zurueckkehrende alte Lauf trennt die neue Verbindung NICHT  [2 Versuche (vorher 2), 2 Serververbindungen (vorher 2), Status verbunden]
  ok      und der neue Lauf beantwortet danach weiter P0  [1 ACKs]
  ok      der neue Telemetrielauf steht nachgewiesen
  ok      dieselbe Zusage gilt fuer den TelemetryClient  [2 Versuche (vorher 2), 2 Serververbindungen (vorher 2), Status verbunden]
FEHLER — 207 Pruefungen, 2 Fehler
```

Die zwei `ok`-Zeilen vor dem Bruch sind die gemessene Vorbedingung: der
Rückstand wird durch Kollisionslöcher wirklich aufgebaut (sechs Positionen), und
der Verbraucher rückt zwischen JEDEM der sechs Versuche weiter. Ohne sie spräche
die Probe über eine Lage, die vielleicht gar nicht eingetreten ist.

---

## B2 · `B-CC-07` — das Write-Zeitlimit sperrt den Lesepfad

**Bruch:** in `IpcVerbindung::schreibenGenau` (`IpcVerbindung.cpp`) im Zeitlimit
wieder `ioAbbrechen()` statt `CancelIoEx (h, &ov)`.

**Befehl:** `EqCopIpcTest.exe`

```text
  ok      Kollisionsloecher lassen den Verbraucher wirklich zurueckfallen (schreib - boden waechst)  [6 Positionen Rueckstand nach 5 Runden]
  ok      der Verbraucher ist zwischen JEDEM Versuch weitergerueckt — kein statisch freier dritter Platz  [6 Spruenge]
  ok      und der Erzeuger bekommt TROTZDEM einen Platz — garantiert, nicht begrenzt versucht  [0 verworfen, 6 Loecher in dieser Veroeffentlichung]
  ok      der neueste Frame liegt danach wirklich da, hinter allen Loechern  [64 Bytes]
== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  ok      Verbindung steht, und der Peer liest kein einziges Byte
  FEHLER  ein vor dem Verbindungsende empfangener ACK geht nicht verloren — auch nach einem Write-Zeitlimit  [0 ACKs, empfangen=0, ]
  ok      der neue Lauf steht NACHGEWIESEN — welcome geprueft, nicht nur ein Versuch gezaehlt
  ok      der zurueckkehrende alte Lauf trennt die neue Verbindung NICHT  [2 Versuche (vorher 2), 2 Serververbindungen (vorher 2), Status verbunden]
  ok      und der neue Lauf beantwortet danach weiter P0  [1 ACKs]
  ok      der neue Telemetrielauf steht nachgewiesen
  ok      dieselbe Zusage gilt fuer den TelemetryClient  [2 Versuche (vorher 2), 2 Serververbindungen (vorher 2), Status verbunden]
FEHLER — 207 Pruefungen, 1 Fehler
```

`empfangen=0` ist der Kern: der ACK lag beim Client, aber `empfangenes (0)` bekam
sofort `LeseAusgang::fehler`, weil das Abbruchflag der ganzen Verbindung stand.
Auch `letzterFehler` ist leer — die Verbindung endete ohne Grund im Snapshot.

---

## B3 · `B-CC-12`/`B-TC-07` — eine Verbindung für alle Läufe

**Bruch:** in beiden Clients liefert `neueVerbindung()` wieder die bestehende
Verbindung statt einer frischen; im `TelemetryClient` zusätzlich die Prüfung nach
dem `helloProvider` entfernt. Das ist die Fassung von `c72d51e`.

**Befehl:** `EqCopIpcTest.exe`

```text
  ok      Kollisionsloecher lassen den Verbraucher wirklich zurueckfallen (schreib - boden waechst)  [6 Positionen Rueckstand nach 5 Runden]
  ok      der Verbraucher ist zwischen JEDEM Versuch weitergerueckt — kein statisch freier dritter Platz  [6 Spruenge]
  ok      und der Erzeuger bekommt TROTZDEM einen Platz — garantiert, nicht begrenzt versucht  [0 verworfen, 6 Loecher in dieser Veroeffentlichung]
  ok      der neueste Frame liegt danach wirklich da, hinter allen Loechern  [64 Bytes]
== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  ok      Verbindung steht, und der Peer liest kein einziges Byte
  ok      ein vor dem Verbindungsende empfangener ACK geht nicht verloren — auch nach einem Write-Zeitlimit  [1 ACKs, empfangen=1, Pipe-Schreiben: Zeitlimit ueberschritten]
  ok      der neue Lauf steht NACHGEWIESEN — welcome geprueft, nicht nur ein Versuch gezaehlt
  FEHLER  der zurueckkehrende alte Lauf trennt die neue Verbindung NICHT  [3 Versuche (vorher 2), 3 Serververbindungen (vorher 2), Status verbunden]
  ok      und der neue Lauf beantwortet danach weiter P0  [1 ACKs]
  ok      der neue Telemetrielauf steht nachgewiesen
  FEHLER  dieselbe Zusage gilt fuer den TelemetryClient  [3 Versuche (vorher 2), 3 Serververbindungen (vorher 2), Status verbunden]
FEHLER — 207 Pruefungen, 2 Fehler
```

Der Beleg ist die DRITTE Verbindung: der zurückkehrende alte Lauf hat die frische
Pipe getrennt, der neue Lauf musste sich neu verbinden. Dass er danach wieder
antwortet (`1 ACKs`) zeigt, warum eine reine Zustandsprobe zu schwach wäre — der
Rückfall heilt sich nach einem Backoff selbst und bleibt nur an den
Verbindungszahlen sichtbar. Beide Clients fallen einzeln.

---

## B4 · `C-LS-06` — Wartepunkt am Kopplungsregister statt am Trennstand

**Bruch:** in `server_v3.rs` liefert `kopplung_loesen` wieder `bool`, und der
Wartepunkt hängt wieder an `if ist_control && hatte_telemetrie`.

**Befehl:**
`cargo test --manifest-path broker/Cargo.toml --lib transport::server_v3::tests::telemetrie_endet_zuerst_control_getrennt_folgt_trotzdem`

```text
test transport::server_v3::tests::telemetrie_endet_zuerst_control_getrennt_folgt_trotzdem ... FAILED
thread 'transport::server_v3::tests::telemetrie_endet_zuerst_control_getrennt_folgt_trotzdem' (12732) panicked at src\transport\server_v3.rs:2930:9:
control_getrennt lief vor telemetrie_getrennt (oder telemetrie_getrennt fehlt ganz), obwohl die Telemetrie ihre Frist hielt: ["control_verbunden", "telemetrie_gekoppelt", "control_getrennt"]
test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 146 filtered out; finished in 0.81s
```

Das Protokoll `["control_verbunden", "telemetrie_gekoppelt", "control_getrennt"]`
ist der Befund selbst: `control_getrennt` steht darin, `telemetrie_getrennt`
fehlt noch — die Telemetrieseite saß zu diesem Zeitpunkt in ihrem
fristgerechten Trenn-Callback, hatte ihren Registereintrag aber schon geräumt.
