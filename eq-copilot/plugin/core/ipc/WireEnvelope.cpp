// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "WireEnvelope.h"

#include <algorithm>
#include <cstring>

namespace nakama::ipc
{
namespace
{
const char* const kNamen[kVerstossAnzahl] = {
    "praefix_unvollstaendig",
    "frame_len_null",
    "frame_len_unter_kopf",
    "frame_len_ueber_grenze",
    "frame_unvollstaendig",
    "ueberlaenge",
    "unterlaenge",
    "crc_bei_json_nicht_null",
    "crc_falsch",
    "encoding_passt_nicht_zur_familie",
    "encoding_unbekannt",
    "flags_reserviert",
    "message_family_unbekannt",
    "schema_major_unbekannt",
};

inline std::uint32_t u32le (const std::uint8_t* p) noexcept
{
    return static_cast<std::uint32_t> (p[0])
         | (static_cast<std::uint32_t> (p[1]) << 8)
         | (static_cast<std::uint32_t> (p[2]) << 16)
         | (static_cast<std::uint32_t> (p[3]) << 24);
}

inline void schreibeU32le (std::vector<std::uint8_t>& ziel, std::uint32_t wert)
{
    ziel.push_back (static_cast<std::uint8_t> (wert & 0xFF));
    ziel.push_back (static_cast<std::uint8_t> ((wert >> 8) & 0xFF));
    ziel.push_back (static_cast<std::uint8_t> ((wert >> 16) & 0xFF));
    ziel.push_back (static_cast<std::uint8_t> ((wert >> 24) & 0xFF));
}

/// Nur ein Urteil mit genau einer Regel — die Stufen 1 und 2 liefern nie mehr.
inline Urteil einzeln (Verstoss v) noexcept
{
    Urteil u;
    u.gueltig = false;
    u.verstoesse = alsBit (v);
    return u;
}
} // namespace

const char* verstossName (Verstoss v) noexcept
{
    const int i = static_cast<int> (v);
    return (i >= 0 && i < kVerstossAnzahl) ? kNamen[i] : "unbekannt";
}

bool verstossAusName (std::string_view name, Verstoss& ziel) noexcept
{
    for (int i = 0; i < kVerstossAnzahl; ++i)
        if (name == kNamen[i])
        {
            ziel = static_cast<Verstoss> (i);
            return true;
        }
    return false;
}

std::uint32_t crc32c (const std::uint8_t* daten, std::size_t laenge) noexcept
{
    constexpr std::uint32_t poly = 0x82F63B78u; // reflektiertes 0x1EDC6F41
    std::uint32_t crc = 0xFFFFFFFFu;
    for (std::size_t i = 0; i < laenge; ++i)
    {
        crc ^= daten[i];
        for (int b = 0; b < 8; ++b)
        {
            const std::uint32_t low = crc & 1u;
            crc >>= 1;
            if (low != 0)
                crc ^= poly;
        }
    }
    return ~crc;
}

Urteil envelopePruefen (const std::uint8_t* daten, std::size_t laenge) noexcept
{
    // ── Stufe 1: Rahmen ───────────────────────────────────────────────────
    if (daten == nullptr || laenge < 4)
        return einzeln (Verstoss::praefixUnvollstaendig);

    const std::uint32_t frameLen = u32le (daten);
    if (frameLen == 0)
        return einzeln (Verstoss::frameLenNull);
    if (frameLen > kMaxFrameBytes)
        return einzeln (Verstoss::frameLenUeberGrenze);
    if (frameLen < kKopfBytes)
        return einzeln (Verstoss::frameLenUnterKopf);

    const std::size_t drahtlaenge = static_cast<std::size_t> (4u) + frameLen;
    if (laenge < drahtlaenge)
        return einzeln (Verstoss::frameUnvollstaendig);

    // ── Stufe 2: Kopfkonsistenz ──────────────────────────────────────────
    const std::uint8_t* kopfBytes = daten + 4;
    const std::uint32_t payloadLen = u32le (kopfBytes + 8);
    // In 64 Bit, NICHT in 32: 16 + 0xFFFFFFFF liefe auf 15 um und ein naiver
    // Pruefer haette den Frame akzeptiert. Genau dafuer gibt es das Fixture
    // ungueltig/kopf-payload-len-u32-ueberlauf.bin.
    const std::uint64_t erwartet = static_cast<std::uint64_t> (kKopfBytes)
                                 + static_cast<std::uint64_t> (payloadLen);
    const std::uint64_t ist = static_cast<std::uint64_t> (frameLen);
    if (ist > erwartet)
        return einzeln (Verstoss::ueberlaenge);
    if (ist < erwartet)
        return einzeln (Verstoss::unterlaenge);

    // ── Stufe 3: Feldwerte ───────────────────────────────────────────────
    // Die Bits werden in Deklarationsreihenfolge gesetzt und spaeter in
    // derselben Reihenfolge gelesen; ein Test haelt fest, dass diese
    // Reihenfolge die kanonische Namenssortierung IST.
    VerstossMenge menge = 0;
    const std::uint8_t encodingByte = kopfBytes[0];
    const std::uint8_t familieByte  = kopfBytes[1];
    const std::uint8_t schemaMajor  = kopfBytes[2];
    const std::uint8_t schemaMinor  = kopfBytes[3];
    const std::uint32_t flags       = u32le (kopfBytes + 4);
    const std::uint32_t crcSoll     = u32le (kopfBytes + 12);

    const bool encodingBekannt = (encodingByte <= 1);
    const bool familieBekannt  = (familieByte  <= 2);

    if (! encodingBekannt)
        menge |= alsBit (Verstoss::encodingUnbekannt);
    if (! familieBekannt)
        menge |= alsBit (Verstoss::messageFamilyUnbekannt);
    if (schemaMajor != kSchemaMajor)
        menge |= alsBit (Verstoss::schemaMajorUnbekannt);
    if (flags != 0)
        menge |= alsBit (Verstoss::flagsReserviert);

    const auto encoding = static_cast<Kodierung> (encodingByte);
    const auto familie  = static_cast<Familie> (familieByte);

    if (encodingBekannt && familieBekannt && encoding != erwarteteKodierung (familie))
        menge |= alsBit (Verstoss::encodingPasstNichtZurFamilie);

    const std::uint8_t* payload = daten + 4 + kKopfBytes;
    // CRC nur bei bekanntem Encoding: ohne Encoding gibt es keine Aussage
    // darueber, ob 0 gefordert oder eine Pruefsumme erwartet ist.
    if (encodingBekannt)
    {
        if (encoding == Kodierung::json)
        {
            if (crcSoll != 0)
                menge |= alsBit (Verstoss::crcBeiJsonNichtNull);
        }
        else if (crcSoll != crc32c (payload, payloadLen))
        {
            menge |= alsBit (Verstoss::crcFalsch);
        }
    }

    Urteil u;
    u.verstoesse = menge;
    u.gueltig = (menge == 0);
    if (! u.gueltig)
        return u;

    u.kopf.encoding    = encoding;
    u.kopf.familie     = familie;
    u.kopf.schemaMajor = schemaMajor;
    u.kopf.schemaMinor = schemaMinor;
    u.kopf.flags       = flags;
    u.kopf.payloadLen  = payloadLen;
    u.kopf.pruefsumme  = crcSoll;
    u.drahtlaenge      = drahtlaenge;
    u.payload          = payload;
    u.payloadLaenge    = payloadLen;
    return u;
}

bool envelopeSchreiben (Familie familie,
                        std::uint8_t schemaMinor,
                        const std::uint8_t* payload,
                        std::size_t payloadLaenge,
                        std::vector<std::uint8_t>& ziel)
{
    if (payloadLaenge > kMaxPayloadBytes)
        return false;

    const auto encoding = erwarteteKodierung (familie);
    const auto payloadLen = static_cast<std::uint32_t> (payloadLaenge);
    const std::uint32_t crc = (encoding == Kodierung::json)
                                ? 0u
                                : crc32c (payload, payloadLaenge);

    ziel.clear();
    ziel.reserve (4 + kKopfBytes + payloadLaenge);
    schreibeU32le (ziel, kKopfBytes + payloadLen);
    ziel.push_back (static_cast<std::uint8_t> (encoding));
    ziel.push_back (static_cast<std::uint8_t> (familie));
    ziel.push_back (kSchemaMajor);
    ziel.push_back (schemaMinor);
    schreibeU32le (ziel, 0u); // flags
    schreibeU32le (ziel, payloadLen);
    schreibeU32le (ziel, crc);
    if (payloadLaenge > 0)
        ziel.insert (ziel.end(), payload, payload + payloadLaenge);
    return true;
}

void StromLeser::fuettern (const std::uint8_t* daten, std::size_t laenge)
{
    if (gelesen > 0 && gelesen == puffer.size())
    {
        puffer.clear();
        gelesen = 0;
    }
    else if (gelesen > 65536)
    {
        // Erst verdichten, wenn es sich lohnt: ein erase() pro Frame waere ein
        // memmove pro Frame.
        puffer.erase (puffer.begin(), puffer.begin() + static_cast<std::ptrdiff_t> (gelesen));
        gelesen = 0;
    }
    puffer.insert (puffer.end(), daten, daten + laenge);
}

StromLeser::Ergebnis StromLeser::naechster()
{
    Ergebnis e;
    const std::size_t offenBytes = puffer.size() - gelesen;
    const Urteil u = envelopePruefen (puffer.data() + gelesen, offenBytes);
    if (u.gueltig)
    {
        e.art = Art::frame;
        e.kopf = u.kopf;
        e.payload = u.payload;
        e.payloadLaenge = u.payloadLaenge;
        gelesen += u.drahtlaenge;
        return e;
    }
    if (u.verstoesse == alsBit (Verstoss::praefixUnvollstaendig)
        || u.verstoesse == alsBit (Verstoss::frameUnvollstaendig))
    {
        e.art = Art::unvollstaendig;
        return e;
    }
    e.art = Art::verstoss;
    e.verstoesse = u.verstoesse;
    return e;
}

Ratengrenze::Ratengrenze (std::uint32_t maxImFensterIn, std::uint64_t fensterMsIn)
    : fensterMs (fensterMsIn),
      maxImFenster (maxImFensterIn == 0 ? 1u : maxImFensterIn)
{
    zeitpunkte.assign (maxImFenster, 0);
}

bool Ratengrenze::erlaubt (std::uint64_t jetztMs) noexcept
{
    while (anzahl > 0)
    {
        const std::uint64_t aeltester = zeitpunkte[kopf];
        if (jetztMs >= aeltester && (jetztMs - aeltester) >= fensterMs)
        {
            kopf = (kopf + 1) % zeitpunkte.size();
            --anzahl;
        }
        else
        {
            break;
        }
    }
    if (anzahl >= maxImFenster)
        return false;
    const std::size_t ende = (kopf + anzahl) % zeitpunkte.size();
    zeitpunkte[ende] = jetztMs;
    ++anzahl;
    return true;
}

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
