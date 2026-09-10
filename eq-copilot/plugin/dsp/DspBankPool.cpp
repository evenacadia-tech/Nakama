#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 - keine JucePlugin_*-Konstante im Kern
#include "DspBankPool.h"

namespace nakama::dsp
{

//==============================================================================
void BandZustand::nullen() noexcept
{
    for (auto& z : statisch) z.nullen();
    for (auto& z : svf)      z.nullen();
    for (auto& z : detektor) z.nullen();
    huelle.nullen();
    schrittRest  = 0;
    svfVon       = {};
    svfNach      = {};
    auslenkungDb = 0.0;
}

bool BandZustand::istEndlich() const noexcept
{
    for (const auto& z : statisch) if (! z.istEndlich()) return false;
    for (const auto& z : svf)      if (! z.istEndlich()) return false;
    for (const auto& z : detektor) if (! z.istEndlich()) return false;
    return huelle.istEndlich() && std::isfinite (auslenkungDb);
}

int BandZustand::riegleDenormale() noexcept
{
    int n = 0;
    for (auto& z : statisch) n += z.riegleDenormale() ? 1 : 0;
    for (auto& z : svf)      n += z.riegleDenormale() ? 1 : 0;
    for (auto& z : detektor) n += z.riegleDenormale() ? 1 : 0;
    n += huelle.riegleDenormale() ? 1 : 0;
    return n;
}

void DspBank::zustaendeNullen() noexcept
{
    for (auto& b : baender) b.nullen();
    monoBassZustand.nullen();
}

bool DspBank::zustaendeSindEndlich() const noexcept
{
    for (const auto& b : baender) if (! b.istEndlich()) return false;
    return monoBassZustand.istEndlich();
}

int DspBank::riegleDenormale() noexcept
{
    int n = 0;
    for (auto& b : baender) n += b.riegleDenormale();
    n += monoBassZustand.riegleDenormale() ? 1 : 0;
    return n;
}

//==============================================================================
DspBankPool::DspBankPool (int ackKapazitaet)
    : ringKapazitaet (ackKapazitaet < 1 ? 1 : (ackKapazitaet > kAckKapazitaet ? kAckKapazitaet : ackKapazitaet))
{
    // Der ganze Speicher entsteht HIER, im Konstruktor: `std::array` traegt
    // seine Elemente im Objekt, es gibt keinen Zeiger und keinen Heap. Nach
    // `prepareToPlay` findet keine weitere Allokation statt (M-41), auch
    // nicht beim ersten Programmwechsel - genau das misst der
    // Allokationszaehler.
    zuruecksetzen();
}

void DspBankPool::zuruecksetzen() noexcept
{
    for (int i = 0; i < kBaenke; ++i)
    {
        zustaende[(size_t) i].store (zustandsWort (0, BankZustand::frei), std::memory_order_relaxed);
        baenke[(size_t) i].programm = DspProgramm {};
        baenke[(size_t) i].zustaendeNullen();
    }
    for (auto& v : veroeffentlicht) v.store (0, std::memory_order_relaxed);
    for (auto& e : ackRing) e = 0;
    ackSchreib.store (0, std::memory_order_relaxed);
    ackLese.store   (0, std::memory_order_relaxed);
    reclaimMaske.store (0, std::memory_order_relaxed);
    ueberlaeufe.store  (0, std::memory_order_relaxed);
    verriegelungen.store (0, std::memory_order_relaxed);
    // B-12: `generationsZaehler` bleibt stehen - Lebenszeit, nicht Ressource.
}

bool DspBankPool::wechsle (int slot, std::uint64_t von, std::uint64_t nach) noexcept
{
    if (slot < 0 || slot >= kBaenke) return false;
    std::uint64_t erwartet = von;
    // Release beim Erfolg, damit alles, was der wechselnde Thread vorher in
    // die Bank geschrieben hat, fuer den naechsten Leser sichtbar ist;
    // Acquire beim Lesen des Ist-Zustands.
    return zustaende[(size_t) slot].compare_exchange_strong (
        erwartet, nach, std::memory_order_acq_rel, std::memory_order_acquire);
}

int DspBankPool::freieSlots() const noexcept
{
    int n = 0;
    for (int i = 0; i < kBaenke; ++i)
        if (zustand (i) == BankZustand::frei)
            ++n;
    return n;
}

//==============================================================================
int DspBankPool::reserviere() noexcept
{
    for (int i = 0; i < kBaenke; ++i)
    {
        const std::uint64_t ist = zustaende[(size_t) i].load (std::memory_order_acquire);
        if (zustandAus (ist) != BankZustand::frei) continue;
        if (wechsle (i, ist, zustandsWort (0, BankZustand::vorbereitend)))
            return i;
    }
    // M-44: kein Slot frei heisst `busy_retry`. Nicht puffern, nicht
    // verwerfen, keine aktive Bank verdraengen - der Aufrufer wiederholt.
    return -1;
}

int DspBankPool::verdraengeBereiten (std::uint64_t altesWort) noexcept
{
    const int alt = slotAus (altesWort);
    if (altesWort == 0 || alt < 0 || alt >= kBaenke) return -1;

    // Verdraengt, bevor der Audiothread ihn uebernommen hat - aber NUR in
    // genau der publizierten Generation. Hat der Audiothread ihn schon
    // genommen, scheitert der CAS, und die Bank bleibt seine.
    const std::uint64_t g = generationAus (altesWort);
    if (wechsle (alt, zustandsWort (g, BankZustand::bereit), zustandsWort (g, BankZustand::frei)))
        return alt;
    return -1;
}

int DspBankPool::publiziere (Pfad p, int slot) noexcept
{
    if (slot < 0 || slot >= kBaenke) return -1;
    const std::uint64_t g = baenke[(size_t) slot].programm.generation;
    if (g == 0) return -1;   // ein Programm ohne Generation ist nicht publizierbar

    if (! wechsle (slot, zustandsWort (0, BankZustand::vorbereitend), zustandsWort (g, BankZustand::bereit)))
        return -1;

    // Release-Store der Publikationsstelle: der Audiothread sieht mit
    // seinem Acquire-Load garantiert die fertig geschriebene Bank.
    const std::uint64_t vorher = veroeffentlicht[(size_t) p].exchange (
        publikationsWort (g, slot), std::memory_order_acq_rel);
    return verdraengeBereiten (vorher);
}

int DspBankPool::publiziereEnde (Pfad p) noexcept
{
    const std::uint64_t vorher = veroeffentlicht[(size_t) p].exchange (
        publikationsWort (naechsteGeneration(), kBaenke), std::memory_order_acq_rel);
    return verdraengeBereiten (vorher);
}

int DspBankPool::ernteAcks (int hoechstens) noexcept
{
    int freigegeben = 0, gelesen = 0;
    const auto maske = reclaimMaske.load (std::memory_order_acquire);

    for (;;)
    {
        if (hoechstens >= 0 && gelesen >= hoechstens) break;
        ++gelesen;
        const auto lese = ackLese.load (std::memory_order_relaxed);
        if (lese == ackSchreib.load (std::memory_order_acquire)) break;

        const std::uint64_t wort = ackRing[(size_t) (lese % (std::uint64_t) ringKapazitaet)];
        ackLese.store (lese + 1, std::memory_order_release);

        const int slot = slotAus (wort);
        if (wort == 0 || slot < 0 || slot >= kBaenke) continue;

        // M-45: ein Slot mit gesetztem Bit bleibt dauerhaft nicht frei, bis
        // `bestaetigeReclaim` ihn loest. Reclaim-Sicherheit gewinnt ueber
        // Verfuegbarkeit.
        if ((maske >> slot) & 1u) continue;

        // B-1/B-12: frei wird der Slot nur in GENAU der gemeldeten
        // Generation. Ein ACK einer aelteren Generation findet ihn dort nie.
        const std::uint64_t g = generationAus (wort);
        if (wechsle (slot, zustandsWort (g, BankZustand::ausgedient), zustandsWort (g, BankZustand::frei)))
            ++freigegeben;
    }
    return freigegeben;
}

bool DspBankPool::bestaetigeReclaim (int slot) noexcept
{
    if (slot < 0 || slot >= kBaenke) return false;
    const std::uint64_t bit = (std::uint64_t) (1ull << slot);

    // E-23: nur ein VERRIEGELTER Slot wird bestaetigt. Ohne Bit liegt sein
    // ACK im Ring, und eine Bestaetigung waere ein zweiter Freigabeweg.
    if ((reclaimMaske.load (std::memory_order_acquire) & bit) == 0) return false;

    const std::uint64_t ist = zustaende[(size_t) slot].load (std::memory_order_acquire);
    if (zustandAus (ist) != BankZustand::ausgedient) return false;
    if (! wechsle (slot, ist, zustandsWort (generationAus (ist), BankZustand::frei))) return false;

    reclaimMaske.fetch_and (~bit, std::memory_order_acq_rel);
    return true;
}

//==============================================================================
int DspBankPool::uebernehme (Pfad p, std::uint64_t beobachtet) noexcept
{
    if (beobachtet == 0) return -1;
    const int slot = slotAus (beobachtet);
    const std::uint64_t g = generationAus (beobachtet);

    if (slot == kBaenke)
    {
        // Die ENDE-Marke belegt keine Bank; genommen ist sie, wenn das Wort
        // noch dasteht. Eine neuere Publikation gewinnt.
        std::uint64_t erwartet = beobachtet;
        return veroeffentlicht[(size_t) p].compare_exchange_strong (
                   erwartet, 0, std::memory_order_acq_rel, std::memory_order_acquire) ? kEnde : -1;
    }
    if (slot < 0 || slot >= kBaenke) return -1;

    // B-1: DIE Zeile, die die Zusage traegt. Uebernommen wird der Slot nur
    // in GENAU der beobachteten Generation. Wurde er inzwischen verdraengt,
    // freigegeben und neu belegt, traegt er eine neuere Generation, und der
    // CAS scheitert - kein fremdes Programm wird Committed oder Candidate.
    if (! wechsle (slot, zustandsWort (g, BankZustand::bereit), zustandsWort (g, BankZustand::audioAktiv)))
        return -1;

    // Erst NACH dem geglueckten Uebergang die Stelle leeren - und nur, wenn
    // sie noch dieses Wort traegt. Hat der Worker inzwischen neu publiziert,
    // bleibt die neue Publikation stehen und wird am naechsten Blockrand
    // genommen.
    std::uint64_t erwartet = beobachtet;
    veroeffentlicht[(size_t) p].compare_exchange_strong (
        erwartet, 0, std::memory_order_acq_rel, std::memory_order_relaxed);
    return slot;
}

bool DspBankPool::beginneVerblassen (int slot) noexcept
{
    if (slot < 0 || slot >= kBaenke) return false;
    const std::uint64_t ist = zustaende[(size_t) slot].load (std::memory_order_acquire);
    if (zustandAus (ist) != BankZustand::audioAktiv) return false;
    return wechsle (slot, ist, zustandsWort (generationAus (ist), BankZustand::verblassend));
}

bool DspBankPool::ackEinreihen (int slot, std::uint64_t generation) noexcept
{
    const auto schreib = ackSchreib.load (std::memory_order_relaxed);
    const auto lese    = ackLese.load (std::memory_order_acquire);
    if (schreib - lese >= (std::uint64_t) ringKapazitaet) return false;

    ackRing[(size_t) (schreib % (std::uint64_t) ringKapazitaet)] = publikationsWort (generation, slot);
    ackSchreib.store (schreib + 1, std::memory_order_release);
    return true;
}

void DspBankPool::meldeAusgedient (int slot) noexcept
{
    if (slot < 0 || slot >= kBaenke) return;
    const std::uint64_t ist = zustaende[(size_t) slot].load (std::memory_order_acquire);
    if (zustandAus (ist) != BankZustand::verblassend) return;
    const std::uint64_t g = generationAus (ist);
    if (! wechsle (slot, ist, zustandsWort (g, BankZustand::ausgedient)))
        return;

    if (! ackEinreihen (slot, g))
    {
        // Der Ring FASST mehr Eintraege als es Slots gibt und darf deshalb
        // strukturell nicht ueberlaufen. Tut er es doch, wird der Slot
        // verriegelt statt der ACK verworfen: ein verlorener ACK liesse den
        // Worker eine Bank ueberschreiben, die der Audiothread noch liest.
        reclaimMaske.fetch_or ((std::uint64_t) (1ull << slot), std::memory_order_acq_rel);
        verriegelungen.fetch_add (1, std::memory_order_relaxed);
        ueberlaeufe.fetch_add (1, std::memory_order_relaxed);
    }
}

} // namespace nakama::dsp

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
