#define main first_model_main
#include "models.cpp"
#undef main
int main(){std::cout<<std::setprecision(12);
 {Feed x(48000);x.e->setzePassagenfenster(0,14*48000,0);x.tone(7*48000,1000,.5);x.tone(7*48000,1000,.0005);auto&f=x.last;std::cout<<"HEADROOM t="<<x.pos/x.sr<<" window_set="<<x.e->passagenfensterIntakt()<<" kept_frames="<<f.headroomFenster<<" p10="<<f.headroomP10Db<<" p50="<<f.headroomP50Db<<" p95="<<f.headroomP95Db<<" expected_p95="<<20*std::log10(.5)<<" passage_max="<<f.truePeakPassageDb<<"\n";}
 {Feed x(48000);x.tone(144000,1000,.2);x.run(9600,[](auto,float&l,float&r){l=r=std::numeric_limits<float>::quiet_NaN();});x.tone(43200,1000,.2);scalars("NAN_4p1_SECOND",x);}
 {Feed x(48000);x.run(3*48000,[](auto n,float&l,float&r){l=r=(float)(.25*std::sin(2*pi*100*(n%2048)/4096.));});auto f=x.e->fingerprint();int onset_nonzero=0;for(auto v:f.onset)if(v)++onset_nonzero;std::cout<<"FP_STEADY_SELF set="<<f.gesetzt<<" windows="<<f.fenster<<" onset_nonzero="<<onset_nonzero<<" similarity="<<fingerprintAehnlichkeit(f,f)<<"\n";}
 {Feed x(48000),y(48000);auto signal=[](int bassBin){return [bassBin](auto n,float&l,float&r){double t=(n%2048)/4096.;l=r=(float)(.1*std::sin(2*pi*100*t)+.5*std::sin(2*pi*bassBin*t));};};x.run(3*48000,signal(8));y.run(3*48000,signal(12));auto a=x.e->fingerprint(),b=y.e->fingerprint();int bd=0,cd=0;for(int i=0;i<Fingerprint::kBaender;++i)if(a.bandEnergie[i]!=b.bandEnergie[i])++bd;for(int i=0;i<Fingerprint::kChroma;++i)if(a.chroma[i]!=b.chroma[i])++cd;std::cout<<"FP_BASS_CHANGED a_set="<<a.gesetzt<<" b_set="<<b.gesetzt<<" band_differences="<<bd<<" chroma_differences="<<cd<<" similarity="<<fingerprintAehnlichkeit(a,b)<<"\n";}
 return 0;
}
