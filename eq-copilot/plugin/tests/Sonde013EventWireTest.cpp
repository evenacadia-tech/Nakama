/*  EqCopSonde013EventWireTest — der Evidenzpfad in C++ (SONDE-013 M-05).

    §33.2 zaehlt den Inhalt des Evidenzsnapshots abschliessend auf: „volle 221
    Baender, P10/P50/P95, Abdeckung, Konvergenz, Ereignisse". Bis zu diesem
    Ticket hatte keiner dieser Punkte einen Weg — der Vertrag stand, der
    Erzeuger fehlte. Dieses Bein misst den Erzeuger.

    ES MISST NICHT DAS SCHEMA. Dass die erzeugte Nachricht dem Vertrag
    genuegt, prueft es zwar (mit derselben Engine wie B3c), aber das ist nur
    die Eintrittskarte. Die eigentliche Frage ist eine andere: sagt der
    Snapshot etwas, das der 10-Hz-Liveframe NICHT sagt? Deshalb steht hier
    kein einziger Fall, der bloss ein Feld auf Anwesenheit prueft. Jeder Fall
    stellt zwei Signale gegenueber, die sich im Mittelwert gleichen und im
    VERLAUF unterscheiden — genau der Unterschied, fuer den es die
    Verteilungspunkte gibt.

    Fail-closed ist die zweite Haelfte. Ein Snapshot ohne Anker, ohne
    Abdeckung oder mit widerspruechlichem Zeitstempel entsteht GAR NICHT; er
    entsteht nicht abgeschwaecht. Ein „leiser Wert" waere die falsche starke
    Aussage, die §34.2 verhindert.
*/

#include "../core/analysis/FeatureEngine.h"
#include "../core/StampedAudioQueue.h"
#include "../vertrag/NakamaEvidenz.h"
#include "../vertrag/NakamaVertrag.h"
#include "../core/ipc/IpcQueues.h"

#include <juce_core/juce_core.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace rt = nakama::echtzeit;
using nakama::analyse::FeatureEngine;
using nakama::analyse::FeatureFrame;

namespace
{
int bestanden = 0;
int fehler = 0;

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {})
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

void abschnitt (const char* text)
{
    std::cout << "\n== " << text << " ==" << std::endl;
}

juce::File finde (const juce::String& relativ)
{
    auto ausCwd = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    if (ausCwd.existsAsFile())
        return ausCwd;
    auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile)
                      .getParentDirectory();
    for (int i = 0; i < 10 && ordner.exists(); ++i)
    {
        auto kandidat = ordner.getChildFile (relativ);
        if (kandidat.existsAsFile())
            return kandidat;
        ordner = ordner.getParentDirectory();
    }
    return ausCwd;
}

/*  Derselbe Speiser wie in B5, auf das Noetige gekuerzt.

    Er fuehrt Strom und Projektzeit selbst — ein Test, der die Buchfuehrung
    dem Pruefling ueberliesse, pruefte gegen einen selbstgebauten Zeitfehler. */
struct Speiser
{
    FeatureEngine& engine;
    double sr { 48000.0 };
    int    frames { 512 };

    std::uint64_t strom { 0 };
    std::int64_t  projekt { 0 };
    std::vector<float> audio;

    explicit Speiser (FeatureEngine& e) : engine (e) {}

    rt::StampedBlock bauen() const
    {
        rt::StampedBlock b;
        b.stromVon = strom;
        b.sampleCount = (std::uint32_t) frames;
        b.segment = 0;
        b.startFolge = 0;
        b.kanaele = 2;
        b.tapMaske = 1;
        b.projectSampleStart = projekt;
        b.sampleRate = sr;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | rt::kFlagSpielt | rt::kFlagZeitGueltig;
        return b;
    }

    /** `f(i)` liefert das Sample beider Kanaele; gibt zurueck, ob ein Frame
        faellig wurde. */
    bool sende (const std::function<float (std::uint64_t)>& f)
    {
        const auto b = bauen();
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            const float v = f (strom + i);
            audio[(std::size_t) i * 2u]      = v;
            audio[(std::size_t) i * 2u + 1u] = v;
        }
        const bool r = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        projekt += (std::int64_t) b.sampleCount;
        return r;
    }
};

/** Ein Sinus mit fester Amplitude — das einfachste Signal, dessen
    Bandverlauf sich nicht selbst bewegt. */
std::function<float (std::uint64_t)> sinus (double amplitude, double hz, double sr)
{
    return [amplitude, hz, sr] (std::uint64_t n)
    {
        return (float) (amplitude * std::sin (2.0 * 3.14159265358979323846
                                              * hz * (double) n / sr));
    };
}

/** Faehrt so lange Bloecke, bis ein Evidenzframe faellig ist, und gibt ihn
    zurueck. `false`, wenn nach `maxBloecke` keiner kam. */
bool bisEvidenz (Speiser& s, const std::function<float (std::uint64_t)>& f,
                 FeatureFrame& aus, int maxBloecke = 400)
{
    for (int i = 0; i < maxBloecke; ++i)
        if (s.sende (f) && s.engine.frame().evidenzFrisch)
        {
            aus = s.engine.frame();
            return true;
        }
    return false;
}

nakama::evidenz::Snapshotkopf testkopf()
{
    nakama::evidenz::Snapshotkopf k;
    k.evidenceId = std::string (32, 'a');
    k.adresse = { "S-1-5-21-1-2-3-1001", std::string (32, '1'), std::string (32, '2'),
                  std::string (32, '3'), std::string (32, '4') };
    k.klasse = "mittel";
    return k;
}

/** Bandwert eines Perzentilsatzes in dB, oder `false`, wenn das Band kein
    Bit traegt. */
bool perzentilDb (const nakama::analyse::EvidenzBaender& satz, int band, double& aus)
{
    if (! nakama::analyse::bitmapLies (satz.bitmap, band))
        return false;
    aus = (double) satz.werte[(std::size_t) band] / 100.0;   // q_db_0p01_i16
    return true;
}

/** Das Band mit dem hoechsten P50 — die Stelle, an der das Testsignal liegt.
    So haengt der Test nicht an einer abgeschriebenen Bandnummer. */
int lautestesBand (const nakama::analyse::EvidenzBaender& p50)
{
    int bestes = -1;
    double best = 0.0;
    for (int b = 0; b < nakama::analyse::Gitter::evidenzBaender; ++b)
    {
        double db = 0.0;
        if (! perzentilDb (p50, b, db))
            continue;
        if (bestes < 0 || db > best) { bestes = b; best = db; }
    }
    return bestes;
}
} // namespace

int main()
{
    std::cout << "SONDE-013 M-05 | Evidenzpfad: Verteilung, Abdeckung, "
                 "Konvergenz und der Ereignisstrom" << std::endl;

    // ── A · Der Snapshot haelt den Vertrag ────────────────────────────────
    abschnitt ("A · Der erzeugte Snapshot haelt den v3-Vertrag");
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        FeatureFrame f {};
        const bool kam = bisEvidenz (s, sinus (0.25, 1000.0, 48000.0), f);
        pruefe (kam, "ein Evidenzframe wird faellig");

        std::string json;
        const bool gebaut = kam && nakama::evidenz::evidenceSnapshotAlsJson (
            f, testkopf(), {}, {}, json);
        pruefe (gebaut, "Snapshot wird gebaut");

        juce::String riegelfehler;
        pruefe (gebaut && nakama::vertrag::textriegel (json, riegelfehler),
                "Snapshot passiert den gemeinsamen Textriegel",
                riegelfehler);

        nakama::vertrag::Schema schema;
        juce::String ladefehler;
        const bool geladen = nakama::vertrag::Schema::laden (
            juce::JSON::parse (finde ("eq-copilot/schemas/v3/eq-ipc-v3.schema.json")),
            schema, ladefehler);
        pruefe (geladen, "v3-Schema laedt", ladefehler);

        const auto daten = juce::JSON::parse (juce::String (json));
        const auto verstoesse = geladen ? schema.pruefe (daten)
                                        : juce::Array<nakama::vertrag::Verletzung> {};
        juce::String erste;
        if (! verstoesse.isEmpty())
            erste = verstoesse[0].instanz + " | " + verstoesse[0].schema
                  + " | " + verstoesse[0].schluessel;
        pruefe (gebaut && geladen && verstoesse.isEmpty(),
                "der erzeugte Snapshot wird von derselben Engine wie B3c angenommen",
                erste);

        // Und die Gegenprobe zur Engine: sie kann ueberhaupt ablehnen.
        auto verdorben = daten;
        if (auto* o = verdorben.getDynamicObject())
            o->setProperty ("abdeckung", 2.0);
        pruefe (geladen && ! schema.pruefe (verdorben).isEmpty(),
                "Gegenprobe: eine Abdeckung ueber 1 faellt an derselben Engine");
    }

    // ── A2 · Der Snapshot MIT Stereoevidenz haelt denselben Vertrag ───────
    //
    // SONDE-013 M-11. `stereo` ist elf Bandsaetze, zwei Metadatenlisten und
    // zwei Skalare — die groesste zusammenhaengende Flaeche, die dieses
    // Ticket auf die Leitung bringt, und `stereo_evidenz` ist
    // `additionalProperties: false`. Ohne diesen Fall waere der ganze
    // Serialisierer ungeprueft: `EqCopSonde013StereoGoldenTest` misst den
    // ERZEUGER (rechnet er richtig?), dieser Fall die WIRE-FORM (darf das so
    // auf die Leitung?). Das sind zwei verschiedene Fragen.
    abschnitt ("A2 · Der Snapshot mit Stereoevidenz haelt denselben Vertrag");
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        e.evidenzIntervallSetzen (1.0);       // genug Welch-Frames fuer Kohaerenz
        FeatureFrame f {};
        const bool kam = bisEvidenz (s, sinus (0.35, 1000.0, 48000.0), f, 900);
        pruefe (kam, "ein Evidenzframe mit langer Kadenz wird faellig");
        pruefe (kam && e.stereoHatInhalt(),
                "und die Engine traegt bandweise Stereoevidenz");

        nakama::evidenz::Stereosicht sicht;
        if (e.stereoHatInhalt())
        {
            sicht.baender = &e.stereoBand (0);
            sicht.skalare = e.stereoSkalare();
        }

        std::string json;
        const bool gebaut = kam && nakama::evidenz::evidenceSnapshotAlsJson (
            f, testkopf(), {}, sicht, json);
        pruefe (gebaut, "Snapshot mit Stereo wird gebaut");

        juce::String riegelfehler;
        pruefe (gebaut && nakama::vertrag::textriegel (json, riegelfehler),
                "er passiert den gemeinsamen Textriegel - kein NaN, kein Inf",
                riegelfehler);

        nakama::vertrag::Schema schema2;
        juce::String ladefehler2;
        const bool geladen2 = nakama::vertrag::Schema::laden (
            juce::JSON::parse (finde ("eq-copilot/schemas/v3/eq-ipc-v3.schema.json")),
            schema2, ladefehler2);
        const auto daten = juce::JSON::parse (juce::String (json));
        const auto verstoesse = geladen2 ? schema2.pruefe (daten)
                                         : juce::Array<nakama::vertrag::Verletzung> {};
        juce::String erste;
        if (! verstoesse.isEmpty())
            erste = verstoesse[0].instanz + " | " + verstoesse[0].schema
                  + " | " + verstoesse[0].schluessel;
        pruefe (gebaut && geladen2 && verstoesse.isEmpty(),
                "und wird von derselben Vertragsengine wie B3c angenommen - alle elf "
                "Bandsaetze, beide Metadatenlisten und die zwei Skalare",
                erste);

        // Die Form im Einzelnen: `saturated` darf NICHT dabei sein. Der
        // Zweig fuer float32 fuehrt die Saettigungsmarke nicht, weil sie zur
        // Quantisierung gehoert und bei float32 nie `true` werden kann - und
        // `additionalProperties: false` macht sie zur Vertragsverletzung.
        const auto stereo = daten.getProperty ("stereo", {});
        pruefe (stereo.isObject(), "das `stereo`-Objekt ist da");
        const auto midDb = stereo.getProperty ("mid_db", {});
        pruefe (midDb.isObject()
                  && ! midDb.getDynamicObject()->hasProperty ("saturated"),
                "und seine Bandsaetze tragen KEIN `saturated` - die Marke gehoert zur "
                "Quantisierung und waere hier ein totes Feld");
        const auto werte = midDb.getProperty ("werte", {}).getArray();
        pruefe (werte != nullptr && werte->size() == nakama::analyse::Gitter::evidenzBaender,
                "jeder Bandsatz traegt genau 221 Werte",
                werte != nullptr ? juce::String (werte->size()) : juce::String ("kein Array"));
        const auto dof = stereo.getProperty ("freiheitsgrade", {}).getArray();
        pruefe (dof != nullptr && dof->size() == nakama::analyse::Gitter::evidenzBaender,
                "und die Freiheitsgrade ebenso - §40.1 verlangt sie je Band");

        // Ohne Stereosicht entsteht das Feld GAR NICHT. Ein Satz aus 221
        // leeren Baendern waere 11 KiB Schweigen auf der Leitung.
        std::string ohne;
        const bool gebautOhne = kam && nakama::evidenz::evidenceSnapshotAlsJson (
            f, testkopf(), {}, {}, ohne);
        const auto datenOhne = juce::JSON::parse (juce::String (ohne));
        pruefe (gebautOhne && ! datenOhne.hasProperty ("stereo"),
                "ohne Stereosicht fehlt das Feld ganz - Abwesenheit ist erlaubt und "
                "billiger als 11 KiB Nullen");
        pruefe (gebautOhne && ohne.size() * 3 < json.size(),
                "und der Snapshot ist dadurch um ein Vielfaches kleiner - die "
                "Rueckstauschwelle misst in EINTRAEGEN, nicht in Bytes",
                juce::String ((int) ohne.size()) + " gegen "
                + juce::String ((int) json.size()) + " Byte");
    }

    // ── B · Die Verteilung traegt den VERLAUF, nicht den Mittelwert ───────
    abschnitt ("B · P10/P50/P95 unterscheiden, was ein Mittelwert gleichmacht");
    {
        // Zwei Signale mit demselben mittleren Pegel: eines konstant, eines
        // zwischen zwei Pegeln springend. Genau hier sagt der Liveframe
        // dasselbe und der Snapshot nicht.
        const double sr = 48000.0;
        FeatureEngine ruhig, springend;
        ruhig.vorbereiten (sr);
        springend.vorbereiten (sr);
        Speiser sr1 { ruhig }, sr2 { springend };

        FeatureFrame fRuhig {}, fSpringend {};
        const bool a = bisEvidenz (sr1, sinus (0.25, 1000.0, sr), fRuhig);
        const bool b = bisEvidenz (sr2, [sr] (std::uint64_t n)
        {
            // 40 ms laut, 40 ms leise — beides innerhalb eines
            // Evidenzfensters von 250 ms.
            const bool laut = ((n / (std::uint64_t) (sr * 0.04)) % 2u) == 0u;
            const double amp = laut ? 0.4 : 0.04;
            return (float) (amp * std::sin (2.0 * 3.14159265358979323846
                                            * 1000.0 * (double) n / sr));
        }, fSpringend);
        pruefe (a && b, "beide Evidenzframes sind da");

        // Das Band kommt aus dem RUHIGEN Signal und gilt fuer beide. Das
        // springende schmiert spektral leicht und haette sonst sein Maximum
        // ein Band daneben — eine Eigenschaft des Signals, nicht der
        // Messung, und kein Grund, zwei verschiedene Baender zu vergleichen.
        const int band = a ? lautestesBand (fRuhig.evidenzP50) : -1;
        double sonde = 0.0;
        pruefe (band >= 0 && b && perzentilDb (fSpringend.evidenzP50, band, sonde),
                "beide Signale tragen im selben Band ein Bit",
                juce::String (band));
        const int bandR = band, bandS = band;

        double r10 = 0.0, r95 = 0.0, s10 = 0.0, s95 = 0.0;
        const bool gelesen = bandR >= 0 && bandS >= 0
            && perzentilDb (fRuhig.evidenzP10, bandR, r10)
            && perzentilDb (fRuhig.evidenzP95, bandR, r95)
            && perzentilDb (fSpringend.evidenzP10, bandS, s10)
            && perzentilDb (fSpringend.evidenzP95, bandS, s95);
        const double spanneRuhig = r95 - r10;
        const double spanneSpringend = s95 - s10;
        pruefe (gelesen && spanneSpringend > spanneRuhig + 6.0,
                "die Spanne P95-P10 trennt das springende vom ruhigen Signal",
                "ruhig " + juce::String (spanneRuhig, 2) + " dB, springend "
                + juce::String (spanneSpringend, 2) + " dB");
        pruefe (gelesen && spanneRuhig < 6.0,
                "das ruhige Signal hat eine schmale Spanne",
                juce::String (spanneRuhig, 2) + " dB");

        // Die Ordnung selbst — sie kann nur halten, wenn wirklich sortiert
        // wurde.
        bool ordnungHaelt = a;
        int mitBit = 0;
        for (int i = 0; a && i < nakama::analyse::Gitter::evidenzBaender; ++i)
        {
            double p10 = 0.0, p50 = 0.0, p95 = 0.0;
            if (! perzentilDb (fRuhig.evidenzP10, i, p10)) continue;
            ordnungHaelt = ordnungHaelt
                        && perzentilDb (fRuhig.evidenzP50, i, p50)
                        && perzentilDb (fRuhig.evidenzP95, i, p95)
                        && p10 <= p50 + 1e-6 && p50 <= p95 + 1e-6;
            ++mitBit;
        }
        pruefe (ordnungHaelt && mitBit > 0,
                "in JEDEM Band mit Bit gilt P10 <= P50 <= P95",
                juce::String (mitBit) + " Baender");
        pruefe (a && fRuhig.evidenzFenster > 0,
                "der Frame nennt, ueber wie viele Fenster die Verteilung geht",
                juce::String ((int) fRuhig.evidenzFenster));
    }

    // ── C · Abdeckung und Konvergenz ─────────────────────────────────────
    abschnitt ("C · Abdeckung ist gemessenes Signal, Konvergenz ist Ruhe");
    {
        const double sr = 48000.0;
        FeatureEngine voll, halb;
        voll.vorbereiten (sr);
        halb.vorbereiten (sr);
        // 🔑 Beide mit 1 Hz. Das Hauptfenster ist 4096 Samples lang (85 ms);
        // eine Stille, die kuerzer ist, liegt in JEDEM Fenster neben Signal
        // und nimmt das Aktivgate trotzdem. Erst ein Evidenzfenster von 1 s
        // hat Platz fuer Stillephasen, die ein ganzes FFT-Fenster fuellen —
        // gemessen: mit 250-ms-Fenster und 50-ms-Stille bleibt die Abdeckung
        // bei exakt 1,000, und der Test bewiese nichts.
        voll.evidenzIntervallSetzen (FeatureEngine::kEvidenzIntervallMaxS);
        halb.evidenzIntervallSetzen (FeatureEngine::kEvidenzIntervallMaxS);
        Speiser sv { voll }, sh { halb };

        FeatureFrame fVoll {}, fHalb {};
        const bool a = bisEvidenz (sv, sinus (0.25, 1000.0, sr), fVoll, 400);
        // Haelfte der Zeit digitale Stille, in Bloecken von 250 ms: die
        // Fenster mitten darin nehmen das Aktivgate nicht und fehlen der
        // Abdeckung.
        const bool b = bisEvidenz (sh, [sr] (std::uint64_t n)
        {
            const bool an = ((n / (std::uint64_t) (sr * 0.25)) % 2u) == 0u;
            return an ? (float) (0.25 * std::sin (2.0 * 3.14159265358979323846
                                                  * 1000.0 * (double) n / sr))
                      : 0.0f;
        }, fHalb, 400);
        pruefe (a && b, "beide Evidenzframes sind da");
        pruefe (a && fVoll.abdeckungGesetzt && fVoll.abdeckung > 0.95f,
                "Dauerton deckt das Evidenzfenster fast vollstaendig ab",
                juce::String (fVoll.abdeckung, 3));
        pruefe (b && fHalb.abdeckungGesetzt && fHalb.abdeckung < fVoll.abdeckung,
                "halbe Stille senkt die Abdeckung messbar",
                juce::String (fHalb.abdeckung, 3));

        pruefe (a && fVoll.konvergenzGesetzt && fVoll.konvergenz > 0.9f,
                "ein stehender Ton konvergiert",
                juce::String (fVoll.konvergenz, 3));

        FeatureEngine wandernd;
        wandernd.vorbereiten (sr);
        Speiser sw { wandernd };
        FeatureFrame fWandernd {};
        // Ein Pegel, der ueber das Fenster laeuft: die zweite Haelfte sagt
        // etwas anderes als die erste.
        const bool c = bisEvidenz (sw, [sr] (std::uint64_t n)
        {
            const double t = (double) n / sr;
            const double amp = 0.02 + 0.4 * (t - std::floor (t));
            return (float) (amp * std::sin (2.0 * 3.14159265358979323846
                                            * 1000.0 * (double) n / sr));
        }, fWandernd);
        pruefe (c && fWandernd.konvergenzGesetzt
                    && fWandernd.konvergenz < fVoll.konvergenz,
                "ein wandernder Pegel konvergiert schlechter als ein stehender",
                juce::String (fWandernd.konvergenz, 3) + " < "
                + juce::String (fVoll.konvergenz, 3));
    }

    // ── D · Der Ereignisstrom ────────────────────────────────────────────
    abschnitt ("D · Ereignisse reisen mit beiden Qualitaetsbits und ihrem Verlust");
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        FeatureFrame f {};
        const bool kam = bisEvidenz (s, sinus (0.25, 1000.0, 48000.0), f);

        // Der Strom wird hier gestellt, nicht aus dem Detektor genommen:
        // dieses Bein misst den TRANSPORT (M-05), nicht die Ausloeser. Dass
        // der Peakpfad ueberhaupt feuert, ist M-86 und gehoert zu B5.
        nakama::analyse::Ereignis eintraege[3] {};
        for (int i = 0; i < 3; ++i)
        {
            eintraege[i].stromSample = f.evidenzStromStart + (std::uint64_t) (i * 480);
            eintraege[i].epoche = f.transport.transport_epoch;
            eintraege[i].segment = f.transport.continuity_segment;
            eintraege[i].staerke = 2.5f + (float) i;
            eintraege[i].bandZentrumHz = 1000.0f * (float) (i + 1);
            eintraege[i].dauerMs = 10.0f;
        }
        eintraege[0].qualitaetFluss = true;                 // nur Fluss
        eintraege[1].qualitaetPeak  = true;                 // nur Peak
        eintraege[2].qualitaetFluss = true;
        eintraege[2].qualitaetPeak  = true;                 // beide im selben Rahmen

        nakama::evidenz::Ereignisstrom strom;
        strom.eintraege = eintraege;
        strom.anzahl = 3;
        strom.verloren = 7;

        std::string json;
        const bool gebaut = kam
            && nakama::evidenz::evidenceSnapshotAlsJson (f, testkopf(), strom, {}, json);
        const auto daten = juce::JSON::parse (juce::String (json));
        const auto* liste = daten.getProperty ("ereignisse", {})
                                 .getProperty ("liste", {}).getArray();
        pruefe (gebaut && liste != nullptr && liste->size() == 3,
                "events_travel_with_quality_and_loss_counter/alle_drei_reisen",
                liste != nullptr ? juce::String (liste->size()) : "keine Liste");

        bool bitsStimmen = liste != nullptr && liste->size() == 3;
        if (bitsStimmen)
        {
            const bool f0 = (*liste)[0].getProperty ("qualitaet_fluss", {});
            const bool p0 = (*liste)[0].getProperty ("qualitaet_peak", {});
            const bool f1 = (*liste)[1].getProperty ("qualitaet_fluss", {});
            const bool p1 = (*liste)[1].getProperty ("qualitaet_peak", {});
            const bool f2 = (*liste)[2].getProperty ("qualitaet_fluss", {});
            const bool p2 = (*liste)[2].getProperty ("qualitaet_peak", {});
            bitsStimmen = f0 && ! p0 && ! f1 && p1 && f2 && p2;
        }
        pruefe (bitsStimmen,
                "events_travel_with_quality_and_loss_counter/beide_bits_getrennt",
                "Fluss-only, Peak-only und beide bleiben unterscheidbar");

        const auto verloren = daten.getProperty ("ereignisse", {})
                                   .getProperty ("verloren", {});
        pruefe (gebaut && (int) verloren == 7,
                "events_travel_with_quality_and_loss_counter/verlustzaehler",
                juce::String ((int) verloren));

        // Ein Ereignis aus einer fremden Epoche haette einen Versatz, der auf
        // nichts zeigt. Es reist nicht — und verschwindet nicht still.
        auto fremd = eintraege[0];
        fremd.epoche = f.transport.transport_epoch + 1;
        nakama::evidenz::Ereignisstrom mitFremd;
        nakama::analyse::Ereignis zwei[2] { eintraege[0], fremd };
        mitFremd.eintraege = zwei;
        mitFremd.anzahl = 2;
        mitFremd.verloren = 0;
        std::string json2;
        const bool gebaut2 = kam
            && nakama::evidenz::evidenceSnapshotAlsJson (f, testkopf(), mitFremd, {}, json2);
        const auto daten2 = juce::JSON::parse (juce::String (json2));
        const auto* liste2 = daten2.getProperty ("ereignisse", {})
                                   .getProperty ("liste", {}).getArray();
        const auto verloren2 = daten2.getProperty ("ereignisse", {})
                                     .getProperty ("verloren", {});
        pruefe (gebaut2 && liste2 != nullptr && liste2->size() == 1
                    && (int) verloren2 == 1,
                "fremde_epoche_reist_nicht_und_wird_gezaehlt",
                liste2 != nullptr ? juce::String (liste2->size()) + " getragen, "
                                      + juce::String ((int) verloren2) + " verloren"
                                  : "keine Liste");

        // Ein Ereignis AUSSERHALB der Vertragsgrenzen faellt einzeln heraus,
        // statt den ganzen Snapshot schemawidrig zu machen. Ohne diesen Fall
        // koennte ein einziger Ausreisser alle Ereignisse eines Fensters
        // mitnehmen - der Empfaenger verwirft dann die ganze Nachricht.
        nakama::analyse::Ereignis ausreisser[2] { eintraege[0], eintraege[0] };
        ausreisser[1].staerke = 5000.0f;            // Vertragsgrenze ist 1000
        nakama::evidenz::Ereignisstrom mitAusreisser { ausreisser, 2, 0 };
        std::string jsonAus;
        const bool gebautAus = kam && nakama::evidenz::evidenceSnapshotAlsJson (
            f, testkopf(), mitAusreisser, {}, jsonAus);
        const auto datenAus = juce::JSON::parse (juce::String (jsonAus));
        const auto* listeAus = datenAus.getProperty ("ereignisse", {})
                                       .getProperty ("liste", {}).getArray();
        bool ausGueltig = false;
        if (gebautAus)
        {
            nakama::vertrag::Schema s2;
            juce::String f2;
            if (nakama::vertrag::Schema::laden (
                    juce::JSON::parse (finde ("eq-copilot/schemas/v3/eq-ipc-v3.schema.json")),
                    s2, f2))
                ausGueltig = s2.pruefe (datenAus).isEmpty();
        }
        pruefe (gebautAus && listeAus != nullptr && listeAus->size() == 1
                    && (int) datenAus.getProperty ("ereignisse", {})
                                     .getProperty ("verloren", {}) == 1
                    && ausGueltig,
                "ein Ereignis ausserhalb der Vertragsgrenzen faellt einzeln heraus",
                listeAus != nullptr ? juce::String (listeAus->size()) + " getragen, Snapshot gueltig: "
                                        + (ausGueltig ? "ja" : "nein")
                                    : "keine Liste");

        // Ein leerer Strom OHNE Verlust laesst das Feld ganz weg.
        std::string json3;
        const bool gebaut3 = kam
            && nakama::evidenz::evidenceSnapshotAlsJson (f, testkopf(), {}, {}, json3);
        pruefe (gebaut3
                    && ! juce::JSON::parse (juce::String (json3))
                            .hasProperty ("ereignisse"),
                "leerer Strom ohne Verlust laesst `ereignisse` weg");
    }

    // ── E · Fail-closed ──────────────────────────────────────────────────
    abschnitt ("E · Kein Snapshot ist besser als ein widerspruechlicher");
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        FeatureFrame f {};
        const bool kam = bisEvidenz (s, sinus (0.25, 1000.0, 48000.0), f);
        std::string json;

        auto ohneEvidenz = f;
        ohneEvidenz.evidenzFrisch = false;
        pruefe (kam && ! nakama::evidenz::evidenceSnapshotAlsJson (
                    ohneEvidenz, testkopf(), {}, {}, json),
                "kein Snapshot ohne faelliges Evidenzfenster");

        auto ohneAbdeckung = f;
        ohneAbdeckung.abdeckungGesetzt = false;
        pruefe (kam && ! nakama::evidenz::evidenceSnapshotAlsJson (
                    ohneAbdeckung, testkopf(), {}, {}, json),
                "kein Snapshot ohne Abdeckungsbit — 0 waere eine andere Aussage");

        auto ohneKonvergenz = f;
        ohneKonvergenz.konvergenzGesetzt = false;
        pruefe (kam && ! nakama::evidenz::evidenceSnapshotAlsJson (
                    ohneKonvergenz, testkopf(), {}, {}, json),
                "kein Snapshot ohne Konvergenzbit");

        auto kopfOhneId = testkopf();
        kopfOhneId.evidenceId = "keine-hex32";
        pruefe (kam && ! nakama::evidenz::evidenceSnapshotAlsJson (
                    f, kopfOhneId, {}, {}, json),
                "kein Snapshot ohne gueltige evidence_id");

        auto kopfFremdeKlasse = testkopf();
        kopfFremdeKlasse.klasse = "ziemlich_gut";
        pruefe (kam && ! nakama::evidenz::evidenceSnapshotAlsJson (
                    f, kopfFremdeKlasse, {}, {}, json),
                "kein Snapshot mit einer erfundenen Konfidenzklasse");

        auto kaputteZeit = f;
        kaputteZeit.transport.sample_rate = 0.0;
        pruefe (kam && ! nakama::evidenz::evidenceSnapshotAlsJson (
                    kaputteZeit, testkopf(), {}, {}, json),
                "kein Snapshot mit unmoeglicher Samplerate");

        // Ein Ereignis ohne Anker haengt an keiner Zeit — es reist nicht.
        auto ohneAnker = f;
        ohneAnker.evidenzStromStartGesetzt = false;
        nakama::analyse::Ereignis eins {};
        eins.epoche = f.transport.transport_epoch;
        eins.segment = f.transport.continuity_segment;
        eins.staerke = 1.0f;
        eins.bandZentrumHz = 1000.0f;
        nakama::evidenz::Ereignisstrom strom { &eins, 1, 0 };
        std::string jsonOhneAnker;
        const bool gebaut = kam && nakama::evidenz::evidenceSnapshotAlsJson (
            ohneAnker, testkopf(), strom, {}, jsonOhneAnker);
        const auto daten = juce::JSON::parse (juce::String (jsonOhneAnker));
        const auto* liste = daten.getProperty ("ereignisse", {})
                                 .getProperty ("liste", {}).getArray();
        pruefe (gebaut && liste != nullptr && liste->isEmpty()
                    && (int) daten.getProperty ("ereignisse", {})
                                  .getProperty ("verloren", {}) == 1,
                "ohne Anker reist kein Ereignis und der Verlust wird gemeldet");
    }

    // ── F · Kadenz ───────────────────────────────────────────────────────
    abschnitt ("F · Die Evidenzkadenz bleibt zwischen 1 und 4 Hz");
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        pruefe (e.evidenzIntervallJetzt() == FeatureEngine::kEvidenzIntervallMinS,
                "frisch vorbereitet laeuft die Evidenz mit 4 Hz");

        e.evidenzIntervallSetzen (100.0);
        pruefe (e.evidenzIntervallJetzt() == FeatureEngine::kEvidenzIntervallMaxS,
                "ein zu grosser Wunsch wird auf 1 Hz geklemmt, nicht abgelehnt");

        e.evidenzIntervallSetzen (0.0);
        pruefe (e.evidenzIntervallJetzt() == FeatureEngine::kEvidenzIntervallMinS,
                "ein zu kleiner Wunsch wird auf 4 Hz geklemmt");

        e.evidenzIntervallSetzen (0.5);
        const auto vorher = e.evidenzIntervallJetzt();
        e.evidenzIntervallSetzen (std::numeric_limits<double>::quiet_NaN());
        pruefe (e.evidenzIntervallJetzt() == vorher,
                "ein NaN verstellt die Kadenz NICHT");

        // Und die Wirkung: bei 1 Hz kommt der Snapshot spaeter, aber er kommt.
        FeatureEngine langsam;
        langsam.vorbereiten (48000.0);
        langsam.evidenzIntervallSetzen (FeatureEngine::kEvidenzIntervallMaxS);
        Speiser sl { langsam };
        FeatureFrame fl {};
        const bool kam = bisEvidenz (sl, sinus (0.25, 1000.0, 48000.0), fl, 400);
        pruefe (kam && fl.evidenzFrisch,
                "auch mit 1 Hz entsteht ein vollstaendiger Snapshot");

        FeatureEngine schnell;
        schnell.vorbereiten (48000.0);
        Speiser ss { schnell };
        FeatureFrame fs {};
        int bloeckeSchnell = 0;
        for (int i = 0; i < 400; ++i)
        {
            ++bloeckeSchnell;
            if (ss.sende (sinus (0.25, 1000.0, 48000.0))
                && schnell.frame().evidenzFrisch)
            { fs = schnell.frame(); break; }
        }
        int bloeckeLangsam = 0;
        {
            FeatureEngine l2;
            l2.vorbereiten (48000.0);
            l2.evidenzIntervallSetzen (FeatureEngine::kEvidenzIntervallMaxS);
            Speiser s2 { l2 };
            for (int i = 0; i < 400; ++i)
            {
                ++bloeckeLangsam;
                if (s2.sende (sinus (0.25, 1000.0, 48000.0))
                    && l2.frame().evidenzFrisch)
                    break;
            }
        }
        pruefe (bloeckeLangsam > bloeckeSchnell * 2,
                "die reduzierte Kadenz braucht wirklich laenger, statt nur anders zu heissen",
                juce::String (bloeckeSchnell) + " gegen " + juce::String (bloeckeLangsam)
                + " Bloecke");
    }

    // ── G · Die Grenze trennt auch diese drei Fenster ─────────────────────
    abschnitt ("G · Eine Grenze trennt Verteilung, Abdeckung und Konvergenz");
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        FeatureFrame f {};
        pruefe (bisEvidenz (s, sinus (0.25, 1000.0, 48000.0), f),
                "ein Evidenzfenster ist gelaufen");
        // 🔑 UNMITTELBAR nach dem Snapshot sind die Traeger LEER — das ist
        // der Vertrag, nicht ein Versaeumnis: `evidenzLeeren()` gehoert zum
        // Veroeffentlichen. Der Riegel darunter braucht aber einen
        // NICHTLEEREN Ausgangszustand, sonst pruefte er nichts. Also erst
        // ein Stueck des NAECHSTEN Fensters fuellen.
        pruefe (e.evidenzVerteilungPlaetze() == 0 && e.evidenzFensterGesamtJetzt() == 0,
                "der Snapshot hat sein Fenster mitgenommen");
        for (int i = 0; i < 8; ++i)
            s.sende (sinus (0.25, 1000.0, 48000.0));
        pruefe (e.evidenzVerteilungPlaetze() > 0 && e.evidenzFensterGesamtJetzt() > 0,
                "das naechste Fenster fuellt die neuen Traeger",
                juce::String ((int) e.evidenzVerteilungPlaetze()) + " Plaetze, "
                + juce::String ((int) e.evidenzFensterGesamtJetzt()) + " Fenster");

        // Ein Seek: derselbe Weg, den B5 fuer alle anderen Fenster misst.
        auto b = s.bauen();
        b.projectSampleStart = 0;                       // Ruecksprung
        s.audio.assign ((std::size_t) b.sampleCount * 2u, 0.0f);
        e.nimmBlock (b, s.audio.data());

        pruefe (e.evidenzVerteilungPlaetze() == 0,
                "der Verteilungsring ist leer",
                juce::String ((int) e.evidenzVerteilungPlaetze()));
        pruefe (e.evidenzFensterGesamtJetzt() == 0 && e.evidenzFensterAktivJetzt() == 0,
                "beide Fensterzaehler der Abdeckung sind leer");
    }

    // === Nacharbeit 1 nach der Erstpruefung 1 (2026-09-04) ==============
    //
    // Fuenf Befunde, die alle am selben Punkt haengen: der Erzeuger sagte
    // nicht, was er wirklich gemessen hat.

    abschnitt ("N1 - B05: die Evidenz reist unter der NAK-40-WIREadresse");
    {
        // Der Bootstrap bildet den Wirealias seit NAK-40 an EINER Stelle. Jeder
        // Sender, der `v3Hello()` direkt benutzte, schickte statt dessen die
        // PERSISTENTE Instance-ID. Fuer eine hex32-ID faellt das nie auf; fuer
        // eine unterstuetzte Legacy-ID wies `adresseGueltig` den Snapshot ab,
        // und die Evidenz dieser Sonde verschwand kommentarlos.
        FeatureEngine engine;
        engine.vorbereiten (48000.0);
        Speiser s (engine);
        FeatureFrame f;
        if (! bisEvidenz (s, sinus (0.25, 1000.0, 48000.0), f))
        {
            pruefe (false, "B05: ein Evidenzframe entsteht", {});
        }
        else
        {
            auto kopf = testkopf();
            kopf.adresse.instanceId = "legacy-instance-id-2019";   // NICHT hex32
            pruefe (! nakama::ipc::istHex32 (kopf.adresse.instanceId),
                    "B05: die Ausgangs-ID ist bewusst KEIN hex32");

            nakama::evidenz::Ereignisstrom leer;
            nakama::evidenz::Stereosicht ohneStereo;
            std::string json;
            const bool ok = nakama::evidenz::evidenceSnapshotAlsJson (
                                f, kopf, leer, ohneStereo, json);
            pruefe (ok,
                    "B05: evidenz_reist_unter_der_wireadresse - eine Legacy-ID "
                    "verhindert den Snapshot NICHT mehr");
            const auto alias = nakama::ipc::instanceAdresseAusState (
                                   kopf.adresse.instanceId);
            pruefe (ok && json.find (alias) != std::string::npos,
                    "B05: und auf der Leitung steht der ALIAS, nicht die "
                    "persistente ID", juce::String (alias));
            pruefe (ok && json.find (kopf.adresse.instanceId) == std::string::npos,
                    "B05: die persistente ID fliesst NICHT auf die Leitung");

            // Idempotenz: eine bereits aliasierte Adresse geht bytegleich durch.
            auto zwei = kopf.adresse;
            zwei.instanceId = alias;
            pruefe (nakama::ipc::wireAdresseAusState (zwei).instanceId == alias,
                    "B05: die Hilfsfunktion ist idempotent - es gibt genau EINEN "
                    "Weg zur Wireadresse");
        }
    }

    abschnitt ("N1 - B06: nur eine erfolgreiche Uebergabe entnimmt den Ring");
    {
        // M-05: bei Ueberlast sinkt die Kadenz, der Ring wird NIE stillschweigend
        // geleert. Der Sender leerte ihn trotzdem - bei Rueckstau UND nach einem
        // abgewiesenen Snapshot -, und weil er dabei auch die Verlustbasis
        // vorsetzte, meldete der naechste Snapshot nicht einmal einen Verlust.
        using nakama::ipc::P1Ergebnis;
        pruefe (nakama::ipc::p1Uebergeben (P1Ergebnis::eingereiht),
                "B06: `eingereiht` gilt als uebergeben");
        pruefe (nakama::ipc::p1Uebergeben (P1Ergebnis::koalesziert),
                "B06: `koalesziert` ebenso - der Platz in der Reihenfolge bleibt");
        pruefe (nakama::ipc::p1Uebergeben (P1Ergebnis::zurWiederholung),
                "B06: `zurWiederholung` ebenso - der Wiederholpuffer wirft nichts "
                "weg (SONDE-010)");
        pruefe (! nakama::ipc::p1Uebergeben (P1Ergebnis::abgewiesen),
                "B06: ring_bleibt_bei_rueckstau - `abgewiesen` ist ein echter "
                "Verlust, die Quelle bleibt stehen");
        pruefe (! nakama::ipc::p1Uebergeben (P1Ergebnis::zuGross),
                "B06: `zuGross` ebenso - an der Tuer abgewiesen ist nicht "
                "uebergeben");

        // Und die Gegenprobe am Ring selbst: er verliert nichts, solange
        // `ereignisseEntnommen()` nicht gerufen wird.
        FeatureEngine engine;
        engine.vorbereiten (48000.0);
        Speiser s (engine);
        FeatureFrame f;
        int mitEreignissen = 0;
        for (int i = 0; i < 200; ++i)
        {
            const bool laut = (i / 10) % 2 == 0;
            s.sende (sinus (laut ? 0.5 : 0.001, 1000.0, 48000.0));
            if (engine.ereignisAnzahlJetzt() > 0)
                ++mitEreignissen;
        }
        const int vorher = engine.ereignisAnzahlJetzt();
        pruefe (vorher > 0, "B06: der Ring traegt Ereignisse",
                juce::String (vorher));
        pruefe (engine.ereignisAnzahlJetzt() == vorher,
                "B06: und behaelt sie, solange niemand sie ENTNIMMT - genau das "
                "tat der Sender bei Rueckstau trotzdem");
        engine.ereignisseEntnommen();
        pruefe (engine.ereignisAnzahlJetzt() == 0,
                "B06: erst `ereignisseEntnommen()` leert ihn - die Gegenprobe, "
                "ohne die der Fall darueber nichts sagt");
        juce::ignoreUnused (mitEreignissen);
    }

    abschnitt ("N1 - B07: nicht-endliche Samples werden gezaehlt und verriegeln");
    {
        // CLAUDE.md: "Nicht-endliche Werte werden verriegelt und gezaehlt".
        // Die Engine ersetzte sie nur durch Stille - der Rahmen sah danach aus
        // wie eine saubere Messung.
        FeatureEngine engine;
        engine.vorbereiten (48000.0);
        Speiser s (engine);
        FeatureFrame sauber;
        pruefe (bisEvidenz (s, sinus (0.25, 1000.0, 48000.0), sauber),
                "B07: ein sauberer Evidenzframe entsteht");
        pruefe (sauber.nichtEndlichRahmen == 0 && sauber.nichtEndlichEvidenz == 0,
                "B07: und zaehlt NULL nicht-endliche Samples - 0 heisst "
                "nachweislich keines");
        pruefe (sauber.peakGesetzt,
                "B07: seine Rahmenskalare sind gesetzt (die Gegenprobe)");

        FeatureEngine kaputt;
        kaputt.vorbereiten (48000.0);
        Speiser s2 (kaputt);
        auto mitNaN = [] (std::uint64_t n) -> float
        {
            if ((n % 4096u) == 100u) return std::numeric_limits<float>::quiet_NaN();
            if ((n % 4096u) == 200u) return std::numeric_limits<float>::infinity();
            return (float) (0.25 * std::sin (2.0 * 3.14159265358979323846
                                             * 1000.0 * (double) n / 48000.0));
        };
        FeatureFrame f;
        pruefe (bisEvidenz (s2, mitNaN, f), "B07: auch mit NaN/Inf entsteht ein Frame");
        pruefe (kaputt.nichtEndlicheSamples() > 0,
                "B07: nicht_endliche_samples_werden_gezaehlt",
                juce::String ((int) kaputt.nichtEndlicheSamples()));
        pruefe (f.nichtEndlichEvidenz > 0,
                "B07: und die Zahl reist im Frame mit",
                juce::String ((int) f.nichtEndlichEvidenz));

        // Die VERRIEGELUNG: die Konfidenzklasse faellt auf `unbrauchbar`.
        nakama::analyse::Konfidenzlage lage;
        lage.coverageBekannt = true;
        lage.abdeckungGesetzt = true;
        lage.abdeckung = 1.0f;
        lage.verteilungFenster = 64;
        lage.sampleFehlerBekannt = true;
        lage.sampleFehler = 0;
        pruefe (nakama::analyse::gesamtklasse (lage) != nakama::analyse::Konfidenzklasse::unbrauchbar,
                "B07: ohne Samplefehler ist die Klasse brauchbar (die Gegenprobe)");
        lage.sampleFehler = 1;
        pruefe (nakama::analyse::gesamtklasse (lage) == nakama::analyse::Konfidenzklasse::unbrauchbar,
                "B07: EIN nicht-endliches Sample macht den Beleg `unbrauchbar` - "
                "nicht `schwach`: die Zahl beschreibt Stille, nicht Musik");

        // Und der Zaehler steht im Snapshot.
        auto kopf = testkopf();
        kopf.klasse = "unbrauchbar";
        nakama::evidenz::Ereignisstrom leer;
        nakama::evidenz::Stereosicht ohneStereo;
        std::string json;
        if (nakama::evidenz::evidenceSnapshotAlsJson (f, kopf, leer, ohneStereo, json))
            pruefe (json.find ("\"samples_nicht_endlich\":") != std::string::npos,
                    "B07: und `samples_nicht_endlich` steht im Snapshot");
        else
            pruefe (false, "B07: der Snapshot entsteht", {});
    }

    abschnitt ("N1 - B08/B09: die Passagenmetriken haengen am Passagenfenster");
    {
        // M-03/M-25: `passageTruePeak`, Headroom, LRA und Fingerprint liefen
        // seit der letzten TRANSPORTgrenze. Eine leise Passage, die nach einem
        // lauten Abschnitt ohne Seek markiert wird, uebernahm dessen Spitze.
        FeatureEngine engine;
        engine.vorbereiten (48000.0);
        Speiser s (engine);

        // Erst laut - der Abschnitt, der NICHT in die Passage gehoert.
        for (int i = 0; i < 60; ++i)
            s.sende (sinus (0.9, 1000.0, 48000.0));
        FeatureFrame lautF;
        bisEvidenz (s, sinus (0.9, 1000.0, 48000.0), lautF);
        pruefe (lautF.truePeakPassageGesetzt && lautF.truePeakPassageDb > -3.0f,
                "B08: der laute Abschnitt setzt ein hohes Passagenmaximum",
                juce::String (lautF.truePeakPassageDb, 2));

        // Jetzt markiert der User eine LEISE Passage - ohne Seek.
        const std::int64_t start = s.projekt;
        pruefe (engine.setzePassagenfenster (start, start + 48000 * 4),
                "B08: das Passagenfenster wird gesetzt");
        pruefe (engine.passagenfensterIntakt(),
                "B08: und ist intakt");
        FeatureFrame leiseF;
        pruefe (bisEvidenz (s, sinus (0.02, 1000.0, 48000.0), leiseF),
                "B08: ein Frame in der leisen Passage entsteht");
        pruefe (! leiseF.truePeakPassageGesetzt
                  || leiseF.truePeakPassageDb < lautF.truePeakPassageDb - 10.0f,
                "B08: passage_metriken_haengen_am_fenster - die leise Passage erbt "
                "die Spitze des lauten Abschnitts NICHT",
                juce::String (leiseF.truePeakPassageDb, 2) + " gegen "
                + juce::String (lautF.truePeakPassageDb, 2));

        // B09: der Polyphasenfilter laeuft am Fensterende aus.
        pruefe (nakama::analyse::TruePeakDetektor::kTapsJePhase >= 2,
                "B09: der Interpolator hat eine Verzoegerung, die geleert werden "
                "muss", juce::String (nakama::analyse::TruePeakDetektor::kTapsJePhase));
        nakama::analyse::TruePeakDetektor tpd;
        tpd.vorbereiten (48000.0);
        double waehrend = 0.0;
        for (int i = 0; i < 8; ++i)
            waehrend = std::max (waehrend, tpd.tick (i == 4 ? 0.8 : 0.0,
                                                     i == 4 ? 0.8 : 0.0));
        const double rest = tpd.nachlauf();
        pruefe (rest > 0.0,
                "B09: polyphasen_nachlauf_am_fensterende - nach dem letzten Sample "
                "steht noch Energie in der Kette, und `nachlauf()` holt sie",
                juce::String (rest, 4));
        pruefe (std::max (waehrend, rest) >= 0.79,
                "B09: zusammen erreichen sie den wahren Scheitel - ohne den "
                "Nachlauf ginge er an der Grenze verloren",
                juce::String (std::max (waehrend, rest), 4));
    }

    std::cout << "\n-----------------------------------------\n"
              << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
