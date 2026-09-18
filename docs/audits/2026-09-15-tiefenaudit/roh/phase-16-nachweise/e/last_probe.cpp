// Phase 16 E: echte Prozessoren, unveraenderte Produktquellen.
// Kein VST3-/FL-Host. V3 ist im Testbau aus; der Gen-v2-OS-Aufruf wird durch
// ../c/PipeClientIsoliert.cpp ausschliesslich auf einen PID-Probenamen gelenkt.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "PluginProcessor.h"
#include "SondeProcessor.h"
#include "NakamaParameter.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#pragma comment(lib, "psapi.lib")

namespace {
using Probe = nakama::sonde::SondeProcessor;
namespace par = nakama::parameter;
namespace tx = nakama::transaktion;
using Clock = std::chrono::steady_clock;
int failures = 0;
void require (bool ok, const std::string& s) {
    std::cout << (ok ? "PASS " : "FAIL ") << s << '\n';
    if (!ok) ++failures;
}
double us (Clock::duration d) { return std::chrono::duration<double, std::micro>(d).count(); }
void messages (int ms) {
    const auto until = Clock::now() + std::chrono::milliseconds(ms);
    do {
        MSG m{};
        while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&m); DispatchMessageW(&m); }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (Clock::now() < until);
}
struct Resource { DWORD handles{}, threads{}; SIZE_T privateBytes{}, workingSet{}; };
Resource resource () {
    Resource r;
    if(!GetProcessHandleCount(GetCurrentProcess(), &r.handles)) throw std::runtime_error("GetProcessHandleCount failed");
    PROCESS_MEMORY_COUNTERS_EX m{}; m.cb=sizeof(m);
    if (!GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&m), sizeof(m)))
        throw std::runtime_error("GetProcessMemoryInfo failed");
    r.privateBytes=m.PrivateUsage; r.workingSet=m.WorkingSetSize;
    HANDLE h=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (h==INVALID_HANDLE_VALUE) throw std::runtime_error("thread snapshot failed");
    THREADENTRY32 t{}; t.dwSize=sizeof(t);
    if(!Thread32First(h,&t)){CloseHandle(h);throw std::runtime_error("Thread32First failed");}
    do { if(t.th32OwnerProcessID==GetCurrentProcessId()) ++r.threads; } while(Thread32Next(h, &t));
    CloseHandle(h);
    return r;
}
void emitResource(std::ofstream& file, const std::string& name, int cycle) {
    auto r=resource();
    file << name << ',' << cycle << ',' << r.handles << ',' << r.threads << ',' << r.privateBytes << ',' << r.workingSet << '\n';
    std::cout << "RESOURCE " << name << " cycle=" << cycle << " handles=" << r.handles << " threads=" << r.threads
              << " private_bytes=" << r.privateBytes << " working_set=" << r.workingSet << '\n';
}
par::DspSatz activeState(int index=0) {
    par::DspSatz s;
    s.werte[(size_t)par::kIndexEqEnabled].b=true;
    s.werte[(size_t)par::kIndexAutoGain].b=(index%2)==0;
    for(int slot=0;slot<8;++slot) {
        auto setNumber=[&](int field,double value){s.werte[(size_t)par::indexBandV1(slot,field)].zahl=value;};
        s.werte[(size_t)par::indexOccupied(slot)].b=true;
        s.werte[(size_t)par::indexBandV1(slot,par::kEnabled)].b=true;
        s.werte[(size_t)par::indexBandV1(slot,par::kChannelMode)].enumIndex=slot%5;
        setNumber(par::kFreqHz, 80.0*std::pow(1.75,slot));
        setNumber(par::kQ, .8+.2*slot);
        setNumber(par::kGainDb, slot%2 ? -3.0 : 3.0);
        s.werte[(size_t)par::indexBandV1(slot,par::kDynamicEnabled)].b=slot<4;
        s.werte[(size_t)par::indexBandV1(slot,par::kSidechainSource)].enumIndex=1; // internal: Detektor wirklich an
        setNumber(par::kDynamicRangeDb,-6.0);
        setNumber(par::kThresholdDb,-35.0);
        setNumber(par::kAttackMs,5.0); setNumber(par::kHoldMs,10.0); setNumber(par::kReleaseMs,120.0);
    }
    return s;
}
void apply (Probe& p, const par::DspSatz& s) {
    tx::Auftrag a; a.tid=p.neueTid(); a.baseRevision=p.stateRevision(); a.art=tx::Art::apply; a.satz=s;
    auto e=p.fuehreTransaktionAus(a);
    if(e.ausgang!=tx::Ausgang::commit) throw std::runtime_error(std::string("apply: ")+tx::wort(e.ausgang)+" "+e.grund.toStdString());
}
void bindProbe(Probe& p) {
    // Derselbe gebaute Stateeingang wie im Produkt, keine handgesetzte Frame-
    // oder Telemetrievorbedingung. Die frische Instanz-ID bleibt eindeutig.
    auto s=p.zustandLesen();
    s.common.projectBindingId="abababababababababababababababab";
    s.common.position=nakama::state::Messposition::insert;
    juce::MemoryBlock bytes;nakama::state::speichere(s,bytes);
    p.setStateInformation(bytes.getData(),(int)bytes.getSize());
}
void hostValue(Probe& p,int index,float value) {
    auto* q=dynamic_cast<juce::RangedAudioParameter*>(p.getParameters()[index]);
    if(!q) throw std::runtime_error("host parameter missing");
    q->setValueNotifyingHost(q->convertTo0to1(value));
}
void prepare(juce::AudioProcessor& p,double rate,int block,int channels) {
    auto layout=p.getBusesLayout();
    layout.getChannelSet(true,0)=channels==1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo();
    layout.getChannelSet(false,0)=layout.getChannelSet(true,0);
    if(!p.setBusesLayout(layout)) throw std::runtime_error("layout rejected");
    p.setRateAndBufferSizeDetails(rate,block); p.prepareToPlay(rate,block);
}
eqcop::hostbruecke::Blockbefund stamp(std::int64_t sample,int n,double rate,bool playing=true) {
    eqcop::hostbruecke::Blockbefund b;
    b.blockGroesse=(std::uint32_t)n;
    b.kontext.processContextPresent=true;
    b.kontext.projectTimeSamples.setze(sample); b.kontext.continuousTimeSamples.setze(sample);
    b.kontext.playing.setze(playing); b.kontext.recording.setze(false); b.kontext.sampleRate.setze(rate);
    b.kontext.tempo.setze(120.0); b.kontext.ppqPosition.setze(2.0*(double)sample/rate);
    return b;
}
void fill(juce::AudioBuffer<float>& b,std::int64_t at,double rate,int instance=0) {
    for(int c=0;c<b.getNumChannels();++c) for(int i=0;i<b.getNumSamples();++i) {
        const double t=(double)(at+i)/rate;
        b.setSample(c,i,(float)(.07*std::sin(6.283185307179586*(173.0+instance*7.0+c*47.0)*t)
                         +.03*std::sin(6.283185307179586*(997.0+c*139.0)*t)));
    }
}
double maxAbs(const juce::AudioBuffer<float>& b) {
    double v=0; for(int c=0;c<b.getNumChannels();++c) for(int i=0;i<b.getNumSamples();++i) v=std::max(v,std::abs((double)b.getSample(c,i))); return v;
}
bool finiteAudio(const juce::AudioBuffer<float>& b) {
    for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<b.getNumSamples();++i)if(!std::isfinite(b.getSample(c,i)))return false;
    return true;
}
void lifecycle(std::ofstream& resources) {
    std::cout << "SECTION lifecycle: echte Callbackeintritte; keine Host-Wrapper-Simulation behauptet\n";
    auto p=std::make_unique<Probe>();
    prepare(*p,48000,333,2); apply(*p,activeState());
    juce::AudioBuffer<float> b(2,333); juce::MidiBuffer midi;
    for(int i=0;i<40;++i){fill(b,i*333,48000);p->processBlock(b,midi);}
    juce::MemoryBlock before; p->getStateInformation(before);
    p->reset(); b.clear(); p->processBlock(b,midi);
    juce::MemoryBlock after; p->getStateInformation(after);
    require(maxAbs(b)==0.0 && before==after,"reset: aktive 8-Band-Historie endet; Stille exakt null; State bytegleich");
    p->releaseResources(); fill(b,0,48000); auto dry=b;
    p->processBlock(b,midi);
    bool equal=true;for(int c=0;c<2;++c) equal&=std::memcmp(b.getReadPointer(c),dry.getReadPointer(c),333*sizeof(float))==0;
    require(equal,"release ohne prepare: bitgleicher Rueckfall");
    prepare(*p,96000,333,1); fill(b,0,96000);
    juce::AudioBuffer<float> mono(1,333); fill(mono,0,96000);
    for(int i=0;i<5;++i){fill(mono,i*333,96000);p->processBlock(mono,midi);}
    require(p->getTotalNumInputChannels()==1 && finiteAudio(mono),"release/prepare: Rate96k Mono aktiv endlich");
    p->setStateInformation(before.getData(),(int)before.getSize());
    auto preview=activeState(); preview.werte[(size_t)par::indexBandV1(0,par::kGainDb)].zahl=-12;
    juce::String reason; const bool previewSet=p->setzePreview(preview,reason);
    p->setzeHoermatrix(nakama::dsp::Hoermatrix::candidate); p->setNonRealtime(true);
    require(previewSet && !p->previewAktiv() && p->gewuenschteHoermatrix()==nakama::dsp::Hoermatrix::processed,"offline: Preview endet und Hoermatrix Processed");
    fill(mono,333,96000); auto bypassDry=mono; p->processBlockBypassed(mono,midi);
    require(std::memcmp(mono.getReadPointer(0),bypassDry.getReadPointer(0),333*sizeof(float))==0,"JUCE-Prozessor-Hostbypass: bitgleich, ohne DSP-Callback");
    p->setNonRealtime(false); prepare(*p,48000,64,2);
    juce::AudioBuffer<float> overload(2,333); fill(overload,0,48000);
    const auto oldDrops=p->analyseDropsOhneAudioFuerTest(); p->processBlock(overload,midi);
    require(p->analyseDropsOhneAudioFuerTest()>oldDrops && finiteAudio(overload),"Block333 bei prepare64: Audio endlich, Analyseverlust gezaehlt");
    p->getStateInformation(after);
    require(before==after,"Lifecycle/Reload/Offline/Bypass veraendern bestaetigte Statebytes nicht");
    p.reset(); messages(50); emitResource(resources,"lifecycle_destroyed",-1);
}
struct Timing { int index{}, n{}; double chainUs{}, maxOneUs{}, lateStartUs{}; bool automated{}; };
double percentile(std::vector<double> a,double p){std::sort(a.begin(),a.end());return a[std::min(a.size()-1,(size_t)std::ceil(p*(double)(a.size()-1)))];}
void activeLoad(const std::string& name,double rate,const std::vector<int>& blocks,int channels,bool offline,const std::string& dir,std::ofstream& resources) {
    std::cout << "SECTION load " << name << " rate=" << rate << " channels=" << channels << " offline=" << offline << '\n';
    auto gen=std::make_unique<eqcop::EqCopilotProcessor>(); prepare(*gen,rate,333,channels);
    gen->setNonRealtime(offline);
    std::vector<std::unique_ptr<Probe>> probes; probes.reserve(10);
    std::array<juce::AudioBuffer<float>,11> buffers;
    for(auto& b:buffers)b.setSize(channels,333);
    for(int k=0;k<10;++k){auto p=std::make_unique<Probe>();bindProbe(*p);prepare(*p,rate,333,channels);apply(*p,activeState(k));p->setNonRealtime(offline);probes.push_back(std::move(p));}
    emitResource(resources,name+"_active",-1);
    std::vector<Timing> timings; timings.reserve(200000);
    juce::MidiBuffer midi;
    std::array<double,10> changed{},peak{},dynamicMax{};
    std::array<std::uint64_t,10> beginPub{};
    for(size_t k=0;k<10;++k)beginPub[k]=probes[k]->producerPublikationenFuerTest();
    const auto begin=Clock::now(); std::int64_t position=0; int index=0; int64_t nextAutomation=0; int automationRound=0;
    std::uint64_t nonfinite=0; double checksum=0;
    const auto wanted=(std::int64_t)(rate*3.0);
    while(position<wanted) {
        const int n=blocks[(size_t)index%blocks.size()];
        const auto scheduled=begin+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>((double)position/rate));
        if(!offline && Clock::now()<scheduled)std::this_thread::sleep_until(scheduled);
        const bool automation=position>=nextAutomation;
        if(automation){
            for(size_t k=0;k<10;++k){
                hostValue(*probes[k],par::indexBandV1(0,par::kGainDb), (float)((automationRound%2 ? -1.0 : 1.0)*(2.0+(double)k*.1)));
                hostValue(*probes[k],par::indexBandV1(1,par::kFreqHz),(float)(140.0+(automationRound%3)*35.0));
            }
            ++automationRound;nextAutomation=position+(std::int64_t)(rate*.02);
        }
        for(size_t k=0;k<buffers.size();++k){buffers[k].setSize(channels,n,false,false,true);fill(buffers[k],position,rate,(int)k);}
        std::array<float,10> original{};
        for(size_t k=0;k<10;++k)original[k]=buffers[k+1].getSample(0,n-1);
        const auto host=stamp(position,n,rate);
        const auto start=Clock::now();
        gen->nakamaBlockEmpfangen(host);gen->processBlock(buffers[0],midi);
        double maxOne=us(Clock::now()-start);
        for(size_t k=0;k<10;++k){const auto one=Clock::now();probes[k]->nakamaBlockEmpfangen(host);probes[k]->processBlock(buffers[k+1],midi);maxOne=std::max(maxOne,us(Clock::now()-one));}
        const auto end=Clock::now();
        timings.push_back({index,n,us(end-start),maxOne,std::max(0.0,us(start-scheduled)),automation});
        for(size_t k=0;k<10;++k){
            std::array<double,8> excursions{};probes[k]->dspKernFuerTest().auslenkungenDb(excursions.data());
            for(auto value:excursions)dynamicMax[k]=std::max(dynamicMax[k],std::abs(value));
            changed[k]=std::max(changed[k],std::abs((double)buffers[k+1].getSample(0,n-1)-original[k]));
            peak[k]=std::max(peak[k],maxAbs(buffers[k+1]));
            for(int c=0;c<channels;++c)for(int i=0;i<n;++i) if(!std::isfinite(buffers[k+1].getSample(c,i)))++nonfinite;
            checksum+=buffers[k+1].getSample(0,n-1);
        }
        position+=n;++index;
    }
    const auto wall=us(Clock::now()-begin)/1e6;
    // Nur ausserhalb der Audiotakt-Messung: endliches Drainfenster fuer Worker.
    messages(150);
    std::ofstream csv(dir+"/"+name+".csv"); csv<<std::setprecision(12)<<"block_index,frames,chain_us,max_one_us,deadline_us,late_start_us,automated\n";
    std::vector<double> t,ratios; t.reserve(timings.size());ratios.reserve(timings.size());
    size_t misses=0;double worst=0;
    for(const auto& v:timings){const double deadline=1e6*v.n/rate;csv<<v.index<<','<<v.n<<','<<v.chainUs<<','<<v.maxOneUs<<','<<deadline<<','<<v.lateStartUs<<','<<v.automated<<'\n';t.push_back(v.chainUs);ratios.push_back(v.chainUs/deadline);if(v.chainUs>deadline)++misses;worst=std::max(worst,v.chainUs/deadline);}
    std::cout<<std::setprecision(12)<<"TIMING "<<name<<" blocks="<<timings.size()<<" audio_seconds="<<(double)position/rate<<" wall_seconds="<<wall<<" mean_us="<<std::accumulate(t.begin(),t.end(),0.0)/t.size()<<" p50_us="<<percentile(t,.5)<<" p99_us="<<percentile(t,.99)<<" p999_us="<<percentile(t,.999)<<" max_us="<<*std::max_element(t.begin(),t.end())<<" misses="<<misses<<" max_deadline_ratio="<<worst<<" p99_deadline_ratio="<<percentile(ratios,.99)<<" automation_rounds="<<automationRound<<" nonfinite="<<nonfinite<<" checksum="<<checksum<<'\n';
    bool everyChanged=true,everyAnalysed=true,everyDynamic=true;
    for(size_t k=0;k<10;++k){
        auto& p=*probes[k];const auto pub=p.producerPublikationenFuerTest()-beginPub[k]; everyChanged&=changed[k]>1e-6;everyAnalysed&=pub>0;
        everyDynamic&=dynamicMax[k]>1e-6;
        std::cout<<"PROBE "<<name<<" index="<<k<<" delta_max="<<changed[k]<<" dynamic_max_db="<<dynamicMax[k]<<" peak="<<peak[k]<<" producer_publications="<<pub<<" frames_built="<<p.framesGebautFuerTest()<<" evidence_built="<<p.evidenzSnapshotsFuerTest()<<" evidence_not_sent="<<p.evidenzNichtGesendetFuerTest()<<" overflow="<<p.analyseDropsUeberlaufFuerTest()<<" oversize="<<p.analyseDropsOversizeFuerTest()<<" lost_frames="<<p.analyseVerloreneFramesFuerTest()<<" automation_epoch="<<p.automationEpoche()<<" state_revision="<<p.stateRevision()<<'\n';
    }
    std::cout<<"GEN "<<name<<" accepted_blocks="<<gen->analyseBloeckeAngenommen()<<" feature_frames="<<gen->merkmaleFrames()<<" overflow="<<gen->analyseDropsUeberlauf()<<" oversize="<<gen->analyseDropsOversize()<<" snapshot_samples="<<gen->messSnapshot().verarbeiteteSamples<<'\n';
    require(everyChanged && everyDynamic && nonfinite==0,name+": alle zehn DSP-Ausgaenge aktiv, dynamisch ausgelenkt und endlich");
    require(everyAnalysed && gen->merkmaleFrames()>0,name+": alle elf Analyseproducer tatsaechlich aktiv");
    probes.clear();gen.reset();messages(75);emitResource(resources,name+"_destroyed",-1);
}
void cycles(std::ofstream& resources) {
    std::cout<<"SECTION resources: 12 Zyklen, je 1 Gen+2 aktive Probes, 3 Editorobjekte, 3 Reloads, reset/release/prepare\n";
    emitResource(resources,"cycle_before",-1);
    for(int c=0;c<12;++c){
        {auto gen=std::make_unique<eqcop::EqCopilotProcessor>();prepare(*gen,48000,64,2);
         std::array<std::unique_ptr<Probe>,2> probes;
         for(auto& p:probes){p=std::make_unique<Probe>();prepare(*p,48000,64,2);apply(*p,activeState(c));}
         juce::AudioBuffer<float>b(2,64);juce::MidiBuffer midi;
         for(int r=0;r<3;++r){
             {std::unique_ptr<juce::AudioProcessorEditor> editor(gen->createEditorIfNeeded());if(!editor)throw std::runtime_error("Gen editor missing");messages(3);gen->editorBeingDeleted(editor.get());}
             juce::MemoryBlock state;gen->getStateInformation(state);gen->setStateInformation(state.getData(),(int)state.getSize());
             for(auto& p:probes){p->getStateInformation(state);p->setStateInformation(state.getData(),(int)state.getSize());p->reset();p->releaseResources();prepare(*p,48000,64,2);}
             for(int i=0;i<24;++i){fill(b,i*64,48000);gen->processBlock(b,midi);for(auto& p:probes){fill(b,i*64,48000);p->processBlock(b,midi);}}
         }
        }
        messages(60);emitResource(resources,"cycle_destroyed",c);
    }
}
}
int main(int argc,char**argv){
    juce::ScopedJuceInitialiser_GUI gui;
    try {
        const std::string dir=argc>1?argv[1]:".";
        std::ofstream resources(dir+"/resources.csv");if(!resources)throw std::runtime_error("output directory missing");
        resources<<"stage,cycle,handles,threads,private_bytes,working_set_bytes\n";
        SYSTEM_INFO si{};GetSystemInfo(&si);
        std::cout<<"Audit16Last build="<<__DATE__<<' '<<__TIME__<<" pid="<<GetCurrentProcessId()<<" logical_processors="<<si.dwNumberOfProcessors<<"\n";
        std::cout<<"EVIDENCE processor-component; normal process priority; scheduled chain service time includes host stamp and QueryPerformanceCounter overhead; synthetic deterministic input; no real VST3 host, ASIO device, installed plugin or live broker\n";
        emitResource(resources,"initial",-1);lifecycle(resources);
        activeLoad("stereo48k64",48000,{64},2,false,dir,resources);
        activeLoad("stereo96k64",96000,{64},2,false,dir,resources);
        activeLoad("stereo48k_mixed",48000,{1,7,64,333},2,false,dir,resources);
        activeLoad("mono48k64",48000,{64},1,false,dir,resources);
        activeLoad("offline48k333",48000,{333},2,true,dir,resources);
        cycles(resources);messages(200);emitResource(resources,"final_200ms",-1);
        messages(1800);emitResource(resources,"final_2s",-1);
        std::cout<<"ASSERTION_FAILURES="<<failures<<"\n";return failures?1:0;
    } catch(const std::exception&e){std::cout<<"HARNESS_ERROR="<<e.what()<<"\n";return 2;}
}
