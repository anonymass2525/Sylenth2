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
    for(double sr:{44100.0,48000.0,96000.0}) {
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
    for(double sr:{44100.0,48000.0,96000.0}) {
        constexpr int frames=4096;
        std::array<float,frames> original{};
        float previousEnergy=0.0f;
        for(int mode=0;mode<4;++mode) {
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
    std::cout<<"PASS: 8 original presets, 44.1/48/96 kHz, silence, MIDI, finite/stable output\n";
}
