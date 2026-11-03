#pragma once
#include "PluginProcessor.h"
class PumpLook:public juce::LookAndFeel_V4 {public:PumpLook();void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;
 void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool)override;};
class CurveButton:public juce::Button {public:explicit CurveButton(int i):juce::Button(curves[(size_t)i].name),index(i){}void paintButton(juce::Graphics&,bool,bool)override;int index;};
class PumpEditor:public juce::AudioProcessorEditor,private juce::Timer {
public:explicit PumpEditor(PumpProcessor&);~PumpEditor()override;void paint(juce::Graphics&)override;void resized()override;
private:void timerCallback()override;void paintDisplay(juce::Graphics&,juce::Rectangle<float>);
PumpProcessor& proc;PumpLook look;juce::Image logo;
juce::Slider mixKnob,threshold,tempo;juce::ComboBox preset;juce::TextButton previous{"<"},next{">"},bypass{"Bypass"};
std::vector<std::unique_ptr<CurveButton>> curveButtons;std::array<juce::TextButton,5> rateButtons;std::array<juce::TextButton,2> triggerButtons;
std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach,thresholdAttach,tempoAttach;
std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;
int shownProgram=-1;float reduction=0;
};
