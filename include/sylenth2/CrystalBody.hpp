#pragma once
// v0.4 Crystal Body: note-tracked, strictly parallel harmonic body.
// Per-voice stereo resonators reinforce existing partials; no nonlinearity.
#include "sylenth2/BeautyEngine.hpp"
#include <algorithm>
#include <cmath>

namespace sylenth2 {
struct CrystalBodyVoice {
    BeautyBandpass fundamentalL, fundamentalR, secondL, secondR;
    float envelope=0.0f, onsetStep=0.0f, secondAvailable=1.0f;
    double lastFrequency=-1.0, lastSampleRate=-1.0;

    void configure(double frequency, double sampleRate) noexcept {
        if (frequency==lastFrequency && sampleRate==lastSampleRate) return;
        lastFrequency=frequency; lastSampleRate=sampleRate;
        const double sr=std::max(8000.0,sampleRate);
        const float f0=static_cast<float>(std::max(20.0,frequency));
        const float f1=std::min(f0,static_cast<float>(sr*0.40));
        const float f2=std::min(2.0f*f0,static_cast<float>(sr*0.40));
        secondAvailable=(2.0*frequency<0.40*sr)?1.0f:0.0f;
        fundamentalL.set(f1,0.85f,sr); fundamentalR.set(f1,0.85f,sr);
        secondL.set(f2,0.95f,sr); secondR.set(f2,0.95f,sr);
        onsetStep=static_cast<float>(1.0-std::exp(-1.0/(0.032*sr)));
    }
    void reset() noexcept {
        fundamentalL.reset(); fundamentalR.reset();
        secondL.reset(); secondR.reset(); envelope=0.0f;
    }
    void tick(float& l,float& r,float depth,float density,float transientProtection) noexcept {
        if(depth<=0.0f) return;
        envelope+=onsetStep*(1.0f-envelope);
        const float gate=(1.0f-transientProtection)+transientProtection*envelope;
        const float mix=std::clamp(depth,0.0f,1.0f)*gate;
        const float amount1=0.085f*mix;
        const float amount2=0.095f*mix*std::clamp(density,0.0f,1.0f)*secondAvailable;
        const float originalL=l,originalR=r;
        l=originalL+amount1*fundamentalL.tick(originalL)+amount2*secondL.tick(originalL);
        r=originalR+amount1*fundamentalR.tick(originalR)+amount2*secondR.tick(originalR);
    }
};
} // namespace sylenth2
