# Sylenth2 v0.3 — Crystal Clarity
## 設計目的
ユーザー評価 Original > Vocal > Fusion > Harmonic と「透明感・清らかさ」を優先する。
Originalは変更しない。既存のMode 0〜3も残し、第5モードを追加する。

## DSP
Crystal Clarityは3400 Hzで分離した高域の短期3ms・長期55ms包絡を比較し、短い突出を最大約10%抑え、持続高域の明るさを最大約4.5%微調整する。
Transparent Resonanceは920 Hzと2750 Hzの広いバンドパス成分を低レベルで並列加算する。
ルックアヘッドやFFTは使わず、音声スレッドでメモリを確保しない。
両パラメーターが0の場合はOriginalとサンプル単位で同一。
VST3では旧パラメーターIDおよびEngine choiceのインデックスは維持。
この段階ではエイリアシングの削減効果は主張しない。

## 検証
同じMIDIを5モードでレンダリングし、OriginalとCrystalを聴き比べる。
比較レンダラーは-3dBFSピーク正規化であり知覚音量一致ではない。
DSPテストの合格、Windows CIビルド、Studio One実機、聴感の優劣を別々に確認する。
