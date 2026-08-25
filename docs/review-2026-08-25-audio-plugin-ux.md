# Review: Codex-Skill `audio-plugin-ux` 0.1.0

- **Datum:** 2026-08-25 · **Prüfer:** Claude (Nakama-Session) · **Auftraggeber:** User
- **Gegenstand:** `audio-plugin-ux` 0.1.0, gefunden unter
  `~/.codex/plugins/cache/personal/audio-plugin-ux/0.1.0/` (Cache — das
  Quellverzeichnis kennt dieser Prüfer nicht).
- **Prüffrage des Users:** „reviewe ihn, ob er seinen zweck erfüllt". Der Zweck
  laut User: „um besser ux designen zu können". Vorbild und Ziel: zwei
  vom User gelieferte FabFilter-Pro-Q-4-Screenshots (Ruhezustand und
  Auswahlzustand).
- **Prüfumfang:** alle 6 Dateien / ~32 KB vollständig gelesen; 4 externe
  Quellen live nachgeschlagen; jede Nakama-Behauptung gegen die
  Repo-Quelle geprüft.

## Urteil

Der generische Teil erfüllt seinen Zweck und liegt über dem, was ein
Standard-UX-Leitfaden liefert. Der Nakama-Teil ist unbrauchbar: Er
beschreibt die Produktarchitektur **vor** dem Umschnitt vom 23.08.2026 und
trägt den Datumsstempel **25.08.2026**. Wer ihm folgt, entwirft für ein
Produkt, das es nicht mehr gibt.

| Datei | Urteil |
|---|---|
| `SKILL.md` | **Erfüllt.** Preflight, Placement Proof, Swap Test, State-Matrix sind brauchbares Handwerk |
| `references/design-reasoning.md` | **Erfüllt.** Visibility-Klassen und die neun Adversarial Checks sind der stärkste Teil |
| `references/fabfilter-pro-q-case-study.md` | **Weitgehend erfüllt.** Korrekt belegt, ehrlich gehedged; Lücken bei Instanzliste und gemischten Werten |
| `references/nakama-transfer.md` | **Verfehlt.** Faktisch überholt, falsch datiert, ohne Repo-Pfade, im Widerspruch zu einem besseren bestehenden Dokument |

---

## Teil A — Was der Skill gut macht

Diese Punkte sind belegt und sollen bei einer Überarbeitung **erhalten**
bleiben.

| Stärke | Beleg |
|---|---|
| **Echte Recherche, keine Halluzination** | Alle vier externen Quellen wurden nachgeschlagen und existieren. Das FabFilter-Zitat stimmt wörtlich: *„the floating band controls will automatically appear, right under the selected bands at the bottom of the display"* |
| **Ehrliche Konfidenz-Trennung** | Die Case Study markiert die Erklärung der Knopfreihenfolge ausdrücklich als *„medium-confidence inference, not a quoted FabFilter rationale"* und führt ein Confidence-Ledger |
| **Die richtige Kernlehre gezogen** | *„The interface is simple in the resting state, not simple in capability."* Das ist exakt der Unterschied zwischen den beiden gelieferten Bildern |
| **Stil-Diebstahl explizit verboten** | `SKILL.md:103` — *„Do not imitate FabFilter's colors, shapes, or arrangement as a style kit"* |
| **Hidden-active-Test** | *„Collapse every panel while its process stays active. Is the sound still explained?"* — genau die Falle, an der Progressive Disclosure zur Lüge wird |
| **Trigger von Scope getrennt** | Die Case Study modelliert Element / Trigger / Behavior / Reason statt Layout zu beschreiben. Ein Screenshot zeigt einen Zustand; die Qualität von Pro-Q liegt in den Übergängen |

### Verifikation der externen Quellen

| Zitat im Skill | Prüfergebnis |
|---|---|
| `fabfilter.com/help/pro-q/using/bandcontrols` u. a. | Existiert, Inhalt korrekt wiedergegeben |
| Tsang, Ho, Chan — Measurement and Control 2015, `10.1177/0020294015569264` | Echt: *„Interface Design and Display-Control Compatibility"*, Steve N. H. Tsang, John K. L. Ho, Alan H. S. Chan, Vol. 48 Issue 3. Behandelt spatial/movement/conceptual compatibility wie behauptet |
| „Designing for Learnability, 2025", `10.1177/10648046241273291` | Echt: *„Designing for Learnability: Improvement Through Layered Interfaces"*, Forsey, Leahy, Fields, Minocha, Attfield, Snell — *Ergonomics in Design*. Der Skill hedged korrekt mit „potential"; die Studie selbst nennt ihre Schlüsse „tentative" |
| Fitts 1954, `10.1037/h0055392` | Standard-DOI, korrekt |
| McGregor, `hdl.handle.net/10179/16620` | Echt: *„Knobs and nodes: a study of UI design in audio plugins"*, Jonathan Peter McGregor, Massey University, 2019. Testet tatsächlich mit Produzenten und Sound-Designern in deren Arbeitsumgebung |

**Fazit Teil A:** Die Recherche ist solide. Der Skill erfindet keine Quellen.

---

## Teil B — Befunde

### B1 — `nakama-transfer.md` beschreibt ein Produkt, das seit dem 23.08. nicht mehr existiert (schwer)

Die Datei trägt in Zeile 19 den Stempel *„As documented on 2026-08-25"* und
wurde am 25.08. um 10:19 geschrieben. Ihr Inhalt ist der Stand **vor** dem
Umschnitt.

| Zeile | Skill behauptet | Repo-Wahrheit | Beleg (Pfad vom Workspace-Root) |
|---|---|---|---|
| 23 | *„**Nakama Suna** is the compact passive probe surface"* — als drittes Produkt geführt | Suna **entfällt als App-Name** und ist in Probeeq aufgegangen | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:55` · `CLAUDE.md:7` · `docs/bauaufteilung-sonden.md:160` |
| 21 | Gen *„advises rather than directly processing the signal"* | Die Absolutform ist **ausdrücklich „überholt"**. Gen erhält einen vollwertigen Master-EQ. Es gilt „nichts Ungefragtes", nicht „berät nur" | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:66-69` |
| 22 | Probeeq: manuelle Bedienung und Gen-Weisungen seien *„equal first-class workflows"* | Bedienung liegt **zentral auf Gen Seite 2**; Probeeq behält nur eine Minimal-Rückfallfläche | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:59-62` · `docs/bauaufteilung-sonden.md:159-166` |
| 44, 52 | *„eight bands at 700x420"*, *„five global values in a 700x420 surface"* | 700×420 ist seit dem 23.08. **nur noch historisches Working Design**. Die reale EQ-Fläche ist Gen Seite 2 mit 760×430 | `docs/bauaufteilung-sonden.md:163-166` · `design/docs/ui-spielregeln-eq-zentrale.md:29-34` |
| 52 | *„twelve displayed parameters per band, a possible thirteenth sidechain source"* | **13 Parameter je Band.** Der dreizehnte (`sidechain_source`) ist entschieden: im ersten Release unsichtbar, Default `none` | `design/docs/ui-spielregeln-eq-zentrale.md:41-48` |
| 35 | *„Probeeq's central sonic object is an EQ band inside the aggregate EQ result"* | Der zentrale Sonic Object der EQ-Arbeit sitzt in **Gen Seite 2**, und dort liegen **zwei** EQ-Spuren in einem Graph | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:50-51, 61-65` |

**Herkunft des Fehlers:** Die Zahl „zwölf plus möglicher dreizehnter" steht
wörtlich in `briefing-hub/data/hub.json` — der Briefing-Seite, die laut
`CLAUDE.md` **abgeschafft** ist. Der Skill hat seine Nakama-Fakten
ausgerechnet aus der Quelle gezogen, die das Repo stillgelegt hat.

**Warum das Datum der eigentliche Schaden ist:** Ein undatierter Altstand
wird misstraut. Ein Stand mit heutigem Datum wird geglaubt.

### B2 — Die geforderte Auffrischung ist nicht ausführbar (schwer)

`nakama-transfer.md` verlangt dreimal, Behauptungen gegen das Repo
aufzufrischen:

- Zeile 3: *„Refresh claims against current source, tests, design decisions, and Figma evidence before acting."*
- Zeile 26: *„Refresh these from the current repository before citing them."*
- Zeile 36: *„Locate the current acceptance documents rather than quoting this file alone."*
- Zeile 85: fordert *„an evidence ledger with current repository links"*

Die Datei nennt **keinen einzigen Dateipfad**. Kein `design/abnahmen/`, kein
`CLAUDE.md`, kein `docs/`. Die Selbstkorrektur, auf der die Datei ihre
Zulässigkeit aufbaut, hat keine Adresse — und fordert vom Nutzer genau das,
was sie selbst nicht liefert.

### B3 — Der Skill dupliziert ein besseres Repo-Dokument und widerspricht ihm (schwer)

`design/docs/ui-spielregeln-eq-zentrale.md` existiert seit dem **23.08.**,
zwei Tage vor dem Skill. Es ist:

- ausdrücklich *„die Spielregeln, die das Design erfüllen muss, um umsetzbar zu sein"*,
- 45+ nummerierte Regeln, jede mit Quellenangabe,
- markiert Claude-Setzungen als **(Vorschlag)** / **(Annahme)** — also genau
  die Epistemik, die der Skill als seinen eigenen Beitrag führt,
- und autorisiert Pro-Q bereits als Referenz mit derselben Abgrenzung.
  Regel 4: *„Handling-Referenzen: Smooth Operator Pro, Pro-Q — Bedienlogik
  studieren erlaubt, die Optik bleibt Nakama."*

Der Skill erwähnt es nirgends und widerspricht ihm in den Zahlen (B1).

### B4 — Er rollt geschlossene Entscheide neu auf (mittel)

Fragenkatalog Punkt 4 (`nakama-transfer.md:47`): *„Which of the **twelve or
thirteen** band parameters exist for every band…"*

Beantwortet am 24.08.; `U5 / NAK-33` ist **geschlossen**
(`design/docs/ui-spielregeln-eq-zentrale.md:287`). Ein Skill, der
abgeschlossene Fragen wieder öffnet, kostet die Zeit, die er sparen soll.

### B5 — Er schließt umgekehrt eine offene Klärung vorschnell (mittel)

`nakama-transfer.md:33`: *„Figma's sources-by-band grid is the current Gen
overview after a later user decision superseded the earlier no-heatmap
interpretation."*

Das Repo sagt schwächer: Die Abnahme 20.08. lautete „Overview + Detail", der
Figma-Stand 21.08. zeigt ein Quellen×Band-Gitter, und die **Klärung läuft
noch** — `NAK-38` (`design/docs/oberflaechen-spezifikation.md:45`). Der Skill
macht aus einer offenen Klärung eine abgeschlossene Entscheidung. Das ist
das Spiegelbild von B4 und derselbe Fehlertyp: der Skill kennt den
Entscheidungsstand nicht und rät.

### B6 — Der entscheidende Transferbruch zu Pro-Q fehlt (schwer)

Pro-Q im gelieferten Ruhezustand-Bild läuft auf großer, frei skalierbarer
Fläche. Nakama Gen ist auf **760×430 bei festem Seitenverhältnis** entworfen.

Präzise Formulierung des Unterschieds — die grobe Fassung „Nakama ist fix"
wäre falsch:

- Der Editor **ist** größenverstellbar. `eq-copilot/plugin/src/PluginEditor.cpp:184-190`:
  `setResizable(true, true)`, `setResizeLimits(600, 416, 1950, 1352)`,
  `setFixedAspectRatio(kEinheitB / kEinheitH)`.
- Aber das Verhältnis ist **fest**. Vergrößern liefert **Zoom**, nicht
  Informationsfläche. Bei 150 % sieht der Nutzer dasselbe größer, nicht mehr.
- Pro-Q gewinnt beim Vergrößern echte Informationsfläche: mehr
  Spektrumauflösung, mehr gleichzeitig lesbare Bänder, das Panel frisst
  anteilig weniger Canvas.

Der Unterschied heißt also **„festes Verhältnis vs. freies Layout"**, nicht
„fix vs. skalierbar". Er bleibt real: Pro-Qs Disclosure-Rechnung geht bei
Nakama enger auf als beim Vorbild, und der Skill benennt das nicht.

> **User-Entscheid 2026-08-25 (wörtlich):** „ja aber flexible skalierung ist
> wesentlich schwerer zu designen. lassen wir es erstmal dabei."
>
> Festes Seitenverhältnis bleibt. Der Entwurf muss 8 Bänder × 13 Parameter
> + 5 globale Werte auf einem nicht umformbaren Feld unterbringen. Das ist
> die Bedingung, unter der der Skill für Nakama nützlich sein muss.

**Nebenbefund, gehört nicht dem Skill, sondern dem Repo:** Der laufende
Editor arbeitet auf `kEinheitB = 750`, `kEinheitH = 520`
(`eq-copilot/plugin/src/EqCopilotAssetKit.h:22-23`) — Verhältnis 1,44,
während die abgenommene Designgröße 760×430 das Verhältnis 1,77 hat. Das ist
die Material-Kit-Front, laut `CLAUDE.md` ein nie abgenommenes Provisorium.
Abgenommenes Designmaß und laufender Editor sind derzeit zwei verschiedene
Formate.

### B7 — Nakamas eigentliche UI-Neuheit kommt nicht vor (schwer)

Der Entscheid vom 23.08. enthält zwei Anforderungen, für die Pro-Q **keine
Vorlage ist** (`design/abnahmen/2026-08-23-gen-eq-zentrale.md:46-51`,
User-Wortlaut):

> „bei 2 EQ spuren liegen sie nicht nebeneinander, sondern sind in EINEM
> Graph visuell unterscheidbar durch zum beispiel farben."

> „auf der 2. seite dem EQ gibt eine UI an dem man die sonden durchklicken
> kann, nicht durch die 1. seite. […] 1 klick oder 3 klicks dazwischen
> liegen welten."

Zwei simultane EQ-Spuren in einem Graph und ein Sonden-Durchschalter auf
derselben Fläche — das ist der Kern des Produkts, und der Skill hat dazu
kein Modell. Er behandelt Probeeq durchgehend als Einzel-EQ.

Verschärfend: Das Analogon ist **in den gelieferten Bildern sichtbar**. Der
Auswahlzustand zeigt unten links `● Guitar` — Pro-Q 4s Instanz-Identität,
der nächste Verwandte des Sonden-Durchschalters. Der Skill listet Pro-Qs
Panels sorgfältig auf und übergeht ausgerechnet dieses. Das Repo kennt es
bereits als Markt-Befund (`design/abnahmen/2026-08-23-gen-eq-zentrale.md:86-90`:
Pro-Q 4 „Instance List", sonible smart:EQ 4 „Group View").

### B8 — Nicht messbar, kein Abbruchkriterium (mittel)

Der Skill fordert durchgehend Beweise, definiert aber keinen ausführbaren
Check und kein Kriterium, wann ein Brief fertig ist. In einem Repo mit
`tools/beweise.ps1`, Beweismanifesten in `docs/beweise/` und Urteilsmarken
liegt reine Prosa unter dem Hausstandard.

### B9 — Zwei bindende User-Gesetze fehlen namentlich (mittel)

| Gesetz | Datum | Was der Skill hat | Lücke |
|---|---|---|---|
| **„Keine toten Elemente"** — „die schlimmste ui/ux sünde sind sinnlose tote elemente"; jedes Element führt einen Handgriff aus oder meldet ehrlich einen Zustand | 24.08. | *„Decorative knobs or meters that imply control or measurement they do not provide"* | Kommt nah, führt es aber nicht als bindende Produktregel |
| **Material-Regel** — „ein button ist ein material das niemals einfach die breite verändert"; Zustand über Fläche/Schatten/Farbe/Transform, nie über Maße; Beweis per `getBoundingClientRect` | 25.08. | *„Controls that move or reorder when modes change, breaking learned motor paths"* | Die User-Regel ist strenger: Maße ändern sich **nie**, auch nicht beim Drücken |

---

## Teil C — Was der Skill an den gelieferten Bildern übersieht

Drei Dinge sind in den beiden Screenshots sichtbar, für Nakama zentral, und
im Skill nicht modelliert.

| Im Auswahlzustand sichtbar | Warum es für Nakama zählt |
|---|---|
| Filtertyp `Bell` mit `(multiple)` darunter | **Mehrfachauswahl mit gemischten Werten.** Der Skill testet Multiselection als Zustandsübergang, hat aber kein Modell für „Wert uneinheitlich" — bei 8 Bändern der Regelfall |
| Die rechte Skala trägt **zwei Maßstäbe**: +9/+6/+3 dB für die Kurve und −10…−120 dB für den Analyzer | Auf 760×430 müssen Achsen doppelt belegt werden. Der Skill hat für Achsen-Doppelbelegung keinen Begriff |
| `● Guitar` unten in der Leiste | Instanz-/Spur-Identität — siehe B7 |

Korrekt erfasst hat er dagegen den GAIN-Regler mit rotem Dynamik-Ring:
*„It also carries the dynamic-range ring, so it receives greater
visual/motor capacity."*

---

## Teil D — Auftrag an Codex

### D1 — `references/nakama-transfer.md` ersetzen, nicht flicken

Vier Kernaussagen sind falsch (Rollen, Suna, Gen-Verarbeitung, Fläche). Ein
Patch würde eine Datei erzeugen, deren Reststruktur weiter auf der alten
Architektur steht. Die Ersatzdatei muss:

1. **Repo-Pfade nennen** statt „refresh from the repository" (behebt B2).
   Verbindliche Einstiege:
   - `CLAUDE.md` — Wahrheitskern und Entscheid-Register
   - `design/abnahmen/2026-08-23-gen-eq-zentrale.md` — der Umschnitt, wörtlich
   - `design/docs/ui-spielregeln-eq-zentrale.md` — die 45+ Regeln, die jedes Design erfüllen muss
   - `docs/bauaufteilung-sonden.md` — welche Oberflächen es gibt
   - `design/LIES-MICH.md` — die 8 harten Designregeln
   - `docs/PLAN-STAND.md` — gerechneter Stand, nie von Hand editieren
2. **Zwei Apps führen, nicht drei.** Gen (Hub/Studio, Seite 1 Übersicht +
   Seite 2 EQ-Zentrale) und Probeeq (die eine Sonde, Minimal-Rückfallfläche).
   Suna ist als App-Name entfallen.
3. **Gen als verarbeitend führen.** Master-EQ in Gen; Grundgesetz ist
   „nichts Ungefragtes", nicht „berät nur".
4. **760×430 bei festem Seitenverhältnis** als Entwurfsbedingung, mit dem
   Vermerk, dass Vergrößern Zoom liefert und keine Informationsfläche
   (B6, inkl. User-Entscheid 25.08.).
5. **13 Parameter je Band**, davon `sidechain_source` im ersten Release
   unsichtbar mit Default `none`; dazu 5 globale Regler je Spur.
6. **Keine Datumsangabe ohne Quelle.** Wenn ein Stand zitiert wird, gehört
   der Pfad der Quelle daneben — sonst kein Datum.

### D2 — Zwei Modelle ergänzen, die dem Skill generell fehlen

Beide gelten über Nakama hinaus und gehören nach `design-reasoning.md`:

- **Instanz-/Kontext-Identität**: Welches Objekt aus einer Menge gleichartiger
  Instanzen wird gerade bedient, wie wechselt man, und wie bleibt der Wechsel
  im Ruhezustand ablesbar (B7).
- **Gemischte Werte bei Mehrfachauswahl**: Darstellung, Bearbeitung und
  Rücksetzung uneinheitlicher Werte über eine Auswahl (Teil C).

### D3 — Fläche als Entwurfszwang, nicht nur als Testfall

`SKILL.md` führt Größe bisher nur unter Validation („smallest supported host
size"). Sie gehört in den Preflight: Wie viel Information trägt die
Zielfläche überhaupt, bevor ein Layout entsteht? Nakama hat dafür bereits
Messwerkzeuge, die der Skill nicht kennt:
`design/werkzeug/sonde-messung.html` (misst 8 Slots × 12 Parameter + 5
globale im Platz-Grenzfall) und `design/werkzeug/formfaktor.html`
(Bildschirmanteil).

### D4 — Die zwei User-Gesetze aufnehmen

„Keine toten Elemente" (24.08.) und die Material-Regel (25.08.) namentlich
und in der strengen Fassung (B9).

### D5 — Was nicht angefasst werden soll

`design-reasoning.md` und `fabfilter-pro-q-case-study.md` sind inhaltlich
tragfähig und sauber belegt. Außer den Ergänzungen aus D2/D3 besteht dort
kein Änderungsbedarf. Insbesondere die Regel *„Do not imitate FabFilter's
colors, shapes, or arrangement as a style kit"* bleibt — sie deckt sich mit
Regel 4 der `ui-spielregeln`.

---

## Teil E — Antwort auf die Prüffrage

**Erfüllt der Skill seinen Zweck?**

- Als **generischer Audio-Plugin-UX-Skill**: ja. Er ist brauchbar, belegt und
  über Standardniveau.
- Als **Nakama-Werkzeug**: nein. Er würde in die falsche Architektur führen,
  und zwar mit dem Autoritätsanschein eines tagesaktuellen Dokuments.

**Strukturelle Beobachtung dahinter:** Der Skill ist **analytisch, nicht
generativ**. Er prüft einen Entwurf, der bereits existiert — Placement Proof,
Swap Test, State Matrix setzen ein Layout voraus. Für „besser UX designen"
fehlt der Schritt davor: aus 8 Bändern × 13 Parametern + 5 globalen Werten
auf 760×430 überhaupt erst einen Vorschlag ableiten. Pro-Q löst dieses
Problem mit Fläche; Nakama hat sie nach dem Entscheid vom 25.08. nicht.

---

## Quellen dieses Reviews

Alle Pfade vom Workspace-Root `C:/Users/phili/Projekte/Nakama`.

| Behauptung | Quelle |
|---|---|
| Suna entfällt als App-Name | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:55` |
| Zwei Apps, EQ rechnet auf Bussen und in Gen | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:59-62` |
| „Gen und Suna beraten nur" ist überholt | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:66-69` |
| Zwei EQ-Spuren in einem Graph, Sonden-Durchschalter auf Seite 2 | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:46-51` |
| Markt-Befund Instance List / Group View | `design/abnahmen/2026-08-23-gen-eq-zentrale.md:86-90` |
| Suna in Probeeq aufgegangen; 760×430 beide Seiten; 700×420 historisch | `docs/bauaufteilung-sonden.md:159-166` |
| 8 Band-Slots × 13 Parameter, 5 globale Regler, 109 Hostparameter | `design/docs/ui-spielregeln-eq-zentrale.md:41-48` |
| Sidechain unsichtbar im ersten Release, U5/NAK-33 geschlossen | `design/docs/ui-spielregeln-eq-zentrale.md:44, 287` |
| 760×430 beide Seiten, freie Skalierung als Produktentscheid offen | `design/docs/ui-spielregeln-eq-zentrale.md:29-34` |
| Pro-Q als Handling-Referenz autorisiert, Optik bleibt Nakama | `design/docs/ui-spielregeln-eq-zentrale.md:36-37` |
| Quellen×Band-Gitter ist offene Klärung (NAK-38) | `design/docs/oberflaechen-spezifikation.md:45` |
| Ursprung 760×430: Messung + User-Urteil „so passt es 3 größen" | `design/abnahmen/2026-08-20-groessen-alle-drei.md` |
| Editor resizable, festes Verhältnis 750:520 | `eq-copilot/plugin/src/PluginEditor.cpp:184-190` · `eq-copilot/plugin/src/EqCopilotAssetKit.h:22-23` |
| Briefing-Hub als Quelle der 12-Parameter-Zahl | `briefing-hub/data/hub.json:165-173` |

Externe Quellen: live nachgeschlagen am 2026-08-25, Ergebnisse in Teil A.
