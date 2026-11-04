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
        for(const auto& original:sylenth2::factoryPatches()) {
          for(int mode=0;mode<(compare?6:(study?5:1));++mode) {
            auto patch=original;
            if (study) {
                if (mode==0) patch.engineMode=sylenth2::EngineMode::Original;
                if (mode==1) patch.engineMode=sylenth2::EngineMode::CrystalClarity;
                if (mode==2) { patch.engineMode=sylenth2::EngineMode::CrystalClarity; patch.crystalResonance=0.0f; }
                if (mode==3) { patch.engineMode=sylenth2::EngineMode::CrystalClarity; patch.crystalClarity=0.0f; }
                if (mode==4) patch.engineMode=sylenth2::EngineMode::CrystalBody;
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
            if(compare)name += std::array<const char*,6>{"_Original","_Harmonic","_Vocal","_Fusion","_CrystalClarity","_CrystalBody"}[static_cast<std::size_t>(mode)];
            if(study)name += std::array<const char*,5>{"_Original","_CrystalClarity","_ClarityOnly","_ResonanceOnly","_CrystalBody"}[static_cast<std::size_t>(mode)];
            const auto file=dir/(name+".wav");
            const float scaling=peak>0.0f ? 0.70794578f/peak : 1.0f; // -3 dBFS peak normalized preview
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
