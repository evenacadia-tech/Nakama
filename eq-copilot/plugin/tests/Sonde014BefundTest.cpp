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

Model::SnapshotErgebnis uebernimm (Model& m, const juce::String& json,
                                   juce::String& fehlertext)
{
    return m.uebernehmeSessionSnapshot (json.toStdString(), 3,
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

    std::cout << "SONDE-014 Befund und Maskierung: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
}
