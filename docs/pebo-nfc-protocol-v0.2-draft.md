# Pebo NFC Protocol v0.2 Draft

**文件狀態：供人工 review；未採用、未實作。**
本文件新增於 v0.1 之後，不覆蓋 `nfc-protocol.md` 或 Android `PROTOCOL-v0.1.md`。
本輪不修改 code、Gradle、manifest、UI、MCU firmware，也不升級任何裝置 protocol。

狀態定義適用全文（含表格、流程與 MUST/必須用語）：
- **CONFIRMED**：使用者已指定的基線、目前程式/證據，或明列來源的硬體文件事實。
  不表示 v0.2 新設計已定稿；資料手冊事實也不等於本板已量測通過。
- **PROPOSED**：作者建議的 v0.2 規則；所有未另標狀態的新規則均屬此類。
- **OPEN**：仍需產品、firmware、安全或 HIL 決策。未確認不得帶入量產承諾。

閱讀配套：[Decision Table](pebo-nfc-protocol-v0.2-decisions.md)。
實作基線：[Android 技術交接](android-development-handoff.md)、
[目前 v0.1 文件](nfc-protocol.md)、`Software/PeboAndroid/PROTOCOL-v0.1.md`。

## 1. Design Goals

**CONFIRMED 基線**：Pebo 用 NFC-V/ISO15693，prototype 指定 ST25DV04K、目標 KC；
STM32L031G6U6 是目前 MCU。Kotlin protocol models 已與 ST/Android transport 分離。
PB magic、binary、LE、UTC uint32、8-byte Pebo Device ID 與 NFC UID 分離、八個既有
command 的語意與 response OR 0x80 規則應盡量保留。Drink Events 與 Completed Periods
不同；每個時段至多完成一次，但可保留多筆事件。58 項測試是 v0.1/Sample 證據。

**PROPOSED 目標**：
1. 有效的舊設定在中斷或掉電後仍可恢復；未完成或未授權的命令不生效。
2. retry 不造成重複配置提交、重新校時或再次清除新紀錄。
3. 成功完成的 History 同步無重複/缺漏；超過有限保留能力則明確失效，不假裝成功。
4. MCU 離線獨立提醒與計算 LED；手機提供設定與時區資料，不是運作必需品。
5. 512-byte tag EEPROM 不承擔高頻事件庫或雙份 256-byte 通訊緩衝。
6. 低 RAM、單一 outstanding request、事件驅動與可測試的 power-loss 行為。

**CONFIRMED 板級限制**：PA4/ST25_LPD 在既有 deep sleep 中保持 Output High；
此文件不改該規則。六個月 CR2032 是目標，不能由此草案或 sleep 數字推定達成。
**OPEN/P0**：實際裝配 K/KC、供電/GPO/LPD wake 行為、完整產品 firmware 資源與功耗預算。

## 2. Transport Layer

### 2.1 分離與工作模式

**PROPOSED** 三條路徑：

| 路徑 | 電源/MCU條件 | 內容 | App 可宣稱 |
| --- | --- | --- | --- |
| Passive EEPROM | RF 場可供 tag 使用；不要求 MCU 醒著 | Identity descriptor、上次提交的公開摘要 | 已讀取 cache，非即時狀態、非認證資料 |
| Active FTM | ST 有有效 DC 條件、MCU 可回應、FTM 已啟用 | 完整 request/response packet | 經 CRC/身份/sequence 檢查的動態結果 |
| MCU local | 裝置自身運作 | 排程、事件、持久化、去重、授權 | 須透過 active command 取回，不能直接推定 |

**CONFIRMED 原廠依據摘要**：04K/KC 的使用者 EEPROM 為 512 bytes；FTM 是另外的
256-byte 揮發性半雙工 mailbox。Mailbox 使用需要 DC 供電條件，不能當無電池被動通道。
K 的 RF mailbox 不受 EEPROM password 保護；兩代 I²C EEPROM 寫入均受 FTM 啟用狀態限制。
KC 的 LPD High 會關閉內部 regulator。這些只在 §23 所列文件範圍內直接核對，非本次板測。
來源：[KC DS13519](https://www.st.com/resource/en/datasheet/st25dv04kc.pdf)、
[K DS10925](https://www.st.com/resource/en/datasheet/st25dv64k.pdf)。

**PROPOSED**：Android protocol interface 收發完整 packet；ST adapter 負責型號探測、
mailbox ready/ownership、長度、RF command、I²C arbitration 與時序。K/KC 差異不進入 models。
不在本文件綁定 ST register 地址；adapter 規格需獨立以確切料號核對。

### 2.2 Active session 與 MTU

**PROPOSED**：裝置一次只服務一個 active session、一個 request；MCU 先完整收取後才產生
response。硬體 mailbox 被讀完/釋放不是 application ACK。過期 mailbox 內容即使仍可讀，
也不得當新命令或新 response。先檢查 owner/length，再檢查 packet 的 session/sequence。

256 是 application packet 的**硬上限**，不是所有手機都可單次傳送的保證。RF Write Message
有額外 flags/command/manufacturer/length 開銷；adapter 以實際最大 transceive 長度扣除開銷，
與裝置能力協商 `effective_mtu <= 256`。**PROPOSED 預設上限 240**，仍須依實際手機下調。
FTM 寫入採單一完整 message；不能把幾次獨立 mailbox 寫入假裝成一份原子 packet。
若最低命令不合 MTU，回 NOT_SUPPORTED；v0.2 baseline 不增加 fragmentation 狀態機。

**PROPOSED**：手機先讀 passive descriptor，觸發經驗證的 wake 機制，等待 active ready。
MCU session 期間可切至 active 所需 LPD 狀態，結束後回既有 sleep 規則。
**OPEN/P0**：RF 場/GPO 是否可在現有 sleep 狀態喚醒、active 電流、首包遺失與啟動時間。
不得假設 energy harvesting 可供 MCU/FTM 持續運作；若沒有 active ready，只允許 cache 讀取，
不提供「先寫 EEPROM、之後偷偷執行」的旁路。

### 2.3 EEPROM/FTM 排程

**PROPOSED**：FTM active 期間不更新 ST user EEPROM。公開 snapshot 延後到 session 結束、
mailbox 空閒且 FTM 已安全停用後更新；先完成 application response 再進維護時窗。
若另一個 RF 操作競爭，adapter 回傳可重試的 transport busy，不把半讀資料送進 parser。
wake、FTM watchdog、busy timeout 屬 transport；不能改變 application commit/dedup 語意。

## 3. Packet Format

本節全部為 **PROPOSED/BREAKING**，wire profile `0x02`；不是把 v0.1 的 VERSION 常數改為 2。
必須另建 codec/negotiation；v0.1 `0x01` header 保持原樣。

| Offset | Size | Field | 規則 |
| ---: | ---: | --- | --- |
| 0 | 2 | Magic | `50 42`，PB |
| 2 | 1 | WireVersion | `02`，draft profile |
| 3 | 1 | Command | response = request OR `80` |
| 4 | 1 | Flags | bit0 AUTH_PRESENT；bits1..7 必須0 |
| 5 | 1 | HeaderLength | 此 profile 固定18 |
| 6 | 2 | Sequence | uint16 LE，§6 |
| 8 | 8 | SessionId | uint64 LE；0只供公開 bootstrap/read |
| 16 | 2 | PayloadLength | uint16 LE，但受實際 MTU 限制 |
| 18 | N | Payload | command-specific |
| 18+N | 0或16 | AuthTag | AUTH_PRESENT=1 時 AES-CMAC tag |
| 18+N+A | 2 | CRC16 | LE，覆蓋 header+payload+AuthTag |

長度 = `18 + N + A + 2`。MTU256 時未認證 payload<=236、認證<=220；
MTU240 時分別<=220、<=204。驗證長度/版本/保留位後才配置或讀取 payload。
ISO15693 RF CRC 仍由 NFC stack 處理；此 CRC 是 application CRC。

**PROPOSED** 固定採既有 Sample 的 CCITT-FALSE：poly1021、initFFFF、非反射、xorout0000；
check(`123456789`)=29B1。Stored LE；MAC 覆蓋方向標籤+DeviceID+完整 header+payload，
不含 AuthTag/CRC。MAC 與 CRC 的用途分開；CRC 不是授權或防惡意改寫。

**PROPOSED**：成功 response 以 status=00 開頭；error 固定4 bytes：
`status:u8, retryAfterMs:u16LE, detail:u8`。detail 是穩定分類，不包含秘密。
mutating command 的 OK ACK：`status=00, operationId[16], committedRevision:u32`，共21 bytes。
ACK 明確代表 §7 的 durable commit，不只表示收到 mailbox。Time 例外恢復狀態見 §11/18。

## 4. Command Table

全部新 wire payload 均為 **PROPOSED**；`0x01..0x08` 數值及目的沿用基線。
所有實際效果由 MCU 驗證後產生，不由 tag register 自行理解 Pebo command。

| Code | Command | Request payload摘要 | OK response | 授權 |
| --- | --- | --- | --- | --- |
| 01 | GET_DEVICE_INFO | 空 | §8 DeviceInfo | 公開可用 |
| 02 | GET_STATUS | 空 | §9 完整動態 Status | 必須；公開最小摘要走passive |
| 03 | SET_TIME | OpId16+expectedClockRev4+expectedPolicyRev4+§11 TimePolicy | durable ACK21 | 必須 |
| 04 | GET_CONFIG | 空 | status+configRev4+§10 Config | 建議必須 |
| 05 | SET_CONFIG | OpId16+expectedConfigRev4+§10 Config | durable ACK21 | 必須 |
| 06 | GET_HISTORY | §13 phase/epoch/token/cursor/count | §13 page | 必須，屬兒童歷史資料 |
| 07 | CLEAR_HISTORY | OpId16+expectedHistoryEpoch4 | durable ACK21，新 epoch | 必須 |
| 08 | PING | 空 | status+bootEpoch4 | 公開可用；非配置 ACK |
| 09 | GET_TIME_POLICY | 空 | status+clockRev4+policyRev4+目前UTC/offset/剩餘轉換表 | 必須 |
| 10 | OPEN_SESSION | clientNonce[16]，seq0/session0 | status+deviceProof16；新SessionId在header | bootstrap proof |
| 11 | AUTH_SESSION | hostProof[16]，seq1 | status；成功 response 帶MAC | §15 |
| 12 | QUERY_OPERATION | originalCommand1+OpId16 | status+OpId16+state1+command1+revision4+originalStatus1 | 必須 |

QUERY_OPERATION state：0=UNKNOWN、1=IN_PROGRESS、2=COMMITTED、3=INDETERMINATE。
UNKNOWN 不等於「未執行」；不得因此自動用新 OpId 再做 CLEAR_HISTORY。
不支援的 command 回 INVALID_COMMAND；已知但未實作功能回 NOT_SUPPORTED。

## 5. Error Codes

**PROPOSED** 保留既有00..06數值；新增07..0D。細節不靠無限增加 status。

| Value | Name | 語意 | Client 行為 |
| --- | --- | --- | --- |
| 00 | OK | 完整成功；修改命令為已提交 | 接受驗證後結果 |
| 01 | INVALID_COMMAND | 不認得 command | 不重送相同內容 |
| 02 | INVALID_LENGTH | framing/payload長度不符 | 修正編碼 |
| 03 | INVALID_DATA | 結構/保留位/資料組合無效 | 修正資料 |
| 04 | BUSY | 尚未接受提交，或相同操作仍進行 | 同一身份與內容，等待 retryAfter |
| 05 | CRC_ERROR | application CRC不符 | 僅在仍有效session內重送原封包 |
| 06 | INTERNAL_ERROR | 無法完成；不得謊稱已commit | 查詢operation/狀態，不盲目新操作 |
| 07 | UNAUTHORIZED | 身份/MAC/權限失敗 | 停止寫入，重新授權 |
| 08 | NOT_SUPPORTED | 已知功能/必要欄位無能力 | 降級讀取或提示 |
| 09 | INVALID_STATE | session過期、舊seq、時間恢復不確定等 | 依detail恢復；不盲目重做副作用 |
| 0A | OUT_OF_RANGE | 數值超界 | 修正輸入 |
| 0B | STALE_SNAPSHOT | token過期/被取消/epoch不同 | 丟棄未完成session，重新同步 |
| 0C | CONFLICT | revision不符，或OpId相同內容不同 | 重新讀取，由使用者處理衝突 |
| 0D | STORAGE_FULL | 無法在保證一致性下提交 | 不刪舊有效資料以假裝成功 |

不新增 DUPLICATE：已完成的同一操作回原成功/原終態結果。沒有通用 RETRY code：
BUSY+retryAfter 已足夠；傳輸失敗是本地 transport error，不能偽造 MCU error response。
未知 error 由新 App 顯示數值並停止副作用；不得當 OK。

## 6. Sequence / Retry Rules

### 6.1 身份與範圍

**PROPOSED**：Sequence 只用於 session 內的配對、排序與 replay control，不是持久化交易ID。
SessionId 為裝置分配的64位識別，例如持久化 bootEpoch32 + 本次 sessionCounter32；
不是秘密或授權。bootEpoch 必須在開啟session前以安全NVM機制更新；耗盡拒絕開啟而非重用。
seq0供 OPEN，seq1供 AUTH，正常命令2..65535；到上限必須另開session，不在同session繞回。
公開無session讀取亦回顯seq，但不承諾任何副作用或去重。v0.1的0..255 wrap行為不變。

**PROPOSED**：有副作用命令另外攜帶 Android CSPRNG 產生的128-bit OperationId。
operation identity = `DeviceID + command + OperationId`；MCU 同時保存完整語意內容的128-bit
keyed digest，不能只比較 CRC16。換session重試仍使用同OpId、同body/expectedRevision。
同身份不同內容回 CONFLICT；相同seq不同command/body亦回CONFLICT，不執行任何一個新變體。

### 6.2 有限 cache 與 durable guard

**PROPOSED**：一次只允許1個 outstanding request。RAM 保留最後完整response最多256 bytes，
另保留最近4筆 mutation 的 compact結果（目標<=64 bytes/筆）。舊seq若已離開cache，回
INVALID_STATE，**不能當成新命令執行**。同session相同完整請求可回原response bytes；
跨session恢復則重包相同已提交的邏輯結果、新session/seq/MAC，不能原封重播舊MAC。

CONFIG 的最後OpId/digest/revision放在其同一 committed slot；TIME/CLEAR 的最後結果放
durable recovery journal。每種 mutation 至少保留最近1個 durable operation result。
CAS expectedConfigRev/ClockRev/PolicyRev/HistoryEpoch 防止更早、已被cache淘汰的操作重新套用。
不宣稱能對無限期、任意多筆歷史操作保存 exactly-once；超出窗口回 UNKNOWN/CONFLICT。
accepted mutation 產生一個新revision（重送原操作不增rev）；避免無意義重複寫NVM。

### 6.3 retry 策略與範例

**PROPOSED 初值，HIL後定稿**：active response等待2s，wake最多3s；逾時在session有效時
最多重送2次（總3次），退避100/300ms加小幅jitter。單session idle30s、絕對120s上限。
nonce/session expiry、BUSY與FTM watchdog不是同一計時器。底層自動retry只能重送相同
bytes，禁止替App產生新seq/OpId；所有retry受同一上限與deadline管理。

例：SET_CONFIG(session S,seq42,op X,expectedRev7)提交rev8但response遺失：
1. 同session同請求：回原rev8 ACK，不重寫、不重算完成時段。
2. 重開session後同op X與同body：由slot的durable結果回rev8 ACK。
3. op X但body不同：CONFLICT。
4. 已有別的配置更新且X已不在durable結果：expectedRev7已過期，CONFLICT，不套回舊配置。
5. 無法確認是否完成：QUERY_OPERATION；UNKNOWN/INDETERMINATE不可盲目重做SET_TIME/CLEAR。

## 7. Atomic Update Rules

### 7.1 方案比較（全部為設計比較，不是已實作）

| 機制 | 優點 | 單獨使用的缺點 | 建議角色 |
| --- | --- | --- | --- |
| valid flag | 成本小 | flag本身可撕裂；不能證明body完整 | 不單獨採用 |
| generation counter | 可選新版、偵測變化 | 一個counter不能保住舊body | 每slot內、受CRC保護 |
| double buffer | 斷電時保住上一份 | 需要兩倍儲存 | 小型settings/snapshot採用 |
| sequence number | 配對請求 | 不證明NVM已提交，重啟會遺失 | 不作commit證據 |
| CRC | 偵測撕裂/錯誤 | 不提供授權、也不提供物理原子寫 | body+metadata校驗 |
| header+payload+commit | 可定義唯一有效狀態 | 仍需順序、readback與備份 | 結合A/B、generation、CRC |

**PROPOSED 首選**：FTM 的完整message接受 + MCU單寫者 + 持久化A/B slot
(header/generation/length/payload/operation身份/CRC32/commit-last)。不用兩份256-byte
EEPROM通訊packet，也不讓RF直接改authoritative config。

### 7.2 Slot 與線性化點

**PROPOSED**：每slot包含 storageSchema、generation32、length、完整資料、需要的OpId/
digest/revision、storage CRC32、兩個互補commit markers。storage CRC採CRC-32/ISO-HDLC，
check123456789=CBF43926，LE；與application CRC16分開。不得把讀回相等當成未來永久不壞。

寫 inactive slot 的順序：
1. 不動目前最高有效generation的slot。使目標commit無效，等待寫完並讀回確認無效。
2. 寫header/body/CRC，逐段等待實際裝置完成；讀回全長、長度與CRC核對。
3. 最後寫commit markers，再讀回整slot。**新slot首次同時通過marker/length/CRC時**是提交點。
4. 之後才切換RAM active指標、套用新revision、發OK ACK。舊slot至少保留到下一次更新。
5. 開機掃兩slot，只接受完整有效者，選最高generation；同generation不同body視為corruption。
   generation到上限進維護，不以簡單比較誤判wrap。雙slot皆壞進INVALID_STATE/default-safe，
   不靜默覆寫並宣稱恢復了使用者配置。

**關鍵限制**：不假設ST EEPROM一次4-byte或STM32一次word寫入在brownout下天然原子。
邏輯原子性靠「完整body先驗證、commit-last、仍有上一份」建立；CRC是有殘餘漏檢機率的
故障偵測，不是數學零機率保證。實際program granule、busy時序、BOR/低壓門檻、斷電注入
需HIL核定；在其完成前不能宣稱power-fail atomic已驗證。

### 7.3 中斷、同時讀取與 recovery

- App在SET_CONFIG半包離場：未完整message、長度/CRC/MAC不符，沒有副作用；stage丟棄。
- 完整命令已送入但離場：MCU可以提交；手機不知道結果，須以同OpId恢復，不建立新操作。
- MCU在inactive slot寫到一半掉電：該slot不完整，boot仍用舊有效slot。
- 新slot提交但ACK遺失：boot以新slot連同OpId辨認已完成，回相同revision。
- Android讀passive snapshot遇到MCU更新：讀A/B完整記錄，驗證schema/gen/CRC/commit；
  不依賴可撕裂的單一active指標。若更新競爭無法取得穩定完整資料，有限次重讀後報busy。
- Active GET_STATUS：MCU先以短critical section擷取RAM snapshot，再編碼，不混用幾個時間點。
- CLEAR_HISTORY以持久化HistoryEpoch更換為線性化點，非逐筆清零；舊bytes不再是有效記錄。
- SET_TIME跨RTC與NVM不具天然跨元件原子性，必須採§11的intent/recovery限制，不能用本節
  的config A/B保證偷換成「RTC與所有狀態也完全原子」。
## 8. Device Info

下表為 **PROPOSED** OK payload；未實作時不得宣稱支援。NFC UID 不在此payload冒充DeviceID。

| Offset | Size | Field |
| ---: | ---: | --- |
| 0 | 1 | Status=00 |
| 1 | 8 | Pebo Device ID，opaque bytes |
| 9 | 2 | HW major/minor，各u8 |
| 11 | 3 | FW major/minor/patch，各u8 |
| 14 | 1 | Negotiated WireVersion |
| 15 | 2 | Capability Flags u16 |
| 17 | 1 | Minimum supported stable major；無stable支援時0 |
| 18 | 1 | Maximum supported stable major；不可預告尚未實作的1.x |
| 19 | 2 | Maximum application packet length u16，<=256 |
| 21 | 1 | Maximum Reminder Periods，baseline16 |
| 22 | 1 | History record wire size，baseline10 |
| 23 | 2 | Normal retained record capacity，baseline建議12而非承諾大量歷史 |
| 25 | 1 | Auth profile，0=disabled工程品，1=本草案shared-key CMAC profile |
| 26 | 1 | Ownership state：0=unclaimed，1=claimed，2=recovery/service |
| 27 | variable | 可選TLV；v0.2本版可省略 |

**PROPOSED** capability描述Pebo功能；IC型號/暫存器探測只屬ST adapter與工程診斷。
public passive descriptor提供可辨認的layout/schema與supported profiles；它本身不做活體認證。

## 9. Device Status

以下 **PROPOSED** 是完整active response基線，固定57 bytes，之後才是可選TLV。
基線建議完整GET_STATUS需認證；公開最小資訊僅走passive摘要。未認證回UNAUTHORIZED，不以假0值填滿完整狀態。

| Offset | Size | Field / validity |
| ---: | ---: | --- |
| 0 | 1 | Status |
| 1 | 2 | StatusFlags u16，見下 |
| 3 | 4 | BootEpoch |
| 7 | 4 | ClockRevision |
| 11 | 4 | TimePolicyRevision |
| 15 | 4 | ConfigRevision |
| 19 | 4 | HistoryEpoch |
| 23 | 4 | DayToken，單調邏輯day識別，非時間戳 |
| 27 | 4 | LocalDayOrdinal，自1970-01-01起的local曆日數；僅TIME_VALID時有意義 |
| 31 | 4 | DeviceUnixTime UTC |
| 35 | 4 | SnapshotUnixTime UTC，資料擷取時間 |
| 39 | 4 | BatteryMeasurementAgeSeconds；不明=FFFFFFFF |
| 43 | 2 | Battery mV；BATTERY_VALID=0時忽略 |
| 45 | 1 | BatteryPercent 0..100；FF=unknown，禁止用0表示未知 |
| 46 | 1 | BatteryStatus：0=normal、1=low、2=critical、FF=unknown；門檻OPEN |
| 47 | 2 | TodayDrinkEventCount u16，飽和而不繞回 |
| 49 | 1 | CompletedReminderPeriods 0..16 |
| 50 | 1 | EligibleReminderPeriods 0..16 |
| 51 | 1 | DailyTarget u8 |
| 52 | 1 | LEDProgress 0..8，MCU計算 |
| 53 | 4 | LastDrinkUnixTime UTC；LAST_DRINK_VALID=0時忽略 |

**PROPOSED StatusFlags**：bit0 TIME_VALID、1 BATTERY_VALID、2 LAST_DRINK_VALID、
3 TIMEZONE_POLICY_STALE、4 COUNTER_SATURATED、5 RECOVERY_REQUIRED、6 READ_ONLY_CACHE、
7 HISTORY_GAP_OR_LOSS、8 SCHEDULE_VALID、9 COUNTS_VALID；10..15保留。UTC=0不作唯一invalid sentinel。
Passive cache永遠標示READ_ONLY_CACHE，DeviceUnixTime若來自cache不是持續走時的現在值。

**PROPOSED**：TIME_VALID=0時不進行依local時段的提醒/完成計分，LED=0；drink仍可記錄但PeriodID=FF且TIME_VALID=0，不猜日期。此行為需產品確認。

**PROPOSED recovery規則**：day count與completion bitmap需持久化checkpoint及其對應RecordID。boot先復原checkpoint，再replay其後完整records；在ring覆寫replay所需資料前先提交新checkpoint。無法完整復原時清COUNTS_VALID、LED=0且不重新發PeriodCompletionEvent，不能拿有限12筆history反算整日總數。checkpoint額外NVM/磨耗須計入§20，尚未完成配置不得宣稱跨掉電的整日資料已保證。

### 9.1 Counter widths 與 ownership

| 欄位 | v0.2建議 | 理由／邊界 |
| --- | --- | --- |
| Today Drink Event Count | **PROPOSED u16**，65535飽和並設flag | debug或未來高事件率不能悄悄uint8 wrap |
| Completed Period Count | **PROPOSED 維持u8**，0..16 | 每DayToken每PeriodID至多1次，不是累計事件 |
| Total/Eligible Period Count | **PROPOSED 維持u8**，0..16 | 排程規模，不是lifetime counter |
| Daily Target | **PROPOSED 維持u8**，baseline0..16 | 產品目標改定為「完成時段數」；不是事件總數 |
| LED Progress | **PROPOSED 維持u8**，0..8 | 顯示狀態，永遠不作累加器 |
| Lifetime events（非baseline） | **OPEN/P2**，若加入至少u32或u64 | 另設capability/欄位，不挪用daily count或RecordID |

**PROPOSED LED方案B**：MCU用相同已提交配置與完成ledger計算；App只顯示回報值。
`goal=min(DailyTarget, EligiblePeriods)`；若target=0、goal=0、排程無效或COUNTS_VALID=0則progress=0。
否則 `min(8, floor(8 * min(CompletedPeriods,goal) / goal))`，用至少u16中間值。
App不得直接SET_LED_PROGRESS來改產品狀態；工程LED測試必須另command/profile，不混入本協定。
方案A需要手機常在附近且會造成邏輯分岔，故不推薦。
**OPEN/P0**：DailyTarget是否確實表示完成時段目標；若產品要「事件目標」，必須改欄位語意/
另設progressMode，不可在同一欄位有時算事件、有時算period。此建議尚未成CONFIRMED。

## 10. Configuration

**PROPOSED** 保留既有緊湊Config結構，GET_CONFIG的status/revision屬外層：

| Offset（Config內） | Size | Field |
| ---: | ---: | --- |
| 0 | 1 | DailyTarget，baseline0..16 |
| 1 | 1 | PeriodCount<=16 |
| 2 | 1 | ReminderEnable，0/1 |
| 3 | 1 | Reserved=0 |
| 4+6i | 1 | PeriodID 0..15 |
| 5+6i | 2 | StartMinute u16LE，0..1439 |
| 7+6i | 2 | EndMinute u16LE，1..1440 |
| 9+6i | 1 | PeriodFlags，baseline0；未定功能不得亂設 |

Config長度恰為4+6*count，最大100。半開區間[start,end)、不重疊、不跨午夜、ID唯一。
End=1440只表示當日結束，不代表隔日排程。零period只在ReminderEnable=0且target=0接受；
enabled時1<=target<=count<=16。這比v0.1 Sample允許任意target更嚴格，是明示breaking proposal。

SET_CONFIG的OpId16+expectedRev4+Config最多120 bytes，可放入認證MTU240。
先驗證整份配置、CAS revision及授權，再一次durable commit；不逐period邊收邊套用。
ACK帶新ConfigRevision，GET_CONFIG讀回相同revision。沒有部分更新命令。

**PROPOSED**：當日修改排程不清零已完成ledger，ledger以(DayToken,PeriodID)標記；
當日新增/改時間的PeriodID若已完成，不再重新計一次。ID不允許在同DayToken刪除後改作
全新時段來重領完成次數。新schedule只影響未來提醒，不回算過去事件；已完成可能超過
修改後的eligible數時，Status的completed定義為目前eligible集合的完成數，ledger仍保留
舊bit防重算。配置變更時只改可見集合，不把DrinkEventCount重設。

**OPEN/P1**：需否保存歷代配置以解釋舊PeriodID？baseline只保證原始ID，不拿「目前設定」
重新解釋過去歷史。若產品要求完整歷史排程重建，需另加ConfigRevision/配置歷史儲存，
會增加record或context成本，不能宣稱現有10-byte記錄已提供該功能。

## 11. Time Handling

本節除UTC基線外均為 **PROPOSED**：
- RTC運作與DeviceUnixTime採UTC；Drink Record永久保存事件當下UTC及flags，校時不改寫舊record。
- Reminder Period使用裝置已提交時區政策下的local minute-of-day，不保存成HH:mm字串。
- 手機所在地改變不自動改瓶子時區；App提示家長選擇保留home zone或明確提交新政策。
- MCU不內建IANA tzdb。App用作業系統/tzdb計算轉換，送有限有效期政策；不能只給一次offset
  然後假定美國/歐洲永遠沒有DST。IANA規則會隨政策更新：[IANA](https://www.iana.org/time-zones)。

### 11.1 SET_TIME中的TimePolicy body

在OpId16、expectedClockRev4、expectedPolicyRev4後：
`utcNow:u32, baseOffsetMinutes:i16, validUntilUtc:u32, transitionCount:u8,
[effectiveUtc:u32,newOffsetMinutes:i16]*count`。
最大count=4，body=11+6count<=35，整個request payload<=59 bytes。
baseOffset限制-840..840；transition時間嚴格遞增、在utcNow之後且<=validUntil；
每次新offset同範圍。App送未來約400日或最多4次轉換的較短範圍。
無DST地區可count0，但仍有validUntil便於資料更新。IANA zone名稱留Android/帳戶側，
DeviceInfo只報有沒有政策表能力，不把時區資料庫塞進小MCU。

**PROPOSED**：到有效期外使用最後已知offset，設TIMEZONE_POLICY_STALE，提醒仍依此偏移，
App清楚提示可能差1小時；不可假裝仍已驗證DST。若產品選擇過期停提醒，屬OPEN/P0，
需人工選擇，不能一邊稱離線可用又默默停功能。

### 11.2 DST、跨國與day

- 春季跳過一段時間：完全不存在的period標為skipped，從當日eligible集合排除；部分存在者
  只在實際存在區間提醒。skipped不是completed。
- 秋季重複時間：兩次UTC區間使用同一(DayToken,PeriodID)，最多完成一次；事件逐筆記錄。
- timezone新政策須家長明確確認；基線建議立即生效，若local日/當下offset改變，建立新DayToken並標示travel/time-change造成的部分日，不刪除舊day。offset轉換表中已排定DST則於精確effectiveUtc生效，不需每次由手機確認。
- 每正常新日建立單調DayToken，token不同於LocalDayOrdinal。政策生效/大幅校時造成日期跳變
  時，關閉舊day並另建帶reason的新day；不覆寫之前日期摘要，不用「把counter倒回0」偽裝同一日。
- 小幅UTC校正若仍屬同一day，保留completion bits。時間倒退跨日則建立新的day context，
  設clock-change旗標；App對GET_STATUS取得的日摘要依DayToken而非單看曆日聚合，避免兩個相同日期混算；不代表raw History已有DayToken。
- clock未同步時，record保留monotonic RecordID，UTC欄可填0但清TIME_VALID；不猜測實際UTC。
  baseline不回填未知舊時間。已知校時前後的時間軸不能靠timestamp作唯一去重。
- Unix表示不編碼閏秒；uint32到2106的壽命限制需在後續major處理，不偷換成signed32。

**OPEN/P0 的歷史顯示界線**：10-byte record只保存UTC/ID/flags，沒有當時offset或DayToken。
因此baseline能保證事件UTC與去重不混淆，但不能從raw History重建「當時所在地的曆日」。
App應預設UTC，或明示正在以哪個view timezone顯示，不能把目前手機時區誤稱事件原始時區。
若產品要求保存原始local time，每record需再加offset:i16，成12bytes；認證MTU256每頁最多16筆、
MTU240最多15筆，storage也增加。該需求須在P0選定後改草案，不能暗藏在現有10bytes承諾中。

### 11.3 RTC與NVM跨元件提交

**PROPOSED**：SET_TIME先durable寫入intent（OpId/digest、目標UTC/政策、預期revision與phase），
再設RTC、完成time/policy revision與結果journal，最後ACK。完成後重試只回原結果，絕不
再把RTC設回舊request的utcNow。RTC設定精度目標暫訂±2秒，另量測radio/處理延遲。

**OPEN/P0實體邊界**：RTC寫入與NVM不能假設一次原子。如果掉電後可用保留的RTC/reference
可靠證明phase，才完成原交易；無法區分「已設RTC但結果未commit」時標
INDETERMINATE+TIME_VALID=0/RECOVERY_REQUIRED，不重套舊timestamp。App查QUERY_OPERATION，
要求明確新鮮校時；不能回假的OK，也不宣稱任意掉電下所有副作用exactly-once。
新鮮SET_TIME需新OpId與讀回的revision，且由使用者/已授權sync流程明確發起。

**PROPOSED** GET_TIME_POLICY回目前UTC、當下有效offset、validUntil與剩餘transition，供App明確讀回。App應另保存home zone的IANA名稱；裝置不藉offset猜國家。新手機若沒有原zone名稱，要求使用者確認，不自動當成手機所在地。

## 12. Drink History

### 12.1 Record格式與必要性

**PROPOSED** 從6升為10 bytes，只增加真正需要的RecordID；不每筆再塞session/generation：

| Offset | Size | Field |
| ---: | ---: | --- |
| 0 | 4 | RecordID u32，範圍1..FFFFFFFE，單調，允許gap |
| 4 | 4 | UnixTime UTC u32 |
| 8 | 1 | PeriodID 00..0F或FF |
| 9 | 1 | Flags：bit0 valid drink、1 period completion、2 reminder active、3 manual/test、4 TIME_VALID；5..7保留 |

唯一鍵為`DeviceID + HistoryEpoch + RecordID`；timestamp不是鍵。
HistoryEpoch在page帶一次，而不是每個record增加4bytes。相同秒兩次事件、RTC倒退均不碰撞。
bit4是v0.2新增，不能傳給會拒絕保留位的v0.1 decoder。
PeriodID=FF時禁止PeriodCompletionEvent=1；沒有時段不能宣稱完成某時段。

**PROPOSED** RecordID以NVM high-water批次預留32個ID，預留成功才使用；重啟跳過未用完
區間以避免重用，gap合法。開機先掃有效record與high-water一致性。到FFFFFFFE之前，必須
結束現有snapshot並建立新的HistoryEpoch；epoch本身到上限需維護/升版，不能靜默wrap。
CLEAR_HISTORY提交新epoch；不重用舊epoch下的ID。App保留已同步key，用epoch辨認清除/重建。

### 12.2 持久化與容量

**PROPOSED baseline小容量方案**：內部data EEPROM用16個實體record slots，每slot概算
20bytes（10-byte wire資料+4-byte HistoryEpoch+4-byte storage CRC+2-byte commit）；正常只保留最新12筆，另4個作
snapshot期間的持久化append reserve。不是「可存數日所有事件」的承諾。
Ring可丟最舊未pin資料，必須回報oldest/newest ID與gap；retention是明確功能能力。
storage必須包含epoch或等效不可混淆的generation：CLEAR只切全域epoch而每slot沒有epoch，會把舊有效slot誤認成新歷史，因此不能省掉這4 bytes卻宣稱可安全復原。

**OPEN/P0**：產品若要求數週歷史，1KB內部EEPROM與512-byte tag不能憑空提供；需選擇較大
tag/外部NVM、明確保留部分事件、或在flash預留wear-levelled log。此草案不直接佔用program
flash或改BOM。RAM-only history不能宣稱掉電保留。Flash每事件擦寫不是baseline方案。

增加RecordID的成本：每筆+4 bytes；以v0.1原封包為基準40筆/頁，v0.2新增session/auth/page
metadata後最多19筆（MTU256/auth）、預設18筆（MTU240/auth）。不把全部下降歸咎於RecordID。
同v0.2認證框架若仍用6-byte record理論可33筆/頁；10-byte為19筆。這個代價換得穩定去重/
校時安全/非index分頁，值得；歷史時間/配置完整脈絡仍是另項產品需求，不宣稱已解決。

### 12.3 現有硬體上的兩個可審查profile

**PROPOSED建議先做產品選擇，不默默混用保證**：
- **A：stock-hardware minimal**。若家長可接受拔電後歷史消失，History用MCU RAM ring
  （12 retained+4 reserve），CAP_PERSISTENT_HISTORY=0；boot先持久化新HistoryEpoch，
  然後重新開始空history。§13仍可在本次供電期間保持一致，掉電就STALE；不得宣稱durable
  reserve。設定/OpId/day checkpoint仍放NVM。§20估算移除320B record NVM後約832B，
  另加RAM約320B；有機會在既有1KB/8KB內完成，但仍須精算/實測。
- **B：persistent-history候選**。保留§12.2的20-byte NVM slots與durable reserve，需接受
  §20約1152B的壓力並另選更精簡的經驗證儲存格式或增加NVM。不能說目前已放得下。

若P0要求歷史必須斷電保留，**不得選A充當達標**。未有產品答案前兩profile都保持PROPOSED；
本文提及durable history/pin reserve的強保證只對B生效。兩者對App用相同record/page模型，
由capability與retention/boot epoch清楚表達不同保證，不靠ST料號讓上層猜測。

## 13. History Pagination

### 13.1 首選：固定上界＋單調cursor＋短時pin

**PROPOSED**：一個device最多1個snapshot；綁認證session，TTL30秒且不無限續期。
OPEN時以短鎖捕捉`HistoryEpoch, snapshotId, lowerRecordId, endExclusive`；
endExclusive為當時最後已提交ID+1。固定集合是該區間內已存在的有效records，ID可有gap。
後來事件ID>=endExclusive，不加入本次同步。cursor是「下一個RecordID下界」，不是陣列index。

| 比較 | 評估 |
| --- | --- |
| timestamp cursor | 同秒/校時會碰撞；不採 |
| generation+index | 可偵測改變，但新增就要重來；不滿足穩定同步 |
| 全份RAM snapshot | 一致但RAM成本隨歷史長度成長；不採 |
| RecordID固定上界，無保留 | 不移位，但舊record可被覆寫；須額外STALE偵測 |
| 固定上界+短時pin+4 reserve slots | **推薦**；不複製整份history，正常新事件仍可durable append |

期間新事件先寫4個持久化reserve slots，不覆寫pin的12個slots。當第5個事件需要空間時，
**先使snapshot失效**、釋放pin、再正常保留新事件；後續page回STALE_SNAPSHOT。
不能為了保snapshot丟新drink，也不能默默回缺漏頁。成功page內部仍需read/CRC一致。
這是「TTL內、reserve未耗盡」的穩定保證，不是任意流量/任意久的無限保證。
MCU重啟、CLEAR_HISTORY、flash/NVM錯誤都使snapshot失效。

### 13.2 Request與response

**PROPOSED request固定14 bytes**：
`phase:u8, historyEpoch:u32, snapshotId:u32, cursor:u32, requestedCount:u8`。
phase0=OPEN（epoch/token/cursor=0），1=NEXT，2=CLOSE（count=0）；OPEN/NEXT count>=1。
OPEN可直接附第一頁；NEXT必須是同snapshot的合法cursor。CLOSE回status-only成功，釋放pin。

**PROPOSED response page**：

| Offset | Size | Field |
| ---: | ---: | --- |
| 0 | 1 | Status |
| 1 | 4 | HistoryEpoch |
| 5 | 4 | SnapshotId |
| 9 | 4 | LowerRecordId |
| 13 | 4 | EndExclusive |
| 17 | 4 | NextCursor |
| 21 | 1 | ReturnedCount |
| 22 | 10*count | Records |

`nextCursor <= endExclusive`；完結時相等，不能用returnedCount小於requestedCount推定結束。
回傳ID嚴格遞增、>=request cursor、<endExclusive，沒有重複；gaps可由ID預留/保留政策產生。
沒有record但尚未結束時必須把cursor推到endExclusive或下一個存在ID，禁止停滯空頁。
每頁count上限由實際MTU扣除header/auth/CRC/pageprefix後計算，而不是硬編碼舊40。

Android累積於暫存同步session，最後一頁成功才標「本次完整同步」。遇STALE或epoch改變，
丟棄未完成snapshot的完整性宣稱；已取得的單筆可按唯一鍵存成partial，但不能混成成功快照。
允許重新OPEN，從新snapshot同步並以key去重。Snapshot不提供device永久保留已被清除資料。

## 14. Memory Ownership

### 14.1 ST25DV user EEPROM邏輯map（PROPOSED，不綁實際address）

定義logical base符號；實際start/end、area protection邊界與NDEF相容性 **OPEN/P0**。
下列大小合計512，mailbox不在此512內。

| Logical Address Range | Budget | Purpose | Owner | RF / I²C permission | Persistence | Update timing |
| --- | ---: | --- | --- | --- | --- | --- |
| `DESC_BASE .. +63` | 64 | identity/layout descriptor、supported profiles、capabilities、CRC | 製造/受控MCU維護 | RF read；RF write拒絕；受控I²C寫 | EEPROM | 製造/受控FW升級，非每次boot |
| `PUB_A_BASE .. +95` | 96 | 公開摘要slot A | MCU單寫者 | RF read only；I²C MCU write | EEPROM | 節流後commit |
| `PUB_B_BASE .. +95` | 96 | 公開摘要slot B | MCU單寫者 | 同上 | EEPROM | A/B交替 |
| `RESERVED_BASE .. +255` | 256 | 預留，不作command inbox | 未分配 | 初期RF read或保護；不可接受產品寫入 | EEPROM | 未分配不寫 |
| `MAILBOX_LOGICAL[0..255]` | **另256** | 單一完整active packet，半雙工 | 由adapter仲裁 | RF/I²C依mailbox ownership | volatile | active session only |

Descriptor內容至少包含layout version、長度、DeviceID、HW/FW、supported wire profiles、
有效MTU上限與公開slot定位/schema；不含root key/password。64byte具體offset/CRC需製造map
review後定稿。沒有足夠證據現在指定真實物理address，也不新增NDEF保證。

公開摘要只放最小資訊：電量有效性/mV/percent、LED、snapshot UTC、battery量測UTC、
config/clock/policy revision等，建議payload<=64、外加generation/CRC/commit仍在96內。
預設不把兒童完整history與具體喝水時間放公開EEPROM；DeviceID公開性需privacy review。
CRC-protected cache不是authenticated active response，UI必須保留此區別。

### 14.2 STM32 RAM / data EEPROM ownership

| 資料 | 權威來源 | Android取用 | 持久化策略（PROPOSED） |
| --- | --- | --- | --- |
| 正式config/時區政策 | MCU | 認證GET_CONFIG/SET_CONFIG/SET_TIME | MCU data EEPROM A/B |
| clock、完成ledger、當日count | MCU | active GET_STATUS | RTC/必要checkpoint，非每tick寫tag |
| pending request / response | MCU+FTM | packet exchange | RAM/volatile，非EEPROM inbox |
| OperationId/結果/revision | MCU | QUERY_OPERATION/ACK | 與變更同slot或recovery journal |
| History records / high-water | MCU | 認證GET_HISTORY | 小型NVM ring+allocator；retention另決策 |
| root/session keys | MCU/Android受保護儲存 | 不可讀回 | 不放tag公開區或文件 |

**PROPOSED 節流**：tag snapshot最多每6小時更新一次，另允許明確配置變更或使用者sync
結束時更新；同一值不重寫。量測與snapshot freshness分開。不要每次drink、RF poll、boot
或每秒時鐘更新EEPROM。內部journal也需磨耗預算；recordID allocator批次預留，不每事件
多寫同一個head cell。溫度/壽命/寫入熱點仍需用確切料號與使用情境計算。

**OPEN/P1**：更大tag/外部NVM可新增archive storage capability；不能改既有field語意或讓
上層model依賴K/KC寄存器。保留區不表示256bytes足以加入所有未來功能。
## 15. Write Authorization

### 15.1 威脅與選項

**PROPOSED 基線要求**：陌生手機即使可讀/寫mailbox，也不能改設定、校時或清History。
「讀取類命令」是沒有裝置副作用，不表示全部內容必須公開。GET_HISTORY/完整Config
建議要授權；兒童資料的privacy policy仍需產品review。

| 選項 | 優點 | 不足 | 建議 |
| --- | --- | --- | --- |
| 任意NFC手機可寫 | 最簡單 | 不符合需求 | 排除量產 |
| 通用PIN/預設password | 容易做 | 低熵、共用或可猜；不能靠UID衍生 | 不作root credential |
| ST RF password/area protection | 可保護被動EEPROM | 不能取代Pebo層的mailbox授權；也不是加密通道 | 製造時限制EEPROM RF寫入 |
| per-device random key + proof/MAC | 離線、小MCU可行、不需帳戶常在線 | 要製造/保管/復原流程及審查 | **推薦第一版最小安全方案** |
| 雲端家長帳戶/多把App key | 可撤銷/分享/管理 | 後端/金鑰生命週期/成本 | 未來upgrade，不作目前前提 |

**PROPOSED 第一版（不是現在實作）**：製造以CSPRNG產生每台獨立128-bit secret，
存MCU受控NVM；提供家長包裝內覆蓋式QR setup secret。不得把它等同DeviceID/NFC UID，
不得放可公開讀的tag EEPROM、log、repo或Debug畫面。陌生手機不能只靠靠近就取得key。
首次claim需證明持有secret；恢復/轉移ownership只走受控service或另經review的實體授權，
不能讓未認證的CLEAR/RESET移除owner。現板沒有已確認的pairing button，不憑空設計成有。
**OPEN/P0**：QR散失、手機遺失、家長轉移、key rotation與factory/service reset流程。

### 15.2 認證profile草案

全部 **PROPOSED，需專門security review與cross-language vectors，不是完整security實作**：
- 用AES-128 CMAC與標準CMAC-KDF，不自行發明弱hash。參考
  [NIST SP800-38B](https://csrc.nist.gov/pubs/sp/800/38/b/final)、
  [SP800-108r1](https://csrc.nist.gov/pubs/sp/800/108/r1/upd1/final)。
- Android產生clientNonce16；MCU提供不重用的SessionId。此方案不假設L031有硬體CSPRNG。
- context固定為DeviceID8 | WireVersion1 | SessionId8 | clientNonce16。
  用KDF-CMAC(root, label=`PEBO-NFC-SESSION`, context, L=128)導出session key。
  OPEN回deviceProof=CMAC(sessionKey, `DEVICE` | context)；AUTH帶
  hostProof=CMAC(sessionKey, `HOST` | context)。方向domain不可混用。
- 成功AUTH之後每request/response均以session key與§3的方向分隔計MAC；驗MAC後才做
  dedup/CAS/副作用。認證失敗不回敏感內容，也不更新持久化配置。
- Session失效刪RAM key；不能只因某張ST RF password session仍開就延長Pebo授權。
  Android端以KeyStore保護持久化secret；實際CMAC backend/非匯出key能力要另核對，
  不能假設所有手機都提供同一原生CMAC provider。
- OpId fingerprint與session key分離：MCU用per-device衍生的固定operation-digest key，
  對command/OpId/expected revision/完整body計128-bit MAC，跨session仍可比較。
- 一個active session期間拒絕陌生OPEN覆蓋它。未授權wake時窗建議最多5秒、失敗proof
  每分鐘最多3次，採RAM cooldown，不把每次攻擊寫NVM而製造磨耗。數值需功耗/HIL review。

安全界線：MAC不加密；RF側錄可看到packet內容。無法防NFC層DoS/持續RF喚醒，也不宣稱
能抵抗實體讀出MCU或偷看QR。量產若須History機密性，**OPEN/P0**選定AEAD profile或
縮減公開資料；不能把「已授權讀取」誤說成「傳輸已加密」。

**PROPOSED upgrade path**：保留authProfile/keyVersion，未來導入每個家長/App獨立key、
撤銷、rotation與帳戶簽發授權；帳戶邏輯留App/後端，MCU不解析JWT/TLS/X.509。
有較高物理威脅時評估secure element或不同MCU；ST EEPROM保護是adapter功能，
不是KC專屬的Pebo安全旗標。沒通過安全流程前，量產品寫入功能應關閉，而非降成任意人可寫。

## 16. Capability Flags

**PROPOSED uint16分配**，描述已實作可用的Pebo功能，不描述ST register/晶片型號。

| Bit | Name | 語意 |
| ---: | --- | --- |
| 0 | HISTORY_READ | 可回傳Drink records |
| 1 | CONFIG_READ | 可讀正式配置 |
| 2 | CONFIG_WRITE | 可提交配置，仍需授權 |
| 3 | TIME_SYNC | 支援受授權SET_TIME |
| 4 | BATTERY_MV | 可提供有效量測mV |
| 5 | BATTERY_PERCENT | 有定義且實作的percent演算法 |
| 6 | LED_PROGRESS | MCU計算並回報progress |
| 7 | ACTIVE_PACKET_EXCHANGE | 可執行active完整packet exchange，不承諾某種IC |
| 8 | PASSIVE_SNAPSHOT | 可讀有schema/generation的passive摘要 |
| 9 | AUTHORIZED_MUTATIONS | 寫入受本協定授權機制限制 |
| 10 | CONSISTENT_HISTORY_SNAPSHOT | 提供§13的有界一致性及STALE處理 |
| 11 | TIMEZONE_TRANSITIONS | 可使用有限UTC-offset轉換表 |
| 12 | OPERATION_RECOVERY | 可查 durable operation結果 |
| 13 | AUTHORIZED_HISTORY_READ | History須驗證身份 |
| 14 | PERSISTENT_HISTORY | 回報的retention包含斷電保留 |
| 15 | OPTIONAL_TLV | 支援已協商的可選欄位擴充 |

Capability表示「功能存在」，不是「當下有電/已授權/時間有效」；runtime另看StatusFlags。
未知capability bit在相同stable major可忽略，不能因此整包失效。
沒有完成量測演算法就不能設BATTERY_PERCENT；Sample能力不能搬到真實DeviceInfo冒充。
不設「KC-specific」bit，register差異只留transport診斷。

## 17. Protocol Versioning

**CONFIRMED 現況**：v0.1 codec固定Version01，會拒絕未知version、未知status與尾隨payload；
不具備讀v0.2的能力。現有真實Pebo transport也未綁定。不能承諾目前App自動向前相容。

**PROPOSED** 一個wire version byte編碼major/minor：高4bit=major，低4bit=minor。
`01`=experimental0.1，`02`=experimental0.2，`10`=stable1.0；**0.x之間可以breaking**。
這只定義將來格式，沒有把目前01 App就地升版。

| 對接 | 行為建議 |
| --- | --- |
| 新App遇v0.1 | 只啟用獨立legacy decoder與明確支援的唯讀功能；沒有授權不可降級寫入 |
| 新App遇v0.2 | 協商profile02/MTU/auth，走新codec；不把v0.1 payload強塞進新header |
| 舊App遇v0.2 | 目前會拒絕；若FW另提供經測試的legacy profile才可唯讀退回，否則提示升級 |
| 新App遇未支援major/v1.0 | 先讀descriptor/版本資訊，不猜測解析；提供明確unsupported |
| 未來同stable major較新minor | 固定prefix不改；可忽略optional TLV，未知critical TLV拒絕有副作用命令 |

**PROPOSED** discovery descriptor提供supported profile list，active DeviceInfo回本次協商結果。
初期若legacy安全語意不成立，不必為「相容」同時提供不安全寫入；capability不是強制開門。
Stable1.x預留TLV格式：type:u8（bit7=critical）、length:u8、value；unknown optional跳過、
unknown critical回NOT_SUPPORTED且不執行。新增可選TLV/新command/capability可minor；
改既有欄位位寬/offset/CRC、單位、語意、認證要求或mandatory欄位需major，或另command+能力協商。
Stable major/minor分配耗盡時再新major，不跨越重用。各profile需保有獨立golden vectors。

## 18. Recovery Behavior

以下皆為 **PROPOSED可驗收規則**，尚未HIL：

| 故障點 | 必須保留／回報 | Recovery |
| --- | --- | --- |
| 半份RF/FTM request | 舊設定與已提交歷史 | 丟stage；新靠近可重送同身份 |
| 長度/CRC/MAC失敗 | 不產生副作用 | 明確本地/remote錯誤；不能當成功 |
| config body寫到一半掉電 | 舊有效slot | boot掃slot；不使用partial新版 |
| config已commit、ACK遺失 | 新rev與OpId結果 | 重送原OpId回原結果，不再寫 |
| CLEAR後ACK遺失 | 已提交新HistoryEpoch | 回原epoch；不得再次clear之後新增records |
| RTC變更結果不確定 | Recovery flag，不假造正確時間 | QUERY_OPERATION；新鮮且授權的time recovery |
| snapshot期間新增<=reserve | pinned舊集合與durable新records | 新records不加入舊snapshot |
| snapshot第5個reserve需求/TTL/boot/clear | 明確STALE_SNAPSHOT | 取消完整性宣稱，重新OPEN；不悄悄漏記 |
| record提交前斷電 | 既有完整records；新record可能未commit | 跳過壞slot/已預留ID，回報可能gap，不把壞資料補成0 |
| NVM雙slot都壞/高水位不一致 | RECOVERY_REQUIRED | safe/read-only，不繼續接受破壞性寫入 |
| FTM watchdog放開舊message | 舊bytes不是新response | session/seq驗證，有限retry；不能以free=ACK |
| 無電池/MCU不可用 | 被動最後摘要（若有效） | active明確不可用；不承諾最新量測 |
| authenticated flood/長期RF場 | 功耗受限、寫入仍受保護 | session截止/cooldown；無DoS免疫承諾 |

保留上一版不等於保留任意多版。設定更新generation到上限或journal耗盡，明確維護；
不靜默factory reset。Factory reset/ownership轉移是另一項受控流程，OPEN/P0，不借CLEAR_HISTORY代替。

## 19. Open Questions / 人工 review gate

| ID | Status / Priority | 決策問題 |
| --- | --- | --- |
| O01 | OPEN P0 | 確切板上料號、LPD High下的RF/GPO喚醒、FTM供電及active功耗是否可接受？ |
| O02 | OPEN P0 | 採FTM命令+被動摘要的分工嗎？descriptor/area protection/物理map由誰定稿？ |
| O03 | OPEN P0 | 接受header18、session、OpId、CRC/MAC與MTU變化的breaking profile02嗎？ |
| O04 | OPEN P0 | ACK=durable commit、RTC不確定結果與NVM power-fail門檻/驗證流程如何定？ |
| O05 | OPEN P0 | 只需12筆斷電history嗎？若不是，需增加NVM或調整產品保留規格。 |
| O06 | OPEN P0 | DayToken/travel/DST過期策略、DailyTarget為完成時段數、LED公式是否符合產品？ |
| O07 | OPEN P0 | per-device秘密的製造/交付/遺失/復原、公開資料範圍與是否需要機密性如何定？ |
| O08 | OPEN P0 | 32KB Flash/8KB SRAM/1KB data EEPROM的完整產品資源預算與降級方案？ |
| O09 | OPEN P1 | Snapshot30s/4 reserve是否足夠？如何注入高事件率與RF離場驗證？ |
| O10 | OPEN P1 | 每款手機有效MTU、FTM watchdog、wake/response等待與重試初值？ |
| O11 | OPEN P1 | stable兼容協商/legacy唯讀是否值得維護？跨語言CRC/MAC/packet vectors由誰簽核？ |
| O12 | OPEN P1 | config變更後舊PeriodID的歷史語意是否需要revision archive？ |
| O13 | OPEN P2 | parent account、多授權key、撤銷與較強secure hardware/AEAD profile何時加入？ |

**PROPOSED 開工門檻**：先review所有P0並記下accept/reject/alternative，再建立firmware
分層規格；P1在真正NFC整合前完成。未定稿不把本文件的欄位編碼寫入既有v0.1。

## 20. STM32L031 RAM / Flash / NVM Cost

**CONFIRMED 文件資源上限**：STM32L031x6為32KB program Flash、8KB SRAM、1KB data EEPROM；
不是32KB都可給新增protocol。來源：[DS10668](https://www.st.com/resource/en/datasheet/stm32l031g6.pdf)，
Memories章節。實際完整產品link map、stack high-water與NVM driver尚未驗證。

以下全為 **PROPOSED工程估計，不是量測或編譯結果**：

| 元件 | 新增RAM約 | 新增program Flash約 | Persistent/其他 |
| --- | ---: | ---: | --- |
| framing/CRC/LE/固定長度parser | 256B共用work buffer+少量state | 2～4KB | 無 |
| 完整最後response cache | 256B | 含於session邏輯 | volatile |
| 4筆compact mutation cache | 256B | 0.5～1.5KB | 最後結果另存NVM |
| config/time policy staging | 140～224B | 1～2KB | A/B slots約448B |
| history cursor/pin/append scratch | 64～128B | 1～2KB | 16實體slots約320B（含epoch），正常12+reserve4 |
| auth keys/nonces/AES工作區 | 128～320B | 3～6KB | device key/identity區，另有journal |
| A/B recovery journal scratch | 128～224B，可與work共用 | 1～2KB | 兩份約256B（需精算） |
| I²C/transport glue | 32～128B | 1～2KB | ST mailbox256B不是MCU SRAM |
| stack/ISR安全餘量 | 至少另估512～1024B | 不適用 | 必須實測 |

多項Flash功能共用；**總新增Flash粗估8～16KB，RAM含stack粗估2～3KB**。
不能把各列最大值當成已精算link map。既有某些工程sketch已用約25KB Flash，
那不是正式產品baseline，也不能直接疊加16KB後宣稱會放得下。若沿用大框架/debug庫，
32KB很可能先超限；review時需準備精簡framework/crypto或換資源較大MCU的決策。
不推薦RTOS、整套tzdb、JSON、TLS/X.509、全history RAM副本或多session並行。

**PROPOSED 內部data EEPROM壓力試算**：
`config/policy 2*224 + journal 2*128 + ring 16*20 + identity/key 64 = 1088B`。
這已**超出1KB**，再加day checkpoint A/B（估2*32B）約為1152B；不是可直接採用的實際address map。slot欄位、CRC/markers、
time intent、三種mutation結果與ID high-water的精算可能超出。必須先壓實儲存格式或
減少retention/其他需求，否則增加NVM；不能把需要的metadata排除預算以假裝能放。
若選§12.3的RAM-only A，移除record NVM約320B後約832B，但另外增加RAM、且不提供掉電History保留。必須讓產品明確接受，不可用來冒充B已可行。OPEN/P0 O05/O08因此是開工阻擋項。歷史至少12筆的建議是小容量候選，不是已核准產品規格。

Tag EEPROM預算為64+96+96+256=512；不在那裡放root key、正式history或兩份packet。
NVM磨耗預算需分開計config提交/RTC校時/boot epoch/high-water/record寫入：
record每事件一slot、allocator每32事件預留、其他只在狀態改變時寫；hot cell與溫度先核對
再算壽命。不得將大容量program flash當無限EEPROM或據此宣稱CR2032六個月達成。

## 21. Android v0.1 實作與文件核對

**CONFIRMED 檢查範圍**：直接讀取目前Wire/Models/PayloadCodec、Sample/transport及原始
`protocol-user-request-v0.1.txt`；本輪未重跑58測試、未安裝或操作手機。

| 項目 | 核對結果 | v0.2處理建議 |
| --- | --- | --- |
| magic/version/command、DeviceInfo17、Status20、Config/record基本offset | 未發現與原始已明定欄位不一致 | 保留語意，breaking欄位另profile |
| Payload Length可編碼255，但實作限制248 | 是256整包上限的正確推導，不是少實作7bytes | v0.2按新header/auth重算 |
| CRC「CCITT」 | 原需求不足以唯一決定；程式採FALSE/LE/coverage，文件標暫定 | review後才定稿，不把舊假設升CONFIRMED |
| GET_CONFIG status、PING/ACK、History prefix | 是原需求未明說部分的補足，已有暫定標示 | 本草案明訂候選；尚未批准 |
| Config ID唯一、Reserved0、End1440、zero periods | 程式加了明確驗證；原始未完整規範這些邊界 | 需接受或修訂規格，不屬已證實的offset bug |
| Packet Version只接受01、status只識別00..06、拒絕尾隨payload | 符合目前嚴格v0.1，不提供forward compatibility | 不能靠換常數支援02；另codec/協商 |
| 真實packet transport明確unbound | 符合「不能假裝firmware已完成」 | 保留，直到P0/P1決策完成 |
| Sample today count/LED固定、clock不自行tick、Clear只清模擬history | 不是真實firmware行為測試；文件已有Sample限制 | 不把Sample語意當裝置規範 |
| Sample個別FF PeriodID可同時出現completion flag | 若產品要求completion一定對應period，Sample語意需在未來修正 | v0.2建議明確拒絕，這輪不改Sample |

結論：未發現已明定v0.1欄位的確定offset/endianness實作偏差；有多項未定義時的補充規則，
以及Sample不是完整行為模型的限制。不能說v0.1已解決真實連線、原子性或安全性。

## 22. Review / Validation Plan（不執行code）

**PROPOSED acceptance**：人工簽核前至少review下列情境；之後才寫跨語言tests/HIL：
1. v01/v02/10互遇、未知major/minor/TLV、MTU縮小與MAC後剩餘空間。
2. config在每個寫入邊界掉電；commit前/後重啟；同generation衝突與雙slotcorrupt。
3. 同seq/op重送、不同body同OpId、丟ACK、boot後重送、cache超窗、CLEAR後新事件不再被清。
4. RTC已改但journal未完成、時鐘倒退、TIME_INVALID、DST spring/fall、政策過期、跨日travel。
5. snapshot有新事件、reserve第5筆、TTL、重啟/clear、同秒records、RecordID預留gap/rollover。
6. 未配對手機、錯MAC、側錄重播、未授權OPEN搶session、掉電重用session、RF耗電攻擊。
7. STM32 link/stack/NVM配置、寫入磨耗，以及sleep/wake/FTM/auth/snapshot的實測電流與電荷。

本草案沒有新增任何v0.2測試通過宣稱；58是既有v0.1/Sample證據。

## 23. Sources / Evidence Boundary

直接開啟核對的原廠文件：
- [ST25DVxxKC DS13519 Rev8](https://www.st.com/resource/en/datasheet/st25dv04kc.pdf)：
  p3、§2.2.2、§5.2 FTM、§6.4 EEPROM/FTM限制。
- [ST25DVxxK DS10925 Rev11](https://www.st.com/resource/en/datasheet/st25dv64k.pdf)：
  內文涵蓋04K/16K/64K，p3、§5.1 FTM/mailbox access、§6.4。
- [STM32L031 DS10668 Rev6](https://www.st.com/resource/en/datasheet/stm32l031g6.pdf)：
  §3.9 Memories。未藉此主張任意寫入granule在brownout下原子。
- NIST CMAC/KDF與IANA來源見§15、§11；其標準不等於本草案自訂session流程已通過安全審核。

本次上述三份manufacturer PDF均直接可讀，因此不需以受限PDF替代說法；未查到的精確
NVM/BOR/功耗/裝配條件均保持OPEN。所有地址分配、crypto流程、RAM/Flash估計與產品政策
都是作者提案，不是從datasheet推定成CONFIRMED的實際板行為。