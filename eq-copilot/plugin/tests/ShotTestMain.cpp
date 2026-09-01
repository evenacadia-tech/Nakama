// Editor-Sichtprüfung ohne Host (Übergabe §14.2/10): baut den echten
// Processor, speist ~20 s Rauschen + 116-Hz-Ton durch processBlock (der
// Worker misst wie im Betrieb) und rendert den Material-Editor offscreen als
// PNG — Kurve, Marker, Messleisten und Statuszellen sind dann echt belegt.
//
// Aufruf:  EqCopShot <ziel.png> [breitePx] [--state <datei.bin>]
// Läuft der Broker, verbindet sich der echte PipeClient (LED grün; der
// Sensor erscheint kurz in der Übersicht und meldet sich sauber ab) — ohne
// Broker zeigt die LED ehrlich „App aus" (rot). Beides ist Betriebsrealität.
// --state laedt vor dem Render einen Host-State (z. B. den read-only-Fall
// fixtures/state/schema2/fremdes-major-3.bin, SONDE-006) — der Sichtbeweis,
// dass der Editor einen Zustand zeigt, den es gibt.
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>

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
        return sonde012Suite (ordner);
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
