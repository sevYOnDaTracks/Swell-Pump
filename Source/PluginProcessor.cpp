#include "ResizableEditor.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
PumpProcessor::PumpProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true).withInput("Sidechain",juce::AudioChannelSet::stereo(),false)),state(*this,nullptr,"SwellPump",layout()){}
juce::AudioProcessorValueTreeState::ParameterLayout PumpProcessor::layout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;juce::StringArray curveNames,rates;for(auto& c:curves)curveNames.add(c.name);for(auto* r:rateNames)rates.add(r);
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix",1},"Mix",juce::NormalisableRange<float>(0,100,.1f),100.f));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"curve",1},"Curve",curveNames,0));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"rate",1},"Rate",rates,2));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"trigger",1},"Trigger",juce::StringArray{"Tempo","Sidechain"},0));
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"threshold",1},"Sidechain threshold",juce::NormalisableRange<float>(-60,0,.1f),-24.f));
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tempo",1},"Manual tempo",juce::NormalisableRange<float>(60,200,.1f),120.f));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));return l;
}
bool PumpProcessor::isBusesLayoutSupported(const BusesLayout& b)const{
 auto out=b.getMainOutputChannelSet();if(b.getMainInputChannelSet()!=out||(out!=juce::AudioChannelSet::mono()&&out!=juce::AudioChannelSet::stereo()))return false;
 if(b.inputBuses.size()<2)return true;auto sc=b.getChannelSet(true,1);return sc.isDisabled()||sc==juce::AudioChannelSet::mono()||sc==juce::AudioChannelSet::stereo();
}
void PumpProcessor::prepareToPlay(double sr,int){sampleRate=sr;phase=1;gain=1;env=0;sinceTrigger=1e9f;armed=true;mix.reset(sr,.03);mix.setCurrentAndTargetValue(value("mix")/100);wet.reset(sr,.02);wet.setCurrentAndTargetValue(value("bypass")>.5f?0.f:1.f);setLatencySamples(0);}
void PumpProcessor::setValue(const char* id,float v){auto* p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}
void PumpProcessor::setCurrentProgram(int i){i=juce::jlimit(0,presetCount-1,i);program=i;auto& p=presets[(size_t)i];setValue("curve",(float)p.curve);setValue("rate",(float)p.rate);setValue("trigger",(float)p.trigger);setValue("mix",p.mix);}
void PumpProcessor::getStateInformation(juce::MemoryBlock& b){auto t=state.copyState();t.setProperty("preset",program.load(),nullptr);auto xml=t.createXml();copyXmlToBinary(*xml,b);}
void PumpProcessor::setStateInformation(const void* data,int size){auto xml=getXmlFromBinary(data,size);if(xml&&xml->hasTagName(state.state.getType())){auto t=juce::ValueTree::fromXml(*xml);program=juce::jlimit(0,presetCount-1,(int)t.getProperty("preset",0));state.replaceState(t);}}
void PumpProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){render(b,false);}
void PumpProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&){render(b,true);}
void PumpProcessor::render(juce::AudioBuffer<float>& b,bool forced){
 juce::ScopedNoDenormals guard;const int samples=b.getNumSamples();if(samples==0)return;
 auto io=getBusBuffer(b,true,0);auto* scBus=getBusCount(true)>1?getBus(true,1):nullptr;bool external=scBus&&scBus->isEnabled()&&scBus->getNumberOfChannels()>0;
 auto sc=external?getBusBuffer(b,true,1):io;sidechainActive=external;
 mix.setTargetValue(value("mix")/100);wet.setTargetValue(forced||value("bypass")>.5f?0.f:1.f);
 const int curve=(int)value("curve"),rate=juce::jlimit(0,4,(int)value("rate"));const bool sidechain=value("trigger")>.5f;const float threshold=value("threshold");
 double bpm=value("tempo");bool sync=false,locked=false;double ppq=0;
 if(auto* ph=getPlayHead())if(auto info=ph->getPosition()){if(auto hostBpm=info->getBpm())if(std::isfinite(*hostBpm)&&*hostBpm>0){bpm=juce::jlimit(20.,400.,*hostBpm);sync=true;}
  if(info->getIsPlaying())if(auto pos=info->getPpqPosition()){ppq=*pos;locked=true;}}
 tempoDisplay=(float)bpm;hostTempo=sync;const double beats=rateBeats[(size_t)rate];
 // The detector holds 50 ms and re-arms 6 dB under the threshold so a low kick tail cannot retrigger.
 // While the DAW plays, the cycle is locked to its bar grid; otherwise it runs freely.
 if(!sidechain&&locked){double p=std::fmod(ppq/beats,1.);phase=(float)(p<0?p+1:p);}
 const float increment=(float)(bpm/60./sampleRate/beats),smoothing=(float)std::exp(-1/(sampleRate*.0008)),envRelease=(float)std::exp(-1/(sampleRate*.05)),retrigger=(float)(sampleRate*.03);
 float lowest=1;
 for(int n=0;n<samples;++n){
  if(sidechain){float det=0;for(int c=0;c<sc.getNumChannels();++c)det=std::max(det,std::abs(sc.getSample(c,n)));env=det>env?det:env*envRelease;float db=juce::Decibels::gainToDecibels(env,-100.f);
   if(armed&&db>threshold&&sinceTrigger>retrigger){phase=0;armed=false;sinceTrigger=0;}else if(db<threshold-6)armed=true;sinceTrigger+=1;phase=std::min(1.f,phase+increment);}
  else{phase+=increment;if(phase>=1)phase-=1;}
  float target=curveGain(curve,phase);gain=target+(gain-target)*smoothing;
  float g=1-mix.getNextValue()*wet.getNextValue()*(1-gain);lowest=std::min(lowest,g);
  for(int c=0;c<io.getNumChannels();++c)io.setSample(c,n,io.getSample(c,n)*g);}
 phaseDisplay=phase;float prior=gainDisplay.load();while(lowest<prior&&!gainDisplay.compare_exchange_weak(prior,lowest)){}
}
juce::AudioProcessorEditor* PumpProcessor::createEditor(){return new ResizableEditor(*this, new PumpEditor(*this));}juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new PumpProcessor();}
