# Sylenth2 Experimental v0.1 — 開発と音響設計

## 独立開発
旧Sylenth2 Concept v0.1、Panoramize、Sylenth1の実装を一切取り込まず、独立したC++17 DSPコアから再開する。JUCEはDAW用ホスト層のみ。汎用合成方式の網羅は目指さず、透明感・シャープな倍音・滑らかなユニゾンに特化。

## Signal path
MIDI → Voice allocator (12 voices) → Oscillator A/B (1–7 lanes each) → Per-voice TPT lowpass (L/R) → Amplitude envelope → Stereo sum → Optional formants → Chorus → Cross-feedback delay → Output

SawとPulseはpolyBLEP。各レーンで位相・デチューン・パンを管理。位相はノート番号から決定論的に初期化し、毎回のWAV比較で再現性を持たせる。共振フィルターはTPT SVF方式でコエフィシェント更新は16サンプル単位（滑らかさとCPU負荷の暫定折衷）。

## Factory sounds
1. Silver Halo — 澄んだ幅広VAスーパーソー
2. Prism Pluck — 鋭い高域と速いフィルター減衰を持つプラック
3. Blue Current — パルス／オクターブ下オシレーター、芯のあるベース
4. Midnight Vowel — 控えめにフォルマントを加えた電子リード
5. Afterimage — 長いアタックと余韻を持つ淡いパッド
6. Crystal Edge — 立ち上がりが鮮明なVAリード
7. Satin Pulse — パルス主体の柔らかい音
8. Arctic Dawn — 正弦波を軸にした浮遊感のあるパッド

## 次の研究課題
- 48/96/192kHzを横断する定量的な折返しノイズ検査（drive・pulse・triangleを含む）
- フィルター係数を音声レート変調しても破綻しない構造、ゼロディレイフィードバックと非線形の検証
- 同音反復・ボイススティール・ステレオ定位・サンプル正確なパラメーター変更
- GUI独自設計、プリセットファイル互換性、内部品質設定、pluginval、Studio Oneでの実機評価
- Sylenth1系と音量を揃えたブラインド比較。主観と技術測定を混同しない

## 検証レベル
1. DSPビルド + CTest
2. Windows VST3ビルド
3. pluginval strictness 5
4. Studio One実機動作
5. 試聴比較 + 定量的音質評価

各段階は独立した合格条件とする。READMEの署名や自動CIが存在することだけでは全段階の成功は証明されない。
