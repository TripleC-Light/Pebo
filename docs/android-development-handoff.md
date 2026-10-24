# Pebo Android 技術交接與實作回報

日期：2026-10-15（Asia/Taipei）。接收機：PC-01 / TripleC_AERO。
本文件是九項技術回報與交接摘要；最新工作歷史仍以 [handoff.md](handoff.md) 為準，
每機環境以 [development-machines.md](development-machines.md) 為準。

## 九項交接內容覆核

本次在來源電腦 LAPTOP-1JNDJFQA 重新核對：下列第 1～9 節均已具體回答
使用者要求，並與保存的程式及測試證據對照。23 個 source-manifest 檔案
SHA256 全部相符。PC-01 的 build-summary.json 與既有 JUnit 證據記錄
58 tests、0 failures/errors/skips；本次是證據審閱，沒有重新建置或執行實機測試。

閱讀順序與狀態差異：
- 最新安裝狀態以 canonical handoff 的 PC-01 安裝紀錄及本文件為準：
  Sony 已使用相容原簽章 APK 升級 v3 並啟動；完整 Sample UI／v3 RF 回歸未驗證。
- 原 ZIP、舊 PROTOCOL-v0.1.md／portable README 中的「v3 尚未安裝」是封存時狀態，
  不是最新狀態。原 ZIP 沒有包含後來的 PC-01 證據與本報告，不應拿它覆寫更新文件。
- 第 8 節是已明列的暫定假設，第 9 節是已識別但尚待定稿的問題；
  「已有交接答案」不代表這些 wire／firmware 決策已取得正式一致。
- 真實 application packet transport 仍 unbound；八種命令目前只有 Sample 可成功。
  既有 Block0 RF 實測成功不能替代 application protocol 整合成功。

## 目前結果與證據範圍

- 正式保存來源：`Software/PeboAndroid`。App ID `com.pebo.nfcprobe20261015`，
  versionCode **3**，versionName **0.1-protocol-sample**。
- Protocol/Sample 功能是來源電腦先前實作；本次接手未重寫 App 原始碼。
  本機核對 23 個來源/設定檔 SHA256 與大小，建置後來源 hash 仍相符。
- 本機 **58 項 unit tests 通過，0 failures/errors/skips**；Debug APK 建置與簽章驗證通過。
- Sony XQ-DQ72 已用原簽章 v3 APK 保留資料升級，`install -r` Success，啟動 Status:ok。
  完整 Sample UI 操作與 v3 RF 回歸尚未驗證；安裝啟動成功不等於這些功能已通過。
- 真實 Pebo application packet transport 尚未綁定；目前不具備真實裝置協定讀寫能力。

## 1. 新增與修改檔案

以下「新增／修改」依來源電腦交接記錄及目前來源核對；不把先前實作算成本次新增。

| 類別 | 相對於 Software/PeboAndroid 的檔案 | 用途 |
| --- | --- | --- |
| 先前新增 | `app/src/main/java/com/pebo/nfcprobe20261015/protocol/Wire.kt` | Packet、CRC、LE、command/status、parse error |
| 先前新增 | `app/src/main/java/com/pebo/nfcprobe20261015/protocol/Models.kt` | Typed models、config validation |
| 先前新增 | `app/src/main/java/com/pebo/nfcprobe20261015/protocol/PayloadCodec.kt` | 八種 command 的 request/reply payload |
| 先前新增 | `app/src/main/java/com/pebo/nfcprobe20261015/transport/PeboTransport.kt` | Packet transport interface、PeboClient、未綁定實機錯誤 |
| 先前新增 | `app/src/main/java/com/pebo/nfcprobe20261015/transport/SamplePeboTransport.kt` | 記憶體 Sample simulator |
| 先前新增 | `app/src/main/java/com/pebo/nfcprobe20261015/transport/St25NfcVBlockTransport.kt` | ISO15693 單區塊讀取 adapter |
| 先前新增 | `app/src/main/java/com/pebo/nfcprobe20261015/ProtocolActivity.kt` | Protocol/Sample UI |
| 先前新增 | `app/src/test/java/com/pebo/nfcprobe20261015/protocol/ProtocolTest.kt` | 38 項 protocol tests |
| 先前修改 | `app/src/main/java/com/pebo/nfcprobe20261015/MainActivity.kt` | 保留單區塊工程讀取，新增 Protocol 入口及 adapter 呼叫 |
| 先前修改 | `app/build.gradle`、`app/src/main/AndroidManifest.xml` | v3、JUnit 設定、內部 ProtocolActivity；維持 NFC permission |
| 先前新增／整理 | `PROTOCOL-v0.1.md`、`protocol-user-request-v0.1.txt`、`README.md`、`.gitignore`、`handoff/` | 規格、需求、跨機建置與歷史證據 |

既有工程檔案 `SingleBlock.kt`、`SingleBlockTest.kt`、`ReadGateTest.kt` 隨專案保存。
歷史 Activity 備份在 `handoff/historical-source/`，不納入編譯。

本次本機接手新增／修改：

- 新增 `Software/PeboAndroid/handoff/evidence/pc01-20261015-build01/`：
  本機 APK、SHA256、建置／簽章 log、JSON 摘要、JUnit XML、HTML report。
- 新增本技術交接文件 `docs/android-development-handoff.md`。
- 追加 `docs/handoff.md`、`docs/development-machines.md`：接收、環境、建置、安裝及本報告入口。
- 本機 TEMP 建置副本有 `local.properties`、Gradle cache/build outputs；未把 SDK、cache 或私鑰放進交接。
- 既有 `Software/PeboAndroid-handoff-20261015.zip` 與 sidecar 是來源電腦提供的原始快照，未覆寫；
  本次新證據與本報告不在該舊 ZIP 裡，下一台請同時取得最新工作目錄及 canonical handoff。

## 2. Protocol architecture

目前為單一 Android app module 裡的獨立 Kotlin packages，沒有拆成另一個 Gradle library module。
`protocol/` 不依賴 Android NFC API、ST register 或 tag block address。

```text
ProtocolActivity
  -> PeboClient
       -> PayloadCodec / PacketCodec / LE / CRC
       -> PeboPacketTransport.exchange(ByteArray): ByteArray
            -> SamplePeboTransport：可執行八種 command
            -> UnboundNfcVPacketTransport：明確失敗，不發送實體 packet

MainActivity + ReadGate / SingleBlock
  -> St25NfcVBlockTransport
       -> Android NfcV.transceive：僅工程用 Read Single Block
```

Typed models：DeviceInfo、DeviceStatus、PeboConfig、ReminderPeriod、DrinkRecord、HistoryPage。
`PeboCommand` 有八種命令，`PeboStatusCode` 有七種狀態。
喝水事件數與已完成提醒時段數是獨立欄位；Sample 示範 3 次事件／1 個完成時段。
Sample 並未實作 STM32 的喝水判定或提醒行為模型。

## 3. Packet encoder / decoder 架構

- `Wire.kt`：`PacketCodec` 處理 envelope，`LittleEndian` 與具邊界檢查的 Reader 處理整數，
  `Crc16CcittFalse` 處理應用層 CRC，`ParseResult` 回傳成功或具 ErrorKind 的明確錯誤。
- `PayloadCodec.kt`：依 command 編解碼 request/reply，驗證長度、尾隨資料、數值與 config/history 限制。
- `Models.kt`：資料模型與 ConfigValidation。沒有直接映射 C struct 或假設 padding。
- `PeboClient`：先編碼 request，再透過 transport 交換，核對 response command、方向、sequence、CRC，
  再解析 typed reply；History 額外核對回傳 startIndex 與 requestedCount。
- Client 同步序列化 execute；sequence 0..255 循環。目前沒有真實 transport 的 retry/session 去重機制。

| Offset | Bytes | 內容 |
| --- | --- | --- |
| 0 | 2 | `50 42`（PB） |
| 2 | 1 | Version `01` |
| 3 | 1 | Command；response = request OR `80` |
| 4 | 1 | Sequence |
| 5 | 1 | Payload Length |
| 6 | N | Payload |
| 6+N | 2 | CRC16，LE |

整包 8..256 bytes，payload 最大 **248 bytes**。UTC uint32 在 Kotlin 用 Long，支援超過 2038。
暫定 CRC CCITT-FALSE：poly1021/initFFFF/refin=false/refout=false/xorout0000；
涵蓋 header+payload，CRC 存 LE。`123456789` check=`29B1`。
獨立 golden PING：`50 42 01 08 00 00 7A F4`。
這個 CRC 與 Android NFC stack 處理的 ISO15693 RF CRC 分開。

## 4. NFC transport 架構

`PeboPacketTransport` 接受／回傳完整 application packet，不包含 ST register 或 ISO15693 framing。
Sample 透過相同 codec 路徑跑完整八種命令，不直接把 UI 填成成功。
真實 `UnboundNfcVPacketTransport` 會報出缺少 STM32 protocol、EEPROM/mailbox、commit/ACK/timeout 定義，
並且不傳送 application packet；Debug 顯示的 TX 是已編碼內容，不等於已上 RF。

獨立工程讀取路徑使用 Android ReaderMode NFC-V + SKIP_NDEF_CHECK：

- 只有按 Prepare 才消耗下一個 callback，發送一次 addressed Read Single Block。
- TX：`22 20 [Android Tag.id 原始順序的 8 bytes] [block]`，block 0..127；不自行附 RF CRC。
- 成功 RX 需為 `00` + 4 bytes；錯誤需為 `01` + error code；錯誤長度／flags 不當成成功。
- MainActivity 擁有 connect／worker／cancel／close lifecycle；ReadGate 防止重複與 stale completion。
- `St25NfcVBlockTransport` 只執行單區塊 transceive，沒有偷渡 EEPROM packet placement 或 mailbox 假設。

實際晶片 K/KC、板子供電與 RF 幾何條件仍不能由 UID 或歷史成功推定。

## 5. UI 修改內容

- 工程主畫面保留 NFC 偵測、Prepare 單區塊讀取與 Copy/Clear log，新增 Protocol/Sample 入口。
- Protocol 預設 SAMPLE，持續顯示來源與暫定格式提示；實際 NFC UID 與 Pebo Device ID 分開。
- Device Info：ID、HW/FW、protocol、capability flags。
- Status：mV、percent、狀態碼、時間、事件數、完成／總時段、Daily Target、LED、最後事件、flags。
- Config：讀取、編輯 target/enable/period ID/start/end/flags、新增／刪除最多 16 時段、驗證後 Write Config。
- History：index/count、下一頁、累計顯示、flags、Sample Clear History；85 筆示範 40/40/5 分頁。
- Time/PING：用手機 UTC 與當下 offset 組 SET_TIME；只有 Sample 會成功修改。
- Debug：完整 application TX/RX、command/status/CRC、實際 UID/technologies；離線 HEX 解析不改裝置面板。
- 切來源清除已解碼顯示；執行中停用操作，pause 以 generation 阻止 stale UI 更新並停 ReaderMode。

限制：Sample 僅記憶體保存，Activity/process 重建會重設；pause 不保證回滾已執行的 Sample 修改。
Config 編輯草稿可能保留於來源切換後，但不代表真實裝置配置。畫面操作尚待完整實機 smoke test。

## 6. Unit test 與建置結果

本機命令：`gradlew.bat testDebugUnitTest assembleDebug --no-daemon --console=plain`。
使用 JBR25.0.3 / Gradle9.3.1 / AGP9.1.1 / Kotlin2.2.10 / SDK37.0 / BT36.0.0，輸出 target17。

| Suite | Tests | Failures / Errors / Skips |
| --- | ---: | --- |
| ProtocolTest | 38 | 0 / 0 / 0 |
| SingleBlockTest | 14 | 0 / 0 / 0 |
| ReadGateTest | 6 | 0 / 0 / 0 |
| 合計 | **58** | **0 / 0 / 0** |

BUILD SUCCESSFUL，2m16s，40/40 tasks executed。包含 CRC golden／LE／封包邊界／sequence、config、
History、Sample 八命令、unbound transport、2000 組隨機 malformed buffers 等。
SDK XML、native-access、META-INF 警告未阻擋建置；未因此升級工具或改來源。
這是 JVM unit tests，不代表 Android lifecycle/UI instrumentation 或真實 RF/HIL 通過。

本機證據根目錄：`Software/PeboAndroid/handoff/evidence/pc01-20261015-build01/`。
HTML：`test-report/index.html`；XML：`junit-xml/`；摘要：`build-summary.json`。

| APK | SHA256 | 安裝狀態 |
| --- | --- | --- |
| 本機 `pc01-20261015-build01/pebo-v3-pc01-debug.apk`，916855 bytes | `C2129CD776F1E06CBDA9799F61A55809BB5BCB2E399A4A05EE612EC4AD98AD4C` | 簽章 v1/v2 PASS；未安裝 |
| 原交接 `pebo-v3-protocol-sample-debug.apk`，916853 bytes | `E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7` | 與舊 App 簽章相符；Sony 已升級 v3 並啟動 |

兩份都在 `handoff/evidence/` 下。兩機 debug key 不同；未轉移私鑰或解除安裝，亦未清除資料。
來源相同不代表 APK 二進位相同。`build-summary.json` 的 installed=false 指本機產物，與原簽章 APK 已安裝不衝突。

## 7. 仍受 STM32 firmware／整合定義限制的功能

目前無法對真實裝置執行八種 application commands：GET_DEVICE_INFO、GET_STATUS、SET_TIME、GET_CONFIG、
SET_CONFIG、GET_HISTORY、CLEAR_HISTORY、PING。Sample 成功不代表真實 MCU 成功。

待實作／確認：MCU packet server、資料產生、持久化、EEPROM/mailbox 配置與所有權、RF/I2C 原子交接、
wake/LPD/GPO 時序、ACK、freshness、timeout/retry、授權、斷電復原。
App 也仍需依最終協議實作實體 packet transport；完成 firmware 不會自動讓現有 unbound transport 可用。

真實 battery mV/百分比、當日事件與完成時段、RTC、歷史、配置讀寫、同步時間效果均尚未由 App 驗證。
不從被動 tag 成功讀取推論 MCU 即時更新；不從 MCU sleep current 推算六個月電池壽命。
STM32 行為與功耗變更仍須遵守 PA4/ST25_LPD deep sleep Output High 規則。

## 8. 實作採用的假設與暫定決策

以下依先前使用者接受的 provisional Sample scope 實作，仍待 firmware ratification：

1. CRC 取 CCITT-FALSE、涵蓋 header+payload、CRC LE；原要求「CCITT」本身不足以決定這些參數。
2. 所有 response 以 Status 開頭；非 OK 只有一個 Status byte；SET_TIME/SET_CONFIG/CLEAR_HISTORY/PING OK 為 status-only ACK。
3. PING request 為空。GET_HISTORY request 為 start:u16/count:u8；回應為 status/start:u16/total:u16/count:u8/records。
4. History 每頁最多 40 筆，允許尾端空頁，非尾端空頁拒絕；沒有 snapshot token。
5. 時段為半開區間 [start,end)，0<=start<end<=1440，邊界可相接；ID 唯一 0..15，不允許跨午夜。
6. 允許 zero periods 與 target=0；Reminder Enable 僅0/1；Reserved=0；record flags 上半 byte 拒絕。
7. BatteryStatus、CapabilityFlags、DeviceStatusFlags、period flags 未定義的語意保留數值，不發明解釋。
8. SET_TIME 傳手機當下 offset；不自動解決未來 DST／跨時區排程。
9. Sample 固定 mV/percent/計數，保存設定與歷史於記憶體；不模擬真實功耗、clock ticking、喝水或清除後計數重算。

## 9. Protocol 衝突、不明確與 firmware 實作風險

| 項目 | 現況／風險 | 整合前應決定 |
| --- | --- | --- |
| Payload Length | uint8 可表示255，但256-byte整包扣8 bytes只剩248 | 固定明文上限248；History公式採 floor((248-6)/6)=40 |
| CRC | 原規格只寫CCITT；profile/coverage/order會影響每一封包 | 定稿參數與跨語言 golden vectors |
| GET_CONFIG Status | 原Config結構從target開始，未明寫回應外層Status | 確認GET_CONFIG為status+config，與SET_CONFIG request區別 |
| ACK／錯誤 | 寫入成功代表收件、RAM套用或持久化尚未定義 | 定義ACK時機、錯誤長度、BUSY後續、斷電保證 |
| History一致性 | index/total在新增、環形覆寫或清除後可能改變；單頁檢查不足 | snapshot/cursor、排序、保留量、rollover與clear語意 |
| Packet physical binding | 尚無EEPROM區域／mailbox選擇、buffer ownership、commit marker | 明定雙方存取與atomic commit，RF/I2C競爭及中斷復原 |
| 小容量預算 | 若依目前工程範圍128x4 bytes估算，512-byte容量不能直接容納兩份256-byte封包再加metadata/history | 先做記憶體配置預算；不可直接假設雙buffer+history都能放 |
| Sequence | uint8會wrap，client重建從0開始；目前只能比對單次response | session/去重、遲到response、retry與SET/CLEAR idempotency |
| Time與當日 | UTC+一次offset不足表達未來DST；invalid/未同步值未定 | local-day/reset、timezone修改、時間倒退、invalid sentinel |
| Counter／LED／target | event uint8超255行為未定；16時段、8級LED、target0..255未有對應規則 | 飽和或改版加寬；定義DailyTarget與completion/LED關係 |
| Config修改與歷史 | period ID雖唯一，但之後改時段／重用ID會讓舊記錄語意不明 | config revision、ID生命週期、當日完成狀態重算規則 |
| 未定狀態／freshness | flags/batteryStatus語意、未量測電量與無最後事件缺乏明確表示；Status未含量測年齡 | validity/sentinel/capabilities、snapshot或measurement age |
| 授權 | CRC只能檢查完整性；UID/DeviceID不是所有權證明 | SET_TIME/SET_CONFIG/CLEAR_HISTORY的存取授權與防重播策略 |
| 格式演進 | parser拒絕未知version/status及保留record bits；無協商 | version/capability與unsupported行為，避免日後無法相容 |

表中「小容量預算」是依現有工程範圍的容量推算，並非本次新量測或對裝配晶片的直接核定。
本次僅核對程式與既有專案證據，未重新查閱或宣稱直接核對 manufacturer PDF。

## 下一位接手者操作順序

1. 讀 canonical handoff、machine registry；核對當前機器、Git狀態及本報告日期，保留未提交工作。
2. 使用 `Software/PeboAndroid` 保存來源；原ZIP僅原始快照，別用舊快照覆寫新證據。
3. PC-01現有本機副本：`C:/Users/tripl/AppData/Local/Temp/PeboAndroid-PC01-fb4640f8/PeboAndroid`；
   專用快取在同父目錄`gradle-cache`。TEMP不是跨機來源，可能被清理；每機建置請用ASCII路徑。
4. 若來源有修改，先把受控檔同步至本機副本再建置，核對manifest／差異，避免誤測舊副本。
5. Sony v3已安裝；依README/HIL檢查Sample標示、Info/Status、Config、History40/40/5、SyncTime、Debug與real mode失敗。
6. 工程Block0另外回歸；五次重備讀取、傳輸中斷恢復、無效輸入、背景取消、Copy/Clear尚待完整實機驗證。
7. 先定稿第9節，再實作firmware與真實packet transport；不得把Sample成功算成裝置整合成功。

Git：main，HEAD `11e2f6e8954aa45e373c615d853d323aac053979`。
App、證據與文件仍有未追蹤／未提交內容；本次未commit/push/fetch，未驗證雲端完整複製。
本機建置daemon已退出；adb先前維持可用，App上次啟動成功，本報告未再查手機／儀器即時狀態。
本次沒有MCU燒錄、電源或儀器操作。
