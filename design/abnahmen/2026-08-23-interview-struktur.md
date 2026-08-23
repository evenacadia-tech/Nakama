# 2026-08-23 — Interview „struktur": Arbeitsfluss der EQ-Zentrale (Runden 1–2)

Phase `struktur` des Interview-Skills, im Anschluss an den Entscheid
`2026-08-23-gen-eq-zentrale.md`. Antworten wörtlich; jede Runde wurde als
Zusammenfassung vorgelesen und vom User bestätigt (R1 mit Korrektur der
Lesart, R2 mit „ok").

## Runde 1 — Arbeitsfluss (Fragen 1–4)

### Frage 1 — der häufigste Handgriff auf Seite 2

> „zwischen den sonden durchschalten, verschiedene eq filter setzen, zurück
> auf den master eq schalten. das durchschalten muss smooth sein, kein
> menü , am besten scrollen mit dem mausrad wenn nichts anderes markiert
> ist per klick. jede EQ ansicht braucht direkt zugriff auf die
> vorgeschlagenen veränderungen die GEN anbietet um sie anzuwenden. jede
> funktion muss da sein wo man sie braucht. die frontseite bleibt trotzdem
> erstmal wie gehabt um nicht alles umzuwerfen. design kann danach noch
> angepasst werden."

### Frage 2 — der Weg vom Befund zum Eingriff

> „man kann die veränderung direkt anwenden indem man den button klickt
> dafür. der clou : man kann den button gedrückt halten und hört die
> veränderung, lässt man ihn los ist sie weg. klickt man einmal ist sie
> angewendet man muss nochmal klicken dass sie permanent ist. ansonsten
> schaltet sie nach 10 sekunden zurück. ansonsten button daneben springt
> direkt zum eq der sonde. dort befindet sich ei ui element mit welchem
> man das selbe machen kann. also halten, klicken klicken."

### Frage 3 — die zwei Spuren im Graph

> „egal auf welchem band man ist , kann man über die ein wählbares band
> hinzufügen. standardmäßig ist es immer nur eine spur. das ist rein
> optional. hat man 2 bänder aktiv kann man auswählen welches band man
> bearbeiten will."

Nachfrage zur Lesart („Band" = Spur/EQ-Kurve, nicht Filter-Band) —
Antwort:

> „ja ich meine den graph, eq kurve."

### Frage 4 — was die Hände schon können

> „smooth operator pro , oder pro-q sind die 2 besten. dafür kannst du
> alles studieren wie sie das handhaben und das beste kopieren. wir nehmen
> das probeeq als working design für jetzt. die GEN 1 und seite 2 werden
> nochmal komplett überarbeitet um den technischen neuen funktionen
> gerecht zu werden."

### Nachtrag bei der Bestätigung (Arbeitsweise + Auftrag)

> „ich werde parallel zum implement die beiden designs nach und nach
> designen, das ist ja ein prozess kein ‚mal eben schnell machen' ich
> brauche hierfür aber nochmal eine eigenständige spec was die UI
> technisch braucht, also die spielregeln die das design erfüllen muss um
> umsetzbar zu sein."

→ Die Spec liegt seit heute als `design/docs/ui-spielregeln-eq-zentrale.md`.

## Runde 2 — Grenzfälle (Fragen 5–8)

### Frage 5 — die Sonde ohne Gen (Rückfallfläche)

> „es geht hier in erster linie um performance, das plugin muss so wenig
> wie möglich auf die leistung schlagen, da sie ja in hoher anzahl
> angewendet wird. wenn GEN abgestürzt ist steht da was der user wissen
> muss : disconnected / Connected . ich kann das nicht ausführlich
> beantworten weil ich nicht im kopf habe was GEN anzeigt bezüglich des
> alten Suna konzeptes. es muss überlegt werden was der user veranlassen
> könnte den probeeq zu öffnen statt gleich Gen? Connected /
> disconnected, EQ Mode grüner oder roter punkt und welche eq werte
> aktuell verändert sind. bypass, und natürlich mix also wieviel sie auf
> den mix anwendet des eq (das gehört aber auch noch in den Gen ganz
> wichtig)"

**Befund dazu (Claude, am Vertrag gemessen):** Der eingefrorene
Parameterbestand `eq-copilot/schemas/state/nakama-parameter-v1.json`
kennt global nur Bypass · Input-Trim · Output-Trim · Width · Mono-Bass —
**einen Mix/Dry-Wet gibt es noch nicht.** Das User-Wort macht ihn zur
Anforderung; Umsetzung als versionierte Erweiterung (neue ID) im
Umschnitt NAK-64, Punkt 4.

### Frage 6 — was „vollwertig" beim Master-EQ heißt

> „das übersteigt mein technik verständnis, er muss das rad nicht neu
> erfinden, aber vorbild ist auf jedenfall besagte eq. dynamisch ist schon
> drin, alle gängigen filter die es gibt highpass und so weiter. der rest
> übersteigt mein verständniss was umsetzbar ist und was nicht. smooth
> operator hat resonanzpeak als fokus zum beispiel."

### Frage 7 — die Überladen-Grenze auf Seite 2

> „das müssen wir gesondert button für button machen. dazu musst du das
> aktuelle probeeq design anschauen, analysieren was fehlt und was da
> ist. dann klären wir das ganz genau. so halb halb bringt das nix."

### Frage 8 — Reihenfolge beim Durchschalten

> „mixer reihenfolge, master erhält ui element"

Zusammenfassung beider Befunde und aller fünf Punkte vorgelesen —
Antwort: **„ok"**.

## Runde 3 — Button für Button am Working Design (Fragen 9–12)

Grundlage: Inventur des Figma-Stands `assets/figma/2026-08-22-probeeq.png`
im Chat (was da ist / was fehlt / was überholt ist), wie in Runde 2
gefordert („button für button").

### Frage 9 — was der Durchschalter je Sonde zeigt

> „Den Namen auf welchem Bus er sitzt mehr nicht"

### Frage 10 — Draft-Kasten und Drei-Stufen-Geste

Vorgelegte Lesart: APPLY = erster Klick; danach wechselt derselbe Button
in einen Bestätigen-Zustand mit sichtbarer Restzeit; REJECT bleibt.

> „stimmt so"

### Frage 11 — Live-Spektrum im Graph

> „spektrum ja, bei 2 nur die spur die bearbeitet wird."

### Frage 12 — PROTECTED (nach erneuter Erklärung, Antwort per Auswahl)

Erklärt wurde: Schutz-Zone = unantastbarer Frequenzbereich; technisch
existieren bereits „User-Schutz" als Ablehnungsgrund im Fernweg und die
Guardrail „geschützte Bereiche" im Advisor-Urteil.

- Wirkung: Wahl **„Harte Sperre für alle"** — auch eigene Bänder lassen
  sich nicht in die Zone ziehen, bis sie gelöst wird.
- Anlegen: Wahl **„Ich + Gen darf vorschlagen"** — Gen darf aus einem
  Befund eine Zone vorschlagen, angelegt wird sie erst durch User-Klick.

## Runde 4 — die letzten Buttons (Fragen 13–16)

### Frage 13 — BYPASS ↔ EQ-Mode-Punkt (Lesart: zwei Dinge)

> „ja korrekt so. also : EQ mit rotem oder Eq mit grünem punkt. Connected
> oder disconnected, bypass und bei eq on die werte + den mode der probe."

### Frage 14 — Undo-Form und -Reichweite

> „undo so groß rechts unten ist übertrieben. da gehört eher ein mix knob
> hin. einen für die gesamte app. undo kann auch ein kleines symbol auf
> dem display werden"

### Nachtrag zum Mix-Knob (nächste Nachricht)

> „BEZÜGLICH DEM mixer knob der ist für die ganze app. jeder sonde
> braucht die funktion für sich den dry wet zu stellen. das heißt der
> knob bekommt klick funktion um von global auf nur für das gewählte band
> umzuschalten."

(„Band" hier wieder im Sinn von Runde 3 = Spur/EQ-Kurve der gewählten
Sonde.)

### Frage 15 — die GLOBAL-Werte (nach Erklärung von Trim/Width/Mono-Bass)

> „das muss einklappbar sein. es ist eingeklappt ein kleiner pfeil klappt
> es aus = platz"

### Frage 16 — Draft-Wortlaut (nach Erklärung, was die Beschriftung tut)

> „es steht klick/hold darunter. darüber kommt das korrekte englische
> wort dafür. also ja draft reicht, aber die limitangabe oder das limit
> selbst ist sinnlos geworden. das kann im eq dann ja angepasst werden
> von selbst"

**Folge (im Register 23.08.):** Das ±3-dB-Remote-Limit entfällt — Anzeige
UND Grenze; damit ist auch die offene Frage **U14** beantwortet.

---

## Festgelegt (User-Wort, 23.08.2026)

1. Kern-Handgriff Seite 2: Sonden durchschalten → Filter setzen → zurück
   zum Master. Durchschalten ohne Menü; Mausrad wechselt die Sonde, wenn
   nichts per Klick markiert ist.
2. Die Advisor-Vorschläge von Gen sind in JEDER EQ-Ansicht direkt
   anwendbar — „jede funktion muss da sein wo man sie braucht".
3. Anwenden-Geste in drei Stufen: **Halten** = hören, loslassen = weg ·
   **1. Klick** = angewendet, fällt nach 10 Sekunden von selbst zurück ·
   **2. Klick** = permanent. Am Befund zusätzlich ein Button, der direkt
   in den EQ der betroffenen Sonde springt; dort dasselbe Element.
4. Graph: standardmäßig EINE Spur; optional eine frei wählbare zweite;
   bei zwei aktiven Spuren wählt der User das Bearbeitungsziel.
5. Referenzen für das Handling: **Smooth Operator Pro** und **Pro-Q** —
   Claude darf ihr Handling studieren und das Beste übernehmen.
6. Probeeq-Figma = Working Design für jetzt; Seite 1 bleibt vorerst wie
   gehabt; später werden Gen Seite 1 UND Seite 2 komplett überarbeitet.
   Das Design entsteht parallel zur Implementierung, als Prozess.
7. **Rückfallfläche der Sonde: Performance zuerst** (hohe Instanzzahl,
   „so wenig wie möglich auf die leistung schlagen"). Inhalt:
   CONNECTED/DISCONNECTED · EQ-Mode-Punkt grün/rot · welche EQ-Werte
   aktuell verändert sind · Bypass · Mix.
8. **Mix (Dry/Wet je Sonde) ist beschlossen** — „und natürlich mix also
   wieviel sie auf den mix anwendet des eq"; er gehört zusätzlich „ganz
   wichtig" in den Gen.
9. **Master-EQ: das Rad nicht neu erfinden.** Vorbild Pro-Q und Smooth
   Operator Pro; dynamische Bänder und alle gängigen Filtertypen; die
   Machbarkeits-Bewertung ist Sache der Technik.
10. **Die Überladen-Grenze wird Button für Button geklärt** — am
    aktuellen Probeeq-Design, „so halb halb bringt das nix".
11. **Durchschalt-Reihenfolge = FL-Mixer-Reihenfolge; der Master ist
    keine Rad-Position, sondern erhält ein eigenes UI-Element.**
12. **Durchschalter zeigt je Sonde nur den Bus-Namen** — sonst nichts.
13. **Draft-Kasten:** HOLD TO AUDITION = Halten; APPLY = erster Klick,
    derselbe Button wird zum Bestätigen mit sichtbarer Restzeit; REJECT
    bleibt daneben („stimmt so").
14. **Live-Spektrum hinter der Kurve: ja** — bei zwei Spuren nur das
    Spektrum der gerade bearbeiteten Spur.
15. **PROTECTED = harte Sperre für alle:** auch eigene Bänder nicht in
    die Zone, bis sie gelöst wird. Anlegen durch den User; Gen darf aus
    einem Befund eine Zone vorschlagen, angelegt erst per User-Klick.
16. **EQ-Mode und Bypass sind zwei Dinge** („ja korrekt so"): EQ-Punkt
    grün/rot als Grundschalter, Bypass als Hörvergleich. Rückfallfläche
    präzisiert: Connected/Disconnected · Bypass · bei EQ on die Werte +
    der Mode der Probe.
17. **Undo wird ein kleines Symbol auf dem Display** — der große Platz
    unten rechts ist „übertrieben"; dort sitzt stattdessen der Mix-Knob.
18. **Der Mix-Knob ist zweistufig:** er gilt der ganzen App UND jede
    Sonde braucht ihren eigenen Dry/Wet — ein Klick auf den Knob schaltet
    zwischen global und der gewählten Spur um.
19. **Die GLOBAL-Sektion ist einklappbar:** eingeklappt als Standard, ein
    kleiner Pfeil klappt sie aus („= platz").
20. **Draft-Kasten:** oben das Wort **„DRAFT"**, darunter die
    Klick/Hold-Elemente. **Die Limit-Angabe UND das Limit selbst
    entfallen** — „das kann im eq dann ja angepasst werden von selbst"
    (beantwortet U14; Register 23.08.).

## Angenommen (unbestätigt — Claude-Lesart, risikoarm)

- Mausrad-Konvention: ein markiertes Element besitzt das Rad (Band-Gain/Q
  wie in Pro-Q üblich); nur im „nichts markiert"-Zustand schaltet es Sonden.
- „das beste kopieren" meint Bedienlogik und Handling, nicht Optik — die
  visuelle Identität bleibt dein Figma (Designvertrag 21.08.).
- Die 10 Sekunden sind ein Startwert (änderbar wie alle Produktzahlen).
- Beim Speichern und Rendern zählt nur der bestätigte Stand; gehaltene
  Previews und nicht bestätigte Erst-Klicks werden nie persistiert.
- Rückfallfläche: **Bypass und Mix bedienbar**, die veränderten EQ-Werte
  nur **Anzeige** (bearbeitet wird in Gen).
- Mix wird eine neue versionierte Parameter-ID (Technik, NAK-64 Punkt 4).
- Smooth Operators Resonanzpeak-Fokus = spätere Ausbaustufe, nicht der
  erste Wurf.
- „Lösen" der Schutz-Zone = entfernen/deaktivieren über dasselbe Element,
  mit dem sie angelegt wurde.
- Ehrliche Grenze der harten Sperre: sie bindet Nakamas Bedienwege
  (Gen-Maus, Advisor, Fernweg) — FLs Host-Automation schreibt an der
  Sonden-Instanz vorbei und „gewinnt sichtbar" (Entwurf); die Zone zeigt
  eine solche Verletzung, verhindern kann sie sie dort nicht.
- „den mode der probe" auf der Rückfallfläche = die Messposition
  (PRE / INSERT / POST).
- Der globale Mix wirkt als zweite Schicht über dem Sonden-Mix
  (global × je Spur); die genaue Mechanik ist Technik (NAK-64 Punkt 4).
- Mit dem Limit fällt auch die alte Q-Sonderbegrenzung der
  Remote-Proposals (0,4–2): Drafts nutzen die manuellen Bereiche
  (Technik-Folge derselben Begründung).
- GLOBAL-Inhalt bleibt bei den vier Werten (nichts ergänzt); Undo-Klick =
  letzter Schritt zurück, ohne Menü.

## Offen (nächste Runden / Kennungen)

- **Undo-Reichweite:** gilt das kleine Undo je Sonde, oder EINE Kette
  über alles (Master + alle Sonden)? In Runde 4 nicht beantwortet.
- Die U2.8-Folgefrage (Undo-Form auf Gen **Seite 1**) bleibt davon
  unberührt offen.
- Rückfallflächen-Zuschnitt final erst, wenn der User sieht, was Gen je
  Sonde zeigt (sein Vorbehalt: „ich habe nicht im kopf was GEN anzeigt
  bezüglich des alten Suna konzeptes"). Leitfrage dafür: „was könnte den
  User veranlassen den probeeq zu öffnen statt gleich Gen?"
- U14 (±3-dB-Cap) · U5/NAK-33 (Sidechain sichtbar) · NAK-65 (Größe
  Seite 2, Figma-Flächen) · Undo-Form (U2.8-Folgefrage).
