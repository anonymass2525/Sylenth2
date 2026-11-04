#pragma once
// Experimental Beauty engines. They do not alter the legacy 0.1 oscillator/filter path.
#include <algorithm>
#include <array>
#include <cmath>

namespace sylenth2 {

enum class EngineMode : int { Original = 0, Harmonic = 1, Vocal = 2, Fusion = 3, CrystalClarity = 4, CrystalBody = 5 };

// A separate, strictly band-limited 20-partial oscillator for the Harmonic mode.
// We use a complex harmonic recurrence: just one sin/cos pair per output sample.
struct HarmonicVoice {
    static constexpr int partials = 20;
    double phase = 0.0, step = 0.0;
    std::array<float, partials> weights{};
    float normalization = 0.0f;
    double cachedFrequency = -1.0, cachedRate = -1.0;
    float cachedColor = -1.0f;

    void configure(double frequency, double sampleRate, float color) noexcept {
        if (frequency == cachedFrequency && sampleRate == cachedRate && color == cachedColor) return;
        cachedFrequency = frequency; cachedRate = sampleRate; cachedColor = color;
        const double hz = std::max(1.0, frequency);
        step = std::min(0.48, hz / sampleRate);
        float sumSquares = 0.0f;
        for (int i = 0; i < partials; ++i) {
            const float harmonic = static_cast<float>(i + 1);
            // Smooth spectral slope plus a soft, non-physical 4th-partial glow.
            const float exponent = 1.12f + 0.62f * (1.0f - color);
            const float peak = (harmonic - 4.0f) / 2.2f;
            const float value = std::pow(harmonic, -exponent)
                * (1.0f + 0.45f * color * std::exp(-0.5f * peak * peak));
            weights[static_cast<std::size_t>(i)] =
                (hz * harmonic < sampleRate * 0.46) ? value : 0.0f;
            sumSquares += weights[static_cast<std::size_t>(i)] * weights[static_cast<std::size_t>(i)];
        }
        normalization = 0.98f / std::sqrt(std::max(sumSquares, 1.0e-8f));
    }

    float tick(float envelope) noexcept {
        constexpr double twoPi = 6.283185307179586476925286766559;
        const double angle = twoPi * phase;
        const double c = std::cos(angle), s = std::sin(angle);
        double re = c, im = s, sum = 0.0;
        // The high-partial envelope blooms briefly at note onset.
        for (int i = 0; i < partials; ++i) {
            const double tilt = 0.70 + 0.30 * envelope * (static_cast<double>(i) / (partials - 1));
            sum += weights[static_cast<std::size_t>(i)] * tilt * im;
            const double nextRe = re * c - im * s;
            im = im * c + re * s;
            re = nextRe;
        }
        phase += step;
        if (phase >= 1.0) phase -= 1.0;
        return static_cast<float>(sum) * normalization;
    }
};

struct BeautyBandpass {
    float b0 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;
    void set(float frequency, float q, double sampleRate) noexcept {
        constexpr float pi = 3.14159265358979323846f;
        const float omega = 2.0f * pi * std::clamp(frequency, 30.0f, float(sampleRate * 0.43)) / float(sampleRate);
        const float alpha = std::sin(omega) / (2.0f * q);
        const float inv = 1.0f / (1.0f + alpha);
        b0 = alpha * inv;
        b2 = -b0;
        a1 = -2.0f * std::cos(omega) * inv;
        a2 = (1.0f - alpha) * inv;
    }
    float tick(float input) noexcept {
        const float out = b0 * input + z1;
        z1 = -a1 * out + z2;
        z2 = b2 * input - a2 * out;
        return out;
    }
    void reset() noexcept { z1 = z2 = 0.0f; }
};

struct VocalBeauty {
    std::array<BeautyBandpass, 3> bands{};
    void configure(int vowel, double sampleRate, float motion, float lfo) noexcept {
        constexpr float frequencies[5][3] = {
            {760.0f, 1200.0f, 2500.0f}, {530.0f, 1780.0f, 2600.0f},
            {310.0f, 2200.0f, 3000.0f}, {570.0f, 920.0f, 2400.0f},
            {370.0f, 1000.0f, 2200.0f}
        };
        const int index = std::clamp(vowel, 0, 4);
        for (int i = 0; i < 3; ++i) {
            const float drift = 1.0f + 0.028f * motion * std::sin(lfo + i * 0.9f);
            bands[static_cast<std::size_t>(i)].set(frequencies[index][i] * drift,
                                                    3.2f + 0.9f * i, sampleRate);
        }
    }
    float tick(float x) noexcept {
        return bands[0].tick(x) + 0.68f * bands[1].tick(x) + 0.34f * bands[2].tick(x);
    }
    void reset() noexcept { for (auto& band : bands) band.reset(); }
};
} // namespace sylenth2
