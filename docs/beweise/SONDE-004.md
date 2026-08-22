# Beweismanifest — SONDE-004 «FL-Aux-/PDC-/Recall-Spike und Capabilityreport» (S4)

> **Die eine harte Regel** (`docs/bauaufteilung-sonden.md` §2): *Eine Behauptung
> ohne eingefügte Rohausgabe ist ein **gescheitertes** Ticket.* Die Rohdaten
> beider FL-Termine liegen unverändert in `docs/beweise/termin-a/` und
> `docs/beweise/termin-b/`; unten stehen ihre entscheidenden Felder wortgleich.

| Feld | Wert |
|---|---|
| Ticket | `SONDE-004` — S1 (`SONDE-004a`, Messgerät, 20.08.) · 👤 Termin A (22.08. 00:17/00:27) · S3b (Messgerät Termin B, 21.08.) · 👤 Termin B (22.08. 12:45–13:27) · **S4 (dieses Manifest)** |
| Phase / Session | P0 / S4 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §65 — **wörtlich:** „FL-Aux-/PDC-/Recall-Spike und Capabilityreport — fertig, wenn: jede Aux-Capability eindeutig supported/unsupported". §53.6 Capabilityvertrag (zehn Bits, „Ein Capabilitybit wird nur nach seinem Golden gesetzt"), §54 Exit-Gate („Kein `unknown, später prüfen` darf P1 passieren"), §32.2 (Aux-Topologie), §33.4/§41.3 (Recording/Render-Regeln) |
| Commits | siehe `git log --oneline` ab `ad16234` (22.08.2026) |
| Datum | 2026-08-22 |
| Prüfstufen | T1 ☑ (§4) · T2 ☐ (§5) · T3 ☐ (G0 ist die eigene Session danach) |

**Was S4 liefert:** `eq-copilot/identity/host-capabilities-fl-v1.json` — die zehn
§53.6-Bits für FL Studio, jedes mit Rohfeld und Datei, in der Vertragsform
`$defs/capabilities` des v3-Schemas — und das Prüfbein
`tools/eq-copilot/pruefe_host_capabilities.py` (Kanon **A13**), das jedes Bit
gegen die Rohdateien und die Vertragsform misst. Dazu `docs/beweise/termin-b/`
(zwei Berichte, Aktionsprotokoll mit Uhrzeiten, LIES-MICH).

---

## 1. Ticket-Behauptungen

| # | Behauptung (Gate-Text) | Befehl | Ergebnis | Rohausgabe | Datum |
|---|---|---|---|---|---|
| 1 | **Jede Aux-Capability eindeutig:** `aux_compare_pre` **supported**, `aux_priority_sidechain` **supported** (beide: aktiv, 2 Kanäle, Protokoll eingehalten, Versatz zum Hauptweg **0 Samples** vor UND nach Speichern/Schließen/Neuladen), `contribution_aux` **unsupported** (kein Gerät misst die Main-Aux-Busse; §54 verbietet `unknown`; Fallback „nur Assoziation statt exakter Attribution") | `py -3.13 tools/eq-copilot/pruefe_host_capabilities.py` | ☑ | [↓ B1](#b1), [↓ B2](#b2) | 2026-08-22 |
| 2 | `host_context_presence` **supported**: Hostbrücke liefert, Kontext in 259 298 von 259 298 Blöcken, `kontext_weg` 0 | dito | ☑ | [↓ B1](#b1), [↓ B3](#b3) | 2026-08-22 |
| 3 | `project_time_samples` **supported**: alle sieben Kontextfelder immer gültig; Seeks ohne Stop in beide Richtungen gemeldet (2 vor, 51 zurück inkl. Loop-Wraps mit exakter Songlänge), Play-nach-Stop als eigene Zeile (5), Render als 2 587 Offline-Blöcke mit `offline_an/aus`, 0 negative Projektzeiten | dito | ☑ | [↓ B3](#b3), [↓ B4](#b4) | 2026-08-22 |
| 4 | `sample_accurate_automation` **unsupported** (§53.6-Sinn): 83 303 Punkte, nie mehr als einer je Block, Offset immer 0, `samplegenau_belegt: false`; FL zerteilt stattdessen die Puffer (Blockgröße 1…4 096). Fallback „Blockrampe; Topologieautomation aus" | dito | ☑ | [↓ B3](#b3) | 2026-08-22 |
| 5 | `presentation_latency` **supported**: FL meldet Eingang Bus 0 = 3 924, Ausgang Bus 0 = 4 410 Samples; Impulsbeweis A zeigt Aux-Wege ohne Versatz. Offen: ein verworfener Wertwechsel (NAK-43) | dito | ☑ | [↓ B2](#b2), [↓ B3](#b3) | 2026-08-22 |
| 6 | `float64_processing` **unsupported**: 259 298 float-Blöcke, 0 double | dito | ☑ | [↓ B3](#b3) | 2026-08-22 |
| 7 | `binary_telemetry`, `remote_control` **unsupported** bis zu ihren Beweisen (SONDE-010 bzw. 016/017) — kein Hostbit, fester Fallback eingetragen | dito | ☑ | [↓ B1](#b1) | 2026-08-22 |
| 8 | Die zehn Bits entsprechen **exakt** der Vertragsform `$defs/capabilities` (v3, strikt, `supported|unsupported`) | dito (jsonschema) | ☑ | [↓ B1](#b1) | 2026-08-22 |
| 9 | Smart Disable: in 12 s und 30 s stummer Wiedergabe **keine Lücke** (0 ausgelassene Blöcke, kein zusätzlicher Vorwärtssprung) — gemessen bei OFFENEM Plugin-Fenster | Aktionsprotokoll + Bericht | ☑ (Befund, kein Capabilitybit) | [↓ B3](#b3), [↓ B5](#b5) | 2026-08-22 |
| 10 | Kanon-Lauf mit A13 als Pflichtbein | `pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-004.md -Anhaengen -Titel 'SONDE-004'` | siehe §3 | [↓ §3](#3-kanon-lauf) | 2026-08-22 |

---

## 2. Rohe Ausgaben

<a id="b1"></a>
### B1 · `pruefe_host_capabilities.py` (Behauptungen 1–8)

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_host_capabilities.py` · **Exitcode:** 0 · **Datum:** 2026-08-22

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Smart Disable: kein ausgelassener Block in 12 s + 30 s Stille
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=supported traegt einen Termin (A + B)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=supported traegt einen Termin (A)
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=supported traegt einen Termin (A)
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen

HOST-CAPABILITIES OK - 36 Pruefungen ok, 0 Fehler
```

<a id="b2"></a>
### B2 · Termin A — Rohfelder (unverändert aus `docs/beweise/termin-a/`)

`aux-spike-20260822-001701.json` (vor Speichern) und `aux-spike-20260822-002722.json` (nach Schließen + Neuladen), je `busse[1]` = `priority_sidechain`, `busse[2]` = `compare_pre`:

```text
001701: priority_sidechain  aktiv=true kanaele=2 protokoll_eingehalten=true impuls_sample=3010082 versatz_zu_main_samples=0 versatz_zu_main_ms=0.0
001701: compare_pre         aktiv=true kanaele=2 protokoll_eingehalten=true impuls_sample=3010082 versatz_zu_main_samples=0 versatz_zu_main_ms=0.0
002722: priority_sidechain  aktiv=true kanaele=2 protokoll_eingehalten=true impuls_sample=175650  versatz_zu_main_samples=0 versatz_zu_main_ms=0.0
002722: compare_pre         aktiv=true kanaele=2 protokoll_eingehalten=true impuls_sample=175650  versatz_zu_main_samples=0 versatz_zu_main_ms=0.0
host=FruityLoops wrapper=VST3 samplerate=44100.0 blockgroesse=2048 (beide Laeufe)
```

Kontext und Aufbau: `docs/beweise/termin-a/LIES-MICH.md`.

<a id="b3"></a>
### B3 · Termin B — Kopf und Zähler des vollständigen Berichts

`docs/beweise/termin-b/host-probe-20260822-132644.json` (der zweite Bericht enthält den ersten; die Probe sammelt seit dem Laden):

```json
{
 "schema": "nakama-hostprobe-1",
 "erzeugt": "2026-08-22T13:26:44.622+02:00",
 "host": "FruityLoops",
 "wrapper": "VST3",
 "bruecke_liefert": true,
 "bloecke": {
  "senke_aufrufe": 259298,
  "verarbeitete_bloecke": 259298,
  "mit_kontext": 259298,
  "ohne_kontext": 0,
  "offline": 2587,
  "echtzeit": 256711,
  "float": 259298,
  "double": 0,
  "samplerate": 44100.0,
  "blockgroesse_min": 1,
  "blockgroesse_max": 4096
 },
 "gueltig_immer": {
  "project_time_samples": true,
  "continous_time_samples": true,
  "tempo": true,
  "ppq_position": true,
  "bar_position": true,
  "cycle_bounds": true,
  "sample_rate": true
 },
 "projektzeit": {
  "spruenge_vorwaerts": 2,
  "spruenge_rueckwaerts": 51,
  "spruenge_ueber_stop": 5,
  "projektzeit_negativ": 0,
  "hinweis_vorwaertssprung": "Seek vorwaerts ODER Smart-Disable-Luecke - aus den Daten allein nicht unterscheidbar",
  "groesster_sprung_vor": 65960,
  "groesster_sprung_zurueck": 539635,
  "letzte_projektzeit": 0,
  "letztes_ppq": 0.0,
  "letztes_tempo": 140.0
 },
 "automation": {
  "punkte_gesamt": 83303,
  "max_punkte_pro_block": 1,
  "bloecke_mit_mehrpunkt": 0,
  "kleinster_offset": 0,
  "groesster_offset": 0,
  "ueberlaeufe": 0,
  "unplausibel": 0,
  "verworfene_letztwerte": 0,
  "bloecke_ohne_zusicherung": 0,
  "mehrpunkt_ohne_zusicherung": 0,
  "samplegenau_belegt": false
 },
 "presentation_latency": {
  "je_gemeldet": true,
  "verworfene_busmeldungen": 0,
  "verworfene_wertwechsel": 1,
  "gemeldet": [
   {
    "richtung": "eingang",
    "bus": 0,
    "samples": 3924
   },
   {
    "richtung": "ausgang",
    "bus": 0,
    "samples": 4410
   }
  ]
 },
 "ereignisse_je_art": {
  "erster_block": 1,
  "kontext_da": 1,
  "kontext_weg": 0,
  "transport_an": 8,
  "transport_aus": 8,
  "aufnahme_an": 0,
  "aufnahme_aus": 0,
  "schleife_an": 8,
  "schleife_aus": 8,
  "zeitsprung_vor": 2,
  "zeitsprung_zurueck": 51,
  "zeitsprung_ueber_stop": 5,
  "projektzeit_negativ": 0,
  "offline_an": 1,
  "offline_aus": 1,
  "genauigkeit_float": 1,
  "genauigkeit_double": 0,
  "latenz_gemeldet": 2,
  "automation_punkt": 1,
  "automation_mehrpunkt": 0,
  "automation_unplausibel": 0,
  "block_ohne_verarbeitung": 0
 }
}
```

<a id="b4"></a>
### B4 · Termin B — alle 98 Ereignisse in Reihenfolge

```text
 0 erster_block           block=      0 pz=       -1 vor=       -1 ppq=  0.000 zusatz=       0.0 ganz=182
 1 kontext_da             block=      0 pz=        0 vor=       -1 ppq=  0.000 zusatz=       0.0 ganz=-1
 2 latenz_gemeldet        block=      0 pz=       -1 vor=       -1 ppq=  0.000 zusatz=    3924.0 ganz=0
 3 latenz_gemeldet        block=      0 pz=       -1 vor=       -1 ppq=  0.000 zusatz=    4410.0 ganz=-1
 4 genauigkeit_float      block=      1 pz=       -1 vor=       -1 ppq=  0.000 zusatz=       0.0 ganz=182
 5 automation_punkt       block=  90685 pz=        0 vor=       -1 ppq=  0.000 zusatz=       1.0 ganz=0
 6 transport_an           block=  91505 pz=        0 vor=        0 ppq=  0.000 zusatz=       0.0 ganz=-1
 7 schleife_an            block=  91505 pz=        0 vor=       -1 ppq=  0.000 zusatz=       9.5 ganz=1
 8 zeitsprung_zurueck     block=  92505 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
 9 zeitsprung_zurueck     block=  93506 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
10 transport_aus          block=  93979 pz=    84922 vor=    84853 ppq=  4.490 zusatz=       0.0 ganz=-1
11 schleife_aus           block=  93979 pz=    84922 vor=       -1 ppq=  0.000 zusatz=       9.5 ganz=1
12 transport_an           block= 105979 pz=    84853 vor=    84922 ppq=  4.490 zusatz=       0.0 ganz=-1
13 zeitsprung_ueber_stop  block= 105979 pz=    84853 vor=    84922 ppq=  4.490 zusatz=     -69.0 ganz=-1
14 schleife_an            block= 105979 pz=    84853 vor=       -1 ppq=  0.000 zusatz=       9.5 ganz=1
15 zeitsprung_zurueck     block= 106507 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
16 zeitsprung_zurueck     block= 107508 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179746.0 ganz=1
17 zeitsprung_zurueck     block= 108508 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
18 zeitsprung_zurueck     block= 109508 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
19 zeitsprung_zurueck     block= 110509 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
20 zeitsprung_zurueck     block= 111509 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
21 zeitsprung_zurueck     block= 112509 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
22 zeitsprung_zurueck     block= 113510 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
23 zeitsprung_zurueck     block= 114510 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
24 zeitsprung_zurueck     block= 115511 pz=        1 vor=   179550 ppq=  0.000 zusatz= -179746.0 ganz=1
25 zeitsprung_zurueck     block= 116511 pz=        0 vor=   179737 ppq=  0.000 zusatz= -179747.0 ganz=1
26 zeitsprung_zurueck     block= 117510 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
27 zeitsprung_zurueck     block= 118511 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
28 zeitsprung_zurueck     block= 119510 pz=        0 vor=   179550 ppq=  0.000 zusatz= -179747.0 ganz=1
29 transport_aus          block= 119871 pz=        0 vor=    64772 ppq=  0.000 zusatz=       0.0 ganz=-1
30 schleife_aus           block= 119871 pz=        0 vor=       -1 ppq=  0.000 zusatz=       9.5 ganz=1
31 transport_an           block= 138927 pz=        0 vor=        0 ppq=  0.000 zusatz=       0.0 ganz=-1
32 schleife_an            block= 138927 pz=        0 vor=       -1 ppq=  0.000 zusatz=      28.6 ganz=1
33 zeitsprung_zurueck     block= 141930 pz=        0 vor=   539438 ppq=  0.000 zusatz= -539634.0 ganz=1
34 zeitsprung_zurueck     block= 144933 pz=        0 vor=   539438 ppq=  0.000 zusatz= -539635.0 ganz=1
35 zeitsprung_zurueck     block= 147936 pz=        0 vor=   539437 ppq=  0.000 zusatz= -539634.0 ganz=1
36 zeitsprung_zurueck     block= 150938 pz=        1 vor=   539438 ppq=  0.000 zusatz= -539634.0 ganz=1
37 zeitsprung_zurueck     block= 153942 pz=        0 vor=   539438 ppq=  0.000 zusatz= -539635.0 ganz=1
38 zeitsprung_zurueck     block= 155612 pz=     1969 vor=   300037 ppq=  0.104 zusatz= -298131.0 ganz=1
39 zeitsprung_zurueck     block= 158604 pz=        0 vor=   539438 ppq=  0.000 zusatz= -539635.0 ganz=1
40 zeitsprung_zurueck     block= 158712 pz=      394 vor=    19293 ppq=  0.021 zusatz=  -18996.0 ganz=1
41 zeitsprung_zurueck     block= 161687 pz=     1575 vor=   534910 ppq=  0.083 zusatz= -533347.0 ganz=1
42 zeitsprung_zurueck     block= 164681 pz=        0 vor=   539437 ppq=  0.000 zusatz= -539634.0 ganz=1
43 transport_aus          block= 165574 pz=     1575 vor=   160257 ppq=  0.083 zusatz=       0.0 ganz=-1
44 schleife_aus           block= 165574 pz=     1575 vor=       -1 ppq=  0.000 zusatz=      28.6 ganz=1
45 transport_an           block= 170464 pz=      984 vor=      984 ppq=  0.052 zusatz=       0.0 ganz=-1
46 zeitsprung_ueber_stop  block= 170464 pz=      984 vor=     1575 ppq=  0.052 zusatz=    -591.0 ganz=-1
47 schleife_an            block= 170464 pz=      984 vor=       -1 ppq=  0.000 zusatz=      28.6 ganz=1
48 zeitsprung_zurueck     block= 173462 pz=        0 vor=   539608 ppq=  0.000 zusatz= -539634.0 ganz=1
49 zeitsprung_zurueck     block= 176465 pz=        0 vor=   539438 ppq=  0.000 zusatz= -539634.0 ganz=1
50 zeitsprung_zurueck     block= 176904 pz=        0 vor=    78750 ppq=  0.000 zusatz=  -78836.0 ganz=1
51 zeitsprung_zurueck     block= 177868 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
52 zeitsprung_zurueck     block= 178833 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
53 zeitsprung_zurueck     block= 179796 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
54 zeitsprung_zurueck     block= 180761 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
55 zeitsprung_zurueck     block= 181725 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
56 zeitsprung_zurueck     block= 182689 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
57 zeitsprung_zurueck     block= 183654 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
58 zeitsprung_zurueck     block= 184617 pz=        0 vor=   173053 ppq=  0.000 zusatz= -173250.0 ganz=1
59 transport_aus          block= 185030 pz=        0 vor=    74222 ppq=  0.000 zusatz=       0.0 ganz=-1
60 schleife_aus           block= 185030 pz=        0 vor=       -1 ppq=  0.000 zusatz=       9.2 ganz=1
61 transport_an           block= 205738 pz=        0 vor=        0 ppq=  0.000 zusatz=       0.0 ganz=-1
62 schleife_an            block= 205738 pz=        0 vor=       -1 ppq=  0.000 zusatz=      28.6 ganz=1
63 zeitsprung_zurueck     block= 208741 pz=        0 vor=   539438 ppq=  0.000 zusatz= -539634.0 ganz=1
64 zeitsprung_zurueck     block= 211744 pz=        0 vor=   539438 ppq=  0.000 zusatz= -539635.0 ganz=1
65 zeitsprung_zurueck     block= 214747 pz=        0 vor=   539437 ppq=  0.000 zusatz= -539634.0 ganz=1
66 transport_aus          block= 215369 pz=        0 vor=   111628 ppq=  0.000 zusatz=       0.0 ganz=-1
67 schleife_aus           block= 215369 pz=        0 vor=       -1 ppq=  0.000 zusatz=      28.6 ganz=1
68 transport_an           block= 219727 pz=     1181 vor=     1181 ppq=  0.062 zusatz=       0.0 ganz=-1
69 zeitsprung_ueber_stop  block= 219727 pz=     1181 vor=        0 ppq=  0.062 zusatz=    1181.0 ganz=-1
70 schleife_an            block= 219727 pz=     1181 vor=       -1 ppq=  0.062 zusatz=      27.0 ganz=1
71 offline_an             block= 219728 pz=       -1 vor=       -1 ppq=  0.000 zusatz=       0.0 ganz=197
72 transport_aus          block= 222313 pz=   510300 vor=   510103 ppq=  0.062 zusatz=       0.0 ganz=-1
73 schleife_aus           block= 222313 pz=   510300 vor=       -1 ppq=  0.062 zusatz=      27.0 ganz=1
74 offline_aus            block= 222315 pz=       -1 vor=       -1 ppq=  0.000 zusatz=       0.0 ganz=197
75 transport_an           block= 226372 pz=        0 vor=        0 ppq=  0.000 zusatz=       0.0 ganz=-1
76 zeitsprung_ueber_stop  block= 226372 pz=        0 vor=   510300 ppq=  0.000 zusatz= -510300.0 ganz=-1
77 schleife_an            block= 226372 pz=        0 vor=       -1 ppq=  0.062 zusatz=      27.0 ganz=1
78 zeitsprung_zurueck     block= 229212 pz=     1181 vor=   510103 ppq=  0.062 zusatz= -509119.0 ganz=1
79 zeitsprung_vor         block= 230578 pz=   296494 vor=   246487 ppq= 15.688 zusatz=   49901.0 ganz=1
80 zeitsprung_zurueck     block= 231768 pz=     1181 vor=   510103 ppq=  0.062 zusatz= -509119.0 ganz=1
81 zeitsprung_zurueck     block= 234601 pz=     1181 vor=   510103 ppq=  0.062 zusatz= -509119.0 ganz=1
82 zeitsprung_vor         block= 234909 pz=   122456 vor=    56306 ppq=  6.479 zusatz=   65960.0 ganz=1
83 zeitsprung_zurueck     block= 237068 pz=     1182 vor=   510103 ppq=  0.063 zusatz= -509118.0 ganz=1
84 zeitsprung_zurueck     block= 237849 pz=   103163 vor=   141554 ppq=  5.458 zusatz=  -38559.0 ganz=1
85 zeitsprung_zurueck     block= 240115 pz=     1182 vor=   510104 ppq=  0.063 zusatz= -509119.0 ganz=1
86 transport_aus          block= 241485 pz=   103163 vor=   247275 ppq=  5.458 zusatz=       0.0 ganz=-1
87 schleife_aus           block= 241485 pz=   103163 vor=       -1 ppq=  0.062 zusatz=      27.0 ganz=1
88 transport_an           block= 243714 pz=        0 vor=        0 ppq=  0.000 zusatz=       0.0 ganz=-1
89 zeitsprung_ueber_stop  block= 243714 pz=        0 vor=   103163 ppq=  0.000 zusatz= -103163.0 ganz=-1
90 schleife_an            block= 243714 pz=        0 vor=       -1 ppq=  0.062 zusatz=      27.0 ganz=1
91 zeitsprung_zurueck     block= 246554 pz=     1181 vor=   510103 ppq=  0.062 zusatz= -509119.0 ganz=1
92 zeitsprung_zurueck     block= 249387 pz=     1182 vor=   510103 ppq=  0.063 zusatz= -509118.0 ganz=1
93 zeitsprung_zurueck     block= 252220 pz=     1182 vor=   510104 ppq=  0.063 zusatz= -509119.0 ganz=1
94 zeitsprung_zurueck     block= 255052 pz=     1181 vor=   510103 ppq=  0.062 zusatz= -509119.0 ganz=1
95 zeitsprung_zurueck     block= 257886 pz=     1181 vor=   510103 ppq=  0.062 zusatz= -509119.0 ganz=1
96 transport_aus          block= 257914 pz=        0 vor=     6103 ppq=  0.000 zusatz=       0.0 ganz=-1
97 schleife_aus           block= 257914 pz=        0 vor=       -1 ppq=  0.062 zusatz=      27.0 ganz=1
```

Lesehilfe (aus dem Aktionsprotokoll B5): 6–30 Vorlauf mit 2-Takt-Song; 31–44 Live + drei Seeks (38, 40, 41 = meine Seeks, alle rückwärts, weil der MCP-Modus „Sekunden" nahe Songanfang landet) + Loop-Wraps (33–37, 39, 42); 45–47 Play nach Stop (Schritt 4b); 48–60 Pattern-Schleife (Wraps ~964 Blöcke) und Song; 61–67 Smart-Disable-Stille 12 s; 68–74 Export (offline_an/aus); 75–87 Nachmessung: **79 und 82 = Vorwärtssprünge** (+49 901, +65 960), 84 = Rücksprung −38 559; 88–97 Smart-Disable-Stille 30 s (nur Loop-Wraps).

<a id="b5"></a>
### B5 · Aktionsprotokoll (Claude über FL-MCP)

```text
# Termin B — Aktionsprotokoll (Claude über FL-MCP), 22.08.2026

Uhrzeiten = Wanduhr des Rechners (Get-Date), Reihenfolge ist die Wahrheit.
Aufbau (per MCP verifiziert): Channel 1 „Grand Piano" → Mixer-Kanal 1, dort Slot 0 = `EqCop-Host-Probe`.
Automationsclip „EqCop-Host-Probe - Testwert (nur Messung)" im Playlist, dichte Zickzack-Kurve (Screenshot des Users).
Song-Länge nach Verlängerung ≈ 4 Takte, ≈ 3,5 s je Takt → Song-Loop alle ≈ 14 s im Song-Modus.

## Vorlauf (VOR der Verlängerung des Songs — steht ebenfalls im Probe-Log)
- ≈12:53–12:56: Song-Modus, Position 0, Play ≈20 s (Song war nur ≈2 Takte, FL loopte im Sekundentakt), dann stand „is_playing=false" bei 2:02:23 (Ende/Stop unklar); erneut Play ≈10 s (Positionen 2:02 → 2:13 → 2:09 = Loop), Stop.

## Messungen (nach Verlängerung)
| Zeit | Aktion | Klicklisten-Schritt |
|---|---|---|
| ≈12:58:22 | Song-Modus, Position 0, Play | 3 Live (≈20 s; enthält ≥1 Song-Loop-Rücksprung bei ≈14 s) |
| 12:58:50 | Seek → 10 s, OHNE Stop | 4 (vorwärts) |
| 12:59:04 | Seek → 2 s, OHNE Stop | 4 (rückwärts) |
| 12:59:16 | Seek → 8 s, OHNE Stop | 4 (vorwärts) |
| 12:59:41 | Stop | 4b |
| ≈12:59:55 | Position → 5 s (im Stop) | 4b |
| 13:00:01 | Play | 4b (Positionswechsel über Stop/Play) |
| 13:00:18 | Umschalten Pattern-Modus (läuft weiter) | 5 Schleife (Pattern = 1 Takt, ≈3 Durchläufe) |
| 13:00:42 | Ende Schleifenfenster | 5 |
| ≈13:00:50 | Stop; zurück in Song-Modus | — |
| (User) | Smart disable im Wrapper-Menü eingeschaltet („hab es an gemacht") | 6b |
| 13:02:45 | Piano-Channel STUMM (fl_mute_channel), Position 0, Play | 6b Stille 12 s |
| 13:03:07 | Piano wieder laut | 6b Ton 8 s |
| 13:03:15 | Stop | — |
| (User) | Export → WAV, Fenster wieder öffnen, „Bericht sichern" | 7 + Abschnitt 4 |

Erwartung für die Auswertung: Seeks ohne Stop = 3 Sprungereignisse (12:58:50 vor, 12:59:04 zurück, 12:59:16 vor) plus periodische Loop-Rücksprünge (Song ≈14 s, Pattern ≈3,5 s); genau EIN Positionswechsel über Stop/Play (13:00:01); Smart-Disable-Sprung(e) vorwärts nur im Fenster 13:02:45–13:03:15; Automationspunkte während aller Song-Modus-Abschnitte.
```

Nachmessung 13:10–13:12 (nicht im Protokoll, Uhrzeiten aus der Session): 13:10:22 Play ab 0 · 13:10:27 Seek → 9 000 ms ohne Stop · 13:10:42 Seek → 2 000 ms ohne Stop · 13:10:57 Seek → 10 000 ms ohne Stop · 13:11:12 Stop · 13:11:35 Play stumm (Smart disable an) 30 s · 13:12:14 laut · 13:12:22 Stop. Bericht gesichert 13:26:44.

---

## 3. Kanon-Lauf

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-004.md -Anhaengen -Titel 'SONDE-004'
```

_(Der angehängte Abschnitt erscheint am Dateiende.)_

---

## 4. T1 — Selbstaudit

| # | Punkt | Befund |
|---|---|---|
| 1 | Numerische Ränder | Alle Zahlen des Reports stammen aus den JSON-Rohfeldern und werden von A13 gegen sie gemessen (36 Prüfungen); keine Rechnung außer der Loop-Längen-Lesart (539 438 Samples / 44 100 Hz = 12,23 s bei 140 bpm und 7,15 Takten — Plausibilität, kein Bit hängt daran). |
| 2 | Gegenpfad | Messung↔Prüfung: Bericht sichern ↔ `pruefe_host_capabilities.py`; supported↔unsupported: jedes Bit trägt entweder einen Termin oder den festen Fallback (gemessen). Kein Produktcode geändert — der Capabilityreport wird erst von SONDE-007b/009 gelesen. |
| 3 | Behauptungs-Integrität | Dateien `host-capabilities-fl-v1.json`, `pruefe_host_capabilities.py`, `termin-b/*.json`, `termin-b/AKTIONSPROTOKOLL.md`, `termin-a/*.json` geöffnet bzw. erzeugt; A13 prüft die Existenz der zitierten Rohdateien. Ereignisnummern 13, 38, 40, 41, 79, 82, 84 gegen B4 gelesen. |
| 4 | Lügt der Text? | Zwei bewusst nicht behauptete Dinge: (a) Smart Disable ist kein Capabilitybit — „keine Lücke gemessen" gilt nur für offenes Fenster; (b) `presentation_latency` trägt den verworfenen Wertwechsel als NAK-43 statt ihn zu verschweigen. `contribution_aux` ist „nicht gemessen ⇒ unsupported", nicht „geht nicht". |
| 5 | Anzeige-Pflichten | kein UI-Diff. |
| 6 | Audiothread | kein Code-Diff im Plugin. |

**T1-Funde:** (a) Der FL-MCP setzt Positionen nur im Modus 1 (ms) brauchbar — die erste Sprungrunde war deshalb rein rückwärts; Nachmessung gefahren statt interpretiert. (b) Die Klickliste B sagte „Suchpfad hinzufügen, kein Admin nötig" — falsch, VST3 wird nur unter `Common Files\VST3` gescannt (Stand-Hinweis in beide Klicklisten). (c) Ein Song von 2 Takten macht Live und Schleife ununterscheidbar — Klickliste verlangt jetzt ≥ 16 Takte.

---

## 5. T2 — Frischkontext-Prüfer

| Feld | Wert |
|---|---|
| Agent | Frischkontext-Subagent (sieht nur Report-JSON, dieses Manifest, Rohdaten, Gate-Text §65/§53.6/§54) |
| Bruchauftrag | „Finde ein Bit in `host-capabilities-fl-v1.json`, das die Rohdaten NICHT tragen, oder eine Zahl in SONDE-004.md, die in den Rohdateien anders steht." |
| Urteil | ☐ PASS ☐ NEEDS_WORK — wird nach dem Lauf eingetragen |
| Datum | |

---

## 6. Befunde aus diesem Ticket

| Befund | Quelle | Verifiziert? | Ausgang |
|---|---|---|---|
| FL legt nie mehr als einen Automationspunkt je Block in die VST3-Queue, Offset immer 0; stattdessen Puffer-Zerteilung bis 1 Sample | Termin B | ☑ `automation.*`, `bloecke.blockgroesse_min` | Bit `sample_accurate_automation: unsupported`; Folge für SONDE-015 im Report (`zusatz`) |
| `presentation_latency`: ein späterer Wertwechsel verworfen, Wert unbekannt | Termin B | ☑ `verworfene_wertwechsel: 1` | **NAK-43** |
| Smart disable erzeugt bei offenem Fenster keine Lücke (12 s, 30 s) | Termin B | ☑ `block_ohne_verarbeitung: 0`, Ereignisse 61–67, 88–97 | Befund im Report (`gemessene_hosttatsachen`); geschlossenes Fenster nicht gemessen |
| Play nach Pause setzte 69 Samples vor der Pauseposition ein | Termin B | ☑ Ereignis 13 | Befund im Report; für SONDE-009 (Kontinuitätsregeln) relevant |
| FL-MCP `fl_set_song_position` Modus 2 („Sekunden") landet nahe Songanfang; Modus 1 (ms) funktioniert | Session | ☑ Transportstände 12:58–12:59 vs. 13:10 | Klickliste B „Arbeitsteilung" + `docs/NEXT-SESSION.md` (`fl_save_project` fehlt weiterhin im Fork) |
| `contribution_aux` ungemessen | Termin A (LIES-MICH) | ☑ | `unsupported` + Fallback; offen im Report |
| Legacy-`.flp`-Fixture (SONDE-002) nicht angelegt | Klickliste B Teil 3 | ☑ | bleibt offen (beim User) |

---

## Kanon-Lauf - SONDE-004

**Lauf:** 2026-08-22 13:34 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 18/18 Kanon-Laeufe bestanden | 4 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-22 13:34:25 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 4d148fb Hub-Selbstaudit: Suna zehn statt elf Lesarten, Gen 16+Export, P8 nur mit G0; Primer druckt UTF-8 |
| Commit (voll) | 4d148fb0281048a48fa16d5d643c4f4a8d3263c3 |
| Arbeitsbaum | 31 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M CLAUDE.md
 M docs/FL-Nakama-Sonden-Design-Entwurf.md
 M docs/NEXT-SESSION.md
 M docs/bauaufteilung-sonden.md
 D docs/handoffs/auto-handoff-2026-08-18-2242-7c7cee38.md
 D docs/handoffs/auto-handoff-2026-08-19-1032-9338b4bb.md
 D docs/handoffs/auto-handoff-2026-08-20-0228-844fc0d0.md
 D docs/handoffs/auto-handoff-2026-08-20-1147-1816d5bd.md
 D docs/handoffs/auto-handoff-2026-08-21-1609-fb24b546.md
 M docs/hub/LIES-MICH.md
 M docs/hub/hub.json
 M docs/offene-punkte.md
 M docs/plugin-wissen.md
 M eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md
 M eq-copilot/docs/FL-TERMIN-B-HOSTZEIT.md
 M tools/beweise.ps1
 M tools/hub/baue_hub.py
 M tools/hub/hub_eingang.py
 M tools/hub/seite.html
 M tools/hub/test/upload_probe.js
?? docs/beweise/SONDE-004.md
?? docs/beweise/termin-b/
?? docs/handoffs/auto-handoff-2026-08-21-2327-4ca90ff5.md
?? docs/handoffs/auto-handoff-2026-08-21-2333-60c295b3.md
?? docs/handoffs/auto-handoff-2026-08-22-0158-70b2680d.md
?? docs/handoffs/auto-handoff-2026-08-22-0200-9bb3d1c8.md
?? docs/handoffs/auto-handoff-2026-08-22-1246-95ab8db0.md
?? docs/hub/bilder/
?? eq-copilot/identity/host-capabilities-fl-v1.json
?? tools/eq-copilot/pruefe_host_capabilities.py
?? tools/hub/test/upload_probe_2.js
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-22 08:50:23 | `00E348AFF2B249F3` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-22 08:50:28 | `7377359932873BDE` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-22 08:49:55 | `763BBBE1CB0457CD` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-22 08:49:51 | `909D1C9D83AF0A9C` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 23:46:42 | `F68284E36B529CEB` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-22 08:48:44**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,53 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 5,98 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,55 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,63 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,12 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,16 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,24 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,98 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,23 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,22 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,18 s | [↓ A13](#a13) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,09 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,13 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,53 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 5,98 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,55 s

stdout:

```text

running 51 tests
test framing::tests::hin_und_zurueck ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::stale_wird_sichtbar_aber_nie_entfernt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 7 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 7 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.07s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,63 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,12 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,16 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,98 s

stdout:

```text
9 gueltige, 42 ungueltige Fixtures
  bytegleich: 52 Dateien, MANIFEST sha256=15bcde347cfa559c0b2992c5f9f5f6e7583f64e7b4ba33347b4c46f808942a35
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Smart Disable: kein ausgelassener Block in 12 s + 30 s Stille
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=supported traegt einen Termin (A + B)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=supported traegt einen Termin (A)
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=supported traegt einen Termin (A)
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen

HOST-CAPABILITIES OK - 36 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: PLUGIN_CODE unveraendert  [Eqcp]
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE unveraendert  [Evna]
  ok      CMake-Quelle: PRODUCT_NAME unveraendert
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [22 Aug 2026 1:34:32pm vs 22 Aug 2026 2:36:57am]
  ok      moduleinfo.json ist nach dem Kommaputz parsebar
  ok      moduleinfo: Produktname wie im Manifest  [EQ-Copilot]
  ok      moduleinfo: Vendor wie im Manifest  [evenacadia]
  ok      moduleinfo: genau zwei Klassen (Component + Controller)  [2]
  ok      moduleinfo: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      moduleinfo: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      moduleinfo: Unterkategorien wie im Manifest
  ok      moduleinfo: keine der vier reservierten Sonden-CIDs im heutigen Bundle
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 67 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,09 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      17 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [17]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 17 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [bbd51bfb120643f9805cd1ed10f7ffbd]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [f3171533fa704fa6b24ef75aa1b27380]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 130 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260822-133452.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (26871 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (51 Fixtures)
[ok]   Binaerkorpus hat Substanz  [51 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 6,22 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
```

</details>

