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
// nak312-messpunkt-panel.png des geoeffneten Panels im selben Ordner. Seit
// NAK-312 Etappe 6b danach der Labelentwurf der Main-Flaeche (312/M-66 bis
// 312/M-71, R-312-9) am echten Editor auf einem echten Main, das
// Ersatz-Hauptziel und die Aktionssteuerung (312/M-73 bis M-75, M-85,
// R-312-6; dazu vier Bilder mit 20, 21, 32 und 64 Quellen im Sichtsatz und
// das Bild nak312-hauptziel-ausserhalb.png eines Hauptziels hinter Zeile 20)
// und das Kennungskonflikt-Panel (312/M-91 bis 312/M-93, R-312-2, R-312-23):
// gepostete Klicks nach dem Ende des Editors sind wirkungslos, baut der Host
// den Editor waehrend des Rueckrufs ab, fasst der Rueckruf ihn danach nicht
// mehr an, und der normale Handgriff bleibt.
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

/// NAK-312 (312/M-73, R-312-6): n Quellen OHNE Hauptziel, die Anzeigeordnung
/// umgekehrt zur Ordnung der instance_id - die kleinste instance_id steht in
/// der letzten Anzeigezeile, das Ersatz-Hauptziel waehlt das Modell.
Model::Sicht ersatzSicht (int n)
{
    auto s = sicht (n);
    for (int i = 0; i < n; ++i)
    {
        auto& q = s.quellen[(std::size_t) i];
        q.hauptziel = false;
        q.hostMixerIndex = (std::uint64_t) (n - i);
        q.sichtbarerName = "Host Bus " + juce::String (n - i);
        q.hostBusName = q.sichtbarerName;
    }
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
    // NAK-312 (312/M-73): das Ersatz-Hauptziel folgt der Anzeigeordnung.
    faelle.push_back ({ "sonde012-20-sources-ersatzziel.png", ersatzSicht (20) });
    faelle.push_back ({ "sonde012-21-sources-ersatzziel.png", ersatzSicht (21) });
    faelle.push_back ({ "sonde012-32-sources-ersatzziel.png", ersatzSicht (32) });
    faelle.push_back ({ "sonde012-64-sources-ersatzziel.png", ersatzSicht (64) });
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
std::function<bool()>& konfliktMarkeFuerTest();   // NAK-312 Etappe 6b (NAK-349)
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

//==============================================================================
// NAK-312 Etappe 6b, Aenderungssatz A (T3-14-01, R-312-9): der Labelentwurf.
//
// Ein Entwurf im Labelfeld gehoert der Quelle, fuer die sein Text geladen
// wurde. Beim Auswahlwechsel gilt die Regel des Fokusverlusts (bestaetigen),
// angewandt auf die Startquelle; auf die neue Auswahl wird ohne neuen Edit nie
// geschrieben; ein leerer Entwurf loescht keinen fremden Namen; faellt die
// Startquelle weg oder kommt sie mit neuer Runtime-Nonce zurueck, verfaellt der
// Entwurf ohne Mutation. Gemessen am echten Editor auf einem echten Main: die
// Namen stehen im Prozessorzustand (bestaetigte Mitglieder, Join ueber den
// ACK-Weg), der Klick laeuft durch `mouseDown` wie bei JUCE - die Komponente
// selbst und danach ihr eigener Listener (`addMouseListener (this, true)`),
// also zweimal -, der Tick ueber den Tick-Testzugang, ohne Nachrichtenschleife.

using Art = eqcop::EqCopilotProcessor::SourcesCommandArt;

int labelFehler = 0, labelGeprueft = 0;

void labelPruefe (bool ok, const std::string& was, const std::string& detail)
{
    ++labelGeprueft;
    if (! ok) ++labelFehler;
    std::printf ("  %s %s  [%s]\n", ok ? "ok     " : "FEHLER ", was.c_str(), detail.c_str());
}

std::string ackAngewandt (const std::string& commandId)
{
    return std::string (R"({"type":"command_ack","command_id":")") + commandId
         + R"(","ergebnis":"angewandt","state_revision":1,"state_hash":")"
         + std::string (64, 'e') + R"("})";
}

const std::string kQuelleA = hex (0xa1), kQuelleB = hex (0xb2);
const std::string kNonceA = hex (0xa100), kNonceA2 = hex (0xa200), kNonceB = hex (0xb100);
const juce::String kEntwurf ("Alpha draft");
constexpr const char* kLabelEingeschleust = "Eingeschleust";   // Label des Joins im Testweg

/// Eine lebende, bestaetigte Zeile; der Mixerindex legt die Anzeigeordnung fest.
Model::Zeile labelZeile (const std::string& id, const std::string& nonce,
                         const juce::String& label, std::uint64_t mixer, bool haupt)
{
    auto q = quelle (0);
    q.instanceId = id;
    q.runtimeNonce = nonce;
    q.userLabel = label;
    q.hostMixerIndex = mixer;
    q.sichtbarerName = "Host Bus " + juce::String ((juce::int64) mixer);
    q.hostBusName = q.sichtbarerName;
    q.hauptziel = haupt;
    return q;
}

/// Ein Main mit den bestaetigten Mitgliedern A und B und ihren Namen im
/// Prozessorzustand, Hauptziel A, Editor offen und einmal getickt.
struct LabelBuehne
{
    std::unique_ptr<eqcop::EqCopilotProcessor> p;
    PanelDirty dirty;
    std::unique_ptr<eqcop::EqCopilotEditor> ed;
    bool aufgebaut = false;

    LabelBuehne (const juce::String& labelA, const juce::String& labelB)
        : p (std::make_unique<eqcop::EqCopilotProcessor>())   // NAK-175: Heap
    {
        p->setzeWorkerDrainFuerTest (false);   // der Tick ist der einzige Drain
        p->setzeEditorOffen (true);
        bool ok = p->setzeBindung ("hub", "Gen", "");
        for (const auto* id : { &kQuelleA, &kQuelleB })
            p->v3AntwortFuerTest (ackAngewandt (p->merkeSourcesCommandFuerTest (Art::confirmJoin, *id)));
        p->sourcesTick();
        ok = ok && label (kQuelleA) == kLabelEingeschleust && label (kQuelleB) == kLabelEingeschleust;
        setzeSicht (true, kNonceA, kQuelleA);
        ok = ok && p->benenneSourcesHauptziel (kQuelleA, labelA);
        ok = ok && p->waehleSourcesHauptziel (kQuelleB) && p->benenneSourcesHauptziel (kQuelleB, labelB);
        ok = ok && p->waehleSourcesHauptziel (kQuelleA);
        aufgebaut = ok && label (kQuelleA) == labelA && label (kQuelleB) == labelB;
        p->addListener (&dirty);
        ed = std::make_unique<eqcop::EqCopilotEditor> (*p);
        ed->timerTickFuerTest();
        aufgebaut = aufgebaut && ed->sourcesLabelTextFuerTest() == labelA
                 && ed->sourcesAktionsZielFuerTest() == kQuelleA;
    }

    ~LabelBuehne()
    {
        ed.reset();   // der Editor stirbt vor seinem Prozessor
        p->removeListener (&dirty);
        p.reset();
    }

    juce::String label (const std::string& id) const
    {
        for (const auto& m : p->holeZustandKopie().mainProjectMitglieder)
            if (m.instanceId.toStdString() == id)
                return m.label;
        return "<kein Mitglied>";
    }

    /// Die fluechtige Sicht neu setzen: A wahlweise fehlend oder mit anderer
    /// Nonce, die Labels aus dem Prozessorzustand, Hauptziel `haupt` (leer =
    /// das Modell waehlt den Ersatz).
    void setzeSicht (bool mitA, const std::string& nonceA, const std::string& haupt)
    {
        Model::Sicht s;
        s.subscriptionAktiv = true;
        s.fuehrendesMain = hex (10);
        s.mainDarfSchreiben = true;
        if (mitA)
            s.quellen.push_back (labelZeile (kQuelleA, nonceA, label (kQuelleA), 1, haupt == kQuelleA));
        s.quellen.push_back (labelZeile (kQuelleB, kNonceB, label (kQuelleB), 2, haupt == kQuelleB));
        p->setzeSourcesFixtureFuerTest (std::move (s));
    }

    std::string beleg() const
    {
        return std::string (aufgebaut ? "" : "AUFBAU FEHLGESCHLAGEN, ") + "A '"
             + label (kQuelleA).toStdString() + "', B '" + label (kQuelleB).toStdString()
             + "', Host-Dirty " + std::to_string (dirty.nichtParameter) + ", Feld '"
             + ed->sourcesLabelTextFuerTest().substring (0, 24).toStdString() + "', Aktionsziel "
             + (ed->sourcesAktionsZielFuerTest() == kQuelleA ? "A"
                : ed->sourcesAktionsZielFuerTest() == kQuelleB ? "B"
                : ed->sourcesAktionsZielFuerTest().empty() ? "leer" : "fremd");
    }
};

/// Ein Klick in Zeile `zeile` der gezeichneten Liste, zugestellt wie von JUCE.
void klicke (eqcop::EqCopilotEditor& ed, std::size_t zeile)
{
    const auto zeilen = ed.sourcesZeilenFuerTest();
    if (zeile >= zeilen.size())
        return;
    const auto punkt = zeilen[zeile].getCentre().toFloat();
    const auto jetzt = juce::Time::getCurrentTime();
    const juce::MouseEvent e (juce::Desktop::getInstance().getMainMouseSource(), punkt,
                              juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier),
                              juce::MouseInputSource::defaultPressure,
                              juce::MouseInputSource::defaultOrientation,
                              juce::MouseInputSource::defaultRotation,
                              juce::MouseInputSource::defaultTiltX,
                              juce::MouseInputSource::defaultTiltY,
                              &ed, &ed, jetzt, punkt, jetzt, 1, false);
    auto& komponente = static_cast<juce::Component&> (ed);
    komponente.mouseDown (e);   // die Komponente selbst ...
    komponente.mouseDown (e);   // ... und ihr eigener Listener
}

bool nak312Labelentwurf()
{
    std::printf ("== NAK-312 Etappe 6b - der Labelentwurf (312/M-66 bis 312/M-71, R-312-9) ==\n");

    {
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelTippenFuerTest (kEntwurf);
        klicke (*b.ed, 1);
        b.ed->timerTickFuerTest();
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == kEntwurf && b.label (kQuelleB) == "Beta"
                         && b.dirty.nichtParameter == 1 && b.ed->sourcesLabelTextFuerTest() == "Beta"
                         && b.ed->sourcesAktionsZielFuerTest() == kQuelleB,
                     "312/M-66 labelentwurf_gehoert_seiner_startquelle (R-312-9): Hauptziel A, Entwurf im "
                     "fokussierten Feld, Klick auf B, Tick, Enter - As Name traegt den Entwurf, Bs Name ist "
                     "unveraendert, genau 1 Host-Dirty (ein geaenderter Name), das Feld zeigt Bs Label",
                     b.beleg());
        // Selbstaudit: dieselbe Regel in der Gegenrichtung (Entwurf auf B, Klick auf A).
        b.ed->sourcesLabelTippenFuerTest ("Beta draft");
        klicke (*b.ed, 0);
        b.ed->timerTickFuerTest();
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == kEntwurf && b.label (kQuelleB) == "Beta draft"
                         && b.dirty.nichtParameter == 2 && b.ed->sourcesLabelTextFuerTest() == kEntwurf
                         && b.ed->sourcesAktionsZielFuerTest() == kQuelleA,
                     "312/M-66 (Gegenrichtung): Entwurf auf B, Klick auf A, Tick, Enter - Bs Name traegt den "
                     "Entwurf, As Name bleibt, ein Host-Dirty mehr, das Feld zeigt As Label",
                     b.beleg());
        // Selbstaudit: ein Klick auf das eigene Ziel ist kein Auswahlwechsel.
        b.ed->sourcesLabelTippenFuerTest ("Alpha draft 2");
        klicke (*b.ed, 0);
        b.ed->timerTickFuerTest();
        labelPruefe (b.label (kQuelleA) == kEntwurf && b.dirty.nichtParameter == 2
                         && b.ed->sourcesLabelTextFuerTest() == "Alpha draft 2",
                     "312/M-66 (eigenes Ziel): Klick auf die Zeile des Hauptziels bestaetigt nichts - kein "
                     "Schreiben, kein Host-Dirty, der Entwurf bleibt im Feld",
                     b.beleg());
    }
    {
        LabelBuehne b ("", "Beta");
        b.ed->sourcesLabelTippenFuerTest ("");
        klicke (*b.ed, 1);
        b.ed->timerTickFuerTest();
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA).isEmpty() && b.label (kQuelleB) == "Beta"
                         && b.dirty.nichtParameter == 0 && b.ed->sourcesLabelTextFuerTest() == "Beta",
                     "312/M-67 leerer_entwurf_loescht_keinen_fremden_namen (Teilfall von 312/M-66): A ohne "
                     "Label, B mit Label, dieselbe Folge - Bs Name bleibt, 0 Host-Dirty",
                     b.beleg());
    }
    {
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelTippenFuerTest (kEntwurf);
        b.ed->sourcesLabelEscapeFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == "Alpha" && b.label (kQuelleB) == "Beta"
                         && b.dirty.nichtParameter == 0,
                     "312/M-68 (a1) escape_schreibt_nichts (Teilfall von 312/M-66): Escape statt Enter - keine "
                     "Mutation, 0 Host-Dirty",
                     b.beleg());
        const auto feldNachEscape = b.ed->sourcesLabelTextFuerTest();
        klicke (*b.ed, 1);
        b.ed->timerTickFuerTest();
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && feldNachEscape == "Alpha" && b.label (kQuelleA) == "Alpha"
                         && b.label (kQuelleB) == "Beta" && b.dirty.nichtParameter == 0,
                     "312/M-68 (a2) escape_verwirft_den_entwurf (Teilfall von 312/M-66): nach Escape zeigt das "
                     "Feld wieder As gespeichertes Label, und Klick auf B, Tick und Enter schreiben weder A "
                     "noch B",
                     b.beleg() + ", Feld nach Escape '" + feldNachEscape.toStdString() + "'");
    }
    {
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelTippenFuerTest (kEntwurf);
        klicke (*b.ed, 1);
        b.ed->timerTickFuerTest();
        b.ed->sourcesLabelFokusVerlustFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == kEntwurf && b.label (kQuelleB) == "Beta"
                         && b.dirty.nichtParameter == 1,
                     "312/M-68 (b) fokusverlust_bestaetigt_auf_der_startquelle (Teilfall von 312/M-66): Klick "
                     "auf B, Tick, Fokusverlust statt Enter - BESTAETIGEN wie Enter, angewandt auf A; B "
                     "unveraendert, genau 1 Host-Dirty",
                     b.beleg());
    }
    {
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelTippenFuerTest (kEntwurf);
        b.setzeSicht (false, kNonceA, {});
        b.ed->timerTickFuerTest();
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == "Alpha" && b.label (kQuelleB) == "Beta"
                         && b.dirty.nichtParameter == 0 && b.ed->sourcesAktionsZielFuerTest() == kQuelleB
                         && b.ed->sourcesLabelTextFuerTest() == "Beta",
                     "312/M-69 (a) startquelle_faellt_weg (Teilfall von 312/M-66): A verschwindet vor dem "
                     "Bestaetigen, das Modell waehlt B, Tick, Enter - weder A noch B geschrieben, 0 Host-Dirty",
                     b.beleg());
    }
    {
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelTippenFuerTest (kEntwurf);
        b.setzeSicht (true, kNonceA2, kQuelleA);
        b.ed->timerTickFuerTest();
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == "Alpha" && b.label (kQuelleB) == "Beta"
                         && b.dirty.nichtParameter == 0 && b.ed->sourcesLabelTextFuerTest() == "Alpha",
                     "312/M-69 (b) neue_runtime_nonce_mit_tick (Teilfall von 312/M-66): A kommt mit neuer "
                     "Runtime-Nonce zurueck, Tick, Enter - der Entwurf verfaellt ohne Mutation, 0 Host-Dirty",
                     b.beleg());
    }
    {
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelTippenFuerTest (kEntwurf);
        b.setzeSicht (true, kNonceA2, kQuelleA);
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == "Alpha" && b.label (kQuelleB) == "Beta"
                         && b.dirty.nichtParameter == 0,
                     "312/M-69 (c) neue_runtime_nonce_ohne_tick (Teilfall von 312/M-66): dasselbe, Enter VOR "
                     "dem naechsten Tick - der Kennungsvergleich faellt gegen das Modell von jetzt, nicht gegen "
                     "die Anzeige des letzten Ticks",
                     b.beleg());
    }
    {
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelTippenFuerTest ("Alpha");
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.aufgebaut && b.label (kQuelleA) == "Alpha" && b.dirty.nichtParameter == 0
                         && ! b.ed->sourcesBedienstatusFuerTest().containsIgnoreCase ("failed"),
                     "312/M-70 unveraenderter_text_mutiert_nicht (Teilfall von 312/M-66): Enter ohne Aenderung "
                     "- keine Mutation, 0 Host-Dirty, keine Fehlermeldung",
                     b.beleg() + ", Status '" + b.ed->sourcesBedienstatusFuerTest().toStdString() + "'");
    }
    {
        // 312/M-71: Zahlenrand in Codepoints, mit einem Zeichen ausserhalb der BMP.
        const auto zeichen = juce::String::charToString ((juce::juce_wchar) 0x1F3B9);
        const auto s120 = juce::String::repeatedString (zeichen, 120);
        const auto s121 = juce::String::repeatedString (zeichen, 121);
        LabelBuehne b ("Alpha", "Beta");
        b.ed->sourcesLabelEinfuegenFuerTest (s121);
        const int eingefuegt = b.ed->sourcesLabelTextFuerTest().length();
        labelPruefe (b.aufgebaut && eingefuegt == 120 && b.dirty.nichtParameter == 0,
                     "312/M-71 (a) eingabebeschraenkung_120 (Teilfall von 312/M-66): 121 Codepoints ausserhalb "
                     "der BMP eingefuegt - das Feld haelt 120",
                     b.beleg() + ", Feldlaenge " + std::to_string (eingefuegt));
        b.ed->sourcesLabelTippenFuerTest (s121);
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (b.label (kQuelleA) == s120 && b.label (kQuelleA).length() == 120 && b.dirty.nichtParameter == 1,
                     "312/M-71 (b) kappung_120: 121 Codepoints im Feld, Enter - gespeichert werden die ersten "
                     "120, genau 1 Host-Dirty",
                     "gespeichert " + std::to_string (b.label (kQuelleA).length()) + " Codepoints, Host-Dirty "
                         + std::to_string (b.dirty.nichtParameter));
        b.ed->sourcesLabelTippenFuerTest ("");
        b.ed->sourcesLabelEnterFuerTest();
        const bool nullAngenommen = b.label (kQuelleA).isEmpty() && b.dirty.nichtParameter == 2;
        b.ed->sourcesLabelTippenFuerTest (s120);
        b.ed->sourcesLabelEnterFuerTest();
        labelPruefe (nullAngenommen && b.label (kQuelleA) == s120 && b.dirty.nichtParameter == 3,
                     "312/M-71 (c) null_und_120_werden_angenommen: 0 Codepoints, danach 120 - beide gespeichert, "
                     "je 1 Host-Dirty",
                     std::string ("0 angenommen ") + (nullAngenommen ? "ja" : "NEIN") + ", danach "
                         + std::to_string (b.label (kQuelleA).length()) + " Codepoints, Host-Dirty "
                         + std::to_string (b.dirty.nichtParameter));
        const bool abgewiesen = ! b.p->benenneSourcesHauptziel (kQuelleA, s121);
        labelPruefe (abgewiesen && b.label (kQuelleA) == s120 && b.dirty.nichtParameter == 3,
                     "312/M-71 (d) 121_wird_abgewiesen: 121 Codepoints am Prozessor - abgewiesen, der gespeicherte "
                     "Name bleibt, kein Host-Dirty",
                     std::string ("abgewiesen ") + (abgewiesen ? "ja" : "NEIN") + ", gespeichert "
                         + std::to_string (b.label (kQuelleA).length()) + " Codepoints, Host-Dirty "
                         + std::to_string (b.dirty.nichtParameter));
    }

    std::printf ("NAK-312 LABEL %d geprueft, %d Fehler\n", labelGeprueft, labelFehler);
    return labelFehler == 0 && labelGeprueft == 15;
}

//==============================================================================
// NAK-312 Etappe 6b, Aenderungssatz B (T3-07-05 Teil a, R-312-6, E-312-9):
// Ersatz-Hauptziel und Aktionssteuerung.
//
// Das Ersatzziel ist die erste ANGEZEIGTE Quelle in Anzeigeordnung, und die
// Aktionssteuerung bindet nur an eine Zeile, die `sourcesZeilen()` wirklich
// herstellt. Rueckt das Hauptziel durch Umordnung hinter die letzte
// gezeichnete Zeile, sind Knopf und Labelfeld unsichtbar, das Aktionsziel ist
// leer, und das Modell bleibt unveraendert - kein automatischer Zielwechsel;
// ein offener Labelentwurf wird vorher nach R-312-9 bestaetigt. Bei exakt
// 760x430 zeichnet die Flaeche ab 20 Quellen genau 20 Zeilen (Manifest
// Paragraph 6.7, M-73 und M-74: 366 / jlimit (18, 34, 366 / n), nachgerechnet).

int zielFehler = 0, zielGeprueft = 0;

void zielPruefe (bool ok, const std::string& was, const std::string& detail)
{
    ++zielGeprueft;
    if (! ok) ++zielFehler;
    std::printf ("  %s %s  [%s]\n", ok ? "ok     " : "FEHLER ", was.c_str(), detail.c_str());
}

/// Ein Main mit der Sicht `fixture`, Editor offen und einmal getickt; wahlweise
/// mit den Quellen als bestaetigten Mitgliedern im Prozessorzustand.
struct ZielBuehne
{
    std::unique_ptr<eqcop::EqCopilotProcessor> p;
    PanelDirty dirty;
    std::unique_ptr<eqcop::EqCopilotEditor> ed;
    bool aufgebaut = false;

    ZielBuehne (Model::Sicht fixture, bool mitMitgliedern)
        : p (std::make_unique<eqcop::EqCopilotProcessor>())   // NAK-175: Heap
    {
        p->setzeWorkerDrainFuerTest (false);
        p->setzeEditorOffen (true);
        aufgebaut = p->setzeBindung ("hub", "Gen", "");
        if (mitMitgliedern)
        {
            for (const auto& q : fixture.quellen)
                p->v3AntwortFuerTest (ackAngewandt (p->merkeSourcesCommandFuerTest (Art::confirmJoin, q.instanceId)));
            p->sourcesTick();
            aufgebaut = aufgebaut && p->holeZustandKopie().mainProjectMitglieder.size() == fixture.quellen.size();
        }
        p->setzeSourcesFixtureFuerTest (std::move (fixture));
        p->addListener (&dirty);
        ed = std::make_unique<eqcop::EqCopilotEditor> (*p);
        ed->timerTickFuerTest();
    }

    ~ZielBuehne()
    {
        ed.reset();
        p->removeListener (&dirty);
        p.reset();
    }

    std::string hauptziel() const
    {
        for (const auto& q : p->sourcesSicht().quellen)
            if (q.hauptziel)
                return q.instanceId;
        return {};
    }

    int anzeigeIndex (const std::string& id) const
    {
        const auto s = p->sourcesSicht();
        for (std::size_t i = 0; i < s.quellen.size(); ++i)
            if (s.quellen[i].instanceId == id)
                return (int) i;
        return -1;
    }

    std::string beleg (const std::string& ziel) const
    {
        const auto aktion = ed->sourcesAktionsZielFuerTest();
        return "Hauptziel in Anzeigezeile " + std::to_string (anzeigeIndex (ziel) + 1) + ", gezeichnet "
             + std::to_string (ed->sourcesZeilenFuerTest().size()) + " Zeilen, Aktionsziel "
             + (aktion.empty() ? std::string ("leer") : aktion == ziel ? std::string ("= Hauptziel")
                                                                        : std::string ("fremd"))
             + ", Knopf " + (ed->sourcesAktionKnopfSichtbarFuerTest() ? "sichtbar" : "unsichtbar")
             + ", Labelfeld " + (ed->sourcesLabelSichtbarFuerTest() ? "sichtbar" : "unsichtbar")
             + ", Host-Dirty " + std::to_string (dirty.nichtParameter) + ", Status '"
             + ed->sourcesBedienstatusFuerTest().toStdString() + "'";
    }
};

/// 25 Quellen, das Hauptziel per Handgriff auf die Quelle in Zeile 5; dazu die
/// Umordnung, die es hinter Zeile 20 schiebt - (i) 16 Quellen mit kleinerem
/// Mixerindex treten bei, (ii) 16 Quellen dahinter bekommen einen Hostbusnamen,
/// der davor sortiert (Quellen ohne Mixerindex, Ordnung nach Namen).
Model::Sicht umordnungsSicht (bool umbenennen, bool nachher, const std::string& haupt)
{
    auto s = sicht (25);
    for (int i = 0; i < 25; ++i)
    {
        auto& q = s.quellen[(std::size_t) i];
        q.hauptziel = q.instanceId == haupt;
        if (umbenennen)
        {
            q.hostMixerIndexVorhanden = false;
            const bool davor = nachher && i >= 5 && i < 21;
            q.hostBusName = (davor ? "Aux " : "Bus ") + juce::String (davor ? i - 4 : i + 1).paddedLeft ('0', 2);
        }
        else
        {
            q.hostMixerIndex = (std::uint64_t) (100 + i);
            q.hostBusName = "Host Bus " + juce::String (100 + i);
        }
        q.sichtbarerName = q.hostBusName;
    }
    if (! umbenennen && nachher)
        for (int j = 0; j < 16; ++j)
        {
            auto q = quelle (0);
            q.instanceId = hex (0x400 + (unsigned) j);
            q.runtimeNonce = hex (0x500 + (unsigned) j);
            q.hostMixerIndex = (std::uint64_t) (1 + j);
            q.hostBusName = "Joined Bus " + juce::String (1 + j);
            q.sichtbarerName = q.hostBusName;
            q.hauptziel = false;
            s.quellen.push_back (q);
        }
    return s;
}

bool nak312Ersatzziel (const juce::File& ordner)
{
    std::printf ("== NAK-312 Etappe 6b - Ersatzziel und Aktionssteuerung (312/M-73 bis M-75, M-85, R-312-6) ==\n");

    for (const int n : { 16, 20, 21, 32, 64 })
    {
        ZielBuehne b (ersatzSicht (n), false);
        const auto ziel = b.hauptziel();
        const int erwartet = n < 20 ? n : 20;
        zielPruefe (b.aufgebaut && ! ziel.empty() && b.anzeigeIndex (ziel) == 0
                        && (int) b.ed->sourcesZeilenFuerTest().size() == erwartet
                        && b.ed->sourcesAktionsZielFuerTest() == ziel && b.ed->sourcesAktionKnopfSichtbarFuerTest()
                        && b.ed->sourcesLabelSichtbarFuerTest(),
                    "312/M-73 ersatzziel_folgt_der_anzeigeordnung (" + std::to_string (n) + " Quellen, 760x430, "
                        + "kleinste instance_id in der letzten Anzeigezeile): das Ersatz-Hauptziel ist die erste "
                        + "angezeigte Quelle, gezeichnet " + std::to_string (erwartet) + " Zeilen, Knopf und Labelfeld "
                        + "stehen auf ihr",
                    b.beleg (ziel));
    }

    for (const bool umbenennen : { false, true })
    {
        const auto haupt = hex (24);   // quelle (4): Zeile 5
        ZielBuehne b (umordnungsSicht (umbenennen, false, {}), false);
        const bool gewaehlt = b.p->waehleSourcesHauptziel (haupt);
        b.ed->timerTickFuerTest();
        const bool vorher = gewaehlt && b.anzeigeIndex (haupt) == 4 && b.ed->sourcesAktionsZielFuerTest() == haupt
                         && b.ed->sourcesAktionKnopfSichtbarFuerTest() && b.ed->sourcesLabelSichtbarFuerTest();
        b.p->setzeSourcesFixtureFuerTest (umordnungsSicht (umbenennen, true, haupt));
        const auto revisionVor = b.p->sourcesSicht().revision;
        const int dirtyVor = b.dirty.nichtParameter;
        b.ed->timerTickFuerTest();
        const bool nachher = b.hauptziel() == haupt && b.anzeigeIndex (haupt) == 20
                          && b.p->sourcesSicht().revision == revisionVor && b.dirty.nichtParameter == dirtyVor
                          && b.ed->sourcesAktionsZielFuerTest().empty() && ! b.ed->sourcesAktionKnopfSichtbarFuerTest()
                          && ! b.ed->sourcesLabelSichtbarFuerTest()
                          && b.ed->sourcesBedienstatusFuerTest().containsIgnoreCase ("outside the drawn list");
        // Ohne Labelfeld auch keine Beschriftung des Feldes: im Bild ist ihre
        // Zeile (x 324 bis 513, y 373 bis 386 bei 760x430, `paintMainFlaeche`)
        // leer - jedes Pixel gleich dem Hintergrund rechts daneben.
        const auto bild = b.ed->createComponentSnapshot (b.ed->getLocalBounds(), true, 1.0f);
        int beschriftungsPixel = 0;
        const auto grund = bild.getPixelAt (700, 380);
        for (int y = 373; y < 387; ++y)
            for (int x = 324; x < 514; ++x)
                if (bild.getPixelAt (x, y) != grund)
                    ++beschriftungsPixel;
        if (umbenennen)
        {
            // Das Bild der Flaeche in diesem Zustand, neben dem Sichtsatz.
            const auto ziel = ordner.getChildFile ("nak312-hauptziel-ausserhalb.png");
            ziel.deleteFile();
            juce::FileOutputStream strom (ziel);
            juce::PNGImageFormat png;
            if (strom.openedOk())
                png.writeImageToStream (bild, strom);
        }
        zielPruefe (b.aufgebaut && vorher && nachher && beschriftungsPixel == 0,
                    std::string ("312/M-74 aktionssteuerung_bindet_nur_an_gezeichnete_zeilen (") + (umbenennen
                        ? "Umordnung durch Hostbusnamen" : "Umordnung durch Beitritt")
                        + "): Hauptziel per Handgriff in Zeile 5, danach rueckt es hinter Zeile 20 - Knopf und "
                        + "Labelfeld unsichtbar, Aktionsziel leer, das Hauptziel im Modell unveraendert, kein "
                        + "Schreibversuch (Modellrevision und Host-Dirty gleich), der Status sagt es, und die "
                        + "Beschriftung des Labelfelds ist mit ihm verschwunden",
                    std::string ("vorher ") + (vorher ? "ja" : "NEIN") + "; nachher: " + b.beleg (haupt)
                        + ", Pixel der Feldbeschriftung " + std::to_string (beschriftungsPixel));
    }

    {
        // 312/M-85: dieselbe Umordnung mit offenem Labelentwurf der Startquelle.
        const auto haupt = hex (24);
        ZielBuehne b (umordnungsSicht (true, false, haupt), true);
        b.ed->sourcesLabelTippenFuerTest ("Target draft");
        b.p->setzeSourcesFixtureFuerTest (umordnungsSicht (true, true, haupt));
        b.ed->timerTickFuerTest();
        int fremdGeschrieben = 0;
        juce::String zielLabel;
        for (const auto& m : b.p->holeZustandKopie().mainProjectMitglieder)
        {
            if (m.instanceId.toStdString() == haupt)
                zielLabel = m.label;
            else if (m.label != kLabelEingeschleust)
                ++fremdGeschrieben;
        }
        zielPruefe (b.aufgebaut && zielLabel == "Target draft" && fremdGeschrieben == 0 && b.dirty.nichtParameter == 1
                        && b.hauptziel() == haupt && b.ed->sourcesAktionsZielFuerTest().empty()
                        && ! b.ed->sourcesAktionKnopfSichtbarFuerTest() && ! b.ed->sourcesLabelSichtbarFuerTest(),
                    "312/M-85 offener_entwurf_wird_vor_dem_leeren_des_ziels_bestaetigt (E-312-9, R-312-9): 25 "
                    "bestaetigte Mitglieder, Entwurf fuer das Hauptziel, dieselbe Umordnung, Tick - der Entwurf "
                    "steht auf der Startquelle, keine fremde Quelle geschrieben, genau 1 Host-Dirty; danach Knopf "
                    "und Labelfeld unsichtbar",
                    "Label der Startquelle '" + zielLabel.toStdString() + "', fremd geschrieben "
                        + std::to_string (fremdGeschrieben) + "; " + b.beleg (haupt));
    }

    {
        ZielBuehne b (sicht (0), false);
        zielPruefe (b.aufgebaut && b.hauptziel().empty() && b.ed->sourcesZeilenFuerTest().empty()
                        && b.ed->sourcesAktionsZielFuerTest().empty() && ! b.ed->sourcesAktionKnopfSichtbarFuerTest()
                        && ! b.ed->sourcesLabelSichtbarFuerTest(),
                    "312/M-75 (0 Quellen) kein_ersatzziel_keine_aktionssteuerung (R-312-6): kein Hauptziel, keine "
                    "gezeichnete Zeile, Knopf und Labelfeld unsichtbar",
                    b.beleg ({}));
    }
    for (const int n : { 1, 16 })
    {
        ZielBuehne b (sicht (n), false);
        int erreichbar = 0;
        const auto quellen = b.p->sourcesSicht().quellen;
        for (std::size_t i = 0; i < quellen.size(); ++i)
        {
            klicke (*b.ed, i);
            b.ed->timerTickFuerTest();
            if (b.hauptziel() == quellen[i].instanceId && b.ed->sourcesAktionsZielFuerTest() == quellen[i].instanceId
                && b.ed->sourcesAktionKnopfSichtbarFuerTest() && b.ed->sourcesLabelSichtbarFuerTest())
                ++erreichbar;
        }
        zielPruefe (b.aufgebaut && (int) b.ed->sourcesZeilenFuerTest().size() == n && erreichbar == n,
                    "312/M-75 (" + std::to_string (n) + (n == 1 ? " Quelle" : " Quellen")
                        + ") jede_quelle_gezeichnet_und_erreichbar: jede Zeile ist gezeichnet, und ein Klick darauf "
                        + "macht sie zum Hauptziel mit sichtbarem Knopf und Labelfeld",
                    "gezeichnet " + std::to_string (b.ed->sourcesZeilenFuerTest().size()) + ", erreichbar "
                        + std::to_string (erreichbar) + " von " + std::to_string (quellen.size()));
    }

    std::printf ("NAK-312 ZIEL %d geprueft, %d Fehler\n", zielGeprueft, zielFehler);
    return zielFehler == 0 && zielGeprueft == 11;
}

//==============================================================================
// NAK-312 Etappe 6b, Aenderungssatz C (NAK-349, R-312-2, R-312-23): das
// Kennungskonflikt-Panel.
//
// Der Knopf "Dieser Instanz eine neue Kennung geben" loest seinen Rueckruf ueber
// eine gepostete Nachricht aus, wenn Enter oder die Barrierefreiheit ihn
// drueckt (`Button::triggerClick`). Baut der Host den Editor zwischen Einstellen
// und Zustellung ab, steht die Aufraeumnachricht des Modal-Managers HINTER dem
// Klick (docs/beweise/roh/NAK-349-quellvalidierung.md V-2): der Rueckruf laeuft
// bei lebender Box auf totem Editor. R-312-2: nach dem Ende des Editors ist er
// wirkungslos, und der Handgriff bleibt derselbe. Die MARKE steht vor jedem
// Zugriff auf Editor und Prozessor (R-312-23); ein Rueckruf, der sie nach dem
// Ende erreicht, wird gezaehlt und kehrt ohne Zugriff zurueck. Der Beleg ist die
// gezaehlte Marke, nie ein Absturz.
//
// Nacharbeit 1 der Etappe 6 (L-5, 312/M-93, R-312-2 Satz 2): der Klick beginnt
// bei lebendem Editor, und der Host baut ihn WAEHREND des Rueckrufs ab.
// `neueSensorId()` meldet Host-Dirty, `updateHostDisplay` ruft die Listener
// synchron (juce_AudioProcessor.cpp:431-436), der VST3-Wrapper auf dem
// Nachrichtenthread ebenso `setDirty` und `restartComponent` des Hosts
// (juce_VST3Common.h:1639-1656), und ein Host darf darin den Editor
// schliessen. Hier tut das ein Listener in `audioProcessorChanged`. Dieselbe
// Marke steht auch hinter der zweiten Lebendpruefung, vor dem ersten
// Schreibzugriff nach dem Hostaufruf.

enum class KonfliktFolge { editorEnde, editorUndProzessorEnde, editorLebt, editorEndeImRueckruf };

struct KonfliktLauf
{
    bool geoeffnet = false, panelWeg = false;
    int  marke = 0, dirty = 0;
    int  abgebaut = 0, eintritteBeimAbbau = 0;   // 312/M-93
    juce::String kennungVorher, kennungBeimEnde, kennungBeimAbbau, status;
};

/** 312/M-93: baut den Editor ab, wie ein Host in seiner Reaktion auf die
    Host-Dirty-Meldung (VST3: in `setDirty` oder `restartComponent`) - im
    Rueckruf von `updateHostDisplay`, also innerhalb von `neueSensorId()`.
    Leer ausser in 312/M-93; laeuft hoechstens einmal. */
struct KonfliktAbbauer final : juce::AudioProcessorListener
{
    std::function<void()> abbau;
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        if (! d.nonParameterStateChanged || ! abbau)
            return;
        const auto einmal = std::move (abbau);
        abbau = nullptr;
        einmal();
    }
};

/** Ein Legacy-Gen mit offenem Editor, das Kennungskonflikt-Popover ueber den
    normalen Handgriff geoeffnet (derselbe onClick wie ein Klick auf "Kennung
    doppelt!"), dann `triggerClick()` auf den Knopf im Panel. */
KonfliktLauf konfliktLauf (KonfliktFolge folge)
{
    KonfliktLauf l;
    struct Vogel { bool lebt = true; int marke = 0, eintritte = 0; };
    auto vogel = std::make_shared<Vogel>();
    eqcop::testzugang::konfliktMarkeFuerTest() = [vogel]
    {
        if (! vogel->lebt)
        {
            ++vogel->marke;
            return false;      // zurueck, ohne Editor oder Prozessor anzufassen
        }
        ++vogel->eintritte;    // 312/M-93: die Marke wurde bei lebendem Editor gefragt
        return true;
    };

    auto proz = std::make_unique<eqcop::EqCopilotProcessor>();   // NAK-175: Heap
    PanelDirty dirty;
    proz->addListener (&dirty);
    KonfliktAbbauer abbauer;   // 312/M-93; in den uebrigen Folgen nicht angemeldet
    l.kennungVorher = proz->holeZustandKopie().common.instanceId;
    auto editor = std::unique_ptr<juce::AudioProcessorEditor> (proz->createEditor());
    editor->setSize (1200, 832);

    juce::TextButton* knopf = nullptr;
    for (auto* kind : editor->getChildren())
        if (auto* b = dynamic_cast<juce::TextButton*> (kind))
            if (b->getButtonText() == "Kennung doppelt!" && b->onClick)
                knopf = b;
    if (knopf != nullptr)
        knopf->onClick();
    auto* box = erstesKind<juce::CallOutBox> (*editor);
    juce::Component* panel = box != nullptr ? box->getChildComponent (0) : nullptr;
    auto* neuKnopf = panel != nullptr ? erstesKind<juce::TextButton> (*panel) : nullptr;
    l.geoeffnet = knopf != nullptr && box != nullptr && panel != nullptr && neuKnopf != nullptr;
    const juce::Component::SafePointer<juce::Component> panelSicher (panel);

    if (folge == KonfliktFolge::editorEndeImRueckruf)
    {
        // 312/M-93: der Host baut den Editor in seiner Reaktion auf die
        // Host-Dirty-Meldung ab - nach dem Eintritt des Rueckrufs, innerhalb von
        // neueSensorId(), vor dem Statuszugriff.
        abbauer.abbau = [&]
        {
            l.eintritteBeimAbbau = vogel->eintritte;
            l.kennungBeimAbbau = proz->holeZustandKopie().common.instanceId;
            editor.reset();
            vogel->lebt = false;             // Ende des Eigentuemers
            ++l.abgebaut;
        };
        proz->addListener (&abbauer);
    }

    if (neuKnopf != nullptr)
        neuKnopf->triggerClick();   // der gepostete Weg von Enter
    if (folge == KonfliktFolge::editorLebt)
    {
        // 312/M-92: der Klick laeuft bei lebendem Editor; danach Panel, Editor, Prozessor.
        l.panelWeg = pumpe (5000, [&] { return panelSicher == nullptr; });
        if (auto* ed = dynamic_cast<eqcop::EqCopilotEditor*> (editor.get()))
            l.status = ed->statusMeldungFuerTest();
        editor.reset();
        vogel->lebt = false;
    }
    else if (folge == KonfliktFolge::editorEndeImRueckruf)
    {
        // 312/M-93: pumpen, bis die Box nach dem Abbau im Rueckruf fort ist; der
        // Prozessor lebt weiter. Lief kein Abbau (dann rot), baut der Lauf den
        // Editor vor dem Prozessor ab.
        l.panelWeg = pumpe (5000, [&] { return panelSicher == nullptr; });
        abbauer.abbau = nullptr;
        if (editor != nullptr)
        {
            editor.reset();
            vogel->lebt = false;
        }
    }
    else
    {
        // 312/M-91: Editor zerstoeren, in der zweiten Variante danach auch den
        // Prozessor, DANN die Nachrichtenschleife pumpen.
        editor.reset();
        vogel->lebt = false;             // Ende des Eigentuemers
        if (folge == KonfliktFolge::editorUndProzessorEnde)
        {
            l.kennungBeimEnde = proz->holeZustandKopie().common.instanceId;
            l.dirty = dirty.nichtParameter;
            proz->removeListener (&dirty);
            proz.reset();
        }
        l.panelWeg = pumpe (5000, [&] { return panelSicher == nullptr; });
    }
    if (proz != nullptr)
    {
        l.kennungBeimEnde = proz->holeZustandKopie().common.instanceId;
        l.dirty = dirty.nichtParameter;
        proz->removeListener (&dirty);
        proz->removeListener (&abbauer);
        proz.reset();
    }
    l.marke = vogel->marke;
    eqcop::testzugang::konfliktMarkeFuerTest() = {};
    return l;
}

std::string konfliktText (const KonfliktLauf& l)
{
    return "geoeffnet " + std::string (l.geoeffnet ? "ja" : "NEIN") + ", Panel abgebaut "
         + (l.panelWeg ? "ja" : "NEIN") + ", Marke " + std::to_string (l.marke) + ", Host-Dirty "
         + std::to_string (l.dirty) + ", Kennung " + (l.kennungBeimEnde == l.kennungVorher ? "unveraendert" : "NEU")
         + (l.kennungBeimEnde.isEmpty() ? " (leer)" : "") + ", Status '" + l.status.toStdString() + "'";
}

bool nak312Konfliktpanel()
{
    std::printf ("== NAK-312 Etappe 6b - das Kennungskonflikt-Panel (312/M-91 bis 312/M-93, R-312-2, R-312-23) ==\n");
    int geprueft = 0, fehler = 0;
    const auto pruefe = [&] (bool ok, const std::string& was, const std::string& detail)
    {
        ++geprueft;
        if (! ok) ++fehler;
        std::printf ("  %s %s  [%s]\n", ok ? "ok     " : "FEHLER ", was.c_str(), detail.c_str());
    };

    const auto a = konfliktLauf (KonfliktFolge::editorEnde);
    pruefe (a.geoeffnet && a.panelWeg && a.marke == 0 && a.dirty == 0 && a.kennungBeimEnde == a.kennungVorher,
            "312/M-91 (a) konfliktrueckruf_nach_editorende_ist_wirkungslos (Prozessor lebt): Legacy-Gen, Popover "
            "offen, triggerClick auf 'Dieser Instanz eine neue Kennung geben', Editor zerstoert, DANN stellt die "
            "Nachrichtenschleife den Klick zu - Marke 0, Kennung unveraendert, 0 Host-Dirty",
            konfliktText (a));
    const auto b = konfliktLauf (KonfliktFolge::editorUndProzessorEnde);
    pruefe (b.geoeffnet && b.panelWeg && b.marke == 0 && b.dirty == 0 && b.kennungBeimEnde == b.kennungVorher,
            "312/M-91 (b) konfliktrueckruf_nach_editorende_ist_wirkungslos (Prozessor danach zerstoert): dasselbe, "
            "der Prozessor stirbt vor der Zustellung - Marke 0, Kennung bis zum Prozessorende unveraendert, "
            "0 Host-Dirty",
            konfliktText (b));
    const auto c = konfliktLauf (KonfliktFolge::editorLebt);
    pruefe (c.geoeffnet && c.panelWeg && c.marke == 0 && c.dirty == 1 && c.kennungBeimEnde != c.kennungVorher
                && c.kennungBeimEnde.isNotEmpty() && c.status.startsWith ("Neue Kennung vergeben"),
            "312/M-92 normaler_handgriff_bleibt (Teilfall von 312/M-91, R-312-2 letzter Satz): der Klick laeuft "
            "bei lebendem Editor - neue Kennung, genau EINE Host-Dirty-Meldung, die Statusmeldung steht, die Box "
            "schliesst",
            konfliktText (c));
    const auto d = konfliktLauf (KonfliktFolge::editorEndeImRueckruf);
    pruefe (d.geoeffnet && d.panelWeg && d.abgebaut == 1 && d.eintritteBeimAbbau == 1
                && d.kennungBeimAbbau != d.kennungVorher && d.marke == 0 && d.dirty == 1
                && d.kennungBeimEnde != d.kennungVorher && d.kennungBeimEnde.isNotEmpty(),
            "312/M-93 konfliktrueckruf_ueberlebt_editorabbau_im_rueckruf (R-312-2 Satz 2): der Klick beginnt bei "
            "lebendem Editor, der Host baut ihn in seiner Reaktion auf die Host-Dirty-Meldung ab (Listener in "
            "audioProcessorChanged, innerhalb von neueSensorId) - die Kennung wechselt trotzdem, genau EINE "
            "Host-Dirty-Meldung, danach kein Zugriff auf den Editor (Marke 0)",
            konfliktText (d) + ", Abbau im Rueckruf " + std::to_string (d.abgebaut) + " nach "
                + std::to_string (d.eintritteBeimAbbau) + " Eintritt(en), Kennung beim Abbau "
                + (d.kennungBeimAbbau != d.kennungVorher ? "schon NEU" : "noch alt"));

    std::printf ("NAK-312 KONFLIKT %d geprueft, %d Fehler\n", geprueft, fehler);
    return fehler == 0 && geprueft == 4;
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
        // NAK-312 Etappe 6b (R-312-9): der Labelentwurf der Main-Flaeche.
        const bool label = nak312Labelentwurf();
        // NAK-312 Etappe 6b (R-312-6): Ersatzziel und Aktionssteuerung.
        const bool ziel = nak312Ersatzziel (ordner);
        // NAK-312 Etappe 6b (NAK-349, R-312-2): das Kennungskonflikt-Panel.
        const bool konflikt = nak312Konfliktpanel();
        return shots == 0 && panel && label && ziel && konflikt ? 0 : 1;
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
