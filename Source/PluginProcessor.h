#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Presets.h"
class PumpProcessor : public juce::AudioProcessor {
public:
 PumpProcessor();
 juce::AudioProcessorValueTreeState state;
 std::atomic<float> phaseDisplay{1},gainDisplay{1},tempoDisplay{120};
 std::atomic<bool> hostTempo{false},sidechainActive{false};
 const juce::String getName() const override{return "Swell Pump";}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;}
 double getTailLengthSeconds() const override{return 0;}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void prepareToPlay(double,int) override;void releaseResources() override{}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorParameter* getBypassParameter() const override{return state.getParameter("bypass");}
 bool hasEditor() const override{return true;}juce::AudioProcessorEditor* createEditor() override;
 int getNumPrograms() override{return presetCount;} int getCurrentProgram() override{return program.load();}
 void setCurrentProgram(int i) override;const juce::String getProgramName(int i) override{return presets[(size_t)juce::jlimit(0,presetCount-1,i)].name;}
 void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override;void setStateInformation(const void*,int) override;
 float value(const char* id)const{return state.getRawParameterValue(id)->load();}
 void setValue(const char* id,float v);
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 void render(juce::AudioBuffer<float>&,bool);
 juce::SmoothedValue<float> mix,wet;
 double sampleRate=44100;float phase=1,gain=1,env=0,sinceTrigger=1e9f;bool armed=true;
 std::atomic<int> program{0};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PumpProcessor)
};
