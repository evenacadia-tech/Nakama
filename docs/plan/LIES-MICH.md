# docs/plan — der Planstand und die offenen Fragen

Hier liegt, woran gearbeitet wird und was beim User liegt. **Der Status wird
gerechnet, nicht gepflegt** — das ist der ganze Unterschied zu vorher.

## Warum es diesen Ordner gibt

Bis zum 23.08.2026 stand der Planstand als `status`-Feld in `docs/hub/hub.json`
und wurde von Hand umgestellt; eine Briefing-Seite spiegelte ihn. Beides musste
jemand nachziehen. Der User hat es abgeschafft:

> „wir brauchen eine andere möglichkeit als den hub. eie gibt keine
> automatische aktualisation , das heißt er wird driften und somit drifted plan
> stand auch."
> → **„saeite ganz weg, alles im repo + Skill bauen, der automatisch die
> offenen fragen nach und nach an mich stellt und gleich einarbeitet in den
> plan"**

Der alte Stand liegt vollständig unter `docs/archiv/hub-2026-08-23.json`.

## Was hier liegt

| Datei | Was drin steht | Wer sie pflegt |
|---|---|---|
| `plan.json` | Phasen, Schritte, Ticket, Klartext, Beleg-Pfad, geforderte Prüfstufe. **Kein Statusfeld.** | Mensch/Session — es ist Text, kein Messwert |
| `fragen.json` | `offen[]` = Fragen an den User · `beantwortet{}` = jede Antwort im Wortlaut mit Datum | der Skill `/fragen` |
| `bilder/` | die Bilder zu den Fragen | Mensch/Session |

Und daraus entsteht, ohne Zutun:

| Erzeugnis | Erzeuger |
|---|---|
| `docs/PLAN-STAND.md` | `tools/plan/planstand.py` — **nie von Hand editieren** |
| `docs/ANTWORTEN-OFFEN.md` | `tools/plan/antworten_blatt.py` |

## Wie der Status gemessen wird

```
kein Beleg                                  →  □ offen
Beleg liegt                                 →  ▣ gebaut
Beleg liegt + Urteilsmarke der geforderten
Stufe mit PASS                              →  ■ abgenommen
```

Die **Urteilsmarke** steht im Beweismanifest selbst — eine Zeile, das einzige
Stück Status, das ein Mensch schreibt, weil ein Urteil kein Messwert ist:

```
<!-- NAKAMA-URTEIL: T2 PASS 2026-08-22 -->
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-23 offen -->
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-23 nachgearbeitet -->
```

- **Stufe** `T1` (Selbstaudit des Erbauers) · `T2` (frischer Prüfer) · `T3`
  (adversariale Gate-Runde). Welche Stufe ein Schritt *braucht*, steht in
  `plan.json`; nur eine Marke auf dieser Stufe oder höher kann abnehmen.
- **Viertes Wort** nur bei `NEEDS_WORK`: `offen` = der Befund steht noch, das
  ist Arbeit · `nachgearbeitet` = geschlossen, es fehlt nur ein frisches
  Urteil. Daraus leitet das Blatt ab, was als Nächstes dran ist.
- Mehrere Runden? Die **letzte** Marke gilt — neue Runde unten anhängen.

🔑 **Fail-closed.** Fehlende Marke, falsche Stufe oder `NEEDS_WORK` heißen
*nicht abgenommen*. Vergessen führt zur Untertreibung, nie zur Übertreibung.
Und eine Zeile, die `NAKAMA-URTEIL` sagt aber die Form verfehlt, wird **nicht**
still übergangen: sie erscheint als Warnung im Blatt, und `planstand.py` gibt
Exitcode 4.

## Was du tun musst

**Nichts nachziehen.** `tools/hooks/planstand.sh` läuft nach jedem Befehl,
misst, ob der Quellstand im Blatt noch stimmt, rechnet sonst neu und committet
das Blatt allein. Der SessionStart-Hook `plan-primer.sh` liest den Kopf vor.

Was du tust, wenn du etwas **änderst**:

| Du hast … | … dann |
|---|---|
| ein Ticket gebaut + Manifest geschrieben | nichts — der Beleg liegt, das Blatt sieht ihn |
| als Prüfer geurteilt | **Marke ins Manifest** setzen. Sonst bleibt der Schritt „gebaut" |
| einen Befund geschlossen | das vierte Wort auf `nachgearbeitet` setzen |
| einen Schritt im Plan ergänzt/umformuliert | `plan.json` — Text, kein Status |
| eine Frage an den User | Karte in `fragen.json` → `offen[]`, Bild nach `bilder/` |
| eine Antwort bekommen | **`/fragen`** benutzen; nie von Hand zusammenfassen |

## Was es nicht mehr gibt

- Die Briefing-Seite `nakama-briefing.philipld.chatgpt.site`. `briefing-hub/`
  bleibt als Verlauf liegen — **nicht deployen, nicht füttern, nicht lesen.**
- `tools/hub/hub_sync.py` (`holen`/`senden`) und `tools/hub/plan_blatt.py`.
- Die Pflicht, je Session einen Hub nachzuziehen.
