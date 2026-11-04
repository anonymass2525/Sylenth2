# Sylenth2 インストール／アップデートについて

## v0.3.1 インストーラー修正
DSPはv0.3.0から変更なし。Windows x64 VST3のインストーラーで、旧バージョンのバイナリが使用中の場合に restartreplace を使用して **Windows再起動後の置換を予約** できるようにした。

- Studio One、他のDAW、VST3スキャナー、プラグインテスターを終了してからインストール。
- インストーラーは管理者権限で実行。
- アップデート中に再起動を要求されたら、DAWを再起動する前にWindowsを再起動。
- Skip this file は旧DLLが残るため選択しない。問題が継続する場合、キャンセルしPCを再起動してから再実行する。
- 再起動後も同じアクセス拒否になる場合、ファイルのACL／セキュリティソフト／別プロセスからのロックなどの調査が必要。
- restartreplace はアクセス権限不足のすべての原因を解決するものではない。

標準インストール先:
C:\Program Files\Common Files\VST3\Sylenth2 Experimental.vst3

Inno Setup公式仕様: https://jrsoftware.org/ishelp/topic_filessection.htm
