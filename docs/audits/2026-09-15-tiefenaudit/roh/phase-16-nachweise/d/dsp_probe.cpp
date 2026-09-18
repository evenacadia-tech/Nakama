// Audit-only caller; links unchanged product .cpp files. No copied DSP formulas.
#include "DspKern.h"
#include "NakamaParameter.h"
#include "NakamaTransaktion.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
using namespace nakama::dsp;
namespace p = nakama::parameter;
using Signal = std::function<double(long long,int)>;
struct Capture { std::vector<float> l,r; std::vector<double> tap; };
double db(double x) { return 20*std::log10(std::max(std::abs(x),1e-300)); }
double rms(const std::vector<float>& a, int first=0) { double e=0; for(size_t i=first;i<a.size();++i)e+=(double)a[i]*a[i]; return std::sqrt(e/(a.size()-first)); }
void set(p::DspSatz& s,int slot,int f,double v) { s.werte[p::indexBandV1(slot,f)].zahl=v; }
void band(p::DspSatz& s,int slot,Filtertyp t,double f,double q,double gain,Kanalmodus m=Kanalmodus::stereo) {
 s.werte[p::indexOccupied(slot)].b=true; s.werte[p::indexBandV1(slot,p::kEnabled)].b=true;
 s.werte[p::indexBandV1(slot,p::kType)].enumIndex=(int)t; s.werte[p::indexBandV1(slot,p::kChannelMode)].enumIndex=(int)m;
 set(s,slot,p::kFreqHz,f);set(s,slot,p::kQ,q);set(s,slot,p::kGainDb,gain);
}
void dynamic(p::DspSatz& s,int slot,double range,double threshold,double attack=10,double hold=0,double release=100) {
 s.werte[p::indexBandV1(slot,p::kDynamicEnabled)].b=true;set(s,slot,p::kDynamicRangeDb,range);set(s,slot,p::kThresholdDb,threshold);
 set(s,slot,p::kAttackMs,attack);set(s,slot,p::kHoldMs,hold);set(s,slot,p::kReleaseMs,release);
 s.werte[p::indexBandV1(slot,p::kSidechainSource)].enumIndex=(int)Sidechain::internal;
}
p::DspSatz state() { p::DspSatz s;s.werte[p::kIndexEqEnabled].b=true;return s; }
std::unique_ptr<DspKern> core(double fs=48000,int max=512) {auto k=std::make_unique<DspKern>();k->bereiteVor(fs,max);return k;}
void publish(DspKern& k,const p::DspSatz& s) {
 juce::String reason,where; if(!p::validiere(s,reason,where)) throw std::runtime_error("Invalid fixture: "+reason.toStdString()+":"+where.toStdString());
 k.pflege();if(!k.uebernehmeZustand(s))throw std::runtime_error("busy publication");
}
Capture run(DspKern& k, long long& pos,int count,const std::vector<int>& chunks,const Signal& signal,int channels=2) {
 Capture c;c.l.reserve(count);c.r.reserve(count);c.tap.reserve(count);int done=0,ci=0;
 while(done<count) {int n=std::min(count-done,chunks[ci++%chunks.size()]);std::vector<float> l(n),r(n);
  for(int i=0;i<n;++i){l[i]=(float)signal(pos+i,0);r[i]=(float)signal(pos+i,1);}float* a[]={l.data(),r.data()};
  k.verarbeite(a,channels,n);k.pflege();c.l.insert(c.l.end(),l.begin(),l.end());c.r.insert(c.r.end(),r.begin(),r.end());
  if(k.tapLaenge()==n){auto t=k.tap(Tap::postCommitted,0);c.tap.insert(c.tap.end(),t,t+n);}pos+=n;done+=n;
 }return c;
}
Signal tone(double fs,double f,double amp=0.25,bool quadrature=false) {return [=](long long n,int ch){return amp*std::sin(2*kPi*f*n/fs+(quadrature&&ch? kPi/2:0));};}
Signal dc(double a) {return [=](long long,int){return a;};}
void emit(const std::string& key,double value) {std::cout<<key<<"="<<std::setprecision(15)<<value<<" ";}
void require(bool ok,const char* name){std::cout<<"ASSERT "<<name<<"="<<(ok?"PASS":"FAIL")<<"\n";if(!ok)throw std::runtime_error(name);}
void trimCase(double oldTrim,double newTrim,const std::vector<int>& chunks,bool reset=false) {
 auto k=core();auto s=state();long long pos=0;s.werte[2].zahl=oldTrim;publish(*k,s);run(*k,pos,1024,chunks,dc(.5));
 s.werte[p::kIndexEqEnabled].b=false;publish(*k,s);run(*k,pos,512,chunks,dc(.5));
 s.werte[2].zahl=newTrim;publish(*k,s);run(*k,pos,512,chunks,dc(.5));if(reset)k->beendeAudiohistorie();
 s.werte[p::kIndexEqEnabled].b=true;publish(*k,s);auto c=run(*k,pos,512,chunks,dc(.5));
 auto mx=std::max_element(c.l.begin(),c.l.end());double jump=0;for(size_t i=1;i<c.l.size();++i)jump=std::max(jump,(double)std::abs(c.l[i]-c.l[i-1]));
 std::cout<<"TRIM ";emit("old",oldTrim);emit("new",newTrim);emit("firstChunk",chunks[0]);emit("reset",reset);emit("maxRatio",*mx/.5);emit("maxDb",db(*mx/.5));emit("maxAt",mx-c.l.begin());emit("jump",jump);emit("finalRatio",c.l.back()/.5);std::cout<<"\n";
 if(oldTrim==24&&newTrim==0){std::cout<<"REGRESSION_ORACLE neutral_reenable_peak_le_1="<<(*mx/.5<=1+1e-6?"PASS":"EXPECTED_FAIL")<<"\n";require(*mx/.5>4.68&&c.l.back()==.5f,"T3_15_05_reproduction_and_unity_end");}
 if(oldTrim==0&&newTrim==0)require(*mx==.5f&&jump==0,"T3_15_05_no_old_gain_control");
}
void monoCases(){for(int mode=0;mode<5;++mode){auto s=state();band(s,0,Filtertyp::lowShelf,20000,.7071067811865476,12,(Kanalmodus)mode);
 for(int channels:{1,2}){auto off=core();auto on=core();long long po=0,pn=0;publish(*off,s);s.werte[p::kIndexAutoGain].b=true;publish(*on,s);s.werte[p::kIndexAutoGain].b=false;
 auto co=run(*off,po,48000,{64},tone(48000,250),channels);auto cn=run(*on,pn,48000,{64},tone(48000,250),channels);
 std::cout<<"MONO ";emit("mode",mode);emit("channels",channels);emit("derivedDb",on->autoGainDb());emit("withoutDb",db(rms(co.l,24000)/(.25/std::sqrt(2.))));emit("withDb",db(rms(cn.l,24000)/(.25/std::sqrt(2.))));std::cout<<"\n";
 }} }
// Independent measurement: impulse through real core, then complex DFT. No RBJ/SVF response equation.
std::vector<double> impulse(const p::DspSatz& s,double fs,int n=32768){auto k=core(fs);long long pos=0;publish(*k,s);run(*k,pos,2048,{512},dc(0));long long start=pos;return run(*k,pos,n,{512},[=](long long t,int){return t==start?1.:0.;}).tap;}
double powerAt(const std::vector<double>& ir,double fs,double f){std::complex<double> a=0,rot=std::polar(1.,-2*kPi*f/fs),z=1.;for(double x:ir){a+=x*z;z*=rot;}return std::norm(a);}
void rateCases(){for(double fs:{32000.,22050.,44100.,48000.})for(double q:{1.,4.}){auto s=state();band(s,0,Filtertyp::bell,14000,q,12);auto ir=impulse(s,fs);auto k=core(fs);publish(*k,s);double full=0,capped=0;int nn=0;for(int i=0;i<kAutoGainStellen;++i){double f=autoGainGitterHz(i),e=powerAt(ir,fs,f);full+=e;if(f<fs/2){capped+=e;++nn;}}std::cout<<"RATE ";emit("fs",fs);emit("q",q);emit("derivedDb",k->autoGainDb());emit("realIrFullGridDb",-10*std::log10(full/kAutoGainStellen));emit("realIrBelowNyquistDb",-10*std::log10(capped/nn));emit("pointsAboveNyquist",kAutoGainStellen-nn);std::cout<<"\n";require(std::abs(k->autoGainDb()+10*std::log10(full/kAutoGainStellen))<1e-6,"autogain_agrees_with_actual_impulse_full_grid");}}
void shelfCases(){for(auto type:{Filtertyp::lowShelf,Filtertyp::highShelf})for(double q:{.7071067811865476,8.,24.}){auto s=state();band(s,0,type,1000,q,12);auto ir=impulse(s,48000);double max=-1e9,at=0;for(int i=0;i<1400;++i){double f=20*std::pow(1000.,i/1399.);double d=10*std::log10(powerAt(ir,48000,f));if(d>max){max=d;at=f;}}auto k=core();publish(*k,s);long long p0=0;auto c=run(*k,p0,96000,{333,7,64,1},tone(48000,at,.01));std::cout<<"SHELF ";emit("type",(int)type);emit("q",q);emit("irPeakDb",max);emit("peakHz",at);emit("actualAudioDb",db(rms(c.l,48000)/(.01/std::sqrt(2.))));std::cout<<"\n";}}
Capture rampSequence(double fs,int phase,const std::vector<int>& chunks){auto k=core(fs,512);auto s=state();band(s,0,Filtertyp::bell,900,2,3);dynamic(s,0,-12,-35,.1,0,50);band(s,2,Filtertyp::highShelf,4000,1.,-3);dynamic(s,2,6,-30,2,10,100);long long pos=0;publish(*k,s);auto sig=tone(fs,1000,.25,true);run(*k,pos,8192+phase,chunks,sig);Capture all;
 for(int event=0;event<5;++event){if(event==0){set(s,0,p::kFreqHz,1400);set(s,0,p::kQ,8);set(s,0,p::kDynamicRangeDb,0);s.werte[1].zahl=3;s.werte[2].zahl=-3;}
 if(event==1){set(s,0,p::kDynamicRangeDb,12);set(s,2,p::kFreqHz,12000);set(s,2,p::kQ,4);s.werte[p::kIndexMix].zahl=.4;}
 if(event==2){set(s,0,p::kDynamicRangeDb,-12);set(s,0,p::kAttackMs,100);s.werte[3].zahl=1.3;}
 if(event==3){set(s,2,p::kDynamicRangeDb,0);set(s,0,p::kFreqHz,20);set(s,0,p::kQ,.15);}
 if(event==4){set(s,0,p::kFreqHz,std::min(20000.,fs*.45));set(s,0,p::kQ,24);set(s,2,p::kDynamicRangeDb,-12);s.werte[p::kIndexMix].zahl=1.;}
 publish(*k,s);auto c=run(*k,pos,777,chunks,sig);for(float x:c.l)if(!std::isfinite(x))throw std::runtime_error("nonfinite ramp output left");for(float x:c.r)if(!std::isfinite(x))throw std::runtime_error("nonfinite ramp output right");for(double x:c.tap)if(!std::isfinite(x))throw std::runtime_error("nonfinite ramp tap");all.l.insert(all.l.end(),c.l.begin(),c.l.end());all.r.insert(all.r.end(),c.r.begin(),c.r.end());all.tap.insert(all.tap.end(),c.tap.begin(),c.tap.end());}return all;}
void blockCases(){for(double fs:{44100.,48000.,96000.,192000.})for(int phase:{0,1,7}){auto a=rampSequence(fs,phase,{1});for(auto pattern:std::vector<std::vector<int>>{{7},{64},{333},{1,7,64,333}}){auto b=rampSequence(fs,phase,pattern);bool same=a.l.size()==b.l.size()&&std::memcmp(a.l.data(),b.l.data(),a.l.size()*sizeof(float))==0&&std::memcmp(a.r.data(),b.r.data(),a.r.size()*sizeof(float))==0;bool tap=a.tap.size()==b.tap.size()&&std::memcmp(a.tap.data(),b.tap.data(),a.tap.size()*sizeof(double))==0;double d=0;for(size_t i=0;i<a.l.size();++i)d=std::max(d,(double)std::abs(a.l[i]-b.l[i]));std::cout<<"BLOCK ";emit("fs",fs);emit("phase",phase);emit("pattern",pattern.size()==1?pattern[0]:1764333);emit("floatBitSame",same);emit("tapBitSame",tap);emit("maxDiff",d);std::cout<<"\n";if(!same||!tap)throw std::runtime_error("block split dependence");}}}
void historyCases(){auto s=state();band(s,0,Filtertyp::bell,1000,2,0);dynamic(s,0,-12,-40,.1,500,5000);auto k=core();long long pos=0;publish(*k,s);run(*k,pos,48000,{64},tone(48000,1000,.25,true));double w[8];k->auslenkungenDb(w);double before=w[0];auto removed=state();publish(*k,removed);publish(*k,s);auto after=run(*k,pos,512,{1},dc(0));k->auslenkungenDb(w);double pk=0;for(float x:after.l)pk=std::max(pk,std::abs((double)x));std::cout<<"HISTORY ";emit("before",before);emit("reoccupiedWithoutAudio",w[0]);emit("peakAfterReoccupy",pk);std::cout<<"\n";
 require(before==-12&&w[0]==-12&&pk>.01,"T3_14_02_foreign_history_reproduced");
 auto fresh=core();long long pf=pos;publish(*fresh,s);auto fc=run(*fresh,pf,512,{1},dc(0));fresh->auslenkungenDb(w);require(w[0]==0&&rms(fc.l)==0,"T3_14_02_fresh_silence_control");
 // Preserve legitimate history on continuous update; discard on unrelated topology (current product behavior).
 auto u=core();long long pu=0;publish(*u,s);run(*u,pu,48000,{64},tone(48000,1000,.25,true));auto changed=s;changed.werte[2].zahl=-3;publish(*u,changed);run(*u,pu,512,{64},dc(0));u->auslenkungenDb(w);std::cout<<"HISTORY ";emit("legitimateGlobalOnly",w[0]);std::cout<<"\n";
 auto v=core();long long pv=0;publish(*v,s);run(*v,pv,48000,{64},tone(48000,1000,.25,true));band(changed,5,Filtertyp::bell,9000,1,0);publish(*v,changed);run(*v,pv,512,{64},dc(0));v->auslenkungenDb(w);std::cout<<"HISTORY ";emit("unrelatedSlotAndGlobal",w[0]);std::cout<<"\n";
}
void telemetryCases(){for(int phase:{0,1,7}){auto k=core();auto s=state();band(s,0,Filtertyp::bell,1000,2,0);dynamic(s,0,-12,0,.1,0,50);long long pos=0;auto sig=tone(48000,1000,.25,true);publish(*k,s);run(*k,pos,8192+phase,{64},sig);set(s,0,p::kThresholdDb,-60);publish(*k,s);bool seen=false;
 for(int i=0;i<256;++i){long long at=pos;auto c=run(*k,pos,1,{1},sig);double w[8];k->auslenkungenDb(w);if(w[0]!=0){double delta=c.tap[0]-(double)(float)sig(at,0);std::cout<<"TELEMETRY_TARGET ";emit("phase",phase);emit("firstNonzeroAt",i);emit("reportedDb",w[0]);emit("tapMinusInput",delta);std::cout<<"\n";require(std::abs(delta)<1e-14,"first_reported_dynamic_target_precedes_applied_audio");seen=true;break;}}
 require(seen,"dynamic_target_was_observed");}}
void transactionHistoryCase(){namespace tx=nakama::transaktion;auto k=core();tx::DspKernAusfuehrung execution(*k);tx::Transaktionskern transaction(execution);transaction.setzeSamplerate(48000);auto s=state();band(s,0,Filtertyp::bell,1000,2,0);dynamic(s,0,-12,-40,.1,500,5000);juce::String reason;require(transaction.ladestart(s,0,{},0,reason),"transaction_initial_load_valid");require(execution.publiziereWirksam(transaction.wirksam(),true),"transaction_initial_publish");long long pos=0;run(*k,pos,48000,{64},tone(48000,1000,.25,true));tx::Auftrag rem;rem.tid={16,1};rem.baseRevision=transaction.revision();rem.art=tx::Art::remove;rem.slot=0;auto r1=transaction.fuehreAus(rem);tx::Auftrag add;add.tid={16,2};add.baseRevision=transaction.revision();add.art=tx::Art::bandBelegen;for(int f=0;f<p::kJeSlot;++f)add.band[f]=s.werte[p::indexBandV1(0,f)];auto r2=transaction.fuehreAus(add);require(r1.ausgang==tx::Ausgang::commit&&r2.ausgang==tx::Ausgang::commit&&r2.slot==0,"remove_and_reoccupy_really_committed");auto c=run(*k,pos,512,{1},dc(0));double w[8];k->auslenkungenDb(w);double pk=0;for(float x:c.l)pk=std::max(pk,std::abs((double)x));std::cout<<"TRANSACTION_HISTORY ";emit("removeRevision",r1.revision);emit("addRevision",r2.revision);emit("slot",r2.slot);emit("dynamicDb",w[0]);emit("audioPeak",pk);std::cout<<"\n";require(w[0]==-12&&pk>.01,"T3_14_02_real_transaction_path_reproduced");}
void sameInstanceReplayCase(){namespace tx=nakama::transaktion;auto a=core(),b=core();tx::DspKernAusfuehrung execution(*b);tx::Transaktionskern transaction(execution);auto s=state();band(s,0,Filtertyp::bell,1000,2,0);dynamic(s,0,-12,-15,500,0,100);juce::String reason;require(transaction.ladestart(s,0,{},0,reason),"warm_reload_initial_ladestart");publish(*a,s);require(execution.publiziereWirksam(transaction.wirksam(),true),"warm_reload_initial_publish");long long pa=0,pb=0;auto sig=tone(48000,1000,.5,true);run(*a,pa,240000,{64},sig);run(*b,pb,240000,{64},sig);require(transaction.ladestart(s,0,{},0,reason),"warm_reload_same_ladestart");require(execution.publiziereWirksam(transaction.wirksam(),true),"warm_reload_forced_republish");auto ca=run(*a,pa,96000,{64},sig),cb=run(*b,pb,96000,{64},sig);bool same=ca.l.size()==cb.l.size()&&ca.tap.size()==cb.tap.size()&&std::memcmp(ca.l.data(),cb.l.data(),ca.l.size()*sizeof(float))==0&&std::memcmp(ca.r.data(),cb.r.data(),ca.r.size()*sizeof(float))==0&&std::memcmp(ca.tap.data(),cb.tap.data(),ca.tap.size()*sizeof(double))==0;std::cout<<"SAME_INSTANCE_LADESTART ";emit("audioAndTapBitSame",same);std::cout<<"\n";require(same,"same_topology_ladestart_keeps_legitimate_dynamic_history");}
void topologyAudioCase(){auto s=state();band(s,0,Filtertyp::bell,50,8,12);band(s,5,Filtertyp::bell,9000,1,0);s.werte[p::indexBandV1(5,p::kEnabled)].b=false;auto uninterrupted=core();auto changed=core();long long a=0,b=0;auto sig=tone(48000,50,.1);publish(*uninterrupted,s);publish(*changed,s);run(*uninterrupted,a,240000,{64},sig);run(*changed,b,240000,{64},sig);s.werte[p::indexBandV1(5,p::kEnabled)].b=true;publish(*changed,s);auto ref=run(*uninterrupted,a,96000,{64},sig);auto got=run(*changed,b,96000,{64},sig);double minDb=1e9;int at=0,lastOutside=0;
 for(int n=0;n<96000;n+=960){std::vector<float> x(got.l.begin()+n,got.l.begin()+n+960),y(ref.l.begin()+n,ref.l.begin()+n+960);double d=db(rms(x)/rms(y));if(d<minDb){minDb=d;at=n;}if(std::abs(d)>1)lastOutside=n+960;}
 std::cout<<"TOPOLOGY_AUDIO ";emit("minPeriodRmsDb",minDb);emit("periodStartsMs",at/48.);emit("within1DbAfterMs",lastOutside/48.);std::cout<<"\n";require(minDb<-5,"T3_15_08_unrelated_topology_loses_warm_history");}
void reloadCases(){for(double threshold:{-15.,-40.})for(double attack:{.1,10.,500.}){auto s=state();band(s,0,Filtertyp::bell,1000,2,0);dynamic(s,0,-12,threshold,attack,0,100);auto warm=core();long long pw=0;publish(*warm,s);auto sig=tone(48000,1000,.5,true);run(*warm,pw,240000,{64},sig);auto cold=core();long long pc=pw;publish(*cold,s);double w[8];warm->auslenkungenDb(w);double settled=w[0];auto wc=run(*warm,pw,96000,{64},sig);auto cc=run(*cold,pc,96000,{64},sig);double max=0;for(size_t i=0;i<wc.l.size();++i)max=std::max(max,(double)std::abs(wc.l[i]-cc.l[i]));std::cout<<"RELOAD_COMPONENT ";emit("threshold",threshold);emit("attack",attack);emit("settledDynamicDb",settled);emit("maxAudioDifference",max);emit("first10msRatioDb",db(rms(std::vector<float>(cc.l.begin(),cc.l.begin()+480))/rms(std::vector<float>(wc.l.begin(),wc.l.begin()+480))));emit("last1sRatioDb",db(rms(cc.l,48000)/rms(wc.l,48000)));std::cout<<"\n";if(threshold==-40)require(settled==-12,"reload_fixture_really_in_dynamic_plateau");}}
int main(){try{std::cout<<"PRODUCT_HEAD=aff2d8188f33a9525aec1869449773bfdbe305a6\n";for(auto c:std::vector<std::vector<int>>{{1},{64},{333},{1,7,64,333}})trimCase(24,0,c);trimCase(24,0,{64},true);trimCase(0,0,{64});trimCase(-24,0,{64});monoCases();rateCases();shelfCases();blockCases();historyCases();transactionHistoryCase();sameInstanceReplayCase();telemetryCases();topologyAudioCase();reloadCases();std::cout<<"HARNESS_FINISHED\n";return 0;}catch(const std::exception& e){std::cerr<<"HARNESS_ERROR="<<e.what()<<"\n";return 2;}}
