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

    /// Ein Versuch, wie der Sessionsnapshot ihn traegt (M-49, Befund R14).
    ///
    /// 🔑 SONDE-013 Nacharbeit 2: das Terminal bekam bis dahin gar keine
    /// `snapshot_ziele` — ein gerechnetes Resultat erreichte Gen NIE. Es
    /// kommt jetzt ueber den bestehenden Snapshot-Pfad an, und dies ist seine
    /// Modellform. KEINE Pixel: die Bedienfragen P-01 bis P-06 gehoeren dem
    /// User (Paragraph 4.2).
    struct Versuch
    {
        std::string experimentId;
        std::string ereignis;
        bool        offen = true;
        /// Leer heisst „der User hat (noch) nicht geurteilt", nie „enthaltung".
        std::string hoerurteil;
        /// Die AUFGEDECKTE Reihenfolge - sie kommt erst mit dem Terminal.
        std::string blindreihenfolge;
        /// Leer heisst „nicht gerechnet", nie „stark".
        std::string vergleichbarkeit;
        /// Eine der fuenf zulaessigen Aussagen aus M-46, oder leer.
        std::string urteil;
    };

    /// Ein PRE/POST-Paarurteil aus dem Sessionsnapshot (M-13, Befund R32).
    struct Paar
    {
        std::string pairId;
        std::string klasse;
        std::string kettenbefund;
        /// Leer heisst „das Paar traegt eine Aussage".
        std::string ausschluss;
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
        /// SONDE-013 Nacharbeit 2 (Befunde R14/R32): der Rueckweg der
        /// Experimente und Paarurteile. Leer heisst „diese Sitzung fuehrt
        /// keine", nie „alle abgeschlossen".
        std::vector<Versuch> experimente;
        std::vector<Paar>    paare;
        /// SONDE-013 Nacharbeit 2 (Befund R28): wie oft in dieser Sitzung
        /// bereits eingegangene Evidenz zurueckgenommen wurde, und warum.
        /// Leere Zeichenketten heissen "keine Ruecknahme", nie "unbekannt".
        std::uint64_t evidenzRuecknahmen = 0;
        std::string ruecknahmeGrund, ruecknahmeUmfang;
    };

    enum class SnapshotErgebnis { ignoriert, uebernommen, ungueltig };
    enum class RuecknahmeErgebnis { ignoriert, uebernommen, ungueltig };

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
    /// Nimmt eine `evidence_invalidate`-Nachricht des Brokers an (M-52/M-57).
    ///
    /// SONDE-013 Nacharbeit 2 (Befund R28): diesen Leser gab es NICHT. Der
    /// Broker legte eine Outbox-Schuld an, `v3Antwort` kannte nur Command-ACK
    /// und Sessionsnapshot, und ein aktiver Subscriber erhielt die Ruecknahme
    /// nie. Eine Zustellschuld ohne Leser ist ein Defekt, kein Zustand.
    RuecknahmeErgebnis uebernehmeEvidenzruecknahme (const std::string& json,
                                                    std::uint8_t schemaMinor,
                                                    juce::String& fehler);
    RuecknahmeErgebnis uebernehmeEvidenzruecknahme (const std::string& json,
                                                    juce::String& fehler)
    {
        return uebernehmeEvidenzruecknahme (
            json, nakama::ipc::kJsonSchemaMinor, fehler);
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
    /// SONDE-013 Nacharbeit 2 (Befunde R14/R32): der zuletzt empfangene Stand
    /// der Versuche und Paarurteile dieser Sitzung.
    std::vector<Versuch> experimente;
    std::vector<Paar>    paare;
    /// SONDE-013 Nacharbeit 2 (Befund R28): Zaehler und letzter Anlass der
    /// Evidenzruecknahme.
    std::uint64_t evidenzRuecknahmen = 0;
    std::string ruecknahmeGrund, ruecknahmeUmfang;

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
