#include "sylenth2/Synth.hpp"
#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include <array>
#include <cmath>

class Sylenth2Processor final : public juce::AudioProcessor {
public:
    Sylenth2Processor()
        : juce::AudioProcessor(BusesProperties().withOutput("Stereo Output",juce::AudioChannelSet::stereo(),true)),
          params_(*this,nullptr,"PARAMETERS",makeLayout()) {}

    const juce::String getName() const override {return "Sylenth2 Experimental";}
    bool acceptsMidi() const override {return true;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return 6.0;}
    int getNumPrograms() override {return static_cast<int>(sylenth2::factoryPatches().size());}
    int getCurrentProgram() override {return currentProgram_;}
    void setCurrentProgram(int n) override {
        currentProgram_=juce::jlimit(0,getNumPrograms()-1,n);
        if(auto* v=params_.getParameter("preset"))
            v->setValueNotifyingHost(static_cast<float>(currentProgram_)/static_cast<float>(getNumPrograms()-1));
    }
    const juce::String getProgramName(int n) override {
        return n>=0 && n<getNumPrograms()?sylenth2::factoryPatches()[static_cast<std::size_t>(n)].name:"";
    }
    void changeProgramName(int,const juce::String&) override {}
    void prepareToPlay(double sr,int) override {synth_.prepare(sr);}
    void releaseResources() override {synth_.reset();}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
        return layouts.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();
    }
    void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) override {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();
        if(buffer.getNumChannels()<2)return;
        const auto load=[this](const char* name)->float {
            return params_.getRawParameterValue(name)->load();
        };
        const int index=juce::jlimit(0,getNumPrograms()-1,static_cast<int>(std::lround(load("preset"))));
        currentProgram_=index;
        auto patch=sylenth2::factoryPatches()[static_cast<std::size_t>(index)];
        patch.cutoffHz*=std::exp2((load("brightness")-0.5f)*3.5f);
        patch.filterEnvOctaves*=0.2f+1.6f*load("motion");
        patch.stereo=std::clamp(patch.stereo*(0.1f+1.8f*load("width")),0.0f,1.0f);
        patch.detuneA*=0.35f+1.3f*load("width");
        patch.detuneB*=0.35f+1.3f*load("width");
        patch.drive=std::clamp(patch.drive+load("drive")*0.45f,0.0f,1.0f);
        patch.engineMode=static_cast<sylenth2::EngineMode>(juce::jlimit(0,3,static_cast<int>(std::lround(load("engine")))));
        patch.harmonicColor=load("harmonicColor");
        patch.harmonicMix=load("harmonicMix");
        patch.vocalBeauty=load("vocalBeauty");
        patch.vocalMotion=load("vocalMotion");
        synth_.setPatch(patch);
        const auto total=buffer.getNumSamples();
        int cursor=0;
        auto process=[&](int last) {
            if(last>cursor) {
                synth_.render(buffer.getWritePointer(0,cursor),buffer.getWritePointer(1,cursor),static_cast<std::size_t>(last-cursor));
                cursor=last;
            }
        };
        for(const auto metadata:midi) {
            const int when=juce::jlimit(cursor,total,metadata.samplePosition);
            process(when);
            const auto& msg=metadata.getMessage();
            if(msg.isNoteOn())synth_.noteOn(msg.getNoteNumber(),msg.getFloatVelocity());
            else if(msg.isNoteOff())synth_.noteOff(msg.getNoteNumber());
            else if(msg.isPitchWheel())synth_.pitchBend((msg.getPitchWheelValue()-8192)/8192.0f);
            else if(msg.isController() && msg.getControllerNumber()==64)synth_.sustainPedal(msg.getControllerValue()>=64);
            else if(msg.isAllNotesOff() || msg.isAllSoundOff())synth_.allNotesOff();
        }
        process(total);
    }
    bool hasEditor() const override {return true;}
    juce::AudioProcessorEditor* createEditor() override {return new juce::GenericAudioProcessorEditor(*this);}
    void getStateInformation(juce::MemoryBlock& dest) override {
        if(const auto state=params_.copyState().createXml())copyXmlToBinary(*state,dest);
    }
    void setStateInformation(const void* data,int len) override {
        if(const auto xml=getXmlFromBinary(data,len)) {
            if(xml->hasTagName(params_.state.getType()))params_.replaceState(juce::ValueTree::fromXml(*xml));
        }
    }
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        juce::StringArray names;
        for(const auto& p:sylenth2::factoryPatches())names.add(p.name);
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"preset",1},"Preset",names,0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"brightness",1},"Brightness",0.0f,1.0f,0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"motion",1},"Motion",0.0f,1.0f,0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"width",1},"Unison Width",0.0f,1.0f,0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive",1},"Drive",0.0f,1.0f,0.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"engine",1},"Engine",juce::StringArray{"Original","Harmonic","Vocal","Fusion"},0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"harmonicColor",1},"Harmonic Color",0.0f,1.0f,0.65f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"harmonicMix",1},"Harmonic Mix",0.0f,1.0f,0.75f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"vocalBeauty",1},"Vocal Beauty",0.0f,1.0f,0.65f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"vocalMotion",1},"Vocal Motion",0.0f,1.0f,0.40f));
        return layout;
    }
    sylenth2::Synth synth_;
    juce::AudioProcessorValueTreeState params_;
    int currentProgram_=0;
};
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {return new Sylenth2Processor();}
