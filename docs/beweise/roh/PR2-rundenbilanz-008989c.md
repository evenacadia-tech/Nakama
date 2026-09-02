# Rohausgabe — Prozessbilanz über alle Ticketmanifeste seit S0

**Ticket:** `PR2`, Runde 1 der Nacharbeit nach der Codex-Erstprüfung (Befund D14). 
**Gefahren am:** 2026-09-02 vom Workspace-Root, Stand `008989c`. 
**Werkzeug:** `tools/dirigent/rundenbilanz.py` — unverändert, nur aufgerufen. 
**Auswertung:** `docs/beweise/PR2.md` §12.2, Befund D14.

Wie Basis und Ende je Ticket bestimmt wurden — dieselbe Regel für alle 24 Manifeste,
in dieser Reihenfolge:

1. `Basis-SHA` und `End-SHA` aus dem Manifestkopf (erste 60 Zeilen), wo beide stehen;
2. sonst `Basis-SHA` beziehungsweise `Basis` aus dem Kopf als Basis;
3. sonst der Elternteil des **ältesten** Commits, dessen Betreff den Ticketnamen trägt
   (vereinigt mit den im Kopf genannten Commit-SHAs);
4. Ende: `End-SHA` aus dem Kopf, sonst der **jüngste** solche Commit.

WICHTIG zur Lesart: eine Spanne ist ein Zeitraum, kein Ticketfilter. Liefen zwischen
dem ersten und dem letzten Commit eines Tickets fremde Commits, stehen deren Zeilen in
der Spannensumme mit drin. Die Tabelle in §12.2 führt deshalb zwei Zahlen nebeneinander:
Commits in der Spanne und davon solche, deren Betreff den Ticketnamen trägt. Nur wo
beide gleich sind, ist die Spannensumme die Ticketsumme.

Runden-SHAs: jede Zeile des Manifests (und seines `-verlauf`-Teils), die eine Runde
benennt (`Runde <n>`, `Nacharbeit <n>`, `Phase A/B`), wird nach Backtick-SHAs
durchsucht; jeder Treffer wird gegen `git cat-file` geprüft, auf die Spanne
Basis..Ende beschnitten und in Commit-Reihenfolge sortiert. Nennt ein Manifest keine
solchen SHAs, steht dort `(nur eine Spanne)` — das ist eine Aussage über das
Manifest, nicht über das Ticket.

```text
### S0
Spanne: 734cf50..6c61ae4   (Kopf: Commit (voll) + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 1
$ py -3.13 tools/dirigent/rundenbilanz.py 734cf50..6c61ae4
734cf50..6c61ae4: Pruefwerkzeug 1 Datei(en) +609/-0 | Doku 5 Datei(en) +476/-39 | Sonstiges 1 Datei(en) +1/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 734cf50 6c61ae4
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-001/002
Spanne: df84e20..0ba87cc   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 1
$ py -3.13 tools/dirigent/rundenbilanz.py df84e20..0ba87cc
df84e20..0ba87cc: Produkt 10 Datei(en) +135/-42 | Tests 2 Datei(en) +489/-9 | Pruefwerkzeug 1 Datei(en) +75/-39 | Doku 6 Datei(en) +845/-90 | Sonstiges 1 Datei(en) +12/-0

$ py -3.13 tools/dirigent/rundenbilanz.py --runden df84e20 0ba87cc
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-003
Spanne: 0ba87cc..e9c6fa0   (Kopf: Commitliste + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 6
$ py -3.13 tools/dirigent/rundenbilanz.py 0ba87cc..e9c6fa0
0ba87cc..e9c6fa0: Produkt 5 Datei(en) +821/-1 | Tests 1 Datei(en) +577/-0 | Pruefwerkzeug 1 Datei(en) +14/-2 | Doku 5 Datei(en) +2954/-59 | Sonstiges 2 Datei(en) +17/-0

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 0ba87cc 359b899 3f2fccd e9c6fa0
0ba87cc..359b899: Produkt 5 Datei(en) +746/-1 | Tests 1 Datei(en) +490/-0 | Pruefwerkzeug 1 Datei(en) +14/-2 | Doku 5 Datei(en) +1375/-59 | Sonstiges 2 Datei(en) +17/-0
359b899..3f2fccd: Produkt 1 Datei(en) +89/-19 | Tests 1 Datei(en) +88/-1 | Doku 1 Datei(en) +1056/-12
3f2fccd..e9c6fa0: Produkt 1 Datei(en) +6/-1 | Doku 4 Datei(en) +542/-7

kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)

### SONDE-003b
KEINE SPANNE ERMITTELBAR - Kopf nennt keinen SHA; Ticketcommits: 0

### SONDE-004
Spanne: 3d820c6..9d99f40   (Kopf: Commitliste + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 3
$ py -3.13 tools/dirigent/rundenbilanz.py 3d820c6..9d99f40
3d820c6..9d99f40: Produkt 307 Datei(en) +28186/-65 | Tests 8 Datei(en) +4210/-5 | Pruefwerkzeug 27 Datei(en) +6889/-158 | Doku 60 Datei(en) +24579/-932 | Sonstiges 460 Datei(en) +784/-84

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 3d820c6 9d99f40
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-004a
Spanne: 3d820c6..df84e20   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 1
$ py -3.13 tools/dirigent/rundenbilanz.py 3d820c6..df84e20
3d820c6..df84e20: Produkt 7 Datei(en) +776/-0 | Tests 1 Datei(en) +290/-0 | Pruefwerkzeug 2 Datei(en) +90/-2 | Doku 4 Datei(en) +469/-42 | Sonstiges 2 Datei(en) +194/-0

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 3d820c6 df84e20
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-005a
Spanne: 5299037..6c314e1   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 15
$ py -3.13 tools/dirigent/rundenbilanz.py 5299037..6c314e1
5299037..6c314e1: Produkt 360 Datei(en) +40548/-645 | Tests 12 Datei(en) +8836/-58 | Pruefwerkzeug 93 Datei(en) +13565/-302 | Doku 324 Datei(en) +126851/-1610 | Sonstiges 514 Datei(en) +20734/-176

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 5299037 6c314e1
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-005b
Spanne: 3c64b98..e99a211   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 10
$ py -3.13 tools/dirigent/rundenbilanz.py 3c64b98..e99a211
3c64b98..e99a211: Produkt 310 Datei(en) +30812/-8311 | Tests 12 Datei(en) +8128/-59 | Pruefwerkzeug 93 Datei(en) +11831/-375 | Doku 325 Datei(en) +135801/-1689 | Sonstiges 514 Datei(en) +20724/-176

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 3c64b98 e99a211
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-006
Spanne: 26b7680..3353fb6   (Kopf: Commitliste + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 8
$ py -3.13 tools/dirigent/rundenbilanz.py 26b7680..3353fb6
26b7680..3353fb6: Produkt 152 Datei(en) +20815/-851 | Tests 12 Datei(en) +7702/-61 | Pruefwerkzeug 89 Datei(en) +9288/-258 | Doku 303 Datei(en) +132002/-1082 | Sonstiges 79 Datei(en) +20512/-110

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 26b7680 3353fb6
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-007a
Spanne: dafa5a5..478e564   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 80
$ py -3.13 tools/dirigent/rundenbilanz.py dafa5a5..478e564
dafa5a5..478e564: Produkt 167 Datei(en) +26583/-931 | Tests 14 Datei(en) +9911/-49 | Pruefwerkzeug 94 Datei(en) +15440/-1014 | Doku 252 Datei(en) +258261/-1193 | Sonstiges 71 Datei(en) +4536/-5525

$ py -3.13 tools/dirigent/rundenbilanz.py --runden dafa5a5 a728fba d3c741c b6003c1 facea2d 5acf7f7 0ea62e4 5538fb0 cd346e1 3353301 da62dec ccb98cd 370e513 b80fdce 69b4d20 60717c5 5df7497 a94c33e 3a20064 5dfe3a3 93e8a7c 401d036 f808ad0 f131090 d4f7ed3 dd896a5 32d86d9 a010d64 70f5bad 308947d 00d2796 e9ea54b 165d9ae b9f7ee1 9602d6c 4a379bb d11be90 196f97e d084296 88255d8 f423527 e63a53f 4287839 1991ff8 713f0ae c212280 50615f7 12fcdab 4a2b8da 3de3a13 219424f 27865ca 75466c0 65e5b77 3ef3efa eb84bec d4900ce ae32ea4 0e7a60e 6cd244d c117e40 f68cd9a 22d3695 7a87b7d 478e564
dafa5a5..a728fba: Produkt 162 Datei(en) +26222/-931 | Tests 14 Datei(en) +9911/-49 | Pruefwerkzeug 91 Datei(en) +8968/-952 | Doku 220 Datei(en) +165559/-1189 | Sonstiges 71 Datei(en) +4526/-5525
a728fba..d3c741c: Pruefwerkzeug 6 Datei(en) +709/-71 | Doku 15 Datei(en) +20141/-15
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d3c741c..b6003c1: Doku 2 Datei(en) +245/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
b6003c1..facea2d: Doku 5 Datei(en) +3026/-7
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
facea2d..5acf7f7: Pruefwerkzeug 3 Datei(en) +727/-58 | Doku 1 Datei(en) +499/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
5acf7f7..0ea62e4: Doku 1 Datei(en) +1/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
0ea62e4..5538fb0: Doku 4 Datei(en) +3030/-3
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
5538fb0..cd346e1: Doku 1 Datei(en) +664/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
cd346e1..3353301: Pruefwerkzeug 2 Datei(en) +1071/-82
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
3353301..da62dec: Produkt 1 Datei(en) +34/-8 | Pruefwerkzeug 1 Datei(en) +16/-6 | Doku 6 Datei(en) +9963/-9
da62dec..ccb98cd: Produkt 1 Datei(en) +27/-3 | Pruefwerkzeug 3 Datei(en) +624/-66 | Doku 5 Datei(en) +3698/-12
ccb98cd..370e513: Pruefwerkzeug 2 Datei(en) +180/-16 | Doku 2 Datei(en) +145/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
370e513..b80fdce: Doku 4 Datei(en) +3079/-4
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
b80fdce..69b4d20: Pruefwerkzeug 1 Datei(en) +430/-98 | Doku 5 Datei(en) +70/-4
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
69b4d20..60717c5: Pruefwerkzeug 2 Datei(en) +90/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
60717c5..5df7497: Doku 3 Datei(en) +415/-6
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
5df7497..a94c33e: Doku 3 Datei(en) +3082/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
a94c33e..3a20064: Pruefwerkzeug 2 Datei(en) +183/-49 | Doku 2 Datei(en) +44/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
3a20064..5dfe3a3: Pruefwerkzeug 2 Datei(en) +94/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
5dfe3a3..93e8a7c: Pruefwerkzeug 1 Datei(en) +1/-1 | Doku 3 Datei(en) +578/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
93e8a7c..401d036: Doku 3 Datei(en) +3056/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
401d036..f808ad0: Pruefwerkzeug 2 Datei(en) +25/-13 | Doku 2 Datei(en) +53/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f808ad0..f131090: Pruefwerkzeug 2 Datei(en) +142/-16 | Doku 3 Datei(en) +476/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f131090..d4f7ed3: Doku 2 Datei(en) +17/-11
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d4f7ed3..dd896a5: Doku 3 Datei(en) +3055/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
dd896a5..32d86d9: Pruefwerkzeug 1 Datei(en) +2/-1 | Doku 1 Datei(en) +45/-10
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
32d86d9..a010d64: Pruefwerkzeug 1 Datei(en) +16/-0 | Doku 3 Datei(en) +48/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
a010d64..70f5bad: Produkt 5 Datei(en) +298/-0 | Pruefwerkzeug 2 Datei(en) +641/-249 | Sonstiges 1 Datei(en) +10/-0
70f5bad..308947d: Pruefwerkzeug 2 Datei(en) +44/-19
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
308947d..00d2796: Pruefwerkzeug 1 Datei(en) +7/-2 | Doku 3 Datei(en) +580/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
00d2796..e9ea54b: Doku 3 Datei(en) +3054/-3
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
e9ea54b..165d9ae: Doku 2 Datei(en) +38/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
165d9ae..b9f7ee1: Pruefwerkzeug 2 Datei(en) +38/-15 | Doku 2 Datei(en) +5/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
b9f7ee1..9602d6c: Pruefwerkzeug 3 Datei(en) +209/-119 | Doku 3 Datei(en) +541/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
9602d6c..4a379bb: Doku 4 Datei(en) +3080/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
4a379bb..d11be90: Doku 2 Datei(en) +425/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d11be90..196f97e: Pruefwerkzeug 2 Datei(en) +100/-21 | Doku 2 Datei(en) +320/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
196f97e..d084296: Doku 3 Datei(en) +3083/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d084296..88255d8: Pruefwerkzeug 2 Datei(en) +14/-6 | Doku 2 Datei(en) +38/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
88255d8..f423527: Pruefwerkzeug 2 Datei(en) +114/-13 | Doku 3 Datei(en) +448/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f423527..e63a53f: Doku 3 Datei(en) +3111/-17
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
e63a53f..4287839: Pruefwerkzeug 1 Datei(en) +6/-1 | Doku 2 Datei(en) +47/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
4287839..1991ff8: Pruefwerkzeug 2 Datei(en) +238/-4 | Doku 3 Datei(en) +613/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
1991ff8..713f0ae: Doku 4 Datei(en) +3135/-28
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
713f0ae..c212280: Pruefwerkzeug 2 Datei(en) +283/-12 | Doku 2 Datei(en) +42/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
c212280..50615f7: Pruefwerkzeug 3 Datei(en) +457/-12
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
50615f7..12fcdab: Pruefwerkzeug 2 Datei(en) +2/-2 | Doku 3 Datei(en) +578/-3
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
12fcdab..4a2b8da: Doku 3 Datei(en) +3092/-3
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
4a2b8da..3de3a13: Pruefwerkzeug 2 Datei(en) +250/-57 | Doku 2 Datei(en) +44/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
3de3a13..219424f: Produkt 1 Datei(en) +15/-2 | Pruefwerkzeug 2 Datei(en) +135/-37 | Doku 4 Datei(en) +745/-5
219424f..27865ca: Doku 3 Datei(en) +3123/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
27865ca..75466c0: Doku 2 Datei(en) +40/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
75466c0..65e5b77: Pruefwerkzeug 2 Datei(en) +254/-11
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
65e5b77..3ef3efa: Pruefwerkzeug 4 Datei(en) +177/-28 | Doku 4 Datei(en) +631/-6
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
3ef3efa..eb84bec: Doku 4 Datei(en) +3149/-10
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
eb84bec..d4900ce: Pruefwerkzeug 2 Datei(en) +122/-10 | Doku 2 Datei(en) +40/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d4900ce..ae32ea4: Doku 1 Datei(en) +32/-11
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
ae32ea4..0e7a60e: Pruefwerkzeug 2 Datei(en) +111/-10 | Doku 3 Datei(en) +510/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
0e7a60e..6cd244d: Doku 3 Datei(en) +3226/-21
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
6cd244d..c117e40: Pruefwerkzeug 2 Datei(en) +34/-16 | Doku 3 Datei(en) +35/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
c117e40..f68cd9a: Doku 1 Datei(en) +353/-15
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f68cd9a..22d3695: Doku 2 Datei(en) +3127/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
22d3695..7a87b7d: Pruefwerkzeug 2 Datei(en) +21/-29 | Doku 6 Datei(en) +144/-17
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
7a87b7d..478e564: Doku 5 Datei(en) +18435/-18344
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 21 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill Â§3.4, keine weitere Punktkorrektur-Runde.

### SONDE-007b
Spanne: 4b500a4..36560b0   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 22
$ py -3.13 tools/dirigent/rundenbilanz.py 4b500a4..36560b0
4b500a4..36560b0: Produkt 162 Datei(en) +25912/-1084 | Tests 14 Datei(en) +9924/-49 | Pruefwerkzeug 95 Datei(en) +10262/-1177 | Doku 257 Datei(en) +263868/-3536 | Sonstiges 71 Datei(en) +4521/-5525

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 4b500a4 a2fe0f5 1ca5fdb 79e337c 72146c1 2da1ce8 d211431 1bfc8b0 3687ea4 36560b0
4b500a4..a2fe0f5: Produkt 44 Datei(en) +7273/-260 | Tests 8 Datei(en) +4898/-45 | Pruefwerkzeug 24 Datei(en) +2070/-748 | Doku 56 Datei(en) +55846/-388 | Sonstiges 49 Datei(en) +1619/-1479
a2fe0f5..1ca5fdb: Produkt 82 Datei(en) +10686/-1184 | Tests 9 Datei(en) +2172/-44 | Pruefwerkzeug 73 Datei(en) +5686/-334 | Doku 192 Datei(en) +72713/-1540 | Sonstiges 55 Datei(en) +4247/-5391
1ca5fdb..79e337c: Doku 2 Datei(en) +21/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
79e337c..72146c1: Doku 2 Datei(en) +2579/-9
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
72146c1..2da1ce8: Tests 1 Datei(en) +53/-2 | Pruefwerkzeug 1 Datei(en) +1/-1 | Doku 3 Datei(en) +599/-17
2da1ce8..d211431: Doku 2 Datei(en) +2445/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d211431..1bfc8b0: Produkt 68 Datei(en) +8381/-68 | Tests 6 Datei(en) +2872/-29 | Pruefwerkzeug 13 Datei(en) +2962/-551 | Doku 52 Datei(en) +145076/-20191
1bfc8b0..3687ea4: Doku 2 Datei(en) +3185/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
3687ea4..36560b0: Doku 3 Datei(en) +29/-9
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 2 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill Â§3.4, keine weitere Punktkorrektur-Runde.

### SONDE-007c
Spanne: 9bb75ad..bae7a4e   (Kopf: Basis + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 13
$ py -3.13 tools/dirigent/rundenbilanz.py 9bb75ad..bae7a4e
9bb75ad..bae7a4e: Produkt 70 Datei(en) +8654/-93 | Tests 6 Datei(en) +3141/-6 | Pruefwerkzeug 13 Datei(en) +3255/-575 | Doku 54 Datei(en) +163084/-10520

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 9bb75ad 25b57ec f94d56e 370e513 cb99ba0 5df7497 a94c33e 5dfe3a3 93e8a7c 898b28b 53c10a3 f131090 d4f7ed3 70f5bad 2f1f89b 00d2796 9602d6c 4c3fbf8 196f97e f423527 1991ff8 12fcdab 219424f 3ef3efa e27974c 0e7a60e aadca53 fff9e09 977bf48 bae7a4e
9bb75ad..25b57ec: Produkt 70 Datei(en) +8656/-94 | Tests 6 Datei(en) +3128/-6 | Pruefwerkzeug 10 Datei(en) +3977/-213 | Doku 30 Datei(en) +91532/-444
25b57ec..f94d56e: Pruefwerkzeug 3 Datei(en) +612/-55 | Doku 3 Datei(en) +419/-9
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f94d56e..370e513: Pruefwerkzeug 1 Datei(en) +1/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
370e513..cb99ba0: Pruefwerkzeug 2 Datei(en) +518/-98 | Doku 6 Datei(en) +3147/-6
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
cb99ba0..5df7497: Pruefwerkzeug 1 Datei(en) +2/-2 | Doku 3 Datei(en) +415/-6
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
5df7497..a94c33e: Doku 3 Datei(en) +3082/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
a94c33e..5dfe3a3: Pruefwerkzeug 3 Datei(en) +277/-54 | Doku 2 Datei(en) +44/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
5dfe3a3..93e8a7c: Pruefwerkzeug 1 Datei(en) +1/-1 | Doku 3 Datei(en) +578/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
93e8a7c..898b28b: Doku 3 Datei(en) +3101/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
898b28b..53c10a3: Pruefwerkzeug 3 Datei(en) +152/-29 | Doku 1 Datei(en) +8/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
53c10a3..f131090: Pruefwerkzeug 1 Datei(en) +16/-1 | Doku 3 Datei(en) +476/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f131090..d4f7ed3: Doku 2 Datei(en) +17/-11
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d4f7ed3..70f5bad: Produkt 5 Datei(en) +298/-0 | Pruefwerkzeug 4 Datei(en) +659/-250 | Doku 5 Datei(en) +3148/-10 | Sonstiges 1 Datei(en) +10/-0
70f5bad..2f1f89b: Pruefwerkzeug 3 Datei(en) +51/-21
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
2f1f89b..00d2796: Doku 3 Datei(en) +580/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
00d2796..9602d6c: Pruefwerkzeug 4 Datei(en) +247/-134 | Doku 4 Datei(en) +3638/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
9602d6c..4c3fbf8: Pruefwerkzeug 2 Datei(en) +100/-21 | Doku 4 Datei(en) +3505/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
4c3fbf8..196f97e: Doku 2 Datei(en) +320/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
196f97e..f423527: Pruefwerkzeug 3 Datei(en) +128/-19 | Doku 4 Datei(en) +3569/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f423527..1991ff8: Pruefwerkzeug 2 Datei(en) +244/-5 | Doku 4 Datei(en) +3771/-18
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
1991ff8..12fcdab: Pruefwerkzeug 3 Datei(en) +740/-24 | Doku 4 Datei(en) +3755/-31
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
12fcdab..219424f: Produkt 1 Datei(en) +15/-2 | Pruefwerkzeug 3 Datei(en) +385/-94 | Doku 5 Datei(en) +3881/-8
219424f..3ef3efa: Pruefwerkzeug 4 Datei(en) +416/-24 | Doku 5 Datei(en) +3794/-6
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
3ef3efa..e27974c: Doku 4 Datei(en) +3189/-10
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
e27974c..0e7a60e: Pruefwerkzeug 3 Datei(en) +233/-20 | Doku 3 Datei(en) +542/-12
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
0e7a60e..aadca53: Produkt 8 Datei(en) +32/-347 | Tests 1 Datei(en) +38/-25 | Pruefwerkzeug 7 Datei(en) +791/-5804 | Doku 17 Datei(en) +68864/-55453 | Sonstiges 1 Datei(en) +0/-10
aadca53..fff9e09: Doku 2 Datei(en) +3169/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
fff9e09..977bf48: Produkt 1 Datei(en) +11/-8 | Doku 1 Datei(en) +51/-0
977bf48..bae7a4e: Doku 3 Datei(en) +27/-10
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 11 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill Â§3.4, keine weitere Punktkorrektur-Runde.

### SONDE-008
Spanne: e330052..5d26002   (Kopf: Commitliste + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 15
$ py -3.13 tools/dirigent/rundenbilanz.py e330052..5d26002
e330052..5d26002: Produkt 14 Datei(en) +4191/-167 | Tests 5 Datei(en) +3915/-3 | Pruefwerkzeug 18 Datei(en) +777/-724 | Doku 55 Datei(en) +41838/-374 | Sonstiges 44 Datei(en) +1080/-1041

$ py -3.13 tools/dirigent/rundenbilanz.py --runden e330052 5d26002
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### SONDE-009
Spanne: ff24908..9aa7887   (Kopf: Commitliste + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 11
$ py -3.13 tools/dirigent/rundenbilanz.py ff24908..9aa7887
ff24908..9aa7887: Produkt 11 Datei(en) +2732/-64 | Tests 1 Datei(en) +2137/-0 | Pruefwerkzeug 18 Datei(en) +749/-721 | Doku 55 Datei(en) +29739/-331 | Sonstiges 38 Datei(en) +910/-1018

$ py -3.13 tools/dirigent/rundenbilanz.py --runden ff24908 b2dc288 3c0230a 9aa7887
ff24908..b2dc288: Produkt 11 Datei(en) +2732/-64 | Tests 1 Datei(en) +2135/-0 | Pruefwerkzeug 18 Datei(en) +749/-721 | Doku 55 Datei(en) +24804/-331 | Sonstiges 38 Datei(en) +910/-1018
b2dc288..3c0230a: Tests 1 Datei(en) +4/-2 | Doku 5 Datei(en) +2308/-21
3c0230a..9aa7887: Doku 4 Datei(en) +2656/-8
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)

### SONDE-010
Spanne: a7b0740..d1ef796   (Kopf: Basis in der Commitliste + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 45
$ py -3.13 tools/dirigent/rundenbilanz.py a7b0740..d1ef796
a7b0740..d1ef796: Produkt 66 Datei(en) +9854/-49 | Tests 5 Datei(en) +3540/-25 | Pruefwerkzeug 12 Datei(en) +2629/-536 | Doku 61 Datei(en) +164403/-25551

$ py -3.13 tools/dirigent/rundenbilanz.py --runden a7b0740 cdff93b 2ac23d0 1b19cd1 10a4806 97c956d 4500785 602e105 6fc3224 d137fa0 65d46a0 e5f5c27 a0053e4 1bdb93d c444ca3 66e4a09 e3e8e57 a88d32e 444e125 05235cf b4fe522 e517165 53f517b 83f7d7e d1ef796
a7b0740..cdff93b: Produkt 38 Datei(en) +317/-0 | Pruefwerkzeug 1 Datei(en) +449/-0
cdff93b..2ac23d0: Produkt 11 Datei(en) +2652/-6 | Tests 2 Datei(en) +386/-0
2ac23d0..1b19cd1: Produkt 12 Datei(en) +2624/-1 | Tests 2 Datei(en) +1442/-0 | Pruefwerkzeug 1 Datei(en) +13/-0
1b19cd1..10a4806: Pruefwerkzeug 2 Datei(en) +209/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
10a4806..97c956d: Tests 1 Datei(en) +39/-0 | Pruefwerkzeug 1 Datei(en) +1/-1
97c956d..4500785: Produkt 9 Datei(en) +1509/-163 | Tests 2 Datei(en) +354/-41 | Pruefwerkzeug 1 Datei(en) +23/-2 | Doku 5 Datei(en) +3547/-23
4500785..602e105: Produkt 1 Datei(en) +6/-1
602e105..6fc3224: Produkt 2 Datei(en) +23/-3
6fc3224..d137fa0: Doku 3 Datei(en) +3354/-12
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d137fa0..65d46a0: Produkt 9 Datei(en) +650/-93 | Tests 2 Datei(en) +462/-25 | Doku 4 Datei(en) +195/-178
65d46a0..e5f5c27: Produkt 1 Datei(en) +14/-10
e5f5c27..a0053e4: Produkt 1 Datei(en) +8/-0 | Tests 1 Datei(en) +114/-0 | Doku 4 Datei(en) +3448/-14
a0053e4..1bdb93d: Produkt 2 Datei(en) +99/-44 | Tests 1 Datei(en) +53/-17 | Doku 9 Datei(en) +6578/-389
1bdb93d..c444ca3: Produkt 2 Datei(en) +473/-31
c444ca3..66e4a09: Produkt 2 Datei(en) +9/-0 | Tests 1 Datei(en) +30/-2 | Pruefwerkzeug 1 Datei(en) +14/-0
66e4a09..e3e8e57: Doku 3 Datei(en) +438/-5
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
e3e8e57..a88d32e: Produkt 1 Datei(en) +4/-4 | Doku 2 Datei(en) +3010/-0
a88d32e..444e125: Doku 1 Datei(en) +2960/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
444e125..05235cf: Produkt 4 Datei(en) +222/-23 | Tests 1 Datei(en) +38/-25 | Pruefwerkzeug 10 Datei(en) +1932/-545 | Doku 48 Datei(en) +146587/-44089
05235cf..b4fe522: Produkt 9 Datei(en) +1726/-343 | Tests 2 Datei(en) +709/-2 | Pruefwerkzeug 1 Datei(en) +1/-1 | Doku 7 Datei(en) +6680/-5
b4fe522..e517165: Produkt 1 Datei(en) +201/-10 | Doku 3 Datei(en) +3386/-0
e517165..53f517b: Doku 1 Datei(en) +114/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
53f517b..83f7d7e: Doku 3 Datei(en) +3248/-1
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
83f7d7e..d1ef796: Doku 4 Datei(en) +36/-13
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 3 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill Â§3.4, keine weitere Punktkorrektur-Runde.

### SONDE-011
Spanne: b75ea06..9abb66f   (Kopf: Basis + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 19
$ py -3.13 tools/dirigent/rundenbilanz.py b75ea06..9abb66f
b75ea06..9abb66f: Produkt 105 Datei(en) +11793/-389 | Tests 9 Datei(en) +4856/-3 | Pruefwerkzeug 6 Datei(en) +678/-24 | Doku 22 Datei(en) +23097/-1361

$ py -3.13 tools/dirigent/rundenbilanz.py --runden b75ea06 b327c1d d172b0a 901598f 5882718 e917526 f176f24 9abb66f
b75ea06..b327c1d: Produkt 87 Datei(en) +4658/-118 | Tests 4 Datei(en) +292/-2 | Pruefwerkzeug 4 Datei(en) +549/-20 | Doku 2 Datei(en) +304/-1
b327c1d..d172b0a: Produkt 9 Datei(en) +83/-89 | Tests 3 Datei(en) +3/-3 | Doku 5 Datei(en) +6649/-7
d172b0a..901598f: Produkt 24 Datei(en) +6887/-278 | Tests 6 Datei(en) +4307/-1 | Pruefwerkzeug 3 Datei(en) +132/-7 | Doku 16 Datei(en) +12555/-1371
901598f..5882718: Produkt 4 Datei(en) +294/-41 | Tests 3 Datei(en) +142/-11 | Doku 1 Datei(en) +48/-0
5882718..e917526: Produkt 1 Datei(en) +15/-10 | Tests 2 Datei(en) +129/-13 | Doku 1 Datei(en) +33/-0
e917526..f176f24: Produkt 1 Datei(en) +13/-10 | Tests 1 Datei(en) +18/-8 | Doku 1 Datei(en) +38/-0
f176f24..9abb66f: Doku 6 Datei(en) +3505/-17
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)

### SONDE-012
Spanne: 9abb66f..c978148   (Kopf: Basis-SHA + End-SHA)
Commits mit Ticketnamen im Betreff: 7
$ py -3.13 tools/dirigent/rundenbilanz.py 9abb66f..c978148
9abb66f..c978148: Produkt 118 Datei(en) +9801/-1925 | Tests 11 Datei(en) +3528/-135 | Pruefwerkzeug 9 Datei(en) +767/-28 | Doku 17 Datei(en) +26248/-37

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 9abb66f c978148
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### NAK-96
Spanne: 2271df5..c82d8ce   (Kopf: Basis-SHA + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 21
$ py -3.13 tools/dirigent/rundenbilanz.py 2271df5..c82d8ce
2271df5..c82d8ce: Pruefwerkzeug 5 Datei(en) +407/-69 | Doku 12 Datei(en) +19910/-8
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 2271df5 8a1ea8a d993894 b4b1e29 f124746 34491e0 631ac34 a9c6450 7be6cd5 202f2f8 c82d8ce
2271df5..8a1ea8a: Pruefwerkzeug 2 Datei(en) +182/-62 | Doku 9 Datei(en) +9085/-4
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
8a1ea8a..d993894: Doku 2 Datei(en) +3064/-0
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
d993894..b4b1e29: Pruefwerkzeug 3 Datei(en) +199/-18 | Doku 4 Datei(en) +3876/-7
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
b4b1e29..f124746: Doku 2 Datei(en) +4/-4
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
f124746..34491e0: Doku 3 Datei(en) +3092/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
34491e0..631ac34: Pruefwerkzeug 2 Datei(en) +7/-3 | Doku 1 Datei(en) +110/-22
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
631ac34..a9c6450: Doku 1 Datei(en) +2/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
a9c6450..7be6cd5: Pruefwerkzeug 1 Datei(en) +50/-22
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
7be6cd5..202f2f8: Pruefwerkzeug 1 Datei(en) +6/-1 | Doku 3 Datei(en) +722/-42
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
202f2f8..c82d8ce: Doku 3 Datei(en) +34/-4
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 10 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill Â§3.4, keine weitere Punktkorrektur-Runde.

### NAK-123
Spanne: e0a0dd6..5e41a28   (Kopf: Basis-SHA + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 10
$ py -3.13 tools/dirigent/rundenbilanz.py e0a0dd6..5e41a28
e0a0dd6..5e41a28: Produkt 22 Datei(en) +2008/-291 | Tests 9 Datei(en) +1814/-19 | Pruefwerkzeug 4 Datei(en) +399/-33 | Doku 43 Datei(en) +12586/-143

$ py -3.13 tools/dirigent/rundenbilanz.py --runden e0a0dd6 d8676e0 0b174d2 5e41a28
e0a0dd6..d8676e0: Produkt 22 Datei(en) +2008/-291 | Tests 9 Datei(en) +1814/-19 | Pruefwerkzeug 4 Datei(en) +399/-33 | Doku 40 Datei(en) +7928/-135
d8676e0..0b174d2: Doku 3 Datei(en) +4584/-2
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
0b174d2..5e41a28: Doku 5 Datei(en) +81/-13
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 2 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill Â§3.4, keine weitere Punktkorrektur-Runde.

### G0
Spanne: 0df26c8..48acd89   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 4
$ py -3.13 tools/dirigent/rundenbilanz.py 0df26c8..48acd89
0df26c8..48acd89: Produkt 1 Datei(en) +124/-0 | Pruefwerkzeug 30 Datei(en) +1718/-899 | Doku 118 Datei(en) +22868/-878 | Sonstiges 100 Datei(en) +21412/-23

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 0df26c8 48acd89
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### G1
Spanne: 4ec7480..b75ea06   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 20
$ py -3.13 tools/dirigent/rundenbilanz.py 4ec7480..b75ea06
4ec7480..b75ea06: Produkt 153 Datei(en) +21621/-1212 | Tests 12 Datei(en) +5839/-38 | Pruefwerkzeug 79 Datei(en) +8762/-658 | Doku 246 Datei(en) +246569/-4264 | Sonstiges 56 Datei(en) +4252/-5392

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 4ec7480 c72d51e b75ea06
4ec7480..c72d51e: Produkt 152 Datei(en) +20977/-1211 | Tests 12 Datei(en) +5521/-38 | Pruefwerkzeug 79 Datei(en) +8740/-658 | Doku 236 Datei(en) +224630/-4220 | Sonstiges 56 Datei(en) +4252/-5392
c72d51e..b75ea06: Produkt 8 Datei(en) +714/-71 | Tests 1 Datei(en) +318/-0 | Pruefwerkzeug 2 Datei(en) +27/-5 | Doku 22 Datei(en) +21978/-83

kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)

### G2
Spanne: e77ebf9..5e41a28   (Kopf nennt keinen SHA + Basis = Elternteil des aeltesten Ticketcommits + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 5
$ py -3.13 tools/dirigent/rundenbilanz.py e77ebf9..5e41a28
e77ebf9..5e41a28: Produkt 22 Datei(en) +2008/-291 | Tests 9 Datei(en) +1814/-19 | Pruefwerkzeug 4 Datei(en) +399/-33 | Doku 43 Datei(en) +12800/-142

$ py -3.13 tools/dirigent/rundenbilanz.py --runden e77ebf9 5e41a28
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

### PR1
Spanne: 6f40eed..d20201e   (Kopf: Basis-SHA + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 18
$ py -3.13 tools/dirigent/rundenbilanz.py 6f40eed..d20201e
6f40eed..d20201e: Doku 8 Datei(en) +1776/-66
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 6f40eed 0e3908e b54a575 6127595 d20201e
6f40eed..0e3908e: Doku 8 Datei(en) +934/-55
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
0e3908e..b54a575: Doku 6 Datei(en) +583/-38
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
b54a575..6127595: Doku 3 Datei(en) +244/-10
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)
6127595..d20201e: Doku 3 Datei(en) +66/-14
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

KONVERGENZ-SIGNAL: 4 Runden in Folge ohne Produktfortschritt - Konvergenzentscheid nach Skill Â§3.4, keine weitere Punktkorrektur-Runde.

### PR2
Spanne: 55cdb91..008989c   (Kopf: Basis-SHA + Ende = juengster Ticketcommit)
Commits mit Ticketnamen im Betreff: 14
$ py -3.13 tools/dirigent/rundenbilanz.py 55cdb91..008989c
55cdb91..008989c: Pruefwerkzeug 2 Datei(en) +371/-3 | Doku 30 Datei(en) +7091/-584
    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)

$ py -3.13 tools/dirigent/rundenbilanz.py --runden 55cdb91 008989c
(nur eine Spanne - das Manifest nennt keine Zwischenstaende)

```
