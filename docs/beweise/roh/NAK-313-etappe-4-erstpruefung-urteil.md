# NAK-313 — Erstprüfung Etappe 4, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Vorlage A)

**Thread:** 01a0d29e-b361-7362-9342-fa93d7a208fe · **HEAD vor und nach dem Lauf:** 64c2fa595fae4e3d848ae6eb9d2bf7877db3dac3 (Code gleich 8306681e; seit dd63c901 nur der Prüfauftrag unter docs/**) · **Lauf:** 24.09.2026, 10:53:30 bis 11:05:44 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak313-e4p1`) · **Prüfbereich:** git diff 69ecbe95...dd63c901 auf die 26 Ticketpfade der Etappe 4 (Prüffragen (a) bis (h); Auftrag unten) · **Auftrag:** docs/beweise/roh/NAK-313-etappe-4-erstpruefung-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Die letzte Nachricht trägt Urteilszeile, HEAD-Werte und FERTIG-Zeile, aber keine Absätze je Prüffrage (Form wie Beobachtung B-9, Manifest §15.3; viertes Mal). Die Prüfabdeckung ist an den 35 lesenden Kommandos des Laufs belegt, die der Dirigent aus dem JSONL des Laufs unten anfügt (gekürzt auf 220 Zeichen je Kommando).

---

URTEIL: PASS — kein DEFEKT im gebundenen Prüfbereich; ausschließlich lesend geprüft, vorhandene Testergebnisse nicht erneut ausgeführt. HEAD zu Beginn und Ende identisch: 64c2fa595fae4e3d848ae6eb9d2bf7877db3dac3. FERTIG Erstprüfung Etappe 4, PASS, 0 Defekte, 0 Lücken, 0 Härtungen

---

## Lesende Kommandos des Laufs (35, aus dem JSONL, gekürzt)

1. `"git rev-parse HEAD; git status --short; Get-Content -LiteralPath CLAUDE.md; Get-ChildItem -LiteralPath . -Force -Name AGENTS*; Get-Content -LiteralPath 'C:/Users/phili/Projekte/Nakama/.agents/skills/nakama-plugin-qualit`
2. `"Get-Content -LiteralPath AGENTS.md; git ls-files '*AGENTS*' '*CLAUDE.md'; git diff --stat 69ecbe957112c56f2aec70d77484659f6e7a065e...dd63c9017be0539ad570f42ff8f954fbca7fa691 -- broker/src/vertrag.rs broker/src/dto.rs br`
3. `'git diff 69ecbe957112c56f2aec70d77484659f6e7a065e...dd63c9017be0539ad570f42ff8f954fbca7fa691 -- broker/src/vertrag.rs broker/src/dto.rs broker/src/coordinator/schema.rs broker/src/coordinator/senke.rs'`
4. `'git diff 69ecbe957112c56f2aec70d77484659f6e7a065e...dd63c9017be0539ad570f42ff8f954fbca7fa691 -- broker/src/coordinator/befehl.rs broker/src/coordinator/evidenz.rs broker/src/coordinator/intent.rs broker/src/coordinator/`
5. `'git diff 69ecbe95...dd63c901 -- eq-copilot/plugin/vertrag/NakamaEvidenz.cpp eq-copilot/schemas/v3/README.md; git diff --unified=0 69ecbe95...dd63c901 -- tools/beweise.ps1'`
6. `'$p='"'docs/beweise/NAK-313.md'; "'$c=Get-Content -LiteralPath $p; foreach($r in @(@(102,233),@(474,559),@(878,946),@(1049,1088),@(1313,1509),@(1541,1563))) { for($i=$r[0];$i -le $r[1];$i++) { '"'{0}: {1}' -f "'$i,$c[$i-`
7. `'$c=Get-Content -LiteralPath docs/beweise/NAK-313.md; foreach($r in @(@(909,945),@(1049,1088),@(1563,1792))) { for($i=$r[0];$i -le $r[1];$i++) { '"'{0}: {1}' -f "'$i,$c[$i-1] }}'`
8. `'$c=Get-Content -LiteralPath docs/beweise/NAK-313.md; foreach($r in @(@(1079,1086),@(2003,2189))) { for($i=$r[0];$i -le $r[1];$i++) { '"'{0}: {1}' -f "'$i,$c[$i-1] }}'`
9. `'$c=Get-Content -LiteralPath docs/beweise/NAK-313.md; foreach($r in @(@(2970,3031),@(3095,3181),@(3529,3584),@(5557,5664),@(5737,5909))) { for($i=$r[0];$i -le [Math]::Min($r[1],$c.Length);$i++) { '"'{0}: {1}' -f "'$i,$c[`
10. `'git diff 69ecbe95...dd63c901 -- tools/eq-copilot/pruefe_v3_vertrag.py tools/eq-copilot/pruefe_v2_schemas.py; git diff 69ecbe95...dd63c901 -- eq-copilot/plugin/tests/SchemaTestMain.cpp eq-copilot/fixtures/v3/MANIFEST.jso`
11. `'git diff 69ecbe95...dd63c901 -- eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp eq-copilot/plugin/tests/PipeClientLifecycleTestMain.cpp eq-copilot/plugin/tests/Sonde013EventWireTest.cpp'`
12. `'git diff 69ecbe95...dd63c901 -- tools/eq-copilot/erzeuge_v3_fixtures.py | Select-Object -First 365'`
13. `'git diff 69ecbe95...dd63c901 -- tools/eq-copilot/erzeuge_v3_fixtures.py | Select-Object -Skip 365'`
14. `"git grep -n -E 'v3_nachricht_lesen_beliebig_mit_minor|json_streng|json_mit_minor_wert' -- broker/src; git grep -n -E 'uebernehmeSessionSnapshot|uebernehmeEvidenzruecknahme|evidenceSnapshotAlsJson' -- eq-copilot/plugin/s`
15. `'$c=Get-Content -LiteralPath eq-copilot/schemas/v3/README.md; for($i=1;$i -le $c.Length;$i++) { '"'{0}: {1}' -f "'$i,$c[$i-1] }; Get-Content -LiteralPath .gitattributes; git ls-files --eol eq-copilot/fixtures/v3/MANIFEST`
16. `'$p='"'eq-copilot/plugin/state/NakamaKanon.cpp'; "'$c=Get-Content -LiteralPath $p; for($i=155;$i -le 435;$i++) { '"'{0}:{1}: {2}' -f "'$p,$i,$c[$i-1] }; $p='"'broker/src/transport/bootstrap.rs'; "'$c=Get-Content -Literal`
17. `"git ls-files 'docs/beweise/roh/NAK-313-rot-M-*.txt' 'docs/beweise/roh/NAK-313-e4-*'; "'$c=Get-Content -LiteralPath docs/beweise/NAK-313.md; for($i=5665;$i -le 5736;$i++) { '"'{0}: {1}' -f "'$i,$c[$i-1] }; Get-Content -L`
18. `'foreach($n in 40..47) { $p='"'docs/beweise/roh/NAK-313-rot-M-{0}.txt' -f "'$n; '"'=== '+"'$p+'"' ==='; Select-String -LiteralPath "'$p -Pattern '"'"'^--- Haelfte'"','"'^Pruefung:'"','"'^  ->'"','"'^  Anmerkung:'"','"'^ `
19. `'foreach($n in 48..54) { $p='"'docs/beweise/roh/NAK-313-rot-M-{0}.txt' -f "'$n; '"'=== '+"'$p+'"' ==='; Select-String -LiteralPath "'$p -Pattern '"'"'^--- Haelfte'"','"'^Pruefung:'"','"'^  ->'"','"'^  Anmerkung:'"','"'^ `
20. `'$p='"'eq-copilot/plugin/src/SourcesModel.cpp'; "'$c=Get-Content -LiteralPath $p; foreach($r in @(@(710,782),@(1477,1547))) {for($i=$r[0];$i -le $r[1];$i++) { '"'{0}:{1}: {2}' -f "'$p,$i,$c[$i-1] }}; $p='"'eq-copilot/plu`
21. `"Get-Content -LiteralPath 'C:/Users/phili/.codex/skills/juce-best-practices/SKILL.md' -TotalCount 180; git grep -n -E 'evidenceSnapshotAlsJson|evidenzSnapshotSenden|workerLauf' -- eq-copilot/plugin/sonde/SondeProcessor.c`
22. `'foreach($n in @('"'S1','S2','K41','P','B3','K4','K65','W1','PY1','PY2','PY3','Z','G1')) { "'$p='"'docs/beweise/roh/NAK-313-e4-mutation-'+"'$n+'"'.txt'; '=== '+"'$p+'"' ==='; "'$c=Get-Content -LiteralPath $p; $ende=($c |`
23. `'foreach($n in @('"'S1','S2','K41','P','B3','K4','K65','W1','PY1','PY2','PY3','Z','G1')) { "'$p='"'docs/beweise/roh/NAK-313-e4-mutation-'+"'$n+'"'.txt'; '=== '+"'$n+'"' ==='; "'$c=Get-Content -LiteralPath $p; $marken=@($`
24. `'foreach($n in @('"'K41','P','B3','K4','K65','W1','PY1','PY2','PY3','Z','G1','G2','R1','R2','R3','R4','R5','R6','R7','E1','SCH','A1','A16','ID')) { "'$p='"'docs/beweise/roh/NAK-313-e4-mutation-'+"'$n+'"'.txt'; '=== '+"'$`
25. `'$t=Get-Content -LiteralPath eq-copilot/fixtures/v3/PRODUKTEINGAENGE-FAELLE.json -Raw | ConvertFrom-Json; '"'Kopf'; "'$t.anzahl; $t.anzahl_je_eingang; $t.anzahl_je_fassung; $t.anzahl_je_nachricht; '"'Gelesene Eintraege j`
26. `"Get-Content -LiteralPath docs/beweise/roh/NAK-313-6fdd827-dirty.md -TotalCount 45; Select-String -LiteralPath docs/beweise/roh/NAK-313-6fdd827-dirty.md -Pattern '70/70|GRUEN|Gesamt|A32|A33|\\| A1 \\||\\| A16 \\||\\| B1 `
27. `"git diff --name-only 8306681e dd63c901 -- broker eq-copilot/plugin eq-copilot/schemas/v3 eq-copilot/fixtures/v3 tools/eq-copilot/erzeuge_v3_fixtures.py tools/eq-copilot/pruefe_v3_vertrag.py tools/eq-copilot/pruefe_v2_sc`
28. `"Select-String -LiteralPath docs/offene-punkte.md -Pattern 'NAK-388|NAK-389|NAK-390|NAK-391|NAK-392' -Context 0,8 | ForEach-Object { "'$_.ToString() }; $p='"'broker/src/vertrag.rs'; "'$a=Get-Content -LiteralPath $p; 548.`
29. `"Select-String -LiteralPath docs/offene-punkte.md -Pattern '"'^'"\\| NAK-38[89] \\||"'^'"\\| NAK-39[0-2] \\|' | ForEach-Object { '{0}: {1}' -f "'$_.LineNumber,$_.Line }'`
30. `'$p='"'eq-copilot/plugin/tests/Sonde013EventWireTest.cpp'; "'$a=Get-Content -LiteralPath $p; 519..595 | ForEach-Object { '"'{0}:{1}' -f "'$_,$a[$_-1] }; $p='"'broker/src/coordinator/senke.rs'; "'$a=Get-Content -LiteralPa`
31. `"git ls-files '*SondeProcessor.cpp'; git grep -n -e 'workerLauf' -e 'evidenzSnapshotSenden' -- eq-copilot/plugin; "'$p='"'eq-copilot/schemas/v3/README.md'; "'$a=Get-Content -LiteralPath $p; 158..175 | ForEach-Object { '"`
32. `'$p='"'docs/beweise/NAK-313.md'; "'$a=Get-Content -LiteralPath $p; 1074..1087 | ForEach-Object { '"'{0}:{1}' -f "'$_,$a[$_-1] }; $p='"'broker/tests/sonde013_verdrahtung.rs'; "'$a=Get-Content -LiteralPath $p; 468..520 | F`
33. `'$p='"'docs/beweise/NAK-313.md'; "'$a=Get-Content -LiteralPath $p; 2149..2180 | ForEach-Object { '"'{0}:{1}' -f "'$_,$a[$_-1] }; 5820..5857 | ForEach-Object { '"'{0}:{1}' -f "'$_,$a[$_-1] }; git grep -n '"'evidence_angen`
34. `'$p='"'broker/tests/contract_cross_language.rs'; "'$a=Get-Content -LiteralPath $p; 125..150 | ForEach-Object { '"'{0}:{1}' -f "'$_,$a[$_-1] }; $p='"'eq-copilot/plugin/tests/SchemaTestMain.cpp'; "'$a=Get-Content -LiteralP`
35. `'git rev-parse HEAD; git status --short; git diff --name-only dd63c9017be0539ad570f42ff8f954fbca7fa691 HEAD; git diff --shortstat; git diff --cached --shortstat'`
