# NEXT-SESSION — Einstieg für die nächste Runde

> Stand: **21.08.2026, nach der Kontext-Inventur und dem Interview mit dem User.**
> Die Wahrheit steht in `CLAUDE.md` (Wahrheitskern + Register der User-Entscheide
> mit Zitat). Dieses Blatt sagt nur, wo wir stehen und was als Nächstes kommt —
> es wiederholt die Wahrheit nicht. Wenn hier und in CLAUDE.md etwas
> Verschiedenes steht, gilt CLAUDE.md; dann dieses Blatt berichtigen.

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
  (S5) und FlatBuffers (S6). Manifeste in `docs/beweise/`.
- **Offen an S5/S6:** Schließungsvorbehalt §65 (erst nach S4); T2 zu SONDE-005a
  lief in zwei Runden NEEDS_WORK, Runde 3 lief am 21.08. in einer parallelen
  Session — Stand im Manifest `docs/beweise/SONDE-005b.md` §6 nachsehen.
- **S4 (Capabilityreport) und Gate G0 warten auf die FL-Termine A/B des Users**
  (`eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md`, `…-B-HOSTZEIT.md`; Spike-Ordner leer).
- **Kanon:** 15 Beine in `tools/beweise.ps1`; letzter vollständiger Lauf siehe
  jüngstes Manifest in `docs/beweise/`.

## ▶ Der eine nächste Schritt (Technik): S7 — `SONDE-006`

State-Schema 2, fester Parameterbestand, reine Schema-1-Migration; dort wird
die RFC-8785-Kanonisierung für `state_hash` erstmals gebraucht. User 21.08.:
„Termine bald; bis dahin S7". Danach S8 (`SONDE-007a`, gemeinsamer Kern ohne
`JucePlugin_*`-Konstanten). Vorher lesen: `docs/FL-Nakama-Sonden-Design-Entwurf.md`
(Errata-Block zuerst) und `docs/bauaufteilung-sonden.md`.

Beim Bauen gilt: 1 Session = 1 Ticket + Beweismanifest + Frischkontext-Prüfer;
Code ohne Manifest heißt *offen*. Beweislauf:
`pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/<Ticket>.md -Anhaengen -Titel '<Ticket>'`.

## Was beim User liegt

| Was | Wo |
|---|---|
| FL-Termin A (Aux/PDC/Recall, ~20 min) und B (Hostzeit/Automation, ~25 min) | Klicklisten in `eq-copilot/docs/`; Ergebnis-JSON landet in `%APPDATA%\evenacadia\nakama\spike\` |
| Figma-Stände der drei Apps (Gen, Probeeq, Suna) | nach `Nakama-Design/assets/figma/` geben; das Design-Repo übersetzt sie |
| Installation des 21.08.-Bundles (mit Hostbrücke, Version 0.3.0) | erst mit dem nächsten bewiesenen Stand, per UAC-Klick |

## Offen-Set

`docs/offene-punkte.md` — neu seit 21.08.: NAK-30 (Umbenennung zu Nakama
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
