# NEXT-SESSION — Einstieg für die nächste Runde

> ## ✅ G3-SOAK PHASE 2 FERTIG — 02.09.2026, Bau-Worker
>
> **Das Werkzeug für das Phasengate G3 steht.** Neues Kanon-Bein **A24**
> (`tools/eq-copilot/pruefe_session_soak.py` + `EqCopSessionSoak` aus
> `eq-copilot/plugin/tests/SessionSoakMain.cpp`), registriert in
> `tools/beweise.ps1` samt Bauziel. Der Probe-Broker blieb unverändert.
> **Kanon GRUEN 40/40** (vorher 39/39), A24 in 146 s.
>
> Manifest: `docs/beweise/G3-SOAK.md` — Verhaltensmatrix S01–S16, Killmatrix
> K-S1…K-S5, gefahrene Läufe (§13), sechs Rotmutationen (§14). **Ohne
> Urteilsmarke**: die setzt der Dirigent nach der Codex-Prüfung.
>
> **Was als Nächstes fällig ist (§11 des Manifests):**
> 1. Codex-Prüfung des Phase-2-Diffs (`a570367`…`a390d75`).
> 2. Der **Gate-Lauf G3 selbst** — er ist nicht Teil dieses Werkzeugtickets:
>    60 min mit 16 Sonden, 30 min mit 32, dazu je 2 min mit 1, 4 und 8 Sonden
>    (Skalierungsreihe S16, Entwurf `:4523`). Aufruf:
>    `py -3.13 tools/eq-copilot/pruefe_session_soak.py --sonden 16 --minuten 60
>    --neustarts 3`. Der Beleg gehört ins G3-Gate-Manifest, nicht nach
>    G3-SOAK.md.
> 3. **XRun bleibt FL-Termin** (§2.2): `SondeProcessor::v3PipeName` ist `const`
>    auf dem Produktionsnamen, deshalb misst A24 den Audiopfad unter
>    `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` und die Kopplung „Drahtrückstau bremst
>    den eigenen Analyseworker" wird nicht gefahren.
>
> **Betriebsnotiz für den nächsten Bauer:** Der echte Coordinator verlangt im
> `heartbeat` ein `adresse`-Feld und prüft es gegen die Wire-Adresse des Links
> (`broker/src/coordinator.rs:2944-2955`). Ein Testheartbeat ohne Adresse wird
> **nie** beantwortet — A22 kommt damit nur durch, weil sein Probe-Broker die
> zählende Standardsenke aus `broker/src/transport/server_v3.rs:297` nutzt.
> Und: der produktive `ControlClient` sendet bei gesetztem `statusProvider`
> eigene Heartbeats ab Sequenz 0 (`ControlClient.cpp:1386`); ein Testzähler im
> selben Zahlenraum ordnet fremde ACKs zu.


> ## ✅ NACHTRAG — 01.09.2026 spät, Fable-Sitzung e0375c12 (User-Gespräch, kein Dirigentenlauf): G2 NEEDS_WORK, Zwischenhalt PR2 eingeplant, Opus baut wieder
>
> **G2 steht auf T3 NEEDS_WORK 2026-09-01** (`docs/beweise/G2.md`): tragender
> Defekt Broker-Impersonation, Register NAK-123. Die Nacharbeit lag fünf
> Stunden als uncommitteter Codex-Bau im Worktree (31 Dateien, nie kompiliert,
> Tests `NOT RUN`) und ist als benannter Zwischenstand committet (`c197fcd`),
> damit der nächste Opus-Worker darauf aufbaut. `cargo check --tests` war grün,
> die C++-Seite ist ungeprüft. Details: Nachtrag am Ende von
> `docs/beweise/NAK-123.md`.
>
> **User-Entscheid 01.09. spät, wörtlich:** „opus wieder bauer und codex
> prüfer . so wie am anfang, habe genug wochen kontigent". Skill §3.6 ist
> umgestellt, Codex-Bau ist nur noch Fallback ab 85 % Claude-Woche; Skill §3.1
> trägt die neue Regel für liegengebliebene Ticketstände.
>
> **Neuer Planschritt PR2** (`docs/plan/plan.json`, Phase P3 nach S18–19, vor
> G3): Entwurf 0.4 plus Errata zu 0.5, Rangfolge Entwurf/Blueprint,
> Fünferblock-Entscheide und Blueprint §15 in S26–28 bis S31b, S31b weg von
> Figma und Rework-Bildern, Register-Triage, Prozessbilanz. Der Anlass steht
> als User-Wortlaut im Schritt.
>
> **Als Nächstes (Planstand):** Nacharbeit an G2 → G2 PASS → PR2 → G3. NAK-121
> (Modul-Aufteilung der Broker-Monolithen) bekommt in der PR2-Triage seinen
> Ort im Plan.
>
> Der Dirigent wurde aus dieser Sitzung über `tools/dirigent/start-dirigent.ps1`
> neu gestartet.

> ## ✅ NACHTRAG — 01.09.2026, Fable-Sitzung 6c9e2d4e (Dirigent): SONDE-011 ABGESCHLOSSEN
>
> **S16–17 `SONDE-011` trägt T2 PASS 2026-09-01** (Manifest-Kopf mit
> Rundenübersicht: `docs/beweise/SONDE-011.md`). Phase B: Erstprüfung 19
> Defekte → Nacharbeit 1 (16 zu) → Runde 2 (2 Folgedefekte) → Runde 3
> (1 Folgedefekt, Budget erreicht) → Konvergenzentscheid (OR-Merge
> `beitritt_bestaetigung_noetig`, B.9/L-13) → Wiederprüfung **PASS** auf
> `f176f24`. Kanon GRÜN 33/33 beglaubigt
> (`docs/beweise/roh/SONDE-011-f176f24-dirty.md`). Bauer durchgehend
> Codex-Thread `01a0590d` (Sol max); der frühere unterbrochene
> Worktree-Zwischenstand ist committet (`901598f`…`f176f24`).
>
> **Als Nächstes:** G2-Gate (volles Programm: C++-, Rust- und
> Sicherheits-Review + Codex). Danach NAK-121 (Modul-Aufteilung der
> Broker-Monolithen, verhaltensneutral, `docs/offene-punkte.md`).
>
> Der folgende Block vom 31.08. ist damit Verlauf.

> ## NACHTRAG — 31.08.2026 spät, Fable-Sitzung 3233a71f (Dirigent), geordneter Terminalwechsel
>
> **User-Wort 31.08. spät:** „mach pause codex kann nicht löschen sonst" ·
> „ich mache das terminal zu und danach ein frisches auf"
>
> ### Stand SONDE-011 Phase B (S16–17)
>
> - **Matrix:** committet und TRAGFÄHIG geprüft (`d172b0a`).
> - **Bau:** drei Teile committet und gepusht — `512790a` (Rust-Kern),
>   `d208ad8` (C++-Seite), `70301d8` (23 Testlücken geschlossen).
> - **Kanon:** GRÜN 33/33 beglaubigt auf `70301d8` (`c8fc3c2`; Roh:
>   `docs/beweise/roh/SONDE-011-70301d8-dirty-3.md`). Ein Zeilenenden-
>   Artefakt der flatc-Driftwache ist dort dokumentiert und gelöst.
> - **Erstprüfung:** **NEEDS_WORK, 19 Defekte** — Urteil, Ursache und
>   Befundliste stehen im Manifest (`docs/beweise/SONDE-011.md`, Abschnitt
>   „Phase B — Erstprüfung") und wörtlich in
>   `docs/beweise/roh/SONDE-011-erstpruefung-c8fc3c2.md`.
> - **Nacharbeit Runde 1 LÄUFT, ist aber unterbrochen:** Codex-Bauer-Thread
>   `01a0590d-e354-7720-a255-e578b936aca6` (Sol max, workspace-write). Sein
>   **uncommitteter Zwischenstand liegt im Worktree** (broker/Cargo.toml,
>   coordinator.rs, lib.rs, server.rs, store.rs, server_v3.rs, zwei
>   Testdateien, ControlClient.cpp/.h) — NICHT verwerfen, NICHT committen;
>   der Thread arbeitet darauf weiter.
>
> ### Nächster Schritt der neuen Dirigenten-Session
>
> 1. `git status` — der oben genannte Zwischenstand ist erwartet; fremde
>    design/-Dateien der Parallel-Session nicht anfassen.
> 2. Codex-Thread resumen (Auftrag: Ist-Zustand prüfen, alle 19 Defekte
>    schließen, Tests fahren, Manifestabschnitt „Phase B — Nacharbeit
>    Runde 1" anhängen, nichts committen):
>    `codex -a never exec -m gpt-5.6-sol -c 'model_reasoning_effort="max"' -C . -s workspace-write resume 01a0590d-e354-7720-a255-e578b936aca6 --json -o <last> -`
>    (User-Config-Sandbox nutzen, KEIN `--ignore-user-config` — der
>    Auto-Classifier blockt das explizite elevated-Flag; die User-Config
>    trägt es bereits.)
> 3. Danach Wiederprüfung nach Vorlage B (nur Fixdiff + Befundliste),
>    betroffene Beine, bei PASS Kanon + Abschluss.
>
> ### Betriebsnotizen dieses Tages
>
> - Sol-Capacity-Abbrüche zweimal; Resume desselben Threads funktioniert
>   verlustfrei (Worktree + Thread-Kontext bleiben).
> - Lange Einzel-Threads vermeiden (3-h-Session wurde müde, 30
>   Kompaktierungen): Restarbeit in enge Einzelaufträge schneiden —
>   User-Wort dazu am Nachmittag.
> - Externe Codex-Datenbereinigung (`C:\Users\phili\.codex\.tmp`, App seit
>   28.08., Platte war bei 95 %) killt laufende CLI-Läufe; vor Neustart
>   prüfen, ob die Bereinigung fertig ist.
> - Registerpunkt noch OFFEN einzutragen (User-Gespräch 31.08.): Modul-
>   Aufteilung zu großer Dateien (server_v3.rs 4100, coordinator.rs ~2000+,
>   store.rs ~1600 Zeilen) als verhaltensneutraler Schritt direkt nach dem
>   G2-Gate — datiert nach docs/offene-punkte.md, sobald das Schreibfenster
>   frei ist.

---

Der frühere Inhalt dieses Blatts (Stand 30.08., S9/S9b/S14–15, G1) ist durch
`docs/PLAN-STAND.md` und die Manifeste überholt; Verlauf steht im Git.
