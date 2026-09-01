#include "SourcesModel.h"

#include "NakamaTelemetrie.h"
#include "NakamaVertrag.h"
#include "TelemetryClient.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace eqcop
{
namespace
{
using Diagnose = SourcesModel::Diagnose;

const juce::DynamicObject* objekt (const juce::var& v)
{
    return v.getDynamicObject();
}

bool exakteFelder (const juce::DynamicObject& o,
                   std::initializer_list<const char*> pflicht,
                   std::initializer_list<const char*> optional = {})
{
    std::set<std::string> erlaubt;
    for (const auto* p : pflicht)
        erlaubt.insert (p);
    for (const auto* p : optional)
        erlaubt.insert (p);
    const auto& props = o.getProperties();
    for (int i = 0; i < props.size(); ++i)
        if (erlaubt.count (props.getName (i).toString().toStdString()) == 0)
            return false;
    for (const auto* p : pflicht)
        if (! o.hasProperty (p))
            return false;
    return true;
}

bool hex32 (const juce::String& s)
{
    return nakama::ipc::istHex32 (s.toStdString());
}

bool nichtnegativeGanzzahl (const juce::var& v, std::uint64_t& aus)
{
    if (v.isInt())
    {
        const int n = static_cast<int> (v);
        if (n < 0) return false;
        aus = static_cast<std::uint64_t> (n);
        return true;
    }
    if (v.isInt64())
    {
        const auto n = static_cast<juce::int64> (v);
        if (n < 0) return false;
        aus = static_cast<std::uint64_t> (n);
        return aus <= 9007199254740991ULL;
    }
    return false;
}

bool adresse (const juce::var& v, std::string& binding, std::string& session,
              std::string& instance, std::string& nonce)
{
    const auto* o = objekt (v);
    if (o == nullptr || ! exakteFelder (*o, { "logon_sid", "project_binding_id",
                                              "session_epoch", "instance_id",
                                              "runtime_nonce" }))
        return false;
    const auto sid = o->getProperty ("logon_sid");
    const auto b = o->getProperty ("project_binding_id");
    const auto s = o->getProperty ("session_epoch");
    const auto i = o->getProperty ("instance_id");
    const auto n = o->getProperty ("runtime_nonce");
    if (! sid.isString() || sid.toString().isEmpty() || sid.toString().length() > 184
        || ! b.isString() || ! s.isString() || ! i.isString() || ! n.isString()
        || ! hex32 (b.toString()) || ! hex32 (s.toString())
        || ! hex32 (i.toString()) || ! hex32 (n.toString()))
        return false;
    binding = b.toString().toStdString();
    session = s.toString().toStdString();
    instance = i.toString().toStdString();
    nonce = n.toString().toStdString();
    return true;
}

bool frische (const juce::var& v, bool& stale, std::uint64_t& alter)
{
    const auto* o = objekt (v);
    if (o == nullptr || ! exakteFelder (*o, { "stale", "letzter_kontakt_ms" }))
        return false;
    const auto s = o->getProperty ("stale");
    if (! s.isBool() || ! nichtnegativeGanzzahl (o->getProperty ("letzter_kontakt_ms"), alter))
        return false;
    stale = static_cast<bool> (s);
    return true;
}

bool capabilities (const juce::var& v)
{
    static constexpr const char* namen[] = {
        "host_context_presence", "project_time_samples",
        "sample_accurate_automation", "presentation_latency",
        "aux_compare_pre", "aux_priority_sidechain", "contribution_aux",
        "float64_processing", "binary_telemetry", "remote_control"
    };
    const auto* o = objekt (v);
    if (o == nullptr || o->getProperties().size() != 10)
        return false;
    for (const auto* name : namen)
    {
        if (! o->hasProperty (name)) return false;
        const auto wert = o->getProperty (name);
        if (! wert.isString()
            || (wert.toString() != "supported" && wert.toString() != "unsupported"))
            return false;
    }
    return true;
}

bool hostNameGueltig (const juce::String& s)
{
    if (s.isEmpty() || s.length() > 120 || s.trim().isEmpty())
        return false;
    for (const auto c : s)
        if ((c >= 0 && c <= 0x1f) || (c >= 0x7f && c <= 0x9f))
            return false;
    return true;
}

SourcesModel::Messpunkt messpunktAus (const juce::String& s)
{
    if (s == "insert") return SourcesModel::Messpunkt::insert;
    if (s == "pre") return SourcesModel::Messpunkt::pre;
    if (s == "post") return SourcesModel::Messpunkt::post;
    return SourcesModel::Messpunkt::unbekannt;
}

SourcesModel::Betrieb betriebAus (const juce::String& s)
{
    if (s == "active") return SourcesModel::Betrieb::active;
    if (s == "suspended") return SourcesModel::Betrieb::suspended;
    if (s == "offline") return SourcesModel::Betrieb::offline;
    return SourcesModel::Betrieb::unbekannt;
}

struct GelesenesMitglied
{
    std::string instanceId, runtimeNonce, pluginKind;
    bool stale = false;
    std::uint64_t controlAlterMs = 0;
    bool descriptor = false;
    SourcesModel::Messpunkt messpunkt = SourcesModel::Messpunkt::unbekannt;
    SourcesModel::Betrieb betrieb = SourcesModel::Betrieb::unbekannt;
    juce::String label, hostName;
    bool hostIndex = false;
    std::uint64_t index = 0;
    bool reject = false;
    juce::String rejectGrund;
    std::uint64_t rejectZaehler = 0;
};

bool liesDescriptor (const juce::var& v, const std::string& memberBinding,
                     const std::string& memberSession,
                     const std::string& memberId,
                     const std::string& memberNonce,
                     const std::string& memberKind,
                     bool memberStale, std::uint64_t memberAlter,
                     std::uint8_t schemaMinor, GelesenesMitglied& aus)
{
    const auto* o = objekt (v);
    const bool feldsatzGueltig = schemaMinor == 0
        ? o != nullptr && exakteFelder (*o,
             { "adresse", "plugin_kind", "measurement_position", "aussageklasse",
               "label", "capabilities", "frische" },
             { "pair_id" })
        : schemaMinor == nakama::ipc::kJsonSchemaMinor
            && o != nullptr && exakteFelder (*o,
                 { "adresse", "plugin_kind", "measurement_position", "aussageklasse",
                   "betrieb", "label", "capabilities", "frische" },
                 { "pair_id", "host_bus_name", "host_mixer_index" });
    if (! feldsatzGueltig)
        return false;
    std::string b, s, i, n;
    if (! adresse (o->getProperty ("adresse"), b, s, i, n)
        || b != memberBinding || s != memberSession || i != memberId || n != memberNonce)
        return false;
    const auto kind = o->getProperty ("plugin_kind");
    const auto pos = o->getProperty ("measurement_position");
    const auto klasse = o->getProperty ("aussageklasse");
    const auto label = o->getProperty ("label");
    if (! kind.isString() || kind.toString().toStdString() != memberKind
        || ! pos.isString() || messpunktAus (pos.toString()) == SourcesModel::Messpunkt::unbekannt
        || ((pos.toString() == "pre" || pos.toString() == "post")
            && memberKind == "main")
        || ! klasse.isString() || klasse.toString() != "beobachtend"
        || ! label.isString() || label.toString().length() > 120
        || ! capabilities (o->getProperty ("capabilities")))
        return false;
    if (schemaMinor == nakama::ipc::kJsonSchemaMinor)
    {
        const auto betrieb = o->getProperty ("betrieb");
        if (! betrieb.isString()
            || betriebAus (betrieb.toString()) == SourcesModel::Betrieb::unbekannt)
            return false;
        aus.betrieb = betriebAus (betrieb.toString());
    }
    bool descriptorStale = false;
    std::uint64_t descriptorAlter = 0;
    if (! frische (o->getProperty ("frische"), descriptorStale, descriptorAlter)
        || descriptorStale != memberStale || descriptorAlter != memberAlter)
        return false;
    if (o->hasProperty ("pair_id"))
    {
        const auto pair = o->getProperty ("pair_id");
        if (! pair.isVoid()
            && (! pair.isString() || pair.toString().isEmpty()
                || pair.toString().length() > 64))
            return false;
    }

    aus.descriptor = true;
    aus.messpunkt = messpunktAus (pos.toString());
    aus.label = label.toString();
    if (o->hasProperty ("host_bus_name"))
    {
        const auto name = o->getProperty ("host_bus_name");
        if (! name.isString() || ! hostNameGueltig (name.toString())) return false;
        aus.hostName = name.toString();
    }
    if (o->hasProperty ("host_mixer_index"))
    {
        if (! nichtnegativeGanzzahl (o->getProperty ("host_mixer_index"), aus.index)
            || aus.index < 1)
            return false;
        aus.hostIndex = true;
    }
    return true;
}

bool liesReject (const juce::var& v, GelesenesMitglied& aus)
{
    static const std::set<juce::String> katalog = {
        "feature_batch_ungueltig", "quellframe_anzahl_ungueltig",
        "routing_nicht_freigegeben", "quelladresse_abweichend",
        "lautheit_ungueltig"
    };
    const auto* o = objekt (v);
    if (o == nullptr || ! exakteFelder (*o, { "grund", "zaehler" })) return false;
    const auto grund = o->getProperty ("grund");
    if (! grund.isString() || katalog.count (grund.toString()) == 0
        || ! nichtnegativeGanzzahl (o->getProperty ("zaehler"), aus.rejectZaehler)
        || aus.rejectZaehler < 1)
        return false;
    aus.reject = true;
    aus.rejectGrund = grund.toString();
    return true;
}

bool inkompatiblerFehler (const std::string& text)
{
    return text.find ("Vertrag") != std::string::npos
        || text.find ("protocol") != std::string::npos
        || text.find ("unerwartet") != std::string::npos
        || text.find ("Broker lehnt ab") != std::string::npos;
}

} // namespace

void SourcesModel::setzePersistenteMitglieder (
    const std::vector<nakama::state::MainProjectMitglied>& mitglieder)
{
    std::map<std::string, juce::String> neu;
    for (const auto& m : mitglieder)
        if (nakama::ipc::istHex32 (m.instanceId.toStdString()) && m.label.length() <= 120
            && neu.size() < static_cast<std::size_t> (
                   nakama::state::maxMainProjectMitglieder))
            neu.emplace (m.instanceId.toStdString(), m.label);

    std::lock_guard<std::mutex> l (mutex);
    if (neu == persistenteMitglieder)
        return;
    persistenteMitglieder = std::move (neu);
    for (auto it = eintraege.begin(); it != eintraege.end();)
    {
        const auto p = persistenteMitglieder.find (it->first);
        if (p != persistenteMitglieder.end())
        {
            it->second.zeile.mitgliedschaft = Mitgliedschaft::bestaetigt;
            it->second.zeile.userLabel = p->second;
            aktualisiereName (it->second);
            ++it;
        }
        else if (! it->second.fluechtigImSnapshot)
        {
            if (hauptziel == it->first) hauptziel.clear();
            it = eintraege.erase (it);
        }
        else
        {
            it->second.zeile.mitgliedschaft = Mitgliedschaft::unclassified;
            it->second.zeile.userLabel = it->second.descriptorLabel;
            aktualisiereName (it->second);
            ++it;
        }
    }
    for (const auto& [id, label] : persistenteMitglieder)
    {
        if (eintraege.count (id) != 0) continue;
        Eintrag e;
        e.zeile.instanceId = id;
        e.zeile.mitgliedschaft = Mitgliedschaft::bestaetigt;
        e.zeile.control = Control::getrennt;
        e.zeile.userLabel = label;
        aktualisiereName (e);
        eintraege.emplace (id, std::move (e));
    }
    stelleZielSicher();
    revidiere();
}

void SourcesModel::projektReload (
    const std::vector<nakama::state::MainProjectMitglied>& mitglieder)
{
    std::map<std::string, juce::String> persistent;
    for (const auto& m : mitglieder)
        if (nakama::ipc::istHex32 (m.instanceId.toStdString()) && m.label.length() <= 120
            && persistent.size() < static_cast<std::size_t> (
                   nakama::state::maxMainProjectMitglieder))
            persistent.emplace (m.instanceId.toStdString(), m.label);

    std::lock_guard<std::mutex> l (mutex);
    persistenteMitglieder = std::move (persistent);
    eintraege.clear();
    for (const auto& [id, label] : persistenteMitglieder)
    {
        Eintrag e;
        e.zeile.instanceId = id;
        e.zeile.mitgliedschaft = Mitgliedschaft::bestaetigt;
        e.zeile.control = Control::getrennt;
        e.zeile.messung = Messung::missing;
        e.zeile.userLabel = label;
        aktualisiereName (e);
        eintraege.emplace (id, std::move (e));
    }
    erwarteteBindung.clear();
    erwarteteSession.clear();
    eigeneMainId.clear();
    brokerEpoch.clear();
    fuehrendesMain.clear();
    hauptziel.clear();
    subscriptionAktiv = false;
    diagnose = Diagnose::brokerUnavailable;
    diagnoseHatHandgriff = true;
    sichtZeit = {};
    stelleZielSicher();
    revidiere();
}

void SourcesModel::beginneSubscription (std::string binding, std::string session,
                                        std::string eigeneMainInstanceId)
{
    std::lock_guard<std::mutex> l (mutex);
    erwarteteBindung = std::move (binding);
    erwarteteSession = std::move (session);
    eigeneMainId = std::move (eigeneMainInstanceId);
    subscriptionAktiv = false;
    diagnose = Diagnose::authenticating;
    diagnoseHatHandgriff = false;
    revidiere();
}

void SourcesModel::controlEnde()
{
    std::lock_guard<std::mutex> l (mutex);
    subscriptionAktiv = false;
    for (auto& [_, e] : eintraege)
    {
        if (e.fluechtigImSnapshot)
            e.zeile.control = Control::getrennt;
        if (e.hatMessZeit && e.zeile.messung != Messung::invalid)
            e.zeile.messung = Messung::stale;
    }
    if (diagnose != Diagnose::incompatible && diagnose != Diagnose::storeDegraded)
    {
        diagnose = Diagnose::brokerUnavailable;
        diagnoseHatHandgriff = true;
    }
    revidiere();
}

void SourcesModel::setzeControlTransport (
    const nakama::ipc::ControlClient::Snapshot& transport)
{
    std::lock_guard<std::mutex> l (mutex);
    Diagnose neu = diagnose;
    bool handgriff = false;
    if (transport.status == nakama::ipc::ControlClient::Status::verbindet
        || (transport.status == nakama::ipc::ControlClient::Status::verbunden
            && ! subscriptionAktiv))
        neu = Diagnose::authenticating;
    else if (transport.status == nakama::ipc::ControlClient::Status::getrennt)
    {
        if (inkompatiblerFehler (transport.letzterFehler))
            neu = Diagnose::incompatible;
        else
        {
            neu = Diagnose::brokerUnavailable;
            handgriff = true;
        }
    }
    else if (subscriptionAktiv && diagnose != Diagnose::confirmationRequired
             && diagnose != Diagnose::storeDegraded)
        neu = Diagnose::keine;
    if (neu != diagnose || handgriff != diagnoseHatHandgriff)
    {
        diagnose = neu;
        diagnoseHatHandgriff = handgriff;
        revidiere();
    }
}

SourcesModel::SnapshotErgebnis SourcesModel::uebernehmeSessionSnapshot (
    const std::string& json, std::uint8_t schemaMinor,
    Zeitpunkt empfangen, juce::String& fehler)
{
    fehler.clear();
    if (schemaMinor > nakama::ipc::kJsonSchemaMinor)
    {
        fehler = "session_snapshot schema_minor is newer than this Main reader";
        return SnapshotErgebnis::ungueltig;
    }
    std::string erwarteteBindungLokal, erwarteteSessionLokal;
    {
        std::lock_guard<std::mutex> l (mutex);
        erwarteteBindungLokal = erwarteteBindung;
        erwarteteSessionLokal = erwarteteSession;
    }
    if (erwarteteBindungLokal.empty() || erwarteteSessionLokal.empty())
        return SnapshotErgebnis::ignoriert;
    juce::String riegel;
    if (! nakama::vertrag::textriegelBytes (json.data(), json.size(), riegel))
    {
        fehler = "session_snapshot text boundary: " + riegel;
        return SnapshotErgebnis::ungueltig;
    }
    juce::var root;
    const auto parse = juce::JSON::parse (juce::String::fromUTF8 (json.data(),
                                                                  static_cast<int> (json.size())), root);
    const auto* o = objekt (root);
    if (parse.failed() || o == nullptr)
        return SnapshotErgebnis::ignoriert;
    const auto typ = o->getProperty ("type");
    if (! typ.isString() || typ.toString() != "session_snapshot")
        return SnapshotErgebnis::ignoriert;
    const bool wurzelFelderGueltig = schemaMinor == 0
        ? exakteFelder (*o,
              { "type", "session_epoch", "broker_epoch", "fuehrendes_main", "mitglieder" },
              { "beitritt_bestaetigung_noetig" })
        : exakteFelder (*o,
              { "type", "session_epoch", "broker_epoch", "fuehrendes_main", "mitglieder" },
              { "beitritt_bestaetigung_noetig", "store_degraded" });
    if (! wurzelFelderGueltig)
    {
        fehler = "session_snapshot has an unexpected or missing field";
        return SnapshotErgebnis::ungueltig;
    }
    const auto session = o->getProperty ("session_epoch");
    const auto epoch = o->getProperty ("broker_epoch");
    const auto fuehrung = o->getProperty ("fuehrendes_main");
    if (! session.isString() || ! hex32 (session.toString())
        || ! epoch.isString() || ! hex32 (epoch.toString())
        || (! fuehrung.isVoid() && ! fuehrung.isString()))
    {
        fehler = "session_snapshot session, broker epoch or leading Main is invalid";
        return SnapshotErgebnis::ungueltig;
    }
    bool bestaetigung = false;
    if (o->hasProperty ("beitritt_bestaetigung_noetig"))
    {
        const auto b = o->getProperty ("beitritt_bestaetigung_noetig");
        if (! b.isBool())
        {
            fehler = "beitritt_bestaetigung_noetig is not boolean";
            return SnapshotErgebnis::ungueltig;
        }
        bestaetigung = static_cast<bool> (b);
    }
    bool storeDegradiert = false;
    if (o->hasProperty ("store_degraded"))
    {
        const auto d = o->getProperty ("store_degraded");
        if (! d.isBool() || ! static_cast<bool> (d))
        {
            fehler = "store_degraded must be true when present";
            return SnapshotErgebnis::ungueltig;
        }
        storeDegradiert = true;
    }
    const auto mitgliederVar = o->getProperty ("mitglieder");
    const auto* mitglieder = mitgliederVar.getArray();
    if (mitglieder == nullptr
        || mitglieder->size() > nakama::state::maxMainProjectMitglieder)
    {
        fehler = "session_snapshot members are not an array of at most 64";
        return SnapshotErgebnis::ungueltig;
    }

    std::vector<GelesenesMitglied> gelesen;
    std::set<std::string> ids;
    bool doppelteId = false;
    for (const auto& wert : *mitglieder)
    {
        const auto* m = objekt (wert);
        if (m == nullptr)
        {
            fehler = "session member has an unexpected or missing field";
            return SnapshotErgebnis::ungueltig;
        }
        const juce::DynamicObject* member = m;
        if (schemaMinor == nakama::ipc::kJsonSchemaMinor
            && ! exakteFelder (*m, { "adresse", "plugin_kind", "frische" },
                                   { "probe_descriptor", "p2_reject" }))
        {
            fehler = "session member has an unexpected or missing field";
            return SnapshotErgebnis::ungueltig;
        }
        GelesenesMitglied gm;
        std::string binding, mitgliedSession;
        if (! adresse (member->getProperty ("adresse"), binding, mitgliedSession,
                       gm.instanceId, gm.runtimeNonce)
            || binding != erwarteteBindungLokal || mitgliedSession != erwarteteSessionLokal)
        {
            fehler = "session member address does not match the subscribed session";
            return SnapshotErgebnis::ungueltig;
        }
        const auto kind = member->getProperty ("plugin_kind");
        if (! kind.isString()
            || (kind.toString() != "main" && kind.toString() != "passive_probe"
                && kind.toString() != "active_probe" && kind.toString() != "legacy")
            || ! frische (member->getProperty ("frische"), gm.stale, gm.controlAlterMs))
        {
            fehler = "session member kind or freshness is invalid";
            return SnapshotErgebnis::ungueltig;
        }
        gm.pluginKind = kind.toString().toStdString();
        const bool hatDescriptor = schemaMinor == 0 || m->hasProperty ("probe_descriptor");
        const auto descriptor = schemaMinor == 0 ? wert : m->getProperty ("probe_descriptor");
        if (hatDescriptor
            && ! liesDescriptor (descriptor, binding,
                                 mitgliedSession, gm.instanceId, gm.runtimeNonce,
                                 gm.pluginKind, gm.stale, gm.controlAlterMs,
                                 schemaMinor, gm))
        {
            fehler = "probe_descriptor is invalid or disagrees with its member";
            return SnapshotErgebnis::ungueltig;
        }
        if (schemaMinor == nakama::ipc::kJsonSchemaMinor
            && m->hasProperty ("p2_reject")
            && ! liesReject (m->getProperty ("p2_reject"), gm))
        {
            fehler = "p2_reject is invalid";
            return SnapshotErgebnis::ungueltig;
        }
        if (! ids.insert (gm.instanceId).second)
            doppelteId = true;
        gelesen.push_back (std::move (gm));
    }

    std::lock_guard<std::mutex> l (mutex);
    if (session.toString().toStdString() != erwarteteSession
        || erwarteteBindung != erwarteteBindungLokal
        || erwarteteSession != erwarteteSessionLokal)
    {
        fehler = "session_snapshot belongs to another session";
        return SnapshotErgebnis::ungueltig;
    }
    const bool brokerNeu = brokerEpoch != epoch.toString().toStdString();
    brokerEpoch = epoch.toString().toStdString();
    fuehrendesMain = fuehrung.isString() ? fuehrung.toString().toStdString() : std::string();
    sichtZeit = empfangen;

    std::map<std::string, Eintrag> neu;
    for (const auto& gm : gelesen)
    {
        if (gm.pluginKind == "main")
            continue; // Sources sind Sonden; Main-Fuehrung reist separat.
        Eintrag e;
        const auto alt = eintraege.find (gm.instanceId);
        if (alt != eintraege.end() && ! brokerNeu
            && alt->second.zeile.runtimeNonce == gm.runtimeNonce)
            e = alt->second;
        e.fluechtigImSnapshot = true;
        e.zeile.instanceId = gm.instanceId;
        e.zeile.runtimeNonce = gm.runtimeNonce;
        e.zeile.pluginKind = gm.pluginKind;
        e.controlBasisAlterMs = gm.controlAlterMs;
        e.controlEmpfangen = empfangen;
        e.zeile.control = gm.stale ? Control::stale : Control::verbunden;
        e.zeile.descriptorVorhanden = gm.descriptor;
        e.zeile.messpunkt = gm.messpunkt;
        const auto betriebVorher = e.zeile.betrieb;
        e.zeile.betrieb = gm.betrieb;
        if (gm.descriptor && betriebVorher != Betrieb::active && gm.betrieb == Betrieb::active)
            e.resumeBrauchtFrame = true;
        if (gm.betrieb == Betrieb::suspended || gm.betrieb == Betrieb::offline)
            e.resumeBrauchtFrame = true;
        e.zeile.hostBusName = gm.hostName;
        e.zeile.hostMixerIndexVorhanden = gm.hostIndex;
        e.zeile.hostMixerIndex = gm.index;
        const auto persistent = persistenteMitglieder.find (gm.instanceId);
        e.zeile.mitgliedschaft = persistent == persistenteMitglieder.end()
                                    ? Mitgliedschaft::unclassified
                                    : Mitgliedschaft::bestaetigt;
        e.zeile.userLabel = persistent == persistenteMitglieder.end()
                                ? gm.label : persistent->second;
        e.descriptorLabel = gm.label;
        if (! gm.descriptor)
        {
            e.zeile.messpunkt = Messpunkt::unbekannt;
            e.zeile.betrieb = Betrieb::unbekannt;
        }
        if (brokerNeu || (alt != eintraege.end()
                          && alt->second.zeile.runtimeNonce != gm.runtimeNonce))
        {
            e.hatMessZeit = false;
            e.zeile.messung = Messung::missing;
            e.zeile.lautheit = Lautheit::missing;
            e.zeile.lufsPaarVorhanden = false;
        }
        if (gm.reject)
        {
            e.zeile.p2RejectGrund = gm.rejectGrund;
            e.zeile.p2RejectZaehler = gm.rejectZaehler;
            if (gm.rejectZaehler > e.letzterRejectZaehler)
            {
                e.letzterRejectZaehler = gm.rejectZaehler;
                e.zeile.p2RejectAktiv = true;
                if (gm.rejectGrund == "lautheit_ungueltig")
                {
                    e.zeile.lautheit = Lautheit::invalid;
                    e.zeile.lufsPaarVorhanden = false;
                }
                else
                {
                    e.zeile.messung = Messung::invalid;
                }
            }
        }
        aktualisiereName (e);
        aktualisiereAbgeleiteteZustaende (e, empfangen);
        neu.emplace (gm.instanceId, std::move (e));
    }
    for (const auto& [id, label] : persistenteMitglieder)
    {
        if (neu.count (id) != 0) continue;
        Eintrag e;
        const auto alt = eintraege.find (id);
        if (alt != eintraege.end()) e = alt->second;
        e.fluechtigImSnapshot = false;
        e.zeile.instanceId = id;
        e.zeile.mitgliedschaft = Mitgliedschaft::bestaetigt;
        e.zeile.userLabel = label;
        e.zeile.control = Control::getrennt;
        if (e.hatMessZeit && e.zeile.messung != Messung::invalid)
            e.zeile.messung = Messung::stale;
        aktualisiereName (e);
        neu.emplace (id, std::move (e));
    }
    eintraege = std::move (neu);
    subscriptionAktiv = true;
    diagnose = storeDegradiert ? Diagnose::storeDegraded
             : (bestaetigung || doppelteId) ? Diagnose::confirmationRequired
                                            : Diagnose::keine;
    diagnoseHatHandgriff = diagnose == Diagnose::confirmationRequired
                        && ! eigeneMainId.empty() && fuehrendesMain == eigeneMainId;
    stelleZielSicher();
    revidiere();
    return SnapshotErgebnis::uebernommen;
}

bool SourcesModel::uebernehmeP2 (const std::uint8_t* daten, std::size_t laenge,
                                 std::uint8_t schemaMinor, Zeitpunkt empfangen,
                                 juce::String& fehler)
{
    fehler.clear();
    if (schemaMinor > nakama::ipc::kFeatureBatchSchemaMinor)
    {
        fehler = "FeatureBatch schema_minor is newer than this Main reader";
        return false;
    }
    std::vector<nakama::telemetrie::Empfangsframe> frames;
    juce::Array<nakama::telemetrie::Verstoss> verstoesse;
    if (! nakama::telemetrie::lese (daten, laenge, frames, verstoesse))
    {
        fehler = verstoesse.isEmpty() ? "FeatureBatch invalid"
                                      : verstoesse.getFirst().pfad + ":" + verstoesse.getFirst().regel;
        return false;
    }

    std::lock_guard<std::mutex> l (mutex);
    const auto erwarteteBindungLokal = erwarteteBindung;
    const auto erwarteteSessionLokal = erwarteteSession;
    bool geaendert = false;
    sichtZeit = empfangen;
    for (const auto& frame : frames)
    {
        if (frame.projectBindingId != erwarteteBindungLokal
            || frame.sessionEpoch != erwarteteSessionLokal)
            continue;
        auto it = eintraege.find (frame.instanceId);
        if (it == eintraege.end() || ! it->second.fluechtigImSnapshot
            || it->second.zeile.runtimeNonce != frame.runtimeNonce)
            continue; // alter Link/Nonce oder fremde Session mutiert nichts.
        auto& e = it->second;
        e.hatMessZeit = true;
        e.messEmpfangen = empfangen;
        e.zeile.messAlterMs = 0;
        e.zeile.fensterDauerMs = frame.sampleRate > 0.0
            ? static_cast<double> (frame.sampleCount) * 1000.0 / frame.sampleRate : 0.0;
        const bool lautheitsRejectAktiv = e.zeile.p2RejectAktiv
            && e.zeile.p2RejectGrund == "lautheit_ungueltig";
        const bool frameHatLautheitsaussage = frame.lufsPaar
            || frame.lufsIStatus == 1 || frame.lufsIStatus == 2;
        if (! lautheitsRejectAktiv || frameHatLautheitsaussage)
            e.zeile.p2RejectAktiv = false;
        if (frame.lufsPaar)
        {
            e.zeile.lautheit = Lautheit::gueltig;
            e.zeile.lufsPaarVorhanden = true;
            e.zeile.lufsI = frame.lufsI;
            e.zeile.lufsIUnsicherheitLu = frame.lufsIUnsicherheitLu;
            e.zeile.messung = Messung::fresh;
        }
        else
        {
            e.zeile.lufsPaarVorhanden = false;
            if (frame.lufsIStatus == 1)
            {
                e.zeile.lautheit = Lautheit::measuring;
                e.zeile.messung = Messung::fresh;
            }
            else if (frame.lufsIStatus == 2)
            {
                e.zeile.lautheit = Lautheit::noSignal;
                e.zeile.messung = Messung::fresh;
            }
            else
            {
                if (! lautheitsRejectAktiv)
                    e.zeile.lautheit = Lautheit::missing;
                e.zeile.messung = Messung::partial;
            }
        }
        if (e.zeile.betrieb == Betrieb::active)
            e.resumeBrauchtFrame = false;
        aktualisiereAbgeleiteteZustaende (e, empfangen);
        geaendert = true;
    }
    if (geaendert) revidiere();
    return geaendert;
}

void SourcesModel::tick (Zeitpunkt jetzt)
{
    std::lock_guard<std::mutex> l (mutex);
    sichtZeit = jetzt;
    bool geaendert = false;
    for (auto& [_, e] : eintraege)
    {
        const auto vorherControl = e.zeile.control;
        const auto vorherMessung = e.zeile.messung;
        const auto vorherControlAlter = e.zeile.controlAlterMs;
        const auto vorherMessAlter = e.zeile.messAlterMs;
        aktualisiereAbgeleiteteZustaende (e, jetzt);
        geaendert = geaendert || vorherControl != e.zeile.control
                   || vorherMessung != e.zeile.messung
                   || vorherControlAlter != e.zeile.controlAlterMs
                   || vorherMessAlter != e.zeile.messAlterMs;
    }
    if (geaendert) revidiere();
}

SourcesModel::Sicht SourcesModel::sicht() const
{
    std::lock_guard<std::mutex> l (mutex);
    Sicht s;
    s.revision = revision;
    s.subscriptionAktiv = subscriptionAktiv;
    s.diagnose = diagnose;
    s.diagnoseHatHandgriff = diagnoseHatHandgriff;
    s.fuehrendesMain = fuehrendesMain;
    s.mainDarfSchreiben = subscriptionAktiv
                       && ! eigeneMainId.empty()
                       && fuehrendesMain == eigeneMainId;
    for (const auto& [_, e] : eintraege)
        s.quellen.push_back (e.zeile);
    std::sort (s.quellen.begin(), s.quellen.end(), [] (const Zeile& a, const Zeile& b)
    {
        if (a.hostMixerIndexVorhanden != b.hostMixerIndexVorhanden)
            return a.hostMixerIndexVorhanden;
        if (a.hostMixerIndexVorhanden && a.hostMixerIndex != b.hostMixerIndex)
            return a.hostMixerIndex < b.hostMixerIndex;
        const auto af = sortierSchluessel (a.sichtbarerName);
        const auto bf = sortierSchluessel (b.sichtbarerName);
        if (af != bf) return af < bf;
        const auto ac = a.sichtbarerName.toStdString();
        const auto bc = b.sichtbarerName.toStdString();
        if (ac != bc) return ac < bc;
        return a.instanceId < b.instanceId;
    });
    for (auto& q : s.quellen)
        q.hauptziel = q.instanceId == hauptziel;
    return s;
}

bool SourcesModel::waehleHauptziel (const std::string& instanceId)
{
    std::lock_guard<std::mutex> l (mutex);
    if (eintraege.count (instanceId) == 0 || hauptziel == instanceId)
        return false;
    hauptziel = instanceId;
    revidiere();
    return true;
}

std::string SourcesModel::aktuellesHauptziel() const
{
    std::lock_guard<std::mutex> l (mutex);
    return hauptziel;
}

bool SourcesModel::istAktuellesHauptziel (const std::string& instanceId) const
{
    std::lock_guard<std::mutex> l (mutex);
    return ! instanceId.empty() && hauptziel == instanceId
        && eintraege.count (instanceId) != 0;
}

void SourcesModel::setzeFindings (const std::string& instanceId, int offen)
{
    std::lock_guard<std::mutex> l (mutex);
    const auto it = eintraege.find (instanceId);
    if (it == eintraege.end()) return;
    const int n = std::max (0, offen);
    if (it->second.zeile.findingsOffen == n) return;
    it->second.zeile.findingsOffen = n;
    revidiere();
}

void SourcesModel::setzeCapabilityEvidenz (const std::string& instanceId,
                                           CapabilityEvidenz evidenz)
{
    std::lock_guard<std::mutex> l (mutex);
    const auto it = eintraege.find (instanceId);
    if (it == eintraege.end() || it->second.zeile.capabilityEvidenz == evidenz) return;
    it->second.zeile.capabilityEvidenz = evidenz;
    revidiere();
}

void SourcesModel::setzeDiagnoseFuerSichtbeweis (Diagnose d, bool handgriff)
{
    std::lock_guard<std::mutex> l (mutex);
    diagnose = d;
    diagnoseHatHandgriff = handgriff;
    revidiere();
}

#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
void SourcesModel::setzeFixtureFuerTest (Sicht fixture)
{
    std::lock_guard<std::mutex> l (mutex);
    eintraege.clear();
    hauptziel.clear();
    for (auto& q : fixture.quellen)
    {
        Eintrag e;
        e.zeile = q;
        e.fluechtigImSnapshot = q.control != Control::nieVerbunden;
        e.hatMessZeit = q.messung != Messung::missing;
        eintraege.emplace (q.instanceId, std::move (e));
        if (q.hauptziel) hauptziel = q.instanceId;
    }
    subscriptionAktiv = fixture.subscriptionAktiv;
    diagnose = fixture.diagnose;
    diagnoseHatHandgriff = fixture.diagnoseHatHandgriff;
    fuehrendesMain = fixture.fuehrendesMain;
    eigeneMainId = fixture.mainDarfSchreiben ? fixture.fuehrendesMain : std::string();
    revision = std::max (revision + 1, fixture.revision);
    stelleZielSicher();
}
#endif

void SourcesModel::stelleZielSicher()
{
    if (! hauptziel.empty() && eintraege.count (hauptziel) != 0)
        return;
    hauptziel = eintraege.empty() ? std::string() : eintraege.begin()->first;
}

void SourcesModel::aktualisiereName (Eintrag& e)
{
    if (hostNameGueltig (e.zeile.hostBusName))
    {
        e.zeile.sichtbarerName = e.zeile.hostBusName;
        e.zeile.namensherkunft = Namensherkunft::host;
    }
    else if (e.zeile.userLabel.isNotEmpty())
    {
        e.zeile.sichtbarerName = e.zeile.userLabel;
        e.zeile.namensherkunft = Namensherkunft::userLabel;
    }
    else
    {
        e.zeile.sichtbarerName = platzhalter (e.zeile.instanceId);
        e.zeile.namensherkunft = Namensherkunft::platzhalter;
    }
}

void SourcesModel::aktualisiereAbgeleiteteZustaende (Eintrag& e, Zeitpunkt jetzt)
{
    if (e.fluechtigImSnapshot
        && (e.zeile.control == Control::verbunden || e.zeile.control == Control::stale))
    {
        const auto delta = jetzt >= e.controlEmpfangen
            ? std::chrono::duration_cast<std::chrono::milliseconds> (
                  jetzt - e.controlEmpfangen).count() : 0;
        const auto d = static_cast<std::uint64_t> (std::max<std::int64_t> (0, delta));
        e.zeile.controlAlterMs = e.controlBasisAlterMs > std::numeric_limits<std::uint64_t>::max() - d
            ? std::numeric_limits<std::uint64_t>::max() : e.controlBasisAlterMs + d;
        if (e.zeile.controlAlterMs > controlStaleNachMs)
            e.zeile.control = Control::stale;
    }
    if (e.hatMessZeit)
    {
        const auto delta = jetzt >= e.messEmpfangen
            ? std::chrono::duration_cast<std::chrono::milliseconds> (
                  jetzt - e.messEmpfangen).count() : 0;
        e.zeile.messAlterMs = static_cast<std::uint64_t> (
            std::max<std::int64_t> (0, delta));
    }

    if (e.zeile.p2RejectAktiv
        && e.zeile.p2RejectGrund != "lautheit_ungueltig")
    {
        e.zeile.messung = Messung::invalid;
        return;
    }
    if (! e.hatMessZeit)
    {
        e.zeile.messung = Messung::missing;
        return;
    }
    if (e.zeile.control != Control::verbunden
        || e.zeile.betrieb == Betrieb::suspended
        || e.zeile.betrieb == Betrieb::offline || e.resumeBrauchtFrame)
    {
        e.zeile.messung = Messung::stale;
        return;
    }
    const auto fensterFrist = messStaleFristMs (e.zeile.fensterDauerMs);
    if (e.zeile.messAlterMs > fensterFrist)
        e.zeile.messung = Messung::stale;
}

std::uint64_t SourcesModel::messStaleFristMs (double fensterDauerMs) noexcept
{
    if (! std::isfinite (fensterDauerMs) || fensterDauerMs < 0.0)
        return messungMindestensStaleNachMs;
    const auto roh = std::max (2.0 * fensterDauerMs,
                               static_cast<double> (messungMindestensStaleNachMs));
    if (roh >= static_cast<double> (std::numeric_limits<std::uint64_t>::max()))
        return std::numeric_limits<std::uint64_t>::max();
    return static_cast<std::uint64_t> (std::ceil (roh));
}

juce::String SourcesModel::platzhalter (const std::string& instanceId)
{
    return "Unnamed " + juce::String (instanceId).substring (0, 8);
}

std::string SourcesModel::sortierSchluessel (const juce::String& text)
{
    // JUCE liefert Unicode-Lowercase je Codepoint. Die drei haeufigen
    // Full-Casefold-Erweiterungen, die daraus nicht folgen, werden explizit
    // expandiert; danach ist der UTF-8-Bytevergleich zugleich Codepointordnung.
    juce::String out;
    for (const auto c : text)
    {
        if (c == 0x00df || c == 0x1e9e) { out += "ss"; continue; }
        if (c == 0x03c2) { out += juce::String::charToString (0x03c3); continue; }
        switch (c)
        {
            case 0xfb00: out += "ff"; continue;
            case 0xfb01: out += "fi"; continue;
            case 0xfb02: out += "fl"; continue;
            case 0xfb03: out += "ffi"; continue;
            case 0xfb04: out += "ffl"; continue;
            case 0xfb05: case 0xfb06: out += "st"; continue;
            default: break;
        }
        out += juce::String::charToString (juce::CharacterFunctions::toLowerCase (c));
    }
    return out.toStdString();
}

const char* wort (SourcesModel::Control v)
{
    switch (v) { case SourcesModel::Control::nieVerbunden: return "never connected";
        case SourcesModel::Control::verbunden: return "connected";
        case SourcesModel::Control::stale: return "stale";
        case SourcesModel::Control::getrennt: return "disconnected";
        case SourcesModel::Control::fehler: return "error"; }
    return "error";
}
const char* wort (SourcesModel::Messung v)
{
    switch (v) { case SourcesModel::Messung::missing: return "missing";
        case SourcesModel::Messung::fresh: return "fresh";
        case SourcesModel::Messung::partial: return "partial";
        case SourcesModel::Messung::stale: return "stale";
        case SourcesModel::Messung::invalid: return "invalid"; }
    return "invalid";
}
const char* wort (SourcesModel::Betrieb v)
{
    switch (v) { case SourcesModel::Betrieb::unbekannt: return "unclassified";
        case SourcesModel::Betrieb::active: return "active";
        case SourcesModel::Betrieb::suspended: return "suspended";
        case SourcesModel::Betrieb::offline: return "offline"; }
    return "unclassified";
}
const char* wort (SourcesModel::Lautheit v)
{
    switch (v) { case SourcesModel::Lautheit::missing: return "missing";
        case SourcesModel::Lautheit::measuring: return "measuring";
        case SourcesModel::Lautheit::noSignal: return "no signal";
        case SourcesModel::Lautheit::gueltig: return "measured";
        case SourcesModel::Lautheit::invalid: return "invalid"; }
    return "invalid";
}
const char* wort (SourcesModel::Namensherkunft v)
{
    switch (v) { case SourcesModel::Namensherkunft::host: return "Host name";
        case SourcesModel::Namensherkunft::userLabel: return "User label";
        case SourcesModel::Namensherkunft::platzhalter: return "Placeholder"; }
    return "Placeholder";
}
const char* wort (SourcesModel::Messpunkt v)
{
    switch (v) { case SourcesModel::Messpunkt::unbekannt: return "unclassified";
        case SourcesModel::Messpunkt::insert: return "insert";
        case SourcesModel::Messpunkt::pre: return "pre";
        case SourcesModel::Messpunkt::post: return "post"; }
    return "unclassified";
}
const char* wort (SourcesModel::Diagnose v)
{
    switch (v) { case Diagnose::keine: return "";
        case Diagnose::brokerUnavailable: return "Broker unavailable";
        case Diagnose::authenticating: return "Authenticating";
        case Diagnose::confirmationRequired: return "Join confirmation required";
        case Diagnose::incompatible: return "Incompatible broker or protocol";
        case Diagnose::storeDegraded: return "Store degraded"; }
    return "Incompatible broker or protocol";
}

} // namespace eqcop
