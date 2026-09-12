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

/// Der Repowurzelordner - der Korpus ist die dritte Partei zwischen den
/// Sprachen, und dieses Bein liest DIESELBEN Dateien wie Rust und A5.
juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
        d = d.getParentDirectory();
    return d;
}

std::string fixture (const char* name)
{
    return wurzel()
        .getChildFile ("eq-copilot/fixtures/v3")
        .getChildFile (name)
        .loadFileAsString()
        .toStdString();
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

/// Ein vertragsgueltiger Befund einer Quelle (SONDE-014 Etappe D/I).
///
/// Alle Pflichtfelder aus `$defs/session_finding`; veraendert wird von Fall zu
/// Fall genau eines - Quelle und Zustand.
std::string befundJson (const std::string& id, const std::string& quelle,
                        const std::string& zustand)
{
    return R"({"finding_id":")" + id
         + R"(","claim_class":"zusammenhang","ursachenklasse":"quelle_resonanz",)"
           R"("target_metric":"band_pegel_db","candidate_source":")" + quelle
         + R"(","band_hz":{"von":96,"bis":100},)"
           R"("beobachtung":{"wert_db":-3.5,"gueltig":true},)"
           R"("rang":{"bandpassung":0.4,"koinzidenz":0.0,"uplift":0.2,)"
           R"("intent_relevanz":0.5,"wiederholbarkeit":0.8,"routingqualitaet":1.0},)"
           R"("confidence":{"class":"mittel","score":0.65},)"
           R"("evidence_ids":[")" + hex (0x1000)
         + R"("],"next_test":"manueller_versuch","zustand":")" + zustand
         + R"(","intent_revision":3,)"
           R"("likely_cause":"Die Quelle draengt im markierten Bereich.",)"
           R"("smallest_test":"Die Quelle kurz absenken und gegenhoeren.",)"
           R"("listen_for":"Ob der Master im markierten Bereich Luft bekommt."})";
}

std::string snapshot (const std::vector<Mitglied>& mitglieder,
                      const std::string& binding = hex (1),
                      const std::string& session = hex (2),
                      const std::string& broker = hex (8),
                      const std::string& fuehrend = hex (10),
                      bool bestaetigung = false,
                      int storeDegraded = 0,
                      const std::vector<std::string>& befunde = {})
{
    std::string liste;
    for (const auto& m : mitglieder)
    {
        if (! liste.empty()) liste += ',';
        liste += mitgliedJson (m, binding, session);
    }
    std::string befundliste;
    for (const auto& b : befunde)
    {
        if (! befundliste.empty()) befundliste += ',';
        befundliste += b;
    }
    return R"({"type":"session_snapshot","session_epoch":")" + session
         + R"(","broker_epoch":")" + broker + R"(","fuehrendes_main":")"
         + fuehrend + R"(","beitritt_bestaetigung_noetig":)"
         + (bestaetigung ? "true" : "false")
         + (storeDegraded == 0 ? "" : storeDegraded == 1
                                      ? ",\"store_degraded\":true"
                                      : ",\"store_degraded\":false")
         + R"(,"mitglieder":[)"
         + liste + "]"
         + (befundliste.empty() ? "" : R"(,"findings":[)" + befundliste + "]")
         + "}";
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

/// NAK-246 D6 (M-30): die Sitzungsfelder einer Sicht gegen die eines frischen
/// Modells, Feld fuer Feld. Das ist die EINE Stelle dieses Beins, die die
/// Sitzungsmenge nennt; ein spaeter ergaenztes Sitzungsfeld gehoert hierher
/// (Manifest docs/beweise/NAK-246.md Paragraph 5.7 Feinheit 4). Ein frisches
/// Modell fuehrt keine Versuche, Paare oder Befunde - Groessengleichheit ist
/// dort Inhaltsgleichheit. `abweichung` nennt jedes Feld, das abweicht.
bool sitzungsfelderWieFrisch (const Model::Sicht& s, const Model::Sicht& frisch,
                              juce::String& abweichung)
{
    juce::StringArray felder;
    if (s.experimente.size() != frisch.experimente.size())
        felder.add ("experimente " + juce::String ((int) s.experimente.size()));
    if (s.paare.size() != frisch.paare.size())
        felder.add ("paare " + juce::String ((int) s.paare.size()));
    if (s.befunde.size() != frisch.befunde.size())
        felder.add ("befunde " + juce::String ((int) s.befunde.size()));
    if (s.evidenzRuecknahmen != frisch.evidenzRuecknahmen)
        felder.add ("evidenzRuecknahmen " + juce::String ((juce::int64) s.evidenzRuecknahmen));
    if (s.ruecknahmeGrund != frisch.ruecknahmeGrund)
        felder.add ("ruecknahmeGrund " + juce::String (s.ruecknahmeGrund));
    if (s.ruecknahmeUmfang != frisch.ruecknahmeUmfang)
        felder.add ("ruecknahmeUmfang " + juce::String (s.ruecknahmeUmfang));
    const Model::Zeile frischeZeile;
    for (const auto& q : s.quellen)
        if (q.findingsOffen != frischeZeile.findingsOffen)
            felder.add ("findingsOffen " + juce::String (q.findingsOffen));
    abweichung = felder.isEmpty()
        ? "keine (" + juce::String ((int) s.quellen.size()) + " Zeile(n))"
        : felder.joinIntoString ("; ");
    return felder.isEmpty();
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
        // NAK-246 Abschluss Nacharbeit 1 Fortsetzung (R-A1 Punkt 4'): die
        // Publikation traegt die Reload-Generation, fuer die sie gilt. Dieses
        // Bein kennt keinen Reload - 0 ist der Startwert des Modells, die Zusage
        // dieser Zeilen bleibt unveraendert.
        //
        // NAK-283 Etappe 2 (F01): dazu die Folgenummer des Standes. Jeder Fall
        // dieses Beins baut ein FRISCHES Modell, dessen zuletzt uebernommene
        // Nummer 0 ist; 1 ist deshalb die erste gueltige Nummer, und die Zusage
        // dieser Zeilen bleibt ebenfalls unveraendert. Die ORDNUNG selbst misst
        // B14 (M-01 bis M-06, M-71, M-72), nicht dieses Bein.
        m.setzePersistenteMitglieder ({ { juce::String (hex (20)), "Piano" } }, 0, 1);
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
        m.setzePersistenteMitglieder ({ { juce::String (hex (20)), "Piano" } }, 0, 1);
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
        // NAK-134/E-V03: der neue Livenessstatus darf NICHT als
        // Sicherheitswarnung erscheinen. `Server not verified` behauptet, der
        // Name gehoere jemand Fremdem — bei einer bloss belegten Pipe ist das
        // schlicht falsch, und der User bekaeme eine Warnung ohne Handgriff.
        // Richtig ist `Broker unavailable` MIT Handgriff: der Broker ist
        // gerade nicht erreichbar, und der User kann etwas tun.
        Model m;
        nakama::ipc::ControlClient::Snapshot s;
        s.status = nakama::ipc::ControlClient::Status::getrennt;
        s.serverPruefstatus = nakama::ipc::ServerPruefStatus::belegtNichtErreicht;
        s.serverPrueffehler = nakama::ipc::ServerPruefFehler::pipeBelegt;
        m.setzeControlTransport (s);
        const auto belegt = m.sicht();
        // Gegenprobe in derselben Zeile: der SICHERHEITSstatus bleibt, wo er
        // war. Ohne sie waere der Test auch gruen, wenn beide Werte auf
        // `brokerUnavailable` faenden.
        Model m2;
        nakama::ipc::ControlClient::Snapshot s2;
        s2.status = nakama::ipc::ControlClient::Status::getrennt;
        s2.serverPruefstatus = nakama::ipc::ServerPruefStatus::belegtAberUnverifiziert;
        s2.serverPrueffehler = nakama::ipc::ServerPruefFehler::pipeOeffnen;
        m2.setzeControlTransport (s2);
        const auto unverifiziert = m2.sicht();
        pruefe (belegt.diagnose == Model::Diagnose::brokerUnavailable
                    && belegt.diagnoseHatHandgriff
                    && unverifiziert.diagnose == Model::Diagnose::serverUnverified
                    && ! unverifiziert.diagnoseHatHandgriff,
                "belegt_nicht_erreicht_zeigt_broker_unavailable_mit_handgriff");
    }
    // ═══════════════════════════════════════════════════════════════════
    // SONDE-014 M-84 - `findingsOffen` bekommt seine QUELLE
    // ═══════════════════════════════════════════════════════════════════
    //
    // Bis Etappe I war das Feld ein toter Zaehler: `setzeFindings` hatte im
    // ganzen Repo genau einen Aufrufer, und der war GENAU DIESER FALL. Die
    // Anzeige zeigte im Produkt immer 0 - das sinnlose tote Element, das
    // CLAUDE.md ausschliesst. Der Setter ist fort; die Zahl wird aus dem
    // Sitzungsschnitt ABGELEITET, und gemessen wird sie am Produktpfad.
    {
        Model m;
        m.beginneSubscription (hex (1), hex (2), hex (10));
        Mitglied a; a.id = hex (20); a.nonce = hex (200);
        Mitglied b; b.id = hex (21); b.nonce = hex (201);
        const bool ok = uebernehme (m, snapshot ({ a, b }, hex (1), hex (2), hex (8),
                                                 hex (10), false, 0,
                                                 { befundJson (hex (0x901), hex (20), "ready_to_send"),
                                                   befundJson (hex (0x902), hex (20), "more_data"),
                                                   befundJson (hex (0x903), hex (20), "stale"),
                                                   befundJson (hex (0x904), hex (21), "ready_to_send") }),
                                    t0);
        const auto s = m.sicht();
        const auto za = std::find_if (s.quellen.begin(), s.quellen.end(),
                                      [] (const auto& q) { return q.instanceId == hex (20); });
        const auto zb = std::find_if (s.quellen.begin(), s.quellen.end(),
                                      [] (const auto& q) { return q.instanceId == hex (21); });
        // Zwei offene je Quelle A - `ready_to_send` und `more_data` -, und der
        // `stale` zaehlt NICHT mit: er ist durch eine Intent-Aenderung
        // ueberholt und beschreibt keine offene Arbeit.
        pruefe (ok && za != s.quellen.end() && zb != s.quellen.end()
                && za->findingsOffen == 2 && zb->findingsOffen == 1
                && s.befunde.size() == 4,
                "findings_count_tracks_only_open_findings_of_its_source");

        // Ein Schnitt OHNE Befunde setzt die Zahl zurueck - ein
        // stehengebliebener Zaehler behauptete Arbeit, die es nicht gibt.
        uebernehme (m, snapshot ({ a, b }), t0 + std::chrono::milliseconds (1));
        const auto leer = m.sicht();
        pruefe (leer.befunde.empty()
                && std::all_of (leer.quellen.begin(), leer.quellen.end(),
                                [] (const auto& q) { return q.findingsOffen == 0; }),
                "findings_count_faellt_mit_seiner_quelle");

        // Und ein SITZUNGSWECHSEL raeumt beides gemeinsam ab.
        uebernehme (m, snapshot ({ a, b }, hex (1), hex (2), hex (8), hex (10),
                                 false, 0, { befundJson (hex (0x905), hex (20), "ready_to_send") }),
                    t0 + std::chrono::milliseconds (2));
        pruefe (m.sicht().quellen.front().findingsOffen >= 0, "Zwischenstand steht");
        m.beginneSubscription (hex (1), hex (3), hex (10));
        const auto neueSitzung = m.sicht();
        pruefe (std::all_of (neueSitzung.quellen.begin(), neueSitzung.quellen.end(),
                             [] (const auto& q) { return q.findingsOffen == 0; }),
                "findings_count_ueberlebt_keinen_sitzungswechsel");
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
        m.setzePersistenteMitglieder ({ { juce::String (q.id), "Stored" } }, 0, 1);
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

    // ── SONDE-013 Nacharbeit 2 · der Rueckweg kommt WIRKLICH an ────────
    //
    // Die Klassenregel von V3 lautet: Nachricht rein -> Outbox-Eintrag ->
    // Push -> im Plugin-Modell angekommen. Die ersten drei Glieder misst
    // A4-verdrahtung im Broker. Das VIERTE steht hier, und es misst gegen
    // dasselbe Korpusfixture, das der Broker erzeugt - nicht gegen einen im
    // Test erfundenen Text.
    {
        Model m;
        // Die Adressen des Korpusfixtures, nicht die des Tests.
        m.beginneSubscription ("11111111111111111111111111111111",
                               "22222222222222222222222222222222",
                               "33333333333333333333333333333333");
        const auto payload = fixture ("gueltig/session-snapshot-mit-experimenten-und-paaren.json");
        juce::String grund;

        // Ein Leser der Fassung 1 kennt `experimente` und `paare` nicht und
        // lehnt sie ab, statt sie still zu verwerfen. Genau so ist die
        // Fassungswahl gebaut (Paragraph 7.1, E-04).
        const bool fassung1Lehnt = m.uebernehmeSessionSnapshot (payload, 1, t0, grund)
                                    == Model::SnapshotErgebnis::ungueltig;

        const bool angenommen = m.uebernehmeSessionSnapshot (payload, 2, t0, grund)
                                 == Model::SnapshotErgebnis::uebernommen;
        const auto s = m.sicht();
        const bool versuchDa = s.experimente.size() == 1
            && s.experimente[0].experimentId == "abababababababababababababababab"
            && s.experimente[0].ereignis == "ergebnis"
            && ! s.experimente[0].offen
            && s.experimente[0].hoerurteil == "kandidat"
            && s.experimente[0].blindreihenfolge == "kandidat_zuerst"
            && s.experimente[0].vergleichbarkeit == "stark"
            && s.experimente[0].urteil == "ziel_verbessert_guardrails_stabil";
        const bool paareDa = s.paare.size() == 2
            && s.paare[0].pairId == "paar-bus-a"
            && s.paare[0].ausschluss.empty()
            && s.paare[1].pairId == "paar-bus-b"
            && s.paare[1].ausschluss == "haelfte_fehlt";
        pruefe (fassung1Lehnt && angenommen && versuchDa && paareDa,
                "R14_R32_experiment_and_pair_verdict_arrive_in_the_model",
                grund);

        // Der Modelleser prueft die geschlossenen Wortmengen nicht selbst -
        // das tut die Vertragsengine (A5/B3c) am Korpus, in beiden Sprachen.
        // Was er prueft, ist die FELDMENGE: ein zusaetzliches Wort faellt.
        auto verdorben = juce::JSON::parse (juce::String (payload));
        if (auto* o = verdorben.getDynamicObject())
            if (auto* liste = o->getProperty ("experimente").getArray())
                if (auto* e = liste->getFirst().getDynamicObject())
                    e->setProperty ("bewertung", "gut");
        pruefe (m.uebernehmeSessionSnapshot (
                    juce::JSON::toString (verdorben, true).toStdString(), 2, t0, grund)
                    == Model::SnapshotErgebnis::ungueltig,
                "unknown_experiment_field_falls_instead_of_travelling_along");
    }
    {
        // R28 - die Evidenzruecknahme hat einen LESER und eine WIRKUNG.
        Model m;
        initialisiereFuer (m, pair, t0);
        const bool frisch = p2InModell (m, pair, t0)
                         && m.sicht().quellen.front().messung == Model::Messung::fresh;
        juce::String grund;
        const auto ruecknahme = fixture ("gueltig/invalidate-ganze-sitzung.json");
        const bool angenommen = m.uebernehmeEvidenzruecknahme (ruecknahme, 2, grund)
                                 == Model::RuecknahmeErgebnis::uebernommen;
        const auto s = m.sicht();
        pruefe (frisch && angenommen
                    && s.quellen.front().messung == Model::Messung::invalid
                    && s.evidenzRuecknahmen == 1
                    && s.ruecknahmeGrund == "routing_unbekannt"
                    && s.ruecknahmeUmfang == "ganze_sitzung",
                "R28_evidence_invalidate_has_a_reader_and_an_effect", grund);

        // Die zwei Gruende der Fassung 2 gehoeren der Fassung 2. Ein Leser
        // der Fassung 1 lehnt sie ab, statt sie still auf `intervention`
        // abzubilden - das waere eine erfundene Begruendung fuer eine echte
        // Ruecknahme.
        const auto material = fixture ("gueltig/invalidate-material-wechsel.json");
        const bool fassung1Lehnt = m.uebernehmeEvidenzruecknahme (material, 1, grund)
                                    == Model::RuecknahmeErgebnis::ungueltig;
        const bool fassung2Nimmt = m.uebernehmeEvidenzruecknahme (material, 2, grund)
                                    == Model::RuecknahmeErgebnis::uebernommen;
        pruefe (fassung1Lehnt && fassung2Nimmt
                    && m.sicht().evidenzRuecknahmen == 2
                    && m.sicht().ruecknahmeGrund == "material_wechsel",
                "invalidate_reason_version_is_chosen_by_the_wire_envelope", grund);

        // Der Discriminator ist kein Etikett: `ganze_sitzung` MIT Bereich und
        // ein verdrehter Bereich fallen beide.
        auto mitBereich = juce::JSON::parse (juce::String (ruecknahme));
        if (auto* o = mitBereich.getDynamicObject())
            if (auto* u = o->getProperty ("umfang").getDynamicObject())
                u->setProperty ("sample_start", 0);
        const bool sitzungMitBereichFaellt = m.uebernehmeEvidenzruecknahme (
            juce::JSON::toString (mitBereich, true).toStdString(), 2, grund)
                == Model::RuecknahmeErgebnis::ungueltig;

        auto verdreht = juce::JSON::parse (juce::String (ruecknahme));
        if (auto* o = verdreht.getDynamicObject())
        {
            auto* u = new juce::DynamicObject();
            u->setProperty ("art", "sample_range");
            u->setProperty ("sample_start", 900);
            u->setProperty ("sample_end", 100);
            o->setProperty ("umfang", juce::var (u));
        }
        const bool verdrehterBereichFaellt = m.uebernehmeEvidenzruecknahme (
            juce::JSON::toString (verdreht, true).toStdString(), 2, grund)
                == Model::RuecknahmeErgebnis::ungueltig;
        pruefe (sitzungMitBereichFaellt && verdrehterBereichFaellt
                    && m.sicht().evidenzRuecknahmen == 2,
                "invalidate_scope_discriminator_is_not_a_label", grund);
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-214 R3/R8 · Ruecknahme und Reconnect (V-24, V-25, V-27)
    // ═══════════════════════════════════════════════════════════════════
    //
    // Der G5-Befund A5: eine Evidenzruecknahme OHNE Folge-Snapshot liess
    // einen `ready_to_send`-Befund handelbar stehen. Gen fuehrt keine
    // Evidenz-IDs und kann nicht sagen, WELCHER Befund die zurueckgenommene
    // Evidenz trug - fail-closed heisst deshalb: JEDER wird `stale`, und die
    // abgeleitete Zahl faellt mit (R8, M-84).
    {
        // Die Buehne traegt eine ECHTE Messung (P2), damit die Messachse in
        // dieser Zeile wirklich etwas zu verlieren hat - sonst stuende sie
        // auf `missing`, und die Zusage „die fuenf Achsen bleiben getrennt"
        // waere an nichts gemessen.
        Model m;
        initialisiereFuer (m, pair, t0);
        const bool frisch = p2InModell (m, pair, t0)
                         && m.sicht().quellen.front().messung == Model::Messung::fresh;
        Mitglied a; a.id = pair.frame.instanceId; a.nonce = pair.frame.runtimeNonce;
        const bool angekommen =
            uebernehme (m, snapshot ({ a }, pair.frame.projectBindingId,
                                     pair.frame.sessionEpoch, hex (8), hex (10), false, 0,
                                     { befundJson (hex (0x911), pair.frame.instanceId,
                                                   "ready_to_send"),
                                       befundJson (hex (0x912), pair.frame.instanceId,
                                                   "more_data") }),
                        t0 + std::chrono::milliseconds (1));
        const auto vorher = m.sicht();
        pruefe (frisch && angekommen && vorher.befunde.size() == 2
                    && vorher.befunde[0].darfDraft()
                    && vorher.quellen.front().findingsOffen == 2,
                "NAK214_V24_vorbedingung_zwei_offene_befunde_einer_handelbar");

        // V-24 - die Ruecknahme, OHNE Folge-Snapshot.
        juce::String grund;
        const auto ruecknahme = fixture ("gueltig/invalidate-ganze-sitzung.json");
        const bool uebernommen = m.uebernehmeEvidenzruecknahme (ruecknahme, 2, grund)
                                  == Model::RuecknahmeErgebnis::uebernommen;
        const auto nachher = m.sicht();
        const bool alleStale =
            std::all_of (nachher.befunde.begin(), nachher.befunde.end(),
                         [] (const auto& b) {
                             return b.zustand == "stale" && ! b.darfDraft()
                                 && ! b.darfAudition();
                         });
        pruefe (uebernommen && nachher.befunde.size() == 2 && alleStale,
                "NAK214_V24_ruecknahme_ohne_folgesnapshot_laesst_keinen_ready_befund", grund);
        pruefe (std::all_of (nachher.quellen.begin(), nachher.quellen.end(),
                             [] (const auto& q) { return q.findingsOffen == 0; }),
                "NAK214_V24_und_der_zaehler_faellt_mit_seiner_quelle");
        // Die Messachse bleibt daneben, wie bisher - die fuenf Achsen sind
        // getrennt, der Befundzustand ist die sechste, eigene Wirkung.
        pruefe (nachher.quellen.front().messung == Model::Messung::invalid
                    && nachher.evidenzRuecknahmen == 1,
                "NAK214_V24_die_messachse_bleibt_ihre_eigene_wirkung");

        // V-25 (Regressionswache) - der Reconnect raeumt bei JEDER Epoche.
        // `beginneSubscription` leert `befunde` unbedingt; einen
        // Epochenvergleich gibt es weder davor noch darin, und dieses Ticket
        // baut ihm keinen. Der Teilfall mit NEUER Epoche steht als
        // `findings_count_ueberlebt_keinen_sitzungswechsel` schon oben; hier
        // faellt der Teilfall mit DERSELBEN Epoche.
        m.beginneSubscription (pair.frame.projectBindingId, pair.frame.sessionEpoch,
                               hex (10));
        const auto nachReconnect = m.sicht();
        pruefe (nachReconnect.befunde.empty() && nachReconnect.experimente.empty()
                    && nachReconnect.paare.empty()
                    && std::all_of (nachReconnect.quellen.begin(),
                                    nachReconnect.quellen.end(),
                                    [] (const auto& q) { return q.findingsOffen == 0; }),
                "NAK214_V25_reconnect_raeumt_die_befunde_bei_gleicher_epoche");

        // V-27 (Regressionswache) - und danach, OHNE Snapshot, bleibt die
        // Liste leer: nichts ist handelbar, und nichts wird behauptet.
        pruefe (m.sicht().befunde.empty(),
                "NAK214_V27_reconnect_ohne_snapshot_laesst_nichts_handelbares");
    }


    // ═══════════════════════════════════════════════════════════════════
    // NAK-181 R6 · geschlossene Mengen im Gen-Empfaenger (V09, V10)
    // ═══════════════════════════════════════════════════════════════════
    //
    // Der Leser hielt fuer `experimente` und `paare` jeden String. „objektiv
    // besser" stand damit genauso im Modell wie eine der fuenf zulaessigen
    // Aussagen aus M-46 — und `SourcesModel.h` behauptete das Gegenteil. Die
    // Faelle bauen auf dem Korpusfixture auf und veraendern GENAU EIN Feld.
    {
        const auto basis = fixture ("gueltig/session-snapshot-mit-experimenten-und-paaren.json");
        auto mitFeld = [&basis] (const char* wo, int index, const char* feld,
                                 const juce::var& wert) -> juce::String
        {
            auto baum = juce::JSON::parse (basis);
            auto* liste = baum.getProperty (wo, {}).getArray();
            if (liste == nullptr || index >= liste->size())
                return {};
            auto* o = (*liste)[index].getDynamicObject();
            if (o == nullptr)
                return {};
            // `juce::var()` wird von `JSON::toString` als `null` geschrieben —
            // genau der Fall, den das Schema fuer `pair_id` erlaubt.
            o->setProperty (feld, wert);
            return juce::JSON::toString (baum, true);
        };
        auto ohneFeld = [&basis] (const char* wo, int index, const char* feld) -> juce::String
        {
            auto baum = juce::JSON::parse (basis);
            auto* liste = baum.getProperty (wo, {}).getArray();
            if (liste == nullptr || index >= liste->size())
                return {};
            auto* o = (*liste)[index].getDynamicObject();
            if (o == nullptr)
                return {};
            o->removeProperty (feld);
            return juce::JSON::toString (baum, true);
        };
        auto ergebnis = [&] (const juce::String& json) -> Model::SnapshotErgebnis
        {
            Model m;
            m.beginneSubscription ("11111111111111111111111111111111",
                                   "22222222222222222222222222222222",
                                   "33333333333333333333333333333333");
            juce::String grund;
            return m.uebernehmeSessionSnapshot (json.toStdString(), 2, t0, grund);
        };

        // N-26 — je Feld ein Wert AUSSERHALB der Menge.
        struct Aussen { const char* wo; const char* feld; const char* wert; };
        const Aussen ausserhalb[] = {
            { "experimente", "ereignis",         "begonnnen" },
            { "experimente", "hoerurteil",       "vielleicht" },
            { "experimente", "blindreihenfolge", "zufaellig" },
            { "experimente", "vergleichbarkeit", "mittel" },
            { "experimente", "urteil",           "objektiv besser" },
            { "paare",       "klasse",           "fast_aligned" },
            { "paare",       "kettenbefund",     "eher_stationaer" },
            { "paare",       "ausschluss",       "keine_lust" },
        };
        for (const auto& a : ausserhalb)
        {
            const auto json = mitFeld (a.wo, a.wo[0] == 'p' ? 1 : 0, a.feld, juce::var (a.wert));
            pruefe (! json.isEmpty()
                    && ergebnis (json) == Model::SnapshotErgebnis::ungueltig,
                    (juce::String ("N-26: ") + a.feld
                        + " ausserhalb der Menge ist ungueltig").toRawUTF8(),
                    a.wert);
        }

        // N-27 — je Feld ein Wert INNERHALB der Menge wird uebernommen.
        const Aussen innerhalb[] = {
            { "experimente", "ereignis",         "verdraengt" },
            { "experimente", "hoerurteil",       "enthaltung" },
            { "experimente", "blindreihenfolge", "baseline_zuerst" },
            { "experimente", "vergleichbarkeit", "unvergleichbar" },
            { "experimente", "urteil",           "vergleich_nicht_gueltig" },
            { "paare",       "klasse",           "audio_aligned" },
            { "paare",       "kettenbefund",     "latenz_wechselt_markiert" },
            { "paare",       "ausschluss",       "sprung_im_fenster" },
        };
        for (const auto& a : innerhalb)
        {
            const auto json = mitFeld (a.wo, a.wo[0] == 'p' ? 1 : 0, a.feld, juce::var (a.wert));
            pruefe (! json.isEmpty()
                    && ergebnis (json) == Model::SnapshotErgebnis::uebernommen,
                    (juce::String ("N-27: ") + a.feld
                        + " innerhalb der Menge wird uebernommen").toRawUTF8(),
                    a.wert);
        }

        // N-28 — pair_id: null, 64, 65, leer, jenseits der BMP.
        {
            juce::String vierundsechzig;
            for (int i = 0; i < 64; ++i) vierundsechzig += "a";
            const auto fuenfundsechzig = vierundsechzig + "a";
            // Ein Zeichen jenseits der BMP zaehlt als EIN Codepoint.
            juce::String jenseits (juce::CharPointer_UTF8 ("\xF0\x9F\x8E\xB5"));  // U+1F3B5
            for (int i = 0; i < 63; ++i) jenseits += juce::String ("a");

            pruefe (ergebnis (mitFeld ("paare", 0, "pair_id", juce::var()))
                        == Model::SnapshotErgebnis::uebernommen,
                    "N-28: pair_id null ist gueltig (Schema: string|null)");
            pruefe (ergebnis (mitFeld ("paare", 0, "pair_id", juce::var (vierundsechzig)))
                        == Model::SnapshotErgebnis::uebernommen,
                    "N-28: 64 Codepoints sind gueltig");
            pruefe (ergebnis (mitFeld ("paare", 0, "pair_id", juce::var (fuenfundsechzig)))
                        == Model::SnapshotErgebnis::ungueltig,
                    "N-28: 65 nicht");
            pruefe (ergebnis (mitFeld ("paare", 0, "pair_id", juce::var (juce::String())))
                        == Model::SnapshotErgebnis::ungueltig,
                    "N-28: der leere String auch nicht - er ist ausdruecklich keine "
                    "zweite Schreibweise fuer kein Paar");
            pruefe (jenseits.length() == 64
                    && ergebnis (mitFeld ("paare", 0, "pair_id", juce::var (jenseits)))
                        == Model::SnapshotErgebnis::uebernommen,
                    "N-28: gezaehlt werden CODEPOINTS - ein Vier-Byte-Zeichen zaehlt als eins",
                    juce::String (jenseits.length()));
        }

        // N-29 — ausschluss als Nicht-String faellt nicht mehr still weg.
        {
            pruefe (ergebnis (mitFeld ("paare", 1, "ausschluss", juce::var (7)))
                        == Model::SnapshotErgebnis::ungueltig,
                    "N-29: ausschluss als Zahl ist ungueltig - der einzige "
                    "fail-open-Zweig der Funktion ist zu");
            pruefe (ergebnis (mitFeld ("paare", 1, "ausschluss", juce::var (true)))
                        == Model::SnapshotErgebnis::ungueltig,
                    "N-29: als Bool ebenso");
            pruefe (ergebnis (ohneFeld ("paare", 1, "ausschluss"))
                        == Model::SnapshotErgebnis::uebernommen,
                    "N-29: FEHLT es, bleibt der Snapshot gueltig - leer heisst: "
                    "das Paar traegt eine Aussage");
            // 🔑 NAK-181 Nacharbeit 1 (EP-01/NR-01): ein VORHANDENES `null`
            // ist ungueltig — nicht „wie fehlend".
            //
            // Bis zu dieser Runde stand hier die umgekehrte Erwartung, und
            // der Leser hatte eine `isVoid()`-Ausnahme. Das Schema kennt fuer
            // `ausschluss` nur die zehn Enumworte (`eq-ipc-v3.schema.json`
            // `$defs/session_paar`), und sein Kommentar sagt woertlich
            // „Abwesenheit heisst: es traegt eine". Aus einem vertragswidrigen
            // Feld wurde damit die Aussage „kein Ausschluss".
            pruefe (ergebnis (mitFeld ("paare", 1, "ausschluss", juce::var()))
                        == Model::SnapshotErgebnis::ungueltig,
                    "N-29: ein VORHANDENES `null` ist ungueltig - nur die "
                    "ABWESENHEIT heisst: das Paar traegt eine Aussage");
            // Dieselbe Regel in den vier optionalen Enums der Experimente:
            // sie gehen durch DENSELBEN Helfer, und keines von ihnen fuehrt
            // `null` im Schema.
            for (const char* feld : { "hoerurteil", "blindreihenfolge",
                                      "vergleichbarkeit", "urteil" })
                pruefe (ergebnis (mitFeld ("experimente", 0, feld, juce::var()))
                            == Model::SnapshotErgebnis::ungueltig,
                        (juce::String ("N-29: ") + feld
                            + " als `null` ist ebenso ungueltig").toRawUTF8());
            // Die Gegenprobe, die den Riegel von einem Pauschalverbot trennt:
            // `pair_id` fuehrt `["string","null"]` und bleibt gueltig — sie
            // laeuft an ihrer eigenen Stelle, nicht ueber diesen Helfer.
            pruefe (ergebnis (mitFeld ("paare", 0, "pair_id", juce::var()))
                        == Model::SnapshotErgebnis::uebernommen,
                    "N-29/N-28: und `pair_id: null` bleibt gueltig - der Riegel "
                    "trifft die geschlossenen Enums, nicht jedes `null`");
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-246 D6 · M-30 reload_und_subscription_leeren_dieselbe_sitzungsmenge
    // ═══════════════════════════════════════════════════════════════════
    //
    // Regel R-D6: `projektReload` und `beginneSubscription` teilen sich EINE
    // Reset-Funktion fuer den Sitzungszustand. Bis dahin leerte nur
    // `beginneSubscription` Experimente, Paare, Befunde, `findingsOffen` und
    // die Ruecknahme; `projektReload` liess sie stehen (Auditbefund D6). Der
    // Fall misst die MENGE: zwei gleich vorbelegte Modelle, das eine laedt ein
    // Projekt, das andere beginnt eine Subscription - danach traegt jedes in
    // jedem Sitzungsfeld denselben Wert wie ein frisches Modell (Manifest
    // docs/beweise/NAK-246.md Paragraph 3.7 M-30, 5.7 Feinheit 4).
    // Rotlauf gegen den Basis-SHA: docs/beweise/roh/NAK-246-rot-M-30.txt.
    {
        // Die Adressen der Korpusfixtures; das Mitglied `3333...` traegt den
        // Befund und damit `findingsOffen`.
        const std::string binding (32, '1'), session (32, '2'), mitglied (32, '3');
        // Deklarierter Mutant des Korpusfixtures mit genau einer Abweichung:
        // `findings` ist die Befundliste aus `session-snapshot-mit-findings.json`,
        // deren Befund auf das Mitglied dieses Snapshots zeigt.
        const std::string vollerSnapshot = [&]
        {
            auto baum = juce::JSON::parse (juce::String (
                fixture ("gueltig/session-snapshot-mit-experimenten-und-paaren.json")));
            auto befunde = juce::JSON::parse (juce::String (
                fixture ("gueltig/session-snapshot-mit-findings.json"))).getProperty ("findings", {});
            if (auto* liste = befunde.getArray())
                for (auto& b : *liste)
                    if (auto* o = b.getDynamicObject())
                        o->setProperty ("candidate_source", juce::String (mitglied));
            if (auto* o = baum.getDynamicObject())
                o->setProperty ("findings", befunde);
            return juce::JSON::toString (baum, true).toStdString();
        }();
        const auto ruecknahme = fixture ("gueltig/invalidate-ganze-sitzung.json");
        // Alle sieben Groessen ueber die Produktleser: Snapshot, Ruecknahme (sie
        // macht jeden Befund `stale` und `findingsOffen` damit 0), derselbe
        // Snapshot (der Befund kommt zurueck, die Ruecknahme bleibt gezaehlt).
        auto vorbelegen = [&] (Model& m, juce::String& beleg) -> bool
        {
            m.beginneSubscription (binding, session, mitglied);
            juce::String grund;
            const bool a = m.uebernehmeSessionSnapshot (vollerSnapshot, t0, grund)
                           == Model::SnapshotErgebnis::uebernommen;
            const bool b = m.uebernehmeEvidenzruecknahme (ruecknahme, grund)
                           == Model::RuecknahmeErgebnis::uebernommen;
            const bool c = m.uebernehmeSessionSnapshot (vollerSnapshot, t0, grund)
                           == Model::SnapshotErgebnis::uebernommen;
            const auto s = m.sicht();
            int offen = -1;
            for (const auto& q : s.quellen)
                if (q.instanceId == mitglied)
                    offen = q.findingsOffen;
            beleg = juce::String ((int) s.experimente.size()) + " Versuch(e), "
                  + juce::String ((int) s.paare.size()) + " Paar(e), "
                  + juce::String ((int) s.befunde.size()) + " Befund(e), findingsOffen "
                  + juce::String (offen) + ", Ruecknahmen "
                  + juce::String ((juce::int64) s.evidenzRuecknahmen) + " ("
                  + juce::String (s.ruecknahmeGrund) + ", "
                  + juce::String (s.ruecknahmeUmfang) + ")" + grund;
            return a && b && c && s.experimente.size() == 1 && s.paare.size() == 2
                && s.befunde.size() == 1 && offen == 1 && s.evidenzRuecknahmen == 1
                && s.ruecknahmeGrund == "routing_unbekannt"
                && s.ruecknahmeUmfang == "ganze_sitzung";
        };
        Model ueberReload, ueberSubscription;
        juce::String belegReload, belegSubscription;
        const bool belegtReload = vorbelegen (ueberReload, belegReload);
        const bool belegtSubscription = vorbelegen (ueberSubscription, belegSubscription);
        pruefe (belegtReload && belegtSubscription,
                "NAK246_M30_vorbedingung_beide_modelle_tragen_alle_sieben_sitzungsgroessen",
                belegReload + " / " + belegSubscription);

        // Die zwei Wege. Der Reload behaelt das Mitglied als persistentes -
        // so hat auch dieser Weg eine Zeile, deren `findingsOffen` zaehlt.
        ueberReload.projektReload ({ { juce::String (mitglied), "Klavier-Bus" } }, 1);
        ueberSubscription.beginneSubscription (binding, session, mitglied);

        const Model frisch;
        const auto referenz = frisch.sicht();
        juce::String restReload, restSubscription;
        const bool reloadWieFrisch =
            sitzungsfelderWieFrisch (ueberReload.sicht(), referenz, restReload);
        const bool subscriptionWieFrisch =
            sitzungsfelderWieFrisch (ueberSubscription.sicht(), referenz, restSubscription);
        pruefe (reloadWieFrisch && subscriptionWieFrisch,
                "NAK246_M30_reload_und_subscription_leeren_dieselbe_sitzungsmenge",
                "projektReload: " + restReload + " / beginneSubscription: " + restSubscription);

        // M-31 (Wache) am Modell selbst: was die zwei Wege NICHT teilen. Der
        // Reload meldet `brokerUnavailable` mit Handgriff, der Subscribe
        // `authenticating` ohne - die Diagnose gehoert nicht zur Sitzungsmenge
        // (Paragraph 5.7 Feinheit 2). Am Prozessor (B14, M-29/M-31) setzt der
        // Reconnect nach dem Laden dieselbe Diagnose ueber `controlEnde` noch
        // einmal; die Zeile dort haengt deshalb nicht an `projektReload` allein.
        const auto nachReload = ueberReload.sicht();
        const auto nachSubscription = ueberSubscription.sicht();
        pruefe (nachReload.diagnose == Model::Diagnose::brokerUnavailable
                    && nachReload.diagnoseHatHandgriff
                    && nachSubscription.diagnose == Model::Diagnose::authenticating
                    && ! nachSubscription.diagnoseHatHandgriff,
                "NAK246_M31_reload_behaelt_broker_unavailable_mit_handgriff_subscription_meldet_authenticating",
                juce::String (wort (nachReload.diagnose)) + (nachReload.diagnoseHatHandgriff ? " +Handgriff" : "")
                    + " / " + wort (nachSubscription.diagnose)
                    + (nachSubscription.diagnoseHatHandgriff ? " +Handgriff" : ""));
    }
    std::cout << "SONDE-012 SourcesModel: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
}
