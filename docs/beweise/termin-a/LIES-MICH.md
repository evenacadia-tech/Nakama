# FL-Termin A — Aux/PDC/Recall: gemessen am 22.08.2026, Ergebnis **geht**

Messgerät `EqCop-Aux-Spike` (SONDE-004a, Wegwerfware, `NkSp`), installiert nach
`C:\Program Files\Common Files\VST3\` (FL scannt VST3 nur dort — die Klickliste
hatte „kein Admin nötig" behauptet, das war falsch). Klickliste:
`eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md`. Der User hat den Aufbau in FL
geklickt, die Berichte schrieb das Plugin nach `%APPDATA%\evenacadia\nakama\spike\`.

## Aufbau (vom User, Screenshots 22.08. 00:2x)

- Drei Audio-Kanäle mit derselben `impuls-44100.wav`, je ein Clip auf Takt 1,
  auf Insert 1 / 2 / 3.
- Insert 1 trägt das Messgerät und geht auf Master; Insert 2 und 3 gehen auf
  Insert 1 — erste Messung über den normalen Pfeil (Ton wird in Insert 1
  **mitgemischt** und steht zugleich als Sidechain bereit), zweite Messung über
  „Sidechain to this track" (nur Nebenweg).
- Wrapper → Processing → Connections: Sidechain **1** → `priority_sidechain`,
  Sidechain **2** → `compare_pre`, Main frei.

## Ergebnis

| Bus | 00:17 (vor Speichern) | 00:27 (nach Schließen + Neuladen) |
|---|---|---|
| Main | aktiv · 2 Kanäle · Impuls Sample 3 010 082 · Spitze 2,70 | aktiv · 2 Kanäle · Impuls 175 650 · Spitze 0,90 |
| `priority_sidechain` | aktiv · 2 Kanäle · **Versatz 0 Samples / 0,0 ms** | aktiv · 2 Kanäle · **Versatz 0 / 0,0 ms** |
| `compare_pre` | aktiv · 2 Kanäle · **Versatz 0 Samples / 0,0 ms** | aktiv · 2 Kanäle · **Versatz 0 / 0,0 ms** |

Host `FruityLoops`, Wrapper VST3, 44100 Hz, Blockgröße 2048,
`protokoll_eingehalten` auf allen Bussen, beide Male.

- **Zwei getrennte Nebenwege kommen gleichzeitig, getrennt und samplegenau an**
  (Versatz 0 gegenüber dem Hauptweg, in beiden Läufen).
- **Recall:** Buslayout (zwei benannte Busse aktiv, je ein Impuls) und Versatz 0
  überleben Speichern, Schließen, Neuladen — mit zwei Grenzen (T2 zu SONDE-004):
  zwischen den Läufen wurde das Routing von Mixroute auf Sidechain-only umgebaut
  (die Läufe sind nicht derselbe Projektstand), und Bus-Identität (welche Quelle
  auf welchem Aux) sowie L/R-Reihenfolge sind nicht unterscheidbar, weil dieselbe
  Impulsdatei (L = R) auf allen drei Spuren lag. **PDC wurde nicht ausgeübt** —
  kein Plugin im Aufbau meldete Latenz; der Fall aus Entwurf §32.2
  („Fehlreihenfolgen bei latenzbehafteten Instanzen") ist ungemessen (NAK-44).
  FL-Version nicht protokolliert (installiert: 25.2.5.5319 und 26.1.4.5589).
- Spitze 2,70 im ersten Lauf = dreifach gemischter Impuls (normale Routen in
  Insert 1); im zweiten Lauf 0,90 = nur Eigensignal (Sidechain-Routen). Für die
  Messung ohne Belang, für den späteren Aufbau die Regel: Nebenwege als
  „Sidechain to this track", nicht als Mixroute.
- Capability-Folge für S4 (`docs/beweise/SONDE-004.md`): `aux_compare_pre`
  **supported** (getrennt, recall-stabil); `aux_priority_sidechain` **unsupported**,
  bis ein Lauf mit latenzmeldendem Plugin „PDC-synchron" belegt (NAK-44). Nicht
  gemessen bleibt `contribution_aux` (Main-Aux-Busse) — kein Gerät misst es
  (Entwurf-Errata (k), NAK-04-Nachtrag).

Rohdaten: die zwei JSON-Dateien in diesem Ordner (unverändert kopiert).
