#include "DspKern.h"
#include "HoerMarkierung.h"
#include "StampedAudioQueue.h"
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <cmath>
using namespace nakama::dsp;
namespace pa=nakama::parameter;
pa::DspSatz clean() { pa::DspSatz s; s.werte[pa::kIndexEqEnabled].b=true; s.werte[3].zahl=1.; s.werte[pa::kIndexMix].zahl=1.; return s; }
int main() {
 auto k=std::make_unique<DspKern>(); k->bereiteVor(48000,64);
 auto s=clean(); s.werte[2].zahl=6.; juce::String reason,where; std::cout << "publicParameterValidation=" << pa::validiere(s,reason,where) << " reason=" << reason << " id=" << where << std::endl; k->uebernehmeZustand(s);
 std::vector<float> l(64,.1f),r(64,.1f); float* pp[]={l.data(),r.data()};
 for(int i=0;i<8;i++) k->verarbeite(pp,2,64);
 std::fill(l.begin(),l.end(),std::numeric_limits<float>::max()/2);r=l;k->verarbeite(pp,2,64);
 std::cout << "DSP +6dB half-FLT_MAX finite=" << std::isfinite(l[0]) << " value=" << l[0] << " inputNonfinite=" << k->nichtEndlicheEingaenge() << " stateHealed=" << k->geheilteFilterzustaende() << '\n';
 std::fill(l.begin(),l.end(),std::numeric_limits<float>::max()*.75f);r=l;k->verarbeite(pp,2,64);
 std::cout << "DSP +6dB 0.75-FLT_MAX finite=" << std::isfinite(l[0]) << " value=" << l[0] << " inputNonfinite=" << k->nichtEndlicheEingaenge() << " stateHealed=" << k->geheilteFilterzustaende() << " doubleTapFinite=" << std::isfinite(k->tap(Tap::postCommitted,0)[0]) << '\n';
 auto m=std::make_unique<eqcop::HoerMarkierungDsp>();m->vorbereiten(64);m->setzeSamplerate(48000);
 eqcop::MarkierungsWunsch w;w.modus=eqcop::MarkierungsModus::puls;w.fs=48000;w.fVon=800;w.fBis=1200;w.fSchwerpunkt=1000;w.breiteOktaven=.5;w.istResonanz=true;eqcop::MarkierungsAuftrag a;
 std::cout << "markerBuilt=" << eqcop::baueMarkierungsAuftrag(a,w) << '\n';m->reicheEin(a);
 juce::AudioBuffer<float> buffer(pp,2,64);int nonfinite=0;int first=-1;
 for(int b=0;b<200;b++){for(int i=0;i<64;i++){l[i]=std::numeric_limits<float>::max()*.8f*(float)std::sin(2*3.141592653589793*1000*(b*64+i)/48000.);r[i]=l[i];}m->verarbeite(buffer,2,true);for(int i=0;i<64;i++)if(!std::isfinite(l[i])){nonfinite++;if(first<0)first=b*64+i;}}
 std::cout << "markerOutputNonfinite=" << nonfinite << " firstSample=" << first << " counter=" << m->nichtEndlicheWetSamples() << '\n';
}


