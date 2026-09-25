# NAK-380 — Matrixprüfung 3, Rohurteil (Codex gpt-6-astra, Effort high nach NAK-396, lesend; Vorlage B, Runde 2 von 3)

**Thread:** 01a0d722-31ec-7b72-9a03-b146f7b9a36e · **HEAD vor und nach dem Lauf:** 55f768c3d7fcb5f36bae9d8c3cb3261a68c8ad64 (Prüfbereich `git diff 7b6f58d2...ac3e9c12 -- docs/beweise/NAK-380.md`; darüber nur §17 und der Prüfauftrag) · **Lauf:** 25.09.2026, 07:55:37 bis 08:02:56 Uhr (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak380-m3`, EXIT=0) · **Auftrag:** `docs/beweise/roh/NAK-380-matrixpruefung-3-auftrag.txt` · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 32 lesende Kommandos aus. Die im Auftrag verlangte FERTIG-Zeile und die Stichprobenliste des Wachenbelegs fehlen im Bericht; das Urteil steht in der ersten Zeile (NEEDS_WORK, B1 bis B4 geschlossen, zwei Brüche).

---
NEEDS_WORK: B1 bis B4 sind geschlossen, aber die neuen Wachen- und Golden-Belege enthalten zwei Brüche. Die Prüfung erfolgte ausschließlich lesend und rechnerisch, ohne Testausführung. HEAD war zu Beginn und Ende identisch: 55f768c3d7fcb5f36bae9d8c3cb3261a68c8ad64.

Full review comments:

- [P2] Trenne M-16(d) vom grünen Wachenbeleg — C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-380.md:2758-2758
  Für M-16(d) belegt Endlichkeit nicht die zugesagte Bandbreitenquote: Bei einem Fenster mit allen 221 P50-Werten auf −300 dB und Befundintervall [10,14) liefert `zusammenhang.rs:190–211` am ZIEL ungewichtet 4/221 = 0,01809955; zugesagt ist dagegen der Gitterbreitenanteil 4,96292588/17929,03239979 = 0,0002768095. Der neue Beleg erklärt somit eine nachweislich rote Teilzusage für grün und verletzt die Etikettendefinition (§6) sowie §15.3. Die Breitenquote muss als rote Hälfte getrennt werden; nur die bereits erfüllten Endlichkeits- und Leerfälle dürfen Wachen bleiben.

- [P2] Erfasse den Altzähler mit dem spezifizierten W1-Signal — C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-380.md:2812-2812
  Die hier als feste W1-Ausgangsreferenz bezeichneten 19 Ereignisse stammen aus einem anderen Signal: `gegenpruefung-statistik/b11_ursache.py:11` verwendet `numpy.random.default_rng(5)`, während §7.1/§7.2 für W1 SplitMix64 mit Box-Muller und Saat `0x3800003` festlegen. Das Log belegt daher keinen alten Referenzinhalt für den vorgeschriebenen W1-Golden; ein Vergleich gegen 19 könnte bereits wegen des Signalwechsels statt wegen des Detektorumbaus rot werden. Nach E-380-18 muss zuerst der tatsächliche W1-Zähler mit unverändertem Code erfasst werden; die 19 dürfen nur als Ergebnis des anderen Nachbaus stehen bleiben.
