Die fünf Altbefunde sind in den verlangten Einzelmechanismen formal geschlossen, der Fixdiff lässt jedoch die drei genannten Nachweislücken beziehungsweise Überbehauptungen bestehen. Geprüft wurden ausschließlich der gebundene Diff und die benannten Quellen; Builds und Laufzeitprozesse wurden auftragsgemäß nicht ausgeführt. HEAD war zu Beginn und Ende `4a831da3d62e3df3b17caad450503e71781c0e2a`.

Full review comments:

- [P2] Prüfe die tatsächliche Ausgabe der Startmeldung — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-310.md:941-941
  Wenn `main.rs` die neue Formatierungsfunktion zwar aufruft, ihr Ergebnis aber verwirft und weiterhin die alte fest verdrahtete v2-Meldung ausgibt, bestehen sowohl `startmeldung_nennt_nur_bediente_endpunkte` als auch die ausdrücklich nur auf den Funktionsaufruf gerichtete Quelltextwache. Damit bleibt die in M-36/R-310-3 zugesagte ehrliche Startmeldung ungemessen; die Ausgabenaht muss den tatsächlich ausgegebenen String prüfen.

- [P2] Prüfe die Übergabe der geparsten Sitzung an den Broker — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-310.md:1015-1015
  Wenn `main.rs` `aufruf_lesen` aufruft und dessen Exitcode übernimmt, die zurückgegebene Sitzungskennung aber nicht an den Brokerstart weitergibt, bleiben `aufruf_lesen_rand`, die direkten Fassadentests und die ausdrücklich nur den Aufruf prüfende Quelltextwache grün. Der ausführbare Broker erhält dann die von M-94 gelieferte Kennung nicht und M-95/R-310-7 ist im Produktpfad verletzt; diese Verdrahtung braucht eine beobachtbare Testnaht.

- [P2] Decke die behauptete 500-ms-Schließung Ende zu Ende ab — C:\Users\phili\Projekte\Nakama\docs\beweise\NAK-310.md:1023-1023
  Wenn beim Sitzungsende bereits `Kern::takt` läuft und erst 600–1400 ms später zurückkehrt, bleiben M-113(a) (nur Konstantenwert), M-113(b) (kein laufender Takt, 2-s-Testfrist) und M-97 (1,5-s-Toleranz) grün. Der neue A4-Satz behauptet dennoch universell `schliesst ... binnen der Schliessfrist von 500 ms`; damit ist M-109 größer als seine Messung und widerspricht R-310-9 sowie §7.4:1892-1895, wo ein laufender Takt ausdrücklich hinzugerechnet wird.