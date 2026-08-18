// M1-Golden-Test: Kreuzvalidierung der AnalyseEngine gegen die eingefrorene
// Referenz des Offline-Analyzers (fixtures/golden-referenz.json, erzeugt aus
// tools/analyze-track.py). Plan §11 M1-Abnahme.
//
// Aufruf:  EqCopGoldenTest <pfad-zu-eq-copilot/fixtures>
// Die WAVs sind nicht eingecheckt — vorher einmal
//   py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav
// laufen lassen. SHA-256 wird gegen das JSON geprüft (Determinismus-Riegel).
//
// Toleranz-Grundsätze (kMetricsVersion, dokumentiert je Prüfung):
//  · Skalare + 8192er-Referenz-LTAS laufen auf EXAKT der analyze-track-Achse
//    → enge Toleranzen.
//  · Die Komposit-Kurve (§5.10.1 Mehrfachauflösung) wird nur bei
//    BREITBAND-Inhalt kurvenverglichen: Schmalband-Bandpegel sind physikalisch
//    auflösungsabhängig (ENBW/Hauptkeule) — beim Sinus zählt stattdessen der
//    strukturelle Peak-Check, beim 116-Hz-Fall wird ±1/6 Okt um den Ton
//    ausgenommen. Alles andere wäre Scheingenauigkeit.
//  · Flatness ist bewusst NUR eine Größenordnungs-Prüfung: die Referenz misst
//    auf librosas 22050-Hz-Achse mit deren amin-Boden, die Engine auf der
//    eigenen 48-kHz-Achse.
//  · stille-mit-tick beweist die Produktregel: die Referenz LISTET dort sechs
//    Kleinstpegel-"Resonanzen" — die Engine muss schweigen (§5.3/§5.10.2).
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_cryptography/juce_cryptography.h>
#include "AnalyseEngine.h"
#include "Diagnose.h"
#include <cmath>

using namespace eqcop;

struct Pruefer
{
    int fehler = 0, ok = 0;
    juce::String signal;

    void melde (bool bestanden, const juce::String& was, const juce::String& detail)
    {
        if (bestanden) { ++ok; return; }
        ++fehler;
        std::printf ("  FEHLER [%s] %s — %s\n", signal.toRawUTF8(), was.toRawUTF8(), detail.toRawUTF8());
    }
    void nahe (double ist, double soll, double tol, const juce::String& was)
    {
        melde (std::isfinite (ist) && std::abs (ist - soll) <= tol, was,
               "ist " + juce::String (ist, 4) + ", soll " + juce::String (soll, 4)
               + " (Toleranz " + juce::String (tol, 3) + ")");
    }
    void relativ (double ist, double soll, double relTol, const juce::String& was)
    {
        melde (std::isfinite (ist) && std::abs (ist - soll) <= relTol * std::abs (soll), was,
               "ist " + juce::String (ist, 4) + ", soll " + juce::String (soll, 4)
               + " (rel. Toleranz " + juce::String (relTol * 100.0, 1) + " %)");
    }
    void wahr (bool cond, const juce::String& was, const juce::String& detail = "Bedingung verletzt")
    {
        melde (cond, was, detail);
    }
};

// Kurvenvergleich über Bänder, deren Referenzwert über dem Boden liegt.
static void vergleicheKurve (Pruefer& p, const juce::String& name,
                             const std::array<double, kLtasBaender>& ist,
                             const std::vector<double>& soll,
                             double meanTol, double maxTol,
                             double bodenRelDb = 70.0,
                             double ausnahmeZentrumHz = 0.0,
                             double ausnahmeOktaven = 0.0,
                             const std::array<double, kLtasBaender>* zentren = nullptr)
{
    double refMax = -1e18;
    for (const double v : soll)
        if (std::isfinite (v))
            refMax = std::max (refMax, v);

    double summe = 0.0, groesste = 0.0;
    int nVergleich = 0, groesstesBand = -1;
    for (size_t b = 0; b < soll.size() && b < (size_t) kLtasBaender; ++b)
    {
        if (! std::isfinite (soll[b]) || soll[b] <= refMax - bodenRelDb)
            continue;
        if (ausnahmeZentrumHz > 0.0 && zentren != nullptr
            && std::abs (std::log2 ((*zentren)[b] / ausnahmeZentrumHz)) <= ausnahmeOktaven)
            continue;
        const double istWert = ist[b];
        if (std::isnan (istWert))
        {
            p.melde (false, name, "Band " + juce::String ((int) b) + " ist NaN, Referenz "
                                  + juce::String (soll[b], 2));
            return;
        }
        const double delta = std::abs (istWert - soll[b]);
        summe += delta;
        if (delta > groesste)
        {
            groesste = delta;
            groesstesBand = (int) b;
        }
        ++nVergleich;
    }
    // Sicherung gegen Leerlauf-Vergleich; ein reiner Sinus hat nur ~5 Bänder
    // über dem Boden — die Mindestzahl muss vom Signal abhängen dürfen.
    p.wahr (nVergleich >= 3, name + " Abdeckung",
            "nur " + juce::String (nVergleich) + " vergleichbare Baender");
    if (nVergleich > 0)
    {
        juce::String wo;
        if (groesstesBand >= 0 && zentren != nullptr)
            wo = " bei " + juce::String ((*zentren)[(size_t) groesstesBand], 1) + " Hz";
        else if (groesstesBand >= 0)
            wo = " (Band " + juce::String (groesstesBand) + ")";
        p.melde (summe / nVergleich <= meanTol, name + " mittl. Abw.",
                 juce::String (summe / nVergleich, 3) + " dB > " + juce::String (meanTol, 2) + " dB");
        p.melde (groesste <= maxTol, name + " max. Abw.",
                 juce::String (groesste, 3) + " dB > " + juce::String (maxTol, 2) + " dB" + wo
                 + " (ist " + juce::String (groesstesBand >= 0 ? ist[(size_t) groesstesBand] : 0.0, 2)
                 + ", soll " + juce::String (groesstesBand >= 0 ? soll[(size_t) groesstesBand] : 0.0, 2) + ")");
    }
}

// ── M3-Diagnose: strukturelle Prüfungen auf konstruierten Fehlern ──────────
// Beweisidee: die diag-*-WAVs tragen GENAU EINEN eingebauten Fehler (Gauß-
// Glocke in log-f auf Pink) — die Diagnose muss genau seine Befundklasse
// liefern; das unveränderte Pink und die Stille bleiben karten-still
// (Falsch-Positiv-Riegel). Jede Karte muss die §7.3/§7.4-Struktur füllen.
static bool hatKlasse (const std::vector<Befund>& bs, BefundKlasse k)
{
    for (const auto& b : bs)
        if (b.klasse == k)
            return true;
    return false;
}

static void pruefeKartenStruktur (Pruefer& p, const std::vector<Befund>& bs)
{
    for (const auto& b : bs)
    {
        p.wahr (b.titel.isNotEmpty() && b.gemessen.isNotEmpty() && b.wirkung.isNotEmpty()
                    && b.tu.isNotEmpty() && b.warum.isNotEmpty() && b.hoeren.isNotEmpty(),
                "Karte vollstaendig (§7.3)", "leeres Feld in: " + b.titel);
        p.wahr (b.gemessen.containsChar ('d') && b.gemessen.contains ("dB"),
                "Gemessen traegt Zahlen", b.gemessen);
        p.wahr (b.hoeren.contains ("Stopp"), "Hoeren hat Stop-Bedingung (§7.4)", b.hoeren);
        p.wahr (b.konfidenzGrund.isNotEmpty(), "Konfidenz benannt", b.titel);
    }
}

static void diagnosePruefungen (Pruefer& p, const juce::String& name, const MessSnapshot& m)
{
    const auto bs = diagnose (m, "hub");

    if (name == "stille-mit-tick")
    {
        p.wahr (bs.empty(), "Diagnose still bei Stille",
                juce::String ((int) bs.size()) + " Karten");
        return;
    }
    pruefeKartenStruktur (p, bs);

    if (name == "pink-minus20")
    {
        // DER Falsch-Positiv-Riegel: neutrales Breitband ⇒ null Karten.
        p.wahr (bs.empty(), "Falsch-Positiv-Riegel: Pink ohne Karten",
                juce::String ((int) bs.size()) + " Karten (erste: "
                + (bs.empty() ? juce::String() : bs.front().titel) + ")");
    }
    else if (name == "sinus-1k-minus12")
    {
        // Sinus: genau die Resonanz-Karte; die Zonen-Gates (Abdeckung §5.10.2)
        // müssen alle Balance-Regeln schlucken.
        p.wahr (bs.size() == 1 && bs.front().klasse == BefundKlasse::resonanz,
                "Sinus: genau 1 Resonanz-Karte",
                juce::String ((int) bs.size()) + " Karten");
    }
    else if (name == "resonanz-116hz")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::resonanz), "116-Hz-Karte vorhanden", "fehlt");
        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch)
                    && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am 116er", "Scoop/Haerte gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::resonanz)
            {
                // Frequenz = Band-Center, nie feiner als das Fenster (§5.10.3)
                // — deshalb ±1/6 Okt um 116, nicht wörtlich „116".
                p.wahr (std::abs (std::log2 (b.fSchwerpunkt / 116.0)) <= 1.0 / 6.0,
                        "Karte am 116er (Bassfenster)", juce::String (b.fSchwerpunkt, 1) + " Hz");
                const juce::String fWort = juce::String ((int) std::lround (b.fSchwerpunkt)) + " Hz";
                p.wahr (b.titel.contains (fWort), "Titel nennt die Frequenz", b.titel);
                // M3a: musikalische Verortung in FL-Zählung (116 Hz = A#3;
                // MIDI 60 = C5 — Landmine tools/midi-ideen).
                p.wahr (b.titel.contains ("A#3"), "Titel nennt die Note (FL-Zaehlung)", b.titel);
                p.wahr (b.titel.contains (juce::CharPointer_UTF8 ("dauerhaft")),
                        "als dauerhaft erkannt", b.titel);
                p.wahr (b.tu.contains (fWort) && b.tu.containsChar ((juce::juce_wchar) 8722 /* − */),
                        "Tu nennt Frequenz + konkreten Absenk-Startwert", b.tu);
                p.wahr (b.konfidenz == Konfidenz::hoch,
                        "messfeste Dauer-Resonanz = Konfidenz hoch", b.konfidenzGrund);
                p.wahr (b.topRang, "Resonanz ist Top-Rang", b.titel);
            }
    }
    else if (name == "diag-scoop-mitte")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::mittenLoch), "Mitten-Loch gefunden",
                juce::String ((int) bs.size()) + " Karten, keine mittenLoch");
        p.wahr (! hatKlasse (bs, BefundKlasse::mulm) && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am Scoop", "Mulm/Haerte faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::mittenLoch)
            {
                p.wahr (b.staerkeDb <= -3.0, "Fehlbetrag >= Schwelle",
                        juce::String (b.staerkeDb, 2) + " dB");
                p.wahr (b.tu.contains ("Fader") || b.tu.contains ("Balance")
                            || b.tu.contains (juce::CharPointer_UTF8 ("nach vorn")),
                        "Erstidee ist Balance, nicht EQ (§5.10.6 breit+dauerhaft)", b.tu);
                p.wahr (b.konfidenz != Konfidenz::niedrig && b.topRang,
                        "Scoop erreicht Top-Rang", b.konfidenzGrund);
            }
    }
    else if (name == "diag-mulm")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::mulm), "Mulm gefunden",
                juce::String ((int) bs.size()) + " Karten, kein mulm");
        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch) && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am Mulm", "Scoop/Haerte faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::mulm)
            {
                p.wahr (b.staerkeDb >= 4.0, "Ueberschuss >= Schwelle",
                        juce::String (b.staerkeDb, 2) + " dB");
                p.wahr (b.fSchwerpunkt > 110.0 && b.fSchwerpunkt < 320.0,
                        "Schwerpunkt in der Zone", juce::String (b.fSchwerpunkt, 1) + " Hz");
                p.wahr (b.warum.contains ("Sustain") || b.warum.contains ("Fundament"),
                        "Warum schuetzt das Klavier-Fundament", b.warum);
            }
    }
    else if (name == "diag-haerte")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::haerte), "Haerte gefunden",
                juce::String ((int) bs.size()) + " Karten, keine haerte");
        p.wahr (! hatKlasse (bs, BefundKlasse::mulm), "kein Mulm an der Haerte", "faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::haerte)
                p.wahr (b.tu.contains ("Smooth Operator"),
                        "dynamische Erstidee nennt sein Werkzeug", b.tu);
    }
    else if (name == "diag-mulm-halb")
    {
        // M3a: das Zeitmittel triggert die Karte, der Zeitverlauf macht sie
        // ehrlich — „Das war in NN % der Musikzeit so." muss auf der Karte
        // stehen (Lernsprache §7.4).
        p.wahr (hatKlasse (bs, BefundKlasse::mulm), "Halbzeit-Mulm gefunden",
                juce::String ((int) bs.size()) + " Karten, kein mulm");
        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch) && ! hatKlasse (bs, BefundKlasse::haerte),
                "keine Fremdklassen am Halbzeit-Mulm", "Scoop/Haerte faelschlich gemeldet");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::mulm)
                p.wahr (b.gemessen.contains (juce::CharPointer_UTF8 ("% der Musikzeit")),
                        "Karte traegt den Zeitverlauf", b.gemessen);
    }
    else if (name == "diag-wander-ton")
    {
        p.wahr (hatKlasse (bs, BefundKlasse::resonanz), "Resonanz-Karte vorhanden", "fehlt");
        for (const auto& b : bs)
            if (b.klasse == BefundKlasse::resonanz)
            {
                p.wahr (! b.dauerhaft, "Karte sagt zeitweise (wandernder Ton)", b.titel);
                p.wahr (b.titel.contains (juce::CharPointer_UTF8 ("zeitweise")),
                        "Titel nennt zeitweise", b.titel);
                p.wahr (b.tu.contains ("Smooth Operator"),
                        "zeitweise ⇒ dynamische Erstidee", b.tu);
            }
    }
}

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        std::printf ("Aufruf: EqCopGoldenTest <pfad-zu-eq-copilot/fixtures>\n");
        return 2;
    }
    const juce::File fixDir { juce::String { juce::CharPointer_UTF8 { argv[1] } } };
    const auto refDatei = fixDir.getChildFile ("golden-referenz.json");
    if (! refDatei.existsAsFile())
    {
        std::printf ("golden-referenz.json fehlt unter %s\n", fixDir.getFullPathName().toRawUTF8());
        return 2;
    }
    const auto wurzel = juce::JSON::parse (refDatei.loadFileAsString());
    if (! wurzel.isObject())
    {
        std::printf ("golden-referenz.json nicht parsebar\n");
        return 2;
    }
    const auto rate = (double) wurzel["rate_hz"];
    const auto shaObjekt = wurzel["wav_sha256"];
    const auto signale = wurzel["signale"];

    juce::AudioFormatManager formate;
    formate.registerBasicFormats();

    int fehlerGesamt = 0, okGesamt = 0;
    // Die vier Kern-Signale tragen die M1-Kreuzvalidierung (analyze-track-
    // Referenz); die diag-*-Signale tragen NUR SHA-Riegel + M3-Diagnose.
    // M3a: diag-mulm-halb + diag-wander-ton beweisen den ZEITVERLAUF
    // (Zonen-Persistenz ≈ 50 % · wandernder Ton = zeitweise, nie dauerhaft).
    const char* namen[] = { "sinus-1k-minus12", "pink-minus20", "resonanz-116hz", "stille-mit-tick",
                            "diag-scoop-mitte", "diag-mulm", "diag-haerte",
                            "diag-mulm-halb", "diag-wander-ton" };
    for (const char* name : namen)
    {
        Pruefer p;
        p.signal = name;
        const auto wav = fixDir.getChildFile (juce::String (name) + ".wav");
        if (! wav.existsAsFile())
        {
            std::printf ("WAV fehlt: %s\n  Erst erzeugen:  py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav\n",
                         wav.getFullPathName().toRawUTF8());
            return 2;
        }
        // Determinismus-Riegel: die WAVs müssen exakt der eingefrorene Stand sein.
        {
            juce::FileInputStream strom (wav);
            const auto sha = juce::SHA256 (strom).toHexString();
            const auto soll = shaObjekt[juce::Identifier (name)].toString();
            if (! sha.equalsIgnoreCase (soll))
            {
                std::printf ("SHA-256-Abweichung bei %s\n  ist  %s\n  soll %s\n  Fixtures neu erzeugen (erzeuge_fixtures.py --nur-wav).\n",
                             name, sha.toRawUTF8(), soll.toRawUTF8());
                return 2;
            }
        }

        std::unique_ptr<juce::AudioFormatReader> leser (formate.createReaderFor (wav));
        if (leser == nullptr || leser->numChannels != 2 || std::abs (leser->sampleRate - rate) > 0.5)
        {
            std::printf ("WAV unlesbar/falsches Format: %s\n", wav.getFullPathName().toRawUTF8());
            return 2;
        }

        AnalyseEngine engine;
        engine.vorbereiten (rate);
        const int blockFrames = 512;
        juce::AudioBuffer<float> puffer (2, blockFrames);
        std::vector<float> interleaved ((size_t) blockFrames * 2);
        juce::int64 gelesen = 0;
        while (gelesen < (juce::int64) leser->lengthInSamples)
        {
            const int n = (int) std::min<juce::int64> (blockFrames, (juce::int64) leser->lengthInSamples - gelesen);
            leser->read (&puffer, 0, n, gelesen, true, true);
            for (int i = 0; i < n; ++i)
            {
                interleaved[(size_t) i * 2]     = puffer.getSample (0, i);
                interleaved[(size_t) i * 2 + 1] = puffer.getSample (1, i);
            }
            engine.verarbeite (interleaved.data(), n, 2);
            gelesen += n;
        }
        engine.auswerten();
        const auto m = engine.snapshot();
        const auto ref = signale[juce::Identifier (name)];

        const bool istStille = juce::String (name) == "stille-mit-tick";
        const bool istDiag = juce::String (name).startsWith ("diag-");

        // ── Zustand + Befund-Sperre (§5.3/§5.10.2) ──
        if (istDiag)
        {
            p.wahr (m.zustand == MessZustand::messbereit, "Zustand messbereit",
                    "20 s aktives Signal muss messbereit sein");

            // ── M3a: Zeitverlauf-Beweise auf Engine-Ebene ──
            if (juce::String (name) == "diag-mulm")
            {
                // Stationärer Mulm: der Zeitverlauf muss „praktisch durchgehend"
                // melden (die EMA braucht ~3 s zum Einschwingen — 0,8 genügt).
                const auto& zz = m.zonenZeit[(size_t) kZonenSlotMulm];
                p.wahr (zz.gueltig && zz.ticks >= 15, "Mulm-Zeitverlauf belegt",
                        "ticks " + juce::String ((int) zz.ticks));
                p.wahr (zz.anteil >= 0.8, "stationaerer Mulm ~durchgehend",
                        "anteil " + juce::String (zz.anteil, 2));
            }
            if (juce::String (name) == "diag-mulm-halb")
            {
                // Halbzeit-Mulm: Anteil MUSS im Fenster um 0,5 liegen — genau
                // das trennt „durchgehend" von „zeitweise" (M3a-Kernbeweis).
                const auto& zz = m.zonenZeit[(size_t) kZonenSlotMulm];
                p.wahr (zz.gueltig && zz.ticks >= 15, "Halbzeit-Zeitverlauf belegt",
                        "ticks " + juce::String ((int) zz.ticks));
                p.wahr (zz.anteil >= 0.30 && zz.anteil <= 0.70, "Halbzeit-Anteil ~0,5",
                        "anteil " + juce::String (zz.anteil, 2));
            }
            if (juce::String (name) == "diag-wander-ton")
            {
                // Wandernder Ton (4 × 5 s): Kandidaten JA, aber keiner darf
                // als dauerhaft gelten — jedes Band trägt den Ton nur ~25 %.
                p.wahr (m.resonanzen.size() >= 2, "wandernder Ton liefert Kandidaten",
                        juce::String ((int) m.resonanzen.size()) + " Kandidaten");
                for (const auto& k : m.resonanzen)
                {
                    p.wahr (k.klasse == ResonanzKlasse::zeitweise,
                            "Kandidat zeitweise (nie dauerhaft)",
                            juce::String (k.freqHz, 1) + " Hz, Persistenz "
                            + juce::String (k.persistenzAnteil, 2));
                    p.wahr (k.persistenzAnteil >= 0.10 && k.persistenzAnteil <= 0.45,
                            "Persistenz ~25 %",
                            juce::String (k.freqHz, 1) + " Hz: "
                            + juce::String (k.persistenzAnteil, 2));
                }
            }
        }
        else if (istStille)
        {
            p.wahr (m.zustand != MessZustand::messbereit, "Zustand",
                    "Stille darf nie messbereit werden (0 s aktiv)");
            p.wahr (m.resonanzen.empty(), "Befund-Sperre",
                    "Referenz listet 6 Kleinstpegel-Funde — die Engine muss schweigen, hat aber "
                    + juce::String ((int) m.resonanzen.size()) + " Kandidaten");
            p.wahr (! m.spektralGueltig, "Spektral-Sperre",
                    "ohne aktive Zeit keine Spektralaussage");
            p.wahr (! m.lufsGueltig, "LUFS null", "Referenz: null (alles unter dem absoluten Gate)");
            p.nahe (m.truePeakDb, (double) ref["tp_dbtp"], 0.4, "True Peak dBTP");
            p.nahe (m.crestDb, (double) ref["crest"], 0.3, "Crest dB");
        }
        else
        {
            p.wahr (m.zustand == MessZustand::messbereit, "Zustand messbereit",
                    "20 s aktives Signal muss messbereit sein");
            p.nahe (m.aktivSekunden, 20.0, 0.5, "aktive Sekunden");

            // ── Skalare (identische 8192er-Achse ⇒ eng) ──
            p.nahe (m.lufsIntegriert, (double) ref["lufs"], 0.3, "LUFS integriert");
            p.nahe (m.truePeakDb, (double) ref["tp_dbtp"], 0.3, "True Peak dBTP");
            p.nahe (m.crestDb, (double) ref["crest"], 0.3, "Crest dB");
            p.relativ (m.centroidMagHz, (double) ref["centroid_mag"], 0.03, "Centroid (magnitude)");
            {
                const double soll = (double) ref["rolloff"];
                const double faktor = m.rolloffHz > 0 ? m.rolloffHz / soll : 0.0;
                p.wahr (faktor > std::pow (2.0, -1.0 / 6.0) && faktor < std::pow (2.0, 1.0 / 6.0),
                        "Rolloff", "ist " + juce::String (m.rolloffHz, 1) + ", soll "
                        + juce::String (soll, 1) + " (Toleranz 1/6 Okt)");
            }
            p.nahe (m.lowFrac, (double) ref["low_frac"], 0.02, "low_frac (<250 Hz)");
            p.nahe (m.width, (double) ref["width"], 0.02, "Stereo-Width");
            p.nahe (m.corr, (double) ref["corr"], 0.02, "Korrelation");
            {
                const auto band = ref["band_pct"];
                for (int b = 0; b < 8 && b < band.size(); ++b)
                    p.nahe (m.band8Prozent[(size_t) b], (double) band[b][1], 1.0,
                            "Band-% " + band[b][0].toString());
            }

            // ── LTAS-Referenzachse: exakt die analyze-track-Achse ──
            const auto ltas = ref["ltas"];
            std::vector<double> sollDb;
            for (int i = 0; i < ltas["db"].size(); ++i)
                sollDb.push_back (ltas["db"][i].isDouble() || ltas["db"][i].isInt()
                                      ? (double) ltas["db"][i]
                                      : std::numeric_limits<double>::quiet_NaN());
            vergleicheKurve (p, "LTAS Referenzachse", m.ltasReferenzDb, sollDb, 0.5, 2.0);

            // ── Komposit §5.10.1: Kurvenvergleich nur für Breitband-Anteile ──
            if (juce::String (name) == "pink-minus20")
                vergleicheKurve (p, "LTAS Komposit", m.ltasKompositDb, sollDb, 1.2, 3.5);
            else if (juce::String (name) == "resonanz-116hz")
                vergleicheKurve (p, "LTAS Komposit", m.ltasKompositDb, sollDb, 1.2, 3.5,
                                 70.0, 116.0, 1.0 / 6.0, &m.ltasZentrenHz);
            else   // Sinus: Bandpegel sind auflösungsabhängig — struktureller Check
            {
                int peakBand = 0; double peakWert = -1e18;
                for (int b = 0; b < kLtasBaender; ++b)
                    if (! std::isnan (m.ltasKompositDb[(size_t) b])
                        && m.ltasKompositDb[(size_t) b] > peakWert)
                    {
                        peakWert = m.ltasKompositDb[(size_t) b];
                        peakBand = b;
                    }
                p.wahr (std::abs (std::log2 (m.ltasZentrenHz[(size_t) peakBand] / 1000.0)) <= 1.0 / 12.0,
                        "Komposit-Peak bei 1 kHz",
                        "Peak liegt bei " + juce::String (m.ltasZentrenHz[(size_t) peakBand], 1) + " Hz");
            }

            // ── Flatness: nur Größenordnung (Achsen-Differenz dokumentiert) ──
            if (juce::String (name) == "sinus-1k-minus12")
                p.wahr (m.flatness < 1e-2, "Flatness-Ordnung Sinus",
                        "ist " + juce::String (m.flatness, 8));
            if (juce::String (name) == "pink-minus20")
                p.wahr (m.flatness > 0.01 && m.flatness < 0.6, "Flatness-Ordnung Pink",
                        "ist " + juce::String (m.flatness, 5));

            // ── Resonanzkandidaten §5.10.3 ──
            if (juce::String (name) == "sinus-1k-minus12")
            {
                p.wahr (m.resonanzen.size() == 1, "genau 1 Kandidat (1 kHz)",
                        juce::String ((int) m.resonanzen.size())
                        + " Kandidaten — Leakage-Funde in unbespielten Baendern muss das "
                          "Abdeckungs-Gate schlucken (Referenz listet 6!)");
                if (! m.resonanzen.empty())
                {
                    p.wahr (std::abs (std::log2 (m.resonanzen[0].freqHz / 1000.0)) <= 1.0 / 12.0,
                            "Kandidat bei 1 kHz", juce::String (m.resonanzen[0].freqHz, 1) + " Hz");
                    p.wahr (m.resonanzen[0].klasse == ResonanzKlasse::dauerhaft,
                            "Kandidat dauerhaft", "Dauerton muss dauerhaft sein");
                }
            }
            if (juce::String (name) == "pink-minus20")
            {
                p.wahr (m.resonanzen.empty(), "keine Kandidaten in Pink",
                        juce::String ((int) m.resonanzen.size()) + " Kandidaten gefunden");
                // M3a: neutrales Pink darf in KEINER Zone Zeitverlauf-Alarm
                // sammeln (Falsch-Positiv-Riegel des Zeitverlaufs) …
                for (int z = 0; z < kZonenAnzahl; ++z)
                {
                    const auto& zz = m.zonenZeit[(size_t) z];
                    p.wahr (zz.gueltig && zz.ticks >= 15, "Zonen-Ticks belegt",
                            "Zone " + juce::String (z) + ": " + juce::String ((int) zz.ticks));
                    p.wahr (zz.anteil <= 0.10, "Pink ohne Zonen-Alarm",
                            "Zone " + juce::String (z) + ": anteil " + juce::String (zz.anteil, 2));
                }
                // … und die Perzentile müssen belegt und geordnet sein.
                p.wahr (m.perzentileGueltig, "Perzentile belegt", "perzentileGueltig=false");
                int geordnet = 0, belegt = 0;
                for (int b = 0; b < kLtasBaender; ++b)
                {
                    const double p10 = m.perzentilP10[(size_t) b];
                    const double p50 = m.perzentilP50[(size_t) b];
                    const double p95 = m.perzentilP95[(size_t) b];
                    if (! std::isfinite (p10) || ! std::isfinite (p50) || ! std::isfinite (p95))
                        continue;
                    ++belegt;
                    if (p10 <= p50 && p50 <= p95)
                        ++geordnet;
                }
                p.wahr (belegt >= 150, "Perzentile weitgehend belegt",
                        juce::String (belegt) + " Baender");
                p.wahr (geordnet == belegt, "P10 <= P50 <= P95 ueberall",
                        juce::String (belegt - geordnet) + " ungeordnete Baender");
            }
            if (juce::String (name) == "resonanz-116hz")
            {
                p.wahr (m.resonanzen.size() == 1, "genau 1 Kandidat (116 Hz)",
                        juce::String ((int) m.resonanzen.size()) + " Kandidaten");
                if (! m.resonanzen.empty())
                {
                    const auto& k = m.resonanzen[0];
                    p.wahr (std::abs (std::log2 (k.freqHz / 116.0)) <= 1.0 / 6.0,
                            "Kandidat bei 116 Hz (Bassfenster §5.10.1)",
                            juce::String (k.freqHz, 1) + " Hz");
                    p.wahr (k.dbOver >= 15.0 && k.dbOver <= 32.0, "db_over plausibel",
                            juce::String (k.dbOver, 1) + " dB (Referenz 21.7 auf groeberem Raster)");
                    p.wahr (k.klasse == ResonanzKlasse::dauerhaft && k.persistenzAnteil >= 0.5,
                            "dauerhaft + Persistenz >= 50 %",
                            "Persistenz " + juce::String (k.persistenzAnteil, 2));
                }
            }

            // ── Abdeckung §5.10.2 ──
            if (juce::String (name) == "pink-minus20")
            {
                int belastbar = 0;
                for (const auto& a : m.abdeckung)
                    if (a.klasse == AbdeckungsKlasse::belastbar)
                        ++belastbar;
                p.wahr (belastbar >= 24, "Pink weitgehend belastbar",
                        juce::String (belastbar) + " von " + juce::String ((int) m.abdeckung.size()));
            }
            if (juce::String (name) == "sinus-1k-minus12")
            {
                int belastbar = 0;
                for (const auto& a : m.abdeckung)
                    if (a.klasse == AbdeckungsKlasse::belastbar)
                        ++belastbar;
                p.wahr (belastbar <= 4, "Sinus nur um 1 kHz belastbar",
                        juce::String (belastbar) + " belastbare 1/3-Okt-Baender — Leakage-Baender "
                        "duerfen nicht belastbar wirken");
            }
        }

        // ── M3: Graph-Diagnose (der Produktkern) auf JEDEM Signal ──
        diagnosePruefungen (p, name, m);

        std::printf ("%-18s  %3d ok, %d Fehler\n", name, p.ok, p.fehler);
        fehlerGesamt += p.fehler;
        okGesamt += p.ok;
    }

    // ── Adversariale Stereo-/Robustheitsprüfungen (Übergabe Paket B/C/D) ────
    // Die Referenzkurve stammt aus analyze-track auf dem L==R-Pink (Mid ≡
    // Kanalenergie); jede Variante unten hat dazu eine mathematisch zwingende
    // Erwartung. Vor der Kanalenergie-Umstellung lieferte Antiphase „keine
    // Daten" — genau dieser Ausfall wird hier dauerhaft verriegelt.
    {
        std::vector<float> pinkL;
        {
            const auto wav = fixDir.getChildFile ("pink-minus20.wav");
            std::unique_ptr<juce::AudioFormatReader> leser (formate.createReaderFor (wav));
            if (leser != nullptr)
            {
                juce::AudioBuffer<float> alles (2, (int) leser->lengthInSamples);
                leser->read (&alles, 0, (int) leser->lengthInSamples, 0, true, true);
                pinkL.resize ((size_t) alles.getNumSamples());
                for (int i = 0; i < alles.getNumSamples(); ++i)
                    pinkL[(size_t) i] = alles.getSample (0, i);
            }
        }
        const auto ref = signale[juce::Identifier ("pink-minus20")];
        std::vector<double> sollDb;
        for (int i = 0; i < ref["ltas"]["db"].size(); ++i)
            sollDb.push_back (ref["ltas"]["db"][i].isDouble() || ref["ltas"]["db"][i].isInt()
                                  ? (double) ref["ltas"]["db"][i]
                                  : std::numeric_limits<double>::quiet_NaN());
        const double refLufs  = (double) ref["lufs"];
        const double refCrest = (double) ref["crest"];
        const double refTp    = (double) ref["tp_dbtp"];
        constexpr double kHalb = 3.0102999566;   // 10·log10(2): ein Kanal = halbe Leistung

        auto fuettere = [] (AnalyseEngine& e, const std::vector<float>& l, const std::vector<float>& r)
        {
            const int blockFrames = 512;
            std::vector<float> inter ((size_t) blockFrames * 2);
            size_t pos = 0;
            while (pos < l.size())
            {
                const int n = (int) std::min<size_t> ((size_t) blockFrames, l.size() - pos);
                for (int i = 0; i < n; ++i)
                {
                    inter[(size_t) i * 2]     = l[pos + (size_t) i];
                    inter[(size_t) i * 2 + 1] = r[pos + (size_t) i];
                }
                e.verarbeite (inter.data(), n, 2);
                pos += (size_t) n;
            }
            e.auswerten();
        };
        auto lauf = [&] (const char* name, const std::vector<float>& l,
                         const std::vector<float>& r, double engineRate,
                         auto&& pruefe)
        {
            Pruefer p;
            p.signal = name;
            AnalyseEngine engine;
            engine.vorbereiten (engineRate);
            fuettere (engine, l, r);
            pruefe (p, engine.snapshot());
            std::printf ("%-18s  %3d ok, %d Fehler\n", name, p.ok, p.fehler);
            fehlerGesamt += p.fehler;
            okGesamt += p.ok;
        };

        if (pinkL.empty())
        {
            std::printf ("pink-minus20.wav nicht ladbar — Stereo-Pruefungen uebersprungen\n");
            ++fehlerGesamt;
        }
        else
        {
            // a) Antiphase (R = −L): reine Side-Energie. Kanalenergie, LUFS,
            //    Crest und True Peak sind zur L==R-Referenz IDENTISCH — nur
            //    corr/width kippen. Der Mid-Mix hätte hier „keine Daten".
            std::vector<float> neg (pinkL.size());
            for (size_t i = 0; i < pinkL.size(); ++i)
                neg[i] = -pinkL[i];
            lauf ("stereo-antiphase", pinkL, neg, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "Antiphase ist messbereit",
                        "Side-only darf nie als still gelten (alter Mid-Mix-Ausfall)");
                p.nahe (m.aktivSekunden, 20.0, 0.5, "aktive Sekunden");
                vergleicheKurve (p, "LTAS Referenzachse", m.ltasReferenzDb, sollDb, 0.5, 2.0);
                p.nahe (m.lufsIntegriert, refLufs, 0.3, "LUFS wie Referenz");
                p.nahe (m.crestDb, refCrest, 0.3, "Crest wie Referenz");
                p.nahe (m.truePeakDb, refTp, 0.3, "True Peak wie Referenz");
                p.nahe (m.corr, -1.0, 0.001, "Korrelation exakt −1");
                p.wahr (m.width > 1000.0, "Width side-dominant",
                        "ist " + juce::String (m.width, 1));
            });

            // b) Nur links: halbe Kanalenergie ⇒ Kurve und LUFS −3,01 dB,
            //    Crest +3,01 dB (Peak bleibt), True Peak unverändert.
            std::vector<float> stille (pinkL.size(), 0.0f);
            std::vector<double> sollMinus3 (sollDb);
            for (auto& v : sollMinus3)
                v -= kHalb;
            lauf ("stereo-nur-links", pinkL, stille, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "messbereit", "20 s aktiv");
                vergleicheKurve (p, "LTAS = Referenz − 3,01 dB", m.ltasReferenzDb, sollMinus3, 0.5, 2.0);
                p.nahe (m.lufsIntegriert, refLufs - kHalb, 0.3, "LUFS − 3,01");
                p.nahe (m.crestDb, refCrest + kHalb, 0.3, "Crest + 3,01");
                p.nahe (m.truePeakDb, refTp, 0.3, "True Peak unveraendert");
                p.nahe (m.width, 1.0, 0.02, "Width = 1 (einseitig)");
            });

            // c) Nur rechts: exakt symmetrisch — beweist, dass der R-Kanal
            //    wirklich eigenständig gerechnet wird (vorher nur im Mid-Mix).
            lauf ("stereo-nur-rechts", stille, pinkL, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "messbereit", "20 s aktiv");
                vergleicheKurve (p, "LTAS = Referenz − 3,01 dB", m.ltasReferenzDb, sollMinus3, 0.5, 2.0);
                p.nahe (m.lufsIntegriert, refLufs - kHalb, 0.3, "LUFS − 3,01");
            });

            // d) Dekorreliert (R = 5 s rotiertes L): gleiche PSD je Kanal ⇒
            //    Kurve bleibt die Referenz, aber corr ≈ 0 und width ≈ 1 —
            //    der Hall-/breite-Chor-Fall des Users.
            std::vector<float> rot (pinkL.size());
            {
                const size_t versatz = (size_t) (5.0 * rate);
                for (size_t i = 0; i < pinkL.size(); ++i)
                    rot[i] = pinkL[(i + versatz) % pinkL.size()];
            }
            lauf ("stereo-dekorr", pinkL, rot, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "messbereit", "20 s aktiv");
                vergleicheKurve (p, "LTAS wie Referenz", m.ltasReferenzDb, sollDb, 1.0, 3.0);
                p.nahe (m.lufsIntegriert, refLufs, 0.4, "LUFS wie Referenz");
                p.melde (std::abs (m.corr) < 0.25, "Korrelation nahe 0",
                         "ist " + juce::String (m.corr, 3));
                p.nahe (m.width, 1.0, 0.15, "Width nahe 1");
            });

            // e) NaN-Recovery (Paket C §8.2): Sekunde 5–6 komplett NaN. Die
            //    Analyse ersetzt exakt 2·48000 Samples durch Stille, bleibt
            //    messbereit und liefert ausschließlich endliche Werte.
            std::vector<float> mitNaN (pinkL);
            {
                const size_t von = (size_t) (5.0 * rate), bis = (size_t) (6.0 * rate);
                for (size_t i = von; i < bis && i < mitNaN.size(); ++i)
                    mitNaN[i] = std::numeric_limits<float>::quiet_NaN();
            }
            lauf ("nan-recovery", mitNaN, mitNaN, rate, [&] (Pruefer& p, const MessSnapshot& m)
            {
                p.wahr (m.zustand == MessZustand::messbereit, "trotz NaN messbereit",
                        "1 s NaN darf 19 s Musik nicht entwerten");
                p.wahr (m.nanErsetzt == (juce::uint64) (2.0 * rate), "Ersetzt-Zaehler exakt",
                        "ist " + juce::String ((juce::int64) m.nanErsetzt)
                        + ", soll " + juce::String ((juce::int64) (2.0 * rate)));
                p.wahr (std::isfinite (m.lufsIntegriert) && m.lufsGueltig, "LUFS endlich+gueltig",
                        "NaN hat das Gating vergiftet");
                p.nahe (m.lufsIntegriert, refLufs, 0.5, "LUFS nahe Referenz");
                p.wahr (std::isfinite (m.truePeakDb), "True Peak endlich", "vergiftet");
                p.wahr (std::isfinite (m.crestDb), "Crest endlich", "vergiftet");
                vergleicheKurve (p, "LTAS erholt", m.ltasReferenzDb, sollDb, 1.0, 3.5);
            });

            // f) Nyquist-Kappe (Paket D): bei fs=32 kHz endet der messbare
            //    Bereich bei 0,95·16 kHz — darüber MUSS die Kurve NaN sein
            //    (vorher wurde der letzte Randwert fortgeschrieben).
            lauf ("nyquist-32k", pinkL, pinkL, 32000.0, [&] (Pruefer& p, const MessSnapshot& m)
            {
                const double grenzeHz = 0.95 * 16000.0;
                int erstesNaNBand = kLtasBaender;
                for (int b = 0; b < kLtasBaender; ++b)
                    if (m.ltasZentrenHz[(size_t) b] > grenzeHz)
                    {
                        erstesNaNBand = b;
                        break;
                    }
                p.wahr (erstesNaNBand < kLtasBaender, "Kappe liegt im Raster",
                        "0,95·Nyquist muss unter 18 kHz liegen");
                bool untenOk = true, obenOk = true;
                for (int b = 0; b < kLtasBaender; ++b)
                {
                    const bool nan = std::isnan (m.ltasKompositDb[(size_t) b]);
                    if (b < erstesNaNBand && nan)  untenOk = false;
                    if (b >= erstesNaNBand && ! nan) obenOk = false;
                }
                p.wahr (m.ltasGueltig, "LTAS gueltig", "messbarer Bereich muss gelten");
                p.wahr (untenOk, "unterhalb der Kappe keine Luecken",
                        "NaN im messbaren Bereich");
                p.wahr (obenOk, "oberhalb der Kappe nur NaN",
                        "fortgeschriebener Randwert = erfundene Evidenz");
                for (const auto& k : m.resonanzen)
                    p.wahr (k.freqHz <= grenzeHz, "kein Kandidat oberhalb der Kappe",
                            juce::String (k.freqHz, 1) + " Hz");
            });

            // g) M3a auswertenLeicht (FPS-Fix): der Leichtpfad publiziert
            //    Live-Kurve/Meter/Zustand OHNE Schwerlauf — und im Leerlauf
            //    publiziert er NICHTS (Revision steht).
            {
                Pruefer p;
                p.signal = "leicht-publikation";
                AnalyseEngine engine;
                engine.vorbereiten (rate);
                const int blockFrames = 512;
                std::vector<float> inter ((size_t) blockFrames * 2);
                auto fuettereBereich = [&] (size_t von, size_t bis)
                {
                    size_t pos = von;
                    while (pos < bis)
                    {
                        const int n = (int) std::min<size_t> ((size_t) blockFrames, bis - pos);
                        for (int i = 0; i < n; ++i)
                        {
                            inter[(size_t) i * 2]     = pinkL[pos + (size_t) i];
                            inter[(size_t) i * 2 + 1] = pinkL[pos + (size_t) i];
                        }
                        engine.verarbeite (inter.data(), n, 2);
                        pos += (size_t) n;
                    }
                };
                fuettereBereich (0, (size_t) (5.0 * rate));
                engine.auswertenLeicht();
                const auto m1 = engine.snapshot();
                p.wahr (m1.revision > 0, "Leichtpfad publiziert", "Revision 0");
                p.wahr (m1.zustand == MessZustand::sammelt, "Zustand sammelt (5 s)",
                        "zustand falsch");
                p.wahr (m1.lufsShortGueltig && std::isfinite (m1.lufsShort),
                        "Kurz-LUFS ohne Schwerlauf", "fehlt");
                p.wahr (std::isfinite (m1.truePeakDb) && m1.truePeakDb > -100.0,
                        "True Peak ohne Schwerlauf", juce::String (m1.truePeakDb, 1));
                int liveBaender = 0;
                for (const double v : m1.ltasLiveDb)
                    if (std::isfinite (v))
                        ++liveBaender;
                p.wahr (liveBaender > 100, "Live-Kurve belegt",
                        juce::String (liveBaender) + " Baender");
                p.wahr (! m1.ltasGueltig, "Komposit bleibt dem Schwerlauf vorbehalten",
                        "ltasGueltig ohne auswerten()");
                engine.auswertenLeicht();   // keine neuen Samples
                p.wahr (engine.snapshot().revision == m1.revision,
                        "Leerlauf publiziert nicht", "Revision lief weiter");
                fuettereBereich ((size_t) (5.0 * rate), (size_t) (6.0 * rate));
                engine.auswertenLeicht();
                p.wahr (engine.snapshot().revision > m1.revision,
                        "neue Daten => neue Revision", "Revision steht");
                std::printf ("%-18s  %3d ok, %d Fehler\n", p.signal.toRawUTF8(), p.ok, p.fehler);
                fehlerGesamt += p.fehler;
                okGesamt += p.ok;
            }

            // h) M3a Konvergenz („Kurve steht"): zwei Schwerläufe im Abstand
            //    von 10 s Aktivzeit — stationäres Pink muss als „steht" lesen.
            {
                Pruefer p;
                p.signal = "konvergenz";
                AnalyseEngine engine;
                engine.vorbereiten (rate);
                const int blockFrames = 512;
                std::vector<float> inter ((size_t) blockFrames * 2);
                auto fuettereBereich = [&] (size_t von, size_t bis)
                {
                    size_t pos = von;
                    while (pos < bis)
                    {
                        const int n = (int) std::min<size_t> ((size_t) blockFrames, bis - pos);
                        for (int i = 0; i < n; ++i)
                        {
                            inter[(size_t) i * 2]     = pinkL[pos + (size_t) i];
                            inter[(size_t) i * 2 + 1] = pinkL[pos + (size_t) i];
                        }
                        engine.verarbeite (inter.data(), n, 2);
                        pos += (size_t) n;
                    }
                };
                fuettereBereich (0, pinkL.size() / 2);
                engine.auswerten();
                p.wahr (! engine.snapshot().konvergenzGueltig,
                        "erstes Fenster: noch kein Konvergenzwert", "zu frueh gueltig");
                fuettereBereich (pinkL.size() / 2, pinkL.size());
                engine.auswerten();
                const auto m2 = engine.snapshot();
                p.wahr (m2.konvergenzGueltig, "Konvergenz nach zweitem Fenster", "fehlt");
                p.wahr (std::isfinite (m2.konvergenzMeanDb)
                            && m2.konvergenzMeanDb >= 0.0 && m2.konvergenzMeanDb < 1.0,
                        "mittlere Bewegung klein (stationaeres Pink)",
                        juce::String (m2.konvergenzMeanDb, 3) + " dB");
                p.wahr (std::isfinite (m2.konvergenzMaxDb) && m2.konvergenzMaxDb < 6.0,
                        "maximale Bewegung plausibel",
                        juce::String (m2.konvergenzMaxDb, 2) + " dB");
                std::printf ("%-18s  %3d ok, %d Fehler\n", p.signal.toRawUTF8(), p.ok, p.fehler);
                fehlerGesamt += p.fehler;
                okGesamt += p.ok;
            }
        }
    }

    std::printf ("\n%s — %d Pruefungen ok, %d Fehler (metrics %s · diagnose %s)\n",
                 fehlerGesamt == 0 ? "GOLDEN OK" : "GOLDEN FEHLGESCHLAGEN",
                 okGesamt, fehlerGesamt, kMetricsVersion, kDiagnoseVersion);
    return fehlerGesamt == 0 ? 0 : 1;
}
