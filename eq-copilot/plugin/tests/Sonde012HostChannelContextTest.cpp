// SONDE-012 B3a — lokaler JUCE-ChannelContext und heartbeat.runtime.
// Das Bein beweist den Callbackpfad im Produktprozessor. Ob FL Studio den
// VST3-Callback wirklich beliefert, bleibt bewusst Sache des User-Termins.

#include "SondeProcessor.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace
{
int bestanden = 0;
int fehler = 0;

void fall (const char* name, bool ok, const std::string& detail = {})
{
    std::cout << (ok ? "[ok]  " : "[ROT] ") << name;
    if (! detail.empty())
        std::cout << " [" << detail << "]";
    std::cout << std::endl;
    ok ? ++bestanden : ++fehler;
}

void binde (nakama::sonde::SondeProcessor& p,
            nakama::state::Messposition position = nakama::state::Messposition::insert,
            const juce::String& label = "Stored Probe Name")
{
    auto z = nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    z.common.klasse = nakama::state::Klasse::active_probe;
    z.common.position = position;
    z.common.label = label;
    z.common.projectBindingId = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    z.hatParameters = true;
    juce::MemoryBlock bytes;
    nakama::state::speichere (z, bytes);
    p.setStateInformation (bytes.getData(), (int) bytes.getSize());
}

void meldeName (nakama::sonde::SondeProcessor& p, const juce::String& name)
{
    juce::AudioProcessor::TrackProperties eigenschaften;
    eigenschaften.name = name;
    p.updateTrackProperties (eigenschaften);
}

void meldeFehlen (nakama::sonde::SondeProcessor& p)
{
    p.updateTrackProperties (juce::AudioProcessor::TrackProperties {});
}

bool hostIst (const nakama::ipc::ControlStatus& s, const juce::String& erwartet)
{
    return s.runtime.gemeldet && s.runtime.hostBusNameGemeldet
        && s.runtime.hostBusName == erwartet.toStdString();
}

juce::String wiederhole (juce::juce_wchar zeichen, int anzahl)
{
    juce::String aus;
    for (int i = 0; i < anzahl; ++i)
        aus += juce::String::charToString (zeichen);
    return aus;
}

bool runtimeAusJson (const std::string& text, juce::var& runtime)
{
    juce::var wurzel;
    const auto ergebnis = juce::JSON::parse (
        juce::String::fromUTF8 (text.data(), (int) text.size()), wurzel);
    if (ergebnis.failed() || wurzel.getDynamicObject() == nullptr)
        return false;
    runtime = wurzel.getDynamicObject()->getProperty ("runtime");
    return runtime.getDynamicObject() != nullptr;
}
/// Ein PlayHead, der Projektzeit und Beweislage steuerbar macht (NAK-181 R7).
struct TestKopf : juce::AudioPlayHead
{
    bool spielt = true;
    bool zeitGueltig = true;
    juce::int64 pos = 0;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setIsPlaying (spielt);
        if (zeitGueltig)
            p.setTimeInSamples (pos);
        return p;
    }
};

} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceStart;

    {
        nakama::sonde::SondeProcessor p;
        binde (p);
        const juce::String name = "Mix \\\"A\\\" / Ω";
        meldeName (p, name);
        const auto status = p.v3StatusFuerTest();
        juce::var runtime;
        const auto heartbeat = nakama::ipc::heartbeatAlsJson (
            p.v3HelloFuerTest().adresse, 7, status);
        const bool jsonOk = runtimeAusJson (heartbeat, runtime)
            && runtime["messpunkt"].toString() == "insert"
            && runtime["betrieb"].toString() == "active"
            && runtime["label"].toString() == "Stored Probe Name"
            && runtime["host_bus_name"].toString() == name;
        fall ("local_update_track_properties_reports_name_on_message_thread",
              p.hostCallbackAufMessageThreadFuerTest() && hostIst (status, name) && jsonOk);
    }

    {
        nakama::sonde::SondeProcessor p;
        binde (p, nakama::state::Messposition::insert, "State Label");
        auto status = p.v3StatusFuerTest();
        juce::var runtime;
        const bool erster = status.runtime.labelGemeldet
            && status.runtime.label == "State Label"
            && runtimeAusJson (nakama::ipc::heartbeatAlsJson (
                   p.v3HelloFuerTest().adresse, 9, status), runtime)
            && runtime["label"].toString() == "State Label";
        binde (p, nakama::state::Messposition::insert, "Changed Label");
        status = p.v3StatusFuerTest();
        const bool geaendert = status.runtime.labelGemeldet
            && status.runtime.label == "Changed Label"
            && runtimeAusJson (nakama::ipc::heartbeatAlsJson (
                   p.v3HelloFuerTest().adresse, 10, status), runtime)
            && runtime["label"].toString() == "Changed Label";
        fall ("runtime_reports_stored_label_on_first_block_and_change",
              erster && geaendert);
    }

    {
        nakama::ipc::ControlStatus status;
        status.runtime.gemeldet = true;
        status.runtime.messpunkt = "insert";
        status.runtime.betrieb = "active";
        status.runtime.labelGemeldet = true;
        status.runtime.label = wiederhole (0x1f642, 120).toStdString();
        juce::var runtime;
        const nakama::ipc::Adresse adresse {};
        const bool maximum = runtimeAusJson (
            nakama::ipc::heartbeatAlsJson (adresse, 11, status), runtime)
            && runtime["label"].toString().length() == 120;
        status.runtime.label = wiederhole (0x1f642, 121).toStdString();
        const bool drueberFehlt = runtimeAusJson (
            nakama::ipc::heartbeatAlsJson (adresse, 12, status), runtime)
            && ! runtime.getDynamicObject()->hasProperty ("label");
        status.runtime.label.clear();
        const bool leerReist = runtimeAusJson (
            nakama::ipc::heartbeatAlsJson (adresse, 13, status), runtime)
            && runtime.getDynamicObject()->hasProperty ("label")
            && runtime["label"].toString().isEmpty();
        fall ("runtime_label_uses_probe_label_codepoint_boundary",
              maximum && drueberFehlt && leerReist);
    }

    {
        nakama::sonde::SondeProcessor p;
        binde (p);
        meldeName (p, "Host Bus");
        meldeFehlen (p);
        const auto status = p.v3StatusFuerTest();
        juce::var runtime;
        const auto heartbeat = nakama::ipc::heartbeatAlsJson (
            p.v3HelloFuerTest().adresse, 8, status);
        const bool jsonOk = runtimeAusJson (heartbeat, runtime)
            && ! runtime.getDynamicObject()->hasProperty ("host_bus_name");
        fall ("local_clear_path_reports_absence",
              status.runtime.gemeldet && ! status.runtime.hostBusNameGemeldet && jsonOk);
    }

    {
        nakama::sonde::SondeProcessor p;
        binde (p, nakama::state::Messposition::insert, "User Label");
        meldeName (p, "Host Wins");
        const auto status = p.v3StatusFuerTest();
        fall ("host_name_takes_precedence_without_overwriting_label",
              hostIst (status, "Host Wins")
                  && p.zustandLesen().common.label == "User Label");

        meldeFehlen (p);
        const auto nachClear = p.v3StatusFuerTest();
        fall ("host_name_clear_restores_stored_fallback",
              ! nachClear.runtime.hostBusNameGemeldet
                  && p.zustandLesen().common.label == "User Label");
    }

    {
        nakama::sonde::SondeProcessor p;
        binde (p);
        bool ok = true;
        const auto eins = wiederhole (0x1f642, 1);
        const auto hundertzwanzig = wiederhole (0x1f642, 120);
        const auto hunderteinundzwanzig = wiederhole (0x1f642, 121);
        meldeName (p, eins);
        ok = ok && hostIst (p.v3StatusFuerTest(), eins);
        meldeName (p, hundertzwanzig);
        ok = ok && hostIst (p.v3StatusFuerTest(), hundertzwanzig);
        meldeName (p, hunderteinundzwanzig);
        ok = ok && ! p.v3StatusFuerTest().runtime.hostBusNameGemeldet;
        meldeName (p, {});
        ok = ok && ! p.v3StatusFuerTest().runtime.hostBusNameGemeldet;
        meldeName (p, juce::String::charToString (0x00a0)
                        + juce::String::charToString (0x3000));
        ok = ok && ! p.v3StatusFuerTest().runtime.hostBusNameGemeldet;

        // U+0000 ist in JUCEs nullterminierter Stringdarstellung die leere
        // Gegenprobe oben; alle weiteren C0-/C1-Codepoints sind darstellbar.
        for (int c = 1; c <= 0x1f; ++c)
        {
            meldeName (p, juce::String::charToString ((juce::juce_wchar) c));
            ok = ok && ! p.v3StatusFuerTest().runtime.hostBusNameGemeldet;
        }
        for (int c = 0x7f; c <= 0x9f; ++c)
        {
            meldeName (p, juce::String::charToString ((juce::juce_wchar) c));
            ok = ok && ! p.v3StatusFuerTest().runtime.hostBusNameGemeldet;
        }

        const juce::String unveraendert = "  Mix \\\"Quoted\\\" \\\\ Ω  ";
        meldeName (p, unveraendert);
        ok = ok && hostIst (p.v3StatusFuerTest(), unveraendert);
        fall ("host_bus_name_acceptance_boundary", ok);
    }

    {
        bool ok = true;
        for (const auto position : { nakama::state::Messposition::insert,
                                     nakama::state::Messposition::pre,
                                     nakama::state::Messposition::post })
        {
            nakama::sonde::SondeProcessor p;
            binde (p, position);
            const auto status = p.v3StatusFuerTest();
            ok = ok && status.runtime.gemeldet
                && status.runtime.messpunkt == nakama::state::wort (position);
        }
        fall ("runtime_reports_persisted_insert_pre_post_without_synthesis", ok);
    }

    {
        nakama::sonde::SondeProcessor p;
        binde (p);
        p.suspendProcessing (true);
        const bool suspendiert = p.v3StatusFuerTest().runtime.betrieb == "suspended";
        p.setNonRealtime (true);
        const bool offline = p.v3StatusFuerTest().runtime.betrieb == "offline";
        p.setNonRealtime (false);
        p.suspendProcessing (false);
        const bool aktiv = p.v3StatusFuerTest().runtime.betrieb == "active";
        fall ("runtime_reports_active_suspended_and_offline", suspendiert && offline && aktiv);
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-181 R7 · Probeeq zieht die Kontinuitaetsgrenze wie Gen
    // (G4-Befund C3, M-53) — N-31 bis N-33, N-35
    // ═══════════════════════════════════════════════════════════════════
    //
    // ⚠️ WARUM DIESE FAELLE HIER STEHEN: sie brauchen einen ECHTEN
    // `SondeProcessor` mit laufendem Analyseworker, und der lebt in diesem
    // Bein. B16 `EqCopSonde013EventWireTest` kennt nur die Engine und den
    // Serialisierer; die Matrix nannte es, weil dort der Stempel gelesen wird.
    // Die Abweichung steht im Manifest.
    //
    // Der Defekt: `merkmale.zuruecksetzen()` beim Quarantaenebruch setzte
    // `habeVorigen` auf false und Epoche wie Segment auf 0. Der naechste Block
    // lieferte `Grenzgrund::keine`, `grenzeZiehen` lief nie, und der Stempel
    // trug 0/0 — der Bruch war auf dem Draht UNSICHTBAR.
    {
        // Landmine NAK-175: der Prozessor gehoert auf den Heap.
        auto halter = std::make_unique<nakama::sonde::SondeProcessor>();
        auto& p = *halter;
        binde (p);
        p.prepareToPlay (48000.0, 512);

        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;
        TestKopf kopf;
        p.setPlayHead (&kopf);

        auto fahreBloecke = [&] (int n)
        {
            for (int i = 0; i < n; ++i)
            {
                for (int c = 0; c < puffer.getNumChannels(); ++c)
                    for (int k = 0; k < puffer.getNumSamples(); ++k)
                        puffer.setSample (c, k, 0.25f * std::sin (
                            6.2831853071795864 * 1000.0
                            * (double) (kopf.pos + k) / 48000.0));
                p.processBlock (puffer, midi);
                kopf.pos += puffer.getNumSamples();
                std::this_thread::sleep_for (std::chrono::milliseconds (2));
            }
        };

        auto stempel = [&p] (std::uint64_t& epoche, std::uint64_t& segment) -> bool
        {
            nakama::analyse::FeatureFrame f {};
            if (! p.letzterProducerFrameFuerTest (f))
                return false;
            epoche = f.transport.transport_epoch;
            segment = f.transport.continuity_segment;
            return true;
        };

        fahreBloecke (40);
        std::uint64_t e0 = 0, s0 = 0;
        // 🔑 NAK-181 Nacharbeit 1 (EP-07): diese Zeile heisst nicht mehr
        // N-31. Sie prueft die Vorbedingung dieses Blocks (es gibt ueberhaupt
        // einen veroeffentlichten Frame) — nicht die Zusage von N-31.
        // Gemessen wird N-31 im eigenen Fall `probeeq_drop_zieht_das_segment_hoch`
        // weiter unten, mit einem ECHTEN Queue-Drop.
        fall ("Vorbedingung: der Producer hat einen Frame veroeffentlicht",
              stempel (e0, s0));

        // 🔑 Ein SEEK: die Projektzeit springt. `schliesstAn` verwirft den
        // gehaltenen Block, und die Engine muss die Grenze am naechsten
        // Deskriptor selbst erkennen.
        kopf.pos += 48000 * 5;
        fahreBloecke (40);
        std::uint64_t e1 = 0, s1 = 0;
        const bool nachSeek = stempel (e1, s1);
        fall ("N-32: ein Seek zieht die Transportepoche hoch (nicht 0/0)",
              nachSeek && e1 > e0);

        // 🔑 Ein BEWEISLAGEWECHSEL: der Host meldet keine Zeit mehr.
        kopf.zeitGueltig = false;
        fahreBloecke (40);
        std::uint64_t e2 = 0, s2 = 0;
        const bool nachWechsel = stempel (e2, s2);
        fall ("N-33: ein Beweislagewechsel zieht die Epoche ebenfalls hoch",
              nachWechsel && e2 > e1);

        // 🔑 NAK-181 Nacharbeit 1 (EP-08): diese Zeile heisst nicht mehr
        // N-35. Monotonie besteht auch bei durchgehend 0 und sagt weder ueber
        // geleerte Fenster noch ueber den erhaltenen Ereignisring etwas.
        // N-35 wird in B16 `EqCopSonde013EventWireTest` gemessen, im Fall
        // `grenze_leert_die_fenster_und_laesst_den_ereignisring` — dort steht
        // die Engine ohne Worker und ihre Zaehler sind direkt lesbar.
        fall ("Quervergleich: die Epoche steigt monoton ueber alle drei "
              "Bruchklassen",
              e2 >= e1 && e1 >= e0);

        // 🔑 N-42 — ein KANALWECHSEL bei gleicher `startFolge`.
        //
        // ⚠️ GEMESSEN, NICHT ANGENOMMEN — und die Matrix lag daneben.
        // Paragraph 2.4 sagte den Weg als `lokaleLuecke` -> `sequenzluecke`
        // zu. Gemessen steigt die EPOCHE.
        //
        // Die Kette: `Blockquarantaene::schliesstAn` vergleicht `kanaele` und
        // `tapMaske` (`StampedAudioQueue.h:732-733`) und verwirft den
        // gehaltenen Block. `grenzeZwischen` vergleicht die zwei zwar nicht
        // (`FeatureEngine.h:1807-1859`) — aber die HOSTZEIT laeuft lueckenlos
        // weiter: der naechste freigegebene Block beginnt genau eine
        // Blocklaenge spaeter als `alt.projectSampleStart + alt.sampleCount`.
        // Das ist ein `zeitSprung`, und der wird VOR `lokaleLuecke` geprueft
        // (`:1833-1850` vor `:1852-1856`).
        //
        // `grenzeZwischen` kann sechs Epochengruende liefern; fuenf sind hier
        // ausgeschlossen: die `startFolge` bleibt (`neustartAnfordern` steht
        // nur in `prepareToPlay`, `SondeProcessor.cpp:132`), die Samplerate
        // bleibt, die Beweislageflags bleiben, der Transport laeuft durch, und
        // ohne aktive Schleife (`kFlagCycleAktiv` ist nicht gesetzt) kann
        // `istLoopWrap` nicht greifen. Es bleibt `zeitSprung`. Nur
        // `lokaleLuecke` haette das SEGMENT gehoben (`:2136-2145`).
        //
        // Der Wechsel reist damit schon heute als `epochwechsel` — STAERKER
        // als die Matrix annahm. Die Invalidierung findet statt, keine starke
        // Aussage ueberlebt, und `grenzeZwischen` wird weiterhin NICHT
        // umgebaut. Die Begruendung der Haertung NAK-194 ist damit
        // gegenstandslos.
        kopf.zeitGueltig = true;
        fahreBloecke (20);
        std::uint64_t e3 = 0, s3 = 0;
        const bool vorWechsel = stempel (e3, s3);

        juce::AudioBuffer<float> mono (1, 512);
        for (int i = 0; i < 40; ++i)
        {
            mono.clear();
            for (int k = 0; k < mono.getNumSamples(); ++k)
                mono.setSample (0, k, (float) (0.25 * std::sin (
                    6.2831853071795864 * 1000.0
                    * (double) (kopf.pos + k) / 48000.0)));
            p.processBlock (mono, midi);
            kopf.pos += mono.getNumSamples();
            std::this_thread::sleep_for (std::chrono::milliseconds (2));
        }
        std::uint64_t e4 = 0, s4 = 0;
        const bool nachWechsel2 = stempel (e4, s4);
        // ⚠️ Gemessen ist der KANALwechsel (stereo -> mono). Der Tapwechsel
        // geht durch DIESELBE Bedingung in `schliesstAn`
        // (`StampedAudioQueue.h:732-733`: `kanaele != ... || tapMaske != ...`)
        // und danach durch denselben Weg; ihn hier zweimal zu fahren, mass
        // dieselbe Zeile ein zweites Mal. Das Manifest sagt deshalb nicht
        // "beide Ausloeser einzeln" (Paragraph 6.9).
        fall ("N-42: ein Kanalwechsel zieht die EPOCHE hoch (zeitSprung, nicht "
              "lokaleLuecke) - der Bruch ist auf dem Draht sichtbar",
              vorWechsel && nachWechsel2 && e4 > e3);

        p.setPlayHead (nullptr);
        p.releaseResources();
    }

    // 🔑 NAK-181 N-31 (Nacharbeit 1, EP-07/NR-07):
    // `probeeq_drop_zieht_das_segment_hoch` — der Test, den die Matrix
    // namentlich zusagte und den es nicht gab.
    //
    // Bis zu dieser Runde fuhr N-31 nur normale Bloecke und prueste, dass ein
    // Frame da ist. Kein Drop wurde ausgeloest, kein Folgestempel verglichen
    // — und deshalb blieb die Zeile sogar mit dem ALTEN vollen Reset gruen
    // (Rotbeweis `roh/NAK-181-rot-b4.txt`).
    //
    // ⚠️ WARUM DIE PROJEKTZEIT HIER STEHT: `grenzeZwischen` prueft den
    // Zeitsprung VOR der lokalen Luecke (`FeatureEngine.h:1833-1856`). Bei
    // laufender Hostzeit ist ein Drop deshalb ein `zeitSprung` und hebt die
    // EPOCHE (genau das misst N-42 nebenan). N-31 sagt `continuity_segment
    // = s + 1` bei UNVERAENDERTER Epoche zu — das ist der Fall, in dem der
    // Host Audio liefert, ohne die Projektzeit zu bewegen (FL zerteilt Puffer
    // und wiederholt dabei dieselbe Projektzeit; `schliesstAn` und
    // `grenzeZwischen` lesen eine STEHENDE Zeit ausdruecklich als "sagt
    // nichts"). Der lokale Strom laeuft trotzdem weiter, und genau er traegt
    // die Luecke.
    {
        // Landmine NAK-175: der Prozessor gehoert auf den Heap.
        auto halter = std::make_unique<nakama::sonde::SondeProcessor>();
        auto& p = *halter;
        binde (p);
        p.prepareToPlay (48000.0, 512);

        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;
        TestKopf kopf;
        kopf.pos = 96000;          // eine gueltige, aber STEHENDE Projektzeit
        p.setPlayHead (&kopf);

        std::uint64_t phase = 0;
        auto fuelle = [&] (juce::AudioBuffer<float>& b)
        {
            for (int c = 0; c < b.getNumChannels(); ++c)
                for (int k = 0; k < b.getNumSamples(); ++k)
                    b.setSample (c, k, 0.25f * std::sin (
                        6.2831853071795864 * 1000.0
                        * (double) (phase + (std::uint64_t) k) / 48000.0));
            phase += (std::uint64_t) b.getNumSamples();
        };
        auto fahre = [&] (int n)
        {
            for (int i = 0; i < n; ++i)
            {
                fuelle (puffer);
                p.processBlock (puffer, midi);
                std::this_thread::sleep_for (std::chrono::milliseconds (2));
            }
        };
        auto stempel = [&p] (std::uint64_t& epoche, std::uint64_t& segment) -> bool
        {
            nakama::analyse::FeatureFrame f {};
            if (! p.letzterProducerFrameFuerTest (f))
                return false;
            epoche = f.transport.transport_epoch;
            segment = f.transport.continuity_segment;
            return true;
        };

        fahre (60);
        std::uint64_t e0 = 0, s0 = 0;
        const bool vorher = stempel (e0, s0);
        fall ("N-31: vor dem Drop steht ein Stempel", vorher,
              "Epoche " + std::to_string (e0) + ", Segment " + std::to_string (s0));
        const auto bruecheVorher   = p.kontinuitaetsbruecheFuerTest();
        const auto getrenntVorher  = p.getrennteFensterFuerTest();
        const auto lueckenVorher   = p.grenzenMitGrundFuerTest (
                                        nakama::analyse::Grenzgrund::lokaleLuecke);
        const auto dropsVorher     = p.analyseDropsOversizeFuerTest();

        // 🔑 DER ECHTE DROP: ein Block ueber `maxBlockFrames` faellt fuer die
        // Analyse als GANZES weg (Paragraph 48.1). Der lokale Strom laeuft
        // trotzdem weiter — nur so kann `kFlagLueckeDavor` die Wahrheit
        // sagen —, und der naechste angenommene Block traegt das Flag.
        {
            juce::AudioBuffer<float> zuGross (
                2, (int) nakama::echtzeit::GenStrom::maxBlockFrames + 1);
            fuelle (zuGross);
            p.processBlock (zuGross, midi);
        }
        fall ("N-31: der Block ist als Analyse-Drop gezaehlt",
              p.analyseDropsOversizeFuerTest() == dropsVorher + 1,
              std::to_string (p.analyseDropsOversizeFuerTest()));

        fahre (60);
        fall ("N-31: die Quarantaene meldet einen Kontinuitaetsbruch",
              p.kontinuitaetsbruecheFuerTest() > bruecheVorher,
              std::to_string (p.kontinuitaetsbruecheFuerTest()));

        std::uint64_t e1 = 0, s1 = 0;
        const bool nachher = stempel (e1, s1);
        fall ("N-31: nach dem Drop steht wieder ein Stempel", nachher,
              "Epoche " + std::to_string (e1) + ", Segment " + std::to_string (s1));
        fall ("N-31: probeeq_drop_zieht_das_segment_hoch - transport_epoch bleibt, "
              "continuity_segment steigt um genau 1",
              nachher && e1 == e0 && s1 == s0 + 1,
              "vorher " + std::to_string (e0) + "/" + std::to_string (s0)
                  + ", nachher " + std::to_string (e1) + "/" + std::to_string (s1));
        fall ("N-31: kein Fenster ueberbrueckt die Grenze - getrennteFenster ist "
              "gestiegen",
              p.getrennteFensterFuerTest() > getrenntVorher,
              std::to_string (p.getrennteFensterFuerTest()));
        fall ("N-31: und der Grundzaehler `lokaleLuecke` ebenso - nicht "
              "`zeitSprung`, nicht `neuanlauf`",
              p.grenzenMitGrundFuerTest (nakama::analyse::Grenzgrund::lokaleLuecke)
                  > lueckenVorher,
              std::to_string (p.grenzenMitGrundFuerTest (
                  nakama::analyse::Grenzgrund::lokaleLuecke)));

        p.setPlayHead (nullptr);
        p.releaseResources();
    }

    std::cout << "SONDE-012 HOST CHANNEL CONTEXT: " << bestanden << " bestanden, "
              << fehler << " fehlgeschlagen" << std::endl;
    return fehler == 0 ? 0 : 1;
}
