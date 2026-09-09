// FeatureEngine — Zeit und Grenzen: wann ein Messfenster reisst.
//
// NAK-225/S25d (09.09.2026), Definitionen herausgeloest aus der Klasse in
// core/analysis/FeatureEngine.h. Inhalt:
//
//   projektVorwaerts, supportVereinen, supportMerken
//                        Der Beweisvorrat (Support) je Stufe: welche
//                        Hostangaben ein Fenster wirklich getragen haben.
//   trennIndex, zuordnung, auskunftBestimmen
//                        Bandzuordnung der beiden Stufen und die Frage, worueber
//                        die Engine bei dieser Samplerate ueberhaupt Auskunft
//                        geben darf.
//   grenzeZwischen, istLoopWrap, ppqProSampleVon, moeglicherStraddleIn
//                        Die Erkennung: Neuanlauf, Sprung, Loop-Wrap, Straddle.
//   grenzeZiehen         Der Vollzug. Er verwirft angefangene Fenster, zaehlt
//                        den Grund und startet Epoche oder Segment neu.
//   blockProjektSpanneGueltig, blockContinuousSpanneGueltig,
//   evidenzContinuousBelegen, rahmenZeitBelegen
//                        Die Belegung der beiden Zeitachsen im Rahmen.
//
// Die Invariante dieser Familie (CLAUDE.md): Zeit ist aktive Musikzeit.
// Zonen-Ticks sind deterministisch und durch Goldens beweisbar; jede Grenze
// traegt ihren Grund, damit ein Golden nicht nur DASS, sondern WARUM prueft.
//
// Alles hier laeuft im Worker, nicht im Audio-Block.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

namespace nakama::analyse
{

inline bool FeatureEngine::projektVorwaerts (std::int64_t start, std::uint64_t delta,
                                             std::int64_t& heraus) noexcept
{
    if (delta > static_cast<std::uint64_t> (std::numeric_limits<std::int64_t>::max()))
        return false;
    const auto d = static_cast<std::int64_t> (delta);
    if (start > std::numeric_limits<std::int64_t>::max() - d)
        return false;
    heraus = start + d;
    return true;
}

/** Vereinigt zwei lokale Anfaenge und erhaelt einen Zeitbeweis nur, wenn
    beide dieselbe affine 1-Sample-Abbildung belegen. */
inline void FeatureEngine::supportVereinen (Support& ziel, const Support& neu) noexcept
{
    if (! neu.gesetzt)
        return;
    if (! ziel.gesetzt)
    {
        ziel = neu;
        return;
    }

    const auto achsePasst = [&] (bool zielGueltig, std::int64_t zielStart,
                                 bool neuGueltig, std::int64_t neuStart)
    {
        if (! zielGueltig || ! neuGueltig)
            return false;
        std::int64_t erwartet = 0;
        if (neu.stromStart < ziel.stromStart)
            return projektVorwaerts (neuStart,
                                     ziel.stromStart - neu.stromStart,
                                     erwartet)
                && erwartet == zielStart;
        return projektVorwaerts (zielStart,
                                 neu.stromStart - ziel.stromStart,
                                 erwartet)
            && erwartet == neuStart;
    };

    const bool projektPasst = achsePasst (ziel.projektGueltig,
                                          ziel.projektStart,
                                          neu.projektGueltig,
                                          neu.projektStart);
    const bool continuousPasst = achsePasst (ziel.continuousGueltig,
                                             ziel.continuousStart,
                                             neu.continuousGueltig,
                                             neu.continuousStart);

    if (neu.stromStart < ziel.stromStart)
    {
        ziel.stromStart = neu.stromStart;
        ziel.projektStart = neu.projektStart;
        ziel.continuousStart = neu.continuousStart;
    }
    ziel.projektGueltig = projektPasst;
    ziel.continuousGueltig = continuousPasst;
}

inline void FeatureEngine::supportMerken (Support& ziel, const Stufe& s) noexcept
{
    supportVereinen (ziel, Support { true, s.fensterStromStart,
                                    s.fensterProjektGueltig,
                                    s.fensterProjektStart,
                                    s.fensterContinuousGueltig,
                                    s.fensterContinuousStart });
}

inline int FeatureEngine::trennIndex() const noexcept
{
    // Erstes Band, dessen MITTE >= kTrennungHz liegt.  Ueber die Mitte und
    // nicht ueber eine Kante, damit die Zustaendigkeit eines Bandes nicht
    // davon abhaengt, welche seiner beiden Kanten man ansieht.
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
        if (Gitter::evidenzMitte (b) >= kTrennungHz)
            return b;
    return Gitter::evidenzBaender;
}

/** Bin-Fenster je Band, exakt die Konvention von `AnalyseEngine`
    (`ceil(kante*n/fs)`, `bis = max(von,bis)`).  Gleiche Konvention, damit
    "dieses Band ist nicht messbar" in beiden Engines dasselbe heisst — auch
    wenn die Achsen verschieden sind. */
inline void FeatureEngine::zuordnung (Stufe& s, int vonBand, int bisBand)
{
    const double kappeHz = std::min (kObergrenzeHz, kNyquistAnteil * sr * 0.5);
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        if (b < vonBand || b >= bisBand)
        {
            s.bandVon[(std::size_t) b] = 0;
            s.bandBis[(std::size_t) b] = 0;      // diese Stufe ist nicht zustaendig
            continue;
        }
        const double lo = Gitter::evidenzKante (b);
        const double hi = Gitter::evidenzKante (b + 1);
        if (hi > kappeHz)
        {
            s.bandVon[(std::size_t) b] = 0;
            s.bandBis[(std::size_t) b] = 0;      // ueber der Kappe: nicht messbar
            continue;
        }
        const int von = (int) std::ceil (lo * (double) s.punkte / s.fs);
        int bis = (int) std::ceil (hi * (double) s.punkte / s.fs);
        bis = std::min (bis, s.punkte / 2 + 1);
        s.bandVon[(std::size_t) b] = von;
        s.bandBis[(std::size_t) b] = std::max (von, bis);
    }
}

/** Auskunft ueber die GRENZEN der Messung — erst aufrufbar, wenn beide
    Stufen ihre Zuordnung haben.  Stuende das in `zuordnung()`, laese der
    erste Aufruf eine noch leere zweite Stufe und `erstesMessbaresBand()`
    haette bis zum zweiten Aufruf einen falschen Wert dagestanden. */
inline void FeatureEngine::auskunftBestimmen() noexcept
{
    const double kappeHz = std::min (kObergrenzeHz, kNyquistAnteil * sr * 0.5);

    erstesBand = Gitter::evidenzBaender;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const bool messbar = bass.bandBis[(std::size_t) b] > bass.bandVon[(std::size_t) b]
                          || haupt.bandBis[(std::size_t) b] > haupt.bandVon[(std::size_t) b];
        if (messbar) { erstesBand = b; break; }
    }
    kappeBand = Gitter::evidenzBaender;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
        if (Gitter::evidenzKante (b + 1) > kappeHz) { kappeBand = b; break; }
}

//== Grenzerkennung =======================================================

/** Warum trennt `neu` von `alt`?  `Grenzgrund::keine` heisst: lueckenlose
    Fortsetzung derselben Epoche und desselben Segments.

    🔑 Die REIHENFOLGE der Fragen ist die Aussage.  Ein Queue-Drop und ein
    Seek koennen im selben Blockpaar zusammenfallen; §32.3 will dann NICHT,
    dass die Host-Zeitachse "faelschlich als Seek bezeichnet" wird, und
    umgekehrt darf ein echter Seek nicht als blosse lokale Luecke
    durchgehen.  Deshalb steht der Neuanlauf ganz oben (er macht jede
    Zeitaussage bedeutungslos), dann die Beweislage, dann die echten
    Transportereignisse, und die lokale Luecke ZULETZT: sie ist die
    schwaechste Aussage von allen. */
inline Grenzgrund FeatureEngine::grenzeZwischen (const echtzeit::StampedBlock& alt,
                                                 const echtzeit::StampedBlock& neu) const noexcept
{
    if (neu.startFolge != alt.startFolge)
        return Grenzgrund::neuanlauf;

    // Samplerate: ein Wechsel ist §32.3 ausdruecklich eine Epochengrenze.
    // Nur vergleichen, wenn BEIDE sie melden — sonst waere "der Host hat
    // aufgehoert zu melden" ein Sampleratewechsel, und das ist er nicht.
    const bool beideSr = (alt.flags & echtzeit::kFlagSampleRateGueltig) != 0
                      && (neu.flags & echtzeit::kFlagSampleRateGueltig) != 0;
    if (beideSr && alt.sampleRate != neu.sampleRate)
        return Grenzgrund::sampleratewechsel;

    constexpr std::uint32_t beweisMaske = echtzeit::kFlagKontextAnwesend
                                        | echtzeit::kFlagZeitGueltig
                                        | echtzeit::kFlagSpieltGueltig
                                        | echtzeit::kFlagNichtEchtzeit;
    if (((alt.flags ^ neu.flags) & beweisMaske) != 0)
        return Grenzgrund::beweislageWechsel;

    const bool spieltBekannt = (neu.flags & echtzeit::kFlagSpieltGueltig) != 0;
    if (spieltBekannt
        && ((alt.flags & echtzeit::kFlagSpielt) != (neu.flags & echtzeit::kFlagSpielt)))
        return Grenzgrund::transportKante;

    // Zeitsprung.  Dieselbe Lesart wie `Blockquarantaene::schliesstAn` —
    // eine STEHENDE Zeit sagt nichts (FL zerteilt Puffer bis auf 1 Sample
    // und die Teilstuecke tragen dieselbe Projektzeit), eine BEWEGTE Zeit
    // muss sich lueckenlos bewegen.  Der Unterschied zur Quarantaene ist
    // nur, dass hier zusaetzlich die URSACHE benannt wird.
    const bool beideZeit = (alt.flags & echtzeit::kFlagZeitGueltig) != 0
                        && (neu.flags & echtzeit::kFlagZeitGueltig) != 0;
    if (beideZeit && spieltBekannt && neu.projectSampleStart != alt.projectSampleStart)
    {
        const bool ueberlauf = alt.projectSampleStart
                                 > std::numeric_limits<std::int64_t>::max()
                                       - (std::int64_t) alt.sampleCount;
        const std::int64_t erwartet = ueberlauf
            ? std::numeric_limits<std::int64_t>::max()
            : alt.projectSampleStart + (std::int64_t) alt.sampleCount;
        if (ueberlauf || neu.projectSampleStart != erwartet)
            return istLoopWrap (alt, neu) ? Grenzgrund::loopWrap : Grenzgrund::zeitSprung;
    }

    // Lokale Analyseluecke — zuletzt, weil die schwaechste Aussage.
    if ((neu.flags & echtzeit::kFlagLueckeDavor) != 0
        || neu.segment != alt.segment
        || neu.stromVon != alt.stromVon + alt.sampleCount)
        return Grenzgrund::lokaleLuecke;

    return Grenzgrund::keine;
}

/** Ein Rueckwaertssprung bei aktiver Schleife, dessen Ziel nicht weiter vom
    Schleifenanfang entfernt liegt als ein Block.

    ⚠️ Ohne bewiesene Sample-Grenzen ist das eine WAHRSCHEINLICHKEITSAUSSAGE,
    keine Messung — und sie aendert am Ergebnis nichts: Loop-Wrap und Seek
    sind BEIDE Epochengrenzen und trennen beide jedes Fenster.  Sie
    unterscheidet nur die Telemetrie.  Genau deshalb darf sie hier stehen,
    ohne einen Beweis zu behaupten, den es nicht gibt. */
inline bool FeatureEngine::istLoopWrap (const echtzeit::StampedBlock& alt,
                                        const echtzeit::StampedBlock& neu) const noexcept
{
    if ((neu.flags & echtzeit::kFlagCycleAktiv) == 0)
        return false;
    if (neu.projectSampleStart >= alt.projectSampleStart)
        return false;                       // vorwaerts: kein Wrap
    if ((neu.flags & echtzeit::kFlagCycleGrenzenGueltig) == 0)
        return true;                        // Schleife an, rueckwaerts: reicht
    const double ppqProSample = ppqProSampleVon (neu);
    if (! (ppqProSample > 0.0))
        return true;
    // Wie weit liegt das Sprungziel hinter dem Schleifenanfang, in PPQ?
    const double zielPpq = neu.ppqPosition;
    if ((neu.flags & echtzeit::kFlagPpqGueltig) == 0)
        return true;
    const double abstand = std::abs (zielPpq - neu.cycleStartPpq);
    return abstand <= ppqProSample * (double) neu.sampleCount;
}

/** PPQ je Sample aus Tempo und Samplerate.  0, wenn eines von beiden fehlt —
    dann ist jede Aussage ueber die Lage der Schleifengrenze unmoeglich. */
inline double FeatureEngine::ppqProSampleVon (const echtzeit::StampedBlock& b) noexcept
{
    if ((b.flags & echtzeit::kFlagTempoGueltig) == 0) return 0.0;
    if ((b.flags & echtzeit::kFlagSampleRateGueltig) == 0) return 0.0;
    if (! (b.tempo > 0.0) || ! (b.sampleRate > 0.0)) return 0.0;
    return b.tempo / 60.0 / b.sampleRate;    // Viertel je Sekunde / Samples je Sekunde
}

/** Liegt die Schleifen-Endgrenze rechnerisch INNERHALB dieses Blocks?

    §32.3: "Loop-Grenzen koennen innerhalb eines Hostblocks liegen.  Bei
    gueltigen, fuer diesen Hostlauf bewiesenen `derived_sample_bounds` wird
    ein solcher Block logisch geteilt.  Liegen nur PPQ-Bounds vor oder
    fehlen die Bounds, wird der moegliche Straddle als ungueltig markiert."

    ⚠️ Fehlen Tempo oder PPQ ganz, wird hier NICHT vorsorglich getrennt.
    Das waere der naheliegende Fehler ("im Zweifel trennen") und es waere
    falsch: bei aktiver Schleife ohne diese Felder traefe es JEDEN Block,
    die Analyse stuerbe waehrend jeder Schleife vollstaendig.  §32.3 sieht
    fuer genau diesen Fall den anderen Weg vor — "spaetestens beim Erkennen
    des Wraps im Folgeblock eine neue Epoche" —, und den faehrt
    `grenzeZwischen()` ueber den Zeitsprung.  Dieselbe Lehre wie T2-4 in
    SONDE-008: naeher am Wortlaut ist nicht automatisch naeher an der
    Absicht. */
inline bool FeatureEngine::moeglicherStraddleIn (const echtzeit::StampedBlock& b) const noexcept
{
    if ((b.flags & echtzeit::kFlagCycleAktiv) == 0) return false;
    if ((b.flags & echtzeit::kFlagCycleGrenzenGueltig) == 0) return false;
    if ((b.flags & echtzeit::kFlagPpqGueltig) == 0) return false;
    const double ppqProSample = ppqProSampleVon (b);
    if (! (ppqProSample > 0.0)) return false;

    const double von = b.ppqPosition;
    const double bis = von + ppqProSample * (double) b.sampleCount;
    // Halboffen [von, bis): liegt das Ende exakt auf `von`, hat der Wrap
    // bereits stattgefunden und `grenzeZwischen` hat ihn gesehen.
    return b.cycleEndePpq > von && b.cycleEndePpq < bis;
}

/** DIE Stelle, an der der Gate-Text zu Code wird.

    Jedes offene Fenster faellt.  Nicht eines, nicht die langen, nicht die,
    die schon halb voll sind — alle.  Die Aufzaehlung steht bewusst
    vollstaendig und ohne Sammelschleife da: wer spaeter ein Fenster
    hinzufuegt, soll an dieser Liste vorbeikommen und merken, dass er es
    eintragen muss.

    ⚠️ GENAU DAS HAT BEIM ERSTEN MAL NICHT FUNKTIONIERT.  Die Liste war
    unvollstaendig (T2-1, 23.08.): sieben Zustandstraeger standen nicht
    darauf, darunter die drei Bandakkus.  Ein Kommentar, der Vollstaendigkeit
    BEHAUPTET, ersetzt keinen Riegel, der sie MISST — deshalb hat jeder
    Traeger hier heute eine Auskunft (`liveAkkuBelegteBaender()` und
    Nachbarn), und `keinFensterUeberbrueckt()` in B5 fragt sie alle ab
    statt nur die fuenf Fuellstaende, an denen der Bruch unsichtbar war. */
inline void FeatureEngine::grenzeZiehen (Grenzgrund grund) noexcept
{
    // 🔑 Luecke B09, ZUERST und vor jedem Nullen: der Polyphasenfilter
    // laeuft aus, bevor sein Zustand faellt.
    //
    // Sein Kern ist um die halbe Laenge zentriert; die letzten zwoelf
    // Eingangssamples stehen beim Reset noch in der Verzoegerungskette.
    // Sie gehoeren zur ALTEN Seite der Grenze, also werden sie hier den
    // Groessen der alten Seite zugeschlagen und nicht weggeworfen.
    //
    // ⚠️ Ehrlich benannt: an einer TRANSPORTgrenze fallen genau diese
    // Groessen unmittelbar danach mit der Epoche (§32.3/§32.4, die Liste
    // unten). Der Nachlauf ist dort deshalb ohne beobachtbaren Wert und
    // garantiert nur, dass kein Zwischenwert die Grenze ueberlebt. Seine
    // Wirkung hat er am PASSAGENfensterende, das `verarbeiteSamples`
    // eigenstaendig behandelt — dort bleibt das Maximum stehen.
    {
        const double rest = tp.nachlauf();
        rahmenTruePeak = std::max (rahmenTruePeak, rest);
        passagenTruePeakRahmen = std::max (passagenTruePeakRahmen, rest);
        passageTruePeak = std::max (passageTruePeak, rest);
    }
    ++zGetrennteFenster;
    ++grundZaehler[(std::size_t) grund];

    zVerworfeneFensterSamples += (std::uint64_t) bass.gefuellt
                               + (std::uint64_t) haupt.gefuellt
                               + (std::uint64_t) zelleStand;

    bass.leeren();                  // FFT-Fenster, Bassstufe
    haupt.leeren();                 // FFT-Fenster, Hauptstufe

    // ── Die Bandakkus: das INTEGRATIONSFENSTER des Bandwertes ────────────
    //
    // 🔑 T2-1 (23.08.): DIESE DREI STANDEN NICHT AUF DER LISTE, und deshalb
    // meldete ein Frame unter dem Stempel der NEUEN Epoche den Ton von VOR
    // der Grenze — 23 Live-Baender, staerkstes bei 1029 Hz mit -23,7 dB,
    // obwohl danach nur noch digitale Stille lief.  Die Fuellstaende sagten
    // dabei korrekt "getrennt": ein geleerter FFT-Ring ist eben nicht
    // dasselbe wie ein geleertes Integrationsfenster.  Der veroeffentlichte
    // Bandwert IST das FFT-Ergebnis, ueber das Frameintervall linear
    // integriert (§33.1) — also ist der Akku ein Fenster wie jedes andere,
    // und der Gate-Text ("trennt JEDES offene Fenster") meint ihn mit.
    for (auto& v : liveAkku)
    {
        zVerworfeneBandfenster += v.n;
        v = { 0.0, 0 };
    }
    for (auto& v : evidenzAkku)
    {
        zVerworfeneBandfenster += v.n;
        v = { 0.0, 0 };
    }
    for (auto& v : liveBreiteAkku) v = { 0.0, 0.0 };
    // SONDE-013 M-05: der Verteilungsring ist ein Fenster wie jedes andere
    // — er traegt den VERLAUF der Bandwerte ueber das Evidenzfenster. Ein
    // Perzentil, das eine Grenze ueberbrueckt, mischte zwei Epochen zu
    // einer Verteilung; die zwei Fensterzaehler daneben ergaeben eine
    // Abdeckung ueber zwei verschiedene Zeitraeume.
    for (auto& r : evidenzVerteilung)
    {
        zVerworfeneBandfenster += (std::uint64_t) r.gefuellt;
        r.leeren();
    }
    evidenzFensterGesamt = 0;
    evidenzFensterAktiv = 0;
    // SONDE-013 M-11: die Stereoevidenz ist ein Fenster wie jedes andere.
    // Ein Kreuzspektrum ueber eine Grenze hinweg mittelte zwei Stellen
    // der Musik zu einer Kohaerenz - und die saehe danach aus wie eine
    // Messung.
    stereoLeeren();
    liveSupport = {};
    evidenzSupport = {};
    evidenzContinuousHabe = false;
    evidenzContinuousDurchgehend = true;
    evidenzContinuousErwartet = 0;

    // Aktivitaetszaehler: `zelleStand` (die angefangene Zelle) faellt schon
    // seit der ersten Fassung, die FERTIGEN Zellen des laufenden Rahmens
    // fielen nicht — `aktivitaet` war damit ein Anteil ueber Zellen aus
    // zwei Epochen.  Dieselbe Auslassung, andere Zeile.
    rahmenAktivZellen = 0;
    rahmenZellen = 0;

    // ⚠️ `liveSamples` und `evidenzSamples` bleiben BEWUSST stehen —
    // Entwurfsentscheid des Erbauers, Begruendung im Manifest §10.1.
    // Kurzfassung: DIE GRENZE SCHNEIDET DEN INHALT, NICHT DIE UHR.  Die
    // beiden Zaehler sind kein Messwert, sondern der Fahrplan (sie werden
    // nirgends sonst gelesen als von den zwei Kadenzabfragen).  Wer sie hier
    // mitnullte, liesse eine Folge dichter Grenzen — Queue-Drops unter Last,
    // eine enge Schleife — die Telemetrie VOLLSTAENDIG verstummen lassen;
    // genau die Todesart, die §4.4 fuer den Straddle schon einmal
    // ausdruecklich verworfen hat ("die Analyse stuerbe waehrend jeder
    // Schleife").  Ein Frame, der kurz nach einer Grenze faellig wird,
    // traegt stattdessen WENIGER oder GAR KEINE Baender — und dass ein Band
    // nichts sagt, statt etwas Falsches zu sagen, ist ueber die Bitmap
    // ehrlich ausdrueckbar ("gemeldete 0" != "nie gesagt").
    zelleStand = 0;                 // Loudness-Zelle, angefangen
    zelleKEnergie = 0.0;
    zelleAktivEnergie = 0.0;
    kurzStand = 0;                  // LUFS-S ueber 3 s — die ganze Historie
    kurzGefuellt = 0;
    for (auto& z : kurzZellen) z = 0.0;
    // SONDE-013 M-03/M-04: die zwei Nachbarringe des 3-s-Fensters fallen
    // MIT ihm. Sie stehenzulassen hiesse, den True-Peak-Maximalwert oder
    // die RMS aus der alten Epoche gegen eine neue Kurzzeitlautheit zu
    // rechnen — genau die Ueberbrueckung, die §32.3 ausschliesst, nur
    // eine Zeile neben der, an der sie 23.08. schon einmal stand.
    for (auto& z : kurzTpZellen)  z = 0.0;
    for (auto& z : kurzRmsZellen) z = 0.0;
    zelleTruePeak = 0.0;
    zelleRmsEnergie = 0.0;
    lraZellenSeitHop = 0;
    // 🔑 UND DIE DREI PASSAGENGROESSEN FALLEN MIT. Das ist die Korrektur
    // eines Entwurfsfehlers dieser Etappe, den G13 gefunden hat.
    //
    // Der erste Bau liess `passageTruePeak`, den Headroomring und das
    // LRA-Histogramm eine Grenze UEBERLEBEN — mit der Begruendung, sie
    // seien Passagenstatistiken wie die integrierte Lautheit, nicht
    // offene Fenster. Die Zwillingsprobe G13 hat das an allen sechs
    // Grenzarten widerlegt: A (vorher lautes Audio) und B (vorher
    // Stille) waren danach in 14 von 14 Frames unterscheidbar.
    //
    // Die Begruendung war falsch, und zwar aus zwei Gruenden. Erstens
    // reist die integrierte Lautheit gar nicht durch diese Engine — sie
    // kommt aus dem `LoudnessAccumulator` und wird erst im
    // Sondenprozessor zugemischt. Zweitens, und das ist der tragende
    // Grund: WAS IM `FeatureFrame` REIST, UNTERLIEGT §32.3. Der Frame
    // traegt seit dieser Etappe `truePeakPassageDb` und die
    // Headroomperzentile, also sind sie Traeger wie jeder andere.
    //
    // Fachlich stimmt das ueberdies mit §32.4 ueberein: eine Passage
    // bindet an GENAU EINE Transportepoche. Nach einer Epochengrenze ist
    // die Passage zu Ende, und ein Maximum ueber sie hinweg waere ueber
    // zwei Passagen gerechnet. Auch bei einem Drop (Segment, keine
    // Epoche) fehlt Audio, und ein Maximum ueber die Luecke behauptete
    // etwas ueber Material, das die Engine nie gesehen hat.
    //
    // Der Preis ist benannt und richtig: LRA braucht seine rund 60 s
    // OHNE Grenze. Genau das heisst "60 s geeignetes Material" (§39.1).
    passageTruePeak = 0.0;
    passagenTruePeakRahmen = 0.0;
    // M-25/§32.4: eine Passage bindet an GENAU EINE Transportepoche. Eine
    // Grenze darin macht das Fenster unbrauchbar — es wird nicht still
    // fortgesetzt, sondern als GEBROCHEN gefuehrt, bis der Besitzer ein
    // neues setzt. Ein Fenster, das eine Grenze ueberlebte, beschriebe
    // zwei Passagen als eine.
    if (passagenfenster.gesetzt)
        passagenfensterGebrochen = true;
    zelleImFensterSamples = 0;
    for (auto& r : headroomRing) r.leeren();
    for (auto& b : lraHistogramm) b = 0u;
    lraGezaehlt = 0;
    // Die K-Filter behalten ihren Zustand NICHT: ihr Nachklang traegt Audio
    // von VOR der Grenze, und genau das ist Ueberbrueckung — nur eben
    // ueber den Filterzustand statt ueber einen Puffer.  Das ist die
    // subtilste Form des Fehlers, gegen den dieses Ticket schuetzt.
    kL.zustandNullen();
    kR.zustandNullen();
    // Derselbe Grund fuer den True-Peak-Interpolator: seine 24 Taps je
    // Phase reichen ueber die Grenze zurueck, und ein daraus gerechneter
    // Zwischenwert gehoert zu keiner der beiden Epochen.
    //
    // Der Nachlauf des Filters (Luecke B09) ist bereits GANZ OBEN in dieser
    // Funktion ausgewertet worden — vor jedem Nullen. Hier bleibt nur das
    // Leeren des Zustands.
    tp.zuruecksetzen();

    rahmenPeak = 0.0;               // Korrelations-/Peakfenster
    rahmenTruePeak = 0.0;
    rahmenSummeQuadrat = 0.0;
    rahmenSamples = 0;
    rahmenStartBlock = {};
    rahmenProjektDurchgehend = false;
    rahmenContinuousDurchgehend = false;
    rahmenMid2 = rahmenSide2 = 0.0;
    rahmenL = rahmenR = rahmenL2 = rahmenR2 = rahmenLR = 0.0;

    vorigesSpektrumGueltig = false; // Fluss: kein Vorgaenger ueber die Grenze
    flussStand = 0;                 // und keine Schwelle aus der alten Epoche
    flussGefuellt = 0;
    // SONDE-013 M-86: derselbe Grund fuer den Peakpfad. Eine Steigung
    // gegen den Rahmen VOR der Grenze vergliche zwei Stellen der Musik.
    vorigerRahmenPeak = 0.0;
    peakEreignisImRahmen = false;
    // SONDE-013 M-27: §32.3 nennt das Fingerprintfenster ausdruecklich
    // neben FFT, Loudness und Korrelation. Ein Fingerprint ueber zwei
    // Stellen der Musik beschriebe keine von beiden — und wuerde als
    // "dasselbe Material" gelesen.
    fingerprintLeeren();

    if (grund == Grenzgrund::lokaleLuecke)
    {
        ++segmentInEpoche;          // §32.3: SEGMENT, nicht Epoche
        ++zSegmentwechsel;
    }
    else
    {
        ++transportEpoche;
        segmentInEpoche = 0;        // Segmente zaehlen INNERHALB einer Epoche
        ++zEpochenwechsel;
    }
}

//== Samples =============================================================

inline bool FeatureEngine::blockProjektSpanneGueltig (const echtzeit::StampedBlock& b) noexcept
{
    const auto muss = echtzeit::kFlagZeitGueltig
                    | echtzeit::kFlagSpieltGueltig
                    | echtzeit::kFlagSpielt;
    if ((b.flags & muss) != muss)
        return false;
    return b.projectSampleStart
        <= std::numeric_limits<std::int64_t>::max()
             - static_cast<std::int64_t> (b.sampleCount);
}

inline bool FeatureEngine::blockContinuousSpanneGueltig (const echtzeit::StampedBlock& b) noexcept
{
    if ((b.flags & echtzeit::kFlagContinuousGueltig) == 0)
        return false;
    return b.continuousTimeSamples
        <= std::numeric_limits<std::int64_t>::max()
             - static_cast<std::int64_t> (b.sampleCount);
}

inline void FeatureEngine::evidenzContinuousBelegen (const echtzeit::StampedBlock& block) noexcept
{
    const bool gueltig = blockContinuousSpanneGueltig (block);
    if (! evidenzContinuousHabe)
    {
        evidenzContinuousHabe = true;
        evidenzContinuousDurchgehend = gueltig;
    }
    else if (evidenzContinuousDurchgehend)
    {
        evidenzContinuousDurchgehend
            = gueltig && block.continuousTimeSamples == evidenzContinuousErwartet;
    }

    std::int64_t ende = 0;
    if (evidenzContinuousDurchgehend
        && projektVorwaerts (block.continuousTimeSamples,
                             block.sampleCount, ende))
        evidenzContinuousErwartet = ende;
    else
        evidenzContinuousDurchgehend = false;
}

inline void FeatureEngine::rahmenZeitBelegen (const echtzeit::StampedBlock& block) noexcept
{
    if (rahmenSamples == 0)
    {
        rahmenStartBlock = block;
        rahmenProjektDurchgehend = blockProjektSpanneGueltig (block);
        rahmenContinuousDurchgehend = blockContinuousSpanneGueltig (block);
        return;
    }

    if (rahmenProjektDurchgehend && blockProjektSpanneGueltig (block))
    {
        std::int64_t erwartet = 0;
        rahmenProjektDurchgehend = projektVorwaerts (
                                      rahmenStartBlock.projectSampleStart,
                                      rahmenSamples, erwartet)
                                && erwartet == block.projectSampleStart;
    }
    else
        rahmenProjektDurchgehend = false;

    if (rahmenContinuousDurchgehend && blockContinuousSpanneGueltig (block))
    {
        std::int64_t erwartet = 0;
        rahmenContinuousDurchgehend = projektVorwaerts (
                                         rahmenStartBlock.continuousTimeSamples,
                                         rahmenSamples, erwartet)
                                   && erwartet == block.continuousTimeSamples;
    }
    else
        rahmenContinuousDurchgehend = false;
}

} // namespace nakama::analyse
