# M3-Kern-Befund — Graph-Diagnose mit konkreten Umsetzungsempfehlungen

Stand 2026-08-14, direkt nach M2. Auslöser: **USER-VORGABE** (Plan §0.3 neu):
„Ich will, dass das Plugin mir sagt, was an meinem Graphen schlecht ist — mit
konkreten Umsetzungsempfehlungen. Diese Kernfunktion verbessern bis zum
Umfallen; alles andere ist Rauschen. Kein Pflicht-Setup."
Alles hier Genannte wurde **ausgeführt und gesehen**.

## Was gebaut wurde

### Diagnose-Engine (`plugin/src/Diagnose.{h,cpp}`, `kDiagnoseVersion m3-2026-08-14`)

Reine Funktionen auf der Snapshot-Kopie — dieselbe `diagnose()` speist den
Hinweis-Knopf, die Snapshot-Datei und den Golden-Test. Fünf Befundklassen,
alle **eigenkurven-relativ** (Zone gegen die log-f-Linie ihrer eigenen
Schultern — kein kalibrierter Zielkorridor nötig, keiner behauptet):

| Klasse | Regel (Startwerte) | Erstidee im „Tu" |
|---|---|---|
| **Resonanz** | aus §5.10.3-Kandidaten; die 2 stärksten als Karten | dauerhaft → schmaler Cut im Fruity PEQ2 (−3 bis −{aus Überstand, max 6} dB) · zeitweise → Smooth Operator Pro (dynamisch) |
| **Mitten-Loch** (500–2000) | Zonenmittel ≥ 3 dB UNTER Schulterlinie (150–400 ↔ 2500–5000) + 60 % der Bänder unter halber Schwelle | **Balance statt EQ** (§5.10.6 breit+dauerhaft): Klavier/Chor am Fader nach vorn; erst danach breite +1…+2 dB um 800–1200 |
| **Mulm** (120–300) | ≥ 4 dB ÜBER Schulterlinie (50–100 ↔ 400–800) | maßvoll + schmal absenken oder dynamisch — Warum-Zeile schützt ausdrücklich das Klavier-Fundament/Sustain |
| **Härte** (2,5–5 k) | ≥ 4 dB ÜBER Schulterlinie (1–2 k ↔ 6–10 k) | dynamische Erstidee (Smooth Operator Pro), statisch nur klein |
| **Höhen-Hype** (ab 8 k) | Air-Mittel > Präsenz-Mittel + 1 dB (fast immer unnatürlich) | Shelf −1…−2 dB zurücknehmen — Stil braucht keinen Air-Hype |

Absicherungen: **Abdeckungs-Gates** — jede beteiligte Zone braucht ≥ 60 %
belastbare 1/3-Okt-Bänder (§5.10.2), deshalb keine Balance-Fehlalarme auf
Einzelquellen mit natürlichen Löchern (Sinus-Fixture beweist es). **60-%-
Bandkriterium** gegen Einzel-Peak-Verzerrung (Peaks gehören dem Resonanzweg).
**Konfidenz §5.10.7** aus vier Komponenten (Abdeckung, Messzeit, Zeitverlauf-
Beleg, Regelsicherheit): Zonen-Befunde erreichen nie „hoch" (Zeitmittel ohne
Frame-Persistenz — ehrlich gedeckelt), niedrig erreicht nie die Top-Drei.
Stil-Priorisierung: dauerhafte Resonanz → **Mitten-Loch (User-Hebel #1)** →
Mulm → Härte → Höhen-Hype → zeitweise Resonanz. Bewusst KEIN „Air fehlt"-
und kein Tilt-Befund (Orientierungs-Aussagen ohne konkreten Griff — lieber
fünf scharfe Regeln als acht halbgare; Kandidaten dokumentiert unten).

### Befundkarten (§7.3/§7.4)

Jede Karte trennt **Gemessen** (nur Zahlen) / **Wirkung** (Hypothese) /
**Tu** (konkreter Versuch, Startwerte als Bereiche in Klammern, Werkzeuge
des Users: Fruity PEQ2, Smooth Operator Pro) / **Warum** (musikalischer
Grund) / **Hören** (erwartetes Ergebnis + **Stop-Bedingung**, im Golden-Test
erzwungen) / **Sicherheit** (Konfidenz + begrenzende Komponente).

### UI: kein Setup-Gefühl

- Die Messpunkt-Bindung (Aufgabe/Name/Paar) ist aus dem Dauer-Kopf in ein
  **„Messpunkt…"-Popover** gewandert (mit dem Hinweis „Alles hier ist
  OPTIONAL — gemessen wird immer"). Der Graph gewinnt die Kopfhöhe.
- Der Hinweis-Knopf heißt jetzt nach der Diagnose („keine Auffälligkeit" /
  „n Auffälligkeiten") und öffnet die **Befundkarten** (scrollbar ab ~4
  Karten); „Beobachten"-Abschnitt für Nicht-Top-Befunde.
- Diagnose läuft 1×/s auf der Snapshot-Kopie — Anzeige bleibt 30 Hz.

### Snapshot v2

Die Snapshot-Datei trägt jetzt `befunde` (+ `diagnose_version`) — die
Empfehlungen sind außerhalb des Plugins lesbar (Claude-Beratung, spätere
Auswertung). Schema: `eq-snapshot.schema.json` v2; v1-Leser ignorieren das
Feld. Broker/Heartbeat (M2, Protokoll v2) sind unberührt.

## Headless bewiesen

| Prüfung | Werkzeug | Ergebnis |
|---|---|---|
| M1-Kreuzvalidierung unverändert (88 Prüfungen — Engine unberührt) | `EqCopGoldenTest` | ok |
| **Falsch-Positiv-Riegel:** Pink Noise ⇒ **null** Karten · Stille ⇒ null Karten | `EqCopGoldenTest` | ok |
| 3 konstruierte Fehler (Gauß-Glocken in log-f auf demselben Pink; Kern-SHAs + alte Referenzwerte per Skript als identisch verifiziert): Scoop ⇒ genau mitten_loch · +7 dB@180 ⇒ genau mulm · +6 dB@3,3k ⇒ genau haerte — jeweils ohne Fremdklassen | `EqCopGoldenTest` | ok |
| 116-Hz-Fall: Karte am Band-Center (±1/6 Okt), „dauerhaft", Konfidenz **hoch**, Top-Rang, Tu nennt Frequenz + konkreten Absenk-Startwert | `EqCopGoldenTest` | ok |
| Sinus: genau 1 Resonanz-Karte — Abdeckungs-Gates schlucken alle Zonen-Regeln | `EqCopGoldenTest` | ok |
| Karten-Struktur: alle 6 Felder gefüllt, Zahlen im Gemessen, Stop-Bedingung im Hören | `EqCopGoldenTest` | ok |
| Gesamt | `EqCopGoldenTest` | **GOLDEN OK 135/135** |
| Passthrough bitgleich, State stabil (mit Diagnose + Popover-UI) | `EqCopNullTest` | **NULLTEST OK** |
| VST3-Lebenszyklus, Editor, Fuzz | `pluginval --strictness-level 8` | **SUCCESS** |

## Bekannte Grenzen / nächste Verbesserungsstufen des Kerns

- Zonen-Befunde stützen sich aufs **Zeitmittel** + Abdeckung; Frame-genaue
  Zonen-Persistenz („durchgehend vs. nur im Refrain") ist der nächste
  Qualitätshebel (Histogramm-Daten existieren in der Engine bereits).
- **Masking** (§5.10.5, braucht PRE/POST- bzw. Mehr-Sensor-Evidenz über den
  Broker) und **passagen-getrennte Diagnose** (Strophe ≠ Drop) stehen an,
  sobald der Ein-Instanz-Kern gesättigt ist.
- Tilt-/Orientierungskurven-Aussagen bewusst ausgelassen (nur qualitativ
  erlaubt per §5.10.4 — kein konkreter Griff, also Rauschen im Sinne der
  Vorgabe). Kommt ggf. mit kalibriertem Korridor.
- Schwellen sind **Startkalibration** — der nächste ehrliche Schritt ist die
  Kalibration gegen echte Mixe des Users (ffff.wav, even-Bounces) statt nur
  synthetischer Fixtures.

## Nur in FL prüfbar (User)

Installierte Kopie ersetzen: `eq-copilot/install/EQ-Copilot.vst3` →
`C:\Program Files\Common Files\VST3\` (UAC). SHA-256 der DLL:
`B597477AEF231091799CEC238F4D85C1F71EE5F726F713BBE1ACD1058274F89E`
(Stand nach Selbst-Audit-Fixes: `dauerhaft` als Befund-Feld statt
Titel-String-Kopplung; Messpunkt-Popover löst ohne echte Änderung keinen
Reconnect mehr aus).
Danach FL-TESTANLEITUNG §16: Song abspielen → Hinweis-Knopf → Befundkarten
lesen; Gegenprobe mit dem 116-Hz-Trick aus §8.
