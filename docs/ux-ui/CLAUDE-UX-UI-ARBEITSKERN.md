---
title: "Claude UX/UI-Arbeitskern - Nakama"
document_type: "active_instruction_companion"
language: "de"
scope: "kompakte Regeln fuer die aktuelle UX/UI-Arbeit"
source_snapshot: "2026-08-20"
durable_reference: "./UX-UI-WISSENSBASIS-FUER-CLAUDE.md"
---

# Claude UX/UI-Arbeitskern

> **Fuehrende Arbeitskopie seit 20.08.2026:**
> `C:\Users\phili\Projekte\Nakama-Design\docs\` — dort mit auf das
> Design-Projekt angepassten Overrides (§3); diese Fassung hier ist das
> Quell-Archiv.

Nutze diese Datei als kurze aktive Anweisung bei UX/UI-Aufgaben. Fuer Hintergruende, Quellenkritik und Detailwissen lies [UX-UI-WISSENSBASIS-FUER-CLAUDE.md](./UX-UI-WISSENSBASIS-FUER-CLAUDE.md).

## 1. Prioritaet

1. Userabsicht und freigegebene Vorstellung.
2. Aktuelle Produkt- und Codewahrheit.
3. Lokale Projektvertraege und bereits getroffene Entscheidungen.
4. Reale Nutzeraufgabe, Bedienbarkeit und ehrliche Rueckmeldung.
5. Markencharakter und visuelle Qualitaet.
6. Trends und persoenlicher Geschmack zuletzt.

Allgemeine UX-Regeln sind Heuristiken. Sie duerfen keinen projektspezifischen Vertrag ueberschreiben.

## 2. Nicht verhandelbare Grundsaetze

- Beginne mit Zweck, Nutzer, Kontext und Erfolgskriterium - nicht mit Stil.
- Entferne unnoetige Reibung, aber erhalte notwendige Profi-Tiefe.
- Uebernimm keine Consumer-App-Schablone ungeprueft in Audio-Software.
- Ordne Funktionen nach realem Workflow, Haeufigkeit und Kritikalitaet.
- Baue zuerst Flow und Wireframe; Politur kommt danach.
- Jeder sichtbare Wert braucht eine reale Quelle. Keine erfundenen Diagnosen oder Demo-Werte in Produktionsansichten.
- Jede Aktion braucht sofortige, proportionale und ehrliche Rueckmeldung.
- Jede Visualisierung muss eine konkrete Nutzerfrage beantworten.
- Akzent ist knapp: Ein primaerer Fokus, stabile semantische Farbrollen, kein Konkurrenzleuchten.
- Abstand kodiert Zugehoerigkeit; ein Rahmen repariert keine schlechte Gruppierung.
- Licht, Schatten und Materialien folgen einer gemeinsamen visuellen Physik.
- Accessibility, Resize, DPI, Hostkontext und Performance sind Teil des Entwurfs, keine spaete Nacharbeit.
- Begruende Entscheidungen kausal. Vermeide leere Urteile wie "clean", "modern" oder "premium".
- Trenne klar: gemessen, im Produkt gesehen, aus Quellen abgeleitet, angenommen oder geschmacklich bewertet.

## 3. Nakama-spezifische Overrides

Vor Designarbeit gelten `CLAUDE.md` und die dort verlinkten aktuellen Designvertraege. Besonders wichtig:

- Die kreative Schleuse wird nicht uebersprungen. Vision und Referenzen kommen vom User; Claude hilft beim Praezisieren und baut erst nach Freigabe.
- Lesbarkeit ist Produktmassstab: Ein Profi muss den relevanten Befund sehr schnell ablesen koennen.
- Glow, Bloom, Weichzeichner und generische Effekt-Baukastenoptik sind kein Ersatz fuer Struktur oder Handschrift.
- Keine neue Grundmetapher erfinden, wenn ein freigegebener visueller Vertrag existiert.
- Das Master-Plugin und die eigenstaendige Prisma-Begleit-App sind verschiedene Produktflaechen; ihre Rollen nicht vermischen.
- Pixelkorrektheit, Bedienbarkeit, Markenpassung und Schoenheit sind getrennte Beweise.

## 4. Arbeitsfolge

### Schritt 1 - Auftrag in einen Designvertrag uebersetzen

Halte knapp fest:

- primaerer User und Nutzungskontext,
- wichtigster Job,
- gewuenschtes Ergebnis,
- 1-3 Erfolgskriterien,
- technische Grenzen,
- 2-3 gewuenschte Gefuehlsattribute,
- freigegebene Referenzen und Antireferenzen mit Gruenden.

Frage nur nach einer fehlenden Information, wenn sie die Richtung wesentlich aendert. Sonst eine konservative Annahme sichtbar machen.

### Schritt 2 - Arbeit und System modellieren

- Hauptpfad und haeufigste Wiederholung.
- Signal-/Datenquelle jedes Wertes.
- Primaere, sekundaere und seltene Funktionen.
- Abhaengigkeiten und technische Grenzen.
- Fehler-, Abbruch-, Reset-, Undo- und Wiederherstellungspfade.
- Relevante Nutzersegmente: Einsteiger, Power-User, Live-/Zeitdruck, praezise Analyse.

### Schritt 3 - Zustandsmatrix bauen

Pruefe je relevantem Control oder Bereich:

- Default,
- Hover/Fokus,
- gedrueckt/gezogen,
- geaendert,
- automatisiert/extern bewegt,
- gesperrt/verknuepft,
- deaktiviert mit Grund,
- ladend/keine Daten,
- Fehler/Verbindung weg,
- Bypass/Offline,
- Reset/Undo.

### Schritt 4 - Wireframe beweisen

Der Wireframe muss ohne Stilpolitur zeigen:

- dominante Leserichtung,
- klare Gruppierung durch Naehe und Freiraum,
- ehrliche visuelle Gewichtung,
- stabile Platzierung haeufiger Aktionen,
- lesbare Labels, Werte und Einheiten,
- Haupt-, Leer-, Lade- und Fehlerzustand.

Wenn der Flow im Graubild nicht funktioniert, nicht mit Farbe oder 3D retten.

### Schritt 5 - Visuelle Richtung uebersetzen

Uebersetze jedes Gefuehlsattribut in konkrete Regeln fuer:

- Raster und Proportion,
- Typorollen,
- Farb- und Akzentrollen,
- Lichttemperatur und -richtung,
- Materialfamilie,
- Detailgrad und Motion.

Maximal wenige Varianten zeigen; jede Variante braucht eine klare Hypothese und einen Unterschied mit Produktrelevanz.

### Schritt 6 - UI-System spezifizieren

- Design Tokens und Komponentenrollen.
- Responsive-/Skalierungsstrategie.
- Assetformat, Schaerfe und Exportpfad.
- Licht-/Materialverfassung bei realistischer Optik.
- Interaktions- und Animationstiming.
- Fokus-, Accessibility- und reduzierte-Bewegung-Verhalten.
- Performancebudget und Degradation.

### Schritt 7 - Real pruefen

Nicht nur Screenshots betrachten. Mit echter Interaktion und realistischen Daten pruefen:

- Findet der User den ersten richtigen Schritt?
- Erkennt er Status, Ursache und Wirkung?
- Bleibt der Routinepfad schnell?
- Sind Fehler und fehlende Daten ehrlich?
- Funktioniert Resize/DPI/Host?
- Bleiben UI und Messung unter Last glaubwuerdig?
- Ist die visuelle Richtung in realer Groesse noch lesbar und eigenstaendig?

## 5. Kompakte Wahrnehmungsregeln

- **Von Restorff:** Nur das wirklich Wichtige isolieren. Wenn alles auffaellt, faellt nichts auf.
- **Serielle Position:** Wichtige Orientierung an stabile Anfaenge/Enden; lange Reihen in Gruppen zerlegen.
- **Kognitive Last:** Fachliche Tiefe erklaeren und staffeln; Suche, Inkonsistenz und unklare Sprache entfernen.
- **Hick:** Wahl gruppieren, gute Defaults anbieten, seltene Entscheidungen aus dem Hauptpfad nehmen.
- **Naehe:** Innerhalb einer Gruppe enger, zwischen Gruppen deutlich weiter.

## 6. Visuelle Kernregeln

### Typografie

- Lesbarkeit vor Charakter.
- Eine UI-Schrift plus hoechstens eine begruendete Display-Schrift.
- Wenige semantische Rollen statt willkuerlicher Groessen.
- Zahlen, Einheiten und Vorzeichen stabil ausrichten.
- Keine Quellen-Pixelzahl ungeprueft uebernehmen; in Zielgroesse und DPI testen.

### Farbe

- Rollen definieren: Grund, Text, primaer, Auswahl, Information, Warnung, Fehler, deaktiviert.
- Bedeutung nie nur ueber Farbe vermitteln.
- Nahe Schwarz-/Weisstoene koennen Material und Stimmung verbessern; Kontrast bleibt zwingend.

### Licht

- Bei 3D-/Materialoptik: ein Key Light, ein schwächeres Fill und optional Rim/Seitenlicht.
- Richtung, Haerte und Temperatur ueber alle Komponenten konsistent halten.
- Licht dient Form, Fokus und Stimmung - nicht beliebigem Glanz.

### Material

- Kleine Materialfamilie mit klaren Rollen.
- Material entsteht aus Reflexion, Rauheit, Kante, Mikrostruktur und Lichtbezug.
- "Beruehrbar" heisst: Affordanz, Zustand und Hitbox passen zusammen.
- Materialdetail darf nie Label, Wert oder Fokus ueberstimmen.

### Motion und Visualisierung

- Bewegung zeigt Zustand, Zusammenhang oder Ergebnis.
- Kein dekoratives Dauer-Metering.
- Intensitaet proportional zur Bedeutung.
- Datenkadenz nicht mit Renderfluessigkeit verwechseln.

## 7. Geometry-Nodes-Entscheidung

Geometry Nodes nur einsetzen, wenn wiederholte Assets, Varianten oder Animationen eine gemeinsame parametrische Grammatik haben. Vorteil: zentral, nicht-destruktiv, schnell iterierbar. Kosten: Mathematik, Materialverwaltung, Handoff und Renderbudget. Eine interessante Technik ist noch kein UX-Grund.

## 8. Erwartetes Ergebnisformat von Claude

Bei einem Entwurf oder einer Empfehlung liefere in dieser Reihenfolge:

1. **Ziel und Nutzerjob**
2. **Gelesene Evidenz und Annahmen**
3. **Funktions- und Informationshierarchie**
4. **Hauptflow plus Gegen-/Fehlerpfade**
5. **Zustandsmatrix**
6. **Visuelle Grammatik mit kausaler Begruendung**
7. **Technische Umsetzung und Grenzen**
8. **Konkrete Abnahmetests**
9. **Offene, richtungsaendernde Unsicherheiten**

Keine unverbindliche Optionssammlung nach einer klaren Empfehlung. Wenn der Auftrag Implementierung umfasst und die Richtung freigegeben ist, die Empfehlung umsetzen und verifizieren.

## 9. Schlusscheck

- Ist der wichtigste Job in einem Satz klar?
- Ist notwendige Profi-Tiefe erhalten?
- Kann der User die Hauptaktion und den Status ohne Dekoration erkennen?
- Sind Gruppierung und Reihenfolge semantisch?
- Hat jeder Akzent genau eine Rolle?
- Sind alle relevanten Zustaende sichtbar und ehrlich?
- Beantwortet jede Visualisierung eine Frage?
- Sind Licht und Materialien kohaerent?
- Funktionieren Lesbarkeit, Fokus, Resize und Performance in realer Umgebung?
- Ist klar, was bewiesen und was nur angenommen wurde?

Wenn eine Antwort "nein" ist, ist der Entwurf noch nicht fertig.
