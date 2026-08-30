# Rohausgabe der Bruchproben — SONDE-010 NAK-104 Runde 2 (2026-08-30)

Ein Restdefekt aus der Wiederprüfung, ein Fix — und weil das Vorziehen der
Trennpflicht selbst eine neue Pflicht erzeugt (Zusage geben ↔ Zusage
zurücknehmen), zwei Brüche: einer je Hälfte des Änderungssatzes.

**Beide Proben laufen gegen denselben committeten Stand `e517165`** — je Probe
wird genau EIN Teil zurückgenommen, gebaut, gefahren und sofort wieder
hergestellt. Der Bruch lief ausschließlich am Arbeitsbaum; committet ist allein
der gefixte Stand. Deshalb trägt jede Ausgabe dieselbe Grundzahl (149 Rust-Lib-
Prüfungen), und an jeder ist ablesbar, dass die ANDERE Probe grün bleibt: die
zwei Tests treffen wirklich je ihre eigene Hälfte und nicht einen gemeinsamen
Nebeneffekt.

**Befehl (beide Proben):** `cargo test --manifest-path broker/Cargo.toml --lib`

---

## B1 · `C-LS-06` — die Trennpflicht erst NACH dem Kopplungs-Callback

**Bruch:** in `verbindung_bedienen` (`broker/src/transport/server_v3.rs`,
Zweig `Bootstrap::V3Telemetry`) den Aufruf
`trennmelder_telemetrie_erwartet(&trennmelder, &h.link_id)` wieder HINTER den
`senkenruf.rufen(… telemetrie_gekoppelt …)`-Block schieben — genau die Fassung
von `cab288b`.

```text
running 149 tests
test transport::server_v3::tests::trennreihenfolge_haelt_im_kopplungs_callback_fenster ... FAILED

failures:

---- transport::server_v3::tests::trennreihenfolge_haelt_im_kopplungs_callback_fenster stdout ----

thread 'transport::server_v3::tests::trennreihenfolge_haelt_im_kopplungs_callback_fenster' (21508) panicked at src\transport\server_v3.rs:3051:9:
control_getrennt lief vor telemetrie_getrennt (oder telemetrie_getrennt fehlt ganz), obwohl beide Clients im Kopplungs-Callback endeten: ["control_verbunden", "control_getrennt"]

failures:
    transport::server_v3::tests::trennreihenfolge_haelt_im_kopplungs_callback_fenster

test result: FAILED. 148 passed; 1 failed; 0 ignored; 0 measured; 0 filtered out; finished in 2.32s
```

Das Log der Senke ist der eigentliche Befund: `["control_verbunden",
"control_getrennt"]` — die Kopplung endet, ohne dass ihr Telemetrieteilnehmer je
ein Ende gemeldet hätte. Das `telemetrie_getrennt` läuft erst danach, wenn der
800-ms-Block der Senke ausgelaufen ist. Genau diese Reihenfolge verbietet
`C-LS-06`.

Die beiden gemessenen Vorbedingungen davor (`betreten_anzahl
("telemetrie_gekoppelt") == 1` und `anzahl("telemetrie_gekoppelt") == 0`) halten
in dieser Probe: der Test ist wirklich IM Fenster gescheitert und nicht daneben.

**Der Vorgängertest bleibt grün.** `telemetrie_endet_zuerst_control_getrennt_
folgt_trotzdem` (NAK-104 Runde 1) steht unter den 148 bestandenen — er wartet,
bis `telemetrie_gekoppelt` durch ist, und betritt das Fenster nie. Deshalb war
der Defekt nach Runde 1 noch da.

---

## B2 · `C-LS-04`/`C-LS-06` — die vorgezogene Zusage ohne Rücknahme

**Bruch:** dieselbe Stelle mit dem NEUEN Setzpunkt, aber ohne
`trennmelder_telemetrie_abgesagt(&trennmelder, &h.link_id)` im Fehlerzweig eines
abgelösten `telemetrie_gekoppelt`.

```text
running 149 tests
test transport::server_v3::tests::abgeloestes_telemetrie_gekoppelt_laesst_control_getrennt_nicht_warten ... FAILED

failures:

---- transport::server_v3::tests::abgeloestes_telemetrie_gekoppelt_laesst_control_getrennt_nicht_warten stdout ----

thread 'transport::server_v3::tests::abgeloestes_telemetrie_gekoppelt_laesst_control_getrennt_nicht_warten' (10116) panicked at src\transport\server_v3.rs:3117:9:
control_getrennt kam erst nach 2.0068661s — es wartet auf ein telemetrie_getrennt, das nach der Abloesung nie kommt (SENKE_FRIST 2s)

failures:
    transport::server_v3::tests::abgeloestes_telemetrie_gekoppelt_laesst_control_getrennt_nicht_warten

test result: FAILED. 148 passed; 1 failed; 0 ignored; 0 measured; 0 filtered out; finished in 4.11s
```

`2.0068661s` ist die Aussage: die Control-Seite klebt exakt an der vollen
`SENKE_FRIST` (2 s) und zählt danach eine Reihenfolgeverletzung, die keine ist —
das Gegenstück kann nach der Ablösung gar nicht mehr kommen. Ohne diesen Bruch
sähe niemand, dass die Rücknahme trägt; sie ist der Preis dafür, die Zusage
vorzuziehen. Der Compiler warnt in dieser Probe zusätzlich
`function 'trennmelder_telemetrie_abgesagt' is never used` — auch das gehört zum
Bild des Bruchs.

**Die Fensterprobe bleibt grün.** `trennreihenfolge_haelt_im_kopplungs_callback_
fenster` steht unter den 148 bestandenen: der Fix aus B1 wirkt hier weiter, nur
seine Rücknahme fehlt.

---

## Grün — derselbe Stand, nichts zurückgenommen

**Befehl:** `cargo test --manifest-path broker/Cargo.toml`

```text
running 149 tests
test result: ok. 149 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 2.31s

running 9 tests
test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.16s

running 9 tests
test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.18s
```

(Die vier leeren Ziele mit `0 tests` — die drei Binaries und der Doctest-Lauf —
sind weggelassen; sie tragen keine Prüfung.)
