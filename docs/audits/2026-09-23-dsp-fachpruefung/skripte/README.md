# Skripte der DSP-Fachprüfung vom 23.09.2026

Nachbauten der Produktformeln in Python, mit denen die Zahlen in
[`../BEFUNDE.md`](../BEFUNDE.md) entstanden sind. Sie rechnen **nicht** am
Plugin-Binary, sondern an Formeln, die aus dem Quellcode abgeschrieben sind
(Quellzeilen jeweils im Skriptkopf). Jede tragende Zahl wurde von einem
zweiten Agenten unabhängig nachgerechnet; die Ordner trennen Erst- und
Gegenrechnung.

| Ordner | Inhalt |
|---|---|
| [`nachrechnung/`](nachrechnung/) | Erstrechnung: Cramping (r1), Decramping nach Vicanek und Orfanidis (r2), SVF gleich RBJ (r3), tiefe Frequenzen float/double (r4), Zeitvarianz von Rampe, SVF und Crossfade (r5 bis r5e); `nk.py` ist der Formelnachbau, `t0_selbsttest.py` prüft ihn |
| [`gegenpruefung-eq-1/`](gegenpruefung-eq-1/) | Gegenprüfung Delta-Hören (v1), Advisor-Güte (v2), Tail (v3), Attack-Boden (v4), Koeffizientenrampe über 1 116 Grenzfälle (v5 bis v5e) |
| [`gegenpruefung-eq-2/`](gegenpruefung-eq-2/) | Gegenprüfung Mono-Bass (v1), Detektor (v2), Güte-Semantik (v3), Mix (v4), Auto-Gain-Gitter (v5), Übergangszeiten und Kennlinie (v6); `v2lib.py` ist der Formelnachbau |
| [`gegenpruefung-statistik/`](gegenpruefung-statistik/) | Gegenprüfung Experiment-Bootstrap (b1), Welch-Streuung (b5), Kohärenz (b10), Ereignisdetektor (b11); Ausgaben als `.out` |
| [`gegenpruefung-pegel/`](gegenpruefung-pegel/) | Gegenprüfung Dichten gegen Bandleistung und Auflösung (b3, b6), K-Gewichtung (b7), LRA (b8), Vergleichspegel (b9), True Peak (b12); ohne gespeicherte Ausgaben |
| [`eigen/`](eigen/) | Gegenprobe der Session: SVF-Bell bei schneller Güte-Absenkung |

Ausführen im jeweiligen Ordner mit `py -3.13 <skript>.py`. Benötigt werden
numpy und scipy, für `nachrechnung/` zusätzlich numba, mpmath und matplotlib.
Ausgaben landen im aktuellen Ordner. `gegenpruefung-eq-1/v5_auswertung.py`
liest die JSON-Datei, die `v5_rampe.py` schreibt; die JSON-Dateien selbst und
die PDF-Quellen (Vicanek, Orfanidis, BS.1770-5, EBU Tech 3341/3342) sind
wegen Größe bzw. Urheberrecht nicht eingecheckt.
