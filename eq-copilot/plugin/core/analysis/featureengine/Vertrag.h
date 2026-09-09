// FeatureEngine — der Vertrag: was die Engine liefert, unabhaengig davon, wie.
//
// NAK-225/S25d (09.09.2026), herausgeloest aus core/analysis/FeatureEngine.h.
// Dieser Teilkopf traegt die Typen und Konstanten, die ZWISCHEN Engine und
// Verbrauchern stehen — Prozessor, Sonde, Broker-Wire, Goldens:
//
//   Grenzgrund, kG*-Beweismaske, Zeitbasis, Herleitung, Transportstempel,
//   nak29Verstoss        Der Zeitvertrag: warum eine Grenze gezogen wurde und
//                        welche Beweisstuecke der Host geliefert hat.
//   Frameversuch, frameversuch
//                        Der Zaehlvertrag fuer Frames, die nicht entstehen.
//   Ereignis             Was die Engine als Ereignis meldet.
//   Fingerprint, kFingerprintMindestFenster, fingerprintAehnlichkeit
//                        Der Passagenvergleich.
//   StereoBandwert, StereoSkalare
//                        Die Stereosicht je Band und im Ganzen.
//   Bandsatz, LiveBaender, EvidenzBaender
//                        Die beiden Bandaufloesungen.
//   FeatureFrame, kFeatureMetricsVersion und die Schwellenkonstanten
//                        Der Messrahmen, den die Engine veroeffentlicht.
//   VerteilungsRing      Der Ring, aus dem die Perzentile entstehen.
//
// WARUM GETRENNT: Die Engine kennt keine Optik (CLAUDE.md), aber ihre
// Verbraucher kennen diesen Vertrag. Er hier zu haben macht sichtbar, was sich
// nicht aendern darf, ohne dass ein Golden es merkt — die metrics_version ganz
// unten ist genau dieser Riegel.
//
// Der Teilkopf wird VOR der Klasse eingebunden; ohne ihn gaebe es keine
// Feldtypen. Er ist nicht eigenstaendig einbindbar: FeatureEngine.h ist die
// eine Tuer.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

#include "../BandGrid.h"
#include "../Konfidenz.h"

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
/** Grober Content-Fingerprint einer Passage (SONDE-013 M-26, Entwurf §32.4).

    Der Vertrag legt Form und Groesse fest
    (`eq-ipc-v3.schema.json`, `$defs/fingerprint`): 32 Bandenergien, 12
    Chromawerte, 32 Onsetstuetzstellen, je EIN Byte. Diese Groesse ist die
    ganze Zusage:

        "Er enthaelt KEIN rekonstruierbares PCM und dient nur dazu, grob
         anderes musikalisches Material zu erkennen."

    76 Byte fuer eine Passage von Sekunden bis Minuten — bei 48 kHz sind das
    weniger als ein Byte je 3000 Samples. Aus dieser Dichte laesst sich kein
    Signal zurueckrechnen, und zwar nicht, weil die Rueckrechnung schwer
    waere, sondern weil die Information nicht da ist. Genau das misst
    `EqCopSonde013FingerprintGoldenTest` an einer adversarialen Probe.

    ── DIE DREI VERLAEUFE, UND WARUM ES DREI SIND ────────────────────────────

    Sie beantworten drei verschiedene Fragen, und keine ersetzt eine andere:

    - **Bandenergie** — WIE klingt das Material spektral? Trennt Bass von
      Gitarre, aber nicht C-Dur von D-Dur.
    - **Chroma** — WELCHE Toene? Trennt Tonarten und Akkorde, ist aber gegen
      Klangfarbe unempfindlich (das ist Absicht: derselbe Akkord auf zwei
      Instrumenten SOLL aehnlich aussehen).
    - **Onset** — WANN passiert etwas? Trennt zwei Passagen mit demselben
      Material und anderem Rhythmus.

    Ein Fingerprint aus nur einem der drei liesse jeweils eine ganze Klasse
    von Materialwechseln durch. §15 verlangt aber „Warnung oder Sperre, wenn
    das musikalische Material nicht vergleichbar ist" — und ein Wechsel, den
    der Vergleich nicht sieht, ist schlimmer als keiner.

    ── DIE GRENZE (M-27) ─────────────────────────────────────────────────────

    §32.3 nennt das Fingerprintfenster ausdruecklich neben FFT, Loudness und
    Korrelation: keines darf eine echte oder moegliche Epochengrenze
    ueberbruecken. Der Akkumulator faellt deshalb in `grenzeZiehen()` wie
    jeder andere. Ein Fingerprint ueber zwei Stellen der Musik beschriebe
    keine von beiden — und wuerde als „dasselbe Material" gelesen. */
struct Fingerprint
{
    /** Version des Erzeugers. Sie reist mit, weil eine spaetere Aenderung an
        der Quantisierung oder der Bandgruppierung alle alten Fingerprints
        unvergleichbar macht — und das soll auffallen, nicht stillschweigend
        zu falschen Aehnlichkeiten fuehren. */
    static constexpr int kVersion = 1;

    static constexpr int kBaender = 32;
    static constexpr int kChroma  = 12;
    static constexpr int kOnsets  = 32;

    bool gesetzt { false };
    /// Kopie von `kVersion` im Objekt, damit ein gespeicherter oder
    /// uebertragener Fingerprint seine Erzeugerversion MITTRAEGT statt sie
    /// beim Lesen aus der jeweils aktuellen Konstante zu holen.
    int  version { kVersion };
    std::uint8_t bandEnergie[kBaender] {};
    std::uint8_t chroma[kChroma] {};
    std::uint8_t onset[kOnsets] {};

    /** Wie viele Analysefenster hinter ihm stehen. Dieselbe Ehrlichkeit wie
        ueberall sonst: ein Fingerprint aus drei Fenstern ist keine Passage. */
    std::uint32_t fenster { 0 };

    bool operator== (const Fingerprint&) const = default;
};

/** Wie viele Analysefenster ein Fingerprint mindestens braucht.

    Unter dieser Zahl gibt es keinen — nicht, weil die Rechnung fehlschluege,
    sondern weil ein Onsetverlauf aus vier Fenstern keine Rhythmusaussage ist
    und eine Bandenergie aus vier Fenstern kein Klangbild. Startwert, am
    Korpus kalibrierbar (§5.3, Risiko 5); er lebt deshalb neben
    `kFeatureMetricsVersion`. */
inline constexpr int kFingerprintMindestFenster = 32;

/** Aehnlichkeit zweier Fingerprints in [0, 1] (M-28, M-31).

    Cosinus-Aehnlichkeit je Verlauf, dann das MINIMUM der drei — nicht der
    Mittelwert. Das ist dieselbe Regel wie bei der Konfidenzklasse (§34.3):
    ein Material, das spektral passt und rhythmisch nicht, ist nicht „zu zwei
    Dritteln dasselbe". Der schwaechste Beleg bestimmt die Aussage.

    ⚠️ Zwei Fingerprints ohne Bit sind NICHT aehnlich. Ohne diese Zeile waere
    „beide leer" die hoechste Aehnlichkeit, die es gibt — und eine Passage
    ohne Material verglichen sich mit jeder anderen als identisch. */
inline double fingerprintAehnlichkeit (const Fingerprint& a,
                                       const Fingerprint& b) noexcept
{
    if (! a.gesetzt || ! b.gesetzt)
        return 0.0;

    auto cosinus = [] (const std::uint8_t* x, const std::uint8_t* y, int n)
    {
        double xy = 0.0, xx = 0.0, yy = 0.0;
        for (int i = 0; i < n; ++i)
        {
            const double u = (double) x[i], v = (double) y[i];
            xy += u * v; xx += u * u; yy += v * v;
        }
        const double nenner = std::sqrt (xx) * std::sqrt (yy);
        if (! (nenner > 0.0))
            return 0.0;
        const double c = xy / nenner;
        return std::isfinite (c) ? std::clamp (c, 0.0, 1.0) : 0.0;
    };

    const double cBand   = cosinus (a.bandEnergie, b.bandEnergie, Fingerprint::kBaender);
    const double cChroma = cosinus (a.chroma,      b.chroma,      Fingerprint::kChroma);
    const double cOnset  = cosinus (a.onset,       b.onset,       Fingerprint::kOnsets);
    return std::min (cBand, std::min (cChroma, cOnset));
}

//==============================================================================
/** Bandweise Stereoevidenz eines Evidenzfensters (SONDE-013 M-08, M-10 bis M-12).

    §40.1 sagt, dass zwei globale Skalare nicht reichen: `breite` und
    `korrelation` im 10-Hz-Liveframe beschreiben die ganze Bandbreite mit einer
    Zahl, und ein Signal, das unten mono und oben breit ist, sieht darin genauso
    aus wie eines mit gleichmaessiger Breite. Diese Struktur ist die bandweise
    Antwort.

    ── WARUM SIE NICHT IM `FeatureFrame` STEHT ──────────────────────────────

    Elf Bandsaetze zu 221 `float` sind rund 11 KiB. Der `FeatureFrame` liegt in
    `baueFrame()` auf dem Stack und wird in B5 von einem Dutzend Engines
    gleichzeitig gehalten; die Etappe C hat den 1-MiB-Stack dieses Beins
    bereits DREIMAL gesprengt (Manifest §10.3, §10.4). Die Stereoevidenz liegt
    deshalb im Heap der Engine, und der Serialisierer liest sie ueber
    `stereoBand()` — dasselbe Muster wie beim Ereignisring, den der
    Sondenprozessor auch direkt aus der Engine holt.

    ⚠️ Das hat einen Preis, und er ist benannt: die Zwillingsprobe G13
    vergleicht `FeatureFrame`-Objekte und sieht diese Traeger deshalb NICHT.
    Dass keiner von ihnen eine Transportgrenze ueberbrueckt, misst statt
    dessen `EqCopSonde013StereoGoldenTest` ausdruecklich und namentlich.

    ── DIE ZWEI FAIL-CLOSED-STUFEN ──────────────────────────────────────────

    §40.1 woertlich: "Bei zu wenig Energie oder Frames ist Kohaerenz `null`"
    und "Interchannel-Phase wird nur in ausreichend kohaerenten Baendern
    interpretiert". Das sind zwei Stufen, nicht eine:

    1. `kohaerenzGesetzt` faellt weg, wenn das Band weniger als
       `kWelchMindestFrames` gueltige Welch-Frames hat oder eine der beiden
       Autospektralsummen null ist.
    2. `phaseGesetzt` faellt zusaetzlich weg, wenn die Kohaerenz
       `kKohaerenzSchwellePhase` nicht ueberschreitet — eine Phase aus einem
       inkohaerenten Band ist der Winkel eines Zufallszeigers.

    Die Phasenbits sind damit eine Teilmenge der Kohaerenzbits. Das kann das
    JSON-Schema nicht ausdruecken (es steht als Kommentar an `stereo_evidenz`);
    der Golden misst es.
*/
struct StereoBandwert
{
    /** Mid-/Side-Energie und Seitenanteil in dB, plus die zwei Korrelationen.
        Ein Band ohne dieses Bit hat in KEINEM der fuenf Felder einen Wert. */
    /** Praesenzbit der drei ENERGIEfelder: `midDb`, `sideDb`,
        `seitenanteilDb`.

        ⚠️ NAK-181 R5: es gilt AUSSCHLIESSLICH diesen dreien. Die zwei
        Korrelationen und die Persistenz tragen seit diesem Ticket ihre eigenen
        Bits — sie beantworten andere Fragen und werden unter anderen
        Bedingungen gesetzt. */
    bool  basisGesetzt { false };
    float midDb { 0.0f }, sideDb { 0.0f }, seitenanteilDb { 0.0f };
    /** Pearson-Korrelation zwischen L und R, bandbegrenzt: der Realteil des
        gemittelten Kreuzspektrums, normiert auf die zwei Autospektren. „Kurz"
        geht ueber `kKorrelationKurzFrames` Welch-Frames, „mittel" ueber das
        ganze Evidenzfenster — zwei Fenster, weil eine kurzzeitig wandernde
        Korrelation etwas anderes ist als eine dauerhaft niedrige. */
    float korrelationKurz { 0.0f }, korrelationMittel { 0.0f };
    /** Praesenzbits der zwei Korrelationen (NAK-181 R5, G4-Befund V06).

        🔑 Bis NAK-181 hingen beide Felder auf der Leitung an `basisGesetzt`
        — einem Bit, das eine ANDERE Frage beantwortet („wurde in diesem Band
        ueberhaupt Energie integriert"). Bei der Vorgabekadenz 0,25 s schliesst
        das Kurzfenster nie (5,86 Hops bei 48 kHz, Schwelle 8), und bei einem
        stillen Kanal ist der Nenner der Mittelkorrelation null: beide reisten
        als GEMESSENE 0,0. M-08 verlangt an genau dieser Stelle, dass „bei
        einem stillen Kanal das Praesenzbit wegfaellt, statt 0 oder NaN zu
        senden", M-11 „null bei zu wenig Frames, nie ein geschaetzter Wert". */
    bool  korrelationKurzGesetzt { false };
    bool  korrelationMittelGesetzt { false };

    /** Magnitude-Squared Coherence in [0, 1]. Stufe 1 des fail-closed. */
    bool  kohaerenzGesetzt { false };  float kohaerenz { 0.0f };
    /** Interchannel-Phase in Radiant. Stufe 2 des fail-closed. */
    bool  phaseGesetzt { false };      float phaseRad { 0.0f };

    /** Anteil der abgeschlossenen Kurzfenster, in denen dieses Band kohaerent
        war — „wie stabil ist der Befund". 0 heisst nicht „keine Persistenz",
        sondern „in keinem Kurzfenster kohaerent"; ohne `basisGesetzt` hat das
        Feld gar keinen Wert. */
    float persistenz { 0.0f };
    /** Praesenzbit der Persistenz (NAK-181 R5a).

        🔑 Es haengt an einem BANDWEISEN Nachweis, nicht am globalen
        `stereoKurzfenster`: dieser Zaehler steigt bei jedem achten Hop, auch
        wenn das Band in keinem der Fenster einen gueltigen Nenner hatte. Bei
        einem stillen Kanal waere die Persistenz dann `0 / N` MIT Bit — wieder
        die gemessene 0. Gezaehlt wird deshalb `stereoKurzfensterBand[b]`,
        genau dort, wo `nenner > 0` gilt.

        Der NENNER der Persistenz bleibt global: sie ist der „Anteil der
        abgeschlossenen Kurzfenster, in denen dieses Band kohaerent war", und
        die Bezugsmenge ist das Evidenzfenster. Der neue Zaehler entscheidet
        nur ueber das Bit. */
    bool  persistenzGesetzt { false };

    /** P10/P50/P95 des Seitenanteils UEBER DIE ZEIT des Evidenzfensters —
        nicht ueber die Baender. Braucht mindestens vier Werte, dieselbe
        Untergrenze wie die Bandkonvergenz: zwei Perzentile aus je einem Wert
        sind kein Verlauf. */
    bool  perzentileGesetzt { false };
    float p10Db { 0.0f }, p50Db { 0.0f }, p95Db { 0.0f };

    /** §40.1 woertlich: "Fensterdauer und Freiheitsgrade werden Teil der
        Evidenz." Beide je Band, weil die Zahl gueltiger Frames je Band
        verschieden sein kann — ein Band ueber der Nyquist-Kappe hat null. */
    float         fensterDauerMs { 0.0f };
    std::uint32_t freiheitsgrade { 0 };
};

/** Die zwei Stereoskalare des Evidenzfensters (M-08, §40.3).

    Beide sind ausdruecklich GEMESSEN und nicht aus einer Korrelation
    geschaetzt: der Mono-Folddown am wirklich gefalteten Puffer `(L+R)/2`, die
    Balance aus den zwei Kanalenergien. §40.3 verlangt eine Uebereinstimmung
    innerhalb 0,25 dB mit dem echten Folddown — eine Schaetzung aus dem
    Korrelationsskalar liegt bei dekorrelierten Kanaelen um Dezibel daneben. */
struct StereoSkalare
{
    bool  folddownGesetzt { false };  float monoFolddownDb { 0.0f };
    bool  balanceGesetzt  { false };  float lrBalanceDb    { 0.0f };
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

    /** SONDE-013 M-07: nicht-endliche EINGANGSsamples, die in diesen Rahmen
        beziehungsweise in dieses Evidenzfenster gelaufen sind.

        Sie sind der GEZAEHLTE Teil der NaN-Ehrlichkeit; die Verriegelung ist
        der andere: ist `nichtEndlichRahmen` groesser als 0, traegt der Rahmen
        seine sampleabhaengigen Skalare NICHT (Wert 0, Praesenzbit falsch)
        statt eine Zahl aus stillgelegtem Audio zu melden. `nichtEndlichEvidenz`
        reist im Evidenzsnapshot und deckelt dort die Konfidenzklasse.

        Beide Zahlen sind IMMER gesetzt: 0 heisst nachweislich keines. Sie
        brauchen deshalb kein Praesenzbit. */
    std::uint32_t nichtEndlichRahmen  { 0 };
    std::uint32_t nichtEndlichEvidenz { 0 };

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
inline constexpr std::uint32_t kFeatureMetricsVersion = 20260904u;

/*  ⚠️ WARUM DIE ZAHL MIT SONDE-013 STEIGT — und warum sie es MUSS.

    Zwei Gruende, und beide waeren ohne diesen Schritt still:

    1. Vier neue kalibrierbare Schwellen (Peaksteigung, Peakcrest,
       Kohaerenzschwelle der Phase, die zwei Konfidenzgates).
    2. `psrDb` rechnet seit diesem Ticket gegen das True-Peak-Maximum des
       3-s-Fensters statt gegen den Sample-Peak des 100-ms-Rahmens. Dasselbe
       Feld, dieselbe Feld-ID, ANDERE Bedeutung — ohne die Version haette ein
       Empfaenger kein Mittel, die zwei Faelle zu unterscheiden.

    Die Schwellen dieser Fassung stehen in
    `eq-copilot/schemas/v3/metriken-v1.json`; **A5**
    (`metrics_version_bindet_schwellen`) haelt Register und Code
    gegeneinander und faellt, wenn eine Zahl ohne Versionsschritt wandert. */

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

/** Ab welchem Anstieg des Rahmenpeaks gegenueber dem Vorrahmen der EIGENE
    Peakpfad des Ereignisdetektors ausloest (SONDE-013 M-86, §39.1: Detektor
    aus spektralem Fluss, Peaksteigung UND Crest).

    12 dB ist ein Faktor 4 im Pegel. Die Wahl ist begruendet, nicht geraten:
    unter 6 dB liegt die normale Pegelschwankung zwischen zwei
    100-ms-Rahmen eines durchlaufenden Arrangements, und ein Detektor, der
    dort ausloest, feuert dauernd. 12 dB trennt einen wirklichen Einsatz von
    einer Lautstaerkebewegung.

    Startwert, am Korpus kalibrierbar (§5.3, Risiko 5) — deshalb steht er
    HIER neben `kFeatureMetricsVersion` und nicht als Literal im Rechenpfad. */
inline constexpr double kPeakSteigungSchwelleDb = 12.0;

/** Ab welchem Crest im Rahmen das Peak-Qualitaetsbit gilt.

    Dieselbe 12 dB, aber eine ANDERE Groesse: hier Peak gegen RMS INNERHALB
    eines Rahmens, oben Peak gegen Peak ZWISCHEN zwei Rahmen. Der Wert stand
    bis SONDE-013 als nacktes Literal im Detektor; er ist damit dieselbe
    Kalibrierungsfalle gewesen, die §5.3 Risiko 5 beschreibt. */
inline constexpr double kPeakCrestSchwelleDb = 12.0;

/** Wie viele gueltige Welch-Frames ein Band mindestens braucht, damit seine
    Kohaerenz ueberhaupt einen Wert bekommt (SONDE-013 M-11, §40.1 woertlich:
    "Auto- und Cross-Spektren werden ueber mindestens ACHT gueltige
    ueberlappende Welch-Frames gemittelt").

    Der Grund ist nicht Geschmack, sondern Statistik: die Magnitude-Squared
    Coherence aus EINEM Frame ist identisch 1, ganz gleich wie unkorreliert
    die zwei Kanaele sind — |L·conj(R)|² = |L|²·|R|² gilt fuer jedes einzelne
    Bin exakt. Erst die Mittelung ueber mehrere Frames macht daraus eine
    Aussage. Acht ist der Startwert aus §40.1; er lebt hier neben
    `kFeatureMetricsVersion` und nicht als Literal im Rechenpfad. */
inline constexpr int kWelchMindestFrames = 8;

/*  ⚠️ WAS DIESE ZAHL FUER DIE BASSSTUFE BEDEUTET, gemessen beim Bau.

    Die Bassstufe hat bei 48 kHz einen Hop von 8192 Samples, also 170,7 ms je
    Frame. Acht davon sind 1,37 s — laenger als das laengste Evidenzfenster
    (`kEvidenzIntervallMaxS` = 1 s). Baender unter `kTrennungHz` tragen damit
    im heutigen Kadenzrahmen NIE eine Kohaerenz und nie eine Phase.

    Das ist fail-closed und kein Fehler: die Kohaerenz aus fuenf Frames waere
    unbrauchbar, und §40.1 verlangt genau dieses Schweigen. Der Empfaenger
    sieht den Grund an den `freiheitsgrade` des Bandes. Aber es ist auch keine
    gute Produkteigenschaft — Phasenprobleme im Bass sind musikalisch die
    wichtigsten. Der Punkt steht als Nebenbefund im Manifest §10.4; ihn zu
    beheben hiesse, der Bassstufe fuer die Stereoanalyse ein eigenes,
    kuerzeres Fenster zu geben, und das ist mehr als eine Zeile. */

/** Ab welcher Kohaerenz die Interchannel-Phase eines Bandes ueberhaupt
    ausgewertet wird (M-11, §40.1: "Interchannel-Phase wird nur in ausreichend
    kohaerenten Baendern interpretiert").

    0,8 ist dieselbe Schwelle, die §38.3 fuer einen zulaessigen Transferwert
    nennt — die Frage ist dieselbe: ab wann beschreibt das Kreuzspektrum eine
    Beziehung und nicht zwei unabhaengige Zufallszeiger. Startwert, am Korpus
    kalibrierbar (§5.3, Risiko 5). */
inline constexpr double kKohaerenzSchwellePhase = 0.8;

/** Laenge des KURZEN Korrelationsfensters in Welch-Frames.

    §40.1 verlangt die bandweise Korrelation "in kurzen und mittleren
    Fenstern". Das mittlere ist das ganze Evidenzfenster; das kurze sind
    `kKorrelationKurzFrames` Frames, also bei 4096 Punkten und 50 % Ueberlappung
    rund 340 ms. Es ist zugleich das Fenster, ueber dem die Persistenz gezaehlt
    wird: „in wie vielen dieser Abschnitte war das Band kohaerent". */
inline constexpr int kKorrelationKurzFrames = 8;

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



} // namespace nakama::analyse
