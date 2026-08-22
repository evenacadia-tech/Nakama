# 2026-08-22 — Figma-Depot: eine Datei, verbindlich; dunkel zuerst

> ## Abnahme (bindend)
>
> Der User hat die Figma-Datei **Nakama-Design** (https://www.figma.com/design/NPCQYSkoZEd4Av0NlKxBOd/Nakama-Design?node-id=0-1, Key `NPCQYSkoZEd4Av0NlKxBOd`)
> als Quelle festgelegt. Wörtlich:
>
> > „das ist das neue verbindliche und einzige Depot indem sich die aktuellsten Designs befinden. ich habe für jedes design vor eine helle und dunke variante zu erstellen. stand jetzt aber erstmal nur dunkel“

## Was damit festliegt

1. **Eine Quelle.** Die Datei `Nakama-Design` ist das einzige Depot der
   aktuellen Stände. `Testdummy` und `fable-dummy` sind Verlauf; was nicht im
   Depot ist, ist nicht aktuell.
2. **Dunkel zuerst.** Je App ist eine helle und eine dunkle Fassung geplant;
   **verbindlich ist heute nur die dunkle.** Helle Host/Ground-Rahmen im Depot
   (`6:2424`, `6:2658`, `6:2863`) sind Arbeit des Users, keine Vorgabe — bis
   er es sagt.
3. **Gen Studie 04 liegt im Depot** (Rahmen `6:3`) und ist damit der aktuelle
   Gen-Stand (dunkel). Die 18 Lesarten aus
   `2026-08-21-gen-auftrag-figma.md` bleiben offen; der User beantwortet sie
   im Nakama-Hub (Karte U2).
4. **Claude holt die Stände** (Arbeitsannahme, kein User-Entscheid): per
   Figma-MCP `download_assets` (PNG, 4×) aus dem Depot in `assets/figma/`,
   datiert, mit Node und SHA-256 in `assets/figma/LIES-MICH.md`. Der User
   exportiert nicht mehr von Hand; die Truhe bleibt der Beleg, gegen den ein
   Blatt gebaut wurde. Dasselbe gilt für gebackene Assets (Wortmarken, Schale,
   Leuchtsäume) je Skalierungsstufe (`2026-08-22-erfolgskriterium-exakt-wie-figma.md`
   Punkt 5): `download_assets` mit `defaultScale` 1 · 1,25 · 1,5 · 2.

## Stände vom 2026-08-22 (Truhe)

| Datei | Node |
|---|---|
| `assets/figma/2026-08-22-gen.png` | `6:13` → `6:14` |
| `assets/figma/2026-08-22-probeeq.png` | `6:1906` → `6:1907` |
| `assets/figma/2026-08-22-suna.png` | `6:2629` → `6:2630` |

## Bezug

- `abnahmen/2026-08-21-designvertrag.md` — „Figma ist Quelle; Repo setzt um"
- `abnahmen/2026-08-21-gen-auftrag-figma.md` — die 18 Punkte (Antworten im Hub)
- `Nakama/docs/hub/` — der Hub (Register-Zeile in `Nakama/CLAUDE.md`)
