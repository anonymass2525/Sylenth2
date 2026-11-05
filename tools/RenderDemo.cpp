#include "sylenth2/Synth.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void le16(std::ostream& os, unsigned x) {
    os.put(static_cast<char>(x & 255));os.put(static_cast<char>((x>>8)&255));
}
void le32(std::ostream& os, std::uint32_t x) {le16(os,x&65535u);le16(os,x>>16);}
void wav(const std::filesystem::path& path,const std::vector<float>& l,const std::vector<float>& r,int sr) {
    std::ofstream out(path,std::ios::binary);
    if(!out)throw std::runtime_error("Cannot open WAV output");
    const auto n=static_cast<std::uint32_t>(l.size());
    out.write("RIFF",4);le32(out,36+n*4);out.write("WAVEfmt ",8);
    le32(out,16);le16(out,1);le16(out,2);le32(out,sr);le32(out,sr*4);le16(out,4);le16(out,16);
    out.write("data",4);le32(out,n*4);
    for(std::size_t i=0;i<l.size();++i) {
        auto to16=[](float v)->std::uint16_t {
            const int sample=static_cast<int>(std::lrint(std::clamp(v,-1.0f,1.0f)*32767.0f));
            return static_cast<std::uint16_t>(static_cast<std::int16_t>(sample));
        };
        le16(out,to16(l[i]));le16(out,to16(r[i]));
    }
}
}
int main(int argc,char** argv) {
    try {
        namespace fs=std::filesystem;
        const fs::path dir=argc>1?fs::path(argv[1]):fs::path("demos");
        fs::create_directories(dir);
        constexpr int sr=48000;
        constexpr int length=6*sr;
        const int notes[]={60,64,67,72,67,64,60};
        int patchNo=0;
        const bool compare=argc>2 && std::string(argv[2])=="--compare";
        const bool study=argc>2 && std::string(argv[2])=="--study";
        const bool depthStudy=argc>2 && (std::string(argv[2])=="--depth-study" || std::string(argv[2])=="--depth-raw");
        const bool raw=argc>2 && std::string(argv[2])=="--depth-raw";
        const bool chords=argc>2 && std::string(argv[2])=="--depth-chords";
        if(chords) {
            constexpr int chordLength=5*sr;
            constexpr std::array<std::array<int,3>,3> groups{{
                {{60,61,64}}, // adjacent notes: potential local masking
                {{48,60,67}}, // octave plus fifth: overtone coincidences
                {{60,64,67}}  // familiar triad
            }};
            constexpr std::array<const char*,3> labels{{"Adjacent_Cluster","Octave_Fifth","Major_Triad"}};
            constexpr std::array<const char*,3> variants{{"Body04","OverlapGuardOnly","DepthAll"}};
            for(std::size_t g=0;g<groups.size();++g) {
                for(int v=0;v<3;++v) {
                    auto patch=sylenth2::factoryPatches()[0];
                    patch.chorus=patch.delay=0.0f; // isolate resonances from spatial FX
                    patch.engineMode=(v==0)?sylenth2::EngineMode::CrystalBody:sylenth2::EngineMode::CrystalDepth;
                    patch.coreFocus=patch.depthHarmonics=patch.stereoDepth=0.0f;
                    patch.resonanceGuard=(v>0)?0.65f:0.0f;
                    if(v==2){patch.coreFocus=0.58f;patch.depthHarmonics=0.45f;patch.stereoDepth=0.52f;}
                    sylenth2::Synth synth;synth.prepare(sr);synth.setPatch(patch);
                    std::vector<float> l(chordLength),r(chordLength);
                    for(int i=0;i<chordLength;++i) {
                        if(i==0)for(int note:groups[g])synth.noteOn(note,0.68f);
                        if(i==3*sr)for(int note:groups[g])synth.noteOff(note);
                        synth.render(&l[static_cast<std::size_t>(i)],&r[static_cast<std::size_t>(i)],1);
                    }
                    float peak=0.0f;
                    for(float x:l)peak=std::max(peak,std::abs(x));
                    for(float x:r)peak=std::max(peak,std::abs(x));
                    if(!std::isfinite(peak)||peak<=0.0f)return 2;
                    const float scale=0.70794578f/peak;
                    for(float& x:l)x*=scale;
                    for(float& x:r)x*=scale;
                    const auto file=dir/(std::string(labels[g])+"_"+variants[static_cast<std::size_t>(v)]+".wav");
                    wav(file,l,r,sr);
                    std::cout<<file.string()<<" peak="<<peak<<"\n";
                }
            }
            return 0;
        }
        for(const auto& original:sylenth2::factoryPatches()) {
          for(int mode=0;mode<(compare?7:(study?5:(depthStudy?6:1)));++mode) {
            auto patch=original;
            if (study) {
                // Dedicated controlled ablations: original, crystal, high-band only,
                // parallel resonance only, body. Engine indices remain host-stable.
                if (mode==0) patch.engineMode=sylenth2::EngineMode::Original;
                if (mode==1) patch.engineMode=sylenth2::EngineMode::CrystalClarity;
                if (mode==2) { patch.engineMode=sylenth2::EngineMode::CrystalClarity; patch.crystalResonance=0.0f; }
                if (mode==3) { patch.engineMode=sylenth2::EngineMode::CrystalClarity; patch.crystalClarity=0.0f; }
                if (mode==4) patch.engineMode=sylenth2::EngineMode::CrystalBody;
            } else if (depthStudy) {
                patch.engineMode=(mode==0)?sylenth2::EngineMode::CrystalBody:sylenth2::EngineMode::CrystalDepth;
                patch.coreFocus=patch.depthHarmonics=patch.stereoDepth=patch.resonanceGuard=0.0f;
                if(mode==1)patch.coreFocus=0.58f;
                if(mode==2)patch.depthHarmonics=0.45f;
                if(mode==3)patch.stereoDepth=0.52f;
                if(mode==4)patch.resonanceGuard=0.65f;
                if(mode==5){patch.coreFocus=0.58f;patch.depthHarmonics=0.45f;patch.stereoDepth=0.52f;patch.resonanceGuard=0.65f;}
            } else patch.engineMode=static_cast<sylenth2::EngineMode>(mode);
            sylenth2::Synth synth;
            synth.prepare(sr);
            synth.setPatch(patch);
            std::vector<float> l(length),r(length);
            constexpr int noteSpace=sr*3/5;
            constexpr int noteTime=sr*2/5;
            for(int i=0;i<length;++i) {
                if(i<static_cast<int>(std::size(notes))*noteSpace && i%noteSpace==0)
                    synth.noteOn(notes[i/noteSpace],0.72f);
                if(i<static_cast<int>(std::size(notes))*noteSpace && i%noteSpace==noteTime)
                    synth.noteOff(notes[i/noteSpace]);
                synth.render(&l[static_cast<std::size_t>(i)],&r[static_cast<std::size_t>(i)],1);
            }
            float peak=0.0f;
            for(auto v:l)peak=std::max(peak,std::abs(v));
            for(auto v:r)peak=std::max(peak,std::abs(v));
            std::string name=patch.name;
            std::replace(name.begin(),name.end(),' ','_');
            if(compare)name += std::array<const char*,7>{"_Original","_Harmonic","_Vocal","_Fusion","_CrystalClarity","_CrystalBody","_CrystalDepth"}[static_cast<std::size_t>(mode)];
            if(study)name += std::array<const char*,5>{"_Original","_CrystalClarity","_ClarityOnly","_ResonanceOnly","_CrystalBody"}[static_cast<std::size_t>(mode)];
            if(depthStudy)name += std::array<const char*,6>{"_Body04","_CoreOnly","_HarmonicOnly","_StereoOnly","_OverlapGuardOnly","_DepthAll"}[static_cast<std::size_t>(mode)];
            const auto file=dir/(name+".wav");
            const float scaling=raw?1.0f:(peak>0.0f ? 0.70794578f/peak : 1.0f); // optional unnormalized depth raw
            for(auto& v:l)v*=scaling;
            for(auto& v:r)v*=scaling;
            wav(file,l,r,sr);
            std::cout<<"Preset "<<++patchNo<<" | peak="<<peak<<" | "<<file.string()<<"\n";
            if(!std::isfinite(peak) || peak<=0.0f) {
                std::cerr<<"ERROR: invalid or silent demo.\n"; return 2;
            }
          }
        }
    } catch(const std::exception& ex) {std::cerr<<ex.what()<<'\n';return 1;}
}
