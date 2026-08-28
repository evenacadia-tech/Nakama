#include "NakamaTelemetrie.h"

#include "generiert/nakama_telemetry_v1_generated.h"

#include <cmath>
#include <set>
#include <string>

namespace nakama::telemetrie
{
namespace fb = evenacadia::nakama::v3;

// ------------------------------------------------------------------ Verstoss

bool Verstoss::operator== (const Verstoss& a) const noexcept
{
    return pfad == a.pfad && regel == a.regel;
}

bool Verstoss::operator< (const Verstoss& a) const noexcept
{
    // Byteweise ueber UTF-8: so ordnet auch Rusts String.
    const std::string ap (pfad.toRawUTF8()), bp (a.pfad.toRawUTF8());
    if (ap != bp) return ap < bp;
    const std::string ar (regel.toRawUTF8()), br (a.regel.toRawUTF8());
    return ar < br;
}

namespace
{

/*  Strikter UTF-8-Pruefer ueber (Zeiger, LAENGE) - mit Codepunktzaehler.

    T2-Runde 3, Blocker: hier stand `juce::String::fromUTF8 (sid->c_str())`.
    Das war gleich DREIFACH falsch:

      1. `c_str()` ignoriert die Laenge. FlatBuffers-Strings tragen eine
         explizite Laenge und duerfen ein NUL enthalten; `flatc` erzeugt
         solche Puffer aus einem u0000-Escape anstandslos. Die Rust-Seite las
         die volle Laenge, C++ brach am NUL ab - zwei Wahrheiten ueber
         dieselbe Zeichenkette, in beide Richtungen falsch.
      2. Die Bytes sind UNVALIDIERT. Rusts FlatBuffers-Verifier prueft
         Stringinhalte auf gueltiges UTF-8, der C++-Verifier NICHT. Der
         Vertrag war darauf gebaut, dass sich dieselbe Bibliothek in beiden
         Sprachen gleich verhaelt - genau die Annahme, die er verbieten soll.
      3. `fromUTF8` auf kaputten Bytes ist nicht nur ein falsches Urteil,
         sondern SPEICHERKORRUPTION: fuer ein nacktes Fortsetzungsbyte 0x80
         liefert `getAndAdvance()` Codepunkt 0, waehrend `isEmpty()` dasselbe
         Byte als "nicht Ende" liest - Laengen- und Kopierdurchlauf laufen
         verschieden weit. Gemessen: STATUS_HEAP_CORRUPTION (0xC0000374) und
         STATUS_STACK_BUFFER_OVERRUN (0xC0000409), aus EINEM geaenderten Byte.
         Dieser Leser laeuft ab SONDE-010 in einem VST3 im Hostprozess.

    Die Semantik ist die von Rusts `str::from_utf8`: Ueberlangkodierungen,
    Surrogate (ED A0..BF) und alles ueber U+10FFFF sind ungueltig; ein NUL ist
    ein voellig normaler Codepunkt.
*/
bool utf8Gueltig (const char* daten, size_t laenge, size_t* codepunkte = nullptr)
{
    const auto* b = reinterpret_cast<const unsigned char*> (daten);
    size_t i = 0, n = 0;

    while (i < laenge)
    {
        const auto c = b[i];
        size_t folge = 0;
        unsigned int cp = 0;

        if (c < 0x80)                { folge = 0; cp = c; }
        else if ((c & 0xE0) == 0xC0) { folge = 1; cp = c & 0x1Fu; }
        else if ((c & 0xF0) == 0xE0) { folge = 2; cp = c & 0x0Fu; }
        else if ((c & 0xF8) == 0xF0) { folge = 3; cp = c & 0x07u; }
        else return false;                        // 0x80..0xBF oder 0xF8..0xFF

        /*  Hier stand zusaetzlich `&& folge > 0`, und der war TOT: die Schleife
            laeuft nur solange `i < laenge`, also ist `i + 0 >= laenge` nie
            wahr - der Zusatz konnte nie ueber ein Urteil entscheiden.
            Nachgemessen mit beiden Fassungen, aus DIESER Quelle geschnitten
            statt abgetippt, erschoepfend ueber alle 4.311.810.305 Bytefolgen
            der Laenge 0..4 (die laengste Folge dieses Lesers ist 4 Byte):
            0 Abweichungen (T2-Runde 4, B-3).

            Ein toter Teilausdruck ist eine Zusicherung, die nie geprueft wird.
            Er kostet nichts - bis jemand ihn beim naechsten Umbau fuer die
            tragende Haelfte haelt und die andere umbaut.  */
        if (i + folge >= laenge)
            return false;                         // abgeschnittene Folge

        for (size_t k = 1; k <= folge; ++k)
        {
            const auto f = b[i + k];
            if ((f & 0xC0) != 0x80)
                return false;
            cp = (cp << 6) | (f & 0x3Fu);
        }

        if (folge == 1 && cp < 0x80)      return false;   // ueberlang
        if (folge == 2 && cp < 0x800)     return false;
        if (folge == 3 && cp < 0x10000)   return false;
        if (cp >= 0xD800 && cp <= 0xDFFF) return false;   // Surrogat
        if (cp > 0x10FFFF)                return false;

        i += folge + 1;
        ++n;
    }

    if (codepunkte != nullptr)
        *codepunkte = n;
    return true;
}

/*  32 Kleinbuchstaben-Hexziffern ueber die ECHTE Laenge.

    Die Rust-Seite prueft `s.len() == 32` (Bytes) plus `bytes().all(...)`.
    Hier stand vorher eine `c_str()`-Schleife, die an einem eingebetteten NUL
    abbrach - "32 Hexziffern, dann Muell" galt damit als gueltig.
*/
bool istHex32 (const flatbuffers::String* s)
{
    if (s == nullptr || s->size() != 32)
        return false;
    const auto* d = s->c_str();
    for (size_t i = 0; i < 32; ++i)
    {
        const auto c = d[i];
        const bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        if (! ok)
            return false;
    }
    return true;
}

void hinzu (juce::Array<Verstoss>& out, const juce::String& pfad, const char* regel)
{
    out.add ({ pfad, juce::String (regel) });
}

/*  Bereichspruefung mit AUS DEM ARRAY DEDUZIERTER Groesse.

    Vorher stand hier ein `size_t anzahl`-Parameter, den jeder Aufruf
    hartcodiert mitgab. Heute stimmten alle vier Zahlen - aber ein spaeter
    ergaenzter Enumwert waere still nicht mehr geprueft worden, waehrend die
    Rust-Seite ueber `ENUM_VALUES` mitwaechst. Genau die Sorte Divergenz, die
    dieser Vertrag verhindern soll. `N` kommt jetzt vom Compiler.
*/
template <typename Enum, size_t N>
bool imBereich (Enum wert, const Enum (&werte)[N])
{
    for (size_t i = 0; i < N; ++i)
        if (werte[i] == wert)
            return true;
    return false;
}

void pruefeAdresse (const fb::Adresse& a, const juce::String& p, juce::Array<Verstoss>& out)
{
    // Die SID wird nie geparst, nur verglichen (§32.1) - geprueft wird
    // deshalb nur, dass sie da ist und nicht ins Uferlose waechst.
    // Laengenbasiert und ueber CODEPUNKTE gezaehlt - genau wie Rusts
    // `sid.chars().count()`. Die UTF-8-Gueltigkeit ist zu diesem Zeitpunkt
    // bereits geprueft (siehe `stringsGueltig` im Wurzelpfad), der Zaehler
    // hier kann also nicht mehr fehlschlagen.
    const auto* sid = a.logon_sid();
    size_t sidLaenge = 0;
    if (sid != nullptr)
        utf8Gueltig (sid->c_str(), sid->size(), &sidLaenge);
    if (sid == nullptr || sidLaenge == 0 || sidLaenge > 184)
        hinzu (out, p + "/logon_sid", "sid_laenge");

    const struct { const char* name; const flatbuffers::String* wert; } tokens[] = {
        { "project_binding_id", a.project_binding_id() },
        { "session_epoch",      a.session_epoch() },
        { "instance_id",        a.instance_id() },
        { "runtime_nonce",      a.runtime_nonce() },
    };
    for (const auto& t : tokens)
        if (! istHex32 (t.wert))
            hinzu (out, p + "/" + t.name, "hex32");
}

void pruefeTransport (const fb::Transportstempel& t, const juce::String& p,
                      juce::Array<Verstoss>& out)
{
    // §32.3: "Die Wrapper-Bridge liefert deshalb process_context_present und
    // unabhaengige Validity-Bits; ohne Bridge gilt Projektzeit als unbewiesen."
    // Das Feld ist ein Optional und kein bool mit Default: sonst waere "der
    // Sender hat es weggelassen" ununterscheidbar von "der Host hat keinen
    // Context angelegt" - und das sind zwei verschiedene Konfidenzaussagen.
    if (! t.process_context_present().has_value())
        hinzu (out, p + "/process_context_present", "context_bit_fehlt");

    if (t.zeitbasis() == fb::Zeitbasis::unbekannt
        || ! imBereich (t.zeitbasis(), fb::EnumValuesZeitbasis()))
        hinzu (out, p + "/zeitbasis", "enum_unbekannt");

    if (t.sample_count() > 1048576u)
        hinzu (out, p + "/sample_count", "sample_count_bereich");

    const auto sr = t.sample_rate();
    if (! std::isfinite (sr))
        hinzu (out, p + "/sample_rate", "nicht_endlich");
    else if (sr <= 0.0 || sr > 768000.0)
        hinzu (out, p + "/sample_rate", "sample_rate_bereich");

    // Ein gesetztes Bit ausserhalb der sieben bekannten. FlatBuffers prueft
    // das nicht; `ANY` ist die von flatc erzeugte Maske aller deklarierten.
    const auto bits = static_cast<uint8_t> (t.gueltigkeit());
    const auto bekannt = static_cast<uint8_t> (fb::Gueltigkeit::ANY);
    if ((bits & static_cast<uint8_t> (~bekannt)) != 0)
        hinzu (out, p + "/gueltigkeit", "validity_unbekanntes_bit");

    if (const auto* s = t.schleife())
    {
        /*  🔑 G1-Befund §4.3, geschlossen am 24.08.2026. `start_ppq` und
            `end_ppq` waren die zwei von vier Fliesskomma-Traegern des
            Vertrags, die in BEIDEN handgeschriebenen Lesern ungeprueft
            blieben — und genau deshalb sah es kein Bein: der Kreuzsprachtest
            vergleicht die beiden Leser miteinander, und sie waren sich einig.
            Er stimmte ueberein, auf der falschen Antwort. Ein Vergleich
            zweier Spiegel findet keine gemeinsame Auslassung. */
        const std::pair<const char*, ::flatbuffers::Optional<double>> ppq[] = {
            { "start_ppq", s->start_ppq() },
            { "end_ppq",   s->end_ppq() }
        };
        for (const auto& [name, wert] : ppq)
            if (wert && ! std::isfinite (*wert))
                hinzu (out, p + "/schleife/" + name, "nicht_endlich");

        /*  Dieselbe Frage wie `grenzen_verdreht` eine Ebene tiefer, nur in der
            Zeitbasis des Hosts. `bounds_valid` ist die Bedingung: ohne es
            behauptet §32.3 ueber die Grenzen nichts, und ein Vergleich waere
            eine erfundene Zusage. */
        if (s->bounds_valid())
        {
            const auto a = s->start_ppq();
            const auto b = s->end_ppq();
            if (a && b && std::isfinite (*a) && std::isfinite (*b) && *b < *a)
                hinzu (out, p + "/schleife", "ppq_verdreht");
        }

        if (const auto* g = s->abgeleitete_grenzen())
        {
            const auto pg = p + "/schleife/abgeleitete_grenzen";
            if (g->herleitung() == fb::Herleitung::unbekannt
                || ! imBereich (g->herleitung(), fb::EnumValuesHerleitung()))
                hinzu (out, pg + "/herleitung", "enum_unbekannt");
            if (g->ende() < g->start())
                hinzu (out, pg, "grenzen_verdreht");
        }
    }
}

void pruefeBaender (const fb::Bandwerte& b, const juce::String& p, juce::Array<Verstoss>& out)
{
    const auto gitter   = b.gitter();
    const auto encoding = b.encoding();

    const bool gitterOk = gitter != fb::Bandgitter::unbekannt
                          && imBereich (gitter, fb::EnumValuesBandgitter());
    const bool encodingOk = encoding != fb::BandEncoding::unbekannt
                            && imBereich (encoding, fb::EnumValuesBandEncoding());
    if (! gitterOk)
        hinzu (out, p + "/gitter", "enum_unbekannt");
    if (! encodingOk)
        hinzu (out, p + "/encoding", "enum_unbekannt");

    const auto* i16er = b.werte_i16();
    const auto* f32er = b.werte_f32();

    // Genau EIN Traeger. Beide gesetzt hiesse zwei Wahrheiten ueber dieselbe
    // Messung; keiner gesetzt hiesse ein Bandsatz ohne Baender.
    if (i16er != nullptr && f32er != nullptr)
        hinzu (out, p, "zwei_traeger");
    if (i16er == nullptr && f32er == nullptr)
        hinzu (out, p, "kein_traeger");

    // Und der EINE muss zur Kodierung passen. §33.1: "Empfaenger raten die
    // Skalierung nie aus dem Nachrichtentyp" - sie steht im Batch, und wenn
    // sie nicht zur Nutzlast passt, ist der Batch falsch, nicht auslegbar.
    if (encodingOk)
    {
        const bool erwartetI16 = encoding == fb::BandEncoding::q_db_0p1_i16
                                 || encoding == fb::BandEncoding::q_db_0p01_i16;
        if (erwartetI16 && i16er == nullptr && f32er != nullptr)
            hinzu (out, p, "encoding_passt_nicht");
        if (! erwartetI16 && f32er == nullptr && i16er != nullptr)
            hinzu (out, p, "encoding_passt_nicht");
    }

    if (i16er == nullptr && f32er == nullptr)
        return;

    const size_t anzahl = i16er != nullptr ? i16er->size() : f32er->size();

    // Die Bandzahl FOLGT aus dem Gitter - die beiden Gitter sind als Zahlen
    // eingefroren, und ein Bandsatz, der sich nicht an sie haelt, misst etwas
    // anderes als er behauptet.
    if (gitterOk)
    {
        const size_t soll = gitter == fb::Bandgitter::nakama_1_24_oct_30_18k_v1
                            ? baenderFein : baenderGrob;
        if (anzahl != soll)
            hinzu (out, p, "bandzahl");
    }

    // Bitmap: ceil(n/8) Bytes, LSB-first. `gueltig_bitmap` ist `required` -
    // die Anwesenheit garantiert der Verifier.
    const auto* bm = b.gueltig_bitmap();
    const size_t soll = (anzahl + 7) / 8;
    if (bm == nullptr || bm->size() != soll)
    {
        hinzu (out, p + "/gueltig_bitmap", "bitmap_laenge");
    }
    else if (anzahl % 8 != 0 && bm->size() > 0)
    {
        // Die Fuellbits des letzten Bytes MUESSEN 0 sein - sonst erzeugen zwei
        // Sender fuer dieselbe Messung zwei verschiedene Puffer.
        const auto genutzt = static_cast<uint8_t> ((1u << (anzahl % 8)) - 1u);
        const auto letztes = bm->Get (static_cast<flatbuffers::uoffset_t> (bm->size() - 1));
        if ((letztes & static_cast<uint8_t> (~genutzt)) != 0)
            hinzu (out, p + "/gueltig_bitmap", "bitmap_fuellbits");
    }

    if (i16er != nullptr && encodingOk)
    {
        const auto min = encoding == fb::BandEncoding::q_db_0p01_i16 ? q0p01Min : q0p1Min;
        const auto max = encoding == fb::BandEncoding::q_db_0p01_i16 ? q0p01Max : q0p1Max;
        for (flatbuffers::uoffset_t i = 0; i < i16er->size(); ++i)
        {
            const auto w = i16er->Get (i);
            if (w < min || w > max)
            {
                // Ein benannter Fall reicht; die Menge bleibt endlich.
                hinzu (out, p + "/werte_i16/" + juce::String ((int) i), "bandwert_bereich");
                break;
            }
        }
    }
    if (f32er != nullptr)
    {
        for (flatbuffers::uoffset_t i = 0; i < f32er->size(); ++i)
        {
            if (! std::isfinite (f32er->Get (i)))
            {
                hinzu (out, p + "/werte_f32/" + juce::String ((int) i), "nicht_endlich");
                break;
            }
        }
    }
}

void pruefeFrame (const fb::Frame& f, const juce::String& p, juce::Array<Verstoss>& out)
{
    if (f.metrics_version() < 1u)
        hinzu (out, p + "/metrics_version", "metrics_version");

    // `transport` und `baender` sind `required` - der Verifier hat ihre
    // Anwesenheit garantiert.
    pruefeTransport (*f.transport(), p + "/transport", out);
    pruefeBaender (*f.baender(), p + "/baender", out);

    // Optionale Kennzahlen: ein nicht messbarer Wert wird WEGGELASSEN, nicht
    // als NaN gesendet (quantisierung-v1.json). Ein NaN auf der Leitung ist
    // deshalb ein Senderfehler und wird abgelehnt, nicht sanitisiert.
    const struct { const char* name; flatbuffers::Optional<float> wert; } werte[] = {
        { "aktivitaet",  f.aktivitaet() },
        { "lufs_s",      f.lufs_s() },
        { "peak_db",     f.peak_db() },
        { "crest_db",    f.crest_db() },
        { "psr_db",      f.psr_db() },
        { "breite",      f.breite() },
        { "korrelation", f.korrelation() },
    };
    for (const auto& w : werte)
        if (w.wert.has_value() && ! std::isfinite (*w.wert))
            hinzu (out, p + "/" + w.name, "nicht_endlich");

    if (const auto k = f.korrelation(); k.has_value() && std::isfinite (*k))
        if (*k < -1.0f || *k > 1.0f)
            hinzu (out, p + "/korrelation", "korrelation_bereich");

    if (const auto b = f.breite(); b.has_value() && std::isfinite (*b) && *b < 0.0f)
        hinzu (out, p + "/breite", "breite_negativ");
}

juce::Array<Verstoss> kanonisch (const juce::Array<Verstoss>& roh)
{
    std::set<Verstoss> menge;
    for (const auto& v : roh)
        menge.insert (v);
    juce::Array<Verstoss> out;
    for (const auto& v : menge)
        out.add (v);
    return out;
}

} // namespace

// ------------------------------------------------------------------ Pruefung

juce::Array<Verstoss> pruefe (const uint8_t* puffer, size_t laenge)
{
    juce::Array<Verstoss> out;

    // 1. Die Dateikennung steht an Offset 4 und ist das Erste, was ein Leser
    //    sehen kann. Ein fremder Puffer faellt hier, VOR jedem Feldzugriff.
    //    BufferHasIdentifier liest ab Offset 4 - ein Puffer, der kuerzer ist,
    //    darf ihm gar nicht erst gereicht werden.
    if (puffer == nullptr || laenge < 8 || ! fb::FeatureBatchBufferHasIdentifier (puffer))
    {
        hinzu (out, "", "dateikennung");
        return kanonisch (out);
    }

    // 2. Der Verifier. Ohne ihn ist jeder Feldzugriff auf einen manipulierten
    //    Puffer undefiniert - das ist der eine Punkt, an dem FlatBuffers
    //    wirklich hart ist, und er wird benutzt.
    flatbuffers::Verifier pruefer (puffer, laenge);
    if (! fb::VerifyFeatureBatchBuffer (pruefer))
    {
        hinzu (out, "", "verifier");
        return kanonisch (out);
    }

    const auto* batch = fb::GetFeatureBatch (puffer);
    const auto* eintraege = batch->eintraege();   // `required`

    /*  Die zweite Haelfte des Verifiers, die der C++-Verifier NICHT hat.

        Rusts FlatBuffers-Verifier prueft Stringinhalte auf gueltiges UTF-8
        und lehnt den ganzen Puffer ab; die C++-Fassung prueft nur Grenzen und
        NUL-Terminierung. Ohne diesen Nachschlag urteilten die beiden Beine
        bei 78 von 591 Byte-Mutanten verschieden - und in neun Faellen stuerzte
        die C++-Seite ab, statt zu urteilen.

        Der Verstoss ist ABSICHTLICH derselbe wie auf der Rust-Seite
        (`{"" , "verifier"}`): dort faellt der Puffer im Verifier, hier im
        Nachschlag - dieselbe Aussage, derselbe Ort in der Verstossmenge.
    */
    for (flatbuffers::uoffset_t i = 0; i < eintraege->size(); ++i)
    {
        const auto* a = eintraege->Get (i)->quelle();
        const flatbuffers::String* strings[] = {
            a->logon_sid(), a->project_binding_id(), a->session_epoch(),
            a->instance_id(), a->runtime_nonce()
        };
        for (const auto* s : strings)
        {
            if (s == nullptr || ! utf8Gueltig (s->c_str(), s->size()))
            {
                juce::Array<Verstoss> nur;
                hinzu (nur, "", "verifier");
                return kanonisch (nur);
            }
        }
    }

    if (eintraege->size() == 0)
        hinzu (out, "/eintraege", "eintraege_leer");
    if (eintraege->size() > maxEintraege)
        hinzu (out, "/eintraege", "eintraege_zu_viele");

    // §33.1: "Ein Batch traegt nie mehrere Frames derselben Quelle." Das kann
    // kein Schema ausdruecken, und es ist genau der Grund, warum das Format
    // keine zweite Wrapper-Ebene braucht.
    std::set<std::string> gesehen;

    for (flatbuffers::uoffset_t i = 0; i < eintraege->size(); ++i)
    {
        const auto* e = eintraege->Get (i);
        const auto p = "/eintraege/" + juce::String ((int) i);

        const auto* a = e->quelle();   // `required`
        pruefeAdresse (*a, p + "/quelle", out);

        const std::string id = a->instance_id() != nullptr ? a->instance_id()->str()
                                                           : std::string();
        if (! gesehen.insert (id).second)
            hinzu (out, p + "/quelle/instance_id", "quelle_doppelt");

        pruefeFrame (*e->frame(), p + "/frame", out);   // `required`
    }

    return kanonisch (out);
}

} // namespace nakama::telemetrie
