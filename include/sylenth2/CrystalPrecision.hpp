#pragma once
// Crystal Precision 0.6 - independent hypotheses, not a claim of perceptual superiority.
// Each optional DSP unit is completely dormant outside EngineMode::CrystalPrecision.
#include "sylenth2/BeautyEngine.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace sylenth2 {

// Timbre-preserving experiments: a note-following mid anchor and restrained
// third/fourth-harmonic reinforcement. No added oscillator or nonlinearity.
// A bounded, slowly adapting energy correction limits loudness bias in A/B.
struct CrystalPrecisionVoice {
    BeautyBandpass fundamentalMid, thirdL, thirdR, fourthL, fourthR;
    float have3=0.0f, have4=0.0f;
    float bloom=0.0f, onsetStep=0.0f, powerStep=0.0f;
    float dryPower=0.0f, wetPower=0.0f;
    float guardGain=1.0f, guardStep=0.0f;
    double lastHz=-1.0, lastSr=-1.0;

    void configure(double hz,double sr) noexcept {
        if (hz==lastHz && sr==lastSr) return;
        lastHz=hz;lastSr=sr;
        sr=std::max(sr,8000.0);
        const float f=std::max(20.0f,static_cast<float>(hz));
        const float maxHz=static_cast<float>(0.40*sr);
        fundamentalMid.set(std::min(f,maxHz),1.15f,sr);
        have3=(3.0*hz<0.40*sr)?1.0f:0.0f;
        have4=(4.0*hz<0.40*sr)?1.0f:0.0f;
        thirdL.set(std::min(3.0f*f,maxHz),1.40f,sr);
        thirdR.set(std::min(3.0f*f,maxHz),1.40f,sr);
        fourthL.set(std::min(4.0f*f,maxHz),1.55f,sr);
        fourthR.set(std::min(4.0f*f,maxHz),1.55f,sr);
        onsetStep=static_cast<float>(1.0-std::exp(-1.0/(0.038*sr)));
        powerStep=static_cast<float>(1.0-std::exp(-1.0/(0.060*sr)));
        guardStep=static_cast<float>(1.0-std::exp(-1.0/(0.022*sr)));
    }
    void reset() noexcept {
        fundamentalMid.reset();
        thirdL.reset(); thirdR.reset(); fourthL.reset(); fourthR.reset();
        bloom=dryPower=wetPower=0.0f;
        guardGain=1.0f;
    }
    float guard(float amount,float overlap) noexcept {
        if(amount<=0.0f) return 1.0f;
        // Smooth note-on/off overlap changes to avoid abrupt timbral steps.
        const float target=1.0f-0.50f*std::clamp(amount,0.0f,1.0f)*std::clamp(overlap,0.0f,1.0f);
        guardGain+=guardStep*(target-guardGain);
        return guardGain;
    }
    void tick(float& l,float& r,float presence,float texture) noexcept {
        if(presence<=0.0f && texture<=0.0f)return;
        const float dryL=l,dryR=r;
        bloom+=onsetStep*(1.0f-bloom);
        if(presence>0.0f) {
            const float center=fundamentalMid.tick(0.5f*(dryL+dryR));
            const float amount=0.10f*presence*bloom;
            l+=amount*center; r+=amount*center;
        }
        if(texture>0.0f) {
            const float amount=texture*(0.40f+0.60f*bloom);
            const float third=0.050f*amount*have3;
            const float fourth=0.028f*amount*have4;
            l+=third*thirdL.tick(dryL)+fourth*fourthL.tick(dryL);
            r+=third*thirdR.tick(dryR)+fourth*fourthR.tick(dryR);
        }
        // Slow, bounded RMS servo (not a perceptual loudness equalizer).
        const float pDry=0.5f*(dryL*dryL+dryR*dryR);
        const float pWet=0.5f*(l*l+r*r);
        dryPower+=powerStep*(pDry-dryPower);
        wetPower+=powerStep*(pWet-wetPower);
        const float ratio=std::sqrt((dryPower+1.0e-6f)/(wetPower+1.0e-6f));
        const float gain=std::clamp(ratio,0.88f,1.04f);
        l*=gain;r*=gain;
    }
};

// Side-only, band-limited delayed ambient cue; the dry Mid is algebraically
// invariant. This tests perceived front/back placement, does not guarantee it.
struct CrystalPrecisionSpace {
    static constexpr std::size_t size=2048;
    std::array<float,size> history{};
    float farReflection=0.0f, farAlpha=0.0f;
    std::size_t cursor=0, delay=1;
    void prepare(double sr) noexcept {
        constexpr double tau=6.2831853071795864769;
        sr=std::max(sr,8000.0);
        delay=std::clamp(static_cast<std::size_t>(sr*0.0043),std::size_t(1),size-1);
        farAlpha=static_cast<float>(1.0-std::exp(-tau*1900.0/sr));
        reset();
    }
    void reset() noexcept { history.fill(0.0f);cursor=0;farReflection=0.0f; }
    void tick(float& l,float& r,float amount) noexcept {
        if(amount<=0.0f)return;
        const float mid=0.5f*(l+r);
        const float side=0.5f*(l-r);
        const std::size_t read=(cursor+size-delay)%size;
        const float past=history[read];
        history[cursor]=side;
        cursor=(cursor+1)%size;
        farReflection+=farAlpha*(past-farReflection);
        // Quiet early reflection; direct sound dominates, no delayed dry path.
        const float nextSide=side+0.066f*amount*farReflection;
        l=mid+nextSide;r=mid-nextSide;
    }
};
} // namespace sylenth2
