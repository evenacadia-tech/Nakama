// SONDE-014 Etappe D — die Befundzustaende auf der Gen-Seite (M-29 bis M-35).
//
// ── WARUM DIESES BEIN IN C++ LIEGT UND NICHT IM BROKER ────────────────────
//
// Abnahme U21 sagt: „SICHERHEIT wird nicht als Wert ausgegeben, sondern auf
// den Zustand des Befunds abgebildet — nur ein sicherer Befund ist handelbar
// (READY TO SEND, Audition und Draft erlaubt)." Die Abbildung rechnet der
// Broker (Etappe C, `hypothese::zustand_aus_sicherheit`); die SPERRE muss
// aber dort greifen, wo die Aktion angeboten wird, und das ist Gen. Ein
// Modell, das `darfDraft()` nicht kennt, haette eine ausgegraute Schaltflaeche
// und keinen Riegel — genau der Fall, den M-30 ausschliesst.
//
// Dieses Bein startet weder Broker noch Pipe. Die Snapshots laufen durch
// DENSELBEN handgeschriebenen Leser wie im Produkt
// (`SourcesModel::uebernehmeSessionSnapshot`).

#include <juce_core/juce_core.h>

#include "SourcesModel.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{
using Model = eqcop::SourcesModel;

int fehler = 0;
int bestanden = 0;

void pruefe (bool ok, const char* name, const juce::String& detail = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name;
    if (detail.isNotEmpty())
        std::cout << "  [" << detail.toRawUTF8() << "]";
    std::cout << '\n';
    ok ? ++bestanden : ++fehler;
}

std::string hex (unsigned long long wert)
{
    std::ostringstream s;
    s << std::hex << std::setfill ('0') << std::setw (32) << wert;
    return s.str();
}

const std::string kBindung = hex (0x11);
const std::string kSession = hex (0x22);
const std::string kMain = hex (1);
const std::string kSonde = hex (2);

/// Ein vertragsgueltiger Befund als JSON-Text.
///
/// Alle sechzehn Pflichtfelder aus `$defs/session_finding`. Die Faelle unten
/// veraendern GENAU EIN Feld gegenueber dieser Grundform — so faellt jede
/// Ablehnung an der Zeile, um die es geht, und nicht an einem Nachbarn.
juce::String befundText (const juce::String& id,
                         const juce::String& zustand,
                         const juce::String& konfidenzKlasse,
                         const juce::String& zusatz = {})
{
    juce::String t;
    t << "{\"finding_id\":\"" << id << "\","
      << "\"claim_class\":\"zusammenhang\","
      << "\"ursachenklasse\":\"quelle_resonanz\","
      << "\"target_metric\":\"band_pegel_db\","
      << "\"candidate_source\":\"" << juce::String (kSonde) << "\","
      << "\"band_hz\":{\"von\":96,\"bis\":100},"
      << "\"beobachtung\":{\"wert_db\":-3.5,\"gueltig\":true},"
      << "\"rang\":{\"bandpassung\":0.4,\"koinzidenz\":0.0,\"uplift\":0.2,"
      << "\"intent_relevanz\":0.5,\"wiederholbarkeit\":0.8,\"routingqualitaet\":1.0},"
      << "\"confidence\":{\"class\":\"" << konfidenzKlasse << "\",\"score\":0.65},"
      << "\"evidence_ids\":[\"" << juce::String (hex (0x1000)) << "\"],"
      << "\"next_test\":\"manueller_versuch\","
      << "\"zustand\":\"" << zustand << "\","
      << "\"intent_revision\":3,"
      << "\"likely_cause\":\"Die Sonde draengt im markierten Bereich.\","
      << "\"smallest_test\":\"Die Quelle kurz absenken und gegenhoeren.\","
      << "\"listen_for\":\"Ob der Master im markierten Bereich Luft bekommt.\"";
    if (zusatz.isNotEmpty())
        t << "," << zusatz;
    t << "}";
    return t;
}

/// Ein Snapshot mit einer Sonde und beliebig vielen Befunden.
juce::String snapshot (const juce::StringArray& befunde,
                       const juce::String& messqualitaet = "fresh")
{
    juce::String t;
    t << "{\"type\":\"session_snapshot\","
      << "\"session_epoch\":\"" << juce::String (kSession) << "\","
      << "\"broker_epoch\":\"" << juce::String (hex (0xbeef)) << "\","
      << "\"fuehrendes_main\":\"" << juce::String (kMain) << "\","
      << "\"mitglieder\":[{"
      << "\"adresse\":{\"logon_sid\":\"S-1-5-21-1-2-3-1001\","
      << "\"project_binding_id\":\"" << juce::String (kBindung) << "\","
      << "\"session_epoch\":\"" << juce::String (kSession) << "\","
      << "\"instance_id\":\"" << juce::String (kSonde) << "\","
      << "\"runtime_nonce\":\"" << juce::String (hex (0x42)) << "\"},"
      << "\"plugin_kind\":\"passive_probe\","
      << "\"frische\":{\"stale\":false,\"letzter_kontakt_ms\":10}"
      << "}]";
    juce::ignoreUnused (messqualitaet);
    if (! befunde.isEmpty())
        t << ",\"findings\":[" << befunde.joinIntoString (",") << "]";
    t << "}";
    return t;
}

/// Nimmt einen Snapshot mit AUSDRUECKLICH vorgegebener eingehender Fassung an.
///
/// NAK-213 (Befund 7 der Matrixpruefung 1): der Overload OHNE
/// Fassungsparameter setzt `kJsonSchemaMinor` selbst als eingehende Fassung
/// ein (`SourcesModel.h`). Ein Rotbeweis, der die Konstante zuruecksetzt,
/// senkte damit Eingangsminor UND Leserobergrenze gemeinsam, und der Vergleich
/// in `SourcesModel.cpp` saehe keine Abweichung. Deshalb steht die Zahl hier
/// als Parameter - die Faelle unten geben sie einzeln vor.
Model::SnapshotErgebnis uebernimm (Model& m, const juce::String& json,
                                   juce::String& fehlertext,
                                   std::uint8_t schemaMinor = 3)
{
    return m.uebernehmeSessionSnapshot (json.toStdString(), schemaMinor,
                                        Model::Uhr::now(), fehlertext);
}

/// Ein frisches Modell auf dem HEAP.
///
/// `SourcesModel` haelt einen `std::mutex` und ist damit weder kopier- noch
/// verschiebbar — ein Rueckgabewert als Objekt scheitert am geloeschten
/// Kopierkonstruktor. Der Heap ist hier ohnehin die Regel: NAK-175, der
/// MSVC-Standardstack ist 1 MiB, und dieses Bein legt bis zu einem Dutzend
/// Modelle je Abschnitt an.
std::unique_ptr<Model> frischesModell()
{
    auto m = std::make_unique<Model>();
    m->beginneSubscription (kBindung, kSession, kMain);
    return m;
}
} // namespace

int main()
{
    // Ungepuffert: stuerzt ein Fall ab, soll die Ausgabe zeigen, WO er stand.
    // Mit Pufferung geht die letzte Zeile verloren, und ein Absturz sieht aus
    // wie ein Programm, das nie angelaufen ist.
    std::cout.setf (std::ios::unitbuf);
    std::cout << "SONDE-014 Etappe D - Befundzustaende (M-29 bis M-35)\n";

    // ═══════════════════════════════════════════════════════════════════
    // M-29 · sicherheit_wird_auf_drei_zustaende_abgebildet
    // ═══════════════════════════════════════════════════════════════════
    //
    // Genau DREI Zustaende, und die Anzeige leitet keinen davon aus einem
    // Zahlenwert ab: sie liest das Feld. Ein vierter Wert macht den GANZEN
    // Snapshot ungueltig, statt still auf einen bekannten abgebildet zu
    // werden.
    {
        for (const char* z : { "ready_to_send", "more_data", "stale" })
        {
            auto m = frischesModell();
            juce::String f;
            const auto ergebnis =
                uebernimm (*m, snapshot ({ befundText (hex (0x900), z, "hoch") }), f);
            pruefe (ergebnis == Model::SnapshotErgebnis::uebernommen,
                    "M-29: der Zustand kommt als Feld an", juce::String (z) + " " + f);
            const auto sicht = m->sicht();
            pruefe (sicht.befunde.size() == 1 && sicht.befunde[0].zustand == z,
                    "M-29: und steht unveraendert im Modell");
        }
        auto m = frischesModell();
        juce::String f;
        pruefe (uebernimm (*m, snapshot ({ befundText (hex (0x900), "vierter", "hoch") }), f)
                    == Model::SnapshotErgebnis::ungueltig,
                "M-29: ein VIERTER Zustand faellt am ganzen Snapshot", f);
        pruefe (m->sicht().befunde.empty(),
                "M-29: und hinterlaesst keinen halben Befund");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-30 · more_data_bietet_keinen_draft / stale_bietet_keinen_draft
    // ═══════════════════════════════════════════════════════════════════
    //
    // Die Sperre liegt im MODELL. `darfDraft()` und `darfAudition()` sind
    // Funktionen des Befunds, keine Eigenschaft einer Schaltflaeche.
    {
        struct Fall { const char* zustand; bool erlaubt; };
        for (const auto& fall : { Fall { "ready_to_send", true },
                                  Fall { "more_data", false },
                                  Fall { "stale", false } })
        {
            auto m = frischesModell();
            juce::String f;
            uebernimm (*m, snapshot ({ befundText (hex (0x900), fall.zustand, "hoch") }), f);
            const auto sicht = m->sicht();
            pruefe (sicht.befunde.size() == 1
                        && sicht.befunde[0].darfDraft() == fall.erlaubt,
                    "M-30: SEND DRAFT haengt am Zustand", fall.zustand);
            pruefe (sicht.befunde.size() == 1
                        && sicht.befunde[0].darfAudition() == fall.erlaubt,
                    "M-30: HOLD TO AUDITION ebenso", fall.zustand);
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-35 · messqualitaet_und_befundsicherheit_sind_zwei_felder
    // ═══════════════════════════════════════════════════════════════════
    //
    // Die schaerfste Zeile dieser Etappe: `CONFIDENCE` unter dem Graphen ist
    // die MESSQUALITAET der Passage (`Zeile::messung`, aus `konfidenz.klasse`
    // des Belegs), die Sicherheit des Befunds ist etwas anderes. Ein Befund
    // mit `confidence.class = hoch`, dessen Zustand `more_data` sagt, bleibt
    // NICHT handelbar — und umgekehrt macht eine frische Messung an der
    // Quellenzeile keinen Befund handelbar.
    {
        auto m = frischesModell();
        juce::String f;
        uebernimm (*m, snapshot ({ befundText (hex (0x900), "more_data", "hoch") }), f);
        const auto sicht = m->sicht();
        pruefe (sicht.befunde.size() == 1
                    && sicht.befunde[0].confidenceKlasse == "hoch"
                    && ! sicht.befunde[0].darfDraft(),
                "M-35: `confidence.class = hoch` macht einen `more_data`-Befund "
                "NICHT handelbar", f);
        pruefe (sicht.quellen.size() == 1,
                "M-35: die Quellenzeile steht neben dem Befund");
        // Zwei Felder, zwei Quellen: der Zustand kommt aus dem Befund, die
        // Messqualitaet aus der Quellenzeile. Sie stehen an verschiedenen
        // Stellen des Modells und koennen sich nicht vertauschen.
        pruefe (sicht.befunde.size() == 1
                    && sicht.befunde[0].zustand == "more_data",
                "M-35: und der Zustand kommt aus dem Befund, nicht aus der Zeile");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-32 · alternative_ist_ein_eigener_befund_mit_eigenem_zustand
    // ═══════════════════════════════════════════════════════════════════
    //
    // Abnahme U21: Alternativen sind EIGENE Befunde in der Liste mit eigenem
    // Zustand. `alternatives` traegt ihre IDs — keinen Text.
    {
        const auto zweite = hex (0x901);
        const juce::String verweis =
            juce::String ("\"alternatives\":[\"") + juce::String (zweite) + "\"]";
        auto m = frischesModell();
        juce::String f;
        const auto ergebnis = uebernimm (
            *m,
            snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch", verweis),
                        befundText (zweite, "more_data", "mittel") }),
            f);
        pruefe (ergebnis == Model::SnapshotErgebnis::uebernommen,
                "M-32: zwei Befunde, einer verweist auf den anderen", f);
        const auto sicht = m->sicht();
        pruefe (sicht.befunde.size() == 2, "M-32: beide stehen in der Liste");
        pruefe (sicht.befunde.size() == 2
                    && sicht.befunde[0].alternatives.size() == 1
                    && sicht.befunde[0].alternatives[0] == zweite,
                "M-32: die Alternative ist eine ID, kein Anhang");
        pruefe (sicht.befunde.size() == 2
                    && sicht.befunde[1].zustand == "more_data"
                    && ! sicht.befunde[1].darfDraft(),
                "M-32: und sie traegt einen EIGENEN Zustand");
        // Ein Freitext an dieser Stelle faellt.
        auto n = frischesModell();
        juce::String g;
        pruefe (uebernimm (*n,
                           snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch",
                                                   "\"alternatives\":[\"vielleicht die Gitarre\"]") }),
                           g)
                    == Model::SnapshotErgebnis::ungueltig,
                "M-32: ein Freitext in `alternatives` faellt", g);
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-33 · beleg_ist_zone_und_zwei_kurven_kein_text
    // ═══════════════════════════════════════════════════════════════════
    //
    // Der Beleg ist die markierte Frequenzzone, nicht ein wiederholter Text.
    // Der Datenweg traegt deshalb ein Bandintervall und KEIN Belegtextfeld —
    // `exakteFelder` laesst gar keines zu.
    {
        auto m = frischesModell();
        juce::String f;
        uebernimm (*m, snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch") }), f);
        const auto sicht = m->sicht();
        pruefe (sicht.befunde.size() == 1 && sicht.befunde[0].bandVon == 96
                    && sicht.befunde[0].bandBis == 100,
                "M-33: die Zone ist ein Bandintervall des eingefrorenen Gitters");
        pruefe (sicht.befunde.size() == 1
                    && ! sicht.befunde[0].candidateSource.empty(),
                "M-33: und sie nennt ihre Quelle");
        auto n = frischesModell();
        juce::String g;
        pruefe (uebernimm (*n,
                           snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch",
                                                   "\"beleg\":\"weil es so klingt\"") }),
                           g)
                    == Model::SnapshotErgebnis::ungueltig,
                "M-33: ein Belegtextfeld faellt am Vertrag", g);
        // Die Bandraender von BEIDEN Seiten: 0 und 221 sind gueltig, ein
        // leeres Intervall und 222 nicht.
        auto rand = [&] (const char* intervall) {
            auto o = frischesModell();
            juce::String h;
            auto text = befundText (hex (0x900), "ready_to_send", "hoch");
            text = text.replace ("\"band_hz\":{\"von\":96,\"bis\":100}",
                                 juce::String ("\"band_hz\":") + intervall);
            return uebernimm (*o, snapshot ({ text }), h);
        };
        pruefe (rand ("{\"von\":0,\"bis\":1}") == Model::SnapshotErgebnis::uebernommen,
                "M-33: Band 0 ist ein gueltiger Index");
        pruefe (rand ("{\"von\":220,\"bis\":221}") == Model::SnapshotErgebnis::uebernommen,
                "M-33: und 221 ein gueltiges `bis`");
        pruefe (rand ("{\"von\":5,\"bis\":5}") == Model::SnapshotErgebnis::ungueltig,
                "M-33: ein LEERES Intervall faellt");
        pruefe (rand ("{\"von\":10,\"bis\":222}") == Model::SnapshotErgebnis::ungueltig,
                "M-33: und eines ueber dem Gitterrand ebenso");
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-34 · drei_zeilen_sind_datenfelder
    // ═══════════════════════════════════════════════════════════════════
    //
    // Genau DREI Zeilen, jede ein eigenes Feld. Die Anzeige setzt keine davon
    // aus mehreren Feldern zusammen, und eine vierte entsteht im Datenweg
    // nicht.
    {
        auto m = frischesModell();
        juce::String f;
        uebernimm (*m, snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch") }), f);
        const auto sicht = m->sicht();
        pruefe (sicht.befunde.size() == 1
                    && sicht.befunde[0].likelyCause.isNotEmpty()
                    && sicht.befunde[0].smallestTest.isNotEmpty()
                    && sicht.befunde[0].listenFor.isNotEmpty(),
                "M-34: alle drei Zeilen stehen als eigene Felder");
        pruefe (sicht.befunde.size() == 1
                    && sicht.befunde[0].nextTest == "manueller_versuch",
                "M-34: SMALLEST TEST bildet `next_test` ab - der Wert reist mit");
        auto vierte = frischesModell();
        juce::String g;
        pruefe (uebernimm (*vierte,
                           snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch",
                                                   "\"vierte_zeile\":\"noch ein Satz\"") }),
                           g)
                    == Model::SnapshotErgebnis::ungueltig,
                "M-34: eine VIERTE Zeile faellt am Vertrag", g);
        // Eine LEERE Zeile ist kein Feld, sondern ein Vertragsbruch.
        auto leer = frischesModell();
        juce::String h;
        auto text = befundText (hex (0x900), "ready_to_send", "hoch");
        text = text.replace ("\"listen_for\":\"Ob der Master im markierten Bereich Luft bekommt.\"",
                             "\"listen_for\":\"\"");
        pruefe (uebernimm (*leer, snapshot ({ text }), h)
                    == Model::SnapshotErgebnis::ungueltig,
                "M-34: eine leere Zeile faellt (minLength 1)", h);
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-87 · die Ausschluesse erreichen Gen
    // ═══════════════════════════════════════════════════════════════════
    //
    // Die Rechnung liegt in Etappe C, die Vertragsstelle in Etappe B. HIER
    // faellt die dritte Haelfte: sie kommen im Modell an, und ein neunter
    // Grund faellt.
    {
        const juce::String aus =
            juce::String ("\"ausschluesse\":[{\"candidate_source\":\"")
            + juce::String (hex (3)) + "\",\"grund\":\"coverage_fehlt\"}]";
        auto m = frischesModell();
        juce::String f;
        uebernimm (*m, snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch", aus) }), f);
        const auto sicht = m->sicht();
        pruefe (sicht.befunde.size() == 1
                    && sicht.befunde[0].ausschluesse.size() == 1
                    && sicht.befunde[0].ausschluesse[0].second == "coverage_fehlt",
                "M-87: der Ausschluss kommt mit seinem Grund an", f);
        const juce::String neunter =
            juce::String ("\"ausschluesse\":[{\"candidate_source\":\"")
            + juce::String (hex (3)) + "\",\"grund\":\"gefiel_mir_nicht\"}]";
        auto n = frischesModell();
        juce::String g;
        pruefe (uebernimm (*n,
                           snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch",
                                                   neunter) }),
                           g)
                    == Model::SnapshotErgebnis::ungueltig,
                "M-87: ein NEUNTER Grund faellt am ganzen Snapshot", g);
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-213 K-46 · fassung_3_kennt_die_neuen_ausschlussgruende_nicht
    // ═══════════════════════════════════════════════════════════════════
    //
    // 🔑 R7/M-77: der Leser waehlt die Fassung, er prueft nicht nur seine
    // eigene Fassungsobergrenze. Wuechse allein die Gruendeliste, naehme er
    // `screening_ueberboten` und `master_duplikat` auch in einem ausdruecklich
    // als Fassung 3 eingehenden Snapshot an - waehrend der Rust-Rueckbau
    // `v3_schema_minor_3_wurzel` denselben Snapshot abweist. Genau diese
    // Einseitigkeit verbietet M-77.
    {
        for (const char* grund : { "screening_ueberboten", "master_duplikat" })
        {
            const juce::String aus =
                juce::String ("\"ausschluesse\":[{\"candidate_source\":\"")
                + juce::String (hex (3)) + "\",\"grund\":\"" + grund + "\"}]";
            const auto text = snapshot ({ befundText (hex (0x900), "ready_to_send",
                                                      "hoch", aus) });
            // Die Namen stehen als eigene Variablen: `pruefe` nimmt einen
            // `const char*`, und ein temporaerer `juce::String` im Aufruf
            // waere ein Zeiger, dessen Lebensdauer man nachrechnen muesste.
            const std::string nameDrei = std::string ("K-46: ") + grund
                                       + " faellt in Fassung 3";
            const std::string nameVier = std::string ("K-46: ") + grund
                                       + " gilt in Fassung 4 und kommt an";
            auto drei = frischesModell();
            juce::String f;
            pruefe (uebernimm (*drei, text, f, 3) == Model::SnapshotErgebnis::ungueltig,
                    nameDrei.c_str(), f);
            auto vier = frischesModell();
            juce::String g;
            pruefe (uebernimm (*vier, text, g, 4) == Model::SnapshotErgebnis::uebernommen
                        && vier->sicht().befunde.size() == 1
                        && vier->sicht().befunde[0].ausschluesse.size() == 1
                        && vier->sicht().befunde[0].ausschluesse[0].second == grund,
                    nameVier.c_str(), g);
        }
        // Gegenprobe: der Fassung-3-Leser ist nicht einfach kaputt. Ein
        // BEKANNTER Grund gilt dort weiter.
        const juce::String bekannt =
            juce::String ("\"ausschluesse\":[{\"candidate_source\":\"")
            + juce::String (hex (3)) + "\",\"grund\":\"coverage_fehlt\"}]";
        auto drei = frischesModell();
        juce::String h;
        pruefe (uebernimm (*drei, snapshot ({ befundText (hex (0x900), "ready_to_send",
                                                          "hoch", bekannt) }), h, 3)
                    == Model::SnapshotErgebnis::uebernommen,
                "K-46 Gegenprobe: ein bekannter Grund gilt in Fassung 3 weiter", h);
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-213 K-50 · ausschlusslaenge_ist_an_die_fassung_gebunden
    // ═══════════════════════════════════════════════════════════════════
    //
    // 🔑 R6 mit der Praezisierung vom 08.09.2026: M-77 nennt "jede LAENGEN-
    // oder Enumgrenze". Bliebe die C++-Grenze unbedingt bei 64, naehme der
    // Leser einen Fassung-3-Snapshot mit 33 Ausschluessen an, den das
    // Fassung-3-Schema und der Rust-Rueckbau abweisen. Fuenf Raender, jeder
    // einzeln gemessen.
    {
        auto mitAusschluessen = [] (int anzahl) {
            juce::StringArray eintraege;
            for (int i = 0; i < anzahl; ++i)
                eintraege.add (juce::String ("{\"candidate_source\":\"")
                               + juce::String (hex (static_cast<unsigned long long> (0x2000 + i)))
                               + "\",\"grund\":\"coverage_fehlt\"}");
            return snapshot ({ befundText (
                hex (0x900), "ready_to_send", "hoch",
                juce::String ("\"ausschluesse\":[") + eintraege.joinIntoString (",") + "]") });
        };
        struct Rand { int anzahl; std::uint8_t minor; bool gueltig; const char* name; };
        const Rand raender[] = {
            { 32, 3, true,  "K-50: Fassung 3 nimmt 32 an (Altstand liest weiter)" },
            { 33, 3, false, "K-50: Fassung 3 weist 33 ab" },
            { 33, 4, true,  "K-50: Fassung 4 nimmt 33 an" },
            { 64, 4, true,  "K-50: Fassung 4 nimmt 64 an (SESSION_CLIENT_CAP)" },
            { 65, 4, false, "K-50: Fassung 4 weist 65 ab" },
        };
        for (const auto& r : raender)
        {
            auto m = frischesModell();
            juce::String f;
            const auto ergebnis = uebernimm (*m, mitAusschluessen (r.anzahl), f, r.minor);
            pruefe (ergebnis == (r.gueltig ? Model::SnapshotErgebnis::uebernommen
                                           : Model::SnapshotErgebnis::ungueltig),
                    r.name, f);
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-213 K-47 · rahmen_der_naechsten_fassung_faellt_am_leser
    // ═══════════════════════════════════════════════════════════════════
    //
    // 🔑 Der messende Rotbeweis fuer `kJsonSchemaMinor` (NB-3). Der Rahmen
    // gibt seine Fassung UNABHAENGIG vor; bliebe die Konstante hinter dem
    // Vertrag zurueck, faellt der Snapshot an der Fassungsobergrenze des
    // Lesers - waehrend der Aufnahmehelfer ohne Fassungsparameter dieselbe
    // Konstante auf BEIDEN Seiten einsetzte und die Abweichung nie sah.
    //
    // SONDE-015 N-12 (Manifest §10.7, §11): die unabhaengige Zahl stand hier
    // als Literal 4 und riss am Fassungsschritt 4 -> 5 der Etappe 2. Sie kommt
    // jetzt aus dem Vertrag selbst - `wire_envelope_schema_minor.aktuell` in
    // reservierte-nachrichten-v1.json, dieselbe Zahl, an die A5 die Konstante
    // bindet. Gemessen wird: die aktive Fassung passiert, die naechste hoehere
    // faellt, und die C++-Konstante steht auf der aktiven. Kein Literal reisst
    // mehr am naechsten Fassungsschritt.
    {
        const auto registerDatei = []
        {
            const juce::String relativ ("eq-copilot/schemas/v3/reservierte-nachrichten-v1.json");
            auto datei = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
            auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
            for (int i = 0; i < 10 && ! datei.existsAsFile() && ordner.exists(); ++i)
            {
                datei = ordner.getChildFile (relativ);
                ordner = ordner.getParentDirectory();
            }
            return datei;
        }();
        const auto aktuell = juce::JSON::parse (registerDatei)["wire_envelope_schema_minor"]["aktuell"];
        const bool gelesen = (aktuell.isInt() || aktuell.isInt64()) && (int) aktuell >= 1 && (int) aktuell < 255;
        const int aktiv = gelesen ? (int) aktuell : -1;
        pruefe (gelesen, "K-47: die aktive P1-Fassung steht im Register (wire_envelope_schema_minor.aktuell)",
                registerDatei.getFullPathName() + " = " + juce::String (aktiv));
        if (gelesen)
        {
            auto m = frischesModell();
            juce::String f;
            pruefe (uebernimm (*m, snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch") }),
                               f, (std::uint8_t) aktiv)
                        == Model::SnapshotErgebnis::uebernommen,
                    "K-47: ein Rahmen der aktiven Fassung passiert den Leser",
                    "Fassung " + juce::String (aktiv) + (f.isEmpty() ? juce::String() : ": " + f));
            auto n = frischesModell();
            juce::String g;
            pruefe (uebernimm (*n, snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch") }),
                               g, (std::uint8_t) (aktiv + 1))
                        == Model::SnapshotErgebnis::ungueltig,
                    "K-47 Gegenprobe nach oben: ein Rahmen der naechsten, unbekannten Fassung faellt am Leser",
                    "Fassung " + juce::String (aktiv + 1) + (g.isEmpty() ? juce::String() : ": " + g));
            pruefe ((int) nakama::ipc::kJsonSchemaMinor == aktiv,
                    "K-47: die C++-Fassungszahl steht auf der aktiven Registerfassung (A5 haelt denselben Text)",
                    "kJsonSchemaMinor = " + juce::String ((int) nakama::ipc::kJsonSchemaMinor)
                        + ", Register = " + juce::String (aktiv));
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-82 · NaN-Ehrlichkeit auf der Leseseite
    // ═══════════════════════════════════════════════════════════════════
    //
    // Eine Zahl ausserhalb ihrer Vertragsgrenzen faellt, und zwar an der
    // Grenze — nicht erst, wenn sie „offensichtlich" falsch ist.
    {
        auto mitScore = [&] (const char* score) {
            auto m = frischesModell();
            juce::String f;
            auto text = befundText (hex (0x900), "ready_to_send", "hoch");
            text = text.replace ("\"score\":0.65",
                                 juce::String ("\"score\":") + score);
            return uebernimm (*m, snapshot ({ text }), f);
        };
        pruefe (mitScore ("0") == Model::SnapshotErgebnis::uebernommen,
                "M-82: score 0 ist gueltig");
        pruefe (mitScore ("1") == Model::SnapshotErgebnis::uebernommen,
                "M-82: score 1 ebenso");
        pruefe (mitScore ("1.0001") == Model::SnapshotErgebnis::ungueltig,
                "M-82: knapp darueber faellt");
        pruefe (mitScore ("-0.0001") == Model::SnapshotErgebnis::ungueltig,
                "M-82: knapp darunter ebenso");
        pruefe (mitScore ("\"hoch\"") == Model::SnapshotErgebnis::ungueltig,
                "M-82: und ein String ist keine Zahl");
    }

    // ═══════════════════════════════════════════════════════════════════
    // Der Riegel selbst: `findings` in der Fassung 3, und nur dort
    // ═══════════════════════════════════════════════════════════════════
    //
    // 🔑 Der Defekt, den Etappe C hinterlassen hat: der Leser kannte
    // `findings` nicht, und Gen wies JEDEN Snapshot mit einem Befund ab. Die
    // Gegenprobe gehoert dazu — ein Leser der Fassung 2 nimmt das Feld
    // ausdruecklich NICHT an.
    {
        auto m = frischesModell();
        juce::String f;
        pruefe (m->uebernehmeSessionSnapshot (
                    snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch") })
                        .toStdString(),
                    2, Model::Uhr::now(), f)
                    == Model::SnapshotErgebnis::ungueltig,
                "Fassungsleiter: ein Leser der Fassung 2 lehnt `findings` ab", f);
        auto n = frischesModell();
        juce::String g;
        pruefe (uebernimm (*n, snapshot ({}), g) == Model::SnapshotErgebnis::uebernommen,
                "Abwesenheit heisst `diese Sitzung fuehrt keine Befunde`", g);
        pruefe (n->sicht().befunde.empty(), "und die Liste bleibt leer, nicht null");
    }

    // ═══════════════════════════════════════════════════════════════════
    // Eine neue Sitzung erbt die Befunde der alten NICHT
    // ═══════════════════════════════════════════════════════════════════
    {
        auto m = std::make_unique<Model>();
        m->beginneSubscription (kBindung, kSession, kMain);
        juce::String f;
        uebernimm (*m, snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch") }), f);
        pruefe (m->sicht().befunde.size() == 1, "Vorbedingung: ein Befund liegt vor");
        m->beginneSubscription (kBindung, hex (0x23), kMain);
        pruefe (m->sicht().befunde.empty(),
                "Sitzungswechsel: die Befunde der alten Sitzung sind fort - ihre "
                "Evidenz-IDs zeigten sonst ins Leere");
    }


    // ═══════════════════════════════════════════════════════════════════
    // M-36 / M-40 · der Maskierungswert haengt AM Befund
    // ═══════════════════════════════════════════════════════════════════
    //
    // Etappe E. Bis dahin stand `maskierung` zwar in der erlaubten Feldmenge,
    // wurde aber nicht geprueft — der einzige fail-open Zweig dieses Lesers.
    {
        const juce::String wert =
            juce::String ("\"maskierung\":{\"quelle_a\":\"") + juce::String (kMain)
            + "\",\"quelle_b\":\"" + juce::String (kSonde)
            + "\",\"band_von\":96,\"band_bis\":100,\"wert_db\":7.25,"
            + "\"gueltig\":true,\"herabgesetzt\":false}";
        auto m = frischesModell();
        juce::String f;
        pruefe (uebernimm (*m, snapshot ({ befundText (hex (0x900), "ready_to_send",
                                                       "hoch", wert) }), f)
                    == Model::SnapshotErgebnis::uebernommen,
                "M-36: der Maskierungswert kommt am Befund an", f);
        const auto sicht = m->sicht();
        pruefe (sicht.befunde.size() == 1 && sicht.befunde[0].maskierungVorhanden
                    && sicht.befunde[0].maskierungBandVon == 96
                    && sicht.befunde[0].maskierungBandBis == 100,
                "M-36: er benennt einen Frequenzbereich des bestehenden Gitters");
        pruefe (sicht.befunde.size() == 1
                    && sicht.befunde[0].maskierungQuelleA == kMain
                    && sicht.befunde[0].maskierungQuelleB == kSonde,
                "M-36: und die beiden Quellen, zwischen denen er gilt");
        pruefe (sicht.befunde.size() == 1 && sicht.befunde[0].maskierungGueltig
                    && ! sicht.befunde[0].maskierungHerabgesetzt,
                "M-37: `gueltig` und `herabgesetzt` sind zwei eigene Bits");

        // ⚠️ Abwesenheit ist etwas ANDERES als ein ungueltiger Wert.
        auto ohne = frischesModell();
        juce::String g;
        uebernimm (*ohne, snapshot ({ befundText (hex (0x900), "ready_to_send", "hoch") }), g);
        pruefe (ohne->sicht().befunde.size() == 1
                    && ! ohne->sicht().befunde[0].maskierungVorhanden,
                "M-36: Abwesenheit heisst `dieser Befund traegt keinen`");

        // Ein Objekt beliebiger Form faellt jetzt — vor Etappe E ging es durch.
        for (const char* kaputt : {
                 "\"maskierung\":{\"quelle_a\":\"kurz\",\"quelle_b\":\"kurz\",\"band_von\":0,\"band_bis\":1,\"wert_db\":0,\"gueltig\":true,\"herabgesetzt\":false}",
                 "\"maskierung\":{\"band_von\":0,\"band_bis\":1,\"wert_db\":0,\"gueltig\":true,\"herabgesetzt\":false}",
                 "\"maskierung\":{\"quelle_a\":\"00000000000000000000000000000001\",\"quelle_b\":\"00000000000000000000000000000002\",\"band_von\":5,\"band_bis\":5,\"wert_db\":0,\"gueltig\":true,\"herabgesetzt\":false}",
                 "\"maskierung\":{\"quelle_a\":\"00000000000000000000000000000001\",\"quelle_b\":\"00000000000000000000000000000002\",\"band_von\":0,\"band_bis\":1,\"wert_db\":500,\"gueltig\":true,\"herabgesetzt\":false}",
                 "\"maskierung\":{\"quelle_a\":\"00000000000000000000000000000001\",\"quelle_b\":\"00000000000000000000000000000002\",\"band_von\":0,\"band_bis\":1,\"wert_db\":0,\"gueltig\":\"ja\",\"herabgesetzt\":false}",
                 "\"maskierung\":{\"quelle_a\":\"00000000000000000000000000000001\",\"quelle_b\":\"00000000000000000000000000000002\",\"band_von\":0,\"band_bis\":1,\"wert_db\":0,\"gueltig\":true,\"herabgesetzt\":false,\"farbe\":\"rot\"}" })
        {
            auto k = frischesModell();
            juce::String h;
            pruefe (uebernimm (*k, snapshot ({ befundText (hex (0x900), "ready_to_send",
                                                           "hoch", kaputt) }), h)
                        == Model::SnapshotErgebnis::ungueltig,
                    "M-36: ein vertragswidriger Maskierungswert faellt", h);
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // M-38 / M-39 / M-80 · der Datenweg traegt keine Zeichenanweisung
    // ═══════════════════════════════════════════════════════════════════
    //
    // Die Markierung aendert weder Achse noch Ausschnitt, und ein Zoom kann aus
    // dem Datenweg gar nicht kommen: es gibt kein Feld dafuer. Das ist eine
    // Aussage ueber den VERTRAG, nicht ueber die Absicht des Erzeugers.
    {
        for (const char* optik : { "\"maskierung\":{\"quelle_a\":\"00000000000000000000000000000001\",\"quelle_b\":\"00000000000000000000000000000002\",\"band_von\":0,\"band_bis\":1,\"wert_db\":0,\"gueltig\":true,\"herabgesetzt\":false,\"zoom\":true}",
                                   "\"achse\":{\"von\":20,\"bis\":20000}",
                                   "\"zoom\":true",
                                   "\"farbe\":\"#ff0000\"" })
        {
            auto m = frischesModell();
            juce::String f;
            pruefe (uebernimm (*m, snapshot ({ befundText (hex (0x900), "ready_to_send",
                                                           "hoch", optik) }), f)
                        == Model::SnapshotErgebnis::ungueltig,
                    "M-38/M-39/M-80: eine Zeichenanweisung faellt am Vertrag", f);
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-214 R3/R8 · das Verbindungsende (V-23, V-26, V-28, V-29)
    // ═══════════════════════════════════════════════════════════════════
    //
    // R3 Satz 1: `controlEnde` setzt JEDEN Befund auf `stale`. Die Liste
    // bleibt stehen - sie war zuletzt wahr -, aber keiner ihrer Befunde ist
    // noch handelbar, und `findingsOffen` faellt mit (R8, M-84). Der Rueckweg
    // ist der naechste vollstaendige Snapshot (verbinden ↔ trennen).
    {
        // V-23 — zwei offene Befunde derselben Quelle, einer handelbar.
        auto m = frischesModell();
        juce::String f;
        const auto ergebnis =
            uebernimm (*m, snapshot ({ befundText (hex (0x921), "ready_to_send", "hoch"),
                                       befundText (hex (0x922), "more_data", "mittel") }), f);
        const auto vorher = m->sicht();
        pruefe (ergebnis == Model::SnapshotErgebnis::uebernommen
                    && vorher.befunde.size() == 2
                    && vorher.befunde[0].darfDraft()
                    && ! vorher.quellen.empty()
                    && vorher.quellen.front().findingsOffen == 2,
                "NAK-214 V-23: Vorbedingung - zwei offene Befunde, einer handelbar", f);

        m->controlEnde();
        const auto nachher = m->sicht();
        const bool alleStale =
            std::all_of (nachher.befunde.begin(), nachher.befunde.end(),
                         [] (const auto& b) {
                             return b.zustand == "stale" && ! b.darfDraft()
                                 && ! b.darfAudition();
                         });
        pruefe (nachher.befunde.size() == 2 && alleStale,
                "NAK-214 V-23: controlEnde macht JEDEN Befund stale - die Liste bleibt");
        pruefe (std::all_of (nachher.quellen.begin(), nachher.quellen.end(),
                             [] (const auto& q) { return q.findingsOffen == 0; }),
                "NAK-214 V-23: und nullt den Zaehler (R8)");

        // V-26 — der Rueckweg: ein vollstaendiger Folge-Snapshot ersetzt die
        // Liste als Ganzes, und JEDER Zustand kommt vom Broker.
        juce::String g;
        const auto zurueck =
            uebernimm (*m, snapshot ({ befundText (hex (0x921), "ready_to_send", "hoch") }), g);
        const auto danach = m->sicht();
        pruefe (zurueck == Model::SnapshotErgebnis::uebernommen
                    && danach.befunde.size() == 1
                    && danach.befunde[0].zustand == "ready_to_send"
                    && danach.befunde[0].darfDraft()
                    && danach.quellen.front().findingsOffen == 1,
                "NAK-214 V-26: der Folge-Snapshot hebt den stale-Zustand auf", g);
    }
    {
        // V-28 — ein Snapshot mit FREMDER Sitzungsepoche wird verworfen; die
        // Befunde der eigenen Sitzung bleiben unveraendert, nicht stale. Ein
        // fremder Snapshot ist kein Ereignis der eigenen Sitzung.
        auto m = frischesModell();
        juce::String f;
        uebernimm (*m, snapshot ({ befundText (hex (0x931), "ready_to_send", "hoch") }), f);
        juce::String fremd = snapshot ({ befundText (hex (0x932), "more_data", "mittel") });
        fremd = fremd.replace ("\"session_epoch\":\"" + juce::String (kSession) + "\"",
                               "\"session_epoch\":\"" + juce::String (hex (0x99)) + "\"");
        juce::String g;
        const auto ergebnis = uebernimm (*m, fremd, g);
        const auto s = m->sicht();
        pruefe (ergebnis != Model::SnapshotErgebnis::uebernommen
                    && s.befunde.size() == 1
                    && s.befunde[0].zustand == "ready_to_send"
                    && s.befunde[0].darfDraft(),
                "NAK-214 V-28: ein fremder Snapshot laesst die Befunde stehen", g);
    }
    {
        // V-29 — der Zahlenrand der leeren Liste: `controlEnde` ohne Befunde
        // ist folgenlos, und die Neuberechnung aus R8 laeuft ins Leere.
        auto m = frischesModell();
        juce::String f;
        uebernimm (*m, snapshot ({}), f);
        pruefe (m->sicht().befunde.empty(),
                "NAK-214 V-29: Vorbedingung - die Sicht fuehrt keine Befunde", f);
        m->controlEnde();
        const auto s = m->sicht();
        pruefe (s.befunde.empty()
                    && std::all_of (s.quellen.begin(), s.quellen.end(),
                                    [] (const auto& q) { return q.findingsOffen == 0; }),
                "NAK-214 V-29: controlEnde ohne Befunde ist folgenlos");
    }

    std::cout << "SONDE-014 Befund und Maskierung: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
}
