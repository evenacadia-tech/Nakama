---
name: interview
description: Strukturiertes User-Interview, um die Design-Vorstellung des Users zu erfassen, aufzufrischen oder abzusichern. Jederzeit einsetzbar; Pflicht zu Beginn jeder Arbeitsplan-Phase. Aufruf mit Phase (designvertrag | struktur | richtung | komponente | zustaende | abnahme) oder ohne — dann die Phase aus dem Arbeitsstand wählen. Antworten werden wörtlich in design/abnahmen/ protokolliert.
---

# /interview — die Vorstellung des Users erfassen

Du interviewst den User, um SEINE Vorstellung zu erfassen — nicht um
deine zu bestätigen. Sein Wort ist die Vision; du übersetzt.

## Regeln

1. **Höchstens 4 Fragen pro Runde**, dann zusammenfassen und erst nach
   seiner Reaktion weiterfragen. Nie den ganzen Katalog auf einmal.
2. **Offene Fragen zuerst**, Auswahlfragen (AskUserQuestion) nur, wenn
   Optionen echte, gleichwertige Alternativen sind — nie mit versteckter
   eigener Präferenz formuliert.
3. **„Weiß ich nicht" ist eine gültige Antwort.** Dann: die risikoärmste
   Annahme benennen, sichtbar festhalten, weitergehen — nicht bohren.
4. **Wörtlich protokollieren:** Ergebnis als
   `design/abnahmen/JJJJ-MM-TT-interview-<phase>.md` mit Zitaten, Festlegungen
   und offenen Punkten. Zitate nie glätten.
5. **Widerspruchs-Check:** Vor jeder Runde die vorhandenen Abnahmen
   überfliegen. Widerspricht eine neue Antwort einer alten Festlegung:
   beide zeigen und fragen, welche gilt.
6. **Truhen-Abgleich:** Wo passend, konkret auf Elemente aus `design/assets/`
   zeigen („passt das hier zu dem, was du meinst?") statt abstrakt zu
   fragen.
7. Antworten nicht bewerten, nicht verkaufen, nicht vorwegnehmen.

## Fragenkatalog nach Phasen

### designvertrag — Projektstart oder Neuausrichtung

Wirkung und Erfolg:
- Was soll ein Musiker nach fünf Minuten mit dem Plugin sagen können,
  das er vorher nicht konnte?
- Woran erkennst DU, dass das Design gelungen ist (1–3 Kriterien)?
- Welche 2–3 Gefühle soll die Oberfläche auslösen — und bei welchem
  Produkt hast du je eines dieser Gefühle schon einmal gehabt?

Referenzen:
- Welche 2–3 Oberflächen (Plugins, Apps, auch Nicht-Audio) gefallen dir —
  und was GENAU daran (Zeigen > Beschreiben; gern in die Truhe legen)?
- Welche Oberflächen stoßen dich ab — und was genau daran?
- Gibt es Materialien, Licht, eine Zeit oder einen Ort, an die die
  Oberfläche erinnern soll?

Rahmen:
- Wie viel sollst du selbst am Ende einstellen können (Themes, Dichte,
  Größen) — und was soll bewusst fest sein?
- Wie frei soll Claude innerhalb deiner Richtung arbeiten: eng
  übersetzen, gemeinsam formen, oder innerhalb eines klaren Kerns frei?

### struktur — vor jedem Wireframe

- Was ist auf DIESER Oberfläche der eine häufigste Handgriff?
- Was willst du beim ersten Blick zuerst sehen — und was darf bewusst
  zurücktreten?
- Gibt es eine Anordnung aus einem anderen Werkzeug, die deine Hände
  schon kennen und die wir übernehmen sollten?
- Was wäre für dich auf dieser Fläche „überladen" — wo ist deine Grenze?
- Lieber wenige große Elemente oder mehr kleine mit Zoom/Fokus?

### richtung — vor der Stilprobe

- Hell, dunkel oder beides — und warum?
- Eher flächig-ruhig oder mit spürbarem Material und Tiefe?
- Wie viel Farbe verträgt die Arbeit: fast monochrom mit einem Akzent,
  eine kleine Palette, oder farbcodierte Bereiche?
- Welche Schrift-Anmutung: technisch-nüchtern, warm, charaktervoll?
  (Beispiele aus der Truhe zeigen, falls vorhanden.)
- Was von den gezeigten Varianten würdest du einem Freund als „typisch
  Nakama" beschreiben?

### komponente — vor dem Bau eines Bausteins

- Wie soll sich dieses Element anfühlen: präzise/technisch, griffig/
  physisch, oder unsichtbar-dienend?
- Welche Information muss AM Element stehen, welche darf erst bei
  Interaktion erscheinen?
- Drehen, ziehen oder tippen — was erwartet deine Hand hier?
- Gibt es ein Vorbild für genau dieses Element (Truhe/Referenz)?

### zustaende — vor der Zustands-/Motion-Phase

- Wie laut darf ein Problem rufen — dezent melden oder deutlich
  unterbrechen?
- Was soll passieren, wenn Daten fehlen oder veraltet sind: leise
  ausgrauen, klar beschriften, beides?
- Wie viel Bewegung erträgst du beim konzentrierten Hören — was wäre zu
  viel?
- Soll Erfolg (Befund gelöst) gefeiert werden oder still verschwinden?

### abnahme — nach jedem gezeigten Stand

- Was gefällt dir an diesem Stand am meisten — was würdest du sofort
  behalten?
- Was stört dich, auch wenn du es nicht benennen kannst? (Deuten reicht —
  „da oben links irgendwas".)
- Ist das noch die Richtung aus dem Designvertrag, oder hat sich deine
  Vorstellung bewegt?
- Freigabe, Änderung oder Verwerfen — und bei Änderung: die kleinste, die
  den Unterschied macht?

## Abschluss jeder Interview-Runde

Zusammenfassung in 3–5 Sätzen vorlesen („Habe ich das richtig
verstanden: …"), erst nach Bestätigung protokollieren. Die
Protokoll-Datei endet mit: **Festgelegt** / **Angenommen (unbestätigt)** /
**Offen** — drei getrennte Listen.
