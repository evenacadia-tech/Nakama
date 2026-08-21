# 2026-08-21 — Codex-Review der Messbank, Volltext

> **Quelle, damit sie nachprüfbar ist:** Codex-Sitzung
> `01a022fa-2e75-7f11-ab0f-2f12df982e5d`, Arbeitsverzeichnis
> `Projekte/Nakama-Design`, Auftrag des Users um 08:20 Uhr:
> *„mach ein ausführliches code review des aktuellen
> implementierungsstandes des eq sondenplanes"*. Schlussbewertung
> geschrieben um **09:16 Uhr**, geprüft wurde `HEAD 86f04c1`.
> Herausgezogen aus
> `~/.codex/sessions/2026/08/21/rollout-2026-08-21T08-20-16-01a022fa-….jsonl`,
> **unverändert** — nicht vom Bildschirm abgelesen.

> ⚠ **SEKUNDÄRQUELLE. Nichts davon ist verifiziert.** `CLAUDE.md` und der
> eigene Prüfbericht dieses Projekts sagen beide: rund ein Viertel der
> AI-Auditbefunde war hier falsch. **Vor jedem Handgriff die Quelldatei
> öffnen.** Kein Befund verschwindet still — gefixt, widerlegt (mit
> Protokoll) oder als offene Zeile.

**Zehn Befunde: vier P1, fünf P2, einer P3.** Die Pfadangaben stehen
absolut, so wie Codex sie geschrieben hat. Die `::code-comment{…}`-Blöcke
sind Codex' eigene Anker-Auszeichnung und bleiben stehen, weil sie Datei
und Zeile tragen.

## Was Codex selbst ausgeführt hat

Am Ende der Bewertung steht ein Verifikationsabschnitt: `pruefen.mjs`
10/10, Gegenprobe 3/3, `sondenprobe.mjs` 12 Proben „ALLES SAUBER" — und
der Satz, auf den es ankommt: *„die oben beschriebenen falschen Grünfälle
bleiben jedoch bestehen."* Dazu echte Playwright-Interaktionen für Hold,
Discard, Bypass, Active-/Passive-Umschalter und Export. **Keine Dateien
verändert.**

## Die vier P1 in einer Zeile

| # | Befund | Datei |
|---|---|---|
| 1 | Der Ergebnisexport verliert die entscheidenden Messwerte | `werkzeug/formfaktor.html:806` |
| 2 | Die Maßstabs-Gegenprobe misst zweimal denselben Maßstab | `werkzeug/pruefung/sondenprobe.mjs:120` |
| 3 | Die Zustandsmatrix übersieht genau die noch falschen Darstellungen | `werkzeug/pruefung/sondenprobe.mjs:342` |
| 4 | Der Grenzfall-Gate ist fest auf Grün verdrahtet (`z(true, …)`) | `werkzeug/sonde-messung.html:1398` |

**Befund 2 und 4 wiegen am schwersten**, weil sie dieselbe Krankheit sind,
gegen die die Sonde überhaupt gebaut wurde: ein Gate, das nicht scheitern
kann, beweist nichts (`werkzeug/PRUEFLISTE.md` 7). Die Sonde entstand,
WEIL die erste Fassung zwei gleich falsche Werte verglich und zufrieden
„0 Abweichungen" meldete.

---

# Volltext der Bewertung

Kurzurteil: Der Stand auf `86f04c1` ist eine gute, reproduzierbare Messbank – aber noch kein belastbarer EQ-Editor. Die zentrale Erkenntnis stimmt: 700×420 reicht nicht für acht dauerhaft vollständige Bandzeilen plus eine brauchbare EQ-Kurve. Mehrere Prüfungen und Exporte erzeugen jedoch noch falsche Sicherheit.

## Befunde

1. **[P1] Der Ergebnisexport verliert ausgerechnet die entscheidenden Messwerte.**  
   [formfaktor.html:806](C:/Users/phili/Projekte/Nakama-Design/werkzeug/formfaktor.html:806) exportiert für Active und Passive nur `name`, `w` und `h`. Im Browser wurden Active `0 px` Überlauf, aber nur `5 px` nutzbare Kurvenhöhe sowie Passive `23 px` vertikaler Überlauf gemessen. Nichts davon landet im JSON. Ein gespeichertes Ergebnis sieht dadurch positiv aus, obwohl beide Oberflächen unbrauchbar sind.

::code-comment{title="[P1] Export verwirft Messbefunde" body="Der Livezustand enthält Überlauf, Richtung und verbleibende Kurvenhöhe, exportiert werden aber nur statische Namen und Maße. Damit verliert der Ergebnisdatensatz genau die Befunde, für die dieses Werkzeug existiert." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/formfaktor.html" start=816 end=817 priority=1}

2. **[P1] Die Maßstabs-Gegenprobe misst zweimal denselben Maßstab.**  
   In [sondenprobe.mjs:120](C:/Users/phili/Projekte/Nakama-Design/werkzeug/pruefung/sondenprobe.mjs:120) wird `--s:1` auf die vorhandenen Rahmen gesetzt; unmittelbar danach ersetzt `zeichnen()` diese DOM-Knoten. Reale Folge: `.46 → 1 → .46`. `beiKlein` und `beiGross` werden somit beide wieder im kleinen Maßstab gelesen, obwohl die Ausgabe fünf geprüfte Maßstabsfälle behauptet.

::code-comment{title="[P1] Maßstabsprobe ist wirkungslos" body="zeichnen ersetzt die Rahmen, auf denen unmittelbar zuvor --s gesetzt wurde. Der zweite Messlauf findet deshalb wieder den CSS-Standardmaßstab vor und kann einen transformabhängigen Regressionsfehler nicht erkennen." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/pruefung/sondenprobe.mjs" start=120 end=123 priority=1}

3. **[P1] Die neue Zustandsmatrix übersieht genau die noch falschen Darstellungen.**  
   [sondenprobe.mjs:342](C:/Users/phili/Projekte/Nakama-Design/werkzeug/pruefung/sondenprobe.mjs:342) kontrolliert nur Fall A und darin nur `.slotz` sowie ausgewählte Felder. Gain, Kanal, der dynamisch-an/aus-Indikator und die Kontextstreifen aus Fall D fehlen. Deshalb meldet die Probe 16/16 sauber, obwohl bei Bypass:

   - die vollständige Zeile weiterhin `dyn` zeigt;
   - die Kontextstreifen weiterhin `24 Hz` beziehungsweise `86 Hz · −3.5 dB` zeigen.

   Ursache sind unter anderem [slotStreifen()](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:474) und die nicht über `dtot` gerenderte Beschriftung in [slotZeile()](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:462).

::code-comment{title="[P1] Zustandsprobe prüft nur eine Darstellungsform" body="Die Schleife behauptet jeden Bandparameter zu prüfen, liest aber nur Fall A, nur slotz und weder dy noch die Kontextstreifen aus Fall D. Dadurch bleiben sichtbare Werte unter Bypass unentdeckt." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/pruefung/sondenprobe.mjs" start=342 end=354 priority=1}

4. **[P1] Der Grenzfall-Gate ist fest auf Grün verdrahtet.**  
   [sonde-messung.html:1398](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:1398) ruft `z(true, ...)` auf. Der Ausgangszustand ist gleichzeitig `realistisch`, Bypass aus und Main verbunden. Trotzdem erscheint „Grenzfall ist die Vorgabe“ grün. Die externe Probe prüft ebenfalls nur drei Felder in [sondenprobe.mjs:311](C:/Users/phili/Projekte/Nakama-Design/werkzeug/pruefung/sondenprobe.mjs:311), obwohl der Plan zusätzlich getrenntes Main, Bypass und Schutzbereich nennt. Sinnvoll wäre die getrennte Prüfung von Platz- und Ehrlichkeitsgrenzfall.

::code-comment{title="[P1] Grenzfall kann nie scheitern" body="Der Status wird unabhängig vom aktuellen WELT-Zustand als erfolgreich ausgegeben. Dadurch ist sogar der normale verbundene Zustand mit realistischen Werten ein grüner Grenzfall." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html" start=1398 end=1403 priority=1}

5. **[P2] Der angeblich spezifizierte Maximalfall enthält erfundene Grenzen.**  
   [sonde-messung.html:382](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:382) nennt Q 24, Schwelle −60 dB, Bereich −18 dB, Attack 300 ms, Hold 500 ms und Release 3 s „nicht geraten“ und „nach der Spezifikation“. Die einzige Sachquelle spezifiziert jedoch nur Gain ±12 dB; für die übrigen Werte stehen keine Grenzen in [oberflaechen-spezifikation.md:55](C:/Users/phili/Projekte/Nakama-Design/docs/oberflaechen-spezifikation.md:55). Diese Zahlen müssen als Testannahmen markiert oder fachlich entschieden werden.

   Dasselbe gilt für die pauschale Lesbarkeitsschwelle von `15,4 px` in [sonde-messung.html:1358](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:1358): Die Höhe einer Textzeile ist ein brauchbarer Hinweis, aber kein bewiesenes Mindestmaß für eine EQ-Kurve.

::code-comment{title="[P2] Maximalwerte stammen nicht aus der Sachquelle" body="Außer Gain ±12 dB definiert die Spezifikation keine dieser Grenzen. Der Datensatz darf deshalb nicht als spezifizierter längstmöglicher Slot bezeichnet werden." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html" start=382 end=387 priority=2}

6. **[P2] Draft, Discard, Undo und Neutralise bilden noch keine ehrliche Zustandsmaschine.**  
   Ein echter Klick auf Discard lässt `LAGE` und die sichtbare Draft-Zeile unverändert. In [sonde-messung.html:1193](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:1193) setzt Discard lediglich das ohnehin falsche `angewandt=false`; ein Zustand „Vorschlag verworfen/nicht mehr vorhanden“ fehlt. Neutralise besitzt überhaupt keinen Handler. Apply ändert keine Bänder, und die Draft-Kurve ist nur die bestehende EQ-Kurve mit Faktor `0.25` statt eines eigenen Vorschlagsdatensatzes ([sonde-messung.html:547](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:547)).

   Positiv: echtes `pointerdown`/`pointerup` beim Audition-Halten funktioniert im Browser korrekt. Für funktionale Abnahme reichen die übrigen Übergänge aber noch nicht.

::code-comment{title="[P2] Discard und Undo ändern keinen fachlichen Zustand" body="Beide Aktionen setzen nur angewandt auf false. Ein verworfener Draft bleibt sichtbar, die statische History bleibt unverändert und Neutralise ist gar nicht verdrahtet. Das interaktive Spezimen kann die geforderten Übergänge daher nicht belegen." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html" start=1193 end=1197 priority=2}

7. **[P2] Die Kurvenauflösung wird um die Rahmenhöhe überschätzt.**  
   Sowohl [sonde-messung.html:1233](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:1233) als auch [formfaktor.html:627](C:/Users/phili/Projekte/Nakama-Design/werkzeug/formfaktor.html:627) benutzen `offsetHeight`. Darin stecken zwei Pixel Rahmen. Gemessen:

   | Fall | gemeldet | tatsächlich zeichnbarer Innenraum |
   |---|---:|---:|
   | A/B | 3 px | 1 px |
   | C | 73 px | 71 px |
   | D | 68 px | 66 px |
   | E | 187 px | 185 px |
   | Formfaktor Active | 7 px | 5 px |

   Die qualitative Aussage „unbrauchbar“ bleibt richtig, wird dadurch sogar stärker. Die dokumentierten `px/dB`-Zahlen sind aber zu hoch und sollten aus `clientHeight` beziehungsweise dem SVG-Innenraum kommen.

::code-comment{title="[P2] px-pro-dB enthält den Rahmen" body="offsetHeight zählt die beiden Rahmenpixel mit, obwohl dort keine Kurve gezeichnet wird. Bei 7 px Gesamtmaß sind dadurch nur 5 px nutzbar; bei A und B bleibt statt 3 px sogar nur 1 px Innenraum." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/formfaktor.html" start=624 end=629 priority=2}

8. **[P2] Das Standband von `formfaktor.html` existiert nur im DOM, ist aber vollständig verdeckt.**  
   `#band` ist fixed, `#schirm` liegt absolut über der Seite, und [formfaktor.html:746](C:/Users/phili/Projekte/Nakama-Design/werkzeug/formfaktor.html:746) reserviert nur die Höhe des 1:1-Bandes. Das 112,75 px hohe Standband darunter wird an jedem Stichpunkt von `#band` oder `#schirm` überdeckt. Die Blattprüfung kontrolliert in [pruefen.mjs:78](C:/Users/phili/Projekte/Nakama-Design/werkzeug/pruefung/pruefen.mjs:78) ausschließlich die Existenz des Elements, nicht seine Sichtbarkeit.

::code-comment{title="[P2] Standband wird vom Messschirm überdeckt" body="Der Bildschirm beginnt nur unterhalb des fixed 1:1-Bandes; die davor im Dokument stehende Statusfläche wird nicht eingerechnet und ist vollständig unsichtbar. Die Prüfung sollte zusätzlich Sichtbarkeit und Überdeckung testen." file="C:/Users/phili/Projekte/Nakama-Design/werkzeug/formfaktor.html" start=735 end=746 priority=2}

9. **[P2] Die Passive-Kachel widerlegt den Kommentar über ihren Engpass.**  
   Der Code behauptet an [formfaktor.html:479](C:/Users/phili/Projekte/Nakama-Design/werkzeug/formfaktor.html:479), bei 260×84 entscheide die Breite. Die reale Messung findet jedoch ausschließlich vertikalen Überlauf: rund 23–25 px Höhe, 0 px Breite. Die Kachelprobe prüft nur, ob „Backing Vox“ und „POST“ vorkommen, nicht Richtung oder Sichtbarkeit. Der Befund ist also: Drei gestapelte Informationszeilen passen in die bestätigte Höhe nicht.

10. **[P3] Zwei kleine Zustandskanten sind sichtbar falsch.**

   - [dbText()](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:415) formatiert `0` und `-0` als `−0.0 dB`. Das wird spätestens nach Neutralise sichtbar.
   - Nach Apply setzt [streifen()](C:/Users/phili/Projekte/Nakama-Design/werkzeug/sonde-messung.html:587) den Zustand auf `apply`, obwohl nur die Tabs `edit` und `audition` existieren. Danach ist kein Tab aktiv.

## Was der Stand belastbar beweist

- A und B sind ausgeschlossen: praktisch keine Kurvenfläche und 14 px interner Überlauf.
- C und D bleiben als Messkandidaten übrig, nicht als gewählte Varianten.
- E ist nur die Obergrenze ohne sichtbare Bandbedienung.
- Mit echten Greif-Affordances sind die Bandzeilen noch nicht vermessen; der Plan dokumentiert diese offene Annahme korrekt.
- Die Passive-Kachel benötigt eine kompaktere Disclosure-Mechanik.
- Aufgabenliste, `/interview struktur` und Designvertrag sind weiterhin offen. Daher ist bewusst noch kein Wireframe oder Prototyp freigabefähig.

## Verifikation

Ausgeführt auf sauberem `HEAD 86f04c1`:

- `pruefen.mjs`: 10/10 Blätter technisch heil.
- Gegenprobe: 3/3 künstliche Fehler erkannt.
- `sondenprobe.mjs`: 12 Proben melden „ALLES SAUBER“ – die oben beschriebenen falschen Grünfälle bleiben jedoch bestehen.
- Reale Playwright-Interaktionen für Hold, Discard, Bypass, Active-/Passive-Toggles und Export.
- Keine Dateien verändert.

Empfohlene Reihenfolge: zuerst Export und falsche Gates reparieren, dann Parametergrenzen und Lesbarkeitsheuristik als Annahmen klären, anschließend Aufgabeninterview und erst danach C/D als echte Disclosure-Varianten entwerfen.
