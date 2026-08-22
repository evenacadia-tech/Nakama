# NEXT-SESSION — Einstieg für die nächste Runde

> Stand: **22.08.2026, nach S7 (`SONDE-006`).** Davor: 21.08., Kontext-Inventur und Interview mit dem User.
> Die Wahrheit steht in `CLAUDE.md` (Wahrheitskern + Register der User-Entscheide
> mit Zitat). Dieses Blatt sagt nur, wo wir stehen und was als Nächstes kommt —
> es wiederholt die Wahrheit nicht. Wenn hier und in CLAUDE.md etwas
> Verschiedenes steht, gilt CLAUDE.md; dann dieses Blatt berichtigen.

> **Seit 22.08. gibt es den Nakama-Hub** (`docs/hub/`, Artefakt-Adresse in
> `hub.json`): das Briefing für den User und jede Session — Plan erledigt/offen,
> was bei ihm liegt, Figma-Stand der drei Apps, Codex-Review-Upload. Pflicht:
> lesen · Eingang prüfen · nachziehen (CLAUDE.md, Abschnitt „Hub"). Dieses
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
- **Kanon:** 18 Beine in `tools/beweise.ps1` (seit 22.08.: B2 `EqCopStateMigrationTest`,
  A12 `erzeuge_state_fixtures.py --pruefen`, A13 `pruefe_host_capabilities.py`);
  letzter vollständiger Lauf siehe jüngstes Manifest in `docs/beweise/` (`SONDE-004.md`).

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

## ▶ Der eine nächste Schritt (Technik): S8 — `SONDE-007a`

**S7 / `SONDE-006` ist gebaut (22.08., Manifest `docs/beweise/SONDE-006.md`):**
State-Schema 2 (`eq-copilot/schemas/state/nakama-state-v2.md`), fester
Parameterbestand (109 IDs, `nakama-parameter-v1.json`), reine Schema-1-Migration
mit Goldens, read-only bei fremdem Major, Host-Dirty, `state_hash` nach RFC 8785
in drei Sprachen bytegleich. Kanon 17 Beine (B2 + A12 neu). **T2: PASS** (Frischkontext-
Prüfer, 22.08.; neun nicht-blockierende Befunde nachgearbeitet, Manifest §5/§6).

**Vorher Gate G0** (T3, eigene Session; Bauaufteilung §3/§5): `/c-review` auf den
Bridge-Patch + Codex, Bruchaufträge Gate 1 (neutrale Instanz verändert Audio)
und Gate 5 (Telemetrie steuert Gain) — P0 schließt erst damit.

**Dann S8 (`SONDE-007a`):** gemeinsamer Kern ohne `JucePlugin_*`-
Konstanten (NAK-23b). `plugin/state/` ist dafür schon JUCE-core-rein gebaut
(`Bundle::eqcp/nkpr/nkac` als Klassenmenge je Bundle). Vorher lesen:
`docs/FL-Nakama-Sonden-Design-Entwurf.md` (Errata-Block zuerst, §53.4),
`docs/bauaufteilung-sonden.md` S8-Zeile. User 21.08.: „Termine bald; bis dahin S7".

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
| Figma-Stände der drei Apps (Gen, Probeeq, Suna) | nach `Nakama-Design/assets/figma/` geben; das Design-Repo übersetzt sie |
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
