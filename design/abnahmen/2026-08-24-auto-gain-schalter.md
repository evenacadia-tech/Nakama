# Entscheid 24.08.2026 — Auto-Gain als Schalter, Standard aus

**Anlass:** Nachfrage des Users, ob Gain In/Out in der Technik bedacht ist
(Bestandsaufnahme `docs/offene-punkte.md` NAK-81). Der Vertrag kennt
Input- und Output-Trim; was fehlte, war eine Antwort auf die
Lautheits-Selbstfalle beim Anheben von Bändern.

## 1. Form der Antwort — ehrlich gekennzeichnet

Der User hat **keinen freien Satz** formuliert, sondern in einer
Auswahl geantwortet. Verbindlich ist deshalb der Wortlaut der
**gestellten Frage** und der **gewählten Option**, nicht eine
nachträgliche Zusammenfassung.

Gestellte Frage (wörtlich):

> „Soll Nakama beim EQ automatisch die Lautstärke ausgleichen — damit du
> beim Vergleichen nur die Klangfarbe hörst und nicht ‚lauter'?"

Gewählte Option (wörtlich):

> „Schalter, Standard aus (Empfehlung) — Ein kleiner AUTO-Schalter neben
> OUTPUT. Aus = alles bleibt wie heute, du drehst selbst. An = Nakama
> gleicht aus. Passt zum Grundgesetz ‚nichts Ungefragtes' und zum Vorbild
> Pro-Q. Du entscheidest pro Situation."

Abgelehnt wurden damit ausdrücklich: **„Gar nicht"** (kein Auto-Gain)
und **„Immer an, unsichtbar"** (das war als Grundgesetz-Bruch
gekennzeichnet und wurde nicht gewählt).

## 2. Vorgeschichte des Vorbilds

Der Verweis auf Pro-Q ist nicht neu, sondern das eigene Wort des Users
zum Master-EQ-Umfang vom 23.08.
(`2026-08-23-interview-struktur.md`, Frage 6):

> „das übersteigt mein technik verständnis, er muss das rad nicht neu
> erfinden, aber vorbild ist auf jedenfall besagte eq."

Belegt am Hersteller: Pro-Qs Auto Gain ist „an educated guess based on
the current EQ settings, and is **not** a dynamic process based on
actually measured levels" (fabfilter.com/help/pro-q/using/output,
abgerufen 24.08.2026). Das ist für Nakama die entscheidende Eigenschaft
— eine deterministische Funktion der Kurve, kein Telemetrie- oder
Messpfad, damit ohne Konflikt mit §46 („kein Telemetrieframe steuert
Audio") und mit dem regelbasierten Advisor.

## 3. Was daraus folgt

| Ebene | Folge |
|---|---|
| Bedienung | Ein Schalter **AUTO** im GLOBAL-Panel, direkt neben OUTPUT. Zwei Zustände, beide sichtbar (Regel „keine toten Elemente"). |
| Standard | **Aus.** Ein frisch eingefügtes Nakama verhält sich exakt wie heute; nichts bewegt sich ungefragt (Grundgesetz). |
| Wirkung an | Nakama rechnet den Pegelgewinn der aktuellen Kurve und zieht ihn am Ausgang ab. Der angezeigte Output-Wert wandert dabei sichtbar mit — ein unsichtbar wirkender Ausgleich wäre ein stiller Eingriff. |
| Verhältnis zum Handregler | Auto-Gain **ersetzt** den Output-Trim nicht. Der bleibt, was er ist; Auto-Gain legt seinen Ausgleich darauf. |
| Verhältnis zum Advisor | Unberührt. Das Vorhören gleicht die Lautheit schon heute eigenständig ab (Match-Gain §41, eingefrorener Vergleichspegel). Auto-Gain löst den **anderen** Fall: Handbetrieb an den Bändern. |

## 4. Wo es gebaut wird — nicht hier und nicht jetzt

Auto-Gain ist ein neuer persistenter Zustand und braucht deshalb eine
**neue Parameter-ID**; `nakama-parameter-v1.json` ist eingefroren und
kennt global nur Bypass · Input-Trim · Output-Trim · Width · Mono-Bass.
Es reist damit im selben Änderungssatz wie der Mix/Dry-Wet-Regler
(NAK-64 Punkt 4, Interview 23.08.) — beide sind Layout-v2-Erweiterungen,
beide gehören in **S26–28** (State/Parameter). Der aktive Audiokern, der
den Ausgleich rechnen muss, entsteht in **P6**.

Offen bleibt eine reine Technikfrage (meine, nicht die des Users): ob der
Ausgleich als eigener Parameter mitläuft oder aus der Kurve abgeleitet
wird. Ein eigener Parameter ist automatisierbar und im State-Hash; eine
Ableitung kann nicht driften. Entschieden wird das in S26–28 an der
Quelle, nicht hier.

**Status:** Der Entscheid ist verbindlich. Die Fläche ist noch nicht
gestaltet — der AUTO-Schalter fehlt im Figma-Stand und steht als
Restarbeit in `design/docs/figma-restarbeiten-nakama-gen.md` §7.5.
