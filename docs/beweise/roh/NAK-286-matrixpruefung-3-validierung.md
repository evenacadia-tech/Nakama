# NAK-286 — Validierung der Codex-Matrixprüfung 3 (lesend, 14.09.2026)

Ticket NAK-286 (Plan S25e), Etappe 1. HEAD
`34c7f63585d2343b5fabb214899c298661dbcbca`, Branch `master`. **Nur gelesen** —
nichts gebaut, kein Test, kein Skript, keine FL-Instanz, Git nur lesend;
geschrieben wurde ausschließlich diese Datei. Die untracked Ordner
`briefing-hub/` und `nimbalyst-local/` blieben unberührt.

Geprüft werden die **zwei P2-Befunde** des Urteils
`docs/beweise/roh/NAK-286-matrixpruefung-3-34c7f63.txt` (Reihenfolge wie in der
Rohdatei).

**Zeilenversatz Urteil ↔ HEAD: null für jede zitierte Zeile.** Das Urteil
zitiert den Stand `961eae38`; `git diff 961eae38 34c7f635 -- docs/beweise/NAK-286.md`
zeigt genau zwei Änderungen: die Kopfzeile „Etappe" (Zeile 10, eine Zeile gegen
eine Zeile) und einen Einschub von 13 Zeilen ab Zeile 1229 (§15). Beide
Befunde zitieren die Zeilen 358 und 416, also weit oberhalb des Einschubs.
Stichprobe: Urteil `:416` = HEAD 416 („Materialzeit einer Antwort"), Urteil
`:358` = HEAD 358 (F-5 Snapshot). Das Urteil nennt seinen Prüfstand außerdem
selbst (`…-matrixpruefung-3-34c7f63.txt:3`: „HEAD blieb vor/nach der Prüfung
identisch bei `34c7f63585d2343b5fabb214899c298661dbcbca`").

---

## Befund 1 — „Prüfe die fortlaufende Hostzeit vor der Berechnung von K"

**Wörtlich (Urteil), `docs/beweise/roh/NAK-286-matrixpruefung-3-34c7f63.txt:9-10`:**

> `[P2] Prüfe die fortlaufende Hostzeit vor der Berechnung von K —
> C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:416-416`
> DEFEKT an F-28/M-67/M-68: Bei FL-Teilblöcken mit identischem
> `projectSampleStart` trägt die Herleitung nicht.
> `eq-copilot/plugin/core/StampedAudioQueue.h:810-823` und
> `eq-copilot/plugin/core/analysis/featureengine/Zeit.h:211-229` akzeptieren
> solche Blöcke ausdrücklich ohne Grenze. Zwei seit Reset verarbeitete
> 256-Sample-Blöcke mit Startstempel 1024 ergeben daher E = 1280, n_L = 512
> und K = 768 statt des tatsächlichen Anfangs 1024; die Prüfung K < 0 greift
> nicht. Damit beziehen sich Referenzausschnitt und U_unten auf anderes
> Material. Ein gültiges Zeitbit reicht nicht: Die Hostzeit muss über das
> gesamte integrierte Material nachweislich fortlaufen, andernfalls muss der
> Anker nach P-5 roh bleiben. Das betrifft auch die Kohärenzinvariante aus
> CLAUDE.md:186.

### Quelle an HEAD

**(1) Die Queue nimmt den stehenden Stempel ausdrücklich an —
`eq-copilot/plugin/core/StampedAudioQueue.h:810-833`
(`Blockquarantaene::schliesstAn`):**

```cpp
        if (beideZeit && (beideSpielen || beideStehen)
            && neu.projectSampleStart != gehalten.projectSampleStart)   // s. u.
        {
            …
            if (neu.projectSampleStart
                    != gehalten.projectSampleStart + (std::int64_t) gehalten.sampleCount)
                return false;
        }
        return true;
        // ⚠️ WARUM `neu.start == gehalten.start` KEIN Bruch ist … FL zerteilt
        // Puffer bis auf 1 Sample, um Automationspunkte zu setzen … Die
        // Teilstuecke tragen dieselbe Projektzeit und sind trotzdem
        // LUECKENLOSE Fortsetzung.
```

Die Kontinuitätsprüfung steht **hinter** einem `!=`: ein Block, dessen Stempel
gleich bleibt, wird gar nicht geprüft und ist nie ein Bruch.

**(2) Dieselbe Lesart in der Grenzerkennung —
`eq-copilot/plugin/core/analysis/featureengine/Zeit.h:206-229`
(`grenzeZwischen`):**

```cpp
    // Zeitsprung.  Dieselbe Lesart wie `Blockquarantaene::schliesstAn` —
    // eine STEHENDE Zeit sagt nichts …
    if (beideZeit && spieltBekannt && neu.projectSampleStart != alt.projectSampleStart)
    {
        …
        if (ueberlauf || neu.projectSampleStart != erwartet)
            return istLoopWrap (alt, neu) ? Grenzgrund::loopWrap : Grenzgrund::zeitSprung;
    }

    // Lokale Analyseluecke — zuletzt, weil die schwaechste Aussage.
    if ((neu.flags & echtzeit::kFlagLueckeDavor) != 0
        || neu.segment != alt.segment
        || neu.stromVon != alt.stromVon + alt.sampleCount)
        return Grenzgrund::lokaleLuecke;
```

Der einzige lückenlose Zähler, der auch den stehenden Fall trägt, ist der
**lokale Strom** `stromVon` (`StampedAudioQueue.h:160-163`: „Lokaler, monotoner
Strom seit `zuruecksetzen()`") — er zählt Samples, nicht Hostzeit. Es gibt an
HEAD keine Zeile, die den stehenden Stempel gegen die Zahl der seither
verarbeiteten Samples aufrechnet.

**(3) Was n_L zählt — `eq-copilot/plugin/src/AnalyseEngine.cpp:744-746`
(`fuelleBasis`):**

```cpp
    s.verarbeiteteSamples = samplesGesamt;
    …
    s.gesamtSekunden = (double) samplesGesamt / sr;
```

`samplesGesamt` steigt in `verarbeite()` um die Samples **jedes** an die Engine
gegebenen Blocks (Worker: `eq-copilot/plugin/src/prozessor/Analyse.cpp:275-281`,
`eq-copilot/plugin/sonde/SondeProcessor.cpp:602-604`) — auch um die Samples
zweier Teilstücke mit identischem Stempel. n_L = `gesamt_sekunden` × 44 100 ist
also die Zahl der verarbeiteten Samples, **E** dagegen `projectSampleStart` +
`sampleCount` des letzten Blocks (F-6, `StampedAudioQueue.h:158-174`). Beide
Seiten der Differenz K = E − n_L laufen bei stehendem Stempel auseinander: je
gespaltenem Hostpuffer um die Summe der Längen aller Teilstücke außer dem
letzten. Die Ein-Block-Quarantäne verschiebt beide Seiten um denselben Block
und hebt sich in der Differenz auf; die Beispielrechnung des Urteils
(zwei **verarbeitete** 256-Sample-Blöcke, E = 1280, n_L = 512, K = 768 statt
1024) rechnet an HEAD nach.

**(4) Was die Belege wirklich sagen.**

`eq-copilot/identity/host-capabilities-fl-v1.json:54` und `:57`
(`sample_accurate_automation`, Termin B):

> „automation.punkte_gesamt = 83303, max_punkte_pro_block = 1,
> bloecke_mit_mehrpunkt = 0, kleinster_offset = 0, groesster_offset = 0,
> samplegenau_belegt = false; bloecke.blockgroesse_min = 1,
> blockgroesse_max = 4096" …
> „FL liefert nie mehr als einen Punkt je Block und nie einen Offset ungleich 0
> - es zerteilt stattdessen die Puffer an den Automationspunkten bis auf 1
> Sample (gemessen ~180-190 Samples je Block bei 140 bpm, 44100 Hz)."

Gemessen ist also **die Zerteilung**, nicht die Zeit der Teilstücke.
`eq-copilot/identity/host-capabilities-fl-v1.json:47` (`project_time_samples`)
belegt nur `gueltig_immer.project_time_samples = true` und zählt Sprünge — das
ist genau das „gültige Zeitbit", das der Befund als unzureichend bezeichnet.

Der Repostand sagt das ausdrücklich. `docs/plugin-wissen.md:917-919`:

> „**Neu offen aus SONDE-008:** ob FL die Projektzeit über die Teilstücke eines
> zerteilten Puffers fortschreibt, ist ungemessen — beide Fälle sind abgedeckt,
> der reale zeigt sich am Zähler `analyseKontinuitaetsbrueche()` (NAK-56)."

`docs/offene-punkte.md:150` (NAK-56, Karte U13):

> „Ob die Teilstücke dabei dieselbe `projectTimeSamples` tragen oder sie
> fortschreiben, steht in den Rohdaten **nicht** — die Messung war auf
> Automationspunkte gerichtet, nicht auf die Zeit der Teilstücke. `SONDE-008`
> deckt beide Fälle ab, ohne zu raten: fortschreitende Zeit ⇒ normale
> Fortsetzung, **stehende** Zeit ⇒ keine Aussage (der lokale monotone Strom
> trägt, §53.6)."

Das ist die schärfere Fassung des Befunds: es ist **nicht belegt, dass FL
Teilblöcke mit identischem Stempel liefert — es ist ebenso wenig belegt, dass
es das nicht tut.** Der Fall ist offen, der Code deckt ihn bewusst ab, und K
= E − n_L setzt stillschweigend voraus, dass er nicht eintritt.

**(5) Einen Fortlaufmesser gibt es an HEAD — aber nur je Rahmen.**
`eq-copilot/plugin/core/analysis/featureengine/Zeit.h:568-587`
(`rahmenZeitBelegen`) prüft exakt die Bedingung, die der Befund verlangt:

```cpp
    if (rahmenProjektDurchgehend && blockProjektSpanneGueltig (block))
    {
        std::int64_t erwartet = 0;
        rahmenProjektDurchgehend = projektVorwaerts (
                                      rahmenStartBlock.projectSampleStart,
                                      rahmenSamples, erwartet)
                                && erwartet == block.projectSampleStart;
    }
    else
        rahmenProjektDurchgehend = false;
```

Startstempel plus **akkumulierte Samples** muss den Stempel des nächsten Blocks
treffen; bei stehendem Stempel fällt das Bit. Es trägt in
`eq-copilot/plugin/core/analysis/featureengine/Frame.h:215` den Support und
entscheidet in `:250-261`, ob der Transportstempel überhaupt eine Projektzeit
führt:

```cpp
    if (gesamt.projektGueltig)
    {
        t.zeitbasis = Zeitbasis::project_samples;
        t.project_sample_start_gesetzt = true;
        …
    }
    else
    {
        t.zeitbasis = Zeitbasis::local_monotonic;
        // KEIN `project_time`-Bit und KEIN `project_sample_start`.
    }
```

Daraus folgt eine Asymmetrie, die der Befund nicht nennt und die ihn stützt:
das **bestehende** Feld `frame.projekt_sample_start` (F-6, aus
`aktuell.transport`) wird im FL-Teilblockfall von selbst null und damit
ehrlich; das in der Matrix-Nacharbeit 2 **neu** eingeführte
`frame.material_ende_projektsample` liest `projectSampleStart` + `sampleCount`
roh aus dem `StampedBlock` und geht an genau diesem Ehrlichkeitsmechanismus
vorbei. `rahmenProjektDurchgehend` selbst trägt die Zusage aber nicht: sein
Geltungsbereich ist ein Merkmalsrahmen, K spannt das ganze Material seit dem
letzten Rücksetzen.

### Gegenstand

**F-28, HEAD-Zeile 416** („Materialzeit einer Antwort", gekürzt):

> „**E** = `frame.material_ende_projektsample` (F-6): Hostposition am Ende des
> letzten vor der Publikation verarbeiteten Blocks
> (`eq-copilot/plugin/core/StampedAudioQueue.h:158-174`; **die Hostzeit ist in
> FL immer gültig**, `eq-copilot/identity/host-capabilities-fl-v1.json:47`);
> null → der Anker ist nicht lesbar … **Kopfverlust K = E − n_L**, eigener
> Term: die Hostframes vom Umlaufbeginn (Hostposition 0) bis zum ersten Sample,
> das die Engine seit ihrem letzten Rücksetzen verarbeitet hat … **Das Material
> der Engine ist seit dem Rücksetzen lückenlos, weil jede Lücke und jeder
> Zeitsprung eine Grenze und damit ein Rücksetzen ist** (`Zeit.h:206-229`).
> K < 0 heißt: die Engine hat Samples ohne fortlaufende Hostzeit mitgezählt,
> etwa aus dem Stillstand vor dem Start — Szenario-Voraussetzung (Exit 5) mit
> K roh."

**§7.1 T-28, HEAD-Zeile 954** (Begründung des Ankerwegs):

> „Eine Schranke über die Blocklänge trüge K nicht: der gemessene Umlauf beginnt
> am Wiedergabestart nach der Transportkante … und Samples ohne fortlaufende
> Hostzeit — etwa aus dem Stillstand vor dem Start — verschöben jede Schranke;
> **die Differenz macht sie als K < 0 sichtbar.**"

**M-67, HEAD-Zeile 608** (Vorbedingung und Bänder): „U_unten ≤
`snapshot.aktiv_sekunden` mit U_unten als Minimum über alle 4 410 Rasterversätze
ab s0 = K − v"; „Referenzausschnitt [max(0, K − v), K + n − v) mit
K = `frame.material_ende_projektsample` − n_L (§13.2 P-7)"; Urteilsweg
„VORAUSSETZUNG … bei K < 0".
**M-68, HEAD-Zeile 609:** „jedes Feld über den Referenzausschnitt des schweren
Ankers n_S … (Ausschnitt nach P-7 mit K aus M-67)".
**M-81, HEAD-Zeile 540:** Zusage „K = `material_ende_projektsample` −
`gesamt_sekunden` × Samplerate = H, im zweiten Durchlauf H2" — der Zustand der
Zeile nennt ausdrücklich „`projectSampleStart` **lückenlos** ab einer bekannten
Hostposition H", prüft die stehende Zeit also gerade nicht.

### Status: BESTÄTIGT

Drei Prüfpunkte, alle an HEAD nachgerechnet:

1. **Der Code akzeptiert den Fall.** `StampedAudioQueue.h:810-833` und
   `Zeit.h:213` schließen den stehenden Stempel ausdrücklich von der
   Kontinuitätsprüfung aus (Antwort auf Teilfrage (a)). Die Engine zählt beide
   Teilstücke voll mit (`AnalyseEngine.cpp:744-746`), n_L sind die verarbeiteten
   Samples.
2. **Der Fall ist nicht ausgeschlossen.** Antwort auf Teilfrage (b): Weder
   `host-capabilities-fl-v1.json` noch `docs/beweise/termin-a/` bzw. `termin-b/`
   belegen die Zeit der Teilstücke; `docs/plugin-wissen.md:917-919` und
   `docs/offene-punkte.md:150` (NAK-56, Karte U13) führen sie als **ungemessen**.
   Das Urteil behauptet an dieser Stelle mehr, als belegt ist („FL liefert
   Teilblöcke mit identischem `projectSampleStart`"); für die Zusage macht das
   keinen Unterschied, sondern verschärft sie: F-28 leitet K aus einer Zeile ab,
   die den einen tragenden Fall gar nicht prüft.
3. **Der Beweisweg trägt die Zusage nicht** (Teilfrage (c)). Der Satz „Das
   Material der Engine ist seit dem Rücksetzen lückenlos, weil jede Lücke und
   jeder Zeitsprung eine Grenze … ist (`Zeit.h:206-229`)" ist an der zitierten
   Stelle wahr und für K dennoch unzureichend: Lückenlosigkeit im **lokalen
   Strom** (`stromVon`) ist bewiesen, Lückenlosigkeit der **Hostzeit** nicht.
   Genau diese zweite Aussage braucht K = E − n_L. Der Riegel „K < 0" (F-28:416,
   T-28:954) fängt den Fall nicht: die stehende Zeit macht K zu **klein**, nicht
   negativ — E wächst je gespaltenem Puffer um weniger als n_L, also bleibt
   K ≥ 0 und verschiebt s0 = K − v, den Referenzausschnitt und damit U_unten,
   R_LUFS, R_TP und alle Bänder aus M-67 und M-68 still um denselben Betrag.

Die Größenordnung ist nicht vernachlässigbar: FL zerteilt bei
automationsreichem Material auf ~180–190 Samples je Block
(`host-capabilities-fl-v1.json:57`), Blockgröße min 1, max 4 096 (`:54`).

**Antwort auf Teilfrage (d).** Einen Fortlaufmesser über das **integrierte
Material** gibt es an HEAD nicht. Es gibt (i) `stromVon`, der nur Samples
zählt, (ii) `quarantaene.kontinuitaetsbrueche()`, der den stehenden Stempel
bewusst nicht zählt, und (iii) `rahmenProjektDurchgehend` (`Zeit.h:568-587`) mit
genau der richtigen Konstruktion (Start + akkumulierte Samples == nächster
Stempel), aber nur über einen Merkmalsrahmen. Ein additiver Zähler in den
Workern ist deshalb nötig — und er braucht **keine neue Pfadfreigabe**: T-28
(HEAD-Zeile 954) hält selbst fest, dass die Aufzeichnung des Materialendes „im
Worker, in Pfaden, die Etappe 2 ohnehin ändert
(`eq-copilot/plugin/src/prozessor/Analyse.cpp`,
`eq-copilot/plugin/sonde/SondeProcessor.cpp`)" liegt, „nicht in der Engine".
P-8 (Freigabe von `AnalyseEngine.h/.cpp`) wird dafür nicht gebraucht.

### Einordnung (Vorschlag): DEFEKT

Verletzt ist nicht ein Rechenfehler, sondern der Maßstab, den der Dirigent in
§13.3 (g) selbst gesetzt und F-28 in seinen Kopf übernommen hat (HEAD-Zeile
414): „jede Zahl, die ein Band oder eine Vorbedingung trägt, nennt ihre
Materialzeit und deren Quelle; ist die Materialzeit nicht aus dem Umschlag
lesbar oder **nicht hergeleitet**, bleibt der Wert `roh`, P-5." K ist eine
solche Zahl (sie trägt s0, den Referenzausschnitt, U_unten und jedes Band in
M-67 und M-68), ihre Herleitung stützt sich auf `Zeit.h:206-229`, und diese
Stelle trägt sie nicht. Nach der Einordnungsregel des Auftrags ist das der Fall
„der Beweisweg trägt die Zusage nicht" — also DEFEKT, nicht LÜCKE: die Matrix
schweigt nicht, sie behauptet.

Mittelbar betroffen ist die CLAUDE.md-Invariante „Engine kennt keine Optik …
Sie liefert **kohärente** Mess-Snapshots" bzw. „Zeit ist aktive Musikzeit": ein
Anker, der Hostzeit und Materialzeit ohne Beleg gleichsetzt, macht den
Snapshot-Ausschnitt inkohärent. Der Verweis des Urteils auf `CLAUDE.md:186` ist
sachlich richtig, aber schwächer als die direkte Verletzung von P-5/§13.3 (g).

### Schließende Änderung (Vorschlag)

Nur Manifesttext, keine Codeänderung in dieser Runde.

1. **F-6 (HEAD-Zeile 359), Gruppe Materialzeit — ein neues Umschlagfeld.**
   `hostzeit_fortlaufend_samples`: die Summe der Startstempel-Differenzen
   aufeinanderfolgender, in die Engine gegebener Blöcke seit dem letzten
   Rücksetzen. Der Worker führt sie an derselben Stelle mit, an der er
   `material_ende_projektsample` und `bloecke_max_samples` festhält
   (`Analyse.cpp:275-281`, `SondeProcessor.cpp:602-604`): je Block
   `stempel(neu) − stempel(alt)` addieren, den ersten Block nach dem Rücksetzen
   als Startpunkt merken; null, sobald ein Block ohne `kFlagZeitGueltig` eingeht.
   Zusätzlich `hostzeit_stillstand_bloecke`: die Zahl der Blöcke, deren
   Stempeldifferenz kleiner war als die Länge des Vorgängerblocks (der
   FL-Teilblockfall), kumulativ und roh.
   **Fortlaufbedingung:** die Hostzeit gilt über das integrierte Material als
   fortlaufend, wenn `hostzeit_fortlaufend_samples` == n_L (verarbeitete
   Samples) und `hostzeit_stillstand_bloecke` == 0 ist. Beides sind rohe
   Zähler; die Bedingung rechnet der Runner aus dem Umschlag, nicht das Plugin.
2. **F-28 (HEAD-Zeile 416), Absatz „Materialzeit einer Antwort".** Den Satz
   „Das Material der Engine ist seit dem Rücksetzen lückenlos, weil jede Lücke
   und jeder Zeitsprung eine Grenze und damit ein Rücksetzen ist
   (`Zeit.h:206-229`)" ersetzen durch: „Lückenlos ist seit dem Rücksetzen der
   **lokale Strom** (`stromVon`, `StampedAudioQueue.h:160-163`; jede Lücke und
   jeder Zeitsprung ist eine Grenze, `Zeit.h:206-229`). Die **Hostzeit** ist es
   nicht von selbst: ein Block mit unverändertem `projectSampleStart` ist nach
   `StampedAudioQueue.h:810-823` und `Zeit.h:213` ausdrücklich keine Grenze —
   FL zerteilt Puffer bis auf 1 Sample (`host-capabilities-fl-v1.json:54`,
   `:57`), und ob die Teilstücke die Zeit fortschreiben, ist ungemessen
   (NAK-56, `docs/plugin-wissen.md:917-919`). **K = E − n_L gilt deshalb nur
   bei erfüllter Fortlaufbedingung aus F-6.** Ist sie nicht erfüllt, ist K
   nicht hergeleitet: der Anker bleibt `roh` (P-5), U_unten und jedes Band
   dieser Antwort werden nicht gelesen, die Zähler stehen roh im Protokoll.
   `K < 0` bleibt Szenario-Voraussetzung (Exit 5) und deckt nur den anderen
   Fall — Samples ohne gültige Hostzeit; die stehende Zeit macht K zu klein,
   nie negativ."
3. **M-67 (608) und M-68 (609), Urteilsweg.** Nach „ohne lesbaren Anker und bei
   K < v nur `roh`" ergänzen: „und bei nicht erfüllter Fortlaufbedingung (F-6)
   nur `roh`".
4. **M-81 (540) um die Lage erweitern** (statt einer eigenen Zeile, weil M-81
   bereits der Materialzeit-Anker ist): dritter Durchlauf mit einer
   **Teilblock-Attrappe** — ein Hostpuffer von 512 Samples als zwei Blöcke von
   je 256 mit demselben `projectSampleStart` H, danach ein Block bei H + 512.
   Zusage: `hostzeit_fortlaufend_samples` = 512 gegen n_L = 768,
   `hostzeit_stillstand_bloecke` = 1, Fortlaufbedingung nicht erfüllt, der Anker
   dieser Antwort ist `roh`.
   **Rotbeweis (3):** Fortlaufbedingung entfernt → die Teilblock-Attrappe
   liefert K = 768 − 512 = 256 statt des tatsächlichen Kopfes H und wird als
   Anker gewertet → rot.
   **Rotbeweis (4):** `hostzeit_fortlaufend_samples` aus `stromVon` statt aus
   den Stempeldifferenzen gebildet → die Bedingung ist im Teilblockfall
   fälschlich erfüllt → rot.
5. **§5.5 Punkt 5 und Punkt 3** um die Mitführung der beiden Zähler ergänzen
   (dieselbe Stelle wie `material_ende_projektsample`); **§7.1 T-28** um den
   Absatz ergänzen, dass der Anker eine gemessene Fortlaufbedingung braucht und
   ohne sie `roh` bleibt. Keine neue Pfadfreigabe: beide Worker-Pfade sind
   bereits Etappe-2-Pfade (§5.5 Ticketpfade, T-28).
6. **Register:** NAK-56 (Karte U13) in F-28 und T-28 als Quelle nennen — die
   Karte ist genau die offene Messung, an der die Bedingung hängt.

---

## Befund 2 — „Erhalte die Probeeq-Antwort ohne eintreffende Analyseblöcke"

**Wörtlich (Urteil), `docs/beweise/roh/NAK-286-matrixpruefung-3-34c7f63.txt:12-13`:**

> `[P2] Erhalte die Probeeq-Antwort ohne eintreffende Analyseblöcke —
> C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-286.md:358-358`
> DEFEKT gegen F-4/M-31: Erhält eine frisch erzeugte Probeeq ohne Messdaten
> eine gültige Anfrage und treffen keine Analyseblöcke ein, setzt dieser Ablauf
> das Flag, schreibt aber niemals die zugesagte Antwort mit
> `noch_keine_messdaten`. Der Worker verlässt bei leerer Queue den Blockzug
> (`eq-copilot/plugin/sonde/SondeProcessor.cpp:551-553`); die neue Bedingung
> „im Zug mindestens ein Block" wird nie erfüllt, das Flag bleibt gesetzt und
> F-13 hält den Briefkasten dauerhaft im Wartezustand. M-31 verlangt dagegen
> ausdrücklich eine geschriebene Antwort auch ohne Messdaten. Dafür braucht der
> Bauplan einen ausdrücklichen Ohne-Messdaten-Antwortzweig, ohne den
> Single-Writer-Vertrag aus P-9 aufzugeben.

### Quelle an HEAD

**(1) Der Worker bei leerer Queue — `eq-copilot/plugin/sonde/SondeProcessor.cpp:548-565`
und `:654-665`:**

```cpp
            constexpr int kMaxBloeckeJeZug = 8;
            for (int i = 0; i < kMaxBloeckeJeZug; ++i)
            {
                const auto* roh = analyseQueue.spitze();
                if (roh == nullptr)
                    break;
                …
            }
            queueHatRest = analyseQueue.spitze() != nullptr;
        }

        if (queueHatRest)
        {
            std::this_thread::yield();
            continue;
        }
        std::unique_lock<std::mutex> l (workerWarteSchloss);
        workerWarte.wait_for (l, std::chrono::milliseconds (20),
                              [this] { return ! workerLaeuft.load(); });
```

Präzisierung gegenüber dem Urteilstext: der Worker **stirbt nicht und schläft
nicht unbegrenzt**. `wait_for` hat eine Frist von 20 ms, das Prädikat prüft nur
das Laufflag; der Kontrolltakt läuft ohnehin alle 5 ms (`:521-525`). Der Worker
dreht also auch ohne Audio ~50-mal je Sekunde seine Schleife und würde ein Flag
sehen. Was nie eintritt, ist das **Ereignis**, an das F-5 und §5.5 Punkt 5 die
Auswertung binden: `break` bei `roh == nullptr` heißt, kein Block geht in die
Engine, also ist „im Zug mindestens ein Block" dauerhaft falsch. Die Zusage
fällt nicht am schlafenden Thread, sondern an der Bedingung — und das ist die
Stelle, die die Zusage trägt.

**(2) Was der Briefkasten dann tut.** F-13 (HEAD-Zeile 375): „Ein Takt, der auf
die Auswertung wartet (Probeeq, §13.2 P-9, F-5), **ist kein Schreibversuch**: er
zählt weder zu den drei Versuchen noch in den Fehlerzähler, liest die Anfrage
nicht erneut und legt nichts an; die Versuche beginnen mit dem ersten Takt, der
schreibt." Der Dreiversuchsdeckel, der jede andere Sackgasse beendet, greift
hier also gerade nicht: der Wartezustand ist unbegrenzt.

**(3) Was eine Auswertung ohne neue Samples täte —
`eq-copilot/plugin/src/AnalyseEngine.cpp:1048-1120`:**

```cpp
void AnalyseEngine::auswerten()
{
    if (sr <= 0.0)
        return;
    MessSnapshot s;
    …
    fuelleBasis (s);   // Zustand, Sekunden, Live-Kurve, TP/Crest/Kurz-LUFS
    …
    s.revision = ++revisionZaehler;
    {
        std::lock_guard<std::mutex> l (snapMutex);
        fertig = std::move (s);
    }
}
```

`auswerten()` hat **keinen** Riegel gegen fehlende neue Samples — nur gegen eine
ungültige Rate. Mit `samplesGesamt == 0` publiziert es einen gültigen Stand mit
`zustand = keineDaten` (`:750-752`: „`s.zustand = samplesGesamt == 0 ?
MessZustand::keineDaten : …`") und `gesamtSekunden = 0`. Das ist der
entscheidende Unterschied zu `auswertenLeicht()` (`:787-798`), das ohne neue
Samples ausdrücklich nichts publiziert („Ohne neue Samples keine Publikation:
die Revision steht"). Variante (ii) der Fragestellung ist damit an HEAD
tragfähig — aber nur über `auswerten()`, nicht über `auswertenLeicht()`.

Fällt `auswerten()` am Rate-Riegel (`sr <= 0.0`, im Worker: `rateGueltig` false,
`SondeProcessor.cpp:530-534`), bleibt der zuletzt publizierte Stand stehen; der
ist nach `zuruecksetzen()` (`AnalyseEngine.cpp:324-335`: `fertig =
MessSnapshot {}`) bzw. nach Konstruktion ohnehin `keineDaten`
(`eq-copilot/plugin/src/AnalyseEngine.h:69`, `:91`). Auch dieser Zweig liefert
also einen beantwortbaren Stand — sofern das Flag trotzdem gelöscht wird.

**(4) Der Single-Writer-Kontrakt —
`eq-copilot/plugin/src/AnalyseEngine.h:181-184`, `:207`:**

```cpp
    // SINGLE-WRITER-KONTRAKT: vorbereiten/zuruecksetzen/verarbeite/auswerten
    // dürfen nur von EINEM Thread (dem Worker) aufgerufen werden. …
    // Einzig snapshot() ist threadsicher.
    …
    MessSnapshot snapshot() const;   // beliebiger Thread
```

`snapshot()` ist ausdrücklich threadsicher; ein Briefkastentakt, der nur liest,
bricht den Kontrakt nicht.

**(5) Warum Gen nicht betroffen ist.** Gens Worker gattert `auswerten()`
genauso an neuem Material (`eq-copilot/plugin/src/prozessor/Analyse.cpp:296-304`:
`if (faellig.schwer) { if (unverarbeitet > 0) … engine.auswerten(); }`), aber
Gens Briefkasten wartet nicht auf den Worker: F-5 (HEAD-Zeile 358) lässt Gen das
Objekt „aus derselben Rechnung wie `schreibeSnapshotDatei`" bauen, ohne
Anfrage-Flag. Gen schreibt die Antwort im selben Takt, mit `keineDaten` aus dem
Konstruktionsstand — M-31 hält für Gen. Der Widerspruch entsteht allein aus der
in der Matrix-Nacharbeit 2 neu eingezogenen Probeeq-Wartestrecke.

### Gegenstand

**F-5, HEAD-Zeile 358** (Stelle, gekürzt):

> „der Worker prüft das Flag in `workerLauf` nach dem Blockzug (Schleife
> `SondeProcessor.cpp:549-653`), noch unter `analyseSchloss` (Sperrbereich
> `:528-655`), und ruft, **wenn es steht und im Zug mindestens ein Block in die
> Engine ging**, genau einmal `analyseEngine.auswerten()` und löscht das Flag …
> Der nächste Takt sieht das Flag gelöscht, kopiert `analyseEngine.snapshot()`,
> den Rahmen und die Materialzeit unter `analyseSchloss` (F-15) und schreibt die
> Antwort. … Ein wartender Takt ist kein Schreibversuch (F-13). Zeile M-80."

**§5.5 Punkt 5, HEAD-Zeile 746** (derselbe Wortlaut im Bauplan):

> „der Worker ruft nach dem Blockzug unter `analyseSchloss`
> (`SondeProcessor.cpp:528-655`) genau einmal `analyseEngine.auswerten()`, **wenn
> das Flag steht und im Zug ein Block in die Engine ging**, und löscht es"

**F-4, HEAD-Zeile 357:** „Ohne Messdaten wird die Antwort **trotzdem
geschrieben** (M-31)."
**M-31, HEAD-Zeile 536:** Zustand „Prozessor ohne Messdaten
(`MessZustand::keineDaten`, 0 gebaute Rahmen)", Ereignis „gültige Anfrage",
Zusage „die Antwort wird geschrieben; `snapshot` null und `frame` null,
`gruende` enthält `noch_keine_messdaten`"; Rotbeweis „Abbruch wie im Knopfweg …
→ 0 Dateien → rot". Die Zeile sagt „Prozessor", nicht „Gen" — sie gilt für
beide Rollen; ihr Testfall heißt `ohne_messdaten_antwort_mit_grund` (§5.5,
HEAD-Zeile 758).
**R-286-1, HEAD-Zeile 350:** „Jede Instanz (Gen, Probeeq, Broker) beantwortet
jede Anfragekennung **genau einmal**".
**M-80, HEAD-Zeile 539:** Zustand mit „Worker läuft, deterministischer
Sampleplan wie M-32" — Blöcke treffen dort ein; die Zeile deckt die Lage ohne
Blöcke nicht ab.
**M-56, HEAD-Zeile 587:** „fehlt eine erwartete Antwort nach 10 s → Schritt
verfehlt mit der Liste der fehlenden Rollen; **Urteilsweg:** VERFEHLT bei
fehlender oder mehrdeutiger Antwort (Szenario-Exit 4)".

### Status: BESTÄTIGT

Antwort auf Teilfrage (a): Der Worker bricht den Blockzug bei leerer Queue mit
`break` ab (`:551-553`) und wartet danach 20 ms auf die Weckbedingung
(`:662-664`) — er läuft weiter, aber die an den Blockzug gebundene Bedingung
tritt nie ein. Antwort auf (b): F-5 (358) und §5.5 Punkt 5 (746) tragen den
Zusatz „und im Zug mindestens ein Block in die Engine ging" wörtlich; F-13 (375)
nimmt den wartenden Takt ausdrücklich aus dem Dreiversuchsdeckel; M-80 (539)
prüft nur die Lage mit laufendem Sampleplan. Antwort auf (c): **Ja, der
Widerspruch ist real.** F-4 (357) und M-31 (536) sagen „Prozessor ohne
Messdaten" ohne Rolleneinschränkung und verlangen die geschriebene Antwort;
R-286-1 (350) verlangt „genau einmal" von **jeder** Instanz. Eine Probeeq mit
gestopptem Transport beantwortet nie — die Zusage wird nicht spät erfüllt,
sondern gar nicht.

Zwei Verschärfungen, die das Urteil nicht nennt:

- **Die Lage ist nicht exotisch, sie ist der Normalfall am Laufanfang.** Der
  Runner startet FL frisch (M-59, `frischer_start`), setzt Position und
  Szenarien und fragt den Briefkasten, bevor die Wiedergabe läuft; genau dafür
  gibt es `lokal.briefkasten` mit `frist_s` (F-25, HEAD-Zeile 408) getrennt von
  `lokal.umlauf`. Probeeq auf Insert 1 sieht bei gestopptem Transport kein
  Audio.
- **Der Fehler wird als Produktfehler sichtbar, nicht als Stillstand.** Nach
  M-56 (587) verfehlt der Schritt nach 10 s mit der Liste der fehlenden Rollen
  → Szenario-Exit 4 → Laufurteil VERFEHLT. Der Laufzeit-Arm meldete damit ein
  rotes Ticket für ein regelkonform gebautes Plugin.

Der einzige Punkt, an dem das Urteil ungenau ist, ist der Mechanismus
(„verlässt den Blockzug" liest sich wie „hört auf zu laufen"); Ort, Bedingung
und Folge stimmen.

### Einordnung (Vorschlag): DEFEKT

Verletzt sind drei Zusagen an HEAD: R-286-1 („beantwortet jede Anfragekennung
genau einmal"), F-4 („Ohne Messdaten wird die Antwort trotzdem geschrieben")
und M-31 (Zusage und Rotbeweis „0 Dateien → rot"). Es ist keine LÜCKE: die
Matrix schweigt nicht, sie sagt in M-31 das Gegenteil dessen, was F-5 und §5.5
Punkt 5 bauen lassen. Es ist auch keine HÄRTUNG: ohne den Fix bleibt eine
zugesagte Antwort aus.

Die Ursache gehört in die Familie §13.3 (h) („Fix aus einer Runde hat das
Modell geändert, eine ältere Zusage blieb stehen"): P-9 hat den Probeeq-Weg von
„immer publiziert" auf „nur auf Anfrage" umgestellt, und die generische
M-31-Zusage wurde nicht gegen den neuen Weg geprüft.

### Schließende Änderung (Vorschlag)

**Bewertung der drei angebotenen Varianten.**

- **(i) Briefkastentakt antwortet nach n Takten selbst.** Technisch zulässig —
  der Takt liest nur `analyseEngine.snapshot()`, und das ist ausdrücklich
  threadsicher (`AnalyseEngine.h:207`), der Single-Writer-Kontrakt bleibt
  unberührt. Aber: die Antwort hinge an einer Frist, die mit der Worker-Kadenz
  rennt — trifft ein erster Block zwischen Takt n−1 und n ein, entstünde je nach
  Timing einmal `noch_keine_messdaten` und einmal ein voller Snapshot, für
  dieselbe Lage. Das ist ein nichtdeterministischer Rotbeweis und widerspricht
  dem Maßstab, dass jede Zahl ihre Materialzeit kennt. Außerdem bliebe der
  Wartezustand als Zustand bestehen, nur gedeckelt.
- **(ii) Worker wertet auch ohne neuen Block aus.** Trägt an HEAD:
  `auswerten()` publiziert mit `samplesGesamt == 0` einen `keineDaten`-Stand
  (`AnalyseEngine.cpp:1048-1120`, `:750-752`), läuft im Worker unter
  `analyseSchloss`, nie auf dem Audio-Thread, und bleibt Single-Writer. P-9
  („Ohne Anfrage wertet Probeeq nie aus", Zähler 0 über 600 Takte) bleibt
  wörtlich erfüllt, weil die Auswertung weiterhin nur bei stehendem Flag
  geschieht. Es entfällt der Wartezustand ganz statt ihn zu deckeln.
- **(iii) Kombination.** Nicht nötig und schädlich: sie erbt die
  Nichtdeterminismus-Kante von (i), ohne etwas hinzuzufügen, das (ii) nicht
  schon leistet.

**Empfehlung: Variante (ii).** Konkret:

1. **F-5 (HEAD-Zeile 358):** „wenn es steht **und im Zug mindestens ein Block in
   die Engine ging**" ersetzen durch: „wenn es steht — **unabhängig davon, ob im
   Zug ein Block in die Engine ging**". Anschließend den tragenden Grund
   ergänzen: „Ein leerer Zug ist der Normalfall bei gestopptem Transport; der
   Worker dreht seine Schleife trotzdem mindestens alle 20 ms
   (`SondeProcessor.cpp:662-664`). `auswerten()` publiziert auch ohne neue
   Samples: mit `samplesGesamt == 0` trägt der Stand
   `MessZustand::keineDaten` und `gesamt_sekunden` 0
   (`eq-copilot/plugin/src/AnalyseEngine.cpp:750-752`, `:1048-1120`) — anders
   als `auswertenLeicht()`, das ohne neue Samples nichts publiziert (`:787-798`)
   und das Probeeq nie ruft. **Der Worker löscht das Flag in jedem Fall**, auch
   wenn `auswerten()` am Rate-Riegel `sr <= 0.0` (`:1050`) zurückkehrt; der
   zuletzt publizierte Stand ist dann der aus `zuruecksetzen()` (`:324-335`),
   also ebenfalls `keineDaten`. Der Briefkasten antwortet damit in jeder Lage
   im übernächsten Takt; einen unbegrenzten Wartezustand gibt es nicht."
2. **§5.5 Punkt 5 (HEAD-Zeile 746):** denselben Zusatz streichen und durch
   „wenn das Flag steht, auch bei leerem Blockzug; das Flag wird in jedem Fall
   gelöscht" ersetzen.
3. **F-13 (HEAD-Zeile 375):** den Satz über den wartenden Takt behalten, aber
   begrenzen: „Ein wartender Takt ist kein Schreibversuch … **Der Wartezustand
   dauert höchstens bis zum übernächsten Takt, weil der Worker das Flag in
   jedem Durchlauf bedient und in jedem Fall löscht (F-5); ein Takt, der ein
   drittes Mal wartet, ist ein Befund.**"
4. **M-80 (HEAD-Zeile 539) um eine Lage (c) erweitern:** „Worker läuft, **keine
   Blöcke in der Queue** (Transport gestoppt), `analyseEngine` ohne Messdaten;
   danach eine gültige Anfrage K2." Zusage: der Takt, der K2 liest, setzt das
   Flag und schreibt nicht; der Worker ruft im nächsten Durchlauf genau einmal
   `auswerten()` (Zähler +1) und löscht das Flag; der nächste Takt schreibt
   genau eine Antwort mit `snapshot` null, `frame` null und `gruende` mit
   `noch_keine_messdaten` (F-4); höchstens ein wartender Takt, Fehlerzähler 0,
   Anlegeversuche 1.
   **Rotbeweis (3):** Bedingung „im Zug mindestens ein Block" wieder eingesetzt
   → in Lage (c) entsteht keine Antwort, der Takt wartet unbegrenzt → rot.
   **Rotbeweis (4):** Flag nur bei erfolgreicher Publikation gelöscht → mit
   ungültiger Rate (`sr <= 0.0`) entsteht keine Antwort → rot.
5. **M-31 (HEAD-Zeile 536):** Zustand um „Gen **und** Probeeq" ergänzen und den
   Probeeq-Weg auf M-80 Lage (c) verweisen — damit steht die Rollenabdeckung in
   der Zeile selbst und nicht nur im generischen Wort „Prozessor". Testfall
   `ohne_messdaten_antwort_mit_grund` (§5.5, HEAD-Zeile 758) über beide Rollen
   fahren.
6. **P-9 (HEAD-Zeile 1281):** den Satz „der Worker ruft nach seinem nächsten
   Verarbeiten genau einmal `auswerten()`" auf „in seinem nächsten Durchlauf"
   ändern — „Verarbeiten" ist genau die Formulierung, aus der der Zusatz in F-5
   entstanden ist. „Ohne Anfrage wertet Probeeq nie aus" bleibt unverändert.

Keine neue Pfadfreigabe nötig: `SondeProcessor.h/.cpp` sind Etappe-2-Pfade
(§5.5 Ticketpfade, §7.3 Punkt 1), und an `AnalyseEngine` ändert sich nichts.

---

## Schlusstabelle

| Befund | Status | Einordnung (Vorschlag) | Schließende Änderung (Kurz) |
|---|---|---|---|
| 1 — Fortlaufende Hostzeit vor K (F-28 Zeile 416, M-67 608, M-68 609, M-81 540) | BESTÄTIGT, in zwei Punkten präzisiert: (a) belegt ist nur, dass FL Puffer bis auf 1 Sample zerteilt (`host-capabilities-fl-v1.json:54`, `:57`) — ob die Teilstücke die Zeit fortschreiben, ist ausdrücklich **ungemessen** (NAK-56, `docs/plugin-wissen.md:917-919`); der Code deckt beide Fälle bewusst ab (`StampedAudioQueue.h:810-833`, `Zeit.h:213`), K = E − n_L setzt nur einen davon voraus; (b) der Riegel `K < 0` greift nicht, weil stehende Zeit K zu **klein** macht, nie negativ | **DEFEKT** — der Beweisweg trägt die Zusage nicht: F-28 begründet die Lückenlosigkeit mit `Zeit.h:206-229`, dort ist aber nur der **lokale Strom** lückenlos, nicht die Hostzeit; damit ist K nicht hergeleitet, und §13.3 (g)/P-5 verlangen dann `roh`. Mittelbar CLAUDE.md „kohärente Mess-Snapshots" | Neue Umschlagfelder in F-6: `hostzeit_fortlaufend_samples` (Summe der Stempeldifferenzen seit Rücksetzen) und `hostzeit_stillstand_bloecke`; Fortlaufbedingung = Summe == n_L und 0 Stillstandsblöcke, gerechnet vom Runner. F-28: Lückenlosigkeitssatz trennen (Strom vs. Hostzeit), K nur bei erfüllter Bedingung gültig, sonst Anker `roh` nach P-5. M-67/M-68 Urteilsweg um „nicht erfüllte Fortlaufbedingung → `roh`" ergänzen. M-81 dritter Durchlauf mit Teilblock-Attrappe (zwei 256er mit gleichem Stempel) plus zwei Rotbeweise. Zähler im Worker, keine neue Pfadfreigabe (T-28: Aufzeichnung liegt ohnehin im Worker) |
| 2 — Probeeq-Antwort ohne eintreffende Analyseblöcke (F-5 Zeile 358, §5.5 Punkt 5 Zeile 746, M-31 536, M-80 539, F-13 375) | BESTÄTIGT, Mechanismus präzisiert und verschärft: der Worker schläft **nicht** — er wartet nur 20 ms (`SondeProcessor.cpp:662-664`) und läuft weiter; was nie eintritt, ist die Bedingung „im Zug mindestens ein Block" (`break` bei leerer Queue, `:551-553`). Verschärfung: die Lage ist der Normalfall bei gestopptem Transport, und nach M-56 (10-s-Frist) endet der Lauf als VERFEHLT — ein falsches Rot für ein regelkonformes Plugin | **DEFEKT** — verletzt R-286-1 („beantwortet jede Anfragekennung genau einmal"), F-4 („ohne Messdaten trotzdem geschrieben") und M-31 (Zusage \| Rotbeweis „0 Dateien → rot"). Keine LÜCKE: M-31 sagt das Gegenteil dessen, was F-5 bauen lässt. Ursachenfamilie §13.3 (h) | Variante **(ii)**: die Bedingung „und im Zug mindestens ein Block" in F-5 (358) und §5.5 Punkt 5 (746) streichen — der Worker bedient das Flag in **jedem** Durchlauf und löscht es in jedem Fall, auch wenn `auswerten()` am Rate-Riegel `sr <= 0.0` zurückkehrt. Trägt an HEAD, weil `auswerten()` mit `samplesGesamt == 0` einen `keineDaten`-Stand publiziert (`AnalyseEngine.cpp:750-752`, `:1048-1120`), anders als `auswertenLeicht()` (`:787-798`). Single-Writer und P-9 („ohne Anfrage nie") bleiben unberührt. Dazu M-80 Lage (c) (keine Blöcke) mit zwei Rotbeweisen, M-31 Zustand auf beide Rollen, F-13 Wartezustand auf höchstens einen Takt begrenzen, P-9 „nächstes Verarbeiten" → „nächster Durchlauf" |
| Gemeinsame Ursache | BESTÄTIGT für Befund 2: §13.3 (h) — P-9 hat den Probeeq-Publikationsweg umgestellt, die generische M-31-Zusage wurde nicht gegen das neue Modell geprüft. Für Befund 1: Fortsetzung von §13.3 (g) auf der **Host**zeitachse — die vier bisherigen Glieder (Rasterphase, Kopf, Versatz v, Publikationslatenz) bekommen ein fünftes: die Fortlaufbarkeit der Hostzeit selbst | Maßstab für die Nacharbeit, kein eigener Befund | (g) erweitern: jede Zahl, die Hostzeit und Materialzeit verrechnet, nennt zusätzlich die **gemessene** Bedingung, unter der beide Achsen dieselbe Länge haben; fehlt sie, bleibt der Wert `roh`. (h) über alle Zeilen ziehen, die eine Publikation an ein eintreffendes Ereignis binden (M-31, M-32, M-80, M-81) |
