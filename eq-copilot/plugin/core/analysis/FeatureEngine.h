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
#include "TruePeak.h"
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

      1. `time_basis=project_samples` verlangt gemeinsam
         `validity.project_time=true` UND `project_sample_start`;
      2. `time_basis=local_monotonic` verbietet sowohl
         `validity.project_time=true` ALS AUCH `project_sample_start`
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

    if (t.zeitbasis == Zeitbasis::project_samples
        && (! zeitBitAn || ! t.project_sample_start_gesetzt))
        return 1;
    if (t.zeitbasis == Zeitbasis::local_monotonic
        && (zeitBitAn || t.project_sample_start_gesetzt))
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

    /** SONDE-012 / E-A02: integrierte Lautheit reist nur als atomares Paar.
        Die drei Praesenzbits bilden FlatBuffers `= null` ab; ein numerischer
        Vorgabewert ist ohne sein Bit keine Messung. `lufsIStatus` ist genau
        1 (collecting) oder 2 (gated) und fehlt bei einem gueltigen Paar. */
    bool  lufsIGesetzt             { false };  float lufsI             { 0.0f };
    bool  lufsIUnsicherheitGesetzt { false };  float lufsIUnsicherheit { 0.0f };
    bool  lufsIStatusGesetzt       { false };  std::uint8_t lufsIStatus { 0 };

    /** NAK-68 (SONDE-013): ueber wie viel Audio DIESER Rahmen integriert wurde,
        in Samples je Kanal — nicht die Wanddauer und nicht die Fensterlaenge.

        An den Baendern ist eine duenne Messung schon ueber die Bitmap ehrlich;
        an den Rahmenskalaren (`peakDb`, `crestDb`, `breite`, `korrelation`)
        war sie es nicht. Ohne dieses Feld kann ein Empfaenger „leise" nicht
        von „kurz gemessen" unterscheiden — der Rahmen nach einer
        Epochengrenze traegt weniger integrierte Fenster und sieht trotzdem
        aus wie jeder andere.

        Das Praesenzbit bildet FlatBuffers `= null` ab: fehlt es, sagt der
        Erzeuger nichts, und ein Leser darf daraus KEINE 0 machen. Ein
        gesetztes 0 ist dagegen ein Senderfehler und wird von beiden Lesern
        abgelehnt (`integration_samples_null`). */
    bool  integrationGesetzt { false };  std::uint32_t integrationSamples { 0 };

    /** SONDE-013 M-01 bis M-04: die drei Loudnessfenster, der True Peak und
        die zwei Headroomgroessen.

        §39.1 zaehlt die Basis abschliessend auf — Momentary (400 ms),
        Short-term (3 s), Integrated, BS.1770-konformer True Peak, LRA erst ab
        rund 60 s, PLR/PSR und Crest in MEHREREN Fenstern.  Bis SONDE-013 trug
        der Frame davon `lufsS` (3 s), `peakDb` (Sample-Peak des Rahmens) und
        ein `psrDb`, das den Sample-Peak gegen LUFS-S rechnete.  Drei der
        sechs Punkte fehlten ganz, und der vierte war die schwaechere Zahl.

        Jedes Feld traegt sein Praesenzbit, weil jedes von ihnen eine eigene
        Bedingung hat, unter der es NICHT belastbar ist:

        - `lufsM` braucht 4 volle Zellen (400 ms), `lufsS` 30 (3 s);
        - `truePeakDb` braucht mindestens ein Sample im Rahmen;
        - `lraLu` braucht `kLraMindestSekunden` GEEIGNETEN, also ueber dem
          absoluten Gate liegenden Stoffs — §39.1 woertlich: "vorher `nicht
          belastbar`".  Ein Wert ohne Bit ist genau diese Aussage; eine 0
          waere eine Behauptung ueber Dynamik, die niemand gemessen hat;
        - `plrDb` braucht die integrierte Lautheit, die der SONDENPROZESSOR
          zumischt (E-A02, dieselbe Stelle wie `lufsI`) — die Engine kennt
          sie nicht und traegt deshalb `truePeakPassageDb` als Zutat;
        - `crestKurzDb` braucht das volle 3-s-Fenster wie `lufsS`.

        ⚠️ `psrDb` (oben) bleibt am selben Platz, rechnet aber seit diesem
        Ticket gegen den TRUE-PEAK-Maximalwert desselben 3-s-Fensters statt
        gegen den Sample-Peak des 100-ms-Rahmens.  Das ist die Definition aus
        §39.1 und keine Erweiterung: der alte Wert hiess schon `psrDb` und
        trug im Kommentar den Vorbehalt, dass er die schwaechere Zahl ist. */
    bool  lufsMGesetzt        { false };  float lufsM        { 0.0f };
    bool  truePeakGesetzt     { false };  float truePeakDb   { 0.0f };
    bool  truePeakPassageGesetzt { false };  float truePeakPassageDb { 0.0f };
    bool  plrGesetzt          { false };  float plrDb        { 0.0f };
    bool  lraGesetzt          { false };  float lraLu        { 0.0f };
    bool  crestKurzGesetzt    { false };  float crestKurzDb  { 0.0f };

    /** Headroom als VERTEILUNG, nicht als Einzelspitze (§39.2 woertlich:
        "Headroom wird in dBTP und als Verteilung ueber die Passage
        dargestellt.  Ein Peak darf nicht als Problem gelten, nur weil er hoch
        ist").

        Die drei Punkte sind P10/P50/P95 der RAHMEN-True-Peaks im laufenden
        Evidenzfenster; `headroomFenster` sagt, ueber wie viele Rahmen sie
        gehen — dieselbe Ehrlichkeit wie `evidenzFenster` bei den Bandpegeln.
        Eine einzelne Spitze hebt P95, laesst P50 aber stehen: genau der
        Unterschied, den ein Einzelwert nicht ausdruecken kann. */
    bool  headroomGesetzt { false };
    float headroomP10Db { 0.0f }, headroomP50Db { 0.0f }, headroomP95Db { 0.0f };
    std::uint32_t headroomFenster { 0 };

    /** SONDE-013 M-05: die Verteilungspunkte des Evidenzsnapshots (§33.2).

        §33.2 zaehlt den Inhalt des Snapshots abschliessend auf — „volle 221
        Baender, P10/P50/P95, Abdeckung, Konvergenz, Ereignisse". Die drei
        Perzentile sind deshalb kein Zusatz, sondern Pflichtinhalt; ohne sie
        traegt ein Snapshot nur seinen Mittelwert und sagt nichts darueber,
        ob dieser Mittelwert einen ruhigen oder einen springenden Verlauf
        zusammenfasst.

        Sie sind nur gefuellt, wenn `evidenzFrisch` gilt — genau wie
        `evidenz` selbst. Ein Band ohne Bit hat in ALLEN dreien kein Bit:
        eine Verteilung ueber nichts ist keine Verteilung. */
    EvidenzBaender evidenzP10 {}, evidenzP50 {}, evidenzP95 {};

    /** Wie viele Analysefenster hinter den drei Perzentilen stehen.

        Dieselbe Ehrlichkeit wie `integrationSamples` bei den Rahmenskalaren:
        ein Perzentil ueber drei Fenster sieht aus wie eines ueber sechzig.
        Der Wert ist die Zahl der WIRKLICH beruecksichtigten Fenster, also
        hoechstens `kVerteilungPlaetze`. */
    std::uint32_t evidenzFenster { 0 };

    /** Abdeckung und Konvergenz des Evidenzfensters, beide in [0, 1].

        `abdeckung` ist der Anteil der Analysefenster, die das Aktivgate
        genommen haben — die tatsaechlich gemessene Signalabdeckung, nicht
        die Wanddauer (§48.2). `konvergenz` misst, ob die Verteilung sich
        noch bewegt: die Uebereinstimmung der Bandmediane zwischen erster
        und zweiter Haelfte des behaltenen Fensters. Beide tragen ein
        Praesenzbit, weil „kein Fenster gesehen" keine 0 ist. */
    bool  abdeckungGesetzt  { false };  float abdeckung  { 0.0f };
    bool  konvergenzGesetzt { false };  float konvergenz { 0.0f };

    /** Fruehester lokaler Stromanfang des Evidenzfensters.

        Der Anker, gegen den ein Ereignis seinen Versatz im Snapshot nennt.
        Er steht hier und nicht im Transportstempel, weil der Stempel den
        RAHMEN beschreibt (10 Hz) und das Evidenzfenster laenger ist. Ohne
        Bit gibt es keinen Anker — dann traegt kein Ereignis einen ehrlichen
        Versatz, und der Snapshot meldet sie als verloren statt sie an eine
        erfundene Null zu haengen. */
    bool  evidenzStromStartGesetzt { false };
    std::uint64_t evidenzStromStart { 0 };

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

/** Wie viele Analysefenster hoechstens in P10/P50/P95 eines Bandes eingehen.

    SONDE-013 M-05. Der Wert ist eine RESSOURCENGRENZE, keine Messaussage:
    er deckelt den festen Speicher (221 Baender x 64 float = rund 57 KiB je
    Instanz) und damit den Sortieraufwand am Rahmenende. Wie viele Fenster
    wirklich eingegangen sind, sagt `FeatureFrame::evidenzFenster` — deshalb
    kann diese Zahl steigen oder fallen, ohne dass ein Empfaenger sie kennen
    muss.

    Er gehoert bewusst NICHT zu `kFeatureMetricsVersion`: eine
    Ressourcengrenze veraendert keine Schwelle und kein Gewicht. */
inline constexpr int kVerteilungPlaetze = 64;

/** Spanne, ueber die aus der Medianabweichung eine Konvergenz in [0, 1] wird.

    12 dB ist ein Startwert und ausdruecklich am Korpus kalibrierbar (§5.3,
    Risiko 5). Er lebt deshalb HIER neben `kFeatureMetricsVersion` und nicht
    als Literal im Rechenpfad: eine Kalibrierung ist dann eine neue
    Metrikversion, kein stiller Bruch. */
inline constexpr double kKonvergenzSpanneDb = 12.0;

/** Fester Ring der letzten Bandwerte EINES Bandes im Evidenzfenster.

    Warum ein Ring und kein Histogramm: P10/P50/P95 sollen exakt sein,
    nicht binquantisiert. Ein Histogramm braeuchte je Band Hunderte Bins,
    um unter 0,1 dB zu bleiben, und selbst dann waere der Wert eine
    Interpolation — eine Genauigkeit, die man behaupten, aber nicht
    messen kann. Bei 0,25 s Evidenzfenster und 50 % Ueberlappung liegen
    typisch deutlich weniger als `kVerteilungPlaetze` Fenster darin, also
    ist der Ring in der Praxis vollstaendig und das Perzentil exakt.

    Laeuft er doch ueber, behaelt er die JUENGSTEN Werte und `gefuellt`
    bleibt bei `kVerteilungPlaetze` stehen. Der Frame traegt diese Zahl
    als `evidenzFenster` mit — ein Empfaenger sieht damit, ueber wie
    viele Fenster die Verteilung wirklich geht, statt es zu raten. */
struct VerteilungsRing
{
    float werte[kVerteilungPlaetze] {};
    int   stand { 0 };       ///< naechster Schreibplatz
    int   gefuellt { 0 };    ///< belegte Plaetze, hoechstens kVerteilungPlaetze

    void schiebe (float db) noexcept
    {
        werte[(std::size_t) stand] = db;
        stand = (stand + 1) % kVerteilungPlaetze;
        if (gefuellt < kVerteilungPlaetze) ++gefuellt;
    }
    void leeren() noexcept { stand = 0; gefuellt = 0; }
};


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
    /** Momentary ueber 400 ms = 4 Zellen (§39.1, BS.1770-5 Blocklaenge).

        Er ist KEIN kuerzeres Short-term und keine Glaettung von `lufsS`: die
        zwei Fenster beantworten verschiedene Fragen (was ist gerade laut,
        gegen was traegt die Passage), und ein Empfaenger, der nur eines
        bekommt, kann das andere nicht daraus rechnen — deshalb reisen sie
        nebeneinander (M-01). */
    static constexpr int kMomentZellen = 4;
    /** LRA gilt erst ab rund 60 s GEEIGNETEN Materials (§39.1, EBU Tech 3342).

        "Geeignet" heisst: ueber dem absoluten Gate.  Eine Minute Stille ist
        keine Minute Material — deshalb zaehlt `lraGezaehlt` nur gegatete
        Kurzzeitwerte, nicht die Wanduhr (dieselbe Regel wie bei der
        Abdeckung, §48.2). */
    static constexpr double kLraMindestSekunden = 60.0;
    /** Hop der LRA-Kurzzeitfolge: 1 s = 10 Zellen (EBU Tech 3342 §2.2). */
    static constexpr int kLraHopZellen = 10;
    /** Absolutes Gate der LRA-Verteilung in LUFS (EBU Tech 3342: -70). */
    static constexpr double kLraAbsGateLufs = -70.0;
    /** Relatives Gate der LRA-Verteilung, LU unter dem gegateten Mittel
        (EBU Tech 3342: -20; das ist NICHT die -10 LU der integrierten
        Lautheit, und die zwei nicht zu vermischen ist der ganze Punkt). */
    static constexpr double kLraRelGateLu = -20.0;
    /** Histogramm der LRA-Kurzzeitwerte: 0,1 LU von -70 bis +30 LUFS.

        Fester Speicher wie beim `LoudnessAccumulator` — ein wachsender Vektor
        von Kurzzeitwerten waere bei einer langen Sitzung genau der unbegrenzte
        Bestand, den §48.1 ausschliesst.  0,1 LU Binbreite ist zehnmal feiner
        als die 1 LU, in der LRA berichtet wird. */
    static constexpr double kLraBinUnten  = -70.0;
    static constexpr double kLraBinBreite = 0.1;
    static constexpr int    kLraBins      = 1001;
    /** Livekadenz 10 Hz, Evidenzkadenz 1 bis 4 Hz (§33.2). */
    static constexpr double kLiveIntervallS    = 0.1;
    /** Schnellster und langsamster zulaessiger Evidenzabstand.

        §33.2 nennt fuer den Evidenzsnapshot ausdruecklich eine SPANNE, keinen
        Punkt: 1 bis 4 Hz. Der Grund steht in M-05 — bei Ueberlast wird die
        KADENZ reduziert, nie der Inhalt verworfen. Genau deshalb liegt die
        Reduktion hier in der Engine und nicht beim Sender: wer einen
        faelligen Snapshot einfach nicht sendete, wuerde sein Fenster trotzdem
        leeren und die Messung stillschweigend wegwerfen. Ein laengeres
        Fenster liefert dagegen weniger, aber VOLLSTAENDIGE Snapshots. */
    static constexpr double kEvidenzIntervallMinS = 0.25;   // 4 Hz
    static constexpr double kEvidenzIntervallMaxS = 1.0;    // 1 Hz
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
        // SONDE-013 M-03/M-04: zwei weitere Zellenringe derselben Laenge.
        // `kurzTpZellen` traegt das True-Peak-Maximum je Zelle (PSR rechnet
        // gegen das Maximum DESSELBEN 3-s-Fensters, §39.1), `kurzRmsZellen`
        // die UNGEWICHTETE Energie (Crest ist ein Pegelverhaeltnis, keine
        // Lautheit — die K-Gewichtung gehoert nicht hinein).
        kurzTpZellen.assign ((std::size_t) kKurzZellen, 0.0);
        kurzRmsZellen.assign ((std::size_t) kKurzZellen, 0.0);
        lraHistogramm.assign ((std::size_t) kLraBins, 0u);
        headroomRing.assign (1u, VerteilungsRing {});
        tp.vorbereiten (sr);

        evidenzVerteilung.assign ((std::size_t) Gitter::evidenzBaender,
                                  VerteilungsRing {});
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
        for (auto& r : evidenzVerteilung) r.leeren();   // SONDE-013 M-05
        evidenzFensterGesamt = 0;
        evidenzFensterAktiv = 0;
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

        // SONDE-013 M-02 bis M-04. Hier faellt AUCH, was eine Grenze
        // ueberlebt: `zuruecksetzen()` ist der Neuanfang der PASSAGE, und die
        // Passagengroessen — Passagen-True-Peak, Headroomverteilung und das
        // LRA-Histogramm — gehoeren genau dorthin.
        for (auto& z : kurzTpZellen)  z = 0.0;
        for (auto& z : kurzRmsZellen) z = 0.0;
        zelleTruePeak = 0.0;
        zelleRmsEnergie = 0.0;
        tp.zuruecksetzen();
        rahmenTruePeak = 0.0;
        passageTruePeak = 0.0;
        for (auto& r : headroomRing) r.leeren();
        for (auto& b : lraHistogramm) b = 0u;
        lraGezaehlt = 0;
        lraZellenSeitHop = 0;

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
    /** SONDE-013 M-05: derselbe Grund, ein Traeger weiter.

        Der Verteilungsring traegt den VERLAUF der Bandwerte, nicht ihre
        Summe — also ist er ein eigenes offenes Fenster und braucht seine
        eigene Auskunft. Ohne sie waere `keineAkkusUeberleben()` in B5
        wieder eine Liste, an der ein neuer Traeger vorbeikaeme; genau
        daran ist T2-1 entstanden. Gezaehlt werden BELEGTE PLAETZE ueber
        alle Baender, nicht Baender: ein einzelner ueberlebender Wert soll
        sichtbar sein. */
    std::uint64_t evidenzVerteilungPlaetze() const noexcept
    {
        std::uint64_t n = 0;
        for (const auto& r : evidenzVerteilung) n += (std::uint64_t) r.gefuellt;
        return n;
    }
    /** Fensterzaehler der Abdeckung: gesamt und aktiv. Beide muessen an einer
        Grenze fallen, sonst waere die Abdeckung ein Anteil ueber zwei
        Epochen. */
    std::uint64_t evidenzFensterGesamtJetzt() const noexcept { return evidenzFensterGesamt; }
    std::uint64_t evidenzFensterAktivJetzt()  const noexcept { return evidenzFensterAktiv; }

    /** SONDE-013 M-05: Evidenzkadenz zwischen 1 und 4 Hz einstellen.

        Der Wert wird auf `[kEvidenzIntervallMinS, kEvidenzIntervallMaxS]`
        GEKLEMMT statt abgelehnt: der Aufrufer ist der Sender, der auf
        Rueckstau reagiert, und ein abgelehnter Wunsch liesse ihn mit einer
        Kadenz weiterlaufen, die er gerade nicht bedienen kann. Nichtendliches
        aendert nichts — eine kaputte Zahl darf die Kadenz nicht verstellen.

        Wirkt ab dem naechsten faelligen Snapshot; ein bereits offenes
        Evidenzfenster wird NICHT abgeschnitten. */
    void evidenzIntervallSetzen (double sekunden) noexcept
    {
        if (! std::isfinite (sekunden))
            return;
        evidenzIntervallS = std::clamp (sekunden, kEvidenzIntervallMinS,
                                        kEvidenzIntervallMaxS);
    }
    double evidenzIntervallJetzt() const noexcept { return evidenzIntervallS; }

    /** Der Evidenzsnapshot hat die Ereignisse UEBERNOMMEN.

        Gegenpfad zu `ereignisAblegen`: ohne ihn traegt der naechste Snapshot
        dieselben Ereignisse noch einmal, und ein Empfaenger zaehlte einen
        Transienten mehrfach. Der Verlustzaehler bleibt stehen — er ist
        laufgebunden und wird vom Sender als Differenz gelesen. */
    void ereignisseEntnommen() noexcept { ereignisAnzahl = 0; }
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
        // SONDE-013 M-05: der Verteilungsring ist ein Fenster wie jedes andere
        // — er traegt den VERLAUF der Bandwerte ueber das Evidenzfenster. Ein
        // Perzentil, das eine Grenze ueberbrueckt, mischte zwei Epochen zu
        // einer Verteilung; die zwei Fensterzaehler daneben ergaeben eine
        // Abdeckung ueber zwei verschiedene Zeitraeume.
        for (auto& r : evidenzVerteilung)
        {
            zVerworfeneBandfenster += (std::uint64_t) r.gefuellt;
            r.leeren();
        }
        evidenzFensterGesamt = 0;
        evidenzFensterAktiv = 0;
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
        // SONDE-013 M-03/M-04: die zwei Nachbarringe des 3-s-Fensters fallen
        // MIT ihm. Sie stehenzulassen hiesse, den True-Peak-Maximalwert oder
        // die RMS aus der alten Epoche gegen eine neue Kurzzeitlautheit zu
        // rechnen — genau die Ueberbrueckung, die §32.3 ausschliesst, nur
        // eine Zeile neben der, an der sie 23.08. schon einmal stand.
        for (auto& z : kurzTpZellen)  z = 0.0;
        for (auto& z : kurzRmsZellen) z = 0.0;
        zelleTruePeak = 0.0;
        zelleRmsEnergie = 0.0;
        lraZellenSeitHop = 0;
        // 🔑 UND DIE DREI PASSAGENGROESSEN FALLEN MIT. Das ist die Korrektur
        // eines Entwurfsfehlers dieser Etappe, den G13 gefunden hat.
        //
        // Der erste Bau liess `passageTruePeak`, den Headroomring und das
        // LRA-Histogramm eine Grenze UEBERLEBEN — mit der Begruendung, sie
        // seien Passagenstatistiken wie die integrierte Lautheit, nicht
        // offene Fenster. Die Zwillingsprobe G13 hat das an allen sechs
        // Grenzarten widerlegt: A (vorher lautes Audio) und B (vorher
        // Stille) waren danach in 14 von 14 Frames unterscheidbar.
        //
        // Die Begruendung war falsch, und zwar aus zwei Gruenden. Erstens
        // reist die integrierte Lautheit gar nicht durch diese Engine — sie
        // kommt aus dem `LoudnessAccumulator` und wird erst im
        // Sondenprozessor zugemischt. Zweitens, und das ist der tragende
        // Grund: WAS IM `FeatureFrame` REIST, UNTERLIEGT §32.3. Der Frame
        // traegt seit dieser Etappe `truePeakPassageDb` und die
        // Headroomperzentile, also sind sie Traeger wie jeder andere.
        //
        // Fachlich stimmt das ueberdies mit §32.4 ueberein: eine Passage
        // bindet an GENAU EINE Transportepoche. Nach einer Epochengrenze ist
        // die Passage zu Ende, und ein Maximum ueber sie hinweg waere ueber
        // zwei Passagen gerechnet. Auch bei einem Drop (Segment, keine
        // Epoche) fehlt Audio, und ein Maximum ueber die Luecke behauptete
        // etwas ueber Material, das die Engine nie gesehen hat.
        //
        // Der Preis ist benannt und richtig: LRA braucht seine rund 60 s
        // OHNE Grenze. Genau das heisst "60 s geeignetes Material" (§39.1).
        passageTruePeak = 0.0;
        for (auto& r : headroomRing) r.leeren();
        for (auto& b : lraHistogramm) b = 0u;
        lraGezaehlt = 0;
        // Die K-Filter behalten ihren Zustand NICHT: ihr Nachklang traegt Audio
        // von VOR der Grenze, und genau das ist Ueberbrueckung — nur eben
        // ueber den Filterzustand statt ueber einen Puffer.  Das ist die
        // subtilste Form des Fehlers, gegen den dieses Ticket schuetzt.
        kL.zustandNullen();
        kR.zustandNullen();
        // Derselbe Grund fuer den True-Peak-Interpolator: seine 24 Taps je
        // Phase reichen ueber die Grenze zurueck, und ein daraus gerechneter
        // Zwischenwert gehoert zu keiner der beiden Epochen.
        tp.zuruecksetzen();

        rahmenPeak = 0.0;               // Korrelations-/Peakfenster
        rahmenTruePeak = 0.0;
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
            // SONDE-013 M-02: der True Peak liegt ZWISCHEN den Samples, also
            // wird hier ueberabgetastet und dort maximiert.  `tick` ist
            // O(kTaps) mit fester Schleife, ohne Allokation und ohne Sperre —
            // dieselbe Klasse Aufwand wie die zwei K-Filter darueber.
            //
            // ⚠️ Der Abtastpunkt SELBST geht mit ein, und das ist keine
            // Beschoenigung, sondern die Definition: der True Peak ist das
            // Maximum der rekonstruierten Wellenform, und die Abtastpunkte
            // gehoeren zu ihr.  Ohne diese Zeile faellt der Wert an einer
            // konkreten Stelle unter den Sample-Peak — der Interpolatorkern
            // ist um seine halbe Laenge zentriert, sieht ein Sample also erst
            // `kTapsJePhase / 2` Samples spaeter, und ein Spitzenwert in den
            // letzten zwoelf Samples eines Rahmens erreicht `rahmenTruePeak`
            // erst im naechsten.  Der Frame truege dann einen True Peak unter
            // seinem eigenen Sample-Peak, und BEIDE Leser lehnten ihn als
            // `true_peak_unter_sample_peak` ab — zu Recht.
            //
            // Die Korrektur steht bewusst NUR hier.  Sie an den drei
            // Verbrauchsstellen zu wiederholen waere eine zweite Wahrheit
            // ueber dieselbe Zahl.  (Fund aus dem Selbstaudit dieser Etappe,
            // gemessen von B18::`impuls_am_rahmenende`.)
            const double tpJetzt = std::max (tp.tick (l, r), std::max (absL, absR));
            rahmenTruePeak = std::max (rahmenTruePeak, tpJetzt);
            zelleTruePeak  = std::max (zelleTruePeak, tpJetzt);
            zelleRmsEnergie += 0.5 * (l * l + r * r);
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
                zelleTruePeak = 0.0;
                zelleRmsEnergie = 0.0;
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

        // SONDE-013 M-05: die Abdeckung des Evidenzfensters ist der Anteil
        // AKTIVER Fenster an ALLEN. Deshalb wird hier gezaehlt, VOR dem
        // Ruecksprung — ein Zaehler hinter dem `return` saehe nie ein stilles
        // Fenster und meldete jede Passage als vollstaendig abgedeckt.
        //
        // Gezaehlt wird nur die HAUPTstufe. Die Bassstufe hat ein anderes
        // Fenster- und Hopmass; beide zusammen zu zaehlen ergaebe ein
        // Verhaeltnis aus zwei verschiedenen Zeitachsen. Dieselbe Begruendung
        // steht beim Ereignisdetektor weiter unten.
        if (&s == &haupt)
        {
            // Saettigend statt umlaufend: ein Umlauf machte die Abdeckung
            // schlagartig zu einer Zahl ueber 1 oder zu 0 — beides saehe wie
            // eine Messung aus. Beide Zaehler werden bei jedem Evidenzframe
            // geleert, der Fall ist also theoretisch; er wird trotzdem
            // behandelt, weil eine stehengebliebene Grenze ehrlicher ist als
            // ein Sprung (CLAUDE.md, Zahlenraender).
            constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
            if (evidenzFensterGesamt < kMax) ++evidenzFensterGesamt;
            if (aktiv && evidenzFensterAktiv < kMax) ++evidenzFensterAktiv;
        }

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

            // SONDE-013 M-05: fuer P10/P50/P95 zaehlt der VERLAUF, nicht die
            // Summe. Der Ring nimmt den dB-Wert DIESES Fensters.
            //
            // Hier steht ABSICHTLICH keine Stufenbedingung, anders als beim
            // Fensterzaehler oben: die beiden Stufen teilen sich die Baender
            // ueberschneidungsfrei (`bandVon`/`bandBis`), jedes Band wird also
            // von genau einer bedient. Ein `&s == &haupt` liesse die
            // Bassbaender leer — eine Verteilung, die genau dort fehlt, wo die
            // Bassstufe ueberhaupt existiert.
            //
            // Nichtendliches oder unplausibles kommt gar nicht erst hinein: ein
            // Perzentil ueber einen NaN waere kein kleiner Fehler, sondern ein
            // NaN im ganzen Band.
            const double db = energieAlsDb (energie);
            if (plausibel (db))
                evidenzVerteilung[(std::size_t) b].schiebe ((float) db);
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
        // SONDE-013 M-03/M-04: die zwei Nachbarringe wandern MIT demselben
        // Stand, damit ein Kurzzeitfenster ueber alle drei dieselben Zellen
        // sieht.  Zwei Ringe mit eigenem Stand waeren zwei Fenster mit
        // demselben Namen — genau die Vermischung, die §39.1 bei PSR und
        // Crest ausschliesst.
        kurzTpZellen[(std::size_t) kurzStand]  = zelleTruePeak;
        kurzRmsZellen[(std::size_t) kurzStand] = zelleRmsEnergie / (double) zellenSamples;
        kurzStand = (kurzStand + 1) % kKurzZellen;
        if (kurzGefuellt < kKurzZellen) ++kurzGefuellt;

        // LRA (M-04, EBU Tech 3342): alle 10 Zellen — also 1 s Hop — geht der
        // aktuelle Kurzzeitwert ins Histogramm, sofern das VOLLE 3-s-Fenster
        // steht und der Wert das absolute Gate nimmt.  Nur diese gezaehlten
        // Werte tragen die 60-s-Regel; Stille laesst den Zaehler stehen.
        if (++lraZellenSeitHop >= kLraHopZellen)
        {
            lraZellenSeitHop = 0;
            double kurzJetzt = 0.0;
            if (kurzLufs (kurzJetzt) && kurzJetzt >= kLraAbsGateLufs)
            {
                const int bin = lraBin (kurzJetzt);
                if (bin >= 0)
                {
                    ++lraHistogramm[(std::size_t) bin];
                    ++lraGezaehlt;
                }
            }
        }

        const double aktivMittel = zelleAktivEnergie / (double) zellenSamples;
        if (aktivMittel > 0.0 && 10.0 * std::log10 (aktivMittel) > kAktivGateDb)
            ++rahmenAktivZellen;
        ++rahmenZellen;
    }

    /** Binindex eines Kurzzeitwertes im LRA-Histogramm, oder -1, wenn er
        ausserhalb des Rasters liegt.  Ein Wert ueber +30 LUFS ist jenseits
        jeder Musik und faellt lieber heraus, als das Raster zu sprengen. */
    static int lraBin (double lufs) noexcept
    {
        if (! std::isfinite (lufs))
            return -1;
        const int b = (int) std::floor ((lufs - kLraBinUnten) / kLraBinBreite);
        return (b >= 0 && b < kLraBins) ? b : -1;
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

    /** LUFS-M = dieselbe Formel ueber die letzten 400 ms (M-01, §39.1).

        Nur bei mindestens `kMomentZellen` gefuellten Zellen — aus demselben
        Grund wie bei `kurzLufs`: eine Momentanlautheit ueber 200 ms ist keine
        Momentanlautheit, sie ist eine andere Zahl mit demselben Namen. */
    bool momentanLufs (double& heraus) const noexcept
    {
        if (kurzGefuellt < kMomentZellen)
            return false;
        double summe = 0.0;
        for (int i = 0; i < kMomentZellen; ++i)
        {
            const int idx = (kurzStand - 1 - i + kKurzZellen * 2) % kKurzZellen;
            summe += kurzZellen[(std::size_t) idx];
        }
        const double mittel = summe / (double) kMomentZellen;
        if (! (mittel > 0.0))
            return false;
        heraus = -0.691 + 10.0 * std::log10 (mittel);
        return std::isfinite (heraus);
    }

    /** True-Peak-Maximum DESSELBEN 3-s-Fensters, gegen das `kurzLufs` rechnet
        (M-03).  Linear, wie der Detektor selbst liefert. */
    bool kurzTruePeak (double& heraus) const noexcept
    {
        if (kurzGefuellt < kKurzZellen)
            return false;
        double groesster = 0.0;
        for (int i = 0; i < kKurzZellen; ++i)
            groesster = std::max (groesster, kurzTpZellen[(std::size_t) i]);
        heraus = groesster;
        return groesster > 0.0;
    }

    /** Crest ueber das 3-s-Fenster: True-Peak-Maximum gegen die ungewichtete
        RMS desselben Fensters (M-04, "Crest-Faktor in MEHREREN Fenstern
        statt nur als globales Maximum").

        Das zweite Fenster neben dem 100-ms-Rahmen in `fuelleSkalare`.  Beide
        reisen, weil ein Signal mit dichten kleinen Spitzen im Rahmen einen
        hohen und ueber 3 s einen niedrigen Crest hat — und umgekehrt eine
        einzelne Spitze in ruhigem Material genau andersherum. */
    bool crestKurz (double& heraus) const noexcept
    {
        double tpMax = 0.0;
        if (! kurzTruePeak (tpMax) || tpMax <= 0.0)
            return false;
        double summe = 0.0;
        for (int i = 0; i < kKurzZellen; ++i)
            summe += kurzRmsZellen[(std::size_t) i];
        const double rms = std::sqrt (summe / (double) kKurzZellen);
        if (! (rms > 0.0))
            return false;
        heraus = 20.0 * std::log10 (tpMax) - 20.0 * std::log10 (rms);
        return std::isfinite (heraus);
    }

    /** LRA nach EBU Tech 3342 (M-04).

        Reihenfolge, und jeder Schritt ist eine eigene Bedingung, die allein
        fallen kann:
        1. mindestens `kLraMindestSekunden` GEGATETE Kurzzeitwerte — sonst
           `false`, also KEIN Wert.  §39.1 woertlich: "vorher `nicht
           belastbar`".  Eine 0 waere hier eine Dynamikaussage, die niemand
           gemessen hat;
        2. gegatetes Mittel ueber die absolut gegatete Verteilung bilden;
        3. relatives Gate bei -20 LU darunter anlegen;
        4. LRA = P95 - P10 der so gegateten Restverteilung.

        ⚠️ Das relative Gate ist -20 LU, NICHT die -10 LU der integrierten
        Lautheit.  Die zwei Zahlen gehoeren zu zwei verschiedenen Normen
        (BS.1770 gegen Tech 3342); sie zu vertauschen ergibt eine Zahl, die
        plausibel aussieht und um mehrere LU falsch ist. */
    bool lraLu (double& heraus) const noexcept
    {
        // Ein gegateter Kurzzeitwert je `kLraHopZellen` Zellen, also je
        // `kLraHopZellen * kZelleSekunden` Sekunden.
        const double sekundenJeWert = kZelleSekunden * (double) kLraHopZellen;
        if ((double) lraGezaehlt * sekundenJeWert < kLraMindestSekunden)
            return false;
        if (lraHistogramm.size() != (std::size_t) kLraBins)
            return false;

        // Schritt 2: energetisches Mittel ueber die absolut gegateten Werte.
        double summeZ = 0.0;
        std::uint64_t n = 0;
        for (int b = 0; b < kLraBins; ++b)
        {
            const auto c = lraHistogramm[(std::size_t) b];
            if (c == 0u) continue;
            summeZ += (double) c * std::pow (10.0, (lraBinMitte (b) + 0.691) / 10.0);
            n += c;
        }
        if (n == 0u || ! (summeZ > 0.0))
            return false;
        const double mittelLufs = -0.691 + 10.0 * std::log10 (summeZ / (double) n);
        const double relGate = mittelLufs + kLraRelGateLu;

        // Schritt 3 und 4: Restverteilung und ihre zwei Perzentile.
        std::uint64_t gesamt = 0;
        for (int b = 0; b < kLraBins; ++b)
            if (lraBinMitte (b) >= relGate)
                gesamt += lraHistogramm[(std::size_t) b];
        if (gesamt == 0u)
            return false;

        const double p10 = lraPerzentil (relGate, gesamt, 0.10);
        const double p95 = lraPerzentil (relGate, gesamt, 0.95);
        heraus = p95 - p10;
        return std::isfinite (heraus) && heraus >= 0.0;
    }

    static constexpr double lraBinMitte (int b) noexcept
    {
        return kLraBinUnten + ((double) b + 0.5) * kLraBinBreite;
    }

    /** Perzentil der relativ gegateten LRA-Verteilung.  Kumulativ ueber die
        Bins von unten; der erste Bin, der den Anteil erreicht, gewinnt. */
    double lraPerzentil (double relGate, std::uint64_t gesamt, double anteil) const noexcept
    {
        const double ziel = anteil * (double) gesamt;
        double lauf = 0.0;
        for (int b = 0; b < kLraBins; ++b)
        {
            const double mitte = lraBinMitte (b);
            if (mitte < relGate) continue;
            lauf += (double) lraHistogramm[(std::size_t) b];
            if (lauf >= ziel)
                return mitte;
        }
        return lraBinMitte (kLraBins - 1);
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
        f.evidenzFrisch = evidenzS >= evidenzIntervallS;
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
        {
            fuelleEvidenz (f.evidenz);
            // SONDE-013 M-05: Verteilung, Abdeckung und Konvergenz gehoeren zu
            // DIESEM Evidenzfenster und werden zusammen mit ihm gefuellt und
            // zusammen mit ihm geleert. Ein Snapshot mit Baendern, aber ohne
            // Verteilung waere die halbe Aussage aus §33.2.
            fuelleVerteilung (f);
            fuelleAbdeckungUndKonvergenz (f);
            f.evidenzStromStartGesetzt = evidenzSupport.gesetzt;
            f.evidenzStromStart = evidenzSupport.stromStart;
        }
        else
        {
            f.evidenz.leeren();
        }

        // SONDE-013 M-02/M-03: das Passagenmaximum und die Headroomverteilung
        // wachsen um GENAU DIESEN Rahmen, bevor die Skalare sie lesen — sonst
        // fehlte dem Frame sein eigener Beitrag und die Verteilung liefe dem
        // Wert daneben um einen Rahmen hinterher.
        if (rahmenTruePeak > 0.0 && ! headroomRing.empty())
        {
            passageTruePeak = std::max (passageTruePeak, rahmenTruePeak);
            headroomRing[0].schiebe ((float) (20.0 * std::log10 (rahmenTruePeak)));
        }

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
        // Verlauf, Abdeckung und Konvergenz gehoeren zu GENAU diesem
        // Evidenzfenster. Sie stehenzulassen hiesse, den naechsten Snapshot
        // aus fremdem Material zu rechnen — dieselbe Regel wie fuer den
        // Bandakku daneben.
        for (auto& r : evidenzVerteilung) r.leeren();
        evidenzFensterGesamt = 0;
        evidenzFensterAktiv = 0;
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
        // SONDE-013 M-02: das RAHMEN-Maximum faellt mit dem Rahmen, das
        // PASSAGEN-Maximum nicht — sonst waere PLR eine Aussage ueber 100 ms
        // und nicht ueber die Passage (§39.1).
        rahmenTruePeak = 0.0;
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

    /** Ein Band des Verteilungsrings in ZEITLICHER Reihenfolge nach `aus`.

        Der Ring speichert in Schreibreihenfolge; ist er voll, liegt der
        aelteste Wert bei `stand`. Fuer Perzentile spielt die Reihenfolge
        keine Rolle, fuer die Konvergenz (erste gegen zweite Haelfte) sehr
        wohl — deshalb gibt es genau eine Stelle, die sie herstellt, statt
        zweier Schleifen, die auseinanderlaufen koennen.

        @returns Zahl der geschriebenen Werte. */
    static int ringInZeitfolge (const VerteilungsRing& r, float* aus) noexcept
    {
        const int n = r.gefuellt;
        const int erster = (n == kVerteilungPlaetze) ? r.stand : 0;
        for (int i = 0; i < n; ++i)
            aus[i] = r.werte[(std::size_t) ((erster + i) % kVerteilungPlaetze)];
        return n;
    }

    /** Perzentil einer AUFSTEIGEND sortierten Folge, linear interpoliert.

        Linear statt „naechster Rang", weil der naechste Rang bei kleinem `n`
        springt: mit acht Werten laege P10 sonst immer exakt auf dem Minimum
        und P95 immer exakt auf dem Maximum, und die drei Punkte trugen
        weniger Information als die Bandwerte selbst. */
    static double perzentil (const float* sortiert, int n, double p) noexcept
    {
        if (n <= 0) return 0.0;
        if (n == 1) return sortiert[0];
        const double pos = p * (double) (n - 1);
        const int    lo  = (int) pos;
        const int    hi  = lo + 1 < n ? lo + 1 : lo;
        const double f   = pos - (double) lo;
        return (double) sortiert[lo] + f * ((double) sortiert[hi] - (double) sortiert[lo]);
    }

    /** SONDE-013 M-05: P10/P50/P95 je Band plus die Zahl der Fenster dahinter.

        `evidenzFenster` ist das MINIMUM ueber alle Baender mit Bit, nicht der
        Mittelwert und nicht das Maximum: die Zahl soll sagen, worauf sich der
        SCHWAECHSTE gezeigte Punkt stuetzt. Die zwei Analysestufen haben
        verschiedene Hopmasse, also traegt die Bassstufe hier regelmaessig die
        kleinere Zahl — genau das ist die ehrliche Auskunft. */
    void fuelleVerteilung (FeatureFrame& f) const noexcept
    {
        for (auto* satz : { &f.evidenzP10, &f.evidenzP50, &f.evidenzP95 })
        {
            satz->gitter   = GitterId::nakama_1_24_oct_30_18k_v1;
            satz->encoding = BandEncoding::q_db_0p01_i16;
            satz->leeren();
        }

        int schwaechste = -1;
        float folge[kVerteilungPlaetze];
        // Vor `vorbereiten()` gibt es keinen Ring. Die drei Bandsaetze stehen
        // dann leer da — richtig so: ohne Messung keine Verteilung.
        if ((int) evidenzVerteilung.size() < Gitter::evidenzBaender)
            return;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& r = evidenzVerteilung[(std::size_t) b];
            if (r.gefuellt == 0)
                continue;                       // kein Bit: eine Verteilung ueber nichts

            const int n = ringInZeitfolge (r, folge);
            std::sort (folge, folge + n);

            bool alleGueltig = true;
            const double punkte[3] = { perzentil (folge, n, 0.10),
                                       perzentil (folge, n, 0.50),
                                       perzentil (folge, n, 0.95) };
            Quant16 q[3] {};
            for (int i = 0; i < 3; ++i)
            {
                if (! plausibel (punkte[i]))
                {
                    alleGueltig = false;
                    break;
                }
                q[i] = quantisiere16 (punkte[i], BandEncoding::q_db_0p01_i16);
                if (! q[i].gueltig)
                {
                    alleGueltig = false;
                    break;
                }
            }
            // Alle drei oder keiner. Ein Band, in dem nur P50 ein Bit haette,
            // saehe aus wie eine Verteilung und waere keine.
            if (! alleGueltig)
                continue;

            EvidenzBaender* saetze[3] = { &f.evidenzP10, &f.evidenzP50, &f.evidenzP95 };
            for (int i = 0; i < 3; ++i)
            {
                saetze[i]->werte[(std::size_t) b] = q[i].wert;
                bitmapSetze (saetze[i]->bitmap, b, true);
                if (q[i].saturiert) saetze[i]->saturated = true;
            }
            if (schwaechste < 0 || n < schwaechste)
                schwaechste = n;
        }
        f.evidenzFenster = schwaechste < 0 ? 0u : (std::uint32_t) schwaechste;
    }

    /** SONDE-013 M-05: Abdeckung und Konvergenz des Evidenzfensters.

        **Abdeckung** ist die tatsaechlich gemessene Signalabdeckung (§48.2),
        also aktive Fenster durch alle Fenster — nicht die Wanddauer. Ohne ein
        einziges Fenster gibt es kein Praesenzbit; 0 hiesse „nur Stille" und
        waere eine andere Aussage als „nichts gesehen".

        **Konvergenz** misst, ob die Verteilung sich noch bewegt: je Band der
        Betrag der Differenz zwischen dem Median der ERSTEN und dem der
        ZWEITEN Haelfte des behaltenen Fensters, gemittelt ueber die Baender
        mit genug Werten, dann ueber `kKonvergenzSpanneDb` auf [0, 1]
        abgebildet. 1 heisst „die beiden Haelften sagen dasselbe", 0 heisst
        „sie sagen Verschiedenes". Baender mit weniger als vier Werten gehen
        NICHT ein: zwei Mediane aus je einem Wert sind kein Konvergenzbeleg. */
    void fuelleAbdeckungUndKonvergenz (FeatureFrame& f) const noexcept
    {
        if (evidenzFensterGesamt > 0)
        {
            f.abdeckungGesetzt = true;
            f.abdeckung = (float) ((double) evidenzFensterAktiv
                                 / (double) evidenzFensterGesamt);
        }

        double summeAbstand = 0.0;
        int    baenderMitBeleg = 0;
        float  folge[kVerteilungPlaetze];
        float  haelfte[kVerteilungPlaetze];
        if ((int) evidenzVerteilung.size() < Gitter::evidenzBaender)
            return;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& r = evidenzVerteilung[(std::size_t) b];
            if (r.gefuellt < 4)
                continue;
            const int n = ringInZeitfolge (r, folge);
            const int h = n / 2;

            for (int i = 0; i < h; ++i) haelfte[i] = folge[i];
            std::sort (haelfte, haelfte + h);
            const double medianFrueh = perzentil (haelfte, h, 0.50);

            const int zweite = n - h;
            for (int i = 0; i < zweite; ++i) haelfte[i] = folge[h + i];
            std::sort (haelfte, haelfte + zweite);
            const double medianSpaet = perzentil (haelfte, zweite, 0.50);

            const double abstand = std::abs (medianSpaet - medianFrueh);
            if (! std::isfinite (abstand))
                continue;
            summeAbstand += abstand;
            ++baenderMitBeleg;
        }
        if (baenderMitBeleg > 0)
        {
            const double mittel = summeAbstand / (double) baenderMitBeleg;
            f.konvergenzGesetzt = true;
            f.konvergenz = (float) std::clamp (1.0 - mittel / kKonvergenzSpanneDb,
                                               0.0, 1.0);
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
        // SONDE-013 M-01: Momentary NEBEN Short-term, nicht statt seiner.
        double lufsMoment = 0.0;
        if (momentanLufs (lufsMoment))
        {
            f.lufsMGesetzt = true;
            f.lufsM = (float) lufsMoment;
        }
        // SONDE-013 M-09 (NAK-68): ueber wie viel Audio DIESER Rahmen
        // integriert wurde.  Das Feld ist die ganze Antwort auf "leise oder
        // kurz gemessen?" — ohne es sehen die Skalare darunter nach einer
        // Grenze aus wie im Dauerbetrieb.  0 waere ein Senderfehler (beide
        // Leser lehnen ihn ab), also traegt ein Rahmen ohne Samples KEIN Bit.
        if (rahmenSamples > 0)
        {
            f.integrationGesetzt = true;
            f.integrationSamples = (std::uint32_t) std::min<std::uint64_t> (
                rahmenSamples, (std::uint64_t) std::numeric_limits<std::uint32_t>::max());
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
        }
        // SONDE-013 M-02: der True Peak des Rahmens.  Er steht NEBEN `peakDb`
        // und ersetzt ihn nicht: der Sample-Peak ist die Zahl, die ein Host
        // anzeigt, der True Peak die, an der ein Encoder clippt.  Wer sie
        // zusammenlegte, verloere genau die Differenz, um die es geht.
        // `rahmenTruePeak` schliesst den Abtastpunkt bereits ein — siehe die
        // Begruendung an seiner einzigen Schreibstelle in
        // `verarbeiteSamples`.
        if (rahmenTruePeak > 0.0)
        {
            f.truePeakGesetzt = true;
            f.truePeakDb = (float) (20.0 * std::log10 (rahmenTruePeak));
        }
        if (passageTruePeak > 0.0)
        {
            f.truePeakPassageGesetzt = true;
            f.truePeakPassageDb = (float) (20.0 * std::log10 (passageTruePeak));
        }
        // SONDE-013 M-03: PSR(3 s) = True-Peak-Maximum DESSELBEN 3-s-Fensters
        // minus LUFS-S (§39.1).  Bis zu diesem Ticket rechnete das Feld gegen
        // den Sample-Peak des 100-ms-Rahmens — zwei verschiedene Fenster und
        // die schwaechere der zwei Peakzahlen.  Beides ist jetzt behoben; das
        // Bit faellt weg, solange das 3-s-Fenster nicht steht, statt eine
        // kuerzere Rechnung unter demselben Namen zu liefern.
        double tpKurz = 0.0;
        if (f.lufsSGesetzt && kurzTruePeak (tpKurz) && tpKurz > 0.0)
        {
            f.psrGesetzt = true;
            f.psrDb = (float) (20.0 * std::log10 (tpKurz) - (double) f.lufsS);
        }
        // SONDE-013 M-04: das zweite Crestfenster.
        double crest3s = 0.0;
        if (crestKurz (crest3s))
        {
            f.crestKurzGesetzt = true;
            f.crestKurzDb = (float) crest3s;
        }
        // SONDE-013 M-04: LRA erst ab rund 60 s geeignetem Material.
        double lra = 0.0;
        if (lraLu (lra))
        {
            f.lraGesetzt = true;
            f.lraLu = (float) lra;
        }
        // SONDE-013 M-03: Headroom als Verteilung.  Vier Rahmen sind die
        // Untergrenze, unter der P10 und P95 derselbe Wert waeren — dieselbe
        // Schwelle wie bei der Bandkonvergenz nebenan.
        if (! headroomRing.empty() && headroomRing[0].gefuellt >= 4)
        {
            float sortiert[kVerteilungPlaetze];
            const int n = ringInZeitfolge (headroomRing[0], sortiert);
            std::sort (sortiert, sortiert + n);
            f.headroomGesetzt = true;
            f.headroomP10Db = (float) perzentil (sortiert, n, 0.10);
            f.headroomP50Db = (float) perzentil (sortiert, n, 0.50);
            f.headroomP95Db = (float) perzentil (sortiert, n, 0.95);
            f.headroomFenster = (std::uint32_t) n;
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

    // SONDE-013 M-05: Verteilung, Abdeckung und Konvergenz des Evidenzfensters.
    // Die zwei Zaehler stehen NEBENEINANDER, weil die Abdeckung genau ihr
    // Verhaeltnis ist: `gesamt` waechst bei JEDEM Hauptstufen-Fenster, `aktiv`
    // nur bei denen ueber dem Aktivgate. Ein einzelner Zaehler koennte
    // "keine Fenster gesehen" nicht von "nur Stille gesehen" trennen.
    /// Ein Ring je Band, rund 58 KiB. Er liegt im HEAP und nicht als Feld
    /// im Objekt — genau wie `kurzZellen`, `ereignisse` und `flussHistorie`
    /// daneben. Der Grund ist gemessen: als Feld sprengten zwei Engines
    /// nebeneinander (die Zwillingsprobe G13 in B5) den 1-MiB-Stack mit
    /// STATUS_STACK_OVERFLOW. Angelegt wird er in `vorbereiten()`, also auf
    /// dem Nachrichtenthread; der Audiothread alloziert weiterhin nie.
    std::vector<VerteilungsRing> evidenzVerteilung;
    std::uint64_t   evidenzFensterGesamt { 0 };
    std::uint64_t   evidenzFensterAktiv  { 0 };
    /// Laufender Evidenzabstand, zwischen Min und Max. Er ueberlebt eine
    /// Grenze und ein `zuruecksetzen()` ABSICHTLICH: der Rueckstau, der ihn
    /// gesetzt hat, verschwindet nicht dadurch, dass die Messreihe neu
    /// beginnt.
    double          evidenzIntervallS { kEvidenzIntervallMinS };

    // Loudness
    KKette kL, kR;
    int    zellenSamples { 0 }, zelleStand { 0 };
    double zelleKEnergie { 0.0 }, zelleAktivEnergie { 0.0 };
    std::vector<double> kurzZellen;
    int    kurzStand { 0 }, kurzGefuellt { 0 };

    // SONDE-013 M-02 bis M-04: True Peak, das zweite und dritte Loudnessfenster
    // und die zwei Headroomgroessen.
    TruePeakDetektor tp;
    /// True-Peak-Maximum je Zelle, Ring wie `kurzZellen`; daraus wird das
    /// Maximum des 3-s-Fensters fuer PSR.
    std::vector<double> kurzTpZellen;
    /// Ungewichtete mittlere Energie je Zelle — die RMS-Haelfte des
    /// Crest-Faktors ueber 3 s.
    std::vector<double> kurzRmsZellen;
    double zelleTruePeak { 0.0 }, zelleRmsEnergie { 0.0 };
    /// Maximum ueber den laufenden 100-ms-Rahmen und ueber die Passage.
    ///
    /// Die "Passage" ist hier der Abschnitt seit der letzten GRENZE, nicht
    /// seit dem letzten `zuruecksetzen()`: §32.4 bindet eine Passage an
    /// genau eine Transportepoche, und G13 misst, dass kein Traeger des
    /// Frames eine Grenze ueberbrueckt. Ein Passageobjekt gibt es in P4 noch
    /// nicht (das ist Etappe E); bis dahin ist die Epochengrenze die
    /// einzige belegbare Passagengrenze.
    double rahmenTruePeak { 0.0 }, passageTruePeak { 0.0 };
    /// LRA: Histogramm der gegateten Kurzzeitwerte plus der Zaehler, der die
    /// 60-s-Regel traegt. `lraZellenSeitHop` erzeugt den 1-s-Hop. Beide
    /// fallen an jeder Grenze — siehe `grenzeZiehen()`.
    std::vector<std::uint32_t> lraHistogramm;
    std::uint64_t lraGezaehlt { 0 };
    int           lraZellenSeitHop { 0 };
    /// Headroomverteilung: ein Ring der Rahmen-True-Peaks in dB.  Er teilt
    /// die Laenge `kVerteilungPlaetze` mit den Bandringen — dieselbe
    /// Ressourcengrenze, deshalb bewusst KEINE zweite Konstante, die davon
    /// abdriften koennte.
    ///
    /// Er liegt im HEAP, mit genau einem Element, aus demselben gemessenen
    /// Grund wie `evidenzVerteilung` daneben: B5 haelt zwanzig Engines
    /// gleichzeitig auf dem Stack, und dort summieren sich auch 264 Byte je
    /// Instanz zu einem `STATUS_STACK_OVERFLOW` (Manifest §10.2, Befund 1 —
    /// und ein zweites Mal beim Bau der Etappe C).
    std::vector<VerteilungsRing> headroomRing;

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
