# NAK-313 — Matrixprüfung 2, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Wiederprüfung nach Matrixnacharbeit 1)

**Thread:** 01a0d07d-c2a3-7451-9d02-2c2647170abf · **HEAD vor und nach dem Lauf:** 25e418497f1876a9e174317eb047ace99ffdcc11 · **Lauf:** 24.09.2026, 00:58:17 bis 01:12:16 (+02:00, aus der Startdatei des Laufs) · **Prüfbereich:** git diff 9a1ae133...35c84a16 -- docs/beweise/NAK-313.md (Fixdiff der Matrixnacharbeit 1, Befundliste D1 bis D14) · **Auftrag:** docs/beweise/roh/NAK-313-matrixpruefung-2-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Die zwei Befunde heißen in der Validierung W1 (D12 offen, M-72) und W2 (M-101 gebrochen).

---

NEEDS_WORK: D12 offen, zusätzlich M-101 gebrochen; die übrigen dreizehn Befunde geschlossen, keine zusätzlichen Lücken oder Härtungen erhoben. Ausschließlich statisch geprüft, keine Testläufe; der angeforderte Produktdiff ist leer. HEAD Beginn: 25e418497f1876a9e174317eb047ace99ffdcc11; HEAD Ende: 25e418497f1876a9e174317eb047ace99ffdcc11.

Full review comments:

- [P2] Ordne dem Bootstrap-Vektor die erreichbare Produktstufe zu — C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-313.md:1503-1504
  Für das in M-72 festgelegte Hello mit `protocol: 3.5` bleibt D12 offen: Der Ganzzahlhelfer aus §8.5 liefert `None`, sodass die Protokollwahl in `broker/src/transport/bootstrap.rs:242–249` mit `KeinHello` endet. §8.6 belässt diese Auswahl vor der Schemaprüfung; nach §7.2 ist dieser Fehler daher `feldregel`, nicht `schema`. Die Verschiebung nach Etappe 6 macht die hier geforderte beidseitige Schemastufe nicht erreichbar und lässt den geplanten A4-Tabellenfall trotz plangemäßer Implementierung scheitern. Weise die Stufenabweichung gemäß [R-313-13](docs/beweise/NAK-313.md#L3034-L3042) aus und gleiche M-72 sowie §8.6 daran an.

- [P2] Benenne wirksame Einzelbrüche für M-101(c/d) — C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-313.md:1186-1186
  Die neue Aussage, die Regressionswachen für `1e-300` und `48000` würden von denselben Brüchen getragen, trifft nicht zu: `dez <= -307` lässt `1e-300` weiterhin passieren; `48000` passiert bereits den Ganzzahlzweig (`broker/src/vertrag.rs:242–248,270–273`). Auch das Überspringen von Textriegel oder Schema sowie das Entfernen der C++-Obergrenze ändern die Annahme dieser beiden gültigen Werte nicht. Andere Teilfälle werden dadurch rot, aber keine der beiden behaupteten Beobachtungen. Für diese Zusagehälften fehlen deshalb wirksame Einzelbrüche nach [R-313-11/12](docs/beweise/NAK-313.md#L3022-L3033).