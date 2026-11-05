# Crystal — 電子音の美の研究型シンセサイザー

## 1. 新しい名称と開発思想
ブランド名: **Crystal**（研究段階の配布識別名は **Crystal typeGPT**）。

中心となる3つの音響美: **透明感・清らかさ・豊かな厚み**。

これまでの試聴評価では、Original → Crystal Clarity → Crystal Body の順に少しずつ好ましい結果が得られた。客観的な優劣の証明ではなく、次の実験仮説を優先する理由である。

- 透明感: 原音の芯、倍音の知覚可能性、不要な粗さの少なさ。
- 清らかさ: 高域の過剰な突起・不要な非調和成分・変調ノイズを抑える。
- 豊かな厚み: 低次倍音の密度・時間変化する共鳴を増やしながら、和音の明瞭さを維持する。

## 2. 音響開発の規律
1. Original 0.1・Crystal Clarity 0.3・Crystal Body 0.4を独立して保持する。
2. Crystal Body 0.4を音色開発の現時点の基準とする。ただし古い音色を削除しない。
3. 単音・和音・高域・低域・アタック・持続・余韻を比較し、音量一致のABを重視。
4. 新技術は可能なら独立モードとして実装し、主観評価が悪化した場合は既存版へ戻せるようにする。
5. 低レイテンシー・リアルタイム安定性・DAW互換性も実験評価する。

## 3. v0.4.1 の範囲
- 音響DSPはv0.4.0から変更しない。既存音色の継承を優先する改名リリース。
- DAW表示名は Crystal typeGPT 0.4.1、VST3ファイル名は Crystal typeGPT.vst3。JUCEのPLUGIN_NAME/PRODUCT_NAMEで区別。
- 新しいVST3識別子を使用するため、Sylenth2 Experimental と共存できる。
- Crystalのインストーラーは新しい安定AppIdを使用し、今後のCrystal更新版は同じ場所へ上書きする。
- 初期エンジンはCrystal Body。Original/Crystal Clarityはメニューから選択可能。
- GitHubリポジトリ名・内部C++名前空間は履歴維持のため現状のまま。

## 4. 次のDSP研究案（v0.5）
- **Harmonic Depth**: 低次倍音の時間変化と各音高の共鳴を聴感音量補正込みで検証。
- **Stereo Depth**: Mid/Sideの低域整合とモノラル互換を維持しながら空間的な厚みを付加。
- **Resonance Interaction**: 複数ノートの共鳴の重なりが過剰に濁る条件を計測。
- **Transparency tests**: 0.4との差分を定量化してから採用する。機能追加のみで音質向上を主張しない。

## 5. 名称衝突への注意
Green Oak Softwareが別製品として古くから Crystal を提供している。2026年11月3日にはGreen Oak Crystal 2.6.0（VST3対応）が発表された。
現行のCrystal typeGPTは混同リスクを減らすための作業用識別名であり、商標等の使用可能性を保証しない。一般配布・商用化の前に名称を再検討する。

参考:
- https://www.greenoak.com/crystal/
- https://www.kvraudio.com/news/green-oak-updates-crystal-to-v2-6-0---a-classic-software-synth-renewed-69594
- https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
