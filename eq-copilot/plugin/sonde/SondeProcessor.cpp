#include "SondeProcessor.h"
#include "PipeToken.h"

#include <cmath>
#include <process.h>

namespace nakama::sonde
{

namespace
{
std::string uuidHex32()
{
    std::string roh = juce::Uuid().toString().toStdString();
    std::string aus;
    aus.reserve (32);
    for (char c : roh)
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            aus.push_back (c);
    return aus.size() == 32 ? aus
                            : nakama::ipc::instanceAdresseAusState ("probe-runtime:" + roh);
}
} // namespace

SondeProcessor::SondeProcessor()
    : juce::AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      v3LogonSid (nakama::ipc::aktuelleLogonSid()),
      v3PipeName (nakama::ipc::pipeNameV3 (v3LogonSid)),
      v3SessionEpoch (uuidHex32()),
      v3RuntimeNonce (uuidHex32()),
      controlV3 ([this] { return v3Hello(); }, v3PipeName, {},
                 [this] { return v3Status(); }),
      telemetryV3 ([this] { return v3TelemetryHello(); }, v3PipeName)
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

#if ! defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // Probeeq ist reiner Connector: beide v3-Verbindungen, kein Lifecycle und
    // kein Spawn. Ohne persistierte Projektbindung lehnt der ControlClient
    // schon seine lokale Adresse ab und beruehrt die Produktionspipe nicht.
    controlV3.start();
    telemetryV3.start();
#endif
}

SondeProcessor::~SondeProcessor()
{
    telemetryV3.stop();
    controlV3.stop();
}

void SondeProcessor::prepareToPlay (double samplerate, int maxBlock)
{
    // Nichts vorzubereiten: kein Zustand ueber Bloecke hinweg, kein Puffer.
    // 0 Samples Latenz ist keine Behauptung, sondern die Vorgabe von JUCE -
    // sie wird hier absichtlich NICHT gesetzt, damit niemand sie fuer
    // konfigurierbar haelt.
    jassert (getLatencySamples() == 0);
    v3Samplerate.store (std::isfinite (samplerate) && samplerate > 0.0
                            && samplerate <= 768000.0 ? samplerate : 0.0);
    v3BlockSize.store (maxBlock >= 0 ? maxBlock : 0);
    v3Channels.store (getTotalNumInputChannels());
    controlV3.reconnect();
    telemetryV3.reconnect();
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

    {
        const juce::ScopedLock l (zustandSchloss);
        // §53.5: erst der Restore klassifiziert. `read-only` faellt auf neutral
        // zurueck, ein gueltiger Stand mit der eigenen Produktklasse setzt sie.
        // Der Automat bekommt den geladenen Stand, BEVOR er weggezogen wird -
        // `std::move` laesst `geladen` sonst als Huelle zurueck.
        lebenslauf.stateRestauriert (ergebnis, geladen);
        zustand = std::move (geladen);
    }
    v3StateRevision.fetch_add (1);
    controlV3.reconnect();
    telemetryV3.reconnect();
}

nakama::ipc::ControlHello SondeProcessor::v3Hello() const
{
    nakama::ipc::ControlHello h;
    {
        const juce::ScopedLock l (zustandSchloss);
        h.adresse.logonSid = v3LogonSid;
        h.adresse.projectBindingId = zustand.common.projectBindingId.toStdString();
        h.adresse.sessionEpoch = v3SessionEpoch;
        h.adresse.instanceId = zustand.common.instanceId.toStdString();
        h.adresse.runtimeNonce = v3RuntimeNonce;
        h.pluginKind = nakama::state::wort (zustand.common.klasse);
    }
    h.hostAngeben = true;
    h.hostPid = static_cast<std::uint32_t> (_getpid());
    h.pluginVersion = "0.3.0";
    h.samplerate = v3Samplerate.load();
    h.blockSize = v3BlockSize.load();
    h.channels = v3Channels.load();
    return h;
}

nakama::ipc::ControlStatus SondeProcessor::v3Status() const
{
    nakama::ipc::ControlStatus s;
    s.dspSchemaVersion = nakama::parameter::kDspSchemaVersion;
    s.stateRevision = v3StateRevision.load();
    {
        const juce::ScopedLock l (zustandSchloss);
        if (zustand.hatParameters && ! zustand.nurLesen)
        {
            juce::String hash, grund;
            if (nakama::parameter::stateHash (zustand.parameters, hash, grund))
                s.stateHash = hash.toStdString();
        }
    }
    // Die heutige Probe-Schale besitzt noch keine Host-Context-Bruecke. Sie
    // meldet deshalb Aufnahmezustand unbekannt statt ihn zu erfinden.
    s.recordStateValid = false;
    s.recording = false;
    return s;
}

nakama::ipc::TelemetryHello SondeProcessor::v3TelemetryHello() const
{
    nakama::ipc::TelemetryHello h;
    h.adresse = v3Hello().adresse;
    h.pluginVersion = "0.3.0";
    controlV3.kopplung (h.linkId, h.challenge);
    return h;
}

} // namespace nakama::sonde
