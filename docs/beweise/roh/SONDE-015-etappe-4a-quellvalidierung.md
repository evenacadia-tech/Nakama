# SONDE-015 Etappe 4a — Quellvalidierung der sechs Erstprüfungsbefunde

Lesender Lauf, 12.09.2026. Repo `C:\Users\phili\Projekte\Nakama`, HEAD `0359e4da`.
Nichts geschrieben, nichts gebaut, nichts gefahren; Git nur `log`, `show`, `diff`.
`docs/audits/2026-09-12-code-review-2/` nicht angefasst.

## 0. Stand und Gültigkeit der Zeilennummern

Urteil: `docs/beweise/roh/SONDE-015-etappe-4a-erstpruefung-dc6c148.txt`,
Zeilenangaben zum Stand `dc6c148a` („SONDE-015 Etappe 4a: Manifest §10,
Rotbeweise und Beinrohdatei", 10.09.2026 22:16).

Gemessen:

```
git diff --stat dc6c148a..HEAD -- \
  eq-copilot/plugin/sonde/SondeProcessor.h \
  eq-copilot/plugin/sonde/SondeProcessor.cpp \
  eq-copilot/plugin/state/NakamaTransaktion.h \
  eq-copilot/plugin/state/NakamaTransaktion.cpp \
  eq-copilot/plugin/tests/TransactionTestMain.cpp \
  eq-copilot/plugin/dsp/DspKern.h eq-copilot/plugin/dsp/DspKern.cpp
 eq-copilot/plugin/dsp/DspKern.h | 18 ++++++++++++++++++
 1 file changed, 18 insertions(+)
```

**Alle sechs Zeilenangaben des Urteils gelten unverändert an HEAD.**
`SondeProcessor.cpp` (1260 Zeilen), `SondeProcessor.h` (461),
`NakamaTransaktion.h` (393), `NakamaTransaktion.cpp` (820),
`TransactionTestMain.cpp` (2251) sind byteidentisch zu `dc6c148a`.
Nur `DspKern.h` trägt +18 Zeilen (lesender Testzugang der Audit-Nacharbeit,
Manifest §11.6 A-1) — keine der sechs Fundstellen liegt darin.

`docs/beweise/SONDE-015.md` hat sich geändert (+177/−8), aber alle Hunks vor
Zeile 4149 sind längengleiche Ersetzungen: **die Matrixzeilen liegen weiterhin
auf denselben Zeilen** (M-67 = `:1136`, M-125 = `:1154`, wie das Urteil zitiert).

`CLAUDE.md` wurde zwischen `dc6c148a` und HEAD verdichtet. Die vom Urteil
zitierten Stellen sind an HEAD umgezogen:

| Urteil (dc6c148a) | HEAD | Invariante |
|---|---|---|
| `CLAUDE.md:43-44` | `CLAUDE.md:32-33` | Audio-Thread: keine Sperren … |
| `CLAUDE.md:226` | `CLAUDE.md:189` | NaN-Ehrlichkeit |
| `CLAUDE.md:232-233` | `CLAUDE.md:194-195` | State bleibt verlustfrei (read-only) |

Der Wortlaut ist in allen drei Fällen inhaltlich unverändert (geprüft mit
`git show dc6c148a:CLAUDE.md`).

## 0.1 Gate-Text S26–28, wörtlich

`docs/plan/plan.json`, Phase P6, Schritt `"id": "S26–28"`, `"ticket":
"SONDE-015"`, `"stufe": "T2"`. Der Schritt trägt **kein eigenes Feld
`gate`/`tor`** — die Schlüssel sind `id`, `leitungsname`, `ticket`, `text`,
`beleg`, `stufe`; `beleg` ist leer. Der Gate-Text IST das Feld `"text"`, und
§1.1 des Manifests zitiert es in voller Länge. Der für diese sechs Befunde
tragende Satz ist der Kopfsatz:

> Lokaler EQ-Kern in der Sonde: vier vorbereitete Bänke, Zustand und
> Automation, A/B — dazu die versionierte Zustands-Erweiterung aus dem
> 23.08.-Entscheid: Mix je Sonde und die Schutz-Zonen bekommen ihren Platz im
> gespeicherten Zustand (Layout v2, neue Kennungen, nie umgewidmet).

Das Gate nennt „Zustand und Automation" und „Schutz-Zonen" als Liefergegenstand,
sagt aber **nichts** über Sperrenordnung, Zählung nicht-endlicher Hostwerte oder
Wachenkadenz im Test. Für die Befunde 2, 4, 5 und 6 ist der Gate-Text stumm; sie
werden gegen CLAUDE.md und die Matrix gehalten.

## 0.2 CLAUDE.md-Invarianten, wörtlich (HEAD)

- `CLAUDE.md:32-33`: „Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe-
  oder Netzzugriffe, kein Logging; Überlast verwirft Analyseframes, nie Audio."
- `CLAUDE.md:184-186`: „**Audio bleibt echtzeitfest.** Steuerung über Pipe oder
  UI wird außerhalb des Audio-Threads übernommen; Zustands- und Parameterhoheit
  bleibt in der Audio führenden Instanz."
- `CLAUDE.md:189-190`: „**NaN-Ehrlichkeit.** Nicht-endliche Werte werden
  verriegelt und gezählt; Nyquist gekappt; ohne genügend endliche Nachbarn keine
  Basislinie."
- `CLAUDE.md:194-196`: „**State bleibt verlustfrei.** Unbekannte Major-Versionen
  oder unzulässige Matrizen werden read-only mit Originalbytes gehalten; jede
  persistente Änderung meldet dem Host Dirty-State."

## 0.3 Thread- und Sperrenkarte (gemessen, Grundlage für 1 bis 4)

**Threads**

| Thread | Wer ihn fährt | Was er in 4a anfasst |
|---|---|---|
| Audiothread (VST3 `process`) | Host | `processBlock`, `parameterValueChanged` (über `processParameterChanges`), `setNonRealtime` |
| Message-/Hostthread | Host bzw. eigene Oberfläche | `setStateInformation`, `getStateInformation`, `parameterGestureChanged`, `fuehreTransaktionAus` |
| Analyseworker (Control-Worker) | `SondeProcessor::workerLauf` | `dspKontrollTakt()` alle ≥ 5 ms |

`SondeProcessor.cpp:445-461` (Worker):

```cpp
void SondeProcessor::workerLauf()
{
    ...
    double naechsterKontrollTakt = juce::Time::getMillisecondCounterHiRes();
    while (workerLaeuft.load())
    {
        // SONDE-015 4a: der Takt des Control-Workers - ACKs ernten,
        // Hostautomation uebernehmen, den wirksamen Zustand publizieren. Er
        // nimmt das Zustandsschloss und NIE zugleich das Analyseschloss.
        if (juce::Time::getMillisecondCounterHiRes() >= naechsterKontrollTakt)
        {
            dspKontrollTakt();
            naechsterKontrollTakt = juce::Time::getMillisecondCounterHiRes() + 5.0;
        }
```

**Sperren**

| Sperre | Typ | Wer nimmt sie |
|---|---|---|
| `zustandSchloss` | `juce::CriticalSection` (rekursiv), `SondeProcessor.h:356` | Message-Thread, Control-Worker, **und** Audiothread über `setNonRealtime` im Offline-Betrieb |
| `getCallbackLock()` | JUCE-Callbacklock | Wrapper vor `processAudio`; `prepareToPlay` `:234`, `releaseResources` `:258`, `v3Status` `:964` |
| `listenerLock` | **je Parameterobjekt**, `juce_AudioProcessorParameter.cpp` | Audiothread (`sendValueChangedMessageToListeners`), Message-Thread (`beginChangeGesture`/`endChangeGesture`) |
| `analyseSchloss` | `std::mutex` | nur Analyseworker |

**Gemessene Sperrenordnung** (kein Zyklus gefunden, also kein Deadlock):

- `getCallbackLock()` → `zustandSchloss`: `prepareToPlay` (`:234-235`),
  `releaseResources` (`:258-259`), und der Audiothread über
  `juce_audio_plugin_client_VST3.cpp:3890` → `:3892` → `SondeProcessor.cpp:1105`.
- `listenerLock(X)` → `zustandSchloss`: `parameterGestureChanged` (`:1128`) und
  alles, was `gestusAbschliessen` (`:1138`) darunter zieht.
- **Kein Pfad nimmt `zustandSchloss` und danach `listenerLock` oder
  `getCallbackLock()`**: `hostParameterAbgleichen` wird in allen drei Aufrufern
  (`:1040`, `:883`, `:1171`) ausdrücklich außerhalb des Schlosses gerufen;
  `v3Status` schließt den `zustandSchloss`-Block bei `:955`, bevor es bei `:964`
  den Callbacklock nimmt.
- Folge: Die Befunde 2 und 4 sind **Blockierung und Prioritätsumkehr**, kein
  Deadlock. Wer `zustandSchloss` hält, hält es lange: `dspKontrollTakt`
  (`:1229-1258`) hält es über `dspKern->pflege()` UND den Programmbau in
  `publiziereWirksam`; `fuehreTransaktionAus` (`:1015-1033`) hält es über die
  ganze Stufenfolge inklusive `state_hash` (S6) und Programmbau (S5).

---

# Befund 1 — Abgleich verwirft überholte Hostereignisse nicht

## Befund wörtlich

> - [P1] Verwerfe überholte Hostereignisse beim Zustandsabgleich — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:1208-1210
>   DEFEKT (M-67/M-84): Wird zwischen zwei Kontrolltakten ein Frequenzgestus von 500 auf 1000 Hz wegen einer Schutz-Zone von 900–1100 Hz abgewiesen, setzt dieser Abgleich nur den Hostparameter zurück. Wegen `AbgleichHerkunft` bleibt der ausstehende Mailboxwert 1000 Hz erhalten; der nächste `dspKontrollTakt` übernimmt ihn erneut als Automation. Der abgewiesene Wert wird damit trotzdem wirksam, entgegen „ohne halbe Anwendung" aus [M-67](docs/beweise/SONDE-015.md#L1136). Dieselben ausstehenden Ereignisse überleben auch einen Reload und gefährden M-84. Der Abgleich muss überholte Mailboxereignisse mit invalidieren.

## Quellzitat an HEAD

`eq-copilot/plugin/sonde/SondeProcessor.cpp:1110-1118` (die Mailbox wird gefüllt):

```cpp
void SondeProcessor::parameterValueChanged (int index, float neuNormiert)
{
    // Laeuft auch im Audiothread: nur Atomics, kein Schloss, keine Allokation.
    if (abgleichTiefe > 0 || index < 0 || index >= nakama::parameter::kHostParameter)
        return;
    hostWert[(size_t) index].store (neuNormiert, std::memory_order_relaxed);
    hostEreignis[(size_t) index].fetch_add (1, std::memory_order_relaxed);
    hostEreignisOffen.store (true, std::memory_order_relaxed);
}
```

`:1161-1172` (Abweisung eines Gestus — nur Overlay und Hostparameter werden zurückgesetzt):

```cpp
    // Abgewiesen (etwa User-Schutz, M-67): Klang und Regler kehren zum
    // bestaetigten Wert zurueck - keine halbe Anwendung.
    nakama::parameter::Satz abgleich;
    {
        const juce::ScopedLock l (zustandSchloss);
        for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
            if (beteiligt[(size_t) i]) transaktion->automationLoesen (i);
        publikationOffen = true;
        abgleich = transaktion->bestaetigt().werte;
    }
    hostParameterAbgleichen (abgleich);
}
```

`:1202-1212` (die vom Urteil benannte Stelle — der Abgleich rührt die Mailbox nicht an):

```cpp
void SondeProcessor::hostParameterAbgleichen (const nakama::parameter::Satz& werte)
{
    const AbgleichHerkunft herkunft;
    for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
    {
        auto* p = hostParameter[(size_t) i];
        const float soll = p->convertTo0to1 (hostWertAus (i, werte[(size_t) i]));
        if (p->getValue() != soll)
            p->setValueNotifyingHost (soll);
    }
}
```

`:1234-1246` (der nächste Takt liest den **Mailboxwert**, nicht den Reglerwert):

```cpp
    // Hostereignisse -> AutomationOverlay (M-81): keine Revision, kein Undo.
    if (hostEreignisOffen.exchange (false, std::memory_order_relaxed))
    {
        for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
        {
            const auto n = hostEreignis[(size_t) i].load (std::memory_order_relaxed);
            if (n == hostEreignisGesehen[(size_t) i]) continue;
            hostEreignisGesehen[(size_t) i] = n;
            transaktion->automationSchreiben (i, zelleAusHost (i, hostWert[(size_t) i].load (std::memory_order_relaxed)));
            samplesBeiLetzterAutomation = verarbeiteteSamples.load (std::memory_order_relaxed);
            publikationOffen = true;
        }
    }
```

Ergänzend `eq-copilot/plugin/state/NakamaTransaktion.cpp:316-324` und `:340-348`
— das Overlay schlägt beim Publizieren voll durch:

```cpp
void Transaktionskern::automationSchreiben (int index, const param::Zelle& wert) noexcept
{
    if (index < 0 || index >= param::kHostParameter) return;
    auto& o = automationOverlay;
    if (! o.laeuft) { o.laeuft = true; ++o.epoche; }   // Beginn einer Hostgeste
    const bool gleich = zelleGleich (index, wert, committed.werte[(size_t) index]);
    o.gesetzt[(size_t) index] = ! gleich;
    o.werte[(size_t) index]   = wert;
}
...
param::DspSatz Transaktionskern::wirksam() const
{
    auto s = committed;
    const auto& t = param::tabelle();
    for (int i = 0; i < param::kHostParameter; ++i)
        if (automationOverlay.gesetzt[(size_t) i] && (samplegenau || ! t[(size_t) i].topologisch))
            s.werte[(size_t) i] = automationOverlay.werte[(size_t) i];
    return s;
}
```

Wichtige Gegenprobe zum Topologie-Filter: `v1.band.<n>.freq_hz` entsteht über
`gleitkomma(...)`, und `NakamaParameter.cpp:30` setzt dort hart
`b.topologisch = false`. Ebenso `enabled` und `v2.global.eq_enabled`
(`boolean(id, false, false)`, `:62`, `:79`). Der Topologiefilter aus E4-12/M-119
fängt den Fall **nicht** ab — das Overlay wird angewandt.

## Zusage, gegen die der Befund gehalten wird

`docs/beweise/SONDE-015.md:1136`, M-67, Spalte Zusage, wörtlich:

> Die Transaktion wird **abgewiesen**, mit dem eigenen Grund „User-Schutz" und
> **ohne halbe Anwendung**: der bestätigte Zustand, die Revision und der Hash
> bleiben unverändert.

Rotbeweisspalte M-67, wörtlich: „Das Band landet in der Zone; oder die Frequenz
wird gesetzt und `enabled` nicht, sodass ein halber Zustand entsteht …"

`docs/beweise/SONDE-015.md:1162`, M-84, Spalte Zusage, wörtlich:

> Der Reload rekonstruiert **denselben `state_hash`** und innerhalb numerischer
> Toleranz denselben Audioausgang. Reihenfolge: State lesen → validieren →
> Committed setzen → Hostparameter synchronisieren → erster Block.

## Status: **BESTÄTIGT**

Aus dem Code reproduzierbar ableitbar, mit einer Präzisierung gegen den
Urteilstext: M-67 sagt die Unveränderlichkeit von *bestätigtem Zustand, Revision
und Hash* zu — die bleiben tatsächlich unberührt. Verletzt wird der zweite Teil
der Zusage, „ohne halbe Anwendung": der abgewiesene Wert wird über das
Automationsoverlay **hörbar**, obwohl der bestätigte Zustand ihn ablehnt. Das
ist genau der Rotfall „Das Band landet in der Zone". Für M-84 ist der
Hash-Anteil unberührt (`gehaltenerStand` schreibt nur `bestaetigt`), verletzt
wird der Audio-Anteil.

## Auslösekette

1. Message-Thread (eigene Oberfläche): `beginChangeGesture` auf `freq_hz`,
   danach `setValueNotifyingHost(1000 Hz)` → `parameterValueChanged` →
   `hostWert[i] = 1000 Hz`, `hostEreignis[i]++`, `hostEreignisOffen = true`.
   `abgleichTiefe` ist hier 0, der Wert landet also in der Mailbox.
2. `endChangeGesture` → `parameterGestureChanged(false)` → `gestusAbschliessen`
   → `fuehreTransaktionAus` → S3 Zonenriegel → `Ausgang::userSchutz`.
3. `:1165-1171`: `automationLoesen(i)` (Overlay-Bit fällt),
   `hostParameterAbgleichen` setzt den Regler auf 500 Hz zurück — unter
   `AbgleichHerkunft`, damit `parameterValueChanged` (`:1113`) den eigenen
   Abgleich verwirft. **Genau deshalb wird `hostWert[i]` nicht überschrieben.**
4. Control-Worker, nächster Takt (≤ 5 ms später): `hostEreignisOffen` ist noch
   true und `hostEreignis[i] != hostEreignisGesehen[i]`, weil zwischen Schritt 1
   und 3 kein Takt lief. `automationSchreiben(i, 1000 Hz)` setzt das Overlay-Bit
   im **selben Takt** wieder, danach publiziert `:1256-1257` `wirksam()` mit
   1000 Hz. Reihenfolge innerhalb des Takts: Hostereignisse (`:1235`) **vor**
   Publikation (`:1256`) — die Rücknahme aus Schritt 3 wird im selben Durchlauf
   überschrieben.
5. `zelleAusHost` kann das nicht abfangen: seine Kurzschlusswache `:1183`
   vergleicht gegen das übergebene `normiert` (= Mailboxwert 1000 Hz), nicht
   gegen `p->getValue()` (= 500 Hz nach dem Abgleich).
6. Reload-Variante (M-84): `setStateInformation` (`:831-886`) leert
   `gesteOffen`/`gesteBeteiligt` (`:873-874`), aber **nicht**
   `hostEreignis`/`hostEreignisGesehen`/`hostEreignisOffen`. Ein vor dem Laden
   eingetroffener, noch nicht konsumierter Hostwert wird nach dem Ladestart auf
   den frisch geladenen Zustand gelegt.

Thread-Zuordnung: Schritte 1–3 Message-Thread (bzw. Audiothread, wenn der
Hostautomationspunkt über `processParameterChanges` kommt — dann fehlt der
Gestus und nur der Reload-Pfad greift), Schritt 4 Control-Worker.

## Vorschlag Einordnung: **DEFEKT**

Grund: verletzt M-67 („ohne halbe Anwendung", `SONDE-015.md:1136`) im hörbaren
Ergebnis und M-84 („innerhalb numerischer Toleranz denselben Audioausgang",
`:1162`). Zusätzlich reißt die Paarregel aus CLAUDE.md „Arbeitsregeln"
(aktivieren ↔ abklingen im selben Änderungssatz): das Gesten-Postfach wird beim
Laden geleert, das Hostereignis-Postfach nicht.

## Existierender Test

- `eq-copilot/plugin/tests/TransactionTestMain.cpp:1214-1225`
  (`band_in_zone_ziehen_wird_abgewiesen`,
  `abweisung_laesst_hash_und_revision_unveraendert`) — fährt **nur den
  Transaktionskern** (`st.tk->fuehreAus`), nie den Prozessor, nie die Mailbox.
- `:2183` `reload_rekonstruiert_denselben_audioausgang` (M-84) — ohne
  ausstehendes Hostereignis vor dem Reload.
- Ein Prozessor-Test für einen **abgewiesenen** Gestus existiert nicht: die
  einzigen `beginChangeGesture`/`endChangeGesture`-Stellen im ganzen B7 sind
  `:2109-2120` (M-82, Gestus committet).

**Würde heute nicht rot.**

---

# Befund 2 — Gestus-Listener hält Sperren und rechnet die Transaktion synchron

## Befund wörtlich

> - [P1] Entkopple den Gestus-Listener von Sperren und Transaktionsarbeit — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:1135-1135
>   DEFEKT bei einem Gestus parallel zur Hostautomation desselben Parameters: JUCE hält während `parameterGestureChanged` bereits `listenerLock`. Dieser Callback wartet zusätzlich auf `zustandSchloss` und führt beim Abschluss synchron die Transaktion einschließlich Hash und Programmbau aus. Der VST3-Audiothread benötigt beim Weiterreichen der Automation über `setValueNotifyingHost` denselben `listenerLock` und kann deshalb auf die vollständige Transaktionsarbeit warten. Das verletzt [CLAUDE.md:43–44](CLAUDE.md#L43-L44). Auch Gesten müssen ohne diese Arbeit beziehungsweise Sperren an den Control-Worker übergeben werden.

## Quellzitat an HEAD

`eq-copilot/plugin/sonde/SondeProcessor.cpp:1120-1136`:

```cpp
void SondeProcessor::parameterGestureChanged (int index, bool beginnt)
{
    // Eine Geste beginnt nur die eigene Oberflaeche - der VST3-Wrapper reicht
    // keine Hostgeste an das Plugin weiter. Ein abgeschlossener manueller
    // Gestus ist EINE Transaktion (§44.3, M-82).
    if (index < 0 || index >= nakama::parameter::kHostParameter) return;
    bool abschliessen = false;
    {
        const juce::ScopedLock l (zustandSchloss);
        gesteOffen[(size_t) index] = beginnt;
        if (beginnt)
            gesteBeteiligt[(size_t) index] = true;
        else
            abschliessen = std::none_of (gesteOffen.begin(), gesteOffen.end(), [] (bool offen) { return offen; });
    }
    if (abschliessen) gestusAbschliessen();
}
```

JUCE, `eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessorParameter.cpp:88-121`:

```cpp
void AudioProcessorParameter::endChangeGesture()
{
    ...
    ScopedLock lock (listenerLock);

    for (int i = listeners.size(); --i >= 0;)
        if (auto* l = listeners[i])
            l->parameterGestureChanged (getParameterIndex(), false);
    ...
}

void AudioProcessorParameter::sendValueChangedMessageToListeners (float newValue)
{
    ScopedLock lock (listenerLock);

    for (int i = listeners.size(); --i >= 0;)
        if (auto* l = listeners [i])
            l->parameterValueChanged (getParameterIndex(), newValue);
    ...
}
```

Der Audiothread nimmt dieselbe Sperre — `juce_audio_plugin_client_VST3.cpp:843-850`
(gerufen aus `processParameterChanges`, `:3684-3688`, gerufen aus `process`, `:3756-3757`):

```cpp
static void setValueAndNotifyIfChanged (AudioProcessorParameter& param, float newValue)
{
    if (approximatelyEqual (param.getValue(), newValue))
        return;

    const InParameterChangedCallbackSetter scopedSetter { inParameterChangedCallback };
    param.setValueNotifyingHost (newValue);
}
```

Und die Arbeit, auf die gewartet wird — `SondeProcessor.cpp:1009-1033`:

```cpp
nakama::transaktion::Ergebnis SondeProcessor::fuehreTransaktionAus (const nakama::transaktion::Auftrag& auftrag)
{
    ...
    {
        const juce::ScopedLock l (zustandSchloss);
        ...
        ergebnis = transaktion->fuehreAus (auftrag);
```

`fuehreAus` umfasst laut Manifest §10.1 S5 (Programmbau, `NakamaTransaktion.cpp:591-593`),
S6 (`state_hash`, `:597-609`) und S8 (Publikation, `:643-707`) — alles unter
`zustandSchloss`.

## Invariante, gegen die der Befund gehalten wird

`CLAUDE.md:32-33` (HEAD), wörtlich:

> Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe- oder Netzzugriffe,
> kein Logging; Überlast verwirft Analyseframes, nie Audio.

`CLAUDE.md:184-186`, wörtlich:

> **Audio bleibt echtzeitfest.** Steuerung über Pipe oder UI wird außerhalb des
> Audio-Threads übernommen; Zustands- und Parameterhoheit bleibt in der Audio
> führenden Instanz.

Matrix M-47 (`SONDE-015.md:1100`), Zusage wörtlich: „Im Callback gibt es **keine
Sperre, keine Allokation, keinen Datei-, Pipe- oder Netzzugriff, kein Logging
und keine geworfene Ausnahme**."

## Status: **BESTÄTIGT (mit Einschränkung der heutigen Erreichbarkeit)**

Die Sperrenkette ist aus dem Code belegt:
`listenerLock(X)` (JUCE, Message-Thread) → `zustandSchloss` (`:1128`) →
`gestusAbschliessen` (`:1135`, **noch innerhalb `listenerLock(X)`**) →
`zustandSchloss` über die ganze Transaktion. Der Audiothread nimmt bei einem
Automationspunkt auf demselben Parameter `X` dieselbe `listenerLock(X)` und
wartet. `listenerLock` ist ein Mitglied **je Parameterobjekt** — die Kollision
verlangt also wirklich denselben Parameter, wie das Urteil sagt.

**Einschränkung, die das Urteil nicht nennt:** `SondeProcessor.h:145-146` meldet
`hasEditor() == false` und `createEditor()` liefert `nullptr`. Der VST3-Wrapper
ruft `beginChangeGesture`/`endChangeGesture` nirgends am Parameterobjekt
(`:1489`/`:1494` sind die Gegenrichtung, Plugin → Host). **Heute erreicht
ausschließlich B7 diesen Pfad**; im Produkt gibt es noch keinen Gestuserzeuger.
Der Pfad wird mit S31b und der Fernsteuerung scharf.

Kein Deadlock: es gibt keinen Pfad, der `zustandSchloss` hält und danach
`listenerLock` nimmt (siehe §0.3). Der Schaden ist Blockierung des Audiothreads
über eine unbegrenzte Zeit (Programmbau + SHA-256 + eventuell Wartezeit auf den
Control-Worker, der `zustandSchloss` bis zu einem vollen Takt hält).

## Auslösekette

1. Message-Thread: letzte offene Geste endet →
   `AudioProcessorParameter::endChangeGesture()` nimmt `listenerLock(X)`.
2. Im Callback: `zustandSchloss` (`:1128`), freigegeben; dann `:1135`
   `gestusAbschliessen()` — weiterhin unter `listenerLock(X)`.
3. `gestusAbschliessen` nimmt `zustandSchloss` (`:1143`), gibt frei, ruft
   `fuehreTransaktionAus` (`:1157`) → `zustandSchloss` über S0–S8,
   danach `hostParameterAbgleichen` (`:1171`) mit 112 `setValueNotifyingHost`.
4. Parallel Audiothread: `process` → `processParameterChanges` →
   `setValueAndNotifyIfChanged(X, …)` → `sendValueChangedMessageToListeners` →
   `ScopedLock (listenerLock)` — **blockiert**, bis Schritt 3 fertig ist.
5. Zusätzlicher Verstärker: hält in Schritt 3 der Control-Worker gerade
   `zustandSchloss` (`dspKontrollTakt` mit `pflege()` und `publiziereWirksam`),
   wartet der Message-Thread dort — und der Audiothread wartet mit.

## Vorschlag Einordnung: **DEFEKT**

Grund: verletzt `CLAUDE.md:32-33` („Audio-Thread: keine Sperren") und die
Zusage M-47. Die Matrix verlangt nirgends, dass ein Gestus synchron im
Listener-Callback abgeschlossen wird; E4-13 (`SONDE-015.md:3953`) sagt nur
„Commit, wenn die letzte offene Geste endet", nicht *auf welchem Thread*. Die
Übergabe an den Control-Worker ist also ohne Zusagebruch möglich.

## Existierender Test

- `TransactionTestMain.cpp:2109-2127` (`abgeschlossener_gestus_ist_eine_revision`,
  M-82) — einthreadig, misst Revision und Endwerte, keine Sperren.
- Der einzige Sperrenzähler der Codebasis liegt in
  `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2974-2988`
  (`null_sperren_im_callback`, M-47) und misst **`DspKern::verarbeite`**, nicht
  `SondeProcessor::processBlock` und nicht den Wrapper-Pfad.

**Würde heute nicht rot.**

---

# Befund 3 — Hostautomation bei read-only gehaltenem Zustand

## Befund wörtlich

> - [P2] Sperre Hostautomation für read-only gehaltene Zustände — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:1239-1242
>   DEFEKT (M-92): Nach dem Laden von `fremdes-major-3.bin` werden Transaktionen abgewiesen, Hostereignisse hier jedoch weiterhin übernommen. Werden anschließend `eq_enabled=true` und `input_trim_db=6` automatisiert, publiziert der nächste Kontrolltakt ein hörbares Programm, obwohl der Stand audio-neutral bleiben muss. Originalbytes bleiben zwar erhalten, passen aber nicht mehr zum hörbaren Verhalten. Der read-only-Riegel aus [CLAUDE.md:232–233](CLAUDE.md#L232-L233) und M-92 muss auch diesen Weg abdecken.

## Quellzitat an HEAD

Der Riegel existiert — aber nur im Transaktionsweg, `SondeProcessor.cpp:1014-1023`:

```cpp
    {
        const juce::ScopedLock l (zustandSchloss);
        if (zustand.nurLesen)
        {
            ergebnis.ausgang  = nakama::transaktion::Ausgang::fehler;
            ergebnis.revision = transaktion->revision();
            ergebnis.hash     = transaktion->hash();
            ergebnis.grund    = "schreibgeschuetzt";
            return ergebnis;
        }
```

Der Automationsweg kennt ihn nicht — `SondeProcessor.cpp:1227-1258` vollständig:

```cpp
void SondeProcessor::dspKontrollTakt()
{
    const juce::ScopedLock l (zustandSchloss);

    // §44.2: erst nach dem ACK des Audiothreads ist eine Bank wieder frei.
    dspKern->pflege();

    // Hostereignisse -> AutomationOverlay (M-81): keine Revision, kein Undo.
    if (hostEreignisOffen.exchange (false, std::memory_order_relaxed))
    {
        for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
        {
            const auto n = hostEreignis[(size_t) i].load (std::memory_order_relaxed);
            if (n == hostEreignisGesehen[(size_t) i]) continue;
            hostEreignisGesehen[(size_t) i] = n;
            transaktion->automationSchreiben (i, zelleAusHost (i, hostWert[(size_t) i].load (std::memory_order_relaxed)));
            samplesBeiLetzterAutomation = verarbeiteteSamples.load (std::memory_order_relaxed);
            publikationOffen = true;
        }
    }
    ...
    // Den wirksamen Zustand publizieren; bei busy_retry im naechsten Takt.
    if (publikationOffen && dspKern->samplerate() > 0.0)
        publikationOffen = ! dspAusfuehrung->publiziereWirksam (transaktion->wirksam(), false);
}
```

Dass „publizieren" hörbar heißt, zeigt `NakamaTransaktion.cpp:753-763`:

```cpp
bool DspKernAusfuehrung::publiziereWirksam (const param::DspSatz& wirksam, bool erzwingen)
{
    if (kern.samplerate() <= 0.0) return true;
    sichtWeg.werte = wirksam.werte;
    setzeFreieSlotsNeutral (sichtWeg.werte);
    if (! erzwingen && zuletztGueltig && werteGleich (sichtWeg.werte, zuletzt)) return true;
    if (! kern.uebernehmeZustand (sichtWeg, dsp::Pfad::committed)) return false;
    ...
}
```

Der read-only-Ladepfad selbst — `SondeProcessor.cpp:860-869` — lädt neutral,
setzt aber keinen Automationsriegel:

```cpp
        juce::String grund;
        const bool eigenerStand = ! zustand.nurLesen && zustand.hatParameters
            && transaktion->ladestart (zustand.dspDto(), (std::uint64_t) zustand.stateRevision,
                                       zustand.undoRing, zustand.undoCursor, grund);
        if (! eigenerStand)
        {
            const bool neutral = transaktion->ladestart (nakama::parameter::DspSatz {}, 0, {}, 0, grund);
            jassert (neutral);
            juce::ignoreUnused (neutral);
        }
```

## Zusage, gegen die der Befund gehalten wird

`docs/beweise/SONDE-015.md:1175`, M-92, Spalte Zusage, wörtlich:

> Er bleibt **read-only mit Originalbytes** wie heute: audio-neutral, keine
> Bindung, `getStateInformation` liefert die Bytes unverändert zurück. Der
> bestehende Zweig in `NakamaState.cpp` bleibt in seiner Wirkung unverändert;
> nur die Bedingung „`Dsp` vorhanden" fällt weg.

`CLAUDE.md:194-196`, wörtlich:

> **State bleibt verlustfrei.** Unbekannte Major-Versionen oder unzulässige
> Matrizen werden read-only mit Originalbytes gehalten; jede persistente
> Änderung meldet dem Host Dirty-State.

Und E4-15 (`SONDE-015.md:3955`), wörtlich: „Der Kern lädt neutral (`eq_enabled`
aus), der Stand bleibt mit Originalbytes gehalten, der Prozessor nimmt keine
Transaktion an (`schreibgeschuetzt`)".

## Status: **BESTÄTIGT**

Die Kette ist vollständig aus dem Code ableitbar. `zustand.nurLesen` wird im
Automationszweig nirgends geprüft (`grep -n nurLesen SondeProcessor.cpp` trifft
nur `:826` in `gehaltenerStand`, `:861`, `:924`, `:929` im Status und `:1016` im
Transaktionsweg). Der Topologiefilter greift nicht: `v2.global.eq_enabled` und
`v1.global.input_trim_db` sind beide `topologisch = false`
(`NakamaParameter.cpp:30`, `:79`), also legt `wirksam()` sie über den neutralen
bestätigten Zustand, und `publiziereWirksam` baut daraus ein engagiertes
Programm. „Audio-neutral" aus M-92 ist damit gebrochen; die Originalbytes
bleiben zwar erhalten (`gehaltenerStand:826` überspringt `schreibeIn`), aber
Klang und gespeicherter Stand laufen auseinander.

## Auslösekette

1. Message-Thread: Host ruft `setStateInformation` mit `fremdes-major-3.bin` →
   `zustand.nurLesen = true`, `ladestart` mit leerem `DspSatz{}` (neutral),
   `publiziereWirksam(..., erzwingen = true)` → Passthrough.
2. Audiothread **oder** Message-Thread: Hostautomation auf
   `v2.global.eq_enabled` und `v1.global.input_trim_db` →
   `parameterValueChanged` (`:1110`) → Mailbox.
3. Control-Worker, nächster Takt (≤ 5 ms): `dspKontrollTakt` schreibt beide in
   das Overlay (`:1242`), setzt `publikationOffen`, publiziert `wirksam()`
   (`:1256-1257`) → `DspKern::uebernehmeZustand` auf dem committed-Pfad.
4. Nächster Audioblock: hörbares Programm auf einem Stand, den das Plugin nicht
   versteht und dessen Bytes es unverändert zurückgeben muss.

Kein Gestus nötig, kein Editor nötig — **dieser Pfad ist heute scharf**, weil
Hostautomation über `processParameterChanges` vom Audiothread kommt.

## Vorschlag Einordnung: **DEFEKT**

Grund: verletzt M-92 („audio-neutral", `SONDE-015.md:1175`), E4-15 und
`CLAUDE.md:194-196`. Zusätzlich reißt die Gegenprobe zum Gate-Nachtrag: der
Gate-Text verlangt „Zustand und Automation" als **ein** Liefergegenstand; ein
Zustand, der Schreiben verweigert, aber Automation annimmt, hat zwei Wahrheiten.

## Existierender Test

`TransactionTestMain.cpp:2002-2021`
(`fremdes_major_laedt_neutral_und_bleibt_schreibgeschuetzt`, M-92
Prozessorhälfte) — prüft `zustandLesen().nurLesen`, neutralen bestätigten
Zustand, `Ausgang::fehler`/`grund == "schreibgeschuetzt"` und
`zurueck == fremd`. Der Test fährt **keine Hostautomation** und ruft
`kontrollTaktFuerTest()` nicht.

**Würde heute nicht rot.** Der Fall ist mit vier Zeilen erweiterbar
(`hostParam(...).setValueNotifyingHost(...)`, `kontrollTaktFuerTest()`,
`wirksamerZustand()` gegen `DspSatz{}`).

---

# Befund 4 — `setNonRealtime` nimmt `zustandSchloss` im Audiothread

## Befund wörtlich

> - [P2] Entferne die Zustandssperre aus dem Offline-Audiocallback — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:1104-1106
>   DEFEKT beim VST3-Offlinerendern: Der gebundene JUCE-Wrapper ruft `setNonRealtime` in `processAudio` vor jedem Audioblock auf, nicht nur bei der Vorbereitung. Diese neue Überschreibung nimmt dadurch im Audiothread `zustandSchloss` und kann auf den Control-Worker beziehungsweise dessen Programmbau warten. Das widerspricht dem Sperrverbot aus [CLAUDE.md:43–44](CLAUDE.md#L43-L44); der B7-Aufruf vor der Blockschleife deckt diesen tatsächlichen Aufrufpfad nicht ab.

## Quellzitat an HEAD

`eq-copilot/plugin/sonde/SondeProcessor.cpp:1101-1108`:

```cpp
void SondeProcessor::setNonRealtime (bool offline) noexcept
{
    juce::AudioProcessor::setNonRealtime (offline);
    if (! offline) return;
    const juce::ScopedLock l (zustandSchloss);
    transaktion->beendePreview();
    dspKern->setzeHoermatrix (nakama::dsp::Hoermatrix::processed);
}
```

Der gebundene Wrapper, `eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3880-3910`:

```cpp
    template <typename FloatType>
    void processAudio (Vst::ProcessData& data)
    {
        ClientRemappedBuffer<FloatType> remappedBuffer { bufferMapper, data };
        auto& buffer = remappedBuffer.buffer;
        ...
        {
            const ScopedLock sl (pluginInstance->getCallbackLock());

            pluginInstance->setNonRealtime (data.processMode == Vst::kOffline);
            ...
            else
                pluginInstance->processBlock (buffer, midiBuffer);
```

Zeile `:3892` ist genau die vom Urteil genannte Stelle, und sie liegt **in der
Blockschleife**, nicht in der Vorbereitung. Die zweite Aufrufstelle `:3605`
(`setupProcessing`) läuft auf dem Hostthread und ist unkritisch.

## Invariante

`CLAUDE.md:32-33`: „Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe-
oder Netzzugriffe, kein Logging; Überlast verwirft Analyseframes, nie Audio."
Matrix M-47 (`SONDE-015.md:1100`), Zusage: „Im Callback gibt es **keine
Sperre** …". Matrix M-120 (`:1224`), Zusage: „Offline-Render läuft mit dem
bestätigten Zustand, nie mit einer Vorschau."

## Status: **BESTÄTIGT**

Aus beiden Quellen wörtlich belegt. Zwei Präzisierungen:

1. **Nur im Offline-Betrieb.** `:1104` `if (! offline) return;` liegt **vor**
   dem `ScopedLock` — im Realtime-Betrieb nimmt der Audiothread die Sperre
   nicht. Der Befund trifft ausschließlich `processMode == kOffline`, und genau
   das sagt das Urteil auch („DEFEKT beim VST3-Offlinerendern").
2. **Kein Deadlock, aber unbegrenzte Wartezeit.** Die Sperrenordnung ist
   konsistent (`getCallbackLock()` → `zustandSchloss`, §0.3). Der Audiothread
   wartet aber auf `dspKontrollTakt` (`:1229-1258`, inklusive `pflege()` und
   Programmbau) oder auf `fuehreTransaktionAus` (`:1015`, inklusive SHA-256 und
   Programmbau). Beim Offline-Render gibt es keine Deadline — ob das die
   Invariante entschärft, ist eine Produkt-/Gateentscheidung des Dirigenten;
   der Wortlaut von CLAUDE.md unterscheidet nicht.

Zusatzbeobachtung: die Überschreibung wird **je Offline-Block** ausgeführt, also
auch `dspKern->setzeHoermatrix(processed)` je Block. Idempotent, aber es ist
zugleich der Grund, warum eine Verlagerung nach `prepareToPlay` nicht reicht:
der Wrapper meldet den Moduswechsel nur hier.

## Auslösekette

1. Host startet Offline-Render (Bounce/Export) → `data.processMode == kOffline`.
2. Audiothread: `processAudio` nimmt `getCallbackLock()` (`:3890`), ruft
   `setNonRealtime(true)` (`:3892`).
3. `SondeProcessor::setNonRealtime` (`:1105`) nimmt `zustandSchloss`.
4. Hält der Control-Worker gerade `zustandSchloss` (`dspKontrollTakt`), wartet
   der Audiothread bis zum Ende des Takts inklusive Bankbau.
   Umgekehrt gilt: der Worker kann in dieser Zeit nicht in `pflege()`, weil er
   selbst auf die Sperre wartet — eine gegenseitige Verzögerung je Block.
5. Danach `processBlock` (`:3909`).

## Vorschlag Einordnung: **DEFEKT**

Grund: verletzt `CLAUDE.md:32-33` und M-47 wörtlich. Die Zusage M-120
(„Offline-Render läuft mit dem bestätigten Zustand, nie mit einer Vorschau")
verlangt das Beenden der Vorschau, aber **nicht** unter `zustandSchloss` im
Callback — die Zusage ist ohne die Sperre erfüllbar (Flagge setzen, der
Control-Worker führt aus).

## Existierender Test

`TransactionTestMain.cpp:2201-2219`
(`offline_render_nutzt_den_bestaetigten_zustand`, M-120):

```cpp
        a->setNonRealtime (true); b->setNonRealtime (true);
        fahreAudio (*a, 8, 256, 51); fahreAudio (*b, 8, 256, 51);
```

Der Aufruf steht **einmal vor** den Blockschleifen; der Test bildet den
Wrapper-Pfad (je Block) nicht nach und misst keine Sperren.
`null_sperren_im_callback` liegt in B6 (`DspGoldenTestMain.cpp:2974`) und misst
den `DspKern`, nicht den Prozessor.

**Würde heute nicht rot.**

---

# Befund 5 — verworfene nicht-endliche Hostwerte werden nicht gezählt

## Befund wörtlich

> - [P2] Zähle verworfene nicht-endliche Hostwerte — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp:1188-1189
>   DEFEKT bei NaN/Inf-Hostautomation: Diese Wache gibt lediglich den bestätigten Wert zurück; weder hier noch am Mailboxeingang wird ein Fehler- oder Verwerfungszähler erhöht. Der vorhandene B7-Fall `nichtendlicher_hostwert_erreicht_das_programm_nie` speist beide Werte bei aktivem EQ ein, prüft aber nur Zustand und Endlichkeit. Damit fehlt der zugesagte zweite Teil „verriegelt und gezählt" aus [CLAUDE.md:226](CLAUDE.md#L226). E4-10 kann den Zustand unverändert lassen, ohne auf die Fehlerzählung zu verzichten.

## Quellzitat an HEAD

`eq-copilot/plugin/sonde/SondeProcessor.cpp:1174-1200`:

```cpp
nakama::parameter::Zelle SondeProcessor::zelleAusHost (int index, float normiert) const
{
    const auto& b = nakama::parameter::tabelle()[(size_t) index];
    const auto* p = hostParameter[(size_t) index];
    const auto& bestaetigt = transaktion->bestaetigt().werte[(size_t) index];

    // Gleicht der Hostwert dem bestaetigten Wert in Hostgenauigkeit, ist es
    // GENAU der bestaetigte Wert: ein float traegt q = 0.7071067811865476
    // nicht, und ein Rundungsrest waere eine erfundene Aenderung.
    if (p->convertTo0to1 (hostWertAus (index, bestaetigt)) == normiert)
        return bestaetigt;

    // Ein nicht-endlicher Hostwert erreicht nie Overlay, Gestus oder Programm:
    // er zaehlt als unveraendert (Manifest SONDE-015 §10.2, E4-10).
    if (! std::isfinite (normiert))
        return bestaetigt;
    ...
}
```

Gegenprobe, dass nirgends gezählt wird: `grep -n "isfinite|nichtEndlich|Zaehler"`
über `SondeProcessor.{h,cpp}` liefert als einzigen Treffer im Automationsweg
`:1188`; die vorhandenen Zähler sind `tidZaehler` (`:456`), die
Analyse-/Evidenzzähler (`:400-402`) und die Queue-Drops — keiner davon zählt
verworfene Hostwerte. Auch `parameterValueChanged` (`:1110-1118`) zählt nicht:
es speichert den NaN roh in `hostWert[i]` und erhöht nur `hostEreignis[i]`.

## Invariante und Vergleichszusage

`CLAUDE.md:189-190` (HEAD), wörtlich:

> **NaN-Ehrlichkeit.** Nicht-endliche Werte werden verriegelt und gezählt;
> Nyquist gekappt; ohne genügend endliche Nachbarn keine Basislinie.

Die Matrix kennt die Zählung nur für den **Audioeingang**, M-49
(`SONDE-015.md:1102`), Zusage wörtlich:

> Er wird **vor jedem Filterzustand** durch 0,0 ersetzt und gezählt; der Zähler
> ist lockfrei lesbar. […] ein zweiter Zähler erhöht.

Für den **Hostparameter** sagt der Entscheid E4-10 (`SONDE-015.md:3950`) wörtlich
nur: „Ein NaN- oder Inf-Wert eines Hostparameters zählt als unverändert: er
erreicht weder Overlay noch Gestus noch Programm" — **ohne Zähler**. M-14
(`:1052`) betrifft den DTO-Validator (S2), nicht den Automationsweg.

## Status: **BESTÄTIGT** (Sachverhalt), Einordnung strittig

Der Sachverhalt ist eindeutig: verriegelt wird, gezählt wird nicht. Strittig ist
nur, ob eine **Zusage** das verlangt. CLAUDE.md formuliert die Regel
unbedingt („Nicht-endliche Werte"), die Matrix führt sie aber nur für den
Audiopfad (M-49) aus, und E4-10 hat den Hostpfad ausdrücklich ohne Zähler
entschieden. Nach der Regel „Source und current evidence define implemented
behavior; Produktintent kommt aus der datierten Entscheidquelle" ist E4-10 ein
Bauentscheid, kein User-Entscheid — CLAUDE.md steht darüber.

## Auslösekette

1. Audiothread (oder Message-Thread): Hostautomation liefert NaN/±Inf →
   `parameterValueChanged` legt den Rohwert in `hostWert[i]`, erhöht
   `hostEreignis[i]`.
2. Control-Worker: `dspKontrollTakt` → `zelleAusHost` (`:1242`) → `:1188` →
   `return bestaetigt` — der Wert verschwindet spurlos. `automationSchreiben`
   sieht den bestätigten Wert, `zelleGleich` ist wahr, `gesetzt[i]` bleibt
   false. Kein Overlay, kein Programm, **kein Zähler**.
3. Gesten-Variante: `gestusAbschliessen` (`:1152`) ruft `zelleAusHost` ebenfalls
   — auch dort kein Zähler.
4. Folge: eine dauerhaft NaN liefernde Automationsspur ist am Plugin nicht
   diagnostizierbar; weder Status (`v3Status`) noch Bericht (`baueBericht`)
   melden etwas. Die Sonde sagt „alles in Ordnung", während jeder Punkt
   verworfen wird.

## Vorschlag Einordnung: **DEFEKT**

Grund: verletzt `CLAUDE.md:189-190` („verriegelt **und gezählt**"). Die
Verriegelung ist da, die Ehrlichkeit fehlt. Die Gegenposition wäre **LÜCKE** —
Matrix und Entwurf schweigen für den Hostpfad, E4-10 hat ohne Zähler
entschieden, und dann fehlte die Regel „jeder Eingang, der nicht-endliche Werte
verwirft, führt einen lockfrei lesbaren Zähler" als eigene Matrixzeile. Ich
schlage DEFEKT vor, weil CLAUDE.md-Invarianten laut Auftragsdefinition
qualifizierende Quelle sind und der Wortlaut nicht auf Audiosamples eingegrenzt
ist; bei Einordnung als LÜCKE wäre die Nacharbeit dieselbe (Zähler plus neue
Matrixzeile).

## Existierender Test

`TransactionTestMain.cpp:2129-2147`
(`nichtendlicher_hostwert_erreicht_das_programm_nie`, E4-10):

```cpp
        gainParam.setValueNotifyingHost (std::numeric_limits<float>::quiet_NaN());
        freqParam.setValueNotifyingHost (std::numeric_limits<float>::infinity());
        p->kontrollTaktFuerTest();
        const auto y = fahreAudio (*p, 40, 512, 61);
        const bool endlich = std::all_of (y.begin(), y.end(), [] (float v) { return std::isfinite (v); });
        const auto w = p->wirksamerZustand();
        pruefe (gain (w, 0) == 3.0 && freq (w, 0) == 1000.0 && p->stateRevision() == r && endlich, ...
```

Prüft Zustand, Revision und Endlichkeit des Ausgangs — **keinen Zähler**, weil
es keinen gibt.

**Würde heute nicht rot.**

---

# Befund 6 — M-125-Wachen laufen nicht nach jeder Eingabe

## Befund wörtlich

> - [P2] Prüfe die Registerinvarianten nach jeder Tabelleneingabe — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/TransactionTestMain.cpp:564-566
>   DEFEKT von M-125: Im 33-Commit-Vorlauf für T10/T11/T17 wird `fahre` wiederholt ohne anschließende Invariantenwache aufgerufen; auch T12 prüft seine 31 Abweisungen erst gesammelt danach. `fahre` selbst führt ebenfalls keine Wache aus. Zwischenzeitliche Verletzungen können dadurch bis zur nächsten Prüfung verschwinden. [M-125](docs/beweise/SONDE-015.md#L1154) verlangt I1/I2/I4 ausdrücklich „nach jeder Eingabe" und nennt ausschließlich abschließende Wachen selbst als Rotfall. Die Wachen müssen auch innerhalb dieser Vorläufe nach jedem Aufruf laufen.

## Quellzitat an HEAD

`eq-copilot/plugin/tests/TransactionTestMain.cpp:141-146` (`fahre` hat keine Wache):

```cpp
tx::Ergebnis fahre (Stand& st, Sitzung& s, const tx::Auftrag& a)
{
    auto e = st.tk->fuehreAus (a);
    if (e.ausgang == tx::Ausgang::commit) s.commits.push_back ({ a.tid, e.revision });
    return e;
}
```

`:553-568` (der vom Urteil benannte Vorlauf, 33 Eingaben ohne Wache):

```cpp
/** Vorlauf fuer T10, T11 und T17: 33 Commits in dieser Sitzung. `hashes[i]`
    ist der Hash nach dem Commit von `auftraege[i]` (e = r0 + i + 1). */
void dreiunddreissigCommits (Stand& st, Sitzung& s, std::vector<tx::Auftrag>& auftraege,
                             std::vector<tx::HashText>& hashes)
{
    auto z = mitEq (true);
    setzeBand (z, 0, 1000.0, 0.0);
    for (int i = 1; i <= 33; ++i)
    {
        z.werte[(size_t) iBand (0, param::kGainDb)].zahl = (double) (i % 24) - 12.0 + 0.25;
        auto a = apply (*st.tk, z, (std::uint64_t) i);
        fahre (st, s, a);
        auftraege.push_back (a);
        hashes.push_back (st.tk->hash());
    }
}
```

`:591-611` (T12: 31 Abweisungen, Wache erst gesammelt danach):

```cpp
    int abgewiesen = 0;
    for (int i = 0; i < 31; ++i)
    {
        auto veraltet = apply (*st.tk, mitGain (*st.tk, 1.0), (std::uint64_t) (1000 + i));
        veraltet.baseRevision = 1;
        if (fahre (st, s, veraltet).ausgang == tx::Ausgang::konflikt) ++abgewiesen;
    }
    pruefe (abgewiesen == 31, "T12 Vorlauf: 31 Abweisungen wegen veralteter base_revision", std::to_string (abgewiesen));
    wachen (*st.tk, s, "T12 Vorlauf");
```

`:581-589` (T11 lässt die Wache nach dem Vorlauf ganz weg):

```cpp
void fallT11()
{
    Stand st; Sitzung s; std::vector<tx::Auftrag> a; std::vector<tx::HashText> h;
    dreiunddreissigCommits (st, s, a, h);
    const auto e = fahre (st, s, a[1]);                          // e = r0 + 2, r - e = 31
```

## Zusage

`docs/beweise/SONDE-015.md:1154`, M-125, Spalte Zusage, wörtlich:

> Dazu drei Invarianten aus §5.11.4 Teil 3 als Wachen **nach jeder Eingabe und
> nach jedem Ladestart**, in ihrer Fassung mit `r0`: die Belegung (**I1**), die
> Fensterzugehörigkeit (**I2**) und „kein Registereintrag ohne Commit" (**I4**).

Spalte Rotbeweis, wörtlich: „… **oder die Invariantenwachen laufen nur am Ende
statt nach jeder Eingabe**; oder die Wache I1 rechnet ohne `r0` …"

## Status: **BESTÄTIGT**, und breiter als die zitierte Stelle

Gemessen an HEAD: **112 statische `fahre`-Aufrufstellen** gegen **27
`wachen`-Aufrufstellen**; durch die Schleifen (33er-Vorlauf viermal gerufen,
31er-Schleife in T12) ist das Laufzeitverhältnis noch deutlich ungünstiger. Über
die vom Urteil genannten Stellen hinaus fehlen Wachen auch:

- `fallT11` (`:584`) — nach dem 33-Commit-Vorlauf gar keine Wache.
- Abschnitt F, `:1008-1016` — `dreiunddreissigCommits`, ein Undo und ein
  **Ladestart** ohne Wache, obwohl M-125 „nach jedem Ladestart" ausdrücklich
  nennt. (Der Ladestart in T17, `:667-670`, hat eine Wache.)
- die Vorläufe von T7 bis T9 (`:479`, `:494`, `:497`, `:511`, `:514`, `:524`,
  `:527`, `:538`, `:541`) und `grundzustand` (`:265`).

Die Wachenfunktion selbst ist korrekt gebaut und rechnet mit `r0`
(`:149-179`) — der Mangel ist ausschließlich ihre Kadenz.

## Auslösekette

Kein Laufzeitthread — der Befund betrifft die Prüfstärke. Wirkung: ein Kern, der
I1/I2/I4 **während** des Fensterdurchlaufs bricht und sie bis zur nächsten
Sammelwache von selbst wiederherstellt (etwa ein kurzzeitig doppelt belegter
Registerplatz beim Verdrängen des 33. Eintrags, `NakamaTransaktion.cpp:691-695`),
bliebe unbemerkt. Genau der 33-Commit-Vorlauf ist der Bereich, in dem das
Register zum ersten Mal umläuft — also der Bereich mit dem höchsten Risiko und
der geringsten Prüfdichte.

## Vorschlag Einordnung: **DEFEKT**

Grund: verletzt M-125 (`SONDE-015.md:1154`) wörtlich, und der Bruch ist dort als
eigener Rotfall benannt. Er trifft den Test, nicht das Produkt — die Klasse ist
trotzdem DEFEKT, weil M-125 laut §10.6 A-7 ausdrücklich „eine Zusage an den
Test selbst" ist und der zugehörige Rotbeweis M-125b die Falltabelle des Tests
mutiert.

## Existierender Test

Es gibt keinen Meta-Test über die Wachenkadenz. Der Rotbeweis
`falltabelle_vollstaendig_gefahren` (`:699-710`) sichert nur, dass jede
Tabellenzeile genau einen Fall hat (`static_assert` `:696`), nicht die
Wachendichte.

**Der Befund kann sich definitionsgemäß nicht selbst rot machen.**

---

# Gemeinsame Ursache

Skillregel: „Haben Befunde eine gemeinsame Ursache, ist die Ursache der Auftrag,
nicht die Punktkorrektur."

## Ursache A — Der Hostereignis-Weg ist eine zweite, ungeregelte Eingangstür in den Zustand (Befunde 1, 3, 5)

Etappe 4a hat **einen** geregelten Weg in den bestätigten Zustand gebaut: die
Stufenfolge S0 bis S8 des Transaktionskerns, mit Validierung (S2), Zonenriegel
(S3), Revisionswache (S1/S4), Hash (S6) und Registerdisziplin (I4). Der
Automationsweg — `parameterValueChanged` → Mailbox (`hostWert`, `hostEreignis`)
→ `dspKontrollTakt` → `automationSchreiben` → `publiziereWirksam` — geht
**vollständig daran vorbei** und wird trotzdem hörbar, weil `publiziereWirksam`
das Overlay in den committed-Pfad des DSP-Kerns schreibt. Der Manifest-Nebenbefund
N-5 (`SONDE-015.md:4143`) benennt genau das: „der Automationsweg geht an S2
vorbei". Behoben wurde damals nur der eine Fall NaN; die **Klasse** blieb offen.

Was auf diesem Weg heute fehlt, ist je Befund ein anderes Stück derselben
Ordnung:

| Befund | Was der geregelte Weg hat | Was der Automationsweg nicht hat |
|---|---|---|
| 1 | Abweisung nimmt die Eingabe zurück | die Mailbox überlebt die Abweisung und den Ladestart |
| 3 | `if (zustand.nurLesen) → schreibgeschuetzt` (`:1016`) | kein read-only-Riegel in `dspKontrollTakt` |
| 5 | S2 validiert und meldet einen Grund | verwirft still, ohne Zähler und ohne Meldung |

Dieselbe Ursache zeigt sich auch als gerissene Paarregel: `setStateInformation`
leert `gesteOffen` und `gesteBeteiligt` (`:873-874`), aber nicht
`hostEreignis`/`hostEreignisGesehen`/`hostEreignisOffen` — von zwei
Eingangspostfächern wird nur eines beim Ladestart geschlossen.

**Vorschlag für den Auftrag:** nicht drei Punktkorrekturen, sondern eine Regel
für den Automationseingang — „ein Hostereignis durchläuft dieselben Riegel wie
eine Transaktion (read-only, Endlichkeit mit Zähler) und wird ungültig, sobald
der bestätigte Zustand sich unter ihm bewegt (Commit, Abweisung, Ladestart)."
Technisch reicht dafür eine Generationsmarke (Übernahme von
`hostEreignis[i]` nach `hostEreignisGesehen[i]` beim Abgleich und beim
Ladestart) plus ein `nurLesen`-Riegel und ein Verwurfszähler in
`dspKontrollTakt`/`zelleAusHost`. Befunde 1, 3 und 5 fallen damit in **einem**
Änderungssatz; die zugehörigen Matrixzeilen (M-67 Prozessorhälfte, M-92
Automationshälfte) und eine neue Zeile für den Verwurfszähler entstehen mit.

## Ursache B — `zustandSchloss` wird von Threads genommen, die nicht warten dürfen (Befunde 2, 4)

`zustandSchloss` ist laut `SondeProcessor.h:356` kommentiert als „nur
Nachrichten-/Hostthread, nie processBlock". Diese Zusage ist an zwei Stellen
gebrochen, beide über Fremdcode, den 4a nicht selbst ruft:

- Der VST3-Wrapper ruft `setNonRealtime` **je Block** im Audiothread
  (`:3892`) — Befund 4.
- JUCE hält `listenerLock` über `parameterGestureChanged`, und derselbe
  `listenerLock` liegt im Audiopfad des Wrappers
  (`processParameterChanges` → `setValueNotifyingHost`) — Befund 2.

In beiden Fällen ist die Wartezeit unbegrenzt, weil `zustandSchloss` die
gesamte schwere Arbeit umschließt: `dspKontrollTakt` hält es über `pflege()`
und Programmbau (`:1229-1257`), `fuehreTransaktionAus` über S0 bis S8
inklusive SHA-256 (`:1015-1033`). Die Sperre ist damit nicht „kurz" im Sinne
eines Echtzeitpfads — sie ist die längste im Prozessor.

Die gemeinsame Ursache ist nicht „eine Sperre zu viel", sondern: **der Prozessor
hat keinen Übergabepunkt, an dem ein fremdes Callback (Audiothread,
Listener-Callback) eine Absicht abgibt, ohne den Zustand selbst anzufassen.**
Für Hostwerte existiert dieser Punkt (die Mailbox aus Ursache A, `:1110-1118`,
ausdrücklich „nur Atomics, kein Schloss") — für Gesten und für den
Offline-Moduswechsel wurde er nicht gebaut.

**Vorschlag für den Auftrag:** die Mailbox auf beide Fälle ausdehnen — Gestus-
Ende und Offline-Wechsel setzen je ein Atomic, der Control-Worker führt beides
im nächsten Takt aus. Befunde 2 und 4 fallen damit in **einem** Änderungssatz,
ohne dass eine Zusage (M-82 „genau eine Revision", M-120 „nie mit einer
Vorschau") verändert werden muss.

## Ursache C — Befund 6 steht allein

Befund 6 teilt keine Ursache mit den anderen fünf: er betrifft die Kadenz der
Invariantenwachen im Test, nicht den Prozessor. Er ist die einzige reine
Prüfarbeit der sechs und kann unabhängig und verhaltensneutral laufen.

## Prüflücke, die alle fünf Produktbefunde erklärt

Vier der fünf Produktbefunde würden heute nicht rot (Befund 6 kann es nicht).
Der Grund ist einheitlich: B7 fährt den echten `SondeProcessor` in den
Abschnitten M bis O, aber

- immer **einthreadig** (`kontrollTaktFuerTest()` statt laufendem Worker),
- nie über den **Wrapper-Pfad** (`processParameterChanges`, `processAudio`),
- nie mit einem **Sperrenzähler** am Prozessor (der einzige liegt in B6 am
  `DspKern`, `DspGoldenTestMain.cpp:2974`),
- nie mit einem **abgewiesenen** Gestus,
- nie mit Hostautomation auf einem **read-only** Stand.

Diese fünf Löcher sind die Messseite derselben zwei Ursachen. Eine Nacharbeit,
die nur die Punkte korrigiert, ohne diese fünf Messwege zu ergänzen, liefert
wieder Zusagen ohne Rotbeweis.
