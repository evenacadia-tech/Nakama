// Selbsttest des Wegwerf-Messgeraets (SONDE-004a).
//
// Zweck: das LINEAL beweisen, BEVOR der User Zeit in FL investiert. Ein
// Messgeraet, dessen Versatzrechnung falsch ist, produziert im Termin einen
// glaubwuerdigen Unsinn — und der waere teurer als gar keine Messung.
//
// Exit 0 nur bei "AUXSPIKE-TEST OK".
#include "AuxSpikeProcessor.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

using namespace eqcop::spike;

namespace
{

int fehler = 0;
int bestanden = 0;

void pruefe (bool ok, const juce::String& name, const juce::String& zusatz = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

constexpr int kBlock = 512;
constexpr double kRate = 48000.0;

struct Impuls
{
    int         bus;         // 0 = Main, 1 = Aux1, 2 = Aux2
    juce::int64 position;    // absolute Sampleposition
    float       wert = 0.9f;
};

juce::uint32 lcg = 0x2545f491u;
float zufall()
{
    lcg = lcg * 1664525u + 1013904223u;
    return ((float) (lcg >> 8) / (float) 0x00ffffff) * 2.0f - 1.0f;
}

/** Baut ein Layout mit Main + so vielen aktiven Aux-Bussen wie angegeben. */
juce::AudioProcessor::BusesLayout layoutMit (int auxAktiv,
                                             juce::AudioChannelSet main = juce::AudioChannelSet::stereo())
{
    juce::AudioProcessor::BusesLayout l;
    l.inputBuses.add (main);
    for (int i = 0; i < 2; ++i)
        l.inputBuses.add (i < auxAktiv ? juce::AudioChannelSet::stereo()
                                       : juce::AudioChannelSet::disabled());
    l.outputBuses.add (main);
    return l;
}

/** Faehrt eine Messung mit gesetzten Impulsen und liefert den Messstand.
    `mainRauschen` fuellt den Main-Bus zusaetzlich mit Rauschen und prueft
    dabei die Bitgleichheit des Ausgangs. */
Schnappschuss fahre (AuxSpikeProcessor& p, int bloecke, const std::vector<Impuls>& impulse,
                     bool mainRauschen = false, bool* bitgleich = nullptr)
{
    const int kanaeleGesamt = juce::jmax (p.getTotalNumInputChannels(), p.getTotalNumOutputChannels());
    juce::AudioBuffer<float> puffer (kanaeleGesamt, kBlock);
    juce::AudioBuffer<float> kopie  (kanaeleGesamt, kBlock);
    juce::MidiBuffer midi;

    if (bitgleich != nullptr)
        *bitgleich = true;

    for (int block = 0; block < bloecke; ++block)
    {
        puffer.clear();
        const juce::int64 basis = (juce::int64) block * kBlock;

        if (mainRauschen)
            for (int k = 0; k < juce::jmin (2, kanaeleGesamt); ++k)
                for (int i = 0; i < kBlock; ++i)
                    puffer.setSample (k, i, zufall());

        for (const auto& imp : impulse)
        {
            if (imp.position < basis || imp.position >= basis + kBlock)
                continue;
            const int versatz = (int) (imp.position - basis);
            for (int k = 0; k < 2; ++k)
            {
                const int kanal = imp.bus * 2 + k;
                if (kanal < kanaeleGesamt)
                    puffer.setSample (kanal, versatz, imp.wert);
            }
        }

        for (int k = 0; k < kanaeleGesamt; ++k)
            kopie.copyFrom (k, 0, puffer, k, 0, kBlock);

        p.processBlock (puffer, midi);

        if (bitgleich != nullptr)
            for (int k = 0; k < juce::jmin (2, kanaeleGesamt); ++k)
                if (std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                                 sizeof (float) * (size_t) kBlock) != 0)
                    *bitgleich = false;
    }

    return p.schnappschuss();
}

} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceStart;

    // ── Layoutvertrag ────────────────────────────────────────────────────
    {
        AuxSpikeProcessor p;
        pruefe (p.getBusCount (true) == 3, "drei Eingangsbusse deklariert",
                juce::String (p.getBusCount (true)));
        pruefe (p.getBus (true, 1) != nullptr && p.getBus (true, 1)->getName() == kNameAux1,
                "Aux 1 traegt den Vertragsnamen priority_sidechain");
        pruefe (p.getBus (true, 2) != nullptr && p.getBus (true, 2)->getName() == kNameAux2,
                "Aux 2 traegt den Vertragsnamen compare_pre");
        pruefe (p.getBus (true, 1) != nullptr && ! p.getBus (true, 1)->isEnabledByDefault(),
                "Aux-Busse sind Vorgabe-inaktiv (Host muss zuschalten)");

        pruefe (p.checkBusesLayoutSupported (layoutMit (2)), "Main stereo + zwei Aux stereo wird angenommen");
        pruefe (p.checkBusesLayoutSupported (layoutMit (0)), "Main allein (beide Aux aus) wird angenommen");
        pruefe (p.checkBusesLayoutSupported (layoutMit (0, juce::AudioChannelSet::mono())),
                "Main mono wird angenommen");
        pruefe (! p.checkBusesLayoutSupported (layoutMit (0, juce::AudioChannelSet::create5point1())),
                "5.1 am Main wird klar abgelehnt (kein stilles Umdeuten)");

        juce::AudioProcessor::BusesLayout schief;
        schief.inputBuses.add (juce::AudioChannelSet::stereo());
        schief.inputBuses.add (juce::AudioChannelSet::create5point1());
        schief.inputBuses.add (juce::AudioChannelSet::disabled());
        schief.outputBuses.add (juce::AudioChannelSet::stereo());
        pruefe (! p.checkBusesLayoutSupported (schief), "5.1 auf einem Aux-Bus wird abgelehnt");
    }

    // ── Messung: Versatz, Blockgrenze, fehlender Impuls ──────────────────
    {
        AuxSpikeProcessor p;
        pruefe (p.setBusesLayout (layoutMit (2)), "Layout mit zwei aktiven Aux-Bussen gesetzt");
        p.prepareToPlay (kRate, kBlock);

        const auto s = fahre (p, 4,
                              { { 0, 100 },            // Main
                                { 1, 612 },            // Aux 1: +512
                                { 2, 512 } });         // Aux 2: exakt auf der Blockgrenze

        pruefe (s.busse.size() == 3, "Messstand kennt drei Busse");
        pruefe (s.busse[0].impulsSample == 100, "Main-Impuls exakt bei Sample 100",
                juce::String (s.busse[0].impulsSample));
        pruefe (s.busse[1].impulsSample == 612, "Aux-1-Impuls exakt bei Sample 612",
                juce::String (s.busse[1].impulsSample));
        pruefe (s.busse[2].impulsSample == 512, "Aux-2-Impuls exakt auf der Blockgrenze (512)",
                juce::String (s.busse[2].impulsSample));
        pruefe (s.hatVersatz (1) && s.versatz (1) == 512, "Versatz Aux 1 = +512 Samples",
                juce::String (s.hatVersatz (1) ? s.versatz (1) : -1));
        pruefe (s.hatVersatz (2) && s.versatz (2) == 412, "Versatz Aux 2 = +412 Samples",
                juce::String (s.hatVersatz (2) ? s.versatz (2) : -1));
        pruefe (s.samplesVerarbeitet == 4 * kBlock, "Samplezaehler stimmt",
                juce::String (s.samplesVerarbeitet));
        pruefe (s.busse[0].protokollOk && s.busse[1].protokollOk && s.busse[2].protokollOk,
                "Stille + ein Impuls gilt als eingehaltenes Messprotokoll");

        // ── Gegenpfad: Reset ──────────────────────────────────────────────
        p.messungZuruecksetzen();
        const auto nachReset = fahre (p, 1, {});
        pruefe (nachReset.busse[0].impulsSample == -1
                    && nachReset.busse[1].impulsSample == -1
                    && nachReset.busse[2].impulsSample == -1,
                "Reset loescht alle Impulsbefunde");
        pruefe (nachReset.samplesVerarbeitet == kBlock, "Reset setzt den Samplezaehler zurueck",
                juce::String (nachReset.samplesVerarbeitet));
    }

    // ── Dauersignal: bitgleich, aber KEINE erfundene Messung ─────────────
    // Der erste Bau ist hier durchgefallen: auf Rauschen meldete "erstes
    // Sample ueber der Schwelle" den Impuls bei 0. Auf einer laufenden
    // Mischung waere das ein glaubwuerdiger Unsinn gewesen.
    {
        AuxSpikeProcessor p;
        p.setBusesLayout (layoutMit (2));
        p.prepareToPlay (kRate, kBlock);

        bool bitgleich = false;
        const auto s = fahre (p, 4, { { 1, 612 } }, true, &bitgleich);

        pruefe (bitgleich, "Main-Ausgang bleibt bitgleich, auch mit aktiven Aux-Bussen");
        pruefe (! s.busse[0].protokollOk,
                "Dauersignal auf Main wird als Protokollverletzung erkannt",
                juce::String (s.busse[0].lautAnteil, 3));
        pruefe (! s.hatVersatz (1),
                "bei verletztem Protokoll gibt es KEINEN Versatzwert");
        pruefe (s.versatzGrund (1).contains ("Messprotokoll"),
                "der Grund steht im Klartext da", s.versatzGrund (1));
        pruefe (s.busse[1].protokollOk,
                "der stille Aux-Bus bleibt davon unberuehrt bewertet");
    }

    // ── Ehrlichkeit: 0 ist nicht 'kein Impuls' ───────────────────────────
    {
        AuxSpikeProcessor p;
        p.setBusesLayout (layoutMit (2));
        p.prepareToPlay (kRate, kBlock);

        // Main und Aux 1 exakt gleichzeitig; Aux 2 bekommt nur Kleinsignal.
        const auto s = fahre (p, 2, { { 0, 0 }, { 1, 0 }, { 2, 40, 0.10f } });

        pruefe (s.busse[0].impulsSample == 0 && s.busse[1].impulsSample == 0,
                "Impuls bei Sample 0 wird als 0 gemeldet");
        pruefe (s.hatVersatz (1) && s.versatz (1) == 0, "Versatz 0 ist ein Messwert, kein Fehlen");
        pruefe (s.busse[2].impulsSample == -1,
                "Signal unter der Schwelle loest KEINEN Impuls aus");
        pruefe (s.busse[2].spitzeGesamt > 0.09f && s.busse[2].spitzeGesamt < 0.11f,
                "Kleinsignal wird trotzdem als angekommen gemessen",
                juce::String (s.busse[2].spitzeGesamt, 3));
        pruefe (! s.hatVersatz (2), "ohne Impuls gibt es keinen Versatz (nicht 0)");
    }

    // ── Inaktiver Bus wird nie als 'kein Signal' verkauft ────────────────
    {
        AuxSpikeProcessor p;
        p.setBusesLayout (layoutMit (0));
        p.prepareToPlay (kRate, kBlock);
        const auto s = fahre (p, 1, { { 0, 10 } });

        pruefe (s.busse[1].aktiv == false && s.busse[2].aktiv == false,
                "abgeschaltete Aux-Busse melden sich als inaktiv");
        pruefe (s.busse[1].impulsSample == -1 && s.busse[1].spitzeGesamt == 0.0f,
                "inaktiver Bus liefert keine erfundenen Messwerte");
    }

    // ── Bericht: schreiben UND wieder einlesen ───────────────────────────
    {
        AuxSpikeProcessor p;
        p.setBusesLayout (layoutMit (2));
        p.prepareToPlay (kRate, kBlock);
        const auto s = fahre (p, 3, { { 0, 64 }, { 1, 320 } });

        const auto json = berichtAlsJson (s);
        auto datei = juce::File::createTempFile ("aux-spike-test.json");
        pruefe (datei.replaceWithText (json), "Bericht als Datei geschrieben");

        const auto gelesen = juce::JSON::parse (datei.loadFileAsString());
        pruefe (gelesen.isObject(), "Bericht wieder eingelesen und geparst");

        auto* obj = gelesen.getDynamicObject();
        const auto busse = obj != nullptr ? obj->getProperty ("busse") : juce::var();
        pruefe (busse.isArray() && busse.size() == 3, "Bericht traegt alle drei Busse");

        if (busse.isArray() && busse.size() == 3)
        {
            auto* main = busse[0].getDynamicObject();
            auto* aux1 = busse[1].getDynamicObject();
            auto* aux2 = busse[2].getDynamicObject();

            pruefe (main != nullptr && (juce::int64) main->getProperty ("impuls_sample") == 64,
                    "Bericht: Main-Impuls 64 unveraendert durch den Roundtrip");
            pruefe (aux1 != nullptr && (juce::int64) aux1->getProperty ("versatz_zu_main_samples") == 256,
                    "Bericht: Versatz Aux 1 = 256 Samples");
            pruefe (aux1 != nullptr
                        && std::abs ((double) aux1->getProperty ("versatz_zu_main_ms")
                                     - (256.0 * 1000.0 / kRate)) < 1.0e-9,
                    "Bericht: Versatz auch in Millisekunden korrekt");
            pruefe (aux2 != nullptr && aux2->getProperty ("impuls_sample").isVoid(),
                    "Bericht: fehlender Impuls ist null, nicht 0");
            pruefe (aux2 != nullptr && aux2->getProperty ("versatz_zu_main_samples").isVoid(),
                    "Bericht: fehlender Versatz ist null, nicht 0");
            pruefe (aux2 != nullptr
                        && aux2->getProperty ("versatz_grund").toString().isNotEmpty(),
                    "Bericht: fehlender Versatz nennt seinen Grund");
        }

        datei.deleteFile();
    }

    std::cout << std::endl
              << (fehler == 0 ? "AUXSPIKE-TEST OK" : "AUXSPIKE-TEST FEHLGESCHLAGEN")
              << " — " << bestanden << " Pruefungen ok, " << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
