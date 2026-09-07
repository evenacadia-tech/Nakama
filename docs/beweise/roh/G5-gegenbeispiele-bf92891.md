URTEIL: NEEDS_WORK

# G5 — adversariale Gegenbeispiele, gefahren durch den Produktpfad

| | |
|---|---|
| Datum | 2026-09-07 |
| Auftrag | Phasengate G5, Phase P5, Prüfstufe T3 — „eine falsche starke Ursachenbehauptung provozieren" (`docs/bauaufteilung-sonden.md`:397–400) |
| Arbeitskopie | `C:\Users\phili\nk-g5` (lokaler Clone), HEAD `bf92891b17b0b6f216dcf1b10a50ac2d9b756dc4` |
| Sichtbarer Checkout | `C:\Users\phili\Projekte\Nakama` — **nicht angefasst**, `git status --short` leer |
| cargo | 1.93.1 (083ac5135 2025-12-15), rustc-Profil `test` (unoptimized + debuginfo) |
| Produktcode | **unverändert**. Der Patch berührt ausschliesslich `tools/eq-copilot/erzeuge_p5_korpus.py`, die daraus erzeugten Korpusdateien und die neue Testdatei `broker/tests/g5_gegenbeispiele.rs`. |

## Gefahrene Befehle und Ergebniszahlen

| Befehl | Ergebnis |
|---|---|
| `py -3.13 tools/eq-copilot/erzeuge_p5_korpus.py` | `2 Dateien, 18 Sitzungen` (6 Bestand + **12 neue Gegenbeispiele**) |
| `py -3.13 tools/eq-copilot/erzeuge_p5_korpus.py --pruefen` | `bytegleich`, Exit 0 |
| `cargo test --test sonde014_p5_korpus -- --nocapture` | `1 passed; 0 failed`, 8,13 s; schreibt `eq-copilot/build/p5-korpus-ergebnis.json` (18 Sitzungen, 28 Befunde) |
| `cargo test --test g5_gegenbeispiele -- --nocapture --test-threads=1` | `11 passed; 0 failed`, 51,69 s (15 adversariale Eingaben) |
| `py -3.13 tools/eq-copilot/pruefe_p5_korpus.py` | **Exit 2**, Riegel `falsche_starke` schlägt an: **6 falsche starke Behauptungen** im Korpusteil |

**Bilanz über alle 27 gefahrenen Fälle:** 10 × Enthaltung oder `mittel` erzwungen · 6 × zulässige starke Aussage (Kontrollfälle, ohne die jeder Riegel trivial erfüllt wäre) · **11 × falsche starke Behauptung** (`hoch` / `READY TO SEND`).

## Vorbemerkung zur Beweiskraft des heutigen Korpus

Bevor der erste Gegenbeispielfall steht, ein Befund über die Messanlage selbst,
weil er alle Passagenzusagen betrifft:

**Der Passagenriegel steht im ganzen P5-Korpus nie unter Last.**
`sonde014_p5_korpus.rs::sitzung_fahren` legt keine Passage an;
`aufnahmen_sammeln` (`hypothese_verdrahtung.rs`:237–238) liest sie aus
`stand.experimente.juengste_passage_im_projekt`, und ohne `experiment_begin`
ist das `None`. Damit wird `gate()` Schritt 4 (`hypothese.rs`:838–854,
`PassageUnvergleichbar` / `PassageZuKurz`) im Korpus **nie ausgeführt** — auch
nicht in der Sitzung `verschobene_passage`. Sie fällt nachweislich am
**Master-Alignment** (`gate()`:828, Ausgabe `alignment_falsch`), nicht an der
Passage. Ebenso zählt `baue_befund` ohne Passage nur die Fenster des
**Kandidaten** (`hypothese.rs`:1373–1376), nicht das Passagenmaterial beider
Seiten. Die Zusagen M-23/M-67 zur verschobenen und zur zu kurzen **Passage**
sind vom Korpus also **ungemessen**; gemessen ist ein anderer Riegel mit
ähnlichem Ergebnis. Fall 5 unten schliesst diese Lücke mit einer echten
Passage über `experiment_begin`.

---

# Die Fälle

## Fall 1 — Korrelierter Distraktor mit MEHR Fenstern, identischer Spektralverlauf (M-21, M-19)

**Eingabe** (`sitzungen.json`, Sitzung `g5_distraktor_mehr_fenster`): Master 12
Fenster @ +9,0 dB in Band 98..102; wahre Ursache (Instanz 02) 12 Fenster
@ +9,0 dB, Mixer 3; Distraktor (Instanz 03) **16 Fenster** @ +9,0 dB, Mixer 4.
Alle Fenster ab Projektsample 44 108 200, Schrittweite 512, kein Versatz.

**Erwartet laut Matrix:** M-21 — der Distraktor erzeugt keine starke
Ursachenbehauptung, darf aber Alternative sein. Höchstens `mittel`.

**Tatsächliche Ausgabe:**

```
src=02 klasse=mittel zust=more_data ursache=zwei_quellen_konkurrenz score=0.266635 alt=[..69ef4d]
src=03 klasse=mittel zust=more_data ursache=zwei_quellen_konkurrenz score=0.266635 alt=[]
```

**Urteil: Enthaltung/mittel erzwungen.** Entscheidender Riegel:
`hypothese.rs`:1210–1228 (`getrennt`) — beide Ränge quantisieren auf denselben
Integer, der führende Befund fällt auf `mittel`.

**Warum der Fall trotzdem etwas misst:** die Fensterzahl geht in **keine**
Rangkomponente ein. `bandpassung` ist ein Leistungsverhältnis über alle
Fenster (invariant gegen ihre Zahl), `koinzidenz` ist 0 (die Onsetreihe der
Fixture ist konstant, `korrelation` gibt bei fehlender Streuung 0 zurück),
`uplift` ist 0 (M-19: es gibt kein Fenster **ohne** die Quelle, deshalb ist
`ohne` leer und `uplift()` gibt 0 statt „kein Wert"), `wiederholbarkeit` ist
aus demselben Grund 0. Mehr Material erzeugt hier also keine Trennung — das
ist die gewünschte Zurückhaltung, aber sie entsteht aus einem **Gleichstand**,
nicht aus einer Kausalprüfung. Genau daran setzt Fall 7 an.

---

## Fall 2 — Distraktor allein im Rennen (M-18, M-21)

### 2a — Der wahre Verursacher fällt am Alignment

**Eingabe** (`g5_distraktor_allein_im_rennen`): Master 12 Fenster @ +9,0 dB;
wahre Ursache (02) 12 Fenster @ +9,0 dB, Mixer 3, **`versatz_fenster = 6`**;
Distraktor (03) 12 Fenster @ +9,0 dB, Mixer 4, kein Versatz.

**Erwartet laut Matrix:** M-21 / M-69 — Enthaltung. Ein Kandidat ist mit
benanntem Grund ausgeschieden; die Datenlage ist damit **nachweislich
unvollständig**.

**Tatsächliche Ausgabe:**

```
src=03 klasse=hoch zust=ready_to_send ursache=quelle_resonanz score=0.266635
       ausschluesse=[(02, alignment_falsch)]
```

**Urteil: FALSCHE STARKE BEHAUPTUNG.** Reproduktion:
`py -3.13 tools/eq-copilot/erzeuge_p5_korpus.py` ·
`cargo test --test sonde014_p5_korpus` · Sitzung `g5_distraktor_allein_im_rennen`
in `eq-copilot/build/p5-korpus-ergebnis.json`.

Entscheidender Riegel: **keiner**. `getrennt` (`hypothese.rs`:1210–1223) hat
für einen einzigen Überlebenden den Zweig `_ => true` — „Ein einziger Kandidat
hat niemanden, von dem er sich abheben müsste." Der Distraktor ist damit
trivial „getrennt". Der Ausschluss reist als `ausschluesse` **auf demselben
Befund** mit und senkt die Sicherheit nicht.

### 2b — Der wahre Verursacher fällt aus dem Kandidatendeckel

**Eingabe** (`g5_kandidatendeckel`): Master 12 Fenster @ +9,0 dB; sechs
Kandidaten mit je 12 Fenstern und eigenem Mixerkanal — wahre Ursache (02)
@ +8,0 dB, Distraktoren (03..07) @ +9,0 / 9,1 / 9,2 / 9,3 / 9,4 dB.

**Erwartet laut Matrix:** M-18 (Deckel fünf, hart), M-87 (jeder ausscheidende
Kandidat trägt einen Grund aus der geschlossenen Menge), M-21.

**Tatsächliche Ausgabe:**

```
src=07 klasse=hoch   zust=ready_to_send score=0.267954 alt=[4 IDs] ausschluesse=[]
src=06 klasse=mittel zust=more_data     score=0.267616
src=05 klasse=mittel zust=more_data     score=0.267283
src=04 klasse=mittel zust=more_data     score=0.266956
src=03 klasse=mittel zust=more_data     score=0.266635
```

Instanz **02 — die wahre Ursache — kommt in der Ausgabe überhaupt nicht vor:**
nicht als Befund, nicht als Alternative, nicht als Ausschluss.

**Urteil: FALSCHE STARKE BEHAUPTUNG, dazu ein zweiter Bruch.** Entscheidender
Riegel: `hypothese.rs`:1154 `ueberlebende.truncate(KANDIDATEN_DECKEL)` — die
Kürzung passiert **nach** der Sortierung und **ohne** Eintrag in
`ausschluesse`. Die geschlossene Menge aus M-87 (`hypothese.rs`:253–274) hat
für „am Deckel abgeschnitten" keinen Grund; der Fall kann also gar nicht
korrekt gemeldet werden. Gen sieht fünf Kandidaten und keinen Hinweis, dass es
einen sechsten gab.

---

## Fall 3 — Parent-Duplikat, Parent nicht im Rennen (M-22)

Drei Varianten, alle mit zwei Sonden auf **Mixerkanal 7**.

| Variante | Partner (Instanz 03) | Ausgabe Instanz 02 | Urteil |
|---|---|---|---|
| `g5_parent_partner_faellt_aus` | 12 Fenster, `versatz_fenster = 6` → fällt am Alignment | `mittel` / `more_data`, `routingqualitaet = 0,5`, Ausschluss `(03, alignment_falsch)` | Enthaltung/mittel erzwungen |
| `g5_parent_partner_zu_wenig_fenster` | 2 Fenster | beide `mittel` / `more_data`, `routingqualitaet = 0,5` | Enthaltung/mittel erzwungen |
| `g5_parent_partner_ohne_evidenz` | 0 Fenster (angemeldet, Deskriptor gesetzt, nie gesendet) | **`hoch` / `ready_to_send`**, `routingqualitaet = 1,0` | **FALSCHE STARKE BEHAUPTUNG** |

**Warum die ersten beiden halten:** die Duplikatmenge `ids` und das Prädikat
`ist_parent` (`hypothese.rs`:1108–1121) lesen `aufnahme.kandidaten` —
**alle** Kandidaten, nicht nur die Überlebenden und nicht nur die fünf im
Deckel. Ein am Gate gefallener oder abgeschnittener Partner setzt die
Duplikatmarke also weiterhin. Das ist der Riegel, und er hält.

**Warum die dritte fällt:** die Kanaltafel `je_kanal` entsteht in
`hypothese_verdrahtung.rs`:164–187 aus den Schlüsseln von `stand.evidenz`.
Eine Sonde ohne einen einzigen Beleg hat dort keinen Schlüssel und steht
deshalb in keiner Kanalgruppe. Sie kann keine falsche Aussage erzeugen, aber
sie verhindert auch nicht die des Partners.
*Einordnung: HÄRTUNG* — eine stumme Sonde ist kein zweites Messgerät auf dem
Kanal, und keine Matrixzeile sagt zu ihr etwas. Der Korpuseintrag steht
trotzdem, weil der Fall die **Herkunft** der Duplikaterkennung offenlegt.

### 3b — Das Duplikat, das die Regel nicht kennt: Sonde auf dem Masterkanal

**Eingabe** (`g5_sonde_auf_masterkanal`): eine einzige Sonde (02), 12 Fenster
@ +9,0 dB, **`mixer = 0`** — derselbe Mixerkanal, den der Master (`main`,
Instanz 01) in seinem Deskriptor führt.

**Tatsächliche Ausgabe:** `klasse=hoch zust=ready_to_send
ursache=quelle_resonanz score=0.266635`, `routingqualitaet = 1,0`.

**Urteil: FALSCHE STARKE BEHAUPTUNG.** Entscheidender Riegel: **keiner**.
`hypothese_verdrahtung.rs`:226–233 setzt `kandidat.parent` zwar auf die
Master-Instanz (die Kanaltafel enthält den `main`), aber
`hypothese.rs`:1108–1112 baut `ids` **nur aus den Kandidaten** — der Master
steht nicht darin, `ids.contains(parent)` ist falsch, `ist_parent` findet
nichts, `duplikat = false`. Eine Sonde, die buchstäblich das Signal des
Masters misst, wird als dessen Ursache mit `READY TO SEND` ausgewiesen.

---

## Fall 4 — Kanal wechselt zwischen den Fenstern, zwei Ebenen

### 4a — Kanalwechsel ohne neuen Messwert (`f4_kanalwechsel_zwischen_den_fenstern`)

**Eingabe:** Sonde A (02) @ +9,1 dB und Sonde C (03) @ +9,0 dB, beide 12
Fenster, beide `host_mixer_index = 7`. Danach `descriptor_setzen` für C mit
`host_mixer_index = 9` — **kein einziger neuer Messwert** —, dann je ein
weiterer Beleg als Rechenauslöser.

**Ausgabe vorher:** beide `mittel` / `more_data`, `routingqualitaet = 0,5`.
**Ausgabe nachher:** A `hoch` / `ready_to_send` (`routingqualitaet = 1,0`),
C `mittel`. Die `bandpassung` beider ist unverändert (0,101738 / 0,099809).

**Urteil: FALSCHE STARKE BEHAUPTUNG** — genauer: eine starke Aussage, die
allein aus einer **Deskriptoränderung** entsteht. Entscheidender Riegel:
`hypothese_verdrahtung.rs`:176–187 liest den Mixerkanal zum **Zeitpunkt der
Rechnung** aus dem Deskriptor; die Fenster tragen keinen Kanal. Die
Duplikaterkennung hat damit kein Gedächtnis.
*Einordnung: LÜCKE* — keine Matrixzeile sagt, wie ein Kanalwechsel während
einer Sitzung zu behandeln ist. Der Effekt (Mess-Sicherheit steigt ohne
Messung) ist trotzdem produktrelevant.

### 4b — Drei Sonden auf einem Kanal (`f4b_drei_sonden_auf_einem_kanal`)

Alle drei tragen `routingqualitaet = 0,5` und `mittel` / `more_data`. Der
flache Duplikatgraph hält transitiv. **Urteil: mittel erzwungen.**

**Großeltern (zwei Ebenen) sind nicht darstellbar.** Der Deskriptor führt genau
ein `host_mixer_index`; die „Elternbeziehung" des Produkts ist die Menge der
Quellen mit gleichem Kanal (`hypothese_verdrahtung.rs`:226–233), keine
Hierarchie. Eine Kette Kind→Eltern→Großeltern hat im heutigen Datenmodell
keine Eingabe. *Einordnung: LÜCKE.*

---

## Fall 5 — Verschobene Passage, Randwerte (M-23, M-67)

Alle Unterfälle mit **echter** Passage über `experiment_begin` (P0, Store,
`state_report` vorab), Master und Sonde je 12 Fenster @ +9,0 dB ab Fenster 0.
Datei `broker/tests/g5_gegenbeispiele.rs`, Fälle `f5_passagenrandwerte` und
`f5b_gleiche_passage_id_verschobenes_fenster`.

| Passage (Fensterindizes) | Konstruktion | Ausgabe | Urteil |
|---|---|---|---|
| `[0,12)` | deckt alles | `hoch` / `ready_to_send` | zulässig (Kontrollfall) |
| `[0,11)` | **genau ein Fenster ausserhalb** | `hoch` / `ready_to_send` | zulässig — Passagenmaterial 11 ≥ 8 |
| `[0,8)` | **Randwert `GATE_MINDEST_FENSTER`** | `hoch` / `ready_to_send` | zulässig — M-23 sagt „mindestens acht" |
| `[0,7)` | ein Fenster unter dem Randwert | `daten_reichen_nicht` / `more_data`, Ausschluss `(02, passage_zu_kurz)` | **Enthaltung erzwungen** |
| `[12,24)` | Belege enden **exakt** am Passagenanfang | `daten_reichen_nicht` / `more_data`, Ausschluss `(02, passage_unvergleichbar)` | **Enthaltung erzwungen** |

Der Randwert ist damit gemessen: `passagenmaterial(...) < GATE_MINDEST_FENSTER`
(`hypothese.rs`:853–854) ist ein `<`, nicht `<=` — acht Fenster reichen, sieben
nicht. Die Fensterzugehörigkeit ist halboffen
(`fenster_in_passage`, `hypothese.rs`:1612–1621:
`f.projekt_bis > passage.projekt_von && f.projekt_von < passage.projekt_bis`);
ein Fenster, das exakt am Passagenanfang endet, zählt nicht — der Fall
`[12,24)` fällt schon vorher an der Zeitüberdeckung (0,0 < 0,95).

**5b — gleiche `passage_id`, verschobenes Zeitfenster.** Zwei
`experiment_begin` mit derselben `passage_id` `…5032`, erst `[0,12)`, dann
`[12,24)`; die Belege liegen im **zweiten** Fenster.
`begin_anwenden_locked` (`experiment_verdrahtung.rs`:445–476) legt eine
bekannte Passage nicht neu an — der Broker misst weiter gegen `[0,12)`.
Ausgabe: `daten_reichen_nicht` / `more_data`, Ausschluss
`(02, passage_unvergleichbar)`. **Urteil: Enthaltung erzwungen** (fail-closed;
der Preis ist, dass eine legitim verschobene Passage unter derselben Kennung
nie mehr misst — *Einordnung: HÄRTUNG*).

**Der Passagenriegel ist der stärkste Riegel der Kette.** Er ist zugleich der
einzige, den der committete Korpus nie ausführt (siehe Vorbemerkung).

---

## Fall 6 — Fensterzahl genau 8 = `GATE_MINDEST_FENSTER` (Randwert `<` vs. `<=`)

| Sitzung | Kandidatenfenster | Ausgabe | Urteil |
|---|---|---|---|
| `g5_fenster_genau_acht` | 8 (Master 12) | `hoch` / `ready_to_send`, score 0,266635 | zulässig — M-23 „mindestens acht" |
| `g5_fenster_sieben` | 7 (Master 12) | `mittel` / `more_data`, score 0,266635 | Enthaltung/mittel erzwungen |

Der Vergleich ist ein `<` (`hypothese.rs`:1388). Beide Sitzungen tragen
identische Scores — die Klasse entsteht **strukturell**, nicht aus dem Score
(M-15), und genau darauf zielen die Fälle EIGEN-3 und EIGEN-4.

---

## Fall 7 — Gleichstand knapp über und knapp unter `RANG_QUANTUM` (M-25, M-26)

| Fall | Rangdifferenz | Ausgabe | Urteil |
|---|---|---|---|
| `korrelierter_distraktor` (Bestand) | exakt 0 → 0 Quanten | beide `mittel` | mittel erzwungen |
| `g5_distraktor_mehr_fenster` | exakt 0 → 0 Quanten | beide `mittel` | mittel erzwungen |
| `g5_distraktor_lauter` (0,1 dB im Befundband) | 3,21 · 10⁻⁴ ≈ **321 Quanten** | Distraktor `hoch` / `ready_to_send` | **FALSCHE STARKE BEHAUPTUNG** |
| `f7_rangdifferenz_am_quantum` | **exakt 2,0 · 10⁻⁶ = 2 Quanten** | Führender `hoch` / `ready_to_send` | **FALSCHE STARKE BEHAUPTUNG** |
| `g5_gleichstand_verursacher_fuehrt` (Verursacher führt) | 321 Quanten | Verursacher `hoch`, Distraktor `mittel` | zulässig, genau **ein** starker Befund (M-21 hält) |

`f7_rangdifferenz_am_quantum` ist die schärfste Form: beide Sonden tragen im
**Befundband 98..102 exakt dieselbe Anhebung** (+9,0 dB, 12 Fenster,
bekanntes Routing). Der Unterschied ist +0,1 dB in **Band 210** — über hundert
Bandindizes vom Befund entfernt und für die Behauptung ohne jede Bedeutung. Er
senkt die `bandpassung` der zweiten Sonde von 0,099809 auf 0,099800 und den
Rang um zwei Quanten. Das genügt: `getrennt` (`hypothese.rs`:1216–1222)
vergleicht die quantisierten Ränge mit `>`, der Führende bleibt `hoch` und
damit `READY TO SEND` — also `HOLD TO AUDITION` und `SEND DRAFT → EQ` (M-30).

**Zwei starke Befunde nebeneinander entstehen nie.** Der Riegel
`hypothese.rs`:1188–1194 (`befunde.iter_mut().skip(1)`) hält in allen sechs
Mehrkandidatenfällen. M-21 Satz 1 ist gemessen und grün; gebrochen wird der
Satz „Der Distraktor erzeugt **keine starke** Ursachenbehauptung".

**Grenze der Messung:** eine Differenz *zwischen* 0 und einem Quantum ist über
die Eingabe nicht darstellbar — das Evidenzgitter quantisiert auf 0,1 dB, und
die kleinste erreichbare Störung ergab bereits zwei Quanten. Die Seite „unter
dem Quantum" ist deshalb mit exakt gleichen Eingaben gemessen (0 Quanten), die
Seite „darüber" mit 2 Quanten. Dazwischen liegt nichts Darstellbares.

---

## Fall 8 — Zwei Master, Master ohne Fenster, widersprüchliches Befundband

### 8a — Zwei `plugin_kind = "main"` in einer Sitzung (`f8_zwei_master_…`)

**Eingabe:** Master A (Instanz 01, Mixer 0) mit Anomalie in **Band 98..102**;
Master B (Instanz 09, Mixer 1) mit Anomalie in **Band 150..154**; eine Sonde
(02, Mixer 3), die ausschliesslich in 98..102 drückt. Alle je 12 Fenster.

**Tatsächliche Ausgabe:**

```
src=02 klasse=hoch zust=ready_to_send ursache=quelle_resonanz score=0.252010
  rang: bandpassung=0.012059 koinzidenz=0 uplift=0 intent=0.5 wiederholbarkeit=0 routing=1.0
  likely_cause="00000000 draengt im Bandbereich 149..152 gegen den Master."
```

**Urteil: FALSCHE STARKE BEHAUPTUNG.** Entscheidender Riegel: **keiner**.
`aufnahmen_sammeln` (`hypothese_verdrahtung.rs`:216) schreibt
`master = Some(profil)` ohne zu prüfen, ob schon einer gesetzt ist; die
Schlüssel sind nach `instance_id` sortiert, der **letzte** `main` gewinnt.
Master A verschwindet vollständig — er wird nicht einmal Kandidat. Die Sonde
wird dann gegen das Band von Master B gemessen, in dem sie **null** Energie
trägt (`bandpassung = 0,0121`), und trägt trotzdem `READY TO SEND` mit einer
Zeile, die auf `149..152` zeigt.

### 8b — Master ohne Fenster (`f8b_master_ohne_fenster`)

**Eingabe:** `main` angemeldet, Deskriptor gesetzt, **kein Beleg**; nur die
Sonde sendet 12 Fenster.

**Tatsächliche Ausgabe:** **kein Befund, kein Snapshot-Eintrag, kein Ausschluss
— Schweigen.** `aufnahmen_sammeln` bricht bei `let Some(master) = master else
{ continue }` (`hypothese_verdrahtung.rs`:221) ab.

**Urteil: keine starke Aussage** — aber auch keine Enthaltung. Das
Kettenbein `sonde014_p5_korpus.rs`:288–296 hält ausdrücklich fest: „Eine
Sitzung ohne Befund wäre ein Schweigen, und M-27 verlangt ein Ergebnis."
Diese Zusage ist **nur für Korpussitzungen** durchgesetzt; im Produktpfad gibt
es einen Zustand, in dem Gen misst, eine Sonde liefert und nichts erscheint.
*Einordnung: LÜCKE* — der Quellkommentar nennt den Zustand ausdrücklich
regulär („Eine Sitzung ohne Main hat keinen Master und rechnet gar nicht —
das ist kein Fehler"), und keine Matrixzeile verlangt für den Fall
„Main angemeldet, aber ohne Beleg" ein Ergebnis.

---

## Fall 9 — Intent-Änderung und Rücknahme nach der Rechnung (M-10, M-24, M-28)

Das Interleaving **während** der Rechnung misst bereits
`broker/tests/sonde014_verdrahtung.rs::intent_und_ruecknahme_waehrend_der_rechnung`
(Testhaken `rechen_test_haken_setzen`, `hypothese_verdrahtung.rs`:91). Geprüft
wurde hier die **andere** Reihenfolge: der Befund steht bereits auf
`READY TO SEND`, dann erst kommt das Ereignis — ohne neuen Beleg, der eine
Rechnung auslösen würde.

| Teil | Eingabe | Ausgabe | Urteil |
|---|---|---|---|
| F9a | `intent_update` Revision 1 setzt die Quelle auf `geschuetzt` | Befund bleibt sichtbar, `confidence.klasse` bleibt `hoch`, **`zustand` wird `stale`**; Snapshot: `klasse="hoch" zustand="stale"` | Enthaltung erzwungen — `stale` ist nicht handelbar (M-30) |
| F9b | `invalidierung_wegen_intervention_fuer_link` nimmt 2 Belege des sechsten Projektfensters zurück | `zustand` wird `stale`, `evidence_ids` fällt von 12 auf 11 | Enthaltung erzwungen |

Kein alter `hoch`-Befund bleibt **handelbar**, nachdem der Intent ihn nicht
mehr deckt oder Belege fehlen. Beide Riegel halten. Anzumerken bleibt: die
Sicherheitsklasse `hoch` reist im Snapshot weiter mit; nur der Zustand trägt
die Entwertung. Das ist genau die von M-29/M-30 verlangte Arbeitsteilung
(zwei Achsen, eine Sperre im Datenweg), also kein Befund — aber Gen darf die
Klasse nie ohne den Zustand anzeigen.

---

## Fall 10 — Eigene Angriffsideen aus dem Quelltext der Klassenwahl

Die Klassenwahl liest **drei** Grössen (`hypothese.rs`:1387–1396):

```rust
let klasse = if fenster < GATE_MINDEST_FENSTER || !kandidat.routing_bekannt || parent_duplikat
    { Sicherheitsklasse::Mittel } else { Sicherheitsklasse::Hoch };
```

Keine davon ist ein Mass für den **Zusammenhang**. Die sechs Rangkomponenten,
die den Zusammenhang messen, gehen ausschliesslich in `confidence.score` ein
und entscheiden nur die Reihenfolge. Daraus folgen drei Angriffe.

### EIGEN-1 — Die Quelle ohne jede Anomalie (`g5_unbeteiligte_quelle`, Korpus)

**Eingabe:** Master 12 Fenster @ +9,0 dB in 98..102; eine Sonde (02),
12 Fenster, **`anhebung_db = 0,0`** — sie trägt im Befundband die reine
Fixture, also **keine** Erhöhung. Mixer 3, kein Versatz.

**Ausgabe:** `klasse=hoch zust=ready_to_send ursache=quelle_resonanz
score=0.253017`.

**Urteil: FALSCHE STARKE BEHAUPTUNG.** Der Prüfer meldet zusätzlich:
`Ursachenklasse 'quelle_resonanz' weicht von der Korpuswahrheit
'daten_reichen_nicht' ab (NR-13, M-64)`. Das Produkt benennt eine Ursache, wo
die Wahrheit „reicht nicht" lautet.

### EIGEN-2 — Die Quelle im falschen Band (`eigen4_quelle_im_falschen_band`)

**Eingabe:** Master @ +9,0 dB in 98..102; Sonde @ +9,0 dB in **30..34**.

**Ausgabe:** `klasse=Hoch zustand=ReadyToSend`, `bandpassung = 0,016079`,
`koinzidenz = 0`, `uplift = 0`, `wiederholbarkeit = 0`, `routing = 1,0`,
`likely_cause = "… draengt im Bandbereich 97..101 gegen den Master."`

**Urteil: FALSCHE STARKE BEHAUPTUNG.** `gate()` (`hypothese.rs`:798–892) prüft
Rücknahme, Capability, Coverage, Alignment, Passage und Intent — es prüft nie,
ob der Kandidat im Befundband überhaupt Energie trägt. Fünf von sechs
Rangkomponenten sagen „kein Zusammenhang", und die Klasse sieht keine davon.

### EIGEN-3 — Die antikorrelierte Quelle (`eigen3_antikorrelierte_quelle`)

**Eingabe:** Master mit alternierendem Pegel im Befundband (ungerade Fenster
+12 dB, gerade +6 dB; das **letzte** Fenster ist laut, damit `masteranomalie`
dasselbe Band findet). Sonde **genau gegenläufig**: ungerade +6 dB, gerade
+12 dB. Sie ist also laut, **wenn der Master leise ist**.

**Ausgabe:**

```
klasse=Hoch zustand=ReadyToSend score=0.435800
rang: bandpassung=0.119789 koinzidenz=0 uplift=0 intent=0.5
      wiederholbarkeit=0.995012 routing=1.0
```

**Urteil: FALSCHE STARKE BEHAUPTUNG, und die härteste der Serie.** Der Grund
steht in zwei Zeilen:

* `uplift()` (`hypothese.rs`:962–967) verwirft eine **negative** Differenz mit
  `if differenz <= 0.0 { return 0.0 }` — ein Gegenbeleg kostet exakt so viel
  wie „keine Angabe": nichts. `koinzidenz()` (:947) klemmt die negative
  Korrelation mit `.max(0.0)` genauso.
* `bootstrap_p()` (`experiment.rs`:1518–1553) ist **zweiseitig**: es zählt
  `nicht_positiv` und `nicht_negativ` und nimmt das Minimum. Eine stabil
  **negative** Reihe bekommt denselben kleinen p-Wert wie eine stabil positive
  — `wiederholbarkeit = 1 − p` steht deshalb bei **0,995**.

Der Gegenbeleg wird also nicht nur ignoriert, er wird **belohnt**: der
antikorrelierte Kandidat erreicht Rang 0,4358 und liegt damit **über** dem
Rang 0,2666, den die echte Ursache in allen flachen Sitzungen (einschliesslich
`wahrer_kandidat` im Bestandskorpus) erreicht.

Der Kontrollfall `eigen5_antikorreliert_neben_korreliert` zeigt, dass die
**Reihenfolge** noch stimmt — die gleichläufige Quelle führt mit 0,6025
(`uplift = 1,0`) vor der gegenläufigen mit 0,4358, und nur sie wird `hoch`.
Allein im Rennen wird die gegenläufige aber `READY TO SEND`.

---

# Tabelle: Fall → entscheidender Riegel → Ausgabe → Urteil

| Fall | Entscheidender Riegel (Datei:Zeile) | Ausgabe | Urteil |
|---|---|---|---|
| `g5_distraktor_mehr_fenster` | `hypothese.rs`:1210–1228 `getrennt` | 2 × `mittel` / `more_data` | mittel erzwungen |
| `g5_distraktor_lauter` | keiner (2 Ränge trennen, 321 Quanten) | `hoch` / `ready_to_send` auf Distraktor | **FALSCHE STARKE** |
| `g5_distraktor_allein_im_rennen` | keiner (`hypothese.rs`:1219 `_ => true`) | `hoch` / `ready_to_send`, Ausschluss `alignment_falsch` mit im Befund | **FALSCHE STARKE** |
| `g5_unbeteiligte_quelle` | keiner (`hypothese.rs`:1387–1396 liest nur 3 Grössen) | `hoch` / `ready_to_send` | **FALSCHE STARKE** |
| `g5_sonde_auf_masterkanal` | keiner (`hypothese.rs`:1108–1121, `ids` ohne Master) | `hoch` / `ready_to_send` | **FALSCHE STARKE** |
| `g5_parent_partner_faellt_aus` | `hypothese.rs`:1113–1121 + 1136–1141 | `mittel` / `more_data` | mittel erzwungen |
| `g5_parent_partner_zu_wenig_fenster` | `hypothese.rs`:1136–1141 | 2 × `mittel` / `more_data` | mittel erzwungen |
| `g5_parent_partner_ohne_evidenz` | keiner (`hypothese_verdrahtung.rs`:164–187 `je_kanal`) | `hoch` / `ready_to_send` | **FALSCHE STARKE** (HÄRTUNG) |
| `g5_fenster_genau_acht` | `hypothese.rs`:1388 `fenster < 8` | `hoch` / `ready_to_send` | zulässig |
| `g5_fenster_sieben` | `hypothese.rs`:1388 | `mittel` / `more_data` | mittel erzwungen |
| `g5_kandidatendeckel` | `hypothese.rs`:1154 `truncate` (ohne Grund) | `hoch` auf lautesten Distraktor, wahre Ursache spurlos | **FALSCHE STARKE** + M-87 |
| `g5_gleichstand_verursacher_fuehrt` | `hypothese.rs`:1188–1194 | `hoch` + `mittel` | zulässig |
| `f5` Passage `[0,12)` | `hypothese.rs`:838–854 | `hoch` / `ready_to_send` | zulässig |
| `f5` Passage `[0,11)` | `hypothese.rs`:853–854 `passagenmaterial` = 11 | `hoch` / `ready_to_send` | zulässig |
| `f5` Passage `[0,8)` | `hypothese.rs`:853–854 (`<`, nicht `<=`) | `hoch` / `ready_to_send` | zulässig |
| `f5` Passage `[0,7)` | `hypothese.rs`:853–854 → `PassageZuKurz` | `daten_reichen_nicht` / `more_data` | **Enthaltung erzwungen** |
| `f5` Passage `[12,24)` | `hypothese.rs`:838–842 → `PassageUnvergleichbar` | `daten_reichen_nicht` / `more_data` | **Enthaltung erzwungen** |
| `f5b` gleiche `passage_id`, verschoben | `experiment_verdrahtung.rs`:445–476 + `hypothese.rs`:838–842 | `daten_reichen_nicht` / `more_data` | **Enthaltung erzwungen** |
| `f8` zwei Master | keiner (`hypothese_verdrahtung.rs`:216 letzter gewinnt) | `hoch` / `ready_to_send` im fremden Band | **FALSCHE STARKE** |
| `f8b` Master ohne Fenster | `hypothese_verdrahtung.rs`:221 `continue` | **gar nichts** | keine Aussage (LÜCKE) |
| `f4` vor dem Kanalwechsel | `hypothese.rs`:1136–1141 | 2 × `mittel` | mittel erzwungen |
| `f4` nach dem Kanalwechsel | keiner (`hypothese_verdrahtung.rs`:176–187 kennt nur den letzten Stand) | `hoch` / `ready_to_send` | **FALSCHE STARKE** (LÜCKE) |
| `f4b` drei Sonden auf einem Kanal | `hypothese.rs`:1113–1141 | 3 × `mittel` | mittel erzwungen |
| `eigen3` antikorreliert | keiner (`hypothese.rs`:962–967 + `experiment.rs`:1542–1552) | `hoch` / `ready_to_send`, Rang 0,4358 | **FALSCHE STARKE** |
| `eigen4` falsches Band | keiner (`hypothese.rs`:798–892 ohne Bandrelevanz) | `hoch` / `ready_to_send`, `bandpassung` 0,0161 | **FALSCHE STARKE** |
| `eigen5` antikorreliert neben korreliert | `hypothese.rs`:1188–1194 | `hoch` (korreliert) + `mittel` (antikorreliert) | zulässig |
| `f7` Rangdifferenz 2 Quanten | keiner (`hypothese.rs`:1216–1222 `a > b`) | `hoch` / `ready_to_send` | **FALSCHE STARKE** |

---

# Befunde mit Klasse

## DEFEKT

**D1 — Der Gate-Satz von G5 hält nicht: Gegenbeispiele erzwingen keine Enthaltung.**
`broker/src/coordinator/hypothese.rs`:1387–1396.
Gebrochen: Gate-Text G5 (`docs/bauaufteilung-sonden.md`:397–400), Exit-Gate §59
Satz 3 („Gegenbeispiele erzwingen nachweislich Enthaltung"), M-69, M-21.
Reproduktion: `py -3.13 tools/eq-copilot/erzeuge_p5_korpus.py` ·
`cargo test --manifest-path broker/Cargo.toml --test sonde014_p5_korpus` ·
`py -3.13 tools/eq-copilot/pruefe_p5_korpus.py` → Exit 2,
`gesamt: 6 falsche starke Behauptung(en)`.
Kern: `confidence.klasse` entsteht aus **drei** strukturellen Grössen
(Fensterzahl, `routing_bekannt`, `parent_duplikat`). Keine misst den
**Zusammenhang**. Die sechs Grössen, die ihn messen, wirken ausschliesslich auf
`confidence.score` und damit nur auf die Reihenfolge. Ein Kandidat ohne jeden
Beleg für einen Zusammenhang (`g5_unbeteiligte_quelle`, `eigen4`) und ein
Kandidat mit **Gegenbeleg** (`eigen3`) erreichen deshalb `hoch` und damit
`READY TO SEND` — und nach M-30 `HOLD TO AUDITION` und `SEND DRAFT → EQ`.
M-15 verbietet, die Klasse aus dem Score zu **runden**; es verbietet nicht,
einen fehlenden oder gegenläufigen Zusammenhang als Gate zu führen.

**D2 — Ein Ausschluss mit Grund senkt die Sicherheit des Überlebenden nicht.**
`hypothese.rs`:1216–1223 (`_ => true`) und 1230–1232.
Gebrochen: M-21, M-69. Reproduktion: Sitzung `g5_distraktor_allein_im_rennen`.
Ein einziger Überlebender gilt als „getrennt", auch wenn im selben Befund ein
`ausschluesse`-Eintrag steht, der belegt, dass ein Konkurrent **nicht messbar
war**. Das Produkt behauptet damit stark auf einer nachweislich
unvollständigen Datenlage. Derselbe Mechanismus trägt
`g5_parent_partner_ohne_evidenz` und die Deckelkürzung aus D3.

**D3 — Der Kandidatendeckel schneidet ohne Ausschlussgrund.**
`hypothese.rs`:1154 `ueberlebende.truncate(KANDIDATEN_DECKEL)`.
Gebrochen: M-87 („**Jeder** Kandidat, der ausscheidet, trägt einen Grund aus
einer geschlossenen Menge"). Reproduktion: Sitzung `g5_kandidatendeckel` —
Instanz `…02` erscheint weder als Befund noch als Alternative noch als
Ausschluss. Die geschlossene Menge (`hypothese.rs`:253–274, gespiegelt in
`MANIFEST.json` und `$defs` des Vertrags) enthält keinen Grund, der den Fall
tragen könnte; der Bruch ist deshalb nicht durch Setzen eines bestehenden
Werts zu heilen.

**D4 — Zwei `main` in einer Sitzung: der erste Master verschwindet lautlos.**
`hypothese_verdrahtung.rs`:216.
Gebrochen: M-69 (Gegenbeispiel erzeugt eine starke Aussage) und der Grundsatz
„keine toten oder lügenden Anzeigen" — `likely_cause` nennt ein Band, in dem
der Kandidat nachweislich keine Energie trägt (`bandpassung = 0,0121`).
Reproduktion: `cargo test --test g5_gegenbeispiele f8_zwei_master…`.
`master = Some(profil)` überschreibt ohne Prüfung; der verdrängte `main` wird
auch nicht Kandidat.

**D5 — Der Gegenbeleg wird belohnt statt gewertet.**
`hypothese.rs`:962–967 (`uplift`, `differenz <= 0.0 → 0.0`), :947
(`koinzidenz`, `.max(0.0)`) und `experiment.rs`:1542–1552 (`bootstrap_p`
zweiseitig).
Gebrochen: M-19 („vergleicht **bedingten Uplift** … prüft Stabilität über
Block-Bootstrap, **alternative Erklärungen**") in Verbindung mit dem
Gate-Text von G5. Reproduktion: `cargo test --test g5_gegenbeispiele eigen3`.
Eine Quelle, die im Befundband **gegenläufig** zum Master schwingt, erreicht
`wiederholbarkeit = 0,995` und Rang 0,4358 — mehr als jede korrekt gemessene
Ursache in den flachen Sitzungen. Sie steht allein im Rennen auf
`READY TO SEND`.

## LÜCKE

**L1 — Der Passagenriegel ist im committeten Korpus ungemessen.**
`sonde014_p5_korpus.rs`:142 (`sitzung_fahren` legt keine Passage an) gegen
`hypothese_verdrahtung.rs`:237–238. Die Sitzung `verschobene_passage` fällt an
`gate()`:828 (`AlignmentFalsch`), nicht an `gate()`:853–878. Die Zusagen
M-23/M-67 zur **Passage** sind damit vom Korpus nicht belegt. Der beigelegte
Test `f5_passagenrandwerte` zeigt, dass der Riegel selbst hält — die Lücke ist
die der Messanlage, nicht des Produkts.

**L2 — Kanalwechsel während einer Sitzung.**
`hypothese_verdrahtung.rs`:176–187. Der Mixerkanal wird zum Rechenzeitpunkt
aus dem Deskriptor gelesen; die Fenster tragen keinen. Eine reine
Deskriptoränderung hebt einen Befund von `mittel` auf `hoch`, ohne dass ein
Messwert dazugekommen wäre. Keine Matrixzeile sagt zu dem Fall etwas.

**L3 — Zwei Ebenen (Großeltern) sind nicht darstellbar.**
Ein Deskriptor trägt genau ein `host_mixer_index`; die Elternbeziehung ist die
flache Menge gleicher Kanäle (`hypothese_verdrahtung.rs`:226–233). Eine
Hierarchie Kind→Eltern→Großeltern hat keine Eingabe.

**L4 — Main angemeldet, aber ohne Beleg: Schweigen statt Ergebnis.**
`hypothese_verdrahtung.rs`:221. Weder Befund noch Enthaltung noch Ausschluss.
Das Kettenbein hält die Zusage „jede Sitzung liefert ein Ergebnis" nur für
Korpussitzungen durch (`sonde014_p5_korpus.rs`:288–296).

## HÄRTUNG

**H1 — Ein Duplikatpartner ohne Evidenz ist unsichtbar.**
`hypothese_verdrahtung.rs`:164–187. `je_kanal` entsteht aus den
Evidenzschlüsseln; eine angemeldete, stumme Sonde auf demselben Kanal setzt
die Duplikatmarke nicht. Bedrohungsmodell ist die versehentliche Regression,
nicht der Angreifer — deshalb Härtung, nicht Defekt.

**H2 — Eine verschobene Passage unter derselben Kennung misst nie mehr.**
`experiment_verdrahtung.rs`:445–476. Fail-closed ist an dieser Stelle richtig;
der Preis ist, dass ein zweiter `experiment_begin` mit gleicher `passage_id`
und neuem Zeitfenster dauerhaft `passage_unvergleichbar` erzeugt.

**H3 — `bandpassung` als Trennschärfe.** In `f7` entscheiden zwei Quanten aus
einem Band, das mit dem Befund nichts zu tun hat, über `READY TO SEND`. Ein
Trennungskriterium, das nur die **Befundband**-Energie liest, wäre robuster.

---

# Ausgabe von `pruefe_p5_korpus.py` auf dem erweiterten Korpus

Roh: `…\G5-gegenbeispiele-bf92891\pruefe-p5-korpus.txt` (Exit 2).

```
P5-Korpus: 18 Sitzungen, 28 ausgegebene Befunde
  daten_reichen_nicht: n=2 precision=0.000 recall=1.000 (soll_starke=0) brier=0.810
                       kalibrierung=0.900 coverage=0.500 enthaltung=0.500 falsche_starke=1
  quelle_resonanz:     n=9 precision=0.400 recall=1.000 (soll_starke=2) brier=0.366
                       kalibrierung=0.162 coverage=0.889 enthaltung=0.111 falsche_starke=3
  zwei_quellen_konkurrenz: n=17 precision=0.333 recall=1.000 (soll_starke=1) brier=0.228
                       kalibrierung=0.229 coverage=1.000 enthaltung=0.000 falsche_starke=2
  GESAMT: n=28 precision=0.333 recall=1.000 brier=0.293 kalibrierung=0.065
          coverage=0.929 enthaltung=0.071 rangmittel=0.250
  ZUVERLAESSIGKEIT hoch:   n=9  nennwert=0.90 gemessene Trefferquote=0.333
  ZUVERLAESSIGKEIT mittel: n=17 nennwert=0.60 gemessene Trefferquote=1.000
  Schwelle hoch:   handelbar=9  davon_falsch=6  haelt=False
  Schwelle mittel: handelbar=26 davon_falsch=15 haelt=False
  Schwelle unklar: handelbar=28 davon_falsch=15 haelt=False
  SCHWELLE (Ausgabe, M-31): niedrigste haltende Stufe = None
  PRODUKTSCHWELLE (M-31): 9 von 28 Befunden handelbar, Sicherheiten ['hoch']
  P4-LUECKE NAK-190 (M-70): gedruckt, inzwischen gemessen durch korrelierter_distraktor,
      g5_distraktor_mehr_fenster, g5_kandidatendeckel, g5_gleichstand_verursacher_fuehrt
  GATE_MINDEST_FENSTER: 3 Faelle aus den zwei Passagensitzungen, 0 davon stark
  ROT: g5_distraktor_lauter: die Alternative '131d40…' gehoert der Quelle '…02', nicht der
       deklarierten Distraktorquelle (NR-14, NAK-190)
  ROT: g5_distraktor_lauter: keine Alternative loest auf die deklarierte Distraktorquelle auf
  ROT: g5_unbeteiligte_quelle: Ursachenklasse 'quelle_resonanz' weicht von der Korpuswahrheit
       'daten_reichen_nicht' ab (NR-13, M-64)
  ROT: daten_reichen_nicht: 1 falsche starke Behauptung(en): g5_unbeteiligte_quelle
  ROT: quelle_resonanz: 3 falsche starke Behauptung(en): g5_distraktor_allein_im_rennen,
       g5_sonde_auf_masterkanal, g5_parent_partner_ohne_evidenz
  ROT: zwei_quellen_konkurrenz: 2 falsche starke Behauptung(en): g5_distraktor_lauter,
       g5_kandidatendeckel
  ROT: gesamt: 6 falsche starke Behauptung(en): g5_distraktor_lauter,
       g5_distraktor_allein_im_rennen, g5_unbeteiligte_quelle, g5_sonde_auf_masterkanal,
       g5_parent_partner_ohne_evidenz, g5_kandidatendeckel
  ROT: keine Sicherheitsstufe haelt die Riegel — die Schwelle aus M-31 ist nicht bestimmbar
```

**Kennzahlen im Vergleich.** Auf dem Bestandskorpus (6 Sitzungen) fand
`schwelle_suchen()` als niedrigste haltende Stufe `unklar` (M-31, Nachtrag
Etappe H). Auf dem erweiterten Korpus hält **keine** Stufe mehr: die
Zuverlässigkeit von `hoch` fällt von 1,000 auf **0,333** — von neun Befunden,
die das Produkt `hoch` nannte, tragen sechs die Wahrheit nicht. Die
Produktschwelle selbst ist unverändert konsistent (`ready_to_send` genau bei
`hoch`, `produktschwelle()` meldet nichts); der Bruch liegt nicht in der
Schwelle, sondern in dem, was `hoch` bedeutet.

**Zwei Nebenbefunde des Prüfers, die zur Messanlage gehören, nicht zum Produkt:**

* Die `NR-14`-Zusage („eine `alternatives`-ID löst auf die **deklarierte**
  Distraktorquelle auf") setzt implizit voraus, dass der **Verursacher**
  führt. In `g5_distraktor_lauter` führt der Distraktor, seine Alternative
  zeigt auf den Verursacher — formal korrekt, vom Riegel aber als Fehler
  gemeldet. Der Riegel ist richtungsabhängig formuliert.
* `p4_luecke` meldet die NAK-190-Zeile jetzt als „gedruckt, inzwischen
  gemessen … sie darf jetzt entfernt werden" — vier Sitzungen lösen eine
  Alternative auf die deklarierte Distraktorquelle auf.

---

# Beigelegte Artefakte

| Datei | Inhalt |
|---|---|
| `G5-gegenbeispiele-bf92891.patch` | `git diff` des Clones nach `git add -N .`: `broker/tests/g5_gegenbeispiele.rs` (neu), `tools/eq-copilot/erzeuge_p5_korpus.py` (+12 Sitzungen), die daraus bytegleich erzeugten `eq-copilot/fixtures/p5-korpus/{sitzungen,MANIFEST}.json` |
| `G5-gegenbeispiele-bf92891\cargo-korpus.txt` | Rohausgabe `cargo test --test sonde014_p5_korpus` |
| `G5-gegenbeispiele-bf92891\cargo-g5-gegenbeispiele.txt` | Rohausgabe `cargo test --test g5_gegenbeispiele -- --nocapture --test-threads=1` (alle Rangkomponenten je Befund) |
| `G5-gegenbeispiele-bf92891\pruefe-p5-korpus.txt` | Rohausgabe `pruefe_p5_korpus.py` (Exit 2) |
| `G5-gegenbeispiele-bf92891\erzeuge-p5-korpus-pruefen.txt` | `erzeuge_p5_korpus.py --pruefen` → `bytegleich` |
| `G5-gegenbeispiele-bf92891\p5-korpus-ergebnis.json` | Das Ergebnis des erweiterten Korpuslaufs (18 Sitzungen, 28 Befunde) |

**Der Patch ändert keinen Produktcode.** `broker/src/**` und
`eq-copilot/plugin/**` sind unberührt; geändert sind ausschliesslich Eingaben
(Korpus), der Korpuserzeuger und eine neue Testdatei.
