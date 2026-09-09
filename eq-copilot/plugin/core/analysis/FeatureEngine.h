// SONDE-009 (S12-13) — FeatureEngine v2: Zeit-, Validity-, Event- und
// Bandvertraege.  Entwurf §32.3, §33.1, §33.2, §53.7, §56; Gate-Text §65:
//
//                "Drop/Seek/Loop trennt jedes offene Fenster."
//
// ─────────────────────────────────────────────────────────────────────────────
// WAS DIESE SCHICHT IST — UND WAS SIE AUSDRUECKLICH NICHT IST.
//
// SONDE-008 hat den Weg vom Audiothread zum Worker gebaut: ganze Bloecke, nie
// Teilbloecke, mit Zeitstempel, und eine Ein-Block-Quarantaene, die einen Block
// erst freigibt, wenn sein Nachfolger beweist, dass er ihn fortsetzt.  Das
// Manifest dazu sagt woertlich (§4.3): "Nicht hier, sondern in SONDE-009:
// Epochen-, Segment- und Fensterbuchhaltung der Features.  Diese Schicht
// liefert nur die Grenze."
//
// Hier ist die andere Haelfte.  Die Quarantaene beantwortet "schliesst dieser
// Block an den vorigen an?"  Diese Engine beantwortet die Frage danach: "und
// was bedeutet das fuer die Fenster, die gerade offen sind?"  Die Antwort ist
// in §32.3 vorgeschrieben und laesst keinen Spielraum:
//
//   "Kein FFT-, Loudness-, Korrelations- oder Fingerprintfenster darf eine
//    ECHTE ODER MOEGLICHE Epochengrenze ueberbruecken."
//
// Nicht "soll moeglichst nicht".  Nicht "wird interpoliert".  Ein Fenster, das
// eine Grenze ueberbrueckt, mittelt zwei Stellen der Musik zu einer Zahl
// zusammen, und diese Zahl sieht danach aus wie eine Messung.  Sie ist keine.
//
// ─────────────────────────────────────────────────────────────────────────────
// WARUM ZWEI AUFLOESUNGSSTUFEN, UND WARUM DAS MIT DEM GATE ZU TUN HAT.
//
// Ein 1/24-Oktav-Band bei 30 Hz ist 0,88 Hz breit.  Eine 4096-Punkt-FFT bei
// 48 kHz hat 11,7 Hz Binbreite — das unterste Band enthaelt dort keinen
// einzigen Bin und ist schlicht nicht messbar.  Deshalb zwei Stufen: 16384 fuer
// alles unter 200 Hz, 4096 darueber (dieselbe Aufteilung wie in `AnalyseEngine`,
// nur ohne deren zwei Zusatzstufen, die der `analyze-track`-Achse dienen).
//
// 🔑 Das ist nicht nur Genauigkeit, es ist die eigentliche BEWEISFLAECHE dieses
// Tickets.  Zwei Stufen heissen zwei GLEICHZEITIG OFFENE Fenster mit
// verschiedener Laenge: bei 48 kHz deckt das Bassfenster 341 ms ab, das
// Hauptfenster 85 ms.  Ein Fehler in der Trennung waere mit nur EINER Stufe
// womoeglich unsichtbar (das kurze Fenster ist an einer Grenze oft ohnehin
// fast leer), mit zwei Stufen faellt er auf: das lange Fenster reicht ueber
// Grenzen, die das kurze laengst hinter sich hat.  Der Golden faehrt genau das.
//
// ─────────────────────────────────────────────────────────────────────────────
// GRUNDGESETZ.  Diese Engine laeuft im WORKER, nie im Audiothread — sie sieht
// nur bereits versiegelte Bloecke.  Trotzdem entsteht ihr gesamter Speicher in
// `vorbereiten()`: eine Allokation je Fenster bei 10 Hz waere Muell in einem
// Blatt, das "feste Obergrenzen" verspricht (§33 "feste Obergrenzen fuer
// Sonden, Baender, Ereignisse, Queue-Tiefe").  Kein Ring waechst, keine Liste
// waechst, der Ereignisstrom ist gedeckelt und zaehlt seine Verluste.
//
// NaN-EHRLICHKEIT.  Ein nicht messbarer Bandwert wird NIE als Zahl ausgegeben:
// er bekommt Bitmap-Bit 0.  Das gilt fuer drei verschiedene Gruende, und die
// Engine haelt sie auseinander: (a) das Band enthaelt bei dieser Aufloesung
// keinen Bin, (b) das Band liegt ueber der Nyquist-Kappe, (c) das Band hat in
// diesem Rahmen keine Energie gesehen.  Alle drei heissen "keine Aussage" —
// keiner davon heisst "0 dB".
#pragma once

#include "BandGrid.h"
#include "Fft.h"
#include "KGewichtung.h"
#include "Konfidenz.h"
#include "TruePeak.h"
#include "../StampedAudioQueue.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>


// NAK-225/S25d: der Vertrag steht in einem eigenen Teilkopf. Er wird VOR
// der Klasse eingebunden, weil die Klasse seine Typen in Feldern und
// Signaturen fuehrt.
#define NAKAMA_FEATUREENGINE_TEIL 1
#include "featureengine/Vertrag.h"

namespace nakama::analyse
{

//==============================================================================
class FeatureEngine
{
public:
    // ── Feste Groessen (§53.7 Schlussabsatz: Startwerte, keine ABI) ─────────
    /** Bassstufe: aufloesungsbestimmend unter `kTrennungHz`. */
    static constexpr int kBassPunkte = 16384;
    /** Hauptstufe. */
    static constexpr int kHauptPunkte = 4096;
    /** Ueberlappung 50 % — die Hann-Fenster addieren sich damit zu 1. */
    static constexpr int kBassHop  = kBassPunkte / 2;
    static constexpr int kHauptHop = kHauptPunkte / 2;
    /** Zustaendigkeitsgrenze der zwei Stufen. */
    static constexpr double kTrennungHz = 200.0;
    /** Obergrenze der Messbarkeit: 18 kHz ODER 0,95·Nyquist, was kleiner ist.
        Dieselbe Kappe wie in `AnalyseEngine` — darueber bleibt es leer, statt
        eine fortgeschriebene Randevidenz zu behaupten. */
    static constexpr double kObergrenzeHz = 18000.0;
    static constexpr double kNyquistAnteil = 0.95;
    /** Aktivitaetsgate, wie M1. */
    static constexpr double kAktivGateDb = -60.0;
    /** Zellenlaenge der Loudness- und Korrelationsfenster. */
    static constexpr double kZelleSekunden = 0.1;
    /** LUFS-S ueber 3 s = 30 Zellen (§39.1). */
    static constexpr int kKurzZellen = 30;
    /** Momentary ueber 400 ms = 4 Zellen (§39.1, BS.1770-5 Blocklaenge).

        Er ist KEIN kuerzeres Short-term und keine Glaettung von `lufsS`: die
        zwei Fenster beantworten verschiedene Fragen (was ist gerade laut,
        gegen was traegt die Passage), und ein Empfaenger, der nur eines
        bekommt, kann das andere nicht daraus rechnen — deshalb reisen sie
        nebeneinander (M-01). */
    static constexpr int kMomentZellen = 4;
    /** LRA gilt erst ab rund 60 s GEEIGNETEN Materials (§39.1, EBU Tech 3342).

        "Geeignet" heisst: ueber dem absoluten Gate.  Eine Minute Stille ist
        keine Minute Material — deshalb zaehlt `lraGezaehlt` nur gegatete
        Kurzzeitwerte, nicht die Wanduhr (dieselbe Regel wie bei der
        Abdeckung, §48.2). */
    static constexpr double kLraMindestSekunden = 60.0;
    /** Hop der LRA-Kurzzeitfolge: 1 s = 10 Zellen (EBU Tech 3342 §2.2). */
    static constexpr int kLraHopZellen = 10;
    /** Absolutes Gate der LRA-Verteilung in LUFS (EBU Tech 3342: -70). */
    static constexpr double kLraAbsGateLufs = -70.0;
    /** Relatives Gate der LRA-Verteilung, LU unter dem gegateten Mittel
        (EBU Tech 3342: -20; das ist NICHT die -10 LU der integrierten
        Lautheit, und die zwei nicht zu vermischen ist der ganze Punkt). */
    static constexpr double kLraRelGateLu = -20.0;
    /** Histogramm der LRA-Kurzzeitwerte: 0,1 LU von -70 bis +30 LUFS.

        Fester Speicher wie beim `LoudnessAccumulator` — ein wachsender Vektor
        von Kurzzeitwerten waere bei einer langen Sitzung genau der unbegrenzte
        Bestand, den §48.1 ausschliesst.  0,1 LU Binbreite ist zehnmal feiner
        als die 1 LU, in der LRA berichtet wird. */
    static constexpr double kLraBinUnten  = -70.0;
    static constexpr double kLraBinBreite = 0.1;
    static constexpr int    kLraBins      = 1001;
    /** Livekadenz 10 Hz, Evidenzkadenz 1 bis 4 Hz (§33.2). */
    static constexpr double kLiveIntervallS    = 0.1;
    /** Schnellster und langsamster zulaessiger Evidenzabstand.

        §33.2 nennt fuer den Evidenzsnapshot ausdruecklich eine SPANNE, keinen
        Punkt: 1 bis 4 Hz. Der Grund steht in M-05 — bei Ueberlast wird die
        KADENZ reduziert, nie der Inhalt verworfen. Genau deshalb liegt die
        Reduktion hier in der Engine und nicht beim Sender: wer einen
        faelligen Snapshot einfach nicht sendete, wuerde sein Fenster trotzdem
        leeren und die Messung stillschweigend wegwerfen. Ein laengeres
        Fenster liefert dagegen weniger, aber VOLLSTAENDIGE Snapshots. */
    static constexpr double kEvidenzIntervallMinS = 0.25;   // 4 Hz
    static constexpr double kEvidenzIntervallMaxS = 1.0;    // 1 Hz
    /** Feste Obergrenze des Ereignisstroms (§33 "feste Obergrenzen"). */
    static constexpr int kEreignisPlaetze = 64;
    /** Historie der adaptiven Flussschwelle (Median/MAD, §39.1). */
    static constexpr int kFlussHistorie = 16;

    //== Einrichtung ==========================================================

    /** Legt allen Speicher an.  Idempotent bei gleicher Samplerate; bei einer
        anderen ist es ein Neuanfang — §32.3 fuehrt den Sampleratewechsel
        ausdruecklich als Epochengrenze, und eine Bin-Zuordnung aus der alten
        Rate waere danach schlicht falsch. */
    void vorbereiten (double samplerate)
    {
        if (! std::isfinite (samplerate) || samplerate <= 0.0 || samplerate > 768000.0)
        {
            // Ein ungueltiger Folge-Prepare darf keine zuvor gueltige Engine
            // unter der alten Rate weiterlaufen lassen. Speicher bleibt
            // angelegt; nur Messzustand und Betriebsfreigabe verfallen.
            if (vorbereitet)
                zuruecksetzen();
            vorbereitet = false;
            sr = 0.0;
            return;
        }
        if (samplerate == sr && vorbereitet)
            return;

        sr = samplerate;
        bass.vorbereiten (kBassPunkte, sr);
        haupt.vorbereiten (kHauptPunkte, sr);

        zuordnung (bass,  0,            trennIndex());
        zuordnung (haupt, trennIndex(), Gitter::evidenzBaender);
        auskunftBestimmen();          // erst wenn BEIDE Stufen zugeordnet sind

        zellenSamples = (int) std::llround (kZelleSekunden * sr);
        if (zellenSamples < 1) zellenSamples = 1;
        kurzZellen.assign ((std::size_t) kKurzZellen, 0.0);
        // SONDE-013 M-03/M-04: zwei weitere Zellenringe derselben Laenge.
        // `kurzTpZellen` traegt das True-Peak-Maximum je Zelle (PSR rechnet
        // gegen das Maximum DESSELBEN 3-s-Fensters, §39.1), `kurzRmsZellen`
        // die UNGEWICHTETE Energie (Crest ist ein Pegelverhaeltnis, keine
        // Lautheit — die K-Gewichtung gehoert nicht hinein).
        kurzTpZellen.assign ((std::size_t) kKurzZellen, 0.0);
        kurzRmsZellen.assign ((std::size_t) kKurzZellen, 0.0);
        lraHistogramm.assign ((std::size_t) kLraBins, 0u);
        headroomRing.assign (1u, VerteilungsRing {});
        // SONDE-013 M-11: alle Stereotraeger im Heap, angelegt auf dem
        // Nachrichtenthread. Der Audiothread alloziert weiterhin nie.
        stereoAkku.assign ((std::size_t) Gitter::evidenzBaender, StereoAkku {});
        stereoKurz.assign ((std::size_t) Gitter::evidenzBaender, StereoAkku {});
        stereoVerlauf.assign ((std::size_t) Gitter::evidenzBaender, VerteilungsRing {});
        stereoErgebnis.assign ((std::size_t) Gitter::evidenzBaender, StereoBandwert {});
        stereoKorrKurz.assign ((std::size_t) Gitter::evidenzBaender, 0.0f);
        stereoKorrKurzGesetzt.assign ((std::size_t) Gitter::evidenzBaender, 0u);
        stereoKurzfensterBand.assign ((std::size_t) Gitter::evidenzBaender, 0u);
        stereoPersistenzZaehler.assign ((std::size_t) Gitter::evidenzBaender, 0u);
        fpBandSumme.assign ((std::size_t) Fingerprint::kBaender, 0.0);
        fpBandAnzahl.assign ((std::size_t) Fingerprint::kBaender, 0u);
        fpChromaSumme.assign ((std::size_t) Fingerprint::kChroma, 0.0);
        fpOnset.assign ((std::size_t) Fingerprint::kOnsets, 0.0);
        tp.vorbereiten (sr);

        evidenzVerteilung.assign ((std::size_t) Gitter::evidenzBaender,
                                  VerteilungsRing {});
        ereignisse.assign ((std::size_t) kEreignisPlaetze, Ereignis {});
        flussHistorie.assign ((std::size_t) kFlussHistorie, 0.0);
        flussSortiert.assign ((std::size_t) kFlussHistorie, 0.0);
        vorigesSpektrum.assign ((std::size_t) Gitter::evidenzBaender, 0.0);

        kL.entwerfen (sr);
        kR.entwerfen (sr);

        vorbereitet = true;
        zuruecksetzen();
    }

    /** Alles auf Anfang — Gegenpfad zu `vorbereiten()`.  Zaehlt NICHT als
        Grenze: wer zuruecksetzt, hat keine Messung mehr, die getrennt werden
        muesste. */
    void zuruecksetzen() noexcept
    {
        bass.leeren();
        haupt.leeren();
        for (auto& v : liveAkku)    v = { 0.0, 0 };
        for (auto& v : evidenzAkku) v = { 0.0, 0 };
        for (auto& v : liveBreiteAkku)    v = { 0.0, 0.0 };
        for (auto& r : evidenzVerteilung) r.leeren();   // SONDE-013 M-05
        stereoLeeren();                                // SONDE-013 M-11
        for (auto& e : stereoErgebnis) e = StereoBandwert {};
        stereoSkalareErgebnis = StereoSkalare {};
        evidenzFensterGesamt = 0;
        evidenzFensterAktiv = 0;
        liveSupport = {};
        evidenzSupport = {};

        zelleStand = 0;
        zelleKEnergie = 0.0;
        zelleAktivEnergie = 0.0;
        kurzStand = 0;
        kurzGefuellt = 0;
        for (auto& z : kurzZellen) z = 0.0;
        kL.zustandNullen();
        kR.zustandNullen();

        // SONDE-013 M-02 bis M-04. Hier faellt AUCH, was eine Grenze
        // ueberlebt: `zuruecksetzen()` ist der Neuanfang der PASSAGE, und die
        // Passagengroessen — Passagen-True-Peak, Headroomverteilung und das
        // LRA-Histogramm — gehoeren genau dorthin.
        for (auto& z : kurzTpZellen)  z = 0.0;
        for (auto& z : kurzRmsZellen) z = 0.0;
        zelleTruePeak = 0.0;
        zelleRmsEnergie = 0.0;
        tp.zuruecksetzen();
        rahmenTruePeak = 0.0;
        passageTruePeak = 0.0;
        passagenTruePeakRahmen = 0.0;
        // M-25: `zuruecksetzen()` ist der Neuanfang der ganzen Engine
        // (`prepareToPlay`). Ein Passagenfenster aus dem vorigen Anlauf ist
        // danach keine Aussage mehr — es faellt GANZ, nicht als "gebrochen".
        // Der Besitzer setzt es neu, wenn die Passage noch gilt.
        passagenfenster = {};
        passagenfensterGebrochen = false;
        hatSampleAusserhalb = false;
        verarbeiteteSamples = 0;
        letztesSampleAusserhalb = 0;
        zelleImFensterSamples = 0;
        // M-07: die drei NaN-Zaehler beginnen mit der Engine von vorn.
        rahmenNichtEndlich = 0;
        evidenzNichtEndlich = 0;
        nichtEndlicheSamplesGesamt = 0;
        for (auto& r : headroomRing) r.leeren();
        for (auto& b : lraHistogramm) b = 0u;
        lraGezaehlt = 0;
        lraZellenSeitHop = 0;

        rahmenPeak = 0.0;
        rahmenSummeQuadrat = 0.0;
        rahmenSamples = 0;
        rahmenStartBlock = {};
        rahmenProjektDurchgehend = false;
        rahmenContinuousDurchgehend = false;
        rahmenMid2 = rahmenSide2 = 0.0;
        rahmenL = rahmenR = rahmenL2 = rahmenR2 = rahmenLR = 0.0;
        rahmenAktivZellen = 0;
        rahmenZellen = 0;

        liveSamples = 0;
        evidenzSamples = 0;
        evidenzContinuousHabe = false;
        evidenzContinuousDurchgehend = true;
        evidenzContinuousErwartet = 0;
        for (auto& v : vorigesSpektrum) v = 0.0;
        vorigesSpektrumGueltig = false;
        flussStand = 0;
        flussGefuellt = 0;
        vorigerRahmenPeak = 0.0;
        peakEreignisImRahmen = false;
        fingerprintLeeren();

        ereignisStand = 0;
        ereignisAnzahl = 0;

        habeVorigen = false;
        transportEpoche = 0;
        segmentInEpoche = 0;
        sequenz = 0;
        frameBereit = false;
        aktuell = FeatureFrame {};

        zGetrennteFenster = 0;
        zVerworfeneFensterSamples = 0;
        zEpochenwechsel = 0;
        zSegmentwechsel = 0;
        zStraddleVerworfen = 0;
        zEreignisseVerworfen = 0;
        zNak29Abgelehnt = 0;
        zBloecke = 0;
        zVerworfeneBandfenster = 0;
        for (auto& g : grundZaehler) g = 0;
    }

    //== Betrieb (Workerthread) ===============================================

    /** Nimmt EINEN versiegelten Block auf.

        `interleaved` traegt `2 * block.sampleCount` floats (L/R), genau das,
        was `Blockquarantaene::Freigabe::audio` liefert.

        Rueckgabe true heisst: `frame()` traegt einen neuen, vollstaendigen
        Frame.  Rueckgabe false heisst nur "noch nicht fertig" — nie einen
        Fehler; ein Block, der an einer Grenze faellt, ist kein Fehler, sondern
        die Regel dieses Tickets bei der Arbeit. */
    bool nimmBlock (const echtzeit::StampedBlock& block, const float* interleaved) noexcept
    {
        frameBereit = false;
        if (! vorbereitet || interleaved == nullptr || block.sampleCount == 0)
            return false;
        ++zBloecke;

        // ── 1. Schliesst der Block an? ──────────────────────────────────────
        const Grenzgrund grund = habeVorigen ? grenzeZwischen (vorigerBlock, block)
                                             : Grenzgrund::keine;
        if (grund != Grenzgrund::keine)
            grenzeZiehen (grund);

        // ── 2. Koennte die Schleifengrenze IN diesem Block liegen? ──────────
        // §32.3: bei bewiesener Abbildung wuerde der Block logisch GETEILT;
        // ohne Beweis wird er als moeglicher Straddle verworfen.  Heute gibt es
        // kein FL-Golden fuer die Abbildung (Capabilityreport S4), also gilt
        // immer der zweite Zweig — und das steht hier als Code, nicht als
        // Absicht: sobald `cycle_derivation` je `validated_block_mapping`
        // liefern darf, faellt dieser Zweig von selbst weg.
        if (moeglicherStraddleIn (block))
        {
            grenzeZiehen (Grenzgrund::moeglicherStraddle);
            ++zStraddleVerworfen;
            zVerworfeneFensterSamples += block.sampleCount;
            vorigerBlock = block;
            habeVorigen = true;
            return false;                     // der Block selbst geht nirgends ein
        }

        // ── 3. Samples in alle offenen Fenster ──────────────────────────────
        verarbeiteSamples (block, interleaved);

        vorigerBlock = block;
        habeVorigen = true;

        // ── 4. Kadenz: ist ein Frame faellig? ───────────────────────────────
        const double liveS = (double) liveSamples / sr;
        if (liveS >= kLiveIntervallS)
            frameBereit = baueFrame();

        return frameBereit;
    }

    /** Der zuletzt gebaute Frame.  Nur nach einem `nimmBlock() == true` frisch. */
    const FeatureFrame& frame() const noexcept { return aktuell; }

    /** Die Ereignisse des letzten Rahmens, aeltestes zuerst. */
    /** Der Fingerprint der laufenden Passage (SONDE-013 M-26).

        Er wird bei JEDEM Aufruf frisch aus den Akkumulatoren gebaut, nicht
        zwischengespeichert: der Aufrufer entscheidet, wann eine Passage
        endet, und ein gecachter Fingerprint waere dann die Antwort auf eine
        Frage, die niemand gestellt hat. Ohne genug Fenster traegt er kein
        Bit. */
    Fingerprint fingerprint() const noexcept { return fingerprintJetzt(); }

    /** Die bandweise Stereoevidenz des zuletzt ausgewerteten
        Evidenzfensters (SONDE-013 M-11).

        Sie liegt bewusst NICHT im `FeatureFrame` - die Begruendung steht bei
        `StereoBandwert`. Der Serialisierer holt sie hier, genau wie den
        Ereignisring daneben. Gueltig ist sie, solange der zuletzt
        veroeffentlichte Frame `evidenzFrisch` trug; danach fuellt sich das
        naechste Fenster. */
    const StereoBandwert& stereoBand (int b) const noexcept
    { return stereoErgebnis[(std::size_t) b]; }
    /** Mono-Folddown und L/R-Balance desselben Fensters (M-08). */
    const StereoSkalare& stereoSkalare() const noexcept
    { return stereoSkalareErgebnis; }
    /** Ob ueberhaupt ein Band eine Basis traegt - der Riegel, mit dem ein
        Erzeuger entscheidet, ob er das `stereo`-Feld ueberhaupt schreibt.
        Ein Satz aus 221 leeren Baendern waere 11 KiB Schweigen auf der
        Leitung. */
    bool stereoHatInhalt() const noexcept
    {
        for (const auto& e : stereoErgebnis)
            if (e.basisGesetzt)
                return true;
        return false;
    }

    //== Passagenfenster (SONDE-013 M-25) =====================================
    //
    // Der EINE Produktpfad zwischen der gespeicherten Passage und der Engine.
    // Er laeuft auf demselben Thread wie `verarbeiteSamples`/`auswerten`
    // (Analyse-Worker), nicht im Audiothread — deshalb genuegt einfacher
    // Zustand ohne Atomics, wie bei `evidenzIntervallSetzen` daneben.

    /** Bindet die Passagenmetriken an [startSample, endeSample) in Projektzeit.

        Die vier Traeger aus M-03/M-04/M-26 — Passagenmaximum, Headroomring,
        LRA-Histogramm und Fingerprint — beginnen dabei VON VORN. Genau das war
        der Fehler ohne Fenster: eine neue Passage erbte die Spitze und die
        Verteilungen des Materials davor.

        `false`, wenn das Fenster leer oder verdreht ist — dann bleibt der
        vorige Zustand unangetastet, statt eine Passage der Laenge 0 zu
        fuehren. */
    bool setzePassagenfenster (std::int64_t startSample, std::int64_t endeSample,
                               std::uint64_t transportEpocheDerPassage) noexcept
    {
        if (endeSample <= startSample)
            return false;
        // 🔑 Nacharbeit 2 (Befund R03, Paragraph 32.4): eine Passage bindet an
        // GENAU EINE Transportepoche. Wurde sie unter einer anderen markiert,
        // ist sie hier keine Aussage mehr — der Seek dazwischen hat die
        // Projektzeit umgehaengt, und dieselben Samplegrenzen zeigen jetzt auf
        // anderes Material. Fail-closed: das Fenster entsteht gar nicht erst.
        if (transportEpocheDerPassage != transportEpoche)
            return false;
        passagenfenster.gesetzt = true;
        passagenfenster.startSample = startSample;
        passagenfenster.endeSample = endeSample;
        passagenfenster.epoche = transportEpocheDerPassage;
        passagenfensterGebrochen = false;
        hatSampleAusserhalb = true;                 // alles VOR dem Fenster zaehlt nicht
        letztesSampleAusserhalb = verarbeiteteSamples;
        passageTruePeak = 0.0;
        passagenTruePeakRahmen = 0.0;
        zelleImFensterSamples = 0;
        // 🔑 Der Polyphasenfilter wird geleert. Ein Passagenanfang IST eine
        // Fenstergrenze (§32.3): seine 24 Taps je Phase reichen zwoelf Samples
        // vor den Anfang zurueck, und ohne diesen Reset trug der erste
        // Passagenrahmen den Nachklang des Materials DAVOR. Genau daran hing
        // der Befund B08: eine leise Passage nach einem lauten Abschnitt
        // uebernahm dessen Spitze — nicht ueber einen Puffer, sondern ueber den
        // Filterzustand, die subtilste Form desselben Fehlers.
        //
        // Der Nachlaufwert wird VERWORFEN: er gehoert zum Material vor der
        // Passage, und die Passage beginnt bei null.
        (void) tp.nachlauf();
        tp.zuruecksetzen();
        for (auto& r : headroomRing) r.leeren();
        for (auto& b : lraHistogramm) b = 0u;
        lraGezaehlt = 0;
        lraZellenSeitHop = 0;
        // 🔑 SONDE-013 Nacharbeit 2 (Befund R04): DAS KURZZEITFENSTER FAELLT
        // MIT.
        //
        // Die Runde 1 leerte Histogramm und Hop-Zaehler, nicht aber die
        // 3-s-Ringe `kurzZellen`, `kurzTpZellen`, `kurzRmsZellen` und die
        // laufende Zelle. Beginnt eine Passage nach bereits gemessenem
        // Material — und das ist der Normalfall, der User markiert mitten im
        // Stueck —, enthielten ihre ersten LRA-, PSR- und Crest-Fenster bis zu
        // zwei Sekunden Audio VOR der Passage. Der Fehler ist derselbe wie an
        // der Transportgrenze eine Bildschirmseite weiter unten, nur an der
        // anderen Fensterart; deshalb faellt hier dasselbe.
        zelleStand = 0;
        zelleKEnergie = 0.0;
        zelleAktivEnergie = 0.0;
        zelleTruePeak = 0.0;
        zelleRmsEnergie = 0.0;
        kurzStand = 0;
        kurzGefuellt = 0;
        for (auto& z : kurzZellen)     z = 0.0;
        for (auto& z : kurzTpZellen)   z = 0.0;
        for (auto& z : kurzRmsZellen)  z = 0.0;
        fingerprintLeeren();
        return true;
    }

    /** Loest die Bindung. Danach gilt wieder die Transportepoche als Fenster —
        der Fall "der User hat keine Passage markiert". */
    void loeschePassagenfenster() noexcept
    {
        passagenfenster = {};
        passagenfensterGebrochen = false;
        hatSampleAusserhalb = false;
        zelleImFensterSamples = 0;
        passageTruePeak = 0.0;
        passagenTruePeakRahmen = 0.0;
        for (auto& r : headroomRing) r.leeren();
        for (auto& b : lraHistogramm) b = 0u;
        lraGezaehlt = 0;
        fingerprintLeeren();
    }

    bool passagenfensterGesetzt() const noexcept { return passagenfenster.gesetzt; }
    /** `false` heisst: eine Transportgrenze lief durch das Fenster, und die
        Passagenmetriken bleiben leer, bis ein neues Fenster gesetzt wird. */
    bool passagenfensterIntakt() const noexcept
    { return passagenfenster.gesetzt && ! passagenfensterGebrochen; }
    std::int64_t passagenfensterStart() const noexcept { return passagenfenster.startSample; }
    std::int64_t passagenfensterEnde() const noexcept  { return passagenfenster.endeSample; }
    /** Die Transportepoche, an die das Fenster gebunden ist (Paragraph 32.4). */
    std::uint64_t passagenfensterEpoche() const noexcept { return passagenfenster.epoche; }

    /** SONDE-013 M-07: nicht-endliche Eingangssamples seit `zuruecksetzen()`.
        0 heisst nachweislich keines, nicht "nicht gemessen". */
    std::uint64_t nichtEndlicheSamples() const noexcept { return nichtEndlicheSamplesGesamt; }
    /** Dieselbe Zahl fuer das laufende EVIDENZfenster — sie reist mit dem
        Beleg und deckelt dort die Konfidenzklasse. */
    std::uint32_t nichtEndlicheSamplesImEvidenzfenster() const noexcept
    { return evidenzNichtEndlich; }

    int ereignisAnzahlJetzt() const noexcept { return ereignisAnzahl; }
    const Ereignis& ereignis (int i) const noexcept
    {
        const int erstes = (ereignisStand - ereignisAnzahl + kEreignisPlaetze) % kEreignisPlaetze;
        return ereignisse[(std::size_t) ((erstes + i) % kEreignisPlaetze)];
    }

    //== Telemetrie ===========================================================
    // NAK-57: diese Zahlen bekommen in DIESEM Ticket keine Anzeige — die
    // Oberflaeche kommt aus Figma.  Sie sind maschinenlesbar, und das ist die
    // ganze Zusage.

    std::uint64_t getrennteFenster() const noexcept        { return zGetrennteFenster; }
    std::uint64_t verworfeneFensterSamples() const noexcept { return zVerworfeneFensterSamples; }
    std::uint64_t epochenwechsel() const noexcept          { return zEpochenwechsel; }
    std::uint64_t segmentwechsel() const noexcept          { return zSegmentwechsel; }
    std::uint64_t straddleVerworfen() const noexcept       { return zStraddleVerworfen; }
    std::uint64_t ereignisseVerworfen() const noexcept     { return zEreignisseVerworfen; }
    std::uint64_t nak29Abgelehnt() const noexcept          { return zNak29Abgelehnt; }
    std::uint64_t bloeckeGesehen() const noexcept          { return zBloecke; }
    /** Wie oft aus Grund `g` getrennt wurde.

        ⚠️ DER BEREICHSRIEGEL IST NICHT ZIERAT (T2-3, 23.08.).  `grundZaehler`
        hat exakt `anzahl` Elemente, `Grenzgrund::anzahl` ist ein oeffentlich
        sichtbarer Enumwert — und `grenzenMitGrund (Grenzgrund::anzahl)` las
        damit EIN ELEMENT HINTER DEM ENDE, ausgerechnet hinter dem letzten
        Member der Klasse.  Der Selbstaudit-Fix `48fcd9c` hat den SCHREIB-
        Ueberlauf geschlossen (Array an die Aufzaehlung gekoppelt statt an eine
        Zahl daneben) und dabei den LESE-Ueberlauf erst aufgemacht: vorher lag
        `[9]` in einem `[10]`-Array noch im Puffer.  🔑 Dieselbe Sorte Landmine
        wie der Fund, den sie ersetzt hat — eine Zahl neben einer Aufzaehlung,
        an der niemand rot wird. */
    std::uint64_t grenzenMitGrund (Grenzgrund g) const noexcept
    {
        const auto i = (std::size_t) g;
        if (i >= (std::size_t) Grenzgrund::anzahl)
            return 0;                   // `anzahl` ist kein Grund, also null Grenzen
        return grundZaehler[i];
    }
    std::uint64_t transportEpocheJetzt() const noexcept { return transportEpoche; }
    std::uint64_t segmentJetzt() const noexcept         { return segmentInEpoche; }

    /** Fuellstand der zwei Fenster in Samples — der direkte Beleg des
        Gate-Textes: nach einer Grenze steht hier 0, und zwar in BEIDEN. */
    int fuellstandBass() const noexcept  { return bass.gefuellt; }
    int fuellstandHaupt() const noexcept { return haupt.gefuellt; }
    int fuellstandLoudnessZelle() const noexcept { return zelleStand; }
    int fuellstandKurzLoudness() const noexcept  { return kurzGefuellt; }
    bool flussHatVorgaenger() const noexcept     { return vorigesSpektrumGueltig; }

    /** Fuellstand der BANDAKKUS — der Traeger, an dem T2-1 unsichtbar war.

        🔑 DIESE VIER AUSKUENFTE EXISTIEREN WEGEN EINES LOCHS IM EIGENEN BEIN,
        genau wie `kFilterZustand()` unten.  `keinFensterUeberbrueckt()` fragte
        bis zum 24.08. fuenf Fuellstaende ab und KEINEN Akkumulator; deshalb war
        B5 gruen, waehrend ein Frame unter neuem Epochenstempel den Ton von vor
        der Grenze meldete.  Ein Fuellstand ist eben nicht dasselbe wie ein
        Integrationsfenster — wer nur ihn misst, misst die halbe Zusage. */
    int liveAkkuBelegteBaender() const noexcept
    {
        int n = 0;
        for (const auto& v : liveAkku) if (v.n > 0) ++n;
        return n;
    }
    int evidenzAkkuBelegteBaender() const noexcept
    {
        int n = 0;
        for (const auto& v : evidenzAkku) if (v.n > 0) ++n;
        return n;
    }
    /** SONDE-013 M-05: derselbe Grund, ein Traeger weiter.

        Der Verteilungsring traegt den VERLAUF der Bandwerte, nicht ihre
        Summe — also ist er ein eigenes offenes Fenster und braucht seine
        eigene Auskunft. Ohne sie waere `keineAkkusUeberleben()` in B5
        wieder eine Liste, an der ein neuer Traeger vorbeikaeme; genau
        daran ist T2-1 entstanden. Gezaehlt werden BELEGTE PLAETZE ueber
        alle Baender, nicht Baender: ein einzelner ueberlebender Wert soll
        sichtbar sein. */
    std::uint64_t evidenzVerteilungPlaetze() const noexcept
    {
        std::uint64_t n = 0;
        for (const auto& r : evidenzVerteilung) n += (std::uint64_t) r.gefuellt;
        return n;
    }
    /** Fensterzaehler der Abdeckung: gesamt und aktiv. Beide muessen an einer
        Grenze fallen, sonst waere die Abdeckung ein Anteil ueber zwei
        Epochen. */
    std::uint64_t evidenzFensterGesamtJetzt() const noexcept { return evidenzFensterGesamt; }
    std::uint64_t evidenzFensterAktivJetzt()  const noexcept { return evidenzFensterAktiv; }

    /** Fingerprint- und Stereofenster — die zwei Traeger aus der Liste in
        `grenzeZiehen()`, die bis NAK-181 keinen Leser hatten.

        🔑 NAK-181 Nacharbeit 2 (WP1-2/WN-02).  Die Matrixzeile N-35 sagt
        zu, dass die Grenze FUENF Fenster leert — Rahmen, Loudness, Spektrum,
        Fingerprint, Stereo —, „gemessen an den Zaehlern".  Zwei davon hatten
        gar keinen Zaehler nach aussen, also konnte B16 die Zusage an ihnen
        nicht messen.  Gemessen statt behauptet: die Rotlagen b und c in
        `docs/beweise/roh/NAK-181-rot-n2-wn02.txt` nehmen `fingerprintLeeren()`
        beziehungsweise `stereoLeeren()` aus `grenzeZiehen()` heraus, und erst
        mit diesen zwei Auskuenften faellt je genau eine Zeile.  Dieselbe Sorte
        Loch wie T2-1, nur eine Etappe spaeter.

        Rein lesend, kein Verhalten: `fpFenster` zaehlt die Welch-Frames im
        Fingerprintakkumulator, `stereoAkku[b].frames` die Frames je Band. */
    std::uint32_t fingerprintFenster() const noexcept { return fpFenster; }
    int stereoAkkuBelegteBaender() const noexcept
    {
        int n = 0;
        for (const auto& a : stereoAkku) if (a.frames > 0) ++n;
        return n;
    }

    /** SONDE-013 M-05: Evidenzkadenz zwischen 1 und 4 Hz einstellen.

        Der Wert wird auf `[kEvidenzIntervallMinS, kEvidenzIntervallMaxS]`
        GEKLEMMT statt abgelehnt: der Aufrufer ist der Sender, der auf
        Rueckstau reagiert, und ein abgelehnter Wunsch liesse ihn mit einer
        Kadenz weiterlaufen, die er gerade nicht bedienen kann. Nichtendliches
        aendert nichts — eine kaputte Zahl darf die Kadenz nicht verstellen.

        Wirkt ab dem naechsten faelligen Snapshot; ein bereits offenes
        Evidenzfenster wird NICHT abgeschnitten. */
    void evidenzIntervallSetzen (double sekunden) noexcept
    {
        if (! std::isfinite (sekunden))
            return;
        evidenzIntervallS = std::clamp (sekunden, kEvidenzIntervallMinS,
                                        kEvidenzIntervallMaxS);
    }
    double evidenzIntervallJetzt() const noexcept { return evidenzIntervallS; }

    /** Der Evidenzsnapshot hat die Ereignisse UEBERNOMMEN.

        Gegenpfad zu `ereignisAblegen`: ohne ihn traegt der naechste Snapshot
        dieselben Ereignisse noch einmal, und ein Empfaenger zaehlte einen
        Transienten mehrfach. Der Verlustzaehler bleibt stehen — er ist
        laufgebunden und wird vom Sender als Differenz gelesen. */
    void ereignisseEntnommen() noexcept { ereignisAnzahl = 0; }
    /** Betragssumme der Breiten-Akkus — sie tragen KEIN `n`, also braucht es
        die Summe selbst, sonst bliebe dieser dritte Akku ungemessen. */
    double liveBreiteAkkuZustand() const noexcept
    {
        double su = 0.0;
        for (const auto& v : liveBreiteAkku) su += std::abs (v.seite) + std::abs (v.gesamt);
        return su;
    }
    /** Wie viele Baender im Breiten-Akku ueberhaupt etwas tragen.

        🔑 DIESE AUSKUNFT EXISTIERT WEGEN EINER DIAGNOSEZEILE, DIE LOG (T2R2-4,
        24.08.).  `liveBreiteAkkuZustand()` liegt beim Signal der G-Faelle bei
        2,6e-04; mit drei Nachkommastellen gedruckt stand in der Diagnosezeile
        `Breite=0.000` — VOR und NACH der Grenze, also sah der Traeger in jeder
        roten Zeile unauffaellig aus.  Eine Diagnosezeile, die bei einem echten
        Bruch alle Traeger unschuldig aussehen laesst, ist SCHLIMMER als keine:
        sie erzeugt Vertrauen, das sie nicht deckt.  Eine ANZAHL kann nicht auf
        null runden — sie ist entweder 0 oder sie ist es nicht. */
    int liveBreiteAkkuBelegteBaender() const noexcept
    {
        int n = 0;
        for (const auto& v : liveBreiteAkku)
            if (v.seite != 0.0 || v.gesamt != 0.0) ++n;
        return n;
    }
    /** Fertige Zellen des laufenden Rahmens.  `rahmenAktivZellen` kann nie
        groesser sein (beide wachsen in `zelleSchliessen()`, die aktive nur
        bedingt) — 0 hier heisst also 0 in beiden. */
    std::uint64_t rahmenZellenJetzt() const noexcept      { return rahmenZellen; }
    std::uint64_t rahmenAktivZellenJetzt() const noexcept { return rahmenAktivZellen; }

    /** Die zwei KADENZ-Zaehler.  Sie ueberleben eine Grenze absichtlich
        (§10.1: die Grenze schneidet den Inhalt, nicht die Uhr) — und weil das
        ein Entscheid ist und kein Versehen, hat er hier seine Auskunft und in
        B5 seinen eigenen Pruefpunkt.  Ein Entscheid ohne Bein laesst sich
        unbemerkt zuruecknehmen; das ist die teuerste Lehre aus S10-11. */
    std::uint64_t liveSamplesJetzt() const noexcept    { return liveSamples; }
    std::uint64_t evidenzSamplesJetzt() const noexcept { return evidenzSamples; }

    /** Wie viele Band-Fensterbeitraege an Grenzen verworfen wurden.

        Er trennt "der Akku ist leer, weil geleert wurde" von "der Akku ist
        leer, weil nie etwas drin war" — ohne ihn koennte ein Bein, das nur
        `liveAkkuBelegteBaender() == 0` prueft, gruen sein, ohne dass die
        Leerung je gelaufen ist. */
    std::uint64_t verworfeneBandfenster() const noexcept { return zVerworfeneBandfenster; }

    /** Betragssumme der Filterzustände beider K-Ketten.

        🔑 DIESE AUSKUNFT EXISTIERT WEGEN EINES LOCHS IM EIGENEN BEIN.  Der
        Filterzustand ist die subtilste Form der Fensterüberbrückung: er trägt
        Audio von VOR einer Grenze in die Zeit danach, und dabei wächst kein
        Puffer, kein Zähler und kein Füllstand — an den anderen Auskünften ist
        er unsichtbar.  `grenzeZiehen()` nullt ihn seit der ersten Fassung; nur
        gemessen hat das nichts, und eine Zusage ohne Bein ist genau der
        T2-2-Befund aus SONDE-008 („der Riegel ist wirksam — aber von keinem
        Kanon-Bein gedeckt").  Jetzt ist er es. */
    double kFilterZustand() const noexcept
    {
        return std::abs (kL.shelf.z1) + std::abs (kL.shelf.z2)
             + std::abs (kL.hochpass.z1) + std::abs (kL.hochpass.z2)
             + std::abs (kR.shelf.z1) + std::abs (kR.shelf.z2)
             + std::abs (kR.hochpass.z1) + std::abs (kR.hochpass.z2);
    }

    /** Erstes Band, das bei dieser Samplerate ueberhaupt einen Bin traegt, und
        erstes Band ueber der Kappe.  Beides ist Auskunft ueber die GRENZEN der
        Messung, nicht ueber ihr Ergebnis. */
    int erstesMessbaresBand() const noexcept { return erstesBand; }
    int erstesBandUeberKappe() const noexcept { return kappeBand; }
    double samplerate() const noexcept { return sr; }

private:
    //== Fensterstufe =========================================================

    struct Stufe
    {
        int punkte { 0 }, hop { 0 }, gefuellt { 0 };
        double fs { 0.0 };
        std::vector<double> ringM, ringS;      // Mid und Side, `punkte` lang
        // Projekt- und Continuous-Zeit werden PRO SAMPLE mitgeschoben. Ein
        // einzelner Startwert plus blindes `+ hop` waere bei fehlenden oder
        // stehenden Hostwerten eine erfundene Zeitachse: lokal schliesst das
        // Audio an, die jeweilige Hostuhr nachweislich nicht.
        std::vector<std::int64_t> ringProjekt;
        std::vector<std::uint8_t> ringProjektGueltig;
        std::vector<std::int64_t> ringContinuous;
        std::vector<std::uint8_t> ringContinuousGueltig;
        std::vector<double> fenster;           // Hann periodisch
        std::vector<double> arbeit;            // ein Fensterausschnitt
        std::vector<double> psd;               // punkte/2+1
        Fft fftM, fftS;
        double fensterEnergie { 0.0 };
        // Bin-Fenster [von,bis) je Evidenzband; bis<=von heisst "nicht messbar".
        std::vector<int> bandVon, bandBis;
        std::uint64_t fensterStromStart { 0 };
        std::int64_t fensterProjektStart { 0 };
        bool fensterProjektGueltig { false };
        std::int64_t fensterContinuousStart { 0 };
        bool fensterContinuousGueltig { false };

        void vorbereiten (int n, double samplerate)
        {
            punkte = n;
            hop = n / 2;
            fs = samplerate;
            ringM.assign ((std::size_t) n, 0.0);
            ringS.assign ((std::size_t) n, 0.0);
            ringProjekt.assign ((std::size_t) n, 0);
            ringProjektGueltig.assign ((std::size_t) n, 0);
            ringContinuous.assign ((std::size_t) n, 0);
            ringContinuousGueltig.assign ((std::size_t) n, 0);
            arbeit.assign ((std::size_t) n, 0.0);
            psd.assign ((std::size_t) (n / 2 + 1), 0.0);
            fenster.assign ((std::size_t) n, 0.0);
            fensterEnergie = 0.0;
            for (int i = 0; i < n; ++i)
            {
                const double w = 0.5 - 0.5 * std::cos (2.0 * kPi * (double) i / (double) n);
                fenster[(std::size_t) i] = w;
                fensterEnergie += w * w;
            }
            fftM.vorbereiten (n);
            fftS.vorbereiten (n);
            bandVon.assign ((std::size_t) Gitter::evidenzBaender, 0);
            bandBis.assign ((std::size_t) Gitter::evidenzBaender, 0);
            gefuellt = 0;
            fensterStromStart = 0;
            fensterProjektStart = 0;
            fensterProjektGueltig = false;
            fensterContinuousStart = 0;
            fensterContinuousGueltig = false;
        }

        /** Verwirft, was gerade gesammelt wird.  DAS ist die Trennung. */
        void leeren() noexcept
        {
            gefuellt = 0;
            fensterStromStart = 0;
            fensterProjektStart = 0;
            fensterProjektGueltig = false;
            fensterContinuousStart = 0;
            fensterContinuousGueltig = false;
            for (auto& v : ringM) v = 0.0;
            for (auto& v : ringS) v = 0.0;
            for (auto& v : ringProjekt) v = 0;
            for (auto& v : ringProjektGueltig) v = 0;
            for (auto& v : ringContinuous) v = 0;
            for (auto& v : ringContinuousGueltig) v = 0;
        }
    };

    /** Lokaler Beginn und — nur bei lueckenlosem Samplebeweis — die dazu
        gehoerende Projektzeit eines offenen Analysefensters.  Live und
        Evidenz brauchen getrennte Buecher, weil ihre Publikationskadenzen
        verschieden sind. */
    struct Support
    {
        bool gesetzt { false };
        std::uint64_t stromStart { 0 };
        bool projektGueltig { false };
        std::int64_t projektStart { 0 };
        bool continuousGueltig { false };
        std::int64_t continuousStart { 0 };
    };

    static bool projektVorwaerts (std::int64_t start, std::uint64_t delta,
                                  std::int64_t& heraus) noexcept;

    static void supportVereinen (Support& ziel, const Support& neu) noexcept;

    static void supportMerken (Support& ziel, const Stufe& s) noexcept;

    int trennIndex() const noexcept;

    void zuordnung (Stufe& s, int vonBand, int bisBand);

    void auskunftBestimmen() noexcept;

    Grenzgrund grenzeZwischen (const echtzeit::StampedBlock& alt,
                               const echtzeit::StampedBlock& neu) const noexcept;

    bool istLoopWrap (const echtzeit::StampedBlock& alt,
                      const echtzeit::StampedBlock& neu) const noexcept;

    static double ppqProSampleVon (const echtzeit::StampedBlock& b) noexcept;

    bool moeglicherStraddleIn (const echtzeit::StampedBlock& b) const noexcept;

    void grenzeZiehen (Grenzgrund grund) noexcept;

    static bool blockProjektSpanneGueltig (const echtzeit::StampedBlock& b) noexcept;

    static bool blockContinuousSpanneGueltig (const echtzeit::StampedBlock& b) noexcept;

    void evidenzContinuousBelegen (const echtzeit::StampedBlock& block) noexcept;

    void rahmenZeitBelegen (const echtzeit::StampedBlock& block) noexcept;

    void verarbeiteSamples (const echtzeit::StampedBlock& block, const float* daten) noexcept;

    void schiebeStufe (Stufe& s, double m, double side,
                       const echtzeit::StampedBlock& block, int sampleOffset) noexcept;

    void rechneFenster (Stufe& s) noexcept;

    static double summeBereich (const Stufe& s, int von, int bis) noexcept;

    void flussSchritt (Stufe& s) noexcept;

    double medianDerHistorie() noexcept;

    void ereignisAblegen (const Ereignis& e) noexcept;

    void stereoSchritt (const Stufe& s, bool zaehlKurzfenster) noexcept;

    void stereoSample (double l, double r) noexcept;

    void stereoAuswerten() noexcept;

    void stereoLeeren() noexcept;

    void fingerprintSchritt (const Stufe& s, double fluss) noexcept;

    Fingerprint fingerprintJetzt() const noexcept;

    void fingerprintLeeren() noexcept;

    void zelleSchliessen() noexcept;

    static int lraBin (double lufs) noexcept;

    bool kurzLufs (double& heraus) const noexcept;

    bool momentanLufs (double& heraus) const noexcept;

    bool kurzTruePeak (double& heraus) const noexcept;

    bool crestKurz (double& heraus) const noexcept;

    bool lraLu (double& heraus) const noexcept;

    static constexpr double lraBinMitte (int b) noexcept;

    double lraPerzentil (double relGate, std::uint64_t gesamt, double anteil) const noexcept;

    bool baueFrame() noexcept;

    void evidenzLeeren() noexcept;

    void rahmenLeeren() noexcept;

    Transportstempel baueStempel (const echtzeit::StampedBlock& b,
                                  bool evidenzWirdPubliziert) const noexcept;

    void fuelleLive (LiveBaender& satz, float* breite, std::uint8_t* breiteBitmap) const noexcept;

    void fuelleEvidenz (EvidenzBaender& satz) const noexcept;

    static int ringInZeitfolge (const VerteilungsRing& r, float* aus) noexcept;

    static double perzentil (const float* sortiert, int n, double p) noexcept;

    void fuelleVerteilung (FeatureFrame& f) const noexcept;

    void fuelleAbdeckungUndKonvergenz (FeatureFrame& f) const noexcept;

    void fuelleSkalare (FeatureFrame& f) const noexcept;

    //== Zustand ==============================================================

    struct Akku    { double summe { 0.0 }; std::uint64_t n { 0 }; };
    struct Breite  { double seite { 0.0 }; double gesamt { 0.0 }; };


    double sr { 0.0 };
    bool   vorbereitet { false };

    Stufe bass, haupt;
    int   erstesBand { 0 }, kappeBand { Gitter::evidenzBaender };

    Akku   liveAkku[Gitter::evidenzBaender] {};
    Akku   evidenzAkku[Gitter::evidenzBaender] {};
    // Nur auf der Live-Seite: §33.2 fuehrt "Breite/Korrelation" bei der
    // Live-Telemetrie und beim Evidenzsnapshot ausdruecklich nicht.
    Breite liveBreiteAkku[Gitter::evidenzBaender] {};
    Support liveSupport {};
    Support evidenzSupport {};

    /// SONDE-013 M-11: die bandweisen Kreuzspektralsummen eines Fensters.
    /// `smm`/`sss` tragen Mid- und Side-Energie, `sll`/`srr` die
    /// L/R-Autospektren, `sxyRe`/`sxyIm` das komplexe Kreuzspektrum.
    /// `frames` ist die Zahl der GUELTIGEN Welch-Frames - also genau das
    /// Feld `freiheitsgrade`, das §40.1 als Teil der Evidenz verlangt.
    struct StereoAkku
    {
        double smm { 0.0 }, sss { 0.0 };
        double sll { 0.0 }, srr { 0.0 };
        double sxyRe { 0.0 }, sxyIm { 0.0 };
        std::uint32_t frames { 0 };
        double dauerMs { 0.0 };
    };
    /// Alle Stereotraeger liegen im HEAP: elf Bandsaetze zu 221 Werten sind
    /// rund 11 KiB, und der Stack dieses Beins ist in Etappe C schon dreimal
    /// gerissen (Manifest §10.3, §10.4).
    std::vector<StereoAkku>      stereoAkku, stereoKurz;
    std::vector<VerteilungsRing> stereoVerlauf;
    std::vector<StereoBandwert>  stereoErgebnis;
    std::vector<float>           stereoKorrKurz;
    std::vector<std::uint8_t>    stereoKorrKurzGesetzt;
    /// NAK-181 R5a: je Band die Zahl der abgeschlossenen Kurzfenster MIT
    /// gueltigem Nenner. Nur das Praesenzbit haengt daran, nie der Wert.
    std::vector<std::uint32_t>   stereoKurzfensterBand;
    std::vector<std::uint32_t>   stereoPersistenzZaehler;
    int           stereoKurzFrames { 0 };
    std::uint32_t stereoKurzfenster { 0 };
    double stereoMonoEnergie { 0.0 }, stereoStereoEnergie { 0.0 };
    double stereoLEnergie { 0.0 }, stereoREnergie { 0.0 };
    StereoSkalare stereoSkalareErgebnis {};

    // SONDE-013 M-05: Verteilung, Abdeckung und Konvergenz des Evidenzfensters.
    // Die zwei Zaehler stehen NEBENEINANDER, weil die Abdeckung genau ihr
    // Verhaeltnis ist: `gesamt` waechst bei JEDEM Hauptstufen-Fenster, `aktiv`
    // nur bei denen ueber dem Aktivgate. Ein einzelner Zaehler koennte
    // "keine Fenster gesehen" nicht von "nur Stille gesehen" trennen.
    /// Ein Ring je Band, rund 58 KiB. Er liegt im HEAP und nicht als Feld
    /// im Objekt — genau wie `kurzZellen`, `ereignisse` und `flussHistorie`
    /// daneben. Der Grund ist gemessen: als Feld sprengten zwei Engines
    /// nebeneinander (die Zwillingsprobe G13 in B5) den 1-MiB-Stack mit
    /// STATUS_STACK_OVERFLOW. Angelegt wird er in `vorbereiten()`, also auf
    /// dem Nachrichtenthread; der Audiothread alloziert weiterhin nie.
    std::vector<VerteilungsRing> evidenzVerteilung;
    std::uint64_t   evidenzFensterGesamt { 0 };
    std::uint64_t   evidenzFensterAktiv  { 0 };
    /// Laufender Evidenzabstand, zwischen Min und Max. Er ueberlebt eine
    /// Grenze und ein `zuruecksetzen()` ABSICHTLICH: der Rueckstau, der ihn
    /// gesetzt hat, verschwindet nicht dadurch, dass die Messreihe neu
    /// beginnt.
    double          evidenzIntervallS { kEvidenzIntervallMinS };

    // Loudness
    KKette kL, kR;
    int    zellenSamples { 0 }, zelleStand { 0 };
    double zelleKEnergie { 0.0 }, zelleAktivEnergie { 0.0 };
    std::vector<double> kurzZellen;
    int    kurzStand { 0 }, kurzGefuellt { 0 };

    // SONDE-013 M-02 bis M-04: True Peak, das zweite und dritte Loudnessfenster
    // und die zwei Headroomgroessen.
    TruePeakDetektor tp;
    /// True-Peak-Maximum je Zelle, Ring wie `kurzZellen`; daraus wird das
    /// Maximum des 3-s-Fensters fuer PSR.
    std::vector<double> kurzTpZellen;
    /// Ungewichtete mittlere Energie je Zelle — die RMS-Haelfte des
    /// Crest-Faktors ueber 3 s.
    std::vector<double> kurzRmsZellen;
    double zelleTruePeak { 0.0 }, zelleRmsEnergie { 0.0 };
    /// Maximum ueber den laufenden 100-ms-Rahmen und ueber die Passage.
    ///
    /// Die "Passage" ist hier der Abschnitt seit der letzten GRENZE, nicht
    /// seit dem letzten `zuruecksetzen()`: §32.4 bindet eine Passage an
    /// genau eine Transportepoche, und G13 misst, dass kein Traeger des
    /// Frames eine Grenze ueberbrueckt. Ein Passageobjekt gibt es in P4 noch
    /// nicht (das ist Etappe E); bis dahin ist die Epochengrenze die
    /// einzige belegbare Passagengrenze.
    double rahmenTruePeak { 0.0 }, passageTruePeak { 0.0 };
    /// SONDE-013 M-03/M-25: das Maximum ueber die Samples, die im
    /// Passagenfenster lagen. Ohne gesetztes Fenster identisch mit
    /// `rahmenTruePeak`.
    double passagenTruePeakRahmen { 0.0 };
    /// SONDE-013 M-07: nicht-endliche EINGANGSsamples. Drei Zeitraeume, weil
    /// drei Verbraucher: der Rahmen verriegelt seine Skalare, das
    /// Evidenzfenster seine Konfidenzklasse, der Gesamtzaehler ist die
    /// Diagnose ueber die Laufzeit.
    std::uint32_t rahmenNichtEndlich { 0 };
    std::uint32_t evidenzNichtEndlich { 0 };
    std::uint64_t nichtEndlicheSamplesGesamt { 0 };
    /// SONDE-013 M-25: das Fenster der markierten Passage in PROJEKTzeit.
    struct Passagenfenster
    {
        bool          gesetzt     { false };
        std::int64_t  startSample { 0 };
        std::int64_t  endeSample  { 0 };   ///< exklusiv
        /// 🔑 SONDE-013 Nacharbeit 2 (Befund R03, Paragraph 32.4): die
        /// Transportepoche, an die das Fenster GEBUNDEN ist. Die Signatur
        /// konnte sie bis dahin gar nicht tragen — eine Passage ohne Epoche
        /// ist ein Zeitfenster, das ueber einen Seek hinweg dieselbe Zahl
        /// traegt wie eines darin.
        std::uint64_t epoche      { 0 };
    };
    Passagenfenster passagenfenster {};
    /// Eine Transportgrenze im Fenster macht es unbrauchbar (§32.4).
    bool passagenfensterGebrochen { false };
    /// Monotone Samplezaehlung und die Stelle des juengsten Samples AUSSERHALB
    /// des Fensters. Aus beiden folgt, ob ein Analysefenster der Laenge
    /// `s.punkte` vollstaendig in der Passage lag.
    std::uint64_t verarbeiteteSamples     { 0 };
    std::uint64_t letztesSampleAusserhalb { 0 };
    bool          hatSampleAusserhalb     { false };
    /// Wie viele Samples der laufenden Loudnesszelle im Fenster lagen.
    std::uint32_t zelleImFensterSamples { 0 };
    /// LRA: Histogramm der gegateten Kurzzeitwerte plus der Zaehler, der die
    /// 60-s-Regel traegt. `lraZellenSeitHop` erzeugt den 1-s-Hop. Beide
    /// fallen an jeder Grenze — siehe `grenzeZiehen()`.
    std::vector<std::uint32_t> lraHistogramm;
    std::uint64_t lraGezaehlt { 0 };
    int           lraZellenSeitHop { 0 };
    /// Headroomverteilung: ein Ring der Rahmen-True-Peaks in dB.  Er teilt
    /// die Laenge `kVerteilungPlaetze` mit den Bandringen — dieselbe
    /// Ressourcengrenze, deshalb bewusst KEINE zweite Konstante, die davon
    /// abdriften koennte.
    ///
    /// Er liegt im HEAP, mit genau einem Element, aus demselben gemessenen
    /// Grund wie `evidenzVerteilung` daneben: B5 haelt zwanzig Engines
    /// gleichzeitig auf dem Stack, und dort summieren sich auch 264 Byte je
    /// Instanz zu einem `STATUS_STACK_OVERFLOW` (Manifest §10.2, Befund 1 —
    /// und ein zweites Mal beim Bau der Etappe C).
    std::vector<VerteilungsRing> headroomRing;

    // Rahmen (zwischen zwei Live-Frames)
    double rahmenPeak { 0.0 }, rahmenSummeQuadrat { 0.0 };
    std::uint64_t rahmenSamples { 0 };
    echtzeit::StampedBlock rahmenStartBlock {};
    bool rahmenProjektDurchgehend { false };
    bool rahmenContinuousDurchgehend { false };
    double rahmenMid2 { 0.0 }, rahmenSide2 { 0.0 };
    double rahmenL { 0.0 }, rahmenR { 0.0 }, rahmenL2 { 0.0 }, rahmenR2 { 0.0 }, rahmenLR { 0.0 };
    std::uint64_t rahmenAktivZellen { 0 }, rahmenZellen { 0 };
    std::uint64_t liveSamples { 0 }, evidenzSamples { 0 };
    bool evidenzContinuousHabe { false };
    bool evidenzContinuousDurchgehend { true };
    std::int64_t evidenzContinuousErwartet { 0 };

    // SONDE-013 M-26: der Fingerprintakkumulator. Alles im HEAP - dieselbe
    // Begruendung wie bei den Stereotraegern daneben.
    std::vector<double>        fpBandSumme, fpChromaSumme, fpOnset;
    std::vector<std::uint32_t> fpBandAnzahl;
    int           fpOnsetStand { 0 };
    /// Wie viele Frames auf EINE Onsetstuetzstelle fallen. Er verdoppelt
    /// sich, sobald das Raster voll ist — die Aufloesung sinkt mit der
    /// Passagenlaenge, statt dass ein Fenster ueberlaeuft.
    int           fpOnsetProStelle { 1 };
    int           fpOnsetInStelle { 0 };
    std::uint32_t fpFenster { 0 };

    // Ereignisse
    /// SONDE-013 M-86: Peak des zuletzt ABGESCHLOSSENEN Rahmens und das Flag,
    /// das den Peakpfad genau einmal je Rahmen ausloesen laesst.
    /// `vorigerRahmenPeak` faellt an einer Grenze mit — eine Peaksteigung
    /// ueber eine Grenze hinweg vergliche zwei Stellen der Musik.
    double vorigerRahmenPeak { 0.0 };
    bool   peakEreignisImRahmen { false };
    std::vector<Ereignis> ereignisse;
    int ereignisStand { 0 }, ereignisAnzahl { 0 };
    std::vector<double> vorigesSpektrum, flussHistorie, flussSortiert;
    bool vorigesSpektrumGueltig { false };
    int  flussStand { 0 }, flussGefuellt { 0 };

    // Zeitbuch
    echtzeit::StampedBlock vorigerBlock {};
    bool          habeVorigen { false };
    std::uint64_t transportEpoche { 0 }, segmentInEpoche { 0 }, sequenz { 0 };

    FeatureFrame aktuell {};
    bool         frameBereit { false };

    // Telemetrie
    std::uint64_t zGetrennteFenster { 0 }, zVerworfeneFensterSamples { 0 };
    std::uint64_t zEpochenwechsel { 0 }, zSegmentwechsel { 0 };
    std::uint64_t zStraddleVerworfen { 0 }, zEreignisseVerworfen { 0 };
    std::uint64_t zNak29Abgelehnt { 0 }, zBloecke { 0 };
    std::uint64_t zVerworfeneBandfenster { 0 };
    std::uint64_t grundZaehler[(std::size_t) Grenzgrund::anzahl] {};
};

} // namespace nakama::analyse

// Die Definitionen der Memberfunktionen je Merkmalsfamilie. Sie stehen
// hinter der vollstaendigen Klassendeklaration, weil eine
// Out-of-class-Definition sie voraussetzt, und hinter dem Namensraumende,
// weil jeder Teilkopf seinen Namensraum selbst oeffnet.
#include "featureengine/Zeit.h"
#include "featureengine/Spektrum.h"
#include "featureengine/Stereo.h"
#include "featureengine/Fingerprint.h"
#include "featureengine/Lautheit.h"
#include "featureengine/Frame.h"

#undef NAKAMA_FEATUREENGINE_TEIL
