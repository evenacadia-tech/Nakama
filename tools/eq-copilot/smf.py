#!/usr/bin/env python3
"""Minimaler Standard-MIDI-File-Schreiber (Format 0 und 1).

Bewusst ohne mido/midiutil: keine Abhaengigkeit, volle Kontrolle ueber Ticks.
Division = FLs PPQ (96) -> Noten landen in FL exakt auf dem Raster.
"""
import struct

PPQ = 96
BEAT = PPQ
BAR = PPQ * 4


# Meta-Text im MIDI ist per Standard ASCII. UTF-8-Umlaute kommen in FL als
# "GerÃ¼st" an (mit mido gegengeprueft) -> vor dem Schreiben umschreiben.
_UML = {"ä": "ae", "ö": "oe", "ü": "ue", "Ä": "Ae", "Ö": "Oe", "Ü": "Ue",
        "ß": "ss", "é": "e", "è": "e", "–": "-", "—": "-", "„": '"', "“": '"'}


def ascii_text(s):
    for a, b in _UML.items():
        s = s.replace(a, b)
    return s.encode("ascii", "replace")


def vlq(n):
    """Variable-length quantity (MIDI-Delta-Zeiten)."""
    if n < 0:
        raise ValueError("negative Delta-Zeit")
    out = bytearray([n & 0x7F])
    n >>= 7
    while n:
        out.insert(0, (n & 0x7F) | 0x80)
        n >>= 7
    return bytes(out)


class Track:
    """Eine MIDI-Spur. Noten werden als (tick, key, len_ticks, vel) gesammelt."""

    def __init__(self, name="", channel=0, program=None):
        self.name = name
        self.channel = channel
        self.program = program
        self.notes = []
        self.texts = []  # (tick, text) -> Marker im Piano Roll
        self.ccs = []  # (tick, controller, value) -> Continuous Controller

    def cc(self, tick, controller, value):
        """Continuous-Controller-Ereignis (z.B. CC1 = Modwheel/Ausdruck).

        ⚠️ FL uebernimmt CC-Daten NICHT beim Ziehen einer .mid in die Piano Roll —
        laut Handbuch ("Import MIDI Data") kommen dort nur die Noten an, CC muss
        separat ueber den Event Editor in den Zielparameter geladen werden.
        Deshalb gehoeren CC-Kurven in EIGENE Dateien, nie still in eine Notendatei:
        sonst sieht es beim Import so aus, als waere die Dynamik verschwunden.
        """
        if not (0 <= controller <= 127):
            raise ValueError(f"Controller {controller} ausserhalb 0..127")
        if not (0 <= value <= 127):
            raise ValueError(f"CC-Wert {value} ausserhalb 0..127")
        self.ccs.append((int(round(tick)), int(controller), int(value)))
        return self

    def note(self, tick, key, length, vel=100):
        if not (0 <= key <= 127):
            raise ValueError(f"Note {key} ausserhalb 0..127")
        if not (1 <= vel <= 127):
            raise ValueError(f"Velocity {vel} ausserhalb 1..127")
        if length <= 0:
            raise ValueError("Notenlaenge muss > 0 sein")
        self.notes.append((int(round(tick)), int(key), int(round(length)), int(vel)))
        return self

    def marker(self, tick, text):
        self.texts.append((int(round(tick)), text))
        return self

    # --- Komfort: Takt/Beat statt Ticks (1-basiert, wie FL sie anzeigt) ---
    def at(self, bar, beat, key, len_beats, vel=100):
        return self.note((bar - 1) * BAR + (beat - 1) * BEAT, key, len_beats * BEAT, vel)

    def chord(self, bar, beat, keys, len_beats, vel=100, spread=0):
        """Akkord; spread = Ticks Versatz pro Stimme (sanftes Anrollen)."""
        for i, k in enumerate(sorted(keys)):
            self.note((bar - 1) * BAR + (beat - 1) * BEAT + i * spread, k,
                      len_beats * BEAT - i * spread, vel)
        return self

    def _events(self):
        """(tick, order, bytes) — order sorgt fuer note-off vor note-on bei gleichem Tick."""
        ev = []
        if self.name:
            nm = ascii_text(self.name)
            ev.append((0, 0, b"\xff\x03" + vlq(len(nm)) + nm))
        if self.program is not None:
            ev.append((0, 1, bytes([0xC0 | self.channel, self.program])))
        for tick, text in self.texts:
            t = ascii_text(text)
            ev.append((tick, 1, b"\xff\x06" + vlq(len(t)) + t))
        # order=1: ein CC am selben Tick muss VOR dem note-on (order=3) stehen,
        # sonst gilt der neue Wert erst ab der naechsten Note — bei einem Swell
        # auf den Einsatz ist genau das der hoerbare Unterschied.
        for tick, ctrl, val in self.ccs:
            ev.append((tick, 1, bytes([0xB0 | self.channel, ctrl, val])))
        for tick, key, length, vel in self.notes:
            ev.append((tick + length, 2, bytes([0x80 | self.channel, key, 64])))
            ev.append((tick, 3, bytes([0x90 | self.channel, key, vel])))
        return sorted(ev, key=lambda x: (x[0], x[1]))

    def chunk(self, extra_head=b""):
        data = bytearray(extra_head)
        last = 0
        for tick, _o, payload in self._events():
            data += vlq(tick - last) + payload
            last = tick
        data += vlq(0) + b"\xff\x2f\x00"
        return b"MTrk" + struct.pack(">I", len(data)) + bytes(data)


def tempo_meta(bpm):
    us = int(round(60_000_000 / bpm))
    return b"\x00\xff\x51\x03" + us.to_bytes(3, "big")


def timesig_meta(num=4, den=4):
    dd = {1: 0, 2: 1, 4: 2, 8: 3, 16: 4}[den]
    return bytes([0x00, 0xFF, 0x58, 0x04, num, dd, 24, 8])


def keysig_meta(flats_sharps, minor):
    """flats_sharps: -7..+7 (negativ = b). B-Moll-Beispiel: Bb-Moll = -5, minor=1."""
    return bytes([0x00, 0xFF, 0x59, 0x02, flats_sharps & 0xFF, 1 if minor else 0])


def write(path, tracks, bpm=120.0, num=4, den=4, key=(-5, True)):
    """Schreibt SMF Format 1 (Tempo-Spur + Inhaltsspuren)."""
    head = tempo_meta(bpm) + timesig_meta(num, den) + keysig_meta(key[0], key[1])
    meta = Track(name="")
    body = b"".join(t.chunk() for t in tracks)
    mthd = b"MThd" + struct.pack(">IHHH", 6, 1, len(tracks) + 1, PPQ)
    with open(path, "wb") as f:
        f.write(mthd + meta.chunk(extra_head=head) + body)
    return path
