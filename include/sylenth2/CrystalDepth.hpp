#pragma once
// Crystal Depth 0.5 - four orthogonal research controls built on Crystal Body 0.4.
// The existing DSP remains untouched unless EngineMode::CrystalDepth is selected.
// All per-sample operations are allocation-free and use no lookahead.
#include "sylenth2/BeautyEngine.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace sylenth2 {

// Two note-tracked enhancements work at per-voice level, before summation.
struct CrystalDepthVoice {
    BeautyBandpass centerAnchor;
    BeautyBandpass upper3L, upper3R, upper4L, upper4R;
    float thirdEnabled = 1.0f, fourthEnabled = 1.0f;
    float bloom = 0.0f, bloomStep = 0.0f;
    double cachedHz = -1.0, cachedSr = -1.0;

    void configure(double hz, double sampleRate) noexcept {
        if(hz==cachedHz && sampleRate==cachedSr)return;
        cachedHz=hz; cachedSr=sampleRate;
        const double sr = std::max(8000.0, sampleRate);
        const float f = std::max(20.0f, static_cast<float>(hz));
        const float limit = static_cast<float>(0.40 * sr);
        centerAnchor.set(std::min(f, limit), 1.25f, sr);
        thirdEnabled = (3.0 * hz < 0.40 * sr) ? 1.0f : 0.0f;
        fourthEnabled = (4.0 * hz < 0.40 * sr) ? 1.0f : 0.0f;
        upper3L.set(std::min(3.0f*f, limit), 1.35f, sr);
        upper3R.set(std::min(3.0f*f, limit), 1.35f, sr);
        upper4L.set(std::min(4.0f*f, limit), 1.55f, sr);
        upper4R.set(std::min(4.0f*f, limit), 1.55f, sr);
        bloomStep = static_cast<float>(1.0 - std::exp(-1.0 / (0.045 * sr)));
    }
    void reset() noexcept {
        centerAnchor.reset();
        upper3L.reset(); upper3R.reset(); upper4L.reset(); upper4R.reset();
        bloom = 0.0f;
    }
    void tick(float& l, float& r, float coreFocus, float harmonicDensity) noexcept {
        if (coreFocus <= 0.0f && harmonicDensity <= 0.0f) return;
        bloom += bloomStep * (1.0f-bloom);
        const float inL=l, inR=r;
        if (coreFocus>0.0f) {
            // A phase-coherent anchor generated from the note's stereo mid.
            const float anchor=centerAnchor.tick(0.5f*(inL+inR));
            const float amount=0.075f*coreFocus*bloom;
            l += amount*anchor;
            r += amount*anchor;
        }
        if (harmonicDensity>0.0f) {
            // Higher, narrow-ish partial reinforcement; no new nonharmonic tones.
            const float scale=harmonicDensity*(0.45f+0.55f*bloom);
            const float third=0.070f*scale*thirdEnabled;
            const float fourth=0.038f*scale*fourthEnabled;
            l += third*upper3L.tick(inL)+fourth*upper4L.tick(inL);
            r += third*upper3R.tick(inR)+fourth*upper4R.tick(inR);
        }
    }
};

// Stereo experiment: strengthen only the upper Side signal and add a short
// Side-only reflection. Mid=(L+R)/2 stays unchanged (mono compatible).
struct CrystalStereoDepth {
    static constexpr std::size_t size = 1024;
    std::array<float,size> sideHistory{};
    float sideLow=0.0f, lowAlpha=0.0f;
    std::size_t cursor=0, reflectionDelay=1;

    void prepare(double sr) noexcept {
        constexpr double twoPi=6.2831853071795864769;
        sr=std::max(sr,8000.0);
        lowAlpha=static_cast<float>(1.0-std::exp(-twoPi*520.0/sr));
        reflectionDelay=std::clamp(static_cast<std::size_t>(sr*0.0018),std::size_t(1),size-1);
        reset();
    }
    void reset() noexcept {
        sideHistory.fill(0.0f); sideLow=0.0f; cursor=0;
    }
    void tick(float& l, float& r, float depth) noexcept {
        if(depth<=0.0f) return;
        const float mid=0.5f*(l+r), side=0.5f*(l-r);
        sideLow += lowAlpha*(side-sideLow);
        const float airySide=side-sideLow;
        const std::size_t past=(cursor+size-reflectionDelay)%size;
        const float reflection=sideHistory[past];
        sideHistory[cursor]=airySide;
        cursor=(cursor+1)%size;
        const float extended=side + depth*(0.105f*airySide+0.040f*reflection);
        l=mid+extended; r=mid-extended;
    }
};

inline float criticalBandErb(float f) noexcept {
    // ERB-number inspired bandwidth heuristic, NOT an auditory masking model.
    return 24.7f * (1.0f + 4.37f * f / 1000.0f);
}
inline float partialOverlap(float a, float b) noexcept {
    const float band=criticalBandErb(0.5f*(a+b));
    const float distance=std::abs(a-b)/std::max(25.0f,0.70f*band);
    const float d2=distance*distance;
    return 1.0f/(1.0f+d2*d2);
}

} // namespace sylenth2
