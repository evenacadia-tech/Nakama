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
#include <utility>
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
        ///
        /// 🔑 NAK-181 R6: seit diesem Ticket ist das WAHR. Der Leser haelt die
        /// geschlossene Menge des Schemas durch; ein fremder Wert macht den
        /// ganzen Snapshot ungueltig, statt ihn zu uebernehmen.
        std::string urteil;
    };

    /// Ein PRE/POST-Paarurteil aus dem Sessionsnapshot (M-13, Befund R32).
    struct Paar
    {
        /// Leer heisst `pair_id: null` — „kein Paar" (NAK-181 R6a).
        ///
        /// Eine LEERE Zeichenkette auf der Leitung ist ausdruecklich keine
        /// zweite Schreibweise dafuer und wird abgewiesen; die zwei Faelle
        /// waeren im Modell sonst nicht mehr zu unterscheiden.
        std::string pairId;
        std::string klasse;
        std::string kettenbefund;
        /// Leer heisst „das Paar traegt eine Aussage".
        ///
        /// 🔑 NAK-181 R6: seit diesem Ticket ist das WAHR. Ein vorhandenes
        /// `ausschluss` muss aus der geschlossenen Menge kommen; ein
        /// Nicht-String faellt nicht mehr still weg (V10). Nacharbeit 1
        /// (NR-01): dazu zaehlt ein vorhandenes `null` — nur die
        /// ABWESENHEIT des Feldes heisst „das Paar traegt eine Aussage".
        std::string ausschluss;
    };

    /// Ein Ursachenbefund aus `session_snapshot.findings` (SONDE-014 E-04).
    ///
    /// 🔑 Bis SONDE-014 gab es im ganzen Produktcode keinen „Befund" im Sinne
    /// von Entwurf Paragraph 8 — nur Messungen, Vergleichsurteile und
    /// Experimentergebnisse. Dies ist seine Modellform auf der Gen-Seite.
    ///
    /// ⚠️ KEINE Pixel. Der Befund traegt Zustand, Zahlen und die drei
    /// Anzeigezeilen als FELDER; wie Flaeche 1 sie anordnet, ist eine
    /// Designfrage und gehoert dem User (S31b).
    struct Befund
    {
        std::string findingId, candidateSource, passageId;
        /// Die drei Aussageklassen aus Paragraph 36.1.
        std::string claimClass;
        /// Eine der SIEBEN Ursachenklassen aus Paragraph 8.
        std::string ursachenklasse;
        std::string targetMetric;
        /// Halboffenes Bandintervall `[von, bis)` im 221er-Evidenzgitter.
        std::uint32_t bandVon = 0, bandBis = 0;
        /// Ohne `beobachtungGueltig` ist `beobachtungWertDb` KEINE Messung.
        double beobachtungWertDb = 0.0;
        bool beobachtungGueltig = false;
        /// `READY TO SEND` | `MORE DATA` | `STALE` (Abnahme U21).
        std::string zustand;
        /// `class` UND `score` — zwei Felder. Die Klasse wird nicht aus dem
        /// Score gerundet, und die Anzeige rundet sie erst recht nicht.
        std::string confidenceKlasse;
        double confidenceScore = 0.0;
        /// Die IDs EIGENER Befunde in derselben Liste, kein Text (U21).
        std::vector<std::string> alternatives;
        /// Die ausgeschiedenen Kandidaten samt Grund aus der geschlossenen
        /// Achtermenge. Ein kommentarlos entfernter Kandidat ist ein Defekt.
        std::vector<std::pair<std::string, std::string>> ausschluesse;
        std::vector<std::string> evidenceIds;
        /// SONDE-014 Etappe E (E-04, M-36 bis M-41): der laufende, je
        /// Frequenzbereich aufloesbare Maskierungswert — AM Befund, nicht in
        /// einer zweiten Liste.
        ///
        /// ⚠️ `maskierungVorhanden == false` heisst „dieser Befund traegt
        /// keinen" und ist etwas ANDERES als ein Wert mit
        /// `maskierungGueltig == false` („hier ist nichts gemessen"). Die
        /// Anzeige braucht beide Faelle getrennt.
        ///
        /// Der Wert traegt Frequenzbereich und Zahl — KEINE Farbe, keine
        /// Achse, keinen Zoom (M-80, M-38, M-39).
        bool maskierungVorhanden = false;
        std::string maskierungQuelleA, maskierungQuelleB;
        std::uint32_t maskierungBandVon = 0, maskierungBandBis = 0;
        double maskierungWertDb = 0.0;
        bool maskierungGueltig = false;
        /// Die Kadenz einer der beiden Quellen ist reduziert (M-37). Ein
        /// Kennzeichen, nie ein interpolierter Zwischenwert.
        bool maskierungHerabgesetzt = false;
        std::string nextTest;
        std::uint64_t intentRevision = 0;
        /// Die drei Zeilen aus Abnahme U21, als DREI Felder — damit die
        /// Anzeige keine davon aus mehreren Feldern zusammensetzt.
        juce::String likelyCause, smallestTest, listenFor;

        /// M-30: **nur** `READY TO SEND` erlaubt `HOLD TO AUDITION`.
        ///
        /// ⚠️ Die Sperre liegt HIER, im Datenweg, und nicht in einer
        /// ausgegrauten Schaltflaeche, die trotzdem sendet. Dieselbe Regel
        /// wie `Befundzustand::erlaubt_audition` im Broker (M-77).
        bool darfAudition() const { return zustand == "ready_to_send"; }
        /// M-30: **nur** `READY TO SEND` erlaubt `SEND DRAFT -> EQ`.
        bool darfDraft() const { return zustand == "ready_to_send"; }
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
        /// SONDE-014 Etappe D (E-04): die Ursachenbefunde dieser Sitzung.
        /// Leer heisst „diese Sitzung fuehrt keine Befunde", nie „alle
        /// geschlossen".
        std::vector<Befund>  befunde;
        /// SONDE-013 Nacharbeit 2 (Befund R28): wie oft in dieser Sitzung
        /// bereits eingegangene Evidenz zurueckgenommen wurde, und warum.
        /// Leere Zeichenketten heissen "keine Ruecknahme", nie "unbekannt".
        std::uint64_t evidenzRuecknahmen = 0;
        std::string ruecknahmeGrund, ruecknahmeUmfang;
    };

    enum class SnapshotErgebnis { ignoriert, uebernommen, ungueltig };
    enum class RuecknahmeErgebnis { ignoriert, uebernommen, ungueltig };

    /** Wie eine Publikation ausgegangen ist (NAK-283 Etappe 2, F01; Manifest
        `docs/beweise/NAK-283.md` M-01, M-72, Paragraph 8.1 Feinheit 19).

        Die Ablehnung hat seit NAK-283 ZWEI Bedeutungen, und beide verlangen
        beim Aufrufer verschiedenes Verhalten - ein gemeinsamer `false`-Zweig
        verloere beides zugleich:

        - `uebernommen`   die Publikation hat stattgefunden. Host-Dirty und
                          Revision wie bisher (auch bei gleichem Inhalt: die
                          Publikation fand statt, nur ohne Aenderung an der
                          Mitgliederkarte).
        - `ueberholt`     eine JUENGERE Publikation derselben Generation liegt
                          schon im Modell. Die eigene Aenderung ist im State
                          ANGEWANDT und gilt - nur die Darstellung ist juenger.
                          Deshalb meldet der Aufrufer Host-Dirty und erhoeht die
                          Revision wie im Erfolgsfall (`nakama-state-v2.md:139`:
                          "steigt bei jeder persistenten Aenderung genau
                          einmal").
        - `reloadAbgewiesen` die Generation weicht von der zuletzt geladenen ab.
                          Die Anwendung ist NICHTIG, der geladene State ist die
                          Wahrheit: kein Dirty, keine Revision (M-04). */
    enum class Publikation
    {
        uebernommen,
        ueberholt,
        reloadAbgewiesen
    };

    /** Die Publikation der persistenten Mitglieder, mit der Reload-Generation,
        FUER die sie gilt (NAK-246 Abschluss Nacharbeit 1 Fortsetzung, Regel
        R-A1 Punkt 4'; Manifest Paragraph 13.5, M-39), und seit NAK-283 Etappe 2
        mit der FOLGENUMMER des Mitgliederstandes, den sie traegt (F01, M-01).

        Der Generationsvergleich faellt HIER, unter demselben `mutex` wie
        `projektReload` - also atomar zur Publikation. Die Fassung davor
        verglich im Prozessor unmittelbar VOR der Publikation und liess zwischen
        Vergleich und Uebernahme wenige Befehle offen; genau dieses Fenster ist
        die verbliebene Auspraegung des Befundes P2 (Paragraph 13.4).

        🔑 NAK-283 F01: die Generation ordnet nur PROJEKTE, nicht die
        Mitgliederstaende INNERHALB eines Projekts. Zwei Publikationen derselben
        Generation - ein Worker-Drain und ein Handgriff auf dem Message-Thread -
        trugen bisher dieselbe Zahl, und die spaeter eintreffende gewann, auch
        wenn ihr Stand aelter war. `folge` ist diese fehlende Ordnung: sie wird
        unter DEMSELBEN `mutex` vergeben, unter dem der Stand entsteht
        (`bindungMutex` im Prozessor), und steigt genau einmal je gezogener
        Kopie. Abgewiesen wird jede Publikation, deren Nummer NICHT GROESSER ist
        als die zuletzt uebernommene.

        Kein Default-Argument: jeder Aufrufer reicht Generation UND Folgenummer,
        die er im selben Block wie die Kopie gelesen hat. Der Startwert ist 0
        wie im Prozessor; die erste vergebene Nummer ist 1. */
    Publikation setzePersistenteMitglieder (
        const std::vector<nakama::state::MainProjectMitglied>& mitglieder,
        std::uint64_t generation,
        std::uint64_t folge);
    /** NAK-283 Etappe 2 (M-72): wie viele Publikationen das Modell wegen einer
        nicht groesseren Folgenummer abgewiesen hat - der Zeuge des Falls, den
        F01 beschreibt. Getrennt von `publikationenNachReloadAbgewiesen`
        gefuehrt, damit die beiden Ablehnungsgruende unterscheidbar sind. */
    std::uint64_t publikationenUeberholt() const;
    /** NAK-283 Etappe 2 (M-72): wie viele Publikationen das Modell wegen einer
        abweichenden Reload-Generation abgewiesen hat. Zaehlt NUR die
        Entscheidungen dieses Modells; der Prozessor zaehlt daneben auch die
        Publikationen, die schon sein frueher Ausstieg abfaengt
        (`Ipc.cpp`, `sourcesNachfuehrungNachReloadUnterblieben`). */
    std::uint64_t publikationenNachReloadAbgewiesen() const;
    /** NAK-283 Etappe 2 (M-06): der persistente Mitgliederbestand, wie ihn das
        Modell fuehrt - Schluessel und Label. Die Zusage "nach Ruhe zeigen
        Prozessorstate und Modell denselben Bestand" braucht beide Seiten; die
        `Sicht` mischt Mitgliedschaft mit Fixture- und Snapshotzeilen und kann
        sie nicht belegen. */
    std::map<std::string, juce::String> persistenteMitgliederKopie() const;
    /** Der Projektwechsel: Mitglieder UND Generation im selben Block wie der
        Neuaufbau der Eintraege (R-A1 Punkt 4' (a)). Ab hier weist das Modell
        jede Publikation einer aelteren Generation ab; eine Publikation, die es
        noch VOR diesem Aufruf erreicht, wird von ihm ueberschrieben - der
        Endzustand ist in beiden Reihenfolgen der geladene State. */
    void projektReload (
        const std::vector<nakama::state::MainProjectMitglied>& mitglieder,
        std::uint64_t generation);
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
    /// SONDE-014 Etappe I (M-84): die Zahl offener Befunde je Quelle,
    /// ABGELEITET aus `befunde` nach jedem Sitzungsschnitt.
    ///
    /// Sie hat keinen Setter. Ein Setter neben der Ableitung waere eine
    /// zweite Wahrheit ueber dieselbe Zahl (M-71); bis Etappe I gab es genau
    /// den - mit einem einzigen Aufrufer, und der war ein Test.
    void zaehleOffeneFindings();
    /// NAK-214 R3/R8: jeder Befund dieser Sitzung wird `stale`, und die
    /// abgeleitete Zahl faellt MIT ihm.
    ///
    /// Zwei lokale Anlaesse rufen sie: das Verbindungsende (`controlEnde`)
    /// und eine Evidenzruecknahme ohne Folge-Snapshot. Beide sind Ereignisse,
    /// die der Broker nicht mehr melden kann — die Liste bleibt sichtbar, sie
    /// war zuletzt wahr, aber KEINER ihrer Befunde ist noch handelbar (M-30:
    /// die Sperre haengt am `zustand`, nicht an einem zweiten Flag).
    ///
    /// ⚠️ Der Aufrufer haelt den `mutex` bereits, wie bei
    /// `zaehleOffeneFindings()` auch.
    void setzeAlleBefundeStale();
    /// NAK-246 D6 (R-D6): die EINE Reset-Funktion fuer den Sitzungszustand.
    ///
    /// Sie leert genau die sieben Groessen einer Sitzung: `experimente`,
    /// `paare`, `befunde`, `findingsOffen` je Zeile, `evidenzRuecknahmen`,
    /// `ruecknahmeGrund` und `ruecknahmeUmfang`. Beide Wege, auf denen eine
    /// Sitzung endet, rufen sie - `beginneSubscription` (eine neue Sitzung)
    /// und `projektReload` (ein anderes Projekt). Bis NAK-246 leerte nur der
    /// erste diese Menge; ein read-only oder ungebunden geladener State baut
    /// keine Subscription auf, und die alte Sitzung blieb in der Sicht des
    /// neuen Projekts stehen (Auditbefund D6, SONDE-013 M-50).
    ///
    /// ⚠️ Der Aufrufer haelt den `mutex` bereits.
    void sitzungszustandLeeren();
    /// SONDE-013 Nacharbeit 2 (Befunde R14/R32): der zuletzt empfangene Stand
    /// der Versuche und Paarurteile dieser Sitzung.
    std::vector<Versuch> experimente;
    std::vector<Paar>    paare;
    /// SONDE-014 Etappe D: der zuletzt empfangene Befundstand dieser Sitzung.
    std::vector<Befund>  befunde;
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
    /// NAK-246 Abschluss Nacharbeit 1 Fortsetzung (R-A1 Punkt 4' (a), M-39):
    /// die Reload-Generation, fuer die `persistenteMitglieder` gilt. Nur
    /// `projektReload` setzt sie, `setzePersistenteMitglieder` vergleicht sie -
    /// beide unter diesem `mutex`. Startwert 0 wie `reloadGeneration` im
    /// Prozessor; kein Wire-, Schema- oder Vertragswert.
    std::uint64_t reloadGeneration = 0;
    /// NAK-283 Etappe 2 (F01, M-01, M-05): die Folgenummer des zuletzt
    /// UEBERNOMMENEN Mitgliederstandes. Sie wird von JEDER angenommenen
    /// Publikation fortgeschrieben - auch von der inhaltsgleichen, die nur die
    /// Mitgliederkarte unberuehrt laesst (M-05, Paragraph 8.1 Feinheit 20).
    /// Eine dort nicht verbrauchte Nummer liesse die angehaltene aeltere Kopie
    /// danach mit einer groesseren Nummer wieder durch. Startwert 0 wie der
    /// Zaehler im Prozessor; kein Wire-, Schema- oder Vertragswert.
    std::uint64_t zuletztUebernommeneFolge = 0;
    /// NAK-283 Etappe 2 (M-72): die beiden Ablehnungsgruende, getrennt gezaehlt.
    /// Ohne Verhalten - sie sind die Zeugen der Faelle.
    std::uint64_t ueberholtZaehler = 0;
    std::uint64_t reloadAbgewiesenZaehler = 0;
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
