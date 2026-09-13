// FeatureEngine — Spektrum und Baender: aus Samples werden Welch-Frames.
//
// NAK-225/S25d (09.09.2026), Definitionen herausgeloest aus der Klasse in
// core/analysis/FeatureEngine.h. Inhalt:
//
//   verarbeiteSamples    Die Sampleschleife: K-Gewichtung, True-Peak, Stereo,
//                        Passagenfenster und das Fuellen beider FFT-Stufen.
//                        Sie bleibt EINE Funktion (Register NAK-235: keine
//                        lange Funktion wird in diesem Ticket zerlegt).
//   schiebeStufe         Ein Sample in eine Stufe schieben und bei vollem
//                        Fenster rechnen lassen.
//   rechneFenster        Fenstern, transformieren, Bandenergien bilden.
//   summeBereich         Energiesumme ueber einen Binbereich.
//   flussSchritt, medianDerHistorie, ereignisAblegen
//                        Spektralfluss, sein laufender Median und die
//                        Ereignisse, die daraus entstehen.
//
// NaN-Ehrlichkeit (CLAUDE.md): nicht-endliche Werte werden verriegelt und
// gezaehlt, Nyquist wird gekappt, und ohne genuegend endliche Nachbarn gibt es
// keine Basislinie. Diese Familie ist die Stelle, an der das zuerst greift.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

namespace nakama::analyse
{

/** NAK-283 F07 (§8.1 Feinheit 17): liegt `startSample` noch vor dem naechsten
    Sample, das `verarbeiteSamples` sehen wird?

    Nur dann trifft der Startmerker den Anfang. Vor dem ersten Block liegt
    jeder Anfang vorn. Ohne Zeitbeweis des letzten Blocks ist die Lage
    unbekannt; dann gilt der Anfang als erreicht - das Kurzzeitfenster faellt
    beim Binden wie vor NAK-283, und der Merker bleibt trotzdem scharf. Der
    Rand saettigt wie in `blockProjektSpanneGueltig` (M-17). */
inline bool FeatureEngine::passagenanfangNochVorn (std::int64_t startSample) const noexcept
{
    if (! habeVorigen)
        return true;
    if ((vorigerBlock.flags & echtzeit::kFlagZeitGueltig) == 0)
        return false;
    const std::int64_t kMax = std::numeric_limits<std::int64_t>::max();
    const auto n = (std::int64_t) vorigerBlock.sampleCount;
    const std::int64_t b0 = vorigerBlock.projectSampleStart;
    const std::int64_t naechstes = b0 > kMax - n ? kMax : b0 + n;
    return startSample >= naechstes;
}

inline void FeatureEngine::verarbeiteSamples (const echtzeit::StampedBlock& block, const float* daten) noexcept
{
    const int n = (int) block.sampleCount;
    const bool stereo = block.kanaele > 1;

    // Der Transportstempel beschreibt den GESAMTEN publizierten Rahmen,
    // nicht bloss den letzten Block, der seine Kadenzschwelle ueberschritt.
    evidenzContinuousBelegen (block);
    rahmenZeitBelegen (block);

    // ── SONDE-013 M-03/M-25: welcher Teil DIESES Blocks in der markierten
    //    Passage liegt ────────────────────────────────────────────────────
    //
    // 🔑 Ohne diesen Ausschnitt liefen `passageTruePeak`, der Headroomring,
    // das LRA-Histogramm und der Fingerprint seit der letzten
    // TRANSPORTgrenze. Eine leise Passage, die ohne Seek nach einem lauten
    // Abschnitt markiert wird, uebernahm damit dessen Spitze und
    // Verteilungen — M-03 und M-25 verlangen aber die Groessen ZWISCHEN
    // den Grenzen der markierten Passage.
    //
    // Ohne gesetztes Fenster bleibt es beim bisherigen Verhalten: dann ist
    // gar keine Passage markiert, und die Transportepoche IST das Fenster.
    // Das ist kein Rueckfall, sondern der Fall "der User hat nichts
    // markiert" — M-03 setzt "Passage liegt vor" ausdruecklich voraus.
    int passVon = 0, passBis = n;
    // Endet das Fenster IN diesem Block? Nur dann laeuft der Filter unten
    // aus (Luecke B09).
    bool fensterEndetHier = false;
    // 🔑 NAK-283 F07 (M-49 bis M-52, §8.1 Feinheit 17): das Gegenstueck am
    // ANFANG. Beginnt die Passage IN diesem Block? `passVon` allein kann das
    // nicht sagen: es ist blocklokal und in jedem Block, der ganz in der
    // Passage liegt, erneut 0. Deshalb gilt der Anfang nur, wenn der
    // Startmerker scharf ist UND der Ausschnitt genau am Startsample beginnt.
    bool fensterBeginntHier = false;
    if (passagenfenster.gesetzt)
    {
        passVon = passBis = 0;
        if (! passagenfensterGebrochen
            && (block.flags & echtzeit::kFlagZeitGueltig) != 0)
        {
            const std::int64_t b0 = block.projectSampleStart;
            // ⚠️ M-17: der Blockrand SAETTIGT. `b0 + n` kann am oberen
            // i64-Rand ueberlaufen, und ein uebergelaufener Rand ergaebe
            // ein Fenster, das vor sich selbst endet — der Ausschnitt
            // waere dann leer, obwohl der Block mitten in der Passage
            // liegt. Dieselbe Regel wie in `blockProjektSpanneGueltig`
            // daneben, nur an dieser zweiten Rechnung.
            const std::int64_t kMax = std::numeric_limits<std::int64_t>::max();
            const std::int64_t bEnde = b0 > kMax - (std::int64_t) n
                                         ? kMax : b0 + (std::int64_t) n;
            const std::int64_t von = std::max (passagenfenster.startSample, b0);
            const std::int64_t bis = std::min (passagenfenster.endeSample, bEnde);
            if (bis > von)
            {
                passVon = (int) (von - b0);
                passBis = (int) (bis - b0);
                // `bis` ist bereits auf `endeSample` gekappt: Gleichheit
                // heisst, dass die Passage genau hier zu Ende ist.
                fensterEndetHier = bis >= passagenfenster.endeSample;
                // `von` ist bereits auf `startSample` angehoben: Gleichheit
                // heisst, dass die Passage genau hier beginnt.
                fensterBeginntHier = passagenStartScharf
                                  && von == passagenfenster.startSample;
            }
            // Liegt der Anfang schon hinter diesem Block, ohne dass ein Block
            // ihn getroffen hat, feuert der Merker nie mehr (Feinheit 17):
            // die Passage hat in dieser Epoche keinen beobachteten Anfang,
            // und `hatSampleAusserhalb` traegt das bereits.
            if (passagenStartScharf && b0 > passagenfenster.startSample)
                passagenStartScharf = false;
        }
    }

    for (int i = 0; i < n; ++i)
    {
        const bool imPassagenfenster = i >= passVon && i < passBis;
        // 🔑 NAK-283 F07 (M-49 bis M-52, R-283-5): DER ANFANG IST EIN SAMPLE,
        // NICHT DER BINDEZEITPUNKT.
        //
        // Ein Passagenanfang IST eine Fenstergrenze (§32.3): die 24 Taps je
        // Phase des Polyphasenfilters reichen zwoelf Samples vor den Anfang
        // zurueck. Bis NAK-283 wurde der Filter beim BINDEN geleert; wurde eine
        // spaetere Passage vorab gebunden, fuellte das Material davor ihn
        // wieder, und der erste Passagenrahmen trug dessen Nachklang - bei
        // einer lauten Kante vor einer leisen Passage 60 dB daneben (Befund
        // F07). Hier laeuft der Anfang an der exakten Grenze, VOR dem ersten
        // Passagensample, und genau einmal je Passage: der Merker faellt danach.
        //
        // Reihenfolge wie am Ende (unten), andere Zuordnung (M-51): am Ende
        // zaehlt der Rest des Filters fuer Rahmen und Passage, am Anfang NUR
        // fuer den Rahmen - er gehoert zum Material davor, und die Live-Metrik
        // verliert ihn nicht.
        //
        // Mit dem Filter beginnt das Kurzzeitfenster (SONDE-013 R04) an
        // derselben Grenze: seine 3-s-Zellen gehoeren zur Passage, nicht zum
        // Material davor (M-52).
        if (fensterBeginntHier && i == passVon)
        {
            const double rest = tp.nachlauf();
            rahmenTruePeak = std::max (rahmenTruePeak, rest);
            tp.zuruecksetzen();
            kurzfensterNeuBeginnen();
            passagenStartScharf = false;
            fensterBeginntHier = false;         // genau EINMAL je Passage
        }
        // 🔑 SONDE-013 Nacharbeit 2 (Befund R05): DER NACHLAUF LAEUFT AM
        // INDEX `passBis`, nicht nach der Schleife.
        //
        // Der Interpolator sieht ein Sample erst `kTapsJePhase / 2` Samples
        // spaeter vollstaendig. Endete die Passage mindestens zwoelf Samples
        // VOR dem Blockende, verarbeitete die Runde 1 zuerst alle
        // nachfolgenden Samples mit `imPassagenfenster = false` — dabei
        // wurden die verzoegerten Ausgaenge der letzten Passagensamples
        // bereits verbraucht und dem Passagenmaximum NICHT zugeschlagen. Der
        // Nachlauf nach der Schleife kam dafuer zu spaet und mischte
        // zusaetzlich Post-Material ein: er lief ueber einen Filter, der
        // inzwischen Samples NACH der Passage trug.
        //
        // Hier laeuft er an der exakten Grenze, VOR dem ersten
        // Post-Sample. Danach ist die Verzoegerungskette leer, und das
        // Material danach beginnt sauber.
        if (fensterEndetHier && i == passBis)
        {
            const double rest = tp.nachlauf();
            rahmenTruePeak = std::max (rahmenTruePeak, rest);
            passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, rest);
            fensterEndetHier = false;   // genau EINMAL je Block
        }
        ++verarbeiteteSamples;
        if (! imPassagenfenster)
        {
            hatSampleAusserhalb = true;
            letztesSampleAusserhalb = verarbeiteteSamples;
        }
        double l = (double) daten[(std::size_t) i * 2u];
        double r = (double) daten[(std::size_t) i * 2u + 1u];
        // NaN-Riegel: nicht-endliche Eingangswerte werden VOR jeder Rechnung
        // durch Stille ersetzt.  Das Audio sieht die Engine ohnehin nie.
        //
        // 🔑 SONDE-013 M-07: sie werden dabei GEZAEHLT und VERRIEGELT.
        // Die stille Ersetzung allein war der Fehler: ein Rahmen mit
        // beschaedigten Samples rechnete danach ueber Stille und sah
        // aus wie eine saubere Messung — dieselbe Zahl, dieselben
        // Praesenzbits, kein Zaehler. CLAUDE.md sagt dazu ausdruecklich
        // "Nicht-endliche Werte werden verriegelt UND gezaehlt"; die
        // Verriegelung wirkt ueber `rahmenNichtEndlich` (Praesenzbits
        // des Rahmens fallen) und `evidenzNichtEndlich` (Konfidenz des
        // Belegs faellt auf `unbrauchbar`).
        if (! std::isfinite (l) || ! std::isfinite (r))
        {
            if (! std::isfinite (l)) l = 0.0;
            if (! std::isfinite (r)) r = 0.0;
            if (rahmenNichtEndlich < 0xFFFFFFFFu) ++rahmenNichtEndlich;
            if (evidenzNichtEndlich < 0xFFFFFFFFu) ++evidenzNichtEndlich;
            ++nichtEndlicheSamplesGesamt;
            // NAK-283 F10 (R-283-5): WO ersetzt wurde, nicht wie oft - daraus
            // entsteht die Marke der Zelle, die dieses Sample traegt
            // (`zelleSchliessen`). Die zwei Zaehler oben saettigen und fallen
            // mit Rahmen und Evidenzfenster; die Stelle tut beides nicht.
            letztesErsetztesSample = verarbeiteteSamples;
        }
        if (! stereo) r = l;

        const double m = 0.5 * (l + r);
        const double s = 0.5 * (l - r);

        schiebeStufe (bass, m, s, block, i);
        schiebeStufe (haupt, m, s, block, i);

        // Loudness: K-Gewichtung je Kanal, Energie in 100-ms-Zellen.
        //
        // ⚠️ BS.1770 SUMMIERT die Kanäle mit ihrem Gewicht (G_L = G_R = 1),
        // es mittelt sie NICHT: z = Σ_i G_i · mean(y_i²).  Ein Mono-Signal
        // trägt deshalb genau EINEN Summanden — dieselbe Wellenform auf
        // beiden Kanälen ist nach der Norm 3 dB lauter als auf einem, und
        // das ist kein Rechenfehler, sondern die Aussage.  Wer hier durch
        // die Kanalzahl teilt, bekommt für Stereo eine um 3,01 LU zu leise
        // Zahl, die trotzdem völlig plausibel aussieht.
        const double kl = kL.tick (l);
        zelleKEnergie += kl * kl;
        if (stereo)
        {
            const double kr = kR.tick (r);
            zelleKEnergie += kr * kr;
        }
        zelleAktivEnergie += 0.5 * (l * l + r * r);

        // Peak/Crest/Stereo im laufenden Rahmen.
        const double absL = std::abs (l), absR = std::abs (r);
        rahmenPeak = std::max (rahmenPeak, std::max (absL, absR));
        // SONDE-013 M-02: der True Peak liegt ZWISCHEN den Samples, also
        // wird hier ueberabgetastet und dort maximiert.  `tick` ist
        // O(kTaps) mit fester Schleife, ohne Allokation und ohne Sperre —
        // dieselbe Klasse Aufwand wie die zwei K-Filter darueber.
        //
        // ⚠️ Der Abtastpunkt SELBST geht mit ein, und das ist keine
        // Beschoenigung, sondern die Definition: der True Peak ist das
        // Maximum der rekonstruierten Wellenform, und die Abtastpunkte
        // gehoeren zu ihr.  Ohne diese Zeile faellt der Wert an einer
        // konkreten Stelle unter den Sample-Peak — der Interpolatorkern
        // ist um seine halbe Laenge zentriert, sieht ein Sample also erst
        // `kTapsJePhase / 2` Samples spaeter, und ein Spitzenwert in den
        // letzten zwoelf Samples eines Rahmens erreicht `rahmenTruePeak`
        // erst im naechsten.  Der Frame truege dann einen True Peak unter
        // seinem eigenen Sample-Peak, und BEIDE Leser lehnten ihn als
        // `true_peak_unter_sample_peak` ab — zu Recht.
        //
        // Die Korrektur steht bewusst NUR hier.  Sie an den drei
        // Verbrauchsstellen zu wiederholen waere eine zweite Wahrheit
        // ueber dieselbe Zahl.  (Fund aus dem Selbstaudit dieser Etappe,
        // gemessen von B18::`impuls_am_rahmenende`.)
        const double tpJetzt = std::max (tp.tick (l, r), std::max (absL, absR));
        rahmenTruePeak = std::max (rahmenTruePeak, tpJetzt);
        // M-03/M-25: das PASSAGENmaximum zaehlt nur, was im Fenster lag.
        // Ohne Fenster ist `imPassagenfenster` fuer jedes Sample wahr, und
        // die zwei Groessen sind identisch.
        if (imPassagenfenster)
        {
            passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, tpJetzt);
            ++zelleImFensterSamples;
        }
        zelleTruePeak  = std::max (zelleTruePeak, tpJetzt);
        zelleRmsEnergie += 0.5 * (l * l + r * r);
        // SONDE-013 M-08: der Mono-Folddown wird am WIRKLICH gefalteten
        // Puffer gemessen (§40.3), nicht aus einer Korrelation
        // geschaetzt. Er gehoert deshalb hierher und nicht zu den
        // Spektren.
        stereoSample (l, r);
        rahmenSummeQuadrat += 0.5 * (l * l + r * r);
        ++rahmenSamples;
        rahmenMid2 += m * m;
        rahmenSide2 += s * s;
        rahmenL += l; rahmenR += r;
        rahmenL2 += l * l; rahmenR2 += r * r; rahmenLR += l * r;

        if (++zelleStand >= zellenSamples)
        {
            zelleSchliessen();
            zelleStand = 0;
            zelleKEnergie = 0.0;
            zelleAktivEnergie = 0.0;
            zelleTruePeak = 0.0;
            zelleRmsEnergie = 0.0;
            zelleImFensterSamples = 0;
        }
    }

    // ── SONDE-013 M-02/M-25, Luecke B09: der Nachlauf des Polyphasen-
    //    filters am Fensterende ──────────────────────────────────────────
    //
    // Der Interpolator sieht ein Sample erst `kTapsJePhase / 2` Samples
    // spaeter vollstaendig. Endete die Passage, waren die Zwischenwerte der
    // letzten zwoelf Samples deshalb noch in der Verzoegerungskette und
    // gingen mit dem naechsten `zuruecksetzen()` verloren: ein
    // Intersample-Peak am Passagenende fehlte im Passagen-True-Peak.
    //
    // Der Nachlauf schiebt Stille nach und wertet die dabei
    // herausfallenden Ausgaenge aus. Eine FRAMEGENAUE Zuordnung innerhalb
    // des Fensters wird damit nicht zugesagt — der Rest landet im
    // Passagenmaximum, und genau das ist die Groesse, um die es geht.
    //
    // 🔑 Nacharbeit 2 (Befund R05): endet das Fenster MITTEN im Block,
    // hat die Schleife oben ihn bei `i == passBis` schon gefahren und
    // `fensterEndetHier` dabei geloescht. Diese Stelle traegt nur noch den
    // Fall `passBis == n` — das Fenster endet mit dem Block, und es gibt
    // kein Post-Material, das der Nachlauf einmischen koennte.
    if (fensterEndetHier)
    {
        const double rest = tp.nachlauf();
        rahmenTruePeak = std::max (rahmenTruePeak, rest);
        passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, rest);
    }

    liveSamples    += (std::uint64_t) n;
    evidenzSamples += (std::uint64_t) n;
}

inline void FeatureEngine::schiebeStufe (Stufe& s, double m, double side,
                                         const echtzeit::StampedBlock& block, int sampleOffset) noexcept
{
    if (s.gefuellt == 0)
        s.fensterStromStart = block.stromVon + static_cast<std::uint64_t> (sampleOffset);
    const bool projektGueltig = blockProjektSpanneGueltig (block);
    const bool continuousGueltig = blockContinuousSpanneGueltig (block);
    const auto index = (std::size_t) s.gefuellt;
    s.ringM[(std::size_t) s.gefuellt] = m;
    s.ringS[(std::size_t) s.gefuellt] = side;
    s.ringProjektGueltig[index] = projektGueltig ? 1u : 0u;
    s.ringProjekt[index] = projektGueltig
        ? block.projectSampleStart + static_cast<std::int64_t> (sampleOffset)
        : 0;
    s.ringContinuousGueltig[index] = continuousGueltig ? 1u : 0u;
    s.ringContinuous[index] = continuousGueltig
        ? block.continuousTimeSamples + static_cast<std::int64_t> (sampleOffset)
        : 0;
    if (++s.gefuellt < s.punkte)
        return;

    // Der Fensterstart ist nur dann Projektzeit, wenn JEDES Sample im Ring
    // dieselbe fortlaufende Achse belegt.  Das ist absichtlich O(N) pro
    // FFT-Fenster und laeuft auf dem Worker; die FFT selbst ist teurer und
    // der Beweis darf nicht aus einer Host-Heuristik bestehen.
    s.fensterProjektGueltig = s.ringProjektGueltig[0] != 0;
    if (s.fensterProjektGueltig)
    {
        s.fensterProjektStart = s.ringProjekt[0];
        for (int i = 1; i < s.punkte; ++i)
        {
            const auto vorher = s.ringProjekt[(std::size_t) (i - 1)];
            if (s.ringProjektGueltig[(std::size_t) i] == 0
                || vorher == std::numeric_limits<std::int64_t>::max()
                || s.ringProjekt[(std::size_t) i] != vorher + 1)
            {
                s.fensterProjektGueltig = false;
                break;
            }
        }
    }

    s.fensterContinuousGueltig = s.ringContinuousGueltig[0] != 0;
    if (s.fensterContinuousGueltig)
    {
        s.fensterContinuousStart = s.ringContinuous[0];
        for (int i = 1; i < s.punkte; ++i)
        {
            const auto vorher = s.ringContinuous[(std::size_t) (i - 1)];
            if (s.ringContinuousGueltig[(std::size_t) i] == 0
                || vorher == std::numeric_limits<std::int64_t>::max()
                || s.ringContinuous[(std::size_t) i] != vorher + 1)
            {
                s.fensterContinuousGueltig = false;
                break;
            }
        }
    }

    rechneFenster (s);

    // 50 % Ueberlappung: die zweite Haelfte wird die erste.
    for (int i = 0; i < s.punkte - s.hop; ++i)
    {
        s.ringM[(std::size_t) i] = s.ringM[(std::size_t) (i + s.hop)];
        s.ringS[(std::size_t) i] = s.ringS[(std::size_t) (i + s.hop)];
        s.ringProjekt[(std::size_t) i] = s.ringProjekt[(std::size_t) (i + s.hop)];
        s.ringProjektGueltig[(std::size_t) i]
            = s.ringProjektGueltig[(std::size_t) (i + s.hop)];
        s.ringContinuous[(std::size_t) i]
            = s.ringContinuous[(std::size_t) (i + s.hop)];
        s.ringContinuousGueltig[(std::size_t) i]
            = s.ringContinuousGueltig[(std::size_t) (i + s.hop)];
    }
    s.gefuellt = s.punkte - s.hop;
    s.fensterStromStart += static_cast<std::uint64_t> (s.hop);
    // Bis zum naechsten vollen Ring gibt es noch keinen neuen Beweis.
    s.fensterProjektGueltig = false;
    s.fensterProjektStart = 0;
    s.fensterContinuousGueltig = false;
    s.fensterContinuousStart = 0;
}

inline void FeatureEngine::rechneFenster (Stufe& s) noexcept
{
    // Mid.
    for (int i = 0; i < s.punkte; ++i)
        s.arbeit[(std::size_t) i] = s.ringM[(std::size_t) i] * s.fenster[(std::size_t) i];
    s.fftM.transformiere (s.arbeit.data());
    // Side.
    for (int i = 0; i < s.punkte; ++i)
        s.arbeit[(std::size_t) i] = s.ringS[(std::size_t) i] * s.fenster[(std::size_t) i];
    s.fftS.transformiere (s.arbeit.data());

    // Kanalenergie-Mittel: (|L|² + |R|²)/2 = |M|² + |S|².  Ein reiner
    // Mono-Mix loeschte Antiphase komplett aus (Uebergabe-Paket B) — dieselbe
    // Falle, die `AnalyseEngine` im Kopf beschreibt.
    const double norm = 2.0 / (s.fs * s.fensterEnergie);
    const int bins = s.punkte / 2 + 1;
    for (int k = 0; k < bins; ++k)
        s.psd[(std::size_t) k] = (s.fftM.leistung (k) + s.fftS.leistung (k)) * norm;

    // 🔑 NAK-283 F06 (R-283-4, §8.1 Feinheiten 5 und 6): ENERGIE GEGEN ENERGIE.
    //
    // `s.psd` ist eine einseitige Leistungsdichte (Amplitude^2 je Hz). Das
    // Aktivgate `kAktivGateDb` ist eine Energie - dieselbe Groesse, die das
    // Zeitbereichsgate in `zelleSchliessen` gegen dieselbe Konstante haelt.
    // Deshalb wird die Dichtesumme HIER ueber die Binbreite Δf = fs / punkte
    // integriert, bevor sie in dB gegen die Schwelle steht. Ohne das lag die
    // Summe um 10*log10 (fs / punkte) unter dem Energiewert, und die Schwelle
    // wanderte mit Samplerate und Stufe - 10,32 dB bei 44,1 kHz in der
    // Hauptstufe, 16,71 dB bei 192 kHz, 6,02 dB zwischen den Stufen -, bis
    // ein Frame Aktivitaet 1 bei Abdeckung 0 meldete.
    //
    // Die Integration steht am GATE, nicht an `s.psd`: Bandakkumulation,
    // Fluss und Stereo lesen die Dichte unveraendert weiter. Δf entsteht je
    // Stufe aus ihrer eigenen Punktzahl, die Schwelle bleibt -60,0. Die
    // Nullpruefung steht vor dem Logarithmus und bleibt: digitale Stille
    // ergibt 0,0, und `log10 (0)` wird nie gerechnet.
    const double binBreiteHz = s.fs / (double) s.punkte;
    const double gesamt = summeBereich (s, 0, bins) * binBreiteHz;
    const bool aktiv = gesamt > 0.0
                    && 10.0 * std::log10 (gesamt) > kAktivGateDb;

    // SONDE-013 M-05: die Abdeckung des Evidenzfensters ist der Anteil
    // AKTIVER Fenster an ALLEN. Deshalb wird hier gezaehlt, VOR dem
    // Ruecksprung — ein Zaehler hinter dem `return` saehe nie ein stilles
    // Fenster und meldete jede Passage als vollstaendig abgedeckt.
    //
    // Gezaehlt wird nur die HAUPTstufe. Die Bassstufe hat ein anderes
    // Fenster- und Hopmass; beide zusammen zu zaehlen ergaebe ein
    // Verhaeltnis aus zwei verschiedenen Zeitachsen. Dieselbe Begruendung
    // steht beim Ereignisdetektor weiter unten.
    if (&s == &haupt)
    {
        // Saettigend statt umlaufend: ein Umlauf machte die Abdeckung
        // schlagartig zu einer Zahl ueber 1 oder zu 0 — beides saehe wie
        // eine Messung aus. Beide Zaehler werden bei jedem Evidenzframe
        // geleert, der Fall ist also theoretisch; er wird trotzdem
        // behandelt, weil eine stehengebliebene Grenze ehrlicher ist als
        // ein Sprung (CLAUDE.md, Zahlenraender).
        constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
        if (evidenzFensterGesamt < kMax) ++evidenzFensterGesamt;
        if (aktiv && evidenzFensterAktiv < kMax) ++evidenzFensterAktiv;
    }

    // Nur AKTIVE Fenster gehen in die Bandakkus.  Stille wuerde den
    // Mittelwert zu einer Aussage ueber die Pausen machen.
    if (! aktiv)
        return;

    bool hatBandBeitrag = false;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const int von = s.bandVon[(std::size_t) b];
        const int bis = s.bandBis[(std::size_t) b];
        if (bis <= von)
            continue;                       // Stufe nicht zustaendig / nicht messbar

        double energie = 0.0, seite = 0.0;
        for (int k = von; k < bis; ++k)
        {
            energie += s.psd[(std::size_t) k];
            seite   += s.fftS.leistung (k) * (2.0 / (s.fs * s.fensterEnergie));
        }
        energie /= (double) (bis - von);
        seite   /= (double) (bis - von);

        // §33.1: LINEAR integrieren, erst danach in dB.
        liveAkku[(std::size_t) b].summe += energie;
        ++liveAkku[(std::size_t) b].n;
        evidenzAkku[(std::size_t) b].summe += energie;
        ++evidenzAkku[(std::size_t) b].n;

        // SONDE-013 M-05: fuer P10/P50/P95 zaehlt der VERLAUF, nicht die
        // Summe. Der Ring nimmt den dB-Wert DIESES Fensters.
        //
        // Hier steht ABSICHTLICH keine Stufenbedingung, anders als beim
        // Fensterzaehler oben: die beiden Stufen teilen sich die Baender
        // ueberschneidungsfrei (`bandVon`/`bandBis`), jedes Band wird also
        // von genau einer bedient. Ein `&s == &haupt` liesse die
        // Bassbaender leer — eine Verteilung, die genau dort fehlt, wo die
        // Bassstufe ueberhaupt existiert.
        //
        // Nichtendliches oder unplausibles kommt gar nicht erst hinein: ein
        // Perzentil ueber einen NaN waere kein kleiner Fehler, sondern ein
        // NaN im ganzen Band.
        const double db = energieAlsDb (energie);
        if (plausibel (db))
            evidenzVerteilung[(std::size_t) b].schiebe ((float) db);
        liveBreiteAkku[(std::size_t) b].seite += seite;
        liveBreiteAkku[(std::size_t) b].gesamt += energie;
        hatBandBeitrag = true;
    }

    if (hatBandBeitrag)
    {
        supportMerken (liveSupport, s);
        supportMerken (evidenzSupport, s);
    }

    // Ereignisse nur aus der HAUPTstufe: sie hat die zeitliche Aufloesung
    // (85 ms Hop gegen 341 ms).  Ein Onset aus dem Bassfenster waere ein
    // Ereignis mit einer Dauer, die groesser ist als der Abstand zweier
    // Ereignisse — das ist keine Detektion mehr.
    // SONDE-013 M-11: die Kreuzspektren entstehen in BEIDEN Stufen.
    //
    // 🔑 Das war beim ersten Bau falsch und der Golden hat es gefunden:
    // die Stereozeile lief nur in der Hauptstufe, und die ist erst ab
    // `kTrennungHz` = 200 Hz zustaendig. Alle Baender darunter — bei
    // 48 kHz die unteren 60 von 221 — trugen KEINE Stereoevidenz, obwohl
    // M-11 sie fuer alle 221 verlangt. Ein 100-Hz-Mono-Signal meldete
    // dort schlicht nichts.
    //
    // Dass die zwei Stufen verschiedene Fensterlaengen haben (341 ms
    // gegen 85 ms bei 48 kHz), ist dabei kein Problem, sondern der Grund
    // fuer `fenster_dauer_ms` und `freiheitsgrade` JE BAND: §40.1
    // verlangt beide ausdruecklich als Teil der Evidenz, und das Schema
    // sagt am Feld "je Band, weil die Fensterlaenge ueber die Baender
    // nicht gleich sein muss". Jede Stufe fuellt nur ihre eigenen
    // Baender (`bandVon`/`bandBis` sind je Stufe zugeordnet), also
    // mischen sie sich nicht.
    stereoSchritt (s, &s == &haupt);

    if (&s == &haupt)
    {
        // Reihenfolge: erst Stereo, dann Fluss. `flussSchritt` schreibt
        // `vorigesSpektrum` fort und ist damit destruktiv fuer die Frage
        // "wie sah das Spektrum in DIESEM Fenster aus".
        flussSchritt (s);
    }
}

inline double FeatureEngine::summeBereich (const Stufe& s, int von, int bis) noexcept
{
    double su = 0.0;
    for (int k = von; k < bis; ++k)
        su += s.psd[(std::size_t) k];
    return su;
}

//== Ereignisse ===========================================================

/** SuperFlux-artiger spektraler Fluss (§39.1) mit adaptiver Median/MAD-
    Schwelle, plus einfachem Peakpfad als Gegenbeleg.

    🔑 Der Vorgaenger (`vorigesSpektrum`) ist genau die Groesse, die eine
    Grenze ueberbruecken WUERDE, wenn man ihn stehen liesse — und zwar
    unsichtbar, weil kein Puffer dabei waechst.  `grenzeZiehen()` setzt ihn
    deshalb ungueltig.  Der Golden prueft das eigens: ein Fluss, der ueber
    einen Seek hinweg gerechnet wird, meldet einen Onset, den es nicht gab. */
inline void FeatureEngine::flussSchritt (Stufe& s) noexcept
{
    double fluss = 0.0, zentrumZaehler = 0.0, zentrumNenner = 0.0;
    const bool hatteVorgaenger = vorigesSpektrumGueltig;

    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const int von = s.bandVon[(std::size_t) b];
        const int bis = s.bandBis[(std::size_t) b];
        double energie = 0.0;
        if (bis > von)
        {
            for (int k = von; k < bis; ++k)
                energie += s.psd[(std::size_t) k];
            energie /= (double) (bis - von);
        }
        const double logJetzt = energie > 0.0 ? std::log10 (energie) : -30.0;
        if (hatteVorgaenger)
        {
            const double delta = logJetzt - vorigesSpektrum[(std::size_t) b];
            if (delta > 0.0)
            {
                fluss += delta;
                zentrumZaehler += delta * Gitter::evidenzMitte (b);
                zentrumNenner  += delta;
            }
        }
        vorigesSpektrum[(std::size_t) b] = logJetzt;
    }
    vorigesSpektrumGueltig = true;
    if (! hatteVorgaenger)
        return;                     // erster Rahmen nach einer Grenze: kein Fluss

    // SONDE-013 M-26: der Fingerprint bekommt DENSELBEN Fluss wie der
    // Detektor - ohne dessen Schwelle. Hier zaehlt der Verlauf, nicht das
    // Ereignis; ihn zweimal zu rechnen waere zwei Wahrheiten ueber
    // dieselbe Groesse.
    //
    // M-25/M-27: bei gesetztem Passagenfenster zaehlt nur ein Analysefenster,
    // dessen SAEMTLICHE Samples in der Passage lagen. `s.punkte` ist seine
    // Laenge; liegt das juengste Sample ausserhalb weiter als diese Laenge
    // zurueck, war das Fenster sauber. Ein Fingerprint aus einem Fenster,
    // das die Passagengrenze ueberlappt, beschriebe zwei Stellen der Musik.
    const bool fpFensterSauber =
        ! passagenfenster.gesetzt
        || ! hatSampleAusserhalb
        || verarbeiteteSamples - letztesSampleAusserhalb
               > (std::uint64_t) s.punkte;
    if (fpFensterSauber)
        fingerprintSchritt (s, fluss);

    // Adaptive Schwelle: Median + 3·MAD ueber die Historie.  Erst ab voller
    // Historie — eine Schwelle aus drei Werten ist keine Schwelle, und ein
    // Detektor, der am Anfang jeder Epoche wild feuert, waere genau das
    // Gegenteil dessen, was dieses Ticket verspricht.
    if (flussGefuellt >= kFlussHistorie)
    {
        const double med = medianDerHistorie();
        double madSumme = 0.0;
        for (int i = 0; i < kFlussHistorie; ++i)
            madSumme += std::abs (flussHistorie[(std::size_t) i] - med);
        const double mad = madSumme / (double) kFlussHistorie;
        const double schwelle = med + 3.0 * mad;

        const double peakDb = rahmenPeak > 0.0 ? 20.0 * std::log10 (rahmenPeak) : -200.0;
        const double rms = rahmenSamples > 0
            ? std::sqrt (rahmenSummeQuadrat / (double) rahmenSamples) : 0.0;
        const double rmsDb = rms > 0.0 ? 20.0 * std::log10 (rms) : -200.0;
        const double crestDb = peakDb - rmsDb;

        // ── SONDE-013 M-86: der EIGENE Peakpfad ──────────────────────
        //
        // §39.1 verlangt den Detektor aus spektralem Fluss, Peaksteigung
        // UND Crest, und ausdruecklich "einen einfachen Peakpfad als
        // Gegenbeleg fuer sehr kurze Impulse". Bis SONDE-013 loeste
        // ausschliesslich der Fluss aus; `qualitaetPeak` trug nur das
        // Crest-Zusatzbit eines Flussereignisses, und `qualitaetFluss`
        // war konstant `true`. Ein Impuls, der zu kurz fuer eine
        // Flussueberschreitung ist, erzeugte damit GAR KEIN Ereignis —
        // genau der Fall, fuer den der Gegenbeleg gedacht ist.
        //
        // Der zweite Ausloeser ist der Anstieg des Rahmenpeaks gegenueber
        // dem zuletzt ABGESCHLOSSENEN Rahmen, zusammen mit einem hohen
        // Crest. Beide Bedingungen muessen gelten: ein Anstieg ohne Crest
        // ist eine Lautstaerkebewegung, ein Crest ohne Anstieg ist ein
        // dauerhaft spitzes Signal.
        //
        // ⚠️ `rahmenPeak` waechst INNERHALB eines Rahmens monoton, also
        // bliebe die Bedingung nach dem ersten Ueberschreiten bis zum
        // Rahmenende wahr und feuerte bei jedem FFT-Fenster erneut.
        // `peakEreignisImRahmen` laesst sie genau einmal je Rahmen
        // ausloesen.
        bool peakAus = false;
        if (! peakEreignisImRahmen && vorigerRahmenPeak > 0.0 && rahmenPeak > 0.0)
        {
            const double steigungDb = peakDb - 20.0 * std::log10 (vorigerRahmenPeak);
            peakAus = steigungDb > kPeakSteigungSchwelleDb
                   && crestDb > kPeakCrestSchwelleDb;
        }
        const bool flussAus = fluss > schwelle && mad > 0.0;

        // Loesen beide im selben Schritt aus, entsteht GENAU EIN Ereignis
        // mit beiden Bits (M-86). Zwei Ereignisse waeren zwei Zeitpunkte,
        // wo einer war.
        if (flussAus || peakAus)
        {
            Ereignis e;
            e.stromSample = s.fensterStromStart;
            e.projektzeitGesetzt = s.fensterProjektGueltig;
            e.projektSample = s.fensterProjektStart;
            e.epoche  = transportEpoche;
            e.segment = segmentInEpoche;
            // Die Staerke bleibt die Flussstaerke, WENN es eine gibt.
            // Ein reines Peakereignis hat keine Flussueberschreitung —
            // es traegt statt dessen seinen Crest ueber der Schwelle,
            // in derselben Einheit wie es gemessen wurde (dB). Eine
            // erfundene MAD-Zahl waere eine Staerke ohne Messung.
            e.staerke = flussAus
                ? (float) ((fluss - med) / mad)
                : (float) (crestDb - kPeakCrestSchwelleDb);
            e.bandZentrumHz = zentrumNenner > 0.0
                ? (float) (zentrumZaehler / zentrumNenner) : 0.0f;
            e.dauerMs = (float) (1000.0 * (double) s.hop / s.fs);
            e.qualitaetFluss = flussAus;
            e.qualitaetPeak = peakAus || (flussAus && crestDb > kPeakCrestSchwelleDb);
            ereignisAblegen (e);
            if (peakAus)
                peakEreignisImRahmen = true;
        }
    }

    flussHistorie[(std::size_t) flussStand] = fluss;
    flussStand = (flussStand + 1) % kFlussHistorie;
    if (flussGefuellt < kFlussHistorie) ++flussGefuellt;
}

inline double FeatureEngine::medianDerHistorie() noexcept
{
    for (int i = 0; i < kFlussHistorie; ++i)
        flussSortiert[(std::size_t) i] = flussHistorie[(std::size_t) i];
    std::sort (flussSortiert.begin(), flussSortiert.end());
    // Gerade Anzahl: Mittel der beiden mittleren.  Der Median ist hier eine
    // Schwellenbasis, keine Kennzahl — die genaue Konvention ist egal,
    // solange sie EINE ist.
    return 0.5 * (flussSortiert[(std::size_t) (kFlussHistorie / 2 - 1)]
                + flussSortiert[(std::size_t) (kFlussHistorie / 2)]);
}

/** Fester Ring, drop-oldest.  Ein Ereignisstrom, der bei Ueberlast waechst,
    waere ein unbegrenzter Vektor mit anderem Namen (§48.1). */
inline void FeatureEngine::ereignisAblegen (const Ereignis& e) noexcept
{
    if (ereignisAnzahl == kEreignisPlaetze)
        ++zEreignisseVerworfen;               // das aelteste faellt
    ereignisse[(std::size_t) ereignisStand] = e;
    ereignisStand = (ereignisStand + 1) % kEreignisPlaetze;
    if (ereignisAnzahl < kEreignisPlaetze) ++ereignisAnzahl;
}

} // namespace nakama::analyse
