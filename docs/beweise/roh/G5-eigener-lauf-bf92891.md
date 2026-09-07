URTEIL: NEEDS_WORK

# G5 / P5 — eigener Falsifikationslauf des Dirigenten, 2026-09-07, HEAD `bf92891`

Prüfstufe T3. Quellstand unter `broker/`, `eq-copilot/`, `tools/` ist mit dem
Kanonstand `4df8855` identisch (`git diff --stat 4df8855..HEAD -- broker
eq-copilot tools` ist leer, vor und nach dem Lauf geprüft). Gate-Text
`docs/bauaufteilung-sonden.md`:395–400, harte Gates 6/7
`docs/FL-Nakama-Sonden-Design-Entwurf.md` §49.2:3571–3572, P5-Exit-Gate §59:4322–4324.
Referenz für die Zusagen: Verhaltensmatrix M-01 bis M-89
(`docs/beweise/SONDE-014.md` §3) sowie SONDE-013 §3 für Gate 6.

**Unterschied zum lesenden Codex-Prüfer: dieser Lauf hat gemessen.** Neben den
vorhandenen Beinen lief eine eigene Messbühne **außerhalb des Repos**
(`…/scratchpad/g5probe/`, Cargo-Pfadabhängigkeit auf `broker/`, ausschließlich
öffentliche API, kein Schreibzugriff auf das Repo). Vier der sechs Defekte sind
damit **gefahren**, nicht hergeleitet.

## Gefahrene Beine

| Bein | Ergebnis |
|---|---|
| `cargo test --test sonde014_hypothese` | 19 bestanden, 0 Fehler |
| `cargo test --test sonde014_proposal` | 18 bestanden, 0 Fehler |
| `cargo test --test sonde014_verdrahtung` | 17 bestanden, 0 Fehler |
| `cargo test --test sonde014_befund` | 7 bestanden, 0 Fehler |
| `cargo test --test sonde014_maskierung` | 5 bestanden, 0 Fehler |
| `cargo test --test sonde014_p5_korpus` | 1 bestanden, 0 Fehler |
| `cargo test --test sonde014_nacharbeit1` | 8 bestanden, 0 Fehler |
| `cargo test --test sonde014_nacharbeit2` | 4 bestanden, 0 Fehler |
| `cargo test --test sonde014_nacharbeit3` | 3 bestanden, 0 Fehler |
| `cargo test --test sonde013_verdrahtung` | 78 bestanden, 0 Fehler |
| `EqCopSonde014IntentTest.exe` | 210 Prüfungen, 0 Fehler |
| `EqCopSonde014BefundTest.exe` | 60/60 grün |
| `EqCopSonde014AssistentTest.exe` | 185/185 grün |
| `EqCopIpcTest.exe` | 381 Prüfungen, 0 Fehler |
| `pruefe_p5_korpus.py` | grün (Exit 0), 6 Sitzungen / 8 Befunde, Schwelle `hoch` |
| `pruefe_p5_korpus.py --selbsttest` | grün (Exit 0) |
| `erzeuge_p5_korpus.py --pruefen` | bytegleich (Exit 0) |
| eigene Bühne `g5probe::probe` | 7 bestanden, 1 Fehler (eigener Bühnenfehler: `experiment_begin.aktive_quellen` nimmt keine drei Quellen; als `probe2::b2` ohne Passage nachgeholt) |
| eigene Bühne `g5probe::probe2` | 6 bestanden, 0 Fehler |
| eigene Bühne `g5probe::probe3` | 1 bestanden, 0 Fehler |

**Der Kanonstand ist grün. Jeder Defekt unten liegt außerhalb dessen, was heute
ein Bein misst** — das ist der Kern dieses Berichts.

---

## B1 — Gate 6 im P5-Kontext

**Ein Pfad gefunden (am Quelltext hergeleitet, nicht gefahren); für die
Experiment-Seite: kein Pfad gefunden.**

**Durchgespielte Wege: 9.**

1. **Fließt ein Paarurteil unterhalb von `stark` in `prepost_paar`?** — **Ja.**
   `broker/src/coordinator/hypothese_verdrahtung.rs`:193–203 setzt
   `prepost_paar = true`, sobald der Deskriptor eine `pair_id` trägt **und**
   `paarurteile[(session, pair_id)].ergebnis.is_some()`. `ergebnis` ist in
   `broker/src/coordinator/prepost.rs`:1118–1128 für **jedes** Paar `Some`, bei
   dem überhaupt ein Restlag benannt werden konnte — auch für
   `Alignmentklasse::Probable`, also für ein Paar mit `AlignmentSchwach`,
   `TeilfensterLagFehlt`, `UeberlappungGering`, `AktivzeitKlafft`,
   `VerschiedeneHostPids` oder `KeinPresentationNachweis`. Für ein solches Paar
   liefert `dreifachergebnis` (:811–840, :842–852) **`ausgerichtet_db = None`,
   `match_gain_db = None`, `wirkung = None`** — SONDE-013 verweigert dort
   bewusst jede Aussage („Kein unbekannter Zeitpfad erzeugt eine starke
   Cross-Probe- oder PRE/POST-Aussage", :896–898).
2. **Wohin es fließt.** Nicht in die Rangkomponenten und nicht in die
   Klassenwahl (`hypothese.rs`:1389–1398 kennt nur `fenster`,
   `routing_bekannt`, `parent_duplikat`) — aber sehr wohl in
   `claim_class = Aussageklasse::Wirkungsbeleg` (:1353–1357), in
   `ursachenklasse = EffektkettePrePost` (:1360–1361), in
   `pre_post = Some("post")` (:1432) und in `next_test` (:1403–1411, der
   Schritt „PRE/POST-Paar messen" entfällt). Alle vier reisen über
   `befund_json` (`hypothese_verdrahtung.rs`:405–423) im `session_snapshot` zu
   Gen und werden dort in `SourcesModel.cpp`:817–836 übernommen.
   → **Ereignisfolge:** zwei Sonden mit gleicher `pair_id` und Messpunkt
   `pre`/`post` melden Evidenz; der Join bildet ein `Probable`-Urteil; die
   nächste Evidenzänderung erzeugt für beide Quellen einen Befund mit
   `claim_class = "wirkungsbeleg"`. **Sichtbares Ergebnis:** eine
   Wirkungsaussage (§36.1 Klasse 2) auf einem Paar, dessen ausgerichtetes
   Delta und dessen Wirkung ausdrücklich fehlen.
   → **Gefahren, aber nicht ausgelöst:** `g5probe::probe3::k` baut das Paar
   über den Produktpfad; mit der committeten Evidenzgrundform bleibt das Urteil
   `Unclear` (kein Restlag), also `ergebnis = None`, `prepost_paar = false`,
   `claim_class = zusammenhang`. Der `Probable`-Zweig braucht Hüllkurven mit
   Korrelationsspitze und damit neuen Fixturecode — deshalb **am Quelltext
   hergeleitet**. Der bestehende Fall
   `broker/tests/sonde013_verdrahtung.rs::prepost_join_laeuft_im_produktpfad`
   (:1113) belegt, dass der Produktpfad **immer** unter `FeatureAligned`
   bleibt, solange die Presentation-Abbildung nicht validiert ist.
3. **Speist ein Experimenturteil unterhalb `stark` einen Befund oder ein
   Proposal?** — **Nein.** `aufnahmen_sammeln` liest aus dem Experimentbestand
   ausschließlich `juengste_passage_im_projekt` (`hypothese_verdrahtung.rs`:236–249,
   `experiment.rs`:919–923), also ein Zeitfenster. Weder `urteil` noch
   `vergleichbarkeit`, `taint`, `match_gain` oder Alignmentwert erreichen
   `Aufnahme`, `CauseHypothesis` oder `Proposal` (grep über `coordinator/`:
   `vergleichbarkeit::` wird in `hypothese*.rs` und `proposal*.rs` nur für
   `GATE_ABDECKUNG`, `GATE_MINDEST_FENSTER`, `GATE_ZEITUEBERDECKUNG` und
   `METRICS_VERSION` benutzt).
4. **`proposal.rs::wirkung` (:818–830)** liest `target_metric` und
   `ursachenklasse`, nie ein Experimenturteil. `confidence` des Proposals ist
   1:1 die des Befunds (:735).
5. **Kann Gen ein Urteil oder eine Vergleichbarkeit setzen, die der Broker
   übernimmt?** — **Nein für Urteil/Vergleichbarkeit, ja für die
   Rollenpriorität.** `user_verdict` (`PluginProcessor.cpp`:2051–2061) trägt
   `urteil ∈ {angenommen, abgelehnt, spaeter, enthaltung}` und wird im Broker
   nur als P0-Ereignis persistiert (`befehl.rs`:545–568) — es fließt in keinen
   Befund und in kein Proposal. `assistant_step_update` (:1979–1992) schreibt
   nichts, es vergleicht nur (:2179–2197). `vergleichbarkeit` und das
   fünfwertige Versuchsurteil sind in Gen reine **Lese**felder
   (`SourcesModel.cpp`:672–680). Der eine Weg, auf dem Gen eine Rangkomponente
   setzt, ist `intent_update.intents[].rolle`: er wird über
   `ROLLENPRIORITAET` (`hypothese.rs`:333–341) zur Komponente
   `intent_relevanz`. Das ist Absicht (§36.2) und keine Vergleichbarkeit.

---

## B2 — Gate 7: „Standard-Insertprobe als exakter Mastersummenbeitrag"

**kein Pfad gefunden.**

**Durchgespielte Wege: 7.**

1. Der Feldkanon von `$defs/session_finding` kennt **kein** Feld für einen
   Summenbeitrag. Vollständige Menge: `finding_id`, `claim_class`,
   `ursachenklasse`, `target_metric`, `candidate_source`, `pre_post`,
   `passage_id`, `band_hz`, `beobachtung`, `rang`, `confidence`,
   `evidence_ids`, `alternatives`, `ausschluesse`, `next_test`, `zustand`,
   `intent_revision`, `likely_cause`, `smallest_test`, `listen_for`,
   `maskierung` — `additionalProperties: false`
   (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `$defs/session_finding`).
2. `ursachenklasse = "summe_auf_master"` (`hypothese.rs`:76) ist ein **Wort**
   aus einer Siebenermenge, kein dB-Wert, und wird von `baue_befund` heute gar
   nicht erzeugt (:1358–1364 kennt nur `ZweiQuellenKonkurrenz`,
   `EffektkettePrePost`, `QuelleResonanz`).
3. `maskierung.wert_db` ist ausdrücklich „wie weit `quelle_b` im Bandbereich
   **über** `quelle_a` liegt" (`maskierung.rs`:58) — eine Pegeldifferenz
   zwischen zwei Messpunkten, kein Anteil an einer Summe. Der Vertragskommentar
   sagt dasselbe.
4. Die drei Textzeilen erfinden keine Zahl:
   `likely_cause = "<8 Hexzeichen> draengt im Bandbereich <von>..<bis> gegen den
   Master."` (`hypothese.rs`:1448–1453) nennt Bandindizes, keinen dB-Beitrag;
   `smallest_test` bildet `next_test` ab (:1455, Abbildung am Enum :237–247);
   `listen_for` ist ein Hörziel (:1456).
5. `expected_effect` ist eine geschlossene Vierermenge
   (`reduce_masking_pressure`, `reduce_peak_pressure`, `restore_stereo_balance`,
   `none`, `proposal.rs`:117–122) — kein Zahlenversprechen. `target` ist eine
   `hex32`-Instanzkennung, keine Beitragsaussage.
6. Auf der Gen-Seite trägt kein gezeichnetes Element einen Quellen-dB-Wert:
   `PluginEditor.cpp` liest aus `Sicht` ausschließlich `quellen[i]`
   (:1136–1184, :1218–1322); `Sicht::befunde` hat in `PluginEditor.cpp` und
   `PluginProcessor.cpp` **null** Leser. Der einzige quellenbezogene dB-Wert der
   Fläche ist die eigene integrierte Lautheit der Sonde
   (`PluginEditor.cpp`:1305–1308), und die ist ohne Konfidenz gesperrt
   (:1299–1303). `"contribution_aux"` (`SourcesModel.cpp`:183) ist ein
   Capability-Schlüssel mit den Werten `supported`/`unsupported` und wird
   nirgends gespeichert oder angezeigt.
7. **Wo trägt die Kette die Unterscheidung „gemessen an der Sonde" ↔ „Beitrag
   zur Summe"? Nirgends als Feldname** — siehe LÜCKE L1. Sichtbar wird daraus
   heute nichts, weil Gen das betroffene Feld nicht zeichnet.

---

## B3 — Falsche starke Ursachenbehauptung, korrelierter Distraktor

**Zwei Pfade gefunden, beide gefahren.**

**Durchgespielte Wege: 12.**

### Pfad 1 — Distraktor mit mehr Fenstern und mehr Bandenergie (DEFEKT D2)

*Eingabe* (`g5probe::probe2::h_distraktor_mit_mehr_fenstern`): Main auf
Mixerkanal 0, wahre Quelle auf Kanal 3 mit 12 Fenstern à 512 Samples und
+9 dB im Band 98..102, Distraktor auf Kanal 4 mit **24** Fenstern à 256 Samples
Schritt und **+12 dB** im selben Band; Vollständigkeitsmarke vor der Evidenz;
alle Belege über `Senke::p1`.

*Ereignisfolge*: beide bestehen alle Gates (`hypothese.rs`:797–892) →
`bandpassung` 0,173 (Distraktor) gegen 0,100 (wahre Quelle),
`routingqualitaet` 1,0 für beide → Ränge 0,2789 gegen 0,2666 → **getrennt**
(:1212–1220), also greift weder der Riegel „nur der führende Befund darf stark
sein" (:1187–1192) noch der Gleichstandsriegel (:1221–1224).

*Sichtbares Ergebnis* (gemessen):
`candidate_source = Distraktor, confidence.class = hoch, zustand = ready_to_send,
ursachenklasse = zwei_quellen_konkurrenz, alternatives = [wahre Quelle]`;
die wahre Quelle steht als `mittel` / `more_data` daneben.
**Die Rangkomponenten `koinzidenz`, `uplift` und `wiederholbarkeit` stehen dabei
auf 0,000** — die starke Aussage ruht auf Bandenergie, Intentmitte und
Routingqualität, nicht auf einem Zusammenhang.

*Heutige Messung*: keine. Der Korpus baut den Distraktor **materialgleich**
(`sitzungen.json`: beide 12 Fenster, beide 9,0 dB, beide Versatz 0), und nur
deshalb greift der Gleichstandsriegel. Kein Fall in `broker/tests/sonde014_*.rs`
gibt zwei konkurrierenden Sonden ungleiche Fensterzahlen.

*Gegenprobe an der Projektlogik*: `tools/eq-copilot/pruefe_p5_korpus.py::_passt`
(:187–193) wertet genau diese Lage als rot — „`sicherheit == HOCH and not
ist_wahre_ursache` → False" — und `kennzahlen` (:112–116) zählte sie als
`falsche_starke`.

### Pfad 2 — Alignment-Gate ist spannenblind (DEFEKT D4)

*Eingabe* (`g5probe::probe::d_umklammernde_spanne_ohne_ueberlappung`): keine
Passage (der Normalfall der passiven P5-Sitzung — der P5-Korpus legt in keiner
seiner sechs Sitzungen eine an); Master 12 Fenster ab `BASIS`; Sonde 6 Fenster
20 000 Samples **vor** und 6 Fenster 40 000 Samples **nach** dem Master. Kein
einziges Sondenfenster überlappt ein Masterfenster.

*Ereignisfolge*: `gate()` Schritt 3 (`hypothese.rs`:823–829) rechnet
`zeitueberdeckung(spanne(kandidat.fenster), spanne(master.fenster))`.
`spanne()` (:1563–1574) ist ein reines min/max und kennt keine Lücken; die
Sondenspanne umklammert die Masterspanne, `zeitueberdeckung` normiert auf das
**kürzere** Intervall (:1581–1592) und liefert **1,0 ≥ 0,95**. Das Gate hält.
Ohne Passage entfällt Schritt 4 vollständig, und `baue_befund` zählt
`unabhaengige_fenster` über die ganze Historie (:1372–1375): 12 ≥ 8.

*Sichtbares Ergebnis* (gemessen): `confidence.class = hoch`,
`zustand = ready_to_send`, `ursachenklasse = quelle_resonanz`,
`koinzidenz = 0,000`, `uplift = 0,000`, `wiederholbarkeit = 0,000`,
`ausschluesse = []`. Eine Quelle, die zu **keinem** Zeitpunkt zugleich mit der
Masteranomalie gemessen hat, wird als starke Ursache ausgewiesen.

### Die übrigen zehn durchgespielten Wege (kein Pfad)

3. **Kandidatendeckel fünf** — `ueberlebende.truncate(KANDIDATEN_DECKEL)`
   (:1154) schneidet **nach** der Sortierung; der Gleichstandsvergleich
   (:1212–1220) läuft über Platz 1 gegen Platz 2, beide innerhalb des Deckels.
   Ein abgeschnittener Kandidat kann keine falsche Trennung erzeugen.
4. **Bedingter Uplift ohne Vergleichsfenster (M-19)** — `uplift` liefert dann
   0,0 (:958–962), aber die Klassenwahl liest ihn nicht (:1389–1398). Ein
   Kandidat **kann** also `hoch` werden, ohne dass ein unabhängiges Fenster
   existiert; das ist gemessen (alle Bühnen oben zeigen `uplift = 0,000` bei
   `hoch`). M-19 sagt zur **Klasse** nichts und M-15 verbietet ausdrücklich, die
   Klasse aus dem Score abzuleiten — deshalb LÜCKE L3, kein Defekt.
5. **Randwert `GATE_MINDEST_FENSTER`** — `<` (:1389, :853), also sind genau 8
   Fenster zulässig. `broker/tests/sonde014_nacharbeit1.rs`:453–455 misst
   exakt 8/8 als Gegenprobe. Korrekt.
6. **Reihenfolge nach ID** — Tie-Break ist `candidate_source` aufsteigend
   (:1148–1152); er entscheidet nur bei gleichem quantisiertem Rang, und dann
   fällt der Führende ohnehin auf `mittel` (:1221–1224). Gemessen von
   `sonde014_hypothese.rs::ungetrennter_erster_platz_ist_nicht_stark`:890.
7. **Ausfall der Vollständigkeitsmarke (M-86)** — fail-closed an **einer**
   Stelle (`intent::darf_gerechnet_werden`:136–138), gerufen in
   `aufnahmen_sammeln`:157 **und** `hypothesen`:1102. `intent == None` sperrt.
8. **Intent-Änderung nach der Rechnung (M-10)** — gemessen
   (`g5probe::probe2::i`): nach `bestand_revision 7` steht der Befund auf
   `zustand = stale`, die Zahlen bleiben (§37.3 wörtlich). Kein alter `hoch`-
   Befund bleibt als READY sichtbar. Das mitlaufende Proposal behält allerdings
   `confidence.class = hoch` → HÄRTUNG H2.
9. **Interleaving Rechnung/Eintragung** — `ergebnis_ist_noch_gueltig`
   (`hypothese_verdrahtung.rs`:327–330) verwirft ein Ergebnis, dessen Grundlage
   sich während der Rechnung geändert hat; gemessen von
   `sonde014_verdrahtung.rs::veraltetes_rechenergebnis_wird_nicht_veroeffentlicht`:487.
10. **Zwei Master in einer Sitzung** — gemessen (`g5probe::probe::c`): der
    letzte `main` in ID-Ordnung wird stiller Master (`hypothese_verdrahtung.rs`:215–216),
    die Befunde entstehen weiter. Eine falsche **starke** Aussage entsteht daraus
    nicht; die Folge liegt im Proposal (DEFEKT D5).
11. **Distraktor mit identischem Verlauf** — vom Gleichstandsriegel gefangen,
    gemessen im Korpus und in `sonde014_hypothese.rs`:857/:889.
12. **Routing unbekannt** — gemessen (`g5probe::probe::g`): `routing = 0,000`,
    Klasse `mittel`, Zustand `more_data`. Korrekt.

---

## B4 — Falsche starke Ursachenbehauptung, Parent-Duplikat

**Ein Pfad gefunden, gefahren (DEFEKT D1).**

**Durchgespielte Wege: 8.**

*Wurzel*: `hypothese_verdrahtung.rs`:178–188 trägt **jeden** Client mit
`host_mixer_index` in `je_kanal` ein — **einschließlich des `main`**. :226–235
setzt `kandidat.parent` auf die **erste andere** Kennung desselben Kanals; ist
der Main dabei und sortiert seine `instance_id` vorn, zeigt jede Sonde des
Kanals auf den **Main**. In `hypothese.rs`:1113–1117 enthält `ids` aber nur
`aufnahme.kandidaten` — der Main steht dort nie. Also gilt
`kandidat.parent.is_some_and(|p| ids.contains(p)) == false` und ebenso
`ist_parent(kandidat) == false` (:1134–1138): **`parent_duplikat` bleibt
falsch**, `routingqualitaet` bleibt 1,0 (:1064–1072) und die Klassenwahl
(:1391) verliert ihren Duplikatterm.

*Zwei gemessene Ausprägungen:*

| Bühne | Aufbau | Ergebnis (gemessen) |
|---|---|---|
| `g5probe::probe::b` | Main auf Kanal 0, **eine** Sonde ebenfalls auf Kanal 0 | `routing = 1,000`, `klasse = Hoch`, `zustand = ReadyToSend` — die Sonde, die **denselben Mixerkanal wie der Master** misst, wird als starke Ursache der Masteranomalie ausgewiesen |
| `g5probe::probe2::b2` | Main **und zwei** Sonden, alle auf Kanal 0 | beide Sonden `routing = 1,000`; führender Befund `Hoch` / `ReadyToSend`, zweiter `Mittel` |
| `g5probe::probe2::b3` (Gegenprobe) | zwei Sonden auf Kanal 5, Main auf 0 | beide `routing = 0,500`, **beide `Mittel` / `MoreData`** — der Riegel greift, sobald der Main nicht im Spiel ist |

Die Gegenprobe zeigt, dass der Unterschied genau am Main hängt und nicht am
Material.

*Die übrigen sechs Wege (kein Pfad):* Parent außerhalb des Deckels —
`ist_parent` und `ids` lesen die **volle** Kandidatenliste, `truncate` läuft
danach (:1154), das Duplikatbit steht also schon; Parent zurückgenommen —
`zurueckgenommen` (:213) entfernt ihn aus den Kandidaten, dann ist er auch kein
Duplikatpartner mehr (konsequent, siehe LÜCKE L4); Parent mit unbekanntem
Routing — er steht dann gar nicht in `je_kanal`, das Kind bleibt einzeln und
erreicht `hoch` (LÜCKE L4); Kanal 0 — kein Sonderfall, `Value::as_i64` liefert
0 wie jede andere Zahl; Kanalwechsel zwischen Fenstern — der Deskriptor ist
zustandsbehaftet, es zählt der letzte; andere `metrics_version` — `Aufnahme`
trägt eine sitzungsweite Fassung (`hypothese_verdrahtung.rs`:251), zwei
Fassungen nebeneinander gibt es nicht.

---

## B5 — Falsche starke Ursachenbehauptung, verschobene Passage

**Ein Pfad gefunden, gefahren (DEFEKT D3).**

**Durchgespielte Wege: 9.**

*Eingabe* (`g5probe::probe2::e2_alle_fenster_beruehren_die_passage`): Passage
`[BASIS, BASIS+6144)` über `experiment_begin` als P0 angelegt, Master 12 Fenster
darin; Sonde 4 Fenster 60 000 Samples **vor** der Passage (sie spannen den
Kandidaten über die Passage auf) plus **10 Fenster, die jeweils mit genau einem
Sample in die Passage ragen** (Start = `passage_bis - 1 - k`, Länge 512).

*Ereignisfolge*: Gate 4a (`hypothese.rs`:832–845) rechnet wieder über die
**Spanne** → 1,0, also keine `PassageUnvergleichbar`. Gate 4b (:853) ruft
`passagenmaterial` → `fenster_in_passage` (:1612–1623), und dessen Bedingung ist
`f.projekt_bis > passage.projekt_von && f.projekt_von < passage.projekt_bis`,
also **jede Überlappung ≥ 1 Sample**. Zehn Fenster, die zu 99,98 % außerhalb
liegen, zählen als zehn „unabhängige Fenster innerhalb der Passage"; 10 ≥ 8.

*Sichtbares Ergebnis* (gemessen): `confidence.class = hoch`,
`zustand = ready_to_send`, `ausschluesse = []`, `evidence_ids = 14`.

*Gegenprobe*: dieselbe Bühne mit nur **einem** wirklich berührenden Fenster
(`g5probe::probe::e`) fällt korrekt mit `PassageZuKurz` und liefert die
Enthaltung `daten_reichen_nicht` / `unklar` / `more_data`.

*Die übrigen acht Wege (kein Pfad):* gleiche `passage_id` mit verschobenem
Fenster → Gate 4a fällt, sobald die Spanne nicht klammert (Korpusfall
`verschobene_passage`, Versatz 6 → Alignment 0,500, `alignment_falsch`);
andere `transport_epoch` → `PassageUnvergleichbar` (:840);
`passage_update` während der Rechnung → `ergebnis_ist_noch_gueltig` verwirft;
Passage ohne Fingerprint / aus fremder Sitzung → `juengste_passage_im_projekt`
ist **projektgebunden** (`experiment.rs`:919–923), eine fremde Sitzung liefert
keine; Loop-Wrap → neue `transport_epoch`, Gate 4a fällt; Passage über einen
Epochenwechsel → dito; `juengste_passage_im_projekt` wählt eine **andere**
Passage als die von Gen angezeigte → das ist möglich (LÜCKE L5), erzeugt aber
keine starke Aussage, sondern zusätzliche Ausschlüsse.

---

## B6 — Exit-Gate Satz 1: „Jede sichtbare Behauptung referenziert existente Evidenz-IDs"

**kein Pfad gefunden.**

**Durchgespielte Wege: 10.**

1. **Rücknahme zwischen Rechnung und Zustellung** — `hypothesen_bilden`
   (`hypothese_verdrahtung.rs`:72–106) rechnet ohne Lock, aber
   `befunde_eintragen` prüft **unter demselben Lock** mit
   `ergebnis_ist_noch_gueltig` (:327–330) und verwirft ganz („entweder alle
   Befunde oder keiner"). Gemessen: `sonde014_verdrahtung.rs`:487.
2. **Härtung gegen den Store beim Eintragen** — `befunde_gegen_store_haerten`
   (:635–667), gemessen `sonde014_nacharbeit1.rs`:318 und :372.
3. **Re-Subscribe aus der gespeicherten Projektion** —
   `projektion_gegen_store_haerten` (:691–780) härtet die committeten Bytes,
   gemessen `sonde014_nacharbeit2.rs`:597.
4. **Storefehler** — beide Härtungen sind ausdrücklich **fail-safe**
   (:653–657, :727–729): eine gescheiterte Abfrage löscht keine belegte
   Behauptung. Das ist die richtige Richtung: sie kann keine Behauptung ohne
   Beleg **erzeugen**.
5. **Kappung auf 32 IDs** (:1421–1424) kann nicht greifen:
   `EVIDENZ_RETENTION = 32` (`evidenz.rs`:39) deckelt die Historie bereits, die
   Kappung ist eine Wache. Auch `enthaltung()` (:1316–1321), das **keine**
   Kappung trägt, kann deshalb `maxItems: 32` nicht reißen.
6. **Teil-/Vollrücknahme** — `befunde_invalidieren_locked` (:557–599): fehlen
   alle IDs → Befund fällt aus der Liste; fehlen einige → `stale`; ein
   unsichtbar gewordener Befund wird zusätzlich aus den `alternatives` der
   übrigen entfernt (:592–594). Ein Verweis ins Leere entsteht nicht.
7. **Zustellung nach der Rücknahme** — `befunde_nach_ruecknahme_zustellen`
   (`invalidierung_verdrahtung.rs`:385–395) markiert dirty und flusht; Gen
   ersetzt seine Liste vollständig (`SourcesModel.cpp`:1222
   `befunde = std::move(geleseneBefunde)`), auch mit leerem Ergebnis.
8. **Gen behält Befunde nach `evidence_invalidate`** — ja, bis zum nächsten
   Snapshot (`SourcesModel.cpp`:1378–1396 rührt `befunde` nicht an), aber
   genau dieser Snapshot wird durch (7) ausgelöst. Kein dauerhafter Zustand.
9. **Sitzungswechsel** — `SourcesModel.cpp`:439–470 räumt `befunde`,
   `findingsOffen`, Rücknahmezähler gemeinsam.
10. **`user_verdict` mit unbekannter `finding_id`** — wird persistiert, ohne
    dass der Befund existieren muss (`befehl.rs`:545–568). Das ist eine
    **Useraussage**, keine sichtbare Systembehauptung → LÜCKE L6.

Nebenbefund zur Gen-Seite: `alternatives` wird dort nur auf hex32 geprüft, nie
gegen die Befunde desselben Snapshots aufgelöst (`SourcesModel.cpp`:927–944).
Weil der Broker die Referenzintegrität bei jeder Rücknahme herstellt, wird
daraus heute keine leere Referenz → HÄRTUNG H1.

---

## B7 — Exit-Gate Satz 2: „jedes Proposal nennt Ziel, Passage, Grenzen, Hörziel, Stopbedingung und Rückweg"

**Ein Pfad gefunden, gefahren (DEFEKT D5).**

**Durchgespielte Wege: 8.**

*Eingabe* (`g5probe::probe::c_zwei_mains`): normale Sitzung (Main auf Kanal 0,
Sonde auf Kanal 3, Passage, 12 Fenster je Seite) — dann tritt **eine zweite
`main`-Instanz** derselben Sitzung bei; danach kommt weitere Evidenz.

*Ereignisfolge*: `FUEHRENDE_MAINS_PRO_SESSION = 1` (`mod.rs`:118), also setzt
`fuehrung_neu_bewerten_locked` (`mitgliedschaft.rs`:87–92)
`fuehrendes_main = None`, sobald zwei bestätigte Mains existieren.
`aufnahmen_sammeln` kennt diese Regel nicht und nimmt weiter einen Master
(`hypothese_verdrahtung.rs`:215–216), es entstehen also weiter Befunde.
`proposallage_locked` (`proposal_verdrahtung.rs`:113–119) füllt
`ziel_instanz` mit `…fuehrendes_main.clone().unwrap_or_default()` → **leerer
String**. `proposal()` (`proposal.rs`:653–742) prüft nur `passage_id` (:656)
und schreibt `target: lage.ziel_instanz.clone()` (:719).

*Sichtbares Ergebnis* (gemessen, wörtlich aus dem Lauf):
`gate_felder = [("target", false), ("passage_id", true), ("allowed_bounds",
true), ("listen_for", true), ("stop_if", true), ("revert", true)]`, und der
Vorschlag wird **persistiert**: `vorschlag_persistieren`
(`proposal_verdrahtung.rs`:153–180) legt ihn als `event_type = "proposal"` im
Store ab, mit dem Payload `…,"target":"",…`. `$defs/proposal` verlangt `target`
als `hex32` (`minLength: 32`, `pattern ^[0-9a-f]{32}$`) — das persistierte
Objekt ist **schemaungültig**.

*Warum es niemand merkt*: `Proposal::gate_felder_vollstaendig` (`proposal.rs`:
360–374) hat im ganzen Repo **keinen Produktivaufrufer** — nur
`broker/tests/sonde014_proposal.rs`:360/:959 und
`sonde014_nacharbeit1.rs`:727 / `nacharbeit2.rs`:713, und alle vier bauen ihre
`Proposallage` von Hand mit `ziel_instanz: hex(1)`
(`sonde014_proposal.rs`:544, :623, :762, :844, :941). Kein Fall im Repo prüft
`target` am Produktpfad (grep `"target"` über `broker/tests/sonde014_*.rs`:
null Treffer).

*Die übrigen sieben Wege (kein Pfad):*

3. **Ohne Passage kein Objekt** — gemessen (`g5probe::probe::f`,
   `g5probe::probe2::b2`, `::b3`, `::h`): `Vorschlaege: 0`. WN-04 hält
   (`proposal.rs`:656, `proposal_verdrahtung.rs`:58–61).
4. **`stop_if` leer** — `stopbedingungen` (:854–869) beginnt immer mit
   `GuardrailAbdeckung`; `minItems: 1` ist strukturell erfüllt.
5. **`listen_for` leer** — kommt aus dem Befund (:731), und `baue_befund`
   /`enthaltung` setzen dort immer einen Satz (:1456, :1331).
6. **`revert`** — `Rueckweg` ist ein dreiwertiges Enum ohne Leerwert
   (:132–136); `dsp_revert` existiert in der Rust-Menge nicht.
7. **Vertrag ↔ Serializer** — `proposal_json` (`proposal_verdrahtung.rs`:310 ff.)
   schreibt alle fünfzehn Pflichtfelder; gemessen von
   `sonde014_proposal.rs::proposal_traegt_die_fuenfzehn_felder`:292 gegen die
   `required`-Liste des eingefrorenen Schemas.
8. **Prüft Gen die sechs Felder?** — Gen liest `draft_offer` überhaupt nicht:
   `WireEnvelope.h`:59–64 nennt den Namen nur im Kommentar, `v3Antwort`
   (`PluginProcessor.cpp`:2455–2473) kennt zwei Familien, und
   `allowed_bounds`/`stop_if`/`revert`/`expected_effect`/`protected_traits`
   haben in `plugin/src`, `plugin/core` und `plugin/state` null Treffer. Die
   Vollständigkeit hat auf der Leseseite heute keinen Prüfer (P6-Lieferumfang,
   ausgeschlossen).
9. **Formal gesetzte Felder** — `("revert", true)` ist konstant wahr, und
   `allowed_bounds` gilt für `no_change`/`more_data` als „genannt", obwohl das
   Objekt `{}` ist (:366–369, gemessen: `"allowed_bounds":{}` im Lauf). Das ist
   die ausdrückliche Lesart von M-46 („sie greifen nicht ein und tragen deshalb
   keine") → LÜCKE L7, kein Defekt.

**Nebenmessung, die den ganzen Abschnitt einordnet:** in P5 entsteht **nie** ein
handelbares Proposal. `messbare_guardrails` (`proposal_verdrahtung.rs`:132–136)
enthält `GuardrailLoudness` nicht, `stopbedingungen` fügt es für
`band_pegel_db` aber immer hinzu (`proposal.rs`:859) — also ist `alle_messbar`
stets falsch (:666–676) und die Aktion stets `more_data` (:709–713). Zusätzlich
ist `capability_vorhanden` fest `false` (:122). `draft_offers = 0` in **jeder**
gefahrenen Bühne. Damit kann aus einem `hoch`-Befund heute kein Audition- oder
Draft-Angebot werden — die Defekte D1 bis D4 wirken in P5 auf die
**Befundanzeige**, nicht auf einen Eingriff.

---

## B8 — Exit-Gate Satz 3: „Gegenbeispiele erzwingen nachweislich Enthaltung"

### (a) Stehen die drei Gegenbeispiele wirklich unter Last?

| Korpusfall | Fenster (Master / Quellen) | Routing | Passage | Was tatsächlich greift | scharf? |
|---|---|---|---|---|---|
| `korrelierter_distraktor` | 12 / 12 + 12, Kanäle 3 und 4 | beide bekannt | keine | Riegel „nur der Führende darf stark sein" (`hypothese.rs`:1187–1192) **und** Gleichstandsriegel (:1212–1224); ohne sie **zwei** `hoch` | **ja** |
| `parent_duplikat` | 12 / 12 + 12, beide Kanal 7 | beide bekannt | keine | Parent-Term (:1391) **und** — weil materialgleich — der Gleichstandsriegel; der Fall isoliert den Parent-Riegel **nicht** | **nein, doppelt abgedeckt** |
| `verschobene_passage` | 12 / 12, Kanal 3, Versatz 6 | bekannt | **keine** | **Gate 3, Alignment** (:823–829): Überdeckung 3072/6144 = 0,500 < 0,95 → `alignment_falsch`. Der **Passagenriegel** (:832–855) läuft im gesamten P5-Korpuslauf **nie**, weil `aufnahme.passage == None` ist | ja, aber am falschen Riegel |
| `zu_kurze_passage` | 3 / 3 | bekannt | keine | allein die absolute Zahl in `baue_befund` (:1389), nicht Gate 4b | ja |

Belege: `eq-copilot/fixtures/p5-korpus/sitzungen.json` (Fensterzahlen und
`mixer` je Quelle), `broker/tests/sonde014_p5_korpus.rs`:142–206 (das Bein legt
**kein** `experiment_begin` an; grep „passage" in der Datei: 0 Treffer). Keine
Korpusquelle trägt `mixer: null`, obwohl `erzeuge_p5_korpus.py`:178 den Fall
„Routing unbekannt" vorsieht.

**Folge:** `GATE_MINDEST_FENSTER` wird in der Kalibrierung (`pruefe_p5_korpus.py`
:587–596, Ausgabe des Laufs: „der Startwert 8 haelt") über zwei Sitzungen
bestätigt, in denen die Konstante **an ihrem eigentlichen Ort — Gate 4b,
`hypothese.rs`:853 — nie ausgewertet wurde**. Das ist der Nährboden von D3.

### (b) Ist „Enthaltung" im Korpus dieselbe wie im Exit-Gate?

Ja, unter der Definition dieses Auftrags („Ursachenklasse `DatenReichenNicht`
**oder** Zustand `MORE DATA`"). `zustand_aus_sicherheit`
(`hypothese.rs`:661–672) bildet `mittel` **und** `unklar` auf `more_data` ab.

* echte `daten_reichen_nicht`-Enthaltung: **`daten_reichen_nicht`** (keine
  Quelle → `enthaltung()`:1156–1165) und **`verschobene_passage`** (einziger
  Kandidat fällt am Alignmentgate, Überlebendenliste leer → derselbe Pfad).
* nur `mittel` / `MORE DATA`: `korrelierter_distraktor`, `parent_duplikat`,
  `zu_kurze_passage`.
* einziger `hoch`-Fall: `wahrer_kandidat`.

Lauf des Prüfers: `enthaltung=0.250`, `coverage=0.750`, `falsche_starke=0`,
Schwelle `hoch` hält, `mittel` und `unklar` halten nicht. Grün.

### (c) Messen die Rust-Beine die Matrixzeilen, deren Testspalte nicht existente C++-Ziele nennt?

`EqCopSonde014HypotheseTest`, `EqCopSonde014ProposalTest` und
`EqCopSonde014MaskierungTest` existieren weder als Quelle noch als CMake-Ziel.
Je Zeile der tatsächlich messende Test:

| ID | messender Test |
|---|---|
| M-14 | `sonde014_hypothese.rs::ursachenklassen_sind_geschlossen_und_sieben`:263 |
| M-15 | `sonde014_hypothese.rs::causehypothesis_traegt_die_zehn_felder`:348 |
| M-16 | `sonde014_hypothese.rs::sechs_teile_im_datenmodell`:395 |
| M-17 | `sonde014_hypothese.rs::parallele_telemetrie_bleibt_klasse_eins`:428 — misst nur den Fall **ohne** Paar; die Kante „Paar unterhalb `stark`" ist ungemessen (D6) |
| M-18 | `sonde014_hypothese.rs::screening_reicht_hoechstens_fuenf_weiter`:459, `::jedes_band_liegt_im_intervall_seiner_gruppe`:1111 |
| M-19 | `sonde014_hypothese.rs::bedingter_uplift_braucht_fenster_ohne_die_quelle`:492 |
| M-20 | `sonde014_hypothese.rs::keine_komponente_kompensiert_coverage_oder_alignment`:542 — der Alignmentteil benutzt eine **disjunkte** Spanne (90 000 000 gegen 44 108 200); die umklammernde Spanne aus D4 ist ungemessen |
| M-21 | `pruefe_p5_korpus.py::_faelle_bilden`:467–486 + `::kennzahlen`:103–115, gefahren über `sonde014_p5_korpus.rs::korpus_kette_laeuft_durch_den_produktpfad`:216 — nur für den **materialgleichen** Distraktor (D2) |
| M-22 | `sonde014_hypothese.rs::parent_duplikat_erzeugt_keine_zwei_starken`:612 + Korpussitzung `parent_duplikat` — beide **ohne** Main auf dem geteilten Kanal (D1) |
| M-23 | `sonde014_hypothese.rs::passage_zu_kurz_traegt_keine_starke_aussage`:666; `sonde014_nacharbeit1.rs::passagengate_misst_auch_die_fenster_des_masters`:424; Kalibrierung `pruefe_p5_korpus.py`:587–593 — kein Fall mit Fenstern, die die Passage nur berühren (D3) |
| M-24 | `sonde014_hypothese.rs::ruecknahme_invalidiert_abhaengige_hypothesen`:755; `sonde014_verdrahtung.rs`:487 |
| M-25 | `sonde014_hypothese.rs::ranking_ist_bytegleich_ueber_hundert_laeufe`:805 |
| M-26 | `sonde014_hypothese.rs::gleichstand_zeigt_beide`:858, `::ungetrennter_erster_platz_ist_nicht_stark`:890 |
| M-27 | `sonde014_hypothese.rs::mehr_daten_ist_ein_ergebnis`:954; `sonde014_p5_korpus.rs`:290–296 |
| M-28 | `sonde014_hypothese.rs::behauptung_ohne_existente_evidenz_wird_nicht_sichtbar`:979; `sonde014_nacharbeit1.rs`:318, :372; `sonde014_nacharbeit2.rs`:597 |
| M-29 | `sonde014_befund.rs::sicherheit_wird_auf_drei_zustaende_abgebildet`:189; `Sonde014BefundTest.cpp`:147–176 |
| M-30 | `sonde014_befund.rs::nur_ready_to_send_erlaubt_audition_und_draft`:238; `Sonde014BefundTest.cpp`:177–201 |
| M-31 | `pruefe_p5_korpus.py::schwelle_suchen`:224 + `::produktschwelle`:264 — **nur** im Python-Bein |
| M-32 | `sonde014_befund.rs::alternative_ist_ein_eigener_befund_mit_eigenem_zustand`:319; `Sonde014BefundTest.cpp`:232–271 |
| M-33 | `sonde014_befund.rs::beleg_ist_zone_und_kein_text`:356 — **halb**: die zweite Kurvenidentität (Referenzkurve) ist ungemessen |
| M-34 | `sonde014_befund.rs::drei_zeilen_sind_datenfelder`:385; `Sonde014BefundTest.cpp`:318–356 |
| M-35 | `sonde014_befund.rs::messqualitaet_und_befundsicherheit_sind_zwei_felder`:421 |
| M-42 | `sonde014_proposal.rs::proposal_traegt_die_fuenfzehn_felder`:292 |
| M-43 | `sonde014_proposal.rs::sechs_gate_felder_sind_pflicht_und_revert_hat_drei_werte`:344; `sonde014_nacharbeit1.rs`:715 — beide mit handgebauter `Proposallage`, deshalb blind für D5 |
| M-44 | `sonde014_proposal.rs::derselbe_eingang_erzeugt_denselben_entwurf`:421 — **halb**: „der kleinste sichere Kandidat gewinnt" ist ungemessen, der in der Matrix genannte Fall `kleinster_sicherer_kandidat_gewinnt` existiert im Repo nicht |
| M-45 | `sonde014_proposal.rs::in_p5_ist_jede_aktion_manual`:449 |
| M-46 | `sonde014_proposal.rs::keine_aenderung_und_mehr_daten_sind_vorschlaege`:477 |
| M-47 | `sonde014_proposal.rs::zielbereich_kommt_aus_dem_befund_nicht_aus_dem_delta`:524 |
| M-48 | `sonde013_verdrahtung.rs::sonde014_m48_gelesenes_ziel_schlaegt_die_heuristik`:5658; `sonde014_nacharbeit1.rs`:475 |
| M-49 | `sonde014_proposal.rs::geschuetzte_eigenschaft_ist_harte_constraint`:615 |
| M-50 | `sonde014_proposal.rs::stop_if_auf_nicht_messbarem_guardrail_ist_nicht_handelbar`:687 |
| M-51 | `sonde014_proposal.rs::jede_zahl_hat_feld_evidenz_und_generatorversion`:717, `::text_entsteht_aus_bausteinen_und_erfindet_keine_zahl`:1113 |
| M-52 | `sonde014_proposal.rs::veraltet_ungueltig_capability_erreichen_keine_probe`:756 |
| M-53 | `sonde014_proposal.rs::hard_caps_und_engeres_userbudget_werden_nie_ueberschritten`:817 |
| M-54 | `sonde014_proposal.rs::evidenz_ids_und_intent_revision_sind_pflicht`:894 |
| M-64 | `pruefe_p5_korpus.py::kennzahlen`:84 + `::riegel`:196, Selbsttest `::selbsttest`:611 |
| M-65 | `sonde014_p5_korpus.rs::korpus_kette_laeuft_durch_den_produktpfad`:216 + `pruefe_p5_korpus.py::_faelle_bilden`:360 |
| M-66 | `pruefe_p5_korpus.py::schwelle_suchen`:224, Gegenprobe `::selbsttest`:746–785 |
| M-67 | `pruefe_p5_korpus.py::kennzahlen`:110–123 über die Sitzung `verschobene_passage` |
| M-68 | `pruefe_p5_korpus.py::selbsttest`:611 (A30) |
| M-69 | `pruefe_p5_korpus.py::kennzahlen`:110–123 / `::riegel`:196 — **halb**: „die Aussage ist auf jedem Gegenbeispiel `unsicher`" hat keinen eigenen Prüfabschnitt; unter der Enthaltungsdefinition dieses Auftrags ist die Zusage dennoch erfüllt |
| M-86 | `sonde014_verdrahtung.rs::keine_rechnung_vor_der_vollstaendigkeitsmarke`:297, `::keine_rechnung_ohne_jede_intentmeldung`:351, `::die_vollstaendigkeitsmarke_deckt_beide_bestaende`:1015; `sonde014_hypothese.rs`:1161 |
| M-87 | `sonde014_hypothese.rs::ausschlussgruende_sind_geschlossen_und_reisen_mit`:1012; `Sonde014BefundTest.cpp`:357–388 — **halb**: „die Ausschlüsse überleben den Neustart am Finding" hat keinen Test |

Keine Zeile ist **ganz** ungemessen. Vier Zeilen sind halb gemessen (M-33, M-44,
M-69, M-87), und drei sind blind für die Defekte oben (M-17, M-20, M-43).

### (d) Adversariale Fälle außerhalb des Korpus

| Fall | im Korpus / in `broker/tests`? | dieser Lauf |
|---|---|---|
| Distraktor mit **mehr** Fenstern als der Verursacher | **nein** (Korpus 12/12; kein Rustfall mit ungleichen Fensterzahlen zweier Sonden) | **gefahren, bricht M-21 → D2** |
| Distraktor mit identischem Verlauf | ja (`sitzungen.json` `korrelierter_distraktor`; `sonde014_hypothese.rs`:857, :890) | hält |
| Parent außerhalb des Kandidatendeckels | **nein** (`screening_reicht_hoechstens_fuenf_weiter`:459 hat sieben Kandidaten auf **einem** Kanal, aber keinen Deckelüberlauf mit Elternbezug) | kein Pfad — Duplikatbit steht vor `truncate` |
| Passage mit genau einem Fenster außerhalb | **nein** (im P5-Korpus existiert gar keine Passage) | **gefahren, bricht M-23 → D3** |
| Zwei `main` in einer Sitzung | **nein** (jede Bühne meldet genau einen `main`) | **gefahren, bricht das Exit-Gate → D5** |
| Kandidat ohne Fensterüberlappung mit dem Master, Spanne umklammernd | **nein** (M-20 (b) benutzt eine disjunkte Spanne) | **gefahren → D4** |
| Sonde auf dem Mixerkanal des Mains | **nein** | **gefahren, bricht die Duplikatzusage → D1** |

---

# Befunde

## DEFEKT

### D1 — Parent-Duplikat-Erkennung fällt aus, sobald der Main den Mixerkanal teilt

* **Datei:Zeile** — `broker/src/coordinator/hypothese_verdrahtung.rs`:178–188
  (der `main` landet in `je_kanal`), :226–235 (`parent` = erste andere Kennung
  desselben Kanals) gegen `broker/src/coordinator/hypothese.rs`:1113–1117
  (`ids` enthält nur `aufnahme.kandidaten`, nie den Main) und :1134–1138.
* **Reproduktion (gefahren)** — `g5probe::probe::b_sonde_teilt_kanal_mit_main`
  und `g5probe::probe2::b2_zwei_sonden_und_main_auf_kanal_null`:
  `routingqualitaet = 1,000`, führender Befund `confidence.class = hoch`,
  `zustand = ready_to_send`. Gegenprobe `g5probe::probe2::b3` (dieselben zwei
  Sonden auf Kanal 5, Main auf 0): `routingqualitaet = 0,500`, **beide
  `mittel` / `more_data`**.
* **Was bricht** — Gate-Text `docs/bauaufteilung-sonden.md`:397–400 wörtlich:
  „Der Prüfer soll eine falsche starke Ursachenbehauptung provozieren —
  korrelierter Distraktor, **Parent-Duplikat**, verschobene Passage. Enthaltung
  … ist das gewünschte Ergebnis." Hier ist das Ergebnis READY TO SEND. Zusätzlich
  die eigene Zusage des Moduls (`hypothese_verdrahtung.rs`:45–48): „zwei Quellen
  auf **demselben** `host_mixer_index` messen dasselbe Signal; beide werden als
  Duplikat geführt und **keine erreicht `hoch`**."
  *Ehrlich abgegrenzt:* M-22s Wortlaut („**nicht zwei** starke Behauptungen")
  bricht nicht — der Riegel „nur der Führende darf stark sein" verhindert die
  zweite. Gebrochen ist die Duplikatzusage selbst.
* **Schärfste Ausprägung** — die Sonde, die **denselben Mixerkanal wie Gen**
  misst (Probeeq neben Gen auf dem Master), wird als starke Ursache der
  Masteranomalie ausgewiesen.

### D2 — Ein korrelierter Distraktor mit mehr Material erhält `hoch`

* **Datei:Zeile** — `broker/src/coordinator/hypothese.rs`:1212–1224 (der
  Gleichstandsriegel greift nur bei **quantengleichem** Rang) zusammen mit
  :1389–1398 (die Klasse liest weder `koinzidenz` noch `uplift` noch
  `wiederholbarkeit`).
* **Reproduktion (gefahren)** — `g5probe::probe2::h_distraktor_mit_mehr_fenstern`:
  wahre Quelle 12 Fenster / +9 dB, Distraktor 24 Fenster / +12 dB im selben
  Band. Ergebnis: Distraktor `hoch` / `ready_to_send`
  (`bandpassung = 0,173`, `koinzidenz = uplift = wiederholbarkeit = 0,000`),
  wahre Quelle `mittel` / `more_data` als seine Alternative.
* **Was bricht** — M-21 wörtlich (`docs/beweise/SONDE-014.md`:740): „Der
  Distraktor erzeugt **keine starke** Ursachenbehauptung. Er darf als
  **Alternative** erscheinen." Hier ist es umgekehrt. Die projekteigene
  Bewertungsregel `tools/eq-copilot/pruefe_p5_korpus.py::_passt`:187–193
  (`sicherheit == HOCH and not ist_wahre_ursache → False`) würde den Fall rot
  melden; der Korpus enthält ihn nicht.

### D3 — Fenster, die die Passage nur berühren, zählen als Passagenmaterial

* **Datei:Zeile** — `broker/src/coordinator/hypothese.rs`:1612–1623
  (`fenster_in_passage`: jede Überlappung ≥ 1 Sample zählt) im Zusammenspiel mit
  :832–845 (Gate 4a rechnet über die **Spanne**) und :853.
* **Reproduktion (gefahren)** —
  `g5probe::probe2::e2_alle_fenster_beruehren_die_passage`: 4 Fenster weit vor
  der Passage plus 10 Fenster mit je **einem** Sample Überlappung ergeben
  `confidence.class = hoch`, `zustand = ready_to_send`, `ausschluesse = []`.
  Gegenprobe `g5probe::probe::e` (nur ein berührendes Fenster): korrekt
  `passage_zu_kurz` und Enthaltung.
* **Was bricht** — M-23 wörtlich (`docs/beweise/SONDE-014.md`:742): „‚Zu kurz'
  wird deshalb als **gemessene Mindestzahl unabhängiger Evidenzfenster je
  beteiligter Quelle innerhalb der Passage** definiert" und „Unter der Schwelle
  erreicht kein Befund die Sicherheit `hoch` und den Zustand `READY TO SEND`".
  Ein Fenster, das zu 99,98 % außerhalb liegt, ist nicht „innerhalb der Passage".

### D4 — Das Alignment-Gate misst Spannen, nicht Überlappung

* **Datei:Zeile** — `broker/src/coordinator/hypothese.rs`:823–829 zusammen mit
  :1563–1574 (`spanne` ist ein lückenblindes min/max) und :1581–1592
  (`zeitueberdeckung` normiert auf das kürzere Intervall).
* **Reproduktion (gefahren)** —
  `g5probe::probe::d_umklammernde_spanne_ohne_ueberlappung`: Sondenfenster
  20 000 Samples vor und 40 000 Samples nach den Masterfenstern, **null**
  paarweise Überlappung. Alignment = 1,0, Gate hält;
  `confidence.class = hoch`, `zustand = ready_to_send`,
  `koinzidenz = uplift = wiederholbarkeit = 0,000`.
* **Was bricht** — Gate-Text (falsche starke Ursachenbehauptung statt
  Enthaltung) und die Zusage des Gates an sich selbst (:823–825: „liegen die
  Fenster des Kandidaten **wirklich dort**, wo der Master seinen Befund hat?").
  M-20 nennt fehlendes Alignment eine **harte Gate-Regel vor der Gewichtung**;
  hier feuert das Gate nicht, obwohl gar kein gemeinsames Zeitfenster existiert.
  *Ehrlich abgegrenzt:* M-20s Rotbeweis („eine Komponente kompensiert ein
  gerissenes Gate") trifft die Lage nicht wörtlich — das Gate reißt nicht, es
  misst die falsche Größe. Ohne Passage — dem Normalfall der passiven
  P5-Sitzung — gibt es danach keinen zweiten Riegel.

### D5 — Ein Proposal ohne `target` entsteht und wird persistiert

* **Datei:Zeile** — `broker/src/coordinator/proposal_verdrahtung.rs`:113–119
  (`unwrap_or_default()` auf `fuehrendes_main`) → `proposal.rs`:719
  (`target: lage.ziel_instanz.clone()`) → `proposal_verdrahtung.rs`:153–180
  (Append als `event_type = "proposal"`), gegen
  `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `$defs/proposal.required`
  enthält `target` mit `$ref hex32` (`minLength: 32`).
* **Reproduktion (gefahren)** — `g5probe::probe::c_zwei_mains`: nachdem eine
  zweite `main`-Instanz derselben Sitzung beitritt, setzt
  `mitgliedschaft.rs`:87–92 `fuehrendes_main = None`
  (`FUEHRENDE_MAINS_PRO_SESSION = 1`, `mod.rs`:118), `aufnahmen_sammeln`
  (`hypothese_verdrahtung.rs`:215–216) nimmt weiter einen Master. Gemessen:
  `gate_felder = [("target", false), …]` und der persistierte Payload
  `…"target":""…`.
* **Was bricht** — Exit-Gate §59 wörtlich: „**jedes Proposal nennt Ziel**,
  Passage, Grenzen, Hörziel, Stopbedingung und Rückweg", und die Invariante
  „Schemas sind Verträge" (`CLAUDE.md`). `Proposal::gate_felder_vollstaendig`
  (`proposal.rs`:360–374) hat **keinen** Produktivaufrufer; alle vier
  Testaufrufer bauen `ziel_instanz: hex(1)` von Hand.

### D6 — `claim_class = wirkungsbeleg` aus einem PRE/POST-Paar ohne reproduzierbare Veränderung

**(am Quelltext hergeleitet, nicht gefahren — der `Probable`-Zweig verlangt
neuen Fixturecode; der Produktpfad mit der committeten Evidenzgrundform bleibt
`Unclear`, gemessen in `g5probe::probe3::k`.)**

* **Datei:Zeile** — `broker/src/coordinator/hypothese_verdrahtung.rs`:193–203
  (`ergebnis.is_some()` als einzige Bedingung) gegen
  `broker/src/coordinator/prepost.rs`:1110–1130 (`ergebnis: Some(…)` auch für
  `Alignmentklasse::Probable`) und :811–852 (`ausgerichtet_db = None`,
  `match_gain_db = None`, `wirkung = None`, sobald die Klasse unter
  `AudioAligned` liegt), Wirkung in `hypothese.rs`:1353–1364, :1432, :1403–1411.
* **Reproduktion** — zwei Sonden mit gleicher `pair_id` (Messpunkt `pre`/`post`),
  deren Paar aus einem der sechs Herabstufungsgründe (`prepost.rs`:1044–1074,
  :1101–1105) nur `Probable` erreicht: der Befund trägt danach
  `claim_class = "wirkungsbeleg"`, `ursachenklasse = "effektkette_pre_post"`,
  `pre_post = "post"`, und `next_test` verlangt kein PRE/POST-Paar mehr.
* **Was bricht** — M-17 wörtlich (`docs/beweise/SONDE-014.md`:736):
  „2. Wirkungsbeleg (PRE/POST-Paar **mit reproduzierbarer Veränderung**)".
  Ein `Probable`-Paar trägt weder ein ausgerichtetes Delta noch eine Wirkung.
  Zugleich Gate 6 im P5-Kontext: SONDE-013 hält an genau dieser Stelle fest
  („Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder
  PRE/POST-Aussage", `prepost.rs`:896–898), und die P5-Kette hebt die Sperre auf.

## LÜCKE

* **L1** — Die Kette trägt **keinen Feldnamen**, der „gemessen an der Sonde" von
  „Beitrag zur Summe" unterscheidet. `beobachtung` eines `session_finding` ist
  die Anomalie des **Masters** (`hypothese.rs`:1284–1288, gefüllt aus
  `masteranomalie(&aufnahme.master)`), steht im Objekt aber neben
  `candidate_source` = der Sonde, und der Vertragskommentar ordnet
  „Beobachtung" den Feldern `target_metric`/`band_hz`/`beobachtung` zu, ohne den
  Messpunkt zu benennen. Heute folgenlos, weil Gen das Feld nicht zeichnet.
* **L2** — `juengste_passage_im_projekt` (`experiment.rs`:919–923) wählt die
  Passage des **jüngsten** Versuchs, unabhängig von dessen Vergleichbarkeit,
  Taint oder Abschlusszustand. Sie kann eine andere sein als die, die Gen
  anzeigt.
* **L3** — Die Sicherheitsklasse liest **keine** Rangkomponente
  (`hypothese.rs`:1389–1398). Ein Befund mit `koinzidenz = uplift =
  wiederholbarkeit = 0,000` erreicht `hoch`. Das ist die ausdrückliche Lesart
  von M-15 („die Klasse wird nicht aus dem Score gerundet"), und die numerische
  Grenze ist Ausgabe der Kalibrierung — deshalb Lücke, nicht Defekt. Sie ist
  aber der Verstärker unter D2 und D4.
* **L4** — Ein Parent, dessen Belege zurückgenommen wurden (:213) oder dessen
  Routing unbekannt ist, steht nicht in `je_kanal` beziehungsweise nicht mehr in
  den Kandidaten; sein Child bleibt dann unmarkiert und kann `hoch` erreichen.
  Matrix und Entwurf sagen zu diesem Fall nichts.
* **L5** — `user_verdict` wird persistiert, ohne dass die genannte `finding_id`
  existieren muss (`befehl.rs`:545–568).
* **L6** — Für `no_change` und `more_data` gilt `allowed_bounds` als „genannt",
  obwohl das Objekt `{}` ist (`proposal.rs`:366–369, gemessen). Das ist die
  ausdrückliche Lesart von M-46.
* **L7** — Vier Matrixzeilen sind nur halb gemessen: M-33 (zweite
  Kurvenidentität), M-44 („der kleinste sichere Kandidat gewinnt"; der in der
  Matrix genannte Fall `kleinster_sicherer_kandidat_gewinnt` existiert im Repo
  nicht), M-69 („die Aussage ist auf jedem Gegenbeispiel unsicher"), M-87
  („Ausschlüsse überleben den Neustart am Finding").

## HÄRTUNG

* **H1** — Gen prüft `alternatives` nur auf hex32, nie gegen die Befunde
  desselben Snapshots (`SourcesModel.cpp`:927–944); dasselbe für
  `ausschluesse[].candidate_source` (:960–962) und `candidateSource` selbst.
  Der Broker stellt die Referenzintegrität heute her, ein zweiter Riegel wäre
  billig.
* **H2** — Nach einem Intent-Revisionssprung geht der Befund auf `stale`, das
  mitlaufende Proposal behält aber `confidence.class = hoch`
  (gemessen `g5probe::probe2::i`). In P5 folgenlos (`action = more_data`,
  `darf_draft_offer` sperrt über `zustand`), aber inkonsistent.
* **H3** — Der P5-Korpus enthält keine Quelle mit `mixer: null`, obwohl
  `erzeuge_p5_korpus.py`:178 den Fall vorsieht; „Routing unbekannt" ist damit
  nur im Rustbein und nicht in der Kalibrierung gemessen.
* **H4** — Der Kandidatendeckel schneidet stumm: die überbotenen Kandidaten
  erscheinen weder in `alternatives` noch in `ausschluesse`, und `rang` wird von
  Gen nicht gespeichert (`SourcesModel.cpp`:882–905). Das ist so entschieden
  („sie sind nicht ausgeschlossen, sondern überboten"), lässt aber keinen Weg,
  die Rangfolge nachzuvollziehen.

---

# Geprüft

Vollständig gelesen und gegen den laufenden Code gehalten: der gesamte
Prüfbereich unter `broker/src/coordinator/` mit dem Schwerpunkt `hypothese.rs`,
`hypothese_verdrahtung.rs`, `proposal.rs`, `proposal_verdrahtung.rs`,
`prepost.rs`, `maskierung.rs`, `invalidierung_verdrahtung.rs`,
`mitgliedschaft.rs`, `liveness.rs`, `sicht.rs`, `evidenz.rs`, `befehl.rs`;
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json` (`$defs/proposal`,
`$defs/session_finding`, `$defs/session_snapshot`, `$defs/maskierung`,
`$defs/hex32`); die Empfangsseite in `SourcesModel.cpp/.h`,
`PluginProcessor.cpp`, `PluginEditor.cpp`, `NakamaState.cpp`, `ControlClient`,
`WireEnvelope.h`; `eq-copilot/fixtures/p5-korpus/sitzungen.json` und
`MANIFEST.json`; `tools/eq-copilot/erzeuge_p5_korpus.py` und
`pruefe_p5_korpus.py`; `broker/tests/sonde014_*.rs` und
`eq-copilot/plugin/tests/Sonde014*Test.cpp` als Belegquelle.
**Gefahren** wurden alle 20 Beine der Tabelle oben — zehn Rust-Beine, vier
Plugin-Exen, drei Pythonläufe und drei eigene Messbühnen. Die Defekte D1 bis D5
sind gefahren und mit Gegenprobe abgesichert.

# Nicht geprüft

Nicht gefahren wurde der `Probable`-Zweig des PRE/POST-Urteils (D6): er
verlangt Hüllkurven mit Korrelationsspitze und damit neuen Fixturecode; der
Auftrag verbietet, Testquellen zu ändern, deshalb steht D6 als am Quelltext
hergeleiteter Weg. Nicht geprüft wurden ferner: der installierte Stand, FL,
Broker auf der Produktions-Pipe (ausdrücklich untersagt); die Ausführung von
`draft_offer`, Preview, Remote-Apply und Entmaskierung (P6-Lieferumfang);
UI-Gestaltung (S31b); Same-user-Szenarien nach Entwurf §48.4; die
Registerzeilen NAK-135 bis NAK-211; `docs/**` als Prüfgegenstand (Manifeste,
Planstand, Testspalte der Matrix als Text); die C++-Seite jenseits der im
Prüfbereich genannten Dateien; Nebenläufigkeit unter echter Last (die
Interleaving-Zusage ist über den Testhaken `rechen_test_haken_ausloesen`
deterministisch gemessen, nicht mit zwei Threads).

**Repo-Zustand nach dem Lauf:** `git status --short` meldet ausschließlich
`?? docs/beweise/roh/G5-codex-falsifikation-auftrag.txt` — die bereits vorhandene
untracked Auftragsdatei. Kein Repo-File wurde geschrieben oder geändert; die
Messbühne liegt vollständig im Scratchpad, geschrieben wurde nur nach
`broker/target/`, `eq-copilot/build/` (Korpusergebnis des Kettenbeins) und in
den Scratchpad.
