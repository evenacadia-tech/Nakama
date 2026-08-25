# 2026-08-21 — Designvertrag: drei Apps, eine Identität, Figma ist die Quelle

> ## Abnahme (bindend)
>
> Herkunft: das **Kontext-Interview vom 2026-08-21** (zwanzig Fragen, vom
> User beantwortet; geführt in der Technik-Session, Protokoll dort im
> Entscheide-Register). Hier steht, was davon dieses Repo bindet — mit dem
> Wortlaut. Zwei Sorten Wortlaut, gekennzeichnet: **(frei)** = vom User
> getippt · **(Option)** = die vom User gewählte Antwortmöglichkeit des
> Interviews, in deren Formulierung.
>
> Diese Datei ist der Designvertrag aus `docs/arbeitsplan.md`, Phase 0. Sie
> öffnet die Schleuse vor `prototyp/`. Was das Interview **nicht** gefragt
> hat, steht unten unter „offen" — es wird nicht aus dem Bild erraten.

## Die wörtlichen Urteile

| # | Frage | Wortlaut | Sorte |
|---|---|---|---|
| 1 | Was ist Nakama heute? | „Familie; Prisma nur Studie" | Option |
| 2 | Die aktive Sonde | „die active Probe fester Name : Nakama Probeeq ist ein vollwertiger hochwertiger EQ der mit Nakama kommuniziert. er kann von nakama direkt anweisungen umsetzen aber auch ganz normal manuell vom user benutzt werden," | frei |
| 3 | Die Namen | „Nakama Gen = Main app   aktive sonde = Nakama Probeeq   passive sonde = Nakama Suna   Bundle = Nakama Studio" | frei |
| 4 | Wo das Design entsteht | „das finale design wird aktuell in figma gemacht . alle 3 apps werden ein design haben mit der selben identität. alle alten sind alt. Ein Design entwickelt sich und ist nicht einfach da. was jetzt aktuell ist kann auch wieder weiter entwickelt werden. probeeq und Gen aktuelle bilder im download ordner" | frei |
| 5 | Rolle dieses Repos | „Figma ist Quelle; Repo setzt um" | Option |
| 6 | Sprache des Produkts | „Englisch – mein Wort" | Option |
| 7 | Glas-und-Licht-Arbeitsannahme, Geschmacksprofil | „Glas/Licht raus; Profil nur Studie" | Option |
| 8 | Eigene Entwürfe des Users (selfmade, Figma) | „Richtung für Look & Stimmung" | Option |
| 9 | Einordnung der Entwürfe (Abnahme vom selben Tag) | „Das ist ein Designprototyp keine technikanleitung, design passt sich am ende der funktion an." | frei |
| 10 | Produktzahlen (16/32 Quellen, 8 Bänder, ±12 dB, Remote ±3 dB) | „Hingenommen, passen aber" | Option |
| 11 | Quellenzahl im Alltag (Abnahme vom selben Tag) | „ich habe schlicht 5 genommen weil 16 bedeutet 16 geladene proben auf instrumenten. das ist sehr viel, die wenigstens werden 16 benutzen. man nutzt das dann eher auf instrumentbusse . 16 plugins nur für eq kostet auch massig leistung" | frei |
| 12 | KI-/Claude-Erklärschicht im Produkt | „Nein – raus aus dem Produkt" | Option |
| 13 | Material-Kit-Front des heutigen Plugins | „Nie abgenommen – bleibt Provisorium" | Option |
| 14 | Aufräumen | alte Design-Richtungen ins Archiv · Prisma-Studie zusammenziehen und parken · tote Memories löschen · Inspirationen und regenerierbare Renders raus — alle vier gewählt | Option |

## Was damit festliegt

1. **Das Produkt sind drei Apps mit EINER Design-Identität.**
   **Nakama Gen** (die Main-App: Quellen, Befunde, Advisor) ·
   **Nakama Probeeq** (aktive Sonde) · **Nakama Suna** (passive Sonde).
   Bundle-Name **Nakama Studio**. Die bisherigen Arbeitsnamen Main /
   Active-Probe-Editor / Passive-Probe-Kachel bezeichnen dieselben drei
   Oberflächen; in neuen Texten gelten die neuen Namen. (Urteile 1, 3, 4)
2. **Probeeq ist ein vollwertiger, hochwertiger EQ.** Er setzt Anweisungen
   von Gen direkt um **und** ist ganz normal manuell bedienbar — beides
   gleichwertig, nichts davon ein Sonderfall. „Nakama berät nur" gilt für
   Gen und Suna; Probeeq verändert Klang, das ist sein Zweck. (Urteil 2)
3. **Die Quelle des Designs ist Figma, und der Autor ist der User.** Die
   aktuellen Stände liegen in `assets/figma/` (je Stand eine datierte Datei,
   beschrieben in `assets/figma/LIES-MICH.md`). Dieses Repo **übersetzt**
   sie in lebende Blätter und prüft, was ein Bild nicht prüfen kann:
   **Zustände, Größen, Grenzfälle.** Keine eigene Stilsuche, keine
   Varianten-Befragung mehr — 2–3 Varianten nebeneinander nur noch dort,
   wo Figma eine Frage offenlässt. Ein Design entwickelt sich: ein neuerer
   Figma-Stand ersetzt den älteren, ohne dass das ein Rückschritt wäre.
   (Urteile 4, 5)
4. **Grad der kreativen Autorenschaft: beim User.** Claude erfindet keine
   Richtung, keine Metapher, keine Farbwelt. Die Eigenentwürfe des Users
   (`Nakama Designausarbeitungen selfmade/`, Figma) sind **Richtung für
   Look & Stimmung** und **Designprototypen, keine Technikanleitung** — das
   Design passt sich am Ende der Funktion an. (Urteile 8, 9)
5. **Referenzen und Antireferenzen.** Referenz ist der jeweils neueste
   Figma-Stand. **Alle alten Richtungen sind alt:** die Welt „Glas und
   Licht" und die Fassung „dunkles Glas, ein Lichtleiter" sind **verworfen**
   (ihr Prüfpunkt ist damit eingelöst — sie werden nicht erneut vorgelegt);
   das Stilprofil aus der Stilbefragung liegt in `assets/verworfen/`; die
   Material-Kit-Front des heutigen Plugins ist ein nie abgenommenes
   Provisorium; die Prisma-/Hörkompass-Studie ist eine Studie des Users,
   kein Produktteil — ihr Vokabular gehört nicht in die Plugin-UI. Das
   Referenzmaterial in der Truhe (301 / 110 / 297) bleibt Referenz; ob
   etwas davon ins Produkt kommt, entscheidet Figma. (Urteile 4, 7, 13, 14)
6. **Sprache: das Produkt ist englisch.** Jeder Text, der im Produkt steht
   — Beschriftungen, Werte, Einheiten, Zustände, Meldungen — und damit
   auch jedes Spezimen in `werkzeug/` und `prototyp/`. Docs, Abnahmen,
   Commits und Gespräch bleiben deutsch. (Urteil 6)
7. **Startwerte, änderbar:** 16 sichtbare Quellen (Verträge bis 32),
   8 Bänder, manuell ±12 dB, ferngesteuert ±3 dB. Der **Regelfall ist eine
   einstellige Quellenzahl** (Instrumentbusse); 16 bleibt die
   Belastungsprobe — was bei 16 bricht, trägt nicht. (Urteile 10, 11)
8. **Technische Grenzen** stehen in den Abnahmen vom 2026-08-20 und werden
   hier nicht neu entschieden: **Gen 760×430 · Probeeq 700×420 · Suna
   260×84** (`2026-08-20-groessen-alle-drei.md`, `2026-08-20-groesse-main.md`),
   Zielbildschirm 1920×1080 bei 100 %, Urteil nur bei 1:1. Gemessen am
   2026-08-21: die Gerätekörper in beiden Figma-Exporten sind **genau**
   4 × 760×430 und 4 × 700×420 — Figma arbeitet auf den abgenommenen Maßen.
9. **Keine KI-/Claude-Erklärschicht im Produkt.** Der Advisor in Gen ist
   regelbasiert; kein Blatt zeigt einen „Ask Claude"-Knopf oder eine
   KI-Erklärung. (Urteil 12)
10. **Der wichtigste Nutzerjob** bleibt, was `docs/oberflaechen-spezifikation.md`
    als Produktidee festhält: den Mix lesen, den wahrscheinlichen Verursacher
    eines Problems finden, ihn hörbar beweisen, eine kleine konkrete
    Änderung am richtigen Bus vorschlagen — in Gen beraten, in Probeeq
    ausführen (auf Anweisung oder von Hand).

## Was ausdrücklich offen bleibt

- **Erfolgskriterien und Gefühlsattribute in Worten.** Das Interview hat
  sie nicht gefragt; der Arbeitsplan Phase 0 sieht sie vor. Der User hat
  stattdessen Bilder geliefert — die Figma-Stände tragen die Stimmung. Wer
  Worte dafür braucht (z. B. für eine Abnahme-Checkliste), fragt den User,
  statt sie aus dem Bild zu lesen.
  > **Nachtrag 2026-08-22 — das Erfolgskriterium hat Worte:** „Hauptsache die
  > Apps selbst in fl Studio sehen dann exakt aus wie in figma" —
  > [2026-08-22-erfolgskriterium-exakt-wie-figma](2026-08-22-erfolgskriterium-exakt-wie-figma.md).
  > Gefühlsattribute bleiben offen.
- **Für Nakama Suna liegt noch kein Figma-Stand vor.** Bis er kommt, gibt
  es für Suna nur die abgenommene Größe 260×84 und den Pflichtinhalt der
  Spezifikation. Nichts davon wird ohne Stand entworfen.
- **Zwölf oder dreizehn Parameter je Band.** Der Probeeq-Stand zeigt zwölf
  (`BAND TYPE FREQ GAIN Q MODE | DYN THRESH RANGE ATTACK HOLD RELEASE`);
  der technische Entwurf zählt eine Sidechain-Quelle als dreizehnten.
  Offener Punkt **NAK-33** im Technik-Repo; hier wird das nicht entschieden
  (`docs/oberflaechen-spezifikation.md`, Abschnitt 2).
- **Wo der Figma-Stand etwas anderes sagt als eine Abnahme vom
  2026-08-20**, wird das beim Übersetzen benannt und dem User vorgelegt —
  nicht hier still entschieden. Bekannt aus
  `Nakama Designausarbeitungen selfmade/LIES-MICH.md`: die `STALE`-Zeile
  ist gestrichelt gezeichnet (Abnahme: hohl = veraltet, Strichlinie =
  getrennt) · `1 STALE` nennt den Namen nicht (Abnahme: Namen bis zwei je
  Sorte) · der Zeilenmarker liegt in Ruhe auf der Fokusquelle (Abnahme:
  markierte Zeile = Vorhören). Dazu die Wortmarke `NAKAMA / MAIN` im
  Gen-Stand, die den Namen Gen noch nicht trägt.
- **Skalierungsmodell.** Feste Faktoren (75–200 %) waren eine Empfehlung von
  Claude, kein User-Wort (`2026-08-20-groesse-main.md`, Punkt 2). Bleibt
  Empfehlung.
- **Bundle-Umbenennung** (Code, Bundle, Pipes heißen heute noch EQ-Copilot)
  ist ein Identitäts-Ticket im Technik-Repo, nicht Thema hier.
- **Alles, was die Bilder nicht zeigen:** Leer-, Lade- und Fehlerzustände,
  alle getrennt, 16 Quellen, acht aktive Bänder mit Dynamik, Bypass,
  getrennt vom Main. Genau das ist die Arbeit der Übersetzung.

## Was das für die Schleuse heißt

Phase 0 des Arbeitsplans wurde mit dieser Datei erfüllt. Die damalige
Prototyp-Schleuse ließ damit Schreibzugriffe in `prototyp/` zu. Weil diese
Bedingung danach dauerhaft erfüllt war, wurde der Hook am 24.08.2026
stillgelegt. Inhaltlich gilt weiter: Der Prototyp übersetzt den Gen-Stand und
ist kein neuer Entwurf.

## Bezug

- `assets/figma/LIES-MICH.md` — die Stände, beschrieben
- `docs/arbeitsplan.md` — Phase 0 erledigt, Phasen danach
- `docs/oberflaechen-spezifikation.md` — der Pflichtinhalt je App
- [2026-08-20-groessen-alle-drei.md](2026-08-20-groessen-alle-drei.md) ·
  [2026-08-20-groesse-main.md](2026-08-20-groesse-main.md) — die Größen
- [2026-08-20-welt-glas-und-licht.md](2026-08-20-welt-glas-und-licht.md) ·
  [2026-08-20-fassung-lichtleiter.md](2026-08-20-fassung-lichtleiter.md) —
  die verworfenen Arbeitsannahmen
- `Nakama Designausarbeitungen selfmade/LIES-MICH.md` — die Entwürfe des
  Users, gegen die Abnahmen geprüft
