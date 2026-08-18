// M3-Kern — Regeln siehe Diagnose.h. Alle Schwellen sind versionierte
// Startwerte (kDiagnoseVersion); Änderung nur über neue Version, nie still.
#include "Diagnose.h"
#include <algorithm>
#include <cmath>

namespace eqcop
{

static juce::String u8 (const char* text) { return juce::String (juce::CharPointer_UTF8 (text)); }

// ── Frequenz in Lernsprache: „116 Hz" · „2,5 kHz" ───────────────────────────
static juce::String fText (double hz)
{
    if (hz < 1000.0)
        return juce::String ((int) std::lround (hz)) + " Hz";
    return juce::String (hz / 1000.0, 1).replace (".", ",") + " kHz";
}

// ── Notenname in FL-ZÄHLUNG (Landmine tools/midi-ideen: FL zeigt MIDI 60 als
//    C5 ⇒ Oktave = MIDI div 12; 116 Hz ⇒ MIDI 46 ⇒ A#3). Der User ist
//    Musiker — „A#3" verortet eine Resonanz musikalisch (RESO-Erbe). ────────
static juce::String flNote (double hz)
{
    if (! (hz > 0.0))
        return {};
    const int midi = (int) std::lround (69.0 + 12.0 * std::log2 (hz / 440.0));
    if (midi < 0 || midi > 131)   // FLs Notenraum C0–G10
        return {};
    static const char* namen[12] = { "C", "C#", "D", "D#", "E", "F",
                                     "F#", "G", "G#", "A", "A#", "B" };
    return juce::String (namen[midi % 12]) + juce::String (midi / 12);
}

// Slot des Zonen-Zeitverlaufs (ZonenRegeln.h) zu einer Befundklasse.
static int zonenSlot (BefundKlasse k)
{
    switch (k)
    {
        case BefundKlasse::mittenLoch: return kZonenSlotMittenLoch;
        case BefundKlasse::mulm:       return kZonenSlotMulm;
        case BefundKlasse::haerte:     return kZonenSlotHaerte;
        case BefundKlasse::hoehenHype: return kZonenSlotHoehenHype;
        case BefundKlasse::resonanz:   break;
    }
    return -1;
}

// M3a: echte Zonen-Persistenz aus dem Engine-Zeitverlauf; ohne ausreichende
// Ticks bleibt die ehrliche 0,5-Deckelung von m3 bestehen.
static double zonenPersistenz (const MessSnapshot& m, BefundKlasse k)
{
    const int slot = zonenSlot (k);
    if (slot >= 0)
    {
        const auto& zz = m.zonenZeit[(size_t) slot];
        if (zz.gueltig && zz.ticks >= 15)
            return zz.anteil;
    }
    return 0.5;
}

// Lernsprache-Anhang „Das war in NN % der Musikzeit so." — nur wenn der
// Zeitverlauf genug aktive Ticks hat (sonst wäre die Zahl Scheinwissen).
static juce::String zeitverlaufText (const MessSnapshot& m, BefundKlasse k)
{
    const int slot = zonenSlot (k);
    if (slot < 0)
        return {};
    const auto& zz = m.zonenZeit[(size_t) slot];
    if (! zz.gueltig || zz.ticks < 15)
        return {};
    return u8 (" Das war in ") + juce::String ((int) std::lround (zz.anteil * 100.0))
           + u8 (" % der Musikzeit so.");
}

// ── Zonen-Statistik auf der Komposit-LTAS (Anzeige-dB; Differenzen sind
//    skalenunabhängig). dB-Mittel — robust gegen Einzel-Peaks, die der
//    Resonanzweg getrennt behandelt. ─────────────────────────────────────────
struct ZonenWert
{
    double mittelDb = 0.0;
    int    belegt = 0, gesamt = 0;
    double logfMitte = 0.0;   // log2 der geometrischen Zonen-Mitte
};

static ZonenWert zonenMittel (const MessSnapshot& m, double fLo, double fHi)
{
    ZonenWert z;
    z.logfMitte = 0.5 * (std::log2 (fLo) + std::log2 (fHi));
    double summe = 0.0;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const double f = m.ltasZentrenHz[(size_t) b];
        if (f < fLo || f >= fHi)
            continue;
        ++z.gesamt;
        const double v = m.ltasKompositDb[(size_t) b];
        if (std::isfinite (v))
        {
            summe += v;
            ++z.belegt;
        }
    }
    if (z.belegt > 0)
        z.mittelDb = summe / z.belegt;
    return z;
}

// Anteil belastbarer 1/3-Okt-Abdeckungsbänder in [fLo, fHi) — §5.10.2-Gate.
static double abdeckungAnteil (const MessSnapshot& m, double fLo, double fHi)
{
    int belastbar = 0, gesamt = 0;
    for (const auto& a : m.abdeckung)
    {
        if (a.zentrumHz < fLo || a.zentrumHz >= fHi)
            continue;
        ++gesamt;
        if (a.klasse == AbdeckungsKlasse::belastbar)
            ++belastbar;
    }
    return gesamt > 0 ? (double) belastbar / (double) gesamt : 0.0;
}

// ── Konfidenz §5.10.7: vier Komponenten 0–1, Klasse aus min + Mittel ────────
struct KonfidenzTeile
{
    double abdeckung = 0.0, messzeit = 0.0, persistenz = 0.0, ziel = 0.0;
};

static void konfidenzRechnen (Befund& b, const KonfidenzTeile& k)
{
    const double werte[4] = { k.abdeckung, k.messzeit, k.persistenz, k.ziel };
    double minW = 1.0, summe = 0.0;
    int minIdx = 0;
    for (int i = 0; i < 4; ++i)
    {
        summe += werte[i];
        if (werte[i] < minW) { minW = werte[i]; minIdx = i; }
    }
    const double mittel = summe / 4.0;
    b.konfidenz = (minW >= 0.6 && mittel >= 0.75) ? Konfidenz::hoch
                : (minW >= 0.4 && mittel >= 0.55) ? Konfidenz::mittel
                                                  : Konfidenz::niedrig;
    const char* namen[4] = { "Signalabdeckung", "Messzeit",
                             "Zeitverlauf-Beleg", "Regelsicherheit" };
    const char* stufe = b.konfidenz == Konfidenz::hoch ? "hoch"
                      : b.konfidenz == Konfidenz::mittel ? "mittel" : "niedrig";
    b.konfidenzGrund = juce::String (stufe) + u8 (" — begrenzt durch ")
                       + u8 (namen[minIdx])
                       + " (" + juce::String (minW, 2).replace (".", ",") + ")";
}

// ── Resonanz-Karten (§5.10.3 → §5.10.6: dauerhaft+schmal = statischer Cut,
//    zeitweise+schmal = dynamische Bearbeitung) ─────────────────────────────
static Befund resonanzKarte (const MessSnapshot& m, const ResonanzKandidat& k)
{
    Befund b;
    b.klasse = BefundKlasse::resonanz;
    b.fSchwerpunkt = k.freqHz;
    const double halbeBreite = std::max (k.breiteOktaven, 1.0 / 12.0) * 0.5;
    b.fVon = k.freqHz * std::pow (2.0, -halbeBreite);
    b.fBis = k.freqHz * std::pow (2.0, halbeBreite);
    b.staerkeDb = k.dbOver;

    const bool dauerhaft = k.klasse == ResonanzKlasse::dauerhaft;
    b.dauerhaft = dauerhaft;
    const int cutBis = (int) juce::jlimit (2.0, 6.0, std::round (k.dbOver * 0.6));
    const juce::String f = fText (k.freqHz);
    // Musikalische Verortung (M3a, RESO-Erbe): Note in FL-Zählung im Titel.
    const juce::String note = flNote (k.freqHz);
    const juce::String noteText = note.isEmpty() ? juce::String() : u8 (" · ") + note;
    const juce::String schmalheit = k.breiteOktaven <= 1.0 / 6.0 ? u8 ("sehr schmal") : u8 ("schmal");

    b.titel = dauerhaft ? u8 ("Ein Ton sticht dauerhaft heraus (") + f + noteText + ")"
                        : u8 ("Ein Ton sticht zeitweise heraus (") + f + noteText + ")";
    b.gemessen = u8 ("Bei ") + f + u8 (" ragt eine ") + schmalheit
                 + u8 ("e Spitze ") + juce::String (k.dbOver, 1).replace (".", ",")
                 + u8 (" dB über ihre Umgebung, in ")
                 + juce::String ((int) std::lround (k.persistenzAnteil * 100.0))
                 + u8 (" % der Musikzeit.");
    if (note.isNotEmpty())
        b.gemessen += u8 (" Als Note: ") + note + u8 (".");
    // Charakter aus P95−P50 des Trägerbands (M3a): ruhiger Pegel → fester
    // Absenker ist sicher; wellender Pegel → dynamisches Werkzeug passt.
    double spanne = std::numeric_limits<double>::quiet_NaN();
    if (m.perzentileGueltig)
    {
        int band = -1;
        double bester = 1e18;
        for (int i = 0; i < kLtasBaender; ++i)
        {
            const double z = m.ltasZentrenHz[(size_t) i];
            if (! (z > 0.0))
                continue;
            const double d = std::abs (std::log2 (z / k.freqHz));
            if (d < bester)
            {
                bester = d;
                band = i;
            }
        }
        if (band >= 0)
        {
            const double p50 = m.perzentilP50[(size_t) band];
            const double p95 = m.perzentilP95[(size_t) band];
            if (std::isfinite (p50) && std::isfinite (p95))
            {
                spanne = p95 - p50;
                const int sp = (int) std::lround (spanne);
                b.gemessen += sp <= 6
                    ? u8 (" Der Pegel dort steht ruhig (Schwankung ~") + juce::String (sp) + u8 (" dB).")
                    : u8 (" Der Pegel dort kommt in Wellen (~") + juce::String (sp) + u8 (" dB Schwankung).");
            }
        }
    }
    b.wirkung = dauerhaft
        ? u8 ("So ein Dauerton kann dröhnen oder klingeln und macht den Mix anstrengend — oft fällt er erst beim leisen Hören auf.")
        : u8 ("Ein Ton, der nur in Momenten hervorsticht, wirkt wie gelegentliches Dröhnen — dauerhaft absenken würde den Rest der Zeit Substanz kosten.");
    // m4.1 (Kalibrier-Runde 1, Testtrack): wogender Pegel überstimmt die
    // Persistenz bei der WERKZEUGWAHL — „dauerhaft + wellig" war real die
    // eigene Melodie (A#5-Tonika); ein fester Cut hätte sie gedämpft.
    const bool wellig = std::isfinite (spanne) && spanne > 10.0;
    if (dauerhaft && ! wellig)
    {
        b.tu = u8 ("Senk genau dort ") + schmalheit + u8 (" ab: im Fruity PEQ2 eine ")
               + schmalheit + u8 ("e Glocke auf ") + f
               + u8 (" (erst −3 dB, bei Bedarf bis −") + juce::String (cutBis)
               + u8 (" dB; die Breite regelt dort Q — hoch heißt schmal).");
        b.warum = u8 ("Es geht nur um diesen einen Ton — der Rest der Kurve ist in Ordnung und soll unangetastet bleiben.");
        b.hoeren = u8 ("Das Dröhnen verschwindet, der Klang räumt sich auf. Stopp, sobald die Stelle dünn oder hohl klingt — dann war es zu viel.");
    }
    else if (dauerhaft)
    {
        b.tu = u8 ("Der Ton ist fast immer da, aber sein Pegel wogt — nimm das Werkzeug, das nur die Spitzen abfängt: Smooth Operator Pro mit ")
               + schmalheit + u8 ("em Fokus um ") + f + u8 (" und sanfter Stärke.");
        b.warum = u8 ("Ein fester Einschnitt würde auch die leisen Momente dämpfen — oft ist so ein wogender Dauerton schlicht die Melodie oder ein tragender Klang, und der soll leben.");
        b.hoeren = u8 ("Die lauten Momente werden zahmer, der Ton selbst bleibt. Stopp, wenn es pumpt oder leblos wird.");
    }
    else
    {
        b.tu = u8 ("Nimm ein Werkzeug, das nur bei Bedarf eingreift: Smooth Operator Pro mit ")
               + schmalheit + u8 ("em Fokus um ") + f
               + u8 (" und sanfter Stärke — es senkt nur ab, wenn die Spitze wirklich da ist.");
        b.warum = u8 ("Der Ton stört nur in Momenten — ein fester EQ-Einschnitt würde die übrige Zeit grundlos Klang wegnehmen.");
        b.hoeren = u8 ("Die störenden Momente werden zahmer, der Grundklang bleibt. Stopp, wenn es leblos oder ‚gluckernd‘ wird.");
    }

    KonfidenzTeile kt;
    kt.abdeckung = abdeckungAnteil (m, k.freqHz * std::pow (2.0, -1.0 / 3.0),
                                    k.freqHz * std::pow (2.0, 1.0 / 3.0));
    kt.messzeit = juce::jlimit (0.0, 1.0, m.aktivSekunden / 30.0);
    kt.persistenz = k.persistenzAnteil;
    kt.ziel = 0.9;   // messfest: Spitze gegen die eigene Hüllkurve
    konfidenzRechnen (b, kt);
    return b;
}

// ── Zonen-Regel: Zone gegen die log-f-Linie ihrer eigenen Schultern ─────────
struct ZonenRegel
{
    BefundKlasse klasse;
    double schulterLoVon, schulterLoBis;
    double zoneVon, zoneBis;
    double schulterHiVon, schulterHiBis;
    double schwelleDb;        // >0: Zone ÜBER Linie ist der Befund; <0: darunter
};

static bool zonenBefund (const MessSnapshot& m, const ZonenRegel& r, Befund& aus)
{
    // §5.10.2/§5.10.4-Gates: alle drei Zonen belastbar abgedeckt + LTAS belegt.
    const double abLo = abdeckungAnteil (m, r.schulterLoVon, r.schulterLoBis);
    const double abZo = abdeckungAnteil (m, r.zoneVon, r.zoneBis);
    const double abHi = abdeckungAnteil (m, r.schulterHiVon, r.schulterHiBis);
    if (abLo < 0.6 || abZo < 0.6 || abHi < 0.6)
        return false;

    const auto lo = zonenMittel (m, r.schulterLoVon, r.schulterLoBis);
    const auto zo = zonenMittel (m, r.zoneVon, r.zoneBis);
    const auto hi = zonenMittel (m, r.schulterHiVon, r.schulterHiBis);
    if (lo.belegt < 3 || hi.belegt < 3
        || zo.gesamt == 0 || (double) zo.belegt / (double) zo.gesamt < 0.7)
        return false;

    // Schulterlinie: log-f-linear zwischen den Schulter-Mitteln.
    auto linieDb = [&] (double logf)
    {
        const double t = (logf - lo.logfMitte) / (hi.logfMitte - lo.logfMitte);
        return lo.mittelDb + t * (hi.mittelDb - lo.mittelDb);
    };

    // Kriterium A: Zonenmittel jenseits der Schwelle.
    const double deltaMittel = zo.mittelDb - linieDb (zo.logfMitte);
    const bool ueber = r.schwelleDb > 0.0;
    if (ueber ? (deltaMittel < r.schwelleDb) : (deltaMittel > r.schwelleDb))
        return false;

    // Kriterium B: ≥ 60 % der Zonen-Bänder jenseits der halben Schwelle —
    // verhindert, dass ein Einzel-Peak (Resonanzweg!) das Mittel zieht.
    int jenseits = 0, belegt = 0;
    double staerkstesDelta = 0.0, schwerpunkt = std::sqrt (r.zoneVon * r.zoneBis);
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const double f = m.ltasZentrenHz[(size_t) b];
        if (f < r.zoneVon || f >= r.zoneBis)
            continue;
        const double v = m.ltasKompositDb[(size_t) b];
        if (! std::isfinite (v))
            continue;
        ++belegt;
        const double delta = v - linieDb (std::log2 (f));
        if (ueber ? (delta >= r.schwelleDb * 0.5) : (delta <= r.schwelleDb * 0.5))
            ++jenseits;
        if (std::abs (delta) > std::abs (staerkstesDelta)
            && (ueber ? delta > 0 : delta < 0))
        {
            staerkstesDelta = delta;
            schwerpunkt = f;
        }
    }
    if (belegt == 0 || (double) jenseits / (double) belegt < 0.6)
        return false;

    aus.klasse = r.klasse;
    aus.fVon = r.zoneVon;
    aus.fBis = r.zoneBis;
    aus.fSchwerpunkt = schwerpunkt;
    aus.staerkeDb = deltaMittel;

    KonfidenzTeile kt;
    kt.abdeckung = std::min ({ abLo, abZo, abHi });
    kt.messzeit = juce::jlimit (0.0, 1.0, m.aktivSekunden / 30.0);
    // M3a: echter Zeitverlauf-Beleg statt pauschaler 0,5-Deckelung — Zonen
    // können jetzt ehrlich „hoch" erreichen, wenn die Evidenz da ist.
    kt.persistenz = zonenPersistenz (m, r.klasse);
    kt.ziel = 0.6;         // Schulterlinien-Heuristik, kein kalibrierter Korridor
    konfidenzRechnen (aus, kt);
    return true;
}

static juce::String dbText (double db)
{
    return juce::String (std::abs (db), 1).replace (".", ",") + " dB";
}

std::vector<Befund> diagnose (const MessSnapshot& m, const juce::String& rolle)
{
    std::vector<Befund> alle;
    if (m.zustand != MessZustand::messbereit || ! m.ltasGueltig)
        return alle;

    // ── Resonanzen: die zwei stärksten als Karten (alle stehen im Graph) ──
    {
        std::vector<ResonanzKandidat> sortiert = m.resonanzen;
        std::sort (sortiert.begin(), sortiert.end(),
                   [] (const auto& a, const auto& b) { return a.dbOver > b.dbOver; });
        for (size_t i = 0; i < sortiert.size() && i < 2; ++i)
            alle.push_back (resonanzKarte (m, sortiert[i]));
    }

    const juce::String ort = rolle == "hub" ? u8 ("auf dem Master") : u8 ("auf diesem Kanal");

    // ── Mitten-Loch 500–2000 („Smile-EQ": der historische Hauptbefund) ──
    {
        Befund b;
        if (zonenBefund (m, { BefundKlasse::mittenLoch,
                              150.0, 400.0, 500.0, 2000.0, 2500.0, 5000.0, -3.0 }, b))
        {
            b.titel = u8 ("Die Mitte fehlt (500–2000 Hz)");
            b.gemessen = u8 ("Zwischen 500 und 2000 Hz liegt die Kurve im Mittel ")
                         + dbText (b.staerkeDb)
                         + u8 (" unter der Verbindungslinie ihrer Nachbarbereiche; am tiefsten um ")
                         + fText (b.fSchwerpunkt) + "."
                         + zeitverlaufText (m, BefundKlasse::mittenLoch);
            b.wirkung = u8 ("Genau hier wohnen Klavier-Anschlag und Stimme — fehlt dieser Bereich, "
                            "klingt der Mix auf kleinen Boxen leer und verliert Wärme (das typische "
                            "„Badewannen“-Muster: viel unten, viel oben, nichts dazwischen).");
            b.tu = u8 ("Erstidee: kein EQ, sondern Balance — schieb die Mitten-Träger (Klavier, "
                       "Stimme/Chor) am Fader ein Stück nach vorn (+1 bis +2 dB). Reicht das nicht: "
                       "eine BREITE, kleine Anhebung um 800–1200 Hz (+1 bis +2 dB) ") + ort + ".";
            b.warum = u8 ("Bei einem breiten, dauerhaften Loch ist meist die Balance die Ursache — "
                          "ein EQ-Berg an derselben Stelle klingt schnell künstlich (Plan-Regel: "
                          "breit + dauerhaft ⇒ erst Fader, dann EQ).");
            b.hoeren = u8 ("Der Mix rückt näher und wird wärmer, Stimme und Klavier tragen das Stück. "
                           "Stopp, wenn es topfig oder nasal wird.");
            alle.push_back (b);
        }
    }

    // ── Mulm 120–300 (User-Stil: tiefe Klavier-Akkorde SOLLEN da sein) ──
    {
        Befund b;
        if (zonenBefund (m, { BefundKlasse::mulm,
                              50.0, 100.0, 120.0, 300.0, 400.0, 800.0, 4.0 }, b))
        {
            b.titel = u8 ("Untenrum staut es sich (120–300 Hz)");
            b.gemessen = u8 ("Zwischen 120 und 300 Hz liegt die Kurve im Mittel ")
                         + dbText (b.staerkeDb)
                         + u8 (" über der Linie ihrer Nachbarbereiche; Schwerpunkt um ")
                         + fText (b.fSchwerpunkt) + "."
                         + zeitverlaufText (m, BefundKlasse::mulm);
            b.wirkung = u8 ("Zu viel Energie hier klingt belegt und wummerig — die tiefen "
                            "Klavier-Akkorde verschwimmen, statt zu tragen.");
            b.tu = u8 ("Senk maßvoll und eher schmal ab: eine Glocke um ") + fText (b.fSchwerpunkt)
                   + u8 (" (−2 bis −3 dB) — oder lass Smooth Operator Pro nur die Stau-Momente "
                         "abfangen. Keine breite Absenkung.");
            b.warum = u8 ("Das Fundament der tiefen Klavier-Akkorde soll bleiben — weg soll nur der "
                          "Stau. Eine breite Absenkung nähme Körper und Sustain gleich mit.");
            b.hoeren = u8 ("Es räumt sich auf, das Klavier behält Körper und Sustain. Stopp, sobald "
                           "es dünn klingt.");
            alle.push_back (b);
        }
    }

    // ── Härte 2,5–5 kHz ──
    {
        Befund b;
        if (zonenBefund (m, { BefundKlasse::haerte,
                              1000.0, 2000.0, 2500.0, 5000.0, 6000.0, 10000.0, 4.0 }, b))
        {
            b.titel = u8 ("Es beißt in den Präsenzen (2,5–5 kHz)");
            b.gemessen = u8 ("Zwischen 2,5 und 5 kHz liegt die Kurve im Mittel ")
                         + dbText (b.staerkeDb)
                         + u8 (" über der Linie ihrer Nachbarbereiche; am stärksten um ")
                         + fText (b.fSchwerpunkt) + "."
                         + zeitverlaufText (m, BefundKlasse::haerte);
            b.wirkung = u8 ("Dieser Bereich macht Lautstärke-Eindruck, aber auch Hörermüdung — er "
                            "sticht zuerst ins Ohr, besonders bei längerem Hören.");
            b.tu = u8 ("Am musikalischsten dynamisch: Smooth Operator Pro über 2,5–5 kHz mit sanfter "
                       "Stärke — er greift nur in den scharfen Momenten. Statisch ginge eine breite "
                       "Glocke um ") + fText (b.fSchwerpunkt) + u8 (" (−1 bis −2 dB).");
            b.warum = u8 ("Meist beißen nur Momente (Anschläge, Konsonanten, Becken) — dauerhaft "
                          "absenken würde den ganzen Mix stumpf machen.");
            b.hoeren = u8 ("Längeres Hören strengt weniger an, ohne dass Glanz verloren geht. Stopp, "
                           "wenn es dumpf oder leblos wird.");
            alle.push_back (b);
        }
    }

    // ── Höhen-Hype: Air ÜBER den Präsenzen ist unnatürlich (Stil: nie hypen) ──
    {
        const double abPr = abdeckungAnteil (m, 2000.0, 6000.0);
        const double abAir = abdeckungAnteil (m, 8000.0, 14000.0);
        const auto pr = zonenMittel (m, 2000.0, 6000.0);
        const auto air = zonenMittel (m, 8000.0, 14000.0);
        if (abPr >= 0.6 && abAir >= 0.6 && pr.belegt >= 5 && air.belegt >= 5
            && air.mittelDb > pr.mittelDb + 1.0)
        {
            Befund b;
            b.klasse = BefundKlasse::hoehenHype;
            b.fVon = 8000.0;
            b.fBis = 14000.0;
            b.fSchwerpunkt = 10000.0;
            b.staerkeDb = air.mittelDb - pr.mittelDb;
            b.titel = u8 ("Die Höhen glitzern über (ab 8 kHz)");
            b.gemessen = u8 ("Der Bereich über 8 kHz liegt im Mittel ")
                         + dbText (b.staerkeDb)
                         + u8 (" ÜBER den Präsenzen (2–6 kHz) — natürlich wäre ein Gefälle nach oben.")
                         + zeitverlaufText (m, BefundKlasse::hoehenHype);
            b.wirkung = u8 ("Viel „Luft“ wirkt im ersten Moment edel, kippt aber schnell in scharf "
                            "und zischelig — und dieser Stil braucht keinen Höhen-Hype.");
            b.tu = u8 ("Nimm die Höhen-Anhebung zurück: ein Shelf (Regal) ab etwa 8 kHz um −1 bis "
                       "−2 dB — oft steckt der Hype in einem „Air“-Regler oder High-Shelf am Master.");
            b.warum = u8 ("Glanz soll aus vollen Mitten und dem Raum kommen, nicht aus einem "
                          "Höhenregal — sonst ermüden die Ohren zuerst.");
            b.hoeren = u8 ("S-Laute und Becken werden angenehmer, nichts klingt mehr aufgerissen. "
                           "Stopp, wenn es dumpf wird.");
            KonfidenzTeile kt;
            kt.abdeckung = std::min (abPr, abAir);
            kt.messzeit = juce::jlimit (0.0, 1.0, m.aktivSekunden / 30.0);
            kt.persistenz = zonenPersistenz (m, BefundKlasse::hoehenHype);   // M3a
            kt.ziel = 0.7;   // „Air über Präsenz" ist fast immer unnatürlich
            konfidenzRechnen (b, kt);
            alle.push_back (b);
        }
    }

    // ── Priorität + Top-Drei (§5.9/§5.10.7: niedrig erreicht nie die Top-3).
    // Stilgewichtung: dauerhafte Resonanz (messfest) → Mitten-Loch (Hebel #1
    // des Users) → Mulm → Härte → Höhen-Hype → zeitweise Resonanz.
    auto rang = [] (const Befund& b) -> int
    {
        switch (b.klasse)
        {
            case BefundKlasse::resonanz:   return b.dauerhaft ? 0 : 5;
            case BefundKlasse::mittenLoch: return 1;
            case BefundKlasse::mulm:       return 2;
            case BefundKlasse::haerte:     return 3;
            case BefundKlasse::hoehenHype: return 4;
        }
        return 9;
    };
    std::stable_sort (alle.begin(), alle.end(), [&] (const Befund& a, const Befund& b)
    {
        const int ra = rang (a), rb = rang (b);
        if (ra != rb)
            return ra < rb;
        return std::abs (a.staerkeDb) > std::abs (b.staerkeDb);
    });
    int top = 0;
    for (auto& b : alle)
        if (b.konfidenz != Konfidenz::niedrig && top < 3)
        {
            b.topRang = true;
            ++top;
        }
    return alle;
}

juce::String diagnoseKnopfText (const std::vector<Befund>& befunde)
{
    const int n = (int) befunde.size();
    if (n == 0)
        return u8 ("keine Auffälligkeit");
    if (n == 1)
        return u8 ("1 Auffälligkeit");
    return juce::String (n) + u8 (" Auffälligkeiten");
}

} // namespace eqcop
