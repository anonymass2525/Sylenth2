#include "sylenth2/Synth.hpp"
#include <algorithm>
#include <array>
#ifdef NDEBUG
#undef NDEBUG // These regression checks MUST run in CI Release builds.
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const auto& patches=sylenth2::factoryPatches();
    assert(patches.size()==8);
    for(double sr:{44100.0,48000.0,96000.0,192000.0}) {
        sylenth2::Synth synth;
        synth.prepare(sr);
        assert(synth.sampleRate()==sr);
        constexpr int n=2048;
        std::vector<float> l(n),r(n);
        synth.render(l.data(),r.data(),n);
        for(int i=0;i<n;++i)assert(l[i]==0.0f && r[i]==0.0f);
        for(const auto& p:patches) {
            synth.reset();synth.setPatch(p);
            synth.noteOn(60,0.7f);
            synth.noteOn(64,0.65f);
            synth.noteOn(67,0.65f);
            synth.pitchBend(.3f);
            synth.render(l.data(),r.data(),n);
            float energy=0,peak=0;
            for(int i=0;i<n;++i) {
                assert(std::isfinite(l[i]) && std::isfinite(r[i]));
                energy+=l[i]*l[i]+r[i]*r[i];
                peak=std::max({peak,std::abs(l[i]),std::abs(r[i])});
            }
            assert(energy>0.0000001f);
            assert(peak<20.0f); // catastrophic instability, not a loudness spec
            synth.noteOff(60);synth.noteOff(64);synth.noteOff(67);
            synth.render(l.data(),r.data(),n);
            for(int i=0;i<n;++i)assert(std::isfinite(l[i])&&std::isfinite(r[i]));
        }
        synth.allNotesOff();
        synth.sustainPedal(true);
        synth.noteOn(48,0.8f);
        synth.noteOff(48);
        synth.sustainPedal(false);
        synth.render(l.data(),r.data(),n);
    }
    // Run identical MIDI for every engine, with disabled time effects to isolate DSP.
    for(double sr:{44100.0,48000.0,96000.0,192000.0}) {
        constexpr int frames=4096;
        std::array<float,frames> original{};
        float previousEnergy=0.0f;
        for(int mode=0;mode<6;++mode) {
            auto patch=patches[3]; // Midnight Vowel
            patch.engineMode=static_cast<sylenth2::EngineMode>(mode);
            patch.chorus=patch.delay=0.0f;
            patch.harmonicMix=0.85f;
            patch.vocalBeauty=0.85f;
            sylenth2::Synth engine;
            engine.prepare(sr);
            engine.setPatch(patch);
            engine.noteOn(60,0.70f);
            std::array<float,frames> l{},r{};
            engine.render(l.data(),r.data(),frames);
            float energy=0.0f;
            for(int i=0;i<frames;++i) {
                assert(std::isfinite(l[i]) && std::isfinite(r[i]));
                assert(std::abs(l[i])<20.0f && std::abs(r[i])<20.0f);
                energy+=l[i]*l[i]+r[i]*r[i];
            }
            assert(energy>1.0e-7f);
            if(mode>0) {
                double diff=0.0;
                for(int i=0;i<frames;++i)diff+=std::abs(l[i]-original[i]);
                assert(diff>0.001); // the new mode must not be a renamed Original
            } else original=l;
            previousEnergy=energy;
        }
        assert(previousEnergy>0.0f);
    }
    // Crystal Clarity: both zero controls must be a bit-identical bypass of Original.
    for (double sr:{44100.0,48000.0,96000.0,192000.0}) {
        for (const auto& factory:patches) {
            auto a=factory; a.engineMode=sylenth2::EngineMode::Original;
            auto b=factory; b.engineMode=sylenth2::EngineMode::CrystalClarity;
            b.crystalClarity=0.0f; b.crystalResonance=0.0f;
            sylenth2::Synth x,y; x.prepare(sr); y.prepare(sr);
            x.setPatch(a); y.setPatch(b);
            x.noteOn(57,0.8f); y.noteOn(57,0.8f);
            std::array<float,256> xl{},xr{},yl{},yr{};
            for (int block=0;block<12;++block) {
                x.render(xl.data(),xr.data(),xl.size());
                y.render(yl.data(),yr.data(),yl.size());
                for (std::size_t i=0;i<xl.size();++i) {
                    assert(xl[i]==yl[i]); assert(xr[i]==yr[i]);
                }
            }
        }
    }
    // New v0.4 branch: with all three Crystal and Body levels zero, the
    // output MUST match the Original exactly, even across sustained notes.
    for(double sr:{44100.0,48000.0,96000.0,192000.0}) {
        for(const auto& preset:patches) {
            auto original=preset;
            original.engineMode=sylenth2::EngineMode::Original;
            auto bypass=preset;
            bypass.engineMode=sylenth2::EngineMode::CrystalBody;
            bypass.crystalClarity=bypass.crystalResonance=0.0f;
            bypass.bodyDepth=bypass.bodyDensity=bypass.bodyDynamics=0.0f;
            sylenth2::Synth a,b;
            a.prepare(sr); b.prepare(sr);
            a.setPatch(original); b.setPatch(bypass);
            a.noteOn(55,0.81f); b.noteOn(55,0.81f);
            a.noteOn(62,0.63f); b.noteOn(62,0.63f);
            std::array<float,128> al{},ar{},bl{},br{};
            for(int block=0;block<18;++block) {
                a.render(al.data(),ar.data(),al.size());
                b.render(bl.data(),br.data(),bl.size());
                for(int i=0;i<128;++i) { assert(al[i]==bl[i]); assert(ar[i]==br[i]); }
            }
        }
    }
    // A meaningful non-bypassed Body mode must change the audible dry signal.
    for(double sr:{44100.0,48000.0,96000.0,192000.0}) {
        auto original=patches[0]; original.engineMode=sylenth2::EngineMode::CrystalClarity;
        auto thick=original; thick.engineMode=sylenth2::EngineMode::CrystalBody;
        sylenth2::Synth a,b; a.prepare(sr); b.prepare(sr);
        a.setPatch(original); b.setPatch(thick);
        a.noteOn(60,0.75f); b.noteOn(60,0.75f);
        std::array<float,4096> al{},ar{},bl{},br{};
        a.render(al.data(),ar.data(),al.size()); b.render(bl.data(),br.data(),bl.size());
        double diff=0.0;
        for(std::size_t i=0;i<al.size();++i) {
            assert(std::isfinite(bl[i]) && std::isfinite(br[i]));
            diff+=std::abs(static_cast<double>(al[i])-bl[i]);
            assert(std::abs(bl[i])<20.0f && std::abs(br[i])<20.0f);
        }
        assert(diff>0.0001);
    }
    // Crystal Depth v0.5: all four axes zero -> bit-exact v0.4 Body.
    for (double sr:{44100.0,48000.0,96000.0,192000.0}) {
        for (const auto& factory:patches) {
            auto body=factory; body.engineMode=sylenth2::EngineMode::CrystalBody;
            auto bypass=body; bypass.engineMode=sylenth2::EngineMode::CrystalDepth;
            bypass.coreFocus=bypass.depthHarmonics=bypass.stereoDepth=bypass.resonanceGuard=0.0f;
            sylenth2::Synth x,y; x.prepare(sr); y.prepare(sr);x.setPatch(body);y.setPatch(bypass);
            x.noteOn(60,0.8f);y.noteOn(60,0.8f);
            x.noteOn(61,0.7f);y.noteOn(61,0.7f);
            std::array<float,256> xl{},xr{},yl{},yr{};
            for(int block=0;block<16;++block){
                x.render(xl.data(),xr.data(),xl.size());y.render(yl.data(),yr.data(),yl.size());
                for(std::size_t i=0;i<xl.size();++i){assert(xl[i]==yl[i]);assert(xr[i]==yr[i]);}
            }
        }
    }
    // Each independently enabled effect must change the signal in a suitable test.
    for (int axis=0;axis<4;++axis) {
        auto body=patches[0];body.engineMode=sylenth2::EngineMode::CrystalBody;
        body.chorus=body.delay=0.0f;
        auto variant=body;variant.engineMode=sylenth2::EngineMode::CrystalDepth;
        variant.coreFocus=variant.depthHarmonics=variant.stereoDepth=variant.resonanceGuard=0.0f;
        if(axis==0)variant.coreFocus=1.0f;
        if(axis==1)variant.depthHarmonics=1.0f;
        if(axis==2)variant.stereoDepth=1.0f;
        if(axis==3)variant.resonanceGuard=1.0f;
        sylenth2::Synth x,y;x.prepare(48000.0);y.prepare(48000.0);
        x.setPatch(body);y.setPatch(variant);
        x.noteOn(60,0.75f);y.noteOn(60,0.75f);
        if(axis==3){x.noteOn(61,0.68f);y.noteOn(61,0.68f);}
        std::array<float,4096> xl{},xr{},yl{},yr{};
        x.render(xl.data(),xr.data(),xl.size());y.render(yl.data(),yr.data(),yl.size());
        double diff=0.0,monoDiff=0.0;
        for(std::size_t i=0;i<xl.size();++i){
            assert(std::isfinite(yl[i])&&std::isfinite(yr[i]));
            diff+=std::abs(xl[i]-yl[i])+std::abs(xr[i]-yr[i]);
            monoDiff+=std::abs((xl[i]+xr[i])-(yl[i]+yr[i]));
        }
        assert(diff>0.00001); // no inert control
        if(axis==2)assert(monoDiff<0.00012); // Mid unchanged except float rounding
    }
    // No resonance collision on a truly monophonic phrase -> guard does nothing.
    {
        auto a=patches[3];a.engineMode=sylenth2::EngineMode::CrystalBody;
        auto b=a;b.engineMode=sylenth2::EngineMode::CrystalDepth;
        b.coreFocus=b.depthHarmonics=b.stereoDepth=0.0f;b.resonanceGuard=1.0f;
        sylenth2::Synth x,y;x.prepare(48000);y.prepare(48000);x.setPatch(a);y.setPatch(b);
        x.noteOn(64,0.7f);y.noteOn(64,0.7f);
        std::array<float,512> xl{},xr{},yl{},yr{};
        x.render(xl.data(),xr.data(),xl.size());y.render(yl.data(),yr.data(),yl.size());
        for(std::size_t i=0;i<xl.size();++i){assert(xl[i]==yl[i]);assert(xr[i]==yr[i]);}
    }
    std::cout<<"PASS: Original and Crystal Body preservation, seven modes, 4 independent depth effects, mono-safe stereo, 44.1/48/96/192 kHz\n";
}
