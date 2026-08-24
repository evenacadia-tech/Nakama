# NAKAMA

Plugin-Familie für FL Studio (Windows 11, JUCE 8/C++20, CMake) mit
eigenständigem Rust-Broker (Named Pipe, `broker/`). Seit 23.08.2026 **zwei**
Apps, **eine** Design-Identität: **Nakama Gen** (Hub/„Studio": Übersicht,
Befunde, Advisor; Seite 2 = EQ-Zentrale aller Sonden + Master-EQ) und
**Nakama Probeeq** (DIE Sonde auf den Bussen, EQ zuschaltbar, bedient in
Gen; Suna ist in Probeeq aufgegangen — Register 23.08.); Bundle-Name
**Nakama Studio**. Code, Bundle, Pipes und Schemas tragen heute noch den Legacy-Namen
**EQ-Copilot** (`EqCop*`, `Eqcp`) — Umbenennung ist ein eigenes
Identitäts-Ticket (NAK-30), kein Nebenbei-Refactor.

**Dieses Repo ist seit 18.08.2026 eigenständig** (Historie davor:
FL-Studio-Repo bis `7964777`). Remote `https://github.com/evenacadia-tech/Nakama`
(privat; User arbeitet wechselnd an Desktop und Laptop): vor Arbeitsbeginn
`git pull`, nach Commits pushen. Parallele Sessions sind möglich: eigene
Edits sofort per **explizitem Pathspec** committen (nie `git add -A`, nie
`--amend`), fremde uncommittete Dateien nie anfassen.
**Seit 22.08.2026 liegt auch das Design hier** (`design/`, vorher eigenes
Repo `Projekte\Nakama-Design`, Historie per Subtree-Merge erhalten) — User:
„ich habe am anfang versucht das zu trennen aber es funktioniert nicht …
im anschluss wird dann nurnoch im nakama repo gearbeitet". Einstieg dort:
`design/LIES-MICH.md`.

<!-- WAHRHEITSKERN:ANFANG — dieser Block wird von tools/hooks/nakama-primer.sh
     bei jedem Sessionstart und nach jeder Compaction injiziert. Er ist die
     EINZIGE Kopie; Hook und Memory dürfen ihn nicht nachbauen. -->
## Wahrheitskern (Stand 21.08.2026, aus dem Kontext-Interview mit dem User)

- **Produkt (seit 23.08.):** Nakama = **zwei** Apps. **Gen** (Hub/„Studio":
  Quellen-Übersicht, Befunde, Advisor; **Seite 2 = EQ-Zentrale**: bedient
  die EQs aller Sonden UND einen vollwertigen Master-EQ; Sonden-Durchschalter
  direkt auf der Seite, zwei EQ-Spuren in EINEM Graph farblich unterschieden
  — Design-Vorgaben des Users) · **Probeeq** (DIE Sonde auf den Bussen:
  misst passiv, EQ zuschaltbar, bedient in Gen). **Suna ist in Probeeq
  aufgegangen** (Merge 23.08.). Bundle „Nakama Studio". EINE Design-Identität.
  Wortlaut: `design/abnahmen/2026-08-23-gen-eq-zentrale.md`.
- **Grundgesetz (Fassung 23.08.): nichts Ungefragtes.** Audio wird nur dort
  verarbeitet, wo der User es einschaltet (Master-EQ in Gen, Sonden-EQ in
  Probeeq, Hör-Markierung); ausgeschaltet = **beweisbar bitidentisch**
  (NullTest-Standard), sonst Audio-Passthrough sampleidentisch (0 Samples
  Latenz, kein Tail). Der EQ rechnet immer dort, wo das Audio fließt (Sonde
  bzw. Gen auf dem Master); Gens Seite 2 ist Fernbedienung, die Parameter-
  und Zustands-Hoheit bleibt in der Instanz. Der Advisor schlägt nur vor,
  der User wendet an. Audiothread überall: keine Sperren, Allokationen,
  Datei-/Pipe-/Netz-Zugriffe, kein Logging; Überlast verwirft
  Analyseframes, nie Audio.
- **Keine KI-/Claude-Erklärschicht** im Produkt (User 21.08.). Der Advisor ist
  regelbasiert.
- **Design-Quelle ist Figma (User)** — seit 22.08. **eine** Datei, `Nakama-Design`
  (Key `NPCQYSkoZEd4Av0NlKxBOd`), dunkel verbindlich, hell geplant. Die Figma-Stände sind die Vorgabe;
  `design/` übersetzt sie in lebende Blätter (Zustände,
  Größen, Grenzfälle) — keine eigene Stilsuche, keine Varianten-Befragung.
  Produkt-Sprache **Englisch**; Docs, Commits, Gespräch Deutsch.
- **Material-Kit-Front** im heutigen Plugin = **Provisorium**, nie abgenommen;
  keine Arbeit mehr daran.
- **Prisma / Hörkompass / Glas-und-Licht** = **User-Idee, Studie, geparkt**
  (`eq-copilot/design/prisma-studie/`). Kein Produktteil, kein Bauplan; ihr
  Vokabular gehört nicht in die Plugin-UI. Das Geschmacksprofil dort bindet
  nur die Studie.
- **Produktzahlen** (16 sichtbare / 32 Vertrag Quellen, 8 Bänder, ±12 dB,
  1,5/3 dB): vom User **hingenommene Startwerte**, änderbar —
  Regelfall einstellige Quellenzahl. **Das Remote-±3-dB-Limit ist 23.08.
  entfallen** (Register; Drafts sind im EQ frei nachjustierbar).
- **Pläne:** `docs/FL-Nakama-Sonden-Design-Entwurf.md` (mit Errata-Block)
  ist der technische Entwurf; die Recherche liegt als **Archiv** in `docs/archiv/`.
  Bauentscheidung erteilt 20.08. („okay dann fangen wir damit nächste
  session an"). Nächste Fläche ohne FL-Termine: S7 (`SONDE-006`).
- **Regel für Claude:** Ein Entscheid existiert nur mit **Datum + Zitat des
  Users** (Register unten). Eigene Vorschläge heißen „Vorschlag", nie
  „abgenommen"/„verbindlich". Keine zweite Kopie dieser Wahrheit in Hooks,
  Memory oder anderen Docs — verweisen, nicht abschreiben.
<!-- WAHRHEITSKERN:ENDE -->

## Entscheide des Users (Register — nur mit Zitat; Vorschläge stehen hier nicht)

| Datum | Entscheid | Wortlaut |
|---|---|---|
| 12.08. | Nakama berät nur (Rückweg vertagt); ein Referenztrack für jede Musik „ist Blödsinn"; Mess-Scope ist Pflicht | FL-Commits `77432e1` `882a964` `f13d2c9` — Paraphrasen im Commit-Body („USER-VORGABE"), Wortlaut nicht protokolliert |
| 16.08. | Umbenennung zu Nakama; Hör-Markierung als eigene Idee | FL-Commits `37aba8b`, `51021d2` — Paraphrasen im Commit-Body, Wortlaut nicht protokolliert |
| 16./17.08. | 3D-Papier-Shader, Tusche-Einzelmarken verworfen | „das sieht sehr schlecht aus" · „zerissenes Stroh", „Kochhut" |
| 17.08. | Kreativer Prozess wird nie übersprungen (Kreativ-Schleuse) | „man arbeitet eine idee aus, konkretisiert sie, solange bis man eine vorstellung hat … Kunst ist wertlos ohne ihren Entstehungsprozess." (Hook `tools/hooks/kreativ-schleuse.sh`) |
| 18.08. | Prisma-Objekt ✓, rohe Punktwolke als Dauerinhalt ✗ | „das prisma sieht top aus … wasserfall aus farbigen pixeln" |
| 19.08. | Nacht-Freistellung verworfen | „tut mir leid aber das war keine gute arbeit … eingebacken" |
| 20.08. | Bauentscheidung Sondenkern | „okay dann fangen wir damit nächste session an" |
| 20.08. | Prisma = eigenständige Begleit-App, Master-Plugin konventionell | Interview 21.08.: „Meine Entscheidung, so gesagt" |
| 20.08. | Design-Abnahmen (Größen 760×430 / 700×420 / 260×84, Overview+Detail, Vorhören nur markierte Zeile, Zustände nach Ausnahme-Prinzip) | `design/abnahmen/2026-08-20-*` |
| 21.08. | Nakama = Familie; Prisma nur Studie, geparkt | Interview: „Familie; Prisma nur Studie" · Prisma-Herkunft: „Meine Idee" · Hörkompass: „Alles nur Studie" |
| 21.08. | Probeeq ist ein EQ — **Bedienort revidiert 23.08.** (zentral in Gen, Seite 2) | „die active Probe fester Name : Nakama Probeeq ist ein vollwertiger hochwertiger EQ der mit Nakama kommuniziert. er kann von nakama direkt anweisungen umsetzen aber auch ganz normal manuell vom user benutzt werden" |
| 21.08. | Namen — **Suna aufgegangen in Probeeq 23.08.** | „Nakama Gen = Main app · aktive sonde = Nakama Probeeq · passive sonde = Nakama Suna · Bundle = Nakama Studio" |
| 21.08. | KI-Erklärschicht raus | „Nein – raus aus dem Produkt" |
| 21.08. | Produktzahlen | „Hingenommen, passen aber" · Regelfall einstellige Quellenzahl: „ich habe schlicht 5 genommen weil 16 bedeutet 16 geladene proben auf instrumenten … 16 plugins nur für eq kostet auch massig leistung" (`design/abnahmen/2026-08-21-…`) |
| 21.08. | Design-Quelle | „das finale design wird aktuell in figma gemacht . alle 3 apps werden ein design haben mit der selben identität. alle alten sind alt. Ein Design entwickelt sich und ist nicht einfach da." · Rolle des Design-Repos: „Figma ist Quelle; Repo setzt um" |
| 21.08. | Material-Kit-Front | „Nie abgenommen – bleibt Provisorium" |
| 21.08. | Eigene Entwürfe (`design/…selfmade`, Figma-Bilder) | „Richtung für Look & Stimmung" · „Das ist ein Designprototyp keine technikanleitung, design passt sich am ende der funktion an." |
| 21.08. | Sprache | „Englisch – mein Wort" |
| 21.08. | Glas/Licht-Annahme raus; Geschmacksprofil nur Studie | „Glas/Licht raus; Profil nur Studie" |
| 21.08. | Recherche | „Archiv" |
| 21.08. | FL-Termine A/B bald; bis dahin S7 | „Termine bald; bis dahin S7" |
| 21.08. | Version 0.3.0 angleichen, noch nicht installieren | „Version 0.3.0 angleichen, noch nicht installieren" |
| 21.08. | „Lernsprache" und „Kernfunktion vor Verwaltung" gelten nicht mehr | „Keines mehr" |
| 21.08. | Aufräumen: alte Richtungen ins Archiv, Prisma-Studie parken, tote Memories löschen, Inspirationen + regenerierbare Renders raus | Interview Frage 20, alle vier gewählt |
| 22.08. | Figma-Depot: die Datei `Nakama-Design` ist die einzige verbindliche Quelle; je App hell + dunkel geplant, verbindlich heute nur dunkel | „das ist das neue verbindliche und einzige Depot indem sich die aktuellsten Designs befinden. ich habe für jedes design vor eine helle und dunke variante zu erstellen. stand jetzt aber erstmal nur dunkel“ · `design/abnahmen/2026-08-22-figma-depot.md` |
| 22.08. | Design-Repo und Technik-Repo zusammengeführt: `Nakama-Design` wird `design/` im Nakama-Repo, danach nur noch hier gearbeitet | „ich möchte dass du die nakama design und nakama arbeitsbereiche zusammenführst. ich habe am anfang versucht das zu trennen aber es funktioniert nicht … einfach infos die im nakama design exisitieren die nicht im nakama folder präsent sind reinholen, so dass nichts verloren ist. im anschluss wird dann nurnoch im nakama repo gearbeitet" · Commits `6fd08a1` (Merge) + Folgecommits |
| 22.08. | Committen und Pushen laufen ohne Rückfrage; Rückfragen nur noch bei Unumkehrbarem (Riegel `tools/hooks/git-riegel.sh`, Auto-Push `tools/hooks/auto-push.sh`) | „wie funktioniert das commiten und pushen hier, ich möchte dass das so automatisch wie möglich ist" · Commit `39bb30a` |
| 22.08. | Gemini (Antigravity-CLI `agy`) nur lesend: Audits und Gegenproben, nie schreibend — auch nicht eigenständig (Riegel `tools/hooks/fremdmodell-riegel.sh`) | „gemini ist nicht sehr zuverlässig , ich möchte dass er ausschlisslich für audits und read only benutzt wird. falls du ihn mal eigenständig nutzen solltest" |
| 22.08. | ~~Der Hub ist die Seite `https://nakama-briefing.philipld.chatgpt.site`~~ — **überholt 23.08.** (Seite abgeschafft, siehe unten); die Aussage „alle anderen Artefakte gelten nicht mehr" gilt weiter | „das ist der neue und einzige hub , alle anderen artefakte sind hiermit nichtmehr zu beachten" · Commits `f55c2fe`, `8974a74` (Codex-Vorarbeit) |
| 22.08. | 35 Antworten im Hub: Gen (18), Suna (10), Figma-gegen-Entwurf (5), `U5`, `U10`. Entscheidungstor Gen offen — Studie 04 wird weiterverfolgt, die Übersetzung beginnt gegen den Figma-Export | Wahl „Vorschlag übernehmen" auf `U2.0`; alle 35 mit Wortlaut in `design/abnahmen/2026-08-22-hub-antworten-35.md` |
| 22.08. | Bei **technischen** Fragen entscheidet der Implementplan (`docs/FL-Nakama-Sonden-Design-Entwurf.md`), nicht der Figma-Stand; für das Aussehen bleibt Figma die Quelle | „figma ist keine technikreferenz, der implementplan ist es" (Hub `U9.1`) |
| 22.08. | Wortmarken aller drei Apps: es gilt der Figma-Export, Claude wählt keine Fassung aus | „alle wortmarken haben den stil wie ich ihn aus figma exportiert habe" (Hub `U6.9`) |
| 22.08. | Hör-Markierung nur noch mit gültigem „spielt"; das heutige fail-open ohne Transport-Information fällt (NAK-35/NAK-24, Tickets S10–S13) | Wahl „Nein, nur mit Signal" (Hub `U10`) |
| 22.08. | Gen-Übersicht: das Quellen×Band-Gitter aus Figma gilt; die Abnahme vom 20.08. ist an der Stelle überholt, die es ausschloss | Wahl „Figma gilt" (Hub `U9.5`) |
| 23.08. | Dirigent-Autonomie: der Dirigent hält nicht nach jedem Ticket an, sondern arbeitet den Plan ohne Zwischenhalt ab (Halt nur an Gate, Bruch oder echter User-Frage) | „Durchlaufen bis der Plan leer ist" (`.claude/skills/dirigent/SKILL.md` §0; NAK-54) |
| 23.08. | Gate-Strenge: **keine** harte Weiterschalt-Regel — je Fall entscheiden. An ihrer Stelle steht die Belegpflicht: jedes Weiterschalten protokolliert (a) worauf es sich stützt UND (b) was ungeprüft blieb | „ich will nicht dass es an einer harten regel scheitert. ich denke das kann variieren. die regel ist einfach das individuell zu entscheiden mit bestem wissen" (`.claude/skills/dirigent/SKILL.md` §3.4; NAK-54) |
| 23.08. | **OpenWiki wird von Hand aufgefrischt, nicht nach Zeitplan.** Der tägliche GitHub-Workflow und der dafür nötige `OPENAI_API_KEY` entfallen; die Auffrischung läuft im Lauf einer Session über die OpenWiki-Werkzeuge (NAK-51 damit verworfen) | „upodate das openwiki und verwerfe das mit dem API key wir machen das manuell" |
| 23.08. | **Die Briefing-Seite ist abgeschafft.** Der Planstand wird nicht mehr gepflegt, sondern aus dem Repo **gerechnet**; die offenen Fragen stellt ein Skill nach und nach im Chat und arbeitet die Antwort sofort ein | „wir brauchen eine andere möglichkeit als den hub. eie gibt keine automatische aktualisation , das heißt er wird driften und somit drifted plan stand auch." → Wahl **„saeite ganz weg, alles im repo + Skill bauen, der automatisch die offenen fragen nach und nach an mich stellt und gleich einarbeitet in den plan"** |
| 23.08. | **Gen wird EQ-Zentrale; Suna und Probeeq verschmelzen zu EINER Sonde; Gen erhält vollwertigen Master-EQ.** Der EQ rechnet auf den Bussen, bedient wird auf Gens Seite 2 (voller Wortlaut `design/abnahmen/2026-08-23-gen-eq-zentrale.md`; Umschnitt NAK-64, Figma NAK-65) | „STATT mehrere eq auf den instrumenten liegen zu haben, könnte man das EQ UI des Probeeq auf eine neue 2. seite auf den Gen legen … Also im Prinzip liegt der EQ auf den Bussen wird aber eingestellt auf dem Gen" · „Gen wird ein vollwertiges HUB oder Studio … es ist ein vollwertiger eq für den master wie andere eq auf dem markt … das ziel ist maximale kontrolle ohne überkomplexe workflow" · Wahl „Merge: eine Sonde" · „ja festhalten" |
| 23.08. | Name der verschmolzenen Sonde: **Nakama Probeeq** (Suna entfällt als App-Name) | Wahl „Nakama Probeeq" (ebd.) |
| 23.08. | Design-Vorgaben EQ-Seite: Sonden-Durchschalter direkt AUF Seite 2, nie über Seite 1; zwei EQ-Spuren in EINEM Graph, farblich unterscheidbar | „auf der 2. seite dem EQ gibt eine UI an dem man die sonden durchklicken kann, nicht durch die 1. seite. das wäre wieder nicht intuitiv. in der musikproduktion ist workflow alles. 1 klick oder 3 klicks dazwischen liegen welten. zudem bei 2 EQ spuren liegen sie nicht nebeneinander, sondern sind in EINEM Graph visuell unterscheidbar durch zum beispiel farben." |
| 23.08. | **Das ±3-dB-Remote-Limit entfällt** (Anzeige UND Grenze; beantwortet U14) — Advisor-Drafts nutzen die manuellen Bereiche und sind im EQ frei nachjustierbar; Draft-Kasten-Wort: „DRAFT" | „ja draft reicht, aber die limitangabe oder das limit selbst ist sinnlos geworden. das kann im eq dann ja angepasst werden von selbst" (Interview `struktur` R4, `design/abnahmen/2026-08-23-interview-struktur.md`) |
| 24.08. | **Gen Seite 1 und Seite 2 sind gleich groß** — die abgenommenen 760×430 gelten für beide Seiten (Größenfrage aus NAK-65 beantwortet). **Tote UI-Elemente sind verboten:** jedes sichtbare Element bedient einen Handgriff oder meldet ehrlich einen Zustand | „da es sich um 2 flächen der selben app handelt müssen die selbstverständlich gleich groß sein" · „die schlimmste ui/ux sünde sind sinnlose tote elemente" (`design/abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md`) |
| 24.08. | ~~**WhatsApp ist der Berichtskanal**~~ — **überholt am 24.08. (Zeile darunter)**; die Regel „nur an den User selbst, nie an Dritte" gilt weiter | „du schickst nur nachrichten an mich selbst. […] das ist der offizielle weg mich zu erreichen ab jetzt. schreib immer wenn eine session fertig ist ein ganz kurzen bericht" |
| 24.08. | **Die claude.ai-Routine ist Bericht- UND Rückfragekanal.** Nach jeder fertigen Session eine kurze Meldung — und **jede Frage an den User geht dort raus**, nicht nur in die Nimbalyst-Unterhaltung. Grund: der User ist oft nur per Handy da; eine Frage, die in der Unterhaltung stehenbleibt, erreicht ihn nicht, sie blockiert nur still. Kanal: Routine **„Nakama: Dirigent-Meldung"** (`trig_01BUKf1i5Y9ztqGkA6Ev4eff`, kein Cron, `push: true`) — Meldung in den Prompt zwischen die `--- MELDUNG ---`-Marker schreiben, dann `run`. **WhatsApp ist optional** und darf ausfallen, ohne dass eine Meldung ausfällt (die Gerätekopplung lässt sich nur von einem Menschen am PC wiederherstellen — also genau dann nicht, wenn der User remote ist) | „ich moechte eine Meldung von ihm in der Routine, er haette auch vorhin die Frage dort stellen muessen." |

**Was NICHT mehr gilt** (und nirgends mehr als gültig auftauchen darf):
Recherche als „kanonischer Plan" · Spectral Field / Bauplan 2.0 / Tiefenfeld /
Kunstwerk / Feld-Alphabet als Hauptansicht („alle alten sind alt") ·
Hörkompass als Zielvertrag der Plugin-UI · Geschmacksprofil für die Plugin-UI ·
Material-Kit-Front als „abgenommen" · Claude-Klick im Grundgesetz · Tauri-Hub-App
als Produktteil · „Lernsprache" / „Kernfunktion vor Verwaltung" als Regeln ·
**die Briefing-Seite `nakama-briefing.philipld.chatgpt.site` und die Pflicht,
sie zu lesen oder zu füttern** (abgeschafft 23.08. — sie hatte keine
automatische Aktualisierung und driftete deshalb, samt Planstand) ·
**Suna als eigene App** und „Gen und Suna beraten nur" in der Absolutform
(Merge + Master-EQ 23.08. — es gilt „nichts Ungefragtes, alles beweisbar") ·
**Probeeqs eigene Voll-UI als primärer Bedienort** (bedient wird auf Gens
Seite 2; eine Minimal-Rückfallfläche der Sonde ist Vorschlag, nicht
abgenommen).

## Wo was liegt

| Was | Pfad |
|---|---|
| Plugin (JUCE 8 + CMake) | `eq-copilot/plugin/` (`src/` Produkt · `hostbridge/` · `hostprobe/` · `spike/` · `vertrag/` · `tests/`) |
| Schemas v2 (Vertrag des heutigen Plugins) / v3 (Sondenfamilie) | `eq-copilot/schemas/` · `eq-copilot/schemas/v3/` |
| State-Schema 2 + Parameterbestand (SONDE-006) | `eq-copilot/schemas/state/` (Vertrag) · `eq-copilot/plugin/state/` (Code) · `eq-copilot/fixtures/state/` (Korpus, drei Beine) |
| Identität (eingefroren, SONDE-001) | `eq-copilot/identity/plugin-identities-v1.json` |
| Installer: Vertrag / Auslieferung / Ausführender (SONDE-007b) | `eq-copilot/schemas/installer/nakama-installer-v1.md` · `eq-copilot/install/nakama-installer-v1.json` · `eq-copilot/install/Install-Nakama.ps1` |
| Broker (eigene Crate, `eqcop-broker.exe`) | `broker/` |
| Beweis-Runner + Manifeste | `tools/beweise.ps1` · `docs/beweise/` |
| Python-Erzeuger und -Prüfer | `tools/eq-copilot/` |
| Technischer Entwurf der Sondenfamilie (Fassung 0.4 + Errata) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` |
| Bauaufteilung (Sessions, Gates, Prüfstufen) | `docs/bauaufteilung-sonden.md` |
| Plugin-Architektur heute | `docs/plugin-wissen.md` |
| Offene Punkte (durabel, nie still löschen) | `docs/offene-punkte.md` |
| Einstieg nächste Session | `docs/NEXT-SESSION.md` |
| **Archiv** (Recherche, Mockups, alte Baupläne, alte Design-Docs — nur Verlauf) | `docs/archiv/` · `eq-copilot/design/archive/` |
| **Prisma-Studie** (geparkt, User-Idee) | `eq-copilot/design/prisma-studie/` (Statusblatt dort) |
| Material-Kit-Kette (Provisorium, technisch lebendig: `tokens.json` → `LeitstandTokens.h`) | `eq-copilot/design/` Wurzel |
| Design der Apps (Figma-Übersetzung, Abnahmen, Truhe, Werkzeug) | `design/` — Einstieg `design/LIES-MICH.md` · `design/abnahmen/` · `design/assets/figma/` · `design/werkzeug/` · `design/docs/` |

## Planstand — gerechnet, nicht gepflegt (seit 23.08.2026)

Der Planstand ist ein **Messwert**, kein Feld. Bis 23.08. stand er als
`status`-Text in `hub.json` und wurde von Hand umgestellt; wer es vergaß,
hinterließ einen still falschen Stand, und die Briefing-Seite driftete
mit. Der User hat beides abgeschafft (Register 23.08.). Es gilt dieselbe
Regel wie im Beweis-Runner: **erst messen, dann melden.**

| Frage | Antwort kommt aus | wer pflegt das |
|---|---|---|
| Welche Schritte gibt es? | `docs/plan/plan.json` — Phasen, Ticket, Klartext. **Kein Statusfeld.** | autoriert (Text ist kein Messwert) |
| Gebaut? | liegt `docs/beweise/<Ticket>.md`? | `tools/beweise.ps1` schreibt es |
| Abgenommen? | **Urteilsmarke** im Manifest | der frische Prüfer (T2/T3) |
| Wie frisch ist das Blatt? | Stempel `gerechnet-aus: <sha>` gegen HEAD | git |

**Die Urteilsmarke** ist eine Zeile im Manifest, das einzige Stück Status,
das ein Mensch schreibt — weil ein Urteil kein Messwert ist:

```
<!-- NAKAMA-URTEIL: T2 PASS 2026-08-23 -->     bzw. T3 / NEEDS_WORK
```

🔑 **Fail-closed:** nur `T2`/`T3` + `PASS` macht einen Schritt *abgenommen*.
Fehlende Marke, `T1` (Selbstaudit des Erbauers) oder `NEEDS_WORK` heißen
**„gebaut, Urteil offen"**. Vergessen führt damit zur Untertreibung, nie zur
Übertreibung — die einzige Fehlerrichtung, die dieses Projekt sich leisten
kann. Bei mehreren Marken gilt die **letzte, die die geforderte Stufe
erreicht** — Prüfrunden werden unten angehängt, ein späteres `T3 NEEDS_WORK`
kassiert also ein früheres `T2 PASS`.

**Erzeugt** wird das Blatt `docs/PLAN-STAND.md` von
`py -3.13 tools/plan/planstand.py`. Es wird **nie von Hand editiert**.
Automatik: `tools/hooks/planstand.sh` (PostToolUse) misst nach jedem Befehl
den Zustand — weicht der Stempel von HEAD ab, wird neu gerechnet und das
Blatt **allein per Pathspec** committet. Auslöser ist der gemessene Zustand,
nicht der Text eines Befehls (dasselbe Muster wie `auto-push.sh`); die Drift
kann damit höchstens einen Commit groß werden.

**Offene Fragen an den User** stehen in `docs/plan/fragen.json`, Bilder in
`docs/plan/bilder/`. Sie werden **im Chat** gestellt — der Skill
`.claude/skills/fragen/` nimmt sie eine nach der anderen, zeigt das Bild, wo
es um Sichtbares geht, schreibt die Antwort **wörtlich mit Datum** ins
Register bzw. nach `design/abnahmen/` und arbeitet die Folge sofort in den
Plan ein. Details `docs/plan/LIES-MICH.md`.

⚠️ **Die Briefing-Seite ist abgeschafft** (User 23.08.). `briefing-hub/`
bleibt als Verlauf liegen — nicht deployen, nicht füttern, nicht lesen.
`tools/hub/hub_sync.py` ist stillgelegt.

## Bauen & Beweisen (vom Workspace-Root)

```powershell
$cmake = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S eq-copilot -B eq-copilot/build -G "Visual Studio 17 2022" -A x64
& $cmake --build eq-copilot/build --config Release --target EqCopilot_VST3 EqCopShot EqCopPaintBench EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopPipeProbe EqCopIdentityTest EqCopHostContextTest EqCopHostProbe_VST3 EqCopHostProbeTest EqCopSchemaTest EqCopStateMigrationTest
```

**Ein Befehl für den ganzen Kanon** (ersetzt die bewusst nicht gebaute CI):

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-0NN.md -Anhaengen -Titel 'SONDE-0NN'
```

Er fährt alle Beine, schreibt die **rohe** Ausgabe ins Manifest und misst den
Baustand: sind Prüfbinaries älter als ihre Quellen, verweigert er mit Exitcode 4
die Beglaubigung (0 grün · 2 rot · 3 Voraussetzung fehlt · 4 nicht beglaubigt).
Vorlage `docs/beweise/VORLAGE.md`, Basislinie `docs/beweise/S0-basislinie.md`.

**Kanon (30 Einträge deklariert, davon **28 heute lauffähig** — gezählt in der
Tabelle von `tools/beweise.ps1`, dort steht die Wahrheit; die 28 decken sich mit
dem `28/28` des jüngsten Manifests):** NullTest · Golden ·
Markierung · `cargo test` (seit 22.08. mit dem JCS-Bein) · sechs Python-Beine
des v3-Vertrags · A11 `pruefe_v2_schemas.py` · **A12
`erzeuge_state_fixtures.py --pruefen`** (SONDE-006) · **A13
`pruefe_host_capabilities.py`** (SONDE-004) · **A14
`pruefe_kern_identitaetsfrei.py`** (SONDE-007a) · **A15/A16
`EqCopSunaNullTest` / `EqCopProbeeqNullTest`** und **A17
`pruefe_installer_manifest.py`** (SONDE-007b) · **A18
`pruefe_installer_gegenpfad.py`** (S9-Nacharbeit 23.08.: fährt den Gegenpfad
installieren↔Rückweg wirklich, in einer Sandbox unter `%TEMP%` — es wird nichts
installiert) · Identität · **B2
`EqCopStateMigrationTest`** (SONDE-006) · Hostkontext · Host-Probe (zählt
89 nur mit PNG-Ziel, sonst 85 — NAK-34) · Schema · **B8
`EqCopLebenslaufTest`** (SONDE-007b) · **B4 `EqCopQueueStressTest`** und **B9
`EqCopLoudnessGoldenTest`** (SONDE-008). **Die Prüfzahlen stehen im
jüngsten Manifest in `docs/beweise/`, nicht hier** (zuletzt
`SONDE-009.md`: 28/28 grün). Nicht im Kanon, aber vorhanden:
`EqCopAuxSpikeTest` (NAK-37), Shot (`--state` lädt einen Host-State vor dem
Render), PaintBench, PipeProbe, `pluginval --strictness-level 8`
(⚠️ liegt **nur** unter `%TEMP%\pluginval.exe` — NAK-26; wer es sucht, sucht
dort zuerst). **Zwei** Beine stehen noch als „geplant" (B6 DSP-Golden, B7
Transaktion, beide Phase P6) und werden Pflicht, sobald ihr Ticket sie baut —
B5 `EqCopAnalysisGoldenTest` ist seit SONDE-009 gebaut und läuft mit.

- Golden-WAVs einmalig: `py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav`
  (Erzeuger der Referenz `tools/analyze-track.py` liegt noch im FL-Studio-Repo — NAK-31).
- Editor-Sichtprüfung ohne FL: `EqCopShot.exe <ziel.png> [breite]`.
- **Broker-Betrieb:** `broker\target\release\eqcop-broker.exe [--bindungen <pfad>]`
  (Standard `%APPDATA%\evenacadia\nakama\eq-copilot-bindungen.json`). Pipe
  Ende-zu-Ende: `eqcop-broker-probe.exe 30` + `EqCopPipeProbe.exe
  "\\.\pipe\evenacadia.eq-copilot.m2probe"` — **immer der Probe-Pipename**, nie
  die Produktion (`…eq-copilot.v1`; zwei Broker auf einem Namen stehlen sich
  still Clients; Produktion verweigert per FIRST_PIPE_INSTANCE).
- **Version:** `project(… VERSION 0.3.0)` und `kPluginVersion` müssen eins sagen —
  ein Configure-Riegel in `eq-copilot/CMakeLists.txt` bricht sonst ab.
  **Installiert ist das Bundle vom 16.08.** (Hash `74D86BD5…`); nichts
  Neueres ist installiert — Installation bleibt ein User-Klick als Admin.
  Der Weg dafür ist seit S9 `eq-copilot\install\Install-Nakama.ps1`
  (manifestgetrieben, mit `-Pruefen` und `-Rueckweg`; NAK-32 geschlossen).
  Vor einer Installation gilt weiterhin NAK-41.
- **FL-Termine A/B sind gemessen (22.08.)** — Rohdaten `docs/beweise/termin-a/`,
  `termin-b/`; **Capabilityreport S4** `eq-copilot/identity/host-capabilities-fl-v1.json`
  (zehn §53.6-Bits mit Rohfeld, Kanon-Bein A13 misst sie gegen die Rohdaten):
  `host_context_presence`, `project_time_samples` **supported** ·
  `sample_accurate_automation` **unsupported** (FL legt nie >1 Punkt je Block
  in die Queue, zerteilt stattdessen Puffer bis 1 Sample) ·
  `presentation_latency` (gemeldet 3 924/4 410, aber kein Impulsgolden),
  `aux_priority_sidechain` und `aux_compare_pre` (getrennt + recall-stabil,
  aber PDC nie ausgeübt, Kanalreihenfolge nicht unterscheidbar — NAK-44,
  Termin A2), `float64_processing`,
  `contribution_aux` (ungemessen), `binary_telemetry`, `remote_control`
  **unsupported**. Host FL Studio 2026 26.1.4.5589, JUCE 8.0.9. Gate G0 (T3,
  eigene Session) steht noch aus. Die
  Wegwerf-Messgeräte `EqCopAuxSpike` (`NkSp`) und `EqCopHostProbe` (`NkHp`)
  liegen installiert unter `C:\Program Files\Common Files\VST3\`.

## Technik-Zement (was heute gilt, weil Code und Beweis es sagen)

- **Identität (SONDE-001):** Bundle `EQ-Copilot`, Codes `Evna`/`Eqcp`, beide
  Class-IDs, `JUCE_VST3_CAN_REPLACE_VST2=0` eingefroren; `NkPr`/`NkAc` für
  Suna/Probeeq reserviert; `EqCopIdentityTest` misst das gebaute
  `moduleinfo.json` UND den CMake-Quelltext; Schema-1-Goldens
  `eq-copilot/fixtures/identity/` sind seit 22.08. eingefrorene **Lade**-Fixtures
  (das Plugin speichert Schema 2).
- **Hostbrücke (SONDE-003):** gevendorter JUCE-8.0.9-Wrapper per Patch
  (`third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`, CRLF in 149/163
  Zeilen, `.gitattributes` hält ihn per `-text` bytegleich) um drei
  Beobachtungen erweitert; Gate `eq-copilot/cmake/NakamaBruecke.cmake`
  (unberührt ⇒ patchen + nachmessen · gepatcht ⇒ No-Op · fremd ⇒ Bauabbruch).
  Gegenseite `plugin/hostbridge/NakamaHostBridge.h` (JUCE-frei, 0 Allokationen).
  Im Produkt kompiliert, aber **ungenutzt** — nur `HostProbeProcessor` ist eine
  `Senke`; der Verbraucher kommt mit SONDE-008/009. ⚠️ `_deps` nie von Hand
  editieren; bei jedem JUCE-Update Patch neu beweisen.
  🔑 Zwei teuer bezahlte Regeln: (1) ein Puffer, der beim Überlauf hinten
  abschneidet, darf nie die Quelle eines Wertes sein, den ein Vertrag überleben
  lässt; (2) ein Zähler beschreibt, was der HOST geliefert hat, nicht was in
  unsere Struktur passte.
- **v3-Vertragsbaum (SONDE-005a):** `eq-copilot/schemas/v3/`, gelesen von drei
  Beinen (`pruefe_v3_vertrag.py`, `EqCopSchemaTest`, `broker/tests/
  contract_cross_language.rs`) gegen ein **handgeschriebenes** Manifest (164
  Fixtures seit der G1-Nacharbeit 24.08., vorher 153). 🔑 **`probe_descriptor`
  ist seit dem 24.08. eine über `measurement_position` diskriminierte Union**
  — die Aussageklasse folgt aus der Position, `insert`+`beitrag` ist damit
  kein gültiges Dokument mehr (Gate-7-Bruch, `docs/beweise/G1.md` §4.1/§10.1;
  bewusst **ohne** Versionsanhebung, Begründung §10.0).
  🔑 Ein Schema mit nicht implementiertem Schlüsselwort bricht den
  Ladevorgang. ⚠️ Bandgitter sind eingefrorene Hex-Zahlen, keine Rechenvorschrift.
  🔑 **Textriegel** = Stufe VOR dem Parser (8 Regeln, 59 Fälle in EINER Datei),
  weil JUCEs Zahlenleser überläuft. 🚨 Ein Riegel darf nie die Bibliothek
  befragen, gegen deren Verhalten er schützt.
- **FlatBuffers (SONDE-005b):** `.fbs` mit expliziten Feld-IDs, `flatc` auf
  COMMIT gepinnt, Codegen-Drift 0; zwei handgeschriebene Leser, weil FlatBuffers
  weder Enumbereiche noch Bitflags noch Feldbeziehungen verifiziert;
  `pruefe_fbs_feldids.py` schließt die Lücken „neue Tabelle ohne ids" und
  (T2-Runde 4) „neues Offsetfeld ohne Riegelzeile".
  🔑 Rusts Verifier kennt C++' Regel „May not point to itself" NICHT; der
  `strukturriegel` in `broker/src/telemetrie.rs` zieht sie über alle 15
  Offsetfelder nach — gemessen an 6215 Byte-Mutanten: 143 liefen auseinander,
  danach 0. 🔑 Ein Riegel, der STRENGER ist als das Bein, das er spiegelt,
  bricht denselben Vertrag wie einer, der schwächer ist.
- **State-Schema 2 (SONDE-006, 22.08.):** Vertrag
  `eq-copilot/schemas/state/nakama-state-v2.md`; das Plugin speichert
  `NakamaState{schema=2}` und migriert `EqCopilotState{schema=1}` **rein**
  (Goldens bytegleich). Unbekanntes Major / verletzte Kind-Matrix ⇒ read-only
  mit Originalbytes, keine Pipe, sichtbar im Editor. Jede persistente Änderung
  meldet Host-Dirty (`withNonParameterStateChanged`) — vorher kam
  `updateHostDisplay` im Plugin nicht vor. Parameterbestand 109 IDs
  handgeschrieben (`nakama-parameter-v1.json`), C++-Tabelle deckungsgleich
  gemessen; heute trägt kein Bundle Hostparameter. `state_hash` = SHA-256 über
  RFC-8785-Kanon mit **eigenem JSON-Leser** — 🔑 JUCEs Zahlenleser flusht
  Subnormale und verweigert `""` als Schlüssel; ein Hash, den drei Sprachen
  bilden, darf die Bibliothek nicht befragen, gegen die er schützt. Drei Beine
  (C++, Python `rfc8785`, Rust `serde_json_canonicalizer`) bytegleich gegen
  einen Korpus, dessen RFC-Zeilen den **vom RFC gedruckten** Text tragen.
  ⚠️ NAK-41: ein Schema-2-Projekt verliert im 16.08.-Build still seine
  Identität — vor der Installation wissen.
  ⚠️ **Messpositionsmatrix §2.2 seit 24.08. (G1-Nacharbeit):**
  `post_fader_contribution` ist für **keine** Klasse mehr erlaubt — zwei
  unabhängige Riegel in `positionErlaubt` (Capability `contribution_aux` ist
  gemessen `unsupported`; Klassenmatrix fail-closed). Vorher ließ
  `passive_probe` sie zu, und das war die State-Hälfte des Gate-7-Bruchs. Ein
  Altstand mit dieser Position wird **read-only mit Originalbytes**, verliert
  also nichts. Der Wortschatz bleibt im v3-Vertrag: verboten ist nicht das
  Wort, sondern die Behauptung einer Instanz ohne den Bus. **Offen (NAK-79):**
  welche Klasse sie führen darf, wenn es den Bus gibt — Produktfrage,
  `SONDE-011`.
- **Gemeinsamer Kern (SONDE-007a / S8, 22.08.):** `NakamaKern` ist eine echte
  Static-Lib (`add_library(… STATIC)`, `plugin/CMakeLists.txt`) mit den vier
  geteilten Quellen (`state/*.cpp` + `vertrag/NakamaVertrag.cpp`), einmal
  übersetzt statt je Ziel; angebunden über `nakama_kern_anbinden()`.
  Werkzeug in `cmake/NakamaKern.cmake`. 🔑 **Der Kern übersetzt gegen
  JUCE-KÖPFE, nicht gegen JUCE-Module** — JUCE-Module sind INTERFACE-Libs,
  deren `.cpp` in JEDES konsumierende Ziel hineinkompiliert wird; eine Lib,
  die sie linkt, trägt eine zweite Kopie. Die Kopf-Fassade
  `nakama_kern_juce_fassade()` leitet Includes und Defines aus den
  Modulzielen ab und lässt deren Quellen liegen (gemessen: `$<COMPILE_ONLY:>`
  streift `INTERFACE_SOURCES` **nicht** ab). ⚠️ **Fünf Riegel, verschiedene
  Fragen, keiner ersetzt einen anderen:** K1 `state/NakamaKernRiegel.h`
  (46 Makros namentlich, im Übersetzer) · K2 Linkhülle per Regex, Configure ·
  K2b gleiche JUCE-**Defines** wie der Verbraucher · K2c gleiche
  **Empfehlungsschalter** (`juce_recommended_*`; ohne `lto_flags`) · K3
  Kanon-Bein A14 misst das **Artefakt** (nur K3 sähe ein Stringliteral, das nie
  ein Makro war). 🔑 Ein Riegel, der etwas NICHT findet, sagt nichts, bis
  gezeigt ist, dass er überhaupt etwas finden kann — A14 trägt seine Gegenprobe
  im Bein, und die hat seine erste Fassung widerlegt (CIDs liegen als 16 rohe
  Bytes in COM-vertauschter Ordnung, nicht als Hextext). 🔑 **Eine Static-Lib
  erbt die PUBLIC-Schalter ihrer Verbraucher nicht mehr** — der Kern übersetzte
  nach dem Umbau als einziger Code im Baum unter `/W1` statt `/W4` (T2-Befund
  23.08.), K2c hält das jetzt fest. Manifest `docs/beweise/SONDE-007a.md`;
  **T2 gefahren 23.08.: NEEDS_WORK, fünf Befunde geschlossen (§5/§6), kein
  PASS auf den neuen Stand**.
- **Drei gebaute Bundles (SONDE-007b / S9, 23.08., Abschnitte 1+2 von 3):**
  Die Identität kommt aus `identity/plugin-identities-v1.json` — das Bauskript
  trägt **kein** Identitätsliteral mehr (NAK-52 geschlossen, `8e32baf`);
  Werkzeug `cmake/NakamaIdentitaet.cmake`, es **erfindet nichts**: ein Feld auf
  `null` bricht den Configure ab. 🔑 Die Class-IDs liest es absichtlich NICHT —
  die rechnet JUCE aus Hersteller- und Plugin-Code, und genau das misst
  `EqCopIdentityTest` am gebauten `moduleinfo.json` gegen dieselbe Datei nach:
  zwei Wege zur selben Zahl, sonst wäre der Test eine Tautologie. Neu gebaut:
  **`Nakama Suna.vst3`** (`NkPr`) und **`Nakama Probeeq.vst3`** (`NkAc`) aus
  EINER geteilten Quelle `plugin/sonde/` über zwei dünne Target-Schichten;
  unterschieden nur durch Identität, Produktklasse (Define — der geteilte Code
  darf sie nicht wissen) und VST3-Kategorie. Die in S2 nur *gerechneten* CIDs
  sind damit erstmals an Artefakten gemessen (§53.5 „P1 verifiziert das erste
  Moduleinfo"), und **kein Bundle trägt eine fremde Ziel-CID** — die
  Artefakt-Seite von §53.4. K2b/K2c messen seit S9 gegen **jeden** Verbraucher
  des Kerns, nicht gegen eine Stichprobe — heute **15** (gemessen 24.08.; sie
  wächst mit jedem neuen Verbraucher, hier steht nur eine Momentaufnahme —
  am 23.08. waren es 14), und die Zahl ist keine
  abgeschriebene: `plugin/CMakeLists.txt:156` trägt jeden
  `nakama_kern_anbinden()`-Aufruf in eine GLOBAL-Property ein, `:627` zählt sie
  und der Configure-Lauf meldet sie („K2b/K2c gegen alle N Verbraucher
  gemessen"); eine leere Liste bricht ab, statt still ins Leere zu messen.
  ⚠️ Beide neuen sind heute
  **Passthrough ohne Hostparameter und ohne Editor**: Probeeqs EQ-DSP gehört zu
  P6, die Oberflächen kommen aus Figma. 🔑 Der Gegenpfad speichern↔laden fand
  zwei echte Fehler, die sonst ins Bundle gegangen wären — `active_probe`
  verlangt laut Kind-Matrix §2.1 genau ein `Parameters`-Kind, und
  `Zustand::parameters` war als `{}` deklariert, also **Nullen statt
  `standardSatz()`** (0 Hz liegt außerhalb von `band.0.freq_hz`). Manifest
  `docs/beweise/SONDE-007b.md`.
- **Lifecycle-Klassifikation (SONDE-007b Abschnitt 3 / S9, 23.08.):**
  `state::Lebenslauf` (im Kern) ist §53.5 als Code — `unclassified` beim Laden
  und **audio-neutral**; Schema-1 `sensor|pre|post` → `legacy` („immer
  passiv"); Schema-1 `hub` und bestätigter Schema-2-Main-State → `main`; ein nie
  gespeicherter Stand wird `main` erst nach **geöffnetem Editor UND** expliziter
  Initialisierung (= der User wählt im Editor die Rolle `hub`). 🔑 **„Ein
  Scannerlauf klassifiziert nicht" ist keine Sonderbehandlung, sondern die
  Folge der Startbedingung** — ein Scanner ruft nie `setStateInformation` und
  öffnet nie einen Editor; das Bein fährt genau diese Sequenz. 🔑 „Bestätigt"
  ist der Leser selbst: `lade()` hat die Kind-Matrix §2.1 geprüft, eine zweite
  Prüfung wäre eine Kopie, die auseinanderläuft. Der Automat hängt an zwei
  Dingen — der Hör-Markierung (Audio, siehe unten) und `darfBrokerStarten()`
  (`main` + offener Editor; **heute gibt es keinen Spawn-Pfad**, SONDE-010 hängt
  ihn dort an). `Bundle::eqcp()` ist kein Literal in `src/` mehr:
  `NAKAMA_BUNDLE_MAIN` kommt aus der Target-Schicht, ein `#error` fängt ein
  Ziel, das sich nicht erklärt. Kanon 21 → **23** (A17, B8).
- **Installer-Manifest (SONDE-007b Abschnitt 3, 23.08., NAK-32 geschlossen):**
  Vertrag `eq-copilot/schemas/installer/nakama-installer-v1.md`, Auslieferung
  `eq-copilot/install/nakama-installer-v1.json`, Ausführender
  `install/Install-Nakama.ps1` (beide versioniert; Bundles, Rollbacks und
  Laufergebnisse bleiben Maschinenartefakte). 🔑 **Dieselbe Regel wie NAK-52,
  nur auf der Auslieferungsseite:** das Manifest trägt weder Produktnamen noch
  Viercodes noch Class-IDs. Weil ein Pfad den Bundlenamen zwangsläufig enthält,
  ist der Riegel zweiteilig — A17 **rechnet** jeden Quellpfad aus `cmake_ziel` +
  Identitätsdatei nach *und* verbietet Codes/CIDs; einzeln wäre jede Hälfte
  löchrig. `sha256: null` heißt **nicht ausliefer-bar** (das Skript bricht ab,
  bevor es etwas anfasst); der committete Stand trägt bewusst `null`.
  Gegenpfad `-Rueckweg` sichert vor dem Tausch und **verweigert** einen
  Rückfall auf kleineres *oder unbekanntes* State-Schema (NAK-41-Riegel;
  unbekannt zählt wie älter). Broker nach `Program Files`, nicht
  `%LOCALAPPDATA%` — er ist ab SONDE-010 ein Spawn-Ziel.
- **Stand S9:** alle drei Bauabschnitte gebaut, `pluginval` 8 an allen drei
  Bundles SUCCESS, Kanon 23/23 **im damaligen Umfang** (heute 26 — die Zahl ist
  gewachsen, nicht das Urteil). **T1 ist gefahren und T2s vier Befunde sind
  geschlossen** (`docs/beweise/SONDE-007b.md` §6). Ein **PASS steht weiter aus**:
  den darf nur ein frischer Prüfer geben, der weder gebaut noch nachgearbeitet
  hat (§6.7). Also „gebaut und nachgebessert", nicht „abgenommen".
- **Hör-Markierung (0.3.0):** färbt auf Klick das Monitorsignal von Gen;
  Verriegelung im Code seit S10–11 **`klassifiziertAlsMain ∧`** `(echtzeitOk ∨
  test) ∧ spielt ∧ ¬isNonRealtime ∧ (editorOffen ∨ test)` —
  `PluginProcessor.cpp:300-306`. Der Term `spielt` ist dabei die UND-Verknüpfung
  **zweier** Stempelbits (`:272`): `stempel.spieltGültig ∧ stempel.spielt`, und
  `spieltGültig` selbst ist `processContextPresent ∧ playing.gueltig` (`:332`) —
  also „der Host hat einen Kontext geliefert UND sein Spielbit ist gültig UND es
  spielt";
  Analyse-Abgriff davor; Render bitidentisch (MarkierungTest). Jede weitere
  Audio-Ausnahme von Gen/Suna braucht denselben Beweisstandard.
  ⚠️ **Der erste Term ist neu (23.08., §53.5 „audio-neutral"):** eine
  `legacy`-Instanz (v2-Rolle `sensor`/`pre`/`post`) färbt **nicht mehr**. Wer
  die Markierung will, wählt im Editor die Rolle `hub`. Der installierte
  16.08.-Stand ist davon unberührt. Der Test-Schalter `testForciereEchtzeit`
  umgeht diesen Term absichtlich **nicht** — er umgeht nur, was an der Wanduhr
  hängt.
  ⚠️ **Das `∨ ¬hatTransport` ist gefallen** (User 22.08., Hub `U10`: „Nein, nur
  mit Signal") — **umgesetzt mit SONDE-008** (23.08.); an seiner Stelle steht ein
  gültiges „spielt". NAK-35 und NAK-24 sind damit geschlossen; die Formel oben
  ist der Ist-Stand, an der Quelle gelesen. In FL ändert das nichts (dort lag
  `hatTransport` ab dem ersten Block auf true, der fail-open-Zweig war tot) —
  **headless färbt ohne Playhead jetzt nichts mehr**. 🔑 Gedeckt ist der Term
  von genau EINEM Bein: `EqCopMarkierungTest` **T11**, erst in der
  SONDE-008-Nacharbeit gebaut. Davor ließ sich das fail-open zurückbauen, ohne
  dass eines der vier Audio-Beine rot wurde (T2-2 in NAK-58) — ein Term ohne
  Bein ist eine Zusage, die sich unbemerkt zurücknehmen lässt.
- **Editor heute:** Material-Kit-Front, festes Verhältnis 750:520, frei ziehbar
  600×416…1950×1352 (`PluginEditor.cpp:176-183`) — Provisorium; für die
  neue UI gilt die abgenommene Größe 760×430 („so oder so die zweitkleinste
  ist das vernünftigste", 20.08.); ob feste Stufen oder freie Skalierung,
  ist Claude-Empfehlung, kein Entscheid (User: „oder eine freie
  skalierungsform eben").

## Invarianten — tragend, jede Runde präsent

- **Schemas sind Verträge** (v2: `eq-ipc` · `eq-measurement` · `eq-report` ·
  `eq-snapshot` v3 · `eq-aggregat`; v3: `schemas/v3/`): neue Felder ⇒ ERST
  Versionierung; alte Snapshots laden ohne die Felder; unbekannte Felder
  zerstören alte Consumer nicht; Save + Load im selben Änderungssatz testen.
  In v3: `additionalProperties: true` nur mit `maxProperties`; Discriminator,
  Zieladresse, Revision, Capability nie additiv.
- **Engine kennt keine Optik:** AnalyseEngine liefert kohärente MessSnapshots
  (~20 Hz Leichtpfad + 250-ms-Schwerauswertung); der Editor hält NUR
  Anzeigezustand und malt nur bei neuer Snapshot-Revision.
- **Gegenpfade** (Invariante aus dem archivierten Bauplan 2.0 §11.5, gilt weiter): starten↔stoppen · öffnen↔schließen ·
  speichern↔laden · aktivieren↔abklingen · installieren↔Rollback — beide
  Hälften im selben Änderungssatz.
- **Zeit ist Aktivzeit:** Zonen-Ticks je 1 s AKTIVER Musikzeit (deterministisch,
  GoldenTest-beweisbar).
- **NaN-Ehrlichkeit:** NaN-Riegel mit Zähler, Nyquist-Kappe; < 5 endliche
  Nachbarn ⇒ keine Basislinie.
- **Probe-Pipe ≠ Produktions-Pipe** · **`ltasReferenzDb` ≠ Sollkurve** (keine
  globale Zielkurve; Befunde messen gegen ihre eigene Schulterlinie) ·
  **paint()-FPS ≠ Datenkadenz** (erst PaintBench, dann optimieren) ·
  **Musterblatt-/Demo-Daten ≠ Plugin-Daten** · **FL zeigt MIDI 60 als C5**.

## Design-Arbeitsmodell (seit 21.08.2026; `design/` im Repo seit 22.08.)

1. **Figma (User) ist die Quelle.** Aktuelle Stände liegen beim User
   (Figma-Depot `Nakama-Design`) und werden per Figma-MCP nach
   `design/assets/figma/` geholt. Claude erfindet keine Richtung, keine Metapher, keine
   Farbwelt — Gesetz aus vier teuren Proben (17.08.): Claudes freie
   Bilderfindung ist Mode-Collapse; Vision kommt vom User.
2. **`design/` übersetzt** Figma in lebende Blätter und prüft Zustände,
   Größen, Grenzfälle (Regeln: `design/LIES-MICH.md`); Abnahmen in
   `design/abnahmen/` sind bindend und tragen das User-Wort. Von dort
   spiegelt nichts ins Plugin (`eq-copilot/`), bis eine Abnahme es sagt.
3. **Unter `eq-copilot/design/`** gibt es keine Design-Arbeit mehr: es hält
   die Material-Kit-Kette (Provisorium), das Archiv und die geparkte
   Prisma-Studie. Der Hook `kreativ-schleuse.sh` blockt Edits darunter ohne
   frischen Freigabe-Marker — das ist gewollt. `design/` ist davon frei;
   dort gilt die Prototyp-Schleuse (`tools/hooks/prototyp-schleuse.sh`).
4. **Geparkt, nie ungefragt reaktivieren:** Prisma-Studie (inkl. Hörkompass,
   Unicorn-Lichtwelt, ST-Map-Refraktion, Schlieren), Tiefenfeld, Bauplan 2.0,
   Kunstwerk-Studie, Feld-Alphabet, Proben P01–P04.

## Maschinen-Landminen

- ⚠️ **PowerShell `Start-Process -ArgumentList` quotiert NICHTS** — jedes
  Argument mit Leerzeichen selbst escapen (`Argument-Quoten` in `tools/beweise.ps1`).
- ⚠️ **Bash-Heredoc frisst Backslashes in Windows-Pfaden** (`\v` → 0x0B);
  Pipes maskieren Exitcodes (`${PIPESTATUS[0]}`).
- ⚠️ **`core.autocrlf` auf dem Zweitrechner**: `.gitattributes` hält Patch,
  v3-Baum und Fixtures per `-text` bytegleich — wer neue bytegleich geprüfte
  Dateien anlegt, trägt sie dort ein.
- 🌀 **Keine GPU-Batch-Render-Loops auf der Arc A770** (Lüfter-Failsafe klemmt
  auf 100 %; Fix `Win+Ctrl+Shift+B`, sonst Kaltstart). Renders nur Software/CPU.

## Read before working on

| Bereich | Zuerst lesen |
|---|---|
| Planstand (gerechnet) · offene Fragen an den User | `docs/PLAN-STAND.md` (Ansicht) · `docs/plan/plan.json` + `docs/plan/fragen.json` (Quelle) · `docs/plan/LIES-MICH.md` |
| Einstieg, der eine nächste Schritt | `docs/NEXT-SESSION.md` |
| Plugin heute (Architektur, Datenfluss, IPC, Tests) | `docs/plugin-wissen.md` |
| Sondenfamilie: Technik + Phasen (Fassung 0.4 + Errata 21.08.) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` · `docs/bauaufteilung-sonden.md` |
| v3-Verträge, Textriegel, Bandgitter, Fixtures | `eq-copilot/schemas/v3/README.md` · `…/flatbuffers/README.md` |
| Beweise je Ticket (rohe Ausgabe) | `docs/beweise/` |
| FL-Termine A/B (gemessen 22.08.) + Capabilityreport | `docs/beweise/termin-a/` · `docs/beweise/termin-b/` · `docs/beweise/SONDE-004.md` · `eq-copilot/identity/host-capabilities-fl-v1.json` |
| Benchmark-Mechaniken (Median-Basislinie, Zonen) | `eq-copilot/docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md` |
| Offene Punkte | `docs/offene-punkte.md` |
| Design der Apps | `design/LIES-MICH.md` + `design/abnahmen/` |
| Verlauf (nur zum Verstehen, nie als Vorgabe) | `docs/archiv/`, `eq-copilot/design/archive/`, `eq-copilot/design/prisma-studie/STATUS.md` |

## Arbeitsweise

- **Code ist die einzige Wahrheit** — Datei öffnen, bevor auf Doku-, Memory-
  oder Audit-Behauptungen gehandelt wird (~25 % der AI-Auditbefunde hier waren falsch).
- **Entscheide nur mit Zitat.** Was der User entschieden hat, steht mit Datum
  und Wortlaut im Register oben bzw. in `design/abnahmen/`. Alles
  andere ist Vorschlag, Arbeitsannahme oder Studie und heißt so — auch in
  Commit-Messages („Vorschlag:", nicht „Abnahme:").
- **Eine Wahrheit, ein Ort.** Zahlen (Testzahlen, Versionen, Fixturezahlen)
  stehen dort, wo sie gemessen werden (Manifeste, Code); Docs verweisen.
  Keine statischen Kopien in Hooks oder Memory.
- **Fortschritt nur mit Beleg aus dieser Session** (Testlauf, Render, Diff);
  Status kommt nach dem Beweis, nie davor. Selbstaudit nach jedem Commit.
- **Im Auftrag bleiben:** keine Neben-Refactors, keine Umbenennung der
  `EqCop*`-Legacy-Namen nebenbei (NAK-30 ist der Ort dafür).
- Out-of-scope-Funde → `docs/offene-punkte.md` (datierte Zeile mit ID).
- Nach großen Schritten `docs/plugin-wissen.md` nachziehen; Session-Memo ins
  Memory nur für Wissen, das NICHT im Repo steht (eine Lehre pro Datei, mit
  dem Warum; Falsches löschen statt stapeln).
- Alle Texte dieses Projekts (Docs, Commits) auf Deutsch; Produkt-Texte Englisch.

<!-- OPENWIKI:START -->

## OpenWiki

See [AGENTS.md](AGENTS.md) for OpenWiki agent instructions.

<!-- OPENWIKI:END -->
