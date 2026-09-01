// SONDE-012 B3a — lokaler JUCE-ChannelContext und heartbeat.runtime.
// Das Bein beweist den Callbackpfad im Produktprozessor. Ob FL Studio den
// VST3-Callback wirklich beliefert, bleibt bewusst Sache des User-Termins.

#include "SondeProcessor.h"

#include <iostream>
#include <string>

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

    std::cout << "SONDE-012 HOST CHANNEL CONTEXT: " << bestanden << " bestanden, "
              << fehler << " fehlgeschlagen" << std::endl;
    return fehler == 0 ? 0 : 1;
}
