// Independent boundary roundtrip through the existing processor and state APIs.
#define main existing_project_reload_test_main
#include "C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp"
#undef main
#include <limits>

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto a = juce::String (id ('a')), b = juce::String (id ('b'));
    using namespace nakama::state;
    for (bool maximum : {false, true})
    {
        for (int kind = 0; kind < 4; ++kind)
        {
            auto p = nak246d3::mainAnlegen (false, true);
            auto z = p->holeZustandKopie();
            bool changed = false;
            juce::String why;
            if (kind == 0) setzeIntent (z, a, {}, Rolle::fuehrt, IntentHerkunft::user, 1.0, changed, why);
            if (kind == 1) setzeSchutzangabe (z, a, Schutzeigenschaft::band, 0, 1, changed, why);
            if (kind == 2) setzeBeziehung (z, a, b, Beziehungsart::fuehrtVor, changed, why);
            if (kind == 3) setzeAssistentenschritt (z, a, Assistentenschritt::coverage, changed, why);
            const auto revision = std::numeric_limits<juce::int64>::max() - (maximum ? 0 : 1);
            if (kind == 3) z.assistent.revision = revision;
            else z.intentBestandRevision = revision;
            juce::MemoryBlock input;
            speichere (z, input);
            p->setStateInformation (input.getData(), static_cast<int> (input.getSize()));
            pruefe (!p->stateNurLesen(), "legal boundary state loads writable");
            DirtyZaehler dirty;
            p->addListener (&dirty);
            bool accepted = false;
            if (kind == 0) accepted = p->entferneQuellenrolle (a, {});
            if (kind == 1) accepted = p->hebeQuellenschutzAuf (a, Schutzeigenschaft::band, 0, 1);
            if (kind == 2) accepted = p->entferneQuellenbeziehung (a, b);
            if (kind == 3) accepted = p->assistentUeberspringen();
            auto after = p->holeZustandKopie();
            const auto remaining = kind == 0 ? after.sourceIntents.size()
                                 : kind == 1 ? after.schutzangaben.size()
                                 : after.intentBeziehungen.size();
            juce::MemoryBlock output;
            p->getStateInformation (output);
            Zustand roundtrip;
            const auto result = lade (output.getData(), output.getSize(), Bundle::eqcp(), roundtrip);
            std::cout << "REVISION_BOUNDARY kind=" << kind << " input_revision=" << revision
                      << " action_return=" << accepted << " dirty=" << dirty.nonParam.load()
                      << " remaining=" << remaining << " assistant_revision=" << after.assistent.revision
                      << " reload_readonly=" << roundtrip.nurLesen << " reason=" << roundtrip.grund << '\n';
            const bool intact = kind == 3 ? !roundtrip.nurLesen
                              : (maximum ? (!accepted && remaining == 1 && dirty.nonParam == 0)
                                         : (accepted && remaining == 0 && dirty.nonParam == 1));
            pruefe (intact, "rejected change preserves state; accepted output remains reloadable");
            p->removeListener (&dirty);
        }
    }
    std::cout << "AUDIT REVISIONS " << bestanden << '/' << (bestanden + fehler) << " expectations met\n";
    return fehler == 0 ? 0 : 1;
}
