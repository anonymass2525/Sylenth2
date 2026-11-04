#pragma once

#include "sylenth2/BeautyEngine.hpp"
#include "sylenth2/CrystalClarity.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sylenth2 {

enum class Wave : std::uint8_t { Saw, Pulse, Triangle, Sine };

struct Patch {
    const char* name = "Init";
    Wave waveA = Wave::Saw;
    Wave waveB = Wave::Saw;
    int unisonA = 3;
    int unisonB = 1;
    float detuneA = 10.0f;         // cents at the outside voices
    float detuneB = 5.0f;
    float stereo = 0.55f;          // 0...1, individual unison voice panning
    float levelA = 0.85f;
    float levelB = 0.30f;
    float octaveB = 0.0f;         // semitones
    float pulseWidth = 0.48f;
    float cutoffHz = 7000.0f;
    float resonance = 0.10f;      // 0...1
    float filterEnvOctaves = 1.0f;
    float keytrack = 0.30f;
    float drive = 0.0f;           // 0...1
    float attack = 0.010f;        // seconds
    float decay = 0.50f;
    float sustain = 0.75f;
    float release = 0.50f;
    float filterAttack = 0.003f;
    float filterDecay = 0.35f;
    float filterSustain = 0.00f;
    float filterRelease = 0.30f;
    float chorus = 0.0f;          // 0...1
    float delay = 0.0f;           // 0...1
    float delaySeconds = 0.34f;
    float delayFeedback = 0.27f;
    float formant = 0.0f;         // 0...1
    int vowel = 0;                // A,E,I,O,U
    float gain = 0.68f;
    EngineMode engineMode = EngineMode::Original; // never changes the eight 0.1 presets
    float harmonicColor = 0.65f; // 0...1, spectral slope + partial emphasis
    float harmonicMix = 0.75f;   // 0...1, old core -> harmonic oscillator
    float vocalBeauty = 0.65f;   // 0...1, new 3-formant resonance
    float vocalMotion = 0.40f;   // 0...1, gentle formant movement
    float crystalClarity = 0.72f; // 0...1, subtle high-band transient polish
    float crystalResonance = 0.38f; // 0...1, strictly parallel gentle resonance
};

const std::array<Patch, 8>& factoryPatches();

// Render does not allocate. Call prepare() before processing and never on the audio thread.
class Synth {
public:
    Synth();
    void prepare(double sampleRate);
    void reset();
    void setPatch(const Patch& patch) noexcept;
    void noteOn(int midiNote, float velocity) noexcept;
    void noteOff(int midiNote) noexcept;
    void sustainPedal(bool down) noexcept;
    void pitchBend(float normalized) noexcept; // -1...+1, +/- 2 semitones
    void allNotesOff() noexcept;
    void render(float* left, float* right, std::size_t numFrames) noexcept;
    double sampleRate() const noexcept { return sr_; }

private:
    static constexpr int maxVoices = 12;
    static constexpr int maxUnison = 7;
    static constexpr int chorusSize = 8192;

    enum class EnvStage : std::uint8_t { Off, Attack, Decay, Sustain, Release };
    struct Envelope {
        EnvStage stage = EnvStage::Off;
        float level = 0.0f;
        float a = 0.01f, d = 0.01f, s = 0.5f, r = 0.01f;
        void configure(float attack, float decay, float sustain, float release, double sr) noexcept;
        void on() noexcept;
        void off() noexcept;
        float tick() noexcept;
        bool active() const noexcept { return stage != EnvStage::Off; }
    };
    struct Filter {
        float ic1 = 0.0f, ic2 = 0.0f;
        float g = 0.1f, k = 1.0f, h = 1.0f;
        void coefficients(float hz, float resonance, double sr) noexcept;
        float tick(float input) noexcept;
    };
    struct Lane {
        double phase = 0.0;
        double step = 0.0;
        float panL = 1.0f, panR = 1.0f;
    };
    struct Voice {
        int note = -1;
        float velocity = 0.0f;
        std::uint64_t age = 0;
        bool heldByPedal = false;
        Envelope amp, filterEnv;
        Filter filterL, filterR;
        std::array<Lane, maxUnison> a{}, b{};
        HarmonicVoice harmonic;
        int filterCounter = 0;
    };
    struct Bandpass {
        float b0=0, b1=0, b2=0, a1=0, a2=0, z1=0, z2=0;
        void set(float frequency, float q, double sr) noexcept;
        float tick(float x) noexcept;
        void clear() noexcept { z1=z2=0; }
    };

    void tune(Voice& voice) noexcept;
    static float waveSample(Wave wave, double phase, double step, float pulseWidth) noexcept;
    static float polyBlep(double t, double dt) noexcept;
    float chorusRead(const std::array<float, chorusSize>& buf, float delay) const noexcept;
    static float clampf(float v, float lo, float hi) noexcept;

    double sr_ = 48000.0;
    Patch patch_{};
    std::array<Voice, maxVoices> voices_{};
    std::uint64_t age_ = 0;
    bool pedal_ = false;
    float bend_ = 0.0f;
    std::array<float, chorusSize> chorusL_{}, chorusR_{};
    int chorusPos_ = 0;
    double chorusPhase_ = 0;
    std::vector<float> delayL_, delayR_;
    std::size_t delayPos_ = 0;
    std::array<Bandpass, 2> vowelL_{}, vowelR_{};
    float lastVowelKey_ = -1.0f;
    VocalBeauty beautyL_, beautyR_;
    CrystalClarity crystalL_, crystalR_;
    float beautyPhase_ = 0.0f;
    unsigned beautyCounter_ = 0;
};
} // namespace sylenth2
