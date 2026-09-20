#pragma once

/*  SONDE-015 Etappe 3 - das unveraenderliche `DspProgram` und sein Bau.

    ZWEI SEITEN, EINE WAHRHEIT. Der Kern hat keinen eigenen Zustandstyp:
    `nakama::parameter::DspSatz` aus Etappe 2 IST der DspState (120 Werte
    plus Zonenliste, Grenzen, Defaults und Reihenfolge aus
    `nakama-parameter-v2.json`). Was hier entsteht, ist ausschliesslich
    ABGELEITET - Koeffizienten, Rampenziele, der Auto-Gain-Betrag, die
    Klemmliste. Ein zweiter Zustandstyp waere die zweite Wahrheit, die R1
    und §33.5 ausschliessen.

    Ein `DspProgramm` ist fuer den Audiothread UNVERAENDERLICH. Es traegt
    keinen Filterzustand: §44.2 verlangt, dass der Worker Koeffizienten
    UND Filterzustand erst nach dem Audio-ACK ueberschreiben darf, und was
    im selben Objekt liegt, ist dabei nicht auseinanderzuhalten. Der
    Zustand gehoert deshalb zur BANK (`DspBankPool.h`).

    Gebaut wird AUSSERHALB des Audiothreads (R9). Der Bau rechnet `sin`,
    `cos`, `pow`, `tan` und die 121 Gitterstellen des Auto-Gains; nichts
    davon laeuft je im Callback.

    NAK-311 R-311-3 (T3-16-04): seit Etappe 4 traegt das Programm die
    KANALZAHL des Busses (`kanaele`, Vorgabe 2). Sie kommt aus
    `SondeProcessor::prepareToPlay` ueber `DspKern::bereiteVor`
    beziehungsweise `Transaktionskern::setzeSamplerate` in `baueProgramm`
    und wird ausschliesslich von `leiteAutoGainAb` gelesen: auf einem
    Monobus rechnet die Ableitung die geordnete Kaskade und wertet nur den
    ausgegebenen Kanal aus. Sie ist Laufzeit des Hosts - kein Statefeld,
    keine Revision, kein Host-Dirty.
*/

#include "DspFilter.h"
#include "DspSvf.h"
#include "NakamaParameter.h"

#include <array>
#include <cstdint>
#include <limits>

namespace nakama::dsp
{

using nakama::parameter::kSlots;

/** Die fuenf Vertragswerte von `channel_mode` in Enumreihenfolge. */
enum class Kanalmodus { stereo = 0, left, right, mid, side };

/** Die drei Vertragswerte von `sidechain_source` in Enumreihenfolge. */
enum class Sidechain { none = 0, internal, prioritySidechain };

/** Die vier Zustaende der Hoermatrix (R10). Transient - kein Parameter,
    kein Feld im DTO, nichts im `state_hash`. */
enum class Hoermatrix { processed = 0, dry, delta, candidate };

//==============================================================================
// Technikkonstanten mit Test. Der Entwurf nennt keine Zahl fuer sie; sie
// stehen hier EINMAL und werden von B6 gemessen (§5.2, §5.7, §5.10).

/** Laenge des Crossfades bei topologischen Wechseln, in SAMPLES gezaehlt
    (§5.2 Feinheit 1). In Bloecken gezaehlt dauerte derselbe Fade bei
    Blockgroesse 1 und 2048 verschieden lang. */
inline constexpr int kFadeSamples = 256;

/** Laenge der Rampe kontinuierlicher Werte, in Samples. "Genau eine
    definierte Rampe" (§53.8, M-17) - es gibt keine zweite Glaettung. */
inline constexpr int kRampeSamples = 256;

/** NAK-311 R-311-16 (T3-Abdeckungsfeld F08, Karte U47): die kleinste
    UNTERSTUETZTE Abtastrate in Hz. Keine gewaehlte Zahl, sondern die Zahl
    einer bestehenden Zusage - R4 Feinheit 1 (`docs/beweise/SONDE-015.md`,
    Abschnitt R4) nennt 44,1 kHz ausdruecklich als kleinste unterstuetzte
    Rate und begruendet damit, dass das Auto-Gain-Gitter bis 20 kHz keine
    Nyquistkappung braucht; die Abnahme U47 vom 19.09.2026 bestaetigt sie
    woertlich ("genau 44,1 kHz bleibt unterstuetzt"). Darunter bleibt der EQ
    neutral auf dem Weg des AUSGESCHALTETEN EQ. */
inline constexpr double kMinSamplerateHz = 44100.0;

/** Der EINE Ort der Vergleichsrichtung (NAK-311 §41.2 F-25): endlich UND
    groesser oder gleich `kMinSamplerateHz`. Gelesen wird das Praedikat von
    `DspKern::bereiteVor` (Kern), `Transaktionskern::setzeSamplerate`
    (Bericht) und `SondeProcessor::prepareToPlay` (Publikation); eine zweite
    Stelle mit der Zahl waere eine zweite Wahrheit, die driften kann.

    Nicht endlich und 0 sind hier ebenfalls "nicht unterstuetzt" - sie sind
    aber das VERRIEGELTE Fenster aus R-311-12 und keine abgelehnte Rate. Den
    Unterschied macht der Aufrufer (`samplerate > 0.0 && samplerate <
    kMinSamplerateHz` ist die abgelehnte Rate), nicht dieses Praedikat. */
constexpr bool samplerateUnterstuetzt (double samplerate) noexcept
{
    // Kein `std::isfinite`: es ist erst in C++23 constexpr. NaN faellt an der
    // ersten Vergleichsrichtung (jeder Vergleich mit NaN ist falsch), +Inf an
    // der zweiten, -Inf an der ersten.
    return samplerate >= kMinSamplerateHz
        && samplerate < std::numeric_limits<double>::infinity();
}

/** Fester Lautheitsabgleich des Delta-Hoerzustands (§5.10 Feinheit 1).
    Fest heisst materialunabhaengig; ein aus dem laufenden Pegel gerechneter
    Abgleich waere ein Kompressor im Hoerweg (§49.2 Gate 5). */
inline constexpr double kDeltaMakeupDb = 12.0;

/** Steuerrate des dynamischen Bandes in Samples: alle so viele Samples
    werden Detektorpegel, Kennlinie und SVF-Koeffizienten neu gerechnet,
    dazwischen laeuft der Mischfaktor linear weiter. Der DETEKTOR selbst
    laeuft mit voller Audiorate; nur `log10` und `pow` haengen an diesem
    Schritt, und genau ihre Kosten sind der Grund: je Sample und Band
    kosten sie ein Vielfaches der Filterarbeit.

    Was der Schritt fuer die REAKTION heisst (NAK-311 T3-15-11, gemessen in
    311/M-73 bis 311/M-75): der frisch entworfene Koeffizientensatz wirkt am
    Entwurfssample mit Gewicht NULL - `schrittRest` steht dort auf
    `kDynamikSchritt`, `tSchritt` also auf 0 - und erst acht Samples spaeter
    mit Gewicht eins. Ein einsetzender Pegel wirkt damit anteilig nach 1 bis
    8 und voll nach 8 bis 15 Samples: 0,167 bis 0,3125 ms bei 48 kHz und
    0,181 bis 0,340 ms bei 44,1 kHz. Das ist GROEBER als die kuerzeste
    einstellbare Attack von 0,1 ms; erst ab 96 kHz ist der Schritt selbst
    (0,083 ms) feiner als sie. Eine Zusage zur Reaktionszeit gibt es nicht -
    E-25 rechnet die Steuerrate ausdruecklich in die Prueftoleranz ein. */
inline constexpr int kDynamikSchritt = 8;

/** Das Auto-Gain-Gitter: 20 Hz bis 20 kHz in 1/12-Oktav-Schritten (§5.4
    Feinheit 1). log2(1000) = 9,966 Oktaven, aufgerundet auf 120
    Intervalle, also 121 Stellen. */
inline constexpr int    kAutoGainStellen = 121;
inline constexpr double kAutoGainVonHz   = 20.0;
inline constexpr double kAutoGainBisHz   = 20000.0;

/** NAK-311 R-311-14 (T3-15-09 Teil b, Karte U54): die Obergrenze des
    ANGEWANDTEN Auto-Gain-Ausgleichs in dB. EINSEITIG - gedeckelt wird nur die
    ANHEBUNG. Die Absenkungsseite bleibt, wie sie ist (dort wirkt weiter allein
    die BERICHTSgrenze `kBerichtAutoGainGrenzeDb`, M-114, M-67): die Gefahr, ueber
    die der User am 19.09.2026 entschieden hat, ist das Aufdrehen.

    Die Zahl ist nicht gewaehlt, sondern uebernommen: `v1.global.output_trim_db`
    reicht bis +24 dB (`eq-copilot/schemas/state/nakama-parameter-v2.json`).
    Nakama hebt automatisch nie weiter an, als der User selbst aufdrehen kann.
    Sie liegt 17,0 dB ueber der groessten Anhebung gewoehnlicher Arbeit
    (Low-Cut 500 Hz und High-Cut 2 kHz zusammen: +7,0 dB) und 18,99 dB unter der
    kleinsten gemessenen Gefahr (acht High-Cuts 20 Hz Q 0,707: +42,99 dB;
    dieselben mit Q 0,15: +150,46 dB) - linear Faktor 8,9. */
inline constexpr double kAutoGainDeckelDb = 24.0;

/** Der Deckel selbst, einzeln aufrufbar (311/M-113). STRIKT groesser
    entscheidet: `kAutoGainDeckelDb` und der naechste `double` darunter kommen
    BITGLEICH zurueck, erst der naechste darueber wird bitgenau auf die Grenze
    gesetzt. Kein `std::clamp`, keine Multiplikation, kein Runden (Muster
    `berichtsAutoGainDb`).

    Jeder Wert auf der Absenkungsseite kommt unveraendert zurueck, auch -0,0;
    +0,0 bleibt +0,0. NaN und +/-Inf erreichen diese Funktion nie - der
    Kurzschluss und die zwei Wachen von `leiteAutoGainAb` stehen davor. Kaeme
    doch ein NaN, faellt der Vergleich wie jeder Vergleich mit NaN, und der Wert
    kaeme unveraendert zurueck: gedeckelt wird nur, was nachweislich zu gross
    ist. */
constexpr double gedeckelterAutoGainDb (double roh) noexcept
{
    return roh > kAutoGainDeckelDb ? kAutoGainDeckelDb : roh;
}

//==============================================================================
/** Ein Band-Slot im gebauten Programm. */
struct BandProgramm
{
    bool        aktiv        { false };   ///< occupied UND enabled
    Kanalmodus  modus        { Kanalmodus::stereo };
    Filtertyp   typ          { Filtertyp::bell };

    /*  Der statische Weg: ein RBJ-Biquad, fest entworfen. `istEinheit`
        erlaubt das Ueberspringen ohne Rechnung - bei `gain_db` = 0 ist ein
        Bell BITGENAU 1 (b0 = a0, b1 = a1, b2 = a2), und ein uebersprungener
        Filter kostet auch keinen Zustand. */
    Biquad      statisch     {};
    bool        statischIstEinheit { false };

    /*  Der dynamische Weg: TPT-SVF, dessen Gain je `kDynamikSchritt`
        Samples neu gemischt wird. `grundG` und `q` bleiben dabei fest. */
    bool        dynamisch    { false };   ///< dynamic_enabled UND aktiv
    bool        nutztSvf     { false };   ///< dynamisch UND der Typ hat einen Gain
    double      grundG       { 0.0 };     ///< tan(pi*fc/fs), fuer den SVF-Neuentwurf
    double      q            { 1.0 };
    double      gainDb       { 0.0 };     ///< statischer Anteil
    SvfKoeffizienten svfRuhe {};          ///< SVF bei Auslenkung 0 (Startwert)

    /*  Der Detektor. `laeuft` ist false bei `sidechain_source` = none und
        bei `dynamic_range_db` = 0 - dann wird gar nichts gerechnet (M-20,
        M-22), nicht nur multipliziert. `detektor` und `huelle` sind trotzdem
        entworfen, sobald das Band einen Detektor hat (Sidechain nicht
        `none`): eine Rampe der Range ueber 0 braucht beide Seiten (W-3,
        E-29). */
    bool        detektorLaeuft { false };
    Sidechain   quelle       { Sidechain::none };   ///< der persistente Vertragswert (topologisch)
    Biquad      detektor     {};
    HuellkurveKoeffizienten huelle {};
    double      thresholdDb  { 0.0 };
    double      rangeDb      { 0.0 };

    /*  Die Klemmliste dieses Slots (M-12, M-23). Der PERSISTENTE Wert
        bleibt in beiden Faellen unberuehrt; geklemmt wird beim Bauen. */
    bool        geklemmtSidechain { false };   ///< priority_sidechain -> internal
    bool        gekapptFreq       { false };   ///< freq_hz > 0,45*fs
    double      freqHzWirksam     { 0.0 };     ///< nach der Kappung

    /*  NAK-311 W03 (R-311-1): die LEBENSZYKLUSKENNUNG dieses Slots. 0 heisst
        "nicht aktiv"; ein aktiver Slot traegt nie 0. Wie `generation` vergibt
        sie erst die Publikation (`DspKern::publiziereVorbau`), `baueProgramm`
        laesst sie auf 0. Gleiche Kennung in zwei Programmen desselben Pfades
        heisst: der Slot blieb ueber JEDE Publikation dazwischen belegt und
        topologisch gleich - auch ueber eine verdraengte, die der Audiothread
        nie gesehen hat. Nur Laufzeit: sie wird nie gespeichert und geht ueber
        keinen Draht. */
    std::uint64_t lebenszyklus { 0 };
};

//==============================================================================
/** Das vollstaendige, unveraenderliche Programm eines Pfades. */
struct DspProgramm
{
    double        samplerate  { 0.0 };
    std::uint64_t generation  { 0 };

    /*  NAK-311 (T3-16-04, R-311-3): die KANALZAHL des Busses, auf dem dieses
        Programm laeuft - 1 im Monobus, 2 im Stereobus (das Layout laesst nur
        diese beiden zu, `SondeProcessor::isBusesLayoutSupported`). Sie kommt
        aus `prepareToPlay` ueber `DspKern::bereiteVor` beziehungsweise
        `Transaktionskern::setzeSamplerate` in `baueProgramm`; die Vorgabe 2
        haelt jeden Aufrufer bitgleich, der sie nicht nennt. Nur
        `leiteAutoGainAb` liest sie: bei 1 rechnet die Ableitung die geordnete
        Monokaskade und wertet nur den ausgegebenen Kanal aus. LAUFZEIT des
        Hosts, kein Statefeld - sie wird nie gespeichert, geht ueber keinen
        Draht und erzeugt weder Revision noch Host-Dirty. */
    int           kanaele     { 2 };

    /*  NAK-311 W03 (R-311-1, §9 F-8): die PFADKENNUNG. Sie wechselt, sobald
        ein globales Feld von `rampenKompatibel` (eq_enabled, bypass,
        Samplerate, Mono-Bass-Stufe an oder aus) seit dem zuletzt publizierten
        Programm dieses Pfades gewechselt hat; dann startet jeder Slot kalt.
        Vergeben wie `lebenszyklus` erst bei der Publikation. */
    std::uint64_t pfadKennung { 0 };

    /*  Die beiden Schalter, die den Pfad ueberhaupt oeffnen. Beide sind im
        Vertrag `blockrand`, gehoeren also ins Programm und nicht in eine
        Rampe. */
    bool   eqEngagiert { false };   ///< v2.global.eq_enabled
    bool   hardBypass  { false };   ///< v1.global.bypass

    /*  Rampenziele. Sie stehen als dB beziehungsweise Verhaeltnis DANEBEN,
        weil der Unity-Kurzschluss am dB-Wert haengt und nicht am linearen
        Faktor: `pow(10, 0/20)` ist zwar exakt 1,0, aber eine Rampe, die auf
        1,0 zulaeuft, kann durch 0.99999994 gehen (M-02). */
    double inputTrimDb  { 0.0 }, inputTrimLin  { 1.0 };
    double outputTrimDb { 0.0 }, outputTrimLin { 1.0 };
    double mix          { 1.0 };
    double width        { 1.0 };
    double monoBassHz   { 0.0 };

    /*  Auto-Gain. `autoGainDb` wird IMMER gerechnet und ist immer lesbar
        (M-35); angewandt wird es nur bei `autoGainAn`.

        NAK-311 R-311-14 (T3-15-09 Teil b, Karte U54): `autoGainDb` traegt seit
        dem Aenderungssatz B den ANGEWANDTEN Wert, auf der Anhebungsseite also
        den gedeckelten. `autoGainLin` folgt ihm, der Kern faehrt ihn ueber
        seine Rampe, und `baueBericht` meldet ihn - drei Leser, EINE Zahl
        (M-117). Daneben haelt `autoGainRohDb` den UNGEDECKELTEN Wert lesbar,
        weil R4 "der abgeleitete Wert in dB ist lesbar" zusagt und ein Deckel,
        der ihn ueberschreibt, dem spaeteren Bedienpunkt die Moeglichkeit
        naehme, ehrlich zu zeigen, wie weit gedeckelt wurde (F-23). Greift der
        Deckel nicht, sind beide BITGLEICH. Beide sind Laufzeit: nie
        gespeichert, nie im `state_hash`, ueber keinen Draht. */
    bool   autoGainAn    { false };
    double autoGainDb    { 0.0 };
    double autoGainRohDb { 0.0 };
    double autoGainLin   { 1.0 };

    /*  Die M/S-Stufe (Width und Mono-Bass). `msStufeAktiv` ist false, wenn
        width == 1,0 UND monoBassHz == 0 - dann wird die Matrix gar nicht
        gerechnet und der Weg ist bitidentisch (M-30). */
    bool   msStufeAktiv { false };
    Biquad monoBassHochpass {};

    /*  NAK-311 (T3-01-01, Manifest §7.2 Punkt 3): der engagierte Pfad
        rechnet mit diesem Programm bauartbedingt die Identitaet - engagiert
        und nicht im Hard-Bypass, jedes aktive Band ein statischer
        Einheitsbiquad ohne SVF, M/S-Stufe aus, Input- und Output-Trim im
        Kurzschluss bei 0 dB. Der Kern liest es am Stueckbeginn fuer die
        Neutralpruefung (`DspKern::verarbeiteStueck`); ob er schreibt,
        entscheiden dort zusaetzlich der Ruhezustand der fuenf Rampen, der
        Uebergang und die Hoermatrix. Der abgeleitete Auto-Gain gehoert NICHT
        dazu: unter der Bandbedingung ist `autoGainDb` exakt 0,0, und
        angewandt wird er nur ueber seine Rampe, die die Pruefung ohnehin auf
        1,0 verlangt. Nach der RBJ-Formel ist schon ein 0-dB-Bell KEIN
        Einheitsbiquad (b1 = a1 != 0, B6 311/F-4): ein aktives Band mit 0 dB
        haelt den Kern schreibend. */
    bool   neutral { false };

    std::array<BandProgramm, (size_t) kSlots> baender {};

    /** Traegt ueberhaupt ein Band Arbeit? Wird fuer die Feldanwesenheit von
        `band_dynamic_gain_db` gebraucht (M-114) und fuer den Kurzschluss
        eines vollstaendig neutralen Programms. */
    bool irgendeinBandAktiv() const noexcept
    {
        for (const auto& b : baender) if (b.aktiv) return true;
        return false;
    }

    bool irgendeinBandDynamisch() const noexcept
    {
        for (const auto& b : baender) if (b.aktiv && b.dynamisch) return true;
        return false;
    }

    /** NAK-311 R-311-14 (§41.2 F-24): greift der Deckel in DIESEM Programm -
        und wirkt der Ausgleich ueberhaupt? Die Bedingung steht hier EINMAL;
        `DspKern::meldeProgramm` und `baueBericht` lesen beide sie, damit die
        zwei Melder nicht auseinanderlaufen koennen (M-117).

        Zwei Bedingungen, beide noetig (F-24): der Deckel greift (strikt
        groesser, dieselbe Richtung wie `gedeckelterAutoGainDb`) UND
        `v2.global.auto_gain` ist an. Ein gemeldeter Deckel ohne wirkenden
        Ausgleich waere eine Meldung ueber etwas, das niemand hoert
        (`CLAUDE.md`, "keine toten UI-Elemente"). Der Zustand haengt allein am
        Programm - er wird nie gehalten und faellt mit dem naechsten Programm
        zurueck (M-116). */
    bool autoGainGedeckelt() const noexcept
    {
        return autoGainAn && autoGainRohDb > kAutoGainDeckelDb;
    }
};

//==============================================================================
/** Baut ein Programm aus dem DTO. AUSSERHALB des Audiothreads (R9).

    `samplerate` muss > 0 sein. Der `DspSatz` gilt als bereits validiert
    (`nakama::parameter::validiere`); dieser Bau prueft ihn nicht erneut,
    sondern KAPPT nur, was samplerateabhaengig ist (Nyquist), und KLEMMT,
    was der Vertrag heute nicht liefern kann (priority_sidechain).

    `kanalzahl` ist die Kanalzahl des Busses (NAK-311 R-311-3). Sie landet
    unveraendert in `DspProgramm::kanaele` und wirkt ausschliesslich auf
    `leiteAutoGainAb`: 1 waehlt die Monoformel, jede andere Zahl die
    Zweikanalformel. Die Vorgabe 2 haelt jeden heutigen Aufrufer bitgleich. */
void baueProgramm (const nakama::parameter::DspSatz& satz, double samplerate,
                   std::uint64_t generation, DspProgramm& aus, int kanalzahl = 2);

/** Die Auto-Gain-Ableitung, einzeln aufrufbar (§5.4).

    Pink-gewichteter ENERGIEmittelwert der statischen Betragsantwort ueber
    die 121 Gitterstellen: `g_db = -10 * log10( (1/N) * SUM |H(f)|^2 )` mit
    `E(f) = 0,5 * (|H_L|^2 + |H_R|^2)`, `H_L = H_stereo * H_left * (H_M +
    H_S)/2` und `H_R = H_stereo * H_right * (H_M + H_S)/2`.

    Der Diagonalterm `(H_M + H_S)/2` ist die materialfreie Faltung der
    M/S-Matrix: ein IDENTISCHES Mid/Side-Paar faellt damit exakt auf den
    Stereo-Fall zusammen (M-37). Der Kreuzterm `(H_M - H_S)/2` geht nicht
    ein - ihn exakt zu behandeln hiesse, zwischen korreliertem und
    dekorreliertem Material zu waehlen, und genau diese Materialannahme
    schliesst R4 aus.

    Traegt das Programm KEIN aktives Band, ist das Ergebnis exakt 0,0
    (Kurzschluss statt Gitterlauf, M-36). Dynamische Anteile, Trims, Width,
    Mono-Bass und Mix gehen nicht ein (§5.4 Feinheit 5).

    NAK-311 R-311-3: die Formel oben ist die Stereo-/M-S-Naeherung - sie
    mittelt ZWEI Ausgangsseiten. Auf einem MONOBUS (`kanaele == 1`) gibt es
    keine zweite Seite. Dort rechnet die Ableitung stattdessen die geordnete
    Kaskade: je Gitterstelle startet das Paar `(a_L, a_R) = (1, 1)` - der
    Monoeingang, den der Kern in beide Komponenten legt -, die aktiven
    Baender wirken in SLOTREIHENFOLGE mit derselben Ruheantwort `H`
    (`stereo` auf beide, `left` auf `a_L`, `right` auf `a_R`, `mid` und
    `side` ueber dieselbe M/S-Rueckfuehrung wie `DspKern::verarbeiteBand`),
    und ausgewertet wird nur `a_L` - der Kanal, den der Kern im Monobus
    schreibt: `E(f) = |a_L(f)|^2`. Folgen: `right` und `side` ergeben exakt
    +0,0, `left`, `mid` und `stereo` die volle Kompensation, und in einer
    Kaskade zaehlt die Reihenfolge. Die Faltung waere hier falsch, nicht nur
    ungenau - der Kern KENNT das Material im Monobus (beide Komponenten sind
    gleich), und nur die Kaskade traegt den Weg, auf dem ein spaeteres
    `mid`-Band den gedachten rechten Kanal wieder nach links mischt.

    NAK-311 R-311-14 (Karte U54): der Rueckgabewert ist der ANGEWANDTE Wert -
    auf der Anhebungsseite durch `gedeckelterAutoGainDb` einseitig gedeckelt.
    Hier und nur hier: an dieser Stelle lesen Programm, Kern und Bericht
    dieselbe Zahl (M-117); ein Deckel erst am Rampenziel liesse drei Zahlen
    nebeneinander laufen. Beide Zweige decken an ihrer eigenen Rueckgabezeile,
    weil der Monozweig mit `return` endet.

    `ungedeckeltAus` nimmt, wenn gesetzt, den UNGEDECKELTEN Wert auf. JEDER
    Rueckweg belegt ihn: die fuenf Kurzschluss- und Wachwege mit 0,0, die zwei
    rechnenden mit dem abgeleiteten Wert. Er ist nie NaN und nie unendlich -
    die Wachen davor bleiben unveraendert. */
double leiteAutoGainAb (const DspProgramm& p, double* ungedeckeltAus = nullptr);

/** Die Gitterfrequenz einer Stelle 0..120. Oeffentlich, damit der Golden
    dieselben Stellen prueft, ohne sie abzuschreiben. */
double autoGainGitterHz (int stelle) noexcept;

/** Unterscheiden sich zwei Programme NUR in Werten, deren Vertragsspalte
    `wechsel = rampe` sagt (R8, Nacharbeit 1 B-4)?

    Dann rampt der Audiothread die laufende Bank auf die neuen Koeffizienten
    und uebernimmt ihren Filter- und Huellkurvenzustand; sonst blendet er
    ueber einen Crossfade zwischen zwei vollstaendigen Programmen. Topologisch
    sind `eq_enabled`, `bypass` (beide `blockrand`), je Slot `occupied` und
    `enabled` (blockrand), `type`, `channel_mode`, `dynamic_enabled` und
    `sidechain_source`, dazu das Ein- und Ausschalten der Mono-Bass-Stufe
    (Entscheid E-20). Laeuft im Audiothread: keine Allokation, keine
    Ausnahme.

    ZWEITE BEDINGUNG seit NAK-311 W03 (R-311-1): der Audiothread verlangt fuer
    den Rampenweg zusaetzlich gleiche `pfadKennung` und gleiche
    `lebenszyklus`-Kennung jedes aktiven Slots. Diese Funktion vergleicht nur
    die beiden Programme; die Kennungen tragen auch den Wechsel einer
    VERDRAENGTEN Zwischenpublikation, die der Audiothread nie gesehen hat.
    Sonst laeuft ein Crossfade, in dem die neue Bank den Zustand genau der
    Slots mit gleicher Kennung uebernimmt (`DspKern::blockrand`). */
bool rampenKompatibel (const DspProgramm& alt, const DspProgramm& neu) noexcept;

} // namespace nakama::dsp
