// FeatureEngine — Frame bauen: aus dem Rahmen wird ein Snapshot.
//
// NAK-225/S25d (09.09.2026), Definitionen herausgeloest aus der Klasse in
// core/analysis/FeatureEngine.h. Inhalt:
//
//   baueFrame            Der Frameschluss: prueft, ob ein Rahmen ueberhaupt
//                        eine Aussage traegt, und fuellt sonst nichts.
//   evidenzLeeren, rahmenLeeren
//                        Die Rueckwege der beiden Akkumulationsebenen.
//   baueStempel          Der Transportstempel des Frames.
//   fuelleLive, fuelleEvidenz
//                        Die beiden Bandaufloesungen.
//   ringInZeitfolge, perzentil, fuelleVerteilung
//                        Die Perzentile aus den Verteilungsringen.
//   fuelleAbdeckungUndKonvergenz, fuelleSkalare
//                        Abdeckung, Konvergenz und die Skalare des Frames.
//
// Die Engine kennt keine Optik (CLAUDE.md): sie liefert einen kohaerenten
// Mess-Snapshot mit Revision, und der Editor rendert nur neue Revisionen. Was
// hier NICHT entsteht, ist eine Empfehlung - der Advisor ist regelbasiert und
// sitzt woanders.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

namespace nakama::analyse
{

//== Frame bauen ==========================================================

inline bool FeatureEngine::baueFrame() noexcept
{
    // Wert-Initialisierung statt Default-Initialisierung: sie nullt das
    // ganze Objekt, bevor die NSDMIs greifen.  ⚠️ Das ist die richtige
    // Vorgabe, aber KEINE Zusage ueber Fuellbytes — der Optimierer darf ihr
    // Nullen weglassen, weil sie unbeobachtbar sind, und unter `/O2` tut er
    // es (gemessen 24.08., Einzelheiten im Kasten bei
    // `FeatureFrame::operator==`).  Ein Frame wird deshalb NACH WERT
    // verglichen, nie nach Bytes.
    FeatureFrame f {};
    f.metricsVersion = kFeatureMetricsVersion;
    const double evidenzS = (double) evidenzSamples / sr;
    f.evidenzFrisch = evidenzS >= evidenzIntervallS;
    f.transport = baueStempel (rahmenStartBlock, f.evidenzFrisch);

    // NAK-29: ein Stempel, der die Feldpflichten verletzt, wird NICHT
    // veroeffentlicht.  Lieber kein Frame als ein Frame, dessen Zeitangabe
    // sich selbst widerspricht — ein Empfaenger kann den fehlenden Frame
    // sehen (die `sequence` springt), einen widerspruechlichen nicht.
    //
    // ⚠️ DIE NUMMER FAELLT VOR DER PRUEFUNG, nicht danach (T2-2).  Genau
    // darin besteht der Meldeweg: ein abgelehnter Versuch verbraucht seine
    // Sequenznummer, und die Luecke ist die Nachricht.
    const auto versuch = frameversuch (sequenz, f.transport);
    if (versuch.verstoss != 0)
    {
        ++zNak29Abgelehnt;
        rahmenLeeren();
        // Ein faelliger Evidenzsnapshot ist mit diesem Versuch verbraucht,
        // auch wenn dessen Stempel am Erzeugerriegel scheitert.  Bliebe
        // sein fruehester Support stehen, waere derselbe zu grosse
        // Zeitbereich im naechsten Versuch wieder enthalten: ein einmaliger
        // Vertragsverstoss wuerde die Telemetrie dauerhaft verriegeln.
        if (f.evidenzFrisch)
            evidenzLeeren();
        return false;
    }

    f.live.gitter    = GitterId::nakama_log64_v1;
    f.live.encoding  = BandEncoding::q_db_0p1_i16;
    fuelleLive (f.live, f.liveBreite, f.liveBreiteBitmap);

    f.evidenz.gitter   = GitterId::nakama_1_24_oct_30_18k_v1;
    f.evidenz.encoding = BandEncoding::q_db_0p01_i16;
    if (f.evidenzFrisch)
    {
        fuelleEvidenz (f.evidenz);
        // SONDE-013 M-05: Verteilung, Abdeckung und Konvergenz gehoeren zu
        // DIESEM Evidenzfenster und werden zusammen mit ihm gefuellt und
        // zusammen mit ihm geleert. Ein Snapshot mit Baendern, aber ohne
        // Verteilung waere die halbe Aussage aus §33.2.
        fuelleVerteilung (f);
        fuelleAbdeckungUndKonvergenz (f);
        // SONDE-013 M-11: die Stereoevidenz gehoert zu DIESEM
        // Evidenzfenster und wird mit ihm ausgewertet - vor
        // `evidenzLeeren()`, das die Akkus raeumt.
        stereoAuswerten();
        f.evidenzStromStartGesetzt = evidenzSupport.gesetzt;
        f.evidenzStromStart = evidenzSupport.stromStart;
    }
    else
    {
        f.evidenz.leeren();
    }

    // SONDE-013 M-02/M-03: das Passagenmaximum und die Headroomverteilung
    // wachsen um GENAU DIESEN Rahmen, bevor die Skalare sie lesen — sonst
    // fehlte dem Frame sein eigener Beitrag und die Verteilung liefe dem
    // Wert daneben um einen Rahmen hinterher.
    // M-25: der Beitrag ist das Maximum ueber die Samples IM Fenster, nicht
    // ueber den ganzen Rahmen. Ohne gesetztes Fenster sind beide gleich.
    //
    // NAK-283 F08 (U41): mit markierter Passage geht der Rahmen in das
    // Histogramm ueber die GANZE Passage, ohne Passage in das gleitende
    // Fenster der letzten `kVerteilungPlaetze` Rahmen. Vor dem Anfang und
    // hinter dem Ende einer Passage liegt kein Sample im Fenster, der Beitrag
    // ist 0 und geht nirgends ein - die Passagengrenzen wirken hier ueber
    // `imPassagenfenster` in `verarbeiteSamples`.
    if (passagenTruePeakRahmen > 0.0 && ! headroomRing.empty())
    {
        passageTruePeak = std::max (passageTruePeak, passagenTruePeakRahmen);
        const double db = 20.0 * std::log10 (passagenTruePeakRahmen);
        if (passagenfenster.gesetzt)
            headroomRing[0].passageSchiebe (db);
        else
            headroomRing[0].ring.schiebe ((float) db);
    }

    fuelleSkalare (f);

    f.transport.sequence = versuch.sequence;
    aktuell = f;

    rahmenLeeren();
    if (f.evidenzFrisch)
        evidenzLeeren();
    return true;
}

inline void FeatureEngine::evidenzLeeren() noexcept
{
    for (auto& v : evidenzAkku) v = { 0.0, 0 };
    // Verlauf, Abdeckung und Konvergenz gehoeren zu GENAU diesem
    // Evidenzfenster. Sie stehenzulassen hiesse, den naechsten Snapshot
    // aus fremdem Material zu rechnen — dieselbe Regel wie fuer den
    // Bandakku daneben.
    for (auto& r : evidenzVerteilung) r.leeren();
    stereoLeeren();                 // SONDE-013 M-11, dasselbe Fenster
    evidenzFensterGesamt = 0;
    evidenzFensterAktiv = 0;
    evidenzSamples = 0;
    // M-07: der Zaehler gehoert zu GENAU diesem Beleg.
    evidenzNichtEndlich = 0;
    evidenzSupport = {};
    evidenzContinuousHabe = false;
    evidenzContinuousDurchgehend = true;
    evidenzContinuousErwartet = 0;
}

inline void FeatureEngine::rahmenLeeren() noexcept
{
    for (auto& v : liveAkku) v = { 0.0, 0 };
    for (auto& v : liveBreiteAkku) v = { 0.0, 0.0 };
    liveSupport = {};
    liveSamples = 0;
    // SONDE-013 M-86: der abgeschlossene Rahmenpeak wird zum Bezugswert
    // des naechsten. Nur ein Rahmen MIT Inhalt zaehlt — ein leerer waere
    // sonst ein Bezugspunkt von 0, gegen den jede Steigung unendlich
    // waere.
    if (rahmenPeak > 0.0)
        vorigerRahmenPeak = rahmenPeak;
    peakEreignisImRahmen = false;
    rahmenPeak = 0.0;
    // SONDE-013 M-02: das RAHMEN-Maximum faellt mit dem Rahmen, das
    // PASSAGEN-Maximum nicht — sonst waere PLR eine Aussage ueber 100 ms
    // und nicht ueber die Passage (§39.1).
    rahmenTruePeak = 0.0;
    passagenTruePeakRahmen = 0.0;
    // M-07: der Rahmenzaehler faellt mit dem Rahmen; der Evidenzzaehler
    // faellt erst mit dem Evidenzfenster, und der Gesamtzaehler nie.
    // NAK-283 F10: die Zellenmarken fallen HIER nicht - sie leben mit ihrer
    // Zelle im 3-s-Ring, so lange wie das Fenster, das sie tragen.
    rahmenNichtEndlich = 0;
    rahmenSummeQuadrat = 0.0;
    rahmenSamples = 0;
    rahmenStartBlock = {};
    rahmenProjektDurchgehend = false;
    rahmenContinuousDurchgehend = false;
    rahmenMid2 = rahmenSide2 = 0.0;
    rahmenL = rahmenR = rahmenL2 = rahmenR2 = rahmenLR = 0.0;
    rahmenAktivZellen = 0;
    rahmenZellen = 0;
    // Der Ereignisring wird hier BEWUSST nicht geleert: ein Ereignis
    // gehoert zu seiner Epoche (es traegt sie mit), nicht zu dem Rahmen,
    // in dem es zufaellig gemeldet wurde. Geleert wird er nur von
    // `zuruecksetzen()`.
}

inline Transportstempel FeatureEngine::baueStempel (const echtzeit::StampedBlock& b,
                                                    bool evidenzWirdPubliziert) const noexcept
{
    // Wert-Initialisierung, gleiche Vorgabe und gleiche Grenze wie in
    // `baueFrame()`: die Fuellbytes zwischen `bool` und `int64` bleiben
    // unter `/O2` trotzdem unbestimmt.  Gelesen werden sie nie — der
    // Vergleich zweier Stempel laeuft ueber `operator==`, also memberweise.
    Transportstempel t {};
    t.transport_epoch    = transportEpoche;
    t.continuity_segment = segmentInEpoche;
    // Der Stempel umfasst nicht nur die skalaren Samples, sondern auch den
    // fruehesten FFT-Support, der TATSAECHLICH in die publizierten Baender
    // eingegangen ist.  Durch 50-%-Ueberlappung kann der vor `b` beginnen.
    Support gesamt { true, b.stromVon, rahmenProjektDurchgehend,
                     b.projectSampleStart, rahmenContinuousDurchgehend,
                     b.continuousTimeSamples };
    supportVereinen (gesamt, liveSupport);
    if (evidenzWirdPubliziert)
    {
        supportVereinen (gesamt, evidenzSupport);
        // Auch band-inaktive Zwischenzeit gehoert zum Beweis der
        // Evidenzspanne. Sonst koennte eine fehlende/springende Hostuhr in
        // einer stillen Luecke verschwinden und spaeter scheinbar passend
        // auf die alte affine Achse zurueckkehren.
        if (! evidenzContinuousDurchgehend)
            gesamt.continuousGueltig = false;
    }

    bool stromSpanneGueltig = b.stromVon
        <= std::numeric_limits<std::uint64_t>::max() - rahmenSamples;
    const auto stromEnde = stromSpanneGueltig ? b.stromVon + rahmenSamples : 0;
    if (stromSpanneGueltig && gesamt.stromStart <= stromEnde)
    {
        const auto laenge = stromEnde - gesamt.stromStart;
        if (laenge > 0 && laenge <= std::numeric_limits<std::uint32_t>::max())
            t.sample_count = static_cast<std::uint32_t> (laenge);
    }
    t.sample_rate        = sr;

    t.process_context_present_gesetzt = true;
    t.process_context_present = (b.flags & echtzeit::kFlagKontextAnwesend) != 0;

    // 🔑 DIE ZEITBASIS FOLGT AUS DEM BEWEIS, NICHT AUS DEM WUNSCH.
    // §32.3: "Ohne gueltige Projektzeit darf `time_basis=local_monotonic`
    // nur lokale Analyse und IPC-Frische tragen."  Wer hier
    // `project_samples` setzte, weil der Host "meistens" eine Zeit liefert,
    // erlaubte damit Cross-Probe-Alignment auf einer Zahl, die es nicht
    // gibt — und NAK-29-Fall 2 faengt genau diesen Griff.
    if (gesamt.projektGueltig)
    {
        t.zeitbasis = Zeitbasis::project_samples;
        t.project_sample_start_gesetzt = true;
        t.project_sample_start = gesamt.projektStart;
        t.gueltigkeit |= kGProjectTime;
    }
    else
    {
        t.zeitbasis = Zeitbasis::local_monotonic;
        // KEIN `project_time`-Bit und KEIN `project_sample_start`.
    }

    if ((b.flags & echtzeit::kFlagSpieltGueltig) != 0)
    {
        t.playing = (b.flags & echtzeit::kFlagSpielt) != 0;
        t.gueltigkeit |= kGPlayState;
    }
    if ((b.flags & echtzeit::kFlagRecordingGueltig) != 0)
    {
        t.recording = (b.flags & echtzeit::kFlagRecording) != 0;
        t.gueltigkeit |= kGRecordState;
    }
    // Auch die optionale Continuous-Uhr bezeichnet denselben Frameanfang.
    // Ihr Beweis wird pro Sample durch FFT-Ringe und ueber alle Skalar-
    // bloecke gefuehrt; eine bloss lokal zurueckgerechnete Hostzahl waere
    // bei fehlendem oder springendem Continuous-Feld erfunden.
    if (gesamt.continuousGueltig)
    {
        t.continuous_time_samples_gesetzt = true;
        t.continuous_time_samples = gesamt.continuousStart;
        t.gueltigkeit |= kGContinuousTime;
    }

    t.cycle_active = (b.flags & echtzeit::kFlagCycleAktiv) != 0;
    const bool cycleGrenzenBrauchbar
        = (b.flags & echtzeit::kFlagCycleGrenzenGueltig) != 0
       && std::isfinite (b.cycleStartPpq)
       && std::isfinite (b.cycleEndePpq)
       && b.cycleEndePpq >= b.cycleStartPpq;
    if (cycleGrenzenBrauchbar)
    {
        t.cycle_bounds_valid = true;
        t.cycle_start_ppq_gesetzt = true;
        t.cycle_start_ppq = b.cycleStartPpq;
        t.cycle_end_ppq_gesetzt = true;
        t.cycle_end_ppq = b.cycleEndePpq;
        t.gueltigkeit |= kGCycleBounds;
        // ⚠️ `unproven`, IMMER.  Die Abbildung PPQ→Sample braucht ein
        // FL-Golden fuer diesen Hostlauf (§32.3), und der Capabilityreport
        // S4 hat keines.  `validated_block_mapping` waere hier eine
        // Behauptung ueber eine Messung, die nie stattgefunden hat.
        t.cycle_derivation = Herleitung::unproven;
    }

    if ((b.flags & echtzeit::kFlagEingangLatenzGemeldet) != 0)
    {
        t.input_presentation_latency_gesetzt = true;
        t.input_presentation_latency = b.eingangLatenzSamples;
        t.gueltigkeit |= kGInputLatency;
    }
    if ((b.flags & echtzeit::kFlagAusgangLatenzGemeldet) != 0)
    {
        t.output_presentation_latency_gesetzt = true;
        t.output_presentation_latency = b.ausgangLatenzSamples;
        t.gueltigkeit |= kGOutputLatency;
    }
    return t;
}

inline void FeatureEngine::fuelleLive (LiveBaender& satz, float* breite, std::uint8_t* breiteBitmap) const noexcept
{
    satz.leeren();
    for (int i = 0; i < Gitter::liveBaender; ++i) breite[i] = 0.0f;
    bitmapNullen (breiteBitmap, Gitter::liveBaender);

    for (int g = 0; g < Gitter::liveBaender; ++g)
    {
        // Lineare Energiesumme der Feinbaender der Gruppe — das Livegitter
        // ist eine GRUPPIERUNG, keine zweite Filterbank (Fixture-Herkunft).
        double summe = 0.0, seiteSumme = 0.0, gesamtSumme = 0.0;
        int belegte = 0;
        for (int b = Gitter::liveVon (g); b < Gitter::liveBisExkl (g); ++b)
            if (liveAkku[(std::size_t) b].n > 0)
            {
                const double n = (double) liveAkku[(std::size_t) b].n;
                summe += liveAkku[(std::size_t) b].summe / n;
                seiteSumme  += liveBreiteAkku[(std::size_t) b].seite  / n;
                gesamtSumme += liveBreiteAkku[(std::size_t) b].gesamt / n;
                ++belegte;
            }
        if (belegte == 0)
            continue;                          // Bitmap bleibt 0

        // Band-Stereo: Seitenanteil der Gruppe. Nur wenn ueberhaupt Energie
        // da ist — 0/0 waere NaN, und NaN ist hier "keine Aussage", also
        // gehoert es hinter das Bit, nicht in den Wert.
        if (gesamtSumme > 0.0)
        {
            const double anteil = seiteSumme / gesamtSumme;
            if (std::isfinite (anteil))
            {
                breite[g] = (float) anteil;
                bitmapSetze (breiteBitmap, g, true);
            }
        }

        const double db = energieAlsDb (summe);
        if (! plausibel (db))
            continue;                          // unter der NaN-Ehrlichkeitsgrenze
        const auto q = quantisiere16 (db, satz.encoding);
        if (! q.gueltig)
            continue;
        satz.werte[g] = q.wert;
        bitmapSetze (satz.bitmap, g, true);
        if (q.saturiert) satz.saturated = true;
    }
}

inline void FeatureEngine::fuelleEvidenz (EvidenzBaender& satz) const noexcept
{
    satz.leeren();
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        if (evidenzAkku[(std::size_t) b].n == 0)
            continue;
        const double mittel = evidenzAkku[(std::size_t) b].summe
                            / (double) evidenzAkku[(std::size_t) b].n;
        const double db = energieAlsDb (mittel);
        if (! plausibel (db))
            continue;
        const auto q = quantisiere16 (db, satz.encoding);
        if (! q.gueltig)
            continue;
        satz.werte[b] = q.wert;
        bitmapSetze (satz.bitmap, b, true);
        if (q.saturiert) satz.saturated = true;
    }
}

/** Ein Band des Verteilungsrings in ZEITLICHER Reihenfolge nach `aus`.

    Der Ring speichert in Schreibreihenfolge; ist er voll, liegt der
    aelteste Wert bei `stand`. Fuer Perzentile spielt die Reihenfolge
    keine Rolle, fuer die Konvergenz (erste gegen zweite Haelfte) sehr
    wohl — deshalb gibt es genau eine Stelle, die sie herstellt, statt
    zweier Schleifen, die auseinanderlaufen koennen.

    @returns Zahl der geschriebenen Werte. */
inline int FeatureEngine::ringInZeitfolge (const VerteilungsRing& r, float* aus) noexcept
{
    const int n = r.gefuellt;
    const int erster = (n == kVerteilungPlaetze) ? r.stand : 0;
    for (int i = 0; i < n; ++i)
        aus[i] = r.werte[(std::size_t) ((erster + i) % kVerteilungPlaetze)];
    return n;
}

/** Perzentil einer AUFSTEIGEND sortierten Folge, linear interpoliert.

    Linear statt „naechster Rang", weil der naechste Rang bei kleinem `n`
    springt: mit acht Werten laege P10 sonst immer exakt auf dem Minimum
    und P95 immer exakt auf dem Maximum, und die drei Punkte trugen
    weniger Information als die Bandwerte selbst. */
inline double FeatureEngine::perzentil (const float* sortiert, int n, double p) noexcept
{
    if (n <= 0) return 0.0;
    if (n == 1) return sortiert[0];
    const double pos = p * (double) (n - 1);
    const int    lo  = (int) pos;
    const int    hi  = lo + 1 < n ? lo + 1 : lo;
    const double f   = pos - (double) lo;
    return (double) sortiert[lo] + f * ((double) sortiert[hi] - (double) sortiert[lo]);
}

/** SONDE-013 M-05: P10/P50/P95 je Band plus die Zahl der Fenster dahinter.

    `evidenzFenster` ist das MINIMUM ueber alle Baender mit Bit, nicht der
    Mittelwert und nicht das Maximum: die Zahl soll sagen, worauf sich der
    SCHWAECHSTE gezeigte Punkt stuetzt. Die zwei Analysestufen haben
    verschiedene Hopmasse, also traegt die Bassstufe hier regelmaessig die
    kleinere Zahl — genau das ist die ehrliche Auskunft. */
inline void FeatureEngine::fuelleVerteilung (FeatureFrame& f) const noexcept
{
    for (auto* satz : { &f.evidenzP10, &f.evidenzP50, &f.evidenzP95 })
    {
        satz->gitter   = GitterId::nakama_1_24_oct_30_18k_v1;
        satz->encoding = BandEncoding::q_db_0p01_i16;
        satz->leeren();
    }

    int schwaechste = -1;
    float folge[kVerteilungPlaetze];
    // Vor `vorbereiten()` gibt es keinen Ring. Die drei Bandsaetze stehen
    // dann leer da — richtig so: ohne Messung keine Verteilung.
    if ((int) evidenzVerteilung.size() < Gitter::evidenzBaender)
        return;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto& r = evidenzVerteilung[(std::size_t) b];
        if (r.gefuellt == 0)
            continue;                       // kein Bit: eine Verteilung ueber nichts

        const int n = ringInZeitfolge (r, folge);
        std::sort (folge, folge + n);

        bool alleGueltig = true;
        const double punkte[3] = { perzentil (folge, n, 0.10),
                                   perzentil (folge, n, 0.50),
                                   perzentil (folge, n, 0.95) };
        Quant16 q[3] {};
        for (int i = 0; i < 3; ++i)
        {
            if (! plausibel (punkte[i]))
            {
                alleGueltig = false;
                break;
            }
            q[i] = quantisiere16 (punkte[i], BandEncoding::q_db_0p01_i16);
            if (! q[i].gueltig)
            {
                alleGueltig = false;
                break;
            }
        }
        // Alle drei oder keiner. Ein Band, in dem nur P50 ein Bit haette,
        // saehe aus wie eine Verteilung und waere keine.
        if (! alleGueltig)
            continue;

        EvidenzBaender* saetze[3] = { &f.evidenzP10, &f.evidenzP50, &f.evidenzP95 };
        for (int i = 0; i < 3; ++i)
        {
            saetze[i]->werte[(std::size_t) b] = q[i].wert;
            bitmapSetze (saetze[i]->bitmap, b, true);
            if (q[i].saturiert) saetze[i]->saturated = true;
        }
        if (schwaechste < 0 || n < schwaechste)
            schwaechste = n;
    }
    f.evidenzFenster = schwaechste < 0 ? 0u : (std::uint32_t) schwaechste;
}

/** SONDE-013 M-05: Abdeckung und Konvergenz des Evidenzfensters.

    **Abdeckung** ist die tatsaechlich gemessene Signalabdeckung (§48.2),
    also aktive Fenster durch alle Fenster — nicht die Wanddauer. Ohne ein
    einziges Fenster gibt es kein Praesenzbit; 0 hiesse „nur Stille" und
    waere eine andere Aussage als „nichts gesehen".

    **Konvergenz** misst, ob die Verteilung sich noch bewegt: je Band der
    Betrag der Differenz zwischen dem Median der ERSTEN und dem der
    ZWEITEN Haelfte des behaltenen Fensters, gemittelt ueber die Baender
    mit genug Werten, dann ueber `kKonvergenzSpanneDb` auf [0, 1]
    abgebildet. 1 heisst „die beiden Haelften sagen dasselbe", 0 heisst
    „sie sagen Verschiedenes". Baender mit weniger als vier Werten gehen
    NICHT ein: zwei Mediane aus je einem Wert sind kein Konvergenzbeleg. */
inline void FeatureEngine::fuelleAbdeckungUndKonvergenz (FeatureFrame& f) const noexcept
{
    if (evidenzFensterGesamt > 0)
    {
        f.abdeckungGesetzt = true;
        f.abdeckung = (float) ((double) evidenzFensterAktiv
                             / (double) evidenzFensterGesamt);
    }

    double summeAbstand = 0.0;
    int    baenderMitBeleg = 0;
    float  folge[kVerteilungPlaetze];
    float  haelfte[kVerteilungPlaetze];
    if ((int) evidenzVerteilung.size() < Gitter::evidenzBaender)
        return;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto& r = evidenzVerteilung[(std::size_t) b];
        if (r.gefuellt < 4)
            continue;
        const int n = ringInZeitfolge (r, folge);
        const int h = n / 2;

        for (int i = 0; i < h; ++i) haelfte[i] = folge[i];
        std::sort (haelfte, haelfte + h);
        const double medianFrueh = perzentil (haelfte, h, 0.50);

        const int zweite = n - h;
        for (int i = 0; i < zweite; ++i) haelfte[i] = folge[h + i];
        std::sort (haelfte, haelfte + zweite);
        const double medianSpaet = perzentil (haelfte, zweite, 0.50);

        const double abstand = std::abs (medianSpaet - medianFrueh);
        if (! std::isfinite (abstand))
            continue;
        summeAbstand += abstand;
        ++baenderMitBeleg;
    }
    if (baenderMitBeleg > 0)
    {
        const double mittel = summeAbstand / (double) baenderMitBeleg;
        f.konvergenzGesetzt = true;
        f.konvergenz = (float) std::clamp (1.0 - mittel / kKonvergenzSpanneDb,
                                           0.0, 1.0);
    }
}

inline void FeatureEngine::fuelleSkalare (FeatureFrame& f) const noexcept
{
    // 🔑 SONDE-013 M-07, DIE VERRIEGELUNG. Sie steht vor allem anderen.
    //
    // Ein nicht-endliches Eingangssample wurde durch Stille ersetzt. Jede
    // Zahl, die dieser Rahmen daraus rechnet, ist damit eine Aussage ueber
    // Audio, das die Engine nie so gesehen hat — endlich, plausibel und
    // falsch. CLAUDE.md verlangt genau hier "verriegelt UND gezaehlt":
    // gezaehlt wird in `nichtEndlichRahmen`, verriegelt wird, indem der
    // Rahmen seine sampleabhaengigen Skalare gar nicht erst setzt. Ein
    // Leser sieht dann "nicht gemessen" statt "gemessen und sauber" — der
    // Unterschied, um den es in dieser Invariante geht.
    //
    // 🔑 NAK-283 F10 (R-283-5, §8.1 Feinheit 8): diese Verriegelung ist die
    // ERSTE von zwei Stufen, und sie lebt einen Rahmen. Die zweite haengt am
    // Fenster jeder Metrik: jede Loudnesszelle traegt das Merkmal "enthaelt
    // ersetztes Material" (`zelleSchliessen`), und Momentary, Short-term, PSR
    // und Crest ueber 3 s bleiben ohne Praesenzbit, bis KEINE Zelle ihres
    // Fensters es mehr traegt (`zellenFensterSauber`). Ohne sie trug schon der
    // erste saubere Rahmen wieder `lufsS` - aus drei Sekunden, die das
    // ersetzte Material noch enthielten. Die Praesenzbits fallen damit je
    // Metrik, nicht pauschal je Rahmen; die zwei Zaehler hier bleiben die
    // gezaehlte Haelfte und werden fuer die Marke nicht gelesen.
    f.nichtEndlichRahmen  = rahmenNichtEndlich;
    f.nichtEndlichEvidenz = evidenzNichtEndlich;
    if (rahmenNichtEndlich > 0)
        return;

    if (rahmenZellen > 0)
    {
        f.aktivitaetGesetzt = true;
        f.aktivitaet = (float) ((double) rahmenAktivZellen / (double) rahmenZellen);
    }
    double lufs = 0.0;
    if (kurzLufs (lufs))
    {
        f.lufsSGesetzt = true;
        f.lufsS = (float) lufs;
    }
    // SONDE-013 M-01: Momentary NEBEN Short-term, nicht statt seiner.
    double lufsMoment = 0.0;
    if (momentanLufs (lufsMoment))
    {
        f.lufsMGesetzt = true;
        f.lufsM = (float) lufsMoment;
    }
    // SONDE-013 M-09 (NAK-68): ueber wie viel Audio DIESER Rahmen
    // integriert wurde.  Das Feld ist die ganze Antwort auf "leise oder
    // kurz gemessen?" — ohne es sehen die Skalare darunter nach einer
    // Grenze aus wie im Dauerbetrieb.  0 waere ein Senderfehler (beide
    // Leser lehnen ihn ab), also traegt ein Rahmen ohne Samples KEIN Bit.
    if (rahmenSamples > 0)
    {
        f.integrationGesetzt = true;
        f.integrationSamples = (std::uint32_t) std::min<std::uint64_t> (
            rahmenSamples, (std::uint64_t) std::numeric_limits<std::uint32_t>::max());
    }
    if (rahmenSamples > 0 && rahmenPeak > 0.0)
    {
        const double peakDb = 20.0 * std::log10 (rahmenPeak);
        f.peakGesetzt = true;
        f.peakDb = (float) peakDb;

        const double rms = std::sqrt (rahmenSummeQuadrat / (double) rahmenSamples);
        if (rms > 0.0)
        {
            f.crestGesetzt = true;
            f.crestDb = (float) (peakDb - 20.0 * std::log10 (rms));
        }
    }
    // SONDE-013 M-02: der True Peak des Rahmens.  Er steht NEBEN `peakDb`
    // und ersetzt ihn nicht: der Sample-Peak ist die Zahl, die ein Host
    // anzeigt, der True Peak die, an der ein Encoder clippt.  Wer sie
    // zusammenlegte, verloere genau die Differenz, um die es geht.
    // `rahmenTruePeak` schliesst den Abtastpunkt bereits ein — siehe die
    // Begruendung an seiner einzigen Schreibstelle in
    // `verarbeiteSamples`.
    if (rahmenTruePeak > 0.0)
    {
        f.truePeakGesetzt = true;
        f.truePeakDb = (float) (20.0 * std::log10 (rahmenTruePeak));
    }
    if (passageTruePeak > 0.0)
    {
        f.truePeakPassageGesetzt = true;
        f.truePeakPassageDb = (float) (20.0 * std::log10 (passageTruePeak));
    }
    // SONDE-013 M-03: PSR(3 s) = True-Peak-Maximum DESSELBEN 3-s-Fensters
    // minus LUFS-S (§39.1).  Bis zu diesem Ticket rechnete das Feld gegen
    // den Sample-Peak des 100-ms-Rahmens — zwei verschiedene Fenster und
    // die schwaechere der zwei Peakzahlen.  Beides ist jetzt behoben; das
    // Bit faellt weg, solange das 3-s-Fenster nicht steht, statt eine
    // kuerzere Rechnung unter demselben Namen zu liefern.
    double tpKurz = 0.0;
    if (f.lufsSGesetzt && kurzTruePeak (tpKurz) && tpKurz > 0.0)
    {
        f.psrGesetzt = true;
        f.psrDb = (float) (20.0 * std::log10 (tpKurz) - (double) f.lufsS);
    }
    // SONDE-013 M-04: das zweite Crestfenster.
    double crest3s = 0.0;
    if (crestKurz (crest3s))
    {
        f.crestKurzGesetzt = true;
        f.crestKurzDb = (float) crest3s;
    }
    // SONDE-013 M-04: LRA erst ab rund 60 s geeignetem Material.
    double lra = 0.0;
    if (lraLu (lra))
    {
        f.lraGesetzt = true;
        f.lraLu = (float) lra;
    }
    // SONDE-013 M-03: Headroom als Verteilung.  Vier Rahmen sind die
    // Untergrenze, unter der P10 und P95 derselbe Wert waeren — dieselbe
    // Schwelle wie bei der Bandkonvergenz nebenan.
    //
    // 🔑 NAK-283 F08 (R-283-5, §8.1 Feinheit 7; U41): die Spanne folgt der
    // Frage. Mit markierter Passage die GANZE Passage aus dem Histogramm
    // (M-53), `headroomFenster` = eingegangene Rahmen der Passage (M-54).
    // Ohne Passage das gleitende Fenster der letzten `kVerteilungPlaetze`
    // Rahmen und seine Belegung (User 13.09.2026). Ein nicht darstellbarer
    // Rahmenwert laesst die Passage ohne Verteilung, statt ihn an den
    // Klassenrand zu kappen.
    if (! headroomRing.empty())
    {
        const auto& h = headroomRing[0];
        if (passagenfenster.gesetzt)
        {
            if (h.rahmen >= 4u && ! h.nichtDarstellbar)
            {
                f.headroomGesetzt = true;
                f.headroomP10Db = (float) h.passagePerzentil (0.10);
                f.headroomP50Db = (float) h.passagePerzentil (0.50);
                f.headroomP95Db = (float) h.passagePerzentil (0.95);
                f.headroomFenster = h.rahmen;
            }
        }
        else if (h.ring.gefuellt >= 4)
        {
            float sortiert[kVerteilungPlaetze];
            const int n = ringInZeitfolge (h.ring, sortiert);
            std::sort (sortiert, sortiert + n);
            f.headroomGesetzt = true;
            f.headroomP10Db = (float) perzentil (sortiert, n, 0.10);
            f.headroomP50Db = (float) perzentil (sortiert, n, 0.50);
            f.headroomP95Db = (float) perzentil (sortiert, n, 0.95);
            f.headroomFenster = (std::uint32_t) n;
        }
    }
    if (rahmenSamples > 0)
    {
        const double gesamt = rahmenMid2 + rahmenSide2;
        if (gesamt > 0.0)
        {
            f.breiteGesetzt = true;
            f.breite = (float) (rahmenSide2 / gesamt);
        }
        const double nen = std::sqrt (std::max (rahmenL2, 0.0)) * std::sqrt (std::max (rahmenR2, 0.0));
        if (nen > 0.0)
        {
            const double k = rahmenLR / nen;
            if (std::isfinite (k))
            {
                f.korrelationGesetzt = true;
                f.korrelation = (float) std::clamp (k, -1.0, 1.0);
            }
        }
    }
}

} // namespace nakama::analyse
