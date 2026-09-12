// EqCopilotProcessor — State: speichern, laden, Migration, Bindung, Quellenintent.
//
// NAK-225/S25d (09.09.2026), herausgeloest aus src/PluginProcessor.cpp. Inhalt:
//
//   getStateInformation / setStateInformation
//                        Der Vertrag mit dem Host. setStateInformation traegt
//                        die Migration und den Read-only-Rueckweg.
//   holeSensorId, holeRolle, holeLabel, holePaarId, holeZustandKopie,
//   holeStateHerkunft, holeStateGrund, holeStateFremdesMajor, stateNurLesen,
//   holeKlassifikation, spiegleKlassifikation, darfBrokerStarten
//                        Lesende Sicht auf denselben Zustand, jeweils unter
//                        `bindungMutex`.
//   neueSensorId, setzeBindung
//                        Schreibende Wechsel an Identitaetsfeldern des States
//                        (nicht an der eingefrorenen Bundle-Identitaet, die
//                        liegt in eq-copilot/identity/ und wird hier nie
//                        beruehrt).
//   merkeManuellePassage, vergissManuellePassage, manuellePassagen
//   setzeQuellenrolle, entferneQuellenrolle, schuetzeQuelle,
//   hebeQuellenschutzAuf, setzeQuellenbeziehung,
//   speichereQuellenGleichrangigkeit, entferneQuellenbeziehung,
//   sourceIntents, intentSchutzangaben, intentBeziehungen,
//   intentBestandRevision, wirkendeQuellenrolle, entmaskierungErlaubt
//                        Der musikalische Intent im MainProjectState.
//
// Zwei Invarianten binden diese Datei (CLAUDE.md):
//
//   State bleibt verlustfrei. Unbekannte Major-Versionen oder unzulaessige
//   Matrizen werden read-only mit den Originalbytes gehalten; ein erneutes
//   Laden waescht nichts.
//
//   Jede persistente Aenderung meldet dem Host Dirty-State. Die Meldung selbst
//   steht in prozessor/Hostbruecke.cpp (meldeHostDirty); hier stehen die
//   Aenderungen, die sie ausloesen. Speichern und Laden gehoeren in denselben
//   Aenderungssatz — beide stehen deshalb in dieser Datei.
//
// Kein Pfad dieser Datei laeuft auf dem Audio-Thread.

#include "../PluginProcessor.h"
#include "Intern.h"
#include <algorithm>

namespace eqcop
{

using prozessor_intern::uuidHex32;

bool EqCopilotProcessor::neueSensorId()
{
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen)
            return false;
        zustand.common.instanceId = juce::Uuid().toString();
    }
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    pipe.reconnect();
    controlV3.reconnect();
    return true;
}

void EqCopilotProcessor::getStateInformation (juce::MemoryBlock& ziel)
{
    // 🔑 NAK-246 D3 (R-D3; Manifest Paragraph 3.3 M-10, 5.3 Feinheit 1a,
    // Abweichung 2 in 5.10): das Speichern ist selbst ein Drain der
    // bestaetigten Sources-Befehle - "spaetestens beim Speichern".
    //
    // Bis hierher serialisierte diese Methode `zustand`, wie er war; ein vom
    // Broker bestaetigter Join oder Unbind, den noch kein Editor-Tick
    // angewandt hatte, fehlte im gespeicherten Projekt, und der Reload leerte
    // die Warteliste (Auditbefund D3). Jetzt: `sourcesDrainMutex` nehmen,
    // die bestaetigten Befehle abholen, unter `bindungMutex` anwenden und
    // UNTER DERSELBEN Sperre serialisieren - die Bytes tragen damit genau den
    // Stand nach den bestaetigten Befehlen. Modell, Host-Dirty und Revision
    // folgen je geaendertem Befehl genau einmal NACH der Freigabe: ein
    // Hostaufruf unter einer eigenen Sperre ist eine Sperrenordnung, die
    // dieses Projekt nirgends fuehrt (unten, Etappe A).
    std::size_t geaendert = 0;
    std::uint64_t generationBeimAbholen = 0;
    {
        std::lock_guard<std::mutex> drain (sourcesDrainMutex);
        const auto befehle = bestaetigteSourcesCommandsAbholen();
        // NAK-246 Abschluss Nacharbeit 1 (R-A1 Punkt 2 und 3): derselbe
        // Generationsvergleich wie im Rahmen (`Ipc.cpp`). Hier ist er trivial
        // erfuellt - Speichern und Laden ruft der Host auf DEMSELBEN
        // Message-Thread, zwischen Abholen und Anwendung kann kein Reload
        // liegen. Er wird trotzdem gefahren, damit kein Drain die Regel
        // umgeht; `speichere` laeuft in jedem Fall.
        generationBeimAbholen = reloadGeneration.load();
        std::lock_guard<std::mutex> l (bindungMutex);
        if (reloadGeneration.load() == generationBeimAbholen)
        {
            for (const auto& befehl : befehle)
                if (wendeSourcesCommandAnUnterBindung (befehl))
                    ++geaendert;
        }
        else if (! befehle.empty())
            sourcesBatchNachReloadVerworfen.fetch_add (1);
        nakama::state::speichere (zustand, ziel);
    }
    for (std::size_t i = 0; i < geaendert; ++i)
        meldeSourcesMitgliederNachBefehl (generationBeimAbholen);
}

void EqCopilotProcessor::setStateInformation (const void* daten, int groesse)
{
    if (daten == nullptr || groesse <= 0)
        return;

    nakama::state::Zustand geladen;
    const auto ergebnis = nakama::state::lade (daten, (size_t) groesse, bundleVertrag(), geladen);
    if (ergebnis == nakama::state::LadeErgebnis::ignoriert)
        return;   // fremder Baumtyp / Muell: Zustand bleibt (wie seit 0.1)
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        ausstehendeSourcesCommands.clear();
        bestaetigteSourcesCommands.clear();
    }
    // 🔑 NAK-181 R3 (G4-Befund V03, M-50): der Projektwechsel beendet den
    // Vergleichszustand — VOR dem Tausch von `zustand`.
    //
    // `versuchKopfJson` baut den Kopf aus dem aktuellen `zustand`. Zwischen
    // Tausch und Leeren gaebe es sonst ein Fenster, in dem ein Handgriff die
    // alte `experiment_id` unter der NEUEN `project_binding_id` serialisiert —
    // genau das, was M-50 mit „keine stillschweigende Fortsetzung einer
    // Messung ueber den Neustart hinweg" ausschliesst.
    //
    // Der Zug laeuft in BEIDEN Zweigen (`nurLesen` und Vollrestore): ein
    // read-only geladener State ist genauso ein anderes Projekt.
    vergleichszustandLeeren();

    if (ergebnis == nakama::state::LadeErgebnis::nurLesen)
    {
        // Keine vertrauenswuerdige Identitaet ⇒ keine Anmeldung beim Broker.
        // ERST stoppen, DANN tauschen: ein gerade laufender (Re-)Connect liest
        // den Zustand im hello-Lambda - nach dem Tausch waere das ein hello
        // mit leerer instance_id (T2-Befund SONDE-006).
        pipe.stop();
        std::uint64_t generation = 0;
        {
            std::lock_guard<std::mutex> l (bindungMutex);
            zustand = geladen;
            // 🔑 NAK-246 Abschluss Nacharbeit 1 (R-A1 Punkt 1, M-38/M-39): die
            // Reload-Generation wechselt im SELBEN Block wie `zustand` - ein
            // Drain, der seinen Batch oder seine Kopie vorher abgeholt hat,
            // sieht danach eine andere Generation und faellt ganz aus. Der
            // read-only-Zweig ist genauso ein Projektwechsel wie der
            // Vollrestore (dieselbe Begruendung wie `vergleichszustandLeeren`);
            // seit R-A1 Punkt 1' ist er eigens rot belegt (M-39, Lauf `m39ro`).
            generation = reloadGeneration.fetch_add (1) + 1;
            // §53.5: read-only ist kein vollstaendiger State-Restore. Zurueck
            // auf neutral - auch aus einer frueheren positiven Klassifikation.
            lebenslauf.stateRestauriert (ergebnis, geladen);
            spiegleKlassifikation();
        }
        // R-A1 Punkt 4' (c): genau die Generation dieses Reloads geht mit -
        // `projektReload` bleibt ausserhalb von `bindungMutex` (Sperrenordnung
        // Paragraph 10.2 Punkt 1 unveraendert).
        sourcesModel.projektReload ({}, generation);
        v3StateRevision.fetch_add (1);
        controlV3.reconnect();
        return;
    }

    std::uint64_t generation = 0;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        zustand = geladen;
        // R-A1 Punkt 1, zweiter Zweig (M-38/M-39); der Wert NACH `fetch_add` ist
        // die Generation, die `projektReload` dem Modell mitgibt (Punkt 4' (c)).
        generation = reloadGeneration.fetch_add (1) + 1;
        // §53.5: JETZT, nach vollstaendigem Restore, darf klassifiziert
        // werden - Schema-1 `sensor|pre|post` ist zu `legacy` migriert,
        // Schema-1 `hub` und ein bestaetigter Schema-2-Main-State zu `main`.
        lebenslauf.stateRestauriert (ergebnis, geladen);
        spiegleKlassifikation();
    }
    sourcesModel.projektReload (geladen.mainProjectMitglieder, generation);
    pipe.start();       // No-Op, wenn sie laeuft; hebt einen frueheren read-only-Stopp auf
    pipe.reconnect();   // frisches hello mit der geladenen Bindung
    v3StateRevision.fetch_add (1);
    controlV3.reconnect();
    // Kein Host-Dirty: Laden und Migration sind keine Aenderung des Users.
}

juce::String EqCopilotProcessor::holeSensorId() const { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.instanceId; }
juce::String EqCopilotProcessor::holeRolle() const    { std::lock_guard<std::mutex> l (bindungMutex); return nakama::state::v2Rolle (zustand.common); }
juce::String EqCopilotProcessor::holeLabel() const    { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.label; }
juce::String EqCopilotProcessor::holePaarId() const   { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.pairId; }

bool EqCopilotProcessor::stateNurLesen() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.nurLesen;
}

nakama::state::Herkunft EqCopilotProcessor::holeStateHerkunft() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.herkunft;
}

juce::String EqCopilotProcessor::holeStateGrund() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.grund;
}

int EqCopilotProcessor::holeStateFremdesMajor() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.fremdesMajor;
}

nakama::state::Zustand EqCopilotProcessor::holeZustandKopie() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand;
}

// ── Lifecycle-Klassifikation (§53.5) ───────────────────────────────────────

void EqCopilotProcessor::spiegleKlassifikation()
{
    // Aufrufer haelt `bindungMutex`. Der Audiothread liest ausschliesslich
    // diese Atomic; er befragt den Automaten nie (§53.5: "Klassifikation,
    // Spawn und Pipe-I/O liegen nie im Audiocallback").
    // Der Store bleibt bewusst seq_cst (Vorgabe) wie `editorOffen` und
    // `echtzeitOk` daneben: er laeuft nie im Audiothread, die Ordnung kostet
    // hier nichts, und eine dritte Ordnungsregel im selben Zustandsblock waere
    // eine Frage, die ein Leser jedes Mal neu beantworten muesste. Gelesen
    // wird im processBlock relaxed - dort haengt kein anderer Wert daran.
    istMainKlassifiziert.store (lebenslauf.audioAusnahmeErlaubt());
}

nakama::state::Klassifikation EqCopilotProcessor::holeKlassifikation() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return lebenslauf.klassifikation();
}

bool EqCopilotProcessor::darfBrokerStarten() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return lebenslauf.darfBrokerStarten();
}

bool EqCopilotProcessor::setzeBindung (const juce::String& r, const juce::String& lbl, const juce::String& p)
{
    // Dieselben Grenzen wie der einzige UI-Aufrufer. Sie gehoeren auch an die
    // API-Kante: der State-Leser beweist seinen Writer-Headroom gegen genau
    // diese Grenzen; ein kuenftiger Caller darf sie nicht umgehen.
    if (lbl.length() > 120 || p.length() > 60)
        return false;

    nakama::state::Klasse klasse;
    nakama::state::Messposition position;
    if (! nakama::state::ausV2Rolle (r, klasse, position))
        return false;

    std::vector<nakama::state::MainProjectMitglied> mainMitglieder;
    std::uint64_t generation = 0;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen)
            return false;
        auto neu = zustand.common;
        neu.klasse = klasse;
        neu.position = position;
        neu.label = lbl;
        neu.pairId = p;
        // Die Projektbindung entsteht nur an diesem sichtbaren User-Akt.
        // Frischzustand und Migration bleiben leer; der State selbst wird
        // danach zur autoritativen, persistierten Quelle fuer alle Clients.
        if (klasse == nakama::state::Klasse::main
            && neu.projectBindingId.isEmpty())
            neu.projectBindingId = juce::String (uuidHex32());
        if (neu == zustand.common)
            return false;   // keine Aenderung: kein Dirty, kein Reconnect-Geflacker
        zustand.common = neu;
        if (klasse != nakama::state::Klasse::main)
            zustand.mainProjectMitglieder.clear();
        mainMitglieder = zustand.mainProjectMitglieder;
        // R-A1 Punkt 4' (b): die Generation, fuer die diese Kopie gilt - im
        // SELBEN Block wie die Kopie gelesen.
        generation = reloadGeneration.load();

        // §53.5, dritter Punkt: "leerer, nie gespeicherter Altstate → Main
        // erst nach geoeffnetem Editor UND expliziter Initialisierung". Genau
        // hier ist dieser Akt - der einzige Aufrufer von `setzeBindung` ist
        // die Rollenwahl im Editor (PluginEditor.cpp), und der Automat
        // verlangt zusaetzlich selbst einen offenen Editor. Ein Scannerlauf
        // kann ihn nicht ausloesen: er bedient nichts und oeffnet nichts.
        //
        // Der Weg gilt in BEIDE Richtungen: stellt der User `hub` zurueck auf
        // `sensor`, faellt die Klassifikation auf `legacy`. Sonst behauptete
        // der Automat etwas anderes als der Stand, den dieselbe Instanz im
        // naechsten Projekt-Save schreibt.
        lebenslauf.expliziteInitialisierung (zustand);
        spiegleKlassifikation();
    }
    // Dieselbe Wache wie bei den Handgriffen in `Ipc.cpp`: Rollenwahl und
    // `setStateInformation` laufen heute beide auf dem Message-Thread, ein Reload
    // kann zwischen Block und Publikation nicht liegen. Kommt er doch, ist die
    // Publikation ganz unterblieben - kein Dirty, keine Revision; der geladene
    // State ist die Wahrheit. Der Reconnect laeuft in beiden Faellen: er haengt
    // an der neuen Bindung, nicht an der Publikation.
    if (sourcesModel.setzePersistenteMitglieder (mainMitglieder, generation))
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
    }
    else
        sourcesNachfuehrungNachReloadUnterblieben.fetch_add (1);
    pipe.reconnect();
    controlV3.reconnect();
    return true;
}

bool EqCopilotProcessor::merkeManuellePassage (const juce::String& passageId,
                                               const juce::String& label,
                                               juce::int64 projektStart,
                                               juce::int64 projektEnde)
{
    // Die Riegel VOR dem Schloss: sie brauchen keinen Zustand und halten die
    // kritische Strecke kurz.
    if (! nakama::ipc::istHex32 (passageId.toStdString()) || label.length() > 120)
        return false;
    if (projektStart < 0 || projektEnde <= projektStart)
        return false;

    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        if (static_cast<int> (zustand.manuellePassagen.size()) >= nakama::state::maxManuellePassagen)
            return false;
        const auto gefunden = std::find_if (
            zustand.manuellePassagen.begin(), zustand.manuellePassagen.end(),
            [&] (const auto& s) { return s.passageId == passageId; });
        if (gefunden != zustand.manuellePassagen.end())
            return false;
        zustand.manuellePassagen.push_back ({ passageId, label, projektStart, projektEnde });
    }
    // 🔑 SONDE-013 Nacharbeit 2 (Befund R03, M-03/M-25): die gespeicherte
    // Passage erreicht die ENGINE. Bis hierher blieb sie Plugin-State, und die
    // Passagenmetriken liefen weiter seit der letzten Transportgrenze — eine
    // leise Passage nach einem lauten Abschnitt erbte dessen Spitze.
    //
    // Der Nachrichtenthread fasst die Engine nicht an; er hinterlegt den Wunsch
    // und der Analyseworker loest ihn unter seiner Steuersperre ein.
    bindePassagenfenster (passageId, projektStart, projektEnde);
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    return true;
}

bool EqCopilotProcessor::vergissManuellePassage (const juce::String& passageId)
{
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        const auto gefunden = std::find_if (
            zustand.manuellePassagen.begin(), zustand.manuellePassagen.end(),
            [&] (const auto& s) { return s.passageId == passageId; });
        if (gefunden == zustand.manuellePassagen.end())
            return false;
        zustand.manuellePassagen.erase (gefunden);
    }
    // Der Gegenpfad zu `merkeManuellePassage`: die Engine loest die Bindung
    // wieder — danach gilt wieder die Transportepoche als Fenster, also der
    // Fall „der User hat keine Passage markiert" (Befund R03).
    //
    // 🔑 Nacharbeit 3 (Befund C3): NUR, wenn diese Passage die gebundene ist.
    // Bis dahin loeschte jedes Vergessen bedingungslos das globale Fenster und
    // stoppte den Pegel — auch den einer ganz anderen, gerade laufenden
    // Passage. Der State haelt bis zu 64 Passagen; ein globaler Slot ohne
    // Zuordnung war die eigentliche Ursache.
    loesePassagenfenster (passageId);
    // Das Vergessen meldet Dirty wie das Merken. Ein Loeschen, das der Host
    // nicht mitbekommt, kaeme beim naechsten Oeffnen zurueck.
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    return true;
}

//==============================================================================
// SONDE-013 M-40 bis M-51, Nacharbeit 2 (Befund R06): der Experimentpfad.
//
// 🔑 `Vergleichspegel` und `Blindvergleich` waren uebersetzt und im Produkt
// UNBENUTZT. `nichtEndlicheSamples()` hatte ausserhalb der C++-Tests keinen
// Leser: ein nichtendliches Sample im Vergleichsmaterial verriegelte lokal den
// Gain und blieb im Produkt ungezaehlt — genau die stille Beschoenigung, die
// M-07 und CLAUDE.md ausschliessen.
//
// Diese Schicht ist MODELL und Nachrichtenweg, kein sichtbares Element: die
// Bedienfragen P-01 bis P-06 gehoeren dem User (§4.2).

std::vector<nakama::state::ManuellePassage> EqCopilotProcessor::manuellePassagen() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.manuellePassagen;
}

// ── SONDE-014 Etappe A: der musikalische Intent im Produktpfad ─────────────
//
// Die sieben Handgriffe teilen sich eine Form, und die Form IST die Zusage:
//
//   Schloss nehmen -> read-only und Klasse pruefen -> die reine Funktion aus
//   `NakamaState.cpp` rufen -> Schloss loesen -> NUR bei echter Aenderung
//   Host-Dirty melden und die v3-Revision heben.
//
// Warum das Melden ausserhalb des Schlosses steht: `updateHostDisplay` ruft
// in den Host. Ein Hostaufruf unter einem eigenen Mutex ist eine
// Sperrreihenfolge, die dieses Projekt nirgends fuehrt.
//
// Warum ein No-op nichts meldet (M-13): ein Dirty ohne Aenderung markiert das
// Projekt als ungespeichert, obwohl sich nichts geaendert hat - und ein
// abgewiesener Wert darf erst recht nichts melden.

// ═══════════════════════════════════════════════════════════════════════════
// SONDE-014 Etappe G: der Assistentenschritt am PRODUKTPFAD (§46.1, M-55 ff.)
// ═══════════════════════════════════════════════════════════════════════════
//
// ── DIE REIHENFOLGE JEDER AENDERUNG ────────────────────────────────────────
//
// Zustandsmaschine unter dem Bindungsschloss → Host-Dirty → Wire. Das ist
// dieselbe Ordnung wie beim Intent (M-13, M-85): ein Wire-Write ist kein
// persistenter Vorgang und steht deshalb NACH dem Dirty; er laeuft ausserhalb
// des Schlosses, weil er in den ControlClient ruft.

/// Der gemeinsame Weg jeder angenommenen Schrittaenderung.

bool EqCopilotProcessor::setzeQuellenrolle (const juce::String& quelleId,
                                            const juce::String& passageId,
                                            nakama::state::Rolle rolle,
                                            nakama::state::IntentHerkunft herkunft,
                                            double konfidenz)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::setzeIntent (zustand, quelleId, passageId, rolle, herkunft,
                                          konfidenz, veraendert, grund))
            return false;
    }
    if (veraendert)
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        // M-85: die Fortschreibung reist unter ihrem eigenen Schluessel. Sie
        // steht NACH dem Dirty, weil ein Wire-Write kein persistenter
        // Vorgang ist - und ausserhalb des Schlosses, weil sie in den
        // ControlClient ruft.
        sendeIntentFortschreibung (quelleId, passageId);
    }
    return true;
}

bool EqCopilotProcessor::entferneQuellenrolle (const juce::String& quelleId,
                                               const juce::String& passageId)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::entferneIntent (zustand, quelleId, passageId, veraendert, grund))
            return false;
    }
    if (veraendert)
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        // Ein ENTFERNTES oder mengenwertiges Objekt hat keine Einzelform, die
        // unter einem Objektschluessel reisen koennte. Es meldet deshalb den
        // VOLLBESTAND mit neuer Marke - teurer, aber ehrlich: eine
        // Fortschreibung, die ein Verschwinden ausdruecken soll, gibt es im
        // Vertrag nicht.
        sendeIntentVollbestand();
    }
    return true;
}

bool EqCopilotProcessor::schuetzeQuelle (const juce::String& quelleId,
                                         nakama::state::Schutzeigenschaft eigenschaft,
                                         int bandVon, int bandBis)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::setzeSchutzangabe (zustand, quelleId, eigenschaft, bandVon, bandBis,
                                                veraendert, grund))
            return false;
    }
    if (veraendert)
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        // Ein ENTFERNTES oder mengenwertiges Objekt hat keine Einzelform, die
        // unter einem Objektschluessel reisen koennte. Es meldet deshalb den
        // VOLLBESTAND mit neuer Marke - teurer, aber ehrlich: eine
        // Fortschreibung, die ein Verschwinden ausdruecken soll, gibt es im
        // Vertrag nicht.
        sendeIntentVollbestand();
    }
    return true;
}

bool EqCopilotProcessor::hebeQuellenschutzAuf (const juce::String& quelleId,
                                               nakama::state::Schutzeigenschaft eigenschaft,
                                               int bandVon, int bandBis)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::entferneSchutzangabe (zustand, quelleId, eigenschaft, bandVon, bandBis,
                                                   veraendert, grund))
            return false;
    }
    if (veraendert)
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        // Ein ENTFERNTES oder mengenwertiges Objekt hat keine Einzelform, die
        // unter einem Objektschluessel reisen koennte. Es meldet deshalb den
        // VOLLBESTAND mit neuer Marke - teurer, aber ehrlich: eine
        // Fortschreibung, die ein Verschwinden ausdruecken soll, gibt es im
        // Vertrag nicht.
        sendeIntentVollbestand();
    }
    return true;
}

bool EqCopilotProcessor::setzeQuellenbeziehung (const juce::String& quelleA,
                                                const juce::String& quelleB,
                                                nakama::state::Beziehungsart art)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        // M-06: die Zyklenpruefung laeuft HIER, beim Speichern - nicht beim
        // Anwenden. Ein Zyklus erreicht die Persistenz nie unmarkiert.
        if (! nakama::state::setzeBeziehung (zustand, quelleA, quelleB, art, veraendert, grund))
            return false;
    }
    if (veraendert)
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        // Ein ENTFERNTES oder mengenwertiges Objekt hat keine Einzelform, die
        // unter einem Objektschluessel reisen koennte. Es meldet deshalb den
        // VOLLBESTAND mit neuer Marke - teurer, aber ehrlich: eine
        // Fortschreibung, die ein Verschwinden ausdruecken soll, gibt es im
        // Vertrag nicht.
        sendeIntentVollbestand();
    }
    return true;
}

bool EqCopilotProcessor::speichereQuellenGleichrangigkeit (const juce::String& quelleA,
                                                           const juce::String& quelleB)
{
    return setzeQuellenbeziehung (quelleA, quelleB,
                                  nakama::state::Beziehungsart::gleichrangig);
}

bool EqCopilotProcessor::entferneQuellenbeziehung (const juce::String& quelleA,
                                                   const juce::String& quelleB)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::entferneBeziehung (zustand, quelleA, quelleB, veraendert, grund))
            return false;
    }
    if (veraendert)
    {
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        // Ein ENTFERNTES oder mengenwertiges Objekt hat keine Einzelform, die
        // unter einem Objektschluessel reisen koennte. Es meldet deshalb den
        // VOLLBESTAND mit neuer Marke - teurer, aber ehrlich: eine
        // Fortschreibung, die ein Verschwinden ausdruecken soll, gibt es im
        // Vertrag nicht.
        sendeIntentVollbestand();
    }
    return true;
}

std::vector<nakama::state::SourceIntent> EqCopilotProcessor::sourceIntents() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.sourceIntents;
}

std::vector<nakama::state::Schutzangabe> EqCopilotProcessor::intentSchutzangaben() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.schutzangaben;
}

std::vector<nakama::state::IntentBeziehung> EqCopilotProcessor::intentBeziehungen() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.intentBeziehungen;
}

juce::int64 EqCopilotProcessor::intentBestandRevision() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.intentBestandRevision;
}

bool EqCopilotProcessor::wirkendeQuellenrolle (const juce::String& quelleId,
                                               const juce::String& passageId,
                                               nakama::state::Rolle& aus) const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    const auto* intent = nakama::state::wirkenderIntent (zustand, quelleId, passageId);
    if (intent == nullptr)
        return false;
    aus = intent->rolle;
    return true;
}

bool EqCopilotProcessor::entmaskierungErlaubt (const juce::String& quelleA,
                                               const juce::String& quelleB,
                                               const juce::String& passageId) const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return nakama::state::entmaskierungErlaubt (zustand, quelleA, quelleB, passageId);
}

} // namespace eqcop
