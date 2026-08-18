# Teststueck 01 — Kalibrier-Runden fuer den EQ-Copilot

**Was das ist:** 8 MIDI-Dateien, EIN einfaches Stueck (B-Moll, 120 BPM,
24 Takte ≈ 48 s). Du laedst sie in Instrumente deiner Wahl, renderst OHNE
Effekte, und wir vergleichen: meine Offline-Analyse ↔ die Befundkarten des
Plugins. Jede Runde mit anderen Instrumenten/Effekten ist neues
Kalibrier-Futter.

**Aufbau (Absicht, nicht Zufall):** Takt 1–8 ruhig (Klavier 1 + Streicher +
Bass) · Takt 9–16 voll (alles spielt, Schlagzeug sparsam) · Takt 17–24 Abbau,
Schluss auf der Tonika. So kann das Plugin zeigen, ob es „nur im vollen Teil"
von „durchgehend" unterscheidet.

## Tu (eine Runde)

1. Neues FL-Projekt (gern die Startrampe Neutral), **120 BPM**.
2. Jede Datei in die Piano Roll ihres Instruments ziehen (Reihenfolge unten).
   Alle Kanaele an den Master, **Fader auf 0 dB lassen, nicht mischen**.
3. **KEINE Effekt-Plugins** — auch nicht auf dem Master (Fruity Limiter im
   Master-Insert entfernen, falls die Vorlage einen traegt).
4. EQ-Copilot als einzigen Master-Effekt laden, Song 1–2x durchspielen
   (bis „KURVE STEHT"), **Festhalten** druecken (schreibt den Snapshot).
5. Rendern als WAV (44,1 kHz, 24 bit) nach:
   `eq-copilot\kalibration\renders\runde-01-<kurzname>.wav`
   (z. B. `runde-01-noire-omnia.wav`) — und mir sagen, welche Instrumente
   du benutzt hast.

## Die 8 Dateien

| Datei | Rolle | Vorschlag (dein Bestand) |
|---|---|---|
| 01_Klavier-1_Haupt.mid | Hauptklavier: Akkorde + Melodie | Virtual Pianist SCORE oder Noire |
| 02_Klavier-2_Arpeggio.mid | zweites Klavier, Achtel oben | Grand Piano / Una Corda |
| 03_Pad-Streicher.mid | Streicher-Flaeche | UVI Augmented Orchestra / Rise and Hit |
| 04_Pad-Chor.mid | Chor-Flaeche | Choir Omnia / Vocal Colors |
| 05_Pad-Elektronisch.mid | elektronische Flaeche | Straylight / Pharlight / Schema |
| 06_Bass.mid | Bass, Wurzeln | Prime Bass |
| 07_Schlagzeug_GM.mid | Schlagzeug (GM: Kick 36, Snare 38, Hats 42/46, Crash 49) | Drum Lab / Butch Vig |
| 08_Gitarre_Motiv.mid | Hintergrund-Achtel | Session Guitarist / Electric Neon |

**Warum ohne Effekte:** Runde 1 misst die ROHEN Instrumentklaenge — nur so
sehen wir, was das Plugin am puren Klang findet. Effekte kommen in spaeteren
Runden gezielt dazu (dann vergleichen wir vorher/nachher).

**Hoeren:** Das Stueck ist absichtlich mechanisch (Messstueck). Es soll
ordentlich klingen, nicht beruehren.
