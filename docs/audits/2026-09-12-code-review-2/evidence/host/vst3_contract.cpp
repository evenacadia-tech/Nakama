#define NOMINMAX
#include <windows.h>
#include <pluginterfaces/base/ipluginbase.h>
#include <pluginterfaces/vst/ivstcomponent.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivsthostapplication.h>
#include <pluginterfaces/vst/vstspeaker.h>
#include <iostream>
#include <pluginterfaces/base/ibstream.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <cstring>
using namespace Steinberg;
class Host final:public Vst::IHostApplication{
public:tresult PLUGIN_API queryInterface(const TUID id,void**out) override{if(std::memcmp(id,Vst::IHostApplication_iid,16)==0||std::memcmp(id,FUnknown_iid,16)==0){*out=this;return kResultOk;}*out=nullptr;return kNoInterface;}
uint32 PLUGIN_API addRef()override{return 1;}uint32 PLUGIN_API release()override{return 1;}
tresult PLUGIN_API getName(Vst::String128 name)override{const char16_t text[]=u"Nakama audit host";std::memcpy(name,text,sizeof(text));return kResultOk;}
tresult PLUGIN_API createInstance(TUID,TUID,void**obj)override{*obj=nullptr;return kNoInterface;}};
class Stream final:public IBStream {
public:std::vector<char> data;int64 pos=0;Stream(const char*path){std::ifstream f(path,std::ios::binary);data.assign(std::istreambuf_iterator<char>(f),{});}
tresult PLUGIN_API queryInterface(const TUID id,void**out)override{if(std::memcmp(id,IBStream_iid,16)==0||std::memcmp(id,FUnknown_iid,16)==0){*out=this;return kResultOk;}*out=nullptr;return kNoInterface;}
uint32 PLUGIN_API addRef()override{return 1;}uint32 PLUGIN_API release()override{return 1;}
tresult PLUGIN_API read(void*out,int32 wanted,int32*read)override{int32 n=(int32)std::min<int64>(wanted,(int64)data.size()-pos);if(n>0)std::memcpy(out,data.data()+pos,n);pos+=n;if(read)*read=n;return n>0?kResultOk:kResultFalse;}
tresult PLUGIN_API write(void*,int32,int32*)override{return kNotImplemented;}
tresult PLUGIN_API seek(int64 off,int32 mode,int64*out)override{int64 next=off+(mode==kIBSeekCur?pos:mode==kIBSeekEnd?(int64)data.size():0);if(next<0||next>(int64)data.size())return kResultFalse;pos=next;if(out)*out=pos;return kResultOk;}
tresult PLUGIN_API tell(int64*out)override{if(out)*out=pos;return kResultOk;}};
void runAudio(Vst::IComponent*c,Vst::IAudioProcessor*a){
std::cout<<std::setprecision(12);Stream gain("C:/Users/phili/AppData/Local/Temp/nakama-host-audit-20260912-w2/gain.bin");std::cout<<"gain_setState="<<c->setState(&gain)<<"\n";a->setProcessing(true);
float values[4][512]{};float*ptr[4]={values[0],values[1],values[2],values[3]};Vst::AudioBusBuffers bus{};bus.numChannels=4;bus.channelBuffers32=ptr;Vst::ProcessData d{};d.processMode=Vst::kRealtime;d.symbolicSampleSize=Vst::kSample32;d.numSamples=512;d.numInputs=1;d.numOutputs=1;d.inputs=&bus;d.outputs=&bus;
for(int n=0;n<20;n++){std::memset(values,0,sizeof(values));a->process(d);}for(auto&ch:values)for(auto&v:ch)v=0.1f;auto result=a->process(d);std::cout<<"vst3_process="<<result<<" vst3_quad_samples="<<values[0][511]<<","<<values[1][511]<<","<<values[2][511]<<","<<values[3][511]<<"\n";
a->setProcessing(false);c->setActive(false);Stream res("C:/Users/phili/AppData/Local/Temp/nakama-host-audit-20260912-w2/resonator.bin");std::cout<<"resonator_setState="<<c->setState(&res)<<"\n";c->setActive(true);a->setProcessing(true);for(int n=0;n<20;n++){std::memset(values,0,sizeof(values));a->process(d);}std::memset(values,0,sizeof(values));values[0][511]=values[1][511]=1.f;a->process(d);std::cout<<"vst3_setProcessing_false="<<a->setProcessing(false)<<" vst3_setProcessing_true="<<a->setProcessing(true)<<"\n";std::memset(values,0,sizeof(values));a->process(d);double peak=0;for(auto&ch:values)for(auto v:ch)peak=std::max(peak,std::abs((double)v));std::cout<<"vst3_after_reset_silent_input_peak="<<peak<<" reported_tail_samples="<<a->getTailSamples()<<"\n";a->setProcessing(false);
}
int main(){const char*path="C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3/Contents/x86_64-win/Nakama Probeeq.vst3";auto dll=LoadLibraryA(path);if(!dll){std::cerr<<"load_error="<<GetLastError()<<"\n";return 2;}auto init=(bool(*)())GetProcAddress(dll,"InitDll");auto fini=(bool(*)())GetProcAddress(dll,"ExitDll");if(init&&!init())return 3;auto get=(IPluginFactory*(PLUGIN_API*)())GetProcAddress(dll,"GetPluginFactory");auto*f=get();Host host;bool tested=false;for(int32 n=0;n<f->countClasses();n++){PClassInfo info{};f->getClassInfo(n,&info);if(std::strcmp(info.category,kVstAudioEffectClass)!=0)continue;Vst::IComponent*c=nullptr;auto cr=f->createInstance(info.cid,Vst::IComponent_iid,(void**)&c);std::cout<<"factory_create="<<cr<<"\n";if(!c)continue;std::cout<<"initialize="<<c->initialize(&host)<<"\n";Vst::IAudioProcessor*a=nullptr;c->queryInterface(Vst::IAudioProcessor_iid,(void**)&a);if(a){Vst::SpeakerArrangement ins=Vst::SpeakerArr::k40Music,outs=ins;auto accepted=a->setBusArrangements(&ins,1,&outs,1);Vst::BusInfo bi{};c->getBusInfo(Vst::kAudio,Vst::kInput,0,bi);std::cout<<"vst3_quad_set_result="<<accepted<<" vst3_input_channels="<<bi.channelCount<<"\n";Vst::ProcessSetup s{};s.processMode=Vst::kRealtime;s.symbolicSampleSize=Vst::kSample32;s.maxSamplesPerBlock=512;s.sampleRate=48000.;std::cout<<"activateInput="<<c->activateBus(Vst::kAudio,Vst::kInput,0,true)<<" activateOutput="<<c->activateBus(Vst::kAudio,Vst::kOutput,0,true)<<"\n";std::cout<<"vst3_setup="<<a->setupProcessing(s)<<" vst3_activate="<<c->setActive(true)<<"\n";runAudio(c,a);c->setActive(false);a->release();tested=true;}c->terminate();c->release();}f->release();if(fini)fini();FreeLibrary(dll);return tested?0:4;}

