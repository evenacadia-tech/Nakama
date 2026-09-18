// Echte Prozessoren und echte Save/Load-/Hostparameter-/Wire-Eingaenge.
// Kein FL/Wrapper. P0-Gegenstelle ist ein Testserver, kein echter Broker.
#include "SondeProcessor.h"
#include "PluginProcessor.h"
#include "NakamaPreset.h"
#include "WireEnvelope.h"
#include "IpcVerbindung.h"
#include "PipeToken.h"
#include "controlclient/Intern.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
namespace {
using namespace nakama::ipc;
#include "V3TestServer.h"
using Probe = nakama::sonde::SondeProcessor;
namespace par = nakama::parameter;
namespace tx = nakama::transaktion;
int fails=0, checks=0;
void check(bool ok,const char* label) { ++checks; if(!ok) ++fails; std::cout<<(ok?"PASS ":"FAIL ")<<label<<std::endl; }
template<class F> bool waitFor(F f,int ms=5000) {
    for(int i=0;i<ms/5;++i) { if(f()) return true; std::this_thread::sleep_for(std::chrono::milliseconds(5)); } return f();
}
struct Dirty final : juce::AudioProcessorListener {
    std::atomic<int> count{0};
    void audioProcessorParameterChanged(juce::AudioProcessor*,int,float) override {}
    void audioProcessorChanged(juce::AudioProcessor*,const ChangeDetails& d) override { if(d.nonParameterStateChanged) ++count; }
};
std::unique_ptr<Probe> probe() {
    auto p=std::make_unique<Probe>(); p->setRateAndBufferSizeDetails(48000,256); p->prepareToPlay(48000,256); return p;
}
tx::Auftrag order(Probe& p,tx::Art kind=tx::Art::apply) {
    tx::Auftrag a; a.tid=p.neueTid(); a.baseRevision=p.stateRevision(); a.art=kind; a.satz=p.bestaetigterZustand(); return a;
}
void host(Probe& p,int index,float v) {
    auto* q=dynamic_cast<juce::RangedAudioParameter*>(p.getParameters()[index]);
    if(!q) { check(false,"Hostparameter vorhanden"); return; } q->setValueNotifyingHost(q->convertTo0to1(v));
}
double steadyGain(Probe& p) {
    juce::AudioBuffer<float> b(2,256); juce::MidiBuffer midi; double sum=0;
    for(int block=0;block<30;++block) {
        for(int c=0;c<2;++c) for(int i=0;i<256;++i) b.setSample(c,i,0.1f);
        p.processBlock(b,midi); if(block==29) for(int i=0;i<256;++i) sum+=b.getSample(0,i)/0.1;
    } return 20*std::log10(sum/256);
}
void persistence() {
    auto p=probe(); Dirty dirty; p->addListener(&dirty);
    auto a=order(*p); a.satz.werte[par::kIndexEqEnabled].b=true;
    const int trim=par::indexVonId("v1.global.output_trim_db"); check(trim>=0,"Trim-ID aus Vertrag"); if(trim<0) {p->removeListener(&dirty);return;}
    a.satz.werte[(size_t)trim].zahl=3; const auto first=p->fuehreTransaktionAus(a); const auto repeated=p->fuehreTransaktionAus(a);
    check(first.ausgang==tx::Ausgang::commit && repeated.ausgang==tx::Ausgang::gespeichertesErgebnis && first.revision==1 && repeated.revision==1 && p->stateRevision()==1 && dirty.count==1,"Commit/Retry: genau ein Dirty und eine Revision");
    auto stale=order(*p); stale.baseRevision=0; stale.satz.werte[(size_t)trim].zahl=6;
    check(p->fuehreTransaktionAus(stale).ausgang==tx::Ausgang::konflikt && dirty.count==1,"Veraltete Revision ohne Dirty abgelehnt");
    juce::MemoryBlock saved; p->getStateInformation(saved); const auto hello=p->v3HelloFuerTest();
    auto q=probe(); q->setStateInformation(saved.getData(),(int)saved.getSize()); const auto qhello=q->v3HelloFuerTest();
    check(hello.adresse.instanceId==qhello.adresse.instanceId && hello.adresse.runtimeNonce!=qhello.adresse.runtimeNonce,"Klon: persistente ID gleich, Laufnonce verschieden");
    check(p->zustandLesen().undoRing==q->zustandLesen().undoRing && p->stateHashText()==q->stateHashText(),"Klon: Undo-Ring und Hash verlustfrei");
    const auto ue=q->fuehreTransaktionAus(order(*q,tx::Art::undo)); const bool undoNeutral=q->bestaetigterZustand()==par::DspSatz{};
    const auto re=q->fuehreTransaktionAus(order(*q,tx::Art::redo));
    check(ue.ausgang==tx::Ausgang::commit && undoNeutral && re.ausgang==tx::Ausgang::commit && q->bestaetigterZustand()==p->bestaetigterZustand(),"Undo/Redo nach Reload erhalten gespeicherte Geschichte");
    auto preview=p->bestaetigterZustand(); preview.werte[(size_t)trim].zahl=-6; juce::String reason;
    check(p->setzePreview(preview,reason),"Preview aufgebaut"); p->setStateInformation(saved.getData(),(int)saved.getSize());
    check(!p->previewAktiv() && p->v3HelloFuerTest().adresse.runtimeNonce==hello.adresse.runtimeNonce && dirty.count==1,"Same-Instance: Preview beendet, Nonce bleibt, kein Dirty");
    auto preset=preview; preset.werte[par::kIndexEqEnabled].b=false; juce::MemoryBlock presetBytes;
    check(nakama::preset::schreibe(preset,presetBytes,reason),"Eigenes Preset geschrieben");
    auto pa=order(*p,tx::Art::presetLaden); pa.preset=presetBytes; const auto before=p->zustandLesen().common;
    const auto pe=p->fuehreTransaktionAus(pa); const auto after=p->zustandLesen().common;
    check(pe.ausgang==tx::Ausgang::commit && before.instanceId==after.instanceId && before.label==after.label && p->bestaetigterZustand().werte[par::kIndexEqEnabled].b && p->bestaetigterZustand().werte[(size_t)trim].zahl==-6,"DSP-Preset uebernimmt Klang und erhaelt Empfaengeridentitaet, Label und eq_enabled");
    p->removeListener(&dirty);
}
void recallCounterexamples() {
    const int trim=par::indexVonId("v1.global.output_trim_db"); if(trim<0) return;
    auto p=probe(); auto a=order(*p); a.satz.werte[par::kIndexEqEnabled].b=true; a.satz.werte[(size_t)trim].zahl=3;
    check(p->fuehreTransaktionAus(a).ausgang==tx::Ausgang::commit,"T3-05-01 Setup +3 dB committed");
    steadyGain(*p); juce::MemoryBlock saved; p->getStateInformation(saved);
    host(*p,trim,-9); p->setStateInformation(saved.getData(),(int)saved.getSize()); p->kontrollTaktFuerTest();
    const double gain=steadyGain(*p), effective=p->wirksamerZustand().werte[(size_t)trim].zahl, committed=p->bestaetigterZustand().werte[(size_t)trim].zahl;
    std::cout<<"T3-05-01 committed="<<committed<<" effective="<<effective<<" output_db="<<gain<<std::endl;
    check(committed==3 && std::abs(effective+9)<0.001 && std::abs(gain+9)<0.01,"T3-05-01 reproduziert: ausstehender Hostwert ueberschreibt Recall-Audio");
    juce::MemoryBlock foreign; check(juce::File("C:/na-audit16-aff2d818/eq-copilot/fixtures/state/schema2/fremdes-major-3.bin").loadFileAsData(foreign),"Fremd-Major Fixture gelesen");
    p->setStateInformation(foreign.getData(),(int)foreign.getSize()); const double neutral=steadyGain(*p);
    host(*p,par::kIndexEqEnabled,1); host(*p,trim,6); p->kontrollTaktFuerTest();
    const double activated=steadyGain(*p); juce::MemoryBlock roundtrip; p->getStateInformation(roundtrip);
    std::cout<<"T3-05-02 neutral_db="<<neutral<<" automated_db="<<activated<<std::endl;
    check(p->zustandLesen().nurLesen && roundtrip==foreign && std::abs(neutral)<0.01 && std::abs(activated-6)<0.01,"T3-05-02 reproduziert: Originalbytes erhalten, Read-only-Audio aktiviert");
}
eqcop::SourcesModel::Sicht sourceFixture(const std::string& id) {
    eqcop::SourcesModel::Sicht s; s.subscriptionAktiv=true; s.mainDarfSchreiben=true; s.fuehrendesMain=std::string(32,'f');
    eqcop::SourcesModel::Zeile q; q.instanceId=id; q.runtimeNonce=std::string(32,'b'); q.pluginKind="active_probe";
    q.control=eqcop::SourcesModel::Control::verbunden; q.messung=eqcop::SourcesModel::Messung::fresh;
    q.mitgliedschaft=eqcop::SourcesModel::Mitgliedschaft::unclassified; q.hauptziel=true; q.descriptorVorhanden=true;
    q.betrieb=eqcop::SourcesModel::Betrieb::active; q.messpunkt=eqcop::SourcesModel::Messpunkt::insert;
    s.quellen.push_back(q); return s;
}
std::vector<std::string> commands(TestServer& s) {
    std::lock_guard<std::mutex> l(s.textMutex); std::vector<std::string> result;
    for(const auto& text:s.p0Texte) { auto v=juce::JSON::parse(text); if(v["type"].toString()=="session_command") result.push_back(v["command_id"].toString().toStdString()); } return result;
}
void replay(bool reload) {
    const auto pipe=testPipeName(reload?"audit16-reload":"audit16-control"); TestServer server(pipe); server.commandAckArt.store(reload?0:1);
    if(!server.starten()) {check(false,"Probe-Server gestartet");return;}
    auto p=std::make_unique<eqcop::EqCopilotProcessor>(); p->prepareToPlay(48000,256); p->setzeEditorOffen(true); p->setzeWorkerDrainFuerTest(false);
    check(p->setzeBindung("hub","Gen",""),"Gen Main initialisiert"); juce::MemoryBlock saved; p->getStateInformation(saved);
    check(p->v3ProbeGegenstelleFuerTest(pipe,testExeErwartung()),"Gen isolierte Probe-Pipe gesetzt"); p->v3StartFuerTest();
    check(waitFor([&]{return p->controlV3Snapshot().status==ControlClient::Status::verbunden;}),"Gen echter ControlClient verbunden");
    Dirty dirty; p->addListener(&dirty); p->setzeSourcesFixtureFuerTest(sourceFixture(std::string(32,'a')));
    check(p->bindeSourcesHauptziel(std::string(32,'a')),"Produkt-Join mit expliziter Sichtfixture eingereiht");
    check(waitFor([&]{return server.commandAckEntschieden.load()>=1;}),"Server erster P0-Entscheidungspunkt erreicht");
    if(reload) { check(p->controlV3Snapshot().inFlight==1,"Vor Reload wartet ein echter InFlight-Auftrag"); server.commandAckArt.store(1); p->setStateInformation(saved.getData(),(int)saved.getSize()); }
    check(waitFor([&]{return p->controlV3Snapshot().inFlight==0;}),"P0-Endergebnis vom echten Client konsumiert");
    waitFor([&]{return reload?commands(server).size()>=2:p->bestaetigteSourcesCommandsFuerTest()>=1;});
    juce::MemoryBlock after; p->getStateInformation(after); const auto cs=commands(server); const auto members=p->holeZustandKopie().mainProjectMitglieder.size();
    std::cout<<"F02 reload="<<reload<<" wire_commands="<<cs.size()<<" members="<<members<<" dirty="<<dirty.count.load()<<" replays="<<p->controlV3Snapshot().inFlightWiederholungen<<std::endl;
    if(reload) check(cs.size()>=2 && cs[0]==cs[1] && members==0 && dirty.count==0 && p->controlV3Snapshot().inFlightWiederholungen==1,"NAK-278 reproduziert: gleiche ID wiederholt, ACK-Erfolg ohne lokale Mitgliedschaft");
    else check(cs.size()==1 && members==1 && dirty.count==1,"F02 Gegenprobe ohne Reload: Mitgliedschaft und Dirty");
    p->removeListener(&dirty); p->v3StopFuerTest(); p.reset(); server.stoppen();
}
void genDescriptorAndIntent() {
    auto p=std::make_unique<eqcop::EqCopilotProcessor>(); p->prepareToPlay(48000,256); p->setzeBindung("hub","Gen","");
    const auto h=p->v3HelloFuerTest(); const auto st=p->v3StatusFuerTest();
    std::cout<<"Gen runtime.gemeldet="<<st.runtime.gemeldet<<std::endl;
    check(!st.runtime.gemeldet,"A-01 Gegenpruefung: echter Gen-Status ohne Runtime-Descriptor");
    const juce::String source(std::string(32,'a'));
    check(p->setzeQuellenrolle(source,"",nakama::state::Rolle::traegt,nakama::state::IntentHerkunft::user,1.0),"Intent erste Rolle gesetzt");
    juce::MemoryBlock saved; p->getStateInformation(saved); const auto r=p->intentBestandRevision();
    check(p->setzeQuellenrolle(source,"",nakama::state::Rolle::fuehrt,nakama::state::IntentHerkunft::user,1.0),"Intent zweite Rolle gesetzt");
    const auto newer=p->intentBestandRevision(); p->setStateInformation(saved.getData(),(int)saved.getSize()); const auto h2=p->v3HelloFuerTest();
    std::cout<<"T3-15-03 older="<<r<<" newer="<<newer<<" reloaded="<<p->intentBestandRevision()<<" full="<<p->v3IntentUpdateFuerTest(true)<<std::endl;
    check(newer>r && p->intentBestandRevision()==r && h.adresse.sessionEpoch==h2.adresse.sessionEpoch && h.adresse.runtimeNonce==h2.adresse.runtimeNonce,"T3-15-03 Eingang: aelterer Vollbestand, gleiche Session/Nonce");
}
}
int main() {
    juce::ScopedJuceInitialiser_GUI juce; std::cout<<std::setprecision(12)<<"Audit16State: Reproduktionsassertions sind kein Produkt-PASS.\n";
    persistence(); recallCounterexamples(); genDescriptorAndIntent(); replay(false); replay(true);
    std::cout<<"checks="<<checks<<" failures="<<fails<<std::endl; return fails?1:0;
}
