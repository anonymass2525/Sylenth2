# Sylenth2 v0.4 — Crystal Body DSP

ユーザー評価に基づく目標は「Originalの清らかさ＋Crystal Clarityの微細な厚み」を発展させること。既存5モードは変更せず、第6モードを追加する。

## 音響構造
- 各ノートの基音f0と第2倍音2f0を追従する、左右独立の広帯域バンドパスを各ボイスに2本配置。既存のVA/TPTローパス後から得た成分を原音へ少量加算。新しい倍音を生成する非線形処理は追加しない。
- Crystal Body Depth：基音補強量、Harmonic Density：第2倍音経路の強さ、Dynamic Transparency：アタック直後を弱くし32msで補強を開く。
- ナイキストに近い第2倍音経路は無効化し、サンプルレートとピッチベンドに追従。
- Crystal Bodyの後にv0.3のCrystal Clarityを適用し、ゼロ追加レイテンシーを意図する。ホスト報告遅延は別途確認。
- 3つの音響補強ゲインをゼロにするとOriginalと完全一致。Crystal Clarity設定だけ0にしてもBodyは残る。
- 既存のEngineインデックス0〜4およびパラメーターIDを保存、新モードをインデックス5へ追加。

## A/B評価
- --compare は8音色 × 6モードの48 WAV。
- --study はOriginal / Crystal Clarity / Clarity Only / Resonance Only / Crystal Body、8音色 × 5条件の40 WAV。
- デモは各音色ピーク-3dBFSに正規化。厳密な知覚音量一致ではない。
- Linux/GCC Release DSP CTest合格、旧5モード40WAVのSHA256一致を確認。Windows JUCEビルド・Studio One・聴感の優劣は個別検証が必要。
- Windows用Inno Setupはv0.3.1のrestartreplaceを継続。アップデート時はDAWを閉じる。
