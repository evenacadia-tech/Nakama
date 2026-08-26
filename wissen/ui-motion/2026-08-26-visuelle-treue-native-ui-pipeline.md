---
id: w-2026-08-26-visuelle-treue-native-ui-pipeline
typ: wissenseintrag
status: aktiv
datum: 2026-08-26
aktualisiert: 2026-08-26
hauptbereich: ui-motion
weitere_bereiche: [engineering, ux]
pruefstand: quellen-geprueft
---

# Visuelle Treue in einer nativen UI-Pipeline

## Menschliches Warum

Ein Mensch bedient ein Audio-Plugin während er hört. Wenn dieselbe Oberfläche
zwischen Entwurf, Skalierungsstufen und Plugin-Fenster sichtbar verrutscht,
weich wird oder ihre Ebenenreihenfolge ändert, muss er zusätzlich prüfen, ob
auch Zustand und Bedienbedeutung noch dieselben sind. Eine reproduzierbare
Übergabe schützt deshalb nicht nur Pixel, sondern das Vertrauen in stabile
Controls und verlässliche Rückmeldung.

Die Kausalkette lautet:

> Weil ein Produzent unter geteilter Hör- und Sehaufmerksamkeit auf räumlich
> stabile Zustände angewiesen ist, braucht die Designübergabe eine
> maschinenlesbare Struktur und objektiv prüfbare Renderresultate, damit
> Maßstab, Schärfe oder Layering keine neue Bedienbedeutung erfinden.

## Belegtes Wissen

Der Vortrag stellt einem manuellen PSD-Export eine strukturierte Pipeline
gegenüber: Ein standardisiert aufgebautes Quelldokument liefert Assets und
Layoutmetadaten; ein Loader rekonstruiert daraus die native Oberfläche. Damit
werden wiederkehrende Export- und Platzierungsentscheidungen aus einzelnen
Handgriffen in einen überprüfbaren Vertrag verschoben.

Die Folien behandeln drei besonders fehleranfällige Übergänge:

1. **Struktur:** Benennung, Layoutmetadaten und explizite Sortierebenen müssen
   aus dem Entwurf eindeutig hervorgehen. Sonst können visuell richtige
   Einzelassets im Programm in der falschen Reihenfolge erscheinen.
2. **Skalierung:** Container- und Einzelkomponentenskalierung können zu
   unterschiedlichen Pixelpositionen führen. Wiederholtes Rasterisieren oder
   ungeeignete Filter erzeugen Jitter, Unschärfe und Kantenartefakte.
3. **Prüfung:** Split-View, Überlagerung und Fehlerkarten machen Abweichungen
   zwischen Entwurf und Implementierung sichtbar. Kennzahlen wie RMS- oder
   GMSD-Fehler helfen beim Finden; die wahrgenommene Qualität bleibt dennoch
   eine eigene Beurteilung.

Der Vortrag zeigt außerdem Figma-zu-Code als mögliche Quelle einer solchen
Pipeline. Seine konkrete Referenzimplementierung ist jedoch Photoshop-basiert.

## Konsequenz für Entwurf und Bedienung

- Entwurfsquelle, Asset-Export, Laufzeitgeometrie und Bildvergleich brauchen
  getrennte Zuständigkeiten. Nur dann ist bei einer Abweichung erkennbar, an
  welcher Grenze sie entstanden ist.
- Material, Grain und Wortmarke dürfen als feste Assets behandelt werden;
  Texte, Kurven, Spektrum, Auswahl und Zustände müssen aus echten Daten
  entstehen. Ein pixelgenaues Bild darf keinen scheinbar lebenden Zustand
  backen.
- Z-Reihenfolge gehört in den Übergabevertrag. Sichtbare Überlagerung darf
  nicht von zufälliger Komponentenreihenfolge abhängen.
- Ein Bildfehlermaß ist Diagnosehilfe, keine automatische Designabnahme.
  Abweichungen an Text-Antialiasing oder Displayfiltern müssen getrennt von
  echten Geometriefehlern beurteilt werden.

## Visuelle Belege

- [Folien 8–9: manueller gegenüber strukturierter Übergabe](https://docs.google.com/presentation/d/1Z91UuA1wCUi3zR9ylCIkjqZXBWPViMn8wDHYC034nqs/view)
  zeigen, wie aus vielen losen Exporthandlungen eine definierte Quelle,
  Metadaten und ein Loader werden.
- [Folie 13: Split-View](https://docs.google.com/presentation/d/1Z91UuA1wCUi3zR9ylCIkjqZXBWPViMn8wDHYC034nqs/view)
  macht Lage- und Größenfehler direkt vergleichbar.
- [Folien 31–33: Sortierebenen](https://docs.google.com/presentation/d/1Z91UuA1wCUi3zR9ylCIkjqZXBWPViMn8wDHYC034nqs/view)
  zeigen, dass korrekte Assets allein die visuelle Hierarchie nicht sichern.
- [Folien 35–44: Skalierung und Resampling](https://docs.google.com/presentation/d/1Z91UuA1wCUi3zR9ylCIkjqZXBWPViMn8wDHYC034nqs/view)
  illustrieren Jitter, Filter- und Subpixelprobleme.
- [Folien 46–51: Fehlerkarten und Qualitätsvergleich](https://docs.google.com/presentation/d/1Z91UuA1wCUi3zR9ylCIkjqZXBWPViMn8wDHYC034nqs/view)
  zeigen die objektive Vergleichsebene. Die dargestellte Form ist keine
  Stilvorlage für Nakama.

## Bedeutung für Nakama

Nakamas bereits beschlossene Trennung bleibt bestehen: Figma ist editierbare
Quelle, die freigegebenen Nutzer-PNGs besitzen die sichtbaren Pixel, und das
[`AssetManifest`](../../design/prototyp/contract/asset-manifest.json) verbindet
Revision, Assets und Maßstab. Der Vortrag stützt dafür folgende Prüfstrategie:

- Overlay- und Differenzbilder ergänzen die menschliche Abnahme des
  Web-Simulators und später des nativen Editors.
- Die vier freigegebenen Rasterstufen werden jeweils aus der hochauflösenden
  Quelle abgeleitet. Die 100-Prozent-Fassung wird nicht zur Quelle der größeren
  Stufen.
- Layoutgeometrie und Ebenenreihenfolge werden als prüfbare Daten geführt.
- Die native JUCE-Oberfläche übernimmt Assets und Geometrie, nicht den
  Photoshop-Loader oder eine Browserlaufzeit.

## Grenzen, Widersprüche und offene Fragen

Der Vortrag ist ein Praxisbericht mit Projektcode, keine kontrollierte Studie.
Für das Video war kein Transkript verfügbar; geprüft wurden die vollständigen
52 Folien, die Videobeschreibung und das verlinkte Repository. Die dort
gezeigte Strategie, Bilder zur Laufzeit aus einer großen Quelle zu resamplen,
ist keine Vorgabe für Nakama und widerspricht nicht den bereits beschlossenen
vier expliziten Qualitätsstufen. Ob eine Bildmetrik einen für Musiker
relevanten Fehler findet, muss immer am sichtbaren Ergebnis geprüft werden.

## Quellen

- Titel: *PSD to DAW – Building a Pixel-Perfect UI Pipeline*
- Autor: Bence Kovács
- Jahr: 2025
- Video: https://www.youtube.com/watch?v=hebLkAR5X3I
- Folien: https://docs.google.com/presentation/d/1Z91UuA1wCUi3zR9ylCIkjqZXBWPViMn8wDHYC034nqs/view
- Beispielcode und Ressourcen: https://github.com/Bogren-Digital/PSD-to-DAW
- Eingang: `web-recherche`
- geprüft: Videobeschreibung, vollständiger Foliensatz und verlinktes
  Ressourcen-Repository; kein Transkript verfügbar

## Verknüpfungen

- [`../../DESIGN.md`](../../DESIGN.md)
- [`../../UX-CONTRACT.md`](../../UX-CONTRACT.md)
- [`../../design/docs/oberflaechen-spezifikation.md`](../../design/docs/oberflaechen-spezifikation.md)
- [`../../design/docs/interaktions-und-motion-spezifikation.md`](../../design/docs/interaktions-und-motion-spezifikation.md)
- [`../engineering/2026-08-26-audiodevcon-videoauswahl.md`](../engineering/2026-08-26-audiodevcon-videoauswahl.md)
