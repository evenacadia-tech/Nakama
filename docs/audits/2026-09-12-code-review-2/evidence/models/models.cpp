#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include "C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/FeatureEngine.h"
#include "C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/analysis/LoudnessAccumulator.h"
using namespace nakama::analyse;
namespace rt=nakama::echtzeit;
constexpr double pi=3.14159265358979323846;
struct Feed {
 std::unique_ptr<FeatureEngine> e=std::make_unique<FeatureEngine>();
 double sr; std::uint64_t pos=0; FeatureFrame last{};
 explicit Feed(double fs):sr(fs) {e->vorbereiten(fs);e->evidenzIntervallSetzen(1.0);}
 template<class F> void run(int count,F f,int blockSize=480) {
  while(count>0) {int n=std::min(count,blockSize);std::vector<float>a(n*2);for(int i=0;i<n;++i) f(pos+i,a[2*i],a[2*i+1]);
   rt::StampedBlock b{};b.stromVon=pos;b.sampleCount=n;b.kanaele=2;b.tapMaske=1;b.projectSampleStart=(std::int64_t)pos;b.sampleRate=sr;
   b.flags=rt::kFlagKontextAnwesend|rt::kFlagZeitGueltig|rt::kFlagSpieltGueltig|rt::kFlagSpielt|rt::kFlagSampleRateGueltig;
   if(e->nimmBlock(b,a.data())) last=e->frame();pos+=n;count-=n; }
 }
 void tone(int count,double hz,double amplitude,int blockSize=480) {run(count,[&](auto n,float&l,float&r){l=r=(float)(amplitude*std::sin(2*pi*hz*n/sr));},blockSize);}
};
int bandFor(double hz){for(int i=0;i<Gitter::evidenzBaender;++i)if(hz>=Gitter::evidenzKante(i)&&hz<Gitter::evidenzKante(i+1))return i;return -1;}
void scalars(const char* tag,const Feed& x){auto& f=x.last;std::cout<<tag<<" t="<<x.pos/x.sr<<" lufsM_set="<<f.lufsMGesetzt<<" M="<<f.lufsM<<" lufsS_set="<<f.lufsSGesetzt<<" S="<<f.lufsS<<" TP="<<f.truePeakDb<<" passageTP="<<f.truePeakPassageDb<<" passageTP_set="<<f.truePeakPassageGesetzt<<" nonfinite_frame="<<f.nichtEndlichRahmen<<" nonfinite_evidence="<<f.nichtEndlichEvidenz<<"\n";}
int main(){std::cout<<std::setprecision(12);
 for(double sr:{8000.,44100.,48000.,96000.,192000.}){
  Feed x(sr);int count=(int)(3*sr);double a=std::sqrt(2.)*std::pow(10.,-55./20.);double e=0.;x.run(count,[&](auto n,float&l,float&r){l=r=(float)(a*std::sin(2*pi*1000.*n/sr));e+=(double)l*l;},(int)(sr/100));
  int bits=0;for(int b=0;b<Gitter::evidenzBaender;++b)if(x.e->stereoBand(b).basisGesetzt)++bits;
  std::cout<<"ACTIVITY sr="<<sr<<" input_rms_db="<<10*std::log10(e/count)<<" activity="<<x.last.aktivitaet<<" coverage="<<x.last.abdeckung<<" evidence="<<x.last.evidenzFrisch<<" stereo_bands="<<bits<<"\n";
 }
 {Feed x(48000);x.tone(96000,1171.875,.5);auto w=x.e->stereoBand(bandFor(1171.875));std::cout<<"STEREO exact_bin_tone=.5 freq=1171.875 band="<<bandFor(1171.875)<<" mid_db="<<w.midDb<<" side_db="<<w.sideDb<<" expected_energy_normalized_mid_db="<<10*std::log10(.25)<<" frames="<<w.freiheitsgrade<<"\n";}
 {Feed x(48000);x.e->setzePassagenfenster(48000,96000,0);x.tone(48000,12000,1.0);x.tone(48000,12000,.001);scalars("FUTURE_PASSAGE",x);Feed ref(48000);ref.e->setzePassagenfenster(0,48000,0);ref.tone(48000,12000,.001);scalars("ISOLATED_REFERENCE",ref);}
 {Feed x(48000);x.tone(144000,1000,.2);scalars("NAN_BEFORE",x);x.run(9600,[](auto,float&l,float&r){l=r=std::numeric_limits<float>::quiet_NaN();});scalars("NAN_DAMAGE",x);x.tone(4800,1000,.2);scalars("NAN_ONE_CLEAN_FRAME",x);x.tone(4800,1000,.2);scalars("NAN_TWO_CLEAN_FRAMES",x);}
 {Feed x(48000);x.tone(48000,20,1.0);x.e->setzePassagenfenster(48000,96000,0);x.tone(19200,1000,.001);scalars("PASSAGE_K_STATE",x);Feed r(48000);r.e->setzePassagenfenster(0,48000,0);r.tone(19200,1000,.001);scalars("PASSAGE_K_REFERENCE",r);}
return 0;
}
