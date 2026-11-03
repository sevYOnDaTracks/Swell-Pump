#include "PluginEditor.h"
#include "BinaryData.h"
namespace {const juce::Colour bg{0xff1b1c20},panel{0xff222328},line{0xff34353b},ink{0xffeeeef1},muted{0xff9a9ba3},orange{0xffff8a3d};
void swellText(juce::Graphics& g,juce::String s,float x,float y,float w,float h,float size,juce::Colour c=ink,juce::Justification j=juce::Justification::centredLeft,bool bold=false){g.setColour(c);g.setFont(juce::FontOptions(size,bold?juce::Font::bold:juce::Font::plain));g.drawText(s,juce::Rectangle<float>(x,y,w,h),j);}
juce::Path curvePath(int curve,float depth,juce::Rectangle<float> r,int points=160){juce::Path p;for(int i=0;i<=points;++i){float t=(float)i/points,v=1-depth*(1-curveGain(curve,t));float x=r.getX()+t*r.getWidth(),y=r.getBottom()-v*r.getHeight();if(i==0)p.startNewSubPath(x,y);else p.lineTo(x,y);}return p;}}
PumpLook::PumpLook(){setColour(juce::TextButton::buttonColourId,juce::Colour(0xff292a30));setColour(juce::TextButton::buttonOnColourId,orange);setColour(juce::TextButton::textColourOffId,ink);setColour(juce::TextButton::textColourOnId,bg);
 setColour(juce::ComboBox::backgroundColourId,panel);setColour(juce::ComboBox::outlineColourId,line);setColour(juce::ComboBox::textColourId,ink);setColour(juce::ComboBox::arrowColourId,orange);
 setColour(juce::PopupMenu::backgroundColourId,bg);setColour(juce::PopupMenu::highlightedBackgroundColourId,orange);setColour(juce::PopupMenu::highlightedTextColourId,bg);setColour(juce::PopupMenu::headerTextColourId,orange);
 setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);setColour(juce::Slider::textBoxTextColourId,ink);setColour(juce::Slider::trackColourId,orange);setColour(juce::Slider::backgroundColourId,line);setColour(juce::Slider::thumbColourId,ink);setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);setColour(juce::Slider::textBoxHighlightColourId,orange.withAlpha(.4f));setColour(juce::TextEditor::outlineColourId,juce::Colours::transparentBlack);setColour(juce::TextEditor::focusedOutlineColourId,orange);}
void PumpLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down){auto r=b.getLocalBounds().toFloat().reduced(.5f);auto c=b.getToggleState()?orange:juce::Colour(0xff292a30);if(over&&!b.getToggleState())c=c.brighter(.15f);if(down)c=c.darker(.1f);g.setColour(c);g.fillRoundedRectangle(r,6);g.setColour(b.getToggleState()?orange:line);g.drawRoundedRectangle(r,6,1);}
void PumpLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider& s){
 auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(6);float size=std::min(r.getWidth(),r.getHeight());r=r.withSizeKeepingCentre(size,size);auto centre=r.getCentre();float radius=size/2;
 juce::Path track,arc;track.addCentredArc(centre.x,centre.y,radius-5,radius-5,0,start,end,true);arc.addCentredArc(centre.x,centre.y,radius-5,radius-5,0,start,start+pos*(end-start),true);
 g.setColour(line);g.strokePath(track,juce::PathStrokeType(6));g.setColour(orange);g.strokePath(arc,juce::PathStrokeType(6));
 auto face=r.reduced(22);g.setGradientFill(juce::ColourGradient(juce::Colour(0xff303138),face.getTopLeft(),juce::Colour(0xff1b1c20),face.getBottomRight(),false));g.fillEllipse(face);g.setColour(juce::Colour(0xff45464b));g.drawEllipse(face,1);
 float angle=start+pos*(end-start);auto a=centre.getPointOnCircumference(radius-30,angle),b=centre.getPointOnCircumference(radius-46,angle);g.setColour(orange);g.drawLine({a,b},4);
 swellText(g,juce::String((int)std::round(s.getValue()))+" %",r.getX(),centre.y-14,r.getWidth(),28,26,ink,juce::Justification::centred,true);}
void CurveButton::paintButton(juce::Graphics& g,bool over,bool){auto r=getLocalBounds().toFloat().reduced(.5f);bool on=getToggleState();g.setColour(on?juce::Colour(0xff3a2a1f):over?juce::Colour(0xff2a2b31):panel);g.fillRoundedRectangle(r,6);g.setColour(on?orange:line);g.drawRoundedRectangle(r,6,on?1.5f:1);
 auto plot=r.reduced(8,7).withTrimmedBottom(13);g.setColour(on?orange:muted);g.strokePath(curvePath(index,1,plot,48),juce::PathStrokeType(1.6f));swellText(g,getButtonText(),r.getX(),r.getBottom()-17,r.getWidth(),14,10,on?ink:muted,juce::Justification::centred);}
PumpEditor::PumpEditor(PumpProcessor& p):AudioProcessorEditor(p),proc(p){setLookAndFeel(&look);logo=juce::ImageCache::getFromMemory(BinaryData::swellmark_png,BinaryData::swellmark_pngSize);
 mixKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);mixKnob.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);mixKnob.setDoubleClickReturnValue(true,100);mixAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.state,"mix",mixKnob);
 for(auto* s:{&threshold,&tempo}){s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);s->setSliderStyle(juce::Slider::LinearHorizontal);s->setTextBoxStyle(juce::Slider::TextBoxRight,false,78,24);addChildComponent(s);}
 threshold.setTextValueSuffix(" dB");tempo.setTextValueSuffix(" BPM");threshold.setDoubleClickReturnValue(true,-24);tempo.setDoubleClickReturnValue(true,120);
 thresholdAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.state,"threshold",threshold);tempoAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.state,"tempo",tempo);
 for(int c=0;c<(int)categories.size();++c){preset.addSectionHeading(categories[(size_t)c]);for(int i=0;i<presetCount;++i)if(juce::String(presets[(size_t)i].category)==categories[(size_t)c])preset.addItem(presets[(size_t)i].name,i+1);}
 preset.onChange=[this]{if(preset.getSelectedId()>0)proc.setCurrentProgram(preset.getSelectedId()-1);};
 previous.onClick=[this]{proc.setCurrentProgram((proc.getCurrentProgram()+presetCount-1)%presetCount);};next.onClick=[this]{proc.setCurrentProgram((proc.getCurrentProgram()+1)%presetCount);};
 for(int i=0;i<curveCount;++i){curveButtons.push_back(std::make_unique<CurveButton>(i));curveButtons.back()->onClick=[this,i]{proc.setValue("curve",(float)i);};addAndMakeVisible(*curveButtons.back());}
 for(int i=0;i<5;++i){rateButtons[(size_t)i].setButtonText(rateNames[(size_t)i]);rateButtons[(size_t)i].onClick=[this,i]{proc.setValue("rate",(float)i);};addAndMakeVisible(rateButtons[(size_t)i]);}
 const char* triggers[]={"Tempo DAW","Sidechain"};for(int i=0;i<2;++i){triggerButtons[(size_t)i].setButtonText(triggers[i]);triggerButtons[(size_t)i].onClick=[this,i]{proc.setValue("trigger",(float)i);};addAndMakeVisible(triggerButtons[(size_t)i]);}
 bypass.setClickingTogglesState(true);bypassAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"bypass",bypass);
 for(juce::Component* c:{(juce::Component*)&mixKnob,(juce::Component*)&preset,(juce::Component*)&previous,(juce::Component*)&next,(juce::Component*)&bypass})addAndMakeVisible(c);
 setSize(620,650);timerCallback();startTimerHz(30);}
PumpEditor::~PumpEditor(){stopTimer();setLookAndFeel(nullptr);}
void PumpEditor::timerCallback(){int program=proc.getCurrentProgram();if(program!=shownProgram){shownProgram=program;preset.setSelectedId(program+1,juce::dontSendNotification);}
 int curve=(int)proc.value("curve"),rate=(int)proc.value("rate");bool sidechain=proc.value("trigger")>.5f;
 for(auto& b:curveButtons)b->setToggleState(b->index==curve,juce::dontSendNotification);for(int i=0;i<5;++i)rateButtons[(size_t)i].setToggleState(i==rate,juce::dontSendNotification);
 triggerButtons[0].setToggleState(!sidechain,juce::dontSendNotification);triggerButtons[1].setToggleState(sidechain,juce::dontSendNotification);
 threshold.setVisible(sidechain);tempo.setVisible(!sidechain&&!proc.hostTempo.load());
 reduction=std::max(reduction*.85f,-juce::Decibels::gainToDecibels(proc.gainDisplay.exchange(1),-60.f));repaint();}
void PumpEditor::resized(){bypass.setBounds(500,25,90,32);previous.setBounds(30,98,36,34);preset.setBounds(72,98,476,34);next.setBounds(554,98,36,34);
 for(int i=0;i<curveCount;++i)curveButtons[(size_t)i]->setBounds(30+i*71,336,64,58);
 mixKnob.setBounds(45,440,200,170);for(int i=0;i<5;++i)rateButtons[(size_t)i].setBounds(290+i*59,448,54,32);
 triggerButtons[0].setBounds(290,520,145,32);triggerButtons[1].setBounds(441,520,140,32);threshold.setBounds(286,578,300,28);tempo.setBounds(286,578,300,28);}
void PumpEditor::paintDisplay(juce::Graphics& g,juce::Rectangle<float> r){
 g.setColour(juce::Colour(0xff121316));g.fillRoundedRectangle(r,9);g.setColour(line);g.drawRoundedRectangle(r,9,1);
 auto plot=r.reduced(20,0).withTrimmedTop(34).withTrimmedBottom(16);int curve=(int)proc.value("curve");float depth=proc.value("mix")/100*(proc.value("bypass")>.5f?0.f:1.f);
 g.setColour(line.withAlpha(.6f));for(int i=1;i<4;++i)g.drawVerticalLine((int)(plot.getX()+plot.getWidth()*i/4),plot.getY(),plot.getBottom());g.drawHorizontalLine((int)plot.getY(),plot.getX(),plot.getRight());g.drawHorizontalLine((int)plot.getBottom(),plot.getX(),plot.getRight());
 auto path=curvePath(curve,depth,plot);juce::Path fill(path);fill.lineTo(plot.getRight(),plot.getBottom());fill.lineTo(plot.getX(),plot.getBottom());fill.closeSubPath();
 g.setGradientFill(juce::ColourGradient(orange.withAlpha(.32f),0,plot.getY(),orange.withAlpha(.03f),0,plot.getBottom(),false));g.fillPath(fill);g.setColour(orange);g.strokePath(path,juce::PathStrokeType(2.2f));
 float t=juce::jlimit(0.f,1.f,proc.phaseDisplay.load()),v=1-depth*(1-curveGain(curve,t));float px=plot.getX()+t*plot.getWidth(),py=plot.getBottom()-v*plot.getHeight();
 g.setColour(ink.withAlpha(.35f));g.drawVerticalLine((int)px,plot.getY(),plot.getBottom());g.setColour(ink);g.fillEllipse(px-5,py-5,10,10);
 bool sidechain=proc.value("trigger")>.5f;juce::String mode=sidechain?"SIDECHAIN  /  "+juce::String(rateNames[(size_t)(int)proc.value("rate")]):"TEMPO  /  "+juce::String(rateNames[(size_t)(int)proc.value("rate")]);
 swellText(g,mode,r.getX()+20,r.getY()+9,300,20,12,orange,juce::Justification::centredLeft,true);
 swellText(g,(proc.hostTempo.load()?"DAW ":"Tempo ")+juce::String(proc.tempoDisplay.load(),1)+" BPM",r.getRight()-330,r.getY()+9,160,20,11,muted,juce::Justification::centredRight);
 swellText(g,"-"+juce::String(reduction,1)+" dB",r.getRight()-160,r.getY()+7,140,24,16,ink,juce::Justification::centredRight,true);}
void PumpEditor::paint(juce::Graphics& g){g.fillAll(bg);g.setColour(line);g.drawHorizontalLine(80,0,(float)getWidth());
 g.drawImageWithin(logo,30,23,38,38,juce::RectanglePlacement::centred);swellText(g,"swell",80,20,100,30,27);swellText(g,"A U D I O",82,49,100,15,8,muted);swellText(g,"PUMP",200,24,200,36,28,ink,juce::Justification::centredLeft,true);
 paintDisplay(g,{30,148,560,174});
 g.setColour(panel);g.fillRoundedRectangle(30,410,232,212,9);g.fillRoundedRectangle(274,410,316,212,9);g.setColour(line);g.drawRoundedRectangle(30,410,232,212,9,1);g.drawRoundedRectangle(274,410,316,212,9,1);
 swellText(g,"MIX",46,418,100,22,13,orange,juce::Justification::centredLeft,true);swellText(g,"RATE",290,418,100,22,13,orange,juce::Justification::centredLeft,true);swellText(g,"TRIGGER",290,490,120,22,13,orange,juce::Justification::centredLeft,true);
 bool sidechain=proc.value("trigger")>.5f;
 if(sidechain)swellText(g,proc.sidechainActive.load()?"Seuil  (entree sidechain)":"Seuil  (pas de sidechain : entree principale)",290,558,300,18,11,muted);
 else if(proc.hostTempo.load())swellText(g,"Cale sur la grille du DAW",290,566,300,30,11,muted);else swellText(g,"Tempo manuel (pas de DAW)",290,558,300,18,11,muted);
 swellText(g,juce::String(presets[(size_t)proc.getCurrentProgram()].name)+"  /  reglages courants",30,(float)getHeight()-24,360,18,11,muted);
 swellText(g,"Swell-audio  /  0.1  /  by @sevyondatracks",300,(float)getHeight()-24,290,18,11,muted,juce::Justification::centredRight);}
