#include "DiagnoseAntwort.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace nakama::diagnose
{

namespace
{
bool addiere (std::int64_t a, std::int64_t b, std::int64_t& aus) noexcept
{
    if ((b > 0 && a > std::numeric_limits<std::int64_t>::max() - b)
        || (b < 0 && a < std::numeric_limits<std::int64_t>::min() - b))
        return false;
    aus = a + b;
    return true;
}

bool subtrahiere (std::int64_t a, std::int64_t b, std::int64_t& aus) noexcept
{
    if ((b < 0 && a > std::numeric_limits<std::int64_t>::max() + b)
        || (b > 0 && a < std::numeric_limits<std::int64_t>::min() + b))
        return false;
    aus = a - b;
    return true;
}

juce::var ganz (std::uint64_t v)
{
    const auto grenze = (std::uint64_t) std::numeric_limits<juce::int64>::max();
    return juce::var ((juce::int64) std::min (v, grenze));
}

juce::var gleitkomma (bool gesetzt, double v)
{
    return (gesetzt && std::isfinite (v)) ? juce::var (v) : juce::var();
}

juce::var grund (const char* feld, const char* code, const char* text)
{
    auto* g = new juce::DynamicObject();
    g->setProperty ("feld", feld);
    g->setProperty ("code", code);
    g->setProperty ("text", text);
    return juce::var (g);
}
} // namespace

//==============================================================================

juce::var snapshotObjekt (const eqcop::MessSnapshot& m, const SnapshotSensor& sensor,
                          const juce::String& createdUtc, const Befundteil* befundteil)
{
    using namespace eqcop;
    auto zahl = [] (double v, bool gueltig = true)
    {
        return (gueltig && std::isfinite (v)) ? juce::var (v) : juce::var();
    };
    auto* wurzel = new juce::DynamicObject();
    // v3 (M3a): zusätzlich Band-Perzentile, Zonen-Zeitverlauf und Konvergenz —
    // v2 brachte die Befundkarten. Ältere Leser ignorieren neue Felder.
    wurzel->setProperty ("snapshot_version", 3);
    wurzel->setProperty ("metrics_version", kMetricsVersion);
    if (befundteil != nullptr)
        wurzel->setProperty ("diagnose_version", befundteil->diagnoseVersion);
    wurzel->setProperty ("created_utc", createdUtc);

    {
        auto* s = new juce::DynamicObject();
        s->setProperty ("sensor_id", sensor.sensorId);
        s->setProperty ("role", sensor.rolle);
        s->setProperty ("label", sensor.label);
        s->setProperty ("pair_id", sensor.paarId.isEmpty() ? juce::var() : juce::var (sensor.paarId));
        s->setProperty ("samplerate", m.samplerate);
        s->setProperty ("channels", sensor.kanaele);
        wurzel->setProperty ("sensor", juce::var (s));
    }

    wurzel->setProperty ("zustand", m.zustand == MessZustand::messbereit ? "messbereit" : "sammelt");
    wurzel->setProperty ("aktiv_sekunden", m.aktivSekunden);
    wurzel->setProperty ("gesamt_sekunden", m.gesamtSekunden);
    // Ehrlichkeits-Ausweis (Paket C): so viele nicht-endliche Eingangssamples
    // hat die Analyse seit dem Messstart durch Stille ersetzt.
    wurzel->setProperty ("nan_ersetzt_samples", (juce::int64) m.nanErsetzt);

    {
        auto* loud = new juce::DynamicObject();
        loud->setProperty ("lufs_integriert", zahl (m.lufsIntegriert, m.lufsGueltig));
        loud->setProperty ("lufs_short", zahl (m.lufsShort, m.lufsShortGueltig));
        loud->setProperty ("true_peak_dbtp", zahl (m.truePeakDb));
        loud->setProperty ("crest_db", zahl (m.crestDb, m.crestGueltig));
        wurzel->setProperty ("loudness", juce::var (loud));
    }
    {
        auto* sp = new juce::DynamicObject();
        sp->setProperty ("centroid_mag_hz", zahl (m.centroidMagHz, m.spektralGueltig));
        sp->setProperty ("rolloff_hz", zahl (m.rolloffHz, m.spektralGueltig));
        sp->setProperty ("low_frac", zahl (m.lowFrac, m.spektralGueltig));
        sp->setProperty ("flatness", zahl (m.flatness, m.spektralGueltig));
        juce::Array<juce::var> band;
        for (const double v : m.band8Prozent)
            band.add (zahl (v, m.spektralGueltig));
        sp->setProperty ("band_pct", band);
        wurzel->setProperty ("spektral", juce::var (sp));
    }
    {
        auto* st = new juce::DynamicObject();
        st->setProperty ("width", zahl (m.width, m.stereoGueltig));
        st->setProperty ("corr", zahl (m.corr, m.stereoGueltig));
        st->setProperty ("echt_stereo", m.stereoGueltig);
        wurzel->setProperty ("stereo", juce::var (st));
    }
    {
        auto* ltas = new juce::DynamicObject();
        juce::Array<juce::var> zentren, komposit, referenzKurve;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            zentren.add (m.ltasZentrenHz[(size_t) b]);
            komposit.add (zahl (m.ltasKompositDb[(size_t) b], m.ltasGueltig));
            referenzKurve.add (zahl (m.ltasReferenzDb[(size_t) b], m.ltasGueltig));
        }
        ltas->setProperty ("zentren_hz", zentren);
        ltas->setProperty ("komposit_db", komposit);
        ltas->setProperty ("referenz_8192_db", referenzKurve);
        wurzel->setProperty ("ltas", juce::var (ltas));
    }
    {
        juce::Array<juce::var> ab;
        for (const auto& a : m.abdeckung)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("zentrum_hz", a.zentrumHz);
            o->setProperty ("anteil", a.anteil);
            o->setProperty ("klasse", a.klasse == AbdeckungsKlasse::belastbar ? "belastbar"
                                    : a.klasse == AbdeckungsKlasse::eingeschraenkt ? "eingeschraenkt"
                                                                                   : "nicht_messbar");
            ab.add (juce::var (o));
        }
        wurzel->setProperty ("abdeckung", ab);
    }
    {
        // M3a: Band-Perzentile (1-dB-Quantisierung, Zuständigkeits-Stufe).
        auto* pz = new juce::DynamicObject();
        juce::Array<juce::var> p10, p50, p95;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            p10.add (zahl (m.perzentilP10[(size_t) b], m.perzentileGueltig));
            p50.add (zahl (m.perzentilP50[(size_t) b], m.perzentileGueltig));
            p95.add (zahl (m.perzentilP95[(size_t) b], m.perzentileGueltig));
        }
        pz->setProperty ("p10_db", p10);
        pz->setProperty ("p50_db", p50);
        pz->setProperty ("p95_db", p95);
        wurzel->setProperty ("perzentile", juce::var (pz));
    }
    {
        // M3a: Zonen-Zeitverlauf (Anteil aktiver 1-s-Ticks jenseits der
        // Regel-Schwelle; Reihenfolge = ZonenRegeln.h).
        static const char* zonenNamen[kZonenAnzahl] = { "mitten_loch", "mulm", "haerte", "hoehen_hype" };
        juce::Array<juce::var> zz;
        for (int z = 0; z < kZonenAnzahl; ++z)
        {
            const auto& w = m.zonenZeit[(size_t) z];
            auto* o = new juce::DynamicObject();
            o->setProperty ("zone", zonenNamen[z]);
            o->setProperty ("gueltig", w.gueltig);
            o->setProperty ("anteil_jenseits", w.anteil);
            o->setProperty ("ticks", (int) w.ticks);
            zz.add (juce::var (o));
        }
        wurzel->setProperty ("zonen_zeit", zz);
    }
    {
        // M3a: Konvergenz „Kurve steht" (rein informativ).
        auto* ko = new juce::DynamicObject();
        ko->setProperty ("gueltig", m.konvergenzGueltig);
        ko->setProperty ("mean_db", zahl (m.konvergenzMeanDb, m.konvergenzGueltig));
        ko->setProperty ("max_db", zahl (m.konvergenzMaxDb, m.konvergenzGueltig));
        wurzel->setProperty ("konvergenz", juce::var (ko));
    }
    {
        juce::Array<juce::var> res;
        for (const auto& r : m.resonanzen)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("freq_hz", r.freqHz);
            o->setProperty ("db_over", r.dbOver);
            o->setProperty ("persistenz", r.persistenzAnteil);
            o->setProperty ("breite_okt", r.breiteOktaven);
            o->setProperty ("klasse", r.klasse == ResonanzKlasse::dauerhaft ? "dauerhaft" : "zeitweise");
            res.add (juce::var (o));
        }
        wurzel->setProperty ("resonanzen", res);
    }
    if (befundteil != nullptr)
        wurzel->setProperty ("befunde", befundteil->befunde);
    wurzel->setProperty ("raw_audio", juce::var());   // per Vertrag immer null

    return juce::var (wurzel);
}

//==============================================================================

void MaterialZaehler::zuruecksetzen() noexcept
{
    habeBlock = false;
    zeitDurchgehend = true;
    uebergelaufen = false;
    letzteZeitGueltig = false;
    letzterStart = 0;
    letzteLaenge = 0;
    summe = 0;
    stillstand = 0;
    // Der publizierte Stand der Engine faellt mit ihr - der festgehaltene
    // Materialstand beschreibt denselben Stand und faellt mit (F-15).
    fest = MaterialStand {};
}

void MaterialZaehler::blockGegeben (const nakama::echtzeit::StampedBlock& block) noexcept
{
    const bool zeit = (block.flags & nakama::echtzeit::kFlagZeitGueltig) != 0;
    groessterBlock = std::max (groessterBlock, (std::uint64_t) block.sampleCount);

    if (! habeBlock)
    {
        habeBlock = true;
        zeitDurchgehend = zeit;
    }
    else
    {
        if (! zeit || ! letzteZeitGueltig)
            zeitDurchgehend = false;
        if (zeitDurchgehend && ! uebergelaufen)
        {
            // P-10: Stempeldifferenz zum Vorgaenger, nicht dessen Laenge.
            std::int64_t differenz = 0;
            if (! subtrahiere (block.projectSampleStart, letzterStart, differenz)
                || ! addiere (summe, differenz, summe))
            {
                uebergelaufen = true;
            }
            else if (differenz < (std::int64_t) letzteLaenge)
            {
                // stehende oder zurueckweichende Hostzeit (FL-Teilblock)
                ++stillstand;
            }
        }
    }
    letzteZeitGueltig = zeit;
    letzterStart = block.projectSampleStart;
    letzteLaenge = block.sampleCount;
}

void MaterialZaehler::festhalten() noexcept
{
    MaterialStand s;
    if (habeBlock)
    {
        std::int64_t ende = 0;
        if (letzteZeitGueltig && addiere (letzterStart, (std::int64_t) letzteLaenge, ende))
        {
            s.endeGueltig = true;
            s.ende = ende;
        }
        // Summe plus Laenge des zuletzt gegebenen Blocks: bei fortlaufender
        // Zeit genau das Materialende minus Startstempel des ersten Blocks.
        std::int64_t fortlaufend = 0;
        if (zeitDurchgehend && ! uebergelaufen && addiere (summe, (std::int64_t) letzteLaenge, fortlaufend))
        {
            s.fortlaufendGueltig = true;
            s.fortlaufend = fortlaufend;
        }
        s.stillstandGueltig = true;
        s.stillstand = stillstand;
    }
    fest = s;
}

//==============================================================================

juce::var rahmenObjekt (const RahmenAuszug& r)
{
    const auto& f = r.rahmen;
    // Ein gebauter Rahmen traegt eine Sequenz ab 1 (`frameversuch`); nach
    // `zuruecksetzen()` der Engine steht wieder der leere Rahmen mit 0.
    const bool rahmen = f.transport.sequence != 0;
    const bool evidenz = rahmen && f.evidenzFrisch;

    auto* o = new juce::DynamicObject();
    // kumulativ, fallen nie
    o->setProperty ("frames_gebaut", ganz (r.framesGebaut));
    o->setProperty ("summe_fenster_gesamt", ganz (r.summeGesamt));
    o->setProperty ("summe_fenster_aktiv", ganz (r.summeAktiv));
    // der zuletzt gebaute Rahmen
    o->setProperty ("evidenz_frisch", rahmen ? juce::var (f.evidenzFrisch) : juce::var());
    o->setProperty ("samplerate", gleitkomma (rahmen, f.transport.sample_rate));
    o->setProperty ("aktivitaet", gleitkomma (rahmen && f.aktivitaetGesetzt, f.aktivitaet));
    o->setProperty ("abdeckung", gleitkomma (evidenz && f.abdeckungGesetzt, f.abdeckung));
    o->setProperty ("konvergenz", gleitkomma (evidenz && f.konvergenzGesetzt, f.konvergenz));
    o->setProperty ("evidenz_fenster", evidenz ? juce::var ((juce::int64) f.evidenzFenster) : juce::var());
    o->setProperty ("lufs_m", gleitkomma (rahmen && f.lufsMGesetzt, f.lufsM));
    o->setProperty ("lufs_s", gleitkomma (rahmen && f.lufsSGesetzt, f.lufsS));
    o->setProperty ("true_peak_db", gleitkomma (rahmen && f.truePeakGesetzt, f.truePeakDb));
    o->setProperty ("peak_db", gleitkomma (rahmen && f.peakGesetzt, f.peakDb));
    o->setProperty ("integration_samples", rahmen && f.integrationGesetzt
                                               ? juce::var ((juce::int64) f.integrationSamples) : juce::var());
    o->setProperty ("nicht_endlich_rahmen", rahmen ? juce::var ((juce::int64) f.nichtEndlichRahmen) : juce::var());
    // das offene Fenster, live und nur roh
    o->setProperty ("offen_fenster_gesamt", ganz (r.offenGesamt));
    o->setProperty ("offen_fenster_aktiv", ganz (r.offenAktiv));
    // Materialzeit
    juce::var schwer;
    if (r.schwerSamples == 0)
        schwer = 0.0;
    else if (std::isfinite (r.samplerate) && r.samplerate > 0.0)
        schwer = (double) r.schwerSamples / r.samplerate;
    o->setProperty ("schwer_sekunden", schwer);
    o->setProperty ("material_ende_projektsample", r.material.endeGueltig
                                                       ? juce::var ((juce::int64) r.material.ende) : juce::var());
    o->setProperty ("bloecke_max_samples", ganz (r.bloeckeMax));
    o->setProperty ("hostzeit_fortlaufend_samples", r.material.fortlaufendGueltig
                                                        ? juce::var ((juce::int64) r.material.fortlaufend) : juce::var());
    o->setProperty ("hostzeit_stillstand_bloecke", r.material.stillstandGueltig
                                                       ? ganz (r.material.stillstand) : juce::var());
    // der Materialausschnitt des Rahmens in Hostzeit
    const bool ausschnitt = rahmen && f.transport.project_sample_start_gesetzt;
    o->setProperty ("projekt_sample_start", ausschnitt ? juce::var ((juce::int64) f.transport.project_sample_start)
                                                       : juce::var());
    o->setProperty ("sample_count", ausschnitt ? juce::var ((juce::int64) f.transport.sample_count) : juce::var());
    o->setProperty ("spielt", rahmen && (f.transport.gueltigkeit & nakama::analyse::kGPlayState) != 0
                                  ? juce::var (f.transport.playing) : juce::var());
    return juce::var (o);
}

void fuelleTeile (const eqcop::MessSnapshot& m, const RahmenAuszug& auszug,
                  const juce::var& snapshot, Umschlag& u)
{
    const bool daten = m.zustand != eqcop::MessZustand::keineDaten;
    const bool rahmen = auszug.rahmen.transport.sequence != 0;
    u.snapshot = daten ? snapshot : juce::var();
    u.frame = (daten || rahmen) ? rahmenObjekt (auszug) : juce::var();
    u.gruende.clearQuick();
    if (! daten)
        u.gruende.add (grund ("snapshot", "noch_keine_messdaten",
                              "die Analyse hat seit dem letzten Ruecksetzen keine Messdaten"));
    if (! daten && ! rahmen)
        u.gruende.add (grund ("frame", "noch_keine_messdaten",
                              "seit dem letzten Ruecksetzen ist kein Rahmen gebaut"));
    u.gruende.add (grund ("aggregat", "rolle_ohne_aggregat",
                          "ein Plugin schreibt kein Aggregat; das Aggregat schreibt der Broker"));
}

std::string umschlagText (const Umschlag& u)
{
    auto* o = new juce::DynamicObject();
    o->setProperty ("format", kAntwortFormat);
    o->setProperty ("anfrage_id", juce::String (u.anfrageId));
    o->setProperty ("rolle", u.rolle);
    o->setProperty ("instanz_id", u.instanzId);
    o->setProperty ("laufzeit_id", juce::String (u.laufzeitId));
    o->setProperty ("pid", (juce::int64) u.pid);
    o->setProperty ("erzeugt_utc", juce::String (u.erzeugtUtc));
    o->setProperty ("version", u.version);
    o->setProperty ("snapshot", u.snapshot);
    o->setProperty ("frame", u.frame);
    o->setProperty ("aggregat", juce::var());
    o->setProperty ("gruende", u.gruende);
    const auto text = juce::JSON::toString (juce::var (o), true);
    return text.toStdString();
}

} // namespace nakama::diagnose
