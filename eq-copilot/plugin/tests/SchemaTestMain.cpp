/*  EqCopSchemaTest — C++-Bein des v3-Vertrags (SONDE-005a).

    Faehrt denselben Fixture-Korpus wie broker/tests/contract_cross_language.rs
    gegen dieselbe Regelmenge und vergleicht mit demselben MANIFEST.json:
    Urteil UND vollstaendige Verletzungsmenge.

    Der Vergleich laeuft ueber das Manifest, nicht gegen eine Ausgabe der
    Rust-Seite. Stimmen beide Engines mit dem Manifest ueberein, stimmen sie
    transitiv miteinander ueberein - und das Manifest ist von Hand
    geschrieben, also nicht die Ausgabe einer der beiden.

    Zusaetzlich: das Bandgitter, der Quantisierungsvertrag und - weil ein
    gruener Test nichts wert ist, solange nicht gezeigt wurde, dass er
    ueberhaupt fallen KANN - eine Reihe Riegelproben.
*/

#include "../vertrag/NakamaVertrag.h"
#include "../vertrag/NakamaTelemetrie.h"
#include "../core/analysis/FeatureEngine.h"
#include "../core/ipc/TelemetryClient.h"
#include "../vertrag/generiert/nakama_telemetry_v1_generated.h"

#include "../core/ipc/WireZahl.h"
#include "../vertrag/NakamaEvidenz.h"

#include <algorithm>
#include <clocale>
#include <cmath>
#include <limits>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace
{
int bestanden = 0;
int fehler = 0;
void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {});

// ═══════════════════════════════════════════════════════════════════════
// NAK-181 R4 · die Zahl auf dem Draht (G4-Befund V05)
// ═══════════════════════════════════════════════════════════════════════

/// N-18, N-18b — jeder Randwert einzeln am Riegel.
///
/// Die Liste ist die Randwerttabelle aus `docs/beweise/NAK-181.md` §2.0 E4,
/// Zeile fuer Zeile gegen A5 `pruefe_v3_vertrag.py` nachgerechnet. Sie prueft
/// nicht „irgendeine Zahl kommt heraus", sondern GENAU den Text und GENAU die
/// Annahme oder Verweigerung.
void fahreWireZahl()
{
    using nakama::wire::wireZahl;
    using nakama::wire::es6Zahl;

    struct Fall { double wert; const char* text; const char* was; };

    // N-18 — die zehn Werte, die alle vier Riegelregeln bestehen.
    const Fall angenommen[] = {
        { -0.0,                 "0",                    "-0 wird 0" },
        { 0.1,                  "0.1",                  "ein Zehntel" },
        { 1.0 / 3.0,            "0.333333333333333",    "ein Drittel, auf 15 Stellen gedeckelt" },
        { 1e-7,                 "1e-7",                 "kleiner Exponent" },
        { 1.1754943508222875e-38, "1.17549435082229e-38", "kleinster normaler float" },
        { 1.401298464324817e-45,  "1.40129846432482e-45", "kleinster subnormaler float" },
        { 1e-300,               "1e-300",               "weit unten, aber ueber der Grenze" },
        { 1e-307,               "1e-307",               "der kleinste angenommene Betrag" },
        { 9007199254740991.0,   "9007199254740991",     "2^53-1 als EXAKTE Ganzzahl" },
        { 48000.0,              "48000",                "eine Samplerate" },
    };
    for (const auto& f : angenommen)
    {
        std::string text;
        const bool ok = wireZahl (f.wert, text);
        pruefe (ok && text == f.text,
                juce::String ("N-18: ") + f.was,
                ok ? juce::String (text.c_str()) : juce::String ("verweigert"));
    }

    // 🔑 N-18 / R4f — der Rundungszeuge. DREI Pruefungen, sonst belegt er
    // nichts: der Wert liegt ECHT unter der Grenze, sein gedeckelter Text
    // liegt darueber, und die UNGEDECKELTE Form faellt.
    //
    // Warum nicht `9.9999999999999999e-308`: dieses Literal ist als Binary64
    // bitgleich mit `1e-307` (beide `0x0031fa182c40c60d`) — es waere gar kein
    // Wert unter der Grenze (MP4-1).
    {
        const double knapp = std::nextafter (1e-307, 0.0);
        pruefe (knapp < 1e-307,
                "N-18/R4f: die Eingabe liegt ECHT unter 1e-307 (Wertvergleich)");
        std::string text;
        const bool ok = wireZahl (knapp, text);
        pruefe (ok && text == "1e-307",
                "N-18/R4f: ihr gedeckelter Wiretext lautet 1e-307",
                ok ? juce::String (text.c_str()) : juce::String ("verweigert"));
        std::string ungedeckelt;
        pruefe (es6Zahl (knapp, ungedeckelt) && ungedeckelt != "1e-307",
                "N-18/R4f: die UNGEDECKELTE Form ist eine andere und faellt am Riegel",
                juce::String (ungedeckelt.c_str()));
    }

    // N-18b — die dreizehn verweigerten Werte.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const Fall verweigert[] = {
        { nan,                  nullptr, "NaN" },
        { inf,                  nullptr, "+Inf" },
        { -inf,                 nullptr, "-Inf" },
        { 9007199254740992.0,   nullptr, "2^53 — eine Ganzzahl ueber der Grenze" },
        { 1e21,                 nullptr, "1e21 an der ES6-Schreibweisengrenze" },
        { 3.4028234663852886e38, nullptr, "groesster normaler float — Ganzzahl ueber 2^53" },
        { 1e300,                nullptr, "1e300 — Ganzzahl, nicht Betragsgrenze" },
        { -1e300,               nullptr, "-1e300" },
        { 1e308,                nullptr, "1e308 an der Betragsgrenze" },
        { 2e-308,               nullptr, "2e-308 — GROESSER als 1e-308 und trotzdem drausen" },
        { 9.99e-308,            nullptr, "9.99e-308 — dez = -308" },
        { 1e-308,               nullptr, "1e-308" },
        { 5e-324,               nullptr, "4.9e-324, der kleinste subnormale double" },
    };
    for (const auto& f : verweigert)
    {
        std::string text;
        pruefe (! wireZahl (f.wert, text),
                juce::String ("N-18b: verweigert — ") + f.was,
                juce::String (text.c_str()));
    }

    // Der Deckel trifft NIE eine exakte Ganzzahl: gedeckelt waere 2^53-1
    // `9.00719925474099e15` — ein anderer Wert.
    {
        std::string text;
        pruefe (wireZahl (9007199254740991.0, text) && text.find ('e') == std::string::npos,
                "N-18: der Deckel laesst exakte Ganzzahlen unangetastet",
                juce::String (text.c_str()));
    }

    // N-19 — `es6Zahl` bleibt ungedeckelt, sonst aendern sich State-Hashes.
    {
        std::string text;
        pruefe (es6Zahl (1.0 / 3.0, text) && text == "0.3333333333333333",
                "N-19: es6Zahl bleibt ungedeckelt (der state_hash laeuft durch keinen Riegel)",
                juce::String (text.c_str()));
    }
}

// ═══════════════════════════════════════════════════════════════════════
// NAK-181 N-16, N-17, N-18c, N-18d · Locale und die Aufrufer
// ═══════════════════════════════════════════════════════════════════════

// N-18d (b) — der Handschlag. Die Snapshot-Haelfte von N-16/N-17/N-18c/N-18d
// liegt in B16 `EqCopSonde013EventWireTest`: dort entsteht der Frame aus einer
// ECHTEN Engine, und ein handgebauter Frame haette hier nur die Riegel des
// Bauers nachgestellt, nicht seine Ausgabe gemessen.
void fahreLocale()
{
    // N-17 — Heartbeat und Hello unter Komma-Locale.
    {
        const char* vorher = std::setlocale (LC_NUMERIC, nullptr);
        const std::string gesichert = vorher != nullptr ? vorher : "C";
        nakama::ipc::Adresse a;
        a.logonSid = "S-1-5-21-1111111111-2222222222-3333333333-1001";
        a.projectBindingId = "11111111111111111111111111111111";
        a.sessionEpoch     = "22222222222222222222222222222222";
        a.instanceId       = "33333333333333333333333333333333";
        a.runtimeNonce     = "44444444444444444444444444444444";
        nakama::ipc::ControlStatus st;
        const auto unterC = nakama::ipc::heartbeatAlsJson (a, 7, st);

        // 🔑 NAK-181 Nacharbeit 2 (WP1-1/WN-01): der Locale-NAME wird
        // gesichert, nicht der Zeiger.
        //
        // `setlocale` gibt einen Zeiger auf einen INTERNEN Puffer zurueck, den
        // ein spaeterer `setlocale`-Aufruf ueberschreiben darf (C-Standard
        // 7.11.1.1/8). Der Rueckwechsel auf `gesichert` unten liegt genau
        // dazwischen: bis zu dieser Runde stand danach `setlocale (LC_NUMERIC,
        // gesetzt)` da und las den entwerteten Zeiger. Was darunter als
        // "unter Komma-Locale gemessen" ausgewiesen war, hing damit an
        // unbestimmtem Verhalten - im guenstigen Fall lief es zweimal unter C
        // und war still gruen.
        const char* kommaZeiger = std::setlocale (LC_NUMERIC, "de-DE");
        if (kommaZeiger == nullptr)
            kommaZeiger = std::setlocale (LC_NUMERIC, "German_Germany.1252");
        pruefe (kommaZeiger != nullptr, "N-17: eine Komma-Locale ist verfuegbar");
        const std::string komma = kommaZeiger != nullptr ? std::string (kommaZeiger)
                                                         : std::string {};
        // 🔑 NAK-181 Nacharbeit 1 (EP-09/NR-09): der HELLO-Text, mit
        // NICHTGANZZAHLIGER Samplerate.
        //
        // Der Heartbeat allein misst die Zahlform gar nicht: seine Felder sind
        // Ganzzahlen und laufen durch `std::to_string`. Die einzige Stelle im
        // Bootstrap, die `zahl()` benutzt, ist `audio.samplerate` im Hello —
        // und die war bis zu dieser Runde ungemessen, weil der Text inline im
        // Verbindungsaufbau hinter der Pipe stand.
        nakama::ipc::ControlHello hallo;
        hallo.adresse = a;
        hallo.pluginKind = "main";
        hallo.samplerate = 44100.5;          // nichtganzzahlig: hier faellt es auf
        hallo.blockSize = 512;
        hallo.channels = 2;
        pruefe (std::setlocale (LC_NUMERIC, gesichert.c_str()) != nullptr,
                "N-17: der Rueckwechsel auf die Ausgangs-Locale gelingt - ohne "
                "ihn maesse die Zeile darunter nicht die C-Seite",
                juce::String (gesichert.c_str()));
        const auto helloUnterC = nakama::ipc::helloAlsJson (hallo);
        pruefe (helloUnterC.find ("44100.5") != std::string::npos,
                "N-17: der Hello traegt die nichtganzzahlige Samplerate mit PUNKT",
                juce::String (helloUnterC.c_str()));

        if (! komma.empty())
        {
            // Das erneute Setzen wird GEPRUEFT, bevor irgendetwas als
            // "unter Komma" gemessen wird. Scheitert es still, verglichen die
            // vier Zeilen darunter zweimal dieselbe C-Ausgabe miteinander und
            // waeren gruen, ohne die Zusage zu beruehren.
            pruefe (std::setlocale (LC_NUMERIC, komma.c_str()) != nullptr,
                    "N-17: dieselbe Komma-Locale laesst sich nach dem "
                    "Rueckwechsel ERNEUT setzen",
                    juce::String (komma.c_str()));
            const auto unterKomma = nakama::ipc::heartbeatAlsJson (a, 7, st);
            pruefe (unterKomma == unterC,
                    "N-17: der Heartbeat ist unter Komma-Locale BYTEGLEICH",
                    juce::String ((int) unterKomma.size()) + " Bytes");

            const auto helloUnterKomma = nakama::ipc::helloAlsJson (hallo);
            pruefe (helloUnterKomma == helloUnterC,
                    "N-17: und der HELLO-Text ebenso - bytegleich unter beiden "
                    "Locales",
                    juce::String ((int) helloUnterKomma.size()) + " Bytes");
            pruefe (helloUnterKomma.find ("44100,5") == std::string::npos,
                    "N-17: kein Komma-Dezimaltrenner im Hello - hier faellt der "
                    "Rotbeweis, wenn `zahl` durch `std::to_string` ersetzt wird");
            const auto geparst = juce::JSON::parse (juce::String (helloUnterKomma));
            pruefe (! geparst.isVoid()
                    && (double) geparst.getProperty ("audio", {})
                                       .getProperty ("samplerate", {}) == 44100.5,
                    "N-17: juce::JSON liest die Samplerate unveraendert zurueck");
            pruefe (std::setlocale (LC_NUMERIC, gesichert.c_str()) != nullptr,
                    "N-17: und die Ausgangs-Locale steht am Ende wieder - sonst "
                    "liefe der REST dieses Beins unter Komma");
        }
    }

    pruefe (! nakama::ipc::audioGueltig (1e-308, 512, 2),
            "N-18d: audioGueltig weist eine riegelwidrige Samplerate ab — der "
            "Client verbindet gar nicht erst, statt ein null in ein Pflicht-number "
            "zu senden");
    pruefe (! nakama::ipc::audioGueltig (5e-324, 512, 2),
            "N-18d: dasselbe fuer den kleinsten subnormalen double");
    pruefe (nakama::ipc::audioGueltig (48000.0, 512, 2),
            "N-18d: und nimmt die gewoehnliche Samplerate an");
    pruefe (nakama::ipc::audioGueltig (44100.5, 512, 2),
            "N-18d: auch eine nichtganzzahlige, die der Riegel traegt");
}

void fahreBandStereoRoundtrip()
{
    nakama::analyse::FeatureFrame f {};
    f.metricsVersion = 1;
    f.transport.transport_epoch = 1;
    f.transport.continuity_segment = 1;
    f.transport.sequence = 1;
    f.transport.zeitbasis = nakama::analyse::Zeitbasis::project_samples;
    f.transport.project_sample_start_gesetzt = true;
    f.transport.project_sample_start = 0;
    f.transport.sample_count = 512;
    f.transport.sample_rate = 48000.0;
    f.transport.gueltigkeit = nakama::analyse::kGProjectTime;
    f.transport.process_context_present_gesetzt = true;
    f.transport.process_context_present = true;
    f.live.gitter = nakama::analyse::GitterId::nakama_log64_v1;
    f.live.encoding = nakama::analyse::BandEncoding::q_db_0p1_i16;
    std::fill (std::begin (f.live.werte), std::end (f.live.werte), std::int16_t { -120 });
    std::fill (std::begin (f.live.bitmap), std::end (f.live.bitmap), std::uint8_t { 0xff });
    f.liveBreite[0] = 0.25f;
    f.liveBreite[63] = 1.0f;
    f.liveBreiteBitmap[0] = 0x01;
    f.liveBreiteBitmap[7] = 0x80;

    nakama::ipc::Adresse a {
        "S-1-5-21-1", std::string (32, '1'), std::string (32, '2'),
        std::string (32, '3'), std::string (32, '4')
    };
    std::vector<std::uint8_t> puffer;
    const bool gebaut = nakama::ipc::featureFrameAlsFlatbuffer (f, a, puffer);
    const auto verstoesse = gebaut
        ? nakama::telemetrie::pruefe (puffer.data(), puffer.size())
        : juce::Array<nakama::telemetrie::Verstoss> {};
    const auto* batch = gebaut
        ? nakama::v3::GetFeatureBatch (puffer.data()) : nullptr;
    const auto* stereo = batch != nullptr && batch->eintraege()->size() == 1
        ? batch->eintraege()->Get (0)->frame()->band_stereo() : nullptr;
    pruefe (gebaut && verstoesse.isEmpty() && stereo != nullptr
            && stereo->werte_f32() != nullptr && stereo->werte_f32()->size() == 64
            && stereo->werte_i16() == nullptr && stereo->gueltig_bitmap()->size() == 8
            && stereo->werte_f32()->Get (0) == 0.25f
            && stereo->werte_f32()->Get (63) == 1.0f,
            "band_stereo_featureframe_to_flatbuffer");

    auto projektOhneBit = f;
    projektOhneBit.transport.gueltigkeit &= ~nakama::analyse::kGProjectTime;
    pruefe (nakama::analyse::nak29Verstoss (projektOhneBit.transport) == 1
                && ! nakama::ipc::featureFrameAlsFlatbuffer (projektOhneBit, a, puffer),
            "nak29_encoder_project_samples_ohne_project_time_bit");

    auto lokalMitProjektstart = f;
    lokalMitProjektstart.transport.zeitbasis = nakama::analyse::Zeitbasis::local_monotonic;
    lokalMitProjektstart.transport.gueltigkeit &= ~nakama::analyse::kGProjectTime;
    pruefe (nakama::analyse::nak29Verstoss (lokalMitProjektstart.transport) == 2
                && ! nakama::ipc::featureFrameAlsFlatbuffer (lokalMitProjektstart, a, puffer),
            "nak29_encoder_local_monotonic_mit_project_sample_start");

    // ── NAK-68 / SONDE-013: Feld-ID 14 ueber Encoder UND Leser ────────────
    //
    // Der Nutzen des Feldes haengt an EINER Unterscheidung: Abwesenheit ist
    // erlaubt und heisst "der Erzeuger sagt es nicht"; eine gesendete 0 ist
    // ein Senderfehler. Wer beides zusammenzieht, hat NAK-68 nicht geloest,
    // sondern nur ein Feld hinzugefuegt. Deshalb drei Zweige, nicht einer.
    auto mitIntegration = f;
    mitIntegration.integrationGesetzt = true;
    mitIntegration.integrationSamples = 4800;
    const bool gebautMit = nakama::ipc::featureFrameAlsFlatbuffer (mitIntegration, a, puffer);
    std::vector<nakama::telemetrie::Empfangsframe> gelesen;
    juce::Array<nakama::telemetrie::Verstoss> leseVerstoesse;
    const bool gelesenOk = gebautMit
        && nakama::telemetrie::lese (puffer.data(), puffer.size(), gelesen, leseVerstoesse);
    pruefe (gelesenOk && gelesen.size() == 1
                && gelesen[0].integrationGesetzt
                && gelesen[0].integrationSamples == 4800u,
            "integration_samples_wird_von_beiden_lesern_klassifiziert",
            "gesetzt: der Leser gibt Bit UND Wert zurueck");

    gelesen.clear();
    leseVerstoesse.clearQuick();
    const bool gebautOhne = nakama::ipc::featureFrameAlsFlatbuffer (f, a, puffer);
    const bool ohneOk = gebautOhne
        && nakama::telemetrie::lese (puffer.data(), puffer.size(), gelesen, leseVerstoesse);
    pruefe (ohneOk && gelesen.size() == 1 && ! gelesen[0].integrationGesetzt
                && gelesen[0].integrationSamples == 0u,
            "integration_samples_abwesend_bleibt_gueltig_und_ist_keine_null",
            "abwesend: gueltig, aber ohne Bit — der Wert 0 ist hier NICHT die Aussage");

    // Die 0 kommt nicht durch den Encoder (er sendet nur gesetzte Felder),
    // also wird sie am rohen Puffer geprueft: genau so, wie sie von einem
    // fremden Sender kaeme. Ohne diesen Zweig waere die Regel nur behauptet.
    auto nullFrame = f;
    nullFrame.integrationGesetzt = true;
    nullFrame.integrationSamples = 0;
    const bool gebautNull = nakama::ipc::featureFrameAlsFlatbuffer (nullFrame, a, puffer);
    const auto nullVerstoesse = gebautNull
        ? nakama::telemetrie::pruefe (puffer.data(), puffer.size())
        : juce::Array<nakama::telemetrie::Verstoss> {};
    bool nullGemeldet = false;
    for (const auto& v : nullVerstoesse)
        nullGemeldet = nullGemeldet || v.regel == "integration_samples_null";
    pruefe (gebautNull && nullGemeldet,
            "integration_samples_null_ist_ein_senderfehler",
            "gesetzt auf 0: abgelehnt, nicht als 'fehlt' umgedeutet");
}

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz)
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

/** Sucht eine Datei relativ zum Arbeitsverzeichnis und, falls das danebengeht,
    relativ zur laufenden Programmdatei - damit der Test auch dann etwas
    Verstaendliches sagt, wenn ihn jemand aus dem Bauordner heraus startet. */
juce::File finde (const juce::String& relativ)
{
    auto ausCwd = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    if (ausCwd.existsAsFile())
        return ausCwd;

    auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    for (int i = 0; i < 10 && ordner.exists(); ++i)
    {
        auto kandidat = ordner.getChildFile (relativ);
        if (kandidat.existsAsFile())
            return kandidat;
        ordner = ordner.getParentDirectory();
    }
    return ausCwd;
}

juce::var lies (const juce::String& relativ, bool& ok)
{
    const auto datei = finde (relativ);
    if (! datei.existsAsFile())
    {
        std::cout << "[ROT]  Datei fehlt: " << datei.getFullPathName().toRawUTF8() << std::endl;
        ++fehler;
        ok = false;
        return {};
    }
    juce::var wert;
    const auto ergebnis = juce::JSON::parse (datei.loadFileAsString(), wert);
    if (ergebnis.failed())
    {
        std::cout << "[ROT]  JSON kaputt in " << relativ.toRawUTF8()
                  << ": " << ergebnis.getErrorMessage().toRawUTF8() << std::endl;
        ++fehler;
        ok = false;
        return {};
    }
    ok = true;
    return wert;
}

double ausHex64 (const juce::String& h)
{
    const auto bits = static_cast<uint64_t> (h.getHexValue64());
    double d {};
    std::memcpy (&d, &bits, sizeof d);
    return d;
}

uint64_t bitsVon (double d)
{
    uint64_t b {};
    std::memcpy (&b, &d, sizeof b);
    return b;
}

juce::String beschreibe (const nakama::vertrag::Verletzung& v)
{
    return "{" + v.instanz + " | " + v.schema + " | " + v.schluessel + "}";
}

// ------------------------------------------- N-18/N-16: die dritte Instanz

/** Misst `wireZahl` gegen `evidenz-zahlen-wire-v1.json`.

    Eigene Funktion und nicht Teil von `fahreWireZahl`, weil sie `lies` und
    `ausHex64` braucht - beide stehen weiter unten. */
void fahreWireZahlFixture()
{
    // 🔑 N-18 / N-16 (Fixturehaelfte) — die DRITTE Instanz.
    //
    // `evidenz-zahlen-wire-v1.json` erzeugt der Fixture-Erzeuger; weder dieser
    // Leser noch der Rust-Leser noch A5 schreiben sie. Alle drei messen
    // dagegen — laeuft eine Seite weg, faellt genau ein Bein. Die Eingaben
    // stehen als IEEE-754-Bitmuster, damit kein Literal auf dem Weg gerundet
    // wird (MP4-1).
    {
        bool da = false;
        const auto baum = lies ("eq-copilot/fixtures/v3/evidenz-zahlen-wire-v1.json", da);
        pruefe (da, "N-18: die Byteinstanz liegt im Korpus");
        if (! da) return;
        auto pruefeListe = [&] (const char* feld, bool erwarteAngenommen)
        {
            const auto* liste = baum.getProperty (feld, {}).getArray();
            pruefe (liste != nullptr && liste->size() >= 5,
                    juce::String ("N-18: die Liste `") + feld + "` traegt Klassen",
                    juce::String (liste != nullptr ? liste->size() : -1));
            if (liste == nullptr) return;
            for (const auto& e : *liste)
            {
                const auto klasse = e.getProperty ("klasse", {}).toString();
                const double x = ausHex64 (e.getProperty ("eingabe_hex64", {}).toString());
                std::string text;
                const bool ok = nakama::wire::wireZahl (x, text);
                if (erwarteAngenommen)
                    pruefe (ok && juce::String (text) == e.getProperty ("wire", {}).toString(),
                            "N-18: " + klasse + " - wireZahl erzeugt GENAU den Text der Fixture",
                            juce::String (text.c_str()) + " gegen "
                            + e.getProperty ("wire", {}).toString());
                else
                    pruefe (! ok,
                            "N-18: " + klasse + " - wireZahl verweigert, wie die Fixture sagt",
                            juce::String (text.c_str()));
            }
        };
        pruefeListe ("angenommen", true);
        pruefeListe ("verweigert", false);
    }
}

// ------------------------------------------------------------------ Korpus

void fahreKorpus (const nakama::vertrag::Schema& schema)
{
    bool ok = false;
    const auto manifest = lies ("eq-copilot/fixtures/v3/MANIFEST.json", ok);
    if (! ok)
        return;

    auto* liste = manifest.getProperty ("fixtures", {}).getArray();
    if (liste == nullptr)
    {
        pruefe (false, "MANIFEST traegt eine Fixtureliste");
        return;
    }

    int geprueft = 0, abweichungen = 0;
    for (const auto& eintrag : *liste)
    {
        const auto name = eintrag.getProperty ("datei", {}).toString();

        // Skalar-Wurzel: JUCEs JSON-Leser folgt RFC 4627 und verlangt { oder [
        // am Anfang, waehrend serde_json und Python RFC 8259 folgen und jeden
        // Wert als Dokument annehmen. Beide Seiten LEHNEN AB - C++ schon im
        // Parser, Rust erst am Schema. Hier wird deshalb genau das geprueft:
        // dass der Parser es zurueckweist. Das Fixture bleibt im Korpus, damit
        // die Abweichung sichtbar bleibt statt weggeraeumt zu werden.
        if (static_cast<bool> (eintrag.getProperty ("wurzel_skalar", false)))
        {
            juce::var weg;
            const auto ergebnis = juce::JSON::parse (
                finde ("eq-copilot/fixtures/v3/" + name).loadFileAsString(), weg);
            pruefe (ergebnis.failed(),
                    "Skalar-Wurzel wird schon im Parser abgelehnt: " + name);
            ++geprueft;
            continue;
        }

        // Der Textriegel laeuft VOR dem Parser, und zwar ueber JEDES Fixture.
        // Die mit `textriegel_lehnt_ab` markierten muessen an ihm fallen; alle
        // uebrigen muessen ihn passieren. Ohne die zweite Haelfte waere der
        // Riegel eine Behauptung, die nur an elf Dateien geprueft wird.
        // BYTES, nicht Text: loadFileAsString streift ein BOM und ersetzt
        // kaputtes UTF-8 still - genau die zwei Faelle, in denen die drei Beine
        // vorher auseinanderliefen (T2-Runde 2, BF-6/BF-7).
        juce::MemoryBlock rohbytes;
        finde ("eq-copilot/fixtures/v3/" + name).loadFileAsData (rohbytes);
        juce::String riegelfehler;
        const bool sauber = nakama::vertrag::textriegelBytes (rohbytes.getData(),
                                                              rohbytes.getSize(),
                                                              riegelfehler);
        if (static_cast<bool> (eintrag.getProperty ("textriegel_lehnt_ab", false)))
        {
            pruefe (! sauber, "Textriegel lehnt ab: " + name, riegelfehler);
            ++geprueft;
            continue;
        }
        if (! sauber)
        {
            std::cout << "[ROT]  " << name.toRawUTF8()
                      << ": Textriegel lehnt ein Fixture ab, das er passieren lassen muss: "
                      << riegelfehler.toRawUTF8() << std::endl;
            ++abweichungen;
            continue;
        }

        bool gelesen = false;
        const auto daten = lies ("eq-copilot/fixtures/v3/" + name, gelesen);
        if (! gelesen)
            { ++abweichungen; continue; }

        const auto ist = schema.pruefe (daten);
        const bool sollGueltig = eintrag.getProperty ("urteil", {}).toString() == "gueltig";

        if (ist.isEmpty() != sollGueltig)
        {
            juce::String erste;
            if (! ist.isEmpty())
                erste = beschreibe (ist.getReference (0));
            std::cout << "[ROT]  " << name.toRawUTF8() << ": Urteil "
                      << (sollGueltig ? "gueltig" : "ungueltig") << " erwartet, Engine sagt "
                      << (ist.isEmpty() ? "gueltig" : "ungueltig")
                      << " " << erste.toRawUTF8() << std::endl;
            ++abweichungen;
        }
        else
        {
            auto* sollListe = eintrag.getProperty ("verletzungen", {}).getArray();
            juce::Array<nakama::vertrag::Verletzung> soll;
            if (sollListe != nullptr)
                for (const auto& v : *sollListe)
                    soll.add ({ v.getProperty ("instanz", {}).toString(),
                                v.getProperty ("schema", {}).toString(),
                                v.getProperty ("schluessel", {}).toString() });

            bool gleich = soll.size() == ist.size();
            for (int i = 0; gleich && i < soll.size(); ++i)
                gleich = soll.getReference (i) == ist.getReference (i);

            if (! gleich)
            {
                std::cout << "[ROT]  " << name.toRawUTF8() << ": Verletzungsmenge weicht ab"
                          << std::endl;
                for (const auto& v : soll) std::cout << "         soll " << beschreibe (v).toRawUTF8() << std::endl;
                for (const auto& v : ist)  std::cout << "         ist  " << beschreibe (v).toRawUTF8() << std::endl;
                ++abweichungen;
            }
        }
        ++geprueft;
    }

    pruefe (abweichungen == 0,
            "Korpus klassifiziert wie das Manifest (" + juce::String (geprueft) + " Fixtures)",
            abweichungen == 0 ? juce::String() : juce::String (abweichungen) + " Abweichungen");
    pruefe (geprueft >= 100, "Korpus hat Substanz", juce::String (geprueft) + " Fixtures");

    const int gueltig   = static_cast<int> (manifest.getProperty ("anzahl_gueltig", {}));
    const int ungueltig = static_cast<int> (manifest.getProperty ("anzahl_ungueltig", {}));
    pruefe (gueltig + ungueltig == geprueft, "Manifestzahlen passen zur Fixtureliste");
}

// ------------------------------------------------------- Binaerkorpus (S005b)

/*  SONDE-005b: derselbe Vergleich fuer den BINAEREN Teil des Vertrags.

    Die Rust-Gegenseite steht in broker/tests/contract_cross_language.rs. Beide
    messen gegen fixtures/v3/flatbuffers/MANIFEST.json - Urteil UND
    vollstaendige Verstossmenge -, und das Manifest ist von Hand geschrieben,
    also nicht die Ausgabe eines der beiden Leser.
*/
void fahreFbKorpus()
{
    bool ok = false;
    const auto manifest = lies ("eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json", ok);
    if (! ok)
        return;

    auto* liste = manifest.getProperty ("fixtures", {}).getArray();
    if (liste == nullptr)
    {
        pruefe (false, "Binaer-MANIFEST traegt eine Fixtureliste");
        return;
    }

    int geprueft = 0, abweichungen = 0;
    for (const auto& eintrag : *liste)
    {
        const auto name = eintrag.getProperty ("datei", {}).toString();
        const auto datei = finde ("eq-copilot/fixtures/v3/flatbuffers/" + name);
        juce::MemoryBlock roh;
        if (! datei.existsAsFile() || ! datei.loadFileAsData (roh))
        {
            std::cout << "[ROT]  Binaerfixture fehlt: " << name.toRawUTF8() << std::endl;
            ++abweichungen;
            continue;
        }

        const auto ist = nakama::telemetrie::pruefe (
            static_cast<const uint8_t*> (roh.getData()), roh.getSize());
        const bool sollGueltig = eintrag.getProperty ("urteil", {}).toString() == "gueltig";

        if (ist.isEmpty() != sollGueltig)
        {
            std::cout << "[ROT]  " << name.toRawUTF8() << ": Urteil "
                      << (sollGueltig ? "gueltig" : "ungueltig") << " erwartet, Leser sagt "
                      << (ist.isEmpty() ? "gueltig" : "ungueltig");
            if (! ist.isEmpty())
                std::cout << " {" << ist.getReference (0).pfad.toRawUTF8() << " | "
                          << ist.getReference (0).regel.toRawUTF8() << "}";
            std::cout << std::endl;
            ++abweichungen;
        }
        else
        {
            auto* sollListe = eintrag.getProperty ("verstoesse", {}).getArray();
            juce::Array<nakama::telemetrie::Verstoss> soll;
            if (sollListe != nullptr)
                for (const auto& s : *sollListe)
                    soll.add ({ s.getProperty ("pfad", {}).toString(),
                                s.getProperty ("regel", {}).toString() });

            bool gleich = soll.size() == ist.size();
            for (int i = 0; gleich && i < soll.size(); ++i)
                gleich = soll.getReference (i) == ist.getReference (i);

            if (! gleich)
            {
                std::cout << "[ROT]  " << name.toRawUTF8() << ": Verstossmenge weicht ab"
                          << std::endl;
                for (const auto& s : soll)
                    std::cout << "         soll {" << s.pfad.toRawUTF8() << " | "
                              << s.regel.toRawUTF8() << "}" << std::endl;
                for (const auto& s : ist)
                    std::cout << "         ist  {" << s.pfad.toRawUTF8() << " | "
                              << s.regel.toRawUTF8() << "}" << std::endl;
                ++abweichungen;
            }
        }
        ++geprueft;
    }

    pruefe (abweichungen == 0,
            "Binaerkorpus klassifiziert wie das Manifest (" + juce::String (geprueft)
                + " Fixtures)",
            abweichungen == 0 ? juce::String() : juce::String (abweichungen) + " Abweichungen");
    // Substanzriegel: mit geleerter Liste ginge der Test sonst gruen durch.
    pruefe (geprueft >= 30, "Binaerkorpus hat Substanz", juce::String (geprueft) + " Fixtures");

    const int g = static_cast<int> (manifest.getProperty ("anzahl_gueltig", {}));
    const int u = static_cast<int> (manifest.getProperty ("anzahl_ungueltig", {}));
    pruefe (g + u == geprueft, "Binaer-Manifestzahlen passen zur Fixtureliste");

    /*  T2-Runde 3, Blocker BL-1: der Puffer mit einem 0x80 in der SID hat den
        Prozess ab der ZWEITEN Verarbeitung getoetet (STATUS_HEAP_CORRUPTION).
        Ein Korpuslauf sieht jedes Fixture nur einmal und haette das nie
        bemerkt - deshalb wird genau dieser Puffer hier wiederholt gefahren.

        Der Riegel prueft zwei Dinge: dass der Prozess ueberlebt (er laeuft
        einfach weiter) und dass das Urteil ueber 200 Durchgaenge STABIL ist.
        Ein Speicherfehler, der nur manchmal zuschlaegt, waere sonst ein
        gruener Test mit einem Zufallsgenerator darin.
    */
    const auto absturzfixture = finde (
        "eq-copilot/fixtures/v3/flatbuffers/ungueltig/sid-ungueltiges-utf8.bin");
    juce::MemoryBlock kaputt;
    if (absturzfixture.existsAsFile() && absturzfixture.loadFileAsData (kaputt))
    {
        bool stabil = true;
        for (int i = 0; i < 200 && stabil; ++i)
        {
            const auto v = nakama::telemetrie::pruefe (
                static_cast<const uint8_t*> (kaputt.getData()), kaputt.getSize());
            stabil = v.size() == 1 && v.getReference (0).regel == "verifier";
        }
        pruefe (stabil, "kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz",
                "BL-1 aus T2-Runde 3");
    }
    else
    {
        pruefe (false, "Absturz-Regressionsfixture vorhanden");
    }
}

/*  T2-Runde 3, Befund 8: die Bandwertgrenzen der beiden Leser standen nur im
    Quelltext, waehrend README und Beweismanifest `bereich_db` als ihre Quelle
    nannten - ein Feld, das etwas ganz anderes bedeutet (den Traegerumfang
    +/-32767). Der Vertrag traegt die Grenze jetzt als `plausibler_bereich_db`,
    und diese Probe macht aus der Quellenangabe eine PRUEFUNG. Die Rust-Seite
    tut dasselbe.
*/
void fahreBandwertgrenzen()
{
    bool ok = false;
    const auto q = lies ("eq-copilot/schemas/v3/quantisierung-v1.json", ok);
    if (! ok)
        return;

    bool schemaOk = false;
    const auto schema = lies ("eq-copilot/schemas/v3/eq-ipc-v3.schema.json", schemaOk);
    if (! schemaOk)
        return;

    const auto plaus = q.getProperty ("plausibler_bereich_db", {});
    const auto grenzen = plaus.getProperty ("traegergrenzen", {});
    auto paar = [&grenzen] (const char* name, int index)
    {
        auto* a = grenzen.getProperty (name, {}).getArray();
        return a != nullptr && a->size() == 2 ? static_cast<int> ((*a)[index]) : -1;
    };

    pruefe (paar ("q_db_0p1_i16", 0) == nakama::telemetrie::q0p1Min
            && paar ("q_db_0p1_i16", 1) == nakama::telemetrie::q0p1Max
            && paar ("q_db_0p01_i16", 0) == nakama::telemetrie::q0p01Min
            && paar ("q_db_0p01_i16", 1) == nakama::telemetrie::q0p01Max,
            "Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json");

    // Und die Traegergrenzen muessen wirklich aus den dB-Werten folgen.
    auto* db = plaus.getProperty ("wert", {}).getArray();
    const double lo = db != nullptr && db->size() == 2 ? static_cast<double> ((*db)[0]) : 0.0;
    const double hi = db != nullptr && db->size() == 2 ? static_cast<double> ((*db)[1]) : 0.0;
    pruefe (static_cast<int> (lo * 10.0) == nakama::telemetrie::q0p1Min
            && static_cast<int> (hi * 100.0) == nakama::telemetrie::q0p01Max,
            "Traegergrenzen folgen aus den dB-Werten mal Skalierung");

    // 28.08.2026: Auch der JSON-Vertrag liest die Grenzen nicht aus einer
    // dritten, ungemessenen Kopie. Seine vier i16-Zweige muessen bei jedem
    // Lauf mit derselben quantisierung-v1.json uebereinstimmen wie die beiden
    // Binaerleser oben.
    const auto defs = schema.getProperty ("$defs", {});
    auto schemaHatGrenzen = [&defs] (const char* definition, const char* encoding,
                                     int minimum, int maximum)
    {
        auto* zweige = defs.getProperty (definition, {}).getProperty ("oneOf", {}).getArray();
        if (zweige == nullptr)
            return false;
        for (const auto& zweig : *zweige)
        {
            const auto props = zweig.getProperty ("properties", {});
            if (props.getProperty ("encoding", {}).getProperty ("const", {}).toString()
                != encoding)
                continue;
            const auto items = props.getProperty ("werte", {}).getProperty ("items", {});
            return items.getProperty ("type", {}).toString() == "integer"
                   && static_cast<int> (items.getProperty ("minimum", {})) == minimum
                   && static_cast<int> (items.getProperty ("maximum", {})) == maximum;
        }
        return false;
    };

    const int q0p1Min = paar ("q_db_0p1_i16", 0);
    const int q0p1Max = paar ("q_db_0p1_i16", 1);
    const int q0p01Min = paar ("q_db_0p01_i16", 0);
    const int q0p01Max = paar ("q_db_0p01_i16", 1);
    pruefe (schemaHatGrenzen ("bandwerte_fein", "q_db_0p1_i16", q0p1Min, q0p1Max)
            && schemaHatGrenzen ("bandwerte_fein", "q_db_0p01_i16", q0p01Min, q0p01Max)
            && schemaHatGrenzen ("bandwerte_grob", "q_db_0p1_i16", q0p1Min, q0p1Max)
            && schemaHatGrenzen ("bandwerte_grob", "q_db_0p01_i16", q0p01Min, q0p01Max),
            "JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json");
}

// ------------------------------------------------------------------ Bandgitter

void fahreBandgitter()
{
    bool a = false, b = false;
    const auto fein = lies ("eq-copilot/schemas/v3/bandgitter/nakama_1_24_oct_30_18k_v1.json", a);
    const auto grob = lies ("eq-copilot/schemas/v3/bandgitter/nakama_log64_v1.json", b);
    if (! a || ! b)
        return;

    auto zahlen = [] (const juce::var& block)
    {
        juce::Array<double> out;
        if (auto* liste = block.getProperty ("hex64", {}).getArray())
            for (const auto& h : *liste)
                out.add (ausHex64 (h.toString()));
        return out;
    };

    const auto kanten = zahlen (fein.getProperty ("kanten_hz", {}));
    const auto mitten = zahlen (fein.getProperty ("mitten_hz", {}));

    pruefe (static_cast<int> (fein.getProperty ("band_anzahl", {})) == 221,
            "feines Gitter hat 221 Baender");
    pruefe (kanten.size() == 222 && mitten.size() == 221,
            "222 Kanten, 221 Mitten",
            juce::String (kanten.size()) + "/" + juce::String (mitten.size()));

    bool monoton = true, eingebettet = true, endlich = true, imBand = true;
    for (int i = 0; i < kanten.size() - 1; ++i)
        monoton = monoton && kanten[i] < kanten[i + 1];
    for (int i = 0; i < mitten.size(); ++i)
    {
        eingebettet = eingebettet && kanten[i] < mitten[i] && mitten[i] < kanten[i + 1];
        endlich = endlich && std::isfinite (mitten[i]);
        imBand = imBand && mitten[i] >= 30.0 && mitten[i] <= 18000.0;
    }
    pruefe (monoton, "Kanten streng monoton");
    pruefe (eingebettet, "jede Mitte liegt echt zwischen ihren Kanten");
    pruefe (endlich, "keine Mitte ist NaN oder unendlich");
    pruefe (imBand, "alle Mitten liegen in 30 Hz .. 18 kHz");

    const auto grobkanten = zahlen (grob.getProperty ("kanten_hz", {}));
    auto* gruppen = grob.getProperty ("gruppen", {}).getArray();
    pruefe (gruppen != nullptr && gruppen->size() == 64, "grobes Gitter hat 64 Gruppen");
    pruefe (grobkanten.size() == 65, "65 grobe Kanten");
    if (gruppen == nullptr)
        return;

    int erwartetVon = 0, summe = 0;
    bool partition = true, bitgleich = true, nichtLeer = true;
    for (int j = 0; j < gruppen->size(); ++j)
    {
        const auto& g = (*gruppen)[j];
        const int von = static_cast<int> (g.getProperty ("fein_von", {}));
        const int bis = static_cast<int> (g.getProperty ("fein_bis_exkl", {}));
        partition = partition && von == erwartetVon;
        nichtLeer = nichtLeer && bis > von;
        if (j < grobkanten.size() && von < kanten.size())
            bitgleich = bitgleich && bitsVon (grobkanten[j]) == bitsVon (kanten[von]);
        summe += bis - von;
        erwartetVon = bis;
    }
    pruefe (partition && erwartetVon == 221, "Gruppen sind lueckenlos und ueberlappungsfrei");
    pruefe (nichtLeer, "keine leere Gruppe");
    pruefe (summe == 221, "Gruppen decken genau 221 feine Baender", juce::String (summe));
    pruefe (bitgleich && bitsVon (grobkanten[64]) == bitsVon (kanten[221]),
            "grobe Kanten sind BITGLEICHE Kopien feiner Kanten");
}

// ------------------------------------------------------------------ Quantisierung

/*  Vertrag aus quantisierung-v1.json. std::round ist bereits "halbe Werte von
    null weg" - genau die dort festgelegte Regel. */
void quantisiereI16 (double db, double skalierung, int& wert, bool& gueltig, bool& saturiert)
{
    if (! std::isfinite (db)) { wert = 0; gueltig = false; saturiert = false; return; }
    const double roh = std::round (db * skalierung);
    gueltig = true;
    if (roh > 32767.0)  { wert =  32767; saturiert = true;  return; }
    if (roh < -32768.0) { wert = -32768; saturiert = true;  return; }
    wert = static_cast<int> (roh);
    saturiert = false;
}

/*  Der Ueberlauf wird VOR dem Cast geprueft: eine double-nach-float-Umwandlung
    ausserhalb des darstellbaren Bereichs ist in C++ undefiniert, waehrend Rust
    dort +-inf liefert und Python eine Ausnahme wirft. Drei Meldewege, eine
    Grenze - der Vertrag legt fest, dass alle drei sie ABFANGEN. */
void quantisiereF32 (double db, uint32_t& bits, bool& gueltig, bool& saturiert)
{
    constexpr double kUeberlauf = 3.4028235677973366e38;   // Rundungsgrenze zu inf
    constexpr float  kMaxF32    = 3.4028234663852886e38f;

    if (! std::isfinite (db)) { float n = 0.0f; std::memcpy (&bits, &n, 4); gueltig = false; saturiert = false; return; }
    gueltig = true;
    if (std::abs (db) >= kUeberlauf)
    {
        const float g = db < 0 ? -kMaxF32 : kMaxF32;
        std::memcpy (&bits, &g, 4);
        saturiert = true;
        return;
    }
    const float eng = static_cast<float> (db);
    std::memcpy (&bits, &eng, 4);
    saturiert = false;
}

void fahreQuantisierung()
{
    bool ok = false;
    const auto q = lies ("eq-copilot/schemas/v3/quantisierung-v1.json", ok);
    if (! ok)
        return;

    auto* kodierungen = q.getProperty ("kodierungen", {}).getDynamicObject();
    if (kodierungen == nullptr)
        { pruefe (false, "Quantisierungsvertrag traegt Kodierungen"); return; }

    int geprueft = 0, abweichungen = 0;
    for (const auto& eintrag : kodierungen->getProperties())
    {
        const auto name = eintrag.name.toString();
        const auto k = eintrag.value;
        const double skalierung = static_cast<double> (k.getProperty ("skalierung", {}));
        const bool istI16 = k.getProperty ("traeger", {}).toString() == "i16";

        auto* vektoren = k.getProperty ("vektoren", {}).getArray();
        if (vektoren == nullptr)
            continue;

        for (const auto& v : *vektoren)
        {
            const double ein = ausHex64 (v.getProperty ("ein_hex64", {}).toString());
            const bool sollGueltig = static_cast<bool> (v.getProperty ("gueltig", {}));
            const bool sollSat     = static_cast<bool> (v.getProperty ("saturiert", {}));
            bool gueltig = false, sat = false;
            bool stimmt = false;

            if (istI16)
            {
                int wert = 0;
                quantisiereI16 (ein, skalierung, wert, gueltig, sat);
                stimmt = wert == static_cast<int> (v.getProperty ("wert", {}));
            }
            else
            {
                uint32_t bits = 0;
                quantisiereF32 (ein, bits, gueltig, sat);
                stimmt = juce::String::toHexString (static_cast<int> (bits))
                            .paddedLeft ('0', 8).toUpperCase()
                         == v.getProperty ("wert_hex32", {}).toString();
            }

            if (! (stimmt && gueltig == sollGueltig && sat == sollSat))
            {
                std::cout << "[ROT]  " << name.toRawUTF8() << " "
                          << v.getProperty ("ein_dezimal", {}).toString().toRawUTF8()
                          << ": Wert/Flags weichen ab" << std::endl;
                ++abweichungen;
            }
            ++geprueft;
        }
    }

    pruefe (abweichungen == 0,
            "Quantisierung stimmt mit den Testvektoren (" + juce::String (geprueft) + ")",
            abweichungen == 0 ? juce::String() : juce::String (abweichungen) + " Abweichungen");
    pruefe (geprueft >= 60, "genug Vektoren", juce::String (geprueft));
}

// ------------------------------------------------------------------ Riegelproben

/** Ein gruener Test ist nichts wert, solange nicht gezeigt wurde, dass er
    ueberhaupt fallen kann. Diese Proben bringen jeden Riegel einmal zum
    Fallen - im Test selbst, damit es niemand von Hand nachstellen muss. */
/*  Der Textriegel gegen die GEMEINSAME Falltabelle.

    T2-Runde 2, Befund BF-5: vorher trug jedes der drei Beine eine EIGENE Kopie
    dieser Tabelle - gezaehlt 31, 32 und 33 Faelle -, waehrend das
    Beweismanifest "dieselbe 31-Faelle-Tabelle" behauptete. Drei handgepflegte
    Kopien driften; genau das ist passiert. Die Tabelle steht deshalb jetzt in
    eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json und wird von allen drei
    Beinen GELESEN.

    Der Text steht dort hex-kodiert: die Tabelle enthaelt NUL-Escapes, rohe
    Steuerzeichen, kaputtes UTF-8 und ein BOM - Inhalte, an denen ein
    JSON-Leser sich verschluckt. Deshalb geht der Riegel hier auch ueber die
    BYTE-Fassung.
*/
void fahreTextriegelproben()
{
    bool ok = false;
    const auto tabelle = lies ("eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json", ok);
    if (! ok)
        return;

    auto* faelle = tabelle.getProperty ("faelle", {}).getArray();
    if (faelle == nullptr)
    {
        pruefe (false, "Falltabelle traegt eine Fallliste");
        return;
    }

    int rot = 0;
    for (const auto& fall : *faelle)
    {
        const auto hex = fall.getProperty ("text_hex", {}).toString();
        juce::MemoryBlock roh;
        roh.loadFromHexString (hex);

        juce::String fallFehler;
        const bool sauber = nakama::vertrag::textriegelBytes (roh.getData(), roh.getSize(), fallFehler);
        const bool sollAbgelehnt = static_cast<bool> (fall.getProperty ("wird_abgelehnt", false));
        if (sauber == sollAbgelehnt)
        {
            ++rot;
            std::cout << "[ROT]  Textriegel #" << (int) fall.getProperty ("nr", 0)
                      << " " << fall.getProperty ("zeigetext", {}).toString().toRawUTF8()
                      << " -> " << (sauber ? "angenommen" : fallFehler.toRawUTF8()) << std::endl;
        }
    }

    pruefe (rot == 0, "Textriegel deckt die gemeinsame Falltabelle",
            juce::String (faelle->size()) + " Faelle");
    pruefe (faelle->size() >= 50, "Falltabelle hat Substanz",
            juce::String (faelle->size()) + " Faelle");

    const char winzig = ' ';
    juce::String grenzenFehler;
    pruefe (! nakama::vertrag::textriegelBytes (
                &winzig, nakama::vertrag::kMaxDokumentBytes + 1u, grenzenFehler)
            && grenzenFehler == "Dokument zu gross",
            "Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab");
}

void fahreRiegelproben()
{
    using nakama::vertrag::Schema;

    auto ausText = [] (const char* json)
    {
        juce::var v;
        juce::JSON::parse (juce::String::fromUTF8 (json), v);
        return v;
    };

    Schema s; juce::String f;

    pruefe (! Schema::laden (ausText (R"({"type":"object","multipleOf":2})"), s, f)
            && f.contains ("multipleOf"),
            "unbekanntes Schluesselwort bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"oneOf":[]})"), s, f)
            && f.contains ("x-nakama-discriminator"),
            "oneOf ohne Discriminator bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"object","additionalProperties":true})"), s, f)
            && f.contains ("maxProperties"),
            "additives Objekt ohne maxProperties bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"string","pattern":"^S-1-.*$"})"), s, f)
            && f.contains ("Muster"),
            "unbekanntes Muster bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"$ref":"https://fremd/schema.json"})"), s, f)
            && f.contains ("nicht-lokale"),
            "nicht-lokale Referenz bricht den Ladevorgang", f);

    // --- T2-Runde 1 -------------------------------------------------------
    // Beide Riegel schliessen dieselbe Klasse wie "unbekanntes Schluesselwort",
    // nur eine Ebene tiefer: etwas im Schema, das eine Engine anders liest als
    // die andere, ohne dass jemand es merkt.

    pruefe (! Schema::laden (ausText (R"({"$ref":"#/$defs/gibtsnicht","$defs":{"a":{"type":"object"}}})"), s, f)
            && f.contains ("haengende Referenz"),
            "haengende Referenz bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"string","maxLength":5.0})"), s, f)
            && f.contains ("Werttyp"),
            "maxLength als Gleitkommazahl bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":5})"), s, f)
            && f.contains ("Werttyp"),
            "type als Zahl bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"object","required":[5]})"), s, f)
            && f.contains ("Werttyp"),
            "required mit Nicht-String bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"object","x-nakama-discriminator":7,"oneOf":[{"type":"object"}]})"), s, f)
            && f.contains ("Werttyp"),
            "Discriminator als Zahl bricht den Ladevorgang", f);

    // Ein kleines, vollstaendiges Schema fuer die Verhaltensproben.
    const char* klein = R"({
      "x-nakama-discriminator": "type",
      "oneOf": [{ "$ref": "#/$defs/a" }],
      "$defs": { "a": {
        "type": "object",
        "required": ["type", "n"],
        "additionalProperties": false,
        "properties": {
          "type": { "const": "a" },
          "n": { "type": "integer", "minimum": 0 },
          "s": { "type": "string", "maxLength": 3 }
        }
      }}
    })";
    if (! Schema::laden (ausText (klein), s, f))
        { pruefe (false, "Probeschema laedt", f); return; }

    pruefe (s.gueltig (ausText (R"({"type":"a","n":1.0})")),
            "integer akzeptiert 1.0 (draft 2020-12)");
    pruefe (! s.gueltig (ausText (R"({"type":"a","n":1.5})")),
            "integer lehnt 1.5 ab");

    const auto lawine = s.pruefe (ausText (R"({"type":"a","n":1,"s":12345})"));
    pruefe (lawine.size() == 1 && lawine.getReference (0).schluessel == "type",
            "Typfehler erzeugt keine Lawine",
            juce::String (lawine.size()) + " Verletzung(en)");

    const auto fremd = s.pruefe (ausText (R"({"type":"b"})"));
    pruefe (fremd.size() == 1 && fremd.getReference (0).instanz == "/type"
            && fremd.getReference (0).schluessel == "oneOf",
            "unbekannter Discriminator wird abgelehnt");

    const char* boolSchema = R"({
      "type":"object","required":["flag"],"additionalProperties":false,
      "properties":{"flag":{"type":"boolean"},"wahr":{"type":"integer"},"falsch":{"type":"integer"}},
      "x-nakama-discriminator":"flag",
      "oneOf":[
        {"required":["wahr"],"properties":{"flag":{"const":true}}},
        {"required":["falsch"],"properties":{"flag":{"const":false}}}
      ]
    })";
    Schema bs; juce::String bf;
    if (Schema::laden (ausText (boolSchema), bs, bf))
    {
        pruefe (bs.gueltig (ausText (R"({"flag":true,"wahr":1})"))
                && bs.gueltig (ausText (R"({"flag":false,"falsch":1})"))
                && ! bs.gueltig (ausText (R"({"flag":true,"falsch":1})")),
                "discriminator_boolean_true_false");
        const auto typ = bs.pruefe (ausText (R"({"flag":"true"})"));
        pruefe (std::any_of (typ.begin(), typ.end(), [] (const auto& x)
                { return x.instanz == "/flag" && x.schluessel == "oneOf"; }),
                "discriminator_boolean_falscher_typ");
        const auto ohne = bs.pruefe (ausText (R"({})"));
        pruefe (std::any_of (ohne.begin(), ohne.end(), [] (const auto& x)
                { return x.instanz == "/flag" && x.schluessel == "oneOf"; }),
                "discriminator_boolean_fehlt");
    }
    else
    {
        pruefe (false, "Boolean-Discriminator-Probeschema laedt", bf);
    }

    const char* pointerSchema = R"({
      "type":"object","required":["validity"],"additionalProperties":false,
      "properties":{"validity":{"type":"object","required":["active"],"additionalProperties":false,
        "properties":{"active":{"type":"boolean"}}}},
      "x-nakama-discriminator":"/validity/active",
      "oneOf":[
        {"properties":{"validity":{"properties":{"active":{"const":true}}}}},
        {"properties":{"validity":{"properties":{"active":{"const":false}}}}}
      ]
    })";
    Schema ps; juce::String pf;
    if (Schema::laden (ausText (pointerSchema), ps, pf))
    {
        pruefe (ps.gueltig (ausText (R"({"validity":{"active":true}})"))
                && ps.gueltig (ausText (R"({"validity":{"active":false}})")),
                "discriminator_json_pointer_boolean");
        const auto segment = ps.pruefe (ausText (R"({"validity":{}})"));
        pruefe (std::any_of (segment.begin(), segment.end(), [] (const auto& x)
                { return x.instanz == "/validity/active" && x.schluessel == "oneOf"; }),
                "discriminator_json_pointer_segment_fehlt");
    }
    else
    {
        pruefe (false, "RFC-6901-Discriminator-Probeschema laedt", pf);
    }

    const auto wurzelString = s.pruefe (ausText (R"("a")"));
    pruefe (wurzelString.size() == 1 && wurzelString.getReference (0).instanz.isEmpty(),
            "Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type");

    const auto fehlt = s.pruefe (ausText (R"({"type":"a"})"));
    pruefe (fehlt.size() == 1 && fehlt.getReference (0).instanz.isEmpty()
            && fehlt.getReference (0).schema == "#/$defs/a/required/n",
            "fehlendes Pflichtfeld zeigt auf das Elternobjekt");

    // "aeoeue" sind 3 Codepunkte, aber 6 UTF-8-Bytes. maxLength ist 3.
    pruefe (s.gueltig (ausText ("{\"type\":\"a\",\"n\":0,\"s\":\"\xc3\xa4\xc3\xb6\xc3\xbc\"}")),
            "Laenge zaehlt Codepunkte, nicht Bytes");
    pruefe (! s.gueltig (ausText ("{\"type\":\"a\",\"n\":0,\"s\":\"\xc3\xa4\xc3\xb6\xc3\xbcx\"}")),
            "vier Codepunkte fallen bei maxLength 3");

    const auto mehrere = s.pruefe (ausText (R"({"type":"a","x":1,"y":2})"));
    bool sortiert = true;
    for (int i = 1; i < mehrere.size(); ++i)
        sortiert = sortiert && (mehrere.getReference (i - 1) < mehrere.getReference (i));
    pruefe (mehrere.size() == 3 && sortiert,
            "Verletzungen sind kanonisch sortiert und doppelfrei",
            juce::String (mehrere.size()));

    // T2-Runde 1, Hypothese des Pruefers: ein OBJEKTWERTIGES `const` haette die
    // beiden Engines auseinanderlaufen lassen - die C++-Seite verglich zwei
    // Objekte immer als ungleich. Heute unerreichbar (alle const/enum-Werte im
    // Schema sind Skalare), deshalb steht die Probe hier und nicht im Korpus.
    // Die Schluesselreihenfolge ist ABSICHTLICH vertauscht: NamedValueSet und
    // serde_json::Map ordnen verschieden.
    Schema o; juce::String of;
    const char* objektConst = R"({
      "x-nakama-discriminator": "type",
      "oneOf": [{ "$ref": "#/$defs/a" }],
      "$defs": { "a": {
        "type": "object",
        "required": ["type", "k"],
        "additionalProperties": false,
        "properties": {
          "type": { "const": "a" },
          "k": { "const": { "p": 1, "q": [2, 3] } }
        }
      }}
    })";
    if (Schema::laden (ausText (objektConst), o, of))
    {
        pruefe (o.gueltig (ausText (R"({"type":"a","k":{"q":[2,3],"p":1}})")),
                "objektwertiges const vergleicht reihenfolgeunabhaengig");
        pruefe (! o.gueltig (ausText (R"({"type":"a","k":{"p":1,"q":[2,4]}})")),
                "objektwertiges const sieht einen Unterschied in der Tiefe");
        pruefe (! o.gueltig (ausText (R"({"type":"a","k":{"p":1}})")),
                "objektwertiges const sieht eine fehlende Eigenschaft");
    }
    else
    {
        pruefe (false, "Probeschema mit objektwertigem const laedt", of);
    }
}

} // namespace

int main (int, char*[])
{
    // Kein ScopedJuceInitialiser: dieses Ziel linkt nur juce_core (File,
    // String, JSON, var) - es gibt weder Message-Loop noch GUI zu starten.
    std::cout << "EqCopSchemaTest - v3-Vertrag (SONDE-005a)" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    fahreTextriegelproben();
    fahreRiegelproben();
    fahreFbKorpus();
    fahreBandwertgrenzen();
    fahreBandStereoRoundtrip();
    fahreWireZahlFixture();

    bool ok = false;
    const auto schemaVar = lies ("eq-copilot/schemas/v3/eq-ipc-v3.schema.json", ok);
    if (ok)
    {
        nakama::vertrag::Schema schema;
        juce::String ladefehler;
        if (nakama::vertrag::Schema::laden (schemaVar, schema, ladefehler))
        {
            pruefe (true, "v3-Schema haelt die Engine-Teilmenge ein");
            fahreKorpus (schema);
        }
        else
        {
            pruefe (false, "v3-Schema haelt die Engine-Teilmenge ein", ladefehler);
        }
    }

    fahreBandgitter();
    fahreQuantisierung();
    fahreWireZahl();
    fahreLocale();

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
