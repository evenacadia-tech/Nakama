#!/usr/bin/env python3
"""EQ-Copilot Kalibrier-Teststück 01 — 8 Instrument-MIDIs, deterministisch.

Zweck (User-Protokoll 2026-08-15): dasselbe Notenmaterial wird vom User in
wechselnde Instrumente geladen und OHNE Effekte gerendert; die Renderdatei
laeuft dann durch tools/analyze-track.py UND durchs Plugin — Abweichungen
zwischen beiden sind Kalibrier-Futter (M3-KERN-BEFUND: „Kalibration gegen
echte Klaenge statt synthetischer Fixtures").

Musik: even-Geruest (B-Moll aeolisch, 120 BPM, Bbm|Gbmaj7|Ab|Ab|Bbm|Gbmaj7|
Ab|Ebm), 24 Takte in drei Teilen — T1–8 ruhig (Klavier 1, Streicher, Bass),
T9–16 voll (alles), T17–24 Abbau; Schlusstakt kehrt zur Tonika zurueck.
Die Drei-Teilung ist Absicht: sie gibt dem Zonen-ZEITVERLAUF des Plugins
(m4: „in NN % der Musikzeit") an echter Musik etwas zu messen.
Bewusst mechanisch aufs Raster gesetzt: Messstueck, kein Kunststueck.

Erwartungen fuer den Vergleich stehen in eq-copilot/kalibration/
KALIBRIER-PROTOKOLL.md — u.a. die Tonika-Ballung um A#3/A#4 (116/233 Hz:
Klavier-LH + Gitarre + Bass-Obertoene treffen genau das Golden-Fixture-Band).

Aufruf:  py -3.13 tools/eq-copilot/erzeuge_testsong.py
Pruefen: py -3.13 tools/eq-copilot/verify_testsong.py
"""
from __future__ import annotations

import sys
from pathlib import Path

WURZEL = Path(__file__).resolve().parents[2]
# smf.py liegt seit dem Workspace-Umzug (18.08.2026) als Kopie direkt hier —
# keine Abhängigkeit mehr zu FL-Studio\tools\midi-ideen.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import smf  # noqa: E402  (eigener Schreiber, PPQ 96 = FL-Raster)

ZIEL = WURZEL / "eq-copilot" / "kalibration" / "testsong-01"
BPM = 120.0

# B-Moll aeolisch als Pitch-Klassen (A# C C# D# F F# G#).
SCALE = {10, 0, 1, 3, 5, 6, 8}
# GM-Schlagzeug-Vokabular (Kick/Snare/Hat zu/Hat offen/Crash) — Drums werden
# gegen DIESES Vokabular geprueft, nie gegen die Tonleiter (lauf62-Lehre 2).
KIT = {36, 38, 42, 46, 49}

# ── Harmonie-Zyklus (8 Takte); Index 0..7 ────────────────────────────────────
# MIDI-Zahlen; FL zeigt MIDI 60 als C5 (46 = A#3 = 116,5 Hz — die tiefen
# Klavier-Akkorde des User-Stils liegen genau dort).
BASS = [34, 30, 32, 32, 34, 30, 32, 39]                     # Wurzeln A#2 F#2 G#2 … D#3
KLAV_LH = [46, 42, 44, 44, 46, 42, 44, 39]                  # Klavier linke Hand
KLAV_MITTE = [
    [58, 61, 65],        # Bbm   (A#4 C#5 F5)
    [54, 58, 61, 65],    # Gbmaj7 (F#4 A#4 C#5 + maj7-F)
    [56, 60, 63],        # Ab    (G#4 C5 D#5)
    [56, 60, 63],        # Ab
    [58, 61, 65],        # Bbm
    [54, 58, 61, 65],    # Gbmaj7
    [56, 60, 63],        # Ab
    [51, 54, 58],        # Ebm   (D#4 F#4 A#4)
]
MELODIE = [70, 70, 65, 63, 70, 68, 65, 61]                  # eveng-Stufen 1 1 5 4 1 b7 5 b3
STREICHER = [[53, 58], [54, 61], [56, 63], [56, 63],
             [53, 58], [54, 61], [56, 63], [51, 58]]
EPAD = [(70, 77), (66, 73), (68, 75), (68, 75),
        (70, 77), (66, 73), (68, 75), (63, 70)]             # root+24 / root+31
GIT_MUSTER = [0, 7, 12, 7]                                  # Achtel: r, Quinte, Oktave, Quinte

SCHLUSS = {"lh": 46, "mitte": [58, 61, 65], "melodie": 70,
           "streicher": [53, 58], "bass": 34}               # T24 = Tonika Bbm


def idx(takt: int) -> int:
    return (takt - 1) % 8


def vel(v: int) -> int:
    """FL-Velocity-Klemme (0..128 in FL, 1..127 in MIDI) — klemmen, nie skalieren."""
    return max(1, min(127, v))


def baue() -> dict[str, smf.Track]:
    t_k1 = smf.Track("Klavier 1 Haupt")
    t_k2 = smf.Track("Klavier 2 Arpeggio")
    t_str = smf.Track("Pad Streicher")
    t_chor = smf.Track("Pad Chor")
    t_epad = smf.Track("Pad Elektronisch")
    t_bass = smf.Track("Bass")
    t_drum = smf.Track("Schlagzeug GM", channel=9)
    t_git = smf.Track("Gitarre Motiv")

    for takt in range(1, 25):
        i = idx(takt)
        schluss = takt == 24
        lh = SCHLUSS["lh"] if schluss else KLAV_LH[i]
        mitte = SCHLUSS["mitte"] if schluss else KLAV_MITTE[i]
        streich = SCHLUSS["streicher"] if schluss else STREICHER[i]
        basston = SCHLUSS["bass"] if schluss else BASS[i]
        ruhig = takt <= 8
        voll = 9 <= takt <= 16

        # ── Klavier 1: LH-Halbe + Mitte-Akkorde; Melodie ab T9 ──
        # Sobald die Melodie spielt, laesst der Akkord ihren Ton weg —
        # Halteakkorde duerfen Melodietoene nicht doppeln (lauf62-Lehre 3;
        # sonst Ueberlappung gleicher Tonhoehe, in MIDI nicht eindeutig).
        vLH = 72 if ruhig else 88 if voll else 70
        vMitte = 68 if ruhig else 84 if voll else 64
        if schluss:
            t_k1.at(takt, 1, lh, 4, vel(70))
            t_k1.chord(takt, 1, [k for k in mitte if k != SCHLUSS["melodie"]],
                       4, vel(66))
            t_k1.at(takt, 1, SCHLUSS["melodie"], 4, vel(84))
        else:
            spiel_mitte = ([k for k in mitte if k != MELODIE[i]]
                           if takt >= 9 else mitte)
            t_k1.at(takt, 1, lh, 2, vel(vLH))
            t_k1.at(takt, 3, lh, 2, vel(vLH - 6))
            t_k1.chord(takt, 1, spiel_mitte, 2, vel(vMitte))
            t_k1.chord(takt, 3, spiel_mitte, 2, vel(vMitte - 8))
            if takt >= 9:
                t_k1.at(takt, 1, MELODIE[i], 4, vel(102 if voll else 78))

        # ── Klavier 2: Achtel-Arpeggio nur im vollen Teil ──
        if voll:
            toene = [k + 12 for k in (KLAV_MITTE[i][:3])]
            folge = [0, 1, 2, 1, 0, 1, 2, 1]
            for n, stufe in enumerate(folge):
                betont = n in (0, 4)
                t_k2.note((takt - 1) * smf.BAR + n * (smf.BEAT // 2),
                          toene[stufe], int(smf.BEAT * 0.45),
                          vel(80 if betont else 68))

        # ── Streicher: ganze Noten durchgehend ──
        vStr = 58 if ruhig else 74 if voll else 64
        t_str.chord(takt, 1, streich, 4, vel(66 if schluss else vStr))

        # ── Chor: ab T9, gleiche Mitte wie Klavier (bewusst — Masking-Futter) ──
        if takt >= 9:
            vCh = 72 if voll else 60
            t_chor.chord(takt, 1, mitte[:3], 4, vel(66 if schluss else vCh))

        # ── E-Pad: nur im vollen Teil, hohe Doppeltoene ──
        if voll:
            a, b = EPAD[i]
            t_epad.chord(takt, 1, [a, b], 4, vel(56))

        # ── Bass ──
        if ruhig or takt >= 17:
            if schluss:
                t_bass.at(takt, 1, basston, 4, vel(78))
            else:
                v = 84 if ruhig else 80
                t_bass.at(takt, 1, basston, 2, vel(v))
                t_bass.at(takt, 3, basston, 2, vel(v - 6))
        else:
            for beat in (1, 2, 3):
                t_bass.at(takt, beat, basston, 1, vel(96 if beat == 1 else 88))
            t_bass.at(takt, 4, basston + 7, 1, vel(84))

        # ── Schlagzeug (GM, sparsam) nur im vollen Teil ──
        if voll:
            for beat in (1, 3):
                t_drum.at(takt, beat, 36, 0.25, vel(100))
            for beat in (2, 4):
                t_drum.at(takt, beat, 38, 0.25, vel(96))
            for n in range(8):
                t_drum.note((takt - 1) * smf.BAR + n * (smf.BEAT // 2),
                            42, int(smf.BEAT * 0.25),
                            vel(62 if n % 2 == 0 else 52))
            if takt == 9:
                t_drum.at(takt, 1, 49, 2, vel(92))
            if takt == 16:
                t_drum.note((takt - 1) * smf.BAR + 3 * smf.BEAT + smf.BEAT // 2,
                            46, int(smf.BEAT * 0.5), vel(70))

        # ── Gitarre: Achtel-Ostinato nur im vollen Teil ──
        if voll:
            r = KLAV_LH[i]
            for n in range(8):
                stufe = GIT_MUSTER[n % 4]
                t_git.note((takt - 1) * smf.BAR + n * (smf.BEAT // 2),
                           r + stufe, int(smf.BEAT * 0.45),
                           vel(70 if n in (0, 4) else 62))

    return {
        "01_Klavier-1_Haupt.mid": t_k1,
        "02_Klavier-2_Arpeggio.mid": t_k2,
        "03_Pad-Streicher.mid": t_str,
        "04_Pad-Chor.mid": t_chor,
        "05_Pad-Elektronisch.mid": t_epad,
        "06_Bass.mid": t_bass,
        "07_Schlagzeug_GM.mid": t_drum,
        "08_Gitarre_Motiv.mid": t_git,
    }


def audit(tracks: dict[str, smf.Track]) -> None:
    """Tonleiter-/Kit-/Overlap-Audit — bricht kontrolliert ab (generate.py-Muster)."""
    for name, tr in tracks.items():
        drums = "Schlagzeug" in name
        belegt: dict[int, list[tuple[int, int]]] = {}
        for tick, key, laenge, v in tr.notes:
            if not (1 <= v <= 127):
                raise SystemExit(f"{name}: Velocity {v} ausserhalb 1..127")
            if drums:
                if key not in KIT:
                    raise SystemExit(f"{name}: {key} nicht im Kit-Vokabular {sorted(KIT)}")
            elif key % 12 not in SCALE:
                raise SystemExit(f"{name}: MIDI {key} nicht in B-Moll aeolisch")
            belegt.setdefault(key, []).append((tick, tick + laenge))
        for key, spannen in belegt.items():
            spannen.sort()
            for (a1, e1), (a2, _e2) in zip(spannen, spannen[1:]):
                if a2 < e1:
                    raise SystemExit(f"{name}: MIDI {key} ueberlappt sich bei Tick {a2}")


LIES_MICH = """# Teststueck 01 — Kalibrier-Runden fuer den EQ-Copilot

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
   `eq-copilot\\kalibration\\renders\\runde-01-<kurzname>.wav`
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
"""


def main() -> int:
    ZIEL.mkdir(parents=True, exist_ok=True)
    tracks = baue()
    audit(tracks)
    for name, tr in tracks.items():
        smf.write(str(ZIEL / name), [tr], bpm=BPM, key=(-5, True))
        print(f"  {name}  ({len(tr.notes)} Noten)")
    (ZIEL / "00_LIES-MICH.md").write_text(LIES_MICH, encoding="utf-8")
    print(f"Teststueck 01 nach {ZIEL}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
