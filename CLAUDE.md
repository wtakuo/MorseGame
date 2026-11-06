# Morse Game for M5Stack

M5Stack 用のモールス打鍵練習ゲーム。画面に出る文字・単語を GPIO に接続したモールスキーで打鍵する。制限時間内に正しく打てないとペナルティ 1（コード上は `lives--`）、5 回でゲームオーバー。2025-11-15 の横浜緑ヶ丘高校ホームカミングデー展示用に作成。

- コードはスケッチ直下のモジュールに分割（2026-11-06、`d34ef7e`）
  - [MorseGame.ino](MorseGame.ino): 各部品をつなぐだけ（`iter()` で 4ms ごとにキーを読み、解読してゲームを進める）
  - [Config.h](Config.h): キーピン、ループ周期
  - [MorseCode.h](MorseCode.h) / [.cpp](MorseCode.cpp): 符号表 `morseTable[]`（先頭 26 個が A-Z、続く 10 個が 0-9）と `morseLookup()`
  - [MorseDecoder.h](MorseDecoder.h) / [.cpp](MorseDecoder.cpp): `StraightKey`（デバウンス。パドルの各接点にも使う）、`SpeedTracker`（速度追従・短点/長点判定）、`IambicKeyer`（パドル用キーヤー、モード A/B）、`CharacterDecoder`（文字・語の組み立て）
  - [Game.h](Game.h) / [.cpp](Game.cpp): ゲーム状態・判定・スコア・レベル。難易度で変える値は `GameRules`。画面への変更通知は `takeChanges()` のビットフラグ
  - [Words.h](Words.h) / [.cpp](Words.cpp): 出題単語、`pickChallenge(level)`
  - [Settings.h](Settings.h) / [.cpp](Settings.cpp): 設定値 `Settings`、難易度ごとの `rulesFor()`、NVS（`Preferences`、名前空間 `morsegame`）への保存とハイスコア（`Storage`）
  - [SettingsMenu.h](SettingsMenu.h) / [.cpp](SettingsMenu.cpp): 設定画面（左=次の項目、中=-、右=+。CoreS3 では開いている間だけ `M5.setTouchButtonHeight(36)` で画面下端をボタンにする）
  - [Screen.h](Screen.h) / [.cpp](Screen.cpp): ゲーム画面のレイアウトと描画。M5GFX は文字サイズがフォント切り替え後も残るので、描画ごとに `setTextSize` を明示する
  - [Sound.h](Sound.h) / [.cpp](Sound.cpp): 側音と効果音
  - MorseCode と MorseDecoder は Arduino 非依存（時刻は引数で渡す）。PC でもビルドでき、`tools/decoder_host.cpp` からシミュレータで検証する。ここに Arduino 依存を持ち込まない
- arduino-cli + Arduino (esp32:esp32 2.0.17) + M5Unified。2026-11-04 に PlatformIO から移行（`platformio.ini` / `include` / `lib` / `test` は削除）
- 対象機種: プロファイル `cores3`（M5Stack CoreS3、既定、キーは Grove Port B の GPIO 8、DAH パドルは GPIO 9）、`core2`（Core2、Grove Port A の GPIO 33、DAH は GPIO 32）。キーピン `KEY_PIN` / `KEY_PIN2` はボードマクロ（`ARDUINO_M5STACK_Core2` など）で自動選択。ストレートキーと DIT パドルは `KEY_PIN`
- ボタン: 待機画面で左=設定、中=開始。プレイ中は中=中断、中断中は中=再開、左=終了。CoreS3 は M5Unified が画面下端のタッチを BtnA/B/C にしている
- ユーザーとの会話は日本語、コード中のコメントとコミットメッセージは英語

## ビルド・書き込み

設定は [sketch.yaml](sketch.yaml) のプロファイル（コア・ライブラリのバージョンを固定）。

成果物はプロファイルごとに `build/<profile>` に置く（`--build-path` を必ず付ける。`build/` は .gitignore 済み）。

```sh
arduino-cli compile --profile cores3 --build-path build/cores3 && arduino-cli compile --profile core2 --build-path build/core2   # 両機種ビルド
arduino-cli upload --profile core2 --build-path build/core2 -p <port>   # Core2 に書き込み
arduino-cli upload --profile cores3 --build-path build/cores3 -p <port>        # CoreS3 に書き込み
```

- `--build-path` を省くとスケッチ単位の共有キャッシュに出力され、別機種のビルド後に `upload` すると別チップのイメージを書こうとして失敗する（`Unexpected chip id in image`）
- 変更後は必ず両プロファイルでビルドを確認する
- 実機での動作確認はユーザーが行う。ビルドが通っても実機未確認なら、そう明記して報告する

## 環境上の注意（ハマりどころ）

- **ESP32 コアは `esp32:esp32 2.0.17` に固定している（PlatformIO 時代は `espressif32@6.12.0`）。** Arduino 3.x（3.3.12）では M5Unified の `spk_task` がスタック不足（`Stack canary watchpoint triggered (spk_task)`）で、最初に音を鳴らした瞬間にクラッシュする。M5Unified 0.2.11 / 0.2.28 どちらでも再現。スタックは `1280 + dma_buf_len*4` で、バッファは `dma_buf_len` 分 alloca するため設定で余裕を増やせない。6.12.0 では残り 328 バイト
- ライブラリは sketch.yaml で両機種とも M5Unified 0.2.28 / M5GFX 0.2.35 に固定（PlatformIO 時代の Core2 は M5Unified 0.2.13 だった）
- プロファイルでは 2.0.17 のボード ID はハイフン区切り（`m5stack-core2`, `m5stack-cores3`）。グローバルに入っている esp32 3.x はアンダースコア区切りで別物
- エディタの clangd が出す `'M5Unified.h' file not found` 等は誤検出（ビルドは通る）
- シリアルポート名は挿す USB ポートで変わる（CoreS3 は `<port>`、`<port>`、`<port>` など）。`arduino-cli board list` で確認する
- CoreS3 は USB Serial/JTAG。シリアルポートを開くとチップがリセットされる。ポートを開いたままのプロセスがあると書き込みに失敗する（`lsof <port>` で確認）
- 調査用のデバッグ出力は `#ifdef DEBUG_xxx` で囲み、`arduino-cli compile --profile <p> --build-property compiler.cpp.extra_flags=-DDEBUG_xxx` で有効化する（コミットはしない）。パニック時のバックトレースは `~/Library/Arduino15/packages/esp32/tools/` 以下のツールチェーンの `addr2line` で、`build/<profile>/MorseGame.ino.elf` に対して解決できる

## 作業の進め方

- 修正は 1 件ずつ小さくコミットする（末尾に `Co-Authored-By` 行）
- 打鍵タイミングに関わる変更は、実機を触る前に [tools/morse_sim.py](tools/morse_sim.py)（`python3 tools/morse_sim.py`）で比較してから入れる。過去にデバウンス修正で認識率を大きく落としたことがある
  - 人の打鍵（速度、長点/短点比、ばらつき、文字の最後の符号を長めに押す癖、チャタリング）を生成し、4ms 周期・10ms デバウンスで `iter()` と同じように解読する。解読方式はクラス（`Current` = `aaf1e06` より前の方式、`WindowMeans` = 現在の `SpeedTracker`）で差し替える
  - ファームウェアの定数を変えたらシミュレータ側も合わせる。`python3 tools/morse_sim.py --check-host` で、ファームウェアの `MorseDecoder.cpp` を PC でビルドし、`WindowMeans` と同じ解読結果になるか確認できる（800 例中 799 例一致。残る 1 例は float/double の丸め差）
  - 符号表はシミュレータも `MorseCode.cpp` から読み込む
  - 単語はお題ごとに「語間＋1.5 秒」空けて打つ想定。採点は単語列を対応付けて行う（位置で比べると分割・結合で以降がすべてずれる）

## 現在の状況（2026-11-06 時点）

ブランチ `main`。`fix/bugs` は 2026-11-04 に main へ fast-forward でマージ済み（ブランチは残してある）。リモートは未設定で push していない。

| コミット | 内容 |
|---|---|
| `d9e5f2c` | 1 文字 10 符号以上で `currentCharacter[9]` があふれるバグ |
| `0c45e4b` | 認識不能文字（`*`）が `inputWord` に入らず `C?AT` が `CAT` 扱いになるバグ |
| `750f15e` → `7e30f7c` で revert | デバウンス修正の初版（チャタリングを偽の短点として受け付け、認識率が激減したため取り消し） |
| `11b1b93` | デバウンスを「ピンが `minDuration`(10ms) 安定したら受理、時刻は安定し始めた時刻」方式に変更 |
| `ac27918` | タイムアウト時に `inputWord` / `wordCompleted` をクリア |
| `7dae329` | 側音（ch 0）と効果音（ch 1）を分け、2 音目は `stop_current_sound=false` でキュー |
| `2b09fc5` | ゲーム開始時にスコア・ライフを再描画 |
| `ac1406f` | platform を 6.12.0 に固定（PlatformIO 時代） |
| `77be2dc` | PlatformIO から arduino-cli へ移行。`src/main.cpp` → `MorseGame.ino`、`platformio.ini` → `sketch.yaml`、キーピンを `-DKEY_PIN` からボードマクロでの自動選択に変更、README にビルド手順を追加 |
| `bcf308c` | 成果物をプロファイルごとに `build/<profile>` に置く（`--build-path`） |
| `c81bc58` | プロファイル名 `core3` を FQBN に合わせて `cores3` に変更 |
| `e73fe5b` | 打鍵のたびにスコア欄を再描画していたのをやめる（`iter()` 内の `scoreChanged = true` を削除） |
| `cd17538` | 残り時間バーを `millis()` 基準で 200ms（`TIME_BAR_INTERVAL_MS`）ごとに更新。新しいお題では即座に満タンで描画、残り時間は 0 で下限 |
| `160ca8f` | `inputWord == challengeWord` になった時点で正解と判定（語間を待たない。誤答は従来どおり語間で判定）。早期判定やタイムアウトの後に来る空の語間は無視（以前はタイムアウト直後の語間で次のお題が誤答扱いになり得た） |
| `13d972d` | 打鍵シミュレータ `tools/morse_sim.py` を追加 |
| `aaf1e06` | 速度追従と短点・長点判定を変更（`classifyMark()`）。短点・長点の平均を別々に持ち、閾値はその中間。直近 8 打鍵に長短両方があれば 2 クラスタに分けて両平均を直接求める。短点 25〜300ms、長点/短点比 2〜4 に制限、1 秒超の長押しは無視、文字間は短点 2 個分、初期値は 80ms（約 15WPM、以前は 60ms）。`dotDuration` / `smoothingFactor` / `maxDashDuration` は削除 |
| `d8b9c11` | `words5` の重複（WORLD, LEVEL, ORDER）を削除 |
| `6bf6299` | 未使用の `DRAW_INTERVAL_MS` / `MAXINPUTTEXT` / `lastDrawTime` を削除 |
| `fded2f3` | `morsePlain` / `morseCode` の並行配列とマジックナンバー `45` をやめ、`morseTable[]`（`{text, code}`、code は `.`/`-` の文字列）に統合。打鍵中の符号も文字列で保持し `strcmp` で照合。レベル 1 は先頭 `NUM_LETTERS`(26) 個から出題 |
| `22876c7` | 画面の座標を `setDrawPosition()` に集約（フォントが固定サイズなので画面の端・中心基準）。時間バーをお題領域（y=70〜165）の外、y=168 に移し、プレイ中以外は消す。入力行の文字数もフォント幅から計算（320px で従来どおり 19） |
| `d34ef7e` | リファクタリング: モジュールに分割（動作は変えない） |
| `15267b7` | `tools/decoder_host.cpp` と `morse_sim.py --check-host` を追加 |
| `441fd8e` | `IambicKeyer` と `tools/keyer_test.cpp`（PC 上のテスト） |
| `31e9467` | 設定・難易度・ハイスコアと NVS 保存（`Settings`）、`KEY_PIN2` |
| `0e65cec` | 設定画面とパドル入力 |
| `d71da03` | ゲームの中断・再開・終了 |

CLAUDE.md もリポジトリで管理している（2026-11-05 に追加）。

### 実機での確認状況

2026-11-06 にユーザーが arduino-cli 版を Core2・CoreS3 の両実機で確認し、以下はいずれも問題なし:
- デバウンス新方式での認識精度（昨年のコードと同等以上）
- 効果音が 2 音順に鳴る
- 側音が押下から約 10ms 遅れても違和感はない
- Core2 実機での動作全般（M5Unified 0.2.28 / M5GFX 0.2.35）

`e73fe5b` と `cd17538` も 2026-11-06 にユーザーが両実機で確認済み（スコア・ライフ表示の更新、時間バーの減り方とお題切り替え時のリセット）。

`160ca8f` も 2026-11-06 にユーザーが両実機で確認済み。

`d8b9c11`〜`22876c7`（細かい点の修正）も 2026-11-06 にユーザーが実機で確認済み（画面の見た目、文字認識、ゲームオーバー・待機画面で時間バーが消えること）。

`d34ef7e`（モジュール分割）は 2026-11-06 にユーザーが両実機で確認済み（表示・解読・ゲーム進行とも従来どおり）。

`441fd8e`〜`d71da03`（機能追加）は両実機に書き込み済みだが、動作は未確認（パドル、設定画面、NVS 保存、ハイスコア、中断、レベル 1 の数字）。

`aaf1e06` は 2026-11-06 にユーザーが実機で試した。打ち始めは誤認識があったが比較的早く追随し、追随後は S/U などの誤認識が減った印象とのこと。もともとユーザーの打鍵で S が U、I が A と認識されることがあった問題への対応。シミュレーションの要点（単語正解率、打ち始め 2 単語 / その後）:
- S・I の誤読（5 単語後）: 長点が短め（比 2.3）の人で 78〜87% → 0〜2%、最後の短点を長く押す人で最大 17% → 2〜5%
- 遅い人（6〜10WPM）の打ち始め: 0〜10% → 40〜98%
- 速い人の打ち始めは少し下がる（初期値を 80ms にしたため。25WPM で 80〜100% → 72〜92%）。最後の短点を長く押す速い人のその後も 89〜92% → 86〜88%
- 初期値は 80ms のままでよいとユーザーが判断（2026-11-06）。参考: 60ms は速い人寄り、100ms は遅い人寄り（8WPM の打ち始め 42% / 55% / 87%、25WPM で 88% / 78% / 68%、それぞれ 60 / 80 / 100ms）PC シミュレーションでは解読結果は変更前と同一、最後に離してから正解判定まで 20WPM で約 350ms → 約 110ms。挙動の変化: お題 `CAT` に `CATS` と打つと、以前は誤答だったが今は `CAT` で正解になり、`S` は次のお題の入力になる。

## 次にやること（1〜6 は完了、6 は実機確認待ち）

1. ~~`fix/bugs` を main にマージ~~（2026-11-04 済み）
2. ~~M5Unified / M5GFX のバージョンを両環境でそろえて固定~~（sketch.yaml で 0.2.28 / 0.2.35 に固定、2026-11-06 に両実機で確認済み）
3. 設計上の問題
   - ~~速度の自動追従~~（`aaf1e06`、実機で効果を確認済み）。残る案: WPM 選択やキャリブレーション、ゲーム開始時の推定のリセット（展示で次の人に前の人の速度が残る。ただし待機画面で試し打ちすれば追従する）
   - ~~正解判定を語間まで待たず、一致した時点で判定する~~（`160ca8f`）
   - ~~打鍵のたびにスコア欄を再描画している~~（`e73fe5b`）
   - ~~残り時間バーの更新を `millis()` で行う~~（`cd17538`）
4. ~~細かい点~~（`d8b9c11`〜`22876c7`、実機確認済み）
5. ~~リファクタリング~~（`d34ef7e`、実機確認待ち）。効果音の非同期シーケンサは不要と判断（M5Unified が同じチャンネルの 2 音目をキューするので `twoNotes()` で足りる）
6. ~~機能追加~~（2026-11-06 に実装、実機確認待ち）。ユーザーが決めた仕様:
   - パドルは同じ Grove ポートのもう 1 本。既定はストレートキーのピンが DIT、新しいピンが DAH。設定で入れ替え可
   - キー種別は設定で STRAIGHT → IAMBIC-A → IAMBIC-B → STRAIGHT の順に切り替え。既定は STRAIGHT
   - WPM と難易度は独立。WPM はキーヤーの速度、ストレートキーでは追従の初期値（既定 15WPM = 80ms で従来と同じ）。難易度に速度は含めない
   - 難易度（既定は中級者）。値は以下のとおり提案どおり入れたもので、ユーザーが実機で調整する予定

     | 項目 | 初心者 | 中級者 | 上級者 |
     |---|---|---|---|
     | 制限時間（レベル 1〜5） | 8/12/16/20/24 秒 | 4/7/10/12/15 秒 | 3/5/7/9/11 秒 |
     | レベルアップに必要な正解数 | 5 | 10 | 10 |
     | ライフ | 5 | 5 | 3 |
     | レベル 1 の出題 | 英字のみ | 英字と数字 | 英字と数字 |
   - 設定画面: CoreS3 はタッチ、Core2 はボタン。リセット（設定をデフォルトに戻す。ハイスコアは消さない）
   - Iambic の実装: 反対側のパドルを要素中に押すとラッチ。B はラッチを使う（スクイーズを離すと反対の要素を 1 つ追加、短く叩いたパドルも記憶）、A は空白の終わりに押されているパドルだけを見る
   - 音量は 0〜10（スピーカー音量 = 値 × 255 / 10。既定 9 ≈ 229、以前は 220 固定）
