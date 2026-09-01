// SONDE-012 B3b — MainProject Save/Load und Host-Dirty.
// Das echte Processor-State-Gespann wird benutzt; die Produkt-v3-Threads sind
// fuer dieses Ziel abgeschaltet, daher wird keine Produktionspipe beruehrt.

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "WireEnvelope.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>

namespace
{
int fehler = 0;
int bestanden = 0;

void pruefe (bool ok, const char* name, const juce::String& detail = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name;
    if (detail.isNotEmpty())
        std::cout << "  [" << detail.toRawUTF8() << "]";
    std::cout << '\n';
    ok ? ++bestanden : ++fehler;
}

template <typename Bedingung>
bool warteAuf (int millisekunden, Bedingung&& bedingung)
{
    for (int i = 0; i < millisekunden / 5; ++i)
    {
        if (bedingung())
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (5));
    }
    return bedingung();
}

struct DirtyZaehler final : juce::AudioProcessorListener
{
    int nonParam = 0;
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        if (d.nonParameterStateChanged) ++nonParam;
    }
};

std::string id (char c)
{
    return std::string (32, c);
}

eqcop::SourcesModel::Sicht lebendeQuelle (const std::string& instance)
{
    eqcop::SourcesModel::Sicht s;
    s.subscriptionAktiv = true;
    s.fuehrendesMain = id ('f');
    s.mainDarfSchreiben = true;
    eqcop::SourcesModel::Zeile q;
    q.instanceId = instance;
    q.runtimeNonce = id ('b');
    q.pluginKind = "active_probe";
    q.mitgliedschaft = eqcop::SourcesModel::Mitgliedschaft::unclassified;
    q.control = eqcop::SourcesModel::Control::verbunden;
    q.messung = eqcop::SourcesModel::Messung::fresh;
    q.betrieb = eqcop::SourcesModel::Betrieb::active;
    q.lautheit = eqcop::SourcesModel::Lautheit::gueltig;
    q.messpunkt = eqcop::SourcesModel::Messpunkt::insert;
    q.descriptorVorhanden = true;
    q.sichtbarerName = "Host Piano";
    q.userLabel = "Reported fallback";
    q.hostBusName = "Host Piano";
    q.namensherkunft = eqcop::SourcesModel::Namensherkunft::host;
    q.lufsPaarVorhanden = true;
    q.lufsI = -18.0f;
    q.lufsIUnsicherheitLu = 0.3f;
    q.hauptziel = true;
    s.quellen.push_back (q);
    return s;
}

std::string commandId (const std::string& command)
{
    juce::var root;
    if (juce::JSON::parse (command, root).failed()) return {};
    const auto* o = root.getDynamicObject();
    return o != nullptr && o->getProperty ("command_id").isString()
             ? o->getProperty ("command_id").toString().toStdString() : std::string();
}

std::string ack (const std::string& id, bool erfolg)
{
    if (erfolg)
        return std::string (R"({"type":"command_ack","command_id":")") + id
             + R"(","ergebnis":"angewandt","state_revision":1,"state_hash":")"
             + std::string (64, 'e') + R"("})";
    return std::string (R"({"type":"command_ack","command_id":")") + id
         + R"(","ergebnis":"abgelehnt","state_revision":0,"code":"unauthorized"})";
}

std::string leererSnapshot (const nakama::ipc::ControlHello& h)
{
    const auto mainId = nakama::ipc::instanceAdresseAusState (h.adresse.instanceId);
    return std::string (R"({"type":"session_snapshot","session_epoch":")")
         + h.adresse.sessionEpoch
         + R"(","broker_epoch":"88888888888888888888888888888888","fuehrendes_main":")"
         + mainId
         + R"(","beitritt_bestaetigung_noetig":false,"mitglieder":[]})";
}

void gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand()
{
    const auto quelle = id ('a');
    eqcop::EqCopilotProcessor processor;
    processor.setzeEditorOffen (true);
    const bool initialisiert = processor.setzeBindung ("hub", "Gen", "");
    processor.setzeSourcesFixtureFuerTest (lebendeQuelle (quelle));
    DirtyZaehler dirty;
    processor.addListener (&dirty);
    const bool eingereiht = processor.bindeSourcesHauptziel (quelle);
    const auto ausstehendVorher = processor.ausstehenderSourcesCommandFuerTest();
    const auto command = commandId (ausstehendVorher);

    juce::MemoryBlock stateVorher;
    processor.getStateInformation (stateVorher);
    const auto mitgliederVorher = processor.holeZustandKopie().mainProjectMitglieder;
    const auto dirtyVorher = dirty.nonParam;
    const auto revisionVorher = processor.v3StateRevisionFuerTest();

    const std::string pipe = std::string ("\\\\.\\pipe\\evenacadia.eq-copilot.probe.nak123.c10.")
                           + std::to_string (GetCurrentProcessId());
    const std::wstring pipeW (pipe.begin(), pipe.end());
    HANDLE server = CreateNamedPipeW (
        pipeW.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
        1, 65536, 65536, 0, nullptr);
    if (server == INVALID_HANDLE_VALUE)
    {
        processor.removeListener (&dirty);
        pruefe (false,
                "gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand",
                "Testpipe konnte nicht angelegt werden");
        return;
    }

    std::atomic<bool> ackSchreibversuch { false };
    std::atomic<bool> ackVollstaendigGeschrieben { false };
    std::atomic<DWORD> clientBytes { 0 };
    const auto gefaelschtesAck = ack (command, true);
    std::vector<std::uint8_t> ackFrame;
    const bool frameGueltig = nakama::ipc::envelopeSchreiben (
        nakama::ipc::Familie::p0, nakama::ipc::kJsonSchemaMinor,
        reinterpret_cast<const std::uint8_t*> (gefaelschtesAck.data()),
        gefaelschtesAck.size(), ackFrame);
    std::thread peer ([&]
    {
        const bool verbunden = ConnectNamedPipe (server, nullptr) != FALSE
                            || GetLastError() == ERROR_PIPE_CONNECTED;
        if (verbunden && frameGueltig)
        {
            ackSchreibversuch.store (true);
            DWORD geschrieben = 0;
            ackVollstaendigGeschrieben.store (
                WriteFile (server, ackFrame.data(), static_cast<DWORD> (ackFrame.size()),
                           &geschrieben, nullptr) != FALSE
                && geschrieben == static_cast<DWORD> (ackFrame.size()));
            unsigned char byte = 0;
            DWORD gelesen = 0;
            ReadFile (server, &byte, 1, &gelesen, nullptr);
            clientBytes.store (gelesen);
        }
        DisconnectNamedPipe (server);
        CloseHandle (server);
    });

    std::atomic<int> callbacks { 0 };
    std::atomic<bool> authVorFehler { false }, authFreigeben { false };
    auto erwartung = nakama::ipc::serverErwartungFuerEigenprozessTest();
    erwartung.testFehler = nakama::ipc::ServerPruefFehler::hashFalsch;
    erwartung.testVorFehlerErreicht = &authVorFehler;
    erwartung.testFehlerFreigeben = &authFreigeben;
    nakama::ipc::ControlClient angreifer (
        [&] { return processor.v3HelloFuerTest(); }, pipe,
        [&] (const std::string& json) {
            ++callbacks;
            processor.v3AntwortFuerTest (json);
        }, {}, {}, {}, erwartung);
    angreifer.start();
    const bool ackVorAblehnung = warteAuf (4000, [&]
    {
        return authVorFehler.load() && ackVollstaendigGeschrieben.load();
    });
    authFreigeben.store (true);
    const bool authFiel = warteAuf (4000, [&]
    {
        const auto s = angreifer.snapshot();
        return s.serverPruefstatus
                    == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert
            && s.serverPrueffehler == nakama::ipc::ServerPruefFehler::hashFalsch
            && s.serverPruefungen == 1;
    });
    const auto authZustand = angreifer.snapshot();
    angreifer.stop();
    peer.join();

    processor.setzeControlTransportFuerTest (authZustand);
    processor.sourcesTick();
    juce::MemoryBlock stateNachher;
    processor.getStateInformation (stateNachher);
    const auto zustandNachher = processor.holeZustandKopie();
    const bool stateBytesGleich = stateVorher.getSize() == stateNachher.getSize()
        && (stateVorher.getSize() == 0
            || std::memcmp (stateVorher.getData(), stateNachher.getData(),
                            stateVorher.getSize()) == 0);
    const bool unverbraucht = processor.ausstehenderSourcesCommandFuerTest()
                           == ausstehendVorher;
    const bool ehrlich = authZustand.status == nakama::ipc::ControlClient::Status::getrennt
        && ! authZustand.brokerPipeFehlt
        && authZustand.serverPruefstatus
                == nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert
        && authZustand.letzterFehler.find ("Server nicht verifiziert")
                != std::string::npos
        && authZustand.p0Gesendet == 0 && authZustand.p1Gesendet == 0
        && authZustand.empfangen == 0
        && processor.sourcesSicht().diagnose
                == eqcop::SourcesModel::Diagnose::serverUnverified
        && ! processor.sourcesSicht().diagnoseHatHandgriff;
    const bool unveraendert = callbacks.load() == 0 && unverbraucht
        && zustandNachher.mainProjectMitglieder == mitgliederVorher
        && dirty.nonParam == dirtyVorher
        && processor.v3StateRevisionFuerTest() == revisionVorher
        && stateBytesGleich;
    processor.removeListener (&dirty);
    pruefe (initialisiert && eingereiht && ! command.empty()
                && frameGueltig && ackSchreibversuch.load()
                && ackVorAblehnung && ackVollstaendigGeschrieben.load()
                && clientBytes.load() == 0
                && authFiel && ehrlich && unveraendert,
            "gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand");
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand();
    const auto quelle = id ('a');

    eqcop::EqCopilotProcessor vor;
    vor.setzeEditorOffen (true);
    pruefe (vor.setzeBindung ("hub", "Gen", ""),
            "main_project_is_explicitly_initialised");
    auto live = lebendeQuelle (quelle);
    auto nurFluechtig = live.quellen.front();
    nurFluechtig.instanceId = id ('c');
    nurFluechtig.runtimeNonce = id ('d');
    nurFluechtig.hauptziel = false;
    live.quellen.push_back (nurFluechtig);
    vor.setzeSourcesFixtureFuerTest (std::move (live));

    DirtyZaehler dirtyVor;
    vor.addListener (&dirtyVor);
    const bool fremdEingereiht = vor.bindeSourcesHauptziel (quelle);
    const auto fremdCommand = vor.ausstehenderSourcesCommandFuerTest();
    vor.v3AntwortFuerTest (ack (commandId (fremdCommand), false));
    vor.sourcesTick();
    const bool fremdAbgelehnt = vor.holeZustandKopie().mainProjectMitglieder.empty()
                             && dirtyVor.nonParam == 0;
    const bool gebundenEingereiht = vor.bindeSourcesHauptziel (quelle);
    const auto bindCommand = vor.ausstehenderSourcesCommandFuerTest();
    const auto bindId = commandId (bindCommand);
    const bool vorAckNichtPersistiert = vor.holeZustandKopie().mainProjectMitglieder.empty()
                                     && dirtyVor.nonParam == 0;
    vor.v3AntwortFuerTest (ack (bindId, true));
    vor.sourcesTick();
    const auto nachBind = vor.holeZustandKopie();
    const bool berichtetesLabelPersistiert = nachBind.mainProjectMitglieder.size() == 1
        && nachBind.mainProjectMitglieder.front().label == "Reported fallback";
    const bool benannt = vor.benenneSourcesHauptziel (quelle, "Stored Piano");
    const bool noOp = ! vor.benenneSourcesHauptziel (quelle, "Stored Piano");
    pruefe (fremdEingereiht && fremdAbgelehnt && gebundenEingereiht
            && bindCommand.find ("\"command\":\"confirm_join\"")
                != std::string::npos && ! bindId.empty() && vorAckNichtPersistiert
            && berichtetesLabelPersistiert && benannt && noOp && dirtyVor.nonParam == 2,
            "confirmed_join_ack_and_name_each_mark_host_dirty",
            juce::String (dirtyVor.nonParam));

    juce::MemoryBlock state;
    vor.getStateInformation (state);
    pruefe (dirtyVor.nonParam == 2 && state.getSize() > 0,
            "save_is_not_a_persistent_mutation");
    vor.setStateInformation (state.getData(), (int) state.getSize());
    const auto reloadGleichesObjekt = vor.sourcesSicht();
    const auto* reloadQuelle = reloadGleichesObjekt.quellen.size() == 1
                                 ? &reloadGleichesObjekt.quellen.front() : nullptr;
    pruefe (reloadQuelle != nullptr && reloadQuelle->instanceId == quelle
            && reloadQuelle->control == eqcop::SourcesModel::Control::getrennt
            && reloadQuelle->messung == eqcop::SourcesModel::Messung::missing
            && ! reloadQuelle->lufsPaarVorhanden && reloadQuelle->runtimeNonce.empty()
            && dirtyVor.nonParam == 2,
            "same_instance_reload_discards_transient_snapshot_and_measurement");
    vor.removeListener (&dirtyVor);

    eqcop::EqCopilotProcessor nach;
    DirtyZaehler dirtyNach;
    nach.addListener (&dirtyNach);
    nach.setStateInformation (state.getData(), (int) state.getSize());
    const auto z = nach.holeZustandKopie();
    const auto sicht = nach.sourcesSicht();
    const auto* q = sicht.quellen.size() == 1 ? &sicht.quellen.front() : nullptr;
    const bool stateWahr = z.common.klasse == nakama::state::Klasse::main
        && z.common.projectBindingId.isNotEmpty()
        && z.mainProjectMitglieder.size() == 1
        && z.mainProjectMitglieder.front().instanceId == juce::String (quelle)
        && z.mainProjectMitglieder.front().label == "Stored Piano";
    const bool keineLiveWahrheit = q != nullptr
        && q->mitgliedschaft == eqcop::SourcesModel::Mitgliedschaft::bestaetigt
        && q->sichtbarerName == "Stored Piano"
        && q->control == eqcop::SourcesModel::Control::getrennt
        && q->messung != eqcop::SourcesModel::Messung::fresh
        && ! q->lufsPaarVorhanden
        && q->runtimeNonce.empty();
    pruefe (stateWahr && keineLiveWahrheit && dirtyNach.nonParam == 0,
            "reload_preserves_identity_label_membership_but_not_live_truth",
            juce::String (dirtyNach.nonParam));

    const auto subscribe = nach.v3SubscribeFuerTest();
    pruefe (subscribe.find ("\"type\":\"subscribe_session\"") != std::string::npos
            && subscribe.find (z.common.projectBindingId.toStdString()) != std::string::npos,
            "project_reload_reconnect_path_builds_a_fresh_subscribe");

    const auto hello = nach.v3HelloFuerTest();
    nach.v3LinkFuerTest (true); // derselbe Callback wie direkt nach welcome
    const bool wartetSnapshot = ! nach.sourcesSicht().subscriptionAktiv;
    nach.v3AntwortFuerTest (leererSnapshot (hello));
    const bool ersterAktiv = nach.sourcesSicht().subscriptionAktiv;
    nach.v3LinkFuerTest (false);
    const bool endeInvalidiert = ! nach.sourcesSicht().subscriptionAktiv;
    nach.v3LinkFuerTest (true);
    const bool reconnectWartet = ! nach.sourcesSicht().subscriptionAktiv;
    nach.v3AntwortFuerTest (leererSnapshot (hello));
    pruefe (wartetSnapshot && ersterAktiv && endeInvalidiert
            && reconnectWartet && nach.sourcesSicht().subscriptionAktiv,
            "main_client_subscribes_after_welcome_and_reconnect_resubscribes");

    auto referenz = nach.sourcesSicht();
    referenz.mainDarfSchreiben = false;
    referenz.fuehrendesMain = id ('e');
    nach.setzeSourcesFixtureFuerTest (std::move (referenz));
    {
        eqcop::EqCopilotEditor editor (nach);
        editor.setSize (760, 430);
        juce::Timer::callPendingTimersSynchronously();
        const bool labelDeaktiviert = ! editor.sourcesLabelAktivFuerTest()
            && editor.sourcesBedienstatusFuerTest().containsIgnoreCase ("leading Main");
        editor.sourcesLabelSchreibversuchFuerTest ("Must not write");
        const bool fehlschlagSichtbar = editor.sourcesBedienstatusFuerTest()
            .containsIgnoreCase ("failed");
        pruefe (labelDeaktiviert && fehlschlagSichtbar
                && nach.holeZustandKopie().mainProjectMitglieder.front().label == "Stored Piano"
                && dirtyNach.nonParam == 0,
                "nonleading_main_disables_label_and_reports_failed_write");
    }

    nach.v3LinkFuerTest (true);
    nach.v3AntwortFuerTest (leererSnapshot (hello));
    const auto nurPersistent = nach.sourcesSicht();
    const bool istNurPersistent = nurPersistent.mainDarfSchreiben
        && nurPersistent.quellen.size() == 1
        && nurPersistent.quellen.front().mitgliedschaft
            == eqcop::SourcesModel::Mitgliedschaft::bestaetigt
        && nurPersistent.quellen.front().runtimeNonce.empty();
    const bool entferntLokal = nach.entferneSourcesHauptziel (quelle);
    const auto nachUnbind = nach.sourcesSicht();
    const bool keineWireBehauptung = nach.ausstehenderSourcesCommandFuerTest().empty();
    const bool entferntSichtbar = nach.holeZustandKopie().mainProjectMitglieder.empty()
        && nachUnbind.quellen.empty();
    const bool zweitesNoOp = ! nach.entferneSourcesHauptziel (quelle);
    pruefe (istNurPersistent && entferntLokal && keineWireBehauptung
            && entferntSichtbar && zweitesNoOp && dirtyNach.nonParam == 1,
            "persistent_only_member_remove_needs_no_runtime_nonce_and_marks_host_dirty_once",
            juce::String (dirtyNach.nonParam));
    nach.removeListener (&dirtyNach);

    std::cout << "SONDE-012 ProjectReload: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
}
