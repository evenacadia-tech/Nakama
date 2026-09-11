// EqCopilotProcessor — IPC-Anbindung: Broker, Control, Telemetrie, Interventionen.
//
// NAK-225/S25d (09.09.2026), herausgeloest aus src/PluginProcessor.cpp. Inhalt:
//
//   v3Hello, v3Status, v3TelemetryHello, v3SubscribeJson
//                        Was die Instanz beim Verbinden ueber sich sagt.
//   v3ControlLink, v3Antwort, v3Frame
//                        Was sie zurueckbekommt: Linkwechsel, Antworten auf
//                        eigene Auftraege, Telemetrieframes.
//   v3IntentUpdateJson, v3AssistantStepJson, v3UserVerdictJson,
//   urteilMitFrischemKopf, sendeIntentVollbestand, sendeIntentFortschreibung
//                        Der Intent- und Assistentenvertrag auf dem Draht.
//   interventionsWireJson, interventionenSenden
//                        Der Interventionsring wird ausserhalb des
//                        Audio-Threads geleert und gesendet.
//   statsSnapshot, messKompakt
//                        Der Messstand, den Heartbeat und Probe mitnehmen.
//   sourcesTick, reconnectSources, bindeSourcesHauptziel,
//   benenneSourcesHauptziel, entferneSourcesHauptziel, sendeSourcesCommand,
//   wendeBestaetigteSourcesCommandsAn (Rahmen), bestaetigteSourcesCommandsAbholen,
//   wendeSourcesCommandAnUnterBindung, meldeSourcesMitgliederNachBefehl,
//   ausstehenderSourcesCommandFuerTest, bestaetigteSourcesCommandsFuerTest,
//   merkeSourcesCommandFuerTest
//                        Die Quellenbefehle und ihr Rueckweg.
//
// Die Invariante dieser Datei (CLAUDE.md, tragende technische Invarianten):
// Steuerung ueber Pipe oder UI wird AUSSERHALB des Audio-Threads uebernommen;
// die Zustands- und Parameterhoheit bleibt in der Audio fuehrenden Instanz.
// Kein Aufruf dieser Datei liegt auf dem Audio-Pfad. Der Ring, aus dem
// interventionenSenden liest, wird vom Audio-Thread nur beschrieben.
//
// Probe-Pipe ist nie Produktions-Pipe: welche Pipe eine Instanz waehlt, kommt
// aus PipeToken.h und wird hier nie ausgerechnet.

#include "../PluginProcessor.h"
#include "../EqCopilotIds.h"
#include "PipeToken.h"
#include "Intern.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <process.h>

namespace eqcop
{

using prozessor_intern::uuidHex32;

namespace
{
std::string alsHex32 (const juce::String& wert, const char* domain)
{
    std::string roh = wert.toStdString();
    std::string aus;
    aus.reserve (32);
    for (char c : roh)
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            aus.push_back (c);
    return aus.size() == 32 ? aus
                            : nakama::ipc::instanceAdresseAusState (
                                  std::string (domain) + ":" + roh);
}

/// Eine endliche Zahl in Wire-Form. NaN und Inf entstehen hier gar nicht
/// erst: der Aufrufer hat sie beim Setzen schon abgewiesen (M-82).
std::string wireZahl (double x)
{
    if (! std::isfinite (x))
        return "0";
    return juce::String (x, 6).toStdString();
}
} // namespace

std::string EqCopilotProcessor::interventionsWireJson (
    const nakama::ipc::Interventionsereignis& e, const std::string& adresseJson) const
{
    // 🔑 Nacharbeit 3 (Befund C1): der Wiretext entsteht getrennt vom Senden.
    //
    // Bis dahin lag beides in einer Schleife, und die Zahl, die WIRKLICH auf
    // die Leitung geht, war ausserhalb des Prozessors nicht messbar. Ein Bein
    // konnte den Handschlag deshalb nur an lokalen Flags pruefen — genau die
    // Sorte Test, die an einem Vertragsbruch zwischen zwei Sprachen nicht
    // fallen kann.
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    std::string json = "{\"type\":\"";
    json += e.beginn ? "audible_intervention_begin" : "audible_intervention_end";
    json += "\",\"intervention_id\":\"";
    json += nakama::ipc::instanceAdresseAusState (
                "intervention:" + h.adresse.instanceId + ":"
                + std::to_string (e.nummer));
    json += "\",\"adresse\":";
    json += adresseJson;
    json += ",\"event_sequence\":";
    json += std::to_string (e.sequenz);
    if (e.beginn)
    {
        // SONDE-013 §7.1 E-08: der Hoermarker bleibt in Gen/Main, und
        // dieses Ticket baut nur ihn. Die drei uebrigen Arten aus dem
        // Schema bekommen ihre Erzeuger in P6 und P7.
        json += ",\"art\":\"hoermarkierung\"";
        json += ",\"project_sample_start\":";
        json += e.projektzeitGesetzt ? std::to_string (e.projektSample)
                                     : std::string ("null");
    }
    else
    {
        json += ",\"project_sample_end\":";
        json += e.projektzeitGesetzt ? std::to_string (e.projektSample)
                                     : std::string ("null");
        json += ",\"tail_samples\":";
        json += std::to_string (e.tailSamples);
    }
    json += "}";
    return json;
}

void EqCopilotProcessor::interventionenSenden()
{
    // 🔑 NAK-180 R8: der Ring ist nicht mehr die einzige Quelle.
    //
    // Ein Replay steht auch dann an, wenn der Ring LEER ist — genau dann
    // naemlich, wenn ein Begin bereits gesendet wurde und sein Marker
    // weiterklingt (C2). Die alte Abbruchbedingung liess diesen Zug nie
    // laufen.
    const bool etwasZuTun = interventionsRing.fuellstand() != 0
                         || replayFaellig.load() != 0
                         || berichtOffen.load() != 0;
    if (! etwasZuTun)
        return;
    // 🔑 NAK-40: DIESELBE Wireadresse wie der Control-Bootstrap. `v3Hello()`
    // liefert die PERSISTENTE Instance-ID; der Alias entsteht erst an der
    // v3-Grenze. Ohne ihn pruefte dieser Pfad eine Adresse, die so nie auf
    // der Leitung steht — bei einer hex32-ID faellt das nicht auf, bei einer
    // unterstuetzten Legacy-ID schlug `adresseGueltig` fehl und der Ring
    // wurde kommentarlos GELEERT: der Marker verschwand.
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
    {
        // Ohne gueltige Adresse kann kein Ereignis reisen. Es wird trotzdem
        // ENTNOMMEN: ein Ring, der sich bei fehlender Adresse fuellt, liefe
        // ueber und meldete einen Ueberlauf, der keiner ist.
        nakama::ipc::Interventionsereignis weg;
        while (interventionsRing.lies (weg)) {}
        return;
    }

    const auto adresseJson = [&h]
    {
        std::string a = "{\"logon_sid\":\"" + h.adresse.logonSid
                      + "\",\"project_binding_id\":\"" + h.adresse.projectBindingId
                      + "\",\"session_epoch\":\"" + h.adresse.sessionEpoch
                      + "\",\"instance_id\":\"" + h.adresse.instanceId
                      + "\",\"runtime_nonce\":\"" + h.adresse.runtimeNonce + "\"}";
        return a;
    }();

    // 🔑 NAK-180 R12: EIN Zug unter `sendeMutex`. Der Vergleich der
    // Zustellgeneration und das Einreihen der Folgenachricht liegen damit in
    // derselben kritischen Zone — zwischen ihnen kann kein Link aufgebaut
    // werden. Vorher entschied die Ordnung beim Einreihen, ihre Gueltigkeit
    // hing aber am Link bei der Zustellung (MP4-1).
    bool neutralErreicht = false;
    std::uint64_t abschlussGeneration = 0;
    controlV3.interventionsZug (
        [this, &adresseJson, &neutralErreicht, &abschlussGeneration]
        (std::uint64_t generation, const nakama::ipc::ControlClient::ZugSenke& senke)
    {
        std::lock_guard<std::mutex> l (sendeZustandMutex);   // Ordnung: sendeMutex zuerst
        if (zugHakenFuerTest)
            zugHakenFuerTest (0);            // Zug begonnen (Test, EP-16/EP-20)

        auto sende = [&] (const nakama::ipc::Interventionsereignis& ev,
                          nakama::ipc::P0Klasse klasse) -> std::uint64_t
        {
            // 🔑 NAK-180 Nacharbeit 1 (EP-07): die Marke kommt aus dem EINEN
            // Markenraum des ControlClients. Zwei Folgen mit eigenem Nullpunkt
            // liessen einen Aufbau-Heartbeat und das erste Marker-Begin
            // dieselbe Zahl tragen; der Zustellrueckruf des Heartbeats buchte
            // dann das Begin als zugestellt, und ein Reconnect erzeugte ein
            // doppeltes Replay (N-27/N-36).
            auto json = interventionsWireJson (ev, adresseJson);
            const auto marke = senke (json, klasse);
            if (marke == 0)
            {
                // Der P0-Ueberlauf ist derselbe Fall wie der Ringueberlauf:
                // die Sequenzluecke kommt beim Coordinator an.
                interventionsRingUeberlauf.store (true, std::memory_order_relaxed);
                return 0;
            }
            // 🔑 EP-13/R7: Mitschnitt und Zaehler entstehen NICHT hier. Das
            // Einreihen ist kein Senden; erst der Wire-Commit ist es. Der Text
            // wartet unter seiner Marke auf den Zustellrueckruf.
            if (ausstehendeMitschnitte.size() >= kAusstehendDeckel)
                ausstehendeMitschnitte.pop_front();
            ausstehendeMitschnitte.emplace_back (marke, std::move (json));
            return marke;
        };

        // Bildet das Replay-Begin des offenen lokalen Begins. Dieselbe
        // `intervention_id` (sie entsteht deterministisch aus `instance_id`
        // und Nummer), dieselbe `art`, dieselbe Projektzeit — nur die
        // Sequenznummer ist die naechste.
        // Darf das offene lokale Begin repliziert werden? (E6, dreiwertiger
        // Zustellstand)
        //   0 — nicht eingereiht        → ja, es ist nirgends
        //   1 — eingereiht, nicht auf dem Draht → NEIN, es liegt in der Queue
        //       und reist auf dem neuen Link von selbst; ein Replay waere die
        //       doppelte `intervention_id` aus N-27
        //   2 — zugestellt              → ja, wenn auf einer AELTEREN
        //       Generation; der laufende Link kennt es dann nicht
        auto replayNoetig = [&] (std::uint64_t g)
        {
            if (! offenesBegin.gueltig)
                return false;
            if (offenesBegin.zustand == 1)
                return false;
            if (offenesBegin.zustand == 2)
                return offenesBegin.zustellGeneration != g;
            return true;                       // Zustand 0
        };

        auto replaySenden = [&] () -> bool
        {
            if (! offenesBegin.gueltig)
                return false;
            // 🔑 NAK-180 Messlauf: das Replay traegt die ORIGINALSEQUENZ.
            //
            // Es ist eine WIEDERHOLUNG, keine neue Nachricht - dieselbe
            // `intervention_id`, dieselbe Nummer. Zog es eine frische Zahl,
            // stand sie ueber der des Ring-Backlogs, das der Audiothread
            // schon nummeriert hat: der Broker sah 3, dann 2, wertete das als
            // Luecke und setzte `unknown` (`sequenz_annehmen`). Der frische
            // Link fuehrt `letzte_event_sequence = None` und nimmt die erste
            // Zahl vorbehaltlos an; das Backlog zaehlt danach lueckenlos
            // weiter.
            auto ev = offenesBegin.ereignis;
            ev.beginn = true;
            const auto marke = sende (ev, nakama::ipc::P0Klasse::bericht);
            if (marke == 0)
                return false;
            offenesBegin.zustand = 1;              // eingereiht
            offenesBegin.marke = marke;
            return true;
        };

        // 🔑 NAK-180 Nacharbeit 1 (EP-01/N-36): das offene Begin faellt erst
        // mit dem WIRE-COMMIT seines `end`, nicht mit dessen Einreihen.
        //
        // Stirbt der Link dazwischen, ueberlebt das `end` als Ereignis in der
        // Queue — der Aufbauzug des naechsten Links faende ohne diese
        // Aufzeichnung kein Begin mehr, stellte kein Replay voran, und das
        // `end` traefe beim Broker auf nichts: die Sitzung nullt nie. Und
        // derselbe Rueckruf stellt die Nachlauffrist (EP-02): erst ab dem
        // Draht zaehlt der Broker `tail_samples_offen`.
        auto sendeEnde = [&] (const nakama::ipc::Interventionsereignis& ende)
        {
            // Lag das Begin selbst noch in der Queue (Zustand 1), reist es vor
            // dem `end` und braucht keine Aufzeichnung — die FIFO-Ordnung
            // traegt sie. Nur ein bereits ZUGESTELLTES Begin ist nirgends mehr
            // greifbar.
            const bool warZugestellt = offenesBegin.gueltig && offenesBegin.zustand == 2;
            auto begin = offenesBegin.ereignis;
            begin.beginn = true;
            const auto marke = sende (ende, nakama::ipc::P0Klasse::intervention);
            if (marke != 0)
            {
                // 🔑 Nacharbeit 2 (WN-03): JEDES eingereihte `end` wartet mit
                // SEINEM Nachlauf auf den Wire-Commit. Der Deckel ist
                // Verteidigung, nicht Politik: jeder Eintrag haelt einen der
                // 64 P0-Plaetze, mehr als das kann hier nicht stehen.
                if (ausstehendeEnden.size() >= kAusstehendDeckel)
                {
                    // Unerreichbar, solange die Invariante haelt - und wenn
                    // doch, dann fail-closed: die Frist des verdraengten
                    // Eintrags gilt ab JETZT, statt ersatzlos zu verfallen.
                    nachlaufFristSetzen (ausstehendeEnden.front().second);
                    ausstehendeEnden.pop_front();
                }
                ausstehendeEnden.emplace_back (marke, ende.tailSamples);
                // 🔑 Nacharbeit 2 (WN-01/N-36): der Prozessor ist ab hier
                // NICHT neutral. Das `end` ist eingereiht, aber der Broker hat
                // es noch nicht gesehen; ein `false` des naechsten Aufbaus
                // loeschte genau den Tail, den es gleich startet.
                abschlussOffen.store (true, std::memory_order_relaxed);
                if (warZugestellt)
                    abschlussBegin = AbschlussBegin { begin, true, marke };
            }
            offenesBegin = OffenesBegin {};
            sendeBeginOffen.store (false, std::memory_order_relaxed);
        };

        // 🔑 EP-08/N-10, zweiter Fall: `prepareToPlay` hat den Marker hart
        // abgeschaltet, waehrend sein Begin noch im RT-Ring lag. Der Uebergang
        // wartet mit seiner Sequenz; er wird genau vor dem ersten Ringereignis
        // mit HOEHERER Sequenz eingereiht, damit die Folge lueckenlos bleibt.
        auto totEndeSenden = [&] ()
        {
            auto ende = offenesBegin.ereignis;
            ende.beginn = false;
            ende.projektzeitGesetzt = false;   // die Endzeit ist ehrlich unbekannt
            ende.sequenz = ausstehenderTotUebergang.sequenz;
            ende.dauerSamples = ausstehenderTotUebergang.dauerSamples;
            ende.tailSamples = ausstehenderTotUebergang.tailSamples;
            if (replayNoetig (generation))
                replaySenden();
            sendeEnde (ende);
            ausstehenderTotUebergang = TotUebergang {};
        };

        // (1) Faelliges Replay des Aufbaus — nur fuer DIESE Generation.
        auto faellig = replayFaellig.load();
        if (faellig != 0)
        {
            if (faellig == generation)
            {
                if (replayFaellig.compare_exchange_strong (faellig, 0)
                    && replayNoetig (generation))
                    replaySenden();
            }
            else
            {
                // Fremde Generation: nicht werten, nur aufraeumen — und auch
                // das nur per CAS, damit ein gerade geschriebenes G+1 stehen
                // bleibt (R13).
                replayFaellig.compare_exchange_strong (faellig, 0);
            }
        }

        // (1b) Ein TOTES Begin wird VOR dem Ringinhalt geschlossen.
        //
        // `prepareToPlay` hat den Marker hart abgeschaltet; sein `end` kommt
        // vom Audiothread nie. Es steht fertig im Sendezustand - mit einer
        // Sequenz, die VOR der des neuen Begins liegt, das der wieder
        // hochfahrende Marker gleich schreibt. Erst danach darf der Ring
        // gelesen werden: ein neues Begin ueberschriebe den Sendezustand,
        // und das tote Intervall bliebe beim Broker fuer immer offen.
        if (offenesBegin.gueltig && offenesBegin.tot)
        {
            if (replayNoetig (generation))
                replaySenden();
            sendeEnde (offenesBegin.totesEnde);
        }

        // (2) Der Ringinhalt, in unveraenderter Reihenfolge.
        nakama::ipc::Interventionsereignis e;
        while (interventionsRing.lies (e))
        {
            // EP-08: das wartende tote Ende gehoert VOR jedes Ereignis mit
            // hoeherer Sequenz — sonst stuende auf dem Draht Begin(n-1),
            // Begin(n+1), End(n), und der Broker saehe eine Luecke.
            if (ausstehenderTotUebergang.gueltig && offenesBegin.gueltig
                && e.sequenz > ausstehenderTotUebergang.sequenz)
            {
                totEndeSenden();
            }

            if (e.beginn)
            {
                const auto marke = sende (e, nakama::ipc::P0Klasse::intervention);
                offenesBegin.ereignis = e;
                offenesBegin.gueltig = true;
                offenesBegin.tot = false;
                offenesBegin.zustand = marke != 0 ? 1 : 0;
                offenesBegin.marke = marke;
                offenesBegin.zustellGeneration = 0;
                sendeBeginOffen.store (true, std::memory_order_relaxed);
                continue;
            }

            // 🔑 R8, Ordnungsregel: ein `end` reist nur, wenn sein Begin auf
            // DIESEM Link zugestellt ist (Zustand 2 mit der laufenden
            // Generation) oder in derselben Queue vor ihm liegt (Zustand 1 —
            // die FIFO-Ordnung stellt es von selbst zu). Sonst geht das
            // Replay-Begin unmittelbar voran; ohne das traefe das `end` beim
            // Broker auf kein Begin und setzte `unknown`.
            if (replayNoetig (generation))
                replaySenden();

            // EP-16/N-35: das Fenster zwischen Vergleich und Einreihen. Im
            // Produkt gibt es keines - beide liegen unter derselben Sperre.
            // Der Haken macht genau das messbar: ein zweiter Faden, der den
            // Linkwechsel HIER erzwingen will, kommt erst nach dem Einreihen
            // zum Zug.
            if (zugHakenFuerTest)
                zugHakenFuerTest (1);

            sendeEnde (e);
        }

        // Ring erschoepft: ein noch wartendes totes Ende geht jetzt raus.
        // Ohne jedes Begin — es ging im Ringueberlauf verloren — faellt es
        // ersatzlos weg; das Sticky-Bit des Ueberlaufs traegt den Zustand.
        if (ausstehenderTotUebergang.gueltig)
        {
            if (offenesBegin.gueltig)
                totEndeSenden();
            else
                ausstehenderTotUebergang = TotUebergang {};
        }

        // (4) Abschluss des Neuaufbau-Berichts (E3.3): ist der Prozessor nach
        //     einem NICHT neutralen Aufbau wieder neutral, darf jetzt genau
        //     ein `false` reisen. Nur fuer die laufende Generation.
        auto bericht = berichtOffen.load();
        if (bericht != 0)
        {
            if (bericht != generation)
            {
                berichtOffen.compare_exchange_strong (bericht, 0);
            }
            else if (interventionsRing.fuellstand() == 0 && ! offenesBegin.gueltig
                     && ! abschlussBegin.gueltig
                     && ! ausstehenderTotUebergang.gueltig
                     // 🔑 EP-02, zweite Haelfte: ein `end`, das eingereiht,
                     // aber noch NICHT auf dem Draht ist, hat den Nachlauf
                     // beim Broker noch gar nicht gestartet. Schloesse der
                     // Bericht hier, reiste das `false` unmittelbar hinter dem
                     // `end` - und traefe dort auf den vollen Nachlauf. Die
                     // Frist allein genuegt nicht: sie steht erst ab dem
                     // Wire-Commit.
                     && ausstehendeEnden.empty()
                     && ! markierung.hoerbar()
                     // 🔑 NAK-180 Nacharbeit 1 (EP-02/N-05/M-58): der
                     // NACHLAUF des zuletzt zugestellten `end` muss in
                     // Echtzeit abgelaufen sein. Erreicht das einmalige
                     // `false` den Broker waehrend `tail_samples_offen > 0`,
                     // verwirft er es — und niemand wiederholt es, weil
                     // `berichtOffen` und die Aufbauaussage mit ihm
                     // verbraucht sind. `berichtOffen` bleibt bis dahin
                     // stehen; der naechste Zug prueft erneut.
                     && nachlaufAbgelaufen())
            {
                if (berichtOffen.compare_exchange_strong (bericht, 0))
                {
                    neutralErreicht = true;
                    // 🔑 EP-03/R12: das Urteil gilt GENAU DIESER Generation.
                    abschlussGeneration = generation;
                }
            }
        }

        if (zugHakenFuerTest)
            zugHakenFuerTest (2);            // Zug beendet (Test)
    });

    // AUSSERHALB des Zugs: `meldeAufbauUrteil` nimmt `sendeMutex` nicht, aber
    // die Regel "im Zug wird nicht gesendet" gilt trotzdem — der naechste
    // Heartbeat traegt das `false`.
    //
    // 🔑 EP-03: die Generation reist MIT. Baut G+1 in diesem Fenster auf,
    // wendet der ControlClient den veralteten Abschluss nicht an — sonst
    // ersetzte ein `false` von G das frische, nicht neutrale Urteil von G+1,
    // und der Bericht des neuen Links waere nie eroeffnet.
    if (neutralErreicht)
        controlV3.meldeAufbauUrteil (true, abschlussGeneration);
}

nakama::ipc::ControlHello EqCopilotProcessor::v3Hello() const
{
    nakama::ipc::ControlHello h;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        h.adresse.logonSid = v3LogonSid;
        h.adresse.projectBindingId = zustand.common.projectBindingId.toStdString();
        h.adresse.sessionEpoch = v3SessionEpoch;
        // Die persistente Original-ID bleibt bis zur gemeinsamen v3-Grenze
        // im ControlClient erhalten; erst dort wird der Phase-A-Wirealias
        // gebildet. So kann er nie in den Host-State zurueckfliessen.
        h.adresse.instanceId = zustand.common.instanceId.toStdString();
        h.adresse.runtimeNonce = alsHex32 (instanceNonce, "runtime_nonce");
        h.pluginKind = nakama::state::wort (zustand.common.klasse);
    }
    h.hostAngeben = true;
    h.hostPid = static_cast<std::uint32_t> (_getpid());
    h.pluginVersion = kPluginVersion;
    h.samplerate = samplerateAtomic.load();
    h.blockSize = blockSizeAtomic.load();
    h.channels = kanaeleAtomic.load();
    return h;
}

nakama::ipc::ControlStatus EqCopilotProcessor::v3Status() const
{
    nakama::ipc::ControlStatus s;
    s.dspSchemaVersion = 1;
    s.stateRevision = v3StateRevision.load();
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.hatParameters && ! zustand.nurLesen)
        {
            juce::String hash, grund;
            if (nakama::parameter::stateHash (zustand.dspDto(), hash, grund))
                s.stateHash = hash.toStdString();
        }
    }
    s.recordStateValid = aufnahmeGueltig.load();
    s.recording = aufnahmeAktiv.load();
    s.framesDropped = queue.verloreneFrames();
    // 🔑 SONDE-013 M-39: das Sticky-Bit des RT→Control-Rings wird GELESEN.
    //
    // Vorher setzten beide Ueberlaufpfade (Audiothread und P0-Sender) das Bit,
    // und niemand las es: ein verlorenes Begin oder End erzeugte dann keine
    // Sequenzluecke mehr — die Nummer war verbraucht, aber wenn das LETZTE
    // Ereignis fiel, folgte kein weiteres, an dem die Luecke sichtbar wuerde.
    // Der Broker blieb scheinbar sauber, und genau das verbietet §34.2:
    // "Ein verlorenes Begin oder End darf niemals eine scheinbar saubere
    // Baseline erzeugen". Der 1-Hz-Heartbeat traegt das Bit deshalb aktiv;
    // beim Empfaenger loest es dasselbe sticky Unknown aus wie die Luecke.
    //
    // Es heilt nicht von selbst: nur `interventionsRing.resync()` bzw.
    // `zuruecksetzen()` loeschen es, und beide laufen ausserhalb des
    // Audiothreads am bestaetigten Neuaufbau.
    s.interventionStateUnknown = interventionsRingUeberlauf.load (std::memory_order_relaxed)
                              || interventionsRing.ueberlaufGesehen();
    return s;
}

nakama::ipc::TelemetryHello EqCopilotProcessor::v3TelemetryHello() const
{
    nakama::ipc::TelemetryHello t;
    const auto h = v3Hello();
    t.adresse = h.adresse;
    t.pluginVersion = h.pluginVersion;
    controlV3.kopplung (t.linkId, t.challenge);
    return t;
}

std::string EqCopilotProcessor::v3SubscribeJson() const
{
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
        return {};
    return std::string ("{\"type\":\"subscribe_session\",\"adresse\":")
         + nakama::ipc::adresseAlsJson (h.adresse)
         + ",\"session_epoch\":\"" + h.adresse.sessionEpoch + "\"}";
}

// ── SONDE-014 E-10/E-11: Intent und Assistentenschritt auf den Draht ───────
//
// Die Zahlen entstehen mit `juce::String` und nicht mit `std::to_string`:
// letzteres ist LOCALE-abhaengig, und ein deutsches Dezimalkomma in einer
// JSON-Zahl faellt beim Textriegel des Empfaengers. Derselbe Grund wie beim
// Evidenzserialisierer (NAK-181 N-16).

std::string EqCopilotProcessor::v3IntentUpdateJson (bool vollstaendig,
                                                    const nakama::state::SourceIntent* nurDieser,
                                                    const nakama::state::Schutzangabe* nurDieserSchutz,
                                                    const nakama::state::IntentBeziehung* nurDiese) const
{
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
        return {};

    std::vector<nakama::state::SourceIntent> intents;
    std::vector<nakama::state::Schutzangabe> schutz;
    std::vector<nakama::state::IntentBeziehung> kanten;
    juce::int64 revision = 0;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        revision = zustand.intentBestandRevision;
        if (vollstaendig)
        {
            intents = zustand.sourceIntents;
            schutz  = zustand.schutzangaben;
            kanten  = zustand.intentBeziehungen;
        }
    }
    // Eine TEILMELDUNG traegt genau EIN Objekt (Consumerregel 5, M-85): der
    // P1-Schluessel adressiert genau eines, und eine Sammelnachricht unter
    // demselben Schluessel koennte fremde Objekte verdraengen.
    if (! vollstaendig)
    {
        if (nurDieser != nullptr)          intents.push_back (*nurDieser);
        else if (nurDieserSchutz != nullptr) schutz.push_back (*nurDieserSchutz);
        else if (nurDiese != nullptr)      kanten.push_back (*nurDiese);
        else return {};
    }

    std::string aus = "{\"type\":\"intent_update\",\"adresse\":"
                    + nakama::ipc::adresseAlsJson (h.adresse)
                    + ",\"session_epoch\":\"" + h.adresse.sessionEpoch + "\""
                    + ",\"bestand_revision\":" + std::to_string ((long long) revision)
                    + ",\"vollstaendig\":" + (vollstaendig ? "true" : "false");

    if (! intents.empty())
    {
        aus += ",\"intents\":[";
        for (size_t i = 0; i < intents.size(); ++i)
        {
            const auto& s = intents[i];
            if (i > 0) aus += ",";
            aus += "{\"quelle_id\":\"" + s.quelleId.toStdString() + "\"";
            // Abwesenheit heisst GLOBALER Scope, nie `null`.
            if (s.passageId.isNotEmpty())
                aus += ",\"passage_id\":\"" + s.passageId.toStdString() + "\"";
            aus += ",\"rolle\":\"" + std::string (nakama::state::wort (s.rolle)) + "\"";
            aus += ",\"revision\":" + std::to_string ((long long) s.revision);
            aus += ",\"herkunft\":\"" + std::string (nakama::state::wort (s.herkunft)) + "\"";
            aus += ",\"konfidenz\":" + wireZahl (s.konfidenz) + "}";
        }
        aus += "]";
    }
    if (! schutz.empty())
    {
        aus += ",\"schutzangaben\":[";
        for (size_t i = 0; i < schutz.size(); ++i)
        {
            const auto& s = schutz[i];
            if (i > 0) aus += ",";
            aus += "{\"quelle_id\":\"" + s.quelleId.toStdString() + "\"";
            aus += ",\"eigenschaft\":\"" + std::string (nakama::state::wort (s.eigenschaft)) + "\"";
            // `band` genau dann, wenn die Eigenschaft es ist (Consumerregel 2).
            if (s.eigenschaft == nakama::state::Schutzeigenschaft::band)
                aus += ",\"band\":{\"von\":" + std::to_string (s.bandVon)
                     + ",\"bis\":" + std::to_string (s.bandBis) + "}";
            aus += "}";
        }
        aus += "]";
    }
    if (! kanten.empty())
    {
        aus += ",\"beziehungen\":[";
        for (size_t i = 0; i < kanten.size(); ++i)
        {
            const auto& k = kanten[i];
            if (i > 0) aus += ",";
            aus += "{\"quelle_a\":\"" + k.quelleA.toStdString() + "\""
                 + ",\"quelle_b\":\"" + k.quelleB.toStdString() + "\""
                 + ",\"art\":\"" + std::string (nakama::state::wort (k.art)) + "\"}";
        }
        aus += "]";
    }
    aus += "}";
    return aus;
}

/*  NR-08 (Nacharbeit 1, 07.09.2026), M-59/M-88/M-89: EINE Wahrheit des
    Schritts.

    Der Sender las bis hierher einen eigenen Schatten (`assistentSchritt`,
    mit NR-08 entfernt) mit EIGENER Revisionszaehlung, waehrend
    `setStateInformation()` ausschliesslich `zustand.assistent` restauriert. Die Folge war messbar: nach
    `getStateInformation` -> neuer Prozessor -> `setStateInformation` lag der
    persistente Schritt vor, die Wire-Nachricht blieb aber LEER; bei einem
    Reload im selben Prozessor konnte statt dessen der VORIGE Schritt reisen.
    Zwei Wahrheiten desselben Objekts sind genau das, was M-71 verbietet.

    Gelesen wird deshalb `zustand.assistent` unter dem Bindungsschloss, mit
    DESSEN Revision. Der Schatten ist fort. */

std::string EqCopilotProcessor::v3AssistantStepJson() const
{
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
        return {};

    nakama::state::Assistentenzustand schritt;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (! zustand.assistent.gesetzt)
            return {};
        schritt = zustand.assistent;
    }
    std::string aus = "{\"type\":\"assistant_step_update\",\"adresse\":"
                    + nakama::ipc::adresseAlsJson (h.adresse)
                    + ",\"session_epoch\":\"" + h.adresse.sessionEpoch + "\""
                    + ",\"step_id\":\"" + schritt.stepId.toStdString() + "\""
                    + ",\"schritt\":\"" + std::string (nakama::state::wort (schritt.schritt)) + "\""
                    + ",\"revision\":" + std::to_string ((long long) schritt.revision)
                    + ",\"offen\":" + (schritt.offen ? "true" : "false");
    if (schritt.findingId.isNotEmpty())
        aus += ",\"finding_id\":\"" + schritt.findingId.toStdString() + "\"";
    if (schritt.proposalId.isNotEmpty())
        aus += ",\"proposal_id\":\"" + schritt.proposalId.toStdString() + "\"";
    if (schritt.experimentId.isNotEmpty())
        aus += ",\"experiment_id\":\"" + schritt.experimentId.toStdString() + "\"";
    aus += "}";
    return aus;
}

/*  NR-10 (Nacharbeit 1, 07.09.2026), M-73/E-09: das USERURTEIL auf dem Draht.

    Der Broker nahm die Familie bis zur Nacharbeit 1 nicht an (`_ => None` in
    `befehl.rs`), und das Plugin sendete sie nie: der gruene B10-Fall bekam
    sein ACK vom Testserver. Beide Haelften stehen jetzt - hier die Sendeseite.

    Das Objekt bindet an den BEFUND, an dem der Schritt haengt. Ohne
    `finding_id` entsteht keines: ein Urteil ohne Gegenstand waere ein Objekt
    ohne Bezug, und ein erfundener Bezug waere schlimmer als kein Urteil. */

std::string EqCopilotProcessor::v3UserVerdictJson (nakama::state::Userurteil urteil,
                                                   const juce::String& findingId,
                                                   const juce::String& notiz) const
{
    juce::String befund = findingId, proposalId;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (! zustand.assistent.gesetzt)
            return {};
        // Der GENANNTE Befund gewinnt: der User urteilt ueber den, den er
        // gerade sieht. Traegt der Schritt selbst einen, ist er der Rueckfall
        // - so bleibt der Weg auch dann geschlossen, wenn die Oberflaeche den
        // Befund nicht mitgibt.
        if (befund.isEmpty())
            befund = zustand.assistent.findingId;
        proposalId = zustand.assistent.proposalId;
    }
    if (! nakama::ipc::istHex32 (befund.toStdString()))
        return {};
    const auto& findingIdGewaehlt = befund;
    // 🔑 WN-01/KR-01: eine Wiederholung behaelt ihre `command_id`. Der Broker
    // erkennt sie idempotent (NR-10) - eine frische ID waere ein zweites
    // Urteil, kein zweiter Versuch. Diese Funktion erzeugt deshalb IMMER ein
    // neues Urteil; die Wiederholung geht seit E-15 gar nicht mehr durch sie
    // hindurch, sondern ersetzt im gesendeten Auftrag genau die abgelehnte
    // `base_revision` (`urteilMitFrischemKopf`). Bis zur Runde 3 stand hier
    // eine `commandIdVorgabe`, seither entfernt - der Neubau zog Befund und
    // Proposal dabei aus dem AKTUELLEN Schritt, also aus einem Zustand, der
    // sich seit dem Absenden geaendert haben kann.
    const juce::String commandId { juce::String (uuidHex32()) };
    const auto kopf = versuchKopfJson (commandId);
    if (kopf.empty())
        return {};
    // 🔑 SONDE-014 KR-01 (E-15, Konvergenzrunde 07.09.2026): hier stand ein
    // ZWEITES Register. Der Prozessor hielt je `command_id` einen Mitschnitt
    // - Urteil, Befund, Notiz - und deckelte ihn auf `kCapP0`, damit er nicht
    // waechst. Der Deckel war falsch begruendet: die 64 P0-Plaetze begrenzen
    // die QUEUE, nicht die Zahl der ausstehenden Auftraege. Nach 65 Urteilen
    // ohne ACK verdraengte er den aeltesten, obwohl dessen Auftrag im
    // In-Flight-Register des ControlClients weiterlief; ein spaeteres
    // `konflikt`-ACK fand dann keinen Text mehr und loeschte das
    // unpersistierte Urteil endgueltig (WP3-1, Bruch von M-73).
    //
    // Es gibt seither genau EIN Register: das des ControlClients. Der Text,
    // den diese Funktion zurueckgibt, IST der Wiederholungsinhalt - er reist
    // mit dem Auftrag und kommt bei einem `konflikt` unveraendert zurueck.
    std::string aus = "{\"type\":\"user_verdict\",\"kopf\":" + kopf;
    aus += ",\"user_verdict_id\":\"" + uuidHex32() + "\"";
    aus += ",\"finding_id\":\"" + findingIdGewaehlt.toStdString() + "\"";
    if (nakama::ipc::istHex32 (proposalId.toStdString()))
        aus += ",\"proposal_id\":\"" + proposalId.toStdString() + "\"";
    aus += ",\"urteil\":\"" + std::string (nakama::state::wort (urteil)) + "\"";
    // Abwesenheit heisst „keine Notiz", nie `null`. Die Schemalaenge ist 500;
    // ein laengerer Text reist gar nicht, statt beim Empfaenger zu fallen.
    if (notiz.isNotEmpty() && notiz.length() <= 500)
        aus += ",\"notiz\":" + juce::JSON::toString (juce::var (notiz), true).toStdString();
    aus += "}";
    return aus;
}

/*  SONDE-014 WN-01 (Nacharbeit 2) / KR-01 (E-15, Konvergenzrunde): derselbe
    Auftrag, frischer Kopf.

    Der Broker hat mit `konflikt` geantwortet und dabei die Revision genannt,
    die er kennt. Das Urteil selbst war richtig - nur sein Kopf trug eine
    Zahl, die der Broker noch nicht gesehen hatte.

    🔑 KR-01: der Wiederholungsinhalt kommt aus dem AUFTRAG, den der
    ControlClient ohnehin haelt. Bis zur Runde 3 baute diese Funktion den Text
    aus einem zweiten, gedeckelten Register des Prozessors neu; das Register
    verlor bei Saettigung den aeltesten offenen Auftrag (WP3-1). Ersetzt wird
    jetzt genau die eine Zahl, die der Broker abgelehnt hat - `user_verdict_id`,
    `finding_id`, `proposal_id`, Urteil und Notiz bleiben bytegleich.

    Die POLITIK bleibt hier: wiederholt wird ein `user_verdict`, weil ein
    Userurteil nicht verloren gehen darf (M-73/E-09). Ein Experimentbefehl
    unter derselben Schiene bekommt keinen frischen Kopf; er ist an sein
    Zeitfenster gebunden, und eine spaete Wiederholung waere ein zweiter
    Eingriff, kein zweiter Versuch.

    Laeuft unter `sendeMutex` des ControlClients (Ordnung: sendeMutex VOR
    Bindungsschloss, wie der Replay-Hook). Er liest und formt nur. */

std::string EqCopilotProcessor::urteilMitFrischemKopf (const juce::String& commandId,
                                                       const std::string& auftragJson,
                                                       std::uint64_t brokerRevision) const
{
    // Nichts erfinden: nur ein Auftrag, der wirklich unter DIESER Kennung
    // steht und wirklich ein `user_verdict` ist, wird wiederholt. Beides sind
    // Wachen gegen eine falsche Verdrahtung, keine zweite Zuordnung - die
    // leistet das In-Flight-Register, das den Eintrag ueber seine
    // `command_id` findet und genau dessen Text uebergibt.
    const auto kennung = commandId.toStdString();
    if (! nakama::ipc::istHex32 (kennung)
        || auftragJson.find ("\"type\":\"user_verdict\"") == std::string::npos
        || auftragJson.find ("\"command_id\":\"" + kennung + "\"") == std::string::npos)
        return {};
    return nakama::ipc::auftragMitBasisRevision (auftragJson, brokerRevision);
}

/*  M-86: die Vollstaendigkeitsmarke, BEVOR der Broker rechnet.

    Reihenfolge nach M-75/M-76: anmelden -> Verbunden-Callback abgeschlossen
    -> subscribe -> Intent-Vollbestand mit Marke -> Assistentenschritt -> erst
    dann rechnet der Broker. Ein Veto darf durch Transportverlust nie
    verschwinden; ein fehlender Intent saehe sonst aus wie „kein Schutz
    gewuenscht".

    Der Schluessel `intent:vollstaendig` ist ein EIGENER Koaleszierungsraum:
    ein Vollbericht darf einen aelteren Vollbericht verdraengen, aber nie eine
    Einzelfortschreibung unter `intent:<quelle>:<scope>` — die traegt ein
    anderes Objekt. */

bool EqCopilotProcessor::sendeIntentVollbestand()
{
    const auto json = v3IntentUpdateJson (true, nullptr, nullptr, nullptr);
    if (json.empty())
        return false;
    const auto ergebnis = controlV3.sendeP1 ("intent:vollstaendig", json);
    if (ergebnis != nakama::ipc::P1Ergebnis::eingereiht)
        return false;

    // Der Schritt reist im selben Zug. Fehlt er, ist das kein Fehler: ein
    // frisches Projekt hat keinen offenen Assistentenschritt, und ein
    // erfundener waere schlimmer als keiner.
    const auto schritt = v3AssistantStepJson();
    if (! schritt.empty())
        controlV3.sendeP1 ("assistant_step:" + v3Hello().adresse.sessionEpoch, schritt);
    return true;
}

void EqCopilotProcessor::sendeIntentFortschreibung (const juce::String& quelleId,
                                                    const juce::String& passageId)
{
    nakama::state::SourceIntent kopie;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        const auto* treffer = nakama::state::findeIntent (zustand, quelleId, passageId);
        if (treffer == nullptr)
            return;   // Ein Entfernen meldet der naechste Vollbestand; hier
                      // gibt es kein Objekt, das reisen koennte.
        kopie = *treffer;
    }
    const auto json = v3IntentUpdateJson (false, &kopie, nullptr, nullptr);
    if (json.empty())
        return;
    // M-85: der Schluessel ist je Quelle UND Scope eigen. Zwei Passagen
    // derselben Quelle sind zwei Objekte und duerfen sich nicht gegenseitig
    // verdraengen; genau deshalb steht der Scope im Schluessel.
    const auto schluessel = std::string ("intent:") + quelleId.toStdString() + ":"
                          + (passageId.isEmpty() ? std::string ("global")
                                                 : passageId.toStdString());
    controlV3.sendeP1 (schluessel, json);
}

void EqCopilotProcessor::v3ControlLink (bool verbunden)
{
    if (! verbunden)
    {
        // 🔑 NAK-180 R11/R13: loeschen NUR fuer die sterbende Generation, und
        // NUR per CAS. Positiver und negativer Callback koennen ueberlappen —
        // `meldeLinkStatus` serialisiert das Statusbit, nicht die
        // Callback-Ausfuehrung, und `reconnect()`/`stop()` rufen den negativen
        // synchron auf ihrem Aufruferthread (MP3-1). Ein blindes Loeschen
        // naehme dem naechsten Link seine gerade geschriebene Aussage mit.
        //
        // Das ist HYGIENE, nicht die Zusage: die Korrektheit traegt der
        // Generationsvergleich beim Verbraucher. Deshalb ist es auch
        // ungefaehrlich, dass hier gar kein zweiter Ende-Callback kommt, wenn
        // das Statusbit schon `false` war.
        // 🔑 NAK-180 Nacharbeit 1 (EP-05/N-37 Fall 2): die STERBENDE
        // Generation kommt vom ControlClient, nicht aus `wireGenerationJetzt()`.
        //
        // Bei `reconnect()` kann der Clientthread G beenden und G+1
        // vollstaendig aufbauen, bevor der externe Aufrufer seinen
        // verspaeteten `false`-Callback erreicht. Er las dann G+1 und loeschte
        // genau dessen Urteil, Replay und Bericht — die Wirkung des NEUEN
        // Links. Der Client hinterlegt die Zahl beim Statuswechsel, und
        // `meldeLinkStatus` serialisiert den per `exchange`.
        // 🔑 NAK-180 Nacharbeit 2 (WN-05): der Einhaengepunkt liegt VOR der
        // Ermittlung der sterbenden Generation - nicht erst vor den
        // Loeschungen.
        //
        // Genau darin steckt der Defekt, den N-37 Fall 2 messen soll: ein
        // Callback, der die Zahl bei AUSFUEHRUNG liest, findet nach einem
        // dazwischen aufgebauten G+1 dessen Zahl und loescht dessen Wirkung.
        // Ein Haken hinter dieser Zeile koennte das nicht mehr zeigen - die
        // Zahl staende dann laengst fest.
        if (linkEndeHakenFuerTest)
            linkEndeHakenFuerTest (controlV3.sterbendeGenerationJetzt());
        const auto sterbend = controlV3.sterbendeGenerationJetzt();
        if (sterbend == 0)
        {
            // Kein gemeldeter Link — es gibt nichts zu loeschen. `0` traefe
            // ohnehin keinen Zustand, der Fruehausstieg macht es sichtbar.
            sourcesModel.controlEnde();
            telemetryV3.reconnect();
            return;
        }
        controlV3.loescheAufbauUrteil (sterbend);
        auto a = sterbend;
        replayFaellig.compare_exchange_strong (a, 0);
        auto b = sterbend;
        berichtOffen.compare_exchange_strong (b, 0);
        sourcesModel.controlEnde();
        telemetryV3.reconnect();
        return;
    }

    // 🔑 NAK-180 N-11: der Klassentest steht VOR dem Neutralitaetsurteil.
    //
    // Er stand bisher dahinter, und damit gab auch eine als Sonde
    // klassifizierte Instanz eine Aussage ueber Interventionen ab — sie hat
    // aber gar keine Marker (§7.1 E-08). Ein ueberlaufbedingtes `true` aus
    // `v3Status()` reist unveraendert weiter; das ist keine Aussage ueber den
    // Aufbau, sondern fail-closed ueber den Ring.
    auto hVoraus = v3Hello();
    hVoraus.adresse = nakama::ipc::wireAdresseAusState (hVoraus.adresse);
    const bool darfAufbauUrteilMelden =
        hVoraus.pluginKind == "main" && nakama::ipc::adresseGueltig (hVoraus.adresse);

    // 🔑 SONDE-013 Nacharbeit 2 (Befund R01, M-61): DER Produktaufrufer von
    // `InterventionsRing::resync()`.
    //
    // Das Sticky-Bit des Ueberlaufs hatte bis hierher KEINEN Loeschpfad im
    // Produkt: `v3Status()` las es, `resync()` und `interventionsRingUeberlauf
    // .store(false)` rief niemand. Nach dem ersten Ueberlauf meldete jeder
    // Heartbeat dauerhaft `intervention_state_unknown`, und die Sperre auf
    // starker Evidenz fiel nie wieder — ein sticky Bit ohne Gegenpfad ist
    // dieselbe Sorte Fehler wie ein fehlendes.
    //
    // Der bestaetigte Neuaufbau des Control-Links IST der Resync (§34.2: eine
    // AUSDRUECKLICHE, bestaetigte Lebenszyklusoperation): die Bruecke ist neu,
    // der Broker fuehrt fuer diesen Link noch keine Sequenzbasis, und die
    // naechste Nummer wird darum vorbehaltlos angenommen.
    //
    // ⚠️ Er behauptet Neutralitaet nur, wenn sie WIRKLICH gilt: kein Ereignis
    // wartet mehr im Ring, und es ist gerade kein Marker hoerbar. Sonst bleibt
    // das Bit stehen — eine Selbstheilung waere genau das, was §34.2 verbietet.
    // 🔑 NAK-180 E1: Neutralitaet ist SENDEBUCHFUEHRUNG, nicht `hoerbar()`.
    //
    // `hoerbar()` allein genuegt nicht: `prepareToPlay` ruft `vorbereiten()`
    // und `setzeSamplerate()`, beide setzen `warHoerbar = false` und ueber
    // `hartAus()` `fade = 0`, OHNE `hoerbarAtomic` zu beruehren — das wird
    // erst im naechsten Audioblock nachgezogen, bei gestopptem Transport also
    // nie. Der dritte Term ist die einzige Aussage darueber, was der BROKER
    // gesehen hat. Jeder Term darf allein sperren, keiner allein entsperren
    // (§34.2: der teure Fehler liegt auf der Seite "zu frueh entsperrt").
    //
    // 🔑 NAK-180 Nacharbeit 2 (WN-01/N-36, N-05/M-58): ein `end`, das auf
    // seinen Wire-Commit wartet, ist der VIERTE Sperrgrund.
    //
    // `sendeBeginOffen` faellt schon beim EINREIHEN des `end`. Stirbt der Link
    // in diesem Fenster, sah der Broker weder Begin-Abschluss noch Tail,
    // waehrend das Praedikat hier bereits „neutral" las: der erste Heartbeat
    // von G+1 trug `false`, `resync_bestaetigen` loeschte das Unknown, und das
    // unmittelbar danach zugestellte `end` startete einen Tail, den der R1-
    // Resync gerade weggeraeumt hatte. Solange ein `end` unterwegs ist, weiss
    // nur das Plugin davon - und genau darum darf es keine Neutralitaet
    // behaupten.
    const bool neutral = interventionsRing.fuellstand() == 0
                      && ! sendeBeginOffen.load (std::memory_order_relaxed)
                      && ! abschlussOffen.load (std::memory_order_relaxed)
                      && ! markierung.hoerbar();
    if (neutral)
    {
        interventionsRing.resync();
        interventionsRingUeberlauf.store (false, std::memory_order_relaxed);
        // 🔑 Befund C1: der Zaehler faellt MIT auf null zurueck.
        //
        // Der Broker fuehrt fuer diesen Link ab hier die Basis 0. Liefe das
        // Plugin mit seiner alten, hohen Nummer weiter, waere der Abstand eine
        // Luecke — und der Resync haette den Zustand nicht geklaert, sondern
        // gerade erst wieder unbekannt gemacht.
        interventionsSequenz.store (0, std::memory_order_relaxed);
    }

    // 🔑 NAK-180 R1/R2: die Aussage des Aufbaus reist — als Generationszahl.
    //
    // Neutral: der erste Heartbeat dieses Links traegt
    // `intervention_state_unknown: false`, und DAS ist der einzige Ausloeser
    // von `resync_bestaetigen` im Broker (D-01).
    //
    // Nicht neutral: er traegt ausdrueckliches `true`. Ein FRISCHER Broker
    // haette sonst kein `unknown` — sein `Stand` ist leer, `taint` wird nicht
    // persistiert —, und die Sitzung waere von der ersten Sekunde an
    // faelschlich sauber, waehrend der Marker klingt (C2). Danach stellt das
    // Replay den wahren Zustand her, und der Bericht bleibt offen, bis der
    // Prozessor wieder neutral ist (E3.3).
    //
    // ⚠️ Der Klassentest umschliesst NUR die Wireaussage, nicht den lokalen
    // Resync darueber: der Ring, sein Sticky-Bit und der Sequenzzaehler sind
    // Prozessorzustand und gehoeren jeder Klasse. Nur die BEHAUPTUNG ueber
    // Interventionen darf eine Sonde nicht abgeben — sie hat gar keine
    // Marker (§7.1 E-08).
    if (! darfAufbauUrteilMelden)
    {
        telemetryV3.reconnect();
        return;
    }

    const auto meineGeneration = controlV3.meldeAufbauUrteil (neutral);
    // 🔑 NAK-180 Nacharbeit 2 (WN-04): der Einhaengepunkt liegt VOR dem CAS,
    // und die Generation ist hier bereits vergeben (R10). Ein Bein haelt den
    // ECHTEN Callback genau hier fest, laesst G+1 vollstaendig aufbauen und
    // gibt ihn dann frei; gemessen wird, ob die Wirkung von G+1 stehen bleibt.
    // Hinter dem Callback gehaengt maesse derselbe Test nichts: der
    // Schreibzugriff waere laengst gelaufen.
    if (linkAufbauHakenFuerTest)
        linkAufbauHakenFuerTest (meineGeneration);
    if (! neutral && meineGeneration != 0)
    {
        // 🔑 NAK-180 Nacharbeit 1 (EP-04/R13/N-37): CAS statt `store`.
        //
        // Ein verspaeteter positiver Callback von G konnte nach dem Aufbau von
        // G+1 diese beiden Zustaende blind mit G ueberschreiben. Der Worker
        // raeumte den fremden Wert danach auf und verlor damit Bericht und
        // Replay von G+1 — das Lost-Update, das R13 gerade verbietet. Gesetzt
        // wird nur, solange kein NEUERER Link den Platz belegt hat.
        auto setzeGeneration = [] (std::atomic<std::uint64_t>& ziel, std::uint64_t g)
        {
            auto gesehen = ziel.load();
            while (gesehen < g)
                if (ziel.compare_exchange_weak (gesehen, g))
                    return;
        };
        setzeGeneration (berichtOffen, meineGeneration);
        setzeGeneration (replayFaellig, meineGeneration);
        // Den Worker wecken, damit das Replay nicht auf den 50-ms-Takt
        // wartet. Eine veraltete Weckung ist harmlos: der Zug prueft
        // `replayFaellig` gegen die laufende Generation.
        std::lock_guard<std::mutex> l (workerWarteMutex);
        workerWarte.notify_all();
    }

    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
        return;
    sourcesModel.beginneSubscription (h.adresse.projectBindingId,
                                      h.adresse.sessionEpoch,
                                      h.adresse.instanceId);
    const auto subscribe = v3SubscribeJson();
    const auto ergebnis = subscribe.empty()
        ? nakama::ipc::P1Ergebnis::abgewiesen
        : controlV3.sendeP1 ("subscribe_session", subscribe);
    if (ergebnis == nakama::ipc::P1Ergebnis::abgewiesen
        || ergebnis == nakama::ipc::P1Ergebnis::zuGross)
    {
        sourcesModel.controlEnde();
        controlV3.reconnect();
        return;
    }
    // 🔑 SONDE-014 M-86: die VOLLSTAENDIGKEITSMARKE, und zwar HIER - nach dem
    // subscribe und vor jeder Rechnung des Brokers.
    //
    // Die Reihenfolge ist die Zusage (M-75/M-76): anmelden, Verbunden-Callback
    // abgeschlossen, subscribe, Intent-Vollbestand mit Marke, Schritt - erst
    // dann rechnet der Broker. Ein Veto darf durch Transportverlust nie
    // verschwinden; ein fehlender Intent saehe sonst aus wie „kein Schutz
    // gewuenscht", und das ist der teuerste Irrtum, den dieser Datenweg
    // machen kann. Auch ein LEERER Bestand wird gemeldet - „diese Sitzung hat
    // keinen Intent" ist eine Aussage, „ich habe nichts gehoert" ist keine.
    sendeIntentVollbestand();
    telemetryV3.reconnect();
}

void EqCopilotProcessor::v3Antwort (const std::string& json,
                                    std::uint8_t schemaMinor)
{
    nakama::ipc::GelesenesCommandAck ack;
    if (nakama::ipc::commandAckHaeltVertrag (json, ack))
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        const auto it = ausstehendeSourcesCommands.find (ack.commandId);
        if (it != ausstehendeSourcesCommands.end())
        {
            if (ack.erfolgreich)
                bestaetigteSourcesCommands.push_back (it->second);
            ausstehendeSourcesCommands.erase (it);
        }
        return;
    }
    juce::String fehler;
    // SONDE-013 Nacharbeit 2 (Befund R28): der LESER der Evidenzruecknahme.
    //
    // Bis hierher kannte diese Stelle genau zwei Nachrichten - Command-ACK und
    // Sessionsnapshot. Der Broker legte fuer `evidence_invalidate` eine
    // Outbox-Schuld an, die niemand abtrug: ein aktiver Subscriber erhielt die
    // Ruecknahme nie, und Gen zeigte weiter Zahlen, deren Grundlage
    // zurueckgezogen war. Eine Zustellschuld ohne Leser ist ein Defekt.
    const auto ruecknahme = sourcesModel.uebernehmeEvidenzruecknahme (
        json, schemaMinor, fehler);
    if (ruecknahme != SourcesModel::RuecknahmeErgebnis::ignoriert)
    {
        if (ruecknahme == SourcesModel::RuecknahmeErgebnis::ungueltig)
            sourcesModel.setzeDiagnoseFuerSichtbeweis (
                SourcesModel::Diagnose::incompatible, false);
        return;
    }
    const auto ergebnis = sourcesModel.uebernehmeSessionSnapshot (
        json, schemaMinor, SourcesModel::Uhr::now(), fehler);
    if (ergebnis == SourcesModel::SnapshotErgebnis::ungueltig)
        sourcesModel.setzeDiagnoseFuerSichtbeweis (
            SourcesModel::Diagnose::incompatible, false);
}

void EqCopilotProcessor::v3Frame (const std::uint8_t* daten, std::size_t laenge,
                                  std::uint8_t schemaMinor)
{
    juce::String fehler;
    sourcesModel.uebernehmeP2 (daten, laenge, schemaMinor,
                               SourcesModel::Uhr::now(), fehler);
}

void EqCopilotProcessor::sourcesTick()
{
    wendeBestaetigteSourcesCommandsAn();
#if ! defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    sourcesModel.tick (SourcesModel::Uhr::now());
    sourcesModel.setzeControlTransport (controlV3.snapshot());
#endif
}

void EqCopilotProcessor::reconnectSources()
{
    controlV3.reconnect();
}

StatsSnapshot EqCopilotProcessor::statsSnapshot() const
{
    StatsSnapshot s;
    s.rmsL = rmsL.load();
    s.rmsR = rmsR.load();
    // SONDE-008: EINE Wahrheit. Der Zähler lebt in der Queue, die den Verlust
    // verursacht; ein zweiter Atomic daneben könnte nur auseinanderlaufen.
    // Einheit unverändert: verlorene Analyse-FRAMES.
    s.framesDropped = queue.verloreneFrames();
    s.nanSeen = nanSeen.load();
    // NAK-180 R4: DER Produktleser des Wet-Riegels. Ein gesetzter Zaehler
    // ohne Leser ist dasselbe wie kein Zaehler (Pruefliste A).
    s.wetNichtEndlich = markierung.nichtEndlicheWetSamples();
    s.hasTransport = hatTransport.load();
    s.transportPlaying = transportSpielt.load();
    s.projectTimeValid = projektZeitGueltig.load();
    s.projectTimeSamples = projektZeitSamples.load();
    return s;
}

// Kompakter Messstand für den v2-Heartbeat (Vertrag: measurement in
// eq-ipc.schema.json). Läuft 1×/s im Pipe-Thread; engine.snapshot() ist die
// threadsichere Kopie. LTAS wird auf 0,1 dB gerundet — die volle Auflösung
// bleibt der lokalen Snapshot-Datei vorbehalten.

MessKompakt EqCopilotProcessor::messKompakt() const
{
    const auto m = engine.snapshot();
    MessKompakt k;
    k.vorhanden = true;   // auch »keine_daten« ist Information für die Übersicht
    k.zustand = m.zustand == MessZustand::messbereit ? "messbereit"
              : m.zustand == MessZustand::sammelt    ? "sammelt"
                                                     : "keine_daten";
    k.metricsVersion = kMetricsVersion;
    k.aktivS  = m.aktivSekunden;
    k.gesamtS = m.gesamtSekunden;
    k.lufsGueltig = m.lufsGueltig;           k.lufsI = m.lufsIntegriert;
    k.lufsShortGueltig = m.lufsShortGueltig; k.lufsS = m.lufsShort;
    k.truePeakGueltig = m.zustand != MessZustand::keineDaten && m.truePeakDb > -199.0;
    k.truePeakDb = m.truePeakDb;
    k.crestGueltig = m.crestGueltig;         k.crestDb = m.crestDb;
    k.spektralGueltig = m.spektralGueltig;
    k.centroidHz = m.centroidMagHz;          k.lowFrac = m.lowFrac;
    k.stereoGueltig = m.stereoGueltig;
    k.width = m.width;                       k.corr = m.corr;
    if (m.ltasGueltig)
    {
        k.ltasKompositDb.resize ((size_t) kLtasBaender);
        for (int b = 0; b < kLtasBaender; ++b)
        {
            const double v = m.ltasKompositDb[(size_t) b];
            k.ltasKompositDb[(size_t) b] = std::isfinite (v)
                ? std::round (v * 10.0) / 10.0
                : std::numeric_limits<double>::quiet_NaN();   // ⇒ null im JSON
        }
    }
    k.fensterSpruenge = fensterSpruenge.load();
    if (fensterAktiv.load())
    {
        k.fensterGueltig  = true;
        k.fensterVon      = fensterVon.load();
        k.fensterBis      = fensterBis.load();
    }
    // Hör-Markierung aktiv ⇒ nachgelagerte Sensoren hören gefärbtes Signal —
    // der Broker markiert den Messstand und pausiert fremde Aggregate
    // (Konzept v2 §5; Broker-Seite folgt nach dem Harness-Lauf).
    k.hoermarkierung = markierung.hoerbar();
    return k;
}

// Sichtbare Antwort auf einen Kennungs-Konflikt (Plan §8.4): DIESE Instanz
// bekommt eine frische persistente ID und meldet sich neu an. Der Host
// speichert sie mit dem nächsten Projekt-Save (getStateInformation) — dafür
// MUSS er die Änderung kennen: Host-Dirty (Vertrag nakama-state-v2.md §6).

bool EqCopilotProcessor::bindeSourcesHauptziel (const std::string& erwarteteInstanceId)
{
    return sendeSourcesCommand (SourcesCommandArt::confirmJoin, erwarteteInstanceId);
}

bool EqCopilotProcessor::benenneSourcesHauptziel (const std::string& erwarteteInstanceId,
                                                  const juce::String& label)
{
    if (label.length() > 120 || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId)
        || ! sourcesModel.sicht().mainDarfSchreiben)
        return false;
    std::vector<nakama::state::MainProjectMitglied> kopie;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main
            || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId))
            return false;
        const auto gefunden = std::find_if (
            zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
            [&] (const auto& m) { return m.instanceId.toStdString() == erwarteteInstanceId; });
        if (gefunden == zustand.mainProjectMitglieder.end() || gefunden->label == label)
            return false;
        gefunden->label = label;
        kopie = zustand.mainProjectMitglieder;
    }
    sourcesModel.setzePersistenteMitglieder (kopie);
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    return true;
}

bool EqCopilotProcessor::entferneSourcesHauptziel (const std::string& erwarteteInstanceId)
{
    const auto sicht = sourcesModel.sicht();
    if (! sicht.mainDarfSchreiben
        || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId)
        || ! nakama::ipc::istHex32 (erwarteteInstanceId))
        return false;
    const auto quelle = std::find_if (sicht.quellen.begin(), sicht.quellen.end(),
        [&] (const auto& q) { return q.instanceId == erwarteteInstanceId && q.hauptziel; });
    if (quelle == sicht.quellen.end()
        || quelle->mitgliedschaft != SourcesModel::Mitgliedschaft::bestaetigt)
        return false;
    if (! nakama::ipc::istHex32 (quelle->runtimeNonce))
    {
        std::vector<nakama::state::MainProjectMitglied> kopie;
        {
            std::lock_guard<std::mutex> l (bindungMutex);
            if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
                return false;
            const auto gefunden = std::find_if (
                zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
                [&] (const auto& m) {
                    return m.instanceId.toStdString() == erwarteteInstanceId;
                });
            if (gefunden == zustand.mainProjectMitglieder.end())
                return false;
            zustand.mainProjectMitglieder.erase (gefunden);
            kopie = zustand.mainProjectMitglieder;
        }
        sourcesModel.setzePersistenteMitglieder (kopie);
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        return true;
    }
    return sendeSourcesCommand (SourcesCommandArt::unbindProbe, erwarteteInstanceId);
}

bool EqCopilotProcessor::sendeSourcesCommand (SourcesCommandArt art,
                                               const std::string& erwarteteInstanceId)
{
    const auto sicht = sourcesModel.sicht();
    if (! sicht.mainDarfSchreiben || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId)
        || ! nakama::ipc::istHex32 (erwarteteInstanceId))
        return false;
    const auto quelle = std::find_if (sicht.quellen.begin(), sicht.quellen.end(),
        [&] (const auto& q) { return q.instanceId == erwarteteInstanceId && q.hauptziel; });
    if (quelle == sicht.quellen.end() || ! nakama::ipc::istHex32 (quelle->runtimeNonce))
        return false; // Ohne aktuellen Brokeradressaten kein behaupteter Wire-Unbind.

    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        const auto hat = std::any_of (
            zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
            [&] (const auto& m) { return m.instanceId.toStdString() == erwarteteInstanceId; });
        if ((art == SourcesCommandArt::confirmJoin && (hat
                || zustand.mainProjectMitglieder.size()
                    >= static_cast<std::size_t> (nakama::state::maxMainProjectMitglieder)))
            || (art == SourcesCommandArt::unbindProbe && ! hat))
            return false;
    }

    auto h = v3Hello();
    h.adresse.instanceId = erwarteteInstanceId;
    h.adresse.runtimeNonce = quelle->runtimeNonce;
    if (! nakama::ipc::adresseGueltig (h.adresse))
        return false;
    SourcesCommand auftrag;
    auftrag.art = art;
    auftrag.commandId = uuidHex32();
    auftrag.instanceId = erwarteteInstanceId;
    auftrag.projectBindingId = h.adresse.projectBindingId;
    auftrag.sessionEpoch = h.adresse.sessionEpoch;
    if (art == SourcesCommandArt::confirmJoin)
        auftrag.label = quelle->userLabel.substring (0, 120);
    const char* command = art == SourcesCommandArt::confirmJoin
                            ? "confirm_join" : "unbind_probe";
    auftrag.json = std::string ("{\"type\":\"session_command\",\"command\":\"")
                 + command + "\",\"command_id\":\"" + auftrag.commandId
                 + "\",\"ziel\":" + nakama::ipc::adresseAlsJson (h.adresse)
                 + ",\"session_epoch\":\"" + h.adresse.sessionEpoch + "\"}";
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        if (std::any_of (ausstehendeSourcesCommands.begin(),
                         ausstehendeSourcesCommands.end(),
                         [&] (const auto& paar) {
                             return paar.second.instanceId == erwarteteInstanceId;
                         }))
            return false;
        ausstehendeSourcesCommands.emplace (auftrag.commandId, auftrag);
    }
    // 🔑 NAK-246 D4 (R-D4, M-16, M-20; Paragraph 5.4 Feinheit 4): die
    // Zuordnung bleibt bei BEIDEN angenommenen Zustaenden. Bis hierher loeschte
    // ein `false` sie auch dann, wenn der Client den Auftrag hielt und nach
    // dem Reconnect nachspielte - der ACK des angewandten Befehls fand die
    // Zuordnung nicht mehr, und die brokerseitig angewandte Mitgliedschaft
    // wurde lokal nie nachgefuehrt (Auditbefund D4). Geloescht wird nur bei
    // `endgueltigAbgewiesen`: dann kommt nie ein ACK.
    if (nakama::ipc::persistenzAngenommen (controlV3.sendePersistenzP0 (auftrag.json)))
        return true;
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    ausstehendeSourcesCommands.erase (auftrag.commandId);
    return false;
}

// ── NAK-246 D3 (R-D3): der Persistenzabschluss mit editorunabhaengigem Besitzer ──
//
// Bis hierher war der Editor-Timer (`PluginEditor.cpp`, `sourcesTick`) der
// EINZIGE Drain der bestaetigten Befehle: ohne offenen Editor wurde ein vom
// Broker bestaetigter Join oder Unbind nie in `zustand.mainProjectMitglieder`
// uebernommen, `getStateInformation` serialisierte den alten Stand, und
// `setStateInformation` leerte die Warteliste (Auditbefund D3; Entwurf
// Paragraph 57 "Save/Reload ... erhalten richtige Mitgliedschaft"; CLAUDE.md
// "State bleibt verlustfrei ... Jede persistente Aenderung meldet dem Host
// Dirty-State").
//
// Seit dieser Etappe gibt es DREI Drains auf EINE Funktion:
//   (a) das Speichern (`State.cpp`, `getStateInformation`): spaetestens dort,
//       unter `bindungMutex` und VOR `speichere`;
//   (b) der Analyse-Workerzug (`Analyse.cpp`, `workerLauf`): regelmaessig,
//       spaetestens alle 50 ms, ohne Editor;
//   (c) der Editor-Tick (`sourcesTick`): wie bisher, zusaetzlich.
// Alle drei serialisieren sich ueber `sourcesDrainMutex` - die einzige
// Klammer ueber Swap UND Anwendung (Manifest Paragraph 5.3 Feinheit 3, M-12).
// `bindungMutex` deckt je Befehl nur die Mutation; Modell, Host-Dirty und
// Revision folgen je geaendertem Befehl genau einmal, NACH der Freigabe.

std::vector<EqCopilotProcessor::SourcesCommand>
EqCopilotProcessor::bestaetigteSourcesCommandsAbholen()
{
    std::vector<SourcesCommand> befehle;
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    befehle.swap (bestaetigteSourcesCommands);
    return befehle;
}

bool EqCopilotProcessor::wendeSourcesCommandAnUnterBindung (const SourcesCommand& befehl)
{
    // Aufrufer haelt `bindungMutex`.
    if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main
        || zustand.common.projectBindingId.toStdString() != befehl.projectBindingId
        || v3SessionEpoch != befehl.sessionEpoch)
        return false; // ACK eines vor Reload gueltigen Laufs mutiert den neuen State nie.
    auto gefunden = std::find_if (
        zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
        [&] (const auto& m) { return m.instanceId.toStdString() == befehl.instanceId; });
    if (befehl.art == SourcesCommandArt::confirmJoin
        && gefunden == zustand.mainProjectMitglieder.end()
        && zustand.mainProjectMitglieder.size()
            < static_cast<std::size_t> (nakama::state::maxMainProjectMitglieder))
    {
        zustand.mainProjectMitglieder.push_back (
            { juce::String (befehl.instanceId), befehl.label });
        return true;
    }
    if (befehl.art == SourcesCommandArt::unbindProbe
        && gefunden != zustand.mainProjectMitglieder.end())
    {
        zustand.mainProjectMitglieder.erase (gefunden);
        return true;
    }
    return false;
}

void EqCopilotProcessor::meldeSourcesMitgliederNachBefehl()
{
    // Der AKTUELLE Stand, nicht eine beim Anwenden gezogene Kopie: zwei
    // Drains, die ihre Nachfuehrung nacheinander fahren, reichen dem Modell
    // so nie einen aelteren Stand als den zuletzt angewandten nach.
    std::vector<nakama::state::MainProjectMitglied> kopie;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        kopie = zustand.mainProjectMitglieder;
    }
    sourcesModel.setzePersistenteMitglieder (kopie);
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
}

void EqCopilotProcessor::wendeBestaetigteSourcesCommandsAn()
{
    std::size_t geaendert = 0;
    {
        std::lock_guard<std::mutex> drain (sourcesDrainMutex);
        const auto befehle = bestaetigteSourcesCommandsAbholen();
        if (befehle.empty())
            return;
        // NAK-246 D3 (M-12): der Testhaken zwischen Swap und Anwendung,
        // innerhalb der Klammer. Im Produkt leer.
        if (sourcesDrainHakenFuerTest)
            sourcesDrainHakenFuerTest (befehle.size());
        for (const auto& befehl : befehle)
        {
            std::lock_guard<std::mutex> l (bindungMutex);
            if (wendeSourcesCommandAnUnterBindung (befehl))
                ++geaendert;
        }
    }
    for (std::size_t i = 0; i < geaendert; ++i)
        meldeSourcesMitgliederNachBefehl();
}

#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)

std::string EqCopilotProcessor::ausstehenderSourcesCommandFuerTest() const
{
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    return ausstehendeSourcesCommands.empty()
             ? std::string() : ausstehendeSourcesCommands.begin()->second.json;
}

std::size_t EqCopilotProcessor::bestaetigteSourcesCommandsFuerTest() const
{
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    return bestaetigteSourcesCommands.size();
}

std::string EqCopilotProcessor::merkeSourcesCommandFuerTest (SourcesCommandArt art,
                                                             const std::string& instanceId,
                                                             const std::string& projectBindingId,
                                                             const std::string& sessionEpoch)
{
    SourcesCommand auftrag;
    auftrag.art = art;
    auftrag.commandId = uuidHex32();
    auftrag.instanceId = instanceId;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        auftrag.projectBindingId = projectBindingId.empty()
                                     ? zustand.common.projectBindingId.toStdString()
                                     : projectBindingId;
    }
    auftrag.sessionEpoch = sessionEpoch.empty() ? v3SessionEpoch : sessionEpoch;
    auftrag.label = "Eingeschleust";
    auftrag.json = std::string ("{\"type\":\"session_command\",\"command\":\"")
                 + (art == SourcesCommandArt::confirmJoin ? "confirm_join" : "unbind_probe")
                 + "\",\"command_id\":\"" + auftrag.commandId + "\"}";
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    ausstehendeSourcesCommands.emplace (auftrag.commandId, auftrag);
    return auftrag.commandId;
}
#endif

// ── Lokaler Mess-Snapshot als Datei (M1 §11: "lokale Snapshot-Erfassung") ──
// Kein Roh-Audio, keine Historie im Plugin-State — nur der Messstand.
// NaN/±inf werden als JSON-null geschrieben (juce::JSON kennt kein NaN).

} // namespace eqcop
