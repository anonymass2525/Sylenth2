# Crystal v0.5 — Crystal Depth（独立4要素の音響研究）

## 研究目的

最も好評だった Crystal Body 0.4 を基準に、音の芯、倍音の密度、ステレオの奥行き、共鳴の重なりを分けて測定する。新しいEngineモードはインデックス6。旧6モード（0〜5）と既存パラメーターIDを変更しない。標準モードもCrystal Bodyのまま。

## 四つの独立処理

1. **Core Focus（芯）**：ノートの基音f0を追う幅広バンドパスから両ch共通の成分を最大7.5%だけ並列加算。約45msの立ち上がりで原音のアタックを保つ。センターの核を保つための仮説。
2. **Harmonic Texture（倍音密度）**：各ノートの第3・第4倍音を穏やかなバンドパスで抽出し、最大7%+3.8%の少量を並列加算。上限0.4 Fsで高域は無効。倍音の「数」を増やすのではなく既存倍音の分布を少し整える。
3. **Stereo Depth（ステレオ空間）**：M=(L+R)/2を維持し、S=(L-R)/2の520Hz以上の一部と約1.8msのSideのみの初期反射を少量加える。モノラル和を数学的に保ち、低域の不自然な拡散を避ける。DAWのchorus/delayはこの後段なので、FX ONでは最終モノラル和に差が出得る。「奥行き」の知覚は試聴で別途判断が必要。
4. **Resonance Guard（共鳴干渉）**：和音のノート間の基音/第2倍音周辺の帯域接近をERB風の距離で推定し、重なりに応じてCrystal Bodyの共鳴量を最大60%減少。ノートON/OFFとピッチベンドで更新。これは簡便な干渉ヒューリスティックであって厳密な聴覚マスキングモデルではない。単音では変化がない設計。

## 構成と制約

- 従来のOriginal → Crystal Clarity → Crystal Bodyを維持。DepthはBodyの後で Core/Harmonic を加え、クリアリティ処理後にSide処理を行う。
- 各パラメーターを0にすれば処理は独立バイパス。**全4つが0のDepthはCrystal Bodyとビット一致**することをテストする。
- 追加のルックアヘッドはなく、固定メモリのみでリアルタイム処理中に動的なメモリ確保はしない。JUCE VST3実機検証・pluginval・CPU計測は別途必要。
- `--depth-study`：Body04 / CoreOnly / HarmonicOnly / StereoOnly / OverlapGuardOnly / DepthAll の6種×8プリセット＝48 WAV。-3dBFS peak normalizationあり。
- `--depth-raw`：同一条件で正規化なし。分析では **音量を揃えたブラインド試聴** と元のraw振幅測定を分離する。
- 注意：デモのフレーズは単音が中心のため、OverlapGuardOnlyは同時に近接音程を弾く実験で効果を評価する。

## 今後の試験

- 48/96/192kHzの微小振幅、最大振幅、ハイノートを検証する。
- 低域のモノ互換、センター保持、ステレオ相関、True Peak、差分/RMS/倍音包絡を測定する。
- 1音・長三和音・隣接音・オクターブ・厚い和音の5種を基準MIDIに加える。
- 最終判定は、透明感・清らかさ・豊かな厚みを損なわないかという聴感評価で行う。

## 音響学の参考
- AES: Width and cross-correlation (2023) https://aes.org/publications/elibrary-page/?id=22093
- Simultaneous masking (critical bands) https://pubmed.ncbi.nlm.nih.gov/1479123/
- Simultaneous consonance, beating/masking https://pmc.ncbi.nlm.nih.gov/articles/PMC7032667/
