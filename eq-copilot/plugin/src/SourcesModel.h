#pragma once

/*  SONDE-012 — designneutrales Main-Quellenmodell.

    Die fuenf Achsen bleiben getrennt: persistente Mitgliedschaft,
    Control/Liveness, Messung, Namensherkunft und Capability-Evidenz. Der
    Broker liefert absolute fluechtige Snapshots und P2; MainProject liefert
    ausschliesslich bestaetigte Identitaet plus User-Label. Nicht fuer den
    Audiothread. */

#include "ControlClient.h"
#include "NakamaState.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace eqcop
{

class SourcesModel
{
public:
    using Uhr = std::chrono::steady_clock;
    using Zeitpunkt = Uhr::time_point;

    enum class Mitgliedschaft { unclassified, bestaetigt };
    enum class Control { nieVerbunden, verbunden, stale, getrennt, fehler };
    enum class Messung { missing, fresh, partial, stale, invalid };
    enum class Betrieb { unbekannt, active, suspended, offline };
    enum class Lautheit { missing, measuring, noSignal, gueltig, invalid };
    enum class Namensherkunft { host, userLabel, platzhalter };
    enum class CapabilityEvidenz { nochNichtGemessen, gemessenNein, gemessenJa };
    enum class Messpunkt { unbekannt, insert, pre, post };
    enum class Diagnose
    {
        keine,
        brokerUnavailable,
        serverUnverified,
        authenticating,
        confirmationRequired,
        incompatible,
        storeDegraded
    };

    struct Zeile
    {
        std::string instanceId, runtimeNonce, pluginKind;
        Mitgliedschaft mitgliedschaft = Mitgliedschaft::unclassified;
        Control control = Control::nieVerbunden;
        Messung messung = Messung::missing;
        Betrieb betrieb = Betrieb::unbekannt;
        Lautheit lautheit = Lautheit::missing;
        Namensherkunft namensherkunft = Namensherkunft::platzhalter;
        CapabilityEvidenz capabilityEvidenz = CapabilityEvidenz::nochNichtGemessen;
        Messpunkt messpunkt = Messpunkt::unbekannt;
        juce::String sichtbarerName, userLabel, hostBusName;
        bool hostMixerIndexVorhanden = false;
        std::uint64_t hostMixerIndex = 0;
        bool descriptorVorhanden = false;
        bool hauptziel = false;
        bool p2RejectAktiv = false;
        juce::String p2RejectGrund;
        std::uint64_t p2RejectZaehler = 0;
        std::uint64_t controlAlterMs = 0, messAlterMs = 0;
        double fensterDauerMs = 0.0;
        bool lufsPaarVorhanden = false;
        float lufsI = 0.0f, lufsIUnsicherheitLu = 0.0f;
        int findingsOffen = 0;
    };

    struct Sicht
    {
        std::uint64_t revision = 0;
        bool subscriptionAktiv = false;
        Diagnose diagnose = Diagnose::keine;
        bool diagnoseHatHandgriff = false;
        bool mainDarfSchreiben = false;
        std::string fuehrendesMain;
        std::vector<Zeile> quellen;
    };

    enum class SnapshotErgebnis { ignoriert, uebernommen, ungueltig };

    void setzePersistenteMitglieder (
        const std::vector<nakama::state::MainProjectMitglied>& mitglieder);
    void projektReload (
        const std::vector<nakama::state::MainProjectMitglied>& mitglieder);
    void beginneSubscription (std::string projectBindingId,
                              std::string sessionEpoch,
                              std::string eigeneMainInstanceId);
    void controlEnde();
    void setzeControlTransport (
        const nakama::ipc::ControlClient::Snapshot& transport);

    SnapshotErgebnis uebernehmeSessionSnapshot (const std::string& json,
                                                 std::uint8_t schemaMinor,
                                                 Zeitpunkt empfangen,
                                                 juce::String& fehler);
    SnapshotErgebnis uebernehmeSessionSnapshot (const std::string& json,
                                                 Zeitpunkt empfangen,
                                                 juce::String& fehler)
    {
        return uebernehmeSessionSnapshot (
            json, nakama::ipc::kJsonSchemaMinor, empfangen, fehler);
    }
    bool uebernehmeP2 (const std::uint8_t* daten, std::size_t laenge,
                       std::uint8_t schemaMinor, Zeitpunkt empfangen,
                       juce::String& fehler);
    void tick (Zeitpunkt jetzt);

    Sicht sicht() const;
    bool waehleHauptziel (const std::string& instanceId);
    std::string aktuellesHauptziel() const;
    bool istAktuellesHauptziel (const std::string& instanceId) const;
    void setzeFindings (const std::string& instanceId, int offen);
    void setzeCapabilityEvidenz (const std::string& instanceId,
                                 CapabilityEvidenz evidenz);
    void setzeDiagnoseFuerSichtbeweis (Diagnose diagnose, bool echterHandgriff);

#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    void setzeFixtureFuerTest (Sicht fixture);
#endif

    static constexpr std::uint64_t controlStaleNachMs = 2500;
    static constexpr std::uint64_t messungMindestensStaleNachMs = 1000;
    static std::uint64_t messStaleFristMs (double fensterDauerMs) noexcept;

private:
    struct Eintrag
    {
        Zeile zeile;
        bool fluechtigImSnapshot = false;
        std::uint64_t controlBasisAlterMs = 0;
        Zeitpunkt controlEmpfangen {};
        bool hatMessZeit = false;
        Zeitpunkt messEmpfangen {};
        bool resumeBrauchtFrame = false;
        std::uint64_t letzterRejectZaehler = 0;
        juce::String descriptorLabel;
    };

    void revidiere() noexcept { ++revision; }
    void stelleZielSicher();
    void aktualisiereName (Eintrag&);
    void aktualisiereAbgeleiteteZustaende (Eintrag&, Zeitpunkt);
    static juce::String platzhalter (const std::string& instanceId);
    static std::string sortierSchluessel (const juce::String&);

    mutable std::mutex mutex;
    std::map<std::string, Eintrag> eintraege;
    std::map<std::string, juce::String> persistenteMitglieder;
    std::string erwarteteBindung, erwarteteSession, eigeneMainId;
    std::string brokerEpoch, fuehrendesMain, hauptziel;
    bool subscriptionAktiv = false;
    Diagnose diagnose = Diagnose::brokerUnavailable;
    bool diagnoseHatHandgriff = true;
    std::uint64_t revision = 0;
    Zeitpunkt sichtZeit {};
};

const char* wort (SourcesModel::Control);
const char* wort (SourcesModel::Messung);
const char* wort (SourcesModel::Betrieb);
const char* wort (SourcesModel::Lautheit);
const char* wort (SourcesModel::Namensherkunft);
const char* wort (SourcesModel::Messpunkt);
const char* wort (SourcesModel::Diagnose);

} // namespace eqcop
