# NEXT-SESSION — Einstieg für die nächste Runde

> ## ⚠ Zuerst lesen — Stand 22.08.2026 abends
>
> **P0 ist geschlossen.** Gate G0 ist gefahren, Urteil **PASS**
> (`docs/beweise/G0.md`): beide Bruchauftraege (Gate 1, Gate 5) gescheitert,
> die P0-Kernflaeche traegt keinen Befund. Damit faellt der
> Schliessungsvorbehalt §65 fuer `SONDE-005`.
>
> **S8 (`SONDE-007a`) ist gebaut** (22.08. spaet, Manifest
> `docs/beweise/SONDE-007a.md`, Commits `5d0e9fd` + `06913aa`): `NakamaKern`
> ist eine echte Static-Lib, einmal uebersetzt statt je Ziel, ohne eine einzige
> `JucePlugin_*`-Konstante. Kanon 18 → **19** Beine (A14).
>
> **T2 ist gefahren** (23.08., Commit `26b94c5`, Manifest §5/§6): Urteil
> **NEEDS_WORK**. Die zentrale Behauptung hielt unter eigener Messung (0 von 74
> `JucePlugin_`-Defines im Kern, Fassade traegt, alle acht nachgezaehlten
> Zahlen stimmten), aber fuenf Befunde — allen voran eine **echte Regression
> des Aenderungssatzes**: als eigene Lib erbt der Kern die PUBLIC-Schalter
> seiner Verbraucher nicht mehr und uebersetzte als einziger Code im Baum unter
> `/W1` statt `/W4`. Alle fuenf am selben Tag geschlossen, fuenfter Riegel
> **K2c** gebaut und beim Fallen vorgefuehrt, Kanon danach 19/19, `pluginval` 8
> erneut SUCCESS. **Kein PASS auf den nachgebesserten Stand** — S8 steht damit
> wie S5 und S6 auf „gebaut", nicht „abgenommen".
> 🔑 Lehre fuer S9: **eine Static-Lib erbt die PUBLIC-Schalter ihrer
> Verbraucher nicht** — wer in S9 eine zweite Lib anlegt, haengt ihr die
> Empfehlungsschalter selbst an; K2c faellt sonst zur Configure-Zeit.
> **S9 (`SONDE-007b`): Abschnitte 1+2 von 3 gebaut** (23.08., Commits
> `8e32baf` + `fdece75`, Manifest `docs/beweise/SONDE-007b.md`).
> **Es gibt jetzt drei Bundles:** `EQ-Copilot.vst3` (`Eqcp`),
> **`Nakama Suna.vst3`** (`NkPr`) und **`Nakama Probeeq.vst3`** (`NkAc`).
> Die Identitaet kommt aus `identity/plugin-identities-v1.json` — kein Literal
> mehr im Bauskript (**NAK-52 geschlossen**). Die beiden neuen entstehen aus
> EINER Quelle `plugin/sonde/` ueber duenne Target-Schichten; ihre CIDs sind
> erstmals an Artefakten gemessen, kein Bundle traegt eine fremde Ziel-CID.
> Auch entschieden und gebaut: K2b/K2c messen gegen **jeden** Kern-Verbraucher
> (12), nicht gegen eine Stichprobe. Kanon 19 → **21** (A15/A16), `pluginval` 8
> an beiden neuen SUCCESS.
>
> ⚠️ **Beide neuen Bundles sind heute Passthrough** — keine Hostparameter,
> `hasEditor() == false`. Probeeqs EQ-DSP gehoert zu P6, die Oberflaechen
> kommen aus Figma. Wenn P6 die DSP bringt, **muss** das Kanon-Bein A16
> umgeschrieben werden.
> 🔑 Zwei Fehler fand der Gegenpfad speichern↔laden, nicht das Nachdenken:
> `active_probe` verlangt laut Kind-Matrix §2.1 genau ein `Parameters`-Kind,
> und `Zustand::parameters` war `{}` — also **Nullen statt `standardSatz()`**,
> und 0 Hz liegt ausserhalb von `band.0.freq_hz`.
>
> **S9 Abschnitt 3 ist gebaut** (23.08., `42bfe6e` + `ff0e0b8`). Damit stehen
> alle drei Bauabschnitte von `SONDE-007b`:
> - `state::Lebenslauf` im gemeinsamen Kern ist §53.5 als Code. Neu ist nicht
>   der Automat, sondern dass er **Zaehne** hat: die Hoer-Markierung faellt
>   unter „audio-neutral", und `EqCopLebenslaufTest` misst das an AUDIO —
>   dieselbe Markierung, die `EqCopMarkierungTest` faerben laesst, faerbt vor
>   der Klassifikation **kein einziges Sample**. `darfBrokerStarten()` ist der
>   eine Haken, an den SONDE-010 den Spawn haengt.
> - Installer-Manifest: Vertrag (`schemas/installer/`), Auslieferung
>   (`install/nakama-installer-v1.json`) und `Install-Nakama.ps1` mit beiden
>   Haelften (installieren ↔ Rueckweg) samt NAK-41-Riegel. **NAK-32 ist damit
>   geschlossen**; NAK-41 bleibt als Sachlage bestehen, ist aber kein Blocker
>   des Installer-Schritts mehr. Der committete Stand traegt bewusst
>   `sha256: null` = nicht ausliefer-bar.
> - Kanon 21 → **23** (A17 Installer-Manifest, B8 Lebenslauf), 23/23 gruen.
>
> ⚠️ **Verhaltensaenderung, die ein Pruefer zuerst ansehen sollte:** eine
> `legacy`-Instanz faerbt nicht mehr. §53.5 nennt `legacy` „immer passiv" —
> gewollt, aber es ist der einzige Punkt, an dem dieser Aenderungssatz das
> Verhalten des Produkts aendert.
> ⚠️ **`pluginval` lief NICHT** am Main-Bundle: das Binary ist auf dieser
> Maschine nicht auffindbar und nirgends im Repo gepinnt (**NAK-53**). Gerade
> dort beruehrt der Aenderungssatz den Audiopfad — T1 muss es nachholen.
>
> **Naechster Schritt: T2 fuer S9** — Frischkontext-Pruefer, eigene Session
> (Sessionregel `docs/bauaufteilung-sonden.md` §0). Danach S10.
>
> **Aber vorher zwei Dinge, die Vorrang haben:**
>
> 1. ~~35 unbearbeitete Antworten~~ — **eingearbeitet am 22.08. spaet**
>    (`hub_sync.py` meldet jetzt „Antworten: 35 (0 neu)"). Jede traegt Wortlaut
>    und Ergebnis in `design/abnahmen/2026-08-22-hub-antworten-35.md`; fuenf
>    Entscheide stehen im Register in `CLAUDE.md` (Rangfolge Implementplan vor
>    Figma · Wortmarken aus dem Figma-Export · Hoer-Markierung nur mit Signal ·
>    Quellen×Band-Gitter als Gen-Uebersicht · Entscheidungstor Gen offen).
>    **Was daraus offen ist: zwoelf Folgefragen** (Abnahme, Abschnitt 6) —
>    ueberall dort, wo der User den Entwurf verworfen hat, der Alternativtext
>    aber zwei Formen anbot, und dort, wo eine Frage schlecht gestellt war
>    (`U6.8`, `U9.3`, `U9.4`: „macht kein sinn die frage", „verstehe frage
>    nicht"). **Nichts davon wurde geraten.**
>    ⚠️ Sie sind **nicht auf der Seite**: `briefing-app.tsx` zieht die
>    Kartentexte aus `briefing-hub/data/friendly-copy.ts` und filtert mit
>    `.filter((item) => item.title)` jede Karte still weg, die dort fehlt —
>    `hub.json` allein reicht nicht, es braucht Katalog-Eintrag **und** Deploy.
>    Vorher aber Punkt 2 klaeren: wandert der Hub ins Nimbalyst-GUI, waere die
>    Arbeit an `friendly-copy.ts` umsonst.
> 2. **NAK-50: Der User will den Hub im Nimbalyst-GUI statt auf der Seite**
>    („EIGENTLICH ist die seite unnötig noise, es wäre mir lieber alles hier im
>    nimbalyst GUI zu haben", 22.08. abends). Noch **kein** Register-Entscheid —
>    Wortlaut ist Praeferenz, nicht Abschaltbefehl, und beruehrt den Entscheid
>    desselben Tages. Vor dem Umbau klaeren; die asynchrone Antwortfunktion
>    (Punkt 1) darf dabei nicht verlorengehen. Details in
>    `docs/offene-punkte.md`.
>
> ## 👤 Was beim User liegt (nur er kann es)
>
> **Der OpenWiki-Schluessel.** Die taegliche Auffrischung
> (`.github/workflows/openwiki-update.yml`, 08:00 UTC) ist seit `ba6d6c8`
> scharf, bricht aber still ab: `OPENAI_API_KEY` ist als Repo-Geheimnis nicht
> gesetzt (gemessen 22.08.: `gh secret list` leer, kein Lauf). Ein Befehl im
> Projektordner, fragt den Wert interaktiv ab:
> `gh secret set OPENAI_API_KEY`. Details NAK-51.
>
> **Die Planblatt-Automation einschalten** (optional):
> `nimbalyst-local/automations/planstand-nakama.md`, taeglich 09:00 — angelegt,
> aber nicht aktiviert; das macht der User selbst ueber die Kopfleiste.
>
> **Klarstellungen vom 22.08. abends, damit sie niemand wieder falsch liest:**
> der Fremdmodell-Riegel bindet **nur Gemini** („gpt IST WAS anderes. das galt
> nur für gemini. gpt vertraue ich."); **OpenWiki ist das Kontextsystem, auf
> dem der Workspace aufbaut** („das workspace hier baut auf openwiki als
> kontextsystem auf") — nicht gegen `docs/` abwaegen, nicht als Zusatzquelle
> behandeln, nicht abschalten vorschlagen.
>
> **Neu seit 22.08.:** `docs/PLAN-STAND.md` zeigt den Planstand direkt in
> Nimbalyst (erzeugt aus `hub.json` mit `tools/hub/plan_blatt.py`, Diagramm
> rendert live). Stand jetzt: **12 von 34 Schritten**. Eine Nimbalyst-Automation
> (`nimbalyst-local/automations/planstand-nakama.md`, taeglich 09:00) frischt es
> auf — **noch nicht aktiviert**, der User schaltet sie selbst ein.
>
> **Unverbuchte Fremd-Aenderungen im Baum** (der User 22.08.: „alle anderen
> sitzungen sind lange schon fertig und ruhen" — sie sind also verwaist, nicht
> in Arbeit): vier Dateien unter `briefing-hub/` und zwei **Loeschungen** unter
> `eq-copilot/design/` (`ASSET-KIT.md`, `eq-copilot-material-preview.png`).
> Nicht blind committen — erst pruefen, ob die Loeschungen gewollt waren.


> Stand: **22.08.2026, nach S7 (`SONDE-006`).** Davor: 21.08., Kontext-Inventur und Interview mit dem User.
> Die Wahrheit steht in `CLAUDE.md` (Wahrheitskern + Register der User-Entscheide
> mit Zitat). Dieses Blatt sagt nur, wo wir stehen und was als Nächstes kommt —
> es wiederholt die Wahrheit nicht. Wenn hier und in CLAUDE.md etwas
> Verschiedenes steht, gilt CLAUDE.md; dann dieses Blatt berichtigen.

> **Seit 22.08. gibt es den Nakama-Hub**: die Seite
> <https://nakama-briefing.philipld.chatgpt.site> (Quelle `docs/hub/hub.json`,
> Werkzeug `tools/hub/hub_sync.py holen|senden`; Quellcode `briefing-hub/`) —
> das Briefing für den User und jede Session: Plan erledigt/offen, was bei ihm
> liegt, Figma-Stand der drei Apps, seine Antworten je Punkt. Pflicht: holen ·
> einarbeiten · senden (CLAUDE.md, Abschnitt „Hub"). Das Claude-Artefakt ist
> stillgelegt (User 22.08.: „alle anderen artefakte sind hiermit nichtmehr zu beachten"). Dieses
> Blatt bleibt der technische Einstieg; der Hub ist die Sicht des Projektleiters.

## Was heute passiert ist

- Neun Prüfer haben beide Repos, drei Memory-Verzeichnisse, Hooks, Docs,
  Design-Artefakte, Code und Git-Historie inventarisiert (Kontextkarte: 20
  Widersprüche, 15 Design-Richtungen in 10 Tagen, fünf parallel lebend, ein
  Produktentscheid in fünf Kopien ohne Zitat). Der User hat 20 Fragen beantwortet.
- Daraus: neue `CLAUDE.md`, Hooks lesen den Wahrheitskern aus CLAUDE.md (keine
  zweite Kopie), alte Design-Richtungen im Archiv, Prisma als Studie geparkt,
  Recherche im Archiv, Memories bereinigt, Nakama-Design auf „Figma ist Quelle".
- Zement-Brüche repariert: `eq-snapshot.schema.json` war seit 15.08. kein JSON
  (neues Kanon-Bein A11); CMake-Version 0.1.0 → 0.3.0 mit Configure-Riegel.

## Wo der Bau steht

- **P0 gebaut:** Beweis-Runner (S0), Aux-/PDC-Messgerät (S1), Identität (S2),
  Hostbrücke + Hostkontext-Test (S3), Termin-B-Messgerät (S3b), v3-Vertrag JSON
  (S5) und FlatBuffers (S6). **P1 begonnen:** State-Schema 2 (S7, 22.08.).
  Manifeste in `docs/beweise/`.
- **Offen an S5/S6:** nur noch der Schließungsvorbehalt §65 (erst nach S4).
  T2 lief in vier Runden; **Runde 4 ist am 21.08. geschlossen** — der
  Selbstbezug (`uoffset` 0) ist nicht mehr an einem Fixture, sondern an der
  **Klasse** belegt (6215 Byte-Mutanten, 143 → 0 Abweichungen), die stale Zahl
  in P10 ist nachgemessen (207) und der tote Teilausdruck in `utf8Gueltig` ist
  raus. Alles in `docs/beweise/SONDE-005b.md` §6.5 + P14/P15.
  **Offen bleibt das Urteil selbst:** eine T2-Runde 5 mit frischem Kontext gegen
  `git diff 4cf05b9..HEAD` ist nicht gelaufen — Befunde schliesst, wer nacharbeitet,
  urteilen darf nur ein Pruefer, der den Umbau nicht geschrieben hat.
- **Termin A ist gemessen (22.08. 00:17 + 00:27): geht, recall-stabil** — zwei
  getrennte Nebenwege samplegenau (Versatz 0), Beweis `docs/beweise/termin-a/`.
  **Termin B ist gemessen (22.08. 12:45–13:27) und S4 gebaut** — Capabilityreport
  `docs/beweise/SONDE-004.md`, Bits in `eq-copilot/identity/host-capabilities-fl-v1.json`
  (Kanon A13). **Nur Gate G0 (T3) steht in P0 noch aus.** Beide Messgeräte sind
  in `C:/Program Files/Common Files/VST3/` installiert (FL scannt VST3 nur dort).
- **Kanon:** 19 Beine in `tools/beweise.ps1` (seit 22.08.: B2 `EqCopStateMigrationTest`,
  A12 `erzeuge_state_fixtures.py --pruefen`, A13 `pruefe_host_capabilities.py`,
  A14 `pruefe_kern_identitaetsfrei.py`);
  letzter vollständiger Lauf siehe jüngstes Manifest in `docs/beweise/` (`SONDE-007a.md`: 19/19).

## ▶ Erledigt am 22.08. (Termin B + S4)

1. ~~Termin B über den FL-MCP fahren~~ — **gemessen 12:45–13:27**: der User legte
   `EqCop-Host-Probe` auf den Piano-Kanal, zeichnete den Automationsclip,
   schaltete Smart disable ein, exportierte und sicherte den Bericht; Claude
   fuhr Live, Seeks (ohne/mit Stop), Pattern-Schleife, Stummschaltung über den
   MCP. **Lehre:** `fl_set_song_position` — Modus 0 = ms, 1 = s, 2 = absolute
   Ticks (die Werkzeugbeschreibung sagt 1 = ms, 2 = s — falsch; im Fork
   `fl-studio-mcp` berichtigen). `fl_save_project` fehlt dem Fork weiterhin —
   Speichern blieb Strg+S beim User.
2. ~~Klicklisten A und B nachziehen~~ — beide tragen einen Stand-Hinweis mit den
   Berichtigungen (VST3 nur unter `Common Files\VST3`; Nebenwege als
   „Sidechain to this track"; Song ≥ 16 Takte; Arbeitsteilung).
3. **Angebot weiter offen beim User:** `C:/Program Files/Common Files/VST3/eq-copilot/`
   (845 MB alte Repo-Kopie vom 13.08. mit zwei alten `EQ-Copilot.vst3`, die FL
   mitscannt) wegräumen — braucht einen Admin-Klick.
4. ~~S4~~ — **gebaut**: `docs/beweise/SONDE-004.md`,
   `eq-copilot/identity/host-capabilities-fl-v1.json` (**2 supported, 8
   unsupported** — T2 hat `presentation_latency`, `aux_priority_sidechain` und
   `aux_compare_pre` herabgestuft: Golden nicht erbracht; Runde 2 PASS), Kanon A13.
   **Offen: Gate G0** (T3, eigene Session: `/c-review` auf den Bridge-Patch +
   Codex, Bruchaufträge Gate 1 und Gate 5) — danach fällt der
   Schließungsvorbehalt §65 für `SONDE-005`. **Termin A2** (NAK-44: PDC-Last,
   Bus-Identität, L/R) hebt beide Aux-Bits, wenn der User ihn fährt.

## ▶ So startet der Projektleiter eine Bau-Session

Neue Session in diesem Workspace aufmachen, diese zwei Zeilen einfügen, fertig.
**Stand 23.08. abends ist das die T2-Prüfung von S9**, nicht der nächste Bau:

```
Pruefe S9 (SONDE-007b) als T2 mit frischem Kontext gegen git diff 657fe57..HEAD.
Urteil PASS/NEEDS_WORK in docs/beweise/SONDE-007b.md, Abschnitt 5.
```

Die drei Stellen, an denen ein Prüfer bei **S9 Abschnitt 3** zuerst graben
sollte (aus dem Selbstaudit, ehrlich benannt statt versteckt):
1. **Die Verhaltensänderung an der Hör-Markierung.** `legacy` färbt nicht mehr.
   Ist der Term an der richtigen Stelle (vor `markierung.verarbeite`, nach dem
   Analyse-Abgriff), und kann die Atomic-Spiegelung `istMainKlassifiziert`
   veralten? Sie wird nur unter `bindungMutex` geschrieben — reicht
   `memory_order_relaxed` für einen Audiothread, der sie liest?
2. **Die explizite Initialisierung hängt an `setzeBindung`.** Das ist heute der
   einzige User-Akt, der die Klasse setzt. Gibt es einen zweiten Weg, auf dem
   `zustand.common.klasse` `main` wird, ohne dass der Automat es erfährt?
   (`neueSensorId`, `setStateInformation`, Editor-Pfade.)
3. **A17s Ableitung.** Der Quellpfad wird aus `cmake_ziel` + Bundlename
   gebildet. Stimmt die Formel noch, wenn ein Ziel andere JUCE-Formate baut,
   und was passiert bei einem Ziel ohne `cmake_ziel`-Feld?

Das ist keine Wahl, sondern die Sessionregel (`docs/bauaufteilung-sonden.md`
§0: „1 Session = 1 Ticket + sein Beweismanifest + **sein Frischkontext-Prüfer**").
S8 hat Manifest und T1, aber kein Prüferurteil — es ist damit **offen**. Ein
T2-Prüfer darf den Umbau nicht selbst geschrieben haben; den frischen Kontext
liefert genau das Aufmachen einer neuen Session, und diese Gelegenheit ist
verbraucht, sobald dieselbe Session anfängt zu bauen. Der Rückstand wächst
sonst weiter: S5, S6 und S8 tragen alle „T2 offen".

Danach S9, mit denselben zwei Zeilen in der Bauform:

```
Baue S9 (SONDE-007b) nach docs/bauaufteilung-sonden.md.
Manifest nach docs/beweise/SONDE-007b.md, T1 + T2.
```

Die drei Stellen, an denen ein Prüfer bei S8 zuerst graben sollte (aus dem
Selbstaudit, ehrlich benannt statt versteckt):
1. **Die Kopf-Fassade** leitet Includes und Defines per Generatorausdruck ab.
   K2 kann Generatorausdrücke zur Konfigurierzeit **nicht** auswerten — steht
   so im Modulkopf. Ist die Lücke wirklich nur durch K3 gedeckt?
2. **K2b vergleicht gegen genau ein Ziel** (`EqCopilot`). Ob sein
   Ausschlusssatz trägt, ist nachgemessen (22.08., Manifest B8): von
   `JUCE_SHARED_CODE`, `JUCE_STANDALONE_APPLICATION` und
   `JUCE_VST3_CAN_REPLACE_VST2` kommt **keines** in irgendeinem Header der vier
   Kernmodule vor; `JUCE_MODULE_AVAILABLE_` trifft zweimal, beide in
   `juce_core/native/juce_BasicNativeHeaders.h` auf `juce_opengl` gegated —
   ein Modul, das weder der Kern noch `EqCopilot` hat, beide sehen es also
   gleich undefiniert. Offen bleibt die Frage für **drei** Ziele: welches ist
   dann die Referenz?
3. **A14s Nadelliste** kommt aus `plugin-identities-v1.json`. Abgeleitete
   Identitätsträger wie `JucePlugin_AAXIdentifier` / `_CFBundleIdentifier`
   (`com.evenacadia.EqCopilot`) stehen dort nicht — geprüft und für gedeckt
   befunden: der Firmenname ist Teilstring jeder abgeleiteten ID, und A14
   sucht Teilstrings. Gegengemessen 22.08.: `com.evenacadia` steht ohnehin
   **auch im gebauten Bundle nicht**, weil beide Makros macOS-/AAX-Wege sind,
   die der Windows-VST3-Bau nie übersetzt. Ein Prüfer sollte trotzdem fragen,
   ob es einen Identitätsträger gibt, der *nicht* den Firmennamen enthält.

Mehr braucht es nicht: die SessionStart-Hooks legen Wahrheitskern, Hub-Stand,
Design-Stand und Git-Stand von selbst vor. **Nur das Ticket muss genannt
werden** — „go" ohne Ticketgrenze widerspricht der Sessionregel in
`docs/bauaufteilung-sonden.md` („1 Session = 1 Ticket + sein Beweismanifest
+ sein Frischkontext-Prüfer") und endet in Code ohne Manifest.

**Prüfstein für den User:** liegt am Ende ein neues Manifest in
`docs/beweise/`? Wenn nein, ist die Session **offen**, egal was sie meldet.
Für das jeweils nächste Ticket dieselben zwei Zeilen mit der nächsten Nummer
aus der Tabelle in `docs/bauaufteilung-sonden.md`.

## ▶ Der eine nächste Schritt (Technik): S9 — `SONDE-007b`

**S8 / `SONDE-007a` ist gebaut (22.08., Manifest `docs/beweise/SONDE-007a.md`):**
`NakamaKern` ist eine echte Static-Lib mit den vier geteilten Quellen
(`state/*.cpp` + `vertrag/NakamaVertrag.cpp`), **einmal** übersetzt statt je
Ziel; angebunden über `nakama_kern_anbinden()`. Vier Riegel, jeder beim Fallen
vorgeführt (K1 Präprozessor · K2 Linkhülle · K2b gleiche JUCE-Konfiguration ·
K3 = Kanon-Bein **A14**, misst das Artefakt). Kanon 18 → **19**.
**T2 ist offen** — kein Frischkontext-Prüferurteil im Manifest.

**Drei Dinge, die S9 aus S8 mitnehmen muss** (sonst wird es teuer):

1. **Der Kern übersetzt gegen JUCE-KÖPFE, nicht gegen JUCE-Module.** Die
   Implementierung kommt vom verbrauchenden Ziel. Die drei Bundles müssen
   daher `juce_core`, `juce_events`, `juce_data_structures` und
   `juce_cryptography` selbst linken — genau das tut `nakama_kern_anbinden()`;
   ein Ziel, das den Kern ohne diese Funktion anbindet, verliert zur Linkzeit
   Symbole. Grund und Messung: Kopf von `eq-copilot/cmake/NakamaKern.cmake`.
2. **K2b vergleicht heute gegen `EqCopilot`.** Kommen Probeeq und Suna dazu,
   muss entschieden werden, ob der Kern gegen *alle drei* geprüft wird oder
   die drei untereinander gleich konfiguriert sein müssen — die Funktion
   `nakama_kern_konfig_pruefen(kern referenz)` nimmt eine Referenz.
3. **NAK-52:** §53.4 verlangt Identität ausschließlich aus
   `plugin-identities-v1.json`. S8 hat nur die Kern-Hälfte gebaut; die
   Target-Schichten lesen die Werte weiterhin nicht, sondern tragen sie als
   CMake-Literale. Das ist **S9s Auftrag** — inklusive Umbau von
   `EqCopIdentityTest`, das die CMake-Quelle heute in genau dieser Textform
   einfriert (`tests/IdentityTestMain.cpp:257-263`).

Vorher lesen: `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Errata-Block zuerst,
§53.4 und §53.5), `docs/bauaufteilung-sonden.md` S9-Zeile,
`docs/beweise/SONDE-007a.md` §1 („Was dieses Ticket ausdrücklich NICHT
behauptet").

**Vor der Installation des neuen Bundles:** NAK-41 — ein Projekt, das der neue
Build speichert (Schema 2), verliert im 16.08.-Build seine Messpunkt-Identität.
Installation bleibt User-Klick (NAK-32).

Beim Bauen gilt: 1 Session = 1 Ticket + Beweismanifest + Frischkontext-Prüfer;
Code ohne Manifest heißt *offen*. Beweislauf:
`pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/<Ticket>.md -Anhaengen -Titel '<Ticket>'`.

## Was beim User liegt

| Was | Wo |
|---|---|
| ~~FL-Termin B~~ — beide Termine erledigt 22.08. | Optional offen: Teil 3 der Klickliste B (`nakama-altprojekt.flp` = Legacy-Fixture für SONDE-002) und das Wegräumen von `Common Files\VST3\eq-copilot\` (Admin-Klick) |
| Figma-Stände der drei Apps (Gen, Probeeq, Suna) | per Figma-MCP nach `design/assets/figma/` holen; `design/` übersetzt sie |
| Installation des 21.08.-Bundles (mit Hostbrücke, Version 0.3.0) | erst mit dem nächsten bewiesenen Stand, per UAC-Klick |

## Offen-Set

`docs/offene-punkte.md` — neu seit 22.08.: NAK-40 (`instance_id` bytegleich vs.
hex32 der v3-Adresse, SONDE-010), NAK-41 (Schema-2-Stand im alten Build = stiller
Identitätsverlust; vor der Installation wissen), NAK-42 (Rust klassifiziert die
ungültigen DTOs noch nicht — vor G1), NAK-43 (`presentation_latency`: ein
verworfener Wertwechsel ohne Protokoll — vor SONDE-009), NAK-44 (Termin A2:
PDC-Golden für `aux_priority_sidechain`, Bus-Identität/L/R, FL-Version). NAK-33 trägt den Nachtrag „State
trägt 13, UI-Frage offen". Neu seit 21.08.: NAK-30 (Umbenennung zu Nakama
Studio / Gen / Probeeq / Suna = Identitätsticket), NAK-31 (`analyze-track.py`
liegt im FL-Repo), NAK-32 (`install/` unversioniert), NAK-33 (12 oder 13
Parameter je Band), NAK-34–37 (HostProbe-Zählung, Hör-Markierungs-Verriegelung,
Broker-Binaries, AuxSpikeTest nicht im Kanon).

## Parallele Sessions

Am 21.08. lief parallel die SONDE-005b-Session (T2 Runde 3, Commit
`4f7182b` 21:57 — Fund: der FlatBuffers-Verifier prüft in C++ kein UTF-8,
C++ stürzte ab, wo Rust sauber ablehnte; gefixt, Binärkorpus 47). Der
geteilte Index hat dabei **elf gestagte Dateien dieser Inventur-Session in
jenen Commit mitgenommen** (CLAUDE.md, NEXT-SESSION, plugin-wissen,
Bauaufteilung, Entwurf-Errata (l)/(m), offene-punkte NAK-38/39, lib.rs- und
Probe-Kommentare, eq-aggregat-Schema, tokens.json, depth-primer): Inhalt
vollständig und richtig, Zuordnung nicht — keine Historie umschreiben.
Lehre steht in `reference_werkzeug-landminen-windows` (Memory) Nr. 6:
bei paralleler Session `git add <dateien>` + `git commit` ohne Pfade, und
zwischen beidem keine Pause.
