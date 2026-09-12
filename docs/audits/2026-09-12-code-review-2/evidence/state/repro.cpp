// Independent audit probe: execute current production processor methods.
// Reuse only existing test infrastructure; no production logic is copied.
#define main existing_project_reload_test_main
#include "C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp"
#undef main

int main (int argc, char**)
{
    juce::ScopedJuceInitialiser_GUI gui;
    const bool interleave = argc == 1;
    std::cout << "MODE=" << (interleave ? "worker-message-thread-interleave" : "serial-counterexample") << '\n';
    using P = eqcop::EqCopilotProcessor;
    using M = eqcop::SourcesModel::Mitgliedschaft;
    const auto a = id ('a');
    const auto b = id ('b');
    nak246::Schranke gate;
    std::atomic<int> publications { 0 };
    auto p = nak246d3::mainAnlegen (false, true);
    auto confirmed = [&] (const std::string& who) {
        for (const auto& row : p->sourcesSicht().quellen)
            if (row.instanceId == who) return row.mitgliedschaft == M::bestaetigt;
        return false;
    };
    p->setzeSourcesFixtureFuerTest (lebendeQuelle (b));
    pruefe (p->bindeSourcesHauptziel (b), "setup: public Join B accepted");
    p->v3AntwortFuerTest (ack (commandId (p->ausstehenderSourcesCommandFuerTest()), true));
    p->sourcesTick();
    pruefe (nak246d3::genau (nak246d3::mitglieder (*p), { b }) && confirmed (b),
            "setup: B confirmed in State and model");
    auto view = lebendeQuelle (a);
    auto rowB = lebendeQuelle (b).quellen.front();
    rowB.mitgliedschaft = M::bestaetigt;
    rowB.hauptziel = false;
    rowB.runtimeNonce = id ('d');
    view.quellen.push_back (rowB);
    p->setzeSourcesFixtureFuerTest (std::move (view));
    p->setzeSourcesPublikationHakenFuerTest ([&] (std::size_t) {
        if (publications.fetch_add (1) == 0 && interleave) gate.halten();
    });
    p->setzeWorkerDrainFuerTest (interleave);
    pruefe (p->bindeSourcesHauptziel (a), "public Join A accepted");
    p->v3AntwortFuerTest (ack (commandId (p->ausstehenderSourcesCommandFuerTest()), true));
    if (!interleave) p->sourcesTick();
    const bool held = !interleave || gate.warteBisErreicht (5000);
    pruefe (held, "Join A applied; interleave mode pauses real worker before publication");
    if (!held) { gate.freigeben(); return 2; }
    pruefe (nak246d3::genau (nak246d3::mitglieder (*p), { b, a }),
            "worker has already applied Join A to State");
    pruefe (!p->sourcesDrainRiegelGehaltenFuerTest(),
            "publication pause holds no drain mutex");
    pruefe (confirmed (b), "B is still confirmed, so normal Unbind is enabled");
    pruefe (p->waehleSourcesHauptziel (b), "public select B accepted");
    pruefe (p->entferneSourcesHauptziel (b), "public Unbind B accepted");
    p->v3AntwortFuerTest (ack (commandId (p->ausstehenderSourcesCommandFuerTest()), true));
    p->sourcesTick(); // exactly the public call made by the editor's message-thread timer
    pruefe (nak246d3::genau (nak246d3::mitglieder (*p), { a }) && !confirmed (b),
            "message-thread drain completes Unbind B before worker publication");
    const auto entries = p->sourcesDrainEintritteFuerTest();
    gate.freigeben();
    pruefe (!interleave || warteAuf (5000, [&] { return p->sourcesDrainEintritteFuerTest() > entries; }),
            "both publications finished");
    const bool stateA = nak246d3::genau (nak246d3::mitglieder (*p), { a });
    const bool modelB = confirmed (b);
    std::cout << "RESULT state_only_A=" << stateA << " model_confirms_removed_B=" << modelB
              << " publication_calls=" << publications.load() << '\n';
    pruefe (stateA && !modelB, "model must reflect the latest committed membership");
    juce::MemoryBlock saved;
    p->getStateInformation (saved);
    nakama::state::Zustand restored;
    auto result = nakama::state::lade (saved.getData(), saved.getSize(), nakama::state::Bundle::eqcp(), restored);
    std::cout << "saved_bytes=" << saved.getSize() << " saved_member_count=" << restored.mainProjectMitglieder.size()
              << " load_result=" << static_cast<int> (result) << '\n';
    pruefe (restored.mainProjectMitglieder.size() == 1
            && restored.mainProjectMitglieder.front().instanceId == juce::String (a),
            "saved project contains only A while model was stale");
    std::cout << "AUDIT PUBLICATION " << bestanden << '/' << (bestanden + fehler) << " expectations met\n";
    return fehler == 0 ? 0 : 1;
}
