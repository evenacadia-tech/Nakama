// SONDE-008 (S10-11) — EBU-Golden der fixed-memory Loudness (Kanon B9).
//
// GATE-TEXT des Tickets (Entwurf §65): "keine Teilbloecke/unbegrenzten
// Vektoren; RT-/EBU-Goldens gruen." Dieses Ziel beantwortet die zweite Haelfte.
//
// DIE REFERENZ IST DIE AUSGEBAUTE RECHNUNG. `Referenz` unten ist Zeile fuer
// Zeile das, was bis zum 23.08.2026 in `AnalyseEngine::finalisiereLoudness` und
// `fuelleBasis` stand: ein unbegrenzt wachsender `std::vector<double> kZellen`
// und ein Zweitdurchgang darueber. Genau das ersetzt dieses Ticket - und genau
// deshalb ist es die einzige Referenz, die die Frage beantwortet. Ein Golden,
// der den neuen Weg gegen sich selbst rechnet, waere eine Tautologie (dieselbe
// Lehre wie bei EqCopIdentityTest, das die CIDs auf ZWEI Wegen bildet).
//
// WAS GEMESSEN WIRD:
//   A  Bitgleichheit des Kurzzeit-LUFS (3 s) - nicht "innerhalb Toleranz".
//   B  Integrierte Lautheit gegen die Referenz, Toleranz ±0,1 LU (Entwurf §49,
//      Tabelle "Loudness-/True-Peak-Referenz").
//   C  Ein ADVERSARIALER Sweep, der die Blockverteilung absichtlich so legt,
//      dass ein grosser Teil der Bloecke im Grenzbin des relativen Gates
//      landet - der einzige Ort, an dem der Histogrammweg ueberhaupt vom
//      Vektorweg abweichen KANN. Ohne diesen Sweep wuerde der Golden die
//      Quantisierung nie zu Gesicht bekommen und trotzdem gruen melden.
//   D  Die selbstgemeldete Schranke `unsicherheitLu()` muss den tatsaechlichen
//      Fehler DECKEN. Eine Schranke, die kleiner sein kann als der Fehler, ist
//      schlimmer als keine.
//   E  Fixed memory, VORGEFUEHRT: eine Million Zellen, Allokationszaehler auf
//      dem Weg. Zum Vergleich laeuft dieselbe Million durch die Referenz - die
//      alloziert, und die Zahl steht im Protokoll.
//   F  Numerische Raender: NaN/Inf-Zellen, Stille unter dem absoluten Gate,
//      exakt -70,0 LUFS, leerer Zustand.
//   G  DAS OBERBAND (Nacharbeit T2-1, 23.08.2026). Der Bereich ueber dem
//      Feingitter, mit MEHR ALS EINEM PEGEL. Bis zur Nacharbeit fuellte §F ihn
//      nur mit einem einzigen (200x z=1e6) - und mit einem Pegel ist der
//      Mittelwert eines Eimers zwangslaeufig die Blocklautheit jedes seiner
//      Bloecke, der Fall konnte konstruktiv nicht scheitern. Der T2-Pruefer hat
//      genau dort das Gegenbeispiel gefunden: zwei Pegel, Gamma_r dazwischen,
//      2,918 LU Fehler bei 0,000000000 LU gemeldeter Schranke. G faehrt diesen
//      Korpus, den Sweep DARUEBER (dieselbe Adversarialitaet wie C, nur oberhalb
//      des Gitters), die Naht zwischen beiden Aufloesungen und die
//      Vollstaendigkeit des Rasters.
//
// Exit 0 nur bei "LOUDNESS-GOLDEN OK".
#include "analysis/LoudnessAccumulator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>
#include <string>
#include <vector>

using nakama::analyse::LoudnessAccumulator;

//==============================================================================
namespace
{
    bool          zaehleAllokationen = false;
    std::uint64_t allokationen       = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
/** Die AUSGEBAUTE Rechnung, unveraendert uebernommen aus
    `AnalyseEngine::finalisiereLoudness` / `fuelleBasis` (Stand `e330052`).
    Absichtlich NICHT aufgeraeumt - sie soll das sein, was sie war. */
struct Referenz
{
    std::vector<double> kZellen;
    double sr { 0.0 };

    void zelle (double e) { kZellen.push_back (e); }

    static double lautheit (double z) { return -0.691 + 10.0 * std::log10 (z + 1e-30); }

    bool integriert (double& heraus) const
    {
        const std::size_t nZellen = kZellen.size();
        if (nZellen < 4) return false;

        std::vector<double> bloecke;
        bloecke.reserve (nZellen - 3);
        const double norm = 0.4 * sr;
        for (std::size_t j = 3; j < nZellen; ++j)
            bloecke.push_back ((kZellen[j - 3] + kZellen[j - 2] + kZellen[j - 1] + kZellen[j]) / norm);

        double summe1 = 0.0; std::uint64_t n1 = 0;
        for (const double z : bloecke)
            if (lautheit (z) >= -70.0)              // absolutes Gate (pyloudnorm: >=)
            { summe1 += z; ++n1; }
        if (n1 == 0) return false;

        const double gammaR = lautheit (summe1 / (double) n1) - 10.0;
        double summe2 = 0.0; std::uint64_t n2 = 0;
        for (const double z : bloecke)
        {
            const double lj = lautheit (z);
            if (lj > gammaR && lj > -70.0)          // relatives Gate (pyloudnorm: >)
            { summe2 += z; ++n2; }
        }
        if (n2 == 0) return false;
        heraus = lautheit (summe2 / (double) n2);
        return true;
    }

    bool kurz (double& heraus) const
    {
        if (kZellen.size() < 30) return false;
        double e = 0.0;
        for (std::size_t j = kZellen.size() - 30; j < kZellen.size(); ++j)
            e += kZellen[j];
        heraus = -0.691 + 10.0 * std::log10 (e / (3.0 * sr) + 1e-30);
        return true;
    }
};

//==============================================================================
namespace
{
    int fehler = 0;
    int geprueft = 0;

    void pruefe (bool ok, const std::string& was, const std::string& detail = {})
    {
        ++geprueft;
        std::cout << (ok ? "  ok      " : "  FEHLER  ") << was;
        if (! detail.empty()) std::cout << "  [" << detail << "]";
        std::cout << std::endl;
        if (! ok) ++fehler;
    }

    std::string zahl (double v, int stellen = 6)
    {
        std::ostringstream s;
        s << std::fixed << std::setprecision (stellen) << v;
        return s.str();
    }

    constexpr double kFs        = 48000.0;
    constexpr double kZelleSek  = 0.1;
    /** Entwurf §49: "Loudness-/True-Peak-Referenz | ±0,1 LU / ±0,1 dB". */
    constexpr double kToleranz  = 0.1;

    /** Zellenergie fuer eine gewuenschte BLOCKleistung z (4 gleiche Zellen). */
    double zelleFuer (double z) { return z * kZelleSek * kFs; }

    struct Ergebnis
    {
        bool   akkuGueltig { false }, refGueltig { false };
        double akku { 0.0 }, ref { 0.0 };
        double schranke { 0.0 };
        bool   kurzAkkuGueltig { false }, kurzRefGueltig { false };
        double kurzAkku { 0.0 }, kurzRef { 0.0 };
        std::uint64_t ueberGitter { 0 }, ueberOberband { 0 };
    };

    Ergebnis fahre (const std::vector<double>& zellen)
    {
        LoudnessAccumulator akku;
        akku.vorbereiten (kFs);
        Referenz ref;
        ref.sr = kFs;
        for (const double e : zellen) { akku.zelle (e); ref.zelle (e); }

        Ergebnis r;
        r.akkuGueltig = akku.integriert (r.akku);
        r.refGueltig  = ref.integriert (r.ref);
        r.schranke    = akku.unsicherheitLu();
        r.kurzAkkuGueltig = akku.kurz (r.kurzAkku);
        r.kurzRefGueltig  = ref.kurz (r.kurzRef);
        r.ueberGitter     = akku.bloeckeUeberGitter();
        r.ueberOberband   = akku.bloeckeUeberOberband();
        return r;
    }

    /** Vergleicht einen Korpus und meldet drei Dinge: Gueltigkeit, Toleranz und
        dass die selbstgemeldete Schranke den Fehler deckt. */
    double vergleiche (const std::string& name, const std::vector<double>& zellen)
    {
        const Ergebnis r = fahre (zellen);
        pruefe (r.akkuGueltig == r.refGueltig,
                name + ": Gueltigkeit wie die Referenz",
                std::string ("akku=") + (r.akkuGueltig ? "1" : "0")
                    + " ref=" + (r.refGueltig ? "1" : "0"));
        // Vollstaendigkeit des Rasters, bei JEDEM Korpus: faellt ein Block durch,
        // meldet `unsicherheitLu()` unendlich und die Schranke unten waere
        // wertlos. Die Zeile kostet nichts und haelt die Voraussetzung fest,
        // von der der ganze Beweis lebt.
        pruefe (r.ueberOberband == 0, name + ": kein Block faellt durch das Raster",
                std::to_string (r.ueberOberband));
        if (! r.refGueltig)
            return 0.0;

        const double d = std::abs (r.akku - r.ref);
        pruefe (d <= kToleranz, name + ": LUFS-I innerhalb ±0,1 LU",
                "akku=" + zahl (r.akku, 6) + " ref=" + zahl (r.ref, 6) + " d=" + zahl (d, 9));
        pruefe (d <= r.schranke + 1e-12,
                name + ": unsicherheitLu() deckt den Fehler",
                "d=" + zahl (d, 9) + " schranke=" + zahl (r.schranke, 9));
        // Staerker als der Gate-Text verlangt: liegt KEIN Block im Grenzbin,
        // gibt es keine Quantisierung - dann darf nur noch die Rundung des
        // double uebrig sein. Eine Toleranz, die auch OHNE Quantisierung
        // gebraucht wuerde, verstecke einen Rechenfehler.
        //
        // ⚠️ Und zwar Rundung, nicht Bitgleichheit: die erste Fassung verlangte
        // hier `d == 0` und fiel prompt mit 5,8e-13 LU. Der Grund ist die
        // SUMMATIONSREIHENFOLGE - der Akku summiert binweise (kompensiert), die
        // Referenz laufend ueber die Bloecke. Beim Kurz-LUFS bleibt die
        // Reihenfolge erhalten, dort ist Bitgleichheit erreichbar und wird
        // unten auch gefordert; bei der integrierten Lautheit ist sie es nicht.
        if (r.schranke == 0.0)
            pruefe (d <= 1.0e-9, name + ": ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)",
                    "d=" + zahl (d, 15));

        if (r.kurzRefGueltig || r.kurzAkkuGueltig)
        {
            pruefe (r.kurzAkkuGueltig == r.kurzRefGueltig, name + ": Kurz-LUFS Gueltigkeit gleich");
            pruefe (r.kurzAkku == r.kurzRef, name + ": Kurz-LUFS BITGLEICH",
                    "akku=" + zahl (r.kurzAkku, 15) + " ref=" + zahl (r.kurzRef, 15));
        }
        return d;
    }

    std::uint32_t lcg = 0x2545f491u;
    double zufall01()
    {
        lcg = lcg * 1664525u + 1013904223u;
        return (double) (lcg >> 8) / (double) 0x01000000;
    }
}

//==============================================================================
int main()
{
    std::cout << "== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==" << std::endl;
    std::cout << "Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang)."
              << std::endl;
    std::cout << "Toleranz: +-" << zahl (kToleranz, 1) << " LU (Entwurf §49)." << std::endl;
    // Die gedruckten Kanten sind AUSGERECHNET, nicht abgelesen: `kBinOben` ist
    // die Rechengroesse fuer die Binzahl, die oberste Kante liegt bei -70 +
    // 10001*0,01 = +30,01 (T2-Berichtigung, Manifest §8.8).
    std::cout << "Feingitter: " << LoudnessAccumulator::kBins << " Bins a "
              << zahl (LoudnessAccumulator::kBinBreite, 2) << " LU ueber ["
              << zahl (LoudnessAccumulator::kBinUnten, 2) << ", "
              << zahl (LoudnessAccumulator::kFeinObersteKante, 2) << ") LUFS." << std::endl;
    std::cout << "Oberband:   " << LoudnessAccumulator::kObenBins << " Bins a "
              << zahl (LoudnessAccumulator::kObenBreite, 2) << " LU ueber ["
              << zahl (LoudnessAccumulator::kObenUnten, 2) << ", "
              << zahl (LoudnessAccumulator::kObenUnten
                       + LoudnessAccumulator::kObenBins * LoudnessAccumulator::kObenBreite, 2)
              << ") LUFS - lautheit(DBL_MAX) = 3081,86." << std::endl;
    std::cout << std::endl;

    double schlimmster = 0.0;

    // ── A · Konstantes Material: hier DARF nichts abweichen ────────────────
    std::cout << "== A - konstante Pegel (kein Block am Gate) ==" << std::endl;
    {
        for (const double z : { 1.0, 0.01, 1e-5, 100.0 })
        {
            std::vector<double> zellen (600, zelleFuer (z));
            schlimmster = std::max (schlimmster,
                                    vergleiche ("konstant z=" + zahl (z, 5), zellen));
        }
    }

    // ── B · Rampe und Rauschen ─────────────────────────────────────────────
    std::cout << "== B - Rampe, Rauschen, lange Laufzeit ==" << std::endl;
    {
        std::vector<double> rampe;
        for (int i = 0; i < 900; ++i)                    // 90 s
            rampe.push_back (zelleFuer (0.001 * std::pow (10.0, 3.0 * i / 900.0)));
        schlimmster = std::max (schlimmster, vergleiche ("Rampe 30 dB ueber 90 s", rampe));

        std::vector<double> rausch;
        for (int i = 0; i < 3000; ++i)                   // 300 s
            rausch.push_back (zelleFuer (std::pow (10.0, -4.0 + 4.0 * zufall01())));
        schlimmster = std::max (schlimmster, vergleiche ("Pseudo-Zufall 300 s", rausch));

        std::vector<double> lang;
        for (int i = 0; i < 36000; ++i)                  // 1 h
            lang.push_back (zelleFuer (0.05 * (0.2 + zufall01())));
        schlimmster = std::max (schlimmster, vergleiche ("eine Stunde (36 000 Zellen)", lang));
    }

    // ── C · Musik mit Pausen: viele Bloecke unter dem absoluten Gate ───────
    std::cout << "== C - Pausen unter dem absoluten Gate (-70 LUFS) ==" << std::endl;
    {
        std::vector<double> zellen;
        for (int takt = 0; takt < 40; ++takt)
        {
            for (int i = 0; i < 20; ++i) zellen.push_back (zelleFuer (0.2));
            for (int i = 0; i < 20; ++i) zellen.push_back (zelleFuer (1e-12));   // ~ -120 LUFS
        }
        schlimmster = std::max (schlimmster, vergleiche ("Musik + echte Stille", zellen));
    }

    // ── D · Der adversariale Sweep: Cluster GENAU auf dem relativen Gate ───
    // Aufbau: zwei konstante Laeufe. Ist z_laut = z_leise * (10*(n_l+n_q) - n_q)
    // / n_l, liegt der leise Lauf exakt 10 LU unter dem Mittel - also exakt auf
    // Gamma_r. Der Sweep faehrt 201 Varianten um diesen Punkt herum, damit der
    // Grenzbin garantiert getroffen wird (und zwar von 1000 Bloecken auf
    // einmal, nicht von einem einzelnen).
    std::cout << "== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==" << std::endl;
    {
        constexpr int nL = 1000, nQ = 1000;
        const double zQ = 1.0;
        const double zLexakt = zQ * (10.0 * (nL + nQ) - nQ) / nL;      // = 19,0

        double maxD = 0.0, maxSchranke = 0.0, beiFaktor = 0.0;
        double maxRefSprung = 0.0, letzteRef = 0.0;
        int gedeckt = 0, laeufe = 0, sicherUndRichtig = 0, sicherLaeufe = 0;
        for (int k = -100; k <= 100; ++k)
        {
            const double faktor = 1.0 + (double) k * 1.0e-5;
            std::vector<double> zellen;
            zellen.reserve ((std::size_t) (nL + nQ));
            for (int i = 0; i < nL; ++i) zellen.push_back (zelleFuer (zLexakt * faktor));
            for (int i = 0; i < nQ; ++i) zellen.push_back (zelleFuer (zQ));

            const Ergebnis r = fahre (zellen);
            if (! (r.akkuGueltig && r.refGueltig)) { pruefe (false, "Sweep: beide gueltig"); break; }
            const double d = std::abs (r.akku - r.ref);
            if (laeufe > 0) maxRefSprung = std::max (maxRefSprung, std::abs (r.ref - letzteRef));
            letzteRef = r.ref;
            ++laeufe;
            if (d <= r.schranke + 1e-12) ++gedeckt;
            if (r.schranke <= kToleranz) { ++sicherLaeufe; if (d <= kToleranz) ++sicherUndRichtig; }
            if (d > maxD) { maxD = d; beiFaktor = faktor; }
            maxSchranke = std::max (maxSchranke, r.schranke);
        }
        std::cout << "  Sweep: " << laeufe << " Laeufe, groesste Abweichung "
                  << zahl (maxD, 6) << " LU bei Faktor " << zahl (beiFaktor, 6)
                  << ", groesste gemeldete Schranke " << zahl (maxSchranke, 6) << " LU"
                  << std::endl;
        std::cout << "  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen "
                  << "(Eingang aendert sich um 1e-5): " << zahl (maxRefSprung, 6) << " LU"
                  << std::endl;
        pruefe (laeufe == 201, "Sweep vollstaendig gefahren", std::to_string (laeufe) + "/201");
        pruefe (maxSchranke > 0.0,
                "Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)",
                "sonst pruefte D nichts; schranke=" + zahl (maxSchranke, 6));
        pruefe (gedeckt == laeufe, "die selbstgemeldete Schranke deckt JEDEN Lauf",
                std::to_string (gedeckt) + "/" + std::to_string (laeufe));
        // DAS ist die belastbare Zusage: wo der Akku sagt, er sei sicher, ist er
        // es auch. Eine Schranke, die man ignorieren muesste, waere keine.
        pruefe (sicherLaeufe > 0 && sicherUndRichtig == sicherLaeufe,
                "wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU",
                std::to_string (sicherUndRichtig) + "/" + std::to_string (sicherLaeufe));
        // Und das ist die Einordnung der uebrigen Laeufe - gemessen, nicht behauptet:
        // die Referenz selbst springt hier um denselben Betrag, wenn sich der
        // Eingang um 1e-5 aendert. Das relative Gate von BS.1770 ist an dieser
        // Stelle UNSTETIG (1000 Bloecke kippen gemeinsam die Gateseite); auf
        // welcher Seite der Kante man landet, ist keine Genauigkeitsfrage.
        // Deshalb misst die Pruefung gegen die Unstetigkeit, nicht gegen 0,1 LU.
        pruefe (maxD <= maxRefSprung + kToleranz,
                "die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)",
                "maxD=" + zahl (maxD, 6) + " refSprung=" + zahl (maxRefSprung, 6));
        std::cout << "  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - "
                  << "er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)" << std::endl;
    }

    // ── E · Fixed memory, vorgefuehrt ──────────────────────────────────────
    std::cout << "== E - fixed memory: eine Million Zellen ==" << std::endl;
    {
        LoudnessAccumulator akku;
        akku.vorbereiten (kFs);                     // die EINE erlaubte Allokation
        allokationen = 0;
        zaehleAllokationen = true;
        for (int i = 0; i < 1'000'000; ++i)
            akku.zelle (zelleFuer (0.05 + 0.04 * (double) (i % 7)));
        double x = 0.0;
        const bool g = akku.integriert (x);
        const double s = akku.unsicherheitLu();
        zaehleAllokationen = false;
        const std::uint64_t akkuAllok = allokationen;

        Referenz ref;
        ref.sr = kFs;
        allokationen = 0;
        zaehleAllokationen = true;
        for (int i = 0; i < 1'000'000; ++i)
            ref.zelle (zelleFuer (0.05 + 0.04 * (double) (i % 7)));
        double y = 0.0;
        const bool gr = ref.integriert (y);
        zaehleAllokationen = false;
        const std::uint64_t refAllok = allokationen;

        std::cout << "  Akku:     " << akkuAllok << " Allokationen, LUFS-I " << zahl (x, 6)
                  << ", Schranke " << zahl (s, 9) << " LU" << std::endl;
        std::cout << "  Referenz: " << refAllok << " Allokationen, LUFS-I " << zahl (y, 6)
                  << std::endl;
        pruefe (akkuAllok == 0, "1 000 000 Zellen: 0 Allokationen im Akku",
                std::to_string (akkuAllok));
        pruefe (refAllok > 0, "Gegenprobe: die alte Rechnung alloziert sehr wohl",
                std::to_string (refAllok));
        pruefe (g && gr && std::abs (x - y) <= kToleranz,
                "1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU", "d=" + zahl (std::abs (x - y), 9));
        std::cout << "  Fester Bedarf laut speicherBytes(): "
                  << LoudnessAccumulator::speicherBytes() << " Byte - haengt an keiner "
                  << "Laufzeitgroesse." << std::endl;
        pruefe (LoudnessAccumulator::speicherBytes()
                    == (std::size_t) LoudnessAccumulator::kBinsGesamt * (sizeof (double) + sizeof (std::uint32_t))
                     + (std::size_t) LoudnessAccumulator::kZellenRing * sizeof (double),
                "speicherBytes() ist eine reine Compile-Time-Groesse");
    }

    // ── F · Numerische Raender ─────────────────────────────────────────────
    std::cout << "== F - numerische Raender ==" << std::endl;
    {
        {   // leer
            LoudnessAccumulator a; a.vorbereiten (kFs);
            double x = 0.0;
            pruefe (! a.integriert (x), "leer: kein LUFS-I (null, nie eine Zahl)");
            pruefe (! a.kurz (x), "leer: kein Kurz-LUFS");
            pruefe (a.unsicherheitLu() == 0.0, "leer: Schranke 0");
        }
        {   // alles unter dem absoluten Gate
            std::vector<double> zellen (200, zelleFuer (1e-12));
            const Ergebnis r = fahre (zellen);
            pruefe (! r.akkuGueltig && ! r.refGueltig,
                    "alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)");
        }
        {   // exakt -70,0 LUFS: absolutes Gate nimmt (">="), relatives verwirft (">")
            const double zGate = std::pow (10.0, (-70.0 + 0.691) / 10.0);
            std::vector<double> zellen (200, zelleFuer (zGate));
            const Ergebnis r = fahre (zellen);
            pruefe (r.akkuGueltig == r.refGueltig,
                    "l_j == -70,0 exakt: Gueltigkeit wie die Referenz",
                    std::string ("akku=") + (r.akkuGueltig ? "1" : "0")
                        + " ref=" + (r.refGueltig ? "1" : "0"));
            if (r.akkuGueltig && r.refGueltig)
                pruefe (std::abs (r.akku - r.ref) <= kToleranz, "l_j == -70,0: Wert wie die Referenz");
        }
        {   // NaN/Inf-Zellen: GEZAEHLT, nie stillschweigend als 0 verbucht
            LoudnessAccumulator a; a.vorbereiten (kFs);
            for (int i = 0; i < 100; ++i) a.zelle (zelleFuer (0.2));
            a.zelle (std::numeric_limits<double>::quiet_NaN());
            a.zelle (std::numeric_limits<double>::infinity());
            a.zelle (-std::numeric_limits<double>::infinity());
            for (int i = 0; i < 100; ++i) a.zelle (zelleFuer (0.2));
            double x = 0.0;
            const bool g = a.integriert (x);
            pruefe (a.bloeckeNichtEndlich() > 0,
                    "NaN/Inf-Zellen sind gezaehlt", std::to_string (a.bloeckeNichtEndlich()));
            pruefe (g && std::isfinite (x),
                    "NaN/Inf vergiften das Gating nicht", zahl (x, 6));
        }
        {   // Zellen ueber dem Gitter (extrem laut) - Gate kann das nicht faelschen
            std::vector<double> zellen (200, zelleFuer (1e6));      // ~ +59 LUFS
            LoudnessAccumulator a; a.vorbereiten (kFs);
            Referenz ref; ref.sr = kFs;
            for (const double e : zellen) { a.zelle (e); ref.zelle (e); }
            // Erst rechnen, DANN melden: die Reihenfolge der Argumente von
            // `pruefe` ist unspezifiziert, und die erste Fassung baute den
            // Meldetext, bevor `integriert()` x gesetzt hatte - sie meldete
            // gruen und druckte dazu „akku=0.000000". Ein Text, der etwas
            // anderes sagt als die Pruefung, ist genau der Fall aus der
            // T1-Liste („Luegt der UI-Text?").
            double x = 0.0, y = 0.0;
            const bool gA = a.integriert (x);
            const bool gR = ref.integriert (y);
            pruefe (gA && gR && std::abs (x - y) <= kToleranz,
                    "ueber dem Gitter: EIN Pegel bleibt wertgleich",
                    "akku=" + zahl (x, 6) + " ref=" + zahl (y, 6)
                        + " ueberGitter=" + std::to_string (a.bloeckeUeberGitter()));
            // Gegenprobe: dieser Fall MUSS das Oberband wirklich erreichen -
            // sonst prueft er nichts ueber das Oberband, sondern nur ueber das
            // Feingitter. Genau diese Zeile fehlte, als §F noch der einzige
            // Beleg fuer den Bereich war (T2-1).
            pruefe (a.bloeckeUeberGitter() > 0,
                    "Gegenprobe: der Korpus liegt wirklich im Oberband",
                    std::to_string (a.bloeckeUeberGitter()));
            pruefe (a.bloeckeUeberOberband() == 0, "und kein Block faellt durch das Raster");
        }
        {   // Blockzahl: Zellen minus 3 (Hop 1, Fenster 4)
            LoudnessAccumulator a; a.vorbereiten (kFs);
            for (int i = 0; i < 500; ++i) a.zelle (zelleFuer (0.2));
            pruefe (a.zellenGesamt() == 500 && a.bloeckeGesamt() == 497,
                    "500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)",
                    std::to_string (a.zellenGesamt()) + "/" + std::to_string (a.bloeckeGesamt()));
        }
        {   // Gegenpfad fuellen<->leeren: zuruecksetzen() ohne Allokation
            LoudnessAccumulator a; a.vorbereiten (kFs);
            for (int i = 0; i < 1000; ++i) a.zelle (zelleFuer (0.2));
            allokationen = 0; zaehleAllokationen = true;
            a.zuruecksetzen();
            zaehleAllokationen = false;
            double x = 0.0;
            pruefe (allokationen == 0, "zuruecksetzen(): 0 Allokationen",
                    std::to_string (allokationen));
            pruefe (! a.integriert (x) && ! a.kurz (x) && a.zellenGesamt() == 0,
                    "zuruecksetzen(): der Akku ist danach wirklich leer");
        }
    }

    // ── G · Das Oberband: MEHR ALS EIN PEGEL ueber dem Feingitter ──────────
    // Nacharbeit T2-1. Bis hierher war der Bereich ueber dem Gitter nur mit
    // EINEM Pegel belegt (§F) - und mit einem Pegel kann eine Zuordnung "ganzer
    // Bereich an seinem Mittelwert" konstruktiv nicht falsch sein.
    std::cout << "== G - Oberband: zwei Pegel ueber dem Feingitter ==" << std::endl;
    {
        // G1 · Das Gegenbeispiel des Pruefers, Zeile fuer Zeile (Manifest §8.9).
        // Vorher: Akku 46,384702 gegen Referenz 49,302611 - 2,917909 LU
        // Abweichung bei 0,000000000 LU gemeldeter Schranke.
        std::vector<double> zwei;
        zwei.reserve (2000);
        for (int i = 0; i < 1000; ++i) zwei.push_back (zelleFuer (1.0e5));   // ~ +49,31 LUFS
        for (int i = 0; i < 1000; ++i) zwei.push_back (zelleFuer (2.0e3));   // ~ +32,32 LUFS
        const Ergebnis g1 = fahre (zwei);
        std::cout << "  G1: akku=" << zahl (g1.akku, 6) << " ref=" << zahl (g1.ref, 6)
                  << " schranke=" << zahl (g1.schranke, 9)
                  << " ueberGitter=" << g1.ueberGitter << std::endl;
        pruefe (g1.ueberGitter == 1997,
                "G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)",
                std::to_string (g1.ueberGitter));
        schlimmster = std::max (schlimmster,
                                vergleiche ("zwei Pegel ueber dem Gitter (T2-1)", zwei));

        // G2 · Die Naht: ein Teil im Feingitter, ein Teil im Oberband. Einmal
        // mit Gamma_r oberhalb der Naht, einmal unterhalb - beide Male muss die
        // Auswahl dieselbe sein wie in der Referenz, obwohl links und rechts
        // der Naht verschiedene Binbreiten gelten.
        {
            std::vector<double> naht;
            for (int i = 0; i < 1000; ++i) naht.push_back (zelleFuer (1.0));     // ~ -0,69 LUFS
            for (int i = 0; i < 1000; ++i) naht.push_back (zelleFuer (1.0e5));   // ~ +49,31 LUFS
            schlimmster = std::max (schlimmster,
                                    vergleiche ("Naht, Gamma_r im Oberband", naht));
        }
        {
            std::vector<double> naht;
            for (int i = 0; i < 1900; ++i) naht.push_back (zelleFuer (1.0));
            for (int i = 0; i <  100; ++i) naht.push_back (zelleFuer (1.0e5));
            schlimmster = std::max (schlimmster,
                                    vergleiche ("Naht, Gamma_r im Feingitter", naht));
        }

        // G3 · Derselbe adversariale Aufbau wie §D - nur OBERHALB des Gitters.
        // z_laut = z_leise * (10*(n_l+n_q) - n_q)/n_l legt den leisen Lauf exakt
        // auf Gamma_r; mit z_leise = 2000 (~ +32,32 LUFS) liegen beide Laeufe im
        // Oberband. Der Grenzbin ist dort 1 LU breit statt 0,01 - die Schranke
        // muss den Fehler trotzdem decken, sonst ist sie keine.
        constexpr int nL = 1000, nQ = 1000;
        const double zQ = 2.0e3;
        const double zLexakt = zQ * (10.0 * (nL + nQ) - nQ) / nL;      // = 19 * zQ
        double maxD = 0.0, maxSchranke = 0.0, beiFaktor = 0.0;
        double maxRefSprung = 0.0, letzteRef = 0.0;
        int gedeckt = 0, laeufe = 0;
        std::uint64_t minUeberGitter = ~0ull, ueberOberband = 0;
        for (int k = -100; k <= 100; ++k)
        {
            const double faktor = 1.0 + (double) k * 1.0e-5;
            std::vector<double> zellen;
            zellen.reserve ((std::size_t) (nL + nQ));
            for (int i = 0; i < nL; ++i) zellen.push_back (zelleFuer (zLexakt * faktor));
            for (int i = 0; i < nQ; ++i) zellen.push_back (zelleFuer (zQ));

            const Ergebnis r = fahre (zellen);
            if (! (r.akkuGueltig && r.refGueltig)) { pruefe (false, "G3: beide gueltig"); break; }
            const double d = std::abs (r.akku - r.ref);
            if (laeufe > 0) maxRefSprung = std::max (maxRefSprung, std::abs (r.ref - letzteRef));
            letzteRef = r.ref;
            ++laeufe;
            if (d <= r.schranke + 1e-12) ++gedeckt;
            if (d > maxD) { maxD = d; beiFaktor = faktor; }
            maxSchranke   = std::max (maxSchranke, r.schranke);
            minUeberGitter = std::min (minUeberGitter, r.ueberGitter);
            ueberOberband += r.ueberOberband;
        }
        std::cout << "  G3-Sweep: " << laeufe << " Laeufe, groesste Abweichung "
                  << zahl (maxD, 6) << " LU bei Faktor " << zahl (beiFaktor, 6)
                  << ", groesste gemeldete Schranke " << zahl (maxSchranke, 6) << " LU"
                  << std::endl;
        std::cout << "  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: "
                  << zahl (maxRefSprung, 6) << " LU" << std::endl;
        pruefe (laeufe == 201, "G3: Sweep vollstaendig gefahren",
                std::to_string (laeufe) + "/201");
        pruefe (minUeberGitter > 0,
                "G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)",
                std::to_string (minUeberGitter));
        pruefe (maxSchranke > 0.0,
                "G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)",
                "sonst pruefte G3 nichts; schranke=" + zahl (maxSchranke, 6));
        // ⚠️ DIE Zeile, die vor der Nacharbeit gefehlt hat. Vor dem Fix meldete
        // der Akku hier 0,000000000 LU Schranke bei mehreren LU Fehler.
        pruefe (gedeckt == laeufe,
                "G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband",
                std::to_string (gedeckt) + "/" + std::to_string (laeufe));
        pruefe (maxD <= maxRefSprung + kToleranz,
                "G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz",
                "maxD=" + zahl (maxD, 6) + " refSprung=" + zahl (maxRefSprung, 6));
        pruefe (ueberOberband == 0, "G3: kein Block faellt durch das Raster",
                std::to_string (ueberOberband));
        std::cout << "  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - "
                  << "1000 Bloecke kippen gemeinsam die Gateseite.)" << std::endl;

        // G4 · Vollstaendigkeit des Rasters am aeussersten ERREICHBAREN Rand.
        // Das Raster ist gegen `lautheit(DBL_MAX)` = +3081,86 LUFS ausgelegt -
        // aber so hoch kommt ein Block bei 48 kHz gar nicht: er ist die Summe
        // VON VIER Zellen, und z = e/(0,4*sr) heisst e = z*19200. Damit ist bei
        // z > 9,363e303 schon die Zellensumme unendlich, und ein nicht-endlicher
        // Wert wird eine Zeile frueher gezaehlt statt gebinnt.
        //
        // ⚠️ Diese vier Zeilen sind die zweite Fassung. Die erste rechnete mit
        // z = 1e304 und lief prompt in genau diesen Ueberlauf: `bloeckeNichtEndlich`
        // stand auf 197 und `integriert()` lieferte gar nichts. Der Code hatte
        // recht, die Probe nicht - dieselbe Lehre wie beim NaN-Fenster in §8.4
        // des Pruefberichts.
        {
            LoudnessAccumulator a; a.vorbereiten (kFs);
            for (int i = 0; i < 200; ++i) a.zelle (zelleFuer (9.0e303));   // ~ +3038,85 LUFS
            double x = 0.0;
            const bool g = a.integriert (x);
            const double s = a.unsicherheitLu();
            std::cout << "  G4: LUFS-I " << zahl (x, 3) << ", Schranke " << zahl (s, 9)
                      << ", ueberGitter " << a.bloeckeUeberGitter()
                      << ", ueberOberband " << a.bloeckeUeberOberband() << std::endl;
            pruefe (g && std::isfinite (x) && x > 3000.0,
                    "G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert",
                    zahl (x, 3));
            pruefe (a.bloeckeNichtEndlich() == 0, "G4: nichts wird faelschlich als nicht-endlich gezaehlt",
                    std::to_string (a.bloeckeNichtEndlich()));
            pruefe (a.bloeckeUeberOberband() == 0,
                    "G4: auch am aeussersten Rand faellt kein Block durch das Raster",
                    std::to_string (a.bloeckeUeberOberband()));
            pruefe (std::isfinite (s), "G4: und die Schranke bleibt endlich", zahl (s, 9));
        }
    }

    std::cout << std::endl;
    std::cout << "Schlimmste gemessene Abweichung ueber alle Korpora: "
              << zahl (schlimmster, 9) << " LU (Toleranz " << zahl (kToleranz, 1) << ")."
              << std::endl;
    std::cout << geprueft << " Pruefungen, " << fehler << " Fehler." << std::endl;
    std::cout << (fehler == 0 ? "LOUDNESS-GOLDEN OK" : "LOUDNESS-GOLDEN FEHLGESCHLAGEN")
              << std::endl;
    return fehler == 0 ? 0 : 1;
}
