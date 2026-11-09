#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
// Builds a synthetic 124 BPM loop (kick + pad + bass), then renders 4 bars dry followed by
// the same 4 bars through PumpProcessor, for several presets. Usage: PumpDemo <output-folder>
using Buffer=juce::AudioBuffer<float>;
constexpr double sr=44100,bpm=124;constexpr int bars=4;
void set(PumpProcessor& p,const char* id,float v){auto* a=p.state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));}
void save(juce::File f,const Buffer& b){juce::WavAudioFormat format;f.deleteFile();auto stream=f.createOutputStream();if(!stream)throw std::runtime_error("output unavailable");
 std::unique_ptr<juce::AudioFormatWriter>w(format.createWriterFor(stream.get(),sr,2,16,{},0));if(!w)throw std::runtime_error("writer error");stream.release();w->writeFromAudioSampleBuffer(b,0,b.getNumSamples());}
int beatLength(){return (int)std::round(sr*60/bpm);}
Buffer kick(int length){Buffer b(2,length);b.clear();int beat=beatLength();for(int start=0;start<length;start+=beat){double ph=0;for(int n=0;n<(int)(sr*.45)&&start+n<length;++n){double t=n/sr,f=48+110*std::exp(-t*28);ph+=juce::MathConstants<double>::twoPi*f/sr;
 float v=(float)(std::sin(ph)*std::exp(-t*6.5)*.9+(n<60?.25*(1-n/60.):0));for(int c=0;c<2;++c)b.addSample(c,start+n,v);}}return b;}
Buffer music(int length){Buffer b(2,length);b.clear();const double chords[4][4]={{57,60,64,69},{53,57,60,65},{48,52,55,60},{55,59,62,67}};// Am F C G
 int bar=beatLength()*4;double low[2]={0,0};double phases[2][8]={},bass=0;
 for(int n=0;n<length;++n){int chord=(n/bar)%4;float sides[2]={0,0};
  for(int c=0;c<2;++c){double s=0;for(int k=0;k<4;++k){double f=440*std::pow(2.,(chords[chord][k]-69)/12.)*(1+(c?.004:-.004)*(k%2?1:-1));auto& ph=phases[c][k];ph+=f/sr;ph-=std::floor(ph);s+=2*ph-1;}
   low[c]+=.08*(s*.11-low[c]);sides[c]=(float)low[c];}
  double fb=440*std::pow(2.,(chords[chord][0]-12-69)/12.);bass+=juce::MathConstants<double>::twoPi*fb/sr;float sub=(float)(std::sin(bass)*.32);
  for(int c=0;c<2;++c)b.setSample(c,n,sides[c]+sub);}
 return b;}
Buffer pump(const Buffer& in,const Buffer& trigger,int preset){PumpProcessor p;p.setNonRealtime(true);p.setCurrentProgram(preset);set(p,"tempo",(float)bpm);bool sc=presets[(size_t)preset].trigger==1;if(sc)p.enableAllBuses();
 p.prepareToPlay(sr,256);Buffer out(2,in.getNumSamples()),block(sc?4:2,256);juce::MidiBuffer midi;
 for(int offset=0;offset<in.getNumSamples();offset+=256){int count=std::min(256,in.getNumSamples()-offset);block.clear();for(int c=0;c<2;++c){block.copyFrom(c,0,in,c,offset,count);if(sc)block.copyFrom(c+2,0,trigger,c,offset,count);}
  p.processBlock(block,midi);for(int c=0;c<2;++c)out.copyFrom(c,offset,block,c,0,count);}return out;}
int main(int argc,char** argv){swell::Licence::unlockForTesting();try{juce::ScopedJuceInitialiser_GUI init;if(argc!=2)throw std::runtime_error("Usage: PumpDemo output-folder");juce::File folder(argv[1]);folder.createDirectory();
 const int length=beatLength()*4*bars;auto k=kick(length);auto m=music(length);
 auto mixdown=[&](const Buffer& musicPart){Buffer b(2,length);for(int c=0;c<2;++c){b.copyFrom(c,0,k,c,0,length);b.addFrom(c,0,musicPart,c,0,length);}return b;};
 auto dry=mixdown(m);const float gain=juce::Decibels::decibelsToGain(-1.f)/std::max(dry.getMagnitude(0,0,length),dry.getMagnitude(1,0,length));
 dry.applyGain(gain);save(folder.getChildFile("00-boucle-AVANT.wav"),dry);
 juce::String report="Boucle synthetique 124 BPM generee par Tools/RenderDemo.cpp (kick, pad Am-F-C-G, basse) : aucun sample externe.\n"
  "Chaque fichier AVANT-APRES : 4 mesures sans effet, puis les memes 4 mesures avec Swell Pump sur le pad + la basse (le kick n'est pas traite).\n"
  "Traitement hors ligne par PumpProcessor, le meme code que le VST3/Standalone. Meme gain de sortie pour avant et apres (pas de compensation : la pompe baisse le niveau moyen).\n";
 const int chosen[]={0,3,8,11,15};
 for(int preset:chosen){auto wet=mixdown(pump(m,k,preset));wet.applyGain(gain);Buffer ab(2,length*2);for(int c=0;c<2;++c){ab.copyFrom(c,0,dry,c,0,length);ab.copyFrom(c,length,wet,c,0,length);}
  if(ab.getMagnitude(0,ab.getNumSamples())>=1)throw std::runtime_error("clipping");
  auto name=juce::String(preset+1).paddedLeft('0',2)+"-"+juce::String(presets[(size_t)preset].name).replaceCharacters(" /","--")+"-AVANT-APRES.wav";save(folder.getChildFile(name),ab);report+=name+"\n";}
 folder.getChildFile("SOURCE-ET-METHODE.txt").replaceWithText(report);std::cout<<report;return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
