#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
void require(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
void set(PumpProcessor& p,const char* id,float v){auto* a=p.state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));}
// Renders 'seconds' of audio; main inputs get 'mainIn(n)', extra (sidechain) channels get 'scIn(n)'. Returns channel 0 output.
template<class F,class G>std::vector<float> run(PumpProcessor& p,int channels,double seconds,F mainIn,G scIn){const int block=256;p.prepareToPlay(48000,block);juce::MidiBuffer m;juce::AudioBuffer<float>b(channels,block);std::vector<float> out;
 for(int start=0;start<(int)(48000*seconds);start+=block){for(int n=0;n<block;++n)for(int c=0;c<channels;++c)b.setSample(c,n,c<2?mainIn(start+n):scIn(start+n));p.processBlock(b,m);for(int n=0;n<block;++n){require(std::isfinite(b.getSample(0,n)),"non-finite output");out.push_back(b.getSample(0,n));}}return out;}
int main(){swell::Licence::unlockForTesting();try{juce::ScopedJuceInitialiser_GUI gui;auto dc=[](int){return .5f;};auto none=[](int){return 0.f;};
 for(int i=0;i<presetCount;++i){PumpProcessor p;p.setCurrentProgram(i);auto out=run(p,2,2,dc,none);for(float v:out)require(v>=-1e-6f&&v<=.5f+1e-6f,"preset gain out of range");}
 for(int c=0;c<curveCount;++c){require(curveGain(c,0)<.05f,"curve does not duck on the beat");require(curveGain(c,.999f)>.95f,"curve does not recover");}
 {PumpProcessor p;set(p,"mix",0);auto out=run(p,2,1,dc,none);for(float v:out)require(v==.5f,"mix 0 is not transparent");}
 {PumpProcessor p;set(p,"bypass",1);auto out=run(p,2,1,dc,none);for(float v:out)require(v==.5f,"bypass is not transparent");}
 // 120 BPM manual tempo, 1/4: one dip every 24000 samples at 48 kHz.
 {PumpProcessor p;auto out=run(p,2,1.1,dc,none);require(out[240]<.05f&&out[24240]<.05f,"tempo mode does not duck on each beat");require(out[14400]>.49f&&out[23000]>.49f,"tempo mode does not recover");}
 {PumpProcessor p;set(p,"rate",3);auto out=run(p,2,.5,dc,none);require(out[12240]<.05f,"1/8 rate does not double the pump");}
 // Sidechain fallback on the main input: loud 20 ms bursts every 0.5 s on a quiet floor.
 {PumpProcessor p;set(p,"trigger",1);auto burst=[](int n){return n%24000<960?.9f:.01f;};auto out=run(p,2,1.1,burst,none);
  require(out[24480]/.9f<.15f,"sidechain fallback does not duck after a hit");require(out[24000+14400]/.01f>.95f,"sidechain fallback does not release");require(out[20000]/.01f>.95f,"sidechain ducks without a hit");}
 // Real sidechain bus: the kick only reaches channels 2-3, the main signal stays steady.
 {PumpProcessor p;p.enableAllBuses();require(p.getBus(true,1)->isEnabled(),"sidechain bus not enabled");set(p,"trigger",1);auto kick=[](int n){return n%24000<960?.9f:0.f;};auto out=run(p,4,1.1,dc,kick);
  require(out[24480]<.07f,"sidechain bus does not duck");require(out[24000+14400]>.49f,"sidechain bus does not release");}
 // A decaying 48 Hz kick tail must trigger once per beat, not on every oscillation.
 {PumpProcessor p;p.enableAllBuses();set(p,"trigger",1);auto low=[](int n){int t=n%24000;return (float)(.9*std::exp(-6.5*t/48000.)*std::sin(6.2831853*48*t/48000.));};auto out=run(p,4,1.1,dc,low);
  for(int n=24000+14400;n<48000;++n)require(out[(size_t)n]>.49f,"kick tail retriggers the pump");}
 {PumpProcessor p;p.setCurrentProgram(11);set(p,"mix",42);juce::MemoryBlock state;p.getStateInformation(state);PumpProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());
  require(restored.getCurrentProgram()==11&&(int)restored.value("curve")==7&&std::abs(restored.value("mix")-42)<.2f,"state not restored");}
 std::cout<<"PASS: 16 presets, 8 curves, mix 0 and bypass transparency, tempo cycle and rate, sidechain fallback, bus and low tail, state\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
