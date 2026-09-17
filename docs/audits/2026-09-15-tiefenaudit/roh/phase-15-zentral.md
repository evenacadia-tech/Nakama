# Phase 15 — zentraler Bericht: Verfahren, Quellprüfung, Nachrechnung, Zähler

Datum **17.09.2026**. Quellstand **`e008811efa7650f3e0cc2332cc21f9e268cdd125`** (HEAD vor und nach dem Lauf gleich; getrackter Diff ausschließlich unter `docs/audits/2026-09-15-tiefenaudit/`). Rein lesend an Produktcode, Tests, Verträgen und vorhandenen Beweisen.

**Auftrag (User, 17.09.2026):** ein weiterer Tiefenaudit zu Verlässlichkeit und funktionaler Qualität in drei Paketen — (1) verspätete Ergebnisse nach Zustandswechseln, (2) DSP-Numerik und dynamische Übergänge innerhalb der tatsächlich zulässigen Parameterbereiche, (3) Erholung nach Teilfehlern im normalen Betrieb. Drei unabhängige Agenten, Prüfung ihrer Ergebnisse durch den Hauptagenten an der Quelle, danach ein gezielter Widerlegungsversuch jedes aufzunehmenden Befunds durch einen anderen Prüfer. Bekannte Befunde zählen nicht erneut; ausdrückliche Sicherheitsbereiche bleiben ausgelassen; geparktes Design bleibt geparkt, die technische Korrektheit vorhandener Bedienhandlungen gehört dazu. Keine Produktreparatur, kein Bau, kein Test, keine Installation, keine Hostaktion.

**Ergebnis:** 16 Kandidaten, **15 aufgenommen** (T3-15-01 bis T3-15-15: 6 DEFEKTE, 5 LÜCKEN, 4 HÄRTUNGEN; 2 high, 6 medium, 7 low), 1 Duplikat nicht aufgenommen, 19 offene Prüffelder ohne Befund-ID, 53 verworfene Verdachte. Der Kopf mit allen Zeilen steht in [../BEFUNDE.md](../BEFUNDE.md), Abschnitt „Phase 15“.

## 1. Rollen und Ablauf

| Rolle | Umfang | Bericht |
| --- | --- | --- |
| Paketprüfer 1 | verspätete Ergebnisse nach Zustandswechseln (Gen, Probeeq, Coordinator) | [phase-15-paket1-verspaetete-ergebnisse.md](phase-15-paket1-verspaetete-ergebnisse.md) |
| Paketprüfer 2 | DSP-Numerik und dynamische Übergänge, Bewertung der Referenztests | [phase-15-paket2-dsp-numerik.md](phase-15-paket2-dsp-numerik.md) |
| Paketprüfer 3 | Erholung nach Teilfehlern (Store, Routing, Intervention, Duplikate, Replay) | [phase-15-paket3-teilfehler.md](phase-15-paket3-teilfehler.md) |
| Zentrale Quellprüfung | Nachlesen der Kernstellen jedes Kandidaten, unabhängige Nachrechnung der DSP-Zahlen, Einordnung, ID-Vergabe | dieser Bericht, Abschnitte 3 und 4 |
| Gegenprüfung 1 bis 3 | je ein anderer Prüfer als der Finder, Auftrag ausschließlich Widerlegung | [phase-15-gegenpruefung-paket1.md](phase-15-gegenpruefung-paket1.md), [phase-15-gegenpruefung-paket2.md](phase-15-gegenpruefung-paket2.md), [phase-15-gegenpruefung-paket3.md](phase-15-gegenpruefung-paket3.md) |

Die drei Pakete arbeiteten disjunkt und ohne Kenntnis voneinander; die Gegenprüfer bekamen die Kandidaten als unbewiesene Behauptungen samt Quellstellen und Zusagenbezug, nicht als Befund. Alle sechs Agenten arbeiteten nur lesend; Schreibrecht hatten sie ausschließlich für ihren eigenen Bericht außerhalb des Repos. Sicherheitsbereiche (Pipe-ACLs, Impersonation, Token, Signaturen, Diagnoseberechtigung, Installer-Privilegien) waren in jedem Auftrag ausgeschlossen; die bestehenden Sicherheitsbefunde T3-12-* wurden weder geprüft noch verwendet.

## 2. ID-Zuordnung und Urteile

| Kandidat | Befund-ID | Kategorie · Schwere | Urteil der Gegenprüfung |
| --- | --- | --- | --- |
| P15-V-01 | T3-15-01 | DEFEKT · medium | BESTÄTIGT, Zusage neu verankert (NAK-246 D4 mit Entwurf §57, CLAUDE.md), Fenster größer als vorgelegt |
| P15-V-02 | T3-15-02 | DEFEKT · medium | BESTÄTIGT, Umfang weiter (kein Produktweg erreicht den Vergleich), Wirkung enger |
| P15-V-03 | T3-15-03 | LÜCKE · low | HERABGESTUFT von DEFEKT/medium |
| P15-V-04 | T3-15-04 | HÄRTUNG · low | BESTÄTIGT als HÄRTUNG |
| P15-D-01 | T3-15-05 | DEFEKT · high | BESTÄTIGT (M-03/E-31 statt M-07); Schwere vom Dirigenten auf high gesetzt |
| P15-D-02 | T3-15-06 | LÜCKE · low | HERABGESTUFT, DEFEKT-Lesart und M-19-Teilbehauptung widerlegt |
| P15-D-03 | T3-15-07 | LÜCKE · medium | BESTÄTIGT, Beweislage verschärft |
| P15-D-04 | T3-15-08 | LÜCKE · medium | BESTÄTIGT, auf tieffrequente hochgütige Bänder eingegrenzt |
| P15-D-05 | T3-15-09 | LÜCKE · low | BESTÄTIGT, Kern auf falsch hergeleitete Vertragsgrenze erweitert |
| P15-D-06 | T3-15-10 | HÄRTUNG · low | HERABGESTUFT von LÜCKE |
| P15-D-07 | T3-15-11 | HÄRTUNG · low | HERABGESTUFT, Arithmetik bestätigt |
| P15-T-01 | T3-15-12 | DEFEKT · high | BESTÄTIGT, zusätzlich Anzeigeweg belegt |
| P15-T-02 | T3-15-13 | HÄRTUNG · low | HERABGESTUFT, auf die unehrliche Anzeige verengt |
| P15-T-03 | — | nicht aufgenommen | **DUPLIKAT** zu NAK-188; Rest deckungsgleich mit T3-14-03 |
| P15-T-04 | T3-15-14 | DEFEKT · medium | BESTÄTIGT |
| P15-T-05 | T3-15-15 | DEFEKT · medium | BESTÄTIGT |

**Bestätigt 10 · herabgestuft 5 · Duplikat 1 · widerlegt 0 · unentschieden 0.** Kein Kandidat hat die Widerlegung nur knapp überlebt, ohne dass der Grund in seiner Zeile steht: die Gegenargumente, die die Herabstufungen tragen, stehen in den drei Gegenprüfungsberichten vollständig.

**Verwertung des Duplikats P15-T-03:** Register **NAK-188** (`docs/offene-punkte.md:212`) führt die Wirkung wörtlich („Ein Link ohne auffindbare Sitzung sperrt alle Sitzungen — heute ohne Produktlöser“), nimmt aber an, sie löse sich mit NAK-180. Diese Annahme ist für zwei Kanten falsch: die Reihenfolge-Race zwischen v2-Abbruch und dem ersten Heartbeat des neuen v3-Links und Gens Destruktor, in dem `controlV3.stop()` vor `pipe.stop()` läuft. Empfehlung an den Dirigenten: NAK-188 um diese v2-Kante und um die Korrektur der Löserannahme ergänzen, statt eine neue Zeile zu führen. Der Löser selbst ist im Produktweg vorhanden (`broker/src/coordinator/befehl.rs:732` → `broker/src/coordinator/intervention.rs:432`, zweiter Weg `broker/src/coordinator/link.rs:470`), und jeder produktive v2-Abbruch ist im selben Rumpf mit einem v3-Reconnect gepaart (`eq-copilot/plugin/src/prozessor/State.cpp:64`, `eq-copilot/plugin/src/prozessor/State.cpp:218`).

## 3. Zentrale Quellprüfung

Selbst gelesen und damit bestätigt (Auswahl der tragenden Stellen je Kandidat; die vollständigen Ketten stehen in den Paketberichten):

| Kandidat | Zentral nachgelesen | Was die Lektüre bestätigt |
| --- | --- | --- |
| T3-15-01 | `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:824-844`; `eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:300-320`; `eq-copilot/plugin/src/prozessor/Ipc.cpp:1060-1080`; `eq-copilot/plugin/src/prozessor/Ipc.cpp:1330-1356`; `eq-copilot/plugin/core/ipc/ControlClient.cpp:145-165`; `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:138-152`; `docs/beweise/NAK-246.md:69`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:4283-4287` | `inFlightAck` steht vor der Generationsprüfung; `v3Antwort` ist der einzige Abbau der Zuordnung; ein zweiter Befehl derselben Quelle wird abgewiesen; `reconnect()` erhöht die Generation vor `ioAbbrechen`; `prepareToPlay` ruft ihn bedingungslos; D4 und das Exit-Gate §57 samt „Entfernen hebt Binding sichtbar auf“ tragen den Zusagenbezug |
| T3-15-02 | `broker/src/coordinator/link.rs:238-265`; `broker/src/coordinator/liveness.rs:420-480` | `descriptor: None` bei jedem Hello mit ausdrücklicher Begründung; `messpunktwechsel` entsteht nur bei zwei vorhandenen Deskriptoren, danach wird der Deskriptor vollständig ersetzt |
| T3-15-03 | `broker/src/coordinator/intent.rs:500-520` | Vollbericht mit kleinerer Revision wird als `AeltereRevision` verworfen, ohne Generation oder Merker |
| T3-15-04 | `eq-copilot/plugin/src/SourcesModel.cpp:555-585`; `eq-copilot/plugin/src/SourcesModel.cpp:1233-1245`; `eq-copilot/plugin/src/SourcesModel.cpp:1350-1362`; `eq-copilot/plugin/src/SourcesModel.cpp:1637-1641` | `controlEnde` setzt alles stale und trägt die NAK-214-R3-Zusage im Kommentar; die Übernahme prüft nur Bindung und Sitzung und setzt danach `subscriptionAktiv`, Diagnose und `mainDarfSchreiben` wieder scharf |
| T3-15-05 bis T3-15-11 | `eq-copilot/plugin/dsp/DspKern.cpp:1-860`; `eq-copilot/plugin/dsp/DspKern.h:270-390`; `eq-copilot/plugin/dsp/DspProgramm.cpp:1-272`; `eq-copilot/plugin/dsp/DspProgramm.h:60-77`; `eq-copilot/plugin/dsp/DspFilter.h:20-369`; `eq-copilot/plugin/sonde/SondeProcessor.cpp:300-475`; `eq-copilot/plugin/sonde/SondeProcessor.cpp:1380-1444`; `eq-copilot/plugin/state/NakamaTransaktion.cpp:785-800`; `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1040-1120`; `eq-copilot/schemas/state/nakama-parameter-v2.json`; `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2318-2332`; `docs/beweise/SONDE-015.md:1036`, `:1040`, `:1055`, `:1061-1062`, `:1069`, `:1573`, `:1696-1711`, `:2951`, `:3397-3425`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:2976-2992`; `docs/offene-punkte.md:121`, `:264`, `:314` | Rampen werden nur in `bereiteVor`/`freigeben` zurückgestellt, die ENDE-Marke behält die Ziele und kehrt in der Ruhe früh zurück; `setzeZiel` rampt ab dem Iststand; der Crossfade aus der Ruhe liest Dry; `rampenKompatibel` kennt keine Wertegrenze und macht jede Topologieabweichung irgendeines Slots zum Crossfade in eine kalt publizierte Bank; die Hüllkurve vergleicht mit der Momentanleistung und lädt Hold bei jedem Anstieg; die Auto-Gain-Ableitung klemmt nicht; `processBlock` ruft den Kern in jedem Block; M-03, M-07, R2 F4, R7 F3, M-18/M-19/M-26, E-6, E-8, E-19, E-25, E-31, E2-7 und die Vertrags-/Registerstellen im Wortlaut |
| T3-15-12 | `broker/src/coordinator/flush.rs:1-318`; `broker/src/coordinator/liveness.rs:495-545`; `broker/src/store/handle.rs:210-245`; `broker/src/coordinator/mod.rs:300-340`; `broker/src/coordinator/senke.rs:120-164`; `broker/src/lebenslauf.rs:140-190`; `broker/src/coordinator/zustand.rs:624` samt Setzersuche; `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:884-900`; `docs/beweise/SONDE-011.md:533`, `:535`, `:629` | Jeder Heartbeat markiert die Sitzung dirty und flusht mit Verursacherlink; der verweigerte Append setzt `trennen` und kehrt ohne Push zurück; `routing_bereit` hat genau drei Setzer und keinen Weg zurück auf wahr; P2 hängt am Routinggate; der Client sendet P0 vor P1; S-01, S-03 und L-08 sagen weiterlaufenden Betrieb und P2 ausdrücklich zu |
| T3-15-13 | `broker/src/coordinator/subscription.rs:60-220`; `broker/src/coordinator/flush.rs:31-37`; `eq-copilot/plugin/src/SourcesModel.cpp:1350-1362` | Lesefehler und Vertragsbrüche rufen `routing_fail_closed`; das setzt brokerweit `routing_bereit = false` und `trennen`; die Degradationsausnahme `!routing_bereit && !store_degradiert` erklärt, warum der Dauerfall ehrlich als `storeDegraded` landet und nur der Fall mit gesundem Store „Authenticating“ zeigt |
| T3-15-14 | `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:715-760`; `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:846-900`; `broker/src/coordinator/befehl.rs:488-515`; `broker/src/coordinator/link.rs:238-265` | `inFlightNachReconnect` läuft im Aufbauzug vor der Schleife, die den ersten Heartbeat sendet; P0 geht vor P1; die Autorisierung prüft `bestaetigt` und Führung vor allem anderen; `bestaetigt` wird nur geerbt, nach Neustart also falsch |
| T3-15-15 | `broker/src/coordinator/link.rs:86-265`; `broker/src/coordinator/flush.rs:247-279`; `eq-copilot/plugin/src/SourcesModel.cpp:1350-1362`; `eq-copilot/plugin/src/PluginEditor.cpp:1205-1215`; Suchen nach `konflikt_guard_aufloesen` und `neueSensorId` über `broker/`, `eq-copilot/`, `tools/` | Jedes Hello verdrängt den älteren Link derselben Adresse und löscht Messwahrheit bei abweichender Nonce; die Kollision quarantänisiert beide Besitzer und persistiert einen Riegel; die Auflösung hat nur Testaufrufer; `neueSensorId` existiert nur im Gen-Prozessor; Gens Diagnosetext bildet den Konflikt auf die Beitrittsbestätigung ab |

Ebenfalls zentral gelesen: `AGENTS.md` und `CLAUDE.md` vollständig, `docs/plugin-wissen.md` §1.5b, §1.5c, §4, §6, §7, `docs/PLAN-STAND.md` (Kopf), `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` vollständig (alle Befundzeilen bis T3-14-04), `docs/audits/2026-09-15-tiefenaudit/roh/phase-14-blindspots.md` vollständig und `docs/offene-punkte.md` (Kopf, Klassen, NAK-110, NAK-188, NAK-254, NAK-278, NAK-306). Nicht zentral nachgelesen und deshalb als Herleitung des jeweiligen Prüfers gekennzeichnet: die in den Paketberichten genannten Nebenstellen und Testkörper jenseits der oben aufgeführten Bereiche.

## 4. Unabhängige Nachrechnung der DSP-Zahlen

Die Zahlen von Paket 2 wurden dreifach erzeugt: vom Finder, vom Gegenprüfer und zentral mit einem unabhängig geschriebenen Programm. Alle drei stimmen in den tragenden Werten überein. Das zentrale Programm ist eine **Nachrechnung der aus der Quelle abgeschriebenen Formeln, keine Ausführung von Produktcode** (kein Bau, keine Bibliothek des Plugins, keine Testbinary).

Zentrale Ausgabe:

```text
D-01: max Ausgang/Eingang 4.6832 (13.41 dB) bei n=128
D-03: stationaer vorher 0.9961, stationaer nachher 0.4999, Spitze im Wechsel 7.3929 = +17.41 dB ueber der groesseren
D-04: tiefster Periodenpegel -6.57 dB bei 40 ms, innerhalb 1 dB ab 220.0 ms
D-05: acht Low-Shelves +12 dB Q8 @1 kHz -> Auto-Gain -199.8 dB
D-05: acht High-Cuts 20 Hz Q0,15 -> Auto-Gain 150.5 dB
D-05: acht Low-Shelves +12 dB Q0,707 @1 kHz -> Auto-Gain -92.3 dB
```

Programm (abgeschriebene Quellen im Kopf benannt):

```python
# Phase 15, zentrale Gegenrechnung. NACHRECHNUNG aus der Quelle abgeschriebener
# Formeln, KEINE Produktausfuehrung.
#   DF2T:            eq-copilot/plugin/dsp/DspFilter.h:90-96
#   RBJ Bell/Shelf:  eq-copilot/plugin/dsp/DspFilter.h:116-167, HighCut :204-218
#   Biquad-Mischung: eq-copilot/plugin/dsp/DspKern.cpp:43-52, t je Sample :493-495
#   Crossfade:       eq-copilot/plugin/dsp/DspKern.cpp:796-805 (t = 1 - rest/256)
#   Rampe:           eq-copilot/plugin/dsp/DspKern.h:310-334
#   Auto-Gain:       eq-copilot/plugin/dsp/DspProgramm.cpp:76-136
import cmath, math

PI = 3.14159265358979323846

def bell(fs, f, q, g):
    A = 10 ** (g / 40); w = 2 * PI * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); a0 = 1 + al / A
    return [(1 + al * A) / a0, -2 * c / a0, (1 - al * A) / a0, -2 * c / a0, (1 - al / A) / a0]

def lowshelf(fs, f, q, g):
    A = 10 ** (g / 40); w = 2 * PI * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); z = 2 * math.sqrt(A) * al
    a0 = (A + 1) + (A - 1) * c + z
    return [A * ((A + 1) - (A - 1) * c + z) / a0, 2 * A * ((A - 1) - (A + 1) * c) / a0,
            A * ((A + 1) - (A - 1) * c - z) / a0, -2 * ((A - 1) + (A + 1) * c) / a0,
            ((A + 1) + (A - 1) * c - z) / a0]

def highcut(fs, f, q):
    w = 2 * PI * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); a0 = 1 + al
    return [(1 - c) * 0.5 / a0, (1 - c) / a0, (1 - c) * 0.5 / a0, -2 * c / a0, (1 - al) / a0]

class Zustand:
    def __init__(self): self.z1 = 0.0; self.z2 = 0.0
    def tick(self, k, x):
        y = k[0] * x + self.z1
        self.z1 = k[1] * x - k[3] * y + self.z2
        self.z2 = k[2] * x - k[4] * y
        return y

# --- T3-15-05: Rampe ueber Aus/Ein (geschlossene Form aus DspKern.h:317-334 und :796-805)
g_alt = 10 ** (24 / 20); ziel = 1.0; schritt = (ziel - g_alt) / 256
akt = g_alt; maxv = 0.0; argmax = -1
for n in range(256):
    akt += schritt
    if n == 255: akt = ziel
    t = 1 - (256 - n) / 256          # erstes Fadesample t = 0
    v = (1 - t) + t * akt
    if v > maxv: maxv, argmax = v, n
print("D-01: max Ausgang/Eingang %.4f (%.2f dB) bei n=%d" % (maxv, 20 * math.log10(maxv), argmax))

# --- T3-15-07: Low-Shelf +6 dB Q0,707, 48 kHz, 5 kHz -> 50 Hz, 1-kHz-Sinus 0,5
fs = 48000.0
ka = lowshelf(fs, 5000.0, 0.707, 6.0); kb = lowshelf(fs, 50.0, 0.707, 6.0)
s = Zustand(); vor = 0.0; nach = 0.0; n_vor = 48000
for n in range(n_vor + 24000):
    x = 0.5 * math.sin(2 * PI * 1000.0 * n / fs)
    if n < n_vor:
        k = ka
    else:
        i = n - n_vor
        t = 1.0 - (256 - i - 1) / 256 if i < 256 else 1.0
        k = [ka[j] + (kb[j] - ka[j]) * t for j in range(5)]
    y = s.tick(k, x)
    if n_vor - 4800 <= n < n_vor: vor = max(vor, abs(y))
    if n >= n_vor: nach = max(nach, abs(y))
sb = Zustand(); stat_b = 0.0
for n in range(n_vor + 24000):
    y = sb.tick(kb, 0.5 * math.sin(2 * PI * 1000.0 * n / fs))
    if n >= n_vor: stat_b = max(stat_b, abs(y))
ref = max(vor, stat_b)
print("D-03: stationaer vorher %.4f, stationaer nachher %.4f, Spitze im Wechsel %.4f = %+.2f dB ueber der groesseren"
      % (vor, stat_b, nach, 20 * math.log10(nach / ref)))

# --- T3-15-08: Bell 50 Hz Q8 +12 dB, 50-Hz-Ton 0,1, Crossfade in kalte Bank
k = bell(fs, 50.0, 8.0, 12.0)
alt = Zustand(); neu = Zustand(); n_vor = int(4 * fs)
for n in range(n_vor):
    alt.tick(k, 0.1 * math.sin(2 * PI * 50.0 * n / fs))
soll = 0.1 * 10 ** (12 / 20); per = 960; spitze = 0.0; spitzen = []
for m in range(int(2 * fs)):
    x = 0.1 * math.sin(2 * PI * 50.0 * (n_vor + m) / fs)
    ya = alt.tick(k, x); yn = neu.tick(k, x)
    y = ya * (1 - m / 256) + yn * (m / 256) if m < 256 else yn
    spitze = max(spitze, abs(y))
    if (m + 1) % per == 0: spitzen.append(spitze); spitze = 0.0
tief = min(spitzen); it = spitzen.index(tief)
erholt = next(((i + 1) * per / fs * 1000 for i, v in enumerate(spitzen) if i > it and abs(20 * math.log10(v / soll)) < 1.0), None)
print("D-04: tiefster Periodenpegel %+.2f dB bei %.0f ms, innerhalb 1 dB ab %s ms"
      % (20 * math.log10(tief / soll), (it + 1) * per / fs * 1000, erholt))

# --- T3-15-09: Auto-Gain (alle Baender stereo), 48 kHz
def autogain(baender, fs):
    summe = 0.0
    for i in range(121):
        f = 20.0 * (1000.0 ** (i / 120)); w = 2 * PI * f / fs
        z = cmath.exp(-1j * w); h = 1.0
        for b in baender:
            h *= (b[0] + b[1] * z + b[2] * z * z) / (1 + b[3] * z + b[4] * z * z)
        summe += abs(h) ** 2          # hL = hR = hStereo -> 0,5*(|hL|^2+|hR|^2) = |h|^2
    return -10 * math.log10(summe / 121)
print("D-05: acht Low-Shelves +12 dB Q8 @1 kHz -> Auto-Gain %.1f dB" % autogain([lowshelf(fs, 1000.0, 8.0, 12.0)] * 8, fs))
print("D-05: acht High-Cuts 20 Hz Q0,15 -> Auto-Gain %.1f dB" % autogain([highcut(fs, 20.0, 0.15)] * 8, fs))
print("D-05: acht Low-Shelves +12 dB Q0,707 @1 kHz -> Auto-Gain %.1f dB" % autogain([lowshelf(fs, 1000.0, 0.707, 12.0)] * 8, fs))
```

Weitere Zahlen der Paketberichte (Detektorversatz, Kaltstart weiterer Bänder, Cramping, Steuerrate, Stabilität der Zwischenentwürfe) sind vom Gegenprüfer mit eigenen Programmen nachgerechnet und in seinem Bericht mit Werten aufgeführt; ihre Skripte lagen im Arbeitsordner des Laufs und sind nicht Teil des Repos. Tragend für die aufgenommenen Zeilen sind die oben abgedruckten Werte.

## 5. Abgrenzung und Zähler

- **Nicht erneut gezählt:** die 59 Befunde nach Phase 10/11, die sechs Phase-13- und die vier Phase-14-Befunde. Die Phase-15-Zeilen sind ein eigener, ungebündelter Nachtrag und nicht rückwirkend durch den Skeptikerpass der Phase 10 gelaufen.
- **Überlappungen ausdrücklich benannt:** T3-15-01 grenzt gegen T3-03-03, T3-07-04 und NAK-278 ab; T3-15-02 gegen T3-08-04 und T3-14-03; T3-15-08 ist die Gegenrichtung zu T3-14-02; T3-15-14 teilt die stille Verwurfshälfte mit T3-07-04; T3-15-13 und der nicht aufgenommene Kandidat P15-T-03 grenzen gegen NAK-188, NAK-125, NAK-164, NAK-165 und T3-14-03 ab.
- **Keine neuen NAK-Nummern**, keine Ticketfreigabe, keine Änderung an `docs/offene-punkte.md`, `docs/plan/*.json` oder `docs/PLAN-STAND.md`. Die Empfehlung zu NAK-188 (Abschnitt 2) und die drei Entscheidbedarfe aus T3-15-06, T3-15-09 und T3-15-10 sind Vorschläge an den Dirigenten.
- **Zähler:** 15 Befunde (DEFEKT 6, LÜCKE 5, HÄRTUNG 4; high 2, medium 6, low 7, critical 0), 19 offene Prüffelder, 53 verworfene Verdachte (19 · 19 · 15), 1 Duplikat, 0 widerlegte Befunde, 0 neue Produktfragenkarten.

## 6. Dokument- und Integritätsprüfung

- Der alte Inhalt von `BEFUNDE.md` ist als vollständiges Präfix bytegleich erhalten: **199.371 Bytes**, SHA-256 **`9f6e8b34d0f51a2a20d389f5cc9a42f9cb73200856a3e873640889e09219ae56`** (vor und nach dem Anfügen geprüft). Die Datei misst danach 243.364 Bytes; der Nachtrag beginnt mit der Überschrift „Phase 15“.
- Alle Befund-IDs `T3-15-01` bis `T3-15-15` kommen im Abschnitt genau einmal als Tabellenzeile vor; die drei Befundtabellen tragen durchgehend zehn Spalten in der Reihenfolge der Phasen 13 und 14.
- Alle **150 verschiedenen Fundstellen** des neuen Abschnitts und die **65** dieses Berichts wurden mit einem eigenen Prüfskript (Arbeitsordner, nicht im Repo) auf vorhandene Datei und nicht leere Zielzeile geprüft: **0 Befunde**. Das ist eine Herkunfts-, keine Fachprüfung.
- `py -3.13 tools/plan/dokuriegel.py` läuft über `BEFUNDE.md` und alle sieben Phase-15-Rohberichte ohne Befund (der eine benannte Verweis `.claude/agents/` stammt unverändert aus dem Phase-11-Abschnitt und ist dort als noch nicht angelegt markiert).
- `git diff --check` meldet keinen Whitespacefehler. Alle Dateien liegen in LF, wie die übrigen Dateien des Auditordners; Git weist auf seine übliche LF→CRLF-Umwandlung beim nächsten Schreibzugriff hin.
- Eigener Änderungsumfang: `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (nur angefügt) und sieben neue Dateien unter `docs/audits/2026-09-15-tiefenaudit/roh/` (`phase-15-zentral.md`, drei Paketberichte, drei Gegenprüfungen). Kein Produktcode, kein Vertrag, kein Werkzeug, kein Register, kein Plan. Die vor dem Lauf vorhandenen fremden Einträge `briefing-hub/` und `nimbalyst-local/` sowie der uncommittete Phase-14-Abschnitt blieben unangetastet.
- Keine dynamische Reproduktion: Produktbuild, Tests, Kanon, Installation, Broker-, Pipe-, Audio-, UI- und Hostläufe, Sanitizer und Benchmarks sind **NICHT GELAUFEN**. Die Nachrechnungen sind eigenständige Programme über abgeschriebene Formeln.
- `py -3.13 tools/plan/planstand.py` wurde in diesem Lauf **nicht** gefahren: der Schreibumfang des Auftrags endet bei diesem Auditordner, und ein lesender Audit verändert keinen gebauten oder abgenommenen Planschritt. `docs/PLAN-STAND.md` bleibt auf seinem Quellstand.
- **Nicht committet:** Die geänderte `BEFUNDE.md` trägt zugleich den uncommitteten Phase-14-Abschnitt einer anderen Session; ein Commit dieser Datei würde fremde Arbeit mit aufnehmen. Der Dirigent entscheidet, ob Phase 14 und Phase 15 gemeinsam committet werden.
