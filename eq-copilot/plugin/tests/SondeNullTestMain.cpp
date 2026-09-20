/*  S9 / SONDE-007b, umgeschrieben in SONDE-015 Etappe 4a (Bauplan §4.4) - der
    Nulltest von Nakama Probeeq.

    WARUM DIESER TEST EXISTIERT: Das Grundgesetz (CLAUDE.md, Wahrheitskern)
    sagt "nichts Ungefragtes": ausgeschaltet ist der Pfad im Nulltest
    bitidentisch, sonst ist der Passthrough sampleidentisch, ohne Latenz oder
    Tail. Bis SONDE-015 war Probeeq in JEDEM Zustand ein Passthrough ohne
    Hostparameter, und genau das mass dieses Bein. Seit Etappe 4a traegt er den
    aktiven DSP-Kern, 112 Host-Parameter und den Transaktionskern - der alte
    Satz "kein Hostparameter" ist damit falsch und faellt (M-116).

    WAS DIESES BEIN JETZT MISST (Manifest SONDE-015 §4.4):
      - Default (`eq_enabled` aus): bitidentisch ueber 1000 Bloecke wechselnder
        Groesse bei 44,1 / 48 / 96 / 192 kHz, danach keine Bank belegt (M-01);
      - `eq_enabled` an, `bypass` aus, sonst alles auf Default: bitidentisch
        (M-02);
      - `eq_enabled` an, `bypass` an, ein hoerbarer Zustand dahinter:
        bitidentisch (M-05);
      - nach `eq_enabled` aus wieder bitidentisch, sobald der Fade vorbei ist
        (M-04);
      - Mix 0 mit Output-Trim 0 dB: bitidentisch, trotz Input-Trim und Band
        (M-33);
      - der Passthrough sanitisiert nichts (M-50);
      - 0 Samples Latenz; kein Tail im Passthrough und im Hard-Bypass (M-51);
      - Speichern, Laden, Speichern bytegleich im Layout v2 mit Kind `Dsp`
        (M-93).
    Seit NAK-283 Etappe 4 (Manifest NAK-283 §5.3) zusaetzlich:
      - der Host-Reset zwischen den Bloecken laesst den Passthrough
        bitidentisch und bewegt keinen Zaehler des Kerns (M-34);
      - das Buslayout folgt der Regel von Gen: Mono und Stereo mit gleichem
        Ein- und Ausgang sind angenommen, Mehrkanal-, ungleiche und
        deaktivierte Busse bekommen ein Nein (M-27 bis M-29).
    Seit NAK-311 Etappe 2 (Manifest NAK-311 §6.1, M-01) zusaetzlich die
    DAZ/FTZ-Messung 311/M-01: sie gibt fuer das Bitmuster aus Subnormals, +-0
    und kleinsten Normalen die Ein- und Ausgangsbits aus und prueft nur, dass
    alle Zustaende, Layouts und Raten liefen - keine Bitgleichheit.
    Seit NAK-311 Etappe 2 Teil b (Manifest NAK-311 §6.1, §22) zusaetzlich:
    engagiert-neutral (SONDE-015 M-02) und bei Mix 0 mit Output-Trim 0 dB
    (M-33) kommen das Bitmuster ab Sample 512, die Wachmarke 0x7F800001, NaN
    und +-Inf bytegleich heraus, auch im Monobus, bei 44,1, 48 und 96 kHz
    (311/M-10, M-11, M-19, M-20, M-21); der Tap post_committed traegt dort den
    Eingang als double (M-92, M-93, R-311-6).
    Seit NAK-311 Etappe 4 Teil a (Manifest §6.4, R-311-3) im Layoutabschnitt:
    der Ausgleich rechnet nur mit dem ausgegebenen Kanal. Ueber den Weg
    Mono -> Stereo -> Mono mit je einem prepareToPlay melden Kern UND
    dspBericht fuer ein Band im Modus `right` im Monobus exakt +0,0 dB, und
    der Ausgang mit Auto-Gain ist bytegleich zum Lauf ohne; im Stereobus
    tragen beide den Zweikanalwert, bitgleich zu einem Stereokern mit
    demselben Zustand (311/M-62). Der Kanalwunsch `channel_mode` bleibt dabei
    im bestaetigten Zustand und in den Statebytes, ohne Revision und ohne
    Host-Dirty (311/M-63).
    Ein eingeschalteter resonanter Filter klingt naturgemaess aus; das ist
    kein Tail im Sinne des Hostvertrags, und dieses Bein behauptet dazu
    nichts.

    Gemessen wird die ECHTE Produktklasse SondeProcessor - dasselbe .cpp wie
    im Bundle, ohne Plugin-Wrapper. Die Gate-7- und Bundlepruefungen aus S9
    und G1 bleiben.

    LANDMINE NAK-175: jeder Prozessor liegt auf dem Heap.

    Aufruf: EqCopProbeeqNullTest.exe        Exit 0 gruen, 1 rot.
*/

#include "SondeProcessor.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#if defined (_M_X64) || defined (__x86_64__)
 #include <xmmintrin.h>
#endif

namespace
{

using Prozessor = nakama::sonde::SondeProcessor;
namespace tx    = nakama::transaktion;
namespace param = nakama::parameter;

int okZahl = 0;
int fehlerZahl = 0;

void pruefe (bool bedingung, const juce::String& text, const juce::String& zusatz = {})
{
    const auto zeile = text + (zusatz.isNotEmpty() ? ("  [" + zusatz + "]") : juce::String());
    if (bedingung) { ++okZahl;     std::cout << "  ok      " << zeile << std::endl; }
    else           { ++fehlerZahl; std::cout << "  FEHLER  " << zeile << std::endl; }
}

void abschnitt (const char* titel)
{
    std::cout << std::endl << "-- " << titel << std::endl;
}

/** Reproduzierbares Rauschen - dieselbe Saat ergibt dieselbe Folge. */
void fuelle (juce::AudioBuffer<float>& puffer, juce::Random& wuerfel)
{
    for (int k = 0; k < puffer.getNumChannels(); ++k)
        for (int n = 0; n < puffer.getNumSamples(); ++n)
            puffer.setSample (k, n, wuerfel.nextFloat() * 1.8f - 0.9f);
}

/** NAK-311 §6.1: das Bitmuster, je Kanal derselbe Block - kleinster
    positiver Subnormal, groesster Subnormal und kleinster Normal, je mit
    beiden Vorzeichen, dazu +0 und -0. An genau diesen Werten entscheidet
    sich, ob die Rueckwandlung float -> double -> float unter FTZ und DAZ
    (`juce::ScopedNoDenormals` in `SondeProcessor::processBlock`) bittreu
    ist; 311/M-01 misst es. */
const std::uint32_t kBitmuster[] = { 0x00000001u, 0x80000001u, 0x007FFFFFu, 0x807FFFFFu,
                                     0x00800000u, 0x80800000u, 0x00000000u, 0x80000000u };

std::string hex32 (std::uint32_t bits)
{
    char text[11];
    std::snprintf (text, sizeof (text), "0x%08X", (unsigned) bits);
    return text;
}

/** NAK-311 §6.1: die Wachmarke - ein signalisierender NaN mit Nutzlast. Eine
    Rueckwandlung float -> double -> float macht ihn ruhig; bytegleich bleibt
    er nur, wenn niemand schreibt (Muster B6, W-1). */
constexpr std::uint32_t kWachmarke = 0x7F800001u;

/** NAK-311 §6.1, R-311-7: Bitmuster und nicht endliche Werte liegen erst ab
    diesem Sample - Engagier-Fade und Rampen sind dann vorbei (wie 311/M-01). */
constexpr std::int64_t kMaterialAb = nakama::dsp::kFadeSamples + nakama::dsp::kRampeSamples;

/** Die sechs nicht endlichen Werte aus 311/M-11: dreimal die Wachmarke, ein
    ruhiger NaN, +Inf und -Inf, jeder auf genau einem Kanal (im Monobus auf
    Kanal 0); der andere Kanal traegt an derselben Stelle Rauschen. */
struct NichtEndlich { int versatz; int kanal; std::uint32_t bits; };
const NichtEndlich kNichtEndlich[] = { { 16, 0, kWachmarke }, { 32, 1, kWachmarke }, { 48, 0, kWachmarke },
                                       { 64, 1, 0x7FC00000u }, { 80, 0, 0x7F800000u }, { 96, 1, 0xFF800000u } };

/** Das Material der NAK-311-Zeilen M-10, M-11, M-17, M-19 und M-20 (§6.1)
    ueber dem Rauschen von `fuelle`. */
struct Material
{
    int  kanaele         = 2;
    bool bitmuster       = false;   ///< M-10: in jedem Block ab kMaterialAb die acht Muster, je Kanal derselbe Block
    bool nurGanzeBloecke = false;   ///< M-20: nur in Bloecken, deren erstes Sample auf oder hinter kMaterialAb liegt
    bool nichtEndlich    = false;   ///< M-11: die sechs Werte im ersten Block, der auf oder hinter kMaterialAb beginnt
};

bool istHex (const std::string& text, std::size_t laenge)
{
    return text.size() == laenge
        && std::all_of (text.begin(), text.end(), [] (char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}

juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
    {
        const auto oben = d.getParentDirectory();
        if (oben == d) break;
        d = oben;
    }
    return d;
}

/** NAK-311 311/M-133: ein Fingerabdruck des Ausgangs (FNV-1a ueber die
    Bytes jedes ausgegebenen float, in Blockreihenfolge). Er traegt den
    Bitvergleich gegen den Basisstand des Aenderungssatzes: derselbe Lauf
    gibt dort dieselbe Zahl aus (§7.1, Zeilenvergleich der Beinausgabe).
    Deterministisch, weil Saat, Blockzahl und Blockgroesse fest sind. */
std::uint64_t fnvAusgang (Prozessor& p, int bloecke, int groesse, int saat, int kanaele = 2)
{
    std::uint64_t h = 1469598103934665603ull;
    juce::Random w (saat);
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> b (kanaele, groesse);
    for (int i = 0; i < bloecke; ++i)
    {
        for (int k = 0; k < kanaele; ++k)
            for (int n = 0; n < groesse; ++n)
                b.setSample (k, n, w.nextFloat() * 1.8f - 0.9f);
        p.processBlock (b, midi);
        for (int k = 0; k < kanaele; ++k)
            for (int n = 0; n < groesse; ++n)
            {
                const auto bits = std::bit_cast<std::uint32_t> (b.getSample (k, n));
                for (int byte = 0; byte < 4; ++byte)
                {
                    h ^= (std::uint64_t) ((bits >> (byte * 8)) & 0xFFu);
                    h *= 1099511628211ull;
                }
            }
    }
    return h;
}

std::unique_ptr<Prozessor> vorbereitet (double rate, int maxBlock)
{
    auto p = std::make_unique<Prozessor>();
    p->setRateAndBufferSizeDetails (rate, maxBlock);
    p->prepareToPlay (rate, maxBlock);
    return p;
}

/** Eine Transaktion ueber die oeffentliche Prozessorschnittstelle. */
tx::Ergebnis setze (Prozessor& p, const param::DspSatz& z)
{
    tx::Auftrag a;
    a.tid          = p.neueTid();
    a.baseRevision = p.stateRevision();
    a.art          = tx::Art::apply;
    a.satz         = z;
    return p.fuehreTransaktionAus (a);
}

int iBand (int slot, int feld) { return param::indexBandV1 (slot, feld); }
int iGlobal (const char* id)   { return param::indexVonId (id); }

/** Ein hoerbarer Zustand: +12-dB-Bell bei 1 kHz, Q 1, auf Slot 0. */
void setzeHoerbaresBand (param::DspSatz& z)
{
    z.werte[(size_t) param::indexOccupied (0)].b        = true;
    z.werte[(size_t) iBand (0, param::kEnabled)].b      = true;
    z.werte[(size_t) iBand (0, param::kFreqHz)].zahl    = 1000.0;
    z.werte[(size_t) iBand (0, param::kGainDb)].zahl    = 12.0;
}

/** NAK-311 311/M-63: zaehlt Host-Dirty ueber den echten JUCE-Weg -
    `updateHostDisplay` ruft `audioProcessorChanged` jedes Listeners synchron
    (Muster B7 `TransactionTestMain.cpp`). */
struct DirtyZaehler final : juce::AudioProcessorListener
{
    int nichtParameter = 0;
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        if (d.nonParameterStateChanged) ++nichtParameter;
    }
};

/** Die wechselnden Blockgroessen der Nulllaeufe - von 1 Sample bis zum
    vorbereiteten Maximum. FL zerteilt Puffer an Automationspunkten bis auf
    1 Sample (host-capabilities-fl-v1.json). */
const int kBlockgroessen[] = { 1, 7, 64, 128, 333, 512, 1024, 2048, 3, 480 };

struct Nulllauf
{
    std::int64_t samples    = 0;
    std::int64_t abweichend = 0;   ///< Samples, deren Rauschen nicht bitgleich zur Eingangskopie ist
    bool         latenzNull = true;

    // NAK-311 §6.1: das Material getrennt vom Rauschen, je Stelle (Kanal, Sample).
    int bitmusterGesetzt = 0, bitmusterAbweichend = 0;
    int nichtEndlichGesetzt = 0, nichtEndlichAbweichend = 0;
    int nichtEndlichEndlichHeraus = 0;   ///< M-17: an der Stelle kam ein endlicher Wert heraus
    int wachmarkenBytegleich = 0;        ///< Wachmarken, die mit 0x7F800001 herauskamen
    int nichtEndlichTapNull = 0;         ///< der Tap post_committed traegt dort +0,0 (M-11)
    std::int64_t rauschenAbweichendAbMaterial = 0;   ///< M-17: Rauschsamples ab kMaterialAb, die veraendert herauskommen
    std::int64_t tapGeprueft = 0, tapAbweichend = 0; ///< R-311-6: Rauschstellen am Tap post_committed
};

/** Faehrt `bloecke` Bloecke Rauschen durch. Abweichungen zaehlen erst ab dem
    globalen Sample `zaehlenAb` - davor liegt ein Fade, der hier nicht
    gemessen wird. `resetAlle > 0` ruft vor jedem `resetAlle`-ten Block den
    Host-Reset (NAK-283 M-34), wie ihn der VST3-Wrapper bei
    `setProcessing (false)` ausloest.

    NAK-311: `material` legt Bitmuster und nicht endliche Werte ueber das
    Rauschen; sie zaehlen getrennt, nie in `abweichend`. `tapAb >= 0`
    vergleicht nach jedem Block den Tap post_committed ab diesem globalen
    Sample an jeder Rauschstelle mit dem Eingang als double (R-311-6) - die
    Stellen des Bitmusters misst 311/M-01. */
Nulllauf fahreNull (Prozessor& p, int bloecke, juce::Random& wuerfel, std::int64_t zaehlenAb = 0,
                    int resetAlle = 0, const Material& material = {}, std::int64_t tapAb = -1)
{
    Nulllauf l;
    juce::MidiBuffer midi;
    const int kanaele = material.kanaele;
    bool nichtEndlichGelegt = false;
    for (int b = 0; b < bloecke; ++b)
    {
        if (resetAlle > 0 && b % resetAlle == resetAlle - 1)
            p.reset();
        const int groesse = kBlockgroessen[(size_t) b % std::size (kBlockgroessen)];
        const std::int64_t start = l.samples;
        juce::AudioBuffer<float> puffer (kanaele, groesse), kopie (kanaele, groesse);
        fuelle (puffer, wuerfel);

        // Je Stelle: 0 Rauschen, 1 Bitmuster, 2 nicht endlich.
        std::vector<std::uint8_t> art ((size_t) (kanaele * groesse), 0);
        const auto lege = [&] (int k, int n, std::uint32_t bits, std::uint8_t a)
        {
            puffer.setSample (k, n, std::bit_cast<float> (bits));
            art[(size_t) (k * groesse + n)] = a;
        };
        if (material.bitmuster && start + groesse > kMaterialAb
            && (! material.nurGanzeBloecke || start >= kMaterialAb))
        {
            const int ab = (int) std::max<std::int64_t> (0, kMaterialAb - start);
            for (int j = 0; j < (int) std::size (kBitmuster) && ab + j < groesse; ++j)
                for (int k = 0; k < kanaele; ++k)
                    lege (k, ab + j, kBitmuster[j], 1);
        }
        if (material.nichtEndlich && ! nichtEndlichGelegt && start >= kMaterialAb
            && groesse > kNichtEndlich[std::size (kNichtEndlich) - 1].versatz)
        {
            for (const auto& w : kNichtEndlich)
                lege (w.kanal % kanaele, w.versatz, w.bits, 2);
            l.nichtEndlichGesetzt += (int) std::size (kNichtEndlich);
            nichtEndlichGelegt = true;
        }

        kopie.makeCopyOf (puffer);
        p.processBlock (puffer, midi);

        auto& kern = p.dspKernFuerTest();
        const bool tapDa = tapAb >= 0 && kern.tapLaenge() == groesse;
        for (int n = 0; n < groesse; ++n, ++l.samples)
        {
            bool rauschenAnders = false;
            for (int k = 0; k < kanaele; ++k)
            {
                const float* aus = puffer.getReadPointer (k) + n;
                const float* ein = kopie.getReadPointer (k) + n;
                const bool anders = std::memcmp (aus, ein, sizeof (float)) != 0;
                const double* tap = tapDa ? kern.tap (nakama::dsp::Tap::postCommitted, k) : nullptr;
                const auto a = art[(size_t) (k * groesse + n)];
                if (a == 1)
                {
                    ++l.bitmusterGesetzt;
                    if (anders) ++l.bitmusterAbweichend;
                }
                else if (a == 2)
                {
                    if (anders) ++l.nichtEndlichAbweichend;
                    if (std::isfinite (*aus)) ++l.nichtEndlichEndlichHeraus;
                    if (std::bit_cast<std::uint32_t> (*ein) == kWachmarke && ! anders) ++l.wachmarkenBytegleich;
                    if (tap != nullptr && tap[n] == 0.0 && ! std::signbit (tap[n])) ++l.nichtEndlichTapNull;
                }
                else
                {
                    if (anders && l.samples >= zaehlenAb) rauschenAnders = true;
                    if (tapAb >= 0 && l.samples >= tapAb)
                    {
                        ++l.tapGeprueft;
                        const double soll = (double) *ein;
                        if (tap == nullptr || std::memcmp (tap + n, &soll, sizeof (double)) != 0) ++l.tapAbweichend;
                    }
                }
            }
            if (rauschenAnders) ++l.abweichend;
            if (rauschenAnders && l.samples >= kMaterialAb) ++l.rauschenAbweichendAbMaterial;
        }
        if (p.getLatencySamples() != 0) l.latenzNull = false;
    }
    return l;
}

} // namespace

int main()
{
    std::cout << "SONDE-NULLTEST - " << std::make_unique<Prozessor>()->getName()
              << " (Produktklasse " << nakama::state::wort (nakama::sonde::kProduktklasse) << ")" << std::endl;

    auto prozessor = std::make_unique<Prozessor>();

    // -- 1. Produktklasse und Bundlevertrag passen zueinander --------------
    // Ein Bundle, dessen frischer Zustand seinen eigenen Vertrag verletzt,
    // kaeme beim ersten Speichern+Laden als read-only zurueck.
    abschnitt ("1. Produktklasse und Bundlevertrag");
    pruefe (prozessor->zustandLesen().common.klasse == nakama::sonde::kProduktklasse,
            "frischer Zustand traegt die Produktklasse des Bundles",
            nakama::state::wort (prozessor->zustandLesen().common.klasse));
    pruefe (nakama::sonde::bundleVertrag().erlaubt (nakama::sonde::kProduktklasse),
            "der Bundlevertrag laesst die eigene Produktklasse zu");
    pruefe (! nakama::sonde::bundleVertrag().erlaubt (nakama::state::Klasse::main),
            "der Bundlevertrag laesst 'main' NICHT zu");
    pruefe (! nakama::sonde::bundleVertrag().erlaubt (nakama::state::Klasse::legacy),
            "der Bundlevertrag laesst 'legacy' NICHT zu");

    // -- 2. Hostparameter und Oberflaeche -----------------------------------
    // M-116: bis SONDE-015 stand hier "das Bundle meldet dem Host keinen
    // Parameter". Die Reihenfolge und die Grenzen der 112 misst B7 (M-88).
    abschnitt ("2. Hostparameter und Oberflaeche");
    pruefe (prozessor->getParameters().size() == param::kHostParameter,
            "das Bundle meldet dem Host die 112 Host-Parameter des Layouts v2",
            juce::String (prozessor->getParameters().size()));
    pruefe (! prozessor->hasEditor(),
            "keine erfundene Oberflaeche (Gestaltung kommt aus Figma)");

    // -- 2b. Lifecycle: neutral bis gueltigem State (§53.5, S9 Abschnitt 3) --
    pruefe (prozessor->klassifikation() == nakama::state::Klassifikation::unclassified,
            "frische Instanz ist neutral, trotz fester Produktklasse",
            nakama::state::wort (prozessor->klassifikation()));
    pruefe (! prozessor->darfBrokerStarten(),
            "eine Sonde darf den Broker nie starten");

    // -- 3. M-01: der Default ist der Passthrough --------------------------
    abschnitt ("3. M-01 default_ist_bitidentisch_ueber_1000_bloecke");
    for (const double rate : { 44100.0, 48000.0, 96000.0, 192000.0 })
    {
        auto p = vorbereitet (rate, 2048);
        juce::Random wuerfel ((juce::int64) rate);
        const auto lauf = fahreNull (*p, 1000, wuerfel);
        pruefe (lauf.abweichend == 0 && lauf.latenzNull,
                "default_ist_bitidentisch_ueber_1000_bloecke bei " + juce::String (rate, 0) + " Hz, Bloecke 1 bis 2048 Samples",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend");
        pruefe (! p->bestaetigterZustand().werte[(size_t) param::kIndexEqEnabled].b
                    && p->dspKernFuerTest().pool().belegteSlots() == 0,
                "  eq_enabled steht auf false, und nach dem Lauf ist keine Bank belegt",
                "belegt " + juce::String (p->dspKernFuerTest().pool().belegteSlots()));
    }

    // -- 3b. NAK-283 M-34: der Host-Reset laesst den Passthrough bitidentisch --
    // Regressionswache (R-283-3, Grundgesetz): reset() laeuft zwischen den
    // Bloecken, der Ausgang bleibt bitgleich zum Eingang, und kein Zaehler des
    // Kerns bewegt sich - ausgeschaltet bei vier Raten, im Hard-Bypass und mit
    // NaN und Inf im Passthrough.
    abschnitt ("3b. NAK-283 M-34 nulltest_bleibt_bitidentisch_mit_host_reset");
    for (const double rate : { 44100.0, 48000.0, 96000.0, 192000.0 })
    {
        auto p = vorbereitet (rate, 2048);
        juce::Random wuerfel ((juce::int64) rate + 34);
        const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 3);
        auto& k = p->dspKernFuerTest();
        pruefe (lauf.abweichend == 0 && lauf.latenzNull && k.nichtEndlicheEingaenge() == 0
                    && k.geheilteFilterzustaende() == 0 && k.verworfeneAnalyseframes() == 0,
                "nulltest_bleibt_bitidentisch_mit_host_reset (M-34) bei " + juce::String (rate, 0)
                    + " Hz: reset() vor jedem dritten Block, Bloecke 1 bis 2048 Samples, kein Zaehler bewegt sich",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend, Zaehler "
                    + juce::String ((juce::int64) k.nichtEndlicheEingaenge()) + "/"
                    + juce::String ((juce::int64) k.geheilteFilterzustaende()) + "/"
                    + juce::String ((juce::int64) k.verworfeneAnalyseframes()));
    }
    {
        auto p = vorbereitet (48000.0, 2048);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
        setzeHoerbaresBand (z);
        const auto e = setze (*p, z);
        juce::Random wuerfel (534);
        const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 3);

        juce::MidiBuffer midi;
        juce::AudioBuffer<float> b (2, 256), kopie (2, 256);
        fuelle (b, wuerfel);
        b.setSample (0, 10, std::numeric_limits<float>::quiet_NaN());
        b.setSample (1, 20, std::numeric_limits<float>::infinity());
        b.setSample (0, 30, -std::numeric_limits<float>::infinity());
        kopie.makeCopyOf (b);
        p->reset();
        p->processBlock (b, midi);
        p->reset();
        bool roh = true;
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < 256; ++n)
                if (std::memcmp (b.getReadPointer (k) + n, kopie.getReadPointer (k) + n, sizeof (float)) != 0) roh = false;
        auto& kern = p->dspKernFuerTest();
        pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && lauf.latenzNull && roh
                    && kern.nichtEndlicheEingaenge() == 0 && kern.geheilteFilterzustaende() == 0,
                "  im Hard-Bypass: bitgleich mit reset() vor jedem dritten Block; NaN, +Inf und -Inf kommen zwischen zwei reset() bitgleich heraus, kein Zaehler steigt",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend, NaN/Inf roh "
                    + (roh ? "ja" : "nein"));
    }

    // -- 4. M-51: Latenz und Tail -------------------------------------------
    abschnitt ("4. M-51 latenz_bleibt_null und kein_tail_im_passthrough");
    {
        auto p = vorbereitet (48000.0, 512);
        const auto stilleNachRauschen = [] (Prozessor& q)
        {
            juce::MidiBuffer midi;
            juce::Random w (7);
            for (int i = 0; i < 20; ++i)
            {
                juce::AudioBuffer<float> b (2, 512);
                fuelle (b, w);
                q.processBlock (b, midi);
            }
            juce::AudioBuffer<float> still (2, 512);
            still.clear();
            q.processBlock (still, midi);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < 512; ++n)
                    if (std::memcmp (still.getReadPointer (k) + n, "\0\0\0\0", sizeof (float)) != 0) return false;
            return true;
        };
        const bool ausgeschaltet = stilleNachRauschen (*p);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
        setzeHoerbaresBand (z);
        const auto e = setze (*p, z);
        const bool hardBypass = stilleNachRauschen (*p);
        pruefe (p->getLatencySamples() == 0 && p->getTailLengthSeconds() == 0.0,
                "latenz_bleibt_null: 0 Samples gemeldet, Tail 0,0 s",
                juce::String (p->getLatencySamples()) + " / " + juce::String (p->getTailLengthSeconds()));
        pruefe (ausgeschaltet && hardBypass && e.ausgang == tx::Ausgang::commit,
                "kein_tail_im_passthrough: nach Rauschen bleibt Stille bitgenau still - ausgeschaltet und im Hard-Bypass");
    }

    // -- 5. M-50: der Passthrough sanitisiert nichts ------------------------
    abschnitt ("5. M-50 passthrough_sanitisiert_nichts");
    {
        auto p = vorbereitet (48000.0, 256);
        const auto nichtEndlichBleibt = [] (Prozessor& q)
        {
            juce::MidiBuffer midi;
            juce::AudioBuffer<float> b (2, 256), kopie (2, 256);
            juce::Random w (11);
            fuelle (b, w);
            b.setSample (0, 10, std::numeric_limits<float>::quiet_NaN());
            b.setSample (1, 20, std::numeric_limits<float>::infinity());
            b.setSample (0, 30, -std::numeric_limits<float>::infinity());
            kopie.makeCopyOf (b);
            q.processBlock (b, midi);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < 256; ++n)
                    if (std::memcmp (b.getReadPointer (k) + n, kopie.getReadPointer (k) + n, sizeof (float)) != 0) return false;
            return true;
        };
        const bool ausgeschaltet = nichtEndlichBleibt (*p);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
        setzeHoerbaresBand (z);
        setze (*p, z);
        const bool hardBypass = nichtEndlichBleibt (*p);
        pruefe (ausgeschaltet && hardBypass && p->dspKernFuerTest().nichtEndlicheEingaenge() == 0,
                "passthrough_sanitisiert_nichts: NaN, +Inf und -Inf kommen bitgleich heraus, ausgeschaltet und im Hard-Bypass; kein Zaehler steigt",
                "Zaehler " + juce::String ((juce::int64) p->dspKernFuerTest().nichtEndlicheEingaenge()));
    }

    // -- 6. M-02: engagiert, aber neutral ----------------------------------
    // NAK-311 (M-10, M-21, M-93): dazu das Bitmuster ab Sample kMaterialAb in
    // jedem Block, 48 kHz als dritte Rate und der Tap post_committed ueber
    // dasselbe Fenster (R-311-6) - er misst die Rechnung der neutralen Kette,
    // die der Ausgangsvergleich nicht mehr sieht, seit der Kern in diesem
    // Zustand nicht schreibt. Das Rauschen zaehlt ab Sample 0, samt
    // Engagier-Fade; fuer das Bitmuster gilt die Zusage ab dem Schreibende
    // (R-311-7), deshalb liegt es erst ab Sample 512.
    abschnitt ("6. M-02 eq_an_bypass_aus_alles_neutral_ist_bitidentisch");
    for (const double rate : { 44100.0, 48000.0, 96000.0 })
    {
        auto p = vorbereitet (rate, 2048);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        const auto e = setze (*p, z);
        juce::Random wuerfel ((juce::int64) rate + 2);
        Material material;
        material.bitmuster = true;
        const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 0, material, 0);
        int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
        p->dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
        const bool rechnet = aktiv >= 0 && p->dspKernFuerTest().pool().bank (aktiv).programm.eqEngagiert
                          && ! p->dspKernFuerTest().pool().bank (aktiv).programm.hardBypass;
        pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && rechnet,
                "eq_an_bypass_aus_alles_neutral_ist_bitidentisch bei " + juce::String (rate, 0) + " Hz - samt Engagier-Fade, durch Trims, M/S und Filterbank",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend, aktive Bank " + juce::String (aktiv));
        pruefe (rechnet && lauf.bitmusterGesetzt > 0 && lauf.bitmusterAbweichend == 0,
                "311/M-10 bitmuster_ab_sample_512_bytegleich bei " + juce::String (rate, 0)
                    + " Hz: Subnormals, +-0 und kleinste Normale kommen im engagiert-neutralen Kern bytegleich heraus",
                juce::String (lauf.bitmusterAbweichend) + " von " + juce::String (lauf.bitmusterGesetzt)
                    + " Musterstellen veraendert");
        pruefe (rechnet && lauf.tapGeprueft > 0 && lauf.tapAbweichend == 0,
                "311/M-93 tap_post_committed_traegt_im_neutralen_kern_den_eingang bei " + juce::String (rate, 0)
                    + " Hz: ab Sample 0, samt Engagier-Fade, an jeder Rauschstelle beider Kanaele exakt der Eingang als double",
                juce::String (lauf.tapAbweichend) + " von " + juce::String (lauf.tapGeprueft) + " Tapstellen abweichend");
    }

    // -- 7. M-05: der Hard-Bypass ------------------------------------------
    abschnitt ("7. M-05 eq_an_bypass_an_ist_bitidentisch");
    {
        auto p = vorbereitet (48000.0, 2048);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
        z.werte[(size_t) iGlobal ("v1.global.width")].zahl = 1.7;
        setzeHoerbaresBand (z);
        const auto e = setze (*p, z);
        juce::Random wuerfel (5);
        const auto lauf = fahreNull (*p, 1000, wuerfel);
        int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
        p->dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
        pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && aktiv >= 0
                    && p->dspKernFuerTest().pool().bank (aktiv).programm.hardBypass,
                "eq_an_bypass_an_ist_bitidentisch: +12-dB-Bell und Width 1,7 liegen hinter dem Hard-Bypass und schlagen nicht durch",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend");
    }

    // -- 8. M-04: nach dem Ausschalten wieder bitidentisch -----------------
    abschnitt ("8. M-04 nach_dem_fade_wieder_bitidentisch");
    {
        auto p = vorbereitet (48000.0, 2048);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        setzeHoerbaresBand (z);
        setze (*p, z);
        juce::Random wuerfel (21);
        const auto an = fahreNull (*p, 200, wuerfel);
        auto aus = p->bestaetigterZustand();
        aus.werte[(size_t) param::kIndexEqEnabled].b = false;
        const auto e = setze (*p, aus);
        // Der Fade laeuft kFadeSamples Samples ab dem ersten Block nach der
        // Publikation; gezaehlt wird ab dem ersten Sample danach.
        const auto nach = fahreNull (*p, 400, wuerfel, nakama::dsp::kFadeSamples);
        int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
        p->dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
        pruefe (an.abweichend > 0 && e.ausgang == tx::Ausgang::commit && nach.abweichend == 0 && aktiv < 0 && quelle < 0,
                "nach_dem_fade_wieder_bitidentisch: nach " + juce::String (nakama::dsp::kFadeSamples)
                    + " Fade-Samples bitgleich, keine Bank faehrt mehr",
                "eingeschaltet " + juce::String (an.abweichend) + " abweichende Samples (das Band war hoerbar); danach "
                    + juce::String (nach.abweichend) + " abweichend in " + juce::String (nach.samples - nakama::dsp::kFadeSamples));
    }

    // -- 9. M-33: Mix 0 ----------------------------------------------------
    abschnitt ("9. M-33 mix_null_ist_bitidentisch");
    {
        auto zustandMitMix = [] (double mix)
        {
            param::DspSatz z;
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            setzeHoerbaresBand (z);
            z.werte[(size_t) iGlobal ("v1.global.input_trim_db")].zahl = 9.0;
            z.werte[(size_t) param::kIndexMix].zahl = mix;
            return z;
        };
        const std::int64_t einschwingen = nakama::dsp::kFadeSamples + nakama::dsp::kRampeSamples;
        // NAK-311 (M-20, M-21, M-92): Bitmuster und die sechs nicht endlichen
        // Werte aus 311/M-11 erst in Bloecken, deren erstes Sample auf oder
        // hinter Sample 512 liegt (das Stueck mit dem Fade-Ende darf der Kern
        // nach §9 F-4 bis zu seinem Ende schreiben), bei 44,1, 48 und 96 kHz;
        // dazu der Tap post_committed ab Sample 512 (R-311-6): der Dry-Zweig
        // ist pre_nakama, vor Input-Trim und Filterbank.
        Material material;
        material.bitmuster = true;
        material.nurGanzeBloecke = true;
        material.nichtEndlich = true;
        for (const double rate : { 44100.0, 48000.0, 96000.0 })
        {
            auto p = vorbereitet (rate, 2048);
            const auto e = setze (*p, zustandMitMix (0.0));
            juce::Random wuerfel ((juce::int64) rate + 33);
            const auto lauf = fahreNull (*p, 1000, wuerfel, einschwingen, 0, material, einschwingen);
            auto gegen = vorbereitet (rate, 2048);
            setze (*gegen, zustandMitMix (1.0));
            juce::Random wuerfelGegen ((juce::int64) rate + 33);
            const auto gegenLauf = fahreNull (*gegen, 1000, wuerfelGegen, einschwingen, 0, material);
            pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && gegenLauf.abweichend > 0,
                    "mix_null_ist_bitidentisch bei " + juce::String (rate, 0)
                        + " Hz: trotz +9 dB Input-Trim und +12-dB-Bell, Output-Trim 0 dB",
                    "Mix 0: " + juce::String (lauf.abweichend) + " abweichend; Gegenprobe Mix 1: "
                        + juce::String (gegenLauf.abweichend) + " abweichend (das Band ist hoerbar)");
            pruefe (e.ausgang == tx::Ausgang::commit && lauf.nichtEndlichGesetzt == 6 && lauf.nichtEndlichAbweichend == 0,
                    "311/M-20 mix_null_laesst_bitmuster_und_wachmarke_bytegleich bei " + juce::String (rate, 0)
                        + " Hz, Wachmarke: Wachmarke, NaN und +-Inf kommen bei Mix 0 bytegleich heraus",
                    juce::String (lauf.nichtEndlichAbweichend) + " von " + juce::String (lauf.nichtEndlichGesetzt)
                        + " nicht endlichen Werten veraendert, " + juce::String (lauf.wachmarkenBytegleich)
                        + " Wachmarken bytegleich");
            pruefe (e.ausgang == tx::Ausgang::commit && lauf.bitmusterGesetzt > 0 && lauf.bitmusterAbweichend == 0,
                    "311/M-20 mix_null_laesst_bitmuster_und_wachmarke_bytegleich bei " + juce::String (rate, 0)
                        + " Hz, Bitmuster: Subnormals, +-0 und kleinste Normale kommen bei Mix 0 bytegleich heraus",
                    juce::String (lauf.bitmusterAbweichend) + " von " + juce::String (lauf.bitmusterGesetzt)
                        + " Musterstellen veraendert");
            pruefe (e.ausgang == tx::Ausgang::commit && lauf.tapGeprueft > 0 && lauf.tapAbweichend == 0,
                    "311/M-92 tap_post_committed_traegt_bei_mix_null_den_eingang bei " + juce::String (rate, 0)
                        + " Hz: ab Sample 512 an jeder Rauschstelle beider Kanaele exakt der Eingang als double",
                    juce::String (lauf.tapAbweichend) + " von " + juce::String (lauf.tapGeprueft) + " Tapstellen abweichend");
        }
    }

    // -- 10. M-93: Speichern, Laden, Speichern im Layout v2 ----------------
    abschnitt ("10. M-93 speichern_laden_speichern_bytegleich_in_v2");
    {
        auto a = vorbereitet (48000.0, 512);
        auto z = a->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        setzeHoerbaresBand (z);
        z.zonen = { { 2, 3000.0, 4000.0, true } };
        setze (*a, z);
        auto z2 = a->bestaetigterZustand();
        z2.werte[(size_t) iBand (0, param::kGainDb)].zahl = -3.0;
        z2.werte[(size_t) iBand (0, param::kQ)].zahl = 0.7071067811865476;
        setze (*a, z2);
        tx::Auftrag undo;
        undo.tid = a->neueTid();
        undo.baseRevision = a->stateRevision();
        undo.art = tx::Art::undo;
        a->fuehreTransaktionAus (undo);

        juce::MemoryBlock erst;
        a->getStateInformation (erst);
        auto b = std::make_unique<Prozessor>();
        b->setStateInformation (erst.getData(), (int) erst.getSize());
        juce::MemoryBlock zweit;
        b->getStateInformation (zweit);

        const auto baum = juce::ValueTree::readFromData (erst.getData(), erst.getSize());
        const auto dsp  = baum.getChildWithName ("Dsp");
        const auto par  = baum.getChildWithName ("Parameters");
        pruefe (erst == zweit && dsp.isValid() && (juce::int64) dsp.getProperty ("state_revision") == (juce::int64) a->stateRevision()
                    && par.isValid() && (int) par.getProperty ("dsp_schema_version") == 2
                    && b->stateRevision() == a->stateRevision() && b->stateHashText() == a->stateHashText()
                    && ! b->zustandLesen().nurLesen,
                "speichern_laden_speichern_bytegleich_in_v2: Kind Dsp mit Revision, Zone und Undo-Ring",
                juce::String ((int) erst.getSize()) + " Bytes, Revision " + juce::String ((juce::int64) a->stateRevision()));
    }

    // -- 11. Gegenpfad: speichern <-> laden (Common, Klassifikation) --------
    // Invariante des Hauses: beide Haelften im selben Aenderungssatz.
    abschnitt ("11. Speichern und Laden: Identitaet und Klassifikation");
    {
        juce::MemoryBlock bytes;
        prozessor->getStateInformation (bytes);
        pruefe (bytes.getSize() > 0, "Zustand laesst sich speichern",
                juce::String ((int) bytes.getSize()) + " Bytes");

        const auto vorher = prozessor->zustandLesen().common;
        auto zweiter = std::make_unique<Prozessor>();
        zweiter->setStateInformation (bytes.getData(), (int) bytes.getSize());

        pruefe (zweiter->zustandLesen().common == vorher,
                "geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)");
        pruefe (! zweiter->zustandLesen().nurLesen,
                "der eigene Stand kommt NICHT als read-only zurueck",
                zweiter->zustandLesen().grund);
        pruefe (zweiter->klassifikation()
                    == (nakama::sonde::kProduktklasse == nakama::state::Klasse::passive_probe
                            ? nakama::state::Klassifikation::passive_probe
                            : nakama::state::Klassifikation::active_probe),
                "nach gueltigem State traegt der Lebenslauf die Produktklasse",
                nakama::state::wort (zweiter->klassifikation()));

        juce::MemoryBlock zurueck;
        zweiter->getStateInformation (zurueck);
        pruefe (zurueck == bytes, "speichern -> laden -> speichern ist bytegleich",
                juce::String ((int) zurueck.getSize()) + " Bytes");
    }

    // -- 11a. Probeeq ist ein gebundener reiner v3-Connector ---------------
    // Die Testschale startet bewusst keine Produktpipe; sie greift aber auf
    // die echten, im Produktprozessor gehaltenen Control-/Telemetry-Clients
    // und deren echte Provider zu.
    {
        nakama::state::Zustand gebunden =
            nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        gebunden.common.klasse = nakama::state::Klasse::active_probe;
        gebunden.common.projectBindingId = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
        gebunden.hatParameters = true;
        juce::MemoryBlock bytes;
        nakama::state::speichere (gebunden, bytes);

        auto connector = std::make_unique<Prozessor>();
        connector->setStateInformation (bytes.getData(), (int) bytes.getSize());
        connector->setRateAndBufferSizeDetails (48000.0, 512);
        connector->prepareToPlay (48000.0, 512);
        const auto hello = connector->v3HelloFuerTest();
        const auto status = connector->v3StatusFuerTest();
        juce::String sollHash, grund;
        const bool hashOk = nakama::parameter::stateHash (gebunden.dspDto(), sollHash, grund);

        pruefe (hello.adresse.projectBindingId == "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                 && hello.adresse.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 && hello.pluginKind == "active_probe"
                 && hello.adresse.sessionEpoch == hello.adresse.projectBindingId
                 && istHex (hello.adresse.runtimeNonce, 32),
                "Probeeq-Controlprovider erfindet keine eigene Session-Epoche");
        // SONDE-015 4a: Revision und Hash kommen aus dem Transaktionskern. Der
        // geladene Stand traegt state_revision 0; bis 4a zaehlte hier ein
        // Ladezaehler, der mit einer bestaetigten Revision nichts zu tun hatte.
        pruefe (status.dspSchemaVersion == nakama::parameter::kDspSchemaVersion
                    && status.stateRevision == 0 && status.stateRevision == connector->stateRevision()
                    && hashOk && status.stateHash == sollHash.toStdString()
                    && status.stateHash == connector->stateHashText().toStdString()
                    && ! status.recordStateValid && ! status.recording,
                "Probeeq-Stateprovider meldet Revision und Hash des Transaktionskerns und keinen erfundenen Record-State");
        pruefe (connector->controlV3FuerTest().status == nakama::ipc::ControlClient::Status::getrennt
                    && connector->telemetryV3FuerTest().status == nakama::ipc::TelemetryClient::Status::getrennt
                    && ! connector->darfBrokerStarten(),
                "Testschale belegt beide Connectoren; Probeeq besitzt keinen Startpfad");
        pruefe (connector->v3ProduktstatusVerdrahtetFuerTest(),
                "Probeeq-ControlClient traegt seinen produktiven Statusprovider");

        const auto produktquelle = wurzel()
            .getChildFile ("eq-copilot/plugin/sonde/SondeProcessor.cpp")
            .loadFileAsString();
        pruefe (produktquelle.contains ("controlV3.start();")
                    && produktquelle.contains ("telemetryV3.start();")
                    && ! produktquelle.contains ("BrokerLifecycle"),
                "Probeeq startet produktiv beide v3-Connectoren, aber niemals einen Broker");
    }

    // -- 5b. Gate 7 auf State-Ebene, gemessen AM BUNDLE (G1 §4.2) ----------
    // Der Gate-Lauf G1 vom 24.08.2026 ist genau diese Kette gefahren:
    // Host-State-Restore -> lade() -> positionAusWort -> positionErlaubt ->
    // uebernommen -> beim naechsten Speichern wieder hinausgeschrieben. Eine
    // Sonde mit genau einem Stereo-Bus (SondeProcessor.cpp:7-9) konnte sich so
    // dauerhaft `post_fader_contribution` nennen - die exakte Bezeichnung
    // eines Mastersummenbeitrags auf einem Aux-Bus, den dieses Bundle nicht
    // hat. Das ist Gate 7 aus §49.2 Nr. 7 im Wortlaut.
    //
    // Der Riegel dagegen sitzt seit a2fe0f5 in `positionErlaubt`
    // (state/NakamaState.cpp) und hat zwei Haelften: Riegel 1 ist die
    // CAPABILITY-Vorpruefung (`kContributionAuxVerfuegbar`, heute gemessen
    // unsupported), Riegel 2 die Klassenmatrix. Beide lehnen
    // `post_fader_contribution` heute fuer jede Klasse ab; von aussen ist nur
    // das GEMEINSAME Ergebnis sichtbar, nicht welche Haelfte greift. Die
    // VOLLSTAENDIGE Matrix - alle vier Klassen, jede in dem Bundle, das sie
    // zulaesst, `passive_probe` also mit Bundlevertrag `nkpr()` - misst B2
    // `EqCopStateMigrationTest` (Block G8b) auf `lade()`-Ebene.
    //
    // 🔑 Hier faehrt die GANZE Kette durch die echte Sondenschale. Das ist
    // keine Wiederholung: `SondeProcessor::setStateInformation` hat einen
    // eigenen Weg - bei `ignoriert` kehrt er VOR dem Schloss um, bei
    // `nurLesen` nicht -, und das Artefakt dieses Tickets ist das Bundle,
    // nicht `lade()`.
    //
    // ⚠️ WIE WEIT DAS TRAEGT - genau und nicht weiter: die Schale traegt
    // GENAU EINE Klasse je Uebersetzung (`kProduktklasse`), und gebaut wird
    // seit S9b/`SONDE-007c` nur noch `active_probe`. Der Durchgriff durch die
    // Schale ist damit fuer DIESE EINE Klasse gemessen, nicht fuer vier. Dass
    // `positionErlaubt` die Position fuer ALLE VIER Klassen ablehnt, misst
    // Punkt 7 unten (direkt an der Funktion) und B2 auf `lade()`-Ebene - beide
    // messen das gemeinsame Ergebnis beider Riegelhaelften, keine von beiden
    // trennt sie auf. Der urspruengliche G1-§4.2-Traeger - eine passive Sonde
    // als PRODUKT - existiert seit S9b nicht mehr.
    {
        // Ein sonst GUELTIGER Stand dieses Bundles: eigene Produktklasse,
        // eigener Bundlevertrag, richtige Kind-Matrix (§2.1) - nur die
        // Messposition ist die verbotene. Gebaut ueber den oeffentlichen
        // State-Weg (`speichere`), nicht ueber eine Hintertuer im Produktcode.
        nakama::state::Zustand gebastelt =
            nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        gebastelt.common.klasse   = nakama::sonde::kProduktklasse;
        gebastelt.common.position = nakama::state::Messposition::post_fader_contribution;
        gebastelt.hatParameters   = (nakama::sonde::kProduktklasse
                                        == nakama::state::Klasse::active_probe);

        juce::MemoryBlock verboten;
        nakama::state::speichere (gebastelt, verboten);

        // Ohne diese Probe waere der ganze Block aus dem FALSCHEN Grund gruen:
        // haette `speichere` die Position stillschweigend begradigt, gaebe es
        // unten gar nichts mehr abzuwehren.
        const auto probe = juce::ValueTree::readFromData (verboten.getData(), verboten.getSize());
        const auto probeWort = probe.isValid()
            ? probe.getChildWithName ("Common").getProperty ("measurement_position").toString()
            : juce::String ("<kein Baum>");
        pruefe (probeWort == "post_fader_contribution",
                "die Probebytes tragen wirklich measurement_position=post_fader_contribution",
                probeWort);

        auto opfer = std::make_unique<Prozessor>();
        opfer->setStateInformation (verboten.getData(), (int) verboten.getSize());

        // 1) Der Stand wird NICHT als eigener uebernommen: `leseSchema2`
        //    bricht an `positionErlaubt` ab, `lade()` faellt auf read-only mit
        //    Grund, und der Prozessor haelt genau diesen Zustand.
        pruefe (opfer->zustandLesen().nurLesen,
                "insert_probe_is_never_called_master_sum: der Stand kommt als read-only zurueck, nicht als eigener",
                opfer->zustandLesen().grund);
        pruefe (opfer->zustandLesen().grund.isNotEmpty(),
                "read-only nennt seinen Grund");
        pruefe (opfer->zustandLesen().common.position
                    != nakama::state::Messposition::post_fader_contribution,
                "das Bundle FUEHRT die verbotene Position nicht",
                nakama::state::wort (opfer->zustandLesen().common.position));

        // 2) Die Instanz bleibt neutral - keine Klassifikation auf die
        //    Produktklasse, kein Brokerstart. §53.5: read-only ist das
        //    Gegenteil eines vollstaendigen State-Restore.
        pruefe (opfer->klassifikation() == nakama::state::Klassifikation::unclassified,
                "die Instanz klassifiziert NICHT auf die Produktklasse, sie bleibt neutral",
                nakama::state::wort (opfer->klassifikation()));
        pruefe (! opfer->darfBrokerStarten(),
                "eine read-only-Sonde darf den Broker nicht starten");

        // 3) §53.8 verlustfrei: die Originalbytes reisen unveraendert zum Host
        //    zurueck. Ein Altprojekt verliert seinen Stand nicht - es darf ihn
        //    nur nicht mehr behaupten.
        juce::MemoryBlock zurueck;
        opfer->getStateInformation (zurueck);
        pruefe (zurueck == verboten,
                "Vertrag 53.8: dieselben Originalbytes gehen unveraendert an den Host zurueck",
                juce::String ((int) zurueck.getSize()) + " Bytes");

        // 4) Und der Rueckweg waescht nichts: eine DRITTE frische Instanz, die
        //    genau die herausgegebenen Bytes laedt, kommt wieder read-only und
        //    neutral. Damit gibt es keinen Umweg, ueber den das Bundle die
        //    Position doch als gueltigen eigenen Stand fuehrt.
        auto dritte = std::make_unique<Prozessor>();
        dritte->setStateInformation (zurueck.getData(), (int) zurueck.getSize());
        pruefe (dritte->zustandLesen().nurLesen,
                "der Rueckweg waescht nichts: erneut geladen bleibt read-only",
                dritte->zustandLesen().grund);
        pruefe (dritte->klassifikation() == nakama::state::Klassifikation::unclassified,
                "und erneut geladen bleibt die Instanz neutral",
                nakama::state::wort (dritte->klassifikation()));

        // 5) Gegenprobe: DERSELBE Stand mit einer fuer die Klasse erlaubten
        //    Position laedt normal und klassifiziert. Ohne sie wuesste
        //    niemand, ob oben die POSITION abgewiesen wurde oder irgendetwas
        //    anderes am Bastelstand.
        gebastelt.common.position = nakama::state::Messposition::insert;
        juce::MemoryBlock erlaubteBytes;
        nakama::state::speichere (gebastelt, erlaubteBytes);

        auto gegenprobe = std::make_unique<Prozessor>();
        gegenprobe->setStateInformation (erlaubteBytes.getData(), (int) erlaubteBytes.getSize());
        pruefe (! gegenprobe->zustandLesen().nurLesen,
                "Gegenprobe: derselbe Stand mit erlaubter Position laedt normal",
                gegenprobe->zustandLesen().grund);
        pruefe (gegenprobe->zustandLesen().common == gebastelt.common,
                "Gegenprobe: der geladene Common ist derselbe (Position insert)",
                nakama::state::wort (gegenprobe->zustandLesen().common.position));
        pruefe (gegenprobe->klassifikation()
                    == (nakama::sonde::kProduktklasse == nakama::state::Klasse::passive_probe
                            ? nakama::state::Klassifikation::passive_probe
                            : nakama::state::Klassifikation::active_probe),
                "Gegenprobe: nach gueltigem Stand traegt der Lebenslauf die Produktklasse",
                nakama::state::wort (gegenprobe->klassifikation()));

        // 6) Und der Weg, den FL wirklich geht: die Instanz steht laengst im
        //    Projekt und ist klassifiziert, DANN reicht der Host ihr den
        //    verbotenen Stand nach (Preset-Browser, Copy/Paste, geoeffnetes
        //    Altprojekt). Punkt 1-5 haben nur frische Instanzen gemessen -
        //    haette der Riegel hier eine Luecke, waere sie die einzige, die im
        //    Betrieb ueberhaupt erreichbar ist.
        //
        //    §53.5: read-only ist das Gegenteil eines vollstaendigen Restore,
        //    also faellt auch eine ZUVOR positiv klassifizierte Instanz auf
        //    neutral zurueck. Sie darf ihre Rechte nicht behalten, nur weil sie
        //    sie einmal hatte.
        pruefe (gegenprobe->klassifikation() != nakama::state::Klassifikation::unclassified,
                "Nachreichen: die Instanz ist VOR dem verbotenen Stand klassifiziert",
                nakama::state::wort (gegenprobe->klassifikation()));

        gegenprobe->setStateInformation (verboten.getData(), (int) verboten.getSize());

        pruefe (gegenprobe->zustandLesen().nurLesen,
                "Nachreichen: der verbotene Stand kommt auch bei einer laufenden Instanz read-only",
                gegenprobe->zustandLesen().grund);
        pruefe (gegenprobe->klassifikation() == nakama::state::Klassifikation::unclassified,
                "Nachreichen: die Klassifikation faellt zurueck auf neutral, alte Rechte bleiben nicht",
                nakama::state::wort (gegenprobe->klassifikation()));

        // §53.8 auch hier: der Prozessor erfindet keinen Stand und faellt auch
        // nicht auf den vorherigen zurueck - er gibt heraus, was der Host ihm
        // gegeben hat. Alles andere waere ein stiller Stand-Tausch hinter dem
        // Ruecken des Projekts.
        juce::MemoryBlock nachReichen;
        gegenprobe->getStateInformation (nachReichen);
        pruefe (nachReichen == verboten,
                "Nachreichen: der Host bekommt genau die Bytes zurueck, die er gab - kein stiller Tausch",
                juce::String ((int) nachReichen.getSize()) + " Bytes");

        // 7) `positionErlaubt` lehnt `post_fader_contribution` fuer ALLE VIER
        //    Klassen ab - gemessen an genau der Funktion, an der Punkt 0-6
        //    oben abbiegen.
        //
        //    ⚠️ WAS DIESE PRUEFUNG IST UND WAS SIE NICHT IST. Gemessen wird das
        //    GEMEINSAME Ergebnis beider Riegelhaelften: Riegel 1 (die
        //    Capability-Vorpruefung `kContributionAuxVerfuegbar`) und Riegel 2
        //    (die Klassenmatrix) lehnen die Position heute beide ab. WELCHE
        //    Haelfte im Einzelfall sperrt, sagt diese Pruefung NICHT -
        //    `positionErlaubt` gibt nur `false` zurueck, und eine Abfrage, die
        //    die Haelften trennt, gibt es im Kern nicht. Die Aussage bleibt
        //    deshalb genau: abgelehnt fuer alle vier Klassen.
        //
        //    Punkt 0-6 fahren die ganze Kette durch das echte Bundle, aber nur
        //    fuer EINE Klasse: `kProduktklasse` ist ein Uebersetzungsschalter
        //    (`plugin/CMakeLists.txt`, `nakama_sonde_nulltest`), und seit
        //    S9b/`SONDE-007c` setzt KEIN Bauziel mehr `NAKAMA_SONDE_PASSIV` -
        //    Nakama Suna ist stillgelegt (`SondeProcessor.h:5-12`). Diese
        //    Schale kann heute also nur `active_probe` bauen.
        //
        //    Der urspruengliche Traeger der G1-§4.2-Regression war aber
        //    gerade `passive_probe`: vor `a2fe0f5` gab es Riegel 1 nicht und
        //    die Matrix trug `case Klasse::passive_probe: return true;`. Setzt
        //    man beides auf jenen Stand zurueck, bleiben Punkt 0-6 alle gruen
        //    - sie werden weiter an der unveraenderten `active_probe`-Zeile
        //    abgewiesen. Ohne die vier Zeilen hier faenge dieses Bein die
        //    Regression, gegen die es antritt, NICHT; mit ihnen faellt genau
        //    die `passive_probe`-Zeile (Mutationsprobe Runde 2, SONDE-007b).
        //
        //    Ein passives Bauziel wiederzubeleben ist der falsche Weg dagegen
        //    (es naehme `SONDE-007c` zurueck), eine Testhintertuer im
        //    Produktcode ebenso. Also wird die Ablehnung direkt an der
        //    oeffentlichen `positionErlaubt` gemessen: derselben Funktion, die
        //    die Schale oben ueber `lade()` aufruft.
        //
        //    Die VOLLSTAENDIGE 16er-Matrix samt Bundlevertraegen - jede Klasse
        //    in dem Bundle, das sie zulaesst, `passive_probe` also in `nkpr()`
        //    - misst B2 `EqCopStateMigrationTest`, Block G8b. Nicht dieses
        //    Bein, und das soll es auch nicht: hier steht die Kette am
        //    gebauten Bundle, dort der Vertrag.
        for (const auto klasse : { nakama::state::Klasse::main,
                                   nakama::state::Klasse::passive_probe,
                                   nakama::state::Klasse::active_probe,
                                   nakama::state::Klasse::legacy })
            pruefe (! nakama::state::positionErlaubt (
                        klasse, nakama::state::Messposition::post_fader_contribution),
                    juce::String ("positionErlaubt (Riegel 1 und Klassenmatrix gemeinsam) "
                                  "lehnt post_fader_contribution ab fuer '")
                        + nakama::state::wort (klasse) + "'");
    }
    // -- 6. Muell aendert nichts --------------------------------------------
    {
        const auto vorher = prozessor->zustandLesen().common;
        const char muell[] = "das ist kein NakamaState";
        prozessor->setStateInformation (muell, (int) sizeof (muell));
        pruefe (prozessor->zustandLesen().common == vorher,
                "Muellbytes lassen den gehaltenen Zustand unveraendert");
        prozessor->setStateInformation (nullptr, 0);
        pruefe (prozessor->zustandLesen().common == vorher,
                "Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert");
    }

    // -- 12. NAK-283 F04: das Buslayout folgt der Regel von Gen --------------
    // Der rechnende Kern kennt zwei Kanaele. Mono und Stereo mit gleichem Ein-
    // und Ausgang sind angenommen; alles andere bekommt ein klares Nein, nie
    // still umgedeutet (Entwurf §48.2) und nie ein Bus, dessen Kanaele 3 und 4
    // unbearbeitet durchliefen.
    abschnitt ("12. NAK-283 F04 Buslayout (M-27 bis M-29)");
    {
        const auto layout = [] (const juce::AudioChannelSet& ein, const juce::AudioChannelSet& aus)
        {
            juce::AudioProcessor::BusesLayout l;
            l.inputBuses.add (ein);
            l.outputBuses.add (aus);
            return l;
        };
        const auto mono   = juce::AudioChannelSet::mono();
        const auto stereo = juce::AudioChannelSet::stereo();
        const auto quad   = juce::AudioChannelSet::quadraphonic();
        const auto fuenf1 = juce::AudioChannelSet::create5point1();
        const auto vier   = juce::AudioChannelSet::discreteChannels (4);
        const auto aus    = juce::AudioChannelSet::disabled();
        const auto jaNein = [] (bool b) { return juce::String (b ? "ja" : "nein"); };

        {
            auto p = std::make_unique<Prozessor>();
            const bool quadAn  = p->checkBusesLayoutSupported (layout (quad, quad));
            const bool fuenfAn = p->checkBusesLayoutSupported (layout (fuenf1, fuenf1));
            const bool vierAn  = p->checkBusesLayoutSupported (layout (vier, vier));
            const bool gesetzt = p->setBusesLayout (layout (quad, quad));
            pruefe (! quadAn && ! fuenfAn && ! vierAn && ! gesetzt
                        && p->getTotalNumInputChannels() == 2 && p->getTotalNumOutputChannels() == 2,
                    "mehrkanallayout_wird_abgelehnt (M-27): Quadrophonie (k40Music), 5.1 und vier diskrete Kanaele "
                    "liefern ein Nein, setBusesLayout scheitert, der Prozessor bleibt bei zwei Kanaelen",
                    "angenommen: Quad " + jaNein (quadAn) + ", 5.1 " + jaNein (fuenfAn) + ", 4 diskret " + jaNein (vierAn)
                        + ", gesetzt " + jaNein (gesetzt) + ", Kanaele " + juce::String (p->getTotalNumInputChannels())
                        + "/" + juce::String (p->getTotalNumOutputChannels()));
        }
        {
            auto p = std::make_unique<Prozessor>();
            const bool stereoAn    = p->checkBusesLayoutSupported (layout (stereo, stereo));
            const bool monoAn      = p->checkBusesLayoutSupported (layout (mono, mono));
            const bool monoGesetzt = p->setBusesLayout (layout (mono, mono));
            p->setRateAndBufferSizeDetails (48000.0, 256);
            p->prepareToPlay (48000.0, 256);
            juce::MidiBuffer midi;
            juce::Random w (28);
            juce::AudioBuffer<float> b (1, 256), kopie (1, 256);
            fuelle (b, w);
            kopie.makeCopyOf (b);
            p->processBlock (b, midi);
            const bool passthrough = std::memcmp (b.getReadPointer (0), kopie.getReadPointer (0), 256 * sizeof (float)) == 0;
            auto z = p->bestaetigterZustand();
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            setzeHoerbaresBand (z);
            const auto e = setze (*p, z);
            bool gerechnet = false, endlich = true;
            for (int blk = 0; blk < 8; ++blk)
            {
                fuelle (b, w);
                kopie.makeCopyOf (b);
                p->processBlock (b, midi);
                if (std::memcmp (b.getReadPointer (0), kopie.getReadPointer (0), 256 * sizeof (float)) != 0)
                    gerechnet = true;
                for (int n = 0; n < 256; ++n)
                    if (! std::isfinite (b.getSample (0, n))) endlich = false;
            }
            pruefe (stereoAn && monoAn && monoGesetzt && p->getTotalNumInputChannels() == 1 && passthrough
                        && e.ausgang == tx::Ausgang::commit && gerechnet && endlich,
                    "mono_und_stereo_bleiben_angenommen (M-28, Regressionswache): Stereo und Mono liefern ein Ja; im "
                    "Monobus bleibt der Passthrough bitgleich, und das hoerbare Band rechnet auf dem einen Kanal endlich",
                    "Stereo " + jaNein (stereoAn) + ", Mono " + jaNein (monoAn) + ", Mono gesetzt " + jaNein (monoGesetzt)
                        + ", Passthrough bitgleich " + jaNein (passthrough) + ", Band rechnet " + jaNein (gerechnet));
        }
        {
            // NAK-311 311/M-11, Teilfall M-19 (Mono): der engagiert-neutrale
            // Kern schreibt auch im Monobus keinen Sample. Material wie
            // 311/M-11 auf Kanal 0. Der Riegel zaehlt jeden Wert zweimal, weil
            // `dryR` im Monobus den linken Kanal traegt und der Riegel beide
            // Komponenten getrennt zaehlt (DspKern::verarbeiteStueck).
            auto p = std::make_unique<Prozessor>();
            const bool monoGesetzt = p->setBusesLayout (layout (mono, mono));
            p->setRateAndBufferSizeDetails (48000.0, 2048);
            p->prepareToPlay (48000.0, 2048);
            auto z = p->bestaetigterZustand();
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            const auto e = setze (*p, z);
            Material material;
            material.kanaele = 1;
            material.bitmuster = true;
            material.nichtEndlich = true;
            juce::Random w (19);
            const auto lauf = fahreNull (*p, 1000, w, 0, 0, material, 0);
            const auto zaehler = p->dspKernFuerTest().nichtEndlicheEingaenge();
            const bool imMono = monoGesetzt && p->getTotalNumInputChannels() == 1 && e.ausgang == tx::Ausgang::commit;
            pruefe (imMono && lauf.nichtEndlichGesetzt == 6 && lauf.nichtEndlichAbweichend == 0 && zaehler == 12,
                    "311/M-19 neutral_engagiert_schreibt_im_monobus_keinen_sample (Teilfall von 311/M-11), Wachmarke: "
                    "Wachmarke, NaN und +-Inf kommen auf Kanal 0 bytegleich heraus; der Riegel zaehlt 12, zwei je Wert",
                    juce::String (lauf.nichtEndlichAbweichend) + " von " + juce::String (lauf.nichtEndlichGesetzt)
                        + " veraendert, Zaehler " + juce::String ((juce::int64) zaehler));
            pruefe (imMono && lauf.abweichend == 0 && lauf.bitmusterGesetzt > 0 && lauf.bitmusterAbweichend == 0,
                    "311/M-19 neutral_engagiert_schreibt_im_monobus_keinen_sample (Teilfall von 311/M-11), Bitmuster: "
                    "Rauschen ab Sample 0 und das Bitmuster ab Sample 512 kommen auf Kanal 0 bytegleich heraus",
                    juce::String (lauf.samples) + " Samples, Rauschen " + juce::String (lauf.abweichend)
                        + " abweichend, Muster " + juce::String (lauf.bitmusterAbweichend) + " von "
                        + juce::String (lauf.bitmusterGesetzt) + " veraendert");
        }
        {
            auto p = std::make_unique<Prozessor>();
            const bool monoStereo = p->checkBusesLayoutSupported (layout (mono, stereo));
            const bool stereoMono = p->checkBusesLayoutSupported (layout (stereo, mono));
            const bool ausStereo  = p->checkBusesLayoutSupported (layout (aus, stereo));
            const bool stereoAus  = p->checkBusesLayoutSupported (layout (stereo, aus));
            const bool beideAus   = p->checkBusesLayoutSupported (layout (aus, aus));
            pruefe (! monoStereo && ! stereoMono && ! ausStereo && ! stereoAus && ! beideAus,
                    "ungleiche_und_deaktivierte_layouts_bleiben_abgelehnt (M-29, Regressionswache): Mono->Stereo, "
                    "Stereo->Mono und jeder deaktivierte Hauptbus liefern ein Nein",
                    "angenommen: Mono->Stereo " + jaNein (monoStereo) + ", Stereo->Mono " + jaNein (stereoMono)
                        + ", aus->Stereo " + jaNein (ausStereo) + ", Stereo->aus " + jaNein (stereoAus)
                        + ", aus->aus " + jaNein (beideAus));
        }
        {
            // NAK-311 311/M-62 (R-311-3, T3-16-04): Layout setzen <-> Programm
            // erneuern. Der Zustand traegt Auto-Gain an und das Band der
            // Phase 16 - ein Low-Shelf 20 kHz / +12 dB / Q 0,707 im Modus
            // `right`. Im Monobus schreibt der Kern nur Kanal 0; dort aendert
            // dieses Band nichts, also gleicht Auto-Gain auch nichts aus.
            // HEUTE ROT: die Phase 16 mass im Monobus -9,177564 dB.
            // Gefahren wird Mono -> Stereo -> Mono; nach JEDEM prepareToPlay
            // muessen Kern UND Bericht den Wert der neuen Kanalzahl tragen -
            // die zwei Lesestellen duerfen nie auseinanderlaufen.
            // Teilfall 311/M-63 (Regressionswache): der Kanalwunsch bleibt.
            const auto mitBand = [] (param::DspSatz z)
            {
                z.werte[(size_t) param::kIndexEqEnabled].b = true;
                z.werte[(size_t) iGlobal ("v2.global.auto_gain")].b = true;
                z.werte[(size_t) param::indexOccupied (0)].b = true;
                z.werte[(size_t) iBand (0, param::kEnabled)].b = true;
                z.werte[(size_t) iBand (0, param::kType)].enumIndex = (int) nakama::dsp::Filtertyp::lowShelf;
                z.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 20000.0;
                z.werte[(size_t) iBand (0, param::kQ)].zahl = 0.707;
                z.werte[(size_t) iBand (0, param::kGainDb)].zahl = 12.0;
                z.werte[(size_t) iBand (0, param::kChannelMode)].enumIndex = (int) nakama::dsp::Kanalmodus::right;
                return z;
            };

            /** Faehrt einen 250-Hz-Sinus ueber den EINEN Kanal des Monobusses
                und gibt den float-Ausgang zurueck. */
            const auto fahreMono = [] (Prozessor& p, int bloecke)
            {
                juce::MidiBuffer midi;
                juce::AudioBuffer<float> b (1, 512);
                std::vector<float> aus;
                long long n0 = 0;
                for (int blk = 0; blk < bloecke; ++blk)
                {
                    for (int i = 0; i < 512; ++i)
                        b.setSample (0, i, (float) (0.25 * std::sin (2.0 * 3.14159265358979323846
                                                                    * 250.0 * (double) (n0 + i) / 48000.0)));
                    p.processBlock (b, midi);
                    for (int i = 0; i < 512; ++i) aus.push_back (b.getSample (0, i));
                    n0 += 512;
                }
                return aus;
            };

            const auto fahreStereo = [] (Prozessor& p, int bloecke)
            {
                juce::MidiBuffer midi;
                juce::AudioBuffer<float> b (2, 512);
                b.clear();
                for (int blk = 0; blk < bloecke; ++blk) p.processBlock (b, midi);
            };

            const auto bericht = [] (Prozessor& p)
            {
                tx::DspBericht r;
                juce::String grund;
                const bool ok = p.dspBericht (r, grund);
                return std::pair<bool, double> { ok, r.autoGainDb };
            };

            // --- Mono ---------------------------------------------------------
            auto p = std::make_unique<Prozessor>();
            const bool monoGesetzt = p->setBusesLayout (layout (mono, mono));
            p->setRateAndBufferSizeDetails (48000.0, 512);
            p->prepareToPlay (48000.0, 512);
            setze (*p, mitBand (p->bestaetigterZustand()));

            DirtyZaehler dirty;
            p->addListener (&dirty);
            const auto monoMit = fahreMono (*p, 94);          // 48 128 Samples
            const int dirtyNachCommit = dirty.nichtParameter;

            juce::MemoryBlock bytesVorher;
            p->getStateInformation (bytesVorher);
            const auto revisionVorher = p->stateRevision();

            auto ohne = std::make_unique<Prozessor>();
            ohne->setBusesLayout (layout (mono, mono));
            ohne->setRateAndBufferSizeDetails (48000.0, 512);
            ohne->prepareToPlay (48000.0, 512);
            auto zOhne = mitBand (ohne->bestaetigterZustand());
            zOhne.werte[(size_t) iGlobal ("v2.global.auto_gain")].b = false;
            setze (*ohne, zOhne);
            const auto monoOhne = fahreMono (*ohne, 94);

            const double monoKern    = p->dspKernFuerTest().autoGainDb();
            const auto   monoBericht = bericht (*p);
            const bool   monoBitgleich = monoMit.size() == monoOhne.size()
                                      && std::memcmp (monoMit.data(), monoOhne.data(),
                                                      monoMit.size() * sizeof (float)) == 0;

            pruefe (monoGesetzt && p->getTotalNumInputChannels() == 1
                        && monoKern == 0.0 && ! std::signbit (monoKern)
                        && monoBericht.first && monoBericht.second == 0.0 && ! std::signbit (monoBericht.second)
                        && monoBitgleich,
                    "311/M-62 kanalzahlwechsel_erneuert_ableitung_und_bericht (R-311-3), Mono: der Kern und "
                    "dspBericht melden exakt +0,0, und der Ausgang mit Auto-Gain ist bytegleich zum Lauf ohne",
                    "Kern " + juce::String (monoKern, 12) + " dB, Bericht "
                        + juce::String (monoBericht.second, 12) + " dB, Ausgang bytegleich "
                        + jaNein (monoBitgleich) + " (Phase 16 mass -9,177564 dB)");

            // --- Stereo -------------------------------------------------------
            const bool stereoGesetzt = p->setBusesLayout (layout (stereo, stereo));
            p->setRateAndBufferSizeDetails (48000.0, 512);
            p->prepareToPlay (48000.0, 512);
            fahreStereo (*p, 4);
            const double stereoKern    = p->dspKernFuerTest().autoGainDb();
            const auto   stereoBericht = bericht (*p);

            auto ref = std::make_unique<Prozessor>();   // ein Stereokern mit demselben Zustand
            ref->setRateAndBufferSizeDetails (48000.0, 512);
            ref->prepareToPlay (48000.0, 512);
            setze (*ref, mitBand (ref->bestaetigterZustand()));
            const double refKern = ref->dspKernFuerTest().autoGainDb();

            pruefe (stereoGesetzt && p->getTotalNumInputChannels() == 2
                        && std::memcmp (&stereoKern, &refKern, sizeof (double)) == 0
                        && stereoBericht.first
                        && std::memcmp (&stereoBericht.second, &refKern, sizeof (double)) == 0
                        && stereoKern != 0.0,
                    "311/M-62 kanalzahlwechsel_erneuert_ableitung_und_bericht (R-311-3), Stereo: Kern und "
                    "Bericht tragen den Zweikanalwert, bitgleich zu einem Stereokern mit demselben Zustand",
                    "Kern " + juce::String (stereoKern, 12) + " dB, Bericht "
                        + juce::String (stereoBericht.second, 12) + " dB, Referenz "
                        + juce::String (refKern, 12) + " dB");

            // --- und zurueck in den Monobus ------------------------------------
            const bool zurueck = p->setBusesLayout (layout (mono, mono));
            p->setRateAndBufferSizeDetails (48000.0, 512);
            p->prepareToPlay (48000.0, 512);
            fahreMono (*p, 4);
            const double zurueckKern    = p->dspKernFuerTest().autoGainDb();
            const auto   zurueckBericht = bericht (*p);
            pruefe (zurueck && p->getTotalNumInputChannels() == 1
                        && zurueckKern == 0.0 && ! std::signbit (zurueckKern)
                        && zurueckBericht.first && zurueckBericht.second == 0.0
                        && ! std::signbit (zurueckBericht.second),
                    "311/M-62 kanalzahlwechsel_erneuert_ableitung_und_bericht (R-311-3), zurueck in Mono: "
                    "Kern und Bericht stehen wieder auf exakt +0,0",
                    "Kern " + juce::String (zurueckKern, 12) + " dB, Bericht "
                        + juce::String (zurueckBericht.second, 12) + " dB");

            // --- 311/M-63: der Kanalwunsch ist State, die Kanalzahl nicht -----
            juce::MemoryBlock bytesNachher;
            p->getStateInformation (bytesNachher);
            p->removeListener (&dirty);
            const auto& z = p->bestaetigterZustand();
            const bool modusBleibt = z.werte[(size_t) iBand (0, param::kChannelMode)].enumIndex
                                     == (int) nakama::dsp::Kanalmodus::right;
            pruefe (modusBleibt && bytesVorher == bytesNachher
                        && p->stateRevision() == revisionVorher
                        && dirty.nichtParameter == dirtyNachCommit,
                    "311/M-63 kanalwunsch_ueberlebt_den_layoutwechsel (Teilfall von 311/M-62, "
                    "Regressionswache): channel_mode bleibt `right`, die Statebytes sind gleich, keine "
                    "Revision, kein Host-Dirty",
                    "channel_mode right " + jaNein (modusBleibt) + ", Bytes gleich "
                        + jaNein (bytesVorher == bytesNachher) + " (" + juce::String ((int) bytesVorher.getSize())
                        + " Bytes), Revision " + juce::String ((juce::int64) p->stateRevision())
                        + ", Dirty-Meldungen ueber die drei Wechsel "
                        + juce::String (dirty.nichtParameter - dirtyNachCommit));
        }
    }

    // -- 13. NAK-311 M-01: DAZ/FTZ-Messung mit dem Bitmuster ---------------
    // Messung nach R-311-2 (Manifest NAK-311 §6.1, §7.2 Punkt 2), KEIN
    // Rotbeweis und KEINE Bitgleichheitspruefung: der echte Prozessor faehrt
    // die Zustaende (a) eq an, sonst Default (SONDE-015 M-02), (b) eq an,
    // Mix 0, Output-Trim 0 dB, Input-Trim +9 dB, +12-dB-Bell (SONDE-015 M-33)
    // und (c) eq aus als Gegenprobe, je Stereo und Mono bei 44,1, 48 und
    // 96 kHz. Nach kFadeSamples + kRampeSamples Samples Rauschen - Engagier-
    // Fade und Rampen sind vorbei - folgt ein Block aus dem Bitmuster; je
    // Muster stehen Eingangs- und Ausgangsbits in der Ausgabe. Geprueft wird
    // nur, dass jede Kombination in ihrem Zustand lief (Wache); ob Muster
    // veraendert herauskommen, haelt das Manifest fest (Ausgang (a) oder (b)).
    abschnitt ("13. NAK-311 311/M-01 daz_ftz_messung_bitmuster");
    {
#if defined (_M_X64) || defined (__x86_64__)
        {
            juce::ScopedNoDenormals keineDenormals;
            const auto csr = (std::uint32_t) _mm_getcsr();
            std::cout << "  311/M-01 MXCSR unter juce::ScopedNoDenormals " << hex32 (csr) << " (FTZ " << ((csr >> 15) & 1u)
                      << ", DAZ " << ((csr >> 6) & 1u) << ")" << std::endl;
        }
#endif
        const std::int64_t einschwingen = nakama::dsp::kFadeSamples + nakama::dsp::kRampeSamples;
        const int musterZahl = (int) std::size (kBitmuster);
        int kombinationen = 0;
        int veraendertJeZustand[3] = { 0, 0, 0 };
        for (const char zustand : { 'a', 'b', 'c' })
            for (const int kanaele : { 2, 1 })
                for (const double rate : { 44100.0, 48000.0, 96000.0 })
                {
                    auto p = std::make_unique<Prozessor>();
                    const auto satz = kanaele == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo();
                    juce::AudioProcessor::BusesLayout layout;
                    layout.inputBuses.add (satz);
                    layout.outputBuses.add (satz);
                    const bool layoutGesetzt = p->setBusesLayout (layout);
                    p->setRateAndBufferSizeDetails (rate, 2048);
                    p->prepareToPlay (rate, 2048);

                    bool eingestellt = true;
                    if (zustand != 'c')
                    {
                        auto z = p->bestaetigterZustand();
                        z.werte[(size_t) param::kIndexEqEnabled].b = true;
                        if (zustand == 'b')
                        {
                            setzeHoerbaresBand (z);
                            z.werte[(size_t) iGlobal ("v1.global.input_trim_db")].zahl = 9.0;
                            z.werte[(size_t) param::kIndexMix].zahl = 0.0;
                        }
                        eingestellt = setze (*p, z).ausgang == tx::Ausgang::commit;
                    }

                    juce::MidiBuffer midi;
                    juce::Random wuerfel (311 + (juce::int64) rate + kanaele);
                    std::int64_t gefahren = 0;
                    while (gefahren < einschwingen)
                    {
                        juce::AudioBuffer<float> rauschen (kanaele, 256);
                        fuelle (rauschen, wuerfel);
                        p->processBlock (rauschen, midi);
                        gefahren += rauschen.getNumSamples();
                    }
                    juce::AudioBuffer<float> block (kanaele, musterZahl);
                    for (int k = 0; k < kanaele; ++k)
                        for (int n = 0; n < musterZahl; ++n)
                            block.setSample (k, n, std::bit_cast<float> (kBitmuster[n]));
                    p->processBlock (block, midi);

                    int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
                    auto& kern = p->dspKernFuerTest();
                    kern.gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
                    bool imZustand = false;
                    if (zustand == 'c')
                        imZustand = aktiv < 0 && ! p->bestaetigterZustand().werte[(size_t) param::kIndexEqEnabled].b;
                    else if (aktiv >= 0)
                    {
                        const auto& pr = kern.pool().bank (aktiv).programm;
                        imZustand = eingestellt && pr.eqEngagiert && ! pr.hardBypass
                                 && (zustand == 'a' ? (pr.mix == 1.0 && pr.inputTrimLin == 1.0)
                                                    : (pr.mix == 0.0 && pr.inputTrimLin > 1.0 && pr.outputTrimLin == 1.0));
                    }

                    const std::string name = std::string (1, zustand) + (kanaele == 2 ? " stereo " : " mono ")
                                           + std::to_string ((int) rate) + " Hz";
                    int veraendert = 0;
                    for (int n = 0; n < musterZahl; ++n)
                    {
                        std::string aus;
                        bool anders = false;
                        for (int k = 0; k < kanaele; ++k)
                        {
                            const auto bits = std::bit_cast<std::uint32_t> (block.getSample (k, n));
                            aus += " K" + std::to_string (k) + " " + hex32 (bits);
                            anders = anders || bits != kBitmuster[n];
                        }
                        if (anders) ++veraendert;
                        std::cout << "  311/M-01 " << name << " ab Sample " << gefahren << ": ein " << hex32 (kBitmuster[n])
                                  << " aus" << aus << (anders ? "  VERAENDERT" : "  bytegleich") << std::endl;
                    }
                    veraendertJeZustand[zustand - 'a'] += veraendert;

                    const bool lief = layoutGesetzt && p->getTotalNumInputChannels() == kanaele
                                   && p->getTotalNumOutputChannels() == kanaele && imZustand && gefahren >= einschwingen;
                    if (lief) ++kombinationen;
                    pruefe (lief, "311/M-01 lief: Zustand " + juce::String (name) + ", Bitmuster ab Sample "
                                      + juce::String (gefahren),
                            juce::String (veraendert) + " von " + juce::String (musterZahl) + " Mustern veraendert");
                }
        std::cout << "  311/M-01 Summe veraenderter Muster je Zustand (je 6 Kombinationen zu " << musterZahl
                  << " Mustern): a " << veraendertJeZustand[0] << ", b " << veraendertJeZustand[1] << ", c "
                  << veraendertJeZustand[2] << std::endl;
        pruefe (kombinationen == 18,
                "daz_ftz_messung_bitmuster (NAK-311 311/M-01): alle 18 Kombinationen liefen - Zustaende a, b, c; Stereo "
                "und Mono; 44,1, 48 und 96 kHz; je " + juce::String (musterZahl) + " Muster ausgegeben",
                juce::String (kombinationen) + " von 18");
    }

    // -- 14. NAK-311 M-11: der engagiert-neutrale Kern schreibt keinen Sample --
    // Manifest NAK-311 §6.1 (M-11, M-15 bis M-17, M-21), §9 F-3: nicht endliche
    // Eingaenge schreibt der neutrale Kern nicht, sie kommen bytegleich heraus
    // wie im Passthrough (SONDE-015 M-50); der Riegel laeuft fuer den inneren
    // Weg weiter und zaehlt sie (M-49), der Tap post_committed traegt 0,0. Die
    // Wachmarke haelt das unabhaengig von DAZ: eine Rueckwandlung machte sie
    // ruhig. Gegenfaelle: ausgeschaltet (M-15), Hard-Bypass (M-16) und ein
    // wirksames Band (M-17), bei dem die Neutralpruefung nicht greift.
    abschnitt ("14. NAK-311 311/M-11 neutral_engagiert_schreibt_keinen_sample (M-11, M-15 bis M-17, M-21)");
    {
        Material material;
        material.bitmuster = true;
        material.nichtEndlich = true;
        const auto detail = [] (const Nulllauf& l, std::uint64_t zaehler)
        {
            return "nicht endlich " + juce::String (l.nichtEndlichAbweichend) + " von " + juce::String (l.nichtEndlichGesetzt)
                 + " veraendert, " + juce::String (l.wachmarkenBytegleich) + " Wachmarken bytegleich, "
                 + juce::String (l.nichtEndlichEndlichHeraus) + " endlich heraus, Tap 0,0 an "
                 + juce::String (l.nichtEndlichTapNull) + ", Zaehler " + juce::String ((juce::int64) zaehler)
                 + "; Rauschen " + juce::String (l.abweichend) + " abweichend (ab Sample 512: "
                 + juce::String (l.rauschenAbweichendAbMaterial) + "), Muster " + juce::String (l.bitmusterAbweichend)
                 + " von " + juce::String (l.bitmusterGesetzt) + " veraendert";
        };

        for (const double rate : { 44100.0, 48000.0, 96000.0 })
        {
            auto p = vorbereitet (rate, 2048);
            auto z = p->bestaetigterZustand();
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            const auto e = setze (*p, z);
            juce::Random wuerfel ((juce::int64) rate + 11);
            const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 0, material, 0);
            const auto zaehler = p->dspKernFuerTest().nichtEndlicheEingaenge();
            pruefe (e.ausgang == tx::Ausgang::commit && lauf.nichtEndlichGesetzt == 6 && lauf.nichtEndlichAbweichend == 0
                        && zaehler == 6 && lauf.nichtEndlichTapNull == 6,
                    "311/M-11 neutral_engagiert_schreibt_keinen_sample bei " + juce::String (rate, 0)
                        + " Hz: Wachmarke, NaN und +-Inf kommen bytegleich heraus, der Riegel zaehlt 6, der Tap post_committed traegt dort 0,0",
                    detail (lauf, zaehler));
        }

        {
            // M-15: ausgeschaltet - der fruehe Rueckweg, kein Zaehler.
            auto p = vorbereitet (48000.0, 2048);
            juce::Random wuerfel (15);
            const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 0, material);
            const auto zaehler = p->dspKernFuerTest().nichtEndlicheEingaenge();
            pruefe (lauf.abweichend == 0 && lauf.bitmusterGesetzt > 0 && lauf.bitmusterAbweichend == 0
                        && lauf.nichtEndlichGesetzt == 6 && lauf.nichtEndlichAbweichend == 0 && zaehler == 0,
                    "311/M-11 Teilfall M-15 ausgeschaltet_bleibt_alles_bytegleich: Rauschen, Bitmuster, Wachmarke, NaN und +-Inf, kein Zaehler steigt",
                    detail (lauf, zaehler));
        }
        {
            // M-16: Hard-Bypass mit +12-dB-Bell und Width 1,7 dahinter.
            auto p = vorbereitet (48000.0, 2048);
            auto z = p->bestaetigterZustand();
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
            z.werte[(size_t) iGlobal ("v1.global.width")].zahl = 1.7;
            setzeHoerbaresBand (z);
            const auto e = setze (*p, z);
            juce::Random wuerfel (16);
            const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 0, material);
            const auto zaehler = p->dspKernFuerTest().nichtEndlicheEingaenge();
            pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && lauf.bitmusterGesetzt > 0
                        && lauf.bitmusterAbweichend == 0 && lauf.nichtEndlichGesetzt == 6 && lauf.nichtEndlichAbweichend == 0
                        && zaehler == 0,
                    "311/M-11 Teilfall M-16 hard_bypass_bleibt_alles_bytegleich: +12-dB-Bell und Width 1,7 dahinter, kein Zaehler steigt",
                    detail (lauf, zaehler));
        }
        {
            // M-17: ein wirksames Band - die Neutralpruefung greift nicht. Der
            // Riegel setzt die nicht endlichen Werte auf 0,0, das Band liefert
            // daraus seinen Zustandsrest, und geschrieben wird ein endlicher Wert.
            auto p = vorbereitet (48000.0, 2048);
            auto z = p->bestaetigterZustand();
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            setzeHoerbaresBand (z);
            const auto e = setze (*p, z);
            juce::Random wuerfel (17);
            const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 0, material);
            const auto zaehler = p->dspKernFuerTest().nichtEndlicheEingaenge();
            pruefe (e.ausgang == tx::Ausgang::commit && lauf.rauschenAbweichendAbMaterial > 0 && lauf.nichtEndlichGesetzt == 6
                        && lauf.nichtEndlichEndlichHeraus == 6 && lauf.wachmarkenBytegleich == 0 && zaehler == 6,
                    "311/M-11 Teilfall M-17 wirksames_band_schreibt: ab Sample 512 kommt Rauschen veraendert heraus, an allen sechs "
                    "nicht endlichen Stellen ein endlicher Wert, nie die Wachmarke; der Riegel zaehlt 6",
                    detail (lauf, zaehler));
        }
    }

    // -- 15. NAK-311 311/M-132: unter 44,1 kHz bleibt der EQ neutral -------
    // Manifest NAK-311 §39.4 (311/M-132 bis 311/M-139), Regeln R-311-16 und
    // R-311-20, Abnahme U47 vom 19.09.2026 ("Weg 1 gilt: unter 44,1 kHz ist
    // der EQ nicht unterstuetzt"). Unter 44 100 Hz bereitet `bereiteVor`
    // nichts vor, `verarbeiteStueck` kehrt vor dem ersten Sample zurueck -
    // der Weg des AUSGESCHALTETEN EQ -, und der Grund steht als Zahl am Kern.
    // Genau 44 100 Hz bleibt unterstuetzt (Gegenfall 311/M-133).
    abschnitt ("15. NAK-311 311/M-132 unter_44_1_khz_bleibt_der_eq_neutral (M-132 bis M-134, M-136, M-137, M-139)");
    {
        namespace dspn = nakama::dsp;

        /** Der Pruefling der Phase 16: Bell 14 kHz, +12 dB, Q 4, Auto-Gain
            an. Bei 32 kHz liegen vier der 121 Gitterstellen des Ausgleichs
            ueber Nyquist, bei 22,05 kHz elf - heute rechnet der Kern damit
            (gemessen -0,397113 dB statt -0,234376 dB). */
        const auto mitBell14k = [] (param::DspSatz z)
        {
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            z.werte[(size_t) iGlobal ("v2.global.auto_gain")].b = true;
            z.werte[(size_t) param::indexOccupied (0)].b = true;
            z.werte[(size_t) iBand (0, param::kEnabled)].b = true;
            z.werte[(size_t) iBand (0, param::kType)].enumIndex = (int) dspn::Filtertyp::bell;
            z.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 14000.0;
            z.werte[(size_t) iBand (0, param::kQ)].zahl      = 4.0;
            z.werte[(size_t) iBand (0, param::kGainDb)].zahl = 12.0;
            return z;
        };
        const auto jaNein = [] (bool b) { return juce::String (b ? "ja" : "nein"); };
        const auto monoBus = [] (Prozessor& p)
        {
            juce::AudioProcessor::BusesLayout l;
            l.inputBuses .add (juce::AudioChannelSet::mono());
            l.outputBuses.add (juce::AudioChannelSet::mono());
            return p.setBusesLayout (l);
        };

        // ── 311/M-132 mit den Teilfaellen 311/M-137 und 311/M-139 ──────────
        for (const double rate : { 32000.0, 22050.0 })
            for (const int kanaele : { 2, 1 })
            {
                auto p = std::make_unique<Prozessor>();
                const bool layoutOk = kanaele == 1 ? monoBus (*p) : true;
                p->setRateAndBufferSizeDetails (rate, 2048);
                p->prepareToPlay (rate, 2048);
                const auto hallo = p->v3HelloFuerTest();
                const auto e = setze (*p, mitBell14k (p->bestaetigterZustand()));
                juce::Random wuerfel ((juce::int64) rate + kanaele);
                Material material;
                material.kanaele    = kanaele;
                material.bitmuster  = true;
                material.nichtEndlich = true;
                const auto lauf = fahreNull (*p, 1000, wuerfel, 0, 0, material);
                auto& kern = p->dspKernFuerTest();
                const double abgelehnt = kern.abgelehnteSamplerateHz();
                const double autoGain  = kern.autoGainDb();
                const bool zaehlerRuhen = kern.nichtEndlicheEingaenge() == 0
                                       && kern.geheilteFilterzustaende() == 0
                                       && kern.verworfeneAnalyseframes() == 0;
                pruefe (e.ausgang == tx::Ausgang::commit && layoutOk
                            && lauf.abweichend == 0 && lauf.latenzNull
                            && lauf.bitmusterGesetzt > 0 && lauf.bitmusterAbweichend == 0
                            && lauf.nichtEndlichGesetzt == 6 && lauf.nichtEndlichAbweichend == 0
                            && lauf.wachmarkenBytegleich == 3 && zaehlerRuhen
                            && std::memcmp (&abgelehnt, &rate, sizeof (double)) == 0
                            && kern.samplerate() == 0.0
                            && autoGain == 0.0 && ! std::signbit (autoGain)
                            && kern.busKanaele() == 2,
                        "311/M-132 unter_44_1_khz_bleibt_der_eq_neutral bei " + juce::String (rate, 0)
                            + " Hz, " + juce::String (kanaele) + "-Kanal-Bus: Bell 14 kHz +12 dB Q 4 mit Auto-Gain "
                            "an, 1000 Bloecke von 1 bis 2048 Samples - Rauschen, Bitmuster, Wachmarke, NaN und "
                            "+-Inf kommen bytegleich heraus, kein Zaehler steigt, der Kern ist unvorbereitet, "
                            "meldet die abgelehnte Rate und exakt +0,0 dB Ausgleich, die Kanalzahl faellt auf 2",
                        juce::String (lauf.samples) + " Samples, Rauschen " + juce::String (lauf.abweichend)
                            + " abweichend, Muster " + juce::String (lauf.bitmusterAbweichend) + " von "
                            + juce::String (lauf.bitmusterGesetzt) + ", nicht endlich "
                            + juce::String (lauf.nichtEndlichAbweichend) + " von "
                            + juce::String (lauf.nichtEndlichGesetzt) + " veraendert, "
                            + juce::String (lauf.wachmarkenBytegleich) + " Wachmarken bytegleich, Zaehler "
                            + juce::String ((juce::int64) kern.nichtEndlicheEingaenge()) + "/"
                            + juce::String ((juce::int64) kern.geheilteFilterzustaende()) + "/"
                            + juce::String ((juce::int64) kern.verworfeneAnalyseframes())
                            + ", abgelehnteSamplerateHz " + juce::String (abgelehnt, 6)
                            + ", Kern-Samplerate " + juce::String (kern.samplerate(), 6)
                            + ", autoGainDb " + juce::String (autoGain, 12) + " (Vorzeichenbit "
                            + juce::String (std::signbit (autoGain) ? 1 : 0) + "), busKanaele "
                            + juce::String (kern.busKanaele()));

                // 311/M-137 (Regressionswache): Messung und Analyse sind vom
                // Entscheid NICHT beruehrt - der Analyseweg kennt keine
                // Mindestrate und traegt weiter die gemessenen Hostwerte.
                pruefe (hallo.samplerate == rate && hallo.blockSize == 2048 && hallo.channels == kanaele,
                        "311/M-137 analyse_bleibt_unberuehrt (Teilfall von 311/M-132, Regressionswache) bei "
                            + juce::String (rate, 0) + " Hz, " + juce::String (kanaele)
                            + "-Kanal-Bus: v3Samplerate, v3BlockSize und v3Channels tragen die gemessenen "
                              "Werte des Hosts, nicht 0",
                        "v3Samplerate " + juce::String (hallo.samplerate, 1) + ", v3BlockSize "
                            + juce::String (hallo.blockSize) + ", v3Channels " + juce::String (hallo.channels)
                            + ", Bloecke ohne Audio in der Analysequeue "
                            + juce::String ((juce::int64) p->analyseDropsOhneAudioFuerTest()));
            }

        // ── 311/M-139: der fruehe Rueckweg bei jeder Blockgroesse ──────────
        // Kein Sample gelesen oder geschrieben, auch nicht ueber `maxBlock`
        // (4096 bei 2048): der Kern ist unvorbereitet, die Stueckelung
        // (M-48) greift gar nicht, und kein Analyseframe gilt als verworfen.
        {
            auto p = vorbereitet (32000.0, 2048);
            const auto e = setze (*p, mitBell14k (p->bestaetigterZustand()));
            juce::MidiBuffer midi;
            juce::Random w (139);
            bool alleBytegleich = true;
            int gefahren = 0;
            for (const int groesse : { 1, 255, 256, 257, 2048, 4096 })
            {
                juce::AudioBuffer<float> b (2, groesse), kopie (2, groesse);
                fuelle (b, w);
                if (groesse > 16) b.setSample (0, 16, std::bit_cast<float> (kWachmarke));
                kopie.makeCopyOf (b);
                p->processBlock (b, midi);
                for (int k = 0; k < 2; ++k)
                    if (std::memcmp (b.getReadPointer (k), kopie.getReadPointer (k),
                                     (size_t) groesse * sizeof (float)) != 0)
                        alleBytegleich = false;
                ++gefahren;
            }
            auto& kern = p->dspKernFuerTest();
            pruefe (e.ausgang == tx::Ausgang::commit && alleBytegleich && gefahren == 6
                        && p->getLatencySamples() == 0 && p->getTailLengthSeconds() == 0.0
                        && kern.nichtEndlicheEingaenge() == 0 && kern.geheilteFilterzustaende() == 0
                        && kern.verworfeneAnalyseframes() == 0 && kern.tapLaenge() == 0,
                    "311/M-139 frueher_rueckweg_bei_jeder_blockgroesse (Teilfall von 311/M-132) bei 32 kHz: "
                    "Bloecke 1, 255, 256, 257, 2048 und 4096 (ueber maxBlock 2048) kommen mit Wachmarke "
                    "bytegleich heraus, kein Tap gefuellt, kein Zaehler bewegt, 0 Samples Latenz, Tail 0,0 s",
                    "Blockgroessen " + juce::String (gefahren) + ", bytegleich " + jaNein (alleBytegleich)
                        + ", Latenz " + juce::String (p->getLatencySamples()) + ", Tail "
                        + juce::String (p->getTailLengthSeconds()) + ", Taplaenge "
                        + juce::String (kern.tapLaenge()) + ", Zaehler "
                        + juce::String ((juce::int64) kern.nichtEndlicheEingaenge()) + "/"
                        + juce::String ((juce::int64) kern.geheilteFilterzustaende()) + "/"
                        + juce::String ((juce::int64) kern.verworfeneAnalyseframes()));
        }

        // ── 311/M-133 (Regressionswache): genau 44 100 Hz bleibt unterstuetzt
        // Der Fingerabdruck ist der Bitvergleich gegen den Basisstand des
        // Satzes: derselbe Lauf gibt dort dieselbe Zahl aus (§7.1,
        // Zeilenvergleich in `docs/beweise/roh/NAK-311-etappe5-a16-*`).
        for (const double rate : { 44100.0, 48000.0, 96000.0, 192000.0 })
        {
            auto p = vorbereitet (rate, 2048);
            const auto e = setze (*p, mitBell14k (p->bestaetigterZustand()));
            const auto abdruck = fnvAusgang (*p, 40, 512, (int) rate + 133);
            auto& kern = p->dspKernFuerTest();
            const double abgelehnt = kern.abgelehnteSamplerateHz();
            int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
            kern.gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
            const bool hoerbar = aktiv >= 0 && kern.pool().bank (aktiv).programm.eqEngagiert
                              && kern.autoGainDb() != 0.0;
            pruefe (e.ausgang == tx::Ausgang::commit && hoerbar
                        && kern.samplerate() == rate
                        && abgelehnt == 0.0 && ! std::signbit (abgelehnt),
                    "311/M-133 genau_44_1_khz_bleibt_unterstuetzt bei " + juce::String (rate, 0)
                        + " Hz: der Kern bereitet sich vor, das Band ist hoerbar, "
                          "abgelehnteSamplerateHz ist exakt +0,0, und der Ausgang traegt den Fingerabdruck "
                          "des Basisstands",
                    "Ausgang FNV-1a 0x" + juce::String::toHexString ((juce::int64) abdruck)
                        + ", Kern-Samplerate " + juce::String (kern.samplerate(), 1)
                        + ", autoGainDb " + juce::String (kern.autoGainDb(), 12)
                        + ", abgelehnteSamplerateHz " + juce::String (abgelehnt, 6) + " (Vorzeichenbit "
                        + juce::String (std::signbit (abgelehnt) ? 1 : 0) + ")");
        }

        // ── 311/M-134: der Zahlenrand steht an genau einer Stelle ──────────
        {
            const double unter1Ulp = std::nextafter (44100.0, 0.0);
            const double ueber1Ulp = std::nextafter (44100.0, 1.0e9);
            const double nanWert = std::numeric_limits<double>::quiet_NaN();
            const double inf     = std::numeric_limits<double>::infinity();
            const bool praedikat = dspn::kMinSamplerateHz == 44100.0
                && dspn::samplerateUnterstuetzt (44100.0)
                && dspn::samplerateUnterstuetzt (ueber1Ulp)
                && dspn::samplerateUnterstuetzt (48000.0)
                && ! dspn::samplerateUnterstuetzt (unter1Ulp)
                && ! dspn::samplerateUnterstuetzt (44099.0)
                && ! dspn::samplerateUnterstuetzt (32000.0)
                && ! dspn::samplerateUnterstuetzt (22050.0)
                && ! dspn::samplerateUnterstuetzt (8000.0)
                && ! dspn::samplerateUnterstuetzt (0.0)
                && ! dspn::samplerateUnterstuetzt (-48000.0)
                && ! dspn::samplerateUnterstuetzt (nanWert)
                && ! dspn::samplerateUnterstuetzt (inf)
                && ! dspn::samplerateUnterstuetzt (-inf);

            // Und dieselbe Kante am ECHTEN Prozessor: ein ULP unter der
            // Schranke wird abgelehnt, genau 44 100 Hz nicht.
            auto pUnter = vorbereitet (unter1Ulp, 512);
            auto pGenau = vorbereitet (44100.0, 512);
            const double abgelehntUnter = pUnter->dspKernFuerTest().abgelehnteSamplerateHz();
            const double abgelehntGenau = pGenau->dspKernFuerTest().abgelehnteSamplerateHz();
            pruefe (praedikat
                        && std::memcmp (&abgelehntUnter, &unter1Ulp, sizeof (double)) == 0
                        && pUnter->dspKernFuerTest().samplerate() == 0.0
                        && abgelehntGenau == 0.0 && ! std::signbit (abgelehntGenau)
                        && pGenau->dspKernFuerTest().samplerate() == 44100.0,
                    "311/M-134 groesser_oder_gleich_entscheidet (Teilfall von 311/M-132): das Praedikat "
                    "`samplerateUnterstuetzt` nimmt genau 44 100 Hz und den naechsten double darueber an und "
                    "lehnt 44 100 minus 1 ULP, 44 099, 32 000, 22 050, 8 000, 0, negativ, NaN und +-Inf ab; "
                    "am echten Prozessor faellt 44 100 minus 1 ULP, 44 100 nicht",
                    "44100 - 1 ULP = " + juce::String (unter1Ulp, 17) + " -> abgelehnt "
                        + juce::String (abgelehntUnter, 17) + ", 44100 -> abgelehnt "
                        + juce::String (abgelehntGenau, 6) + ", Kern-Samplerate "
                        + juce::String (pUnter->dspKernFuerTest().samplerate(), 1) + " / "
                        + juce::String (pGenau->dspKernFuerTest().samplerate(), 1)
                        + ", Praedikat vollstaendig " + jaNein (praedikat));
        }

        // ── 311/M-136 (Regressionswache): das verriegelte Fenster bleibt ───
        // R-311-12 gilt fuer seine zwei Fenster weiter: bei Rate 0, nicht
        // endlich, ueber 768 kHz oder `maxBlock` = 0 gibt `prepareToPlay`
        // weder Rate noch Kanalzahl weiter - der Kern behaelt seine letzte
        // Vorbereitung, und ABGELEHNT ist dabei nichts.
        {
            auto p = vorbereitet (48000.0, 512);
            const auto e = setze (*p, mitBell14k (p->bestaetigterZustand()));
            juce::MidiBuffer midi;
            juce::AudioBuffer<float> b (2, 512);
            juce::Random w (136);
            for (int i = 0; i < 4; ++i) { fuelle (b, w); p->processBlock (b, midi); }

            tx::DspBericht vorher;
            juce::String grund;
            const bool okVorher = p->dspBericht (vorher, grund);

            bool rateStehtStill = true, keineAblehnung = true;
            const double nanWert = std::numeric_limits<double>::quiet_NaN();
            const double inf     = std::numeric_limits<double>::infinity();
            for (const double rate : { 0.0, nanWert, inf, -inf, 1.0e300 })
            {
                p->prepareToPlay (rate, 512);
                if (p->dspKernFuerTest().samplerate() != 48000.0) rateStehtStill = false;
                if (p->dspKernFuerTest().abgelehnteSamplerateHz() != 0.0) keineAblehnung = false;
            }
            p->prepareToPlay (48000.0, 0);   // maxBlock 0, gueltige Rate
            if (p->dspKernFuerTest().samplerate() != 48000.0) rateStehtStill = false;
            if (p->dspKernFuerTest().abgelehnteSamplerateHz() != 0.0) keineAblehnung = false;

            tx::DspBericht nachher;
            const bool okNachher = p->dspBericht (nachher, grund);
            const bool berichtGleich = okVorher && okNachher
                && std::memcmp (&vorher.autoGainDb, &nachher.autoGainDb, sizeof (double)) == 0
                && vorher.revision == nachher.revision && vorher.jcs == nachher.jcs
                && nachher.abgelehnteSamplerateHz == 0.0;
            pruefe (e.ausgang == tx::Ausgang::commit && rateStehtStill && keineAblehnung && berichtGleich
                        && p->dspKernFuerTest().busKanaele() == 2,
                    "311/M-136 verriegeltes_fenster_bleibt_unterscheidbar (Teilfall von 311/M-132, "
                    "Regressionswache): `prepareToPlay` mit 0, NaN, +-Inf, 1e300 und mit maxBlock 0 laesst die "
                    "Vorbereitung von 48 kHz stehen, meldet KEINE abgelehnte Rate, und der dspBericht traegt "
                    "weiter den Stand der letzten Vorbereitung",
                    "Kern-Samplerate steht " + jaNein (rateStehtStill) + ", keine Ablehnung "
                        + jaNein (keineAblehnung) + ", Bericht auto_gain_db "
                        + juce::String (vorher.autoGainDb, 12) + " -> "
                        + juce::String (nachher.autoGainDb, 12) + ", abgelehnte Rate im Bericht "
                        + juce::String (nachher.abgelehnteSamplerateHz, 6));
        }

        // ── bereiteVor <-> releaseResources (Teilfall von 311/M-132) ───────
        // CLAUDE.md "Beziehungen mitpruefen": die gemerkte Rate faellt auf
        // BEIDEN Wegen zurueck. Ohne neue Vorbereitung gibt es keinen Bus,
        // dessen Rate noch abgelehnt waere.
        {
            auto p = vorbereitet (32000.0, 512);
            const double imFenster = p->dspKernFuerTest().abgelehnteSamplerateHz();

            // Zahlenraender ohne Audio dazwischen: zweimal dieselbe nicht
            // unterstuetzte Rate, dann eine ANDERE nicht unterstuetzte, dann
            // maxBlock 0 - der Melder folgt jedem Aufruf, ohne zu haengen.
            p->prepareToPlay (32000.0, 512);
            const double zweimal = p->dspKernFuerTest().abgelehnteSamplerateHz();
            p->prepareToPlay (22050.0, 512);
            const double gewechselt = p->dspKernFuerTest().abgelehnteSamplerateHz();
            p->prepareToPlay (32000.0, 0);            // maxBlock 0: R-311-12, kein Durchgriff
            const double mitMaxBlockNull = p->dspKernFuerTest().abgelehnteSamplerateHz();
            pruefe (zweimal == 32000.0 && gewechselt == 22050.0 && mitMaxBlockNull == 22050.0,
                    "311/M-132 Teilfall zwei_aufrufe_ohne_audio_und_ratenwechsel_im_fenster: zweimal dieselbe "
                    "nicht unterstuetzte Rate meldet dieselbe Zahl, eine andere meldet die neue, und "
                    "maxBlock 0 laesst den Stand stehen (R-311-12)",
                    "zweimal 32 kHz -> " + juce::String (zweimal, 1) + ", danach 22,05 kHz -> "
                        + juce::String (gewechselt, 1) + ", danach maxBlock 0 -> "
                        + juce::String (mitMaxBlockNull, 1));

            p->releaseResources();
            const double nachFreigabe = p->dspKernFuerTest().abgelehnteSamplerateHz();
            pruefe (imFenster == 32000.0 && nachFreigabe == 0.0 && ! std::signbit (nachFreigabe)
                        && p->dspKernFuerTest().samplerate() == 0.0
                        && p->dspKernFuerTest().busKanaele() == 2,
                    "311/M-132 Teilfall bereiteVor_und_releaseResources_nehmen_die_rate_zurueck: nach "
                    "`releaseResources` steht die gemerkte Rate wieder auf exakt +0,0 - wie Abtastrate und "
                    "Kanalzahl, mit denen sie dieselbe Lebensdauer hat",
                    "im Fenster " + juce::String (imFenster, 6) + " Hz, nach releaseResources "
                        + juce::String (nachFreigabe, 6) + " (Vorzeichenbit "
                        + juce::String (std::signbit (nachFreigabe) ? 1 : 0) + "), Kern-Samplerate "
                        + juce::String (p->dspKernFuerTest().samplerate(), 1) + ", busKanaele "
                        + juce::String (p->dspKernFuerTest().busKanaele()));
        }
    }

    std::cout << std::endl
              << (fehlerZahl == 0 ? "SONDE-NULLTEST OK - " : "SONDE-NULLTEST FEHLGESCHLAGEN - ")
              << okZahl << " Pruefungen ok, " << fehlerZahl << " Fehler" << std::endl;
    return fehlerZahl == 0 ? 0 : 1;
}
