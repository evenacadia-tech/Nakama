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
| `plan.json` | Phasen, Schritte, Ticket, Klartext, kurzer `leitungsname`, Beleg-Pfad, geforderte Prüfstufe. **Kein Statusfeld.** | Mensch/Session — es ist Text, kein Messwert |
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

Zu Arbeitsbeginn und nach jedem abgeschlossenen Ticket bewusst ausführen:

```powershell
py -3.13 tools/plan/planstand.py
```

Ändert sich das gerechnete Blatt aus bereits committeten Quellen, wird nur
`docs/PLAN-STAND.md` mit explizitem Pathspec committet und direkt gepusht. Bei
uncommittierten Plan- oder Beweisquellen wird nichts automatisch committet.

Was du tust, wenn du etwas **änderst**:

| Du hast … | … dann |
|---|---|
| ein Ticket gebaut + Manifest geschrieben | Generator ausführen; der Beleg wird dadurch sichtbar |
| als Prüfer geurteilt | **Marke ins Manifest** setzen. Sonst bleibt der Schritt „gebaut" |
| einen Befund geschlossen | das vierte Wort auf `nachgearbeitet` setzen |
| einen Schritt im Plan ergänzt/umformuliert | `plan.json` — Text, kein Status |
| eine Frage an den User | Karte in `fragen.json` → `offen[]`, Bild nach `bilder/` |
| eine Antwort bekommen | **`/fragen`** benutzen; nie von Hand zusammenfassen |

## Das Zielbild

Seit dem 23.09.2026 steht neben dem Planstand das **Zielbild**: `docs/ZIELBILD.md`
beschreibt in Alltagssprache, wie Nakama arbeitet, wenn der heutige Plan mit allen
bisherigen Entscheidungen fertig ist. Der User sieht es im Plan-Tab (Taste Z). Sein
Auftrag:

> „ich brauche ein sich stets aktualisierendes standbild in form einer übersichtlichen
> beschreibung was nakama aus sicht eines potentiellen users können würde"

Getrennt wie beim Planstand: der **Text ist autoriert**, seine **Aktualität gerechnet**.
Der Fußblock `ZIELBILD-ABGLEICH` hält fest, welche Entscheidquellen eingearbeitet sind:
beantwortete, offene und geparkte Karten aus `fragen.json`, die Abnahmen unter
`design/abnahmen/`, die Registerzeilen mit einer Klasse „Produkt…" und die Planschritte.
Jede neue oder geänderte Quelle macht das Zielbild *veraltet*, bis sie eingearbeitet ist.

| Befehl | Wirkung |
|---|---|
| `py -3.13 tools/plan/zielbild.py pruefen` | Exit 0 aktuell · 4 veraltet, jede neue oder geänderte Quelle beim Namen · 2 Werkzeugfehler |
| `py -3.13 tools/plan/zielbild.py abgleichen` | schreibt nur den Fußblock; verweigert mit Exit 4, solange eine offene oder geparkte Karte im Text fehlt oder eine Planmarke ins Leere zeigt |
| `py -3.13 tools/plan/zielbild.py anzeige` | das Bild für den Plan-Tab |
| `py -3.13 tools/plan/zielbild_test.py` | Proben des Werkzeugs |

Wer nachzieht: `/fragen` im selben Änderungssatz wie die Antwort, alles andere der
Dirigent im Abschlussfenster. Nachziehen heißt: die betroffene Stelle in Alltagssprache
ändern — nur Entschiedenes und den gültigen Plan, nie Vorschläge —, dann `abgleichen`,
dann `py -3.13 tools/plan/dokuriegel.py docs/ZIELBILD.md`, dann mit Pathspec committen.
Eine Planmarke `<!-- plan: P4–P5 -->` steht als eigene Zeile unter einer Überschrift oder
am Ende einer Listenzeile; die Anzeige rechnet daraus den Stand aus `docs/PLAN-STAND.md`
(✓ fertig · ◐ läuft · ○ offen).

## Was es nicht mehr gibt

- Die Briefing-Seite `nakama-briefing.philipld.chatgpt.site`. Der Ordner
  `briefing-hub/` wurde am 12.09.2026 aus dem Baum entfernt; sein letzter
  Stand liegt im Verlauf bis Commit `f9750046`.
- `tools/hub/hub_sync.py` (`holen`/`senden`) und `tools/hub/plan_blatt.py`.
- Die Pflicht, je Session einen Hub nachzuziehen.
