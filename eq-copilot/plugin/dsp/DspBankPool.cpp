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
DspBankPool::DspBankPool()
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
        zustaende[(size_t) i].store (BankZustand::frei, std::memory_order_relaxed);
        generationen[(size_t) i].store (0, std::memory_order_relaxed);
        baenke[(size_t) i].programm = DspProgramm {};
        baenke[(size_t) i].zustaendeNullen();
    }
    for (auto& v : veroeffentlicht) v.store (-1, std::memory_order_relaxed);
    for (auto& e : ackRing) e = -1;
    ackSchreib.store (0, std::memory_order_relaxed);
    ackLese.store   (0, std::memory_order_relaxed);
    reclaimMaske.store (0, std::memory_order_relaxed);
    ueberlaeufe.store  (0, std::memory_order_relaxed);
    verriegelungen.store (0, std::memory_order_relaxed);
    generationsZaehler.store (0, std::memory_order_relaxed);
}

bool DspBankPool::wechsle (int slot, BankZustand von, BankZustand nach) noexcept
{
    if (slot < 0 || slot >= kBaenke) return false;
    BankZustand erwartet = von;
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
        if (zustaende[(size_t) i].load (std::memory_order_acquire) == BankZustand::frei)
            ++n;
    return n;
}

//==============================================================================
int DspBankPool::reserviere() noexcept
{
    for (int i = 0; i < kBaenke; ++i)
        if (wechsle (i, BankZustand::frei, BankZustand::vorbereitend))
            return i;
    // M-44: kein Slot frei heisst `busy_retry`. Nicht puffern, nicht
    // verwerfen, keine aktive Bank verdraengen - der Aufrufer wiederholt.
    return -1;
}

int DspBankPool::publiziere (Pfad p, int slot) noexcept
{
    if (! wechsle (slot, BankZustand::vorbereitend, BankZustand::bereit))
        return -1;

    generationen[(size_t) slot].store (baenke[(size_t) slot].programm.generation,
                                       std::memory_order_release);

    // Release-Store der Publikationsstelle: der Audiothread sieht mit
    // seinem Acquire-Load garantiert die fertig geschriebene Bank.
    const int vorher = veroeffentlicht[(size_t) p].exchange (slot, std::memory_order_acq_rel);

    if (vorher >= 0 && vorher != slot)
    {
        // Verdraengt, bevor der Audiothread ihn je gesehen hat. Er war nie
        // `audioAktiv`, also hat er keinen Filterzustand im Umlauf und
        // braucht keinen ACK - direkt zurueck auf `frei`.
        if (wechsle (vorher, BankZustand::bereit, BankZustand::frei))
            return vorher;
    }
    return -1;
}

int DspBankPool::ernteAcks() noexcept
{
    int freigegeben = 0;
    const auto maske = reclaimMaske.load (std::memory_order_acquire);

    for (;;)
    {
        const auto lese = ackLese.load (std::memory_order_relaxed);
        if (lese == ackSchreib.load (std::memory_order_acquire)) break;

        const int slot = ackRing[(size_t) (lese % kAckKapazitaet)];
        ackLese.store (lese + 1, std::memory_order_release);

        if (slot < 0 || slot >= kBaenke) continue;

        // M-45: ein Slot mit gesetztem Bit bleibt dauerhaft nicht frei, bis
        // `bestaetigeReclaim` ihn loest. Reclaim-Sicherheit gewinnt ueber
        // Verfuegbarkeit.
        if ((maske >> slot) & 1u) continue;

        if (wechsle (slot, BankZustand::ausgedient, BankZustand::frei))
            ++freigegeben;
    }
    return freigegeben;
}

void DspBankPool::bestaetigeReclaim (int slot) noexcept
{
    if (slot < 0 || slot >= kBaenke) return;
    reclaimMaske.fetch_and (~(std::uint64_t) (1ull << slot), std::memory_order_acq_rel);
    wechsle (slot, BankZustand::ausgedient, BankZustand::frei);
}

//==============================================================================
int DspBankPool::uebernehmeBereiten (Pfad p) noexcept
{
    const int slot = veroeffentlicht[(size_t) p].load (std::memory_order_acquire);
    if (slot < 0) return -1;

    if (! wechsle (slot, BankZustand::bereit, BankZustand::audioAktiv))
        return -1;

    // Erst NACH dem geglueckten Uebergang zuruecksetzen: sonst koennte ein
    // gescheiterter Wechsel die Publikation verlieren.
    int erwartet = slot;
    veroeffentlicht[(size_t) p].compare_exchange_strong (
        erwartet, -1, std::memory_order_acq_rel, std::memory_order_relaxed);
    return slot;
}

bool DspBankPool::beginneVerblassen (int slot) noexcept
{
    return wechsle (slot, BankZustand::audioAktiv, BankZustand::verblassend);
}

void DspBankPool::meldeAusgedient (int slot) noexcept
{
    if (! wechsle (slot, BankZustand::verblassend, BankZustand::ausgedient))
        return;

    const auto schreib = ackSchreib.load (std::memory_order_relaxed);
    const auto lese    = ackLese.load (std::memory_order_acquire);

    if (schreib - lese >= (std::uint64_t) kAckKapazitaet)
    {
        // Der Ring FASST mehr Eintraege als es Slots gibt und darf deshalb
        // strukturell nicht ueberlaufen. Tut er es doch, wird der Slot
        // verriegelt statt der ACK verworfen: ein verlorener ACK liesse den
        // Worker eine Bank ueberschreiben, die der Audiothread noch liest.
        reclaimMaske.fetch_or ((std::uint64_t) (1ull << slot), std::memory_order_acq_rel);
        verriegelungen.fetch_add (1, std::memory_order_relaxed);
        ueberlaeufe.fetch_add (1, std::memory_order_relaxed);
        return;
    }

    ackRing[(size_t) (schreib % kAckKapazitaet)] = slot;
    ackSchreib.store (schreib + 1, std::memory_order_release);
}

} // namespace nakama::dsp

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
