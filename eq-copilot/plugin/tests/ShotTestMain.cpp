// Editor-Sichtprüfung ohne Host (Übergabe §14.2/10): baut den echten
// Processor, speist ~20 s Rauschen + 116-Hz-Ton durch processBlock (der
// Worker misst wie im Betrieb) und rendert den Material-Editor offscreen als
// PNG — Kurve, Marker, Messleisten und Statuszellen sind dann echt belegt.
//
// Aufruf:  EqCopShot <ziel.png> [breitePx] [--state <datei.bin>]
// Seit NAK-309 (M-74, R-309-7) trägt der v2-Client im Testbau einen
// Probe-Namen, auf dem kein Broker lauscht: auch neben einem laufenden
// Broker verbindet er sich nie mit der Produktions-Pipe, und die LED zeigt
// ehrlich „App aus" (rot).
// --state laedt vor dem Render einen Host-State (z. B. den read-only-Fall
// fixtures/state/schema2/fremdes-major-3.bin, SONDE-006) — der Sichtbeweis,
// dass der Editor einen Zustand zeigt, den es gibt.
// --sonde012-suite <ordner> (Kanon B15) rendert den SONDE-012-Sichtsatz und
// faehrt seit NAK-312 Etappe 4 danach das Messpunkt-Panel am echten Editor
// (312/M-35 bis 312/M-37, R-312-2): Abbau nach dem Ende von Editor und
// Prozessor, normaler Handgriff mit und ohne Aenderung; dazu das Bild
// nak312-messpunkt-panel.png des geoeffneten Panels im selben Ordner.
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>

namespace
{
using Model = eqcop::SourcesModel;

std::string hex (unsigned long long wert)
{
    std::ostringstream s;
    s << std::hex << std::setfill ('0') << std::setw (32) << wert;
    return s.str();
}

Model::Zeile quelle (int n)
{
    Model::Zeile q;
    q.instanceId = hex (20 + (unsigned) n);
    q.runtimeNonce = hex (200 + (unsigned) n);
    q.pluginKind = "active_probe";
    q.mitgliedschaft = Model::Mitgliedschaft::bestaetigt;
    q.control = Model::Control::verbunden;
    q.messung = Model::Messung::fresh;
    q.betrieb = Model::Betrieb::active;
    q.lautheit = Model::Lautheit::gueltig;
    q.namensherkunft = Model::Namensherkunft::host;
    q.capabilityEvidenz = Model::CapabilityEvidenz::nochNichtGemessen;
    q.messpunkt = Model::Messpunkt::insert;
    q.sichtbarerName = "Host Bus " + juce::String (n + 1);
    q.userLabel = "Stored source " + juce::String (n + 1);
    q.hostBusName = q.sichtbarerName;
    q.hostMixerIndexVorhanden = true;
    q.hostMixerIndex = (std::uint64_t) n + 1;
    q.descriptorVorhanden = true;
    q.hauptziel = n == 0;
    q.controlAlterMs = 140;
    q.messAlterMs = 90;
    q.fensterDauerMs = 42.7;
    q.lufsPaarVorhanden = true;
    q.lufsI = -18.0f - (float) n * 0.1f;
    q.lufsIUnsicherheitLu = 0.25f;
    return q;
}

Model::Sicht sicht (int n)
{
    Model::Sicht s;
    s.subscriptionAktiv = true;
    s.fuehrendesMain = hex (10);
    s.mainDarfSchreiben = true;
    for (int i = 0; i < n; ++i)
        s.quellen.push_back (quelle (i));
    return s;
}

bool schreibeShot (eqcop::EqCopilotProcessor& proz, const juce::File& ordner,
                    const char* name, Model::Sicht fixture)
{
    proz.setzeSourcesFixtureFuerTest (std::move (fixture));
    std::unique_ptr<juce::AudioProcessorEditor> editor (proz.createEditor());
    editor->setSize (760, 430);
    for (int i = 0; i < 3; ++i)
        juce::Timer::callPendingTimersSynchronously();
    const auto bild = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
    editor.reset();

    const auto ziel = ordner.getChildFile (name);
    ziel.deleteFile();
    juce::FileOutputStream strom (ziel);
    juce::PNGImageFormat png;
    const bool ok = strom.openedOk() && png.writeImageToStream (bild, strom);
    strom.flush();
    std::printf ("%s %s (%dx%d)\n", ok ? "SHOT OK" : "SHOT FEHLGESCHLAGEN",
                 ziel.getFullPathName().toRawUTF8(), bild.getWidth(), bild.getHeight());
    return ok && bild.getWidth() == 760 && bild.getHeight() == 430;
}

int sonde012Suite (const juce::File& ordner)
{
    if (ordner.createDirectory().failed())
    {
        std::printf ("SHOT FEHLGESCHLAGEN — Ausgabeordner nicht anlegbar: %s\n",
                     ordner.getFullPathName().toRawUTF8());
        return 1;
    }

    eqcop::EqCopilotProcessor proz;
    proz.setzeEditorOffen (true);
    if (! proz.setzeBindung ("hub", "Gen", ""))
    {
        std::printf ("SHOT FEHLGESCHLAGEN — Main konnte nicht initialisiert werden\n");
        return 1;
    }

    struct Fall { const char* name; Model::Sicht fixture; };
    std::vector<Fall> faelle;
    faelle.push_back ({ "sonde012-0-sources.png", sicht (0) });
    faelle.push_back ({ "sonde012-1-source.png", sicht (1) });
    faelle.push_back ({ "sonde012-16-sources.png", sicht (16) });
    faelle.push_back ({ "sonde012-fresh.png", sicht (1) });

    auto partial = sicht (1);
    partial.quellen[0].messung = Model::Messung::partial;
    partial.quellen[0].lautheit = Model::Lautheit::missing;
    partial.quellen[0].lufsPaarVorhanden = false;
    faelle.push_back ({ "sonde012-partial.png", partial });

    auto stale = sicht (1);
    stale.quellen[0].messung = Model::Messung::stale;
    stale.quellen[0].messAlterMs = 1400;
    faelle.push_back ({ "sonde012-stale.png", stale });

    auto getrennt = stale;
    getrennt.subscriptionAktiv = false;
    getrennt.quellen[0].control = Model::Control::getrennt;
    faelle.push_back ({ "sonde012-disconnected.png", getrennt });

    auto invalid = sicht (1);
    invalid.quellen[0].messung = Model::Messung::invalid;
    invalid.quellen[0].lautheit = Model::Lautheit::missing;
    invalid.quellen[0].lufsPaarVorhanden = false;
    invalid.quellen[0].p2RejectAktiv = true;
    invalid.quellen[0].p2RejectGrund = "feature_batch_ungueltig";
    invalid.quellen[0].p2RejectZaehler = 2;
    faelle.push_back ({ "sonde012-invalid-frame.png", invalid });
    auto loudnessInvalid = sicht (1);
    loudnessInvalid.quellen[0].lautheit = Model::Lautheit::invalid;
    loudnessInvalid.quellen[0].lufsPaarVorhanden = false;
    loudnessInvalid.quellen[0].p2RejectAktiv = true;
    loudnessInvalid.quellen[0].p2RejectGrund = "lautheit_ungueltig";
    loudnessInvalid.quellen[0].p2RejectZaehler = 2;
    faelle.push_back ({ "sonde012-loudness-invalid-half-pair.png", loudnessInvalid });

    auto nameChange = sicht (1);
    nameChange.quellen[0].hostBusName = "Host Rename";
    nameChange.quellen[0].sichtbarerName = "Host Rename";
    nameChange.quellen[0].userLabel = "Stored source 1";
    faelle.push_back ({ "sonde012-name-change.png", nameChange });

    auto leerLufs = sicht (1);
    leerLufs.quellen[0].lautheit = Model::Lautheit::measuring;
    leerLufs.quellen[0].lufsPaarVorhanden = false;
    faelle.push_back ({ "sonde012-loudness-empty.png", leerLufs });

    auto unavailable = sicht (1);
    unavailable.diagnose = Model::Diagnose::brokerUnavailable;
    unavailable.diagnoseHatHandgriff = true;
    faelle.push_back ({ "sonde012-broker-unavailable.png", unavailable });
    auto confirmation = sicht (1);
    confirmation.diagnose = Model::Diagnose::confirmationRequired;
    faelle.push_back ({ "sonde012-join-confirmation.png", confirmation });
    auto incompatible = sicht (1);
    incompatible.diagnose = Model::Diagnose::incompatible;
    faelle.push_back ({ "sonde012-incompatible.png", incompatible });
    auto degraded = sicht (1);
    degraded.diagnose = Model::Diagnose::storeDegraded;
    faelle.push_back ({ "sonde012-store-degraded.png", degraded });

    auto bestaetigtNie = sicht (1);
    bestaetigtNie.subscriptionAktiv = false;
    bestaetigtNie.quellen[0].control = Model::Control::getrennt;
    bestaetigtNie.quellen[0].messung = Model::Messung::missing;
    bestaetigtNie.quellen[0].lautheit = Model::Lautheit::missing;
    bestaetigtNie.quellen[0].lufsPaarVorhanden = false;
    faelle.push_back ({ "sonde012-confirmed-never-connected.png", bestaetigtNie });

    auto unbound = sicht (1);
    unbound.quellen[0].mitgliedschaft = Model::Mitgliedschaft::unclassified;
    faelle.push_back ({ "sonde012-unbound-removed.png", unbound });
    auto offline = sicht (1);
    offline.quellen[0].betrieb = Model::Betrieb::offline;
    offline.quellen[0].messung = Model::Messung::stale;
    faelle.push_back ({ "sonde012-offline.png", offline });
    auto suspended = sicht (1);
    suspended.quellen[0].betrieb = Model::Betrieb::suspended;
    suspended.quellen[0].messung = Model::Messung::stale;
    faelle.push_back ({ "sonde012-suspended.png", suspended });

    auto fallback = sicht (1);
    fallback.quellen[0].hostBusName.clear();
    fallback.quellen[0].hostMixerIndexVorhanden = false;
    fallback.quellen[0].sichtbarerName = fallback.quellen[0].userLabel;
    fallback.quellen[0].namensherkunft = Model::Namensherkunft::userLabel;
    faelle.push_back ({ "sonde012-label-fallback.png", fallback });
    auto unnamed = fallback;
    unnamed.quellen[0].instanceId = "a1b2c3d4000000000000000000000000";
    unnamed.quellen[0].userLabel.clear();
    unnamed.quellen[0].sichtbarerName = "Unnamed "
        + juce::String (unnamed.quellen[0].instanceId).substring (0, 8);
    unnamed.quellen[0].namensherkunft = Model::Namensherkunft::platzhalter;
    faelle.push_back ({ "sonde012-unnamed-instance-prefix.png", unnamed });
    faelle.push_back ({ "sonde012-loudness-valid.png", sicht (1) });

    auto findings = sicht (2);
    findings.quellen[0].findingsOffen = 0;
    findings.quellen[1].findingsOffen = 7;
    faelle.push_back ({ "sonde012-findings-count-0-and-many.png", findings });
    faelle.push_back ({ "sonde012-one-main-target-with-references.png", sicht (4) });

    auto nichtFuehrend = sicht (1);
    nichtFuehrend.mainDarfSchreiben = false;
    nichtFuehrend.fuehrendesMain = hex (11);
    faelle.push_back ({ "sonde012-nonleading-label-disabled.png", nichtFuehrend });

    int ok = 0;
    for (auto& fall : faelle)
        if (schreibeShot (proz, ordner, fall.name, std::move (fall.fixture)))
            ++ok;
    std::printf ("SONDE-012 SHOTS %d/%zu gruen\n", ok, faelle.size());
    return ok == (int) faelle.size() ? 0 : 1;
}
} // namespace

//==============================================================================
// NAK-312 Etappe 4 (T3-04-03, R-312-2, R-312-12): das Messpunkt-Panel.
//
// Das Panel ist eine CallOutBox, die ihren Editor ueberleben kann: FL schliesst
// das Fenster mit offenem Popover, der Editor stirbt, die Box erst im naechsten
// Durchlauf der Nachrichtenschleife (ModalComponentManager). Ihr Destruktor
// ruft `uebernehmen()`. R-312-2: nach dem Ende seines Eigentuemers ist dieser
// Rueckruf wirkungslos - kein Zugriff, keine Mutation, keine Dirty-Meldung -,
// und der normale Handgriff bleibt derselbe.
//
// Die MARKE sitzt in `uebernehmen()` unmittelbar vor dem ersten Zugriff auf den
// Prozessor (R-312-12). Sie liegt ausserhalb von Editor und Prozessor, weil sie
// deren Ende ueberleben muss; ein Rueckruf, der sie nach dem Ende erreicht,
// wird gezaehlt und kehrt ohne Zugriff zurueck. Der Beleg ist die gezaehlte
// Marke, nie ein Absturz.

namespace eqcop::testzugang
{
// Definiert in src/PluginEditor.cpp unter NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3.
std::function<bool()>& messpunktMarkeFuerTest();
}

namespace
{
int panelFehler = 0, panelGeprueft = 0;

void panelPruefe (bool ok, const std::string& was, const std::string& detail)
{
    ++panelGeprueft;
    if (! ok) ++panelFehler;
    std::printf ("  %s %s  [%s]\n", ok ? "ok     " : "FEHLER ", was.c_str(), detail.c_str());
}

/** Die Nachrichtenschleife, wie ein Host sie pumpt (Muster B30): JUCE stellt
    Nachrichten, Timer und die Aufraeumrunde des ModalComponentManagers ueber
    ein verstecktes Fenster zu. Endet, sobald `bis` erfuellt ist; die Frist ist
    nur eine Obergrenze gegen einen haengenden Lauf. */
bool pumpe (int fristMs, const std::function<bool()>& bis)
{
    const auto ende = juce::Time::getMillisecondCounterHiRes() + (double) fristMs;
    for (;;)
    {
        MSG nachricht;
        while (PeekMessageW (&nachricht, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage (&nachricht);
            DispatchMessageW (&nachricht);
        }
        if (bis())
            return true;
        if (juce::Time::getMillisecondCounterHiRes() >= ende)
            return bis();
        MsgWaitForMultipleObjects (0, nullptr, FALSE, 5, QS_ALLINPUT);
    }
}

struct PanelDirty final : juce::AudioProcessorListener
{
    int nichtParameter = 0;
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        if (d.nonParameterStateChanged) ++nichtParameter;
    }
};

template <typename Typ>
Typ* erstesKind (juce::Component& eltern)
{
    for (auto* kind : eltern.getChildren())
        if (auto* treffer = dynamic_cast<Typ*> (kind))
            return treffer;
    return nullptr;
}

enum class PanelFolge { editorVorPanel, panelVorEditor };

struct PanelLauf
{
    bool geoeffnet = false, panelWeg = false;
    int  marke = 0, dirty = 0;
    juce::String labelVorher, labelNachPanel, labelBeimEnde;
};

constexpr const char* kNeuesLabel = "NAK-312 Etappe 4";

/** Ein Legacy-Gen mit offenem Editor, das Messpunkt-Popover ueber den
    normalen Handgriff geoeffnet (derselbe onClick wie ein Klick), im Panel
    wahlweise eine echte Aenderung gesetzt. */
PanelLauf panelLauf (PanelFolge folge, bool aendern, const juce::File* bildZiel)
{
    PanelLauf l;
    struct Vogel { bool lebt = true; int marke = 0; };
    auto vogel = std::make_shared<Vogel>();
    eqcop::testzugang::messpunktMarkeFuerTest() = [vogel]
    {
        if (! vogel->lebt)
        {
            ++vogel->marke;
            return false;      // zurueck, ohne den Prozessor anzufassen
        }
        return true;
    };

    auto proz = std::make_unique<eqcop::EqCopilotProcessor>();   // NAK-175: Heap
    PanelDirty dirty;
    proz->addListener (&dirty);
    l.labelVorher = proz->holeLabel();
    auto editor = std::unique_ptr<juce::AudioProcessorEditor> (proz->createEditor());
    editor->setSize (1200, 832);

    juce::TextButton* knopf = nullptr;
    for (auto* kind : editor->getChildren())
        if (auto* b = dynamic_cast<juce::TextButton*> (kind))
            if (b->getButtonText().startsWith ("Messpunkt") && b->onClick)
                knopf = b;
    if (knopf != nullptr)
        knopf->onClick();
    auto* box = erstesKind<juce::CallOutBox> (*editor);
    juce::Component* panel = box != nullptr ? box->getChildComponent (0) : nullptr;
    auto* feld = panel != nullptr ? erstesKind<juce::TextEditor> (*panel) : nullptr;
    l.geoeffnet = knopf != nullptr && box != nullptr && panel != nullptr && feld != nullptr;
    const juce::Component::SafePointer<juce::Component> panelSicher (panel);

    if (l.geoeffnet && aendern)
        feld->setText (kNeuesLabel, juce::dontSendNotification);
    if (l.geoeffnet && bildZiel != nullptr)
    {
        const auto bild = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
        bildZiel->deleteFile();
        juce::FileOutputStream strom (*bildZiel);
        juce::PNGImageFormat png;
        if (strom.openedOk())
            png.writeImageToStream (bild, strom);
    }

    if (folge == PanelFolge::editorVorPanel)
    {
        // 312/M-35: Editor zerstoeren, danach Prozessor zerstoeren, DANN der
        // Nachrichtenschleife einen Durchlauf geben.
        editor.reset();
        vogel->lebt = false;             // Ende des Eigentuemers
        l.labelBeimEnde = proz->holeLabel();
        proz->removeListener (&dirty);
        proz.reset();
        l.panelWeg = pumpe (5000, [&] { return panelSicher == nullptr; });
    }
    else
    {
        // 312/M-36 und 312/M-37: das Panel schliesst, DANN Editor, DANN Prozessor.
        if (box != nullptr)
            box->dismiss();
        l.panelWeg = pumpe (5000, [&] { return panelSicher == nullptr; });
        l.labelNachPanel = proz->holeLabel();
        editor.reset();
        vogel->lebt = false;
        l.labelBeimEnde = proz->holeLabel();
        proz->removeListener (&dirty);
        proz.reset();
    }
    l.marke = vogel->marke;
    l.dirty = dirty.nichtParameter;
    eqcop::testzugang::messpunktMarkeFuerTest() = {};
    return l;
}

std::string panelText (const PanelLauf& l)
{
    return "geoeffnet " + std::string (l.geoeffnet ? "ja" : "NEIN") + ", Panel abgebaut "
         + (l.panelWeg ? "ja" : "NEIN") + ", Marke " + std::to_string (l.marke) + ", Host-Dirty "
         + std::to_string (l.dirty) + ", Label vorher '" + l.labelVorher.toStdString() + "', nach dem Panel '"
         + l.labelNachPanel.toStdString() + "', beim Ende '" + l.labelBeimEnde.toStdString() + "'";
}

bool nak312Messpunktpanel (const juce::File& ordner)
{
    std::printf ("== NAK-312 Etappe 4 - das Messpunkt-Panel (312/M-35 bis 312/M-37, R-312-2) ==\n");

    const auto m35 = panelLauf (PanelFolge::editorVorPanel, true, nullptr);
    panelPruefe (m35.geoeffnet && m35.panelWeg && m35.marke == 0 && m35.dirty == 0
                     && m35.labelBeimEnde == m35.labelVorher,
                 "312/M-35 panel_abbau_nach_prozessorende_ist_wirkungslos (R-312-2, R-312-12): Legacy-Gen, Popover "
                 "offen, im Panel eine echte Aenderung - Editor zerstoert, Prozessor zerstoert, DANN baut die "
                 "Nachrichtenschleife das Panel ab: sein Destruktor fasst weder Prozessor noch Editor an (Marke 0), "
                 "die Bindung ist bis zum Ende des Prozessors unveraendert, 0 Host-Dirty",
                 panelText (m35));

    const auto bild = ordner.getChildFile ("nak312-messpunkt-panel.png");
    const auto m36 = panelLauf (PanelFolge::panelVorEditor, true, &bild);
    panelPruefe (m36.geoeffnet && m36.panelWeg && m36.marke == 0 && m36.dirty == 1
                     && m36.labelNachPanel == kNeuesLabel && m36.labelBeimEnde == kNeuesLabel,
                 "312/M-36 normaler_handgriff_bleibt (Teilfall von 312/M-35, R-312-2 letzter Satz): das Panel "
                 "schliesst, solange der Editor lebt - die Bindung wird uebernommen, und genau EINE Host-Dirty-"
                 "Meldung entsteht",
                 panelText (m36) + ", Bild " + bild.getFileName().toStdString());

    const auto m37 = panelLauf (PanelFolge::panelVorEditor, false, nullptr);
    panelPruefe (m37.geoeffnet && m37.panelWeg && m37.marke == 0 && m37.dirty == 0
                     && m37.labelNachPanel == m37.labelVorher,
                 "312/M-37 ohne_aenderung_keine_mutation (Teilfall von 312/M-35, CLAUDE.md State verlustfrei): "
                 "dasselbe ohne Aenderung im Panel - keine Mutation und keine Dirty-Meldung",
                 panelText (m37));

    std::printf ("NAK-312 PANEL %d geprueft, %d Fehler\n", panelGeprueft, panelFehler);
    return panelFehler == 0 && panelGeprueft == 3;
}
} // namespace

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI init;
    if (argc >= 2 && juce::String (argv[1]) == "--sonde012-suite")
    {
        const auto ordner = argc >= 3
            ? juce::File::getCurrentWorkingDirectory().getChildFile (
                  juce::String (juce::CharPointer_UTF8 (argv[2])))
            : juce::File::getCurrentWorkingDirectory().getChildFile (
                  "eq-copilot/build/sonde012-shots");
        const int shots = sonde012Suite (ordner);
        // NAK-312 Etappe 4 (R-312-2): das Messpunkt-Panel am echten Editor.
        const bool panel = nak312Messpunktpanel (ordner);
        return shots == 0 && panel ? 0 : 1;
    }
    const juce::File ziel = juce::File::getCurrentWorkingDirectory()
        .getChildFile (argc > 1 ? juce::String (juce::CharPointer_UTF8 (argv[1]))
                                : juce::String ("eqcop-shot.png"));
    const int breite = argc > 2 && juce::String (argv[2]) != "--state"
                           ? juce::jlimit (600, 1950, juce::String (argv[2]).getIntValue())
                           : 1200;
    juce::File stateDatei;
    for (int i = 1; i + 1 < argc; ++i)
        if (juce::String (argv[i]) == "--state")
            stateDatei = juce::File::getCurrentWorkingDirectory().getChildFile (juce::String (juce::CharPointer_UTF8 (argv[i + 1])));

    eqcop::EqCopilotProcessor proz;
    proz.prepareToPlay (48000.0, 512);
    if (stateDatei != juce::File())
    {
        juce::MemoryBlock state;
        if (! stateDatei.loadFileAsData (state))
        {
            std::printf ("SHOT FEHLGESCHLAGEN — State %s nicht lesbar\n", stateDatei.getFullPathName().toRawUTF8());
            return 1;
        }
        proz.setStateInformation (state.getData(), (int) state.getSize());
        std::printf ("State geladen: %s (read-only: %s, Herkunft %d)\n", stateDatei.getFileName().toRawUTF8(),
                     proz.stateNurLesen() ? "ja" : "nein", (int) proz.holeStateHerkunft());
    }

    // 20 s Material: Rauschbett −24 dBFS + 116-Hz-Ton — liefert messbereit,
    // eine lebende Kurve und den Resonanz-Marker aus der echten Engine.
    juce::AudioBuffer<float> block (2, 512);
    juce::MidiBuffer midi;
    juce::Random rnd (20260814);
    double phase = 0.0;
    const double dphi = juce::MathConstants<double>::twoPi * 116.0 / 48000.0;
    for (int b = 0; b < 1875; ++b)
    {
        for (int i = 0; i < 512; ++i)
        {
            const float rausch = (rnd.nextFloat() * 2.0f - 1.0f) * 0.06f;
            const float ton = 0.12f * (float) std::sin (phase);
            phase += dphi;
            block.setSample (0, i, rausch + ton);
            block.setSample (1, i, rausch + ton);
        }
        proz.processBlock (block, midi);
        if ((b + 1) % 48 == 0)
            juce::Thread::sleep (60);   // Worker-Takt 50 ms: FIFO leeren lassen
    }
    juce::Thread::sleep (500);          // letzte auswerten()-Runde (~250 ms)

    std::unique_ptr<juce::AudioProcessorEditor> editor (proz.createEditor());
    editor->setSize (breite, juce::roundToInt ((float) breite * 520.0f / 750.0f));
    // FPS-Fix (m4): paint() liest die Timer-Kopie des Editors — headless
    // feuern Timer nur über callPendingTimersSynchronously (JUCE 8 hat
    // runDispatchLoopUntil hinter JUCE_MODAL_LOOPS_PERMITTED verriegelt).
    for (int i = 0; i < 10; ++i)
    {
        juce::Thread::sleep (35);
        juce::Timer::callPendingTimersSynchronously();
    }
    const auto bild = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
    editor.reset();

    ziel.deleteFile();
    juce::FileOutputStream strom (ziel);
    juce::PNGImageFormat png;
    if (! strom.openedOk() || ! png.writeImageToStream (bild, strom))
    {
        std::printf ("SHOT FEHLGESCHLAGEN — %s nicht schreibbar\n",
                     ziel.getFullPathName().toRawUTF8());
        return 1;
    }
    strom.flush();
    std::printf ("SHOT OK %s (%dx%d)\n", ziel.getFullPathName().toRawUTF8(),
                 bild.getWidth(), bild.getHeight());
    return 0;
}
