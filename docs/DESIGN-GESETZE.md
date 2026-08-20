---
title: "Grundgesetze modernen Interface-Designs — Dauerwissen für Nakama"
document_type: "durable_reference"
language: "de"
recherchiert: "2026-08-20 (Websuche; searx lokal offline)"
gilt_fuer: "jede Gestaltungsentscheidung in diesem Projekt"
---

# Grundgesetze modernen Interface-Designs

Diese Datei entstand, weil derselbe Fehler zweimal passiert ist: fünf
Wireframe-Entwürfe für Main, und **alle fünf** waren Varianten von „möglichst
viel gleichzeitig sichtbar". Verschoben wurden nur Kästen. Das Gesetz darunter
war nie angewandt.

Sie ist ab jetzt Pflichtlektüre vor jeder Anordnungs- und
Sichtbarkeitsentscheidung, gleichrangig mit `CLAUDE-UX-UI-ARBEITSKERN.md`.

---

## 1. Das erste Gesetz

> **Sichtbar ist nur, was der aktuelle Handgriff braucht. Alles andere ist
> einen Schritt entfernt — nicht weg, aber nicht da.**

Der Fachbegriff ist **Progressive Disclosure**, 1995 von Jakob Nielsen als
Interaktionsmuster eingeführt: seltener gebrauchte oder fortgeschrittene
Funktionen wandern auf eine zweite Ebene, ohne dass Fähigkeit verlorengeht.
Gemessene Wirkung: **30–50 % schnellere Ersterledigung** gegenüber
Oberflächen, die alles zeigen.

Für datenreiche Oberflächen — und Nakama ist eine — gibt es die schärfere
Fassung, Ben Shneidermans **Visual Information-Seeking Mantra** (1996):

> **Overview first, zoom and filter, then details-on-demand.**

Drei Schritte, in dieser Reihenfolge:

1. **Übersicht zuerst** — das Ganze ohne Einzelheiten. Die Übersicht muss die
   übergreifende Geschichte erzählen, nicht jeden Wert zeigen.
2. **Zoomen und filtern** — Uninteressantes verschwindet, der Nutzer steuert,
   was übrig bleibt.
3. **Details auf Abruf** — erst wenn der Ort feststeht, kommt die Tiefe.

---

## 2. Der Einwand, der nicht zieht

Die naheliegende Ausrede lautet: *„Nakama ist ein Profi-Werkzeug, Profis wollen
Dichte, Progressive Disclosure ist etwas für Anfänger."*

Das ist falsch, und zwar aus einem präzisen Grund. Alan Coopers Beobachtung der
**„perpetual intermediates"**: Nutzer lernen genau so viel, wie sie für ihre
Arbeit brauchen, und hören dann auf. Der Experte ist ein normaler Nutzer in
einem seltenen Moment — er steht 80 % der Zeit an der Werkbank und geht nur
gelegentlich an die Schublade.

Daraus folgt der eigentliche Satz:

> **Progressive Disclosure teilt Aufgaben nach Häufigkeit — nicht Menschen
> nach Können.**

Es gibt also keinen „Anfängermodus" und keinen „Profimodus". Es gibt den
häufigen Handgriff, der sofort da sein muss, und den seltenen, der einen
Schritt entfernt sein darf. Wer den Unterschied nicht bestimmen kann, hat die
Arbeit nicht verstanden — und darf die Oberfläche noch nicht bauen.

---

## 3. Warum das keine Geschmacksfrage ist

Das Arbeitsgedächtnis fasst **etwa vier Einheiten**, nicht sieben. Millers
berühmtes 7 ± 2 von 1956 wurde von Nelson Cowan 2001 neu ausgewertet: unter
kontrollierten Bedingungen sind es rund **4 Chunks**.

Zwei Folgerungen, die ständig verwechselt werden:

- **Es zählt nicht die Zahl der Elemente, sondern die Zahl der Einheiten.**
  Elf Ziffern sind elf Elemente — als `(415) 555-2671` geschrieben sind sie
  drei Einheiten. Gruppierung senkt die Last, ohne Information zu entfernen.
- **Es geht nicht um Menülängen, sondern um Erinnern gegen Erkennen.** Die
  Frage ist nie „wie viele", sondern „muss der Nutzer sich etwas merken oder
  nur wiedererkennen". Was gleichzeitig sichtbar ist, muss nicht erinnert
  werden — deshalb ist *Kontext* billig und *Wechsel* teuer.

Daraus folgt kein Verbot von Dichte. Es folgt: **jede sichtbare Sache muss
einen Grund haben, jetzt sichtbar zu sein.**

---

## 4. Die drei Bauformen — und wann welche

Wer Übersicht und Tiefe verbinden will, hat genau drei Grundformen. Die Wahl
ist eine echte Entscheidung mit Vor- und Nachteilen, kein Geschmack.

| Bauform | Prinzip | Stark bei | Preis |
|---|---|---|---|
| **Overview + Detail** | Zwei getrennte Ansichten nebeneinander, die kleine zeigt, wo man in der großen steht | Einordnen, Vergleichen über Entfernungen; nachweislich schneller bei Kontextaufgaben | Zwei Ansichten kosten Fläche und müssen im Kopf verbunden werden |
| **Focus + Context** | Eine einzige Fläche: der Fokus in voller Tiefe, das Umliegende verzerrt/gedämpft mitlaufend | Alles in einem Blickfeld, kein Hin-und-Her; geringere Last als zwei Ansichten | Verzerrung muss verstanden werden; schwer präzise zu treffen |
| **Zoom (semantisch)** | Eine Fläche, mehrere Dichtestufen; beim Hineingehen wird dasselbe Objekt genauer, nicht nur größer | Große Mengen, natürliche Hierarchie | Man sieht nie beides; Orientierung muss aktiv gestützt werden |

Auf kleinen Fenstern gilt zusätzlich: eine große Detailansicht schlägt oft den
Gewinn durch ein zusätzliches Übersichtsfenster.

**Für Nakama steht die Antwort teilweise schon in der eigenen Spezifikation** —
und wurde bisher überlesen:

- „Dichtestufen **Liste → Heatmap → Detail**" — das ist semantischer Zoom.
- „ein **Minimap-/Zoomprinzip** schlägt ‚alles gleichzeitig'" — wörtlich das
  Gesetz, wörtlich die Absage an meine fünf Entwürfe.
- „Assistent: führt genau **EINEN Schritt zur Zeit**" — Step-by-step-Disclosure
  als Produktanforderung.
- „Vier Bedienebenen, **sichtbar getrennt**" — Zustand vor Funktion.

---

## 5. Die weiteren Gesetze

Kurz, weil sie unstrittig sind — aber sie gelten mit.

- **Hick:** Jede zusätzliche gleichgewichtete Option kostet Entscheidungszeit.
  Hierarchie löst das: eine primäre Handlung, eine sekundäre, sonst nichts.
- **Fitts:** Zielzeit hängt von Größe und Entfernung ab. Was häufig ist, gehört
  groß und nah; was gefährlich ist, klein und fern.
- **Jakob:** Nutzer verbringen ihre Zeit in anderen Programmen. Konventionen,
  die ihre Hände schon kennen, sind kein Mangel an Eigenständigkeit.
- **Von Restorff:** Erinnert wird, was sich abhebt. Wenn alles auffällt, fällt
  nichts auf — ein Akzent, nicht fünf.
- **Doherty-Schwelle:** Unter etwa **400 ms** Antwortzeit bleibt die Aufmerksamkeit
  im Fluss. Darüber wandert sie weg.
- **Ästhetik-Usability-Effekt:** Schön Aussehendes wird als bedienbarer
  *empfunden*, auch wenn es das nicht ist (Kurosu/Kashimura). Deshalb ist
  Schönheit kein Beweis für Bedienbarkeit — und der Grund, warum reine
  Gefällt-mir-Urteile in Aufgaben-Rahmungen gegengeprüft werden müssen.
- **Gestalt:** Nähe, Ähnlichkeit, gemeinsame Region und Fortsetzung ordnen
  stärker als jeder Rahmen. Abstand kodiert Zugehörigkeit; ein Kasten
  repariert keine schlechte Gruppierung.

---

## 6. Wo Progressive Disclosure schiefgeht

Das Gesetz hat eine Kehrseite, und sie muss mitgedacht werden:

1. **Versteckte Funktion.** Was niemand findet, existiert nicht. Jede zweite
   Ebene braucht einen sichtbaren, benannten Zugang — kein Rätselraten.
2. **Der häufige Handgriff hinter einem Klick.** Der schlimmste Fehler: das
   Oftgebrauchte wegräumen und das Seltene stehenlassen. Erst die Häufigkeit
   bestimmen, dann verstecken.
3. **Moduswechsel ohne Rückweg.** Wer eine Ebene öffnet, muss jederzeit sehen,
   wo er ist und wie er zurückkommt.
4. **Orientierungsverlust.** Beim semantischen Zoom muss immer ablesbar
   bleiben, welcher Ausschnitt des Ganzen gerade gezeigt wird.
5. **Aufklappen als Ausrede.** Ein Akkordeon, das zehn Abschnitte enthält,
   von denen sieben immer offen sind, hat nichts gelöst.

---

## 7. Verbindliche Folgen für dieses Projekt

1. **Vor jeder Anordnung wird die Aufgabenliste geschrieben**, nach Häufigkeit
   sortiert. Ohne diese Liste kein Wireframe.
2. **Jedes Element auf dem Blatt muss die Frage beantworten: welchen aktuellen
   Handgriff bedient es?** Wer die Frage nicht beantworten kann, entfernt das
   Element von dieser Ebene — nicht aus dem Produkt.
3. **Entwurfsvarianten unterscheiden sich in der Disclosure-Mechanik**, nicht
   in der Kastenanordnung. „Liste links oder rechts" ist keine Variante.
4. **Eine Disclosure-Gestaltung wird nie an einem Standbild beurteilt.** Sie
   muss durchgeschaltet werden können, sonst ist der Vergleich wertlos.
5. **Die Spezifikation ist bereits ein Disclosure-Auftrag** (Abschnitt 4).
   Wer gegen sie baut, muss das ausdrücklich begründen.

---

## Quellen

- [Progressive Disclosure — Nielsen Norman Group](https://www.nngroup.com/articles/progressive-disclosure/)
- [Progressive Disclosure: From Training Wheels to Week-Long AI Agents — UX Tigers (Nielsen)](https://www.uxtigers.com/post/progressive-disclosure)
- [Progressive Disclosure — Wikipedia](https://en.wikipedia.org/wiki/Progressive_disclosure)
- [Shneiderman, *The Eyes Have It: A Task by Data Type Taxonomy for Information Visualizations* (1996, PDF)](https://www.cs.umd.edu/~ben/papers/Shneiderman1996eyes.pdf)
- [Visual Information-Seeking Mantra — InfoVis:Wiki](https://infovis-wiki.net/wiki/Visual_Information-Seeking_Mantra)
- [Focus-plus-Context — InfoVis:Wiki](https://infovis-wiki.net/wiki/Focus-plus-Context)
- [A Review of Overview+Detail, Zooming, and Focus+Context Interfaces (Cockburn/Karlson/Bederson)](https://www.researchgate.net/publication/220566544_A_Review_of_OverviewDetail_Zooming_and_FocusContext_Interfaces)
- [Cognitive Load Is a Budget, Not an Enemy: Design for the Brain's 4 Chunks — UX Tigers](https://www.uxtigers.com/post/cognitive-load)
- [Your navigation menu doesn't need Miller's 7±2 rule — Stéphanie Walter](https://stephaniewalter.design/blog/your-menu-doesnt-need-millers-7-plus-minus-2-rule/)
- [Laws of UX](https://lawsofux.com/)
- [What are the laws of UX? All 21 laws explained — UX Design Institute](https://www.uxdesigninstitute.com/blog/laws-of-ux/)

Nicht gefunden: belastbare, veröffentlichte Untersuchungen speziell zu
Progressive Disclosure in Audio-Plugin-Oberflächen. Die Übertragung auf diese
Domäne ist damit **abgeleitet, nicht belegt** — und gehört entsprechend
gekennzeichnet, wo sie als Begründung dient.
