# Crystal — Electronic Beauty Synthesizer (typeGPT research edition)

**Crystal v0.4.1 — identity transition (DSP unchanged from v0.4.0)**

An independently written virtual-analog synthesizer experiment. **Sound signature: transparent, pure, clear, and harmonically full without becoming muddy.** This repository does **not** incorporate the previous Sylenth2 prototype, Sylenth1 code, or Panoramize source. Its current focus is a clear, crystalline electronic timbre with strong oscillator character.

> **Research name.** Crystal is also the name of the established Green Oak Software synthesizer. This independent research project is not affiliated with Green Oak, and uses the distinguishing development label **Crystal typeGPT** on the VST3 and Windows installer. Review branding and trademark clearance before public/commercial release. Historic code namespace and GitHub repository slug remain `sylenth2` to preserve source history.

## Beauty engine modes (v0.2)

The original v0.1 engine and its **eight unmodified factory patches** remain the default.
The generic JUCE editor adds an Engine choice: **Original / Harmonic / Vocal / Fusion**.
Harmonic adds a separate 20-partial Nyquist-limited oscillator with spectral slope/glow and attack-dependent brightness.
Vocal adds a separate three-band moving formant network. Fusion combines both.
These are experimental algorithms, not claims of universally better sound; compare with the Original at matched levels.
`./build/sylenth2_render demos-beauty --compare` renders all four engines for the same eight factory patches.

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

Install Visual Studio 2022 with C++ Desktop Development, CMake and Git. Network access to fetch JUCE 8.0.17-security is required on first CMake configure.

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

## v0.3 Crystal Clarity

New independent fifth mode for conservative high-frequency polish and subtle parallel resonance. The Original path and engine indices 0–3 are unchanged. Crystal Clarity and Transparent Resonance controls default to modest values, and both set to zero yields exact Original output. See [Crystal Clarity design (Japanese)](docs/CRYSTAL_CLARITY_JA.md). Use `sylenth2_render demos-compare --compare` to render 40 comparison WAVs.

## v0.3.1 Installer-only hotfix

Same Crystal Clarity audio DSP as v0.3.0. Inno Setup now uses restartreplace for VST3 files locked by Studio One or other hosts, prompting for a Windows restart if required. See [Japanese update troubleshooting](docs/INSTALL_TROUBLESHOOTING_JA.md). Always close the DAW before updating. This does not bypass permissions or security software.

## v0.4 Crystal Body

Adds a sixth selectable engine with note-tracked fundamental/second-partial parallel reinforcement and attack transient protection, followed by the unchanged v0.3 Crystal Clarity. Earlier five engine modes, IDs, and audio behavior are preserved. New controls: Crystal Body Depth, Harmonic Density, Dynamic Transparency. Render all 6 modes using `sylenth2_render demos-compare --compare` or the focused five-way ablation with `sylenth2_render demos-study --study`. See [v0.4 Japanese DSP notes](docs/CRYSTAL_BODY_JA.md). Installer retains v0.3.1 locked-file `restartreplace` behavior.

## Crystal v0.4.1 — naming transition

The sound engine remains **bit-equivalent to Sylenth2 v0.4.0**. The physical VST3 bundle is now `Crystal typeGPT.vst3` while the DAW-visible JUCE `PLUGIN_NAME` is `Crystal typeGPT 0.4.1`. A new stable plugin identifier and separate installer AppId prevent accidental replacement of the historic `Sylenth2 Experimental.vst3`. Installer upgrades starting from this version are intended to replace `Crystal typeGPT.vst3` in place. The default DAW Engine selection is the user-preferred **Crystal Body**; all other modes remain selectable. On first install, the old Sylenth2 experimental VST3 remains installed and can be uninstalled separately after verification. The physical project name and plugin metadata were changed; the internal `sylenth2` namespace is retained temporarily. More: [Vision & migration (Japanese)](docs/CRYSTAL_VISION_JA.md).
