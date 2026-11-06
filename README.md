# Crystal — Electronic Beauty Synthesizer (typeGPT research edition)

**Prototype 0.4 / 2026-11-04**

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

A fifth **Crystal Clarity** mode extends the original DSP with conservative time-adaptive high-frequency polishing and a parallel transparent resonance layer. The Original 0.1 path and the first four mode indices remain unchanged. The two new controls default to modest levels and bypass bit-identically when both are zero. For methodology and limits, see [Crystal Clarity (Japanese)](docs/CRYSTAL_CLARITY_JA.md).

Render five modes using `sylenth2_render demos-compare --compare` (40 WAV files total). Windows CI builds an EXE installer for in-place VST3 upgrades.

## v0.4 Crystal Body (research mode)

Adds a **sixth**, selectable `Crystal Body` engine with note-tracked low-partial reinforcement (fundamental and second harmonic), time-smoothed attack protection and the unchanged v0.3 Crystal Clarity stage. Original/Harmonic/Vocal/Fusion/CrystalClarity and their parameter IDs/engine indices remain unchanged. New parameters: Crystal Body Depth, Harmonic Density and Dynamic Transparency. `--study` generates 5 matched-MIDI ablation conditions per preset; `--compare` generates all six engine modes. See [v0.4 Japanese notes](docs/CRYSTAL_BODY_JA.md). CI generates Windows x64 VST3 and in-place upgrade EXE.

## Crystal v0.4.1 — naming transition

The sound engine remains **bit-equivalent to Sylenth2 v0.4.0**. The physical VST3 bundle is now `Crystal typeGPT.vst3` while the DAW-visible JUCE `PLUGIN_NAME` is `Crystal typeGPT 0.4.1`. A new stable plugin identifier and separate installer AppId prevent accidental replacement of the historic `Sylenth2 Experimental.vst3`. Installer upgrades starting from this version are intended to replace `Crystal typeGPT.vst3` in place. The default DAW Engine selection is the user-preferred **Crystal Body**; all other modes remain selectable. On first install, the old Sylenth2 experimental VST3 remains installed and can be uninstalled separately after verification. The physical project name and plugin metadata were changed; the internal `sylenth2` namespace is retained temporarily. More: [Vision & migration (Japanese)](docs/CRYSTAL_VISION_JA.md).

## Crystal Depth v0.5.0 - Four independent audio research axes

Seventh, entirely optional Engine mode: **Crystal Depth**. The default remains the previously preferred **Crystal Body**; all six old mode indices are preserved. Core Focus, Harmonic Texture, Stereo Depth and Resonance Guard have independent controls. `--depth-study` renders Body 0.4 versus four single-axis ablations plus Depth All for each factory preset (48 WAVs). `--depth-raw` renders the same WAVs without per-file peak normalization for objective measurements. See [Crystal Depth methodology (Japanese)](docs/CRYSTAL_DEPTH_JA.md). Crystal version 0.5.0 is a research prototype, not yet a proof of improved perceived quality.

## Crystal v0.6.0 — Crystal Precision (experimental, selectable)

Preserves all previous 7 engine modes and all previous parameter IDs. New independent DSP controls: Core Presence (note-tracked mid anchor with bounded RMS compensation), Partial Clarity (subtle third/fourth partial shaping), Front-Back Depth (filtered early Side reflection preserving the dry mono Mid), and Chord De-Masking (smoothed chord-overlap resonance protection). Initial Engine selection remains Crystal Body; all four new controls zero in Crystal Precision is bit-exact Crystal Body. Offline renderer supports `--precision-study`, `--precision-raw`, and `--precision-chords`. See [design and limitations (Japanese)](docs/CRYSTAL_PRECISION_JA.md).
