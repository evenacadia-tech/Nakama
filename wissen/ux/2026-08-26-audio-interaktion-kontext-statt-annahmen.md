---
id: w-2026-08-26-audio-interaktion-kontext-statt-annahmen
typ: wissenseintrag
status: aktiv
datum: 2026-08-26
aktualisiert: 2026-08-26
hauptbereich: ux
weitere_bereiche: [ui-motion, dsp, produkt]
pruefstand: quellen-geprueft
---

# Audio-Interaktion: Kontext statt ungeprüfter Annahmen

## Menschliches Warum

Bei einem Audio-Produkt ist eine sichtbare Geste nur die halbe Handlung. Ein
Mensch muss verstehen, welcher Klangparameter gemeint ist, wie die Bewegung
abgebildet wird und ob die hörbare Reaktion seiner Erwartung entspricht.
Konventionen wie „oben ist mehr“, ein vermeintlich eindeutiges Icon oder eine
bekannte Geste können je nach Person, Rolle und Kontext etwas anderes
bedeuten.

Die Kausalkette lautet:

> Weil Menschen Audio mit unterschiedlicher Erfahrung, Wahrnehmung und
> Aufmerksamkeit bedienen, müssen räumliche Zuordnungen, Icons und Gesten als
> überprüfbare Annahmen behandelt werden, damit eine scheinbar intuitive
> Oberfläche nicht nur für ihre Entwerfenden verständlich ist.

## Belegtes Wissen

Der Vortrag rahmt Audio-UX ausdrücklich breiter als Latenz. Seine sichtbaren
Kapitel und Folien behandeln Kontext, Annahmen, Unterschiede zwischen
Einsteigern und Experten, Icons, knappe Bildschirmfläche, Gesten,
Psychoakustik, nichtvisuelle Interaktion und Accessibility.

Mehrere direkt sichtbare Aussagen sind für die Entwurfsprüfung relevant:

- Die Folie „ASS + U + ME“ macht ungeprüfte Annahmen selbst zum Risiko des
  Entwurfsprozesses.
- „Does UP always equal MORE?“ stellt räumliche Parameterabbildungen als
  kontextabhängig dar, nicht als Naturgesetz.
- Einsteiger- und Expertenbedürfnisse werden als unterschiedliche Wege über
  dieselbe Lernstrecke gezeigt.
- Nichtvisuelle Interaktion wird zusammen mit wechselnden Rollen, mehreren
  Nutzeranforderungen, Geräten, Betriebssystemen, Orientierung und gemeinsamem
  Verständnis betrachtet. Audio-UX ist damit ein Systemkontext, keine
  isolierte Bildschirmfläche.

## Konsequenz für Entwurf und Bedienung

- Jede räumliche Geste braucht eine ablesbare Zuordnung. Bei einem EQ müssen
  Frequenz, Gain und Wert während des Zugs gemeinsam reagieren; die Bewegung
  darf nicht allein durch Gewohnheit erklärt werden.
- Icons und Positionen benötigen einen verständlichen Namen oder Status. Ein
  Symbol, das nur für erfahrene Entwickler eindeutig wirkt, ist noch kein
  bewiesenes mentales Modell der Nutzer.
- Direkte Bedienung und Präzisionsweg müssen dasselbe Ergebnis liefern. Das
  hilft Experten bei Geschwindigkeit und selteneren Nutzern beim
  Wiedererkennen.
- Psychoakustische und sichtbare Rückmeldung werden zusammen getestet: Nicht
  jede sichtbare Differenz ist hörbar, und eine hörbare Änderung darf nicht
  durch eine widersprüchliche Animation falsch erklärt werden.

## Visuelle Belege

- [20:05 – Annahmen als Entwurfsrisiko](https://www.youtube.com/watch?v=E3zUIR8xkrw&t=1205s):
  die Folie zerlegt „assume“ in „ASS + U + ME“.
- [28:25 – Einsteiger und Experten](https://www.youtube.com/watch?v=E3zUIR8xkrw&t=1705s):
  ein Bergdiagramm zeigt unterschiedliche Erfahrungspositionen statt eines
  einzigen Durchschnittsnutzers.
- [32:40 – „Does UP always equal MORE?“](https://www.youtube.com/watch?v=E3zUIR8xkrw&t=1960s):
  macht eine vertraute Gestenzuordnung zur prüfbaren Frage.
- [33:50 – Psychoakustik](https://www.youtube.com/watch?v=E3zUIR8xkrw&t=2030s):
  ordnet Ohr, Schall und Gehirn als zusammenhängende Wahrnehmungskette ein.
- [37:05 – nichtvisueller und technischer Nutzungskontext](https://www.youtube.com/watch?v=E3zUIR8xkrw&t=2225s):
  zeigt mehrere Anforderungen, Geräte, Rollen und Verständigungsformen.

## Bedeutung für Nakama

Nakamas bestehender Vertrag beantwortet einen Teil dieser Risiken bereits:
Bandzug, Zahlenwert und Kurve teilen einen Zustand; das Feld `BAND`,
Tastaturwege und numerische Eingabe bieten eindeutige Alternativen. Der
Vortrag begründet, warum diese Gleichwertigkeit nicht als Zusatzkomfort,
sondern als Schutz vor falschen Annahmen geprüft werden sollte.

Besonders zu testen sind die logarithmische horizontale Frequenzachse, die
vertikale Gain-Zuordnung, kleine Symbole für Bypass und Undo sowie die
Unterscheidung von Draft, bestätigter Kurve und externer Automation. Eine
Person soll Bedeutung und Rückwirkung erkennen können, ohne die interne
Nakama-Architektur zu kennen.

## Grenzen, Widersprüche und offene Fragen

Für dieses Video war kein Transkript verfügbar. Die Auswertung ist deshalb auf
Videobeschreibung, vollständige Kapitelstruktur und direkt geprüfte sichtbare
Folien beschränkt. Detailargumente zwischen den Zeitmarken werden nicht als
belegt ausgegeben. Die in der Videobeschreibung genannte frühe
Durchschnittsreaktion von ungefähr 100 Millisekunden wurde nicht unabhängig
wissenschaftlich geprüft und wird hier nicht als Designwert übernommen. Ob
eine Zuordnung für Nakamas Zielgruppe verständlich ist, braucht einen
Interaktions- und Hörtest.

## Quellen

- Titel: *The UX of Audio Experiences*
- Autorin: Amy Dickens
- Jahr: 2018
- Video: https://www.youtube.com/watch?v=E3zUIR8xkrw
- Autorinnenprofil: https://adickens.co.uk/
- Eingang: `web-recherche`
- geprüft: Videobeschreibung, vollständige Kapitelstruktur, ausgewählte
  sichtbare Folien und Autorinnenprofil; kein Transkript verfügbar

## Verknüpfungen

- [`../../UX-CONTRACT.md`](../../UX-CONTRACT.md)
- [`../../DESIGN.md`](../../DESIGN.md)
- [`../../design/docs/interaktions-und-motion-spezifikation.md`](../../design/docs/interaktions-und-motion-spezifikation.md)
- [`2026-08-26-nutzerzentriertes-audio-design.md`](2026-08-26-nutzerzentriertes-audio-design.md)
- [`../engineering/2026-08-26-audiodevcon-videoauswahl.md`](../engineering/2026-08-26-audiodevcon-videoauswahl.md)
