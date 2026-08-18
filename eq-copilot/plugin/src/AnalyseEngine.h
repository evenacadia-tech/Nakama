// M1 — deterministische Einzelinstanz-Messung (Plan §5.10, §11 M1).
//
// Läuft im Worker-Thread des Processors, NIE im Audiothread. Kennt keinerlei
// UI: Anzeige-Glättung, Roh/Slope, Kurvenwahl passieren ausschließlich auf der
// Snapshot-KOPIE im Editor — dadurch ist "UI-Zustand beeinflusst DSP nicht"
// (M1-Abnahme) eine Eigenschaft der Architektur, kein Versprechen.
//
// Vier Welch-Stufen auf der mittleren KANALenergie: PSD = (PSD_L + PSD_R)/2.
//   Ein Mono-Mix 0.5·(L+R) löschte Side-/Antiphase-Anteile komplett aus
//   (breite Chöre/Pads erschienen „inaktiv" — Übergabe-Paket B); das
//   Leistungsmittel ist bei L==R bitidentisch zum alten Mid-Spektrum, weshalb
//   die analyze-track-Kreuzvalidierung (L==R-Fixtures) der Maßstab bleibt.
//   16384 Bass (<200 Hz, §5.10.1 Bassfenster-Pflicht) · 8192 REFERENZACHSE
//   (exakt die Achse von tools/analyze-track.py — Maßstab der Kreuzvalidierung)
//   · 4096 Mitten · 2048 Höhen. Komposit in LeistungsDICHTE (dB/Hz), weil
//   'spectrum'-Bandmittel mit der FFT-Länge wandern; für Anzeige/Vergleich wird
//   der konstante ENBW-Offset der 8192er-Achse addiert (Werte decken sich dann
//   mit den analyze-track-Tafeln des Users).
//
// Robustheit: nicht-endliche Eingangssamples werden VOR jeder Rechnung durch
// Stille ersetzt und gezählt (nanErsetzt) — Akkus, Biquads und EMA bleiben
// dadurch beweisbar endlich; das Audio selbst berührt die Engine nie.
// LTAS endet bei min(18 kHz, 0.95·Nyquist): darüber bleibt NaN („nicht
// messbar"), nie eine fortgeschriebene Randevidenz.
//
// Loudness: BS.1770-Kette wie pyloudnorm (RBJ-High-Shelf +4 dB/1500 Hz/Q√½⁻¹ +
// Hochpass 38 Hz/Q 0.5, 400-ms-Blöcke, 75 % Overlap, Gating −70 abs/−10 rel).
// True Peak: 8×-Polyphase, Kaiser β 5.0, 161 Taps (scipy resample_poly-gleich).
//
// Alle Schwellen sind versionierte Startwerte (kMetricsVersion) — Änderung nur
// über neue Version, nie still (§5.10).
#pragma once

#include <juce_dsp/juce_dsp.h>
#include "ZonenRegeln.h"
#include <array>
#include <limits>
#include <mutex>
#include <vector>

namespace eqcop
{

// m4-2026-08-15 (Benchmark-Studie RESO/smart:EQ/Pro-Q → M3a):
//  · Resonanz-Basislinie = MEDIAN ±1/2 Okt statt Mittel ±1/3 Okt — das Mittel
//    wurde vom Peak selbst hochgezogen und unterschätzte die Prominenz.
//  · Band-Perzentile P10/P50/P95 aus dem vorhandenen Pegelhistogramm.
//  · Zonen-Zeitverlauf: aktivitäts-getaktete 1-s-Ticks je Diagnose-Zone.
//  · Konvergenz („Kurve steht"): Komposit-Abweichung gegen den Stand von
//    vor ~10 s Aktivzeit.
//  · auswertenLeicht(): 20-Hz-Publikation der leichten Felder (FPS-Fix —
//    die sichtbare Datenrate war vorher an den 250-ms-Schwertakt gekettet).
// m4.1 (Kalibrier-Runde 1, Testtrack): Zonen-Zeitverlauf-Ticks brauchen einen
// Mindestpegel — ohne ihn zaehlte die Haerte-Zone „50 % jenseits" auf einem
// Track mit 0,24 % Energie ueber 2 kHz (Schulterlinie fiel steil, die Live-EMA
// hielt nur Teppich; die Karten schwiegen korrekt, aber die Zahl log).
inline constexpr const char* kMetricsVersion = "m4.1-2026-08-15";

// 1/24-Okt-Raster der LTAS — identisch zu analyze-track.py _log_spectrum():
// edges = 30·2^(k/24), centers = √(edge·edge), 221 Bänder bis <18 kHz.
inline constexpr int    kLtasBpo  = 24;
inline constexpr double kLtasFmin = 30.0;
inline constexpr double kLtasFmax = 18000.0;
inline constexpr int    kLtasBaender = 221;

enum class MessZustand { keineDaten, sammelt, messbereit };
enum class AbdeckungsKlasse { nichtMessbar, eingeschraenkt, belastbar };
enum class ResonanzKlasse { zeitweise, dauerhaft };

struct ResonanzKandidat
{
    double freqHz = 0.0;
    double dbOver = 0.0;          // Spitze über der 1/3-Okt-geglätteten Hüllkurve
    double persistenzAnteil = 0.0; // Anteil der aktiven Segmente mit ≥6 dB excess
    double breiteOktaven = 0.0;   // Breite des −6-dB-Bereichs unter der Spitze
    ResonanzKlasse klasse = ResonanzKlasse::zeitweise;
};

struct DrittelOktavAbdeckung
{
    double zentrumHz = 0.0;
    double anteil = 0.0;          // Anteil aktiver Frames über Bandteppich+6 dB
    AbdeckungsKlasse klasse = AbdeckungsKlasse::nichtMessbar;
};

struct MessSnapshot
{
    MessZustand zustand = MessZustand::keineDaten;
    double aktivSekunden = 0.0;
    double gesamtSekunden = 0.0;
    double samplerate = 0.0;

    // Loudness/Dynamik — lufsGueltig=false heißt: kein Block über dem
    // absoluten Gate (Referenz-JSON kodiert das als null, nie als Zahl).
    bool   lufsGueltig = false;
    double lufsIntegriert = 0.0;
    bool   lufsShortGueltig = false;
    double lufsShort = 0.0;       // letzte 3 s
    double truePeakDb = -200.0;   // dBTP über die ganze Passage (ungegated)
    double crestDb = 0.0;
    bool   crestGueltig = false;

    // Spektral-Skalare — von der 8192er-Referenzachse (Kreuzvalidierung).
    bool   spektralGueltig = false;
    double centroidMagHz = 0.0;
    double rolloffHz = 0.0;
    double lowFrac = 0.0;
    double flatness = 0.0;
    std::array<double, 8> band8Prozent {};

    // Stereo
    double width = 0.0;
    double corr = 1.0;
    bool   stereoGueltig = false;

    // LTAS auf dem 1/24-Okt-Raster, dB in analyze-track-Skala
    // ('spectrum'@8192-Äquivalent). NaN = Band (noch) unbelegt.
    std::array<double, kLtasBaender> ltasZentrenHz {};
    std::array<double, kLtasBaender> ltasKompositDb {};   // Mehrfachauflösung §5.10.1
    std::array<double, kLtasBaender> ltasReferenzDb {};   // reine 8192er-Achse
    std::array<double, kLtasBaender> ltasLiveDb {};       // 3-s-EMA (§5.10.1 Anzeige)
    bool ltasGueltig = false;

    std::vector<DrittelOktavAbdeckung> abdeckung;
    std::vector<ResonanzKandidat> resonanzen;   // nur Bänder mit Klasse belastbar

    // M3a: Band-Perzentile aus dem 1-dB-Pegelhistogramm der zuständigen Stufe
    // (nur aktive Segmente; 1-dB-Quantisierung; NaN = Band ohne Segmente).
    // P95−P50 trennt Moment-Probleme von Dauerzuständen (Werkzeugwahl).
    std::array<double, kLtasBaender> perzentilP10 {};
    std::array<double, kLtasBaender> perzentilP50 {};
    std::array<double, kLtasBaender> perzentilP95 {};
    bool perzentileGueltig = false;

    // M3a: Zeitverlauf je Diagnose-Zone (ZonenRegeln.h) — Anteil der aktiven
    // 1-s-Ticks, in denen die Zone jenseits ihrer Regel-Schwelle lag. Ersetzt
    // die ehrliche 0,5-Deckelung der Zonen-Konfidenz durch echte Evidenz.
    struct ZonenZeit
    {
        bool gueltig = false;
        double anteil = 0.0;
        juce::uint32 ticks = 0;
    };
    std::array<ZonenZeit, kZonenAnzahl> zonenZeit {};

    // M3a: Konvergenz („Kurve steht") — mittlere/maximale Abweichung der
    // Komposit-LTAS gegen den Stand von vor ~10 s AKTIVZEIT. Rein informativ
    // (UI/Snapshot); false/NaN bis das erste Fenster voll ist.
    double konvergenzMeanDb = std::numeric_limits<double>::quiet_NaN();
    double konvergenzMaxDb  = std::numeric_limits<double>::quiet_NaN();
    bool   konvergenzGueltig = false;

    // Publikationszähler (FPS-Fix): der Editor malt nur bei neuer Revision.
    juce::uint64 revision = 0;

    juce::uint64 verarbeiteteSamples = 0;
    // Nicht-endliche Eingangssamples, die die ANALYSE seit dem letzten Reset
    // durch Stille ersetzt hat (das Audio bleibt unberührt — Passthrough).
    juce::uint64 nanErsetzt = 0;
};

class AnalyseEngine
{
public:
    AnalyseEngine();

    // SINGLE-WRITER-KONTRAKT: vorbereiten/zuruecksetzen/verarbeite/auswerten
    // dürfen nur von EINEM Thread (dem Worker) aufgerufen werden. Reset- und
    // Samplerate-Wünsche anderer Threads laufen als Atomics über den Processor
    // und werden vom Worker ausgeführt. Einzig snapshot() ist threadsicher.
    void vorbereiten (double samplerate);
    void zuruecksetzen();

    // Interleaved L/R (immer 2·frames Werte). kanaele==1 ⇒ R ist L-Duplikat,
    // LUFS läuft dann einkanalig, Stereo-Maße werden als ungültig markiert.
    void verarbeite (const float* interleaved, int frames, int kanaele);

    // Schwere Auswertung (Gating, Kandidaten, Abdeckung) — im Worker-Takt
    // aufrufen, nicht pro Block. Danach liefert snapshot() den frischen Stand.
    void auswerten();

    // Leichte Publikation (FPS-Fix, M3a): aktualisiert NUR Live-Kurve,
    // Kurz-LUFS, True Peak, Crest und Zustand im veröffentlichten Snapshot —
    // jeden Worker-Tick (~20 Hz) aufrufbar. Ohne neue Samples publiziert sie
    // nichts (Revision steht ⇒ der Editor malt im Leerlauf nicht).
    // Gleicher Single-Writer-Kontrakt wie auswerten().
    void auswertenLeicht();

    MessSnapshot snapshot() const;   // beliebiger Thread

private:
    struct WelchStufe
    {
        int n = 0, hop = 0, gefuellt = 0;
        double fs = 0.0;
        std::vector<float> ringL, ringR;  // Fensterpuffer je Kanal (n)
        std::vector<float> fenster;       // Hann periodisch
        std::vector<float> fftDaten;      // 2·n für juce::dsp::FFT
        std::vector<double> summePsd;     // Σ Dichte-PSD aktiver Segmente (Bins)
        juce::uint64 aktiveSegmente = 0;
        std::unique_ptr<juce::dsp::FFT> fft;
        double dichteFaktor = 0.0;        // 2/(fs·Σw²)
        // vorberechnete Bin-Fenster [von,bis) je 1/24-Band; bis==von ⇒ leer
        std::array<int, kLtasBaender> bandBinVon {};
        std::array<int, kLtasBaender> bandBinBis {};

        void init (int ordnung, double samplerate,
                   const std::array<double, kLtasBaender>& kantenLo,
                   const std::array<double, kLtasBaender>& kantenHi);
        // true ⇒ ein Segment wurde fertig gerechnet: einzelPsd hält n/2+1
        // Dichte-Werte als KANALenergie-Mittel (PSD_L+PSD_R)/2, warAktiv das
        // −60-dBFS-Gate über dieselbe Kanalenergie. stereo=false spart die
        // zweite FFT (R ist dann per FIFO-Vertrag das L-Duplikat).
        bool schiebe (float l, float r, bool stereo,
                      std::vector<double>& einzelPsd, bool& warAktiv);
    };

    double sr = 0.0;

    // ── Welch-Stufen ──
    WelchStufe bass;       // 16384
    WelchStufe referenz;   // 8192 — analyze-track-Achse
    WelchStufe mitten;     // 4096
    WelchStufe hoehen;     // 2048
    std::vector<double> scratchPsd;

    // ── 1/24-Okt-Akkumulation (Dichte, linear) ──
    // Die drei Auflösungs-Stufen laufen asynchron (Hop 8192 vs. 1024) — die
    // Komposition mit Übergabebändern passiert deshalb erst in
    // finalisiereLtas(), nie zur Segmentzeit. Pro Stufe ein Akku.
    struct BandAkku
    {
        std::array<double, kLtasBaender> summe {};
        std::array<juce::uint64, kLtasBaender> segmente {};
        // Persistenz §5.10.3: Segmente, in denen das Band ≥6 dB über der
        // 1/3-Okt-Glättung SEINES Segments lag / Segmente gesamt.
        std::array<juce::uint64, kLtasBaender> excessSegmente {};
    };

    // Ein fertiges Welch-Segment in die Bandwelt einarbeiten. Aktive Segmente
    // füllen Akku [akkuVon,akkuBis) + Histogramm/Persistenz im
    // Zuständigkeitsbereich [zustVon,zustBis); inaktive nur den Teppich.
    void segmentInBaender (const WelchStufe&, const std::vector<double>& einzelPsd,
                           bool aktiv, BandAkku& akku,
                           int akkuVon, int akkuBis, int zustVon, int zustBis);
    // Gemeinsamer Leicht-Teil beider Auswertungen: Zustand, Sekunden,
    // Live-Kurve, True Peak, Crest, Kurz-LUFS — EINE Quelle, damit der
    // 20-Hz-Pfad nie von der 250-ms-Auswertung abweichen kann.
    void fuelleBasis (MessSnapshot&) const;
    void finalisiereLtas (MessSnapshot&) const;
    void finalisiereLoudness (MessSnapshot&) const;
    void finalisiereSkalar (MessSnapshot&) const;
    void findeResonanzen (MessSnapshot&) const;
    void berechneAbdeckung (MessSnapshot&) const;
    void berechnePerzentile (MessSnapshot&) const;
    // M3a: ein aktivitäts-getakteter 1-s-Tick des Zonen-Zeitverlaufs —
    // läuft in verarbeite() (nicht im Auswerte-Takt), damit die Persistenz
    // unabhängig von der Auswerte-Kadenz und im GoldenTest beweisbar ist.
    void zonenTick();

    BandAkku bassAkku;     // 16384 — zuständig < 200 Hz (+ Naht bis 250)
    BandAkku mittenAkku;   // 4096 — 200–2000 (+ Nähte 160–250, 1.6k–2.5k)
    BandAkku hoehenAkku;   // 2048 — ≥ 2000 (+ Naht ab 1.6k)
    BandAkku referenzAkku; // reine 8192er über die volle Achse
    std::array<double, kLtasBaender> zentrenHz {};
    std::array<double, kLtasBaender> kantenLoHz {};
    std::array<double, kLtasBaender> kantenHiHz {};
    // Band-Indizes: harte Zuständigkeit (Kandidaten/Histogramm) und Nahtzonen
    // (Kurven-Blend) — bei vorbereiten() aus den Zentren bestimmt.
    int idxZust200 = 0, idxZust2000 = 0;
    int idxNaht160 = 0, idxNaht250 = 0, idxNaht1600 = 0, idxNaht2500 = 0;
    // Messbarer LTAS-Bereich [0, ltasBisBand): Bänder mit Zentrum oberhalb
    // min(18 kHz, 0.95·Nyquist) bleiben NaN — nie fortgeschriebene Randwerte.
    int ltasBisBand = kLtasBaender;
    // Abdeckung §5.10.2 ohne Henne-Ei (der Teppich steht erst am Ende fest):
    // pro Band ein 1-dB-Histogramm der Segment-Pegel der ZUSTÄNDIGEN Stufe
    // über aktive Segmente; inaktive Segmente füttern nur den Teppich-Tracker.
    static constexpr int kHistMinDb = -130, kHistMaxDb = 10;
    static constexpr int kHistStufen = kHistMaxDb - kHistMinDb + 1;   // 141
    std::vector<juce::uint32> pegelHistogramm;    // [kLtasBaender * kHistStufen]
    std::array<double, kLtasBaender> teppichInaktivDb {};   // min über inaktive
    juce::uint64 inaktiveSegmente = 0;
    // Live-Hüllkurve §5.10.1: 3-s-EMA der Segment-Band-LEISTUNG (nie dB!),
    // nur aus der zuständigen Stufe (kein Naht-Flackern), nur aktive Segmente.
    std::array<double, kLtasBaender> liveEmaLinear {};

    // ── M3a: Zonen-Zeitverlauf (Ticks je 1 s AKTIVZEIT, auf der Live-EMA) ──
    juce::uint64 tickLetzteAktiveZellen = 0;
    std::array<juce::uint32, kZonenAnzahl> zonenTicksGesamt {};
    std::array<juce::uint32, kZonenAnzahl> zonenTicksJenseits {};

    // ── M3a: Konvergenz-Referenz (nur Worker) ──
    std::array<double, kLtasBaender> konvRefKurveDb {};
    double konvRefAktivS = -1.0;
    double konvLetzterMeanDb = std::numeric_limits<double>::quiet_NaN();
    double konvLetzterMaxDb  = std::numeric_limits<double>::quiet_NaN();
    bool   konvLetzterGueltig = false;

    // Publikationszähler — monotone Revision über beide Auswerte-Pfade.
    juce::uint64 revisionZaehler = 0;

    // ── Loudness (BS.1770, pyloudnorm-gleich) ──
    struct Biquad
    {
        double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        double z1 = 0, z2 = 0;
        double tick (double x) noexcept
        {
            const double y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };
    Biquad shelfL, shelfR, hpL, hpR;
    // 100-ms-Zellen der K-gewichteten Energie (Σ über Kanäle) + Kanalenergie-
    // Zellen für Aktivität §5.10.2. 10 Werte/s — unproblematisch über Stunden.
    int zellenSamples = 0, zellenStand = 0;
    double zelleKEnergie = 0.0, zelleAktivEnergie = 0.0;
    std::vector<double> kZellen;        // Σ_ch Σ x_K² pro 100 ms
    juce::uint64 aktiveZellen = 0;      // Zellen über dem −60-dBFS-Gate

    // ── True Peak: 8×-Polyphase (Kaiser β5, 161 Taps) ──
    static constexpr int kTpTaps = 161;
    static constexpr int kTpFaktor = 8;
    std::array<double, kTpTaps> tpFir {};
    std::vector<float> tpVerlaufL, tpVerlaufR;   // Filterzustand (letzte Taps/8)
    double truePeakLinear = 0.0;

    // ── Peak/RMS/Stereo (laufende Summen) ──
    double peakAbs = 0.0;
    double summeMid2 = 0.0, summeSide2 = 0.0;   // echte M/S-Größen (width)
    double summeL = 0.0, summeR = 0.0, summeL2 = 0.0, summeR2 = 0.0, summeLR = 0.0;
    juce::uint64 samplesGesamt = 0;
    bool istStereo = false;
    // Zähler der NaN/Inf-Ersetzungen im Analysezweig (seit Reset).
    juce::uint64 nanErsetzt = 0;

    // ── Ergebnis-Doppelpuffer ──
    mutable std::mutex snapMutex;
    MessSnapshot fertig;

    JUCE_DECLARE_NON_COPYABLE (AnalyseEngine)
};

} // namespace eqcop
