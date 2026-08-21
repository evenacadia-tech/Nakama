> **ARCHIV (21.08.2026). Quell-Archiv; die lebende Fassung liegt in Projekte/Nakama-Design/docs. Keine Vorgabe mehr. Gültige Produktdefinition: CLAUDE.md (Wahrheitskern + Entscheide-Register); technischer Entwurf: docs/FL-Nakama-Sonden-Design-Entwurf.md.**

---
title: "UX/UI-Wissensbasis fuer Claude - Audio- und Musiksoftware"
document_type: "durable_knowledge_base"
language: "de"
audience: ["Claude", "Produktdesign", "UI-Entwicklung", "Audio-Software-Teams"]
scope: "UX-Strategie, UI-Gestaltung, Audio-Plugin-Workflows, visuelle Produktion"
source_snapshot: "2026-08-20"
status: "referenz"
companion: "./CLAUDE-UX-UI-ARBEITSKERN.md"
---

# UX/UI-Wissensbasis fuer Claude

> **Fuehrende Arbeitskopie seit 20.08.2026:**
> `C:\Users\phili\Projekte\Nakama-Design\docs\` — die UX-Arbeit laeuft im
> eigenstaendigen Design-Projekt; diese Fassung hier ist das Quell-Archiv.

Diese Datei ist eine dauerhafte, quellenkritische Wissensbasis fuer UX/UI-Arbeit an Audio- und Musiksoftware. Sie verdichtet sechs Webquellen und zwei Voger-Design-PDFs zu einem handlungsfaehigen Modell fuer Claude. Sie ist kein Stilrezept: Produktziel, reale Nutzerarbeit, technische Wahrheit und projektspezifische Regeln haben immer Vorrang.

## 1. So muss Claude dieses Dokument benutzen

### 1.1 Prioritaetsordnung

Bei jeder Designentscheidung gilt folgende Reihenfolge:

1. Explizite Absicht und freigegebene Vorstellung des Users.
2. Verifizierte Produktanforderungen, Nutzeraufgaben und technische Grenzen.
3. Bestehende Projektregeln, Designsysteme und bereits getroffene Entscheidungen.
4. Die in diesem Dokument formulierten UX/UI-Prinzipien.
5. Persoenlicher Geschmack, Trends und dekorative Ideen.

Eine allgemeine Regel darf niemals eine konkrete Produktwahrheit ueberschreiben. Wenn eine Quelle beispielsweise "drei Schriftgroessen" empfiehlt, ist das eine Reduktionsheuristik und keine universelle technische Grenze.

### 1.2 Drei Arten von Wissen auseinanderhalten

- **Quellenwissen:** Aussage oder Methode, die direkt aus einer der acht Quellen stammt.
- **Synthese:** belastbare Folgerung, die mehrere Quellen miteinander verbindet.
- **Projektentscheidung:** nur gueltig, wenn der User oder der aktuelle Projektvertrag sie bestaetigt.

Claude soll diese Ebenen in Begruendungen nicht vermischen. Eine Agenturpraeferenz ist kein Naturgesetz; eine attraktive Referenz ist kein Usability-Beweis; eine produktionsseitige Effizienz ist noch kein Nutzerwert.

### 1.3 Kernauftrag

Das Ziel ist nicht, eine komplizierte Profi-Anwendung oberflaechlich "einfach" aussehen zu lassen. Das Ziel ist:

- unnoetige Reibung zu entfernen,
- notwendige fachliche Tiefe lesbar zu organisieren,
- Absicht schnell in Handlung und hoerbares Ergebnis zu uebersetzen,
- Konzentration und kreativen Flow zu schuetzen,
- Status, Ursache und Wirkung ehrlich sichtbar zu machen,
- dem Produkt eine passende, eigenstaendige Persoenlichkeit zu geben.

## 2. Quellenkarte und Belastbarkeit

| ID | Quelle | Primaerer Beitrag | Einordnung |
|---|---|---|---|
| S1 | [UX Planet: Psychology Principles](https://uxplanet.org/the-psychology-principles-every-ui-ux-designer-needs-to-know-24116fd65778) | Von-Restorff-Effekt, serielle Position, kognitive Last, Hick, Naehe | Populaere Einfuehrung von 2017; als Heuristik nutzen, nicht als alleinigen Forschungsbeleg |
| S2 | [Resonant Design Manifesto](https://www.resonant.design/manifesto) | Domainverstaendnis, kreative Werkzeuge, Forschung, Profi-Komplexitaet, fruehe Designbeteiligung | Branchenbezogene Designposition; stark fuer Haltung und Prozess, nicht neutral-empirisch |
| S3 | [Voger: Make an Audio Plugin with Great UX](https://vogerdesign.com/blog/make-audio-plugin-with-great-ux/) | Einfachheit, Konsistenz, Feedback, Accessibility, Performance, positive und negative Beispiele | Praxisartikel mit Marketinganteil; Beispiele sind zeit- und produktabhaengig |
| S4 | [Voger: Three-Point Lighting in UI](https://vogerdesign.com/blog/understanding-three-point-lighting-in-ui-design/) | Key-, Fill- und Seiten-/Rim-Licht, Tiefe, Form, Materialwirkung | Visuelle Produktionsheuristik fuer realistische GUIs; nicht fuer jede Designsprache erforderlich |
| S5 | [Voger: Geometry Nodes for UI](https://vogerdesign.com/blog/geometry-nodes-a-way-to-ease-your-ui-creation-process/) | Prozedurale Assets, Animation, Varianten, nicht-destruktive Produktion | Produktionswissen; verbessert UX nur indirekt ueber Konsistenz, Iteration und Performance |
| S6 | [Voger: Crafting Exceptional Audio Plugin Experiences](https://vogerdesign.com/blog/crafting-exceptional-audio-plugin-experiences/) | Zweck, Zielgruppe, Workflow, visuelle Konsistenz, Responsive Design, Feedback, Anpassbarkeit | Breite Praxisempfehlungen; einzelne Aussagen muessen gegen reale Plattformgrenzen geprueft werden |
| S7 | `Project-Brief-by-Voger-Design.pdf`, 5 Seiten | Briefing, Zielgruppe, Differenzierung, Technik, Stil, Zusammenarbeit, Erfolgskriterien | Nuetzliche Projektvorlage; Beispiele und Zahlen sind Platzhalter oder kontextgebunden |
| S8 | `GUI-Tips.pdf`, 6 Seiten | Gefuehl, UX-vor-UI, Raster, Typografie, Licht, Materialien | Kompakte visuelle Checkliste; mehrere Regeln sind bewusste Vereinfachungen |

Lokale PDF-Provenienz zum Zeitpunkt der Auswertung:

- S7: `C:\Users\phili\Downloads\Assets\Project-Brief-by-Voger-Design.pdf`; SHA-256 `32F01D5AFD0FB2B4FA11FF05E44CFE0EB1E8937A1DB3C1603EDC8B7C8C950262`.
- S8: `C:\Users\phili\Downloads\Assets\GUI-Tips.pdf`; SHA-256 `E54802F538DD36F941D7D7102FD245ECE4C00452CE0E62F71A7DD43355947C11`.

Die Webquellen wurden am 20. August 2026 mit Crawlberg als LLM-optimierter Artikeltext extrahiert. Beide PDFs wurden vollstaendig textuell ausgelesen und seitenweise gerendert; die visuellen Beispiele wurden in die Synthese einbezogen.

Die Quellen stimmen in ihrem wichtigsten Punkt ueberein: Gute Oberflaechen beginnen nicht bei Farben oder 3D-Assets, sondern bei Zweck, Nutzer, Arbeitsfluss und Hierarchie. Die visuelle Ebene traegt danach Klarheit, Charakter und emotionale Richtung auf dieses Fundament auf.

## 3. Das Vier-Ebenen-Modell

| Ebene | Leitfrage | Ergebnis | Typischer Fehler |
|---|---|---|---|
| 1. Wirkung | Welches Ergebnis und welches Gefuehl soll entstehen? | Produktzweck, Erfolgskriterien, 2-3 Gefuehlsattribute | Eine Stilrichtung waehlen, bevor das Problem klar ist |
| 2. Arbeit | Wer tut in welchem Kontext was? | Nutzersegmente, Kernaufgaben, Signal-/Datenfluss, Prioritaeten | Eine Consumer-App-Schablone auf Profi-Software legen |
| 3. Wahrnehmung | Wie erkennt der User Struktur, Zustand und Wirkung? | Hierarchie, Gruppierung, Beschriftung, Feedback, Zustaende | Alle Funktionen gleich stark darstellen oder Komplexitaet nur verstecken |
| 4. Koerper | Wie fuehlt sich das Produkt visuell und in Bewegung an? | Typografie, Farbe, Licht, Material, Motion, Designsystem | Dekoration ohne funktionale oder emotionale Begruendung |

Claude muss die Ebenen in dieser Reihenfolge bearbeiten. Rueckspruenge sind erlaubt, aber Politur darf keine ungeklärte Aufgabe kaschieren.

## 4. Leitbild fuer Audio- und Musiksoftware

### 4.1 Das Werkzeug soll in der Arbeit verschwinden

Ein gutes kreatives Werkzeug beansprucht Aufmerksamkeit nur dann, wenn eine Entscheidung ansteht. Es uebersetzt Absicht in Klang, statt den User mit Navigation, Modalfenstern oder unklaren Zustaenden aus seiner Arbeit zu reissen. "Unsichtbar" bedeutet dabei nicht visuell leer. Es bedeutet, dass Bedienlogik, Rueckmeldung und mentale Modelle so gut zusammenpassen, dass die Oberflaeche nicht zum Hindernis wird.

### 4.2 Professionelle Tiefe ist nicht der Feind

Musiksoftware ist oft notwendig komplex. Ein Mastering-Engineer, ein Live-Performer, ein Sounddesigner und ein Einsteiger bringen verschiedene Ziele, Sprachen und Risikotoleranzen mit. Gute UX:

- entfernt **extrinsische** Schwierigkeit wie Suche, Inkonsistenz oder unklare Labels,
- erhaelt **fachlich notwendige** Kontrolle,
- staffelt Tiefe nach Aufgabe und Expertise,
- laesst Power-User schnell arbeiten,
- gibt Einsteigern Orientierung, ohne sie dauerhaft in einem vereinfachten Modus einzusperren.

"Weniger" ist nur dann besser, wenn weniger Reibung entsteht. Das blosse Verstecken wichtiger Parameter verschiebt Reibung in Menues und Erinnerungsarbeit.

### 4.3 Domainmuster zuerst verstehen

Consumer-Muster duerfen nicht ungeprueft importiert werden. Vor einer neuen Interaktion muss Claude klaeren:

- Welches mentale Modell ist in DAW, Host oder Geraeteklasse bereits etabliert?
- Welche Konvention beschleunigt Experten wirklich?
- Welche Konvention ist nur historischer Ballast?
- Was muss wegen Rueckwaertskompatibilitaet stabil bleiben?
- Ist die Aufgabe explorativ, performativ oder chirurgisch-praezise?
- Welche Information braucht der User waehrend des Hoerens und welche erst bei Analyse oder Einrichtung?

Abweichungen von Konventionen brauchen einen nachweisbaren Gewinn und eine klare Lernbruecke.

## 5. Wahrnehmungspsychologie als Werkzeug

### 5.1 Von-Restorff-Effekt: Unterschied erzeugt Erinnerung

Ein Element, das sich deutlich von einer Gruppe aehnlicher Elemente unterscheidet, wird eher bemerkt und erinnert.

**Geeignet fuer:**

- den primaeren naechsten Schritt,
- einen bewusst ausgeloesten kreativen "Hero Moment",
- einen kritischen oder ungewoehnlichen Zustand,
- das aktuell aktive Werkzeug oder Ziel.

**Regeln:**

- Akzent ist ein knappes Gut. Wenn alles leuchtet, ist nichts hervorgehoben.
- Semantische Bedeutung muss konsistent bleiben: dieselbe Akzentfarbe darf nicht einmal "primaer", einmal "Warnung" und einmal nur Schmuck bedeuten.
- Differenz nicht nur durch Farbe erzeugen; Form, Position, Groesse, Beschriftung und Bewegung koennen redundante Signale liefern.
- Eine CTA-Hervorhebung ersetzt keine verstaendliche Bezeichnung.

### 5.2 Serieller Positionseffekt: Anfang und Ende bleiben haengen

Menschen erinnern in einer Reihe haeufig die ersten und letzten Eintraege besser. Daraus folgt kein pauschales Gebot fuer eine bestimmte Navigation, aber eine brauchbare Priorisierungsregel:

- kritische Orientierung und haeufigste Aktion an stabile, leicht auffindbare Positionen setzen,
- lange Reihen nicht als einzige Informationsstruktur verwenden,
- mittlere Eintraege durch Gruppierung, Zwischenueberschriften oder visuelle Anker entlasten,
- Reihenfolge nach Aufgabe statt nach interner Implementierung bestimmen.

### 5.3 Kognitive Last: notwendige Tiefe von unnoetiger Last trennen

- **Intrinsische Last:** Die Sache selbst ist anspruchsvoll, etwa dynamische EQ-Entscheidungen oder komplexes Routing. Sie wird durch gute Erklaerung und Stufung handhabbar, aber nicht wegdekoriert.
- **Extrinsische Last:** Unklare Begriffe, wechselnde Anordnung, ueberladene Anzeigen, versteckte Abhaengigkeiten oder unnoetige Schritte. Diese Last muss reduziert werden.
- **Lernfoerdernde Last:** Der User bildet ein belastbares mentales Modell. Konsistente Muster, sinnvolle Visualisierung und erklaerende Rueckmeldung helfen dabei.

**Konkrete Mittel:**

- kurze, aufgabenbezogene Microcopy,
- fachsprachlich richtige Labels statt kreativer Raetselbegriffe,
- gute Defaults und nachvollziehbare Presets,
- Wiedererkennung durch stabile Muster,
- kontextnahe Erklaerung statt globalem Handbuchzwang,
- progressive Offenlegung, wenn sie den Hauptweg tatsaechlich entlastet.

### 5.4 Hick's Law: Mehr Wahl kostet Entscheidungszeit

Viele gleichrangige Optionen verlaengern die Entscheidung. Claude soll deshalb nicht blind Funktionen entfernen, sondern die Wahl architektonisch verbessern:

- Optionen nach Nutzerziel gruppieren,
- einen sinnvollen Standard anbieten,
- seltene Entscheidungen aus dem Hauptpfad nehmen,
- eine grosse Wahl in nachvollziehbare Stufen zerlegen,
- Such-, Vergleichs- oder Preview-Hilfen geben,
- die aktuelle Auswahl und ihre Wirkung sichtbar halten.

Ein einzelner "Advanced"-Schalter ist keine gute progressive Offenlegung, wenn er eine unstrukturierte Parameterwand oeffnet.

### 5.5 Gesetz der Naehe: Abstand ist Bedeutung

Elemente, die nah beieinander stehen, werden als zusammengehoerig gelesen. Darum muss Spacing semantisch sein:

- Abstand innerhalb einer Funktionsgruppe kleiner als Abstand zwischen Gruppen,
- Label, Wert, Einheit und Steuerung eindeutig koppeln,
- gekoppelte Signalwege sichtbar zusammenhalten,
- gefaehrliche oder irreversible Aktionen bewusst von Routineaktionen trennen,
- leere Flaeche als Strukturmittel verwenden, nicht als Restflaeche.

Ein Rahmen kann Gruppierung verstaerken, sollte aber eine schlechte Abstandslogik nicht kompensieren.

## 6. Briefing: Die Qualitaet des Inputs bestimmt die Qualitaet des Designs

Der Voger-Projektbrief zeigt, welche Informationen vor ernsthafter UI-Arbeit geklaert werden sollten. Claude soll fehlende Angaben nicht mit beliebiger "Kreativitaet" fuellen. Es soll entweder eine risikoarme Annahme sichtbar machen oder eine gezielte Frage stellen, wenn die Antwort den Entwurf wesentlich veraendern wuerde.

### 6.1 Minimaler Designvertrag

#### Produkt und Wirkung

- Was ist das Produkt in einem Satz?
- Welches konkrete Problem loest es?
- Welcher Moment oder welche Funktion ist der Kernnutzen?
- Was soll der User nach der Nutzung koennen, wissen oder fuehlen?
- Welche 1-3 Kriterien entscheiden, ob das Design erfolgreich ist?

#### Nutzer und Kontext

- Primaeres Nutzersegment; weitere relevante Segmente.
- Erfahrungsniveau und verwendete Fachsprache.
- Nutzungskontext: Studio, Live, unterwegs, Einrichtung, Analyse, wiederholte Routine.
- Haeufigste, kritischste und fehleranfaelligste Aufgaben.
- Bestehende mentale Modelle, Host-Konventionen und Konkurrenzprodukte.

#### Positionierung

- Was macht die Konkurrenz kompliziert, langsam oder austauschbar?
- Welcher Unterschied soll sofort spuerbar werden?
- Was ist der "Hero Moment" des Produkts?
- Welche Markenattribute sind verbindlich?

#### Technik

- UI-Framework und Assetgrenzen: Vektor, Raster, 3D, Shader, Animation.
- Zielgroessen, Resize-Verhalten, Skalierungsstufen und DPI-Szenarien.
- Plattformen, Hosts, Eingabemethoden und Performancebudget.
- Animations- und Framegrenzen.
- Hell-/Dunkelvarianten und Farbraumgrenzen.
- Handoff- und Lieferformate.

#### Visuelle Richtung

- Maximal 2-3 gewuenschte Gefuehlsattribute.
- Zwei bis drei Kernreferenzen, jeweils mit begruendetem Gefallen.
- Antireferenzen mit konkretem Ablehnungsgrund.
- Materialien, Licht, Temperatur, Zeit-/Ortsassoziation und Markencharakter.
- Grad der kreativen Autorenschaft: Vision umsetzen, Richtung gemeinsam formen oder innerhalb eines klaren Kerns frei arbeiten.

#### Zusammenarbeit und Scope

- Entscheider, Umsetzer und technische Ansprechpartner.
- Kommunikationskanal und erwartete Feedbackzeit.
- Releaseziel, Review-Runden, Rechte und Lieferumfang.
- Was bedeutet "dev-ready" in diesem Projekt konkret?

### 6.2 Gute Gefuehlsrichtung

"Modern" oder "cool" ist zu leer. Eine brauchbare Richtung verbindet 2-3 Attribute mit wahrnehmbaren Konsequenzen.

| Attribut | Layout | Typografie | Licht/Farbe | Material/Motion |
|---|---|---|---|---|
| praezise | strenges Raster, klare Wertebeziehungen | ruhige Ziffern, hohe Lesbarkeit | kontrollierter Kontrast, wenige Akzente | definierte Kanten, kurze direkte Bewegung |
| verspielt | explorative, aber stabile Hauptstruktur | charaktervolle Akzentschrift sparsam | lebendige, semantisch begrenzte Akzente | elastische Details, unmittelbares Feedback |
| handwerklich | sichtbare Ordnung mit individuellen Details | warme, nicht sterile Anmutung | gerichtetes, weiches Licht | glaubhafte Oberflaechen, kleine Unregelmaessigkeit |

Die Tabelle ist ein Uebersetzungsmuster, keine Stilvorgabe. Claude muss fuer jedes Projekt eigene Zusammenhaenge begruenden.

## 7. UX vor UI: Der belastbare Arbeitsablauf

### Stufe 1 - Zweck und reale Arbeit verstehen

**Aktivitaeten:** Research, Interviews oder vorhandene Evidenz lesen; Kernaufgaben und Probleme kartieren; Nutzersegmente und Kontexte unterscheiden.

**Ergebnis:** Problemformulierung, Zielgruppe, Erfolgskriterien, priorisierte Aufgaben.

**Gate:** Kein visueller Stilentwurf, solange nicht klar ist, welche Arbeit die Oberflaeche besser machen soll.

### Stufe 2 - System und Zustaende modellieren

**Aktivitaeten:** Funktionsinventar, Informationsarchitektur, Signal-/Datenfluss, Abhaengigkeiten, Zustands- und Fehlerpfade.

**Ergebnis:** Taskflow, Zustandsmatrix, Funktionshierarchie, technische Risiken.

**Gate:** Jeder sichtbare Wert braucht eine Quelle; jede Aktion braucht Rueckmeldung und einen Gegen-/Abbruchpfad, wo sinnvoll.

### Stufe 3 - Wireframe

Der Wireframe prueft Anordnung, Kontrolle, Beschriftung, Reihenfolge und Interaktion ohne den Schutz visueller Politur.

**Ergebnis:** Graustufige Hauptzustaende und Kernpfade, inklusive leerer, ladender, fehlerhafter und nicht verfuegbarer Zustaende.

**Gate:** Ein User muss die Hauptaufgabe und den Systemzustand ohne dekorative Hinweise erkennen koennen.

### Stufe 4 - Fruehe visuelle Ebene

Kontrast, Gruppierung, Typografie und erste Stilhinweise werden hinzugefuegt. Die Richtung soll Klarheit und Emotion unterstuetzen, ohne bereits teure Detailproduktion festzuschreiben.

**Ergebnis:** Wenige Varianten mit expliziten Hypothesen, nicht viele austauschbare Moodboards.

**Gate:** Jede Variante benennt, welches Nutzer- oder Markenproblem sie besser loest.

### Stufe 5 - UI-System und Politur

Jetzt entstehen Design Tokens, Komponenten, Licht-/Materiallogik, Motion und responsive Regeln.

**Ergebnis:** Dev-taugliche Spezifikation, Komponenten- und Zustandsbibliothek, Assetplan.

**Gate:** Visuelle Wirkung darf Labels, Werte, Fokus, Interaktion oder Performance nicht schwaechen.

### Stufe 6 - Prototyp und reale Pruefung

Mit echten Aufgaben und realistischen Daten testen. Portfolio-Screenshots beweisen weder Bedienbarkeit noch Verhalten.

**Zu pruefen:**

- Findet der User den ersten sinnvollen Schritt?
- Kann er Ursache und Wirkung einer Aenderung zuordnen?
- Bleibt der Arbeitsfluss bei haeufigen Aufgaben schnell?
- Erholt sich die Oberflaeche verstaendlich von Fehlern?
- Funktioniert sie in kleinem Fenster, hoher Skalierung und relevantem Host?
- Bleiben Status und Interaktion unter Last glaubwuerdig?

### Stufe 7 - Handoff und Lernschleife

Design ist nicht mit einer statischen Datei abgeschlossen. Handoff umfasst Verhalten, Datenquellen, Grenzwerte, Zustaende, Assetformate, Animationstiming und Abnahmekriterien. Nach Implementierung folgen visueller Vergleich und echte Interaktionstests.

## 8. Layout, Hierarchie und Raster

### 8.1 Eine dominante Leserichtung

Die GUI-Tips empfehlen eine klare Hierarchie, etwa von oben nach unten oder entlang einer konsistenten horizontalen Richtung. Fuer Claude bedeutet das:

- eine dominante Scan- und Arbeitsreihenfolge festlegen,
- Signalfluss und Taskflow nicht gegeneinander laufen lassen,
- primaere, sekundaere und tertiaere Kontrollen sichtbar unterscheiden,
- Werte und Einheiten dort zeigen, wo die Hand-Auge-Aufgabe stattfindet,
- haeufige Aktionen an stabile, gut erreichbare Orte setzen.

### 8.2 Modularer Grid statt Rasterreligion

Ein unsichtbares Raster verbessert Ausrichtung, Proportion und Wiederholbarkeit. Die Quelle nennt Golden-Ratio-Hilfen in Figma und Blender. Daraus folgt nicht, dass der Goldene Schnitt jede UI verbessert.

**Robuste Regel:** Waehle das einfachste Raster, das Gruppen, Groessenverhaeltnisse, Resize und Entwicklerumsetzung konsistent traegt.

Moegliche Bausteine:

- Baseline-/Spacing-Raster,
- modulare Spalten und Zeilen,
- feste Kontrollgroessen plus flexible Inhaltsbereiche,
- optische Korrekturen fuer runde Controls und Schrift,
- Safe Areas fuer Host-Rahmen, Skalierung und variable Texte.

### 8.3 Visuelle Gewichtung

Groesse, Kontrast, Position, Freiraum, Bewegung und Detailgrad erzeugen Hierarchie. Sie muessen Funktionsprioritaet ausdruecken. Ein grosser Regler ist ein Versprechen: Er sagt, dass dieser Parameter wichtig, haeufig oder identitaetsstiftend ist. Wenn das nicht stimmt, ist die Hierarchie unehrlich.

## 9. Controls, Feedback und Zustaende

### 9.1 Natuerliche Platzierung

Controls sollen dort stehen, wo Nutzer sie aufgrund von Aufgabe, Signalfluss und Konvention erwarten. "Intuitiv" ist kein Stilwort, sondern die beobachtbare Uebereinstimmung zwischen Erwartung und Verhalten.

### 9.2 Sofortige, proportionale Rueckmeldung

Nach einer Aktion muss der User erkennen:

- dass die Eingabe registriert wurde,
- welcher Wert oder Zustand sich geaendert hat,
- ob die Aenderung noch laeuft, abgeschlossen oder fehlgeschlagen ist,
- welche akustische oder systemische Wirkung zu erwarten ist,
- ob eine Grenze, Automation oder Fremdsteuerung eingreift.

Rueckmeldung soll proportional sein. Eine kleine Parameterbewegung braucht keine raumgreifende Animation; ein gefaehrlicher globaler Wechsel darf nicht nur durch einen subtilen Farbton signalisiert werden.

### 9.3 Empfohlene Zustandsmatrix fuer Audio-Controls

Nicht jeder Control braucht jeden Zustand, aber Claude muss bewusst pruefen:

- Standard,
- Hover oder Vorfokus,
- Tastatur-/Accessibility-Fokus,
- gedrueckt/gezogen,
- geaendert gegenueber Default,
- automatisiert oder extern bewegt,
- gesperrt/verknuepft,
- deaktiviert mit erkennbarem Grund,
- begrenzt/geclippt,
- ladend oder Daten fehlen,
- Fehler oder Verbindung verloren,
- Bypass/Offline,
- Zuruecksetzen/Undo.

### 9.4 Visualisierungen muessen eine Frage beantworten

Meter, Spektren, Wellenformen und Animationen sind sinnvoll, wenn sie Entscheidung, Orientierung oder Vertrauen verbessern. Vor jeder Visualisierung fragt Claude:

- Welche konkrete Frage beantwortet sie?
- In welchem Zeitraum?
- Welche Handlung folgt aus dem Gesehenen?
- Was bedeutet keine Aktivitaet?
- Wie verhindert sie falsche Sicherheit oder visuelle Ueberforderung?

Eine bewegte Anzeige ohne handlungsrelevante Semantik verbraucht Aufmerksamkeit und kann kreativen Flow verschlechtern.

## 10. Typografie

Die GUI-Tips setzen Lesbarkeit an erste Stelle und empfehlen eine kleine Zahl von Schriften und semantischen Groessen. Die belastbare Uebersetzung lautet:

- eine gut lesbare UI-Schrift als Basis,
- hoechstens eine zusaetzliche charaktervolle Schrift, wenn sie einen klaren Markenjob hat,
- wenige semantische Textrollen wie Heading, Label, Wert, Hilfetext,
- stabiler Zahlenstand fuer Messwerte,
- eindeutige Einheiten und Vorzeichen,
- ausreichender Kontrast und Abstand,
- Tests in realer Fenstergroesse, DPI-Skalierung und Hostumgebung.

Die im PDF genannten Mindestwerte "Kontakt 12 px" und "VST 25 px" sind keine universellen Standards. Pixelwerte sind ohne Betrachtungsabstand, Skalierung, Exportauflösung und Hostkontext nicht uebertragbar. Claude soll physische Lesbarkeit und reale Zielgeraete pruefen, nicht eine isolierte Zahl kopieren.

Beispiele fuer in der Quelle genannte Paarungen sind Montserrat/Bebas Neue, Proxima Nova/Roboto, Helvetica/Open Sans sowie Oxygen/Oswald. Diese Namen sind Referenzen, keine Defaultauswahl. Lizenz, Zeichensatz, Ziffernqualitaet, Rendering und Markenpassung entscheiden.

## 11. Licht als visuelle Grammatik

### 11.1 Drei-Punkt-Logik

Der Voger-Artikel uebertraegt klassische Drei-Punkt-Beleuchtung auf realistische UI-Elemente:

- **Key Light:** dominante Richtung; definiert Hauptform, Fokus, Highlights und Schatten.
- **Fill Light:** schwächeres Gegenlicht; oeffnet Schatten und erhaelt Details.
- **Rim-/Back-/Seitenlicht:** trennt Kanten, fuegt Reflexe hinzu und macht Material lesbar. Der Artikel nennt diese Rolle teilweise "shadow light".

### 11.2 Ein konsistentes Lichtfeld

Claude muss fuer eine realistische oder materialbetonte UI eine kleine "Lichtverfassung" festlegen:

- Richtung und Hoehe des Key Lights,
- Verhaeltnis von Key zu Fill,
- Schattenhaerte und Kontaktverschattung,
- Farbtemperatur der Lichtquellen,
- Verhalten auf Metall, Kunststoff, Glas und Papier,
- Regeln fuer aktive, inaktive und fokussierte Zustaende.

Einzelne Komponenten duerfen nicht so wirken, als kaeme ihr Licht aus widerspruechlichen Welten. Zufalls-Highlights und beliebige Schlagschatten zerstoeren Materialglaubwuerdigkeit.

### 11.3 Lichttypen und Wirkung

- **Weiches Licht:** sanfte Schatten, ruhige Waerme, freundliche Volumenwirkung.
- **Hartes Licht:** klare Kanten, technische oder dramatische Praezision; schnell anstrengend bei Uebertreibung.
- **Invertiertes Licht:** ungewohnte Richtung oder Negativwirkung; charakterstark, aber erklaerungsbeduerftig.
- **Farbiges Licht:** Atmosphaere und Fokus; muss mit semantischen Statusfarben vereinbar bleiben.

Die GUI-Tips empfehlen Nuancen statt reinem Schwarz und Weiss. Das ist eine Material- und Stimmungsheuristik, keine Accessibility-Regel. Nahe Schwarz-/Weisstoene koennen realistischer und angenehmer wirken; fuer Text, Fokus und kritische Signale muss der effektive Kontrast dennoch geprueft werden.

### 11.4 Wann Drei-Punkt-Licht sinnvoll ist

Gut geeignet, wenn:

- die Oberflaeche reale Geraete, Materialien oder Bedienelemente bewusst evoziert,
- ein zentraler Control als Produktgesicht fungiert,
- Form und Material durch Licht lesbarer werden,
- eine konsistente 3D-Produktion und ein Performancebudget existieren.

Weniger geeignet, wenn:

- die UI datenreich und flach-informativ sein muss,
- Licht die Statusfarben oder Zahlenlesbarkeit konkurrenziert,
- Schatten falsche Affordanzen erzeugen,
- die Darstellung bei Resize oder Themewechsel bricht,
- Realismus nur als oberflaechliche Nostalgie dient.

## 12. Materialien: "beruehrbar" heisst glaubhaft und bedienbar

Die GUI-Tips unterscheiden beispielhaft Kunststoff, Metall, Glas und Papier. Materialwahl soll die Atmosphaere und den gewuenschten Eindruck unterstuetzen.

### 12.1 Material wird durch Beziehungen sichtbar

Ein Material ist nicht nur eine Textur. Es entsteht aus:

- Reflexionsbreite und -staerke,
- Rauheit,
- Kantenreaktion,
- Lichtdurchlaessigkeit oder Opazitaet,
- Mikrostruktur und Massstab,
- Kontakt zum Untergrund,
- Reaktion auf Bewegung und Interaktionszustand.

Metall ohne passende Reflexion liest sich als grauer Kunststoff. Glas ohne nachvollziehbaren Hintergrund, Kante und Transmission liest sich als blauer Verlauf. Papier ohne Massstab und Lichtbezug liest sich als Rauschen.

### 12.2 Material und Affordanz verbinden

"Touchable" bedeutet operativ:

- Ein drehbares Element sieht greifbar und rotationsfaehig aus.
- Eine Vertiefung oder Spur erklaert Bewegungsrichtung.
- Gedrueckte, aktive und gesperrte Zustaende bleiben unterscheidbar.
- Materialdetail verbessert Formwahrnehmung, nicht bloss Dekoration.
- Die Hitbox entspricht der sichtbaren Erwartung.

Material darf nie wichtiger werden als Wert, Label, Fokus und Ursache-Wirkung-Beziehung.

### 12.3 Kohärenzregel

Waehle eine kleine Materialfamilie mit Rollen. Beispiel: matte Grundflaeche, praezise Metallcontrols, Glas nur fuer eine spezifische Anzeigeebene. Jede weitere Materialart braucht einen semantischen oder markenbezogenen Grund.

## 13. Geometry Nodes als Produktionssystem

Geometry Nodes sind laut Voger ein blockbasiertes, prozedurales System in Blender fuer Geometrie, Kurven und Animation. Der Nutzen liegt nicht automatisch in besserer UX, sondern in einem besseren Herstellungs- und Iterationsprozess.

### 13.1 Geeignete Anwendungen

- wiederholte oder variierte Controls,
- prozedurale Muster, Raster und Hintergruende,
- animierte Text-/Zahlenobjekte,
- parametrische Slider-, Level- oder Linienanimationen,
- abstandsabhaengige Reaktionen,
- Kurven mit dynamischen Endformen,
- zentral gesteuerte Knob-Varianten,
- konsistente Assetfamilien fuer mehrere Aufloesungen.

### 13.2 Vorteile

- Parameter sind zentral gekoppelt.
- Varianten bleiben kohaerent.
- Aenderungen sind nicht-destruktiv.
- Echtzeit-Feedback beschleunigt Iteration.
- Wiederholungsarbeit skaliert besser mit wachsender Assetzahl.
- Animation und Geometrie koennen aus denselben Regeln entstehen.

### 13.3 Kosten und Risiken

- mathematische und technische Komplexitaet,
- aufwendige Materialzuweisung,
- schwerere Uebergabe an ein Team ohne Blender-/Node-Erfahrung,
- Gefahr, eine elegante Produktionsloesung mit einem Nutzerproblem zu verwechseln,
- potenziell hohe Render-, Speicher- oder Runtimekosten bei unpassendem Export.

### 13.4 Entscheidungsregel

Geometry Nodes einsetzen, wenn mehrere Assets oder Zustaende dieselbe veraenderbare Grammatik teilen und voraussichtlich iteriert werden. Fuer zwei einmalige, stabile Objekte kann manuelle Arbeit schneller und wartbarer sein. Vorher Exportpfad, Sprite-/Vektorbedarf, Farbraum, Skalierung und Animationsbudget klaeren.

## 14. Accessibility, Responsive Design und Performance

Die Voger-Quellen nennen alle drei als Kerneigenschaften guter Plugin-UX. Claude muss sie als Produktqualitaet behandeln, nicht als spaete Zusatzrunde.

### 14.1 Accessibility

- Information nie ausschliesslich ueber Farbe vermitteln.
- Fokus und aktuelle Auswahl klar sichtbar machen.
- Labels und Werte auch bei kleinen Fenstern lesbar halten.
- Bedienziele nicht kleiner machen, als praezise Eingabe erlaubt.
- Tastatur, Screenreader-/Semantikmoeglichkeiten und alternative Eingaben im verwendeten Framework frueh pruefen.
- Bewegungsintensitaet und blinkende Rueckmeldung begrenzen; reduzierte Bewegung ermoeglichen, wo technisch sinnvoll.
- Deaktivierung erklaerbar machen; ein blassgraues Element ohne Grund erzeugt Unsicherheit.

### 14.2 Responsive und plattformuebergreifend

Responsive bedeutet bei Plugins nicht automatisch Web-Fluiditaet. Moegliche Strategien sind feste Stufen, proportionale Skalierung, umgebrochene Bereiche oder adaptive Detailstufen. Fuer jede Zielgroesse muss definiert sein:

- was erhalten bleibt,
- was umgeordnet wird,
- was verdichtet oder verborgen werden darf,
- wie Schrift und Hitboxes skalieren,
- wie Rasterassets oder 3D-Frames scharf bleiben,
- welche Host-/OS-Skalierungen getestet werden.

### 14.3 Performance

Schnelle visuelle Rueckmeldung ist Teil der UX. Die UI darf den Audiopfad nicht gefaehrden und soll unter Last keine falschen Zustaende zeigen. Claude muss bei bewegten oder prozeduralen Elementen nach Datenkadenz, Paint-Kosten, Assetgroesse, GPU-/CPU-Budget und Degradationsverhalten fragen.

Eine fluessige dekorative Animation ist kein Erfolg, wenn Controls verzoegert reagieren oder Messwerte unehrlich geglaettet werden.

## 15. Personalisierung und Anpassbarkeit

Voger hebt anpassbare Layouts hervor. Anpassbarkeit ist sinnvoll, wenn Nutzer tatsaechlich unterschiedliche stabile Workflows haben. Sie erzeugt jedoch auch Kosten:

- mehr Zustaende und Persistenzlogik,
- schwierigere Dokumentation und Support,
- Gefahr inkonsistenter Layouts,
- komplexere Tests.

Darum zuerst einen starken Default bauen. Personalisierung nur fuer nachgewiesene Unterschiede anbieten, etwa kompakte/erweiterte Ansicht, skalierbare Anzeige, anpassbare Messdarstellung oder gespeicherte Arbeitsbereiche. Jede Option braucht einen Ruecksetzpfad.

## 16. Forschung, Prototyping und Messung

Das Resonant-Manifest fordert, Annahmen mit realen Nutzern zu pruefen und Design frueh in Produktentscheidungen einzubeziehen. Claude soll bei Tests nicht nur fragen, ob etwas "gefaellt".

### 16.1 Beobachtbare Kriterien

- Zeit bis zur ersten korrekten Aktion.
- Zahl und Art von Fehlversuchen.
- Verstaendnis von Systemzustand und Wirkung.
- Unterbrechungen im Kernworkflow.
- Sicherheit vor einer kritischen Aktion.
- Wiederauffindbarkeit nach einer Pause.
- Geschwindigkeit wiederholter Expertenarbeit.
- Qualitaet der Uebergabe zwischen Plugin, Host und weiteren Werkzeugen.

### 16.2 Testfaelle

- Erster Start ohne Vorwissen.
- Haeufigster Routinepfad.
- Komplexer Expertenpfad.
- Fehler, fehlende Daten oder Verbindungsausfall.
- Resize, hohe Skalierung und kleines Fenster.
- Automation oder externe Parameterbewegung.
- Bypass, Presetwechsel, Undo/Reset und Wiederherstellung.
- Nutzung unter Zeitdruck oder waehrend konzentriertem Hoeren.

### 16.3 Visuelles Urteil getrennt pruefen

Usability, technische Pixelkorrektheit, Markenpassung und aesthetische Qualitaet sind verschiedene Beweise. Ein Test kann eine Ebene bestaetigen und eine andere offenlassen. Claude muss genau benennen, was tatsaechlich geprueft wurde.

## 17. Anti-Patterns

### "Clean" durch Verstecken

Symptom: Die Hauptansicht wirkt leer, aber jeder reale Arbeitsgang oeffnet Menues. Gegenmittel: Funktionshierarchie und progressive Offenlegung anhand realer Aufgaben neu ordnen.

### Gleichrangige Kontrollwand

Symptom: Alle Knobs, Labels und Sektionen haben dasselbe Gewicht. Gegenmittel: Haeufigkeit, Kritikalitaet, Signalfluss und Hero-Funktion sichtbar priorisieren.

### Akzentinflation

Symptom: Viele Farben, Glows oder Animationen konkurrieren. Gegenmittel: semantische Akzentrollen und ein klarer Aufmerksamkeitsbudget festlegen.

### Dekoratives Metering

Symptom: Bewegung sieht technisch aus, beantwortet aber keine Nutzerfrage. Gegenmittel: Messziel, Zeitfenster, Referenz und Folgehandlung definieren oder Anzeige entfernen.

### Inkonsistente Physik

Symptom: Schatten, Highlights und Materialien folgen keiner gemeinsamen Lichtwelt. Gegenmittel: Licht- und Materialverfassung vor Assetproduktion festlegen.

### Skeuomorphie ohne Bediennutzen

Symptom: Hardwaredetails nehmen Platz ein, verbessern aber weder Affordanz noch Markenwirkung. Gegenmittel: jedes reale Detail auf Funktion, Orientierung oder Emotion pruefen.

### Einsteiger gegen Experten ausspielen

Symptom: Entweder eine unerklaerte Parameterwand oder eine dauerhaft kastrierte Simple-Ansicht. Gegenmittel: gemeinsame Grundlogik, gute Defaults, kontextuelle Hilfe und nachvollziehbare Vertiefung.

### Zahlen aus Quellen als Gesetz

Symptom: Schriftgroesse, Raster oder Framezahl werden ohne Zielkontext uebernommen. Gegenmittel: reale Anzeige, Plattform, Performance und Nutzeraufgabe testen.

### Produktionsinnovation als UX-Argument

Symptom: Geometry Nodes, 3D oder Shader werden eingesetzt, weil der Prozess interessant ist. Gegenmittel: zuerst den sichtbaren Nutzerwert und die Wartungskosten benennen.

## 18. Arbeitsvertrag fuer Claude

Wenn Claude eine UX/UI-Aufgabe bearbeitet, soll sein Ergebnis mindestens folgende Punkte enthalten, soweit fuer den Auftrag relevant:

1. **Ziel:** Nutzer, Aufgabe, Kontext und Erfolg in wenigen Saetzen.
2. **Evidenz:** gelesene Quellen, existierende UI, reale Daten oder explizite Annahmen.
3. **Hierarchie:** primaere, sekundaere und seltene Funktionen.
4. **Flow:** Hauptpfad plus Abbruch-, Fehler- und Wiederherstellungspfad.
5. **Zustaende:** sichtbare Zustandsmatrix fuer relevante Controls und Flaechen.
6. **Visuelle Grammatik:** Raster, Typorollen, Farbrollen, Licht, Material und Motion mit Begruendung.
7. **Technik:** Groessen, Skalierung, Assetformat, Performance und Frameworkgrenzen.
8. **Pruefung:** konkrete Aufgaben und sichtbare Abnahmekriterien.
9. **Offenes:** Unsicherheiten, die das Resultat noch wesentlich veraendern koennen.

Begruendungen muessen kausal sein. Nicht: "Das wirkt moderner." Besser: "Der einzige warme Akzent markiert den naechsten kreativen Schritt; alle Mess- und Statusfarben bleiben davon getrennt."

## 19. Selbstpruefung vor einer Empfehlung oder Uebergabe

### Produkt und Nutzer

- Ist klar, fuer wen und in welchem Kontext gestaltet wird?
- Ist der wichtigste Nutzerjob in einem Satz formulierbar?
- Bleibt notwendige Profi-Tiefe erhalten?
- Wurde eine Domainkonvention nur mit gutem Grund gebrochen?

### Struktur

- Ist die dominante Leserichtung eindeutig?
- Kodiert Abstand echte Zusammengehoerigkeit?
- Sind primaere und seltene Aktionen verschieden gewichtet?
- Ist die Mitte langer Reihen ausreichend gegliedert?
- Gibt es fuer Hauptaktionen verstaendliche Gegen- und Fehlerpfade?

### Verhalten

- Ist jede Eingabe sofort und ehrlich rueckgemeldet?
- Sind aktive, automatisierte, deaktivierte und fehlerhafte Zustaende unterscheidbar?
- Beantwortet jede Visualisierung eine konkrete Frage?
- Unterbrechen Pop-ups oder Modals den kreativen Prozess unnoetig?

### Visuelle Sprache

- Lassen sich 2-3 Gefuehlsattribute in konkreten Entscheidungen wiederfinden?
- Sind Typografie und Werte in realer Groesse lesbar?
- Haben Akzentfarben stabile Bedeutungen?
- Folgen Licht und Schatten einer gemeinsamen Welt?
- Fuegen Materialien Affordanz oder Charakter hinzu?
- Ist Detailgrad dort hoch, wo Bedeutung hoch ist?

### Technik und Inklusion

- Sind Zielgroessen, DPI und Resize geprueft?
- Funktionieren Farbe, Fokus und Auswahl redundant?
- Bleiben Interaktion und Anzeige unter Last reaktionsfaehig?
- Sind Assets, Animationen und Handoff reproduzierbar?

### Evidenz

- Wurde Verhalten mit echter Interaktion statt nur mit Screenshots geprueft?
- Ist klar getrennt, was gemessen, gesehen, angenommen oder nur geschmacklich bewertet wurde?
- Sind Quellenheuristiken als solche markiert?

## 20. Kompakte Quellen-Digests

### S1 - Psychologieprinzipien

Die Quelle erklaert, wie Unterschied, Reihenfolge, kognitive Last, Anzahl von Wahlmoeglichkeiten und raeumliche Naehe Wahrnehmung und Entscheidung beeinflussen. Fuer Claude ist der wichtigste Transfer: Aufmerksamkeit und Arbeitsgedaechtnis sind begrenzt; Hierarchie, Gruppierung und bekannte Muster muessen diese Ressourcen bewusst schuetzen.

### S2 - Resonant Manifesto

Die Quelle positioniert Audio-UX als spezialisierte Disziplin. Gute Werkzeuge machen Faehigkeiten zugaenglich, ohne professionelle Tiefe zu zerstoeren. Research, reale Workflows, schnelle Prototypen und Nutzertests gehen visueller Selbstdarstellung voraus. Gleichzeitig ist die visuelle Identitaet bei Instrumenten und Effekten ein funktionaler Teil des Erlebnisses. Fruehe Designbeteiligung kann Produktentscheidungen praegen; spaete Beteiligung kann oft nur polieren.

### S3 - Gute und schlechte Plugin-UX

Die Quelle nennt Einfachheit, Konsistenz, Feedback, Accessibility und Performance als Kern. Positive Beispiele werden fuer direkte Manipulation, Echtzeitvisualisierung und klare Beschriftung angefuehrt; negative Muster sind ueberladene Interfaces, fehlendes Feedback, schwache Accessibility, intrusive Pop-ups und mangelhafte Anpassung an Bildschirmgroessen. Die genannten Produkte sind Beispiele, keine dauerhafte Rangliste.

### S4 - Drei-Punkt-Beleuchtung

Key, Fill und ein zusaetzliches Seiten-/Rim-Licht erzeugen Form, oeffnen Schatten und definieren Kanten. Warm-kalt-neutrale Kombinationen koennen Volumen und Stimmung tragen. Entscheidend ist nicht die Anzahl der Lichter, sondern konsistente Richtung, Hierarchie und Materialreaktion.

### S5 - Geometry Nodes

Prozedurale Geometrie und gekoppelte Parameter ermoeglichen live iterierbare, nicht-destruktive Assetfamilien und Animationen. Der Nutzen steigt bei Wiederholung und Varianten. Kosten entstehen durch Mathematik, Materialverwaltung und Pipelinekomplexitaet. Claude soll das Verfahren nur vorschlagen, wenn der wiederverwendbare Systemnutzen groesser als die Wartungslast ist.

### S6 - Aussergewoehnliche Plugin-Erlebnisse

Zweck, Zielgruppe, Workflow, visuelle Konsistenz und Bildschirm-/Plattformanpassung bilden den Prozesskern. Natuerlich platzierte Controls, sofortiges Feedback, sinnvolle Anpassbarkeit und Accessibility werden als Qualitaetsmerkmale genannt. Technische Renderingqualitaet ist Teil des Ergebnisses, aber nur in Verbindung mit Nutzbarkeit.

### S7 - Project Brief

Der Brief verbindet Marke, Positionierung, Konkurrenz, Team, Technik, Hero-Funktionen, Referenzen, Gefuehlsrichtung, Rechte, Scope und Erfolgskriterien. Besonders wertvoll ist die Aufforderung, Gefallen und Ablehnung an Referenzen zu begruenden und nur wenige Kerninspirationen zu waehlen.

### S8 - GUI Tips

Die sechs Stationen sind: Gefuehl definieren, UX-Layout bauen, modular ausrichten, Typografie begrenzen, Licht bewusst waehlen und Materialien fuehlbar machen. Die visuelle Abfolge Wireframe -> fruehe visuelle Ebene -> polierte UI ist der staerkste uebertragbare Prozessbeitrag der Quelle.

## 21. Schlussformel

Eine starke Audio-Oberflaeche ist weder eine dekorierte Parameterliste noch eine vereinfachte Consumer-App. Sie ist ein fachlich fluentes, visuell ehrliches Werkzeug: klare Aufgabe, belastbarer Flow, erkennbare Ursache und Wirkung, kontrollierte Aufmerksamkeit, konsistente visuelle Physik und eine Persoenlichkeit, die aus dem Produkt statt aus einem Trend entsteht.
