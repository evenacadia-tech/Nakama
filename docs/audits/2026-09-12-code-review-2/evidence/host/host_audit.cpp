#include "SondeProcessor.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <memory>
using P=nakama::sonde::SondeProcessor;
namespace pa=nakama::parameter;
namespace tx=nakama::transaktion;
void apply(P& p, const pa::DspSatz& z){tx::Auftrag a; a.tid=p.neueTid();a.baseRevision=p.stateRevision();a.art=tx::Art::apply;a.satz=z;auto r=p.fuehreTransaktionAus(a);std::cout<<"transaction_commit="<<(r.ausgang==tx::Ausgang::commit)<<"\n";}
void block(P& p,juce::AudioBuffer<float>& b){juce::MidiBuffer m;p.processBlock(b,m);p.kontrollTaktFuerTest();}
void silence(P& p,int n=20,int c=2){juce::AudioBuffer<float>b(c,512);for(int i=0;i<n;i++){b.clear();block(p,b);}}
void resonator(P& p){auto z=p.bestaetigterZustand();z.werte[(size_t)pa::kIndexEqEnabled].b=true;z.werte[(size_t)pa::indexOccupied(0)].b=true;z.werte[(size_t)pa::indexBandV1(0,pa::kEnabled)].b=true;z.werte[(size_t)pa::indexBandV1(0,pa::kFreqHz)].zahl=1000.;z.werte[(size_t)pa::indexBandV1(0,pa::kGainDb)].zahl=12.;z.werte[(size_t)pa::indexBandV1(0,pa::kQ)].zahl=10.;apply(p,z);silence(p);}
double peak(juce::AudioBuffer<float>&b){double v=0;for(int c=0;c<b.getNumChannels();c++)for(int n=0;n<b.getNumSamples();n++)v=std::max(v,std::abs((double)b.getSample(c,n)));return v;}
int main(){juce::ScopedJuceInitialiser_GUI init;std::cout<<std::setprecision(12);
{
auto p=std::make_unique<P>();auto layout=p->getBusesLayout();layout.inputBuses.set(0,juce::AudioChannelSet::quadraphonic());layout.outputBuses.set(0,juce::AudioChannelSet::quadraphonic());std::cout<<"quad_supported="<<p->isBusesLayoutSupported(layout)<<" quad_set="<<p->setBusesLayout(layout)<<" input_channels="<<p->getTotalNumInputChannels()<<"\n";p->setRateAndBufferSizeDetails(48000,512);p->prepareToPlay(48000,512);auto z=p->bestaetigterZustand();z.werte[(size_t)pa::kIndexEqEnabled].b=true;z.werte[(size_t)pa::indexVonId("v1.global.output_trim_db")].zahl=6.;apply(*p,z);{juce::MemoryBlock state;p->getStateInformation(state);juce::File("C:/Users/phili/AppData/Local/Temp/nakama-host-audit-20260912-w2/gain.bin").replaceWithData(state.getData(),state.getSize());}silence(*p,20,4);juce::AudioBuffer<float>b(4,512);for(int c=0;c<4;c++)for(int n=0;n<512;n++)b.setSample(c,n,0.1f);block(*p,b);std::cout<<"quad_samples="<<b.getSample(0,511)<<","<<b.getSample(1,511)<<","<<b.getSample(2,511)<<","<<b.getSample(3,511)<<"\n";
}
{
auto p=std::make_unique<P>();p->setRateAndBufferSizeDetails(48000,512);p->prepareToPlay(48000,512);resonator(*p);{juce::MemoryBlock state;p->getStateInformation(state);juce::File("C:/Users/phili/AppData/Local/Temp/nakama-host-audit-20260912-w2/resonator.bin").replaceWithData(state.getData(),state.getSize());}juce::AudioBuffer<float>b(2,512);b.clear();b.setSample(0,511,1.f);b.setSample(1,511,1.f);block(*p,b);p->reset();b.clear();block(*p,b);std::cout<<"after_reset_silent_input_peak="<<peak(b)<<" tail_report="<<p->getTailLengthSeconds()<<"\n";p->releaseResources();b.clear();block(*p,b);std::cout<<"after_release_silent_input_peak="<<peak(b)<<"\n";p->prepareToPlay(48000,512);silence(*p);b.clear();block(*p,b);std::cout<<"after_reprepare_silent_input_peak="<<peak(b)<<"\n";
}
}
