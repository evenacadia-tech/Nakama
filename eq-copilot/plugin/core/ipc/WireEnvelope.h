// SONDE-010 — v3-Envelope, C++-Fassung (Entwurf §33.1, §53.4).
//
// Bewusst JUCE-frei und bibliotheksfrei, wie core/StampedAudioQueue.h und
// core/analysis/LoudnessAccumulator.h: die riskante Grenzarbeit liegt damit in
// UNSEREM Repo und ist headless pruefbar.
//
//   u32 frame_len (little-endian, wie v2)
//   u8  encoding        0 = JSON, 1 = FlatBuffers
//   u8  message_family  0 = P0, 1 = P1, 2 = P2
//   u8  schema_major
//   u8  schema_minor
//   u32 flags           heute vollstaendig reserviert => muss 0 sein
//   u32 payload_len
//   u32 crc32c          ueber GENAU die payload_len Payloadbytes
//
// Die Regelmenge, ihre Namen und die Reihenfolge sind IDENTISCH mit
// broker/src/transport/v3.rs. Beide messen gegen
// eq-copilot/fixtures/v3/envelope/MANIFEST.json.
//
// KEINE ALLOKATION im Pruefpfad: die Verstossmenge ist eine Bitmaske, der
// Payload ein Zeiger in den Eingabepuffer. Das ist kein Selbstzweck — der
// Envelope wird auf jedem Pfad gelesen, und ein Pruefer, der pro Frame
// alloziert, waere im Rueckstau genau dann teuer, wenn es am wenigsten
// verkraftbar ist.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace nakama::ipc
{

/// Harte Paketgrenze — identisch mit eqcop::kMaxFrameBytes (v2) und
/// framing::MAX_FRAME_BYTES im Broker. Sie gilt fuer frame_len, also fuer
/// Kopf PLUS Payload.
inline constexpr std::uint32_t kMaxFrameBytes = 262144u;

/// Laenge des v3-Kopfes hinter dem Laengenpraefix.
inline constexpr std::uint32_t kKopfBytes = 16u;

/// Groesster heute erlaubter Payload.
inline constexpr std::uint32_t kMaxPayloadBytes = kMaxFrameBytes - kKopfBytes;

/// Die einzige heute unterstuetzte Major-Version.
inline constexpr std::uint8_t kSchemaMajor = 3u;

/// Aktive JSON-Vertragsfassung fuer P0/P1. Minor 0 bleibt lesbar, wird aber
/// gegen seinen historischen Feldsatz validiert; hoehere Werte sind fremd.
inline constexpr std::uint8_t kJsonSchemaMinor = 1u;

/// Bootstrap-Hello: hoechstens 16 KiB, nur u32-laengenpraefigiertes JSON (§53.9).
inline constexpr std::uint32_t kMaxBootstrapBytes = 16u * 1024u;

/// Nachrichtenratengrenze je Verbindung (§33.1 "Parser erhalten Tiefen-,
/// Laengen- und Nachrichtenratenlimits"). DIESELBEN Zahlen wie im Broker
/// (`server_v3.rs`, `RATE_PRO_SEKUNDE`): das Limit ist eine Eigenschaft des
/// Vertrags, nicht der Sprache. Sie gilt auf JEDER Parserseite — bis
/// 2026-08-29 stand die C++-`Ratengrenze` nur im Test, und ein Peer konnte
/// den Client hinter dem welcome beliebig zupipelinen (T2-Befund 5).
inline constexpr std::uint32_t kRateProSekunde = 4000;
inline constexpr std::uint64_t kRateFensterMs  = 1000;

enum class Kodierung : std::uint8_t { json = 0, flatBuffers = 1 };
enum class Familie   : std::uint8_t { p0 = 0, p1 = 1, p2 = 2 };

/// Die vom Vertrag vorgeschriebene Kodierung je Familie: P0/P1 verwenden JSON,
/// P2 verweist auf ein FeatureBatch (§33.1). Erst dadurch sind "CRC ist fuer
/// P2 Pflicht" und "bei JSON exakt 0" ueberschneidungsfrei.
constexpr Kodierung erwarteteKodierung (Familie f) noexcept
{
    return f == Familie::p2 ? Kodierung::flatBuffers : Kodierung::json;
}

/// Geschlossene Regelmenge. Die REIHENFOLGE ist Vertrag: sie ist zugleich die
/// kanonische Sortierung nach dem Regelnamen (ein Test haelt das fest), damit
/// C++ und Rust ohne Sortierschritt dieselbe Menge in derselben Ordnung
/// liefern.
enum class Verstoss : std::uint8_t
{
    // Stufe 1 — Rahmen
    praefixUnvollstaendig = 0,
    frameLenNull,
    frameLenUnterKopf,
    frameLenUeberGrenze,
    frameUnvollstaendig,
    // Stufe 2 — Kopfkonsistenz
    ueberlaenge,
    unterlaenge,
    // Stufe 3 — Feldwerte
    crcBeiJsonNichtNull,
    crcFalsch,
    encodingPasstNichtZurFamilie,
    encodingUnbekannt,
    flagsReserviert,
    messageFamilyUnbekannt,
    schemaMajorUnbekannt,
};

inline constexpr int kVerstossAnzahl = 14;

const char* verstossName (Verstoss v) noexcept;

/// Rueckweg aus dem Manifestnamen. `false` heisst: der Name steht nicht in der
/// geschlossenen Menge — ein Manifestfehler, kein Parserzustand.
bool verstossAusName (std::string_view name, Verstoss& ziel) noexcept;

/// Verstossmenge als Bitmaske; Bit n = Regel n.
using VerstossMenge = std::uint16_t;

constexpr VerstossMenge alsBit (Verstoss v) noexcept
{
    return static_cast<VerstossMenge> (1u << static_cast<int> (v));
}

constexpr bool enthaelt (VerstossMenge m, Verstoss v) noexcept
{
    return (m & alsBit (v)) != 0;
}

/// CRC32C (Castagnoli, reflektiert). Bitweise gerechnet — dieselbe Rechnung
/// wie in Rust und Python; eine Tabelle waere schneller und eine vierte
/// Wahrheit ueber dasselbe Polynom.
std::uint32_t crc32c (const std::uint8_t* daten, std::size_t laenge) noexcept;

struct Kopf
{
    Kodierung     encoding     = Kodierung::json;
    Familie       familie      = Familie::p0;
    std::uint8_t  schemaMajor  = 0;
    std::uint8_t  schemaMinor  = 0;
    std::uint32_t flags        = 0;
    std::uint32_t payloadLen   = 0;
    std::uint32_t pruefsumme   = 0;
};

struct Urteil
{
    /// Genau dann true, wenn `verstoesse == 0`.
    bool          gueltig      = false;
    VerstossMenge verstoesse   = 0;
    Kopf          kopf {};
    /// Gesamtlaenge auf dem Draht: 4 + frame_len. Nur bei `gueltig` gesetzt.
    std::size_t   drahtlaenge  = 0;
    /// Zeiger IN den Eingabepuffer — er lebt genau so lange wie dieser.
    const std::uint8_t* payload = nullptr;
    std::size_t   payloadLaenge = 0;
};

/// Kernpruefung ueber einen Bytepuffer, der mit dem Laengenpraefix beginnt.
Urteil envelopePruefen (const std::uint8_t* daten, std::size_t laenge) noexcept;

/// Schreibt einen v3-Frame. `false` heisst: der Payload ist groesser als die
/// Paketgrenze zulaesst; `ziel` bleibt dann unveraendert.
bool envelopeSchreiben (Familie familie,
                        std::uint8_t schemaMinor,
                        const std::uint8_t* payload,
                        std::size_t payloadLaenge,
                        std::vector<std::uint8_t>& ziel);

/// Streamender Leser: nimmt beliebig zerteilte Bytes und liefert ganze Frames.
/// Eine Pipe im Bytemodus kennt keine Nachrichtengrenzen — ein Leser, der nur
/// ganze Puffer versteht, ist im Betrieb keiner.
class StromLeser
{
public:
    enum class Art { unvollstaendig, frame, verstoss };

    struct Ergebnis
    {
        Art           art = Art::unvollstaendig;
        VerstossMenge verstoesse = 0;
        Kopf          kopf {};
        /// Zeiger in den INTERNEN Puffer; gueltig bis zum naechsten Aufruf.
        const std::uint8_t* payload = nullptr;
        std::size_t   payloadLaenge = 0;
    };

    void fuettern (const std::uint8_t* daten, std::size_t laenge);
    std::size_t offen() const noexcept { return puffer.size() - gelesen; }

    /// Naechster Frame, falls vollstaendig.
    Ergebnis naechster();

private:
    std::vector<std::uint8_t> puffer;
    std::size_t gelesen = 0;
};

/// Nachrichtenratengrenze (§33.1). Gleitendes Fenster ueber eine monotone
/// Millisekundenuhr des AUFRUFERS — die Klasse liest selbst keine Uhr, damit
/// der Test sie deterministisch fahren kann.
class Ratengrenze
{
public:
    Ratengrenze (std::uint32_t maxImFenster, std::uint64_t fensterMs);

    /// true = darf verarbeitet werden.
    bool erlaubt (std::uint64_t jetztMs) noexcept;

private:
    std::uint64_t fensterMs;
    std::uint32_t maxImFenster;
    std::vector<std::uint64_t> zeitpunkte;  // Ringpuffer fester Groesse
    std::size_t kopf = 0, anzahl = 0;
};

} // namespace nakama::ipc
