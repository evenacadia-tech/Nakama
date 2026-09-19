---
name: tiefenaudit
description: Agenten-Tiefenaudit der Nakama-Codebase — dünner Starter. Rechnet aus der Bereichskarte je Bereich und je Naht einen Auftrag, lässt lesende Agenten parallel sammeln, Nahtprüfer die Übergänge lesen und Skeptiker jeden Befund angreifen; das Urteil fällt im Hauptlauf. Aufträge, Phasen und Agentenzahl stehen im Repo (docs/gesundheit/tiefenaudit/), nie hier. Aufruf /tiefenaudit [stand | betroffen <basis>..<ziel> | bereich <IDs> | naht <IDs> | lauf | nachtrag]; ohne Argument gilt stand (kein Agent).
---

# /tiefenaudit — Starter

Der Audit ist im Repo definiert, nicht hier. Dieser Skill sorgt nur dafür, dass
er richtig beginnt. **Er nennt absichtlich keine Agentenzahl und enthält keinen
Auftragstext:** beides rechnet `tools/pruefung/tiefenaudit_karte.py` aus
`docs/gesundheit/tiefenaudit/karte.json`. Steht hier je eine Zahl oder ein
Auftrag, ist das Drift und ein Befund.

## In dieser Reihenfolge

1. **Wo laufe ich?** Im Dirigentenkontext (Session `nakama-dirigent`) wird nie
   auditiert. Für alles außer `stand` und `betroffen` startet der Dirigent eine
   Prüfsession wie einen Worker (`docs/gesundheit/KONZEPT.md` §5) und gibt ihr
   diesen Aufruf als ersten Auftrag; er liest später nur den Kopf von
   `BEFUNDE.md`.
2. **Spielregeln lesen:** `docs/gesundheit/tiefenaudit/LIES-MICH.md`
   vollständig. Dort stehen die Phasen 0 bis 9, die Abbruchregeln, die drei
   Agentenarten, die Sicherheitsspur, Modelle und Wellen, die Betriebsarten und
   die Regeln, nach denen die Karte wächst.
3. **Vorprüfung, immer zuerst:**
   ```powershell
   py -3.13 tools/pruefung/tiefenaudit_karte.py --selbsttest
   py -3.13 tools/pruefung/tiefenaudit_karte.py --pruefen
   ```
   Alles außer `TIEFENAUDIT_SELBSTTEST: PASS` und
   `TIEFENAUDIT_VORPRUEFUNG: PASS` beendet den Aufruf: erst die Karte heilen
   (LIES-MICH §6), nie über einen Riss hinweg Agenten starten.
4. **Betriebsart ausführen** nach LIES-MICH §4 und §5:
   - `stand`: dazu `--befunde`; berichten, wie sich die bekannten Stellen auf
     die Bereiche verteilen und welche gemessene Naht in keiner Naht der Karte
     liegt. Kein Agent.
   - `betroffen <basis>..<ziel>`: `--betroffen`; Bereiche, Nähte und offene
     Kopplungsenden nennen. Kein Agent.
   - `bereich`, `naht`, `lauf`: Stand festhalten, Arbeitskopie anlegen, Aufträge
     mit `--auftraege <Laufordner>/auftraege --sha <Stand>` rechnen und jeden
     gerechneten Auftrag **wörtlich** an genau einen Agenten geben. Nach jeder
     Welle `--berichte <Laufordner>/roh`; ein Bereich ohne sauberen Scope-Beweis
     läuft allein neu.
   - `nachtrag`: nur Phase 8 auf einem fertigen Lauf.

## Harte Regeln

- Kein Auftrag wird von Hand geschrieben, umformuliert, gekürzt oder mit einem
  zweiten gebündelt. Ein Agent, ein Auftrag, ein Bericht.
- Agenten lesen nur und schreiben genau ihren einen Bericht. Sie bauen nicht,
  testen nicht, starten kein FL Studio und kein Codex.
- Kein Agent ordnet in Defekt, Lücke, Härtung ein; das Urteil fällt im
  Hauptlauf am Gate-Wortlaut und an den Invarianten aus `CLAUDE.md`.
- Jeder Roh-Befund geht durch den Skeptiker, bevor er eine Kennung bekommt.
- Sicherheitsstellen werden inventarisiert und an die Sicherheitsspur gereicht
  (Codex `gpt-5.6-sol` max), nie von einem Claude-Agenten beurteilt.
- Ein Ledger: bestätigte Befunde werden über den Kopf von `BEFUNDE.md`
  Registerzeilen in `docs/offene-punkte.md`; es gibt keine Punktwertung und
  keine zweite Befundbasis.
- Der sichtbare Checkout wird nie berührt. Geschrieben wird nur unter
  `docs/audits/<Lauf>/` und, im Nachtrag, in `karte.json`; Commit mit Pathspec
  aus der Arbeitskopie, nie `git add -A`, nie `--amend`.
- Keine Meldungen an den User außer einer Produktfrage als Kartenentwurf.
