// SONDE-012 B3b — designneutrales Quellenmodell von Gen/Main.
//
// Dieses Bein startet weder Broker noch Produktionspipe. JSON-Snapshots und
// die gepinnten FlatBuffers-Fixtures laufen durch dieselben handgeschriebenen
// Leser wie das Produkt; nur reine Sicht-/Sortierfaelle benutzen den
// ausdruecklichen Fixture-Haken.

#include <juce_core/juce_core.h>

#include "NakamaTelemetrie.h"
#include "SourcesModel.h"
#include "TelemetryClient.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace
{
using Model = eqcop::SourcesModel;
using Zeitpunkt = Model::Zeitpunkt;

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

std::string hex (unsigned long long wert)
{
    std::ostringstream s;
    s << std::hex << std::setfill ('0') << std::setw (32) << wert;
    return s.str();
}

std::string jsonText (const juce::String& s)
{
    return juce::JSON::toString (juce::var (s), true).toStdString();
}

struct Mitglied
{
    std::string id = hex (20), nonce = hex (200), kind = "active_probe";
    bool descriptor = true, stale = false;
    std::uint64_t controlAlter = 0;
    std::string position = "insert", betrieb = "active";
    juce::String label;
    std::optional<juce::String> hostName;
    std::optional<std::uint64_t> hostIndex;
    std::optional<std::pair<std::string, std::uint64_t>> reject;
};

const char* capabilities()
{
    return R"({"host_context_presence":"supported","project_time_samples":"supported","sample_accurate_automation":"supported","presentation_latency":"unsupported","aux_compare_pre":"supported","aux_priority_sidechain":"unsupported","contribution_aux":"supported","float64_processing":"unsupported","binary_telemetry":"supported","remote_control":"unsupported"})";
}

std::string adresse (const std::string& binding, const std::string& session,
                     const std::string& id, const std::string& nonce)
{
    return R"({"logon_sid":"S-1-5-21-1","project_binding_id":")" + binding
         + R"(","session_epoch":")" + session + R"(","instance_id":")"
         + id + R"(","runtime_nonce":")" + nonce + R"("})";
}

std::string mitgliedJson (const Mitglied& m, const std::string& binding,
                          const std::string& session)
{
    const auto a = adresse (binding, session, m.id, m.nonce);
    const auto frische = std::string (R"({"stale":)")
        + (m.stale ? "true" : "false") + R"(,"letzter_kontakt_ms":)"
        + std::to_string (m.controlAlter) + "}";
    std::string aus = R"({"adresse":)" + a + R"(,"plugin_kind":")" + m.kind
                    + R"(","frische":)" + frische;
    if (m.descriptor)
    {
        aus += R"(,"probe_descriptor":{"adresse":)" + a
             + R"(,"plugin_kind":")" + m.kind
             + R"(","measurement_position":")" + m.position
             + R"(","aussageklasse":"beobachtend","betrieb":")" + m.betrieb
             + R"(","label":)" + jsonText (m.label)
             + R"(,"capabilities":)" + capabilities()
             + R"(,"frische":)" + frische;
        if (m.hostName)
            aus += R"(,"host_bus_name":)" + jsonText (*m.hostName);
        if (m.hostIndex)
            aus += R"(,"host_mixer_index":)" + std::to_string (*m.hostIndex);
        aus += "}";
    }
    if (m.reject)
        aus += R"(,"p2_reject":{"grund":")" + m.reject->first
             + R"(","zaehler":)" + std::to_string (m.reject->second) + "}";
    return aus + "}";
}

std::string snapshot (const std::vector<Mitglied>& mitglieder,
                      const std::string& binding = hex (1),
                      const std::string& session = hex (2),
                      const std::string& broker = hex (8),
                      const std::string& fuehrend = hex (10),
                      bool bestaetigung = false,
                      int storeDegraded = 0)
{
    std::string liste;
    for (const auto& m : mitglieder)
    {
        if (! liste.empty()) liste += ',';
        liste += mitgliedJson (m, binding, session);
    }
    return R"({"type":"session_snapshot","session_epoch":")" + session
         + R"(","broker_epoch":")" + broker + R"(","fuehrendes_main":")"
         + fuehrend + R"(","beitritt_bestaetigung_noetig":)"
         + (bestaetigung ? "true" : "false")
         + (storeDegraded == 0 ? "" : storeDegraded == 1
                                      ? ",\"store_degraded\":true"
                                      : ",\"store_degraded\":false")
         + R"(,"mitglieder":[)"
         + liste + "]}";
}

std::string snapshotMinor0 (const Mitglied& m,
                            const std::string& binding = hex (1),
                            const std::string& session = hex (2))
{
    const auto a = adresse (binding, session, m.id, m.nonce);
    const auto frische = std::string (R"({"stale":)")
        + (m.stale ? "true" : "false") + R"(,"letzter_kontakt_ms":)"
        + std::to_string (m.controlAlter) + "}";
    const auto descriptor = R"({"adresse":)" + a
        + R"(,"plugin_kind":")" + m.kind
        + R"(","measurement_position":")" + m.position
        + R"(","aussageklasse":"beobachtend","label":)" + jsonText (m.label)
        + R"(,"capabilities":)" + capabilities()
        + R"(,"frische":)" + frische + "}";
    return R"({"type":"session_snapshot","session_epoch":")" + session
        + R"(","broker_epoch":")" + hex (8)
        + R"(","fuehrendes_main":")" + hex (10)
        + R"(","beitritt_bestaetigung_noetig":false,"mitglieder":[)"
        + descriptor + "]}";
}

bool uebernehme (Model& m, const std::string& json, Zeitpunkt t)
{
    juce::String grund;
    return m.uebernehmeSessionSnapshot (json, t, grund)
        == Model::SnapshotErgebnis::uebernommen;
}

const Model::Zeile* einzige (const Model::Sicht& s)
{
    return s.quellen.size() == 1 ? &s.quellen.front() : nullptr;
}

juce::File finde (const juce::String& relativ)
{
    auto f = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    if (f.existsAsFile()) return f;
    auto p = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                 .getParentDirectory();
    for (int i = 0; i < 8 && p.exists(); ++i)
    {
        f = p.getChildFile (relativ);
        if (f.existsAsFile()) return f;
        p = p.getParentDirectory();
    }
    return f;
}

struct P2Fixture
{
    juce::MemoryBlock bytes;
    nakama::telemetrie::Empfangsframe frame;
    bool ok = false;
};

P2Fixture p2Fixture (const char* name)
{
    P2Fixture f;
    const auto datei = finde ("eq-copilot/fixtures/v3/flatbuffers/gueltig/"
                              + juce::String (name) + ".bin");
    if (! datei.loadFileAsData (f.bytes)) return f;
    std::vector<nakama::telemetrie::Empfangsframe> frames;
    juce::Array<nakama::telemetrie::Verstoss> verstoesse;
    f.ok = nakama::telemetrie::lese (
        static_cast<const std::uint8_t*> (f.bytes.getData()), f.bytes.getSize(),
        frames, verstoesse) && frames.size() == 1;
    if (f.ok) f.frame = frames.front();
    return f;
}

bool p2InModell (Model& model, const P2Fixture& p2, Zeitpunkt t)
{
    juce::String grund;
    return p2.ok && model.uebernehmeP2 (
        static_cast<const std::uint8_t*> (p2.bytes.getData()), p2.bytes.getSize(),
        nakama::ipc::kFeatureBatchSchemaMinor, t, grund);
}

// SourcesModel enthaelt absichtlich ein Mutex und ist nicht bewegbar. Dieser
// Helfer initialisiert daher ein vom Aufrufer gehaltenes Modell.
void initialisiereFuer (Model& m, const P2Fixture& p2, Zeitpunkt t)
{
    m.beginneSubscription (p2.frame.projectBindingId, p2.frame.sessionEpoch, hex (10));
    Mitglied q;
    q.id = p2.frame.instanceId;
    q.nonce = p2.frame.runtimeNonce;
    uebernehme (m, snapshot ({ q }, p2.frame.projectBindingId,
                             p2.frame.sessionEpoch), t);
}

Model::Zeile sichtZeile (std::string id, juce::String name,
                         std::optional<std::uint64_t> index = {})
{
    Model::Zeile q;
    q.instanceId = std::move (id);
    q.sichtbarerName = std::move (name);
    q.namensherkunft = Model::Namensherkunft::userLabel;
    q.mitgliedschaft = Model::Mitgliedschaft::bestaetigt;
    q.control = Model::Control::verbunden;
    if (index)
    {
        q.hostMixerIndexVorhanden = true;
        q.hostMixerIndex = *index;
    }
    return q;
}

} // namespace

int main()
{
    const auto t0 = Zeitpunkt {} + std::chrono::seconds (10);
    const auto pair = p2Fixture ("loudness-i-pair");
    const auto collecting = p2Fixture ("loudness-i-collecting");
    const auto gated = p2Fixture ("loudness-i-gated");
    const auto partial = p2Fixture ("ohne-optionale-kennzahlen");
    pruefe (pair.ok && collecting.ok && gated.ok && partial.ok,
            "flatbuffer_fixtures_are_read_by_product_reader");

    {
        Model m;
        const bool leer = m.sicht().quellen.empty();
        m.setzePersistenteMitglieder ({ { juce::String (hex (20)), "Piano" } });
        const auto s = m.sicht();
        const auto* q = einzige (s);
        pruefe (leer && q != nullptr && q->mitgliedschaft == Model::Mitgliedschaft::bestaetigt
                && q->control == Model::Control::getrennt,
                "never_connected_unknown_vs_confirmed_member");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied a, b; b.id = hex (21); b.nonce = hex (201);
        const bool erster = uebernehme (m, snapshot ({ a, b }), t0);
        const bool zweiter = uebernehme (m, snapshot ({ b }), t0 + std::chrono::milliseconds (1));
        pruefe (erster && zweiter && m.sicht().quellen.size() == 1
                && m.sicht().quellen[0].instanceId == b.id,
                "absolute_snapshot_replaces_the_transient_model");
        m.controlEnde();
        const auto ende = m.sicht();
        m.beginneSubscription (hex (1), hex (2), hex (10));
        pruefe (! ende.subscriptionAktiv
                && ! ende.mainDarfSchreiben
                && ende.quellen[0].control == Model::Control::getrennt
                && ! m.sicht().subscriptionAktiv,
                "control_end_invalidates_and_reconnect_requires_subscribe");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied mainPre;
        mainPre.kind = "main";
        mainPre.position = "pre";
        juce::String grund;
        pruefe (m.uebernehmeSessionSnapshot (snapshot ({ mainPre }), t0, grund)
                    == Model::SnapshotErgebnis::ungueltig,
                "snapshot_reader_enforces_the_strict_descriptor_branch");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied legacy;
        legacy.label = "Minor zero label";
        juce::String grund;
        const bool minor0Alt = m.uebernehmeSessionSnapshot (
            snapshotMinor0 (legacy), 0, t0, grund)
                == Model::SnapshotErgebnis::uebernommen;
        legacy.hostName = "Minor one host";
        const auto minor1Payload = snapshot ({ legacy });
        const bool minor0LehntMinor1Feld = m.uebernehmeSessionSnapshot (
            minor1Payload, 0, t0, grund) == Model::SnapshotErgebnis::ungueltig;
        const bool minor1Traegt = m.uebernehmeSessionSnapshot (
            minor1Payload, 1, t0, grund) == Model::SnapshotErgebnis::uebernommen
            && m.sicht().quellen.front().hostBusName == "Minor one host";
        pruefe (minor0Alt && minor0LehntMinor1Feld && minor1Traegt,
                "json_minor_selects_historical_or_current_session_contract");
    }
    {
        Model m;
        initialisiereFuer (m, pair, t0);
        const auto vor = m.sicht().quellen.front();
        const bool p2ok = p2InModell (m, pair, t0);
        const auto nach = m.sicht().quellen.front();
        pruefe (vor.messung == Model::Messung::missing && p2ok
                && nach.messung == Model::Messung::fresh,
                "first_frame_missing_to_fresh");
        pruefe (nach.fensterDauerMs > 0.0 && nach.messAlterMs == 0,
                "age_and_window_are_not_hidden");
        m.tick (t0 + std::chrono::milliseconds (1000));
        const bool gleichFrisch = m.sicht().quellen.front().messung == Model::Messung::fresh;
        m.tick (t0 + std::chrono::milliseconds (1001));
        pruefe (gleichFrisch && m.sicht().quellen.front().messung == Model::Messung::stale
                && Model::messStaleFristMs (600.0) == 1200,
                "stale_keeps_row_and_downgrades_measurement");
    }
    {
        Model m;
        m.setzePersistenteMitglieder ({ { juce::String (hex (20)), "Piano" } });
        m.beginneSubscription (hex (1), hex (2), hex (10));
        uebernehme (m, snapshot ({ Mitglied {} }), t0);
        uebernehme (m, snapshot ({}), t0 + std::chrono::milliseconds (1));
        const auto* q = einzige (m.sicht());
        pruefe (q != nullptr && q->mitgliedschaft == Model::Mitgliedschaft::bestaetigt
                && q->control == Model::Control::getrennt,
                "eviction_does_not_delete_confirmed_membership");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        uebernehme (m, snapshot ({ Mitglied {} }, hex (1), hex (2), hex (8),
                                 hex (10), true), t0);
        const auto s = m.sicht();
        pruefe (s.diagnose == Model::Diagnose::confirmationRequired
                && s.diagnoseHatHandgriff && s.mainDarfSchreiben,
                "ambiguous_join_exposes_the_real_leading_main_resolution_action");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        juce::String grund;
        const auto degraded = m.uebernehmeSessionSnapshot (
            snapshot ({}, hex (1), hex (2), hex (8), hex (10), false, 1), t0, grund);
        const bool sichtbar = degraded == Model::SnapshotErgebnis::uebernommen
                           && m.sicht().diagnose == Model::Diagnose::storeDegraded
                           && ! m.sicht().diagnoseHatHandgriff;
        const auto falseErgebnis = m.uebernehmeSessionSnapshot (
            snapshot ({}, hex (1), hex (2), hex (8), hex (10), false, 2), t0, grund);
        const auto ohne = m.uebernehmeSessionSnapshot (
            snapshot ({}), t0 + std::chrono::milliseconds (1), grund);
        pruefe (sichtbar && falseErgebnis == Model::SnapshotErgebnis::ungueltig
                && ohne == Model::SnapshotErgebnis::uebernommen
                && m.sicht().diagnose != Model::Diagnose::storeDegraded,
                "store_degraded_is_strict_true_only_and_absence_clears_the_diagnosis");
    }
    {
        Model m;
        Model::Sicht f;
        f.quellen = { sichtZeile (hex (20), "A"), sichtZeile (hex (21), "B") };
        f.quellen[0].hauptziel = true;
        m.setzeFixtureFuerTest (f);
        const bool erster = m.istAktuellesHauptziel (hex (20));
        m.waehleHauptziel (hex (21));
        const auto s = m.sicht();
        const auto n = std::count_if (s.quellen.begin(), s.quellen.end(),
                                      [] (const auto& q) { return q.hauptziel; });
        pruefe (erster && m.istAktuellesHauptziel (hex (21)) && n == 1,
                "actions_address_only_current_main_target_across_target_change");
        pruefe (s.quellen[0].hauptziel != s.quellen[1].hauptziel,
                "references_expose_no_source_action");
    }
    {
        Model m;
        initialisiereFuer (m, pair, t0);
        p2InModell (m, pair, t0);
        Mitglied gewechselt;
        gewechselt.id = pair.frame.instanceId;
        gewechselt.nonce = hex (999);
        uebernehme (m, snapshot ({ gewechselt }, pair.frame.projectBindingId,
                                 pair.frame.sessionEpoch), t0 + std::chrono::milliseconds (1));
        const bool altIgnoriert = ! p2InModell (m, pair, t0 + std::chrono::milliseconds (2));
        const auto q = m.sicht().quellen.front();
        pruefe (altIgnoriert && q.runtimeNonce == hex (999)
                && q.messung == Model::Messung::missing,
                "bridge_pid_change_has_no_false_member_or_address");
    }
    {
        Model m;
        nakama::ipc::ControlClient::Snapshot s;
        s.status = nakama::ipc::ControlClient::Status::verbindet;
        m.setzeControlTransport (s);
        const bool auth = m.sicht().diagnose == Model::Diagnose::authenticating;
        s.status = nakama::ipc::ControlClient::Status::getrennt;
        s.letzterFehler = "protocol mismatch";
        m.setzeControlTransport (s);
        const bool inkompatibel = m.sicht().diagnose == Model::Diagnose::incompatible;
        m.controlEnde();
        const bool unverfuegbar = m.sicht().diagnose == Model::Diagnose::incompatible;
        m.setzeDiagnoseFuerSichtbeweis (Model::Diagnose::storeDegraded, false);
        pruefe (auth && inkompatibel && unverfuegbar
                && m.sicht().diagnose == Model::Diagnose::storeDegraded,
                "diagnostics_are_distinct_and_actions_are_live");
    }
    {
        Model m;
        Model::Sicht f;
        f.quellen = { sichtZeile (hex (20), "A"), sichtZeile (hex (21), "B") };
        m.setzeFixtureFuerTest (f);
        m.setzeFindings (hex (20), 3);
        const auto s = m.sicht();
        const auto a = std::find_if (s.quellen.begin(), s.quellen.end(),
                                     [] (const auto& q) { return q.instanceId == hex (20); });
        const auto b = std::find_if (s.quellen.begin(), s.quellen.end(),
                                     [] (const auto& q) { return q.instanceId == hex (21); });
        pruefe (a != s.quellen.end() && b != s.quellen.end()
                && a->findingsOffen == 3 && b->findingsOffen == 0,
                "findings_count_tracks_only_open_findings_of_its_source");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied q;
        q.label = "User Keys";
        q.hostName = "Host Keys";
        q.hostIndex = 7;
        uebernehme (m, snapshot ({ q }), t0);
        auto z = m.sicht().quellen.front();
        const bool host = z.sichtbarerName == "Host Keys"
                       && z.namensherkunft == Model::Namensherkunft::host
                       && z.hostMixerIndex == 7;
        q.hostName.reset(); q.hostIndex.reset();
        uebernehme (m, snapshot ({ q }), t0 + std::chrono::milliseconds (1));
        z = m.sicht().quellen.front();
        pruefe (host && z.sichtbarerName == "User Keys"
                && z.namensherkunft == Model::Namensherkunft::userLabel,
                "runtime_host_to_fallback_has_no_blank_or_connection_order");
        q.hostName = "Host Again";
        uebernehme (m, snapshot ({ q }), t0 + std::chrono::milliseconds (2));
        pruefe (m.sicht().quellen.front().sichtbarerName == "Host Again",
                "runtime_fallback_to_host_switch_is_atomic");
    }
    {
        Model m;
        Model::Sicht f;
        f.quellen = { sichtZeile (hex (21), "zeta"),
                      sichtZeile (hex (22), juce::String::fromUTF8 ("\xc3\x84pfel")),
                      sichtZeile (hex (23), "beta", 8),
                      sichtZeile (hex (24), "alpha", 2) };
        m.setzeFixtureFuerTest (f);
        const auto s = m.sicht();
        pruefe (s.quellen[0].instanceId == hex (24)
                && s.quellen[1].instanceId == hex (23),
                "partial_indices_sort_indexed_first_then_fallback");
        pruefe (s.quellen[2].sichtbarerName == "zeta"
                && s.quellen[3].sichtbarerName == juce::String::fromUTF8 ("\xc3\x84pfel"),
                "no_index_sorts_casefold_codepoint_then_instance");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        uebernehme (m, snapshot ({ Mitglied {} }, hex (1), hex (2), hex (8), hex (99)), t0);
        const bool ref = ! m.sicht().mainDarfSchreiben;
        uebernehme (m, snapshot ({ Mitglied {} }, hex (1), hex (2), hex (8), hex (10)), t0);
        pruefe (ref && m.sicht().mainDarfSchreiben,
                "main_handover_is_visible");
    }
    {
        Model m;
        Model::Sicht f;
        auto q = sichtZeile (hex (20), "Fallback");
        q.descriptorVorhanden = true;
        f.quellen = { q };
        m.setzeFixtureFuerTest (f);
        m.setzeCapabilityEvidenz (hex (20), Model::CapabilityEvidenz::gemessenJa);
        const auto z = m.sicht().quellen.front();
        pruefe (z.descriptorVorhanden
                && z.capabilityEvidenz == Model::CapabilityEvidenz::gemessenJa,
                "measured_yes_still_uses_only_descriptor_values");
        m.setzeCapabilityEvidenz (hex (20), Model::CapabilityEvidenz::nochNichtGemessen);
        pruefe (m.sicht().quellen.front().descriptorVorhanden,
                "unmeasured_uses_fallback_but_never_hides_real_descriptor");
    }
    {
        Model a, b, c, d;
        initialisiereFuer (a, collecting, t0);
        initialisiereFuer (b, gated, t0);
        initialisiereFuer (c, partial, t0);
        p2InModell (a, collecting, t0);
        p2InModell (b, gated, t0);
        p2InModell (c, partial, t0);
        pruefe (a.sicht().quellen.front().lautheit == Model::Lautheit::measuring
                && b.sicht().quellen.front().lautheit == Model::Lautheit::noSignal
                && c.sicht().quellen.front().lautheit == Model::Lautheit::missing,
                "missing_collecting_and_gated_map_to_missing_measuring_no_signal");
    }
    {
        Model m;
        initialisiereFuer (m, pair, t0);
        p2InModell (m, pair, t0);
        const auto q = m.sicht().quellen.front();
        pruefe (q.lautheit == Model::Lautheit::gueltig && q.lufsPaarVorhanden
                && std::isfinite (q.lufsI) && std::isfinite (q.lufsIUnsicherheitLu),
                "valid_loudness_pair_is_co_located_with_confidence");
        Mitglied reject;
        reject.id = pair.frame.instanceId; reject.nonce = pair.frame.runtimeNonce;
        reject.reject = std::make_pair (std::string ("lautheit_ungueltig"), 1ULL);
        uebernehme (m, snapshot ({ reject }, pair.frame.projectBindingId,
                                 pair.frame.sessionEpoch), t0 + std::chrono::milliseconds (1));
        const auto inv = m.sicht().quellen.front();
        pruefe (inv.messung == Model::Messung::fresh
                && inv.lautheit == Model::Lautheit::invalid
                && ! inv.lufsPaarVorhanden,
                "half_or_nonfinite_pair_invalidates_only_loudness_without_number");

        Model p2DannP1, p1DannP2;
        initialisiereFuer (p2DannP1, partial, t0);
        initialisiereFuer (p1DannP2, partial, t0);
        p2InModell (p2DannP1, partial, t0);
        uebernehme (p2DannP1,
                    snapshot ({ reject }, partial.frame.projectBindingId,
                              partial.frame.sessionEpoch),
                    t0 + std::chrono::milliseconds (1));
        uebernehme (p1DannP2,
                    snapshot ({ reject }, partial.frame.projectBindingId,
                              partial.frame.sessionEpoch), t0);
        p2InModell (p1DannP2, partial, t0 + std::chrono::milliseconds (1));
        const auto a = p2DannP1.sicht().quellen.front();
        const auto b = p1DannP2.sicht().quellen.front();
        pruefe (a.messung == Model::Messung::partial
                && b.messung == Model::Messung::partial
                && a.lautheit == Model::Lautheit::invalid
                && b.lautheit == Model::Lautheit::invalid
                && a.p2RejectAktiv && b.p2RejectAktiv
                && ! a.lufsPaarVorhanden && ! b.lufsPaarVorhanden,
                "loudness_reject_is_deterministic_for_p1_p2_both_orders");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied q; q.label = "";
        uebernehme (m, snapshot ({ q }), t0);
        pruefe (m.sicht().quellen.front().sichtbarerName
                    == "Unnamed " + juce::String (q.id).substring (0, 8),
                "empty_label_uses_unnamed_plus_instance_prefix");
        m.setzePersistenteMitglieder ({ { juce::String (q.id), "Stored" } });
        pruefe (m.sicht().quellen.front().sichtbarerName == "Stored"
                && ! m.sicht().quellen.front().hostMixerIndexVorhanden,
                "no_host_context_uses_persisted_untrusted_label_without_order_claim");
    }
    {
        for (const auto* pos : { "insert", "pre", "post" })
        {
            Model m;
            m.beginneSubscription (hex (1), hex (2), hex (10));
            Mitglied q; q.position = pos;
            uebernehme (m, snapshot ({ q }), t0);
            const auto erwartet = std::string (pos) == "insert" ? Model::Messpunkt::insert
                                : std::string (pos) == "pre" ? Model::Messpunkt::pre
                                                              : Model::Messpunkt::post;
            pruefe (m.sicht().quellen.front().messpunkt == erwartet,
                    (std::string ("measurement_point_") + pos + "_reaches_main").c_str());
        }
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied q; q.descriptor = false;
        uebernehme (m, snapshot ({ q }), t0);
        const auto z = m.sicht().quellen.front();
        pruefe (! z.descriptorVorhanden && z.messpunkt == Model::Messpunkt::unbekannt,
                "unclassified_is_visible_not_error_or_role");
    }
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied q; q.betrieb = "offline";
        uebernehme (m, snapshot ({ q }), t0);
        const bool offline = m.sicht().quellen.front().betrieb == Model::Betrieb::offline;
        q.betrieb = "active";
        uebernehme (m, snapshot ({ q }), t0 + std::chrono::milliseconds (1));
        const bool wartet = m.sicht().quellen.front().messung == Model::Messung::missing;
        pruefe (offline && wartet, "offline_is_visible_and_requires_explicit_resume");
        q.betrieb = "suspended";
        uebernehme (m, snapshot ({ q }), t0 + std::chrono::milliseconds (2));
        const auto z = m.sicht().quellen.front();
        pruefe (z.betrieb == Model::Betrieb::suspended
                && z.lautheit != Model::Lautheit::noSignal
                && z.control == Model::Control::verbunden,
                "suspended_never_collapses_into_no_signal_or_disconnect");
    }
    {
        Model m;
        initialisiereFuer (m, pair, t0);
        p2InModell (m, pair, t0);
        m.tick (t0 + std::chrono::milliseconds (2500));
        const bool equality = m.sicht().quellen.front().control == Model::Control::verbunden;
        m.tick (t0 + std::chrono::milliseconds (2501));
        const auto stale = m.sicht().quellen.front();
        pruefe (equality && stale.control == Model::Control::stale,
                "control_stale_boundary_is_exclusive");
        p2InModell (m, pair, t0 + std::chrono::milliseconds (2600));
        pruefe (m.sicht().quellen.front().control == Model::Control::stale,
                "p2_does_not_revive_control_and_heartbeat_does_not_refresh_measurement");
    }
    {
        Model m;
        const auto leer = m.sicht();
        Model::Sicht f;
        f.quellen = { sichtZeile (hex (20), "A"), sichtZeile (hex (21), "B") };
        m.setzeFixtureFuerTest (f);
        const auto s = m.sicht();
        pruefe (leer.quellen.empty()
                && std::count_if (s.quellen.begin(), s.quellen.end(),
                                  [] (const auto& q) { return q.hauptziel; }) == 1,
                "zero_sources_has_no_target_and_nonempty_model_has_exactly_one_main_target");
    }
    {
        auto lauf = [] (int n)
        {
            std::vector<Mitglied> quellen;
            for (int i = 0; i < n; ++i)
            {
                Mitglied q;
                q.id = hex (20 + (unsigned) i);
                q.nonce = hex (200 + (unsigned) i);
                q.label = "Source " + juce::String (i);
                quellen.push_back (q);
            }
            std::vector<double> ms;
            for (int r = 0; r < 25; ++r)
            {
                Model m;
                m.beginneSubscription (hex (1), hex (2), hex (10));
                const auto a = Model::Uhr::now();
                uebernehme (m, snapshot (quellen), a);
                const auto sicht = m.sicht();
                const auto b = Model::Uhr::now();
                if ((int) sicht.quellen.size() != n) return 1.0e9;
                ms.push_back (std::chrono::duration<double, std::milli> (b - a).count());
            }
            std::sort (ms.begin(), ms.end());
            return ms[(ms.size() * 95) / 100];
        };
        const auto p16 = lauf (16), p32 = lauf (32);
        pruefe (p16 <= 300.0 && p32 <= 300.0,
                "visible_latency_16_and_32_sources",
                juce::String (p16, 2) + " / " + juce::String (p32, 2) + " ms p95");
    }

    std::cout << "SONDE-012 SourcesModel: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
}
