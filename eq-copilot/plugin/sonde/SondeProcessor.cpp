#include "SondeProcessor.h"

namespace nakama::sonde
{

SondeProcessor::SondeProcessor()
    : juce::AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Frische Instanz. `frisch()` legt `legacy` an - das ist die Vorgabe des
    // Main-Bundles und fuer eine Sonde falsch: ihr Bundle-Vertrag
    // (Bundle::nkpr/nkac) laesst `legacy` gar nicht zu, ein so gespeicherter
    // Stand kaeme beim naechsten Laden als read-only zurueck. Die
    // Produktklasse ist am Bundle fest (§53.5), also steht sie hier.
    zustand = nakama::state::frisch (juce::Uuid().toString());
    zustand.common.klasse = kProduktklasse;

    // Kind-Matrix des State-Vertrags §2.1: `active_probe` verlangt GENAU EIN
    // Parameters-Kind, `passive_probe` keines. Ohne diese Zeile schrieb
    // Probeeq einen Stand, den es selbst nicht mehr laden konnte - er kam als
    // read-only mit "active_probe requires exactly one Parameters" zurueck.
    // Gefunden vom Passthrough-Beweis (EqCopProbeeqNullTest, Gegenpfad
    // speichern<->laden), nicht im Nachdenken.
    zustand.hatParameters = (kProduktklasse == nakama::state::Klasse::active_probe);
}

void SondeProcessor::prepareToPlay (double, int)
{
    // Nichts vorzubereiten: kein Zustand ueber Bloecke hinweg, kein Puffer.
    // 0 Samples Latenz ist keine Behauptung, sondern die Vorgabe von JUCE -
    // sie wird hier absichtlich NICHT gesetzt, damit niemand sie fuer
    // konfigurierbar haelt.
    jassert (getLatencySamples() == 0);
}

bool SondeProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    // Ein Passthrough, der Kanaele erfinden oder verschlucken muesste, waere
    // kein Passthrough. Deshalb nur gleiche Ein-/Ausgangsbelegung, und keine
    // deaktivierten Hauptbusse.
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();

    if (ein.isDisabled() || aus.isDisabled())
        return false;

    return ein == aus;
}

void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)
{
    // GRUNDGESETZ: sampleidentischer Passthrough. Der Host reicht EINEN
    // Puffer fuer Ein- und Ausgang; nichts zu tun heisst hier woertlich
    // nichts zu tun - keine Kopie, keine Rechnung, kein Zugriff auf
    // gemeinsamen Zustand, keine Sperre.
    //
    // Die einzige Arbeit ist das Loeschen von Ausgangskanaelen ohne Eingang.
    // Bei gleicher Belegung (isBusesLayoutSupported) gibt es keine; die
    // Schleife laeuft dann null Mal und steht hier nur, weil ein Host die
    // Belegung auch anders anbieten darf, als der Vertrag zulaesst.
    for (int k = getTotalNumInputChannels(); k < getTotalNumOutputChannels(); ++k)
        puffer.clear (k, 0, puffer.getNumSamples());
}

void SondeProcessor::getStateInformation (juce::MemoryBlock& ziel)
{
    const juce::ScopedLock l (zustandSchloss);
    nakama::state::speichere (zustand, ziel);
}

void SondeProcessor::setStateInformation (const void* daten, int groesse)
{
    if (daten == nullptr || groesse <= 0)
        return;

    nakama::state::Zustand geladen;
    const auto ergebnis = nakama::state::lade (daten, (size_t) groesse, bundleVertrag(), geladen);

    // `ignoriert` heisst: fremder Baumtyp oder Muell. Dann bleibt der
    // gehaltene Zustand, wie er ist - ein halb uebernommener Stand waere
    // genau der Teilstate, den §53.8 verbietet.
    if (ergebnis == nakama::state::LadeErgebnis::ignoriert)
        return;

    const juce::ScopedLock l (zustandSchloss);
    // §53.5: erst der Restore klassifiziert. `read-only` faellt auf neutral
    // zurueck, ein gueltiger Stand mit der eigenen Produktklasse setzt sie.
    // Der Automat bekommt den geladenen Stand, BEVOR er weggezogen wird -
    // `std::move` laesst `geladen` sonst als Huelle zurueck.
    lebenslauf.stateRestauriert (ergebnis, geladen);
    zustand = std::move (geladen);
}

} // namespace nakama::sonde
