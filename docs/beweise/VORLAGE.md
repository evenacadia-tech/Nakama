# Beweismanifest — SONDE-0NN «Kurztitel»

> **Das hier ist die Vorlage.** Kopieren nach `docs/beweise/SONDE-0NN.md`, Kopf
> ausfüllen, Abschnitte 1–6 abarbeiten. Bei einem Phasengate zusätzlich
> Abschnitt 7.
>
> **Die eine harte Regel** (`docs/bauaufteilung-sonden.md` §2): *Eine Behauptung
> ohne Rohausgabe ist ein **gescheitertes** Ticket, kein fast fertiges.* Nichts
> zusammenfassen, nichts kürzen — stdout und stderr gehen wortgleich in den
> Beweis. **Wo** sie liegen, hängt allein an der Art der Messung:
>
> | Art der Messung | Wohin die Rohausgabe gehört |
> |---|---|
> | **Eigene Ticketproben** — Handmessungen, Bruchproben, Grenzfälle; alles, was der Worker selbst fährt | wortgleich in **dieses Manifest**, Abschnitt 2 |
> | **Kanon-Läufe** von `tools/beweise.ps1` | wortgleich nach `docs/beweise/roh/<TICKET>-<sha7>.md`; im Manifest bleiben Überschrift, die `**Lauf:** …`-Zeile mit Urteil und Exitcode, der Kopf „woran gemessen wurde“ und die Übersicht mit Verweis dorthin — Abschnitt 3 |
>
> Die Trennung verschiebt den **Ort** der Rohausgabe, nie ihren **Umfang**
> (NAK-96, 29.08.2026). Zusammengefasst statt eingefügt wird in **keinem** der
> beiden Fälle. Ein Ticket ohne Manifest ist **offen**, auch wenn alles
> kompiliert.

| Feld | Wert |
|---|---|
| Ticket | `SONDE-0NN` |
| Phase / Session | P? / S? |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §?? — **wörtlich**, nicht meine Zusammenfassung |
| Commits | `…` |
| Datum | JJJJ-MM-TT |
| Prüfstufen | T1 ☐ · T2 ☐ · T3 ☐ (nur am Gate) |

> ⚠️ **Ein Häkchen wird gesetzt, NACHDEM die Stufe gelaufen ist — nie vorher.**
> In `SONDE-001-002` stand am 20.08. `T2 ✅` im Kopf, bevor der Prüfer überhaupt
> startete; er hat es selbst gefunden. Wer ein Manifest maschinell erzeugt, baut
> den Riegel in den Erzeuger ein: kein Urteil eingetragen ⇒ keine Datei
> geschrieben.

---

## 1. Ticket-Behauptungen

Jede Zeile ist eine Behauptung aus dem **Gate-Text**, nicht aus meinem Kopf.

| # | Behauptung (Gate-Text) | Befehl | Ergebnis | Rohausgabe | Datum |
|---|---|---|---|---|---|
| 1 | | `…` | ☐ | [↓ B1](#b1) | |
| 2 | | `…` | ☐ | [↓ B2](#b2) | |

---

## 2. Rohe Ausgaben der eigenen Ticketproben

Wortgleich und ungekürzt: alles, was der Worker **selbst** gemessen hat —
Handmessungen, Bruchproben, Grenzfälle. Die Rohausgaben der **Kanon-Läufe**
stehen nicht hier, sondern unter `docs/beweise/roh/`; was davon im Manifest
bleibt, sagt Abschnitt 3.

<a id="b1"></a>
### B1 · «Behauptung 1»

**Befehl:** `…` · **Exitcode:** ? · **Datum:** JJJJ-MM-TT

```text
(unveränderte Ausgabe — kein Kürzen, kein Umformulieren)
```

<a id="b2"></a>
### B2 · «Behauptung 2»

**Befehl:** `…` · **Exitcode:** ? · **Datum:** JJJJ-MM-TT

```text

```

---

## 3. Kanon-Lauf

Der komplette Beweis-Kanon gehört in **jedes** Ticket-Manifest — er belegt, dass
der Zuwachs nichts Bestehendes umgeworfen hat. Der Runner hängt sich selbst an:

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-0NN.md -Anhaengen -Titel 'SONDE-0NN'
```

`-Bauen` ist der Normalfall: ohne frischen Bau beweist der Lauf den **alten**
Quellstand, und der Runner verweigert dann die Beglaubigung (Exitcode 4).
Vergleichspunkt ist `docs/beweise/S0-basislinie.md`.

Der Lauf schreibt in **zwei** Dateien (NAK-96, 29.08.2026):

| Datei | Inhalt |
|---|---|
| dieses Manifest | Überschrift, die Zeile `**Lauf:** … **Urteil:** … **Exitcode:** … **Rohausgabe:** …`, der Kopf „woran gemessen wurde" und die Übersichtstabelle. Jede Zeile der Übersicht verlinkt in die Rohausgabe. |
| `docs/beweise/roh/<TICKET>-<sha7>.md` | derselbe Kopf plus Baustand der Prüfbinaries, vollständige stdout/stderr jedes Beins und das Bauprotokoll. |

Die Rohausgabe trägt den Stand, den sie beweist: `-dirty` bei unbestätigtem
Arbeitsbaum, ein Zählsuffix beim zweiten Lauf auf demselben Commit. Bestehende
Rohausgaben werden nie überschrieben.

Diese Aufteilung gilt **nur für Kanon-Läufe**. Abschnitt 2 ist davon nicht
berührt: die Rohausgaben der eigenen Ticketproben bleiben wortgleich im
Manifest.

> ⚠️ Die Lauf-Zeile ist wortgleich festgelegt: `tools/plan/planstand.py` liest
> die Kanon-Zahl per Regex `KANON` daraus zurück. Verweise werden angehängt,
> die Zeile selbst nie umformuliert.

_(Der angehängte Abschnitt erscheint unter dieser Zeile.)_

---

## 4. T1 — Selbstaudit

Feste Liste aus `docs/bauaufteilung-sonden.md` §2, **nie abgekürzt**. „Nicht
betroffen" ist eine gültige Antwort — leer bleiben ist keine.

| # | Punkt | Befund |
|---|---|---|
| 1 | Numerische Ränder: NaN, ±inf, 0, negativ, Überlauf, Wrap | |
| 2 | Gegenpfad vorhanden? Save↔Load · Bind↔Unbind · Start↔Stop · Apply↔Revert · Subscribe↔Unsubscribe · Install↔Rollback | |
| 3 | Behauptungs-Integrität: existiert jede genannte Datei / Funktion / Zeile **wirklich**? (Datei geöffnet, nicht zitiert) | |
| 4 | Lügt der UI-Text? Zeigt die Anzeige einen Zustand, den es nicht gibt? | |
| 5 | Anzeige-Pflichten (nur bei UI-Diff, Entwurf §0.4): Frische/stale · Konfidenz · `arming`/`audible_ready` · Capability-Degradation · Konflikt · welche Aktion gerade **nicht** aktiv ist | |
| 6 | Audiothread: nachweislich keine Allokation, kein Lock, kein I/O, kein Logging? | |

---

## 5. T2 — Frischkontext-Prüfer

| Feld | Wert |
|---|---|
| Agent | `evaluator` (eigener Kontext, sieht meine Zusammenfassung **nie**) |
| Vorgelegt | 1. Diff · 2. dieses Manifest · 3. Gate-Text §?? im Wortlaut |
| Urteil | ☐ PASS ☐ NEEDS_WORK |
| Datum | |

**NEEDS_WORK ist blockierend**, kein Hinweis. Die Befunde kommen unten in
Abschnitt 6 — nachgebessert wird vor dem nächsten Ticket, nicht später.

---

## 6. Befunde aus diesem Ticket

Kein Befund verschwindet still: **gefixt**, oder als NAK-Zeile in
`docs/offene-punkte.md`, oder als protokollierte Widerlegung.

| Befund | Quelle (T1/T2/T3) | Gegen die **Quelldatei** verifiziert? | Ausgang |
|---|---|---|---|
| | | ☐ ja | gefixt / NAK-?? / widerlegt weil … |

> Erfahrungswert dieses Projekts: **~25 % der AI-Auditbefunde waren falsch.**
> Deshalb wird jeder Befund an der Quelldatei geprüft, **bevor** gehandelt wird.

---

## 7. Nur bei Phasengates — T3, adversariale Runde

Eigene Session. Prüfmittel nach angefasster Fläche
(`docs/bauaufteilung-sonden.md` §2/§5); **Codex gehört bei jeder Phase dazu** —
zwei Läufe desselben Modells finden korrelierte Fehler.

| Feld | Wert |
|---|---|
| Gate | G? (Phase P?) |
| Prüfmittel | ☐ `/c-review` ☐ `/rust-review` ☐ `/security-review` ☐ **Codex** |
| Bruchaufträge (die acht harten Systemgates, **Entwurf** §49.2) | z. B. Gate 1, Gate 5 |

**Falsifikations-Vorgabe** — der Prüfer bekommt einen Bruchauftrag, keine
Prüfliste. Wortlaut-Muster:

> „Finde einen Eingabepfad, der Gate 1 verletzt: Passive Probe, neutrales Main
> oder neutraler Hard-Bypass verändert einen gültigen Audiopuffer. Antworte mit
> dem konkreten Pfad oder mit ‚kein Pfad gefunden' — nicht mit einer
> Einschätzung."

| Bruchauftrag | Prüfer | Antwort | Verifiziert an der Quelle | Ausgang |
|---|---|---|---|---|
| Gate ? | | Pfad gefunden / kein Pfad gefunden | ☐ | |
