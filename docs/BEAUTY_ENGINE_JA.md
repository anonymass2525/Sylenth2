# Beauty Engine Prototype 0.2

- Original: v0.1 signal path and factory presets are unchanged. Default host parameter is Original.
- Harmonic: 20-partial independent additive oscillator; dynamic upper-partial energy controlled by filter envelope; Nyquist guard at 0.46*sampleRate. Mixed with classic oscillator before filter/drive.
- Vocal: separate three-formant network with gently time-varying formant frequencies. Original two-formant stage is bypassed only in Vocal/Fusion.
- Fusion: both new systems active.

This is a research preview, NOT a guarantee of better perceived beauty. Especially compare level-matched WAVs.
No parameter-memory migration for unknown older host versions is implied; new parameters default to Original when absent.

## Build and test

```bash
cmake -S . -B build -DSYLENTH2_BUILD_PLUGIN=OFF
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/sylenth2_render demos-beauty --compare
```

Expected: 32 WAVs, 8 sounds x 4 engine modes. The plain `sylenth2_render demos` still generates the original 8 WAVs without any changes.

## Caveats

- Switching modes during a sustained note is not crossfaded in this prototype; an audible step is possible.
- Nonlinear drive is still not oversampled.
- Subjective beauty and perceived loudness require listening tests; CTest proves neither.
- Formant frequencies update every 128 samples to reduce CPU load.
