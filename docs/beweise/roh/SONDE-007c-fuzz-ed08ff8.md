# Rohausgabe - NAK-94 `[3c]` Byte-Kipp-Fuzz, VOLLSTAENDIG (jedes Byte jeder gelesenen JSON-Datei)

**Lauf:** 2026-08-30 09:40:14 +0200 | **Bein:** A17 `tools/eq-copilot/pruefe_installer_manifest.py` | **Exitcode:** 0

**Manifest:** [../SONDE-007c.md](../SONDE-007c.md) - Abschnitt "NAK-94 Nacharbeit Runde 12"

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-30 09:40:14 +0200 |
| Zweig | master |
| Messcode-Stand | ed08ff8 NAK-94 Nacharbeit 12: der Erzeuger liest das Korpusmanifest wie A17 |
| Messcode-Stand (voll) | ed08ff8aba790804d378ce8d7c444891750cd08a |
| Arbeitsbaum (Messcode) WAEHREND des Laufs | sauber - `tools/`, `eq-copilot/` und `broker/` standen unveraendert auf `ed08ff8` |
| Arbeitsbaum (uebriges) WAEHREND des Laufs | `docs/beweise/SONDE-007a.md`, `docs/beweise/SONDE-007c.md` und `docs/offene-punkte.md` in Arbeit (inzwischen committet als `b56b6e8`) - reine Doku, kein Messcode |
| Arbeitsbaum bei Erstellung dieses Belegs | ?? docs/beweise/roh/SONDE-007c-fuzz-ed08ff8.md |
| Befehl | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --fuzz-voll` |
| Dauer des `[3c]`-Blocks | 933.2 s |
| Python | Python 3.13.14 |

### Urteil und Zaehler

| Zeile | Wert |
|---|---|
| Deckung | ok      [3c/0] der Fuzz faehrt jeden Block aus _lauf(), der eine gelesene JSON-Datei anfassen kann - die uebrigen stehen namentlich in FUZZ_OHNE_JSON (8); Verbraucher: _fuzz_verbraucher, _fuzz_erzeuger |
| Repo-Manifest | ok      [3c/1] der gefuzzte --hashen-Zweig hat das Manifest im Repo nicht angefasst - sha256 a214b58f0741bb15; sein Schreibziel ist die Kopie C:\Users\phili\AppData\Local\Temp\nakama-nak94-fuzz-manifest.json  [a214b58f0741bb15 -> a214b58f0741bb15] |
| `[3c]` | ok      [3c] 8 gelesene JSON-Datei(en), 33304 gekippte Byte-Stellen, 61797 Laeufe: KEINE Ausnahme ausser Strukturhalt (21936) und dem eigenen 'Gegenprobe unmoeglich' (43); Befund 19706, gruen 20112  [933.2s] |
| Bein gesamt | 118 ok, 0 Fehler |

### Was diese Zahlen gegenueber Nacharbeit 11 sagen

Gleich geblieben sind Laeufe (**61797**), gekippte Byte-Stellen (**33304**) und - das Urteil - **UNKONTROLLIERT 0**. Verschoben hat sich genau eine Zahl: die 5411 Laeufe ueber `eq-copilot/fixtures/installer/journale/MANIFEST.json` liefern jetzt **3557 Strukturhalt** statt 0. Dieselben 3557 fehlen in der Befund-Spalte - gesamt 21936 statt 18379 Strukturhalt und 19706 statt 23263 Befund. Das ist der zweite Verbraucher: bis Nacharbeit 11 sah nur A17s Z1 diese Mutationen und meldete sie als Befund; seit Nacharbeit 12 liest `pruefen()` des Erzeugers dieselbe Datei ueber denselben Strukturvertrag und faellt vorher kontrolliert. Die Differenz ist damit kein Rauschen, sondern die Messung der Nacharbeit selbst.

### Rohausgabe des `[3c]`-Blocks, woertlich

```text
[3c] Byte-Kipp-Fuzz - jede gelesene JSON-Datei, jedes Byte auf 0xFF und 0x20
  ok      ok-erstinstallation.json  status=OK  438D8DB5B5550E53
  ok      rueckweg-nach-gegenpfad.json  status=RUECKWEG  5B99904AB9B6B80A
  ok      ok-nach-tausch.json  status=OK  1CB23A231AD96F14
  ok      error-rueckgerollt.json  status=ERROR_RUECKGEROLLT  5769F5ADDD2025BA

4 Writer-Fixturen bytegleich zum MANIFEST (Stand der Erzeugung: a010d64).
  ok      [3c/0] der Fuzz faehrt jeden Block aus _lauf(), der eine gelesene JSON-Datei anfassen kann - die uebrigen stehen namentlich in FUZZ_OHNE_JSON (8); Verbraucher: _fuzz_verbraucher, _fuzz_erzeuger
      eq-copilot/install/nakama-installer-v1.json           19700 Laeufe | Strukturhalt  11122 | Gegenprobe unmoeglich    16 | Befund   250 | gruen  8312 | UNKONTROLLIERT 0
      eq-copilot/identity/plugin-identities-v1.json         12466 Laeufe | Strukturhalt   7257 | Gegenprobe unmoeglich    27 | Befund     0 | gruen  5182 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/MANIFEST.json   5411 Laeufe | Strukturhalt   3557 | Gegenprobe unmoeglich     0 | Befund   124 | gruen  1730 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/error-rueckgerollt.json   5977 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  5977 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/ok-erstinstallation.json   5785 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  5785 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/ok-nach-tausch.json   6139 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  6139 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/rueckweg-nach-gegenpfad.json   1431 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  1431 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/install/install-ergebnis.json               4888 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund     0 | gruen  4888 | UNKONTROLLIERT 0
  ok      [3c/1] der gefuzzte --hashen-Zweig hat das Manifest im Repo nicht angefasst - sha256 a214b58f0741bb15; sein Schreibziel ist die Kopie C:\Users\phili\AppData\Local\Temp\nakama-nak94-fuzz-manifest.json  [a214b58f0741bb15 -> a214b58f0741bb15]
  ok      [3c] 8 gelesene JSON-Datei(en), 33304 gekippte Byte-Stellen, 61797 Laeufe: KEINE Ausnahme ausser Strukturhalt (21936) und dem eigenen 'Gegenprobe unmoeglich' (43); Befund 19706, gruen 20112  [933.2s]

[4] Auslieferungsstand  - Kanon: eine Abweichung ist ein Hinweis, kein Fehler
  hinweis main: Bau weicht vom festgeschriebenen Paket ab (nach Relink erwartet; vor einer Auslieferung --hashen)  [Manifest AC8102F23EDC7D7C | gebaut 6D3B791AE9DB2141]
  hinweis active-probe: Bau weicht vom festgeschriebenen Paket ab (nach Relink erwartet; vor einer Auslieferung --hashen)  [Manifest 1DDC92E3B8525F1F | gebaut 50C5A93DF6D55A40]
  ok      eqcop-broker.exe: gebautes Artefakt stimmt mit dem festgeschriebenen Hash  [21C7A8DC985BCA16]

[4b] Installierter Stand  - Bericht, kein Urteil
  Journal: status='OK'  zeit='2026-08-29T09:46:53.0057417Z'
  hinweis main: installierter Stand ist ein anderer als der im Manifest festgeschriebene  [installiert 4E0BED966D834BC1 | Manifest AC8102F23EDC7D7C]  C:\Program Files\Common Files\VST3\EQ-Copilot.vst3
  hinweis active-probe: installierter Stand ist ein anderer als der im Manifest festgeschriebene  [installiert AD7678B7C34A64FE | Manifest 1DDC92E3B8525F1F]  C:\Program Files\Common Files\VST3\Nakama Probeeq.vst3
  hinweis eqcop-broker.exe: installierter Stand ist ein anderer als der im Manifest festgeschriebene  [installiert 53808359C59B5D09 | Manifest 21C7A8DC985BCA16]  C:\Program Files\evenacadia\Nakama\eqcop-broker.exe

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

118 ok, 0 Fehler
```
