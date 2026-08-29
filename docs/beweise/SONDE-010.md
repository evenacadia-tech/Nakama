# Beweismanifest — SONDE-010 «v3-Control-/Telemetry-Clients und Rust-Envelopeparser»

<!-- NAKAMA-URTEIL: T1 PASS 2026-08-29 -->

| Feld | Wert |
|---|---|
| Ticket | `SONDE-010` |
| Phase / Session | P2 / S14–15 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §65, Zeile `SONDE-010`: **„v3-Control-/Telemetry-Clients und Rust-Envelopeparser — fertig, wenn: CRC/Fuzz/Backpressure/Reconnect ohne P0-Starvation"** |
| Commits | `cdff93b` · `2ac23d0` · `1b19cd1` · `10a4806` · `97c956d` (Basis `a7b0740`) |
| Datum | 2026-08-29 |
| Prüfstufen | T1 ☑ · T2 ☐ · T3 ☐ (kein Gate) |

> Das T1-Häkchen steht **nach** dem Lauf, nicht davor. T2 setzt später ein
> frischer Prüfer; hier steht bewusst keine T2-Marke.

---

## 0. Was gebaut wurde — und was ausdrücklich nicht

**Gebaut.** Der v3-Nachrichtenweg in beiden Sprachen: der 16-Byte-Envelope
(§33.1) als reine Funktion über Bytes in Rust *und* C++, der Bootstrap mit der
Kopplung zweier Verbindungen (§53.9), die vier Backpressure-Politiken, der
Pipename als reine Funktion mit dem Golden aus §48.3, und die beiden
C++-Clients `ControlClient` (P0/P1) und `TelemetryClient` (P2) mit eigenen
Threads, Reconnect-Backoff und vorallokierter SPSC-Übergabe.

**Nicht gebaut, bewusst.** Der `Coordinator` — Session, Eviction, Store,
Outbox — ist `SONDE-011`. Und: **Gen und Probeeq sprechen weiterhin v2.** Der
heutige Heartbeat-Pfad über `plugin/src/PipeClient` bleibt unangetastet der
Produktpfad; die neuen Clients sind gebaut, gelinkt und bewiesen, ihre
Produktverdrahtung folgt mit Broker-Coordinator und Landkarte
(`SONDE-011`/`SONDE-012`). Kein toter Code: das Kanon-Bein **B10** linkt und
fährt genau diesen Produktionsquellpfad, und **A22** fährt ihn über eine echte
Pipe gegen den echten Rust-Listener.

### Die eine technische Entscheidung, die das Ticket offen ließ

> „ob der Broker die SID-gebundene v3-Pipe bereits produktiv zusätzlich öffnet,
> entscheidest du technisch und begründest es im Manifest"

**Entscheidung: nein, in diesem Ticket nicht.** Ein zusätzlicher, SID-gebundener
v3-Endpunkt in `eqcop-broker.exe` wäre heute ein Endpunkt, der Verbindungen
annimmt, sein `welcome` schickt — und danach nichts damit anfangen kann: ohne
Coordinator gibt es keine Session, kein Register, keinen Store. Das ist genau
die Sorte Element, die das Grundgesetz „keine toten Elemente" verbietet, eine
Ebene unter der Oberfläche. Stattdessen:

* `transport::pipetoken` liegt fertig, ist eine reine Funktion und trägt das
  Golden aus §48.3 **in beiden Sprachen**;
* der Listener (`transport::server_v3`) ist vollständig gebaut und wird über
  einen **Probe-Namen** gefahren — nie über die Produktions-Pipe. Beide
  Programme des Lastbeins verweigern den Dienst, wenn ihnen
  `\\.\pipe\evenacadia.eq-copilot.v1` genannt wird;
* v2-Pipe, v2-Verhalten und Pipe-Sicherheit (SDDL nur aktueller User,
  `PIPE_REJECT_REMOTE_CLIENTS`, `FILE_FLAG_FIRST_PIPE_INSTANCE`) sind
  unverändert — der v3-Listener benutzt **dieselben** Sicherheitshelfer aus
  `server.rs`, damit es nur eine Wahrheit über die Pipe-Sicherheit gibt.

`SONDE-011` schaltet den Endpunkt zusammen mit dem Coordinator ein.

---

## 1. Ticket-Behauptungen

| # | Behauptung (Gate-Text §65 / Lieferumfang) | Befehl | Ergebnis | Rohausgabe |
|---|---|---|---|---|
| 1 | **CRC** — CRC32C trifft die RFC-3720-Vektoren; P0/P1 tragen exakt 0, P2 die Pflichtsumme über genau die `payload_len` Payloadbytes; ein gekipptes Bit fällt. In **beiden** Sprachen. | `EqCopIpcTest` · `cargo test --lib transport::` | ✅ | [↓ B1](#b1), [↓ B2](#b2) |
| 2 | **Ein Korpus, zwei Leser** — C++ und Rust klassifizieren 37 Envelope-Fixtures identisch (Urteil **und** Verstoßmenge); jede der 14 Regeln hat ein Negativfixture. | `cargo test --test contract_cross_language envelope` · `EqCopIpcTest` | ✅ | [↓ B3](#b3), [↓ B1](#b1) |
| 3 | **Fuzz** — Längen, Header, Flags, CRC, Fragmentierung, Ratengrenzen; auch byteweise zugestellte Frames. | `cargo test --test transport_fuzz` | ✅ | [↓ B4](#b4) |
| 4 | **Bootstrap und Kopplung** — v2 bleibt v2; `protocol=3` bekommt ein v3-gerahmtes `welcome{link_id, challenge}`; die Telemetrieverbindung koppelt nur mit allen drei Merkmalen; ungekoppelter Connect und Binärframe statt Hello werden geschlossen. **v2-Isolation in beide Richtungen.** | `cargo test --lib transport::` · `EqCopIpcTest` | ✅ | [↓ B2](#b2), [↓ B1](#b1) |
| 5 | **Backpressure** — P0 64 / nichts verwerfen, P1 128 / koaleszieren + Wiederholpuffer, P2 je Sonde 2 / replace-oldest, Ingress 256 / P2 zuerst. Übergabe zur Telemetrie vorallokiert und ohne Allokation. | `EqCopIpcTest` · `cargo test --lib transport::` | ✅ | [↓ B1](#b1), [↓ B2](#b2) |
| 6 | **Reconnect** — nach Broker-Neustart verbindet der Client von selbst wieder; ein kaputter Envelope und ein P0-Überlauf schließen die Verbindung, statt still zu kürzen. | `EqCopIpcTest` | ✅ | [↓ B1](#b1) |
| 7 | **Pipe-Token** — `S-1-5-21-111111111-222222222-333333333-1001 → BNSM62JZZCCXIDV3PJZAEHMZPA` in C++ **und** Rust. | `EqCopIpcTest` · `cargo test --lib transport::` | ✅ | [↓ B1](#b1), [↓ B2](#b2) |
| 8 | **„ohne P0-Starvation" als Zahl** — 32 Sonden fluten P2 über eine Probe-Pipe gegen den echten Rust-Listener; **10 048 von 10 048** P0-Frames beantwortet, max. **21 ms**, p99 **20 ms**, bei **40 800** ersetzten P2-Frames. | `py -3.13 tools/eq-copilot/pruefe_ipc_last.py` | ✅ | [↓ B5](#b5) |
| 9 | **Jedes neue Bein schlägt an** — A20, A21, A22 und B10 einmal absichtlich gebrochen. | siehe unten | ✅ | [↓ B6](#b6) |

---

## 2. Rohe Ausgaben

<a id="b1"></a>
### B1 · C++-Bein B10 — `EqCopIpcTest`

**Befehl:** `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` · **Datum:** 2026-08-29

```text
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  ok      MANIFEST.json gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\v3\envelope\MANIFEST.json]
  ok      Regelmenge und ihre Reihenfolge stimmen mit dem Manifest  [14 Regeln]
  ok      die SAMMELNDE Stufe 3 ist in Deklarationsreihenfolge namenssortiert
  ok      alle Fixtures klassifiziert wie das Manifest (Urteil UND Verstossmenge)  [8 gueltig / 29 ungueltig]
  ok      Korpus ist substanziell  [37 Fixtures]
  ok      jede der 14 Regeln hat mindestens ein Negativfixture  [14/14]

== B · CRC32C und Envelope-Rundlauf ==
  ok      CRC32C("") == 0
  ok      CRC32C("123456789") == 0xE3069283 (RFC 3720)
  ok      CRC32C(32x 0x00) == 0x8A9136AA
  ok      CRC32C(32x 0xFF) == 0x62A8AB43
  ok      P0-Frame geschrieben
  ok      P0 ist JSON und traegt CRC exakt 0
  ok      P2 traegt die Pflicht-CRC ueber genau die Payloadbytes
  ok      ein gekipptes Payloadbit faellt an der CRC
  ok      Payload an der Paketgrenze wird angenommen
  ok      ein Byte darueber wird nicht einmal geschrieben

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  ok      kein angenommener Zufallsframe verletzt eine Kopfregel  [0 angenommen / 40000 abgelehnt]
  ok      ein gekipptes Bit in einem GUELTIGEN Frame: was durchkommt, haelt jede Kopfregel  [7671 angenommen / 12329 abgelehnt]
  ok      3000 gekippte P2-Payloadbits: jedes einzelne faellt an der CRC
  ok      byteweise Zustellung liefert exakt dieselben 40 Frames  [40 Frames, Rest 0]
  ok      ein kaputter Frame im Strom wird gemeldet, nicht uebersprungen

== D · v3-Pipename als reine Funktion ==
  ok      Golden aus Entwurf §48.3
  ok      vollstaendiger Pipename
  ok      Schreibweise der SID ist egal (Grossschreibung vor dem Hashen)
  ok      verschiedene SIDs, verschiedene Token
  ok      SHA-256("abc") wie FIPS 180-4
  ok      SHA-256 des 56-Byte-Vektors (Zweiblockfall)
  ok      Base32 gegen die Testvektoren aus RFC 4648 §10
  ok      eigene Logon-SID ist wohlgeformt  [S-1-5-21-369263037-1611576211-2081231688-1001]

== E · Backpressure: P0 · P1 · P2 ==
  ok      P0 nimmt 64 Nachrichten
  ok      die 65. meldet Ueberlauf (nichts verwerfen ⇒ Verbindung schliessen)
  ok      alle 64 kommen unveraendert und in Reihenfolge zurueck
  ok      P1 nimmt Snapshot a
  ok      P1 nimmt Ereignis
  ok      P1 nimmt Snapshot b
  ok      ein zweiter Snapshot desselben Objekts koalesziert, ohne zu wachsen
  ok      Koaleszieren behaelt die Position, tauscht nur den Inhalt
  ok      nicht koaleszierbare Ereignisse gehen in den Wiederholpuffer
  ok      ein voller Wiederholpuffer verdraengt gezaehlt, nie still
  ok      Reconnect holt beide zurueck
  ok      und zwar in der urspruenglichen Reihenfolge

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  ok      der dritte Frame ersetzt den aeltesten ungesendeten (Cap 2)
  ok      abgeholt wird der zweite, nicht der erste  [2]
  ok      danach der dritte
  ok      danach ist die Schleuse leer
  ok      ein Frame ueber der Slotgroesse wird gezaehlt verworfen, nie halb geschrieben
  ok      100 000 Uebergaben mit 0 Allokationen  [0]
  ok      Gegenprobe: derselbe Zaehler sieht eine echte Allokation  [1]
  ok      unter Flut ist kein abgeholter Frame zerrissen  [29236 geholt, 0 zerrissen, 197342 ersetzt]

== F · Ratengrenze ==
  ok      5000 Nachrichten in derselben Millisekunde: 1000 gehen durch  [1000]
  ok      ein volles Fenster spaeter wieder 1000
  ok      gleitend: 1 Nachricht je 10 ms laeuft unbegrenzt

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  ok      Control verbindet und bekommt ein v3-gerahmtes welcome
  ok      welcome liefert link_id und challenge
  ok      Telemetry koppelt mit link_id + challenge + derselben runtime_nonce
  ok      P0 kommt beim Server als P0-Familie an
  ok      und die Antwort erreicht den Client
  ok      P1 kommt als P1-Familie an
  ok      P2 fliesst ueber die zweite Verbindung  [20 Frames]
  ok      beide Bootstrap-Hellos tragen dieselbe runtime_nonce
  ok      das Bootstrap-Hello ist v3 und nicht v2
  ok      stop() trennt beide Verbindungen und kehrt zurueck

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  ok      der Server weist die ungekoppelte Telemetrieverbindung ab
  ok      der Client gilt nicht als verbunden

== G3 · Reconnect nach Serverneustart ==
  ok      erste Verbindung steht
  ok      nach dem Serverende faellt der Client auf getrennt
  ok      der Client verbindet von selbst wieder (Backoff 500..8000 ms)
  ok      und zaehlt die Versuche ehrlich mit  [2]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  ok      der Client weist den Frame ab und verbindet nicht
  ok      Status bleibt ungleich 'verbunden'

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  ok      64 P0-Nachrichten passen
  ok      die 65. meldet Ueberlauf an den Aufrufer
  ok      der Ueberlauf ist gezaehlt
  ok      Verbindung steht (der Server liest nur nicht mehr)
  ok      ein 200-KiB-Befehl wird eingereiht
  ok      die Queue laeuft ueber, weil der Peer nicht liest
  ok      die Verbindung wird deswegen geschlossen, nicht stillschweigend gekuerzt  [P0-Ueberlauf: Verbindung wird geschlossen]

== H · Bootstrapgrenze und JSON-Riegel ==
  ok      16 KiB sind noch ein Bootstrap-Hello
  ok      ein Byte darueber nicht mehr
  ok      flaches Objekt wird gelesen
  ok      Zahlen kommen als Text zurueck
  ok      Verschachtelung wird ABGELEHNT, nicht geraten
  ok      Arrays ebenso
  ok      ein Escape wird abgelehnt statt interpretiert
  ok      doppelter Schluessel ist eine Ablehnung, keine 'letzter gewinnt'-Regel
  ok      Text hinter dem Objekt wird abgelehnt
  ok      das leere Objekt ist gueltig

ALLE PRUEFUNGEN GRUEN — 87 Pruefungen, 0 Fehler
EXIT_B10=0
```

<a id="b2"></a>
### B2 · Rust-Unittests des Transports

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --lib transport:: --color never` · **Datum:** 2026-08-29

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)

running 32 tests
test transport::bootstrap::tests::hex32_riegel ... ok
test transport::bootstrap::tests::binaerframe_statt_hello_wird_abgelehnt ... ok
test transport::bootstrap::tests::hello_ueber_16_kib_faellt_vor_dem_parser ... ok
test transport::legacy_v2::tests::v2_json_ist_kein_gueltiger_v3_envelope ... ok
test transport::legacy_v2::tests::echtes_v2_json_geht_weiterhin_durch ... ok
test transport::bootstrap::tests::offene_kopplungen_sind_gedeckelt ... ok
test transport::bootstrap::tests::v3_control_wird_vollstaendig_validiert ... ok
test transport::bootstrap::tests::unbekanntes_feld_im_hello_faellt ... ok
test transport::bootstrap::tests::koppeln_und_entkoppeln_sind_symmetrisch ... ok
test transport::legacy_v2::tests::v3_binaerframe_wird_nie_als_v2_json_gedeutet ... ok
test transport::bootstrap::tests::v2_hello_bleibt_v2 ... ok
test transport::bootstrap::tests::kopplung_verlangt_alle_drei_merkmale ... ok
test transport::pipetoken::tests::base32_gegen_rfc_4648 ... ok
test transport::pipetoken::tests::golden_aus_dem_entwurf ... ok
test transport::pipetoken::tests::schreibweise_der_sid_ist_egal ... ok
test transport::pipetoken::tests::tokenform_ist_stabil ... ok
test transport::pipetoken::tests::verschiedene_sids_verschiedene_token ... ok
test transport::v3::tests::crc32c_gegen_bekannte_vektoren ... ok
test transport::v3::tests::grenzen_des_rahmens ... ok
test transport::v3::tests::jeder_regelname_ist_rueckuebersetzbar ... ok
test transport::v3::tests::json_rundlauf ... ok
test transport::v3::tests::p2_traegt_pflicht_crc ... ok
test transport::v3::tests::ratengrenze_haelt_das_fenster ... ok
test transport::v3::tests::stromleser_vertraegt_byteweise_zustellung ... ok
test transport::v3::tests::stufe_drei_sammelt_und_sortiert ... ok
test transport::v3::tests::u32_ueberlauf_wird_nicht_akzeptiert ... ok
test transport::warteschlange::tests::ingress_droppt_p2_zuerst_und_trennt_bei_p0 ... ok
test transport::warteschlange::tests::p0_verwirft_nie_und_meldet_ueberlauf ... ok
test transport::warteschlange::tests::p1_haelt_ereignisse_fuer_den_reconnect_vor ... ok
test transport::warteschlange::tests::p1_koalesziert_snapshots_an_ihrer_position ... ok
test transport::warteschlange::tests::p2_ersetzt_den_aeltesten_ungesendeten ... ok
test transport::warteschlange::tests::p2_flut_hungert_p0_nicht_aus ... ok

test result: ok. 32 passed; 0 failed; 0 ignored; 0 measured; 90 filtered out; finished in 0.00s

EXIT=0
```

<a id="b3"></a>
### B3 · Envelope-Korpus in `contract_cross_language`

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --test contract_cross_language envelope --color never` · **Datum:** 2026-08-29

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-0cc75d7ee20a9bba.exe)

running 1 test
test envelope_korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 1 passed; 0 failed; 0 ignored; 0 measured; 8 filtered out; finished in 0.02s

EXIT=0
```

<a id="b4"></a>
### B4 · `transport_fuzz`

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` · **Datum:** 2026-08-29

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.07s
     Running tests\transport_fuzz.rs (broker\target\debug\deps\transport_fuzz-c05695697064a160.exe)

running 9 tests
test ein_kaputter_frame_im_strom_beendet_den_strom ... ok
test feindliche_laengen_loesen_keine_allokation_aus ... ok
test ratengrenze_haelt_unter_flut ... ok
test zufallsbytes_bringen_den_pruefer_nie_aus_dem_tritt ... ok
test jedes_gekippte_payloadbit_eines_p2_frames_faellt_an_der_crc ... ok
test payload_an_der_paketgrenze_wird_angenommen_darueber_nicht ... ok
test schreiben_und_pruefen_sind_zueinander_invers ... ok
test fragmentierung_aendert_das_ergebnis_nie ... ok
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.15s

EXIT=0
```

<a id="b5"></a>
### B5 · Ende-zu-Ende-Lastbein A22 — zwei Sprachen, ein Draht

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_ipc_last.py` · **Datum:** 2026-08-29

Der Aufbau: `eqcop-broker-v3probe.exe` öffnet den echten Rust-Listener auf einer
Probe-Pipe; `EqCopIpcLast.exe` fährt 32 **echte** `ControlClient`/
`TelemetryClient`-Paare dagegen. Ein Erzeugerthread flutet alle 32 P2-Schleusen,
ein zweiter sendet jeder Sonde alle 25 ms einen `heartbeat` und misst die Zeit
bis zum `heartbeat_ack`.

```text
Probe-Pipe: \\.\pipe\evenacadia.nakama.v3.last.22332.1787977039
  BEREIT \\.\pipe\evenacadia.nakama.v3.last.22332.1787977039
SONDE-010 | Lastbein: 32 Sonden, 8 s, Pipe \\.\pipe\evenacadia.nakama.v3.last.22332.1787977039
  verbunden: 32 Control, 32 Telemetrie
{"sonden":32,"sekunden":8,"p2_veroeffentlicht":133568,"p2_gesendet":92903,"p2_ersetzt":40800,"p0_gesendet":10048,"p0_beantwortet":10048,"p0_latenz_max_ms":21,"p0_latenz_p99_ms":20,"p0_ueberlaeufe":0,"envelope_abweisungen":0}
  ok      P0-Befehle wurden ueberhaupt gesendet  [10048]
  ok      KEIN P0-Frame geht verloren  [10048/10048]
  ok      P0-Antwortlatenz bleibt beschraenkt  [max 21 ms, p99 20 ms, Schranke 1000 ms]
  ok      es lag WIRKLICH Rueckstau an (Cap 2 hat ersetzt)  [40800 ersetzte P2-Frames]
  ok      kein Envelope wurde abgewiesen  [0]
  ok      keine P0-Queue lief ueber  [0]
LASTBEIN GRUEN
Brokerbericht: {"abweisungen": 0, "angenommen": 64, "control_verbindungen": 32, "geschlossen_bootstrap": 0, "geschlossen_envelope": 0, "geschlossen_p0_ueberlauf": 0, "geschlossen_rate": 0, "ingress_p1_verworfen": 0, "ingress_p2_verworfen": 0, "letzter_grund": "", "p0": 10048, "p0_beantwortet": 10048, "p1": 0, "p2": 92903, "pipe": "\\\\.\\pipe\\evenacadia.nakama.v3.last.22332.1787977039", "telemetrie_verbindungen": 32}
  ok      das C++-Lastbein selbst ist gruen  [Exit 0]
  ok      der Rust-Broker hat einen Bericht geliefert  [16 Felder]
  ok      32 Control-Verbindungen im Broker angekommen  [32]
  ok      32 Telemetrieverbindungen GEKOPPELT (nicht nur verbunden)  [32]
  ok      der Broker hat P0 gesehen und beantwortet  [10048 empfangen / 10048 beantwortet]
  ok      und P2 ueber die zweiten Verbindungen  [92903]
  ok      keine Verbindung wegen Envelope, Rate oder P0-Ueberlauf geschlossen  [envelope=0 rate=0 p0=0]
  ok      kein P0-Frame geht verloren (Clientseite)  [10048/10048]
  ok      es lag wirklich Rueckstau an  [40800 ersetzte P2-Frames]
GRUEN
EXIT=0
```

**Was die Zahlen sagen.** 133 568 P2-Frames veröffentlicht, 92 903 gesendet,
40 800 vom Cap-2-Ersetzen verworfen — der Rückstau war also real und nicht
behauptet. Währenddessen: 10 048 P0-Frames gesendet, **10 048** beantwortet,
Maximum **21 ms**, p99 **20 ms** bei einer Schranke von 1000 ms. Keine
Verbindung wurde wegen Envelope, Rate oder P0-Überlauf geschlossen.

Der strukturelle Grund steht im Design und wird hier **gemessen**, nicht
erfunden: P0 und P2 laufen über getrennte Pipe-Verbindungen (§33.1), ein
Telemetriestau kann Steuerung deshalb gar nicht erst blockieren. Das Lastbein
zeigt, dass diese Trennung im gebauten Weg auch wirklich trägt.

**Ein ehrlicher Zwischenstand.** Ein Lauf dieses Beins war zwischendurch ROT —
`9920/10048`. Ursache war kein Fehler im Weg, sondern ein **veraltetes
Artefakt**: der Probe-Broker war noch aus dem absichtlich gebrochenen Stand
gebaut (jeder 100. Heartbeat unbeantwortet; 4 × 32 = 128 fehlende ACKs, exakt
die Differenz). Genau dafür baut `tools/beweise.ps1 -Bauen` den Probe-Broker
jetzt ausdrücklich im Release-Profil mit.

<a id="b6"></a>
### B6 · Jedes neue Bein einmal absichtlich gebrochen

Ein Riegel, der nie angeschlagen hat, ist keiner.

**A20 — ein Fixturebyte gekippt:**

```text
=== A20 absichtlich gebrochen ===
8 gueltige, 29 ungueltige Envelope-Fixtures; 14 Regeln, jede mit Negativfall
  ROT: eq-copilot\fixtures\v3\envelope\ungueltig\crc-p2-null-statt-summe.bin weicht ab
EXIT=2
=== wiederhergestellt ===
8 gueltige, 29 ungueltige Envelope-Fixtures; 14 Regeln, jede mit Negativfall
  bytegleich: 38 Dateien, MANIFEST sha256=efa50c0384865d8a3d4eb560c7037937b60af6a4f32731f7d7dc5e953cf13307
EXIT=0
```

**A21 und der Envelope-Korpus — `16 + payload_len` wieder in u32 gerechnet.**
Der Unterschied zeigt sich im REGELNAMEN: die naive Rechnung sagt
`ueberlaenge`, die richtige `unterlaenge`.

```text
=== A21 absichtlich gebrochen: 16+payload_len wieder in u32 gerechnet ===
test ratengrenze_haelt_unter_flut ... ok
test zufallsbytes_bringen_den_pruefer_nie_aus_dem_tritt ... ok
test jedes_gekippte_payloadbit_eines_p2_frames_faellt_an_der_crc ... ok
test payload_an_der_paketgrenze_wird_angenommen_darueber_nicht ... ok
test schreiben_und_pruefen_sind_zueinander_invers ... ok
test fragmentierung_aendert_das_ergebnis_nie ... ok
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

failures:

---- feindliche_laengen_loesen_keine_allokation_aus stdout ----

thread 'feindliche_laengen_loesen_keine_allokation_aus' (18068) panicked at tests\transport_fuzz.rs:171:9:
assertion `left == right` failed: payload_len 4294967295
  left: Err([Ueberlaenge])
 right: Err([Unterlaenge])
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    feindliche_laengen_loesen_keine_allokation_aus

test result: FAILED. 8 passed; 1 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.16s

error: test failed, to rerun pass `--test transport_fuzz`
--- und der Envelope-Korpus (A4/contract_cross_language) dazu ---
thread 'envelope_korpus_klassifiziert_wie_das_manifest' (20788) panicked at tests\contract_cross_language.rs:640:5:
1 von 37 Envelope-Fixtures weichen ab:
ungueltig/kopf-payload-len-u32-ueberlauf.bin: Verstossmenge weicht ab
  soll [Unterlaenge]
  ist  [Ueberlaenge]
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    envelope_korpus_klassifiziert_wie_das_manifest

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 8 filtered out; finished in 0.02s

error: test failed, to rerun pass `--test contract_cross_language`
=== wiederhergestellt ===
test einzelne_gekippte_bytes_werden_nie_still_angenommen ... ok

test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.19s
```

**B10 — dieselbe Rechnung in der C++-Fassung gebrochen:**

```text
=== B10 absichtlich gebrochen: 16+payload_len wieder in 32 Bit gerechnet ===

  FEHLER  alle Fixtures klassifiziert wie das Manifest (Urteil UND Verstossmenge)  [8 gueltig / 29 ungueltig; ungueltig/kopf-payload-len-u32-ueberlauf.bin: soll unterlaenge, ist ueberlaenge]
FEHLER — 86 Pruefungen, 1 Fehler
```

**A22 — der Broker lässt jeden 100. Heartbeat unbeantwortet:**

```text
=== A22 absichtlich gebrochen: der Broker laesst jeden 100. Heartbeat unbeantwortet ===

SONDE-010 | Lastbein: 32 Sonden, 4 s, Pipe \\.\pipe\evenacadia.nakama.v3.last.17892.1787976657
{"sonden":32,"sekunden":4,"p2_veroeffentlicht":66720,"p2_gesendet":46075,"p2_ersetzt":20704,"p0_gesendet":5024,"p0_beantwortet":4960,"p0_latenz_max_ms":20,"p0_latenz_p99_ms":20,"p0_ueberlaeufe":0,"envelope_abweisungen":0}
  FEHLER  KEIN P0-Frame geht verloren  [4960/5024]
LASTBEIN ROT
  ROT     das C++-Lastbein selbst ist gruen  [Exit 1]
  ROT     kein P0-Frame geht verloren (Clientseite)  [4960/5024]
ROT
```

Alle vier Eingriffe sind zurückgenommen; die Läufe B1–B5 stehen auf dem
committeten Stand.

---

## 3. Kanon-Lauf

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-010.md -Anhaengen -Titel 'SONDE-010'
```

_(Der angehängte Abschnitt erscheint unter dieser Zeile.)_

---

## 4. T1 — Selbstaudit

Feste Liste aus `docs/bauaufteilung-sonden.md` §2, nie abgekürzt.

| # | Punkt | Befund |
|---|---|---|
| 1 | Numerische Ränder: NaN, ±inf, 0, negativ, Überlauf, Wrap | **Der Kern des Envelope.** `frame_len == 16 + payload_len` wird in **u64** gerechnet; die u32-Fassung liefe bei `payload_len = 0xFFFFFFFF` auf 15 um und akzeptierte den Frame. Das Fixture `kopf-payload-len-u32-ueberlauf.bin` unterscheidet beide über den REGELNAMEN, und beide Sprachen wurden damit einmal absichtlich gebrochen (B6). Dazu: `frame_len` 0, 15, 262 144, 262 145, 0x7FFFFFFF, 0xFFFFFFFF; `payload_len` 0 (gültig), an der Grenze (gültig) und darüber; Bootstrap 16 KiB und 16 KiB + 1. Nicht-endliche Zahlen gibt es hier nicht — der Envelope urteilt nie über Inhalt; `samplerate` im Hello wird trotzdem auf `is_finite()` geprüft. |
| 2 | Gegenpfad vorhanden? | Ja, jeweils im selben Änderungssatz: `control_anmelden`↔`control_abmelden`, `telemetrie_koppeln`↔`telemetrie_entkoppeln` (mit Test, dass zweimal Abmelden **kein** Erfolg ist), `start`↔`stop`, `oeffnen`↔`schliessen`, `v3_server_starten`↔`V3Griff::stoppen` (auch im `Drop`), `veroeffentlichen`↔`abholen`. Die Control-Verbindung besitzt die Kopplung: geht sie, geht der Telemetrieplatz mit — sonst bliebe eine halb offene Kopplung stehen. |
| 3 | Behauptungs-Integrität: existiert jede genannte Datei / Funktion / Zeile wirklich? | Jede in diesem Manifest genannte Datei wurde in dieser Session geschrieben oder gelesen. Die Regelmenge wird nicht behauptet, sondern **gemessen**: `contract_cross_language` vergleicht die Regelliste des Manifests elementweise mit `ALLE_VERSTOESSE` des Lesers, das C++-Bein tut dasselbe — eine Regel im Code ohne Zeile im Manifest fällt sofort auf. Der Envelope-Erzeuger bricht ab, wenn eine Regel ohne Negativfixture bleibt. |
| 4 | Lügt der UI-Text? | Keine UI in diesem Ticket. Die entsprechende Ebene ist die Zählerwahrheit, und dort wurde **eine Lüge gefunden und behoben**: `P2Schleuse::ersetzteFrames()` zählte anfangs jede Veröffentlichung nach den ersten beiden als Ersetzung, auch wenn der Verbraucher längst leergeräumt hatte (133 056 „Ersetzungen" bei 133 120 Veröffentlichungen und 92 857 gesendeten Frames — die Zahlen widersprachen einander). Der Erzeuger kennt jetzt den Fortschritt des Verbrauchers. Was bleibt, steht als Schranke im Header: die Zahl ist eine **obere** Grenze, nie eine kleinere als der eingetretene Verlust. |
| 5 | Anzeige-Pflichten (nur bei UI-Diff) | Nicht betroffen — kein UI-Diff. Die Zustände, die eine spätere Oberfläche zeigen muss, liegen als ehrliche Felder bereit: `ControlClient::Snapshot` trägt Status, `letzterFehler`, Versuche, P0-Überläufe, P1-Wiederholungen und Envelope-Abweisungen; `TelemetryClient::Snapshot` zusätzlich `wartetAufKopplung` als **eigenen** Zustand — „noch nicht gekoppelt" ist etwas anderes als „getrennt". |
| 6 | Audiothread: nachweislich keine Allokation, kein Lock, kein I/O, kein Logging? | Der Audiothread wird in diesem Ticket **nicht angefasst**; kein Sample geht durch neuen Code. Die Grenze dorthin ist die `P2Schleuse`: vorallokiert, lockfrei, wartefrei, und mit dem thread-lokalen Allokationszähler gemessen — **100 000 Übergaben mit 0 Allokationen**, mit Gegenprobe am selben Zähler (er sieht eine echte Allokation, also schweigt er nicht bloß). Ein Nebenläufigkeitslauf über 200 000 Frames zeigt: kein abgeholter Frame ist zerrissen. Beide Clients führen ihre I/O ausschließlich auf eigenen Threads; `sendeP0`/`sendeP1`/`veroeffentlichen` reihen nur ein. |

---

## 5. Befunde aus diesem Ticket

| Befund | Quelle | Gegen die Quelldatei verifiziert? | Ausgang |
|---|---|---|---|
| `PipeToken.cpp` trug den eingefrorenen Herstellernamen `evenacadia` in `NakamaKern.lib` | T1 / Kanon-Bein **A14** beim ersten Lauf nach dem Zuwachs | ☑ ja — A14-Rohausgabe, Abschnitt [2] | **gefixt**, ohne den Riegel anzufassen: die Datei ist aus dem geteilten Boden genommen. Welche Pipe eine Instanz wählt, ist dieselbe Art Aussage wie welche Identität sie trägt — sie gehört in die dünne Zielschicht. Der Kern bekommt den Pipenamen als Zeichenkette. |
| `ersetzteFrames()` widersprach den übrigen Zählern | T1, Lastbein-Ausgabe | ☑ ja — `IpcQueues.h`, `veroeffentlichen()` | **gefixt**: der Erzeuger liest jetzt auch den Fortschritt des Verbrauchers. Die verbleibende Ungenauigkeit (~1 ‰, konservativ) steht als Schranke im Header. |
| Der C++-Fuzz sprach über eine leere Menge (`0 angenommen / 40000 abgelehnt`) | T1, eigener Lauf | ☑ ja — `IpcTestMain.cpp`, Abschnitt C | **gefixt** (`97c956d`): dieselbe Invariante zusätzlich an Einbit-Mutanten gültiger Frames — 7671 von 20 000 kommen durch und halten jede Kopfregel. |
| 27 von 32 Clientpaaren standen nach 20 s; die übrigen warteten auf verdoppelten Backoff | Lastbein A22, erster Lauf | ☑ ja — `IpcVerbindung::oeffnen` | **gefixt**: `ERROR_PIPE_BUSY` heißt nicht „nicht da", sondern „gerade horcht keine Instanz". Der Client nutzt jetzt `WaitNamedPipe` statt Backoff. Danach 32/32. |
| Die Bootstrapschranke prüfte **vor** dem Parsen und hätte ein legitimes Hello mit angehängten v3-Frames in einem einzigen Read abgewiesen | T1, Diff-Lesung | ☑ ja — `server_v3.rs`, Bootstrap-Leseschleife | **gefixt**: die Längengrenze des Hellos prüft `bootstrap_lesen` selbst (ab dem vierten Byte, vor jeder Allokation); die Schleife bewacht nur noch den pipelinenden Peer. |
| A22 lief einmal ROT (`9920/10048`) | eigener Lauf | ☑ ja — Differenz exakt 4 × 32 = 128, also die Bruchprobe | **kein Befund am Weg**, sondern ein veraltetes Artefakt: der Probe-Broker war noch aus dem gebrochenen Stand gebaut. `tools/beweise.ps1 -Bauen` baut ihn jetzt ausdrücklich mit. Das Bein hat dabei genau das getan, wofür es da ist. |
| NAK-07 (Pipe-Backpressure/Deadlines) | `docs/offene-punkte.md` | ☑ ja | **geschlossen** — siehe die Zeile dort. |
| NAK-49 / ACCESS-001 (kein `SECURITY_SQOS_PRESENT`) | `docs/offene-punkte.md` | ☑ ja — `IpcVerbindung::oeffnen` | **nicht wiederholt**: der neue Client setzt `SECURITY_SQOS_PRESENT \| SECURITY_IDENTIFICATION` von Anfang an. Der Altbestand `src/PipeClient.cpp` trug den Fix bereits (Bein A4b). |

---

## 6. Was dieses Ticket **nicht** geprüft hat

Damit ein frischer Prüfer nicht danach sucht:

* **Kein FL.** Nichts hier lief in einem Host. Der Weg ist headless bewiesen.
* **Keine HMAC-Authentisierung, kein Pairing, kein Replaycache.** Das ist
  `SONDE-016` (§48.4). Die Kopplung hier ist `link_id + challenge +
  runtime_nonce` über eine per DACL auf den aktuellen User begrenzte Pipe —
  sie ersetzt keine Signatur und behauptet es nirgends.
* **Keine Impersonation-Kette im v3-Listener.** §48.4 verlangt
  `ImpersonateNamedPipeClient` nach dem Bootstrap-Hello mit `RevertToSelf` in
  jedem Pfad. Der v3-Listener fährt heute nur über Probe-Namen und trägt sie
  deshalb noch nicht; sie gehört in denselben Änderungssatz wie das produktive
  Öffnen des Endpunkts (`SONDE-011`). Neu als **NAK-90** eingetragen.
* **Kein Coordinator, kein Store, keine Outbox** — `SONDE-011`.
* **Keine P1-Last.** Das Lastbein fährt P0 und P2; P1 ist in den Unittests und
  im C++-Bein geprüft, aber nicht unter Ende-zu-Ende-Last. Neu als **NAK-91**
  eingetragen.
