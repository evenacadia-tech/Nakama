# Rohausgabe - NAK-94 `[3c]` Byte-Kipp-Fuzz, VOLLSTAENDIG (jedes Byte jeder gelesenen JSON-Datei)

**Lauf:** 2026-08-30 08:05:06 +02:00 | **Bein:** A17 `tools/eq-copilot/pruefe_installer_manifest.py` | **Exitcode:** 0

**Manifest:** [../SONDE-007c.md](../SONDE-007c.md) - Abschnitt "NAK-94 Nacharbeit Runde 11"

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-30 08:05:06 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 01c9cd3 NAK-94 Nacharbeit 11: A17-Behauptung und Skriptkopf nennen die gemeinsame Schutzfunktion |
| Commit (voll) | 01c9cd31acb094a4d4f29f4a4bc0c6814d48c6aa |
| Arbeitsbaum (Messcode) | sauber (tools/, eq-copilot/, broker/) |
| Arbeitsbaum (gesamt) |  M docs/beweise/SONDE-007a.md;  M docs/beweise/SONDE-007c.md;  M docs/offene-punkte.md |
| Befehl | `py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --fuzz-voll` |
| Dauer des ganzen Beins | 870.9 s |
| Python | Python 3.13.14 |

### Urteil und Zaehler

| Zeile | Wert |
|---|---|
| Deckung |   ok      [3c/0] der Fuzz faehrt jeden Block aus _lauf(), der eine gelesene JSON-Datei anfassen kann - die uebrigen stehen namentlich in FUZZ_OHNE_JSON (8) |
| Repo-Manifest |   ok      [3c/1] der gefuzzte --hashen-Zweig hat das Manifest im Repo nicht angefasst - sha256 a214b58f0741bb15; sein Schreibziel ist die Kopie C:\Users\phili\AppData\Local\Temp\nakama-nak94-fuzz-manifest.json  [a214b58f0741bb15 -> a214b58f0741bb15] |
| `[3c]` |   ok      [3c] 8 gelesene JSON-Datei(en), 33304 gekippte Byte-Stellen, 61797 Laeufe: KEINE Ausnahme ausser Strukturhalt (18379) und dem eigenen 'Gegenprobe unmoeglich' (43); Befund 23263, gruen 20112  [870.0s] |
| Bein gesamt | 118 ok, 0 Fehler |

### Rohausgabe des `[3c]`-Blocks, woertlich

```text
[3c] Byte-Kipp-Fuzz - jede gelesene JSON-Datei, jedes Byte auf 0xFF und 0x20
  ok      [3c/0] der Fuzz faehrt jeden Block aus _lauf(), der eine gelesene JSON-Datei anfassen kann - die uebrigen stehen namentlich in FUZZ_OHNE_JSON (8)
      eq-copilot/install/nakama-installer-v1.json           19700 Laeufe | Strukturhalt  11122 | Gegenprobe unmoeglich    16 | Befund   250 | gruen  8312 | UNKONTROLLIERT 0
      eq-copilot/identity/plugin-identities-v1.json         12466 Laeufe | Strukturhalt   7257 | Gegenprobe unmoeglich    27 | Befund     0 | gruen  5182 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/MANIFEST.json   5411 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  3681 | gruen  1730 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/error-rueckgerollt.json   5977 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  5977 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/ok-erstinstallation.json   5785 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  5785 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/ok-nach-tausch.json   6139 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  6139 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/rueckweg-nach-gegenpfad.json   1431 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  1431 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/install/install-ergebnis.json               4888 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund     0 | gruen  4888 | UNKONTROLLIERT 0
  ok      [3c/1] der gefuzzte --hashen-Zweig hat das Manifest im Repo nicht angefasst - sha256 a214b58f0741bb15; sein Schreibziel ist die Kopie C:\Users\phili\AppData\Local\Temp\nakama-nak94-fuzz-manifest.json  [a214b58f0741bb15 -> a214b58f0741bb15]
  ok      [3c] 8 gelesene JSON-Datei(en), 33304 gekippte Byte-Stellen, 61797 Laeufe: KEINE Ausnahme ausser Strukturhalt (18379) und dem eigenen 'Gegenprobe unmoeglich' (43); Befund 23263, gruen 20112  [870.0s]
```

### Vollstaendige Rohausgabe des Beins, woertlich

```text
Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR dem ersten Zugriff strukturell geprueft;
                 Verstoss = kontrollierter Abbruch mit Klartext, nie ein Traceback - im urteilsfreien [4b] ein Hinweis.
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag  [2 vs 2 aktiv (3 Kennungen gesamt); identity=ok]
  ok      jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an verdorbener Eingabe
  ok      faellt an verdorbener Eingabe: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt an verdorbener Eingabe: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt an verdorbener Eingabe: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt an verdorbener Eingabe: Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag
  ok      faellt an verdorbener Eingabe: jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt
  ok      faellt an verdorbener Eingabe: jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`
  ok      faellt an verdorbener Eingabe: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt an verdorbener Eingabe: kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)
  ok      faellt an verdorbener Eingabe: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt an verdorbener Eingabe: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt an verdorbener Eingabe: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt an verdorbener Eingabe: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt an verdorbener Eingabe: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt an verdorbener Eingabe: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt an verdorbener Eingabe: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird
  ok      faellt, wenn ein stillgelegtes Ziel nirgends benannt ist
  ok      faellt, wenn die Stilllegungsmarke null ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke null ist
  ok      faellt, wenn die Stilllegungsmarke eine Zeichenkette ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke eine Zeichenkette ist
  ok      faellt, wenn die Stilllegungsmarke ein leeres Array ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein leeres Array ist
  ok      faellt, wenn die Stilllegungsmarke eine Zahl ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke eine Zahl ist
  ok      faellt, wenn die Stilllegungsmarke ein Boolean ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Boolean ist
  ok      faellt, wenn die Stilllegungsmarke ein leeres Objekt ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein leeres Objekt ist
  ok      faellt, wenn die Stilllegungsmarke ein Objekt mit leerem `am` ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Objekt mit leerem `am` ist
  ok      faellt, wenn die Stilllegungsmarke ein Objekt ohne `entscheid` ist
  ok      und das Ziel bleibt trotzdem stillgelegt, wenn die Marke ein Objekt ohne `entscheid` ist
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine leere Liste
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist ein Objekt
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine leere Zeichenkette
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist nur Leerraum
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id ist eine Zahl
  ok      faellt kontrolliert (ohne Absturz), wenn ziel_id fehlt ganz
  ok      faellt kontrolliert bei gemischten ziel_id-Typen in einer Liste
  ok      faellt, wenn `seit` null ist
  ok      faellt, wenn `seit` ein leeres Array ist
  ok      faellt, wenn `seit` ein leeres Objekt ist
  ok      faellt, wenn `seit` leer ist
  ok      faellt, wenn `seit` nur Leerraum ist
  ok      faellt, wenn `seit` eine Zahl ist
  ok      faellt, wenn `warum` null ist
  ok      faellt, wenn `warum` ein leeres Array ist
  ok      faellt, wenn `warum` ein leeres Objekt ist
  ok      faellt, wenn `warum` leer ist
  ok      faellt, wenn `warum` nur Leerraum ist
  ok      faellt, wenn `warum` eine Zahl ist
  ok      faellt, wenn `umgang_mit_altbestand` null ist
  ok      faellt, wenn `umgang_mit_altbestand` ein leeres Array ist
  ok      faellt, wenn `umgang_mit_altbestand` ein leeres Objekt ist
  ok      faellt, wenn `umgang_mit_altbestand` leer ist
  ok      faellt, wenn `umgang_mit_altbestand` nur Leerraum ist
  ok      faellt, wenn `umgang_mit_altbestand` eine Zahl ist
  ok      faellt, wenn `kennung_bleibt` null ist
  ok      faellt, wenn `kennung_bleibt` ein leeres Array ist
  ok      faellt, wenn `kennung_bleibt` ein leeres Objekt ist
  ok      faellt, wenn `kennung_bleibt` leer ist
  ok      faellt, wenn `kennung_bleibt` nur Leerraum ist
  ok      faellt, wenn `kennung_bleibt` eine Zahl ist
  ok      faellt, wenn ein aktives Ziel still aus der Auslieferung faellt
  ok      faellt an einem Produktnamen ausserhalb der Pfade
  ok      faellt an einem Bundlenamen ausserhalb der Pfade
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[3b] Gegenproben zu [4] Auslieferungsstand und [4b] installiertem Stand
     Gegliedert nach Zusagen Z1..Z7 - je Zusage ein eigener Bruch (B6-Zx).
  ok      Z1 [Writer-Korpus]: jeder in MANIFEST.json gefuehrte Fall liegt vor und ist bytegleich, keine verwaiste Datei daneben, und die Statusachse OK, RUECKWEG, ERROR_RUECKGEROLLT ist vertreten  [4 Faelle, Status ERROR_RUECKGEROLLT, OK, RUECKWEG]
  ok      Z2 [Kanon]: ein fehlendes Artefakt ist auch im Kanon ROT, wenn ein anderes keinen festgeschriebenen Hash traegt (Befund C1)  [eqcop-broker.exe: das festgeschriebene Artefakt liegt nicht vor  [broker/target/release/eqcop-broker.exe-GIBT-ES-NICHT]]
  ok      Z2 [Kanon]: das Artefakt ohne Hash wird trotzdem gemessen (liegt vor, Ordner-Hash bildbar) statt uebersprungen  [ok      main: Artefakt liegt vor, Ordner-Hash bildbar  [gebaut 6014545C7E5F18AF; kein festgeschriebener Hash zum Vergleich]]
  ok      Z2 [--release]: dieselbe Lage macht unter --release BEIDE Befunde zu Fehlern - den fehlenden Hash und das fehlende Artefakt  [nicht ausgeliefert - 1 Artefakt(e) ohne Hash: main | active-probe: gebautes Artefakt stimmt mit dem festgeschriebenen Hash  [Manifest 1DDC92E3B8525F1F | gebaut 990C462A5848D686]]
  ok      Z3 [Mutant von ok-erstinstallation.json: eintraege -> [Eintrag mit ziel_id als Liste, Nicht-Objekt]]: eine unbrauchbare Kennung IM JOURNAL ist ein Hinweis, kein TypeError - und [4b] faellt kein Urteil  [hinweis 0: Journaleintrag ohne lesbare Kennung (['main']) / hinweis 1: Journaleintrag ist kein Objekt (str)]
  ok      Z3 [Writer-Fixtur ok-erstinstallation.json]: dieselbe Zusage von der anderen Seite - ein Fehler IM MANIFEST bleibt ebenfalls ein Hinweis, [4b] toetet keinen Kanonlauf  [hinweis install-ergebnis.json nicht auswertbar: TypeError("unhashable type: 'list'")]
  ok      Z4 [Writer-Fixtur ok-erstinstallation.json]: bei Journalstatus OK und abgeschlossenen, nicht zurueckgerollten Eintraegen bleibt der Hashvergleich und sein ok (OK nach Erstinstallation)  [alle Artefakte ok]
  ok      Z4 [Writer-Fixtur ok-nach-tausch.json]: bei Journalstatus OK und abgeschlossenen, nicht zurueckgerollten Eintraegen bleibt der Hashvergleich und sein ok (OK nach Tausch)  [alle Artefakte ok]
  ok      Z5 [Writer-Fixtur error-rueckgerollt.json]: Journalstatus ERROR_RUECKGEROLLT meldet artefaktweise 'unbekannt' - ohne Hashvergleich und ohne 'keine Liste'  [Journal: status='ERROR_RUECKGEROLLT'  zeit='2026-08-29T23:13:25.0824342Z']
  ok      Z5 [Writer-Fixtur rueckweg-nach-gegenpfad.json]: Journalstatus RUECKWEG meldet artefaktweise 'unbekannt' - ohne Hashvergleich und ohne 'keine Liste'  [Journal: status='RUECKWEG'  zeit='2026-08-29T23:13:21.8923065Z']
  ok      Z5 [Mutant von ok-erstinstallation.json: status -> VORBEREITET]: meldet den installierten Stand als unbekannt - ohne Hashvergleich  [alle Artefakte als unbekannt gemeldet]
  ok      Z5 [Mutant von ok-erstinstallation.json: status -> KOMPENSATION]: meldet den installierten Stand als unbekannt - ohne Hashvergleich  [alle Artefakte als unbekannt gemeldet]
  ok      Z5 [Mutant von ok-erstinstallation.json: status -> ERROR_TEILSTAND]: meldet den installierten Stand als unbekannt - ohne Hashvergleich  [alle Artefakte als unbekannt gemeldet]
  ok      Z5 [Mutant von ok-erstinstallation.json: status -> ERROR_RUECKGEROLLT]: meldet den installierten Stand als unbekannt - ohne Hashvergleich  [alle Artefakte als unbekannt gemeldet]
  ok      Z5 [Mutant von ok-erstinstallation.json: status -> RUECKWEG_AKTIV]: meldet den installierten Stand als unbekannt - ohne Hashvergleich  [alle Artefakte als unbekannt gemeldet]
  ok      Z5 [Mutant von ok-erstinstallation.json: status -> RUECKWEG]: meldet den installierten Stand als unbekannt - ohne Hashvergleich  [alle Artefakte als unbekannt gemeldet]
  ok      Z5 [Mutant von ok-erstinstallation.json: status -> NEUER_STATUS_2099]: meldet den installierten Stand als unbekannt - ohne Hashvergleich  [alle Artefakte als unbekannt gemeldet]
  ok      Z5 [Mutant von ok-erstinstallation.json: status entfernt]: ein Journal OHNE status meldet den installierten Stand als unbekannt - Schweigen ist kein OK  [hinweis main: installierter Stand unbekannt (Journalstatus fehlt)]
  ok      Z6 [Mutant von ok-erstinstallation.json: eintraege entfernt]: bei Status OK ohne Eintragsliste bleibt es bei 'fuehrt keine Liste eintraege' - die Statussperre verschluckt sie nicht  [hinweis install-ergebnis.json fuehrt keine Liste 'eintraege']
  ok      Z7 [Writer-Fixturen]: jede eingefrorene Transaktions-ID besteht die Ist-TransaktionsId-Regex aus Install-Nakama.ps1 - eine gestrichelte UUID taete es nicht  [Muster '^[0-9a-f]{32}$' gegen ['35eedadd0d3b414e88a57efa4b592067', 'f7e6cf96074d4f2baf04efb141e6fbde', 'f7e6cf96074d4f2baf04efb141e6fbde', 'faa9dbe2c40e4d5f9ca3d7a4d9bf1cc1']]

[3c] Byte-Kipp-Fuzz - jede gelesene JSON-Datei, jedes Byte auf 0xFF und 0x20
  ok      [3c/0] der Fuzz faehrt jeden Block aus _lauf(), der eine gelesene JSON-Datei anfassen kann - die uebrigen stehen namentlich in FUZZ_OHNE_JSON (8)
      eq-copilot/install/nakama-installer-v1.json           19700 Laeufe | Strukturhalt  11122 | Gegenprobe unmoeglich    16 | Befund   250 | gruen  8312 | UNKONTROLLIERT 0
      eq-copilot/identity/plugin-identities-v1.json         12466 Laeufe | Strukturhalt   7257 | Gegenprobe unmoeglich    27 | Befund     0 | gruen  5182 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/MANIFEST.json   5411 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  3681 | gruen  1730 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/error-rueckgerollt.json   5977 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  5977 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/ok-erstinstallation.json   5785 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  5785 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/ok-nach-tausch.json   6139 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  6139 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/fixtures/installer/journale/rueckweg-nach-gegenpfad.json   1431 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund  1431 | gruen     0 | UNKONTROLLIERT 0
      eq-copilot/install/install-ergebnis.json               4888 Laeufe | Strukturhalt      0 | Gegenprobe unmoeglich     0 | Befund     0 | gruen  4888 | UNKONTROLLIERT 0
  ok      [3c/1] der gefuzzte --hashen-Zweig hat das Manifest im Repo nicht angefasst - sha256 a214b58f0741bb15; sein Schreibziel ist die Kopie C:\Users\phili\AppData\Local\Temp\nakama-nak94-fuzz-manifest.json  [a214b58f0741bb15 -> a214b58f0741bb15]
  ok      [3c] 8 gelesene JSON-Datei(en), 33304 gekippte Byte-Stellen, 61797 Laeufe: KEINE Ausnahme ausser Strukturhalt (18379) und dem eigenen 'Gegenprobe unmoeglich' (43); Befund 23263, gruen 20112  [870.0s]

[4] Auslieferungsstand  - Kanon: eine Abweichung ist ein Hinweis, kein Fehler
  hinweis main: Bau weicht vom festgeschriebenen Paket ab (nach Relink erwartet; vor einer Auslieferung --hashen)  [Manifest AC8102F23EDC7D7C | gebaut 6014545C7E5F18AF]
  hinweis active-probe: Bau weicht vom festgeschriebenen Paket ab (nach Relink erwartet; vor einer Auslieferung --hashen)  [Manifest 1DDC92E3B8525F1F | gebaut 990C462A5848D686]
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
