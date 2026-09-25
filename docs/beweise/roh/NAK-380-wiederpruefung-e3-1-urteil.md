URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.

Geprüft: ausschließlich der gebundene Fixdiff, statisch gegen Produktcode, Matrixfassung §31, Gate und die vier Rotbelege einschließlich aller 25 Mutationsprotokolle:
- D1: ja — eq-copilot/plugin/tests/GoldenTestMain.cpp:793-816 prüft jedes Feld und jedes Band auf NaN beziehungsweise Endlichkeit; die drei Feldmutationen treffen jeweils ausschließlich die betreffende Feldprüfung
- D2: ja — GoldenTestMain.cpp:749-787 und :839-902 tragen beide vollständigen Läufe und die formelbasierten Segment-, Teilblock- und Restprüfungen vor und nach Reset
- D3: ja — GoldenTestMain.cpp:818-820 und :880-895 prüfen false ohne Teilblock und true ab 9216 Samples; die Mutation „immer wahr“ trifft beide False-Prüfungen
- D4: ja — tools/eq-copilot/pruefe_v3_vertrag.py:2179-2277 und :2487-2520 erfassen alle sieben vorhandenen Paare einschließlich Klassenkonstanten; fehlende, mehrdeutige, unlesbare und abweichende Werte werden abgewiesen
- Z1: ja — GoldenTestMain.cpp:554-641 und eq-copilot/plugin/tests/Nak380Pruefsignale.h:202-228 tragen die hergeleiteten Spannen 5/8/13 dB und vollständigen Charakter-/Werkzeugtexte; 6→4 trifft M36a, 10→7 ausschließlich M36b
- Z2: ja — GoldenTestMain.cpp:653-679 und :699-727 verriegeln positive Segmentzahlen vor Gleichheitsprüfung und Abdeckungsnachbau
- K1: ja — eq-copilot/plugin/src/AnalyseEngine.h:130-131 nennt Acht-Segment-Teilblockhistogramm und NaN bei weniger als acht aktiven Segmenten
Herleitungen: Segmentformel aus AnalyseEngine.cpp:70-71, :106-109 und :157-159; erster Höhen-Teilblock bei 2048+7·1024=9216; Bandzuordnung und Binzählung ergeben 196/25 Bänder; Bin 128 der Mittenstufe liegt in Band 135, Hann-/PSD-Skalierung ergibt gerundet −30 dB und Spanne gleich Modulationstiefe
Rotmutationen: an den geprüften Eingängen formgültig und an Zusageprüfungen wirksam; insbesondere Restbuchung mit positivem Nenner und geklemmtem Histogrammindex sowie Feldmutationen ohne Arrayüberschreitung oder UB
Brüche: keine festgestellt — exakt vier erlaubte Dateien geändert; AnalyseEngine.cpp, Diagnose.cpp und metriken-v1.json zwischen VORHER/ZIEL blobidentisch; vollständiger A2 einschließlich Stereoblock erhalten, Umgebungsvariable nur zusätzliche Ausgabe; neue Engines auf dem Heap; C++ im Arbeitsbaum CRLF, Python LF; kein eingeführter Bruch von M-21 bis M-41, Gate oder CLAUDE-Invarianten.

Nicht geprüft: keine Builds, Testläufe, ausgeführten Mutationen, Kanon- oder FL-Läufe; dokumentierte Laufresultate nur gelesen und statisch nachvollzogen, ausgeschlossene Bereiche nicht neu bewertet.

HEAD Beginn: fb591eed34dd944608c027a4d4ff3202c2b3ad4f
HEAD Ende: fb591eed34dd944608c027a4d4ff3202c2b3ad4f
Arbeitsbaum unverändert: ausschließlich die drei benannten fremden untracked Einträge

FERTIG Wiederprüfung 1 Etappe 3, PASS, 0 offen, 0 Brüche
