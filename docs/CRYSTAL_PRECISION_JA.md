# Crystal v0.6 — Crystal Precision：4軸の独立検証

ユーザー評価: Crystal Depth v0.5は透明感・清らかさを維持したが、v0.4との音質差はごく微細。改良を強く誇張せず、独立操作と測定を優先する。

## 4つの実験DSP

1. **Core Presence**: 各ノート基音に追従するMid成分を約10%以下並列補強し、発音後38msかけて立ち上げる。60msのRMS追従に基づく限定ゲイン補正（0.88〜1.04倍）を使い、大幅な音量差を抑える。音量一致ではなく測定値で検証。
2. **Partial Clarity**: 第3・4倍音のバンドパス成分を5.0%/2.8%以下で並列補強。ナイキスト帯域付近は無効化し、倍音自体を新規生成しない。Coreと共通の緩やかなレベル補正を使う。
3. **Front-Back Depth**: Midを変えず、Sideのみへ低域通過1900Hz・約4.3msの微弱な反射音（最大6.6%）を加える。新規の遅延をドライ経路に入れない。左右の広がりと距離感は同じではなく、前後定位の成立は試聴で検証する。
4. **Chord De-Masking**: ノート間の基音／第2倍音の重なりヒューリスティックを再利用し、Body共鳴の補強分だけ最大50%低減。約22msの平滑化によりノート追加時の急激な音色変化を低減。聴覚マスキングの厳密モデルではない。

## 完全保存

既存Engineインデックス0〜6はそのまま。新モードだけインデックス7。Factoryプリセットは維持。Crystal Bodyの音は変更しない。追加4値をすべてゼロにしたPrecisionはBodyとサンプル単位で一致する設計。初期モードはCrystal Bodyのまま。

## 比較

- `--precision-study`: Body04 / Depth05 / CoreOnly / PartialOnly / SpaceOnly / GuardOnly / PrecisionAll、8音色×7条件=56 WAV。比較レンダーはピーク正規化。
- `--precision-raw`: 上記と同じ構成、正規化しない（分析向け）。
- `--precision-chords`: 近接3音・オクターブ＋5度・長三和音の3条件 × Body04 / PrecisionGuardOnly / PrecisionAll = 9 WAV。
- `ctest`: 全ゼロのビット一致、単独4処理の有効性、Mono Mid保持、単音Guard同値、44.1/48/96/192kHz。

## まだ証明できていないこと

RMS一致は知覚音量の完全一致ではない。前後定位、透明感、清らかさ、音の美しさの向上は実機での試聴評価が必要。JUCE VST3のホスト動作はGitHub Windows CIとは別にStudio Oneで確認する。
