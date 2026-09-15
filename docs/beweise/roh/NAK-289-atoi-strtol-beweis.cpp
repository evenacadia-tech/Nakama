// NAK-289 Etappe 1, Fundklasse f): cert-err34-c in
// eq-copilot/plugin/core/ipc/WireZahl.h, detail::zerlegen.
//
// Frage: liefert static_cast<int> (std::strtol (s, nullptr, 10)) fuer jede
// Eingabe dasselbe wie std::atoi (s) - Wert UND errno?
//
// Primaerquelle: Windows SDK 10.0.26100.0 (und 10.0.19041.0),
//   ucrt/convert/atox.cpp:   atoi   -> __crt_strtox::parse_integer_from_string<long> (string, nullptr, 10, nullptr)
//   ucrt/convert/strtox.cpp: strtol -> __crt_strtox::parse_integer_from_string<long> (string, end_ptr, base, nullptr)
// Mit end_ptr = nullptr und base = 10 ist es derselbe Aufruf; long ist unter
// Windows (LLP64) 32 Bit breit wie int. Dieses Programm misst es an der
// gelinkten UCRT nach.
#include <cerrno>
#include <charconv>
#include <clocale>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace
{
struct Ergebnis
{
    std::uint64_t n = 0, abweichend = 0;
    std::string erste;
};

bool vergleiche (const std::string& s, Ergebnis& e, int* atoiWert = nullptr, int* strtolWert = nullptr,
                 int* atoiErrno = nullptr, int* strtolErrno = nullptr)
{
    errno = 0;
    const int a = std::atoi (s.c_str());
    const int ea = errno;
    errno = 0;
    const int b = static_cast<int> (std::strtol (s.c_str(), nullptr, 10));
    const int eb = errno;
    ++e.n;
    if (atoiWert != nullptr) *atoiWert = a;
    if (strtolWert != nullptr) *strtolWert = b;
    if (atoiErrno != nullptr) *atoiErrno = ea;
    if (strtolErrno != nullptr) *strtolErrno = eb;
    const bool gleich = a == b && ea == eb;
    if (! gleich)
    {
        ++e.abweichend;
        if (e.erste.empty())
            e.erste = s.empty() ? std::string ("(leer)") : s;
    }
    return gleich;
}

std::string sichtbar (const std::string& s)
{
    std::string aus;
    for (const unsigned char c : s)
    {
        if (c >= 0x21 && c <= 0x7e)
            aus += (char) c;
        else
        {
            char puffer[8];
            std::snprintf (puffer, sizeof (puffer), "\\x%02X", (unsigned) c);
            aus += puffer;
        }
    }
    return aus.empty() ? std::string ("(leer)") : aus;
}

// Exponententext genau so, wie detail::zerlegen ihn an atoi gibt.
bool exponent (double betrag, int stellen, std::string& aus)
{
    char puffer[64];
    const auto r = stellen > 0
        ? std::to_chars (puffer, puffer + sizeof (puffer), betrag,
                         std::chars_format::scientific, stellen - 1)
        : std::to_chars (puffer, puffer + sizeof (puffer), betrag,
                         std::chars_format::scientific);
    if (r.ec != std::errc())
        return false;
    const std::string w (puffer, r.ptr);
    const auto e = w.find ('e');
    if (e == std::string::npos)
        return false;
    aus = w.substr (e + 1);
    return true;
}

bool vorzeichenUndZiffern (const std::string& s)
{
    if (s.size() < 3 || s.size() > 4)
        return false;
    if (s[0] != '+' && s[0] != '-')
        return false;
    for (std::size_t i = 1; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9')
            return false;
    return true;
}

int lauf (const char* localeName, std::uint64_t zufallsAnzahl)
{
    int rc = 0;
    std::printf ("== Locale: %s ==\n", localeName);

    // 1) Jeder Exponententext, den der Aufrufer erzeugen kann: zerlegen bekommt
    //    den Betrag eines endlichen x != 0, mit stellen 0 (es6Zahl, ganzzahlige
    //    wireZahl) oder 15 (nichtganzzahlige wireZahl).
    std::set<std::string> texte;
    std::uint64_t aufrufe = 0, formVerletzt = 0;
    const auto sammle = [&] (double x)
    {
        if (! std::isfinite (x) || ! (x > 0.0))
            return;
        for (const int stellen : { 0, 15 })
        {
            std::string e;
            if (exponent (x, stellen, e))
            {
                ++aufrufe;
                texte.insert (e);
                if (! vorzeichenUndZiffern (e))
                    ++formVerletzt;
            }
        }
    };
    for (int k = -1074; k <= 1023; ++k)
    {
        const double p = std::ldexp (1.0, k);
        sammle (p);
        sammle (std::nextafter (p, 0.0));
        sammle (std::nextafter (p, std::numeric_limits<double>::infinity()));
    }
    for (int d = -330; d <= 310; ++d)
    {
        const std::string t = "1e" + std::to_string (d);
        const double x = std::strtod (t.c_str(), nullptr);
        sammle (x);
        sammle (std::nextafter (x, 0.0));
        sammle (std::nextafter (x, std::numeric_limits<double>::infinity()));
        sammle (x * 9.999999999999999);
        sammle (x * 0.99999999999999995);
    }
    sammle (std::numeric_limits<double>::max());
    sammle (std::numeric_limits<double>::min());
    sammle (std::numeric_limits<double>::denorm_min());
    std::mt19937_64 gen (0x4E414B323839ull);
    for (int i = 0; i < 2000000; ++i)
    {
        const std::uint64_t bits = gen();
        double x = 0.0;
        std::memcpy (&x, &bits, sizeof (x));
        sammle (std::fabs (x));
    }
    Ergebnis e1;
    for (const auto& s : texte)
        vergleiche (s, e1);
    std::printf ("1) Exponententexte aus std::to_chars: %llu Zerlegungen, %zu verschiedene Texte "
                 "(kleinster %s, groesster %s), Form [+-]Ziffern{2,3} verletzt: %llu, atoi != strtol: %llu\n",
                 (unsigned long long) aufrufe, texte.size(),
                 texte.empty() ? "-" : texte.begin()->c_str(), texte.empty() ? "-" : texte.rbegin()->c_str(),
                 (unsigned long long) formVerletzt, (unsigned long long) e1.abweichend);
    if (formVerletzt != 0 || e1.abweichend != 0)
        rc = 1;

    // 2) Adversariale Texte. Sie kommen im Aufrufer nicht vor; der Ersatz muss
    //    trotzdem fuer jede Eingabe dasselbe liefern (Auftrag Regel f).
    const std::vector<std::string> adversarial = {
        "", " ", "\t", "\n", "+", "-", "+-1", "-+1", "++1", "--1", " 42", "\t-42", "\n7", "\v8",
        "\f9", "\r10", "007", "+007", "-007", "12abc", "0x1A", "1e5", "3.9", "2147483647",
        "2147483648", "-2147483648", "-2147483649", "4294967296", "99999999999999999999",
        "-99999999999999999999", "  +0042abc", "\xC2\xA0" "5", "\xEF\xBC\x95", "\xD9\xA1\xD9\xA2",
        "+05", "-307", "+308", "-324", "+00", "-00"
    };
    Ergebnis e2;
    for (const auto& s : adversarial)
    {
        int a = 0, b = 0, ea = 0, eb = 0;
        const bool gleich = vergleiche (s, e2, &a, &b, &ea, &eb);
        std::printf ("2) %-26s atoi %11d (errno %d)  strtol %11d (errno %d)  %s\n",
                     sichtbar (s).c_str(), a, ea, b, eb, gleich ? "gleich" : "ABWEICHUNG");
    }
    if (e2.abweichend != 0)
        rc = 1;

    // 3) Zufallstexte ueber dem Alphabet der Grammatik (Leerraum, Vorzeichen,
    //    Ziffern, Buchstaben, Punkt, NBSP-Bytes), Laenge 0 bis 24.
    const std::string alphabet = std::string (" \t\n\v\f\r+-0123456789xXeE.a") + "\xC2\xA0";
    std::uniform_int_distribution<int> laenge (0, 24);
    std::uniform_int_distribution<std::size_t> zeichen (0, alphabet.size() - 1);
    Ergebnis e3;
    for (std::uint64_t i = 0; i < zufallsAnzahl; ++i)
    {
        std::string s;
        const int n = laenge (gen);
        for (int k = 0; k < n; ++k)
            s += alphabet[zeichen (gen)];
        vergleiche (s, e3);
    }
    std::printf ("3) Zufallstexte: %llu, atoi != strtol: %llu%s%s\n",
                 (unsigned long long) e3.n, (unsigned long long) e3.abweichend,
                 e3.abweichend != 0 ? ", erste: " : "", e3.abweichend != 0 ? sichtbar (e3.erste).c_str() : "");
    if (e3.abweichend != 0)
        rc = 1;
    return rc;
}
} // namespace

int main()
{
    int rc = lauf ("C (Programmstart)", 20000000ull);
    if (std::setlocale (LC_ALL, "de-DE") != nullptr)
        rc |= lauf ("de-DE", 5000000ull);
    else
        std::printf ("== Locale de-DE nicht verfuegbar ==\n");
    std::printf ("URTEIL: %s\n", rc == 0 ? "atoi und static_cast<int> (strtol (s, nullptr, 10)) gleich - Wert und errno"
                                         : "ABWEICHUNG");
    return rc;
}
