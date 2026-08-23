# Archiv der Design-Richtungen — nur Verlauf, nie Vorgabe

**Stand 21.08.2026.** Der User hat im Kontext-Interview festgelegt: „das finale
design wird aktuell in figma gemacht. alle 3 apps werden ein design haben mit der
selben identität. **alle alten sind alt.** Ein Design entwickelt sich und ist
nicht einfach da." Die aktive Design-Quelle ist Figma (User); übersetzt wird in
`design/` (seit 22.08. im Repo, vorher eigenes Repo Projekte/Nakama-Design). Alles hier ist Verlauf: lesbar, um Irrwege nicht zu
wiederholen — nicht, um daraus etwas abzuleiten.

Jede Datei trägt im Kopf ihren Status. Was kein Kopf tragen kann (Bilder,
Fonts, JSON), steht hier.

## `alt-2026-08/` — die Richtungen vom 13.–17.08. für die Plugin-Hauptansicht

| Artefakt | Richtung | Status | Wer, wann |
|---|---|---|---|
| `eq-copilot-spectral-field.html` + 2 PNG | HUD-Fassung des Spectral Field (hell, „ZIELKORRIDOR") | alt; der Zielkorridor widerspricht der Invariante „keine globale Zielkurve" | Claude 15.08. |
| `nakama-spectral-field-claude.html` | Spectral Field / Tiefenfeld-Musterblatt (Relief, Licht, Nebel, Songschleife, 7 Demo-Befunde) | geparkt 17.08. (User), alt seit 21.08. | Claude 16./17.08. |
| `nakama-spectral-field-vorentwurf.html` | Spectral Field, Codex-Vorentwurf (Codex-Besitz; Edits blockt seit 22.08. die `kreativ-schleuse.sh` mit dem ganzen Ordner — der Extra-Wächter `guard-codex-besitz.sh` ist entfallen, Commit `ef1efee`) | alt | Codex 16.08. |
| `nakama-kunstwerk-claude.html`, `render-kunstwerk.mjs` | Kunstwerk-Studie (Garten / Gewebe / Wesen) | geparkt 17.08., alt | Claude 17.08. |
| `nakama-problem-effects-studie.html` | Tusche-Einzelmarken (Borsten, Siegel) | **verworfen 17.08.** — User: „zerissenes Stroh", „Kochhut" | Claude 16.08. |
| `concepts/nakama-field-alphabet-v2.html`, `concepts/nakama-symbolalphabet-v1.png`, `assets/nakama-symbols.svg`, `sync-nakama-symbols.mjs` | Feld-Alphabet / 仲間-Bildmarke / Nakama-Phase-Schrift (Basisbild OpenAI-generiert) | hing am geparkten Bauplan 2.0; alt | Claude/Codex 16./17.08. |
| `fonts/` (5) | Manrope, NakamaPhase (für die obigen Blätter; nie ins Plugin eingebettet) | alt | — |
| `proben/probe-01…04.html`, `proben/render-probe.mjs`, `proben/renders/` | Vier Mechanismus-Proben zur Suche eines Hauptansicht-Inhalts | P01 Perkolation: Ästhetik abgenommen, als Dauerinhalt 18.08. verworfen („wasserfall aus farbigen pixeln"); P02–P04 verworfen (User) | 17.08. |
| `vorentwurf-renders/` (46) | Renders aus fünf Richtungen (HUD 01–10, Codex 13–16, Problemreaktionen 21–24, Claude t3/n4, Kunstwerk k1) | alt | 15.–17.08. |
| `render-blatt.mjs` | Playwright-Render der Spectral-Field-Blätter (Pfad zu `../../vendor-build` beim Umzug nachgezogen) | alt | — |

Die Demo-Befunde in diesen Blättern („118 Hz · +8,2 dB · SICHERHEIT 92 %" usw.)
sind erfundene Beispielwerte und wandern nie ins Plugin.

## `nakama-geraetefront/` — der User-Entscheid, der still ersetzt wurde

`NakamaGehaeuse-v1.h` ist die dunkle NAKAMA-Gerätefront, die der User am
14.08. als Plugin-Look entschieden hat (FL-Commit `7836128`). Am 15.08. ersetzte
Codex sie ohne Protokoll durch die Material-Kit-Front (`625b97a`, kein Body),
die seither kompiliert und installiert ist. Der User 21.08.: Material-Kit „Nie
abgenommen – bleibt Provisorium". `referenz/` sind die sieben externen
Moodboard-Bilder (fremde Designer, 14.08.), die diese Front inspiriert haben.
Die neue Front aller drei Apps kommt aus Figma — diese hier ist Verlauf.

## Was NICHT hier liegt

- `../prisma-studie/` — die Prisma-/Hörkompass-Studie (User-Idee, geparkt,
  eigenes Statusblatt).
- `../tokens.json`, `../gen-tokens.mjs` — die Material-Kit-
  Kette ist technisch lebendig (`tokens.json` → `plugin/src/LeitstandTokens.h`)
  und bleibt im Wurzelordner, beschriftet als Provisorium. Ihr
  Beschreibungsblatt `../ASSET-KIT.md` wurde am 23.08.2026 auf User-Wort
  gelöscht („alt und längst verworfen"; Historie in git).
- `../vendor-build/` — Playwright-Render-Infrastruktur (gemeinsam, `npm ci` dort).
- `docs/inspirationen/` (14 OpenAI-generierte Prisma-Bilder) — gelöscht 21.08.
  (User: „Weiß nicht mehr – raus"); in der Git-Historie bis `e2b553e` lesbar.
