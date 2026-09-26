/*  EqCopSonde013StereoGoldenTest — bandweise Stereoevidenz
    (SONDE-013 M-08, M-10, M-11, M-12).

    §40.1 sagt, warum es dieses Bein gibt: zwei globale Skalare reichen nicht.
    `breite` und `korrelation` im 10-Hz-Liveframe beschreiben die ganze
    Bandbreite mit je einer Zahl, und ein Signal, das unten mono und oben
    breit ist, sieht darin genauso aus wie eines mit gleichmaessiger Breite.

    ── DIE FUENF KLASSEN AUS §40.3 ──────────────────────────────────────────

    Mono, identisches Stereo, Polaritaetsinvertierung, bekannte Laufzeit und
    unkorrelierte Kanaele. Jede hat eine ANALYTISCH bekannte Antwort, und
    genau daran wird gemessen — nicht an einer zweiten eigenen Rechnung:

    - Mono (L bitgleich R): Korrelation exakt 1, Seitenanteil bei -inf,
      Mono-Folddown 0 dB.
    - Polaritaet (R = -L): Korrelation exakt -1, und der Mono-Folddown ist
      totale Ausloeschung.
    - Laufzeit (R = L um n Samples verzoegert): die Interchannel-PHASE ist in
      jedem Band linear in der Frequenz, phi = -2*pi*f*tau. Das ist die
      schaerfste Probe des ganzen Beins, weil sie die Phase gegen eine Formel
      haelt statt gegen ein Vorzeichen.
    - Unkorreliert: Kohaerenz nahe null bei genug Frames.

    ── DIE ZWEI FAIL-CLOSED-STUFEN ──────────────────────────────────────────

    §40.1 woertlich: "Bei zu wenig Energie oder Frames ist Kohaerenz `null`"
    und "Interchannel-Phase wird nur in ausreichend kohaerenten Baendern
    interpretiert". Dass die Phasenbits eine TEILMENGE der Kohaerenzbits sind,
    kann das JSON-Schema nicht ausdruecken (es steht dort als Kommentar); hier
    wird es gemessen.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/FeatureEngine.h"
#include "Nak380Pruefsignale.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace rt = nakama::echtzeit;
using nakama::analyse::FeatureEngine;
using nakama::analyse::FeatureFrame;
using nakama::analyse::Gitter;
using nakama::analyse::StereoBandwert;

namespace
{
int bestanden = 0;
int fehler = 0;

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {})
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

void abschnitt (const char* text)
{
    std::cout << "\n== " << text << " ==" << std::endl;
}

constexpr double kZweiPi = 6.283185307179586476925286766559;

/** Speist Bloecke mit GETRENNTEM Audio je Kanal — der Unterschied zu den
    Speisern der Nachbarbeine, und fuer Stereo der ganze Punkt. */
struct Speiser
{
    FeatureEngine& engine;
    double sr { 48000.0 };
    int    frames { 512 };
    std::uint64_t strom { 0 };
    std::int64_t  projekt { 0 };
    std::vector<float> audio;

    explicit Speiser (FeatureEngine& e) : engine (e) {}

    rt::StampedBlock bauen() const
    {
        rt::StampedBlock b;
        b.stromVon = strom;
        b.sampleCount = (std::uint32_t) frames;
        b.segment = 0;
        b.startFolge = 0;
        b.kanaele = 2;
        b.tapMaske = 1;
        b.projectSampleStart = projekt;
        b.sampleRate = sr;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | rt::kFlagSpielt | rt::kFlagZeitGueltig;
        return b;
    }

    /** `f(n, l, r)` schreibt beide Kanaele fuer die absolute Stromposition n. */
    bool sende (const std::function<void (std::uint64_t, float&, float&)>& f)
    {
        const auto b = bauen();
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            float l = 0.0f, r = 0.0f;
            f (strom + i, l, r);
            audio[(std::size_t) i * 2u]      = l;
            audio[(std::size_t) i * 2u + 1u] = r;
        }
        const bool ok = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        projekt += (std::int64_t) b.sampleCount;
        return ok;
    }

    /** Faehrt, bis ein Evidenzframe entsteht, und laesst die Stereoevidenz
        des zugehoerigen Fensters stehen. `false`, wenn keiner kam. */
    bool bisEvidenz (const std::function<void (std::uint64_t, float&, float&)>& f,
                     int maxBloecke = 900)
    {
        for (int i = 0; i < maxBloecke; ++i)
            if (sende (f) && engine.frame().evidenzFrisch)
                return true;
        return false;
    }
};

/** Das Band, in dem eine Frequenz liegt — ausgerechnet aus dem Gitter, nicht
    abgeschrieben. Eine feste Bandnummer im Test waere eine zweite Wahrheit
    ueber das Gitter. */
int bandFuer (double hz)
{
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
        if (hz >= Gitter::evidenzKante (b) && hz < Gitter::evidenzKante (b + 1))
            return b;
    return -1;
}

/** Wie viele Baender ueberhaupt eine Basis tragen. */
int baenderMitBasis (const FeatureEngine& e)
{
    int n = 0;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
        if (e.stereoBand (b).basisGesetzt)
            ++n;
    return n;
}

/** Die Blockgroessenmenge des NAK-182-Sweeps und der NAK-380-Laufzeitfaelle
    (M-74). Sie steht genau EINMAL, hier; Begruendung der Auswahl beim Sweep
    unten. */
constexpr int kSweepBlockgroessen[] = {
    1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512,
    1024, 2048, 4096, 8192, 12345, 16384 };
constexpr int kSweepN = (int) (sizeof (kSweepBlockgroessen) / sizeof (kSweepBlockgroessen[0]));
} // namespace

#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
namespace nakama::analyse
{
/** NAK-380 Etappe 5 (T-380-11): der Testzugang DIESES Programms (eigenes
    Programm, keine ODR-Kollision mit B5, B16 und B18). Nur lesend; das
    Produkt definiert und ruft ihn nie. */
struct FeatureEngineTestzugang
{
    /** Von der Engine verarbeitete Samples (je Sample einmal gezaehlt, nur
        `zuruecksetzen` setzt ihn auf 0). */
    static std::uint64_t verarbeitet (const FeatureEngine& e) noexcept { return e.verarbeiteteSamples; }

    /** M-85: Bytes der Ringnutzdaten im Heap, Elemente mal
        sizeof (StereoBinAkku), und die tatsaechlich reservierten Bytes. */
    static std::size_t ringBytes (const FeatureEngine& e) noexcept
    {
        return e.stereoRing.size() * sizeof (FeatureEngine::StereoBinAkku);
    }
    static std::size_t ringKapazitaetBytes (const FeatureEngine& e) noexcept
    {
        return e.stereoRing.capacity() * sizeof (FeatureEngine::StereoBinAkku);
    }

    /** M-87: Summe aller Ringzaehler (jeder Slot und das Kurzfenster jedes
        Bandes) - 0 heisst: kein Frame liegt mehr im Ring. */
    static double ringFramesAlle (const FeatureEngine& e) noexcept
    {
        const int trenn = e.trennIndex();
        double summe = 0.0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
            for (int j = 0; j < FeatureEngine::stereoRingSlots (b, trenn); ++j)
                summe += e.stereoZaehlerWert (FeatureEngine::stereoZaehlerBasis (b, trenn) + (std::size_t) j);
        return summe;
    }
};
} // namespace nakama::analyse
#endif

namespace
{
//==============================================================================
// NAK-380 Etappe 5 — Kohaerenz je Bin ueber Frames (Manifest NAK-380 §6.4,
// M-73 bis M-93; DSP-23, R-380-3, T-380-6). Die Faelle heissen
// `380/M-nn <name>`; `--nak380 M-nn` faehrt nur diesen Fall (Rotbeweise nach
// §8.1). Jede Zaehlgroesse ist hergeleitet (Kadenzmodell, Gitter), nie aus
// dem Lauf abgelesen; jeder Formelwert steht gegen eine hier nachgerechnete
// Referenz (R-380-8).

namespace sig = nakama::test::nak380;
using nakama::analyse::FeatureEngineTestzugang;

const char* nak380Nur = nullptr;

bool nak380Waehlt (const char* id)
{
    return nak380Nur == nullptr || std::strcmp (nak380Nur, id) == 0;
}

/** Bestehende Abschnitte: Kopf drucken und fahren, wenn kein Filter steht
    oder die Liste `ids` (durch Leerzeichen getrennt) den Filter nennt. */
bool abschnittWaehlt (const char* ids, const char* text)
{
    bool ja = nak380Nur == nullptr;
    if (! ja && ids != nullptr)
    {
        const std::string liste = std::string (" ") + ids + " ";
        ja = liste.find (std::string (" ") + nak380Nur + " ") != std::string::npos;
    }
    if (ja)
        abschnitt (text);
    return ja;
}

// Die Ringlaengen der Matrix (T-380-6: W_H = 3, W_B = 7), die FFT-Laengen vor
// Etappe 6 und die Mindestframezahl aus §40.1 - bewusst NICHT aus dem Produkt
// gelesen: eine Mutation einer Produktkonstante muss hier rot werden.
constexpr int kNak380RingHaupt = 3;
constexpr int kNak380RingBass  = 7;
constexpr int kNak380NHaupt = 4096;
constexpr int kNak380NBass  = 16384;
constexpr double kNak380TrennungHz = 200.0;

/** Binfenster eines Evidenzbandes, unabhaengig vom Produkt aus dem Gitter
    gerechnet: zustaendig ist die Bassstufe (16 384 Punkte), wenn die
    Bandmitte unter 200 Hz liegt, sonst die Hauptstufe (4096); Bins
    [ceil(kante_b*N/fs), min(ceil(kante_b+1*N/fs), N/2+1)); ueber der Kappe
    min(18 kHz, 0,95*fs/2) kein Bin (Manifest §6.4, M-85: "Binzahlen aus
    Zeit.h:120-145"). */
struct Nak380Binfenster
{
    int von { 0 }, bis { 0 };
    bool bass { false };
    int bins() const noexcept { return bis > von ? bis - von : 0; }
    int punkte() const noexcept { return bass ? kNak380NBass : kNak380NHaupt; }
    int ring() const noexcept { return bass ? kNak380RingBass : kNak380RingHaupt; }
};

Nak380Binfenster nak380Binfenster (int b, double fs)
{
    Nak380Binfenster x;
    x.bass = Gitter::evidenzMitte (b) < kNak380TrennungHz;
    const int n = x.punkte();
    const double kappe = std::min (18000.0, 0.95 * fs * 0.5);
    if (Gitter::evidenzKante (b + 1) > kappe)
        return x;
    x.von = (int) std::ceil (Gitter::evidenzKante (b) * (double) n / fs);
    const int bis = std::min ((int) std::ceil (Gitter::evidenzKante (b + 1) * (double) n / fs), n / 2 + 1);
    x.bis = std::max (x.von, bis);
    return x;
}

/** Unabhaengiges Kadenzmodell (R-380-8): nach jedem Block wird der Liveframe
    faellig, sobald live/fs >= 0,1 s (`nimmBlock`), und live faellt auf 0
    (`rahmenLeeren`); an einem Liveframe entsteht ein Evidenzsnapshot, sobald
    evid/fs >= Intervall (`baueFrame`), und evid faellt auf 0
    (`evidenzLeeren`). Jedes Segment wird fuer sich in Bloecke zerlegt, sein
    letzter Block traegt den Rest (so speist der Blocklaeufer); ueber die
    Segmentgrenze laufen live und evid weiter (eine Grenze laesst die Kadenz
    weiterlaufen, §10.1). Rueckgabe: die gespeisten Samples an jedem
    Snapshot. */
std::vector<std::uint64_t> nak380SnapshotsSegmente (double fs, int block, double intervall,
                                                    const std::vector<std::uint64_t>& segmente)
{
    std::vector<std::uint64_t> aus;
    std::uint64_t gespeist = 0, live = 0, evid = 0;
    for (const auto segment : segmente)
    {
        std::uint64_t imSegment = 0;
        while (imSegment < segment)
        {
            const std::uint64_t n = std::min<std::uint64_t> ((std::uint64_t) block, segment - imSegment);
            imSegment += n;
            gespeist += n;
            live += n;
            evid += n;
            if ((double) live / fs >= 0.1)
            {
                live = 0;
                if ((double) evid / fs >= intervall)
                {
                    aus.push_back (gespeist);
                    evid = 0;
                }
            }
        }
    }
    return aus;
}

std::vector<std::uint64_t> nak380Snapshots (double fs, int block, double intervall, std::uint64_t samples)
{
    return nak380SnapshotsSegmente (fs, block, intervall, { samples });
}

/** Welch-Frames einer Stufe mit N Punkten (Hop N/2) nach `samples` Samples
    seit Start oder Grenze: der j-te schliesst am Sample N + (j-1)*N/2. */
std::uint64_t nak380Frames (std::uint64_t samples, int punkte)
{
    return samples < (std::uint64_t) punkte
        ? 0u : (samples - (std::uint64_t) punkte) / (std::uint64_t) (punkte / 2) + 1u;
}

/** Frames im Ring am Snapshot i (1-basiert): die Frames der letzten W
    Evidenzfenster, also Frames bis Snapshot i minus Frames bis Snapshot
    i - W. */
std::uint64_t nak380RingFrames (const std::vector<std::uint64_t>& snap, int i, int punkte, int w)
{
    const std::uint64_t bis = nak380Frames (snap[(std::size_t) (i - 1)], punkte);
    const std::uint64_t von = i - w >= 1 ? nak380Frames (snap[(std::size_t) (i - w - 1)], punkte) : 0u;
    return bis - von;
}

/** Referenz der Kohaerenz einer reinen Laufzeit d (Kopf §6.4): die MSC je Bin
    weissen Rauschens ist rho_w(tau)^2 mit der normierten Hann-Autokorrelation
    rho_w(tau) = [(1 - tau)(2 + cos 2 pi tau) + (3/(2 pi)) sin 2 pi tau]/3,
    tau = d/N (E[X Y*] = sigma^2 Summe w[n] w[n+d], E|X|^2 = sigma^2 Summe w^2).
    d = 48, N = 4096: 0,99819; N = 16 384: 0,99989. */
double nak380RhoQuadrat (int d, int n)
{
    const double tau = (double) d / (double) n;
    const double rho = ((1.0 - tau) * (2.0 + std::cos (kZweiPi * tau))
                        + (3.0 / kZweiPi) * std::sin (kZweiPi * tau)) / 3.0;
    return rho * rho;
}

/** Referenzpruefung der Kohaerenz (R-380-8) am MITTEL, nicht je Band:
    q_b = (1 - C_b)/(1 - rho_w(d)^2) mit der Referenz der zustaendigen Stufe;
    das Mittel ueber alle Baender mit Bit muss in [0,8; 1,2] liegen.
    Herleitung: je Bin ist E[C] = g2 + (1 - g2)^2/n (n unabhaengige Frames),
    also E[q] = 1 - (1 - g2)/n, fuer g2 >= 0,97 und n >= 8 ueber 0,996; ein
    Bandmittel C_b hat denselben Erwartungswert. Die Streuung je Bin ist die
    einer chi^2_nu/nu-Groesse mit nu >= K (50 % ueberlappende Hann-Frames
    tragen zwischen K und 2K Freiheitsgrade), also hoechstens sqrt(2/K) =
    0,30 bei K = 22; ueber M >= 50 Baender mit mindestens einem Bin faellt das
    Mittel auf hoechstens 0,30/sqrt(50) = 0,043, +-0,2 sind 4,7 sigma (bei
    einem Mittel ueber mehrere Snapshots mehr). Gegenprobe am 26.09.2026 in
    Python (N 4096, d 48 und 192, K 20 bis 84, 75 200 Bins je Fall):
    Mittel q 0,998 bis 0,999, das 99,99-%-Quantil je Bin 1,7 bis 3,3 - eine
    Schranke je Einzelband waere deshalb weit, das Mittel ist scharf. Die
    Bandsumme (alter Weg) laege bei q = 250 bis 350. */
constexpr double kNak380RefMittelUnten = 0.8, kNak380RefMittelOben = 1.2;

/** Gewickelt auf (-pi, pi]. */
double nak380Wickeln (double phi)
{
    while (phi >  3.14159265358979) phi -= kZweiPi;
    while (phi <= -3.14159265358979) phi += kZweiPi;
    return phi;
}

/** Bandwerte eines Snapshots, wie der Lauf sie meldet. */
struct Nak380Band
{
    bool basis { false }, koh { false }, phase { false }, lauf { false }, pers { false };
    float kohWert { 0.0f }, phaseWert { 0.0f }, laufMs { 0.0f }, persWert { 0.0f }, dauerMs { 0.0f };
    std::uint32_t dof { 0 };
};

Nak380Band nak380Lesen (const StereoBandwert& w)
{
    Nak380Band z;
    z.basis = w.basisGesetzt;
    z.koh = w.kohaerenzGesetzt;
    z.kohWert = w.kohaerenz;
    z.phase = w.phaseGesetzt;
    z.phaseWert = w.phaseRad;
    z.pers = w.persistenzGesetzt;
    z.persWert = w.persistenz;
    z.dauerMs = w.fensterDauerMs;
    z.dof = w.freiheitsgrade;
    z.lauf = w.laufzeitGesetzt;           // A-3: Engine-Ergebnis, nicht auf der Leitung
    z.laufMs = w.laufzeitMs;
    return z;
}

using Nak380Schnapp = std::vector<Nak380Band>;

struct Nak380Pos
{
    std::uint64_t strom { 0 };
    std::int64_t  projekt { 0 };
};

struct Nak380Lauf
{
    std::uint64_t gespeist { 0 };               ///< Summe der sampleCount dieses Laufs
    std::vector<std::uint64_t> snapshots;       ///< gespeiste Samples an jedem Evidenzsnapshot
};

using Nak380JeSnapshot = std::function<void (FeatureEngine&, int)>;

/** DER Blocklaeufer der Etappe-5-Faelle (Muster `nak380Korpuslauf` in
    `AnalysisGoldenTestMain.cpp`, Lehre D1 aus §39.1): Bloecke von
    min(block, Rest) Samples, nie ein Sample hinter dem Puffer; Strom und
    Projektzeit laufen in `pos` fort, ein Folgelauf auf derselben Engine setzt
    dort an (Seek: `pos.projekt` springt). Ereignisse werden nach jedem Frame
    entnommen (§7.3), damit der Verlustzaehler eine Aussage ist; `jeSnapshot`
    sieht die Engine hinter jedem Evidenzsnapshot. */
__declspec(noinline) Nak380Lauf nak380Stereolauf (FeatureEngine& e, Nak380Pos& pos, double fs, int block,
                                                  const std::vector<float>& l, const std::vector<float>& r,
                                                  const Nak380JeSnapshot& jeSnapshot = {})
{
    Nak380Lauf lauf;
    std::vector<float> audio ((std::size_t) block * 2u);
    const auto n = (std::uint64_t) std::min (l.size(), r.size());
    std::uint64_t i = 0;
    while (i < n)
    {
        const auto anzahl = (std::uint32_t) std::min<std::uint64_t> ((std::uint64_t) block, n - i);
        for (std::uint32_t k = 0; k < anzahl; ++k)
        {
            audio[(std::size_t) k * 2u]      = l[(std::size_t) (i + k)];
            audio[(std::size_t) k * 2u + 1u] = r[(std::size_t) (i + k)];
        }
        rt::StampedBlock b;
        b.stromVon = pos.strom;
        b.sampleCount = anzahl;
        b.segment = 0;
        b.startFolge = 0;
        b.kanaele = 2;
        b.tapMaske = 1;
        b.projectSampleStart = pos.projekt;
        b.sampleRate = fs;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | rt::kFlagSpielt | rt::kFlagZeitGueltig;
        const bool frame = e.nimmBlock (b, audio.data());
        pos.strom += anzahl;
        pos.projekt += (std::int64_t) anzahl;
        i += anzahl;
        if (frame)
        {
            if (e.frame().evidenzFrisch)
            {
                lauf.snapshots.push_back (i);
                if (jeSnapshot)
                    jeSnapshot (e, (int) lauf.snapshots.size());
            }
            e.ereignisseEntnommen();
        }
    }
    lauf.gespeist = i;
    return lauf;
}

/** Ein Rauschlauf (§7.2 D1 bis D7, Mono-Rauschen fuer M-82 bis M-84 und M-88):
    frische Engine im Heap (NAK-175), Evidenzintervall gesetzt, das Paar vor
    dem Speisen selbstgeprueft, jeder Snapshot mit allen 221 Bandwerten
    festgehalten. Gleiche Parameter fahren nur einmal (Zwischenspeicher). */
struct Nak380Rauschlauf
{
    double fs { 0.0 };
    int block { 0 };
    double intervall { 0.25 };
    int d { 0 };
    std::uint64_t saat { 0 }, soll { 0 }, gespeist { 0 }, verarbeitet { 0 }, verworfen { 0 };
    sig::RauschpaarSelbstpruefung selbst;
    std::vector<std::uint64_t> snapIst, snapSoll;
    std::vector<Nak380Schnapp> schnapp;
};

__declspec(noinline) const Nak380Rauschlauf& nak380Rauschen (double fs, int block, double intervall,
                                                            std::uint64_t samples, int d,
                                                            std::uint64_t saat)
{
    using Schluessel = std::tuple<long long, int, long long, std::uint64_t, int, std::uint64_t>;
    static std::map<Schluessel, std::unique_ptr<Nak380Rauschlauf>> ablage;
    const Schluessel k { std::llround (fs), block, std::llround (intervall * 1000.0), samples, d, saat };
    if (const auto it = ablage.find (k); it != ablage.end())
        return *it->second;

    auto L = std::make_unique<Nak380Rauschlauf>();
    L->fs = fs;
    L->block = block;
    L->intervall = intervall;
    L->d = d;
    L->saat = saat;
    L->soll = samples;
    const auto paar = sig::rauschpaar (fs, samples, d, saat);
    // Kopffunktion des Erzeugers VOR dem Nutzer (Lehre D4 aus §39.1).
    L->selbst = sig::rauschpaarSelbstpruefung (paar, fs, d, saat, samples);
    auto engine = std::make_unique<FeatureEngine>();
    engine->vorbereiten (fs);
    engine->evidenzIntervallSetzen (intervall);
    Nak380Pos pos;
    auto* ziel = L.get();
    const auto lauf = nak380Stereolauf (*engine, pos, fs, block, paar.l, paar.r,
        [ziel] (FeatureEngine& e, int)
        {
            Nak380Schnapp s ((std::size_t) Gitter::evidenzBaender);
            for (int b = 0; b < Gitter::evidenzBaender; ++b)
                s[(std::size_t) b] = nak380Lesen (e.stereoBand (b));
            ziel->schnapp.push_back (std::move (s));
        });
    L->gespeist = lauf.gespeist;
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
    L->verarbeitet = FeatureEngineTestzugang::verarbeitet (*engine);
#endif
    L->verworfen = engine->ereignisseVerworfen();
    L->snapIst = lauf.snapshots;
    L->snapSoll = nak380Snapshots (fs, block, intervall, samples);
    return *ablage.emplace (k, std::move (L)).first->second;
}

juce::String nak380Kopf (const char* id, const char* name, const Nak380Rauschlauf& L)
{
    return juce::String ("380/") + id + " " + name + " (" + juce::String (L.fs / 1000.0, 1)
         + " kHz, B " + juce::String (L.block) + ")";
}

/** Die Vorbedingungen je Nutzer als eigene Pruefungen (Lehren D1, D4, D5 aus
    §39.1, R1 und R2 aus §42.1): Selbstpruefung des Erzeugers, gespeiste
    Samplezahl = Signallaenge (Laeufer und Engine), Verlustzaehler 0 und die
    Snapshots an den Samplezahlen des Kadenzmodells. */
void nak380Vorbedingungen (const juce::String& kopf, const Nak380Rauschlauf& L)
{
    pruefe (L.selbst.ok, kopf + ": Vorbedingung rauschpaar_selbstpruefung (Saat, Amplitude 0,35, d = "
                + juce::String (L.d) + ", Laenge)", L.selbst.meldung);
    pruefe (L.gespeist == L.soll && L.verarbeitet == L.soll,
            kopf + ": Vorbedingung gespeiste Samplezahl = " + juce::String ((juce::int64) L.soll)
                + ", Laeufer und Engine",
            "gespeist " + juce::String ((juce::int64) L.gespeist) + ", von der Engine verarbeitet "
                + juce::String ((juce::int64) L.verarbeitet));
    pruefe (L.verworfen == 0u, kopf + ": Zaehlregel 7.3 - kein Ringverlust, ereignisseVerworfen() = 0",
            "verworfen " + juce::String ((juce::int64) L.verworfen));
    pruefe (! L.snapSoll.empty() && L.snapIst == L.snapSoll,
            kopf + ": Vorbedingung Evidenzsnapshots an den Samplezahlen des Kadenzmodells",
            juce::String ((int) L.snapIst.size()) + " Snapshots, Modell "
                + juce::String ((int) L.snapSoll.size()));
}

/** Ein Befund ueber die Kohaerenz eines Snapshots (alle Baender). */
struct Nak380KohBefund
{
    int mitBins { 0 }, ohneBit { 0 }, bitOhneBins { 0 }, unter095 { 0 };
    int dofFalsch { 0 }, ohneBasis { 0 };
    double minKoh { 2.0 }, qSumme { 0.0 }, qMax { 0.0 };
    int minBand { -1 }, qAnzahl { 0 };
    double qMittel() const noexcept { return qAnzahl > 0 ? qSumme / (double) qAnzahl : 0.0; }
    bool referenzHaelt() const noexcept
    {
        return qAnzahl > 0 && qMittel() >= kNak380RefMittelUnten && qMittel() <= kNak380RefMittelOben;
    }
};

void nak380KohPruefen (const Nak380Rauschlauf& L, int i, Nak380KohBefund& f)
{
    const auto& s = L.schnapp[(std::size_t) (i - 1)];
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto bf = nak380Binfenster (b, L.fs);
        const auto& z = s[(std::size_t) b];
        if (bf.bins() == 0)
        {
            if (z.koh) ++f.bitOhneBins;
            continue;
        }
        ++f.mitBins;
        const auto k = nak380RingFrames (L.snapSoll, i, bf.punkte(), bf.ring());
        if (z.dof != (std::uint32_t) k) ++f.dofFalsch;
        if (! z.basis) ++f.ohneBasis;
        if (! z.koh)
        {
            ++f.ohneBit;
            continue;
        }
        if ((double) z.kohWert < f.minKoh)
        {
            f.minKoh = (double) z.kohWert;
            f.minBand = b;
        }
        if (! ((double) z.kohWert >= 0.95))
            ++f.unter095;
        const double ref = nak380RhoQuadrat (L.d, bf.punkte());
        const double q = (1.0 - (double) z.kohWert) / (1.0 - ref);
        f.qSumme += q;
        f.qMax = std::max (f.qMax, q);
        ++f.qAnzahl;
    }
}

/** Die Laufzeitzusage je Band (M-74 bis M-80): Baender mit mindestens zwei
    Bins und Kohaerenzbit tragen die Gruppenlaufzeit 1000*d/fs ms +- 0,02 ms;
    Baender mit einem Bin und Baender ohne Kohaerenzbit tragen KEINE (nie 0).
    Die Abweichungen je Binzahl werden fuer den Manifestabschnitt
    mitgeschrieben (groesste je Klasse). */
struct Nak380LaufzeitBefund
{
    int geprueft { 0 }, ohneBit { 0 }, daneben { 0 }, einBinMitBit { 0 }, ohneKohMitBit { 0 };
    double maxAbw { 0.0 }, summeAbw { 0.0 };
    int maxBand { -1 };
    std::map<int, double> maxAbwJeBins;
};

Nak380LaufzeitBefund nak380LaufzeitPruefen (const Nak380Rauschlauf& L)
{
    Nak380LaufzeitBefund f;
    if (L.schnapp.empty())
        return f;
    const double soll = 1000.0 * (double) L.d / L.fs;
    const auto& s = L.schnapp.back();
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto bf = nak380Binfenster (b, L.fs);
        const auto& z = s[(std::size_t) b];
        if (z.lauf && ! z.koh) ++f.ohneKohMitBit;
        if (bf.bins() < 2)
        {
            if (z.lauf) ++f.einBinMitBit;
            continue;
        }
        if (! z.koh)
            continue;
        ++f.geprueft;
        if (! z.lauf)
        {
            ++f.ohneBit;
            continue;
        }
        const double abw = std::abs ((double) z.laufMs - soll);
        f.summeAbw += (double) z.laufMs - soll;
        auto& je = f.maxAbwJeBins[std::min (bf.bins(), 40)];
        je = std::max (je, abw);
        if (abw > f.maxAbw)
        {
            f.maxAbw = abw;
            f.maxBand = b;
        }
        if (! (abw <= 0.02))
            ++f.daneben;
    }
    return f;
}

juce::String nak380JeBinsText (const Nak380LaufzeitBefund& f)
{
    juce::String t;
    for (const auto& [bins, abw] : f.maxAbwJeBins)
        if (bins == 2 || bins == 3 || bins == 5 || bins == 10 || bins == 20 || bins >= 39)
            t << (t.isEmpty() ? "" : ", ") << bins << (bins >= 40 ? "+" : "") << " Bins "
              << juce::String (abw, 4);
    return t;
}

/** M-74 bis M-80: die Laufzeitlaeufe D1 bis D6, 10 s, Standardintervall. */
const Nak380Rauschlauf& nak380Laufzeitlauf (double fs, int block)
{
    return nak380Rauschen (fs, block, 0.25, (std::uint64_t) std::llround (10.0 * fs),
                           sig::laufzeitSamples (fs), sig::kD1D6Saat);
}

/** Kohaerenzzusage eines Laufzeitlaufs am letzten Snapshot (M-74 bis M-79):
    jedes Band mit Energie traegt ein Bit (sonst waere "jedes Band mit Bit"
    leer erfuellbar), jedes Band mit Bit Kohaerenz >= 0,95, und jede gegen die
    Referenz rho_w(d)^2 der zustaendigen Stufe innerhalb der hergeleiteten
    Toleranz. */
void nak380LaufzeitKohaerenz (const char* id, const char* name, const Nak380Rauschlauf& L)
{
    const auto kopf = nak380Kopf (id, name, L);
    Nak380KohBefund f;
    if (! L.schnapp.empty())
        nak380KohPruefen (L, (int) L.schnapp.size(), f);
    pruefe (! L.schnapp.empty() && f.mitBins > 0 && f.ohneBit == 0 && f.bitOhneBins == 0
                && f.unter095 == 0 && f.dofFalsch == 0,
            kopf + ": letzter Snapshot - jedes Band mit Energie traegt ein Bit, jedes Band mit Bit "
                "Kohaerenz >= 0,95; Freiheitsgrade nach Kadenzmodell",
            juce::String (f.mitBins) + " Baender mit Bins, ohne Bit " + juce::String (f.ohneBit)
                + ", Minimum " + juce::String (f.minKoh, 4) + " (Band " + juce::String (f.minBand)
                + "), unter 0,95 " + juce::String (f.unter095) + ", Freiheitsgrade falsch "
                + juce::String (f.dofFalsch));
    pruefe (! L.schnapp.empty() && f.referenzHaelt(),
            kopf + ": Kohaerenz gegen die Referenz rho_w(" + juce::String (L.d) + ")^2 = "
                + juce::String (nak380RhoQuadrat (L.d, kNak380NHaupt), 5) + " (N 4096) bzw. "
                + juce::String (nak380RhoQuadrat (L.d, kNak380NBass), 5)
                + " (N 16 384): Mittel von (1 - C)/(1 - rho^2) ueber die Baender mit Bit in [0,8; 1,2]",
            "Mittel " + juce::String (f.qMittel(), 4) + ", groesstes q " + juce::String (f.qMax, 3) + " ueber "
                + juce::String (f.qAnzahl) + " Baender");
}

void nak380LaufzeitWert (const char* id, const char* name, const Nak380Rauschlauf& L)
{
    const auto kopf = nak380Kopf (id, name, L);
    const auto f = nak380LaufzeitPruefen (L);
    const double soll = 1000.0 * (double) L.d / L.fs;
    pruefe (f.geprueft > 0 && f.ohneBit == 0 && f.daneben == 0,
            kopf + ": jedes Band mit mindestens zwei Bins und Kohaerenzbit traegt die Gruppenlaufzeit "
                + juce::String (soll, 6) + " ms +- 0,02 ms",
            juce::String (f.geprueft) + " Baender, ohne Laufzeitbit " + juce::String (f.ohneBit)
                + ", ausserhalb " + juce::String (f.daneben) + ", groesste Abweichung "
                + juce::String (f.maxAbw, 4) + " ms (Band " + juce::String (f.maxBand) + "), mittlere "
                "Abweichung mit Vorzeichen " + juce::String (f.geprueft > f.ohneBit
                    ? f.summeAbw / (double) (f.geprueft - f.ohneBit) : 0.0, 4) + " ms; groesste je Binzahl: "
                + nak380JeBinsText (f));
}

__declspec(noinline) void nak380M73()
{
    const auto& L = nak380Laufzeitlauf (48000.0, 512);
    const auto kopf = nak380Kopf ("M-73", "laufzeit_1ms_breitband_48k", L);
    nak380Vorbedingungen (kopf, L);
    // Ab dem siebten Snapshot sind beide Ringe voll (W_B = 7).
    Nak380KohBefund f;
    int geprueft = 0;
    for (int i = 7; i <= (int) L.schnapp.size(); ++i)
    {
        nak380KohPruefen (L, i, f);
        ++geprueft;
    }
    // Band 216 (15,77 kHz, 39 Bins) am letzten Snapshot, der dort ein Bit traegt.
    juce::String band216 ("Band 216 (15,77 kHz, 39 Bins) in keinem Snapshot mit Bit");
    for (int i = (int) L.schnapp.size(); i >= 1; --i)
        if (const auto& z = L.schnapp[(std::size_t) (i - 1)][216u]; z.koh)
        {
            band216 = "Band 216 (15,77 kHz, 39 Bins) im Snapshot " + juce::String (i) + ": "
                    + juce::String (z.kohWert, 4) + " bei " + juce::String ((int) z.dof) + " Frames";
            break;
        }
    pruefe (geprueft > 0 && geprueft + 6 == (int) L.snapSoll.size() && f.dofFalsch == 0 && f.ohneBasis == 0,
            kopf + ": Vorbedingung Snapshots 7 bis n, Freiheitsgrade = Frames der letzten W Fenster "
                "je Stufe (Kadenzmodell), jedes Band mit Bins traegt eine Basis",
            juce::String (geprueft) + " Snapshots, Freiheitsgrade falsch " + juce::String (f.dofFalsch)
                + ", ohne Basis " + juce::String (f.ohneBasis));
    pruefe (geprueft > 0 && f.ohneBit == 0 && f.bitOhneBins == 0,
            kopf + ": jedes Band mit Energie traegt ab dem siebten Snapshot ein Kohaerenzbit",
            juce::String (f.mitBins) + " Bandbefunde mit Bins, ohne Bit " + juce::String (f.ohneBit)
                + ", Bit ohne Bins " + juce::String (f.bitOhneBins));
    pruefe (geprueft > 0 && f.unter095 == 0 && f.minKoh <= 1.0,
            kopf + ": jedes Band 30 Hz bis 18 kHz mit Kohaerenzbit traegt Kohaerenz >= 0,95",
            "Minimum " + juce::String (f.minKoh, 4) + " (Band " + juce::String (f.minBand) + "), unter 0,95 "
                + juce::String (f.unter095) + "; " + band216);
    pruefe (geprueft > 0 && f.referenzHaelt(),
            kopf + ": Kohaerenz gegen die Referenz rho_w(48)^2 = "
                + juce::String (nak380RhoQuadrat (48, kNak380NHaupt), 5) + " (N 4096) bzw. "
                + juce::String (nak380RhoQuadrat (48, kNak380NBass), 5)
                + " (N 16 384): Mittel von (1 - C)/(1 - rho^2) ueber alle Bandbefunde mit Bit in [0,8; 1,2]",
            "Mittel " + juce::String (f.qMittel(), 4) + ", groesstes q " + juce::String (f.qMax, 3) + " ueber "
                + juce::String (f.qAnzahl) + " Bandbefunde");
}

__declspec(noinline) void nak380M74()
{
    for (const int block : kSweepBlockgroessen)
    {
        const auto& L = nak380Laufzeitlauf (48000.0, block);
        const auto kopf = nak380Kopf ("M-74", "laufzeit_1ms_alle_blockgroessen", L);
        nak380Vorbedingungen (kopf, L);
        nak380LaufzeitKohaerenz ("M-74", "laufzeit_1ms_alle_blockgroessen", L);
        nak380LaufzeitWert ("M-74", "laufzeit_1ms_alle_blockgroessen", L);
    }
}

__declspec(noinline) void nak380Rate (const char* id, const char* name, double fs)
{
    for (const int block : { 64, 512, 16384 })
    {
        const auto& L = nak380Laufzeitlauf (fs, block);
        nak380Vorbedingungen (nak380Kopf (id, name, L), L);
        nak380LaufzeitKohaerenz (id, name, L);
        nak380LaufzeitWert (id, name, L);
    }
}

/** M-80: dieselben Laeufe D1 bis D6, je Rate und Block eine Ausgabe; dazu
    "Baender mit einem Bin tragen keine Laufzeit, nie 0" und "keine Laufzeit
    ohne Kohaerenzbit". */
__declspec(noinline) void nak380M80()
{
    std::vector<std::pair<double, int>> laeufe;
    for (const int block : kSweepBlockgroessen)
        laeufe.push_back ({ 48000.0, block });
    for (const double fs : { 44100.0, 88200.0, 96000.0, 176400.0, 192000.0 })
        for (const int block : { 64, 512, 16384 })
            laeufe.push_back ({ fs, block });
    for (const auto& [fs, block] : laeufe)
    {
        const auto& L = nak380Laufzeitlauf (fs, block);
        const auto kopf = nak380Kopf ("M-80", "laufzeit_aus_phase", L);
        nak380Vorbedingungen (kopf, L);
        nak380LaufzeitWert ("M-80", "laufzeit_aus_phase", L);
        const auto f = nak380LaufzeitPruefen (L);
        pruefe (! L.schnapp.empty() && f.einBinMitBit == 0 && f.ohneKohMitBit == 0,
                kopf + ": Baender mit einem Bin und Baender ohne Kohaerenzbit tragen keine Laufzeit",
                "Einbinbaender mit Laufzeit " + juce::String (f.einBinMitBit) + ", ohne Kohaerenz mit Laufzeit "
                    + juce::String (f.ohneKohMitBit));
    }
}

/** M-81 (§7.2 D7): die Phase wird am Bin der geometrischen Bandmitte
    gelesen. Band 216: Mitte 15 770,61 Hz, bei 48 kHz und N = 4096 die Bins
    1327 bis 1365 (39); der Mitte am naechsten liegt 1346 (15 770,61/11,71875 =
    1345,76), 19 Bins ueber dem ersten. Soll = gewickeltes +2*pi*f_k*1 ms am
    gewaehlten Bin. Toleranz 0,25 rad (Matrix): die Phasenstreuung je Bin ist
    sqrt((1 - g2)/(2 K g2)) = sqrt(0,0018/44) = 0,0064 rad bei K = 22, 0,25 rad
    sind 39 sigma; die Erstbinwahl verschoebe die Referenz um
    2*pi*19*48 000/4096*0,001 = 1,39899 rad. */
__declspec(noinline) void nak380M81()
{
    const auto& L = nak380Rauschen (48000.0, 512, 0.25, 480000u, 48, sig::kD7Saat);
    const auto kopf = nak380Kopf ("M-81", "phase_am_bandmittenbin_D7", L);
    nak380Vorbedingungen (kopf, L);
    int b = -1;
    for (int i = 0; i < Gitter::evidenzBaender; ++i)
        if (std::abs (Gitter::evidenzMitte (i) - 15770.61) < 0.01)
            b = i;
    const auto bf = b >= 0 ? nak380Binfenster (b, 48000.0) : Nak380Binfenster {};
    const int kMitte = b >= 0 ? std::clamp ((int) std::llround (Gitter::evidenzMitte (b) * 4096.0 / 48000.0),
                                            bf.von, bf.bis - 1) : -1;
    pruefe (b == 216 && bf.von == 1327 && bf.bis == 1366 && kMitte == 1346 && kMitte - bf.von == 19,
            kopf + ": Vorbedingung Band 216 mit Mitte 15 770,61 Hz, Bins 1327 bis 1365 (39), Mittenbin 1346, "
                "19 Bins ueber dem ersten",
            "Band " + juce::String (b) + ", Bins " + juce::String (bf.von) + " bis " + juce::String (bf.bis - 1)
                + ", Mittenbin " + juce::String (kMitte));
    if (b < 0 || L.schnapp.empty())
    {
        pruefe (false, kopf + ": kein Band oder kein Snapshot");
        return;
    }
    const auto& z = L.schnapp.back()[(std::size_t) b];
    pruefe (z.koh && z.phase, kopf + ": Kohaerenz-Praesenzbit und Phasenbit gesetzt",
            "Kohaerenz " + (z.koh ? juce::String (z.kohWert, 4) : juce::String ("ohne Bit"))
                + ", Phase " + (z.phase ? juce::String (z.phaseWert, 4) : juce::String ("ohne Bit")));
    const double fk = (double) kMitte * 48000.0 / 4096.0;
    const double soll = nak380Wickeln (kZweiPi * fk * 0.001);
    const double sollErst = nak380Wickeln (kZweiPi * (double) bf.von * 48000.0 / 4096.0 * 0.001);
    const double abw = std::abs (nak380Wickeln ((double) z.phaseWert - soll));
    pruefe (z.phase && abw <= 0.25,
            kopf + ": Phase = gewickeltes +2*pi*f_k*1 ms am Mittenbin 1346 +- 0,25 rad",
            "ist " + juce::String (z.phaseWert, 5) + " rad, Soll " + juce::String (soll, 5) + " (f_k "
                + juce::String (fk, 3) + " Hz), Abweichung " + juce::String (abw, 5) + "; Erstbin-Soll "
                + juce::String (sollErst, 5) + ", Abstand "
                + juce::String (std::abs (nak380Wickeln (sollErst - soll)), 5) + " rad");
}

/** M-82: 44,1 kHz, Bloecke 64 bis 512, Standardintervall, Mono-Rauschen. Das
    erste Fenster endet nach 13 248 bis 13 824 Samples und traegt 5
    Hauptstufenframes (Kohaerenz `null`); ab dem zweiten liegen 11 bis 12
    Frames im Ring, und jedes Hauptstufenband mit Energie traegt ein Bit. */
__declspec(noinline) void nak380M82()
{
    for (const int block : { 64, 128, 256, 512 })
    {
        const auto& L = nak380Rauschen (44100.0, block, 0.25, 44100u, 0, sig::kD1D6Saat);
        const auto kopf = nak380Kopf ("M-82", "kohaerenz_44k1_kleine_bloecke", L);
        nak380Vorbedingungen (kopf, L);
        if (L.schnapp.size() < 2 || L.snapSoll.size() < 2)
        {
            pruefe (false, kopf + ": zwei Snapshots entstehen");
            continue;
        }
        const auto k1 = nak380RingFrames (L.snapSoll, 1, kNak380NHaupt, kNak380RingHaupt);
        const auto k2 = nak380RingFrames (L.snapSoll, 2, kNak380NHaupt, kNak380RingHaupt);
        int baender = 0, dof1Falsch = 0, bit1 = 0, dof2Falsch = 0, ohneBit2 = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto bf = nak380Binfenster (b, 44100.0);
            if (bf.bass || bf.bins() == 0)
                continue;
            ++baender;
            const auto& z1 = L.schnapp[0][(std::size_t) b];
            const auto& z2 = L.schnapp[1][(std::size_t) b];
            if (z1.dof != (std::uint32_t) k1) ++dof1Falsch;
            if (z1.koh) ++bit1;
            if (z2.dof != (std::uint32_t) k2) ++dof2Falsch;
            if (! z2.koh) ++ohneBit2;
        }
        pruefe (baender > 0 && k1 == 5u && dof1Falsch == 0 && bit1 == 0,
                kopf + ": erster Snapshot - 5 Hauptstufenframes je Band, Kohaerenz null",
                "Modell " + juce::String ((int) k1) + ", Freiheitsgrade falsch " + juce::String (dof1Falsch)
                    + ", mit Bit " + juce::String (bit1) + " von " + juce::String (baender));
        pruefe (baender > 0 && (k2 == 11u || k2 == 12u) && dof2Falsch == 0 && ohneBit2 == 0,
                kopf + ": zweiter Snapshot - 11 bis 12 Frames im Ring, jedes Hauptstufenband mit Energie traegt "
                    "ein Kohaerenzbit",
                "Modell " + juce::String ((int) k2) + ", Freiheitsgrade falsch " + juce::String (dof2Falsch)
                    + ", ohne Bit " + juce::String (ohneBit2) + " von " + juce::String (baender));
    }
}

/** M-83 (NAK-177): Bassbaender unter 200 Hz tragen Kohaerenz. 48 kHz, Block
    512, Mono-Rauschen. 0,25 s: die Evidenz faellt alle 15 360 Samples, der
    Bassring sieht am vierten Snapshot 6, am fuenften 8 Frames (danach 13 bis
    14); 1 s: Fenster 51 200 Samples, 5 bzw. 11 Bassframes. Geprueft wird
    beiderseits: vor der achten Frame kein Bit, ab ihr jedes Bassband mit
    Energie. */
__declspec(noinline) void nak380M83()
{
    struct Fall { double intervall; std::uint64_t samples; int ersterMitBit; std::uint64_t kVorher, kAb; };
    for (const Fall fall : { Fall { 0.25, 144000u, 5, 6u, 8u }, Fall { 1.0, 144000u, 2, 5u, 11u } })
    {
        const auto& L = nak380Rauschen (48000.0, 512, fall.intervall, fall.samples, 0, sig::kD1D6Saat);
        const auto kopf = nak380Kopf ("M-83", "bass_kohaerenz_nak177", L)
                        + " Intervall " + juce::String (fall.intervall, 2) + " s";
        nak380Vorbedingungen (kopf, L);
        const int n = (int) std::min (L.schnapp.size(), L.snapSoll.size());
        int baender = 0, vorherMitBit = 0, abOhneBit = 0, dofFalsch = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto bf = nak380Binfenster (b, 48000.0);
            if (! bf.bass || bf.bins() == 0)
                continue;
            ++baender;
            for (int i = 1; i <= n; ++i)
            {
                const auto& z = L.schnapp[(std::size_t) (i - 1)][(std::size_t) b];
                if (z.dof != (std::uint32_t) nak380RingFrames (L.snapSoll, i, kNak380NBass, kNak380RingBass))
                    ++dofFalsch;
                if (i < fall.ersterMitBit && z.koh) ++vorherMitBit;
                if (i >= fall.ersterMitBit && ! z.koh) ++abOhneBit;
            }
        }
        const bool modell = n >= fall.ersterMitBit
            && nak380RingFrames (L.snapSoll, fall.ersterMitBit - 1, kNak380NBass, kNak380RingBass) == fall.kVorher
            && nak380RingFrames (L.snapSoll, fall.ersterMitBit, kNak380NBass, kNak380RingBass) == fall.kAb;
        pruefe (baender > 0 && modell && dofFalsch == 0,
                kopf + ": Vorbedingung Bassframes im Ring nach Kadenzmodell (" + juce::String ((int) fall.kVorher)
                    + " vor, " + juce::String ((int) fall.kAb) + " am Snapshot " + juce::String (fall.ersterMitBit) + ")",
                juce::String (baender) + " Bassbaender mit Bins, Freiheitsgrade falsch " + juce::String (dofFalsch));
        pruefe (baender > 0 && vorherMitBit == 0,
                kopf + ": vor Snapshot " + juce::String (fall.ersterMitBit) + " traegt kein Bassband ein Kohaerenzbit "
                    "(weniger als acht Frames)",
                juce::String (vorherMitBit) + " Bandbefunde mit Bit");
        pruefe (baender > 0 && abOhneBit == 0,
                kopf + ": ab Snapshot " + juce::String (fall.ersterMitBit) + " traegt jedes Bassband mit Energie ein "
                    "Kohaerenzbit",
                juce::String (abOhneBit) + " Bandbefunde ohne Bit");
    }
}

/** M-84 und M-88: 44,1 kHz, Block 512, 0,25 s, Mono-Rauschen, zehn Fenster
    zu 13 824 Samples (138 240 Samples). Hauptstufe: 5, 7, 7, ... Frames je
    Fenster, 66 seit dem Start, die letzten drei Fenster am zehnten Snapshot
    66 - 46 = 20; Bass: 15 seit dem Start, die letzten sieben 15 - 4 = 11.
    Zweiter Snapshot: 5 + 7 = 12 bzw. 2. */
__declspec(noinline) void nak380M84M88 (bool m84)
{
    const auto& L = nak380Rauschen (44100.0, 512, 0.25, 138240u, 0, sig::kD1D6Saat);
    const auto kopf = m84 ? nak380Kopf ("M-84", "ringlaenge", L)
                          : nak380Kopf ("M-88", "ring_ueberlebt_evidenzfenster", L);
    nak380Vorbedingungen (kopf, L);
    const int i = m84 ? 10 : 2;
    if ((int) L.schnapp.size() < i || (int) L.snapSoll.size() < i)
    {
        pruefe (false, kopf + ": Snapshot " + juce::String (i) + " entsteht");
        return;
    }
    const auto kHaupt = nak380RingFrames (L.snapSoll, i, kNak380NHaupt, kNak380RingHaupt);
    const auto kBass = nak380RingFrames (L.snapSoll, i, kNak380NBass, kNak380RingBass);
    const auto seitStart = nak380Frames (L.snapSoll[(std::size_t) (i - 1)], kNak380NHaupt);
    int haupt = 0, bass = 0, hauptFalsch = 0, bassFalsch = 0;
    std::uint32_t hauptIst = 0, bassIst = 0;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto bf = nak380Binfenster (b, 44100.0);
        if (bf.bins() == 0)
            continue;
        const auto dof = L.schnapp[(std::size_t) (i - 1)][(std::size_t) b].dof;
        if (bf.bass) { ++bass; bassIst = dof; if (dof != (std::uint32_t) kBass) ++bassFalsch; }
        else         { ++haupt; hauptIst = dof; if (dof != (std::uint32_t) kHaupt) ++hauptFalsch; }
    }
    if (m84)
    {
        pruefe (haupt > 0 && kHaupt == 20u && seitStart == 66u && hauptFalsch == 0,
                kopf + ": zehnter Snapshot - jedes Hauptstufenband traegt die Frames der letzten 3 Fenster (20), "
                    "aeltere fallen heraus (66 seit dem Start)",
                "Modell " + juce::String ((int) kHaupt) + " von " + juce::String ((int) seitStart) + ", gemeldet "
                    + juce::String ((int) hauptIst) + ", abweichend " + juce::String (hauptFalsch) + " von "
                    + juce::String (haupt));
        pruefe (bass > 0 && kBass == 11u && bassFalsch == 0,
                kopf + ": zehnter Snapshot - jedes Bassband traegt die Frames der letzten 7 Fenster (11)",
                "Modell " + juce::String ((int) kBass) + ", gemeldet " + juce::String ((int) bassIst)
                    + ", abweichend " + juce::String (bassFalsch) + " von " + juce::String (bass));
    }
    else
    {
        // Nur die Hauptstufe: das erste Fenster (13 824 Samples) traegt noch
        // keinen Bassframe (der erste schliesst nach 16 384), die Bassstufe
        // haette am zweiten Snapshot mit und ohne Ring 2 Frames - keine
        // Pruefung mit Trennschaerfe (Lehre Z1 aus §31). Die Bassringlaenge
        // traegt M-84 (11 am zehnten Snapshot).
        pruefe (haupt > 0 && kHaupt == 12u && hauptFalsch == 0,
                kopf + ": zweiter Snapshot - Freiheitsgrade jedes Hauptstufenbands = Frames beider Fenster (5 + 7 = 12)",
                "Modell " + juce::String ((int) kHaupt) + ", gemeldet " + juce::String ((int) hauptIst)
                    + ", abweichend " + juce::String (hauptFalsch) + " von " + juce::String (haupt)
                    + "; Bassstufe Modell " + juce::String ((int) kBass) + " (ohne Pruefung)");
        juce::ignoreUnused (bass, bassIst, bassFalsch);
    }
}

/** M-85: die Ringbytes einer Engine, unabhaengig aus dem Gitter nachgerechnet
    (Kopf §6.4, T-380-6): 32 B (vier double) je Bin und Slot, W + 1 Slots je
    Bin (W Ringfenster und das Kurzfenster der Persistenz, W_H = 3 und
    W_B = 7), dazu 8 B je Band und Slot fuer die Frames aller 221 Baender,
    auf ganze 32-B-Elemente aufgerundet. 44,1 kHz: 1650 Haupt- und 62
    Bassbins, 1650*128 + 62*256 + 1144*8 = 236 224 B; 48 kHz: 1516 und 57,
    217 792 B. */
std::size_t nak380RingBytesSoll (double fs)
{
    std::size_t zaehler = 0, binBytes = 0;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto bf = nak380Binfenster (b, fs);
        const auto slots = (std::size_t) bf.ring() + 1u;
        zaehler += slots;
        binBytes += (std::size_t) bf.bins() * slots * 32u;
    }
    return (zaehler + 3u) / 4u * 32u + binBytes;
}

/** M-85: Speicher der Akkus je Bin (R-380-3 "Speicher ... in der Matrix als
    Zahl benannt"). Je Rate eine frische Engine; die Ringbytes stehen exakt
    auf der Nachrechnung, bei 44,1 und 48 kHz zusaetzlich auf den Zahlen der
    Matrix, und unter der Schranke 240 KiB = 245 760 B je Engine (mit
    Etappe 6 skalieren die FFT-Laengen; heute liegen 96 und 192 kHz darunter).
    Auch die reservierte Kapazitaet haelt die Schranke. */
__declspec(noinline) void nak380M85()
{
    struct Fall { double fs; std::size_t matrix; };
    for (const Fall fall : { Fall { 44100.0, 236224u }, Fall { 48000.0, 217792u },
                             Fall { 96000.0, 0u }, Fall { 192000.0, 0u } })
    {
        auto halter = std::make_unique<FeatureEngine>();
        halter->vorbereiten (fall.fs);
        std::size_t ist = 0, kapazitaet = 0;
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
        ist = FeatureEngineTestzugang::ringBytes (*halter);
        kapazitaet = FeatureEngineTestzugang::ringKapazitaetBytes (*halter);
#endif
        const std::size_t soll = nak380RingBytesSoll (fall.fs);
        pruefe (ist == soll && (fall.matrix == 0u || soll == fall.matrix) && ist <= 245760u
                    && kapazitaet <= 245760u && ist > 0u,
                "380/M-85 stereo_speicher (" + juce::String (fall.fs / 1000.0, 1) + " kHz): Ringbytes = "
                    + juce::String ((juce::int64) soll) + " B (nachgerechnet"
                    + (fall.matrix != 0u ? juce::String (", Matrix ") + juce::String ((juce::int64) fall.matrix)
                                         : juce::String()) + ") <= 245 760 B",
                "ist " + juce::String ((juce::int64) ist) + " B, reserviert " + juce::String ((juce::int64) kapazitaet)
                    + " B");
    }
}

/** Ein 1-kHz-Sinus L = R (Amplitude 0,4) ab Stromsample `ab`, `n` Samples,
    deterministisch ohne Saat (M-86, M-87). */
std::vector<float> nak380Sinus (double fs, std::uint64_t ab, std::uint64_t n)
{
    std::vector<float> x ((std::size_t) n);
    for (std::uint64_t i = 0; i < n; ++i)
        x[(std::size_t) i] = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) (ab + i) / fs));
    return x;
}

/** M-86, Erweiterung des bestehenden Falls `coherence_carries_window_and_dof`
    ueber den Blocklaeufer: 48 kHz, 1 s, Block 512, Sinus L = R, drei Fenster
    zu 51 200 Samples (153 600). Freiheitsgrade am dritten Snapshot = Frames
    der letzten drei Fenster = floor((153 600 - 4096)/2048) + 1 = 74 (Golden
    der Klasse (ii), E-380-18; am unveraenderten Code waren es 25, nur das
    letzte Fenster); die Fensterdauer ist die Summe der Hopdauern genau dieser
    Frames, 74 * 2048/48 000 s = 3157,33 ms, +- 1 ms (Klasse (i); float32 von
    3157 ms rundet auf 0,00024 ms). */
constexpr std::uint32_t kNak380M86Freiheitsgrade = 74u;

__declspec(noinline) void nak380M86()
{
    const juce::String kopf ("380/M-86 fensterdauer_und_freiheitsgrade_ueber_den_ring (48,0 kHz, B 512, 1 s)");
    auto halter = std::make_unique<FeatureEngine>();
    auto& e = *halter;
    e.vorbereiten (48000.0);
    e.evidenzIntervallSetzen (1.0);
    const auto x = nak380Sinus (48000.0, 0, 153600u);
    const int b1k = bandFuer (1000.0);
    std::vector<Nak380Band> je;
    Nak380Pos pos;
    const auto lauf = nak380Stereolauf (e, pos, 48000.0, 512, x, x,
        [&je, b1k] (FeatureEngine& eng, int) { je.push_back (nak380Lesen (eng.stereoBand (b1k))); });
    std::uint64_t verarbeitet = 0;
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
    verarbeitet = FeatureEngineTestzugang::verarbeitet (e);
#endif
    const auto snapSoll = nak380Snapshots (48000.0, 512, 1.0, 153600u);
    pruefe (lauf.gespeist == 153600u && verarbeitet == 153600u,
            kopf + ": Vorbedingung gespeiste Samplezahl = 153 600 (3 s * 48 000), Laeufer und Engine",
            juce::String ((juce::int64) lauf.gespeist) + " / " + juce::String ((juce::int64) verarbeitet));
    pruefe (e.ereignisseVerworfen() == 0u, kopf + ": Zaehlregel 7.3 - kein Ringverlust, ereignisseVerworfen() = 0");
    pruefe (lauf.snapshots == snapSoll && snapSoll.size() == 3u && je.size() == 3u,
            kopf + ": Vorbedingung drei Snapshots an 51 200, 102 400 und 153 600 Samples (Kadenzmodell)",
            juce::String ((int) lauf.snapshots.size()) + " Snapshots");
    if (je.size() != 3u)
        return;
    const double hopMs = 1000.0 * 2048.0 / 48000.0;
    const auto k3 = nak380RingFrames (snapSoll, 3, kNak380NHaupt, kNak380RingHaupt);
    pruefe (je[2].dof == kNak380M86Freiheitsgrade && k3 == (std::uint64_t) kNak380M86Freiheitsgrade,
            kopf + ": Freiheitsgrade am dritten Snapshot = Frames im Ring ueber W = 3 Fenster (74), nicht nur "
                "das letzte (25)",
            "gemeldet " + juce::String ((int) je[2].dof) + ", Modell " + juce::String ((int) k3) + ", Golden "
                + juce::String ((int) kNak380M86Freiheitsgrade));
    pruefe (std::abs ((double) je[2].dauerMs - hopMs * (double) je[2].dof) < 1.0 && je[2].dof > 0u,
            kopf + ": Fensterdauer = Summe der Hopdauern genau der gezaehlten Frames +- 1 ms (dritter Snapshot)",
            juce::String (je[2].dauerMs, 3) + " ms gegen " + juce::String (hopMs * (double) je[2].dof, 3) + " ms");
    pruefe (je[0].dof == 24u && std::abs ((double) je[0].dauerMs - hopMs * 24.0) < 1.0,
            kopf + ": erster Snapshot unveraendert 24 Frames und 1024 ms (floor((51 200 - 4096)/2048) + 1)",
            juce::String ((int) je[0].dof) + " Frames, " + juce::String (je[0].dauerMs, 3) + " ms");
}

/** M-87: der Ring leert an Grenze, Ruecksetzen und Ratenwechsel
    (starten <-> stoppen). 44,1 kHz, Block 512, 0,25 s, Sinus L = R. Vor dem
    Ereignis 5 s (220 500 Samples, der letzte Block traegt 340); danach 1 s.
    (a) Seek: die Projektzeit springt um 441 000 Samples (Grund zeitSprung);
    die Kadenz laeuft weiter (§10.1), der erste Snapshot danach traegt nur die
    Frames seit der Grenze (hoechstens 5, weil das erste Fenster nach
    spaetestens 13 824 Samples endet; hier faellt er schon 684 Samples nach
    der Grenze, mit 0 Frames), Kohaerenz `null`. (b) `zuruecksetzen()`:
    Kadenz von vorn, 5 Frames, Kohaerenz `null`. (c) `vorbereiten (96000)`:
    Binzuordnung fuer 96 kHz neu, der erste Snapshot traegt genau die Frames
    der neuen Rate (13), kein Frame der alten ueberlebt. In allen drei Beinen
    traegt auch der zweite Snapshot genau die Frames seit dem Ereignis (ein
    ueberlebender Ring truege dort noch Fenster von davor). */
__declspec(noinline) void nak380M87()
{
    constexpr double fs = 44100.0;
    constexpr std::uint64_t vorher = 220500u, nachher = 44100u;
    const int b1k = bandFuer (1000.0);
    for (const char* bein : { "seek", "reset", "rate_change" })
    {
        const juce::String kopf = juce::String ("380/M-87 ring_leert ") + bein;
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        e.evidenzIntervallSetzen (0.25);
        Nak380Pos pos;
        const auto a = nak380Sinus (fs, 0, vorher);
        const auto lauf1 = nak380Stereolauf (e, pos, fs, 512, a, a);
        const bool rate = std::strcmp (bein, "rate_change") == 0;
        const bool seek = std::strcmp (bein, "seek") == 0;
        const auto getrenntVorher = e.getrennteFenster();
        const auto spruengeVorher = e.grenzenMitGrund (nakama::analyse::Grenzgrund::zeitSprung);
        if (seek)
            pos.projekt += 441000;
        else if (rate)
            e.vorbereiten (96000.0);
        else
            e.zuruecksetzen();
        const double fs2 = rate ? 96000.0 : fs;
        const std::uint64_t n2 = rate ? 96000u : nachher;
        const auto b2 = nak380Sinus (fs2, pos.strom, n2);
        // Der erste Block danach fuer sich: an ihm zieht der Seek seine Grenze,
        // und 512 Samples schliessen keinen Frame - der Ring muss danach leer
        // sein (Ruecksetzen und Ratenwechsel leeren ihn schon beim Aufruf).
        const std::vector<float> b2a (b2.begin(), b2.begin() + 512);
        const std::vector<float> b2b (b2.begin() + 512, b2.end());
        std::vector<Nak380Band> danach;
        const auto merken = [&danach, b1k] (FeatureEngine& eng, int)
        {
            if (danach.size() < 2u) danach.push_back (nak380Lesen (eng.stereoBand (b1k)));
        };
        const auto lauf2a = nak380Stereolauf (e, pos, fs2, 512, b2a, b2a, merken);
        double ringNachEreignis = -1.0;
        std::size_t ringBytesNach = 0;
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
        ringNachEreignis = FeatureEngineTestzugang::ringFramesAlle (e);
        ringBytesNach = FeatureEngineTestzugang::ringBytes (e);
#endif
        const auto lauf2 = nak380Stereolauf (e, pos, fs2, 512, b2b, b2b, merken);
        std::uint64_t verarbeitet = 0;
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
        verarbeitet = FeatureEngineTestzugang::verarbeitet (e);
#endif
        // Samples seit dem Ereignis bis zum ersten und zweiten Snapshot danach,
        // aus dem Kadenzmodell: beim Seek laeuft die Kadenz ueber die Grenze
        // weiter (Segmente 220 500 und 44 100, jedes mit eigenem Restblock),
        // sonst beginnt sie von vorn.
        std::vector<std::uint64_t> seit;
        if (seek)
        {
            for (const auto sn : nak380SnapshotsSegmente (fs, 512, 0.25, { vorher, nachher }))
                if (sn > vorher) seit.push_back (sn - vorher);
        }
        else
            seit = nak380Snapshots (fs2, 512, 0.25, n2);
        // Ringbytes nach dem Ereignis aus dem Gitter der geltenden Rate (M-85).
        const std::size_t ringBytesSoll = nak380RingBytesSoll (fs2);
        pruefe (ringNachEreignis == 0.0 && ringBytesNach == ringBytesSoll,
                kopf + ": direkt nach dem Ereignis (und dem ersten Block) traegt der Ring keinen Frame, und sein "
                    "Aufbau folgt der geltenden Binzuordnung (" + juce::String ((juce::int64) ringBytesSoll) + " B)",
                "Ringframes " + juce::String (ringNachEreignis, 0) + ", Ringbytes "
                    + juce::String ((juce::int64) ringBytesNach));
        const auto gespeist2 = lauf2a.gespeist + lauf2.gespeist;
        const bool zwei = danach.size() >= 2u && seit.size() >= 2u;
        const auto soll1 = zwei ? nak380Frames (seit[0], kNak380NHaupt) : 0u;
        const auto soll2 = zwei ? nak380Frames (seit[1], kNak380NHaupt) : 0u;
        pruefe (lauf1.gespeist == vorher && gespeist2 == n2
                    && verarbeitet == (seek ? vorher + nachher : n2)
                    && e.ereignisseVerworfen() == 0u && lauf1.snapshots.size() >= 7u,
                kopf + ": Vorbedingung gespeiste Samplezahl " + juce::String ((juce::int64) vorher) + " + "
                    + juce::String ((juce::int64) n2) + " (Laeufer und Engine), kein Ringverlust, Ring vorher voll "
                    "(mindestens 7 Snapshots)",
                juce::String ((juce::int64) lauf1.gespeist) + " + " + juce::String ((juce::int64) gespeist2)
                    + ", Engine " + juce::String ((juce::int64) verarbeitet) + ", Snapshots vorher "
                    + juce::String ((int) lauf1.snapshots.size()));
        if (seek)
            pruefe (e.getrennteFenster() == getrenntVorher + 1
                        && e.grenzenMitGrund (nakama::analyse::Grenzgrund::zeitSprung) == spruengeVorher + 1,
                    kopf + ": Vorbedingung genau eine Grenze mit Grund zeitSprung");
        // (a), (b): hoechstens 5 Frames; (c): genau die 13 der neuen Rate.
        pruefe (zwei && (rate ? soll1 == 13u : soll1 <= 5u) && danach[0].dof == (std::uint32_t) soll1,
                kopf + ": der erste Snapshot danach traegt nur die Frames seit dem Ereignis ("
                    + juce::String ((int) soll1) + ", Kadenzmodell)",
                zwei ? "gemeldet " + juce::String ((int) danach[0].dof) + " nach "
                           + juce::String ((juce::int64) seit[0]) + " Samples" : juce::String ("keine zwei Snapshots"));
        pruefe (zwei && danach[1].dof == (std::uint32_t) soll2,
                kopf + ": und der zweite die Frames beider Fenster seit dem Ereignis ("
                    + juce::String ((int) soll2) + ", Kadenzmodell)",
                zwei ? "gemeldet " + juce::String ((int) danach[1].dof) + " nach "
                           + juce::String ((juce::int64) seit[1]) + " Samples" : juce::String ("keine zwei Snapshots"));
        if (rate)
            pruefe (zwei && danach[0].koh && danach[0].kohWert > 0.99f,
                    kopf + ": bei 96 kHz traegt das 1-kHz-Band aus 13 Frames der neuen Rate ein Kohaerenzbit (Mono)",
                    zwei && danach[0].koh ? juce::String (danach[0].kohWert, 4) : juce::String ("ohne Bit"));
        else
            pruefe (zwei && ! danach[0].koh,
                    kopf + ": und die Kohaerenz des ersten ist null (weniger als acht Frames)",
                    zwei && danach[0].koh ? juce::String (danach[0].kohWert, 4) : juce::String ("ohne Bit"));
    }
}

/** M-91 (Etappe 5): Mittel und Maximum der Bandkohaerenz ueber alle Baender
    mit Kohaerenzbit (Erweiterung des Sweeps `sweep_uncorrelated_channels`). */
struct Nak380KohMittel
{
    int baender { 0 };
    double mittel { 0.0 }, maximum { 0.0 };
    int maxBand { -1 };
    std::uint32_t kMin { 0 }, kMax { 0 };
};

Nak380KohMittel nak380KohMittel (const FeatureEngine& e)
{
    Nak380KohMittel m;
    double summe = 0.0;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto& w = e.stereoBand (b);
        if (! w.kohaerenzGesetzt)
            continue;
        ++m.baender;
        summe += (double) w.kohaerenz;
        if ((double) w.kohaerenz > m.maximum || m.maxBand < 0)
        {
            m.maximum = (double) w.kohaerenz;
            m.maxBand = b;
        }
        m.kMin = m.baender == 1 ? w.freiheitsgrade : std::min (m.kMin, w.freiheitsgrade);
        m.kMax = std::max (m.kMax, w.freiheitsgrade);
    }
    m.mittel = m.baender > 0 ? summe / (double) m.baender : 0.0;
    return m;
}

/** M-92 (b), Grenze beiderseits: zwei unabhaengige Rauschstroeme (Saaten des
    Sweeps), 48 kHz, 0,25 s. Block 4096: der erste Snapshot faellt bei
    2 * 8192 = 16 384 Samples, floor((16 384 - 4096)/2048) + 1 = 7 Frames;
    Block 9216: bei 2 * 9216 = 18 432 Samples, 8 Frames. */
__declspec(noinline) void nak380M92Grenze()
{
    for (const int block : { 4096, 9216 })
    {
        const juce::String kopf = "380/M-92 coherence_is_null_below_eight_frames (48,0 kHz, B "
                                + juce::String (block) + ")";
        constexpr std::uint64_t n = 24000u;
        std::vector<float> l ((std::size_t) n), r ((std::size_t) n);
        for (std::uint64_t i = 0; i < n; ++i)
        {
            l[(std::size_t) i] = (float) (0.35 * sig::laufzeitRauschen ((std::int64_t) i, 0x1234ABCD5678EF01ull));
            r[(std::size_t) i] = (float) (0.35 * sig::laufzeitRauschen ((std::int64_t) i, 0xC0FFEE1234567890ull));
        }
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (48000.0);
        e.evidenzIntervallSetzen (0.25);
        Nak380Pos pos;
        Nak380Schnapp erster;
        const auto lauf = nak380Stereolauf (e, pos, 48000.0, block, l, r,
            [&erster] (FeatureEngine& eng, int nr)
            {
                if (nr != 1) return;
                erster.resize ((std::size_t) Gitter::evidenzBaender);
                for (int b = 0; b < Gitter::evidenzBaender; ++b)
                    erster[(std::size_t) b] = nak380Lesen (eng.stereoBand (b));
            });
        const auto snap = nak380Snapshots (48000.0, block, 0.25, n);
        const std::uint64_t k = snap.empty() ? 0u : nak380Frames (snap.front(), kNak380NHaupt);
        const std::uint64_t kSoll = block == 4096 ? 7u : 8u;
        std::uint64_t verarbeitet = 0;
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
        verarbeitet = FeatureEngineTestzugang::verarbeitet (e);
#endif
        pruefe (lauf.gespeist == n && verarbeitet == n && e.ereignisseVerworfen() == 0u
                    && ! lauf.snapshots.empty() && lauf.snapshots.front() == (snap.empty() ? 0u : snap.front())
                    && k == kSoll,
                kopf + ": Vorbedingung 24 000 Samples, kein Ringverlust, erster Snapshot nach Kadenzmodell mit "
                    + juce::String ((int) kSoll) + " Hauptstufenframes",
                "gespeist " + juce::String ((juce::int64) lauf.gespeist) + ", Engine "
                    + juce::String ((juce::int64) verarbeitet) + ", Modell " + juce::String ((int) k));
        int baender = 0, dofFalsch = 0, mitBit = 0, ungueltig = 0;
        for (int b = 0; b < Gitter::evidenzBaender && ! erster.empty(); ++b)
        {
            const auto bf = nak380Binfenster (b, 48000.0);
            if (bf.bass || bf.bins() == 0)
                continue;
            ++baender;
            const auto& z = erster[(std::size_t) b];
            if (z.dof != (std::uint32_t) kSoll) ++dofFalsch;
            if (z.koh)
            {
                ++mitBit;
                if (! (z.kohWert >= 0.0f && z.kohWert < 1.0f)) ++ungueltig;
            }
        }
        if (block == 4096)
            pruefe (baender > 0 && dofFalsch == 0 && mitBit == 0,
                    kopf + ": 7 Frames - kein Hauptstufenband traegt ein Kohaerenzbit (null, nicht 1)",
                    juce::String (mitBit) + " mit Bit von " + juce::String (baender) + ", Freiheitsgrade falsch "
                        + juce::String (dofFalsch));
        else
            pruefe (baender > 0 && dofFalsch == 0 && mitBit == baender && ungueltig == 0,
                    kopf + ": 8 Frames - jedes Hauptstufenband traegt einen Kohaerenzwert in [0, 1)",
                    juce::String (mitBit) + " mit Bit von " + juce::String (baender) + ", ausserhalb [0, 1) "
                        + juce::String (ungueltig) + ", Freiheitsgrade falsch " + juce::String (dofFalsch));
    }
}

/** M-93 (A-4): die Persistenz rechnet mit der MSC je Bin. Derselbe Lauf wie
    M-73 (D2, 48 kHz, Block 512, 0,25 s). Das Kurzfenster (8 Hauptstufen-
    frames) schliesst in einem Evidenzfenster genau dann, wenn es mindestens
    acht Frames traegt (das Kurzfenster faellt mit dem Evidenzfenster); dort
    ist die MSC je Bin im Band 15,77 kHz rund 0,998 >= 0,8, die Persistenz
    also 1 > 0,5. Die Bandsumme laege bei 0,477 < 0,8 und gaebe 0. */
__declspec(noinline) void nak380M93()
{
    const auto& L = nak380Laufzeitlauf (48000.0, 512);
    const auto kopf = nak380Kopf ("M-93", "persistenz_je_bin", L);
    nak380Vorbedingungen (kopf, L);
    const int b = 216;
    int soll = 0, ist = 0, niedrig = 0;
    double minimum = 2.0;
    for (int i = 1; i <= (int) std::min (L.schnapp.size(), L.snapSoll.size()); ++i)
    {
        const auto fenster = nak380Frames (L.snapSoll[(std::size_t) (i - 1)], kNak380NHaupt)
                           - (i >= 2 ? nak380Frames (L.snapSoll[(std::size_t) (i - 2)], kNak380NHaupt) : 0u);
        const bool kurz = fenster >= 8u;
        const auto& z = L.schnapp[(std::size_t) (i - 1)][(std::size_t) b];
        if (kurz) ++soll;
        if (z.pers)
        {
            ++ist;
            minimum = std::min (minimum, (double) z.persWert);
            if (! (z.persWert > 0.5f)) ++niedrig;
        }
    }
    pruefe (soll > 0 && ist == soll,
            kopf + ": Vorbedingung Band 216 traegt die Persistenz genau in den Fenstern mit mindestens acht "
                "Hauptstufenframes (Kadenzmodell)",
            juce::String (ist) + " Snapshots mit Bit, Modell " + juce::String (soll));
    pruefe (ist > 0 && niedrig == 0,
            kopf + ": Persistenz im Band 15,77 kHz > 0,5 (Kurzfenster 8 Frames, MSC je Bin rund 0,998 >= 0,8)",
            "Minimum " + juce::String (minimum, 3) + ", nicht ueber 0,5 " + juce::String (niedrig));
}

__declspec(noinline) void nak380Etappe5()
{
    std::cout << "\n== NAK-380 Etappe 5 - Kohaerenz je Bin ueber Frames (§6.4) ==" << std::endl;
    if (nak380Waehlt ("M-73")) { abschnitt ("380/M-73 laufzeit_1ms_breitband_48k"); nak380M73(); }
    if (nak380Waehlt ("M-74")) { abschnitt ("380/M-74 laufzeit_1ms_alle_blockgroessen"); nak380M74(); }
    if (nak380Waehlt ("M-75")) { abschnitt ("380/M-75 laufzeit_1ms_44k1"); nak380Rate ("M-75", "laufzeit_1ms_44k1", 44100.0); }
    if (nak380Waehlt ("M-76")) { abschnitt ("380/M-76 laufzeit_1ms_88k2"); nak380Rate ("M-76", "laufzeit_1ms_88k2", 88200.0); }
    if (nak380Waehlt ("M-77")) { abschnitt ("380/M-77 laufzeit_1ms_96k"); nak380Rate ("M-77", "laufzeit_1ms_96k", 96000.0); }
    if (nak380Waehlt ("M-78")) { abschnitt ("380/M-78 laufzeit_1ms_176k4"); nak380Rate ("M-78", "laufzeit_1ms_176k4", 176400.0); }
    if (nak380Waehlt ("M-79")) { abschnitt ("380/M-79 laufzeit_1ms_192k"); nak380Rate ("M-79", "laufzeit_1ms_192k", 192000.0); }
    if (nak380Waehlt ("M-80")) { abschnitt ("380/M-80 laufzeit_aus_phase"); nak380M80(); }
    if (nak380Waehlt ("M-81")) { abschnitt ("380/M-81 phase_am_bandmittenbin_D7"); nak380M81(); }
    if (nak380Waehlt ("M-82")) { abschnitt ("380/M-82 kohaerenz_44k1_kleine_bloecke"); nak380M82(); }
    if (nak380Waehlt ("M-83")) { abschnitt ("380/M-83 bass_kohaerenz_nak177"); nak380M83(); }
    if (nak380Waehlt ("M-84")) { abschnitt ("380/M-84 ringlaenge"); nak380M84M88 (true); }
    if (nak380Waehlt ("M-85")) { abschnitt ("380/M-85 stereo_speicher"); nak380M85(); }
    if (nak380Waehlt ("M-87")) { abschnitt ("380/M-87 ring_leert"); nak380M87(); }
    if (nak380Waehlt ("M-88")) { abschnitt ("380/M-88 ring_ueberlebt_evidenzfenster"); nak380M84M88 (false); }
    if (nak380Waehlt ("M-93")) { abschnitt ("380/M-93 persistenz_je_bin"); nak380M93(); }
}
} // namespace

int main (int argc, char* argv[])
{
    // NAK-380 Etappe 5: `--nak380 M-nn` faehrt nur diesen Fall (Rotbeweise
    // nach §8.1: nur der betroffene Fall). Bestehende Abschnitte, die eine
    // Matrixzeile tragen, laufen unter ihrer Kennung mit.
    if (argc == 3 && std::strcmp (argv[1], "--nak380") == 0)
        nak380Nur = argv[2];
    else if (argc != 1)
        return 2;

    constexpr double fs = 48000.0;
    std::cout << "== Nakama SONDE-013 - bandweise Stereoevidenz (§40.1, §40.3) =="
              << std::endl;

    // ── M-08: mono_identity ───────────────────────────────────────────────
    if (abschnittWaehlt ("M-89", "M-08  mono_identity"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // ⚠️ 1 Hz statt der Vorgabe 4 Hz. Bei 0,25 s Evidenzfenster liegen nur
        // rund fuenf Welch-Frames darin, und die Kohaerenz braucht acht
        // (§40.1). Der Fall waere sonst gruen, ohne die Kohaerenz je zu
        // sehen - gemessen beim Bau.
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;                                  // bitgleich, echtes Mono
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int b1k = bandFuer (1000.0);
        pruefe (b1k >= 0, "das 1-kHz-Band ist im Gitter auffindbar",
                "Band " + juce::String (b1k));
        const auto& w = e.stereoBand (b1k);
        pruefe (w.basisGesetzt, "es traegt eine Basis");
        pruefe (w.basisGesetzt && w.korrelationMittel > 0.999f,
                "bei echtem Mono ist die Korrelation exakt 1",
                juce::String (w.korrelationMittel, 6));
        pruefe (w.basisGesetzt && w.seitenanteilDb < -60.0f,
                "und der Seitenanteil liegt am Boden - es gibt keine Seitenenergie",
                juce::String (w.seitenanteilDb, 2) + " dB");
        pruefe (w.kohaerenzGesetzt && w.kohaerenz > 0.99f,
                "die Kohaerenz ist 1 - zwei identische Kanaele sind vollstaendig kohaerent",
                juce::String (w.kohaerenz, 6));
        pruefe (w.phaseGesetzt && std::abs (w.phaseRad) < 0.01f,
                "und die Phase ist 0 - kein Laufzeitunterschied",
                juce::String (w.phaseRad, 6) + " rad");

        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt && std::abs (sk.monoFolddownDb) < 0.25f,
                "der Mono-Folddown ist 0 dB innerhalb 0,25 dB (§40.3) - eine "
                "Monosumme aus zwei gleichen Kanaelen verliert nichts",
                juce::String (sk.monoFolddownDb, 4) + " dB");
        pruefe (sk.balanceGesetzt && std::abs (sk.lrBalanceDb) < 0.01f,
                "und die L/R-Balance ist 0 dB",
                juce::String (sk.lrBalanceDb, 4) + " dB");
    }

    // ── M-08: silent_channel_has_no_bit ──────────────────────────────────
    if (abschnittWaehlt ("M-92", "M-08  silent_channel_has_no_bit"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = 0.0f;                               // ein stiller Kanal
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int b1k = bandFuer (1000.0);
        const auto& w = e.stereoBand (b1k);
        // Die Kohaerenz braucht BEIDE Autospektren; mit einem stillen Kanal
        // ist eines davon null, und der Nenner waere es auch. Es entsteht
        // kein Wert - keine 0 und kein NaN.
        pruefe (! w.kohaerenzGesetzt,
                "mit einem stillen Kanal faellt das Kohaerenzbit weg, statt 0 oder NaN "
                "zu senden");
        pruefe (! w.phaseGesetzt,
                "und die Phase erst recht - sie ist eine Teilmenge der Kohaerenzbits");
        pruefe (w.basisGesetzt && w.seitenanteilDb > -6.0f,
                "die Basis steht trotzdem: ein stiller Kanal ist maximal breit "
                "(Mid und Side tragen dieselbe Energie)",
                juce::String (w.seitenanteilDb, 2) + " dB");

        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt && sk.monoFolddownDb < -2.5f
                  && sk.monoFolddownDb > -3.5f,
                "der Mono-Folddown ist rund -3 dB - die halbe Energie ist im "
                "stillen Kanal",
                juce::String (sk.monoFolddownDb, 3) + " dB");
        pruefe (sk.balanceGesetzt && sk.lrBalanceDb > 300.0f,
                "und die Balance kippt vollstaendig nach links - der Wert laeuft an "
                "die Vertragsgrenze, statt zu fehlen: 'ganz links' ist eine Aussage, "
                "kein Schweigen",
                juce::String (sk.lrBalanceDb, 1) + " dB");
    }

    // ── M-08: folddown_matches_real_buffer_within_0p25db ─────────────────
    //
    // §40.3 verlangt, dass der ANGEZEIGTE Verlust dem wirklich gefalteten
    // Audiopuffer entspricht. Der Fall rechnet den Puffer hier noch einmal
    // von Hand - eine Schaetzung aus der Korrelation laege bei diesem Signal
    // um Dezibel daneben.
    if (abschnittWaehlt (nullptr, "M-08  folddown_matches_real_buffer_within_0p25db"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };

        double monoEnergie = 0.0, stereoEnergie = 0.0;
        const auto signal = [&] (std::uint64_t n, float& l, float& r)
        {
            const double t = (double) n / 48000.0;
            l = (float) (0.35 * std::sin (kZweiPi * 700.0 * t));
            r = (float) (0.28 * std::sin (kZweiPi * 1100.0 * t + 1.1));
            const double mono = 0.5 * ((double) l + (double) r);
            monoEnergie   += mono * mono;
            stereoEnergie += 0.5 * ((double) l * l + (double) r * r);
        };
        const bool kam = s.bisEvidenz (signal);
        pruefe (kam, "ein Evidenzfenster entsteht");

        // ⚠️ Die Handrechnung laeuft ueber ALLE gesendeten Samples, die
        // Engine ueber ihr Evidenzfenster. Beide sind hier gleich lang,
        // weil das erste Fenster mit dem ersten Block beginnt - deshalb
        // steht die Zusage in einer Toleranz und nicht als Gleichheit.
        const double erwartet = 10.0 * std::log10 (monoEnergie / stereoEnergie);
        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt
                  && std::abs ((double) sk.monoFolddownDb - erwartet) < 0.25,
                "der gemeldete Verlust stimmt mit dem von Hand gefalteten Puffer "
                "innerhalb 0,25 dB ueberein",
                "gemeldet " + juce::String (sk.monoFolddownDb, 3) + " dB, von Hand "
                + juce::String (erwartet, 3) + " dB");
    }

    // ── M-11: bandwise_ms_and_correlation ────────────────────────────────
    //
    // Der eigentliche Grund fuer die bandweise Rechnung: ein Signal, das
    // UNTEN mono und OBEN breit ist. Zwei globale Skalare koennen das nicht
    // ausdruecken - 221 Baender schon.
    if (abschnittWaehlt (nullptr, "M-11  bandwise_ms_and_correlation"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            const double t = (double) n / 48000.0;
            const double tief = 0.4 * std::sin (kZweiPi * 100.0 * t);
            // Oben: zwei verschiedene Frequenzen, also dekorreliert.
            const double hochL = 0.25 * std::sin (kZweiPi * 5000.0 * t);
            const double hochR = 0.25 * std::sin (kZweiPi * 5300.0 * t + 0.4);
            l = (float) (tief + hochL);
            r = (float) (tief + hochR);
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int bTief = bandFuer (100.0);
        const int bHoch = bandFuer (5000.0);
        pruefe (bTief >= 0 && bHoch >= 0 && bTief != bHoch,
                "100 Hz und 5 kHz liegen in verschiedenen Baendern",
                "Band " + juce::String (bTief) + " gegen " + juce::String (bHoch));

        const auto& t100 = e.stereoBand (bTief);
        const auto& h5k  = e.stereoBand (bHoch);
        pruefe (t100.basisGesetzt && h5k.basisGesetzt,
                "beide tragen eine Basis");
        pruefe (t100.korrelationMittel > 0.9f,
                "unten ist das Material mono - Korrelation nahe 1",
                juce::String (t100.korrelationMittel, 4));
        pruefe (h5k.korrelationMittel < t100.korrelationMittel - 0.3f,
                "oben ist es dekorreliert - und GENAU DAS koennen zwei globale "
                "Skalare nicht ausdruecken",
                "unten " + juce::String (t100.korrelationMittel, 4) + ", oben "
                + juce::String (h5k.korrelationMittel, 4));
        pruefe (t100.seitenanteilDb < h5k.seitenanteilDb - 6.0f,
                "und der Seitenanteil trennt die beiden Baender ebenso",
                "unten " + juce::String (t100.seitenanteilDb, 2) + " dB, oben "
                + juce::String (h5k.seitenanteilDb, 2) + " dB");
    }

    // ── M-11: coherence_carries_window_and_dof ───────────────────────────
    if (abschnittWaehlt ("M-86", "M-11  coherence_carries_window_and_dof"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // Langsame Kadenz, damit sicher mehr als acht Frames zusammenkommen.
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        const int b1k = bandFuer (1000.0);
        const auto& w = e.stereoBand (b1k);
        pruefe (w.freiheitsgrade >= (std::uint32_t) nakama::analyse::kWelchMindestFrames,
                "das Band traegt seine Freiheitsgrade - die Zahl der gemittelten "
                "gueltigen Welch-Frames (§40.1: 'Fensterdauer und Freiheitsgrade "
                "werden Teil der Evidenz')",
                juce::String ((int) w.freiheitsgrade) + " Frames");
        pruefe (w.fensterDauerMs > 0.0f,
                "und seine Fensterdauer in Millisekunden",
                juce::String (w.fensterDauerMs, 1) + " ms");
        pruefe (w.kohaerenzGesetzt,
                "mit genug Frames traegt die Kohaerenz ein Bit");

        // Die Fensterdauer muss zur Zahl der Frames passen: sie ist ihre
        // Summe ueber den Hop. Eine Dauer, die nicht mitwaechst, waere eine
        // Konstante mit dem Namen einer Messung.
        const double hopMs = 1000.0 * (double) (FeatureEngine::kHauptPunkte / 2) / fs;
        const double erwartet = hopMs * (double) w.freiheitsgrade;
        pruefe (std::abs ((double) w.fensterDauerMs - erwartet) < 1.0,
                "und die Dauer ist die Summe der Hops ueber genau diese Frames",
                juce::String (w.fensterDauerMs, 2) + " ms gegen "
                + juce::String (erwartet, 2) + " ms");

        // NAK-380 M-86: dieselbe Zusage ueber den Ring aus W = 3 Fenstern,
        // am dritten Snapshot gemessen (Blocklaeufer, Kadenzmodell).
        nak380M86();
    }

    // ── M-11: coherence_is_null_below_eight_frames ───────────────────────
    //
    // Die schaerfste Stelle des fail-closed. Die Magnitude-Squared Coherence
    // aus EINEM Frame ist identisch 1, egal wie unkorreliert die Kanaele
    // sind - |L·conj(R)|² = |L|²·|R|² gilt fuer jedes Bin exakt. Ein
    // Erzeuger ohne Mindestframezahl meldete also volle Kohaerenz fuer
    // Rauschen.
    //
    // NAK-380 M-92 (b): gemessen wird der ERSTE Snapshot nach dem Start. Seit
    // dem Stereoring (T-380-6) ist nur er kurz: die folgenden tragen die
    // Frames der letzten drei Fenster. Bei 48 kHz, Block 512 und 0,25 s endet
    // das erste Fenster nach 3 * 5120 = 15 360 Samples, also
    // floor((15 360 - 4096)/2048) + 1 = 6 Frames. Die Grenze selbst steht
    // beiderseits in `nak380M92Grenze` (7 Frames ohne, 8 mit Bit).
    if (abschnittWaehlt ("M-92", "M-11  coherence_is_null_below_eight_frames"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // Schnellste Kadenz: 4 Hz, also 0,25 s Evidenzfenster; das erste
        // Fenster traegt 6 Hauptstufenframes - weniger als acht.
        e.evidenzIntervallSetzen (0.25);
        std::uint32_t lcg = 0x2468acef;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();                              // eigener Zug: unkorreliert
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        int mitBit = 0, ohneBitTrotzBasis = 0, unterAcht = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (! w.basisGesetzt) continue;
            if (w.freiheitsgrade < (std::uint32_t) nakama::analyse::kWelchMindestFrames)
            {
                ++unterAcht;
                if (w.kohaerenzGesetzt) ++mitBit; else ++ohneBitTrotzBasis;
            }
        }
        pruefe (unterAcht > 0,
                "es gibt wirklich Baender mit weniger als acht Frames - der Fall "
                "ist hergestellt, nicht ausgerechnet",
                juce::String (unterAcht) + " von "
                + juce::String (baenderMitBasis (e)) + " Baendern mit Basis");
        pruefe (mitBit == 0,
                "und KEINES davon traegt ein Kohaerenzbit - unter acht Frames ist "
                "die Kohaerenz `null`, nicht 1",
                juce::String (mitBit) + " mit Bit, "
                + juce::String (ohneBitTrotzBasis) + " ohne");
        int hauptFalsch = 0, haupt = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto bf = nak380Binfenster (b, fs);
            if (bf.bass || bf.bins() == 0) continue;
            ++haupt;
            if (e.stereoBand (b).freiheitsgrade != 6u) ++hauptFalsch;
        }
        pruefe (kam && haupt > 0 && hauptFalsch == 0,
                "380/M-92 coherence_is_null_below_eight_frames: der erste Snapshot nach dem Start "
                "traegt je Hauptstufenband 6 Frames (Kadenzmodell)",
                juce::String (hauptFalsch) + " abweichend von " + juce::String (haupt));
        nak380M92Grenze();
    }

    // ── M-11: phase_only_in_coherent_bands ───────────────────────────────
    if (abschnittWaehlt (nullptr, "M-11  phase_only_in_coherent_bands"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        // Ein kohaerenter Traeger bei 1 kHz plus unkorreliertes Rauschen je
        // Kanal. Das Traegerband wird kohaerent, die Rauschbaender nicht.
        //
        // ⚠️ Der Traeger liegt bei 1 kHz und nicht bei 120 Hz, und das ist
        // keine Willkuer: unter `kTrennungHz` ist die BASSSTUFE zustaendig,
        // und ihr Hop von 170,7 ms erreicht die acht Welch-Frames im ERSTEN
        // 1-s-Evidenzfenster nicht (5 Bassframes). Seit NAK-380 R-380-3
        // (T-380-6, NAK-177) erreicht sie sie ueber ihren Ring aus sieben
        // Evidenzfenstern ab dem zweiten 1-s-Snapshot (11 Frames, M-83);
        // dieser Fall misst aber den ersten Snapshot, und dort bliebe ein
        // Bassband fail-closed ohne Kohaerenz.
        std::uint32_t lcg = 0x13572468;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t n, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (double) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0);
            };
            const double traeger = 0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0);
            l = (float) (traeger + 0.12 * zug());
            r = (float) (traeger + 0.12 * zug());
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        int phaseOhneKohaerenz = 0, phaseUnterSchwelle = 0;
        int mitPhase = 0, mitKohaerenz = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (w.kohaerenzGesetzt) ++mitKohaerenz;
            if (w.phaseGesetzt)
            {
                ++mitPhase;
                if (! w.kohaerenzGesetzt) ++phaseOhneKohaerenz;
                else if ((double) w.kohaerenz <= nakama::analyse::kKohaerenzSchwellePhase)
                    ++phaseUnterSchwelle;
            }
        }
        pruefe (mitPhase > 0 && mitKohaerenz > mitPhase,
                "es gibt Baender MIT und OHNE Phase - der Fall trennt wirklich",
                juce::String (mitPhase) + " mit Phase, "
                + juce::String (mitKohaerenz) + " mit Kohaerenz");
        pruefe (phaseOhneKohaerenz == 0,
                "kein Band traegt eine Phase ohne Kohaerenz - die Phasenbits sind "
                "eine TEILMENGE der Kohaerenzbits (das kann das Schema nicht sagen)",
                juce::String (phaseOhneKohaerenz) + " Verletzungen");
        pruefe (phaseUnterSchwelle == 0,
                "und keines unterhalb der benannten Schwelle "
                + juce::String (nakama::analyse::kKohaerenzSchwellePhase, 2),
                juce::String (phaseUnterSchwelle) + " Verletzungen");
    }

    // ── M-11: die Phase gegen eine FORMEL, nicht gegen ein Vorzeichen ────
    //
    // Eine bekannte Laufzeit tau zwischen den Kanaelen erzeugt eine Phase,
    // die linear in der Frequenz ist: phi(f) = -2*pi*f*tau. Das ist die
    // schaerfste Probe des Beins - ein Erzeuger, der irgendeinen Winkel
    // liefert, faellt hier, und einer mit vertauschtem Vorzeichen auch.
    if (abschnittWaehlt (nullptr, "M-11  bekannte Laufzeit: die Phase folgt -2*pi*f*tau"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        constexpr int kVerzoegerung = 8;            // Samples, R hinter L
        const double tau = (double) kVerzoegerung / fs;

        // Breitbandig, damit viele Baender Energie haben - und deterministisch,
        // damit die Verzoegerung exakt ist.
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            auto welle = [] (double m)
            {
                const double t = m / 48000.0;
                return 0.20 * std::sin (kZweiPi * 300.0 * t)
                     + 0.20 * std::sin (kZweiPi * 900.0 * t + 0.3)
                     + 0.20 * std::sin (kZweiPi * 2000.0 * t + 1.1);
            };
            l = (float) welle ((double) n);
            r = (float) welle ((double) n >= (double) kVerzoegerung
                               ? (double) n - (double) kVerzoegerung : 0.0);
        });
        pruefe (kam, "ein Evidenzfenster entsteht");

        int geprueft = 0, daneben = 0;
        double groessteAbweichung = 0.0;
        for (const double hz : { 300.0, 900.0, 2000.0 })
        {
            const int b = bandFuer (hz);
            if (b < 0) continue;
            const auto& w = e.stereoBand (b);
            if (! w.phaseGesetzt) continue;
            ++geprueft;
            // R ist die VERZOEGERTE Haelfte, also eilt L vor: arg(L·conj(R))
            // ist positiv und gleich +2*pi*f*tau.
            double erwartet = kZweiPi * hz * tau;
            // Auf (-pi, pi] wickeln - dieselbe Wicklung, die der Erzeuger
            // ueber atan2 liefert.
            while (erwartet >  3.14159265358979) erwartet -= kZweiPi;
            while (erwartet <= -3.14159265358979) erwartet += kZweiPi;
            const double ist = (double) w.phaseRad;
            double diff = ist - erwartet;
            while (diff >  3.14159265358979) diff -= kZweiPi;
            while (diff <= -3.14159265358979) diff += kZweiPi;
            groessteAbweichung = std::max (groessteAbweichung, std::abs (diff));
            if (std::abs (diff) > 0.25)
                ++daneben;
        }
        pruefe (geprueft >= 2,
                "mindestens zwei der drei Traegerfrequenzen tragen eine Phase",
                juce::String (geprueft) + " von 3");
        pruefe (daneben == 0,
                "und jede folgt der Formel +2*pi*f*tau - die Phase ist gemessen, "
                "nicht geraten",
                "groesste Abweichung " + juce::String (groessteAbweichung, 4) + " rad");
    }

    // ── M-11: persistence_is_reported ────────────────────────────────────
    if (abschnittWaehlt (nullptr, "M-11  persistence_is_reported"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;
        });
        pruefe (kam, "ein Evidenzfenster entsteht");
        const auto& w = e.stereoBand (bandFuer (1000.0));
        pruefe (w.basisGesetzt && w.persistenz > 0.5f,
                "ein durchgehend kohaerentes Band meldet hohe Persistenz",
                juce::String (w.persistenz, 3));

        auto halter2 = std::make_unique<FeatureEngine>();
        auto& e2 = *halter2;
        e2.vorbereiten (fs);
        Speiser s2 { e2 };
        e2.evidenzIntervallSetzen (1.0);
        std::uint32_t lcg = 0x0f0f0f0f;
        const bool kam2 = s2.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();
        });
        const auto& w2 = e2.stereoBand (bandFuer (1000.0));
        pruefe (kam2 && w2.persistenz < 0.5f,
                "ein unkorreliertes Band meldet niedrige - die Zahl misst wirklich "
                "die Stabilitaet des Befundes",
                juce::String (w2.persistenz, 3));
    }

    // ── M-10: band_stereo_metrics_share_the_nyquist_cap ──────────────────
    if (abschnittWaehlt (nullptr, "M-10  band_stereo_metrics_share_the_nyquist_cap"))
    {
        // Bei 22,05 kHz Samplerate liegt die Kappe bei min(18 kHz,
        // 0,95·11,025 kHz) = 10,474 kHz. Jedes Band darueber darf KEINE
        // Stereometrik tragen - es entsteht keine zweite Kappenregel.
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (22050.0);
        Speiser s { e };
        s.sr = 22050.0;
        e.evidenzIntervallSetzen (1.0);
        std::uint32_t lcg = 0x77777777;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = l * 0.9f;
        });
        pruefe (kam, "ein Evidenzfenster bei 22,05 kHz entsteht");

        const double kappe = std::min (18000.0, 0.95 * 22050.0 * 0.5);
        int ueberKappeMitWert = 0, ueberKappe = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            if (Gitter::evidenzKante (b) < kappe)
                continue;                           // Band beginnt unter der Kappe
            ++ueberKappe;
            const auto& w = e.stereoBand (b);
            if (w.basisGesetzt || w.kohaerenzGesetzt || w.phaseGesetzt)
                ++ueberKappeMitWert;
        }
        pruefe (ueberKappe > 0,
                "es gibt Baender ueber der Kappe - der Fall ist hergestellt",
                juce::String (ueberKappe) + " Baender ab "
                + juce::String (kappe / 1000.0, 2) + " kHz");
        pruefe (ueberKappeMitWert == 0,
                "und KEINES traegt eine Stereometrik - die neuen Bandmetriken erben "
                "die Nyquist-Kappe, es entsteht keine zweite Regel",
                juce::String (ueberKappeMitWert) + " Verletzungen");
    }

    // ── M-12: width_alone_is_never_a_defect ──────────────────────────────
    //
    // §40.2: "Breite ist kein Qualitaetswert." Das ist eine Zusage ueber das,
    // was die Engine NICHT tut - sie liefert Messwerte und kein Urteil. Der
    // Fall misst es an der Fläche: es gibt kein Feld, das ein Urteil traegt.
    if (abschnittWaehlt (nullptr, "M-12  width_alone_is_never_a_defect / low_coherence_yields_no_recommendation"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        std::uint32_t lcg = 0x9abcdef0;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();                              // sehr breit, sehr inkohaerent
        });
        pruefe (kam, "ein sehr breites, inkohaerentes Evidenzfenster entsteht");

        // Die Zusage in Feldern: ein `StereoBandwert` traegt Messwerte und
        // Gueltigkeitsbits - keinen Befund, keine Empfehlung, keinen Lag.
        // Waere hier je ein Urteilsfeld, faellt dieser Fall beim Kompilieren
        // (der Test kennt die Struktur) oder spaetestens beim Lesen.
        int mitPhase = 0, breiteBaender = 0;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (! w.basisGesetzt) continue;
            if (w.seitenanteilDb > -6.0f) ++breiteBaender;
            if (w.phaseGesetzt) ++mitPhase;
        }
        pruefe (breiteBaender > 0,
                "viele Baender sind maximal breit",
                juce::String (breiteBaender) + " von "
                + juce::String (baenderMitBasis (e)));
        pruefe (mitPhase == 0,
                "und trotzdem entsteht in KEINEM eine Phase - bei niedriger "
                "Kohaerenz gibt es keine Lag- oder Polaritaetsaussage (§40.2), "
                "und Breite allein ist kein Befund",
                juce::String (mitPhase) + " Baender mit Phase");
    }

    // ── §40.3: Polaritaetsinvertierung ───────────────────────────────────
    if (abschnittWaehlt ("M-90", "§40.3  Polaritaetsinvertierung: Korrelation -1, Monosumme loescht aus"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = -l;
        });
        pruefe (kam, "ein Evidenzfenster entsteht");
        const auto& w = e.stereoBand (bandFuer (1000.0));
        pruefe (w.basisGesetzt && w.korrelationMittel < -0.999f,
                "die Korrelation ist -1",
                juce::String (w.korrelationMittel, 6));
        pruefe (w.kohaerenzGesetzt && w.kohaerenz > 0.99f,
                "die KOHAERENZ ist dabei 1 - sie misst den Zusammenhang, nicht das "
                "Vorzeichen. Wer beide verwechselt, meldet Antiphase als Rauschen",
                juce::String (w.kohaerenz, 6));
        pruefe (w.phaseGesetzt && std::abs (std::abs ((double) w.phaseRad)
                                            - 3.14159265358979) < 0.05,
                "und die Phase ist +/-pi - genau die Aussage 'invertiert'",
                juce::String (w.phaseRad, 4) + " rad");
        const auto& sk = e.stereoSkalare();
        pruefe (sk.folddownGesetzt && sk.monoFolddownDb < -300.0f,
                "die Monosumme loescht sich VOLLSTAENDIG aus - der Wert laeuft an die "
                "Vertragsgrenze, statt zu fehlen. Gemessen am wirklich gefalteten "
                "Puffer, nicht aus der Korrelation geschaetzt: das ist die schaerfste "
                "Stereoaussage, die es gibt, und sie darf nicht schweigen",
                juce::String (sk.monoFolddownDb, 1) + " dB");
    }

    // ── §32.3: keine Stereogroesse ueberbrueckt eine Grenze ──────────────
    //
    // ⚠️ Dieser Abschnitt ist die Wache, die G13 NICHT leisten kann: die
    // Zwillingsprobe vergleicht `FeatureFrame`-Objekte, und die
    // Stereoevidenz liegt bewusst daneben (Begruendung bei
    // `StereoBandwert`). Ohne diesen Fall waere sie der einzige Traeger des
    // Tickets ohne Grenzwache.
    if (abschnittWaehlt ("M-87", "§32.3  keine Stereogroesse ueberbrueckt eine Transportgrenze"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);

        // Vorher: stark korreliertes, lautes Material.
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.5 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = l;
        });
        pruefe (kam, "vor der Grenze steht ein Evidenzfenster");
        const int b1k = bandFuer (1000.0);
        const auto vorher = e.stereoBand (b1k);
        const auto skVorher = e.stereoSkalare();
        pruefe (vorher.basisGesetzt && vorher.korrelationMittel > 0.99f
                  && skVorher.folddownGesetzt,
                "mit hoher Korrelation und gemessenem Folddown",
                "Korr " + juce::String (vorher.korrelationMittel, 4) + ", Folddown "
                + juce::String (skVorher.monoFolddownDb, 3) + " dB");

        // Die Grenze, danach dekorreliertes Material.
        s.strom += 65536;
        std::uint32_t lcg = 0x5a5a5a5a;
        const bool kam2 = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = zug();
        });
        pruefe (kam2, "nach der Grenze entsteht ein neues Evidenzfenster");
        const auto& nachher = e.stereoBand (b1k);
        pruefe (! nachher.basisGesetzt || nachher.korrelationMittel < 0.8f,
                "und seine Korrelation ist die des NEUEN Materials - kein "
                "Kreuzspektrum hat die Grenze ueberbrueckt",
                nachher.basisGesetzt
                    ? juce::String (nachher.korrelationMittel, 4)
                    : juce::String ("kein Bit"));
        const auto& skNachher = e.stereoSkalare();
        pruefe (! skNachher.folddownGesetzt
                  || std::abs (skNachher.monoFolddownDb - skVorher.monoFolddownDb) > 1.0f,
                "auch der Folddown ist neu gemessen und nicht fortgeschrieben",
                "vorher " + juce::String (skVorher.monoFolddownDb, 3) + " dB, nachher "
                + juce::String (skNachher.monoFolddownDb, 3) + " dB");

        // NAK-380 M-87 (Regressionswache, erweitert): auch der Stereoring
        // ueberbrueckt die Grenze nicht. Die Kadenz laeuft ueber die Grenze
        // weiter (§10.1), das Fenster davor endete mit seinem Snapshot, also
        // faellt der naechste nach 10 * 5120 = 51 200 Samples und traegt
        // floor((51 200 - 4096)/2048) + 1 = 24 Frames; ein ueberlebender Ring
        // truege 24 + 24 = 48, und das Kreuzspektrum des Sinus davor hielte
        // die Kohaerenz hoch. Unkorreliertes Rauschen aus 24 Frames hat je Bin
        // den Erwartungswert 1/24 = 0,042; P(MSC > 0,5) = 0,5^23 = 1,2e-7 je Bin.
        pruefe (kam2 && nachher.freiheitsgrade == 24u,
                "380/M-87 ring_leert (§32.3): der erste Snapshot nach der Grenze traegt nur die 24 Frames "
                "seit der Grenze",
                juce::String ((int) nachher.freiheitsgrade) + " Frames");
        pruefe (kam2 && (! nachher.kohaerenzGesetzt || nachher.kohaerenz < 0.5f),
                "380/M-87 ring_leert (§32.3): und seine Kohaerenz ist die des neuen, unkorrelierten Materials",
                nachher.kohaerenzGesetzt ? juce::String (nachher.kohaerenz, 4) : juce::String ("kein Bit"));
    }

    std::cout << "\n-----------------------------------------" << std::endl;

    // ═══════════════════════════════════════════════════════════════════
    // NAK-181 R5 · Praesenzbits fuer die Stereofelder (G4-Befund V06)
    // ═══════════════════════════════════════════════════════════════════
    //
    // ⚠️ Alle bestehenden Faelle dieses Beins fahren `evidenzIntervallSetzen(1.0)`
    // — genau deshalb hat keiner den Defekt gesehen. Bei der VORGABEkadenz
    // 0,25 s schliesst das Kurzfenster nie (12000 Samples / 2048 = 5,86 Hops,
    // Schwelle 8), und `korrelation_kurz` wie `persistenz` reisten mit dem
    // Bit von `basisGesetzt` als gemessene 0,0.
    if (abschnittWaehlt (nullptr, "NAK-181 N-21  kurzkorrelation_und_persistenz_schweigen_ohne_kurzfenster"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        // KEIN evidenzIntervallSetzen: die Vorgabe ist 0,25 s.
        pruefe (std::abs (e.evidenzIntervallJetzt() - 0.25) < 1e-9,
                "N-21: der Fall faehrt die VORGABEkadenz",
                juce::String (e.evidenzIntervallJetzt(), 3));
        std::uint32_t lcg = 0x13572468;
        const bool kam = s.bisEvidenz ([&lcg] (std::uint64_t, float& l, float& r)
        {
            auto zug = [&lcg]
            {
                lcg = lcg * 1664525u + 1013904223u;
                return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
            };
            l = zug();
            r = l * 0.8f + zug() * 0.2f;      // korreliert, aber nicht identisch
        });
        pruefe (kam, "N-21: ein Evidenzfenster entsteht");

        int mitBasis = 0, mitKurz = 0, mitPersistenz = 0;
        for (int b = 0; b < nakama::analyse::Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (w.basisGesetzt) ++mitBasis;
            if (w.korrelationKurzGesetzt) ++mitKurz;
            if (w.persistenzGesetzt) ++mitPersistenz;
        }
        pruefe (mitBasis > 0, "N-21: Baender mit Basis gibt es", juce::String (mitBasis));
        pruefe (mitKurz == 0,
                "N-21: aber KEIN Band traegt korrelation_kurz - das Kurzfenster "
                "schliesst bei 0,25 s nie",
                juce::String (mitKurz));
        pruefe (mitPersistenz == 0,
                "N-21: und keines persistenz", juce::String (mitPersistenz));
    }

    if (abschnittWaehlt (nullptr, "NAK-181 N-22  praesenzbits_ueberleben_das_fensterleeren"))
    {
        // Bei 1 s schliessen 23 Hops (48 kHz) beziehungsweise 46 (96 kHz)
        // Kurzfenster - die Bits werden gesetzt UND muessen den Leser erreichen.
        // `baueFrame` ruft `evidenzLeeren` -> `stereoLeeren` in sich selbst,
        // BEVOR der Writer `stereoBand()` liest; ein Bit, das dort genullt
        // wuerde, waere in jedem Snapshot schon weg.
        for (const double sr : { 48000.0, 96000.0 })
        {
            auto halter = std::make_unique<FeatureEngine>();
            auto& e = *halter;
            e.vorbereiten (sr);
            Speiser s { e };
            e.evidenzIntervallSetzen (1.0);
            const bool kam = s.bisEvidenz ([sr] (std::uint64_t n, float& l, float& r)
            {
                l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / sr));
                r = l;
            });
            pruefe (kam, juce::String ("N-22: ein Evidenzfenster bei ")
                         + juce::String (sr / 1000.0, 0) + " kHz");
            const auto& w = e.stereoBand (bandFuer (1000.0));
            pruefe (w.korrelationKurzGesetzt && w.persistenzGesetzt,
                    juce::String ("N-22: beide Bits stehen NACH dem Fensterleeren (")
                    + juce::String (sr / 1000.0, 0) + " kHz)",
                    juce::String (w.persistenz, 3));
            pruefe (w.persistenz > 0.5f,
                    "N-22: und die Persistenz traegt ihren Wert",
                    juce::String (w.persistenz, 3));
        }
    }

    if (abschnittWaehlt (nullptr, "NAK-181 N-23  stiller_kanal_laesst_alle_drei_bits_weg"))
    {
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);        // die Kurzfenster SCHLIESSEN
        const bool kam = s.bisEvidenz ([] (std::uint64_t n, float& l, float& r)
        {
            l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
            r = 0.0f;                          // ein stiller Kanal
        });
        pruefe (kam, "N-23: ein Evidenzfenster entsteht");
        const auto& w = e.stereoBand (bandFuer (1000.0));
        // Das globale `stereoKurzfenster` ist hier GROESSER null - genau
        // deshalb reicht es als Bedingung nicht: dieses Band hatte in keinem
        // der Fenster einen gueltigen Nenner.
        pruefe (w.basisGesetzt, "N-23: die Basis steht (ein stiller Kanal ist maximal breit)");
        pruefe (! w.korrelationMittelGesetzt,
                "N-23: korrelation_mittel traegt KEIN Bit - der Nenner ist null "
                "(M-08: bei stillem Kanal faellt das Praesenzbit weg)");
        pruefe (! w.korrelationKurzGesetzt,
                "N-23: korrelation_kurz ebenso");
        pruefe (! w.persistenzGesetzt,
                "N-23: und persistenz ebenso - obwohl Kurzfenster geschlossen "
                "haben, hatte DIESES Band in keinem einen gueltigen Nenner");
    }
    // ── NAK-182 M-82: der Blockgroessen-Sweep (R3/E3, N-15 bis N-20) ─────
    //
    // M-82 verlangt die fuenf Klassen aus §40.3 "ueber alle unterstuetzten
    // Blockgroessen".  Die Abschnitte oben messen jede Klasse TIEF, aber bei
    // einer einzigen Blockgroesse (`Speiser::frames` = 512).  Dieser Sweep
    // misst dieselbe analytisch bekannte Antwort ueber die Blockgroessenmenge
    // - flacher, dafuer auf der Achse, die M-82 nennt.
    //
    // DIE MENGE steht genau EINMAL, oben im Namensraum (`kSweepBlockgroessen`,
    // seit NAK-380 Etappe 5 dort, weil M-74 dieselbe Menge faehrt):
    // dieselben 18 Groessen, die
    // B4 `EqCopQueueStressTest` §J fuer den Passthrough faehrt.  Ihre
    // Obergrenze ist die Slotkapazitaet des Layouts (`GenStrom =
    // StreamLayout<1, 131072, 2048, 16384>`, `core/StampedAudioQueue.h`): ein
    // groesserer Hostblock wird als GANZES nur fuer die Analyse verworfen.
    //
    // ⚠️ Sie ist eine AUSWAHL, keine Erschoepfung.  "Alle unterstuetzten"
    // waeren 16384 Werte; 256 zum Beispiel bleibt ungemessen.  Die
    // Bein-Behauptung in `tools/beweise.ps1` nennt deshalb die 18 Werte
    // einzeln und sagt nie "ueber alle" (Manifest §2.5).  Die vier Anker der
    // Auswahl:
    //
    //       1  die kleinste; FL zerteilt Puffer bis auf 1 Sample (S4)
    //     333  NICHTteiler des Hauptfensterhops 2048 und teilerfremd zur
    //          Rahmenkadenz - die Phasenfalle aus `AnalysisGoldenTestMain`
    //     512  der Wert der Abschnitte oben, damit der Sweep sie reproduziert
    //    2048  GENAU der Hauptfensterhop: Block- und Fenstergrenze fallen
    //          zusammen
    //   16384  die groesste; zugleich `kBassPunkte` und die Slotkapazitaet
    //
    // ⚠️ DIE FALLE, an der ein naiver Sweep still gruen waere: `bisEvidenz`
    // zaehlt BLOECKE, nicht Samples.  Bei Blockgroesse 1 waeren 900 Bloecke
    // 900 Samples - kein Evidenzfenster, `kam == false`, und ohne Pruefung
    // von `kam` haette der Fall nichts gemessen und trotzdem bestanden.  Die
    // Schranke wird deshalb auf dieselbe SAMPLEZAHL gerechnet wie bei 512,
    // und `kam` ist je Lauf eine eigene Zusage.
    //
    // ⚠️ Jedes Material ist eine reine Funktion der ABSOLUTEN Stromposition
    // `n`.  Nur so ist der Sweep ueberhaupt eine Aussage ueber Blockgroessen:
    // ein Signal, das aus einem Blockindex entstuende, waere je Blockgroesse
    // ein anderes Signal.
    if (abschnittWaehlt ("M-89 M-90 M-91", "NAK-182 M-82  Blockgroessen-Sweep: fuenf Klassen und Folddown"))
    {
        const juce::String mengenText ("1, 2, 3, 7, 15, 16, 31, 64, 127, 128, "
                                       "333, 512, 1024, 2048, 4096, 8192, 12345, 16384");

        // Deterministisches Breitbandrauschen als reine Funktion von (n, Saat).
        //
        // ⚠️ Der erste Versuch nahm `(n * 2654435761) ^ saat` mit einem
        // xorshift32 darueber.  Zwei Saaten ergaben damit KEINE unabhaengigen
        // Stroeme: die Bandkorrelation lag bei 0,93 statt nahe null, und
        // `sweep_uncorrelated_channels` fiel zu Recht.  Die Saat muss VOR der
        // Durchmischung in den Zustand, nicht danach - deshalb der
        // splitmix64-Abschluss.
        const auto rausch = [] (std::uint64_t n, std::uint64_t saat) -> double
        {
            std::uint64_t x = n * 0x9E3779B97F4A7C15ull + saat;
            x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ull;
            x ^= x >> 27; x *= 0x94D049BB133111EBull;
            x ^= x >> 31;
            return (double) (x >> 11) / 4503599627370495.5 - 1.0;
        };

        /*  Ein TRAEGER: das Band, in dem eine Frequenz liegt, mit seinen
            Bandwerten.  NAK-182 Nacharbeit 1 (EP-02/NR-02): der Sweep speiste
            drei Traeger ein und wertete nur einen aus - die Matrixzeile N-18
            verlangt alle drei.  Ein Sample Versatz ergibt bei 2000 Hz
            0,2618 rad; genau der Fall blieb bei 900 Hz gruen. */
        struct Traeger
        {
            double hz { 0.0 };
            bool basis { false }, kohGesetzt { false }, phaseGesetzt { false };
            bool korrGesetzt { false };
            float korrelation { 0.0f }, kohaerenz { 0.0f }, phase { 0.0f };
            std::uint32_t dof { 0 };
        };

        struct Lauf
        {
            bool kam { false };
            bool basis { false }, kohGesetzt { false }, phaseGesetzt { false };
            bool korrGesetzt { false }, folddownGesetzt { false };
            float korrelation { 0.0f }, kohaerenz { 0.0f }, phase { 0.0f };
            float folddown { 0.0f };
            std::uint32_t dof { 0 };
            double handFolddownDb { 0.0 };
            std::vector<Traeger> traeger;
            /*  NAK-182 Nacharbeit 1 (EP-03/NR-03): dieselben zwei Zahlen, die
                der 512er-Abschnitt fuer M-12 erhebt - Baender mit Basis und
                davon welche mit Phase.  "Keine Lag- oder Polaritaetsempfehlung"
                ist genau `phaseGesetzt == false`, und ohne diese Zahl haette
                eine faelschlich vorhandene Phase bei niedriger Korrelation als
                richtige Antwort bestanden. */
            int baenderMitBasis { 0 }, baenderMitPhase { 0 };
            /*  NAK-380 M-91: Mittel und Maximum der Bandkohaerenz ueber ALLE
                Baender mit Kohaerenzbit (erster 1-s-Snapshot). */
            Nak380KohMittel kohMittel;
        };

        // Ein Lauf je Blockgroesse.  Die Engine liegt auf dem HEAP - der
        // MSVC-Standardstack ist 1 MiB, eine `FeatureEngine` rund 0,5 MB
        // (Register NAK-175).
        const auto fahre = [&] (int frames, const std::vector<double>& hzListe,
                                const std::function<void (std::uint64_t, float&, float&)>& f,
                                bool mitHandfaltung) -> Lauf
        {
            auto halter = std::make_unique<FeatureEngine>();
            auto& e = *halter;
            e.vorbereiten (fs);
            e.evidenzIntervallSetzen (1.0);
            Speiser s { e };
            s.frames = frames;
            const long long noetig = (900LL * 512LL) / (long long) frames;
            const int hoechstens = (int) std::max<long long> (900LL, noetig);

            Lauf L {};
            double monoEnergie = 0.0, stereoEnergie = 0.0;
            L.kam = s.bisEvidenz ([&] (std::uint64_t n, float& l, float& r)
            {
                f (n, l, r);
                if (mitHandfaltung)
                {
                    const double mono = 0.5 * ((double) l + (double) r);
                    monoEnergie   += mono * mono;
                    stereoEnergie += 0.5 * ((double) l * (double) l
                                            + (double) r * (double) r);
                }
            }, hoechstens);
            if (! L.kam)
                return L;

            for (double hz : hzListe)
            {
                Traeger t;
                t.hz = hz;
                const int b = bandFuer (hz);
                if (b >= 0)
                {
                    const auto& w = e.stereoBand (b);
                    t.basis       = w.basisGesetzt;
                    t.korrGesetzt = w.korrelationMittelGesetzt;
                    t.korrelation = w.korrelationMittel;
                    t.kohGesetzt  = w.kohaerenzGesetzt;
                    t.kohaerenz   = w.kohaerenz;
                    t.phaseGesetzt = w.phaseGesetzt;
                    t.phase       = w.phaseRad;
                    t.dof         = w.freiheitsgrade;
                }
                L.traeger.push_back (t);
            }
            if (! L.traeger.empty())
            {
                // Die flachen Felder sind der ERSTE Traeger - die fuenf
                // uebrigen Klassen messen genau ihn, unveraendert.
                const auto& t = L.traeger.front();
                L.basis        = t.basis;
                L.korrGesetzt  = t.korrGesetzt;
                L.korrelation  = t.korrelation;
                L.kohGesetzt   = t.kohGesetzt;
                L.kohaerenz    = t.kohaerenz;
                L.phaseGesetzt = t.phaseGesetzt;
                L.phase        = t.phase;
                L.dof          = t.dof;
            }
            for (int b = 0; b < Gitter::evidenzBaender; ++b)
            {
                const auto& w = e.stereoBand (b);
                if (! w.basisGesetzt) continue;
                ++L.baenderMitBasis;
                if (w.phaseGesetzt) ++L.baenderMitPhase;
            }
            L.kohMittel = nak380KohMittel (e);
            const auto& sk = e.stereoSkalare();
            L.folddownGesetzt = sk.folddownGesetzt;
            L.folddown        = sk.monoFolddownDb;
            if (mitHandfaltung && stereoEnergie > 0.0 && monoEnergie > 0.0)
                L.handFolddownDb = 10.0 * std::log10 (monoEnergie / stereoEnergie);
            return L;
        };

        // Jede Klasse: fuenf Laeufe, dann EINE Zusage ueber alle fuenf.
        const auto klasse = [&] (const char* bezeichner, const std::vector<double>& hzListe,
                                 const std::function<void (std::uint64_t, float&, float&)>& f,
                                 const std::function<bool (const Lauf&)>& antwortStimmt,
                                 const std::function<juce::String (const Lauf&)>& zeigen,
                                 bool mitHandfaltung,
                                 const std::function<void (int, const Lauf&)>& zusatz = {})
        {
            Lauf laeufe[kSweepN];
            int kamAlle = 0, richtig = 0;
            juce::String bericht;
            for (int i = 0; i < kSweepN; ++i)
            {
                laeufe[i] = fahre (kSweepBlockgroessen[i], hzListe, f, mitHandfaltung);
                if (laeufe[i].kam) ++kamAlle;
                if (laeufe[i].kam && antwortStimmt (laeufe[i])) ++richtig;
                bericht << (i ? ", " : "") << kSweepBlockgroessen[i] << ":"
                        << zeigen (laeufe[i]);
            }
            const juce::String kopf (bezeichner);
            // 1. Jeder Lauf hat wirklich gemessen.
            pruefe (kamAlle == kSweepN,
                    kopf + ": bei jeder Blockgroesse aus {" + mengenText
                        + "} entsteht ein Evidenzfenster",
                    juce::String (kamAlle) + " von " + juce::String (kSweepN));
            // 2. Die analytisch bekannte Antwort steht bei JEDER Blockgroesse.
            pruefe (richtig == kSweepN,
                    kopf + ": und jede traegt dieselbe analytisch bekannte Antwort",
                    bericht);
            // 3. Kein Lauf besteht, weil zu wenig Material floss: die
            //    Freiheitsgrade des Bandes sind ueber die Menge vergleichbar.
            std::uint32_t sortiert[kSweepN];
            for (int i = 0; i < kSweepN; ++i) sortiert[i] = laeufe[i].dof;
            std::sort (sortiert, sortiert + kSweepN);
            const double median = (double) sortiert[kSweepN / 2];
            bool dofOk = median > 0.0;
            juce::String dofText;
            for (int i = 0; i < kSweepN; ++i)
            {
                dofText << (i ? ", " : "") << (int) laeufe[i].dof;
                if (median > 0.0
                    && std::abs ((double) laeufe[i].dof - median) > 0.2 * median)
                    dofOk = false;
            }
            pruefe (dofOk,
                    kopf + ": je Blockgroesse wurden vergleichbar viele Fenster "
                           "gemittelt (Freiheitsgrade innerhalb 20 % des Medians)",
                    dofText + " (Median " + juce::String (median, 0) + ")");
            // 4. NAK-380 Etappe 5: Zusagen einer Klasse je Blockgroesse.
            if (zusatz)
                for (int i = 0; i < kSweepN; ++i)
                    zusatz (kSweepBlockgroessen[i], laeufe[i]);
        };

        // ── sweep_mono_identity ──────────────────────────────────────────
        if (nak380Waehlt ("M-89"))
        klasse ("sweep_mono_identity", { 1000.0 },
                [] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
                    r = l;
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation > 0.999f
                        && L.kohGesetzt && L.kohaerenz > 0.99f
                        && L.phaseGesetzt && std::abs (L.phase) < 0.01f
                        && L.folddownGesetzt && std::abs (L.folddown) < 0.25f;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.kohaerenz, 4) + "/"
                         + juce::String (L.phase, 4) + "/"
                         + juce::String (L.folddown, 3);
                }, false);

        // ── sweep_identical_stereo ───────────────────────────────────────
        // Breitbandiges Material, auf beiden Kanaelen BITGLEICH: dieselbe
        // Antwort wie Mono, aber aus einem Signal, das ohne die Gleichheit
        // breit waere.
        if (nak380Nur == nullptr)
        klasse ("sweep_identical_stereo", { 1000.0 },
                [&] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.35 * rausch (n, 0x51ED2701A17B93C5ull));
                    r = l;
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation > 0.999f
                        && L.kohGesetzt && L.kohaerenz > 0.99f
                        && L.phaseGesetzt && std::abs (L.phase) < 0.01f
                        && L.folddownGesetzt && std::abs (L.folddown) < 0.25f;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.kohaerenz, 4) + "/"
                         + juce::String (L.phase, 4) + "/"
                         + juce::String (L.folddown, 3);
                }, false);

        // ── sweep_polarity_inversion ─────────────────────────────────────
        // Korrelation -1 bei Kohaerenz 1, Phase +/-pi, und die Monosumme
        // laeuft an die Vertragsgrenze statt zu schweigen (§40.3).
        if (nak380Waehlt ("M-90"))
        klasse ("sweep_polarity_inversion", { 1000.0 },
                [] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
                    r = -l;
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation < -0.999f
                        && L.kohGesetzt && L.kohaerenz > 0.99f
                        && L.phaseGesetzt
                        && std::abs (std::abs ((double) L.phase) - 3.14159265358979) < 0.05
                        && L.folddownGesetzt && L.folddown < -300.0f;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.kohaerenz, 4) + "/"
                         + juce::String (L.phase, 3) + "/"
                         + juce::String (L.folddown, 0);
                }, false);

        // ── sweep_known_delay ────────────────────────────────────────────
        // R ist um 8 Samples verzoegert, also eilt L vor: die Phase folgt
        // +2*pi*f*tau.  Gemessen an DREI Traegern wie im Abschnitt oben, mit
        // derselben Toleranz von 0,25 rad - die Phase ist ein BANDwert, und
        // der Traeger sitzt in einem Band endlicher Breite.
        //
        // 🔑 NAK-182 Nacharbeit 1 (EP-02/NR-02): bis hierher speiste die Zeile
        // drei Traeger ein und wertete NUR 900 Hz aus.  N-18 verlangt drei je
        // Blockgroesse, und der Unterschied ist messbar: ein Sample Versatz
        // sind bei 900 Hz 0,1178 rad und bei 2000 Hz 0,2618 rad - der
        // Rotbeweis M-82 fiel deshalb an dieser Zeile NICHT.  Die eingespeiste
        // Welle und die ausgewerteten Baender kommen ab jetzt aus DERSELBEN
        // Liste; ein vierter Traeger im Signal ohne Auswertung ist damit
        // ausgeschlossen.
        if (nak380Nur == nullptr)
        {
            constexpr int kVerzoegerung = 8;
            // EINE Zahl fuer Test und Matrixzeile N-18 (Begruendung: Manifest
            // Paragraph 6.4 Nr. 6 - die Phase ist ein Bandwert).
            constexpr double kPhaseToleranzRad = 0.25;
            const double tau = (double) kVerzoegerung / fs;
            // EINE Quelle fuer Signal und Auswertung: Frequenz und Startphase
            // stehen zusammen, und die ausgewertete Liste wird daraus
            // abgeleitet.  Zwei getrennte Listen koennten auseinanderlaufen -
            // genau das war der Defekt.
            struct Quelle { double hz, startphase; };
            const Quelle quellen[] = { { 300.0, 0.0 }, { 900.0, 0.3 }, { 2000.0, 1.1 } };
            std::vector<double> traeger;
            for (const auto& q : quellen)
                traeger.push_back (q.hz);
            const auto welle = [&quellen] (double m)
            {
                const double t = m / 48000.0;
                double s = 0.0;
                for (const auto& q : quellen)
                    s += 0.20 * std::sin (kZweiPi * q.hz * t + q.startphase);
                return s;
            };
            const auto speise = [&] (std::uint64_t n, float& l, float& r)
            {
                l = (float) welle ((double) n);
                r = (float) welle ((double) n >= (double) kVerzoegerung
                                   ? (double) n - (double) kVerzoegerung : 0.0);
            };
            const auto phasenfehler = [&] (double hz, float gemessen)
            {
                double erwartet = kZweiPi * hz * tau;
                while (erwartet >  3.14159265358979) erwartet -= kZweiPi;
                while (erwartet <= -3.14159265358979) erwartet += kZweiPi;
                double diff = (double) gemessen - erwartet;
                while (diff >  3.14159265358979) diff -= kZweiPi;
                while (diff <= -3.14159265358979) diff += kZweiPi;
                return std::abs (diff);
            };
            klasse ("sweep_known_delay", traeger, speise,
                    [&] (const Lauf& L)
                    {
                        if (L.traeger.size() != traeger.size())
                            return false;
                        for (const auto& t : L.traeger)
                            if (! t.basis || ! t.phaseGesetzt
                                || phasenfehler (t.hz, t.phase) > kPhaseToleranzRad)
                                return false;
                        return true;
                    },
                    [&] (const Lauf& L)
                    {
                        juce::String s;
                        for (const auto& t : L.traeger)
                            s << (s.isEmpty() ? "" : "|") << juce::String (t.hz, 0) << "Hz "
                              << juce::String (t.phase, 4) << "(d="
                              << juce::String (phasenfehler (t.hz, t.phase), 4) << ")";
                        return s;
                    }, false);
        }

        // ── sweep_uncorrelated_channels ──────────────────────────────────
        // Zwei unabhaengige Rauschstroeme: die Korrelation faellt weit unter
        // die Schwelle, und die Kohaerenz traegt keine Empfehlung (M-12).
        //
        // 🔑 NAK-182 Nacharbeit 1 (EP-03/NR-03): N-19 verlangt neben der
        // niedrigen Korrelation ausdruecklich "keine Lag- oder
        // Polaritaetsempfehlung".  Bis hierher las das Praedikat nur Basis,
        // Korrelationsbit und Korrelation - eine faelschlich vorhandene Phase
        // waere bei niedriger Korrelation als richtige Antwort durchgegangen.
        // Gemessen wird ab jetzt mit DENSELBEN Feldern wie der 512er-Abschnitt
        // M-12 (`width_alone_is_never_a_defect`): kein Band mit Basis traegt
        // eine Phase.  Die Phase ist Stufe 2 des fail-closed - ohne sie gibt es
        // weder Lag- noch Polaritaetsaussage (Paragraph 40.2).
        //
        // NAK-380 M-91 (Etappe 5): dazu je Blockgroesse Mittel und Maximum der
        // Bandkohaerenz ueber ALLE Baender mit Kohaerenzbit. Referenz: die
        // Welch-MSC unabhaengiger Kanaele aus K Frames hat den Erwartungswert
        // 1/K und P(C > x) = (1 - x)^(K - 1). Im ersten 1-s-Snapshot traegt
        // jedes Hauptstufenband 22 bis 24 Frames (die Bassbaender erreichen
        // acht erst ab dem zweiten Snapshot, M-83): Erwartung hoechstens
        // 1/22 = 0,046, Schranke fuer das Mittel 0,15; P(C > 0,7) = 0,3^21 =
        // 1,0e-11 je Band, Schranke fuer das Maximum 0,7. Eine feste Schranke
        // 0,2 je Band traefe jedes Einbinband mit 0,8^21 = 0,9 %.
        if (nak380Waehlt ("M-91"))
        klasse ("sweep_uncorrelated_channels", { 1000.0 },
                [&] (std::uint64_t n, float& l, float& r)
                {
                    l = (float) (0.35 * rausch (n, 0x1234ABCD5678EF01ull));
                    r = (float) (0.35 * rausch (n, 0xC0FFEE1234567890ull));
                },
                [] (const Lauf& L)
                {
                    return L.basis && L.korrGesetzt && L.korrelation < 0.2f
                        && L.baenderMitBasis > 0 && L.baenderMitPhase == 0;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.korrelation, 4) + "/"
                         + juce::String (L.baenderMitPhase) + " von "
                         + juce::String (L.baenderMitBasis) + " mit Phase";
                },
                false,
                [] (int block, const Lauf& L)
                {
                    const auto& m = L.kohMittel;
                    const juce::String kopf = "380/M-91 sweep_uncorrelated_channels B=" + juce::String (block);
                    const juce::String zahlen = juce::String (m.baender) + " Baender mit Bit, K "
                        + juce::String ((int) m.kMin) + " bis " + juce::String ((int) m.kMax) + " (1/K bis "
                        + juce::String (m.kMin > 0u ? 1.0 / (double) m.kMin : 0.0, 4) + "), Mittel "
                        + juce::String (m.mittel, 4) + ", Maximum " + juce::String (m.maximum, 4)
                        + " (Band " + juce::String (m.maxBand) + ")";
                    pruefe (L.kam && m.baender > 0 && m.mittel <= 0.15,
                            kopf + ": Mittel der Bandkohaerenz ueber alle Baender mit Kohaerenzbit <= 0,15",
                            zahlen);
                    pruefe (L.kam && m.baender > 0 && m.maximum <= 0.7,
                            kopf + ": kein Band mit Kohaerenzbit ueber 0,7", zahlen);
                });

        // ── sweep_folddown_within_0p25db ─────────────────────────────────
        // Der gemeldete Monoverlust gegen den WIRKLICH gefalteten Puffer,
        // je Blockgroesse, innerhalb 0,25 dB (§40.3).  Die Handrechnung
        // laeuft ueber dieselben Samples, die die Engine gesehen hat.
        if (nak380Nur == nullptr)
        klasse ("sweep_folddown_within_0p25db", { 700.0 },
                [] (std::uint64_t n, float& l, float& r)
                {
                    const double t = (double) n / 48000.0;
                    l = (float) (0.35 * std::sin (kZweiPi * 700.0 * t));
                    r = (float) (0.28 * std::sin (kZweiPi * 1100.0 * t + 1.1));
                },
                [] (const Lauf& L)
                {
                    return L.folddownGesetzt
                        && std::abs ((double) L.folddown - L.handFolddownDb) < 0.25;
                },
                [] (const Lauf& L)
                {
                    return juce::String (L.folddown, 3) + " gegen "
                         + juce::String (L.handFolddownDb, 3);
                }, true);
    }

    nak380Etappe5();

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
