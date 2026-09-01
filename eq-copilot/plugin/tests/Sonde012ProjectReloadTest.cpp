// SONDE-012 B3b — MainProject Save/Load und Host-Dirty.
// Das echte Processor-State-Gespann wird benutzt; die Produkt-v3-Threads sind
// fuer dieses Ziel abgeschaltet, daher wird keine Produktionspipe beruehrt.

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

#include <algorithm>
#include <iostream>

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
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
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
    const bool benannt = vor.benenneSourcesHauptziel (quelle, "Stored Piano");
    const bool noOp = ! vor.benenneSourcesHauptziel (quelle, "Stored Piano");
    pruefe (fremdEingereiht && fremdAbgelehnt && gebundenEingereiht
            && bindCommand.find ("\"command\":\"confirm_join\"")
                != std::string::npos && ! bindId.empty() && vorAckNichtPersistiert
            && benannt && noOp && dirtyVor.nonParam == 2,
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

    auto entfernenLive = lebendeQuelle (quelle);
    entfernenLive.quellen.front().mitgliedschaft = eqcop::SourcesModel::Mitgliedschaft::bestaetigt;
    nach.setzeSourcesFixtureFuerTest (std::move (entfernenLive));
    const bool entferntEingereiht = nach.entferneSourcesHauptziel (quelle);
    const auto unbindCommand = nach.ausstehenderSourcesCommandFuerTest();
    const auto unbindId = commandId (unbindCommand);
    const bool vorUnbindAckNochDa = nach.holeZustandKopie().mainProjectMitglieder.size() == 1
                                 && dirtyNach.nonParam == 0;
    nach.v3AntwortFuerTest (ack (unbindId, true));
    nach.sourcesTick();
    const auto nachUnbind = nach.sourcesSicht();
    const bool entferntSichtbar = nach.holeZustandKopie().mainProjectMitglieder.empty()
        && nachUnbind.quellen.size() == 1
        && nachUnbind.quellen.front().mitgliedschaft
            == eqcop::SourcesModel::Mitgliedschaft::unclassified;
    const bool zweitesNoOp = ! nach.entferneSourcesHauptziel (quelle);
    pruefe (entferntEingereiht
            && unbindCommand.find ("\"command\":\"unbind_probe\"") != std::string::npos
            && ! unbindId.empty() && vorUnbindAckNochDa
            && entferntSichtbar && zweitesNoOp && dirtyNach.nonParam == 1,
            "confirmed_unbind_ack_removes_membership_and_marks_host_dirty_once",
            juce::String (dirtyNach.nonParam));
    nach.removeListener (&dirtyNach);

    std::cout << "SONDE-012 ProjectReload: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
}
