# NAK-380 Abschlussprüfung T2 — Validierung der Befunde L1, L2, H1, H2 an der Quelle (Opus 5.5, Effort xhigh, lesend)

Worker `9a55c88a` (`nakama-nak380-3e5d366-validt2`, `dontAsk`, Werkzeuge Read, Grep, Glob, Bash), 26.09.2026, gestartet 13:16 Uhr, beendet 13:25 Uhr (Beobachter „beendet“ vor 13:25:08); HEAD vor und nach dem Lauf `3e5d3660`. Auftrag als Positionsargument (die vier Befunde wörtlich aus `docs/beweise/roh/NAK-380-abschlusspruefung-t2-urteil.md`; je Befund Zitat, Matrixzeile, Gate-Satz, Urteil BESTÄTIGT/PRÄZISIERT/WIDERLEGT, Erreichbarkeit; für L1 die gemessenen Hostraten, für H1 die Zusagenlage von T-380-1, für H2 R-380-7). Einordnung durch den Dirigenten in `docs/beweise/NAK-380.md` §62. Wortlaut des Validierers unverändert (aus dem Sitzungsprotokoll `9a55c88a-183d-4a57-a3e8-f54f17ea07be.jsonl`, letzter Assistententext):

---
`★ Insight ─────────────────────────────────────`
- In `fuelleLive` sieht ein Feinband über der Nyquist-Kappe genauso aus wie ein Feinband ohne FFT-Bin: `Zeit.h` setzt `bandVon = bandBis = 0`, deshalb bleibt `liveAkku[b].n` 0. Die Ergänzung nach T-380-1 greift darum in beiden Fällen, obwohl der Bauplan sie nur für Bänder ohne Bin begründet.
- Das arithmetische und das breitengewichtete Mittel sind gleich, wenn alle belegten Dichten gleich sind (M-02) oder nur ein Feinband belegt ist (M-03). Die beiden Matrixzeilen können den Unterschied also grundsätzlich nicht messen.
`─────────────────────────────────────────────────`

**L1 – PRÄZISIERT.** Der Mechanismus ist an der Quelle bestätigt. `Zeit.h:122,133-137` sperrt jedes Feinband, dessen Oberkante über `min(18 kHz, 0,95·fs/2)` liegt (`bandVon = bandBis = 0`). `Spektrum.h:494-497` überspringt es, `liveAkku[b].n` bleibt 0. `Frame.h:353` behandelt es deshalb wie ein Band ohne Bin, und `Frame.h:371-376` ergänzt bis `evidenzKante(liveBisExkl(g))`, also über die Kappe hinaus. `FeatureEngine.h:642` nimmt jede Rate in (0, 768 kHz] an. `fensterPunkte` (`FeatureEngine.h:164-176`) gibt jeder Rate unter 48 kHz die Basislängen 16 384/4 096; unter 44,1 kHz gibt es keine Sonderbehandlung.

Präzisierungen:
- **Schwelle:** Sie liegt bei fs < 37 809 Hz (0,475·fs < 17 959,39 Hz), nicht bei „etwa 37,9 kHz“. Bei 37 895 Hz fällt nur 0,95·fs/2 unter 18 kHz; zwischen beiden Raten schneidet die Kappe kein Band.
- **Wer schweigt:** Die Gate-Zeile (`NAK-380.md:6`) und die Matrix schweigen. Die Nyquist-Kappe steht nur in M-43 und M-114, beide erst ab 44,1 kHz (§6.6 `:1068-1069`). Die Fassungen §50.2, §55.2 und §58.2 ändern daran nichts. R-380-5 begrenzt seinen Geltungsbereich ausdrücklich auf „44,1 bis 192 kHz“ (`:143`). T-380-1 begründet die Ergänzung nur mit Bändern ohne Bin bei 44,1 und 48 kHz (`:1245-1250`).
- **Wer nicht schweigt:** Die Grenze-Zeile des Kopfs („Nyquist gekappt“, `:18`), die CLAUDE.md-Invariante zur NaN-Ehrlichkeit und der Kappenkommentar `FeatureEngine.h:571-573` („darueber bleibt es leer, statt eine fortgeschriebene Randevidenz zu behaupten“). Diesem Satz widerspricht die Ergänzung bei den betroffenen Raten.
- **Größe** (von Hand gerechnet aus den Kanten 1000·2^((b−121)/24) und `BandGridZahlen.h:150-162`): Bei 32 kHz ist in Gruppe 62 nur Band 214 gemessen; ergänzt wird 15 102 bis 16 000 Hz, davon 800 Hz über der Kappe von 15 200 Hz. Bei 22,05 kHz ergänzt Gruppe 58 bis 10 679 Hz, die Kappe liegt bei 10 474 Hz.
- **Bestehender Test:** Der B5-Fall bei 22,05 kHz (`AnalysisGoldenTestMain.cpp:5102-5109`) prüft nur die Kappenauskunft, nicht die Livegruppen.

Erreichbarkeit: Im Code ist der Fall erreichbar, im Host unbelegt. Als gemessen ist nur 44 100 Hz ausgewiesen (`host-capabilities-fl-v1.json:8`, `termin-a/aux-spike-*.json:7`, `termin-b/host-probe-*.json:16`). Ob FL Raten unter 37,8 kHz anbietet, ist nicht belegt (`docs/plan/fragen.json`, U47 „warum“). Das Produkt will die Messung dort aber ausdrücklich („Messung ja, EQ nein“, `docs/ZIELBILD.md:314`, U47). Beide Prozessoren bereiten die FeatureEngine mit jeder Rate größer 0 vor (`SondeProcessor.cpp:786-790`, `Analyse.cpp:173-181`).

**L2 – BESTÄTIGT.** `suchgrenzeSatz` hat genau zwei Aufrufer: `Diagnose.cpp:220` in der Resonanzkarte und `PluginEditor.cpp:465`. Der Leertext erscheint nur bei `befunde.empty()` (`PluginEditor.cpp:459-468`). Die Zonenkarten Mittenloch, Mulm, Härte und Höhen-Hype hängen den Satz nie an (etwa `Diagnose.cpp:423-441`, `:445-480`). Eine Liste nur mit Zonenkarten nennt die Suchgrenze also nirgends.
- **Matrix:** M-106 nennt genau diese zwei Orte (`NAK-380.md:1027`; Fassung §58.2 `:2271`, nur K4 bei 44,1 kHz ergänzt). M-107 sagt nur „Satz wie M-106“.
- **Regeln:** R-380-5 verlangt den Satz nur in der Resonanzkarte (`:144-146`), T-380-8 zusätzlich im Leertext (`:1357-1366`).
- **Gate:** DSP-20 lautet „koppeln **oder** … ehrlich melden“. Mit der Kopplung (M-98 bis M-105) ist es erfüllt; wo gemeldet wird, regelt es nicht, schweigt also.

Erreichbarkeit: heute erreichbar im Gen-Editor, sobald die Messung bereit ist und eine Zonenregel ohne Resonanzkarte greift.

**H1 – PRÄZISIERT.**
- **Abweichung:** Der Code ergänzt mit `dichteSumme / belegte` (`Frame.h:376`), also mit dem arithmetischen Mittel. T-380-1 legt P_g = (Σ D_b·w_b / Σ w_b)·W_g fest (`NAK-380.md:1238-1242`).
- **Zusage, kein Vorschlag:** T-380-1 steht unter „Technikentscheide (innerhalb von Gate, Invarianten und R-380-1 bis R-380-8)“ (`:1236`). §8.2 schreibt vor, der Code „bildet P_g nach T-380-1“ (`:1583-1584`). Nur der Begleitsatz „mit der mittleren Dichte“ (`:1248-1249`) und der Codekommentar `Frame.h:346-347` sind mehrdeutig.
- **Matrix:** Keine Zeile unterscheidet die beiden Mittel. M-02 und M-04 nutzen gleiche Erwartungsdichten, M-03 ein belegtes Feinband. In M-05, M-06 und M-08 sind die Gruppen voll belegt.
- **Gate:** Es sagt nur „Livegruppe Σ Dichte·Bandbreite“ und schweigt zur Ergänzung.
- **Präzisierung zum Ort** (48 kHz, von Hand gerechnet, 25 Bänder ohne Bin): Die beiden Mittel weichen in den Gruppen 3, 5–9 und 11 ab (etwa 40–100 Hz, Bassstufe), aber auch in den Gruppen 19, 21–23 und 25 (etwa 200–390 Hz, Hauptstufe). Es sind also nicht nur die untersten Bassgruppen. Die Gruppen 0, 1, 2, 4 und 20 haben nur ein belegtes Band und liefern gleiche Werte.
- **Größe:** höchstens etwa 5 % der ergänzten Dichte (drei belegte Bänder mit Breitenverhältnis bis 2^(3/24)), also ≤ 0,2 dB am Ergänzungsanteil und ≤ 1,5 % von P_g.

Erreichbarkeit: bei jedem Signal mit ungleichen Nachbardichten in diesen Gruppen, also im normalen Betrieb.

**H2 – BESTÄTIGT.** Der Test `telemetrie.rs:1228-1236` prüft `[20_260_904, 20_260_925, 20_260_926]`. Das Register führt zusätzlich die Fassungen 20260927, 20260928 und 20260929, `aktuell` ist 20260929 (`metriken-v1.json:21`, `:422`, `:595`, `:780`). Der Leser lehnt nur Werte unter 1 ab (`telemetrie.rs:633`). Die Wache „Leser lehnt ≠ aktuell ab“ fällt deshalb weiter an 20260904 und hält.
- **M-18:** Die Zeile verlangt wörtlich nur 20260904 „und einer mit der neuen Version“ (`NAK-380.md:881`). Sie ist eine Zeile der Etappe 2, die neue Version war dort 20260925. Die Liste wurde einmal um 20260926 erweitert (Korrektur der Etappe 3, `NAK-380-verlauf.md:2248`). Keine spätere Fassung (§50.2, §55.2, §58.2) ändert M-18.
- **R-380-7** (`:158-164`) verlangt nur, dass alte und neue Evidenz lesbar bleiben, keine Prüfung je Fassung. Ebenso allgemein sagen M-40 und M-96 „alte … bleibt lesbar (A4, B3c)“. Die Gate-Grenze „Altstände laden“ ist durch den Leser erfüllt.
- **Verwandt:** Die frühere Härtung NAK-402 betrifft den C++-Leser (`NAK-380-verlauf.md:1510`).

Erreichbarkeit: kein Produktfehler, weil jede Fassung ab 1 angenommen wird; es fehlt nur Testabdeckung.

HEAD vor und nach der Prüfung `3e5d3660`, identisch. Der Arbeitsbaum ist unverändert (nur der fremde Broker-Pin und die zwei fremden unversionierten Ordner).

FERTIG Validierung T2 NAK-380, 2 bestätigt, 2 präzisiert, 0 widerlegt, HEAD 3e5d366
