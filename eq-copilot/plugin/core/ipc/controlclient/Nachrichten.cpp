// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

// ControlClient — Nachrichten: senden, zustellen, quittieren.
//
// NAK-225/S25d (09.09.2026), herausgeloest aus core/ipc/ControlClient.cpp.
// Inhalt:
//
//   ControlClient::sendeP0 / sendePersistenzP0 / sendeP1
//   Laufzeit::sendeP0 / sendePersistenzP0 / sendeP1
//                        Die drei Prioritaetsklassen auf dem Draht. Die Politik
//                        bei vollem Puffer steht ausdruecklich im Rumpf, und
//                        die Rueckgabewerte werden ausgewertet, nicht ignoriert.
//   Laufzeit::inFlightNachWireWrite / inFlightAck / inFlightNachReconnect
//                        Der Weg eines Auftrags von "geschrieben" ueber
//                        "quittiert" bis "nach Reconnect erneut faellig".
//   ControlClient::interventionsZug
//                        Der Zug der Interventionen aus dem Ring.
//   gemeldeteStateRevision, Laufzeit::snapshotIntern
//                        Was der Aufrufer ueber den Stand erfaehrt.
//   fuelleP0QueueFuerTest, leereP0QueueFuerTest, zustelleAllesFuerTest
//                        Die Griffe, mit denen das Lastbein die Politik misst.
//
// Pruefliste A gilt hier woertlich: je Klasse steht die Politik bei *voll*
// ausdruecklich (abweisen, koaleszieren, aeltesten ersetzen) - nie
// stillschweigend Akzeptiertes loeschen. Ein Puffer, der spaeter wiederholt,
// hat einen Abflussweg OHNE Reconnect.

#include "ControlClient.h"
#include "NakamaKanon.h"
#include "WireEnvelope.h"
#include "controlclient/Intern.h"
#include "controlclient/Laufzeit.h"
#include <algorithm>

namespace nakama::ipc
{

// Die Helfer des internen Kopfes; die Aufrufstellen bleiben damit
// woertlich wie im Urstand.
using namespace controlclient_intern;

bool ControlClient::sendeP0 (const std::string& json, P0Klasse klasse, std::uint64_t marke)
{
    return k->sendeP0 (json, klasse, marke);
}

std::uint64_t ControlClient::gemeldeteStateRevision() const noexcept
{
    return k->stateReportRevisionGemeldet.load();
}

void ControlClient::interventionsZug (
    const std::function<void (std::uint64_t, const ZugSenke&)>& zug)
{
    if (! zug)
        return;
    // 🔑 NAK-180 R12: Vergleich und Wirkung unter DERSELBEN Sperre.
    //
    // Der Aufbauzug 2+3 haelt `sendeMutex` ebenfalls. Zwischen dem Lesen der
    // Generation hier und dem Einreihen der Folgenachricht kann sie deshalb
    // nicht wechseln - das Fenster, durch das ein `end` ohne sein Begin auf
    // den naechsten Link geriet (MP4-1), existiert nicht mehr.
    std::lock_guard<std::mutex> l (k->sendeMutex);
    const auto generation = k->wireGeneration.load();
    bool ueberlauf = false;
    // 🔑 Nacharbeit 1 (EP-07): DIE Senke vergibt die Marke, aus dem EINEN
    // Markenraum `p0MarkenFolge`. Der Einreicher erfaehrt sie als Rueckgabe;
    // `0` heisst abgewiesen. Zwei Folgen mit eigenem Nullpunkt liessen einen
    // Aufbau-Heartbeat und ein Marker-Begin dieselbe Zahl tragen, und der
    // Zustellrueckruf des einen buchte den anderen als zugestellt.
    const ZugSenke senke = [this, generation, &ueberlauf]
        (const std::string& json, P0Klasse klasse) -> std::uint64_t
    {
        if (json.size() > kMaxPayloadBytes)
        {
            std::lock_guard<std::mutex> z (k->zustandMutex);
            ++k->zustand.zuGross;
            return 0;
        }
        const auto marke = k->p0MarkenFolge.fetch_add (1) + 1;
        if (! k->p0.einreihen (P0Eintrag { json, klasse, generation, marke }))
        {
            ueberlauf = true;
            k->p0UeberlaufZaehler.fetch_add (1);
            std::lock_guard<std::mutex> z (k->zustandMutex);
            k->zustand.p0Ueberlaeufe = k->p0.ueberlauf();
            return 0;
        }
        return marke;
    };
    zug (generation, senke);
    if (ueberlauf)
    {
        // Wie in `sendeP0`: der Verbindungsthread haengt womoeglich in einem
        // blockierten Write. Der Abbruch steht AUSSERHALB der Sperre nicht
        // zur Verfuegung, ohne den Zug zu zerreissen - er folgt deshalb hier,
        // und `ioAbbrechen` nimmt `sendeMutex` nicht.
        k->aktuelleVerbindung()->ioAbbrechen();
    }
}

bool ControlClient::sendePersistenzP0 (const std::string& json)
{
    return k->sendePersistenzP0 (json);
}

P1Ergebnis ControlClient::sendeP1 (const std::string& schluessel, const std::string& json)
{
    return k->sendeP1 (schluessel, json);
}

bool ControlClient::Laufzeit::sendeP0 (const std::string& json,
                                       P0Klasse klasse, std::uint64_t marke)
{
    // An der TUER, nicht am Draht. Eine eingereihte Nachricht ueber der
    // Paketgrenze koennte NIE gesendet werden — sie bliebe dank der
    // Reservierung fuer immer vorn in der Queue und liesse jede neue
    // Verbindung an derselben Stelle scheitern.
    if (json.size() > kMaxPayloadBytes)
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        ++zustand.zuGross;
        return false;
    }

    bool ueberlauf = false;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        // NAK-180 R10: die Generation wird HIER gelesen, beim Einreihen -
        // nicht beim Bilden der Nachricht. Ein frueh gebauter, spaet
        // eingereihter Bericht truege sonst eine ueberholte Zahl, und das
        // Vorziehen der Generationsvergabe waere wirkungslos.
        if (! p0.einreihen (P0Eintrag { json, klasse, wireGeneration.load(), marke }))
        {
            ueberlauf = true;
            p0UeberlaufZaehler.fetch_add (1);
            std::lock_guard<std::mutex> z (zustandMutex);
            zustand.p0Ueberlaeufe = p0.ueberlauf();
        }
    }
    if (ueberlauf)
    {
        // Der Verbindungsthread haengt womoeglich in einem blockierten Write
        // (genau deshalb ist die Queue ja voll). Ohne diesen Abbruch merkte er
        // den Ueberlauf erst nach seiner Frist — und "Verbindung schliessen"
        // waere eine Zusage mit fuenf Sekunden Verspaetung.
        aktuelleVerbindung()->ioAbbrechen();
        return false;
    }
    return true;
}

bool ControlClient::Laufzeit::sendePersistenzP0 (const std::string& json)
{
    std::string commandId;
    if (json.size() > kMaxPayloadBytes || ! commandIdAusAuftrag (json, commandId))
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        if (json.size() > kMaxPayloadBytes)
            ++zustand.zuGross;
        else
            zustand.letzterFehler =
                "Persistenzauftrag braucht genau eine gueltige command_id";
        return false;
    }

    bool ueberlauf = false;
    std::uint64_t verworfeneMarke = 0;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        const auto bekannt = std::find_if (inFlight.begin(), inFlight.end(),
            [&] (const InFlightEintrag& e) { return e.commandId == commandId; });
        if (bekannt != inFlight.end())
        {
            // Derselbe logische Auftrag darf vom Aufrufer erneut angeboten
            // werden, aber dieselbe ID darf nie zwei verschiedene Inhalte
            // bedeuten. Die bereits gehaltene Fassung bleibt die Wahrheit.
            return bekannt->json == json;
        }
        inFlight.push_back (InFlightEintrag { commandId, json, 0, true });
        // Persistenzpflichtige Befehle sind EREIGNISSE: sie ueberleben jeden
        // Linkwechsel und werden nie verworfen (§53.9).
        //
        // 🔑 SONDE-014 WN-05 (Nacharbeit 2, 07.09.2026), M-73 woertlich: der
        // abgewiesene Auftrag wird NICHT stillschweigend geloescht.
        //
        // Bis hierher nahm der Ueberlauf den Eintrag per `pop_back` wieder aus
        // `inFlight` - der Auftrag war fort, `beiP0Verworfen` erfuhr nichts
        // davon, und ein Aufrufer, der den Rueckgabewert ignorierte, meldete
        // Erfolg (WP1-5). M-73 sagt fuer P0 aber woertlich: "Ueberlauf =>
        // Verbindung wird verworfen und der Eintrag geht an `beiP0Verworfen` -
        // nie stillschweigend geloescht."
        //
        // Der Eintrag BLEIBT deshalb im Register, nur nicht mehr in der
        // Queue: `inFlightNachReconnect` reiht ihn nach dem naechsten
        // Verbindungsaufbau unter DERSELBEN `command_id` erneut ein
        // (idempotent, NR-10). Die Marke kommt aus dem gemeinsamen
        // Markenraum, damit die Verwurfmeldung eine echte Kennung traegt.
        const auto marke = p0MarkenFolge.fetch_add (1) + 1;
        if (! p0.einreihen (P0Eintrag { json, P0Klasse::ereignis, wireGeneration.load(), marke }))
        {
            inFlight.back().inQueue = false;
            verworfeneMarke = marke;
            ueberlauf = true;
            p0UeberlaufZaehler.fetch_add (1);
        }

        std::lock_guard<std::mutex> z (zustandMutex);
        zustand.p0Ueberlaeufe = p0.ueberlauf();
        zustand.inFlight = static_cast<std::uint64_t> (inFlight.size());
    }
    if (ueberlauf)
    {
        // Erst die Meldung, dann der Abbruch: der Aufrufer soll erfahren, DASS
        // sein Auftrag den Draht nicht gesehen hat, bevor die Verbindung
        // faellt. Beides ausserhalb von `sendeMutex` - ein Rueckruf darf den
        // Sendezustand des Prozessors nehmen (Ordnung: sendeMutex VOR
        // Sendezustand), und diese Reihenfolge haelt sie ein.
        if (beiP0Verworfen && verworfeneMarke != 0)
            beiP0Verworfen (verworfeneMarke);
        aktuelleVerbindung()->ioAbbrechen();
        return false;
    }
    return true;
}

void ControlClient::Laufzeit::inFlightNachReconnect (std::uint64_t generation)
{
    std::uint64_t wiederholt = 0;
    std::uint64_t anzahl = 0;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        for (auto& e : inFlight)
        {
            if (e.inQueue || e.gesendetInGeneration == generation)
                continue;
            if (! p0.einreihen (P0Eintrag { e.json, P0Klasse::ereignis,
                                            wireGeneration.load(), 0 }))
                break;
            e.inQueue = true;
            ++wiederholt;
        }
        anzahl = static_cast<std::uint64_t> (inFlight.size());
    }
    if (wiederholt != 0)
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        zustand.inFlight = anzahl;
        zustand.inFlightWiederholungen += wiederholt;
    }
}

void ControlClient::Laufzeit::inFlightNachWireWrite (const std::string& json,
                                                      std::uint64_t generation)
{
    std::lock_guard<std::mutex> l (sendeMutex);
    const auto eintrag = std::find_if (inFlight.begin(), inFlight.end(),
        [&] (const InFlightEintrag& e) { return e.json == json && e.inQueue; });
    if (eintrag != inFlight.end())
    {
        eintrag->inQueue = false;
        eintrag->gesendetInGeneration = generation;
    }
}

void ControlClient::Laufzeit::inFlightAck (const std::string& json)
{
    std::string commandId;
    std::uint64_t brokerRevision = 0;
    const auto art = commandAckArtLesen (json, commandId, &brokerRevision);
    if (art == CommandAckArt::keinAck)
        return;

    bool gefunden = false;
    bool wiederholt = false;
    std::uint64_t anzahl = 0;
    std::uint64_t verworfeneMarke = 0;
    // Der Fuellstand wird UNTER `sendeMutex` gelesen: `zustandMutex` schuetzt
    // den Snapshot, nicht die Queue - ein `p0.ueberlauf()` dort waere ein
    // Datenrennen auf der Deque, auch wenn das Ergebnis nur eine Zahl ist.
    std::uint64_t ueberlaeufe = 0;
    {
        std::lock_guard<std::mutex> l (sendeMutex);
        const auto eintrag = std::find_if (inFlight.begin(), inFlight.end(),
            [&] (const InFlightEintrag& e) { return e.commandId == commandId; });
        if (eintrag != inFlight.end())
        {
            gefunden = true;
            // 🔑 SONDE-014 WN-01 (Nacharbeit 2, 07.09.2026): ein
            // `konflikt`-ACK ist kein endgueltiger Verlust.
            //
            // Bis hierher loeschte JEDES ACK den Eintrag - auch das, das nur
            // sagt "dein Kopf trug eine Revision, die ich nicht kenne". Das
            // Urteil war damit weder persistiert noch wiederholbar, obwohl
            // der Auftrag selbst richtig war (WP1-1, M-73).
            //
            // Der Hook baut den Kopf mit der Revision NEU, die der Broker
            // soeben genannt hat, und behaelt die `command_id` - der Broker
            // erkennt die Wiederholung idempotent (NR-10). Erst `angewandt`
            // schliesst den Auftrag ab.
            if (art == CommandAckArt::konflikt
                && hookKonfliktWiederholung
                && eintrag->konfliktWiederholungen < kKonfliktWiederholungenMax)
            {
                const auto frisch =
                    hookKonfliktWiederholung (commandId, eintrag->json, brokerRevision);
                if (! frisch.empty())
                {
                    ++eintrag->konfliktWiederholungen;
                    eintrag->json = frisch;
                    eintrag->gesendetInGeneration = 0;
                    eintrag->inQueue = true;
                    const auto marke = p0MarkenFolge.fetch_add (1) + 1;
                    if (p0.einreihen (P0Eintrag { frisch, P0Klasse::ereignis,
                                                  wireGeneration.load(), marke }))
                    {
                        wiederholt = true;
                    }
                    else
                    {
                        // Dieselbe Politik wie beim Einreihen (WN-05): der
                        // Auftrag bleibt im Register und geht an den
                        // Verwurfweg, statt still zu verschwinden.
                        eintrag->inQueue = false;
                        verworfeneMarke = marke;
                        p0UeberlaufZaehler.fetch_add (1);
                        wiederholt = true;
                    }
                }
            }
            if (! wiederholt)
                inFlight.erase (eintrag);
        }
        anzahl = static_cast<std::uint64_t> (inFlight.size());
        ueberlaeufe = p0.ueberlauf();
    }
    if (! gefunden)
        return;

    if (verworfeneMarke != 0 && beiP0Verworfen)
        beiP0Verworfen (verworfeneMarke);

    // 🔑 SONDE-014 WN3-01 (Nacharbeit 3, 07.09.2026): der Auftrag ist FORT.
    //
    // Wer sich zu dieser `command_id` etwas gemerkt hat - der Prozessor haelt
    // je ausstehendem Urteil einen Mitschnitt -, gibt ihn hier frei. Ohne
    // diesen Rueckweg wuechse das Register des Prozessors mit jedem Urteil,
    // und ein Deckel muesste raten, welcher Eintrag noch gebraucht wird.
    //
    // NICHT unter `sendeMutex`: der Hook nimmt den Sendezustand des
    // Prozessors, und die Ordnung ist sendeMutex VOR Sendezustand.
    if (! wiederholt && hookAuftragAbgeschlossen)
        hookAuftragAbgeschlossen (commandId);

    std::lock_guard<std::mutex> z (zustandMutex);
    zustand.inFlight = anzahl;
    zustand.p0Ueberlaeufe = ueberlaeufe;
    if (wiederholt)
    {
        // Weder Erfolg noch endgueltiger Fehlschlag: der Auftrag laeuft noch.
        ++zustand.inFlightWiederholungen;
        return;
    }
    if (art == CommandAckArt::angewandt
        || art == CommandAckArt::idempotentWiederholt)
        ++zustand.inFlightErfolg;
    else
        ++zustand.inFlightEndgueltigOhneErfolg;
}

P1Ergebnis ControlClient::Laufzeit::sendeP1 (const std::string& schluessel,
                                            const std::string& json)
{
    if (json.size() > kMaxPayloadBytes)
    {
        std::lock_guard<std::mutex> z (zustandMutex);
        ++zustand.zuGross;
        return P1Ergebnis::zuGross;
    }

    std::lock_guard<std::mutex> l (sendeMutex);
    const auto e = p1.einreihen (schluessel, json);
    const auto tiefe = p1.groesse();
    const auto wiederholungen = p1.wiederholungen();
    std::lock_guard<std::mutex> z (zustandMutex);
    zustand.p1Wiederholungen = wiederholungen;
    zustand.p1Tiefe = tiefe;
    zustand.p1WiederholTiefe = wiederholungen;
    return e;
}

ControlClient::Snapshot ControlClient::Laufzeit::snapshotIntern() const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    return zustand;
}

std::size_t ControlClient::fuelleP0QueueFuerTest()
{
    std::lock_guard<std::mutex> l (k->sendeMutex);
    std::size_t n = 0;
    while (k->p0.einreihen (P0Eintrag { "{\"fuellung\":1}", P0Klasse::ereignis,
                                        k->wireGeneration.load(), 0 }))
        ++n;
    return n;
}

void ControlClient::leereP0QueueFuerTest()
{
    std::lock_guard<std::mutex> l (k->sendeMutex);
    P0Eintrag e;
    while (k->p0.entnehmen (e))
        k->p0.bestaetigen();
}

std::size_t ControlClient::zustelleAllesFuerTest()
{
    // 🔑 Nacharbeit 1: Marke UND die Generation, unter der Sperre gelesen.
    // Die Meldung laeuft ausserhalb; die "aktuelle" Zahl waere dort schon
    // eine andere, wenn ein Bein dazwischen einen Aufbauzug faehrt.
    //
    // 🔑 Nacharbeit 2 (WN-02/N-27): gemeldet wird - wie in der echten
    // Sendeschleife - die Generation des COMMITS, nicht die des Einreihens.
    // Dieser Haken IST der Wire-Commit ohne Draht; buchte er die alte Zahl,
    // maesse jedes Bein den Pfad, den das Produkt gerade nicht mehr hat.
    std::vector<std::pair<std::uint64_t, std::uint64_t>> marken;
    {
        std::lock_guard<std::mutex> l (k->sendeMutex);
        const auto dieseWireGeneration = k->wireGeneration.load();
        P0Eintrag e;
        while (k->p0.entnehmen (e))
        {
            k->p0.bestaetigen();
            if (e.marke != 0)
                marken.emplace_back (e.marke, dieseWireGeneration);
        }
    }
    for (const auto& m : marken)
        if (k->beiP0Zugestellt)
            k->beiP0Zugestellt (m.first, m.second);
    return marken.size();
}

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
