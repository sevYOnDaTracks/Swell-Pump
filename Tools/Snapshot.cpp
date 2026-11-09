#include "../Source/PluginProcessor.h"
// Renders the editor to a PNG for documentation: Snapshot <out.png> [preset]
int main(int argc,char** argv){swell::Licence::unlockForTesting();juce::ScopedJuceInitialiser_GUI gui;PumpProcessor p;if(argc>2)p.setCurrentProgram(juce::String(argv[2]).getIntValue());p.prepareToPlay(48000,256);
 juce::MidiBuffer m;juce::AudioBuffer<float>b(2,256);for(int i=0;i<40;++i){for(int c=0;c<2;++c)for(int n=0;n<256;++n)b.setSample(c,n,.5f);p.processBlock(b,m);}
 std::unique_ptr<juce::AudioProcessorEditor> e(p.createEditor());auto image=e->createComponentSnapshot(e->getLocalBounds(),true,2.f);
 juce::File out(juce::File::getCurrentWorkingDirectory().getChildFile(argc>1?argv[1]:"snapshot.png"));out.deleteFile();juce::FileOutputStream s(out);juce::PNGImageFormat().writeImageToStream(image,s);return 0;}
