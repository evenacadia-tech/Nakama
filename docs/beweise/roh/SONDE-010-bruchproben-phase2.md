# Rohausgabe der Bruchproben — SONDE-010 Ursachenrunde Phase 2 (2026-08-30)

Jede in dieser Runde geänderte Zusage wurde einmal absichtlich gebrochen; hier
steht die rote Ausgabe. Basis der Brüche: `7457684`. Nach jeder Probe wurde der
Bruch zurückgenommen und der grüne Stand erneut gefahren (B10 188/188,
Rust 146 + 9 + 9).

Die Brüche wurden am Arbeitsbaum gefahren und danach zurückgenommen; committet
ist ausschließlich der gefixte Stand.

---

## Rust — sechs Brüche, je ein eigener Test

```text
--- R1 Regel 3 (A-P1-05): voller Wiederholpuffer verdraengt wieder ---
test transport::warteschlange::tests::p1_haelt_ereignisse_fuer_den_reconnect_vor ... FAILED
test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 145 filtered out; finished in 0.00s
thread 'transport::warteschlange::tests::p1_haelt_ereignisse_fuer_den_reconnect_vor' (21780) panicked at src\transport\warteschlange.rs:445:9:
assertion `left == right` failed

--- R2 Regel 1 (A-P1-06): Abfluss nur noch beim Reconnect ---
test transport::warteschlange::tests::p1_wiederholpuffer_fliesst_ohne_reconnect_ab ... FAILED
test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 145 filtered out; finished in 0.00s
thread 'transport::warteschlange::tests::p1_wiederholpuffer_fliesst_ohne_reconnect_ab' (25188) panicked at src\transport\warteschlange.rs:470:9:
assertion `left == right` failed: genau eine Wiederholung ist nachgerueckt

--- R3 Regel 2 (A-P1-03): Schluessel ueberlebt den Wiederholpuffer nicht ---
test transport::warteschlange::tests::p1_wiederholpuffer_haelt_den_schluessel ... FAILED
test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 145 filtered out; finished in 0.00s
thread 'transport::warteschlange::tests::p1_wiederholpuffer_haelt_den_schluessel' (4776) panicked at src\transport\warteschlange.rs:497:9:
assertion `left == right` failed

--- R4 A-IN-04: P1 faellt bei vollem Ingress wieder still ---
test transport::warteschlange::tests::ingress_voll_ohne_p2_trennt_auch_bei_p1 ... FAILED
test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 145 filtered out; finished in 0.00s
thread 'transport::warteschlange::tests::ingress_voll_ohne_p2_trennt_auch_bei_p1' (17720) panicked at src\transport\warteschlange.rs:550:9:
assertion `left == right` failed

--- R5 Regel 5 (C-LS-02): Welcome wieder VOR control_verbunden ---
test transport::server_v3::tests::welcome_folgt_dem_abgeschlossenen_control_verbunden ... FAILED
thread 'transport::server_v3::tests::welcome_folgt_dem_abgeschlossenen_control_verbunden' (23436) panicked at src\transport\server_v3.rs:2661:9:
das Welcome kam schon nach 440.9µs — also VOR dem abgeschlossenen control_verbunden (die Senke haelt es 400 ms)
test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 145 filtered out; finished in 0.41s

--- R6 Regel 5 (C-LS-06): kein Wartepunkt vor control_getrennt ---
test transport::server_v3::tests::trennreihenfolge_je_callback_genau_einmal ... FAILED
thread 'transport::server_v3::tests::trennreihenfolge_je_callback_genau_einmal' (1800) panicked at src\transport\server_v3.rs:2753:9:
control_getrennt kam vor telemetrie_getrennt: ["control_verbunden", "telemetrie_gekoppelt", "control_getrennt", "telemetrie_getrennt"]
test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 145 filtered out; finished in 0.40s
```

---

## C++ — fünf Brüche in EINEM Bau, elf rote Prüfungen in B10

Gebrochen wurden gleichzeitig:

* **C1** `A-P1-06`: `P1Warteschlange::bestaetigen()` ohne `abfliessen()`;
* **C2** `A-P1-03`: die Schlüsselsuche im Wiederholpuffer entfernt;
* **C3** `A-P2-04`: die Platzsuche wieder auf zwei Versuche begrenzt;
* **C4** `B-CC-06`: das alte `continue` am Lesepfad vorbei;
* **C5** `B-CC-12`: `kStopFristMs` von 2000 auf 60000.

```text
  FEHLER  ein einziges Senden zieht eine Wiederholung nach — ohne Reconnect  [2 noch vorgehalten]
  FEHLER  beim Reconnect ist nichts mehr nachzuholen — es floss schon ab
  FEHLER  und zwar JEDES angenommene Ereignis, in der urspruenglichen Reihenfolge  [a2,ereignis]
  FEHLER  und er ueberholt sie nicht — Annahmereihenfolge ueber beide Puffer  [2,]
  FEHLER  ein Snapshot im Wiederholpuffer behaelt seinen Schluessel und koalesziert dort  [2 vorgehalten, 0 abgewiesen]
  FEHLER  und der NEUERE geht raus, nicht der aeltere  [fremd,]
  FEHLER  erzwungene Slot-Kollision: der neueste Frame findet immer einen Platz  [1 verworfen, 2 Loecher]
  FEHLER  der Verbraucher ueberspringt die Loecher und bekommt genau diesen Frame  [0 Bytes]
  FEHLER  der ACK erreicht beiAntwort, obwohl P1 rueckstaut  [0 ACKs, 1 P1 gesendet, ]
  FEHLER  stop() kehrt trotz blockierendem Callback binnen Frist zurueck  [20000 ms]
  FEHLER  und die Fristueberschreitung ist sichtbar gezaehlt  [0]
FEHLER — 188 Pruefungen, 11 Fehler
```

Bemerkenswert an **C3**: `beanspruchtVerworfen` steigt auf 1, obwohl die alte
Fassung diesen Fall im Kommentar als „unerreichbar" führte. Genau das ist
NAK-98 — nur eben deterministisch statt unter Baulast.

Bemerkenswert an **C5**: `stop()` steht 20 000 ms (die Selbstbegrenzung des
Testcallbacks), statt nach 2000 ms abzulösen; der Zähler bleibt 0.

---

## Grün nach der Rücknahme

```text
ALLE PRUEFUNGEN GRUEN — 188 Pruefungen, 0 Fehler
test result: ok. 146 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out
running 9 tests (contract_cross_language) ... ok
running 9 tests (transport_fuzz) ... ok
```

---

## Nachtrag Selbstaudit: der Laufnummern-Riegel

Nach dem Kanon fiel beim adversarialen Lesen auf, dass ein nach `kStopFristMs`
ABGELOESTER Thread bei einem erneuten `start()` `laeuft == true` wiedersaehe und
auf derselben `Laufzeit` weiterliefe — zwei Threads auf einer Pipe. Beide
Clients tragen deshalb eine Laufnummer (`Laufzeit::lebenslauf`); jeder Lauf
endet, sobald sie nicht mehr die aktuelle ist, und nur der aktuelle Lauf setzt
`fertig`. Die Probe steht in B10 G17 Fall (2b).

Bruch (Laufnummer aus der Schleifenbedingung entfernt):

```text
  FEHLER  und zwar GENAU EIN neuer Lauf — der abgeloeste Vorgaenger faehrt nicht daneben weiter  [3 Versuche, 1 Fristueberschreitungen]
FEHLER — 192 Pruefungen, 1 Fehler
```

Nach der Ruecknahme:

```text
  ok      nach dem Abloesen verbindet ein neuer start() wieder  [2 Versuche]
  ok      und zwar GENAU EIN neuer Lauf — der abgeloeste Vorgaenger faehrt nicht daneben weiter  [2 Versuche, 1 Fristueberschreitungen]
ALLE PRUEFUNGEN GRUEN — 192 Pruefungen, 0 Fehler
```
