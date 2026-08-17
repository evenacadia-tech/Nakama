# Geschmacksprofil — der gemeinsame Nenner

> **Zweck (User-Idee, 17.08.):** Claude kann Schönheit nicht fühlen und
> nimmt Logik als Ersatzmaßstab; der User fühlt und sieht Schönheit.
> Deshalb: nacheinander Stil-/Technik-Proben bauen (2D/3D, verschiedene
> Fundamente), der User reagiert auf ECHTE Bilder, die Reaktionen werden
> hier destilliert. Dieses Dokument ist der wachsende Nenner — vor jeder
> Design-Entscheidung lesen.
>
> Proben: `eq-copilot/design/proben/probe-NN-*.html` (FL-Studio-Repo),
> alle auf der 30-s-Songschleife. Renders: `proben/renders/`.

## Erkenntnisse (Destillat — wächst mit jeder Probe)

- **Referenzbilder des Users (17.08., vor Probe 01):** Wasser-Perkolation
  (Partikel altern/sedimentieren; 2× geschickt = Gewicht) · filmische
  Lichtbänder · elegante blaue Klingen mit Tropfen. Gemeinsame DNA:
  dunkler Raum, leuchtende Materie, tausende Teile bilden Struktur,
  Schwerkraft, Farbe trägt Bedeutung.
- *(weitere Einträge nach User-Reaktionen)*

## Proben-Protokoll

| Probe | Stil-/Technik-Achse | User-Reaktion (wörtlich) | Destillat |
|---|---|---|---|
| 01 Perkolation (v1) | WebGL-Punktwolke, Physik (Fallen/Altern/Sediment), additive Lichtmischung, 2D-Raum | „Der effekt sieht optisch sehr gut aus, farblich ist da aber zu wenig unterschied. aber grundsätzlich gefällt mir stil, optik, technische darstellung. nur der rote betonklotz ergibt für mich technisch und optisch kein sinn" | **Punktwolken-Ästhetik bestätigt** (Stil + Optik + Technik). **Farbe braucht echte Trennung** — ein monochromer Verlauf (blau→violett) reicht nicht, das Bild braucht mehrere klar unterscheidbare Farbwelten. **Massive flächige Formen fallen durch** — das Sediment las sich als „Betonklotz": Akkumulation als kompakte Masse ist weder technisch noch optisch überzeugend. Leichtigkeit und Licht schlagen Masse. |
| 01 Perkolation (v2) | wie v1, aber: Sediment entfernt; Farbe zweidimensional (Frequenzbereich färbt, Alter lässt zur Glut abkühlen) | „gefällt mir, musst nicht weiter dran tweaken, das kann man machen falls es für nakama zum einsatz kommt" | **Abgenommen als Kalibrier-Datum.** Bandfarben-Palette (Amber/Magenta/Eis) + Punktwolken-Substanz bestätigt. Feinschliff bewusst vertagt auf einen echten Nakama-Einsatz. |
| 02 Lichtschleier | GLSL-Fragment-Shader, Licht als KONTINUIERLICHE Materie (Aurora-Vorhang, Faltenwurf per fbm, Kick-Woge, Reinhard-Tonemapping); gleiche Palette wie 01 → Vergleich misst nur die Substanz (Körnung vs. Fluss) | *ausstehend* | *ausstehend* |

## Geplante Achsen (je Probe eine andere Welt — Reihenfolge flexibel)

1. ~~Punktwolken-Physik (Perkolation)~~ → Probe 01
2. Fließendes Shader-Feld (GLSL-Fragment: Licht als kontinuierliche
   Materie, kein Teilchen — Nebel/Aurora/Flüssigkeit)
3. Vektor-Präzision (scharfe Geometrie, Klingen/Lamellen, Galerie-Ruhe)
4. Echtes 3D (Raumtiefe, Kamera, Struktur als Objekt)
5. Filmisches Licht (Streaks, Korn, Blooms — Kino-Standbild-Ästhetik)
