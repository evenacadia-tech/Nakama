# Beweismanifest — SONDE-0NN «Kurztitel»

> **Das hier ist die Vorlage.** Kopieren nach `docs/beweise/SONDE-0NN.md`, Kopf
> ausfüllen, Abschnitte 1–6 abarbeiten. Bei einem Phasengate zusätzlich
> Abschnitt 7.
>
> **Die eine harte Regel** (`docs/bauaufteilung-sonden.md` §2): *Eine Behauptung
> ohne eingefügte Rohausgabe ist ein **gescheitertes** Ticket, kein fast
> fertiges.* Nichts zusammenfassen — stdout und stderr wortgleich einfügen.
> Ein Ticket ohne Manifest ist **offen**, auch wenn alles kompiliert.

| Feld | Wert |
|---|---|
| Ticket | `SONDE-0NN` |
| Phase / Session | P? / S? |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §?? — **wörtlich**, nicht meine Zusammenfassung |
| Commits | `…` |
| Datum | JJJJ-MM-TT |
| Prüfstufen | T1 ☐ · T2 ☐ · T3 ☐ (nur am Gate) |

---

## 1. Ticket-Behauptungen

Jede Zeile ist eine Behauptung aus dem **Gate-Text**, nicht aus meinem Kopf.

| # | Behauptung (Gate-Text) | Befehl | Ergebnis | Rohausgabe | Datum |
|---|---|---|---|---|---|
| 1 | | `…` | ☐ | [↓ B1](#b1) | |
| 2 | | `…` | ☐ | [↓ B2](#b2) | |

---

## 2. Rohe Ausgaben

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
