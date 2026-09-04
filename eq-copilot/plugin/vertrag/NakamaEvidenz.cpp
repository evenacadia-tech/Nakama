#include "NakamaEvidenz.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace nakama::evidenz
{
namespace
{
using nakama::analyse::Bandsatz;
using nakama::analyse::BandEncoding;
using nakama::analyse::GitterId;
using nakama::analyse::Transportstempel;
using nakama::analyse::Zeitbasis;

constexpr char kAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*  Base64 ohne Zeilenumbrueche, mit Fuellzeichen.

    Warum von Hand und nicht ueber JUCE: der Vertrag prueft die Bitmap gegen
    ein MUSTER, dessen letzte Zeichenklasse die Fuellbits auf null zwingt
    (`^[A-Za-z0-9+/]{37}[AQgw]==$`). Ein Kodierer, der irgendwo anders
    umbricht oder das Padding weglaesst, faellt dort — also steht er hier
    sichtbar und nicht in einer Bibliotheksoption. */
std::string base64 (const std::uint8_t* daten, std::size_t n)
{
    std::string aus;
    aus.reserve (((n + 2) / 3) * 4);
    std::size_t i = 0;
    for (; i + 3 <= n; i += 3)
    {
        const std::uint32_t w = (std::uint32_t) daten[i] << 16
                              | (std::uint32_t) daten[i + 1] << 8
                              | (std::uint32_t) daten[i + 2];
        aus.push_back (kAlphabet[(w >> 18) & 0x3f]);
        aus.push_back (kAlphabet[(w >> 12) & 0x3f]);
        aus.push_back (kAlphabet[(w >> 6) & 0x3f]);
        aus.push_back (kAlphabet[w & 0x3f]);
    }
    if (i + 1 == n)
    {
        const std::uint32_t w = (std::uint32_t) daten[i] << 16;
        aus.push_back (kAlphabet[(w >> 18) & 0x3f]);
        aus.push_back (kAlphabet[(w >> 12) & 0x3f]);
        aus += "==";
    }
    else if (i + 2 == n)
    {
        const std::uint32_t w = (std::uint32_t) daten[i] << 16
                              | (std::uint32_t) daten[i + 1] << 8;
        aus.push_back (kAlphabet[(w >> 18) & 0x3f]);
        aus.push_back (kAlphabet[(w >> 12) & 0x3f]);
        aus.push_back (kAlphabet[(w >> 6) & 0x3f]);
        aus.push_back ('=');
    }
    return aus;
}

std::string boolJson (bool b) { return b ? "true" : "false"; }

/*  Zahl fuer den Draht.

    Derselbe Riegel wie in `ControlClient.cpp`: eine Wandlung nach
    `long long` ist fuer NaN, ±Inf und alles ausserhalb des darstellbaren
    Bereichs undefiniertes Verhalten, und undefiniertes Verhalten passiert
    VOR jeder Pruefung, die danach kaeme. Nichtendliches erreicht diesen
    Bauer gar nicht (der Aufrufer prueft), der Riegel steht trotzdem. */
std::string zahlJson (double w)
{
    if (! std::isfinite (w))
        return "null";
    if (w >= -9.007199254740992e15 && w <= 9.007199254740992e15
        && w == static_cast<double> (static_cast<long long> (w)))
        return std::to_string (static_cast<long long> (w));
    return std::to_string (w);
}

const char* gitterName (GitterId g)
{
    switch (g)
    {
        case GitterId::nakama_1_24_oct_30_18k_v1: return "nakama_1_24_oct_30_18k_v1";
        case GitterId::nakama_log64_v1:           return "nakama_log64_v1";
        case GitterId::unbekannt:                 break;
    }
    return nullptr;
}

const char* encodingName (BandEncoding e)
{
    switch (e)
    {
        case BandEncoding::q_db_0p1_i16:  return "q_db_0p1_i16";
        case BandEncoding::q_db_0p01_i16: return "q_db_0p01_i16";
        case BandEncoding::float32:       return "float32";
        case BandEncoding::unbekannt:     break;
    }
    return nullptr;
}

/*  Ein `bandwerte`-Objekt. `false` bei unbekanntem Gitter oder Encoding —
    ein Bandsatz ohne Kennung waere auf der Leitung nicht zuzuordnen. */
template <int N>
bool bandsatzJson (const Bandsatz<N>& satz, std::string& aus)
{
    const char* gitter = gitterName (satz.gitter);
    const char* enc    = encodingName (satz.encoding);
    if (gitter == nullptr || enc == nullptr)
        return false;

    aus += "{\"gitter_id\":\"";
    aus += gitter;
    aus += "\",\"encoding\":\"";
    aus += enc;
    aus += "\",\"werte\":[";
    for (int i = 0; i < N; ++i)
    {
        if (i > 0) aus += ',';
        aus += std::to_string ((int) satz.werte[i]);
    }
    aus += "],\"gueltig_bitmap\":\"";
    aus += base64 (satz.bitmap, sizeof satz.bitmap);
    aus += "\",\"saturated\":";
    aus += boolJson (satz.saturated);
    aus += '}';
    return true;
}

std::string validityJson (std::uint8_t g)
{
    using namespace nakama::analyse;
    std::string aus = "{\"project_time\":";
    aus += boolJson ((g & kGProjectTime) != 0);
    aus += ",\"play_state\":";      aus += boolJson ((g & kGPlayState) != 0);
    aus += ",\"record_state\":";    aus += boolJson ((g & kGRecordState) != 0);
    aus += ",\"cycle_bounds\":";    aus += boolJson ((g & kGCycleBounds) != 0);
    aus += ",\"continuous_time\":"; aus += boolJson ((g & kGContinuousTime) != 0);
    aus += ",\"input_presentation_latency\":";
    aus += boolJson ((g & kGInputLatency) != 0);
    aus += ",\"output_presentation_latency\":";
    aus += boolJson ((g & kGOutputLatency) != 0);
    aus += '}';
    return aus;
}

/*  Der Transportstempel als JSON.

    Der Diskriminator ist `time_basis`, und die bedingten Feldpflichten sind
    dieselben wie im FlatBuffers-Pfad (NAK-29). Sie werden hier NICHT erneut
    ausgerechnet: `nak29Verstoss` ist die eine Wahrheit darueber, und der
    Aufrufer hat sie bereits befragt. Was hier steht, ist nur die Abbildung
    Feld fuer Feld — jede zweite Kopie der Regel waere eine Stelle, an der
    die zwei Pfade auseinanderlaufen koennen. */
bool transportJson (const Transportstempel& t, std::string& aus)
{
    const char* basis = t.zeitbasis == Zeitbasis::project_samples ? "project_samples"
                      : t.zeitbasis == Zeitbasis::local_monotonic ? "local_monotonic"
                      : nullptr;
    if (basis == nullptr || ! std::isfinite (t.sample_rate)
        || t.sample_rate <= 0.0 || t.sample_rate > 768000.0)
        return false;

    aus += "{\"transport_epoch\":";
    aus += std::to_string (t.transport_epoch);
    aus += ",\"continuity_segment\":";
    aus += std::to_string (t.continuity_segment);
    aus += ",\"sequence\":";
    aus += std::to_string (t.sequence);
    aus += ",\"process_context_present\":";
    aus += boolJson (t.process_context_present_gesetzt && t.process_context_present);
    aus += ",\"time_basis\":\"";
    aus += basis;
    aus += "\",\"project_sample_start\":";
    // `null` heisst hier nicht "fehlt", sondern "diese Zeitbasis kennt keinen
    // Projektstart" — der Zweig `local_monotonic` verlangt genau das.
    aus += t.project_sample_start_gesetzt
         ? std::to_string (t.project_sample_start) : std::string ("null");
    aus += ",\"sample_count\":";
    aus += std::to_string (t.sample_count);
    aus += ",\"sample_rate\":";
    aus += zahlJson (t.sample_rate);
    aus += ",\"playing\":";   aus += boolJson (t.playing);
    aus += ",\"recording\":"; aus += boolJson (t.recording);
    if (t.continuous_time_samples_gesetzt)
    {
        aus += ",\"continuous_time_samples\":";
        aus += std::to_string (t.continuous_time_samples);
    }
    if (t.cycle_start_ppq_gesetzt && t.cycle_end_ppq_gesetzt
        && std::isfinite (t.cycle_start_ppq) && std::isfinite (t.cycle_end_ppq))
    {
        aus += ",\"cycle\":{\"start_ppq\":";
        aus += zahlJson (t.cycle_start_ppq);
        aus += ",\"end_ppq\":";
        aus += zahlJson (t.cycle_end_ppq);
        aus += '}';
    }
    if (t.input_presentation_latency_gesetzt)
    {
        aus += ",\"input_presentation_latency\":";
        aus += std::to_string (t.input_presentation_latency);
    }
    if (t.output_presentation_latency_gesetzt)
    {
        aus += ",\"output_presentation_latency\":";
        aus += std::to_string (t.output_presentation_latency);
    }
    aus += ",\"validity\":";
    aus += validityJson (t.gueltigkeit);
    aus += '}';
    return true;
}

bool klasseGueltig (const std::string& k)
{
    return k == "stark" || k == "mittel" || k == "schwach" || k == "unbrauchbar";
}
} // namespace

bool evidenceSnapshotAlsJson (const nakama::analyse::FeatureFrame& frame,
                              const Snapshotkopf& kopf,
                              const Ereignisstrom& ereignisse,
                              std::string& aus)
{
    // Fail-closed, in dieser Reihenfolge: erst die Identitaet, dann der
    // Zeitvertrag, dann der Inhalt. Ein Snapshot ohne Identitaet ist kein
    // Beleg, und ein Beleg mit widerspruechlicher Zeit ist schlimmer als
    // keiner — er sieht aus wie eine Messung.
    if (! frame.evidenzFrisch)
        return false;
    if (! nakama::ipc::istHex32 (kopf.evidenceId))
        return false;
    if (! nakama::ipc::adresseGueltig (kopf.adresse))
        return false;
    if (! klasseGueltig (kopf.klasse))
        return false;
    if (nakama::analyse::nak29Verstoss (frame.transport) != 0)
        return false;
    if (frame.metricsVersion == 0)
        return false;
    // Abdeckung und Konvergenz sind PFLICHT im Vertrag. Ohne Praesenzbit
    // gibt es sie nicht — dann entsteht kein Snapshot, statt eine 0 zu
    // erfinden, die wie „gemessen, aber leer" aussaehe.
    if (! frame.abdeckungGesetzt || ! frame.konvergenzGesetzt)
        return false;
    if (! std::isfinite (frame.abdeckung) || ! std::isfinite (frame.konvergenz))
        return false;

    std::string text;
    text.reserve (64 * 1024);
    text += "{\"type\":\"evidence_snapshot\",\"evidence_id\":\"";
    text += kopf.evidenceId;
    text += "\",\"adresse\":";
    text += nakama::ipc::adresseAlsJson (kopf.adresse);
    text += ",\"transport\":";
    if (! transportJson (frame.transport, text))
        return false;
    text += ",\"metrics_version\":";
    text += std::to_string (frame.metricsVersion);

    text += ",\"verteilung\":{\"p10\":";
    if (! bandsatzJson (frame.evidenzP10, text)) return false;
    text += ",\"p50\":";
    if (! bandsatzJson (frame.evidenzP50, text)) return false;
    text += ",\"p95\":";
    if (! bandsatzJson (frame.evidenzP95, text)) return false;
    text += '}';

    text += ",\"baender\":";
    if (! bandsatzJson (frame.evidenz, text)) return false;

    // ── Ereignisse (M-05) ────────────────────────────────────────────────
    //
    // Nur Ereignisse DIESER Epoche und DIESES Segments reisen. Der
    // `sample_offset` auf dem Draht ist relativ zum Transportstempel dieses
    // Snapshots; ein Ereignis aus einer anderen Epoche haette dort einen
    // Versatz, der auf nichts zeigt. Was aussortiert wird, verschwindet
    // nicht still, sondern erhoeht `verloren`.
    {
        std::uint64_t verloren = ereignisse.verloren;
        int getragen = 0;
        std::string liste;
        for (int i = 0; i < ereignisse.anzahl && ereignisse.eintraege != nullptr; ++i)
        {
            const auto& e = ereignisse.eintraege[i];
            const bool passt = e.epoche == frame.transport.transport_epoch
                            && e.segment == frame.transport.continuity_segment;
            // Die Grenzen sind die des Vertrags, nicht der Plausibilitaet:
            // `staerke_mad` <= 1000, `band_zentrum_hz` <= 384000. Ein Ereignis
            // darueber wird wie ein nichtendliches behandelt und faellt
            // EINZELN heraus. Wuerde es mitgeschrieben, verletzte der ganze
            // Snapshot das Schema und der Empfaenger verwuerfe ALLE Ereignisse
            // dieses Fensters - ein schlechter Wert darf nicht die guten
            // mitnehmen.
            const bool zahlenOk = std::isfinite (e.staerke)
                               && e.staerke >= 0.0f && e.staerke <= 1000.0f
                               && std::isfinite (e.bandZentrumHz)
                               && e.bandZentrumHz > 0.0f && e.bandZentrumHz <= 384000.0f
                               && std::isfinite (e.dauerMs) && e.dauerMs >= 0.0f;
            if (! passt || ! zahlenOk)
            {
                if (verloren < std::numeric_limits<std::uint64_t>::max())
                    ++verloren;
                continue;
            }
            if (getragen >= nakama::analyse::FeatureEngine::kEreignisPlaetze)
            {
                if (verloren < std::numeric_limits<std::uint64_t>::max())
                    ++verloren;
                continue;
            }
            // Samplezeit RELATIV zum Anfang des EVIDENZFENSTERS. Ohne Anker
            // oder vor dem Anker gibt es keinen ehrlichen Versatz — dann
            // reist das Ereignis nicht, statt an einer erfundenen Null zu
            // haengen.
            if (! frame.evidenzStromStartGesetzt
                || e.stromSample < frame.evidenzStromStart)
            {
                if (verloren < std::numeric_limits<std::uint64_t>::max())
                    ++verloren;
                continue;
            }
            const std::uint64_t versatz = e.stromSample - frame.evidenzStromStart;
            if (getragen > 0) liste += ',';
            liste += "{\"sample_offset\":";
            liste += std::to_string (versatz);
            liste += ",\"staerke_mad\":";
            liste += zahlJson ((double) e.staerke);
            liste += ",\"band_zentrum_hz\":";
            liste += zahlJson ((double) e.bandZentrumHz);
            liste += ",\"dauer_samples\":";
            liste += std::to_string ((std::uint64_t) std::llround (
                (double) e.dauerMs * frame.transport.sample_rate / 1000.0));
            liste += ",\"qualitaet_fluss\":";
            liste += boolJson (e.qualitaetFluss);
            liste += ",\"qualitaet_peak\":";
            liste += boolJson (e.qualitaetPeak);
            liste += '}';
            ++getragen;
        }
        if (getragen > 0 || verloren > 0)
        {
            text += ",\"ereignisse\":{\"liste\":[";
            text += liste;
            text += "],\"verloren\":";
            text += std::to_string (verloren);
            text += '}';
        }
    }

    text += ",\"abdeckung\":";
    text += zahlJson ((double) frame.abdeckung);
    text += ",\"konvergenz\":";
    text += zahlJson ((double) frame.konvergenz);

    // `konfidenz` ist im Vertrag ausdruecklich ADDITIV (§34.3). Deshalb darf
    // `verteilung_fenster` hier stehen, ohne dass es ein Fassungsschritt
    // waere: das Objekt sagt selbst, dass es waechst. Der Wert ist die
    // ehrliche Auskunft, auf wie viele Analysefenster sich der SCHWAECHSTE
    // gezeigte Verteilungspunkt stuetzt — dieselbe Rolle wie
    // `integration_samples` bei den Rahmenskalaren.
    text += ",\"konfidenz\":{\"metrics_version\":";
    text += std::to_string (frame.metricsVersion);
    text += ",\"klasse\":\"";
    text += kopf.klasse;
    text += "\",\"verteilung_fenster\":";
    text += std::to_string (frame.evidenzFenster);
    text += "}}";

    aus = std::move (text);
    return true;
}

} // namespace nakama::evidenz
