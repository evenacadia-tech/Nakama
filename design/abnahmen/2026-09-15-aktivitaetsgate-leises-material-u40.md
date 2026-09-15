# 2026-09-15 — Der Advisor spricht nach der Korrektur des Aktivitätsgates auch auf leisem Material (Karte U40)

**Frage (Dirigent, 15.09.2026, 14:03 Uhr, in der Dirigentensession als Text;
`AskUserQuestion` ist im dontAsk-Modus gesperrt):** Das Gate, das entscheidet,
ob eine Stelle als „gemessen" gilt, verglich vor der Korrektur (zweiter
Codeaudit 12.09.2026, Befund F06; NAK-283 Etappe 5) eine falsche Einheit, die
Grenze wanderte mit der Samplerate um bis zu 13,7 dB. Nach der Korrektur gelten
leise Stellen (Intro, Ausklang, ruhige Passage) als gemessen, und der Advisor
schlägt dort vor, wo er vorher schwieg. Vorgelegt wurden die Messwerte aus dem
echten FL Studio (Laufzeit-Arm NAK-286, Szenario `u40-aktivitaetsgate`,
Diagnoseprojekt mit dem Testtrack; `../../docs/beweise/NAK-286.md` §43.8,
Rohdaten `../../docs/beweise/roh/NAK-286-laufzeit-7e39db9a.md` und
`NAK-286-laufzeit-d6382571.md`): an allen drei Stellen — Intro 0 bis 3,7 s
(leise), Passage 3,7 bis 11 s (laut), Ausklang 42,5 bis 45,6 s (leise) — zählten
Gen und Probeeq jedes Messfenster als aktiv (Intro 59 von 59 und 40 von 40,
Passage 86 von 86 und 126 von 126, Ausklang 40 von 40 und 46 von 46),
Kurzzeitlautheit etwa −24 bis −25 LUFS, eine Spitze im Ausklang bei −36 dB. An
keiner Stelle schwieg das Gate. Sehr leises Material nahe der Stille war im
Testtrack nicht enthalten und ist nicht gemessen. Angeboten: „1. So lassen
(empfohlen)" — leise Stellen gelten als gemessen, der Advisor spricht dort;
gilt auch ohne Entscheidung — „2. Eigene Sichtbarkeitsgrenze für sehr leises
Material" — unterhalb schweigt der Advisor, die Grenze steht sichtbar in der
Oberfläche — oder „3. Erst hören, dann entscheiden" — die Grenze kommt als
eigene Karte nach dem Hören an eigenem Material.

**User-Wort, 15.09.2026, 14:06 Uhr** (wörtlich):

> „1"

## Was damit festliegt

- **Weg 1 gilt:** die Korrektur des Aktivitätsgates (NAK-283 Etappe 5, F06)
  bleibt ohne zusätzliche Sichtbarkeitsgrenze. Leise Stellen gelten als
  gemessen; der Advisor spricht dort genauso wie auf lautem Material.
- Keine neue Produktregel, keine sichtbare Grenze in der Oberfläche, keine
  Folgekarte. Am Bau ändert sich nichts; keine Phase verschiebt sich.
- Die Messwerte, an denen entschieden wurde, liegen in NAK-286 (§43.8) und
  gelten für den Testtrack; sehr leises Material nahe der Stille bleibt
  ungemessen. Zeigt das Hören an eigenem Material später etwas anderes,
  entsteht eine neue Karte, nicht eine stille Änderung.
- Karte U40 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-284
  (Antwort auf U40: Messung durch den Dirigenten statt Blindwahl) war mit der
  Abnahme von S25e bereits geschlossen.
