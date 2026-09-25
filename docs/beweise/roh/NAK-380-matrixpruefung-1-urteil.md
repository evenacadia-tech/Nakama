# NAK-380 — Matrixprüfung 1, Rohurteil (Codex gpt-6-astra, Effort high nach NAK-396, lesend; Vorlage A)

**Thread:** 01a0d6ac-9f89-7b83-8c7e-ab5d7051097f · **HEAD vor und nach dem Lauf:** 80205786862462b67b146a94f657155055493351 (Matrixstand 5f48fee3; darüber nur §10, drei Registerzeilen und der Prüfauftrag) · **Lauf:** 25.09.2026, 05:47:12 bis 05:56:30 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak380-m1`) · **Prüfbereich:** git diff b5a0bf9c...5f48fee3 auf docs/beweise/NAK-380.md (§5 bis §9, 121 Matrixzeilen) · **Auftrag:** docs/beweise/roh/NAK-380-matrixpruefung-1-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 30 lesende Kommandos aus.

---
URTEIL: NEEDS_WORK — 10 DEFEKTE

Geprüft: vollständiger Prüfbereich, Gate und Entscheide, 121 Matrixzeilen sowie Quellketten aller sechs Gegenstände am ZIEL; Git-Umfang und Zeilenzahl selbst gemessen, Formeln und Mutationsketten statisch nachgerechnet.

Nicht geprüft: Compiler, Tests, Kanon und Produktlaufzeit; Skripte und frühere Messergebnisse ausschließlich gelesen, externe Norm-PDFs nicht unabhängig abgerufen.

HEAD zu Beginn: 80205786862462b67b146a94f657155055493351
HEAD am Ende: 80205786862462b67b146a94f657155055493351
Arbeitsbaum unverändert; keine Dateien geschrieben.

FERTIG Matrixprüfung 1, NEEDS_WORK, 10 Defekte, 0 Lücken, 0 Härtungen

Full review comments:

- [P2] Beschränke die Größenbenennung auf spektrale Bandwerte — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:1533-1538
  DEFEKT (§8.2, M-17): `Bandwerte` wird auch für `Frame.band_stereo` verwendet (`nakama_telemetry_v1.fbs:253-256`). Dieses Feld trägt auf demselben 64er-Gitter dimensionslose Seitenanteile in [0,1]; `TelemetryClient.cpp:122-131` schreibt sie weiterhin so, und M-06 verlangt ausdrücklich 0,2609. Die Aussage, die Größe folge allein aus `gitter`, würde diese Werte dagegen als dBFS-Bandleistung deklarieren. Die Vertragspräzisierung muss den spektralen Bandsatz vom Stereoanteil unterscheiden, sonst widerspricht sie dem bestehenden Vertrag und der eigenen Matrix (R-380-1; [CLAUDE.md:178-181](CLAUDE.md#L178-L181)).

- [P2] Prüfe Rosa mit konstanter Leistung je Oktave — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:1081-1084
  DEFEKT (§7.1): Bei rosa Rauschen fällt die Leistungsdichte um 3,01 dB je Oktave, nicht die integrierte Oktavbandleistung: Für S(f)=C/f gilt ∫f bis 2f S(ν)dν=C·ln(2), unabhängig von f. Die vorgeschriebene Erzeugerprüfung verwirft daher korrektes rosa Rauschen bereits vor den Korpusfällen P1/P2 und könnte stattdessen braunes Rauschen akzeptieren. Das verletzt die zugesagte unabhängige Signalreferenz nach R-380-8; geprüft werden muss entweder konstante Oktavbandleistung oder die fallende Dichte.

- [P2] Berücksichtige die neuen Snapshot-Felder im Größenbudget — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:1030-1030
  DEFEKT (M-119): §8.6 verlangt `resonanzSucheAbHz` und zwei zusätzliche Masken im `MessSnapshot`. Am ZIEL enthält `AnalyseEngine` diesen Snapshot direkt als Mitglied `fertig` (`AnalyseEngine.h:369`); damit wächst auch `sizeof(AnalyseEngine)`, selbst wenn die Masken ihre Nutzdaten auf dem Heap halten. Die geforderte Gleichheit mit der Größe am Etappenstart widerspricht somit dem eigenen Bauplan und würde einen korrekten Bau ablehnen. Benenne die erlaubte, begrenzte Strukturvergrößerung statt exakter Gleichheit und erhalte die Heap-/Stapelwache nach [CLAUDE.md:271-273](CLAUDE.md#L271-L273).

- [P2] Plane den Nachtrag M-122 vor dem Bau der Etappe 6 ein — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:1729-1733
  DEFEKT (§8.6): Der Bauplan nennt weder M-122 noch den Versionsvergleich der Fingerprints. E-380-10/R-380-9 verlangt ausdrücklich, diesen Nachtrag vor dem Bau der Etappe 6 mit A4-Test und Rotbeweis vorzusehen. Die bloße Beobachtung B-2 ersetzt diesen Bauschritt nicht: `broker/src/telemetrie.rs:89-108` vergleicht weiterhin nur die Fingerprint-Vektoren, während Etappe 6 deren Erzeugung bei hohen Abtastraten verändert. Ergänze die geforderte Voraussetzung einschließlich des Broker-Testpfads; das Fehlen einer sofortigen M-122-Zeile in §6 ist hingegen kein Befund.

- [P2] Messe die Laufzeit auch in den Raten- und Blocksweeps — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:978-978
  DEFEKT (M-80): Die einzige numerische Laufzeitprüfung erbt aus M-73 ausschließlich 48 kHz und Block 512; M-74 bis M-79 prüfen nur Kohärenz. Eine Laufzeitrechnung mit fest verdrahteten 48 kHz würde deshalb sämtliche vorgesehenen Prüfungen bestehen, obwohl sie beispielsweise bei 96 kHz die falsche Verzögerung meldet. R-380-3 und das Gegenbeispiel aus §1 verlangen auch die Laufzeit mit Toleranz über die unterstützten Raten und Blockgrößen. Übernimm die Laufzeitassertion mit Referenz d/fs in die entsprechenden Sweepfälle.

- [P2] Wähle für die Binphase einen wirksamen Rotbeweis — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:979-979
  DEFEKT (M-81): Der referenzierte B19-Fall verwendet acht Samples Verzögerung, nicht 1 ms (`Sonde013StereoGoldenTest.cpp:502-503`, `:1211-1215`). Selbst eine Phasendifferenz über die gesamte rund 58 Hz breite 2000-Hz-Bandbreite beträgt damit nur etwa 0,061 rad, deutlich weniger als die erlaubten 0,25 rad; beim einzelnen Trägerton bleibt die Kreuzphase über dessen Leakage-Bins zudem nahezu gleich. Der Wechsel vom mittleren zum ersten Bin erzwingt daher nicht den behaupteten Fehler. Für die zugesagte Binwahl braucht M-81 ein unterscheidendes Signal und eine passende Toleranz, sonst fehlt der geforderte Rotbeweis an der Zusage.

- [P2] Prüfe die absolute Tickkadenz statt nur Ratengleichheit — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:1023-1023
  DEFEKT (M-112): Nach der geplanten Längenskalierung dauert der Mittenhop bei beiden Prüfraten gleich lange: 2048/48000 = 4096/96000 = 42,67 ms. Die vorgeschriebene Mutation erzeugt daher weiterhin gleiche Tickzahlen bei 48 und 96 kHz, nur mit falscher Kadenz; auch die Untergrenze von 15 Ticks bleibt erfüllt. Sie bricht die behauptete Ratengleichheit also nicht. Ergänze eine absolute Sollzahl beziehungsweise Tickzeit für die eingespeiste Aktivzeit, damit die zugesagte Musikzeit tatsächlich golden-beweisbar wird ([CLAUDE.md:187-188](CLAUDE.md#L187-L188)).

- [P2] Korrigiere die Hann-Korrelation der Welch-Referenz — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:896-896
  DEFEKT (M-32, weitergeführt in §7.4 und T-380-4): Der Code verwendet periodisches Hann mit 50 % Überlappung (`AnalyseEngine.cpp:73-87`). Dafür ist die normierte Überlappung (N/16)/(3N/8)=1/6 und die Korrelation benachbarter Periodogramme folglich 1/36, nicht 1/9. In der angegebenen Freiheitsgradformel ergibt K=8 damit ν≈15,26 statt 13,40 und nach derselben Wilson-Hilferty-Näherung etwa 2,39 statt 2,54 dB. Die als nachgerechnet bezeichnete unabhängige Referenz nach R-380-8 stimmt somit nicht zum spezifizierten Fenster; korrigiere auch die daraus abgeleiteten Referenzangaben.

- [P2] Nimm M-68 aus dem verpflichtenden Golden-Nachzug heraus — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:1456-1459
  DEFEKT (§8.1): M-68 verlangt ausdrücklich unveränderte Fingerprint-Bytes und führt den Golden als Regressionswache; T-380-5 hält dafür den alten Bandfluss unverändert. Hier wird für denselben Fall dagegen vorgeschrieben, dass neuer Code gegen den alten Golden zunächst rot sein muss und anschließend der Erzeuger läuft. Bei erfülltem M-68 bleibt dieser Vergleich gerade grün. Der Bauablauf darf keinen Golden-Nachzug verlangen, der die zu bewahrende Bytegleichheit aufhebt; für M-68 ist ausschließlich die dort benannte temporäre Mutation der Rotbeweis.

- [P2] Trenne bei M-08 die rote Messzusage von der Regressionswache — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-380.md:860-860
  DEFEKT (M-08): Am ZIEL summiert `Frame.h:343-349` weiterhin ungewichtete Feinbanddichten. Für das angegebene Weißrauschen ergibt Gruppe 63 deshalb ungefähr 10·log10(4·2/48000)=−37,78 dB und nicht die zugesagten −10,88±1 dB. Die vollständige Zeile ist somit bereits ohne Mutation rot und kann nach der Definition im §6-Kopf nicht ausschließlich eine Regressionswache sein. Trenne die neue Leistungszusage von der bestehenden Träger-/Validity-Wache und ziehe die Klassifikationszählung nach; sonst wird ein notwendiger Vorher-Rotbeweis ausgelassen.
