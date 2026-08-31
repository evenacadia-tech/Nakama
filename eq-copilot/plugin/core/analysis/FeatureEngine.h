// SONDE-009 (S12-13) — FeatureEngine v2: Zeit-, Validity-, Event- und
// Bandvertraege.  Entwurf §32.3, §33.1, §33.2, §53.7, §56; Gate-Text §65:
//
//                "Drop/Seek/Loop trennt jedes offene Fenster."
//
// ─────────────────────────────────────────────────────────────────────────────
// WAS DIESE SCHICHT IST — UND WAS SIE AUSDRUECKLICH NICHT IST.
//
// SONDE-008 hat den Weg vom Audiothread zum Worker gebaut: ganze Bloecke, nie
// Teilbloecke, mit Zeitstempel, und eine Ein-Block-Quarantaene, die einen Block
// erst freigibt, wenn sein Nachfolger beweist, dass er ihn fortsetzt.  Das
// Manifest dazu sagt woertlich (§4.3): "Nicht hier, sondern in SONDE-009:
// Epochen-, Segment- und Fensterbuchhaltung der Features.  Diese Schicht
// liefert nur die Grenze."
//
// Hier ist die andere Haelfte.  Die Quarantaene beantwortet "schliesst dieser
// Block an den vorigen an?"  Diese Engine beantwortet die Frage danach: "und
// was bedeutet das fuer die Fenster, die gerade offen sind?"  Die Antwort ist
// in §32.3 vorgeschrieben und laesst keinen Spielraum:
//
//   "Kein FFT-, Loudness-, Korrelations- oder Fingerprintfenster darf eine
//    ECHTE ODER MOEGLICHE Epochengrenze ueberbruecken."
//
// Nicht "soll moeglichst nicht".  Nicht "wird interpoliert".  Ein Fenster, das
// eine Grenze ueberbrueckt, mittelt zwei Stellen der Musik zu einer Zahl
// zusammen, und diese Zahl sieht danach aus wie eine Messung.  Sie ist keine.
//
// ─────────────────────────────────────────────────────────────────────────────
// WARUM ZWEI AUFLOESUNGSSTUFEN, UND WARUM DAS MIT DEM GATE ZU TUN HAT.
//
// Ein 1/24-Oktav-Band bei 30 Hz ist 0,88 Hz breit.  Eine 4096-Punkt-FFT bei
// 48 kHz hat 11,7 Hz Binbreite — das unterste Band enthaelt dort keinen
// einzigen Bin und ist schlicht nicht messbar.  Deshalb zwei Stufen: 16384 fuer
// alles unter 200 Hz, 4096 darueber (dieselbe Aufteilung wie in `AnalyseEngine`,
// nur ohne deren zwei Zusatzstufen, die der `analyze-track`-Achse dienen).
//
// 🔑 Das ist nicht nur Genauigkeit, es ist die eigentliche BEWEISFLAECHE dieses
// Tickets.  Zwei Stufen heissen zwei GLEICHZEITIG OFFENE Fenster mit
// verschiedener Laenge: bei 48 kHz deckt das Bassfenster 341 ms ab, das
// Hauptfenster 85 ms.  Ein Fehler in der Trennung waere mit nur EINER Stufe
// womoeglich unsichtbar (das kurze Fenster ist an einer Grenze oft ohnehin
// fast leer), mit zwei Stufen faellt er auf: das lange Fenster reicht ueber
// Grenzen, die das kurze laengst hinter sich hat.  Der Golden faehrt genau das.
//
// ─────────────────────────────────────────────────────────────────────────────
// GRUNDGESETZ.  Diese Engine laeuft im WORKER, nie im Audiothread — sie sieht
// nur bereits versiegelte Bloecke.  Trotzdem entsteht ihr gesamter Speicher in
// `vorbereiten()`: eine Allokation je Fenster bei 10 Hz waere Muell in einem
// Blatt, das "feste Obergrenzen" verspricht (§33 "feste Obergrenzen fuer
// Sonden, Baender, Ereignisse, Queue-Tiefe").  Kein Ring waechst, keine Liste
// waechst, der Ereignisstrom ist gedeckelt und zaehlt seine Verluste.
//
// NaN-EHRLICHKEIT.  Ein nicht messbarer Bandwert wird NIE als Zahl ausgegeben:
// er bekommt Bitmap-Bit 0.  Das gilt fuer drei verschiedene Gruende, und die
// Engine haelt sie auseinander: (a) das Band enthaelt bei dieser Aufloesung
// keinen Bin, (b) das Band liegt ueber der Nyquist-Kappe, (c) das Band hat in
// diesem Rahmen keine Energie gesehen.  Alle drei heissen "keine Aussage" —
// keiner davon heisst "0 dB".
#pragma once

#include "BandGrid.h"
#include "Fft.h"
#include "KGewichtung.h"
#include "../StampedAudioQueue.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace nakama::analyse
{

//==============================================================================
// ── Zeitvertrag ─────────────────────────────────────────────────────────────

/** Warum eine Grenze gezogen wurde.  Das ist Telemetrie UND Beweis: der Golden
    prueft nicht nur DASS getrennt wurde, sondern dass die richtige Ursache
    genannt ist — sonst waere "es hat irgendwie getrennt" die Zusage, und die
    haelt auch ein Fehler ein, der bei jedem Block trennt. */
enum class Grenzgrund : std::uint8_t
{
    keine = 0,
    /** Queue-Drop oder Oversize.  §32.3: trennt das SEGMENT, nicht die Epoche —
        "Die Host-Zeitachse wird dadurch nicht faelschlich als Seek bezeichnet." */
    lokaleLuecke,
    /** `spielt` ist gekippt: Stop oder Start (§32.3 "Stop/Start"). */
    transportKante,
    /** Die Projektzeit ist gesprungen, ohne dass es ein Loop-Wrap war: Seek. */
    zeitSprung,
    /** Rueckwaertssprung auf den Schleifenanfang bei aktiver Schleife. */
    loopWrap,
    /** Die Samplerate hat sich geaendert (§32.3 nennt sie ausdruecklich). */
    sampleratewechsel,
    /** `prepareToPlay` — neuer Anlauf, im Zweifel andere Hostbedingungen. */
    neuanlauf,
    /** Die Schleifengrenze koennte INNERHALB dieses Blocks liegen und die
        Abbildung PPQ→Sample ist fuer diesen Hostlauf nicht bewiesen.  §32.3:
        "wird der moegliche Straddle als ungueltig markiert". */
    moeglicherStraddle,
    /** Die Beweislage selbst hat gewechselt (Kontext kommt oder geht,
        Offline-Render beginnt).  Danach bedeutet dieselbe Zahl etwas anderes. */
    beweislageWechsel,

    /** Kein Grund, sondern die ANZAHL.  ⚠️ Sie steht hier, damit der
        Ursachenzaehler an die Aufzaehlung GEKOPPELT ist statt an eine
        handgeschriebene Zahl daneben.  Die erste Fassung trug
        `grundZaehler[10]` — richtig fuer neun Gruende, und ein zehnter Grund
        haette still ueber das Array hinausgeschrieben, ohne dass irgendetwas
        rot geworden waere (Selbstaudit 23.08., T1 Punkt 1 „Ueberlauf"). */
    anzahl
};

/** Die sieben Gueltigkeitsbits des v3-Transportstempels.  Bitwerte und
    Reihenfolge sind der Vertrag aus `nakama_telemetry_v1.fbs`
    (`enum Gueltigkeit : ubyte (bit_flags)`) — sie duerfen sich nie bewegen.

    ⚠️ `process_context_present` ist BEWUSST KEINES dieser Bits.  Es beschreibt
    nicht die Gueltigkeit EINES Feldes, sondern ob der Host ueberhaupt eine
    Zeitquelle bereitgestellt hat.  Der `.fbs`-Kommentar sagt genau das; hier
    steht es nur, damit niemand es "der Vollstaendigkeit halber" dazunimmt. */
inline constexpr std::uint8_t kGProjectTime     = 1;
inline constexpr std::uint8_t kGPlayState       = 2;
inline constexpr std::uint8_t kGRecordState     = 4;
inline constexpr std::uint8_t kGCycleBounds     = 8;
inline constexpr std::uint8_t kGContinuousTime  = 16;
inline constexpr std::uint8_t kGInputLatency    = 32;
inline constexpr std::uint8_t kGOutputLatency   = 64;
inline constexpr std::uint8_t kGAlleSieben      = 127;

enum class Zeitbasis : std::uint8_t { unbekannt = 0, project_samples = 1, local_monotonic = 2 };
enum class Herleitung : std::uint8_t { unbekannt = 0, validated_block_mapping = 1, unproven = 2 };

/** Gegenstueck zu `table Transportstempel` aus `nakama_telemetry_v1.fbs`.

    Die Feldnamen folgen dem Vertrag, nicht dem Hausstil — wer die `.fbs` neben
    diese Struktur legt, soll Zeile fuer Zeile vergleichen koennen. */
struct Transportstempel
{
    std::uint64_t transport_epoch     { 0 };
    std::uint64_t continuity_segment  { 0 };
    std::uint64_t sequence            { 0 };
    Zeitbasis     zeitbasis           { Zeitbasis::unbekannt };

    bool          project_sample_start_gesetzt { false };
    std::int64_t  project_sample_start { 0 };

    std::uint32_t sample_count        { 0 };
    double        sample_rate         { 0.0 };
    bool          playing             { false };
    bool          recording           { false };

    bool          continuous_time_samples_gesetzt { false };
    std::int64_t  continuous_time_samples { 0 };

    // Schleife (`table Schleife`)
    bool          cycle_active        { false };
    bool          cycle_bounds_valid  { false };
    bool          cycle_start_ppq_gesetzt { false };
    double        cycle_start_ppq     { 0.0 };
    bool          cycle_end_ppq_gesetzt { false };
    double        cycle_end_ppq       { 0.0 };
    /** `table AbgeleiteteGrenzen`.  Solange kein FL-Golden die Abbildung
        PPQ→Sample belegt, ist `unproven` der einzige ehrliche Wert (§32.3;
        Capabilityreport S4 hat kein solches Golden). */
    Herleitung    cycle_derivation    { Herleitung::unbekannt };

    bool          input_presentation_latency_gesetzt  { false };
    std::uint32_t input_presentation_latency  { 0 };
    bool          output_presentation_latency_gesetzt { false };
    std::uint32_t output_presentation_latency { 0 };

    std::uint8_t  gueltigkeit         { 0 };
    bool          process_context_present_gesetzt { false };
    bool          process_context_present { false };

    /** Siehe den Kasten bei `FeatureFrame::operator==` — der Vergleich ist
        memberweise und wird vom Compiler gepflegt, nicht von einer Liste. */
    bool operator== (const Transportstempel&) const = default;
};

//==============================================================================
/** NAK-29 — bedingte Feldpflichten des Transportstempels.

    Der offene Punkt (21.08.2026, Eigentuemer laut eigenem Text: "Gehoert zu
    SONDE-009, wo der Erzeuger dieser Felder entsteht") nennt vier Faelle, die
    das JSON-Schema heute alle durchlaesst, weil eine diskriminierte Union
    dafuer fehlt:

      1. `time_basis=project_samples` + `validity.project_time=true`
         OHNE `project_sample_start`;
      2. `time_basis=local_monotonic` MIT `validity.project_time=true`
         (direkter Widerspruch zu §32.3);
      3. `cycle.bounds_valid=true` ohne `start_ppq`/`end_ppq`;
      4. `cycle.bounds_valid=false` mit `derivation=validated_block_mapping`.

    🔑 DER PUNKT IST NICHT DIE PRUEFUNG, SONDERN IHR ORT.  Ein Riegel im
    Empfaenger faengt einen kaputten Stempel, nachdem er ueber die Leitung
    gegangen ist.  Ein Riegel im ERZEUGER sorgt dafuer, dass er nie entsteht —
    und wenn er doch entsteht, ist der Fehler hier, an einer Stelle, statt
    verteilt ueber jeden Empfaenger, der ihn haette fangen sollen.

    Rueckgabe 0 heisst "in Ordnung"; sonst die Nummer des verletzten Falles
    (1..7).  Eine Nummer statt eines bool, damit der Golden zeigen kann, WELCHE
    Regel gefallen ist — ein Riegel, der nur "nein" sagt, laesst offen, ob er
    aus dem richtigen Grund nein gesagt hat. */
inline int nak29Verstoss (const Transportstempel& t) noexcept
{
    const bool zeitBitAn = (t.gueltigkeit & kGProjectTime) != 0;

    if (t.zeitbasis == Zeitbasis::project_samples && zeitBitAn
        && ! t.project_sample_start_gesetzt)
        return 1;
    if (t.zeitbasis == Zeitbasis::local_monotonic && zeitBitAn)
        return 2;
    if (t.cycle_bounds_valid
        && (! t.cycle_start_ppq_gesetzt || ! t.cycle_end_ppq_gesetzt))
        return 3;
    if (! t.cycle_bounds_valid && t.cycle_derivation == Herleitung::validated_block_mapping)
        return 4;

    // Zwei Faelle mehr, als NAK-29 nennt — sie folgen aus demselben Satz von
    // §32.3 ("Zusaetzlich werden … MIT EIGENEN GUELTIGKEITSBITS gespeichert")
    // und waeren sonst genau dieselbe Luecke an einer anderen Zeile:
    // ein gesetztes Bit ohne den Wert, auf den es sich bezieht.
    if ((t.gueltigkeit & kGCycleBounds) != 0
        && (! t.cycle_start_ppq_gesetzt || ! t.cycle_end_ppq_gesetzt))
        return 5;
    if ((t.gueltigkeit & kGContinuousTime) != 0 && ! t.continuous_time_samples_gesetzt)
        return 6;

    // Ein vom Feature-Erzeuger publiziertes Intervall ist nur dann sinnvoll,
    // wenn es nicht leer ist, seine Vertragsobergrenze haelt und die halboffene
    // Projektgrenze `start + sample_count` noch in int64 passt.  Der allgemeine
    // Wire-Vertrag kann 0 darstellen; dieser Erzeuger baut aber nie Leerframes.
    if (t.sample_count == 0 || t.sample_count > 1048576u
        || (t.zeitbasis == Zeitbasis::project_samples
            && t.project_sample_start_gesetzt
            && t.project_sample_start
                 > std::numeric_limits<std::int64_t>::max()
                     - static_cast<std::int64_t> (t.sample_count)))
        return 7;

    return 0;
}

//==============================================================================
/** Der Entscheid ueber EINEN Frameversuch — und der Grund, warum er als eigene
    Funktion dasteht statt als zwei Zeilen in `baueFrame()`.

    §4.5 verspricht dem Empfaenger einen Meldeweg: "ein Frame mit verletztem
    Stempel wird nicht veroeffentlicht; der Empfaenger sieht die Luecke an der
    springenden `sequence`."  ⚠️ BIS ZUM 24.08. HATTE DER CODE DIESEN WEG NICHT
    (T2-2): `++sequenz` stand HINTER dem Ablehnungszweig, ein abgelehnter Frame
    verbrauchte also gar keine Nummer, und der Empfaenger saehe `1, 2, 3, …`
    ohne Luecke.  Der Frameverlust waere still — und `SONDE-010` haengt den
    ersten echten Leser dort an.

    🔑 UND EIN MELDEWEG BRAUCHT EIN BEIN. Als reine Funktion ist der Entscheid
    mit gezielt kaputten Stempeln fahrbar (B5 §L5). Der Produktionszweig wird
    zusaetzlich durch den gedeckelten Oversize-Versuch erreicht; dessen
    Erholung beweist, dass Ablehnung weder Sequenz noch Evidenz verriegelt. */
struct Frameversuch
{
    std::uint64_t sequence { 0 };   ///< die verbrauchte Nummer — auch bei Verstoss
    int           verstoss { 0 };   ///< 0 = veroeffentlichen, sonst NAK-29-Fallnummer
};

inline Frameversuch frameversuch (std::uint64_t& zaehler,
                                  const Transportstempel& t) noexcept
{
    return { ++zaehler, nak29Verstoss (t) };
}

//==============================================================================
// ── Eventvertrag ────────────────────────────────────────────────────────────

/** Ein `DynamicsEvent` nach §39.1: "Jedes Ereignis traegt Samplezeit, Staerke,
    Bandzentrum, Dauer und Qualitaet."

    `qualitaetFluss` unterscheidet die zwei Wege, die §39.1 nebeneinander
    verlangt: den SuperFlux-artigen spektralen Fluss und "einen einfachen
    Peakpfad als Gegenbeleg fuer sehr kurze Impulse".  Ein Ereignis, das nur
    einer der beiden sieht, ist etwas anderes als eines, das beide sehen — und
    diese Unterscheidung geht verloren, sobald man sie zu einem Bit verdichtet. */
struct Ereignis
{
    /** Lokaler monotoner Strom (immer gueltig) des Fensteranfangs. */
    std::uint64_t stromSample { 0 };
    /** Projektzeit, NUR wenn `projektzeitGesetzt`. */
    bool          projektzeitGesetzt { false };
    std::int64_t  projektSample { 0 };
    /** Epoche und Segment, in denen es gesehen wurde — ein Ereignis ohne diese
        beiden liesse sich spaeter neben eines aus einer anderen Epoche legen. */
    std::uint64_t epoche  { 0 };
    std::uint64_t segment { 0 };

    float staerke      { 0.0f };   // Fluss ueber der adaptiven Schwelle, in MAD
    float bandZentrumHz{ 0.0f };   // Schwerpunkt des positiven Flusses
    float dauerMs      { 0.0f };   // Fensterhop — die Aufloesung, nicht mehr
    bool  qualitaetFluss { false };
    bool  qualitaetPeak  { false };
};

//==============================================================================
// ── Bandvertrag ─────────────────────────────────────────────────────────────

/** Gegenstueck zu `table Bandwerte`.  Fester Speicher: die Groessen stehen im
    Gitter, nicht in der Laufzeit. */
template <int N>
struct Bandsatz
{
    GitterId      gitter   { GitterId::unbekannt };
    BandEncoding encoding  { BandEncoding::unbekannt };
    std::int16_t  werte[N] {};
    std::uint8_t  bitmap[(N + 7) / 8] {};
    bool          saturated { false };

    static constexpr int baender = N;

    void leeren() noexcept
    {
        for (int i = 0; i < N; ++i) werte[i] = 0;
        bitmapNullen (bitmap, N);
        saturated = false;
    }

    /** Siehe den Kasten bei `FeatureFrame::operator==`. */
    bool operator== (const Bandsatz&) const = default;
};

using LiveBaender    = Bandsatz<Gitter::liveBaender>;      // 64
using EvidenzBaender = Bandsatz<Gitter::evidenzBaender>;   // 221

/** Ein Messframe EINER Quelle — Gegenstueck zu `table Frame`.

    Alle Kennzahlen tragen ihr eigenes `…Gesetzt`: das ist die NaN-Ehrlichkeit
    des `.fbs` ("ein nicht messbarer Wert wird WEGGELASSEN, nicht als 0 oder als
    NaN gesendet") in C++-Form.  Ein `float` ohne dieses Bit anzusehen ist ein
    Fehler des Lesers, kein Wert. */
struct FeatureFrame
{
    Transportstempel transport {};
    LiveBaender      live      {};
    EvidenzBaender   evidenz   {};
    bool             evidenzFrisch { false };   // Evidenzkadenz ist langsamer

    /** Band-Stereo (§56): Seitenanteil je LIVE-Band, 0..1 — S/(M+S).
        Bitmap wie bei den Bandwerten: ein Band ohne Bit hat keinen Wert.

        Seit SONDE-011 liegt dieses Feld optional als `Frame.band_stereo`
        (Feld-ID 10, schema_minor 1) auf der Leitung. Der Worker serialisiert
        nur gesetzte Bitmapwerte; der Audiothread bleibt unberuehrt.

        Warum nur auf dem LIVE-Gitter: §33.2 fuehrt "Breite/Korrelation" bei der
        Live-Telemetrie und beim Evidenzsnapshot ausdruecklich NICHT. */
    float        liveBreite[Gitter::liveBaender] {};
    std::uint8_t liveBreiteBitmap[(Gitter::liveBaender + 7) / 8] {};

    std::uint32_t metricsVersion { 0 };

    bool  aktivitaetGesetzt  { false };  float aktivitaet  { 0.0f };
    bool  lufsSGesetzt       { false };  float lufsS       { 0.0f };
    bool  peakGesetzt        { false };  float peakDb      { 0.0f };
    bool  crestGesetzt       { false };  float crestDb     { 0.0f };
    bool  psrGesetzt         { false };  float psrDb       { 0.0f };
    bool  breiteGesetzt      { false };  float breite      { 0.0f };
    bool  korrelationGesetzt { false };  float korrelation { 0.0f };

    /** ZWEI FRAMES SIND GLEICH, WENN JEDES FELD GLEICH IST — und welche Felder
        es gibt, weiss der Compiler, nicht eine Liste (T2R2-1, 24.08.).

        🔑 Das ist die tragende Hälfte der Antwort auf T2R2-1.  Das Bein G13
        (Zwillingsprobe) behauptet: nach einer Grenze ist eine Engine, die
        vorher lautes Audio gesehen hat, von einer ununterscheidbar, die Stille
        gesehen hat.  „Ununterscheidbar" muss dabei ALLE Felder meinen, auch
        die, die es morgen erst gibt — sonst waere das Bein wieder eine Liste,
        die neben `grenzeZiehen()` verrottet, und genau daran sind T2-1 und
        T2R2-1 entstanden.  `= default` erzeugt den memberweisen Vergleich und
        zieht jedes neue Feld automatisch mit hinein.

        ⚠️ WARUM NICHT `memcmp`, obwohl der Typ trivial kopierbar ist — das ist
        GEMESSEN, nicht befuerchtet (24.08.): `FeatureFrame` mischt `bool` und
        `float`/`double` und hat deshalb Fuellbytes (Offsets 26..31, 44..47,
        59..63, 89..95 im Stempel).  Fuellbytes sind unbeobachtbar, also darf
        der Optimierer ihr Nullen weglassen — unter `/O2` standen dort Reste
        eines frueheren Objekts (an 46/47 die oberen Bytes einer `double 1.0`),
        und ein `memcmp` meldete 18 bis 21 Byte Unterschied ohne einen einzigen
        Feldunterschied.  Auch `FeatureFrame f {}` aendert daran nichts
        VERLAESSLICH; es bleibt richtig, aber es ist keine Zusage ueber Bytes.
        Merksatz: EIN FRAME WIRD NACH WERT VERGLICHEN, NIE NACH BYTES — wer ihn
        je hashen oder als Golden einfrieren will, muss ihn vorher feldweise
        serialisieren.

        ⚠️ Die zwei bekannten Grenzen dieses Vergleichs: `NaN != NaN` (ein Frame
        mit NaN gilt als ungleich sich selbst — im Frame ist NaN ohnehin ein
        Fehler, §"NaN-Ehrlichkeit"), und `-0.0 == +0.0`. */
    bool operator== (const FeatureFrame&) const = default;
};

//==============================================================================
/** Versionierte Startwerte.  Aenderung nur ueber eine neue Zahl, nie still —
    dieselbe Regel wie `kMetricsVersion` in `AnalyseEngine`, nur maschinenlesbar,
    weil `table Frame` ein `uint` verlangt. */
inline constexpr std::uint32_t kFeatureMetricsVersion = 20260823u;

//==============================================================================
class FeatureEngine
{
public:
    // ── Feste Groessen (§53.7 Schlussabsatz: Startwerte, keine ABI) ─────────
    /** Bassstufe: aufloesungsbestimmend unter `kTrennungHz`. */
    static constexpr int kBassPunkte = 16384;
    /** Hauptstufe. */
    static constexpr int kHauptPunkte = 4096;
    /** Ueberlappung 50 % — die Hann-Fenster addieren sich damit zu 1. */
    static constexpr int kBassHop  = kBassPunkte / 2;
    static constexpr int kHauptHop = kHauptPunkte / 2;
    /** Zustaendigkeitsgrenze der zwei Stufen. */
    static constexpr double kTrennungHz = 200.0;
    /** Obergrenze der Messbarkeit: 18 kHz ODER 0,95·Nyquist, was kleiner ist.
        Dieselbe Kappe wie in `AnalyseEngine` — darueber bleibt es leer, statt
        eine fortgeschriebene Randevidenz zu behaupten. */
    static constexpr double kObergrenzeHz = 18000.0;
    static constexpr double kNyquistAnteil = 0.95;
    /** Aktivitaetsgate, wie M1. */
    static constexpr double kAktivGateDb = -60.0;
    /** Zellenlaenge der Loudness- und Korrelationsfenster. */
    static constexpr double kZelleSekunden = 0.1;
    /** LUFS-S ueber 3 s = 30 Zellen (§39.1). */
    static constexpr int kKurzZellen = 30;
    /** Livekadenz 10 Hz, Evidenzkadenz 4 Hz (§33.2). */
    static constexpr double kLiveIntervallS    = 0.1;
    static constexpr double kEvidenzIntervallS = 0.25;
    /** Feste Obergrenze des Ereignisstroms (§33 "feste Obergrenzen"). */
    static constexpr int kEreignisPlaetze = 64;
    /** Historie der adaptiven Flussschwelle (Median/MAD, §39.1). */
    static constexpr int kFlussHistorie = 16;

    //== Einrichtung ==========================================================

    /** Legt allen Speicher an.  Idempotent bei gleicher Samplerate; bei einer
        anderen ist es ein Neuanfang — §32.3 fuehrt den Sampleratewechsel
        ausdruecklich als Epochengrenze, und eine Bin-Zuordnung aus der alten
        Rate waere danach schlicht falsch. */
    void vorbereiten (double samplerate)
    {
        if (! std::isfinite (samplerate) || samplerate <= 0.0 || samplerate > 768000.0)
        {
            // Ein ungueltiger Folge-Prepare darf keine zuvor gueltige Engine
            // unter der alten Rate weiterlaufen lassen. Speicher bleibt
            // angelegt; nur Messzustand und Betriebsfreigabe verfallen.
            if (vorbereitet)
                zuruecksetzen();
            vorbereitet = false;
            sr = 0.0;
            return;
        }
        if (samplerate == sr && vorbereitet)
            return;

        sr = samplerate;
        bass.vorbereiten (kBassPunkte, sr);
        haupt.vorbereiten (kHauptPunkte, sr);

        zuordnung (bass,  0,            trennIndex());
        zuordnung (haupt, trennIndex(), Gitter::evidenzBaender);
        auskunftBestimmen();          // erst wenn BEIDE Stufen zugeordnet sind

        zellenSamples = (int) std::llround (kZelleSekunden * sr);
        if (zellenSamples < 1) zellenSamples = 1;
        kurzZellen.assign ((std::size_t) kKurzZellen, 0.0);

        ereignisse.assign ((std::size_t) kEreignisPlaetze, Ereignis {});
        flussHistorie.assign ((std::size_t) kFlussHistorie, 0.0);
        flussSortiert.assign ((std::size_t) kFlussHistorie, 0.0);
        vorigesSpektrum.assign ((std::size_t) Gitter::evidenzBaender, 0.0);

        kL.entwerfen (sr);
        kR.entwerfen (sr);

        vorbereitet = true;
        zuruecksetzen();
    }

    /** Alles auf Anfang — Gegenpfad zu `vorbereiten()`.  Zaehlt NICHT als
        Grenze: wer zuruecksetzt, hat keine Messung mehr, die getrennt werden
        muesste. */
    void zuruecksetzen() noexcept
    {
        bass.leeren();
        haupt.leeren();
        for (auto& v : liveAkku)    v = { 0.0, 0 };
        for (auto& v : evidenzAkku) v = { 0.0, 0 };
        for (auto& v : liveBreiteAkku)    v = { 0.0, 0.0 };
        liveSupport = {};
        evidenzSupport = {};

        zelleStand = 0;
        zelleKEnergie = 0.0;
        zelleAktivEnergie = 0.0;
        kurzStand = 0;
        kurzGefuellt = 0;
        for (auto& z : kurzZellen) z = 0.0;
        kL.zustandNullen();
        kR.zustandNullen();

        rahmenPeak = 0.0;
        rahmenSummeQuadrat = 0.0;
        rahmenSamples = 0;
        rahmenStartBlock = {};
        rahmenProjektDurchgehend = false;
        rahmenContinuousDurchgehend = false;
        rahmenMid2 = rahmenSide2 = 0.0;
        rahmenL = rahmenR = rahmenL2 = rahmenR2 = rahmenLR = 0.0;
        rahmenAktivZellen = 0;
        rahmenZellen = 0;

        liveSamples = 0;
        evidenzSamples = 0;
        evidenzContinuousHabe = false;
        evidenzContinuousDurchgehend = true;
        evidenzContinuousErwartet = 0;
        for (auto& v : vorigesSpektrum) v = 0.0;
        vorigesSpektrumGueltig = false;
        flussStand = 0;
        flussGefuellt = 0;

        ereignisStand = 0;
        ereignisAnzahl = 0;

        habeVorigen = false;
        transportEpoche = 0;
        segmentInEpoche = 0;
        sequenz = 0;
        frameBereit = false;
        aktuell = FeatureFrame {};

        zGetrennteFenster = 0;
        zVerworfeneFensterSamples = 0;
        zEpochenwechsel = 0;
        zSegmentwechsel = 0;
        zStraddleVerworfen = 0;
        zEreignisseVerworfen = 0;
        zNak29Abgelehnt = 0;
        zBloecke = 0;
        zVerworfeneBandfenster = 0;
        for (auto& g : grundZaehler) g = 0;
    }

    //== Betrieb (Workerthread) ===============================================

    /** Nimmt EINEN versiegelten Block auf.

        `interleaved` traegt `2 * block.sampleCount` floats (L/R), genau das,
        was `Blockquarantaene::Freigabe::audio` liefert.

        Rueckgabe true heisst: `frame()` traegt einen neuen, vollstaendigen
        Frame.  Rueckgabe false heisst nur "noch nicht fertig" — nie einen
        Fehler; ein Block, der an einer Grenze faellt, ist kein Fehler, sondern
        die Regel dieses Tickets bei der Arbeit. */
    bool nimmBlock (const echtzeit::StampedBlock& block, const float* interleaved) noexcept
    {
        frameBereit = false;
        if (! vorbereitet || interleaved == nullptr || block.sampleCount == 0)
            return false;
        ++zBloecke;

        // ── 1. Schliesst der Block an? ──────────────────────────────────────
        const Grenzgrund grund = habeVorigen ? grenzeZwischen (vorigerBlock, block)
                                             : Grenzgrund::keine;
        if (grund != Grenzgrund::keine)
            grenzeZiehen (grund);

        // ── 2. Koennte die Schleifengrenze IN diesem Block liegen? ──────────
        // §32.3: bei bewiesener Abbildung wuerde der Block logisch GETEILT;
        // ohne Beweis wird er als moeglicher Straddle verworfen.  Heute gibt es
        // kein FL-Golden fuer die Abbildung (Capabilityreport S4), also gilt
        // immer der zweite Zweig — und das steht hier als Code, nicht als
        // Absicht: sobald `cycle_derivation` je `validated_block_mapping`
        // liefern darf, faellt dieser Zweig von selbst weg.
        if (moeglicherStraddleIn (block))
        {
            grenzeZiehen (Grenzgrund::moeglicherStraddle);
            ++zStraddleVerworfen;
            zVerworfeneFensterSamples += block.sampleCount;
            vorigerBlock = block;
            habeVorigen = true;
            return false;                     // der Block selbst geht nirgends ein
        }

        // ── 3. Samples in alle offenen Fenster ──────────────────────────────
        verarbeiteSamples (block, interleaved);

        vorigerBlock = block;
        habeVorigen = true;

        // ── 4. Kadenz: ist ein Frame faellig? ───────────────────────────────
        const double liveS = (double) liveSamples / sr;
        if (liveS >= kLiveIntervallS)
            frameBereit = baueFrame();

        return frameBereit;
    }

    /** Der zuletzt gebaute Frame.  Nur nach einem `nimmBlock() == true` frisch. */
    const FeatureFrame& frame() const noexcept { return aktuell; }

    /** Die Ereignisse des letzten Rahmens, aeltestes zuerst. */
    int ereignisAnzahlJetzt() const noexcept { return ereignisAnzahl; }
    const Ereignis& ereignis (int i) const noexcept
    {
        const int erstes = (ereignisStand - ereignisAnzahl + kEreignisPlaetze) % kEreignisPlaetze;
        return ereignisse[(std::size_t) ((erstes + i) % kEreignisPlaetze)];
    }

    //== Telemetrie ===========================================================
    // NAK-57: diese Zahlen bekommen in DIESEM Ticket keine Anzeige — die
    // Oberflaeche kommt aus Figma.  Sie sind maschinenlesbar, und das ist die
    // ganze Zusage.

    std::uint64_t getrennteFenster() const noexcept        { return zGetrennteFenster; }
    std::uint64_t verworfeneFensterSamples() const noexcept { return zVerworfeneFensterSamples; }
    std::uint64_t epochenwechsel() const noexcept          { return zEpochenwechsel; }
    std::uint64_t segmentwechsel() const noexcept          { return zSegmentwechsel; }
    std::uint64_t straddleVerworfen() const noexcept       { return zStraddleVerworfen; }
    std::uint64_t ereignisseVerworfen() const noexcept     { return zEreignisseVerworfen; }
    std::uint64_t nak29Abgelehnt() const noexcept          { return zNak29Abgelehnt; }
    std::uint64_t bloeckeGesehen() const noexcept          { return zBloecke; }
    /** Wie oft aus Grund `g` getrennt wurde.

        ⚠️ DER BEREICHSRIEGEL IST NICHT ZIERAT (T2-3, 23.08.).  `grundZaehler`
        hat exakt `anzahl` Elemente, `Grenzgrund::anzahl` ist ein oeffentlich
        sichtbarer Enumwert — und `grenzenMitGrund (Grenzgrund::anzahl)` las
        damit EIN ELEMENT HINTER DEM ENDE, ausgerechnet hinter dem letzten
        Member der Klasse.  Der Selbstaudit-Fix `48fcd9c` hat den SCHREIB-
        Ueberlauf geschlossen (Array an die Aufzaehlung gekoppelt statt an eine
        Zahl daneben) und dabei den LESE-Ueberlauf erst aufgemacht: vorher lag
        `[9]` in einem `[10]`-Array noch im Puffer.  🔑 Dieselbe Sorte Landmine
        wie der Fund, den sie ersetzt hat — eine Zahl neben einer Aufzaehlung,
        an der niemand rot wird. */
    std::uint64_t grenzenMitGrund (Grenzgrund g) const noexcept
    {
        const auto i = (std::size_t) g;
        if (i >= (std::size_t) Grenzgrund::anzahl)
            return 0;                   // `anzahl` ist kein Grund, also null Grenzen
        return grundZaehler[i];
    }
    std::uint64_t transportEpocheJetzt() const noexcept { return transportEpoche; }
    std::uint64_t segmentJetzt() const noexcept         { return segmentInEpoche; }

    /** Fuellstand der zwei Fenster in Samples — der direkte Beleg des
        Gate-Textes: nach einer Grenze steht hier 0, und zwar in BEIDEN. */
    int fuellstandBass() const noexcept  { return bass.gefuellt; }
    int fuellstandHaupt() const noexcept { return haupt.gefuellt; }
    int fuellstandLoudnessZelle() const noexcept { return zelleStand; }
    int fuellstandKurzLoudness() const noexcept  { return kurzGefuellt; }
    bool flussHatVorgaenger() const noexcept     { return vorigesSpektrumGueltig; }

    /** Fuellstand der BANDAKKUS — der Traeger, an dem T2-1 unsichtbar war.

        🔑 DIESE VIER AUSKUENFTE EXISTIEREN WEGEN EINES LOCHS IM EIGENEN BEIN,
        genau wie `kFilterZustand()` unten.  `keinFensterUeberbrueckt()` fragte
        bis zum 24.08. fuenf Fuellstaende ab und KEINEN Akkumulator; deshalb war
        B5 gruen, waehrend ein Frame unter neuem Epochenstempel den Ton von vor
        der Grenze meldete.  Ein Fuellstand ist eben nicht dasselbe wie ein
        Integrationsfenster — wer nur ihn misst, misst die halbe Zusage. */
    int liveAkkuBelegteBaender() const noexcept
    {
        int n = 0;
        for (const auto& v : liveAkku) if (v.n > 0) ++n;
        return n;
    }
    int evidenzAkkuBelegteBaender() const noexcept
    {
        int n = 0;
        for (const auto& v : evidenzAkku) if (v.n > 0) ++n;
        return n;
    }
    /** Betragssumme der Breiten-Akkus — sie tragen KEIN `n`, also braucht es
        die Summe selbst, sonst bliebe dieser dritte Akku ungemessen. */
    double liveBreiteAkkuZustand() const noexcept
    {
        double su = 0.0;
        for (const auto& v : liveBreiteAkku) su += std::abs (v.seite) + std::abs (v.gesamt);
        return su;
    }
    /** Wie viele Baender im Breiten-Akku ueberhaupt etwas tragen.

        🔑 DIESE AUSKUNFT EXISTIERT WEGEN EINER DIAGNOSEZEILE, DIE LOG (T2R2-4,
        24.08.).  `liveBreiteAkkuZustand()` liegt beim Signal der G-Faelle bei
        2,6e-04; mit drei Nachkommastellen gedruckt stand in der Diagnosezeile
        `Breite=0.000` — VOR und NACH der Grenze, also sah der Traeger in jeder
        roten Zeile unauffaellig aus.  Eine Diagnosezeile, die bei einem echten
        Bruch alle Traeger unschuldig aussehen laesst, ist SCHLIMMER als keine:
        sie erzeugt Vertrauen, das sie nicht deckt.  Eine ANZAHL kann nicht auf
        null runden — sie ist entweder 0 oder sie ist es nicht. */
    int liveBreiteAkkuBelegteBaender() const noexcept
    {
        int n = 0;
        for (const auto& v : liveBreiteAkku)
            if (v.seite != 0.0 || v.gesamt != 0.0) ++n;
        return n;
    }
    /** Fertige Zellen des laufenden Rahmens.  `rahmenAktivZellen` kann nie
        groesser sein (beide wachsen in `zelleSchliessen()`, die aktive nur
        bedingt) — 0 hier heisst also 0 in beiden. */
    std::uint64_t rahmenZellenJetzt() const noexcept      { return rahmenZellen; }
    std::uint64_t rahmenAktivZellenJetzt() const noexcept { return rahmenAktivZellen; }

    /** Die zwei KADENZ-Zaehler.  Sie ueberleben eine Grenze absichtlich
        (§10.1: die Grenze schneidet den Inhalt, nicht die Uhr) — und weil das
        ein Entscheid ist und kein Versehen, hat er hier seine Auskunft und in
        B5 seinen eigenen Pruefpunkt.  Ein Entscheid ohne Bein laesst sich
        unbemerkt zuruecknehmen; das ist die teuerste Lehre aus S10-11. */
    std::uint64_t liveSamplesJetzt() const noexcept    { return liveSamples; }
    std::uint64_t evidenzSamplesJetzt() const noexcept { return evidenzSamples; }

    /** Wie viele Band-Fensterbeitraege an Grenzen verworfen wurden.

        Er trennt "der Akku ist leer, weil geleert wurde" von "der Akku ist
        leer, weil nie etwas drin war" — ohne ihn koennte ein Bein, das nur
        `liveAkkuBelegteBaender() == 0` prueft, gruen sein, ohne dass die
        Leerung je gelaufen ist. */
    std::uint64_t verworfeneBandfenster() const noexcept { return zVerworfeneBandfenster; }

    /** Betragssumme der Filterzustände beider K-Ketten.

        🔑 DIESE AUSKUNFT EXISTIERT WEGEN EINES LOCHS IM EIGENEN BEIN.  Der
        Filterzustand ist die subtilste Form der Fensterüberbrückung: er trägt
        Audio von VOR einer Grenze in die Zeit danach, und dabei wächst kein
        Puffer, kein Zähler und kein Füllstand — an den anderen Auskünften ist
        er unsichtbar.  `grenzeZiehen()` nullt ihn seit der ersten Fassung; nur
        gemessen hat das nichts, und eine Zusage ohne Bein ist genau der
        T2-2-Befund aus SONDE-008 („der Riegel ist wirksam — aber von keinem
        Kanon-Bein gedeckt").  Jetzt ist er es. */
    double kFilterZustand() const noexcept
    {
        return std::abs (kL.shelf.z1) + std::abs (kL.shelf.z2)
             + std::abs (kL.hochpass.z1) + std::abs (kL.hochpass.z2)
             + std::abs (kR.shelf.z1) + std::abs (kR.shelf.z2)
             + std::abs (kR.hochpass.z1) + std::abs (kR.hochpass.z2);
    }

    /** Erstes Band, das bei dieser Samplerate ueberhaupt einen Bin traegt, und
        erstes Band ueber der Kappe.  Beides ist Auskunft ueber die GRENZEN der
        Messung, nicht ueber ihr Ergebnis. */
    int erstesMessbaresBand() const noexcept { return erstesBand; }
    int erstesBandUeberKappe() const noexcept { return kappeBand; }
    double samplerate() const noexcept { return sr; }

private:
    //== Fensterstufe =========================================================

    struct Stufe
    {
        int punkte { 0 }, hop { 0 }, gefuellt { 0 };
        double fs { 0.0 };
        std::vector<double> ringM, ringS;      // Mid und Side, `punkte` lang
        // Projekt- und Continuous-Zeit werden PRO SAMPLE mitgeschoben. Ein
        // einzelner Startwert plus blindes `+ hop` waere bei fehlenden oder
        // stehenden Hostwerten eine erfundene Zeitachse: lokal schliesst das
        // Audio an, die jeweilige Hostuhr nachweislich nicht.
        std::vector<std::int64_t> ringProjekt;
        std::vector<std::uint8_t> ringProjektGueltig;
        std::vector<std::int64_t> ringContinuous;
        std::vector<std::uint8_t> ringContinuousGueltig;
        std::vector<double> fenster;           // Hann periodisch
        std::vector<double> arbeit;            // ein Fensterausschnitt
        std::vector<double> psd;               // punkte/2+1
        Fft fftM, fftS;
        double fensterEnergie { 0.0 };
        // Bin-Fenster [von,bis) je Evidenzband; bis<=von heisst "nicht messbar".
        std::vector<int> bandVon, bandBis;
        std::uint64_t fensterStromStart { 0 };
        std::int64_t fensterProjektStart { 0 };
        bool fensterProjektGueltig { false };
        std::int64_t fensterContinuousStart { 0 };
        bool fensterContinuousGueltig { false };

        void vorbereiten (int n, double samplerate)
        {
            punkte = n;
            hop = n / 2;
            fs = samplerate;
            ringM.assign ((std::size_t) n, 0.0);
            ringS.assign ((std::size_t) n, 0.0);
            ringProjekt.assign ((std::size_t) n, 0);
            ringProjektGueltig.assign ((std::size_t) n, 0);
            ringContinuous.assign ((std::size_t) n, 0);
            ringContinuousGueltig.assign ((std::size_t) n, 0);
            arbeit.assign ((std::size_t) n, 0.0);
            psd.assign ((std::size_t) (n / 2 + 1), 0.0);
            fenster.assign ((std::size_t) n, 0.0);
            fensterEnergie = 0.0;
            for (int i = 0; i < n; ++i)
            {
                const double w = 0.5 - 0.5 * std::cos (2.0 * kPi * (double) i / (double) n);
                fenster[(std::size_t) i] = w;
                fensterEnergie += w * w;
            }
            fftM.vorbereiten (n);
            fftS.vorbereiten (n);
            bandVon.assign ((std::size_t) Gitter::evidenzBaender, 0);
            bandBis.assign ((std::size_t) Gitter::evidenzBaender, 0);
            gefuellt = 0;
            fensterStromStart = 0;
            fensterProjektStart = 0;
            fensterProjektGueltig = false;
            fensterContinuousStart = 0;
            fensterContinuousGueltig = false;
        }

        /** Verwirft, was gerade gesammelt wird.  DAS ist die Trennung. */
        void leeren() noexcept
        {
            gefuellt = 0;
            fensterStromStart = 0;
            fensterProjektStart = 0;
            fensterProjektGueltig = false;
            fensterContinuousStart = 0;
            fensterContinuousGueltig = false;
            for (auto& v : ringM) v = 0.0;
            for (auto& v : ringS) v = 0.0;
            for (auto& v : ringProjekt) v = 0;
            for (auto& v : ringProjektGueltig) v = 0;
            for (auto& v : ringContinuous) v = 0;
            for (auto& v : ringContinuousGueltig) v = 0;
        }
    };

    /** Lokaler Beginn und — nur bei lueckenlosem Samplebeweis — die dazu
        gehoerende Projektzeit eines offenen Analysefensters.  Live und
        Evidenz brauchen getrennte Buecher, weil ihre Publikationskadenzen
        verschieden sind. */
    struct Support
    {
        bool gesetzt { false };
        std::uint64_t stromStart { 0 };
        bool projektGueltig { false };
        std::int64_t projektStart { 0 };
        bool continuousGueltig { false };
        std::int64_t continuousStart { 0 };
    };

    static bool projektVorwaerts (std::int64_t start, std::uint64_t delta,
                                  std::int64_t& heraus) noexcept
    {
        if (delta > static_cast<std::uint64_t> (std::numeric_limits<std::int64_t>::max()))
            return false;
        const auto d = static_cast<std::int64_t> (delta);
        if (start > std::numeric_limits<std::int64_t>::max() - d)
            return false;
        heraus = start + d;
        return true;
    }

    /** Vereinigt zwei lokale Anfaenge und erhaelt einen Zeitbeweis nur, wenn
        beide dieselbe affine 1-Sample-Abbildung belegen. */
    static void supportVereinen (Support& ziel, const Support& neu) noexcept
    {
        if (! neu.gesetzt)
            return;
        if (! ziel.gesetzt)
        {
            ziel = neu;
            return;
        }

        const auto achsePasst = [&] (bool zielGueltig, std::int64_t zielStart,
                                     bool neuGueltig, std::int64_t neuStart)
        {
            if (! zielGueltig || ! neuGueltig)
                return false;
            std::int64_t erwartet = 0;
            if (neu.stromStart < ziel.stromStart)
                return projektVorwaerts (neuStart,
                                         ziel.stromStart - neu.stromStart,
                                         erwartet)
                    && erwartet == zielStart;
            return projektVorwaerts (zielStart,
                                     neu.stromStart - ziel.stromStart,
                                     erwartet)
                && erwartet == neuStart;
        };

        const bool projektPasst = achsePasst (ziel.projektGueltig,
                                              ziel.projektStart,
                                              neu.projektGueltig,
                                              neu.projektStart);
        const bool continuousPasst = achsePasst (ziel.continuousGueltig,
                                                 ziel.continuousStart,
                                                 neu.continuousGueltig,
                                                 neu.continuousStart);

        if (neu.stromStart < ziel.stromStart)
        {
            ziel.stromStart = neu.stromStart;
            ziel.projektStart = neu.projektStart;
            ziel.continuousStart = neu.continuousStart;
        }
        ziel.projektGueltig = projektPasst;
        ziel.continuousGueltig = continuousPasst;
    }

    static void supportMerken (Support& ziel, const Stufe& s) noexcept
    {
        supportVereinen (ziel, Support { true, s.fensterStromStart,
                                        s.fensterProjektGueltig,
                                        s.fensterProjektStart,
                                        s.fensterContinuousGueltig,
                                        s.fensterContinuousStart });
    }

    int trennIndex() const noexcept
    {
        // Erstes Band, dessen MITTE >= kTrennungHz liegt.  Ueber die Mitte und
        // nicht ueber eine Kante, damit die Zustaendigkeit eines Bandes nicht
        // davon abhaengt, welche seiner beiden Kanten man ansieht.
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
            if (Gitter::evidenzMitte (b) >= kTrennungHz)
                return b;
        return Gitter::evidenzBaender;
    }

    /** Bin-Fenster je Band, exakt die Konvention von `AnalyseEngine`
        (`ceil(kante*n/fs)`, `bis = max(von,bis)`).  Gleiche Konvention, damit
        "dieses Band ist nicht messbar" in beiden Engines dasselbe heisst — auch
        wenn die Achsen verschieden sind. */
    void zuordnung (Stufe& s, int vonBand, int bisBand)
    {
        const double kappeHz = std::min (kObergrenzeHz, kNyquistAnteil * sr * 0.5);
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            if (b < vonBand || b >= bisBand)
            {
                s.bandVon[(std::size_t) b] = 0;
                s.bandBis[(std::size_t) b] = 0;      // diese Stufe ist nicht zustaendig
                continue;
            }
            const double lo = Gitter::evidenzKante (b);
            const double hi = Gitter::evidenzKante (b + 1);
            if (hi > kappeHz)
            {
                s.bandVon[(std::size_t) b] = 0;
                s.bandBis[(std::size_t) b] = 0;      // ueber der Kappe: nicht messbar
                continue;
            }
            const int von = (int) std::ceil (lo * (double) s.punkte / s.fs);
            int bis = (int) std::ceil (hi * (double) s.punkte / s.fs);
            bis = std::min (bis, s.punkte / 2 + 1);
            s.bandVon[(std::size_t) b] = von;
            s.bandBis[(std::size_t) b] = std::max (von, bis);
        }
    }

    /** Auskunft ueber die GRENZEN der Messung — erst aufrufbar, wenn beide
        Stufen ihre Zuordnung haben.  Stuende das in `zuordnung()`, laese der
        erste Aufruf eine noch leere zweite Stufe und `erstesMessbaresBand()`
        haette bis zum zweiten Aufruf einen falschen Wert dagestanden. */
    void auskunftBestimmen() noexcept
    {
        const double kappeHz = std::min (kObergrenzeHz, kNyquistAnteil * sr * 0.5);

        erstesBand = Gitter::evidenzBaender;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const bool messbar = bass.bandBis[(std::size_t) b] > bass.bandVon[(std::size_t) b]
                              || haupt.bandBis[(std::size_t) b] > haupt.bandVon[(std::size_t) b];
            if (messbar) { erstesBand = b; break; }
        }
        kappeBand = Gitter::evidenzBaender;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
            if (Gitter::evidenzKante (b + 1) > kappeHz) { kappeBand = b; break; }
    }

    //== Grenzerkennung =======================================================

    /** Warum trennt `neu` von `alt`?  `Grenzgrund::keine` heisst: lueckenlose
        Fortsetzung derselben Epoche und desselben Segments.

        🔑 Die REIHENFOLGE der Fragen ist die Aussage.  Ein Queue-Drop und ein
        Seek koennen im selben Blockpaar zusammenfallen; §32.3 will dann NICHT,
        dass die Host-Zeitachse "faelschlich als Seek bezeichnet" wird, und
        umgekehrt darf ein echter Seek nicht als blosse lokale Luecke
        durchgehen.  Deshalb steht der Neuanlauf ganz oben (er macht jede
        Zeitaussage bedeutungslos), dann die Beweislage, dann die echten
        Transportereignisse, und die lokale Luecke ZULETZT: sie ist die
        schwaechste Aussage von allen. */
    Grenzgrund grenzeZwischen (const echtzeit::StampedBlock& alt,
                               const echtzeit::StampedBlock& neu) const noexcept
    {
        if (neu.startFolge != alt.startFolge)
            return Grenzgrund::neuanlauf;

        // Samplerate: ein Wechsel ist §32.3 ausdruecklich eine Epochengrenze.
        // Nur vergleichen, wenn BEIDE sie melden — sonst waere "der Host hat
        // aufgehoert zu melden" ein Sampleratewechsel, und das ist er nicht.
        const bool beideSr = (alt.flags & echtzeit::kFlagSampleRateGueltig) != 0
                          && (neu.flags & echtzeit::kFlagSampleRateGueltig) != 0;
        if (beideSr && alt.sampleRate != neu.sampleRate)
            return Grenzgrund::sampleratewechsel;

        constexpr std::uint32_t beweisMaske = echtzeit::kFlagKontextAnwesend
                                            | echtzeit::kFlagZeitGueltig
                                            | echtzeit::kFlagSpieltGueltig
                                            | echtzeit::kFlagNichtEchtzeit;
        if (((alt.flags ^ neu.flags) & beweisMaske) != 0)
            return Grenzgrund::beweislageWechsel;

        const bool spieltBekannt = (neu.flags & echtzeit::kFlagSpieltGueltig) != 0;
        if (spieltBekannt
            && ((alt.flags & echtzeit::kFlagSpielt) != (neu.flags & echtzeit::kFlagSpielt)))
            return Grenzgrund::transportKante;

        // Zeitsprung.  Dieselbe Lesart wie `Blockquarantaene::schliesstAn` —
        // eine STEHENDE Zeit sagt nichts (FL zerteilt Puffer bis auf 1 Sample
        // und die Teilstuecke tragen dieselbe Projektzeit), eine BEWEGTE Zeit
        // muss sich lueckenlos bewegen.  Der Unterschied zur Quarantaene ist
        // nur, dass hier zusaetzlich die URSACHE benannt wird.
        const bool beideZeit = (alt.flags & echtzeit::kFlagZeitGueltig) != 0
                            && (neu.flags & echtzeit::kFlagZeitGueltig) != 0;
        if (beideZeit && spieltBekannt && neu.projectSampleStart != alt.projectSampleStart)
        {
            const bool ueberlauf = alt.projectSampleStart
                                     > std::numeric_limits<std::int64_t>::max()
                                           - (std::int64_t) alt.sampleCount;
            const std::int64_t erwartet = ueberlauf
                ? std::numeric_limits<std::int64_t>::max()
                : alt.projectSampleStart + (std::int64_t) alt.sampleCount;
            if (ueberlauf || neu.projectSampleStart != erwartet)
                return istLoopWrap (alt, neu) ? Grenzgrund::loopWrap : Grenzgrund::zeitSprung;
        }

        // Lokale Analyseluecke — zuletzt, weil die schwaechste Aussage.
        if ((neu.flags & echtzeit::kFlagLueckeDavor) != 0
            || neu.segment != alt.segment
            || neu.stromVon != alt.stromVon + alt.sampleCount)
            return Grenzgrund::lokaleLuecke;

        return Grenzgrund::keine;
    }

    /** Ein Rueckwaertssprung bei aktiver Schleife, dessen Ziel nicht weiter vom
        Schleifenanfang entfernt liegt als ein Block.

        ⚠️ Ohne bewiesene Sample-Grenzen ist das eine WAHRSCHEINLICHKEITSAUSSAGE,
        keine Messung — und sie aendert am Ergebnis nichts: Loop-Wrap und Seek
        sind BEIDE Epochengrenzen und trennen beide jedes Fenster.  Sie
        unterscheidet nur die Telemetrie.  Genau deshalb darf sie hier stehen,
        ohne einen Beweis zu behaupten, den es nicht gibt. */
    bool istLoopWrap (const echtzeit::StampedBlock& alt,
                      const echtzeit::StampedBlock& neu) const noexcept
    {
        if ((neu.flags & echtzeit::kFlagCycleAktiv) == 0)
            return false;
        if (neu.projectSampleStart >= alt.projectSampleStart)
            return false;                       // vorwaerts: kein Wrap
        if ((neu.flags & echtzeit::kFlagCycleGrenzenGueltig) == 0)
            return true;                        // Schleife an, rueckwaerts: reicht
        const double ppqProSample = ppqProSampleVon (neu);
        if (! (ppqProSample > 0.0))
            return true;
        // Wie weit liegt das Sprungziel hinter dem Schleifenanfang, in PPQ?
        const double zielPpq = neu.ppqPosition;
        if ((neu.flags & echtzeit::kFlagPpqGueltig) == 0)
            return true;
        const double abstand = std::abs (zielPpq - neu.cycleStartPpq);
        return abstand <= ppqProSample * (double) neu.sampleCount;
    }

    /** PPQ je Sample aus Tempo und Samplerate.  0, wenn eines von beiden fehlt —
        dann ist jede Aussage ueber die Lage der Schleifengrenze unmoeglich. */
    static double ppqProSampleVon (const echtzeit::StampedBlock& b) noexcept
    {
        if ((b.flags & echtzeit::kFlagTempoGueltig) == 0) return 0.0;
        if ((b.flags & echtzeit::kFlagSampleRateGueltig) == 0) return 0.0;
        if (! (b.tempo > 0.0) || ! (b.sampleRate > 0.0)) return 0.0;
        return b.tempo / 60.0 / b.sampleRate;    // Viertel je Sekunde / Samples je Sekunde
    }

    /** Liegt die Schleifen-Endgrenze rechnerisch INNERHALB dieses Blocks?

        §32.3: "Loop-Grenzen koennen innerhalb eines Hostblocks liegen.  Bei
        gueltigen, fuer diesen Hostlauf bewiesenen `derived_sample_bounds` wird
        ein solcher Block logisch geteilt.  Liegen nur PPQ-Bounds vor oder
        fehlen die Bounds, wird der moegliche Straddle als ungueltig markiert."

        ⚠️ Fehlen Tempo oder PPQ ganz, wird hier NICHT vorsorglich getrennt.
        Das waere der naheliegende Fehler ("im Zweifel trennen") und es waere
        falsch: bei aktiver Schleife ohne diese Felder traefe es JEDEN Block,
        die Analyse stuerbe waehrend jeder Schleife vollstaendig.  §32.3 sieht
        fuer genau diesen Fall den anderen Weg vor — "spaetestens beim Erkennen
        des Wraps im Folgeblock eine neue Epoche" —, und den faehrt
        `grenzeZwischen()` ueber den Zeitsprung.  Dieselbe Lehre wie T2-4 in
        SONDE-008: naeher am Wortlaut ist nicht automatisch naeher an der
        Absicht. */
    bool moeglicherStraddleIn (const echtzeit::StampedBlock& b) const noexcept
    {
        if ((b.flags & echtzeit::kFlagCycleAktiv) == 0) return false;
        if ((b.flags & echtzeit::kFlagCycleGrenzenGueltig) == 0) return false;
        if ((b.flags & echtzeit::kFlagPpqGueltig) == 0) return false;
        const double ppqProSample = ppqProSampleVon (b);
        if (! (ppqProSample > 0.0)) return false;

        const double von = b.ppqPosition;
        const double bis = von + ppqProSample * (double) b.sampleCount;
        // Halboffen [von, bis): liegt das Ende exakt auf `von`, hat der Wrap
        // bereits stattgefunden und `grenzeZwischen` hat ihn gesehen.
        return b.cycleEndePpq > von && b.cycleEndePpq < bis;
    }

    /** DIE Stelle, an der der Gate-Text zu Code wird.

        Jedes offene Fenster faellt.  Nicht eines, nicht die langen, nicht die,
        die schon halb voll sind — alle.  Die Aufzaehlung steht bewusst
        vollstaendig und ohne Sammelschleife da: wer spaeter ein Fenster
        hinzufuegt, soll an dieser Liste vorbeikommen und merken, dass er es
        eintragen muss.

        ⚠️ GENAU DAS HAT BEIM ERSTEN MAL NICHT FUNKTIONIERT.  Die Liste war
        unvollstaendig (T2-1, 23.08.): sieben Zustandstraeger standen nicht
        darauf, darunter die drei Bandakkus.  Ein Kommentar, der Vollstaendigkeit
        BEHAUPTET, ersetzt keinen Riegel, der sie MISST — deshalb hat jeder
        Traeger hier heute eine Auskunft (`liveAkkuBelegteBaender()` und
        Nachbarn), und `keinFensterUeberbrueckt()` in B5 fragt sie alle ab
        statt nur die fuenf Fuellstaende, an denen der Bruch unsichtbar war. */
    void grenzeZiehen (Grenzgrund grund) noexcept
    {
        ++zGetrennteFenster;
        ++grundZaehler[(std::size_t) grund];

        zVerworfeneFensterSamples += (std::uint64_t) bass.gefuellt
                                   + (std::uint64_t) haupt.gefuellt
                                   + (std::uint64_t) zelleStand;

        bass.leeren();                  // FFT-Fenster, Bassstufe
        haupt.leeren();                 // FFT-Fenster, Hauptstufe

        // ── Die Bandakkus: das INTEGRATIONSFENSTER des Bandwertes ────────────
        //
        // 🔑 T2-1 (23.08.): DIESE DREI STANDEN NICHT AUF DER LISTE, und deshalb
        // meldete ein Frame unter dem Stempel der NEUEN Epoche den Ton von VOR
        // der Grenze — 23 Live-Baender, staerkstes bei 1029 Hz mit -23,7 dB,
        // obwohl danach nur noch digitale Stille lief.  Die Fuellstaende sagten
        // dabei korrekt "getrennt": ein geleerter FFT-Ring ist eben nicht
        // dasselbe wie ein geleertes Integrationsfenster.  Der veroeffentlichte
        // Bandwert IST das FFT-Ergebnis, ueber das Frameintervall linear
        // integriert (§33.1) — also ist der Akku ein Fenster wie jedes andere,
        // und der Gate-Text ("trennt JEDES offene Fenster") meint ihn mit.
        for (auto& v : liveAkku)
        {
            zVerworfeneBandfenster += v.n;
            v = { 0.0, 0 };
        }
        for (auto& v : evidenzAkku)
        {
            zVerworfeneBandfenster += v.n;
            v = { 0.0, 0 };
        }
        for (auto& v : liveBreiteAkku) v = { 0.0, 0.0 };
        liveSupport = {};
        evidenzSupport = {};
        evidenzContinuousHabe = false;
        evidenzContinuousDurchgehend = true;
        evidenzContinuousErwartet = 0;

        // Aktivitaetszaehler: `zelleStand` (die angefangene Zelle) faellt schon
        // seit der ersten Fassung, die FERTIGEN Zellen des laufenden Rahmens
        // fielen nicht — `aktivitaet` war damit ein Anteil ueber Zellen aus
        // zwei Epochen.  Dieselbe Auslassung, andere Zeile.
        rahmenAktivZellen = 0;
        rahmenZellen = 0;

        // ⚠️ `liveSamples` und `evidenzSamples` bleiben BEWUSST stehen —
        // Entwurfsentscheid des Erbauers, Begruendung im Manifest §10.1.
        // Kurzfassung: DIE GRENZE SCHNEIDET DEN INHALT, NICHT DIE UHR.  Die
        // beiden Zaehler sind kein Messwert, sondern der Fahrplan (sie werden
        // nirgends sonst gelesen als von den zwei Kadenzabfragen).  Wer sie hier
        // mitnullte, liesse eine Folge dichter Grenzen — Queue-Drops unter Last,
        // eine enge Schleife — die Telemetrie VOLLSTAENDIG verstummen lassen;
        // genau die Todesart, die §4.4 fuer den Straddle schon einmal
        // ausdruecklich verworfen hat ("die Analyse stuerbe waehrend jeder
        // Schleife").  Ein Frame, der kurz nach einer Grenze faellig wird,
        // traegt stattdessen WENIGER oder GAR KEINE Baender — und dass ein Band
        // nichts sagt, statt etwas Falsches zu sagen, ist ueber die Bitmap
        // ehrlich ausdrueckbar ("gemeldete 0" != "nie gesagt").
        zelleStand = 0;                 // Loudness-Zelle, angefangen
        zelleKEnergie = 0.0;
        zelleAktivEnergie = 0.0;
        kurzStand = 0;                  // LUFS-S ueber 3 s — die ganze Historie
        kurzGefuellt = 0;
        for (auto& z : kurzZellen) z = 0.0;
        // Die K-Filter behalten ihren Zustand NICHT: ihr Nachklang traegt Audio
        // von VOR der Grenze, und genau das ist Ueberbrueckung — nur eben
        // ueber den Filterzustand statt ueber einen Puffer.  Das ist die
        // subtilste Form des Fehlers, gegen den dieses Ticket schuetzt.
        kL.zustandNullen();
        kR.zustandNullen();

        rahmenPeak = 0.0;               // Korrelations-/Peakfenster
        rahmenSummeQuadrat = 0.0;
        rahmenSamples = 0;
        rahmenStartBlock = {};
        rahmenProjektDurchgehend = false;
        rahmenContinuousDurchgehend = false;
        rahmenMid2 = rahmenSide2 = 0.0;
        rahmenL = rahmenR = rahmenL2 = rahmenR2 = rahmenLR = 0.0;

        vorigesSpektrumGueltig = false; // Fluss: kein Vorgaenger ueber die Grenze
        flussStand = 0;                 // und keine Schwelle aus der alten Epoche
        flussGefuellt = 0;

        if (grund == Grenzgrund::lokaleLuecke)
        {
            ++segmentInEpoche;          // §32.3: SEGMENT, nicht Epoche
            ++zSegmentwechsel;
        }
        else
        {
            ++transportEpoche;
            segmentInEpoche = 0;        // Segmente zaehlen INNERHALB einer Epoche
            ++zEpochenwechsel;
        }
    }

    //== Samples =============================================================

    static bool blockProjektSpanneGueltig (const echtzeit::StampedBlock& b) noexcept
    {
        const auto muss = echtzeit::kFlagZeitGueltig
                        | echtzeit::kFlagSpieltGueltig
                        | echtzeit::kFlagSpielt;
        if ((b.flags & muss) != muss)
            return false;
        return b.projectSampleStart
            <= std::numeric_limits<std::int64_t>::max()
                 - static_cast<std::int64_t> (b.sampleCount);
    }

    static bool blockContinuousSpanneGueltig (const echtzeit::StampedBlock& b) noexcept
    {
        if ((b.flags & echtzeit::kFlagContinuousGueltig) == 0)
            return false;
        return b.continuousTimeSamples
            <= std::numeric_limits<std::int64_t>::max()
                 - static_cast<std::int64_t> (b.sampleCount);
    }

    void evidenzContinuousBelegen (const echtzeit::StampedBlock& block) noexcept
    {
        const bool gueltig = blockContinuousSpanneGueltig (block);
        if (! evidenzContinuousHabe)
        {
            evidenzContinuousHabe = true;
            evidenzContinuousDurchgehend = gueltig;
        }
        else if (evidenzContinuousDurchgehend)
        {
            evidenzContinuousDurchgehend
                = gueltig && block.continuousTimeSamples == evidenzContinuousErwartet;
        }

        std::int64_t ende = 0;
        if (evidenzContinuousDurchgehend
            && projektVorwaerts (block.continuousTimeSamples,
                                 block.sampleCount, ende))
            evidenzContinuousErwartet = ende;
        else
            evidenzContinuousDurchgehend = false;
    }

    void rahmenZeitBelegen (const echtzeit::StampedBlock& block) noexcept
    {
        if (rahmenSamples == 0)
        {
            rahmenStartBlock = block;
            rahmenProjektDurchgehend = blockProjektSpanneGueltig (block);
            rahmenContinuousDurchgehend = blockContinuousSpanneGueltig (block);
            return;
        }

        if (rahmenProjektDurchgehend && blockProjektSpanneGueltig (block))
        {
            std::int64_t erwartet = 0;
            rahmenProjektDurchgehend = projektVorwaerts (
                                          rahmenStartBlock.projectSampleStart,
                                          rahmenSamples, erwartet)
                                    && erwartet == block.projectSampleStart;
        }
        else
            rahmenProjektDurchgehend = false;

        if (rahmenContinuousDurchgehend && blockContinuousSpanneGueltig (block))
        {
            std::int64_t erwartet = 0;
            rahmenContinuousDurchgehend = projektVorwaerts (
                                             rahmenStartBlock.continuousTimeSamples,
                                             rahmenSamples, erwartet)
                                       && erwartet == block.continuousTimeSamples;
        }
        else
            rahmenContinuousDurchgehend = false;
    }

    void verarbeiteSamples (const echtzeit::StampedBlock& block, const float* daten) noexcept
    {
        const int n = (int) block.sampleCount;
        const bool stereo = block.kanaele > 1;

        // Der Transportstempel beschreibt den GESAMTEN publizierten Rahmen,
        // nicht bloss den letzten Block, der seine Kadenzschwelle ueberschritt.
        evidenzContinuousBelegen (block);
        rahmenZeitBelegen (block);

        for (int i = 0; i < n; ++i)
        {
            double l = (double) daten[(std::size_t) i * 2u];
            double r = (double) daten[(std::size_t) i * 2u + 1u];
            // NaN-Riegel: nicht-endliche Eingangswerte werden VOR jeder Rechnung
            // durch Stille ersetzt.  Das Audio sieht die Engine ohnehin nie.
            if (! std::isfinite (l)) l = 0.0;
            if (! std::isfinite (r)) r = 0.0;
            if (! stereo) r = l;

            const double m = 0.5 * (l + r);
            const double s = 0.5 * (l - r);

            schiebeStufe (bass, m, s, block, i);
            schiebeStufe (haupt, m, s, block, i);

            // Loudness: K-Gewichtung je Kanal, Energie in 100-ms-Zellen.
            //
            // ⚠️ BS.1770 SUMMIERT die Kanäle mit ihrem Gewicht (G_L = G_R = 1),
            // es mittelt sie NICHT: z = Σ_i G_i · mean(y_i²).  Ein Mono-Signal
            // trägt deshalb genau EINEN Summanden — dieselbe Wellenform auf
            // beiden Kanälen ist nach der Norm 3 dB lauter als auf einem, und
            // das ist kein Rechenfehler, sondern die Aussage.  Wer hier durch
            // die Kanalzahl teilt, bekommt für Stereo eine um 3,01 LU zu leise
            // Zahl, die trotzdem völlig plausibel aussieht.
            const double kl = kL.tick (l);
            zelleKEnergie += kl * kl;
            if (stereo)
            {
                const double kr = kR.tick (r);
                zelleKEnergie += kr * kr;
            }
            zelleAktivEnergie += 0.5 * (l * l + r * r);

            // Peak/Crest/Stereo im laufenden Rahmen.
            const double absL = std::abs (l), absR = std::abs (r);
            rahmenPeak = std::max (rahmenPeak, std::max (absL, absR));
            rahmenSummeQuadrat += 0.5 * (l * l + r * r);
            ++rahmenSamples;
            rahmenMid2 += m * m;
            rahmenSide2 += s * s;
            rahmenL += l; rahmenR += r;
            rahmenL2 += l * l; rahmenR2 += r * r; rahmenLR += l * r;

            if (++zelleStand >= zellenSamples)
            {
                zelleSchliessen();
                zelleStand = 0;
                zelleKEnergie = 0.0;
                zelleAktivEnergie = 0.0;
            }
        }

        liveSamples    += (std::uint64_t) n;
        evidenzSamples += (std::uint64_t) n;
    }

    void schiebeStufe (Stufe& s, double m, double side,
                       const echtzeit::StampedBlock& block, int sampleOffset) noexcept
    {
        if (s.gefuellt == 0)
            s.fensterStromStart = block.stromVon + static_cast<std::uint64_t> (sampleOffset);
        const bool projektGueltig = blockProjektSpanneGueltig (block);
        const bool continuousGueltig = blockContinuousSpanneGueltig (block);
        const auto index = (std::size_t) s.gefuellt;
        s.ringM[(std::size_t) s.gefuellt] = m;
        s.ringS[(std::size_t) s.gefuellt] = side;
        s.ringProjektGueltig[index] = projektGueltig ? 1u : 0u;
        s.ringProjekt[index] = projektGueltig
            ? block.projectSampleStart + static_cast<std::int64_t> (sampleOffset)
            : 0;
        s.ringContinuousGueltig[index] = continuousGueltig ? 1u : 0u;
        s.ringContinuous[index] = continuousGueltig
            ? block.continuousTimeSamples + static_cast<std::int64_t> (sampleOffset)
            : 0;
        if (++s.gefuellt < s.punkte)
            return;

        // Der Fensterstart ist nur dann Projektzeit, wenn JEDES Sample im Ring
        // dieselbe fortlaufende Achse belegt.  Das ist absichtlich O(N) pro
        // FFT-Fenster und laeuft auf dem Worker; die FFT selbst ist teurer und
        // der Beweis darf nicht aus einer Host-Heuristik bestehen.
        s.fensterProjektGueltig = s.ringProjektGueltig[0] != 0;
        if (s.fensterProjektGueltig)
        {
            s.fensterProjektStart = s.ringProjekt[0];
            for (int i = 1; i < s.punkte; ++i)
            {
                const auto vorher = s.ringProjekt[(std::size_t) (i - 1)];
                if (s.ringProjektGueltig[(std::size_t) i] == 0
                    || vorher == std::numeric_limits<std::int64_t>::max()
                    || s.ringProjekt[(std::size_t) i] != vorher + 1)
                {
                    s.fensterProjektGueltig = false;
                    break;
                }
            }
        }

        s.fensterContinuousGueltig = s.ringContinuousGueltig[0] != 0;
        if (s.fensterContinuousGueltig)
        {
            s.fensterContinuousStart = s.ringContinuous[0];
            for (int i = 1; i < s.punkte; ++i)
            {
                const auto vorher = s.ringContinuous[(std::size_t) (i - 1)];
                if (s.ringContinuousGueltig[(std::size_t) i] == 0
                    || vorher == std::numeric_limits<std::int64_t>::max()
                    || s.ringContinuous[(std::size_t) i] != vorher + 1)
                {
                    s.fensterContinuousGueltig = false;
                    break;
                }
            }
        }

        rechneFenster (s);

        // 50 % Ueberlappung: die zweite Haelfte wird die erste.
        for (int i = 0; i < s.punkte - s.hop; ++i)
        {
            s.ringM[(std::size_t) i] = s.ringM[(std::size_t) (i + s.hop)];
            s.ringS[(std::size_t) i] = s.ringS[(std::size_t) (i + s.hop)];
            s.ringProjekt[(std::size_t) i] = s.ringProjekt[(std::size_t) (i + s.hop)];
            s.ringProjektGueltig[(std::size_t) i]
                = s.ringProjektGueltig[(std::size_t) (i + s.hop)];
            s.ringContinuous[(std::size_t) i]
                = s.ringContinuous[(std::size_t) (i + s.hop)];
            s.ringContinuousGueltig[(std::size_t) i]
                = s.ringContinuousGueltig[(std::size_t) (i + s.hop)];
        }
        s.gefuellt = s.punkte - s.hop;
        s.fensterStromStart += static_cast<std::uint64_t> (s.hop);
        // Bis zum naechsten vollen Ring gibt es noch keinen neuen Beweis.
        s.fensterProjektGueltig = false;
        s.fensterProjektStart = 0;
        s.fensterContinuousGueltig = false;
        s.fensterContinuousStart = 0;
    }

    void rechneFenster (Stufe& s) noexcept
    {
        // Mid.
        for (int i = 0; i < s.punkte; ++i)
            s.arbeit[(std::size_t) i] = s.ringM[(std::size_t) i] * s.fenster[(std::size_t) i];
        s.fftM.transformiere (s.arbeit.data());
        // Side.
        for (int i = 0; i < s.punkte; ++i)
            s.arbeit[(std::size_t) i] = s.ringS[(std::size_t) i] * s.fenster[(std::size_t) i];
        s.fftS.transformiere (s.arbeit.data());

        // Kanalenergie-Mittel: (|L|² + |R|²)/2 = |M|² + |S|².  Ein reiner
        // Mono-Mix loeschte Antiphase komplett aus (Uebergabe-Paket B) — dieselbe
        // Falle, die `AnalyseEngine` im Kopf beschreibt.
        const double norm = 2.0 / (s.fs * s.fensterEnergie);
        const int bins = s.punkte / 2 + 1;
        for (int k = 0; k < bins; ++k)
            s.psd[(std::size_t) k] = (s.fftM.leistung (k) + s.fftS.leistung (k)) * norm;

        const double gesamt = summeBereich (s, 0, bins);
        const bool aktiv = gesamt > 0.0
                        && 10.0 * std::log10 (gesamt) > kAktivGateDb;

        // Nur AKTIVE Fenster gehen in die Bandakkus.  Stille wuerde den
        // Mittelwert zu einer Aussage ueber die Pausen machen.
        if (! aktiv)
            return;

        bool hatBandBeitrag = false;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const int von = s.bandVon[(std::size_t) b];
            const int bis = s.bandBis[(std::size_t) b];
            if (bis <= von)
                continue;                       // Stufe nicht zustaendig / nicht messbar

            double energie = 0.0, seite = 0.0;
            for (int k = von; k < bis; ++k)
            {
                energie += s.psd[(std::size_t) k];
                seite   += s.fftS.leistung (k) * (2.0 / (s.fs * s.fensterEnergie));
            }
            energie /= (double) (bis - von);
            seite   /= (double) (bis - von);

            // §33.1: LINEAR integrieren, erst danach in dB.
            liveAkku[(std::size_t) b].summe += energie;
            ++liveAkku[(std::size_t) b].n;
            evidenzAkku[(std::size_t) b].summe += energie;
            ++evidenzAkku[(std::size_t) b].n;
            liveBreiteAkku[(std::size_t) b].seite += seite;
            liveBreiteAkku[(std::size_t) b].gesamt += energie;
            hatBandBeitrag = true;
        }

        if (hatBandBeitrag)
        {
            supportMerken (liveSupport, s);
            supportMerken (evidenzSupport, s);
        }

        // Ereignisse nur aus der HAUPTstufe: sie hat die zeitliche Aufloesung
        // (85 ms Hop gegen 341 ms).  Ein Onset aus dem Bassfenster waere ein
        // Ereignis mit einer Dauer, die groesser ist als der Abstand zweier
        // Ereignisse — das ist keine Detektion mehr.
        if (&s == &haupt)
            flussSchritt (s);
    }

    static double summeBereich (const Stufe& s, int von, int bis) noexcept
    {
        double su = 0.0;
        for (int k = von; k < bis; ++k)
            su += s.psd[(std::size_t) k];
        return su;
    }

    //== Ereignisse ===========================================================

    /** SuperFlux-artiger spektraler Fluss (§39.1) mit adaptiver Median/MAD-
        Schwelle, plus einfachem Peakpfad als Gegenbeleg.

        🔑 Der Vorgaenger (`vorigesSpektrum`) ist genau die Groesse, die eine
        Grenze ueberbruecken WUERDE, wenn man ihn stehen liesse — und zwar
        unsichtbar, weil kein Puffer dabei waechst.  `grenzeZiehen()` setzt ihn
        deshalb ungueltig.  Der Golden prueft das eigens: ein Fluss, der ueber
        einen Seek hinweg gerechnet wird, meldet einen Onset, den es nicht gab. */
    void flussSchritt (Stufe& s) noexcept
    {
        double fluss = 0.0, zentrumZaehler = 0.0, zentrumNenner = 0.0;
        const bool hatteVorgaenger = vorigesSpektrumGueltig;

        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const int von = s.bandVon[(std::size_t) b];
            const int bis = s.bandBis[(std::size_t) b];
            double energie = 0.0;
            if (bis > von)
            {
                for (int k = von; k < bis; ++k)
                    energie += s.psd[(std::size_t) k];
                energie /= (double) (bis - von);
            }
            const double logJetzt = energie > 0.0 ? std::log10 (energie) : -30.0;
            if (hatteVorgaenger)
            {
                const double delta = logJetzt - vorigesSpektrum[(std::size_t) b];
                if (delta > 0.0)
                {
                    fluss += delta;
                    zentrumZaehler += delta * Gitter::evidenzMitte (b);
                    zentrumNenner  += delta;
                }
            }
            vorigesSpektrum[(std::size_t) b] = logJetzt;
        }
        vorigesSpektrumGueltig = true;
        if (! hatteVorgaenger)
            return;                     // erster Rahmen nach einer Grenze: kein Fluss

        // Adaptive Schwelle: Median + 3·MAD ueber die Historie.  Erst ab voller
        // Historie — eine Schwelle aus drei Werten ist keine Schwelle, und ein
        // Detektor, der am Anfang jeder Epoche wild feuert, waere genau das
        // Gegenteil dessen, was dieses Ticket verspricht.
        if (flussGefuellt >= kFlussHistorie)
        {
            const double med = medianDerHistorie();
            double madSumme = 0.0;
            for (int i = 0; i < kFlussHistorie; ++i)
                madSumme += std::abs (flussHistorie[(std::size_t) i] - med);
            const double mad = madSumme / (double) kFlussHistorie;
            const double schwelle = med + 3.0 * mad;

            const double peakDb = rahmenPeak > 0.0 ? 20.0 * std::log10 (rahmenPeak) : -200.0;
            const double rms = rahmenSamples > 0
                ? std::sqrt (rahmenSummeQuadrat / (double) rahmenSamples) : 0.0;
            const double rmsDb = rms > 0.0 ? 20.0 * std::log10 (rms) : -200.0;
            const bool peakPfad = (peakDb - rmsDb) > 12.0;   // Crest im Rahmen

            if (fluss > schwelle && mad > 0.0)
            {
                Ereignis e;
                e.stromSample = s.fensterStromStart;
                e.projektzeitGesetzt = s.fensterProjektGueltig;
                e.projektSample = s.fensterProjektStart;
                e.epoche  = transportEpoche;
                e.segment = segmentInEpoche;
                e.staerke = (float) ((fluss - med) / mad);
                e.bandZentrumHz = zentrumNenner > 0.0
                    ? (float) (zentrumZaehler / zentrumNenner) : 0.0f;
                e.dauerMs = (float) (1000.0 * (double) s.hop / s.fs);
                e.qualitaetFluss = true;
                e.qualitaetPeak = peakPfad;
                ereignisAblegen (e);
            }
        }

        flussHistorie[(std::size_t) flussStand] = fluss;
        flussStand = (flussStand + 1) % kFlussHistorie;
        if (flussGefuellt < kFlussHistorie) ++flussGefuellt;
    }

    double medianDerHistorie() noexcept
    {
        for (int i = 0; i < kFlussHistorie; ++i)
            flussSortiert[(std::size_t) i] = flussHistorie[(std::size_t) i];
        std::sort (flussSortiert.begin(), flussSortiert.end());
        // Gerade Anzahl: Mittel der beiden mittleren.  Der Median ist hier eine
        // Schwellenbasis, keine Kennzahl — die genaue Konvention ist egal,
        // solange sie EINE ist.
        return 0.5 * (flussSortiert[(std::size_t) (kFlussHistorie / 2 - 1)]
                    + flussSortiert[(std::size_t) (kFlussHistorie / 2)]);
    }

    /** Fester Ring, drop-oldest.  Ein Ereignisstrom, der bei Ueberlast waechst,
        waere ein unbegrenzter Vektor mit anderem Namen (§48.1). */
    void ereignisAblegen (const Ereignis& e) noexcept
    {
        if (ereignisAnzahl == kEreignisPlaetze)
            ++zEreignisseVerworfen;               // das aelteste faellt
        ereignisse[(std::size_t) ereignisStand] = e;
        ereignisStand = (ereignisStand + 1) % kEreignisPlaetze;
        if (ereignisAnzahl < kEreignisPlaetze) ++ereignisAnzahl;
    }

    //== Loudness =============================================================

    void zelleSchliessen() noexcept
    {
        // `zelleKEnergie` ist bereits die Kanal-SUMME (s. o.); geteilt wird nur
        // durch die Samplezahl, damit aus Σ y² das mean(y²) der Norm wird.
        const double mittel = zelleKEnergie / (double) zellenSamples;
        kurzZellen[(std::size_t) kurzStand] = mittel;
        kurzStand = (kurzStand + 1) % kKurzZellen;
        if (kurzGefuellt < kKurzZellen) ++kurzGefuellt;

        const double aktivMittel = zelleAktivEnergie / (double) zellenSamples;
        if (aktivMittel > 0.0 && 10.0 * std::log10 (aktivMittel) > kAktivGateDb)
            ++rahmenAktivZellen;
        ++rahmenZellen;
    }

    /** LUFS-S = −0,691 + 10·log10(Σ Zellenergie / n) ueber die letzten 3 s.
        Nur bei VOLLER Historie — eine Kurzzeitlautheit ueber 0,4 s ist keine
        Kurzzeitlautheit, sie ist eine andere Zahl mit demselben Namen. */
    bool kurzLufs (double& heraus) const noexcept
    {
        if (kurzGefuellt < kKurzZellen)
            return false;
        double su = 0.0;
        for (int i = 0; i < kKurzZellen; ++i)
            su += kurzZellen[(std::size_t) i];
        const double mittel = su / (double) kKurzZellen;
        if (! (mittel > 0.0))
            return false;
        heraus = -0.691 + 10.0 * std::log10 (mittel);
        return std::isfinite (heraus);
    }

    //== Frame bauen ==========================================================

    bool baueFrame() noexcept
    {
        // Wert-Initialisierung statt Default-Initialisierung: sie nullt das
        // ganze Objekt, bevor die NSDMIs greifen.  ⚠️ Das ist die richtige
        // Vorgabe, aber KEINE Zusage ueber Fuellbytes — der Optimierer darf ihr
        // Nullen weglassen, weil sie unbeobachtbar sind, und unter `/O2` tut er
        // es (gemessen 24.08., Einzelheiten im Kasten bei
        // `FeatureFrame::operator==`).  Ein Frame wird deshalb NACH WERT
        // verglichen, nie nach Bytes.
        FeatureFrame f {};
        f.metricsVersion = kFeatureMetricsVersion;
        const double evidenzS = (double) evidenzSamples / sr;
        f.evidenzFrisch = evidenzS >= kEvidenzIntervallS;
        f.transport = baueStempel (rahmenStartBlock, f.evidenzFrisch);

        // NAK-29: ein Stempel, der die Feldpflichten verletzt, wird NICHT
        // veroeffentlicht.  Lieber kein Frame als ein Frame, dessen Zeitangabe
        // sich selbst widerspricht — ein Empfaenger kann den fehlenden Frame
        // sehen (die `sequence` springt), einen widerspruechlichen nicht.
        //
        // ⚠️ DIE NUMMER FAELLT VOR DER PRUEFUNG, nicht danach (T2-2).  Genau
        // darin besteht der Meldeweg: ein abgelehnter Versuch verbraucht seine
        // Sequenznummer, und die Luecke ist die Nachricht.
        const auto versuch = frameversuch (sequenz, f.transport);
        if (versuch.verstoss != 0)
        {
            ++zNak29Abgelehnt;
            rahmenLeeren();
            // Ein faelliger Evidenzsnapshot ist mit diesem Versuch verbraucht,
            // auch wenn dessen Stempel am Erzeugerriegel scheitert.  Bliebe
            // sein fruehester Support stehen, waere derselbe zu grosse
            // Zeitbereich im naechsten Versuch wieder enthalten: ein einmaliger
            // Vertragsverstoss wuerde die Telemetrie dauerhaft verriegeln.
            if (f.evidenzFrisch)
                evidenzLeeren();
            return false;
        }

        f.live.gitter    = GitterId::nakama_log64_v1;
        f.live.encoding  = BandEncoding::q_db_0p1_i16;
        fuelleLive (f.live, f.liveBreite, f.liveBreiteBitmap);

        f.evidenz.gitter   = GitterId::nakama_1_24_oct_30_18k_v1;
        f.evidenz.encoding = BandEncoding::q_db_0p01_i16;
        if (f.evidenzFrisch)
            fuelleEvidenz (f.evidenz);
        else
            f.evidenz.leeren();

        fuelleSkalare (f);

        f.transport.sequence = versuch.sequence;
        aktuell = f;

        rahmenLeeren();
        if (f.evidenzFrisch)
            evidenzLeeren();
        return true;
    }

    void evidenzLeeren() noexcept
    {
        for (auto& v : evidenzAkku) v = { 0.0, 0 };
        evidenzSamples = 0;
        evidenzSupport = {};
        evidenzContinuousHabe = false;
        evidenzContinuousDurchgehend = true;
        evidenzContinuousErwartet = 0;
    }

    void rahmenLeeren() noexcept
    {
        for (auto& v : liveAkku) v = { 0.0, 0 };
        for (auto& v : liveBreiteAkku) v = { 0.0, 0.0 };
        liveSupport = {};
        liveSamples = 0;
        rahmenPeak = 0.0;
        rahmenSummeQuadrat = 0.0;
        rahmenSamples = 0;
        rahmenStartBlock = {};
        rahmenProjektDurchgehend = false;
        rahmenContinuousDurchgehend = false;
        rahmenMid2 = rahmenSide2 = 0.0;
        rahmenL = rahmenR = rahmenL2 = rahmenR2 = rahmenLR = 0.0;
        rahmenAktivZellen = 0;
        rahmenZellen = 0;
        // Der Ereignisring wird hier BEWUSST nicht geleert: ein Ereignis
        // gehoert zu seiner Epoche (es traegt sie mit), nicht zu dem Rahmen,
        // in dem es zufaellig gemeldet wurde. Geleert wird er nur von
        // `zuruecksetzen()`.
    }

    Transportstempel baueStempel (const echtzeit::StampedBlock& b,
                                  bool evidenzWirdPubliziert) const noexcept
    {
        // Wert-Initialisierung, gleiche Vorgabe und gleiche Grenze wie in
        // `baueFrame()`: die Fuellbytes zwischen `bool` und `int64` bleiben
        // unter `/O2` trotzdem unbestimmt.  Gelesen werden sie nie — der
        // Vergleich zweier Stempel laeuft ueber `operator==`, also memberweise.
        Transportstempel t {};
        t.transport_epoch    = transportEpoche;
        t.continuity_segment = segmentInEpoche;
        // Der Stempel umfasst nicht nur die skalaren Samples, sondern auch den
        // fruehesten FFT-Support, der TATSAECHLICH in die publizierten Baender
        // eingegangen ist.  Durch 50-%-Ueberlappung kann der vor `b` beginnen.
        Support gesamt { true, b.stromVon, rahmenProjektDurchgehend,
                         b.projectSampleStart, rahmenContinuousDurchgehend,
                         b.continuousTimeSamples };
        supportVereinen (gesamt, liveSupport);
        if (evidenzWirdPubliziert)
        {
            supportVereinen (gesamt, evidenzSupport);
            // Auch band-inaktive Zwischenzeit gehoert zum Beweis der
            // Evidenzspanne. Sonst koennte eine fehlende/springende Hostuhr in
            // einer stillen Luecke verschwinden und spaeter scheinbar passend
            // auf die alte affine Achse zurueckkehren.
            if (! evidenzContinuousDurchgehend)
                gesamt.continuousGueltig = false;
        }

        bool stromSpanneGueltig = b.stromVon
            <= std::numeric_limits<std::uint64_t>::max() - rahmenSamples;
        const auto stromEnde = stromSpanneGueltig ? b.stromVon + rahmenSamples : 0;
        if (stromSpanneGueltig && gesamt.stromStart <= stromEnde)
        {
            const auto laenge = stromEnde - gesamt.stromStart;
            if (laenge > 0 && laenge <= std::numeric_limits<std::uint32_t>::max())
                t.sample_count = static_cast<std::uint32_t> (laenge);
        }
        t.sample_rate        = sr;

        t.process_context_present_gesetzt = true;
        t.process_context_present = (b.flags & echtzeit::kFlagKontextAnwesend) != 0;

        // 🔑 DIE ZEITBASIS FOLGT AUS DEM BEWEIS, NICHT AUS DEM WUNSCH.
        // §32.3: "Ohne gueltige Projektzeit darf `time_basis=local_monotonic`
        // nur lokale Analyse und IPC-Frische tragen."  Wer hier
        // `project_samples` setzte, weil der Host "meistens" eine Zeit liefert,
        // erlaubte damit Cross-Probe-Alignment auf einer Zahl, die es nicht
        // gibt — und NAK-29-Fall 2 faengt genau diesen Griff.
        if (gesamt.projektGueltig)
        {
            t.zeitbasis = Zeitbasis::project_samples;
            t.project_sample_start_gesetzt = true;
            t.project_sample_start = gesamt.projektStart;
            t.gueltigkeit |= kGProjectTime;
        }
        else
        {
            t.zeitbasis = Zeitbasis::local_monotonic;
            // KEIN `project_time`-Bit und KEIN `project_sample_start`.
        }

        if ((b.flags & echtzeit::kFlagSpieltGueltig) != 0)
        {
            t.playing = (b.flags & echtzeit::kFlagSpielt) != 0;
            t.gueltigkeit |= kGPlayState;
        }
        if ((b.flags & echtzeit::kFlagRecordingGueltig) != 0)
        {
            t.recording = (b.flags & echtzeit::kFlagRecording) != 0;
            t.gueltigkeit |= kGRecordState;
        }
        // Auch die optionale Continuous-Uhr bezeichnet denselben Frameanfang.
        // Ihr Beweis wird pro Sample durch FFT-Ringe und ueber alle Skalar-
        // bloecke gefuehrt; eine bloss lokal zurueckgerechnete Hostzahl waere
        // bei fehlendem oder springendem Continuous-Feld erfunden.
        if (gesamt.continuousGueltig)
        {
            t.continuous_time_samples_gesetzt = true;
            t.continuous_time_samples = gesamt.continuousStart;
            t.gueltigkeit |= kGContinuousTime;
        }

        t.cycle_active = (b.flags & echtzeit::kFlagCycleAktiv) != 0;
        const bool cycleGrenzenBrauchbar
            = (b.flags & echtzeit::kFlagCycleGrenzenGueltig) != 0
           && std::isfinite (b.cycleStartPpq)
           && std::isfinite (b.cycleEndePpq)
           && b.cycleEndePpq >= b.cycleStartPpq;
        if (cycleGrenzenBrauchbar)
        {
            t.cycle_bounds_valid = true;
            t.cycle_start_ppq_gesetzt = true;
            t.cycle_start_ppq = b.cycleStartPpq;
            t.cycle_end_ppq_gesetzt = true;
            t.cycle_end_ppq = b.cycleEndePpq;
            t.gueltigkeit |= kGCycleBounds;
            // ⚠️ `unproven`, IMMER.  Die Abbildung PPQ→Sample braucht ein
            // FL-Golden fuer diesen Hostlauf (§32.3), und der Capabilityreport
            // S4 hat keines.  `validated_block_mapping` waere hier eine
            // Behauptung ueber eine Messung, die nie stattgefunden hat.
            t.cycle_derivation = Herleitung::unproven;
        }

        if ((b.flags & echtzeit::kFlagEingangLatenzGemeldet) != 0)
        {
            t.input_presentation_latency_gesetzt = true;
            t.input_presentation_latency = b.eingangLatenzSamples;
            t.gueltigkeit |= kGInputLatency;
        }
        if ((b.flags & echtzeit::kFlagAusgangLatenzGemeldet) != 0)
        {
            t.output_presentation_latency_gesetzt = true;
            t.output_presentation_latency = b.ausgangLatenzSamples;
            t.gueltigkeit |= kGOutputLatency;
        }
        return t;
    }

    void fuelleLive (LiveBaender& satz, float* breite, std::uint8_t* breiteBitmap) const noexcept
    {
        satz.leeren();
        for (int i = 0; i < Gitter::liveBaender; ++i) breite[i] = 0.0f;
        bitmapNullen (breiteBitmap, Gitter::liveBaender);

        for (int g = 0; g < Gitter::liveBaender; ++g)
        {
            // Lineare Energiesumme der Feinbaender der Gruppe — das Livegitter
            // ist eine GRUPPIERUNG, keine zweite Filterbank (Fixture-Herkunft).
            double summe = 0.0, seiteSumme = 0.0, gesamtSumme = 0.0;
            int belegte = 0;
            for (int b = Gitter::liveVon (g); b < Gitter::liveBisExkl (g); ++b)
                if (liveAkku[(std::size_t) b].n > 0)
                {
                    const double n = (double) liveAkku[(std::size_t) b].n;
                    summe += liveAkku[(std::size_t) b].summe / n;
                    seiteSumme  += liveBreiteAkku[(std::size_t) b].seite  / n;
                    gesamtSumme += liveBreiteAkku[(std::size_t) b].gesamt / n;
                    ++belegte;
                }
            if (belegte == 0)
                continue;                          // Bitmap bleibt 0

            // Band-Stereo: Seitenanteil der Gruppe. Nur wenn ueberhaupt Energie
            // da ist — 0/0 waere NaN, und NaN ist hier "keine Aussage", also
            // gehoert es hinter das Bit, nicht in den Wert.
            if (gesamtSumme > 0.0)
            {
                const double anteil = seiteSumme / gesamtSumme;
                if (std::isfinite (anteil))
                {
                    breite[g] = (float) anteil;
                    bitmapSetze (breiteBitmap, g, true);
                }
            }

            const double db = energieAlsDb (summe);
            if (! plausibel (db))
                continue;                          // unter der NaN-Ehrlichkeitsgrenze
            const auto q = quantisiere16 (db, satz.encoding);
            if (! q.gueltig)
                continue;
            satz.werte[g] = q.wert;
            bitmapSetze (satz.bitmap, g, true);
            if (q.saturiert) satz.saturated = true;
        }
    }

    void fuelleEvidenz (EvidenzBaender& satz) const noexcept
    {
        satz.leeren();
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            if (evidenzAkku[(std::size_t) b].n == 0)
                continue;
            const double mittel = evidenzAkku[(std::size_t) b].summe
                                / (double) evidenzAkku[(std::size_t) b].n;
            const double db = energieAlsDb (mittel);
            if (! plausibel (db))
                continue;
            const auto q = quantisiere16 (db, satz.encoding);
            if (! q.gueltig)
                continue;
            satz.werte[b] = q.wert;
            bitmapSetze (satz.bitmap, b, true);
            if (q.saturiert) satz.saturated = true;
        }
    }

    void fuelleSkalare (FeatureFrame& f) const noexcept
    {
        if (rahmenZellen > 0)
        {
            f.aktivitaetGesetzt = true;
            f.aktivitaet = (float) ((double) rahmenAktivZellen / (double) rahmenZellen);
        }
        double lufs = 0.0;
        if (kurzLufs (lufs))
        {
            f.lufsSGesetzt = true;
            f.lufsS = (float) lufs;
        }
        if (rahmenSamples > 0 && rahmenPeak > 0.0)
        {
            const double peakDb = 20.0 * std::log10 (rahmenPeak);
            f.peakGesetzt = true;
            f.peakDb = (float) peakDb;

            const double rms = std::sqrt (rahmenSummeQuadrat / (double) rahmenSamples);
            if (rms > 0.0)
            {
                f.crestGesetzt = true;
                f.crestDb = (float) (peakDb - 20.0 * std::log10 (rms));
            }
            if (f.lufsSGesetzt)
            {
                // PSR(3 s) = True-Peak-Maximum desselben 3-s-Fensters minus
                // LUFS-S (§39.1).  Hier steht der SAMPLE-Peak des Rahmens, nicht
                // der True Peak — das ist eine schwaechere Zahl, und sie heisst
                // deshalb im Frame `psrDb` mit demselben Vorbehalt, den §39.1
                // fuer PLR/PSR ohnehin macht: ergaenzende Produktmetrik, kein
                // EBU-Qualitaetsurteil.
                f.psrGesetzt = true;
                f.psrDb = (float) (peakDb - (double) f.lufsS);
            }
        }
        if (rahmenSamples > 0)
        {
            const double gesamt = rahmenMid2 + rahmenSide2;
            if (gesamt > 0.0)
            {
                f.breiteGesetzt = true;
                f.breite = (float) (rahmenSide2 / gesamt);
            }
            const double nen = std::sqrt (std::max (rahmenL2, 0.0)) * std::sqrt (std::max (rahmenR2, 0.0));
            if (nen > 0.0)
            {
                const double k = rahmenLR / nen;
                if (std::isfinite (k))
                {
                    f.korrelationGesetzt = true;
                    f.korrelation = (float) std::clamp (k, -1.0, 1.0);
                }
            }
        }
    }

    //== Zustand ==============================================================

    struct Akku    { double summe { 0.0 }; std::uint64_t n { 0 }; };
    struct Breite  { double seite { 0.0 }; double gesamt { 0.0 }; };

    double sr { 0.0 };
    bool   vorbereitet { false };

    Stufe bass, haupt;
    int   erstesBand { 0 }, kappeBand { Gitter::evidenzBaender };

    Akku   liveAkku[Gitter::evidenzBaender] {};
    Akku   evidenzAkku[Gitter::evidenzBaender] {};
    // Nur auf der Live-Seite: §33.2 fuehrt "Breite/Korrelation" bei der
    // Live-Telemetrie und beim Evidenzsnapshot ausdruecklich nicht.
    Breite liveBreiteAkku[Gitter::evidenzBaender] {};
    Support liveSupport {};
    Support evidenzSupport {};

    // Loudness
    KKette kL, kR;
    int    zellenSamples { 0 }, zelleStand { 0 };
    double zelleKEnergie { 0.0 }, zelleAktivEnergie { 0.0 };
    std::vector<double> kurzZellen;
    int    kurzStand { 0 }, kurzGefuellt { 0 };

    // Rahmen (zwischen zwei Live-Frames)
    double rahmenPeak { 0.0 }, rahmenSummeQuadrat { 0.0 };
    std::uint64_t rahmenSamples { 0 };
    echtzeit::StampedBlock rahmenStartBlock {};
    bool rahmenProjektDurchgehend { false };
    bool rahmenContinuousDurchgehend { false };
    double rahmenMid2 { 0.0 }, rahmenSide2 { 0.0 };
    double rahmenL { 0.0 }, rahmenR { 0.0 }, rahmenL2 { 0.0 }, rahmenR2 { 0.0 }, rahmenLR { 0.0 };
    std::uint64_t rahmenAktivZellen { 0 }, rahmenZellen { 0 };
    std::uint64_t liveSamples { 0 }, evidenzSamples { 0 };
    bool evidenzContinuousHabe { false };
    bool evidenzContinuousDurchgehend { true };
    std::int64_t evidenzContinuousErwartet { 0 };

    // Ereignisse
    std::vector<Ereignis> ereignisse;
    int ereignisStand { 0 }, ereignisAnzahl { 0 };
    std::vector<double> vorigesSpektrum, flussHistorie, flussSortiert;
    bool vorigesSpektrumGueltig { false };
    int  flussStand { 0 }, flussGefuellt { 0 };

    // Zeitbuch
    echtzeit::StampedBlock vorigerBlock {};
    bool          habeVorigen { false };
    std::uint64_t transportEpoche { 0 }, segmentInEpoche { 0 }, sequenz { 0 };

    FeatureFrame aktuell {};
    bool         frameBereit { false };

    // Telemetrie
    std::uint64_t zGetrennteFenster { 0 }, zVerworfeneFensterSamples { 0 };
    std::uint64_t zEpochenwechsel { 0 }, zSegmentwechsel { 0 };
    std::uint64_t zStraddleVerworfen { 0 }, zEreignisseVerworfen { 0 };
    std::uint64_t zNak29Abgelehnt { 0 }, zBloecke { 0 };
    std::uint64_t zVerworfeneBandfenster { 0 };
    std::uint64_t grundZaehler[(std::size_t) Grenzgrund::anzahl] {};
};

} // namespace nakama::analyse
