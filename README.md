# Sylenth2 Experimental — from-scratch VA synthesizer

**Prototype 0.1 / 2026-11-04**

An independently written virtual-analog synthesizer experiment. This repository does **not** incorporate the previous Sylenth2 prototype, Sylenth1 code, or Panoramize source. Its current focus is a clear, crystalline electronic timbre with strong oscillator character.

> **Unofficial project / working title.** Not affiliated with, endorsed by, or a successor to LennarDigital / Sylenth1. Consider renaming before distributing a commercial product; the working title may create trademark confusion.

## What's implemented

- Independent portable C++17 DSP core; JUCE-based Windows x64 VST3 target.
- Two oscillators per note: polyBLEP anti-aliased saw/pulse, triangle and sine.
- Up to seven independently tuned and panned unison lanes per oscillator; 12-note polyphony.
- TPT state-variable lowpass filter, filter envelope, optional soft drive.
- Amp/filter ADSR, pitch bend, MIDI note on/off, sustain pedal.
- Stereo chorus, feedback delay, optional two-formant vocal coloring.
- Eight **original** factory patches and an offline WAV renderer (48 kHz, stereo, PCM 16-bit).
- Generic JUCE editor with Preset, Brightness, Motion, Unison Width, and Drive macros; preset state serialization.
- Offline DSP regression tests for sample-rate stability and finite output.

## Build DSP and demos (no JUCE dependency)

```bash
cmake -S . -B build -DSYLENTH2_BUILD_PLUGIN=OFF
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
./build/sylenth2_render demos
```

## Windows VST3 build

Install Visual Studio 2022 with C++ Desktop Development, CMake and Git. Network access to fetch JUCE 8.0.10 is required on first CMake configure.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DSYLENTH2_BUILD_PLUGIN=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
.\build\Release\sylenth2_render.exe .\demos
```

The main-branch GitHub Actions workflow also attempts a Windows x64 Release VST3 build and uploads VST3 + eight WAV demonstrations (90-day retention).

## Audio intent and limitations

The eight sounds are new patches created for this engine: Silver Halo, Prism Pluck, Blue Current, Midnight Vowel, Afterimage, Crystal Edge, Satin Pulse and Arctic Dawn.

**Important:** a passing DSP test is not proof of VST3 loading, pluginval compliance or Studio One operation. PolyBLEP reduces discontinuity aliasing, but triangle and non-linear drive are not yet exhaustively bandlimited; rigorous spectral analysis, oversampling choices, host testing and UI refinement remain pending. Current MIDI/polyphony is intended for experimentation and is not yet a production instrument. The code is provided as original research, not as a claim of a fundamentally novel synthesis algorithm.

Documentation: [Architecture (Japanese)](docs/ARCHITECTURE_JA.md).