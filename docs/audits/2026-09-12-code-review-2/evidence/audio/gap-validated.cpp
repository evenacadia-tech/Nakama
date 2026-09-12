#include "DspKern.h"
#include "StampedAudioQueue.h"
#include <iostream>
#include <memory>
#include <vector>
using namespace nakama::dsp; namespace pa=nakama::parameter; namespace rt=nakama::echtzeit;

int main() {
 auto k=std::make_unique<DspKern>();k->bereiteVor(48000,64);pa::DspSatz s;s.werte[pa::kIndexEqEnabled].b=true;s.werte[3].zahl=1.;s.werte[pa::kIndexMix].zahl=1.;s.werte[2].zahl=6.;juce::String reason,where;std::cout<<"publicParameterValidation="<<pa::validiere(s,reason,where)<<std::endl;k->uebernehmeZustand(s);
 std::vector<float> l(128,.1f),r(128,.1f),al(64),ar(64);float* pp[]={l.data(),r.data()};for(int i=0;i<8;i++) k->verarbeite(pp,2,64);
 auto queue=std::make_unique<rt::StampedAudioQueue<rt::GenStrom>>(); queue->vorbereiten();
 rt::Stempel st; st.sampleRateGueltig=true;st.sampleRate=48000;
 // Exact active-DSP tap gate from SondeProcessor.cpp 337-370: inactive fallback unreachable here.
 for(int n:{64,128,64}) {std::fill(l.begin(),l.end(),.1f);std::fill(r.begin(),r.end(),.1f);k->verarbeite(pp,2,n);auto a=k->tap(Tap::postCommitted,0);auto b=k->tap(Tap::postCommitted,1);if(a&&b&&k->tapLaenge()==n&&(size_t)n<=al.size()){for(int i=0;i<n;i++){al[i]=(float)a[i];ar[i]=(float)b[i];}rt::StampedAudioQueue<rt::GenStrom>::TapQuelle tap{al.data(),ar.data()};queue->veroeffentliche(&tap,1,2,n,st);}std::cout<<"hostFrames="<<n<<" tapFrames="<<k->tapLaenge()<<" dspDrops="<<k->verworfeneAnalyseframes()<<" queueDrops="<<queue->dropsGesamt()<<'\n';}
 auto before=*queue->spitze();queue->freigeben();auto after=*queue->spitze();
 std::cout<<"beforeLocal="<<before.stromVon<<" beforeFrames="<<before.sampleCount<<" afterLocal="<<after.stromVon<<" expectedAfterLocal=192 sameSegment="<<(before.segment==after.segment)<<" gapFlag="<<bool(after.flags&rt::kFlagLueckeDavor)<<" quarantineAccepts="<<rt::Blockquarantaene<rt::GenStrom>::schliesstAn(before,after)<<" queueDroppedFrames="<<queue->verloreneFrames()<<'\n';
 return (after.stromVon==192 && !rt::Blockquarantaene<rt::GenStrom>::schliesstAn(before,after))?0:1;
}

