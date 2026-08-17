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

**Die drei Gesetze (aus der Probe-02-Ablehnung, 17.08. — tragend):**

1. **Schärfe + Detail schlagen Weichzeichner.** Unscharf/strukturlos liest
   sich als „billig". Detailauflösung ist Wertarbeit (P01: zehntausende
   scharfe Einzelpunkte ✓; P02: weiche Verläufe ✗).
2. **Persönlicher Stil schlägt generische Komplexität.** „Unity-Baukasten"-
   Effekte (Bloom, Lichtnebel, weiche Glows) sind als Kategorie verboten —
   lieber einfach mit Handschrift und Liebe zum Detail als komplex-generisch.
3. **Der Denkvorgang muss sichtbar sein — mit konzeptueller Distanz.**
   Kunstwert entsteht, wenn ein interessanter Gedanke sichtbar wird. Die
   Abbildung Musik→Bild muss selbst der Gedanke sein und eine Strecke
   überqueren: laut→hell ist Tautologie („beides zu nah aneinander");
   Musik→Materie-mit-Lebensdauer (P01) trägt. Jede künftige Probe braucht
   ein benennbares System, das man beim Zusehen LESEN kann.

**Verschärfung aus der Probe-03-Ablehnung (17.08. — Gesetz 3 war zu flach
gelesen; „lesbares System" allein reicht NICHT):**

3a. **Notwendigkeit statt Konstruktion.** Der Zusammenhang darf nicht
    ausgedacht wirken: Die Form muss aus der Natur der Sache folgen (was
    Musik/Frequenz/Energie IST, was EQ-Arbeit BEDEUTET), sodass man fühlt,
    es musste so aussehen. „Kein Grund, dass es da ist, aber es ist da"
    = Todesurteil. Gefäß/Tropfen war konstruiert; Perkolation war
    begründet (Musik ist Energie über Frequenz im Zeitfluss).
3b. **Entdeckt statt entworfen (Emergenz).** Bestanden: Phänomene —
    tausende einfache Teile + echter Prozess, Struktur entsteht von
    selbst. Durchgefallen: entworfene Einzelobjekte (Vorhang, Klingen),
    denen man das Ausdenken ansieht.
3c. **Optik-Datum:** glänzende Vektor-Verläufe + niedliche Physik =
    „Smartphone-App-Minispiel". Schärfe muss aus Auflösung in echte
    Punkte/Struktur kommen, nicht aus Gloss.

- **Referenzbilder des Users (17.08., vor Probe 01):** Wasser-Perkolation
  (Partikel altern/sedimentieren; 2× geschickt = Gewicht) · filmische
  Lichtbänder · elegante blaue Klingen mit Tropfen. Gemeinsame DNA:
  dunkler Raum, leuchtende Materie, tausende Teile bilden Struktur,
  Schwerkraft, Farbe trägt Bedeutung.
- Bandfarben-Palette bestätigt (P01 v2): Bass-Amber → Mitten-Magenta →
  Höhen-Eis; Leichtigkeit/Licht schlagen Masse („Betonklotz").

## Proben-Protokoll

| Probe | Stil-/Technik-Achse | User-Reaktion (wörtlich) | Destillat |
|---|---|---|---|
| 01 Perkolation (v1) | WebGL-Punktwolke, Physik (Fallen/Altern/Sediment), additive Lichtmischung, 2D-Raum | „Der effekt sieht optisch sehr gut aus, farblich ist da aber zu wenig unterschied. aber grundsätzlich gefällt mir stil, optik, technische darstellung. nur der rote betonklotz ergibt für mich technisch und optisch kein sinn" | **Punktwolken-Ästhetik bestätigt** (Stil + Optik + Technik). **Farbe braucht echte Trennung** — ein monochromer Verlauf (blau→violett) reicht nicht, das Bild braucht mehrere klar unterscheidbare Farbwelten. **Massive flächige Formen fallen durch** — das Sediment las sich als „Betonklotz": Akkumulation als kompakte Masse ist weder technisch noch optisch überzeugend. Leichtigkeit und Licht schlagen Masse. |
| 01 Perkolation (v2) | wie v1, aber: Sediment entfernt; Farbe zweidimensional (Frequenzbereich färbt, Alter lässt zur Glut abkühlen) | „gefällt mir, musst nicht weiter dran tweaken, das kann man machen falls es für nakama zum einsatz kommt" | **Abgenommen als Kalibrier-Datum.** Bandfarben-Palette (Amber/Magenta/Eis) + Punktwolken-Substanz bestätigt. Feinschliff bewusst vertagt auf einen echten Nakama-Einsatz. |
| 02 Lichtschleier | GLSL-Fragment-Shader, Licht als KONTINUIERLICHE Materie (Aurora-Vorhang, Faltenwurf per fbm, Kick-Woge, Reinhard-Tonemapping); gleiche Palette wie 01 → Vergleich misst nur die Substanz (Körnung vs. Fluss) | „optisch und künstlerisch sehr schlecht und nichtssagend […] unscharf, keine besonderen details, wirkt ‚billig'. aus künstlerischer sicht […] ideenlos, charakterlos, trägt keine stimmung, weil es kein stil hat. […] lieber einfache effekte mit einem persönlichen stil […] und maximal mit liebe zum detail, als komplex aber mit generischen effekten aus dem unity baukasten. […] Blasse lichtshader mit aufblitzen als Bass sind einfallslos aus menschlicher sicht, weil beides zu nah aneinander liegt." | **Klar abgelehnt — Quelle der drei Gesetze (siehe oben).** Weiche kontinuierliche Lichtfelder ohne lesbares System sind für diesen User wertlos, egal wie technisch aufwendig. |
| 03 Tropfenklingen | Vektor-Präzision (scharfe Canvas-Pfade, kein Glow); Gefäß-System: gestaute Energie → Überlauf → Tropfen-Ereignis; Detailliebe (Meniskus, Glanzkante, Abriss, Nachschwingen) | „optisch: smartphone app minispiel niveau, nichtssagend, kein charakter, weder kein logischer denkzusammenhang noch kreativer denkvorgang, ‚es ist einfach da' Die Darstellung ist die definition von Generisch. Der Zusammenhang ist konstruiert, kein grund für eq bedienung noch künstlerischer perspektive dass es da ist, aber es ist da" | **Klar abgelehnt — Quelle der Verschärfung 3a–3c.** Lesbares System reicht nicht: die Metapher (Gefäß/Tropfen) war beliebig gewählt, nichts an Musik oder EQ verlangt sie. Glossy Vektor-Objekte = Minispiel-Optik. |

## Geplante Achsen — REVIDIERT nach P02/P03-Ablehnung

Die Technik-Achsen (2D/3D/Shader/Vektor) waren die falsche Landkarte.
Was zählt, ist das PHÄNOMEN: begründet (3a), emergent (3b), in scharfe
Punkte/Struktur aufgelöst (3c). Künftige Proben variieren das Phänomen
innerhalb dieser Familie:

1. ~~Punktwolken-Perkolation~~ → Probe 01 ✓ (bestanden)
2. ~~Weiches Shader-Feld~~ → Probe 02 ✗ · ~~Vektor-Objekte~~ → Probe 03 ✗
3. Wellen-Interferenz (Klang IST Welle — echtes Wellenfeld) → Probe 04
4. Strömung/Advektion (Materie, die vom Klang getragen wird)
5. Feldlinien (Kraftfeld der Musik — Eisenspäne-Prinzip)
