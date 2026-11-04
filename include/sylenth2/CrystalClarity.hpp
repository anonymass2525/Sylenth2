#pragma once
// Crystal Clarity 0.3: conservative spectral polish on the original signal.
// Purely optional, zero-latency and independently switchable. No allocation per sample.
#include "sylenth2/BeautyEngine.hpp"
#include <algorithm>
#include <cmath>

namespace sylenth2 {

struct CrystalClarity {
    float low = 0.0f, fast = 0.0f, slow = 0.0f;
    float lpAlpha = 0.0f, fastAlpha = 0.0f, slowAlpha = 0.0f;
    BeautyBandpass gentleBody, airyResonance;

    void prepare(double sampleRate) noexcept {
        constexpr double pi = 3.14159265358979323846;
        const double sr = std::max(8000.0, sampleRate);
        lpAlpha = static_cast<float>(1.0 - std::exp(-2.0 * pi * 3400.0 / sr));
        fastAlpha = static_cast<float>(1.0 - std::exp(-1.0 / (0.003 * sr)));
        slowAlpha = static_cast<float>(1.0 - std::exp(-1.0 / (0.055 * sr)));
        gentleBody.set(920.0f, 1.7f, sr);
        airyResonance.set(2750.0f, 2.0f, sr);
        reset();
    }
    void reset() noexcept {
        low=fast=slow=0.0f;
        gentleBody.reset(); airyResonance.reset();
    }
    float tick(float x, float clarity, float resonance) noexcept {
        if (clarity<=0.0f && resonance<=0.0f) return x;
        const float input=x;
        if (clarity>0.0f) {
            low += lpAlpha * (x-low);
            const float high=x-low;
            fast += fastAlpha * (std::abs(high)-fast);
            slow += slowAlpha * (std::abs(high)-slow);
            const float excess=std::clamp((fast-slow)/(slow+0.035f),0.0f,1.0f);
            const float shine=0.045f*clarity;
            const float restraint=0.100f*clarity*excess;
            x += high*(shine-restraint);
        }
        if (resonance>0.0f) {
            const float body=gentleBody.tick(input);
            const float air=airyResonance.tick(input);
            x += resonance*(0.11f*body+0.080f*air);
        }
        return x;
    }
};

} // namespace sylenth2
