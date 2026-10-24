# Pebo NFC Protocol v0.2 Minimal Draft

日期：2026-10-25。狀態：**PROPOSED，供人工 review；未實作、未取代 v0.1。**
本文件依新的簡化要求獨立提出，不沿用 Full Draft 的 session/OpId/crypto 架構。
配套：[Minimal Decisions](pebo-nfc-protocol-v0.2-minimal-decisions.md)。
原 [Full Draft](pebo-nfc-protocol-v0.2-draft.md) 保留為設計歷史，不與本方案混用。

狀態適用全文：**CONFIRMED**＝目前來源／已量測事實或本輪明確限制；
**PROPOSED**＝待接受的候選規則，包括 MUST 語氣與所有新 bytes；
**OPEN**＝尚缺產品決策、原廠條件核定或實測。文件存在不等於批准。

## 1. Design Goals

**CONFIRMED**：STM32L031G6 為32KB Flash、8KB SRAM、1KB data EEPROM；
既有 Android v0.1 為獨立 codec/models、Sample 與未綁定的真實 packet transport。
本輪只修 Android Status crash，不改 v0.1 wire/models、firmware、Gradle 或新增 dependency/UI。

**PROPOSED**：單一主動手機、一次一筆 request、固定 bounds、共用256B工作區、無heap配置。
優先使用位元運算CRC、簡單switch dispatch、固定兩份config，避免RTOS、TLV、session、
全量history快照、OperationId、持久transaction journal、crypto/database/framework。
成本與保證一同縮減：不宣稱跨重啟 exactly-once、任意離線期間的無損history或自動DST。

## 2. Minimal Architecture

**CONFIRMED 架構邊界**：Android App ↔ NFC-V ↔ ST25DV ↔ I²C ↔ STM32L031。
K/KC差異只在adapter；Pebo model不依賴IC register。PA4/ST25_LPD在既有deep sleep
維持GPIO Output High的量測規則保留；不得為FTM直接更改它。

**PROPOSED**：動態八命令走FTM/Mailbox；EEPROM只做identity、schema/recovery提示及
必要快取摘要。Android不得把RF可讀摘要當即時MCU或授權證明；無active MCU時不得假成功。
不把root secret、完整history或request/response雙buffer放tag EEPROM。
logical tag預算：identity/descriptor32B＋summary兩份各48B＋保留384B＝512B。
這不是實際address map；摘要需generation/length/CRC/commit完整檢查，MCU單寫者，
讀者讀commit/generation→body→再讀commit/generation，變動則重讀；非僅靠valid一位。
更新節流候選為6小時一次及明確sync/config變更，非每次drink更新；失效摘要標記stale。
FTM期間延後EEPROM摘要寫入；mailbox完整message驗證後才執行命令，單一request無分片。

**OPEN/P0**：現板料號、LPD/GPO/wake/active ready條件、FTM與EEPROM互斥；
**OPEN/P1**：手機MTU、timeout、錯誤復原、實際tag區域／保護邊界及睡眠／喚醒電荷量測。
ST原廠K/KC文件確認獨立256B半雙工mailbox；不由此推定本板可無電池執行動態命令。

## 3. Packet Format

以下為 **PROPOSED profile02**；目前程式仍使用01。

| Offset | Bytes | Field |
| --- | ---: | --- |
| 0 | 2 | Magic = 50 42 |
| 2 | 1 | Version = 02 |
| 3 | 1 | Command，response=request OR80 |
| 4 | 1 | Sequence u8 |
| 5 | 1 | Payload Length u8 |
| 6 | L | Payload |
| 6+L | 2 | CRC16 LE |

整包8..256B，L<=248；adapter實際MTU可更小，沒有fragmentation framework。
CRC候選延續CCITT-FALSE：poly1021/initFFFF/refin=false/refout=false/xorout0000，
涵蓋header+payload，CRC本身不含在內；check123456789=29B1。CRC不是授權或無碰撞證明。
整數LE、timestamp UTC u32、DeviceID8B不等於NFC UID，均延續既有語意。
FTM長度必須恰等8+L；拒絕截斷、尾隨bytes、未知command、錯version/CRC/長度。

## 4. Command Table

全部 **PROPOSED**；只沿用八個command，沒有新增command或更改目前implementation。

| Code | Command | Minimal request | OK response payload |
| --- | --- | --- | --- |
| 01 | GET_DEVICE_INFO | 空 | v0.1同形17B：status、ID8、HW2、FW3、profile1、capability2 |
| 02 | GET_STATUS | 空 | v0.1同序，TodayEvent由u8變u16；含status共21B |
| 03 | SET_TIME | UTC u32＋offset i16 | status-only ACK |
| 04 | GET_CONFIG | 空 | status＋config body |
| 05 | SET_CONFIG | config body | status-only ACK |
| 06 | GET_HISTORY | §10固定6B cursor request | §10 page header＋8B records |
| 07 | CLEAR_HISTORY | 空 | status-only ACK；只清history，非factory reset |
| 08 | PING | 空 | status-only ACK |

Config body候選：target u8、periodCount u8、enable u8、reserved u8=0、
offsetMinutes i16，再接periodCount筆既有6B period（id1/start2/end2/flags1）。
長度6+6N<=102B；GET_CONFIG最多103B。SET_TIME與SET_CONFIG更新同一份offset，
App先讀目前config，所有修改序列化；不另加revision/CAS，不能保護兩支手機並行編輯。
UTC offset限制-840..840分鐘；N<=16、ID唯一0..15、0<=start<end<=1440，
半開且不跨午夜／不重疊；enable0/1。target語意仍需§13決策。
GET_STATUS除event加寬外保持v0.1欄位次序；flags候選bit0 TIME_VALID、1 BATTERY_VALID、
2 LAST_DRINK_VALID、3 COUNTS_VALID、4 HISTORY_GAP、5 INSECURE_ENGINEERING；其他reserved。
無效欄位不得顯示成已量測值；未定義bit不可由App猜測。這些變動只屬02，不回填01。

## 5. Error Codes

**PROPOSED** 所有response以status u8起始；非OK只含status1B，不加新error envelope。

| Code | Name | Scope |
| --- | --- | --- |
| 00 | OK | 命令要求的效果已達成 |
| 01 | INVALID_COMMAND | 未知command |
| 02 | INVALID_LENGTH | 長度不符 |
| 03 | INVALID_DATA | flags/欄位組合或資料無效 |
| 04 | BUSY | 尚未執行；可在有限時間內原封重送 |
| 05 | CRC_ERROR | 收到可辨識header但CRC錯，無副作用 |
| 06 | INTERNAL_ERROR | 內部／儲存失敗；不能當可安全重送的保證 |
| 07 | UNAUTHORIZED | 缺寫入權限 |
| 08 | NOT_SUPPORTED | 已知但未支援的功能 |
| 09 | RETRY_CONFLICT | 有效cache內同sequence卻不同command/length/payload |
| 0A | OUT_OF_RANGE | 範圍／cursor過舊或歧義，須重新讀取 |

不新增DUPLICATE/RETRY/STORAGE_FULL等。無可信header的packet可直接丟棄，不回錯誤sequence。
OUT_OF_RANGE不區分所有細因，App提示重新同步而不謊稱完整。unknown version在host顯示不相容。

## 6. Retry / Deduplication

**PROPOSED 最小快取**：只快取最近一筆已完成的SET_TIME/SET_CONFIG/CLEAR_HISTORY。
資料為valid1、lastCommand1、lastSequence1、lastResult1、lastPayloadCRC2、
lastPayloadLength1、reserved1＝8B，加32-bit monotonic expiry＝12B RAM。
保存完整response不必要：mutation ACK只有status，重建header/CRC即可。讀命令不覆蓋此cache。

同sequence時先檢查command/length/CRC：不同回09、不執行、不覆蓋cache；相同則先作
必要的payload equality核對，再回前次status、不重做副作用。不同sequence視新操作，
成功後覆蓋上一筆。busy/crc/unauthorized/validation錯誤不搶占成功cache。
cache在該筆副作用完成後、發布response前建立；尚在執行中不得重入，回BUSY或等待。

**CRC限制與最小可靠建議**：相同CRC不等於完全相同payload。若只保存上述12B，
只能提供機率式fingerprint去重，不能承諾所有不同payload必定回09。
候選補強不需第二份102B cache：SET_TIME額外保留原始6B（總18B）；
SET_CONFIG與目前已提交的canonical config逐byte比對（最多102B，重用現有配置）；
CLEAR_HISTORY payload固定空。加入此精確檢查後才使用「完全相同」保證。
若另一成功mutation改了config，前筆cache已被覆蓋，因此不會誤比過時config。
**OPEN/P0**：接受12B的碰撞限制，或採18B＋既有config比較；推薦後者，禁止CRC當安全驗證。

Sequence維持u8足夠配對單筆在線request，**不足以識別跨App／跨重啟的transaction**。
候選規則：只允許一支主動手機、一筆outstanding；逾時最多2次原封retry，
總retry窗口<=5秒，cache TTL10秒（monotonic clock，數字待HIL；計時須涵蓋STOP睡眠，不能直接依賴停走的HAL tick）。保留一筆即可，
不得在未解決A結果前送B後再重試A；多筆cache增加成本卻無法解決跨重啟。
counter可0..255循環，但TTL內不得將last mutation sequence用於新mutation，
即使中途已讀256次也需跳過它；新App不知道前序號時先讓RF/FTM安靜>10秒並清理舊mailbox，
讀回狀態後才接受新的使用者寫入意圖。這是操作限制，不是session安全隔離。

掉電、cache過期、RF長中斷或App重建後，**不自動重放未確認的mutation**：
SET_CONFIG先readback；SET_TIME重新取得現在UTC並要求新同步；CLEAR_HISTORY結果未知就
提示使用者，不自動再清（期間可能已有新事件）。RAM去重不持久化，不能保證exactly-once。
不同手機可破壞此流程，prototype限制單操作者；量產保護見§14。CRC不防replay。

## 7. Config Atomic Update

**PROPOSED 比較**：

| 方案 | NVM | 掉電保護 | 複雜度 |
| --- | ---: | --- | --- |
| A 單header/payload/CRC/commit | 約112B | 可辨識新資料未完成，但覆寫後舊版已可能消失 | 最少，但不滿足保留上一版 |
| B 固定two-slot | 2*112=224B | 寫inactive、最後commit，active保留 | 一個選slot函式，無通用framework |

推薦B。每slot明訂112B：generation u16、schema u8、length u8、
payload102B（不足補0）、CRC16 2B、獨立對齊commit word4B。
CRC涵蓋offset0..105（gen/schema/length/padded payload）；CRC在106..107，commit在108..111。
流程：驗完整request→驗config→將inactive marker無效化並readback→寫header/body/CRC→
readback驗證→最後寫commit常數→再次驗證→切RAM active→建retry cache→ACK。
commit常數與擦除狀態不同；marker完整相等、schema/length/CRC皆有效才採用。
不假設word在任意brownout下天然原子；兩slot、readback、電壓門檻及fault injection共同驗收。
不要在舊marker仍有效時先改body；更新失敗保留原active。兩份都壞時回invalid/default提示，
不得靜默factory reset。generation modulo16比較：diff1..32767較新，0同內容可接受，
0但內容不同或diff32768視損壞待修復，不任選。
**OPEN/P0**：實際EEPROM programming/erase流程、BOR/PVD門檻及commit marker值；未核定前不實作。

## 8. Time Handling

**PROPOSED**：RTC與Drink Record均保存UTC u32；提醒用local minute-of-day。
localSeconds=UTC+offsetMinutes*60，以足夠寬的中間值計算，local日期與minute一起取得。
改offset只改排程解釋，不重寫歷史timestamp。App顯示歷史UTC／選定顯示時區，
不宣稱能還原「事件當時所在國家的本地時間」（8B record未存offset）。

V1最簡DST：App在使用者靠近並同步時更新UTC/offset，不放tzdb/transition engine。
不在附近跨DST的裝置會繼續舊offset，提醒可能差1小時；不以手機背景活動假裝已完成NFC同步。
home-zone或跟隨旅行由使用者選擇，App不能默默把手機新國家offset寫回。
同步改local日期／時間倒退時，不自動重算舊完成事件；本候選將當日counts標invalid，
下一個正常local午夜重新開始；需產品確認。沒有可靠RTC時不做時段完成判定。

SET_TIME同時涉及config offset與RTC，無法由two-slot保證跨元件原子。
候選先提交offset再寫RTC，只有兩者成功才ACK；中斷回結果不明，重啟TIME_VALID清除，
要求新同步，不自動套用過時UTC。保留元件可驗證的reset情境可後續優化，不先建journal。
**OPEN/P0**：接受重啟／校時後當日計數暫無效的簡化語意；若要求完整恢復須另估成本。

## 9. Drink Record

**PROPOSED wire record固定8B**：offset0 RecordID u16 LE，2 UTC u32 LE，
6 PeriodID u8（0..15或FF），7 flags u8。既有drink/completion/reminder/test bits0..3保留；
bit4 UTC_VALID候選，bits5..7=0。沒有有效UTC時timestamp0且bit4=0，不把0當真的1970事件。
RecordID每新增完整事件遞增，允許缺號；FFFF→0000。
同epoch比較：d=(a-b)&FFFF，d=0相等、0<d<8000為a較新、d=8000歧義、其餘較舊。
只在已證明距離<32768時適用；ring最多256筆並不代表離線手機的舊cursor也在此窗口。

**避免歧義的必要小metadata提案**：HistoryEpoch u16放page metadata與response header，
不逐record增加bytes；每次ID rollover、CLEAR或無法復原counter時持久化遞增epoch。
唯一鍵(DeviceID,HistoryEpoch,RecordID)，不是timestamp；epoch不作snapshot或session。
禁止同一key重用；epoch自身繞回時要求重新建立history身分／完整重同步，不承諾無限唯一。
換電池後counter若可由log復原可延續，否則先提交新epoch再開log。
**OPEN/P0**：若堅持完全沒有epoch，只能承諾半範圍內短期去重，不能做任意離線長期DB主鍵。

## 10. History Pagination

**PROPOSED**：以cursor取代index，不建立snapshot buffer、pin、reserve事件cache或新command。
GET_HISTORY request6B：cursorEpoch u16、afterRecordId u16、requestedCount u8、flags u8。
flags bit0 START；START=1時忽略cursor，從oldest開始；其餘bits0、count1..255。
START避免拿RecordID0或FFFF當永遠不可使用的sentinel。

OK header13B：status1、oldestKey4、newestKey4、pageEpoch2、returnedCount1、flags1。
key為epoch u16＋id u16，LE；接count筆8B records，單page只含同epoch。
flags bit0 EMPTY（oldest/newest為0且非有效key）、bit1 MORE、其餘0。
非空回應count>=1，或cursor等於newest時count0/MORE0；count不得超出requested及MTU。
256B packet最多floor((248-13)/8)=29筆；effective MTU240時最多27筆；
v0.1為6B record/40筆，此處不混稱32筆。response epoch可能跨到下一個epoch，仍按key遞增。

一般同步不需凍結資料：每頁一次短臨界區讀metadata，從Flash串流進共用TX buffer，
讀頁期間禁止log erase/append改動所讀範圍，送完後放開，不跨RF等待持鎖。
新event不移動ID，下一頁不會因index位移重複；未同步先遭覆寫的資料不可恢復。
cursor比oldest的前一key還舊、epoch不存在／歧義就回0A，App標記gap後START。
允許因ID缺號造成較少筆，但不可將讀不到的毀損record當完整同步。
本方案是live cursor遍歷，**不承諾整次同步是同一時刻snapshot**。App可記首頁newestKey
作本次截止、忽略更晚資料；若途中覆寫截止前資料，仍必須報gap而不是假完整。
Android長期DB是未來工作，本輪不加入database。

## 11. History Persistence

**PROPOSED**：config/metadata放data EEPROM，history候選放program Flash circular log；
ST EEPROM不放完整history。不是直接認定Flash適合：必須通過link reserve、供電和fault測試。

| 保留目標 | 純8B record | 僅算raw的128B頁數 | 建議保守physical reserve |
| --- | ---: | ---: | ---: |
| 64 | 512B | 4 | 12頁=1536B |
| 128 | 1024B | 8 | 21頁=2688B |
| 256 | 2048B | 16 | 39頁=4992B |

physical候選：每128B頁header16B，record實體slot16B＝wire8＋CRC16 2＋reserved2＋
commit word4；每頁7筆。預留ceil(N/7)+2頁，含回收／輪替裕量，不能把1024B當完整log預算。
最低N筆保留在穩態公開時成立，擦頁期間可BUSY；partial/corrupt資料不計入有效retention。
header16B候選：magic1/schema1/pageOrder2/baseKey4/reserved2/CRC16 2/commit4，
CRC涵蓋offset0..9、commit在12..15；baseKey按epoch2/id2欄位順序編碼，tuple比較而非比較raw bytes。
packing/pageOrder模數比較與counter預留仍屬OPEN/P0；若調整大小需重算表格，不能悄悄刪CRC。
單頁不可混epoch，rollover提前換頁也需裕量。
slot先寫body/CRC、最後獨立commit，重啟scan頁標頭/slots，跳過未提交與壞CRC。
頁標頭預留該頁ID範圍，含torn slot的ID不再重用；具體base/order packing須一起核定。
只回收最舊且已退出retention的頁，絕不erase目前唯一有效metadata/config；保留可用spare，
擦除中斷頁必須重新erase並readback才重用。CLEAR只提交新epoch/logical empty，舊頁延後回收，
不依賴一次擦完整log；ACK後新事件不能被舊CLEAR retry再次清掉（僅§6窗口內保證）。

**CONFIRMED 原廠可讀依據**：ST HAL定義Flash page128B、標準word program32-bit，
以page擦除；STM32L031 datasheet Table47列-40..105°C program endurance10k cycles，
125°C條件降為0.2k，不能無條件套10k。EEPROM為對應100k／2k，非無限耐寫。
L0不能套其他MCU的擦除值或任意反覆bit-clear假設；marker用不同word、每word一次寫，
具體erase值/word重寫/ECC/保護granularity以RM0377及選定driver核定。
**未直接核對受限 PDF**：本輪RM0377 web工具超過大小限制，官方下載亦失敗；
所以BOR下原子性、精確erase值及全部操作時序未宣稱已直接核對。以上page/word依官方HAL，
耐久與write/erase電氣條件依已直接讀取DS10668；見§20來源。

磨耗粗算非壽命保證：128方案19 data頁*7≈133 append才同頁一輪，
10k名目cycle對應約1.33M append量級；故障重試、boot、metadata熱點、溫度皆會縮短。
erase/write造成CPU取指stall/延遲、瞬時電流與CR2032壓降需量測；本候選用word write，
不先導入half-page快寫、RAM ISR搬移或複雜GC。保留區須從linker可用程式空間扣掉，
firmware update/ mass erase是否保留log另定，不能假設換firmware仍有history。
推薦先以64筆條件驗證，再考慮128；256先defer。若link/電源不合，history capability關閉，
或明確採RAM-only工程方案；不可把volatile方案當已達持久history需求。

## 12. NVM Layout Estimate

以下 **PROPOSED logical預算**，不是實際address，也不把C struct padding當wire。

| Durable data | 成本 | 內容／更新 |
| --- | ---: | --- |
| Identity | 32B | DeviceID8＋schema/CRC/commit/reserved；製造時寫，非從UID猜ID |
| Config A/B | 224B | 各112；包含16 periods、target/enable、timezone |
| Metadata A/B | 64B | 各32；generation、history epoch/ID保留上界、日期、count/bitmap/validity及CRC/commit |
| Optional security reserve | 32B | 只保留容量，未設計key protocol；工程模式不用可保留空白 |
| 合計 | 352B | 1024B中餘672B，非預先填滿 |

metadata32B可行候選：schema1/length1/generation2/epoch2/reserved2/
reservedNextKey4/localDay4/events2/completionBitmap2/stateFlags2/reserved2/CRC32 4/commit4。
ID上界分批預留（候選32），成功提交才使用；允許缺號，不逐事件更新同一counter cell。
Flash page base ID與NVM預留必須一致；重啟不得使用已可能曝光的舊key。
daily checkpoint不另建大journal：counts在無法證明完整replay時COUNTS_VALID=0，
不從僅64/128筆反推完整當日總數，不重發當日completion；下個正常local日重啟計分。
需要跨掉電完整當日計數是額外產品要求，不能由本352B預算自動推定已保證。
熱metadata雙slot不等於wear leveling；高事件率/頻繁重啟仍需估算或利用剩餘容量輪替。
identity低頻可單份；config要保舊版因此兩份必要；大history不做全量雙份。

## 13. LED Ownership

**PROPOSED → 建議升為 CONFIRMED，本文不代人工簽核**：MCU owns LED Progress；
App寫配置、讀回LED，不指定目前應亮幾顆。離線照常運作，邏輯只一份。
Counter候選：TodayEvent u16飽和65535，completed/total/target u8、LED u8(0..8)。
completed/total<=16、每local日每PeriodID至多完成一次，非lifetime累加器。
target若採完成時段目標0..16，LED=floor(8*min(completed,target)/target)，target0→0；
TIME/COUNTS無效→0並顯示invalid；事件數不直接轉完成時段數。
**OPEN/P0**：產品是否接受target指完成時段數、floor與target0語意；現有v0.1仍允許0..255，
不順便改model/validation。lifetime counter留P2，不放大所有欄位。

## 14. Security Scope

**PROPOSED 工程模式**：可有寫入，但裝置/工程工具必須明示 `INSECURE ENGINEERING MODE`，
cap SECURE_WRITE_SUPPORTED=0，非量產安全方案。本輪不新增Android標示/UI，因未實作02。
工程模式須明確開啟，不假裝UID/CRC/DeviceID是密碼；外人能干擾設定／時間／history。
P0只要求可表示07 UNAUTHORIZED及cap位；未授權拒絕時無副作用。
P1量產前選最小write protection與實測，否則不得出貨；ST EEPROM password保護不一定
等於Mailbox命令授權，須按實際K/KC路徑驗證。pairing secret可作候選，現在不設計crypto。
P2才做家長帳戶、多key/recovery/advanced security。CRC去重不防惡意重放，有限TTL也不防攻擊。

## 15. Capability Flags

**PROPOSED u16足夠**；只設真正已支援的bit，未知bit忽略但不能推出未知payload。

| Bit | Name |
| --- | --- |
| 0 | HISTORY_SUPPORTED |
| 1 | CONFIG_WRITE_SUPPORTED |
| 2 | TIME_SYNC_SUPPORTED |
| 3 | BATTERY_MEASUREMENT_SUPPORTED |
| 4 | LED_PROGRESS_SUPPORTED |
| 5 | SECURE_WRITE_SUPPORTED |
| 6 | PASSIVE_SUMMARY_SUPPORTED |
| 7 | FTM_SUPPORTED |
| 8..15 | reserved，送0 |

FTM_SUPPORTED表示Pebo能提供動態exchange，不是某register存在；History bit不代表無限保留。
工程不安全模式不設secure bit；cap描述能力，status validity描述此刻是否有效。

## 16. Versioning

**PROPOSED 選A**：單byte explicit profiles01/02，Android用獨立codec明確識別；
full草案也曾建議02但未實作，正式採用時只能選一個02 layout，不可兩者共用version。
同一version只容許不影響packet長度/意義的新optional capability與新optional command，
舊App忽略unknown cap、不發未知命令；未知命令回01，已知但未支援回08。
欄位加寬、插入payload、CRC參數／offset變更屬breaking，必須換profile。
major/minor各4bits可表達家族，但仍需相容表與parser，不為本小型裝置增加協商機制。
unknown profile拒絕解析，不能fallback用01猜；新App可保留01 readonly/工程功能。
本02的event寬度、record、cursor/config差異不可由現有v0.1App自動理解。

## 17. STM32 Resource Budget

以下全為 **PROPOSED 手寫固定buffer C實作的工程估算**，不是新增firmware編譯結果。
Flash為新增code；不含startup/HAL/framework、motion/RTC/buzzer/debug產品功能。
RAM為靜態上界候選；共用256B buffer只算一次，不把ST mailbox算入MCU RAM。

| Item | Flash code bytes | MCU RAM bytes | NVM／storage |
| --- | ---: | ---: | --- |
| Protocol parser | 256..512 | 8＋共用256 | 0 |
| CRC bitwise | 96..192 | 2 | 0；不使用512B lookup table |
| Command dispatch | 128..256 | 4 | 0 |
| Retry cache | 96..192 | 12，精確SET_TIME比較另6 | 不持久化 |
| Config handling | 512..896 | 104 | data EEPROM224B |
| History metadata | 192..384 | 24 | 共享metadata64B |
| History storage/driver | 512..1024 | 16 | Flash另1536/2688/4992B |
| FTM transport glue | 384..768 | 16 | 無，使用同一256B工作區 |
| Security placeholder | 16..64 | 0 | optional reserve32B |

Core＝parser/CRC/dispatch/retry/FTM/placeholder：**976..1984B Flash，298B static RAM**；
精確payload比較後304B。加入config/history基本管理總code **2192..4288B**，
static **442B**（精確比較448B），另估stack128..256B → **576..704B**含該部分stack。
這不含整個產品ISR/HAL stack高水位；仍需link map/stack量測，不能把704B當整機RAM。
core能達幾KB/幾百B是目標，若HAL glue生成更大應回報實值，不能刪驗證以湊數。
Core與總成本不同；Flash log佔用程式Flash另外扣，不能只報core隱藏storage。
128方案扣2688B後剩30080B可供所有code（尚未扣bootloader/其他保留）；
256方案剩27776B。現有25KB工程sketch不是產品baseline，不可直接宣稱放得下。

最吃code：config validation/recovery、Flash log、FTM錯誤處理；最吃RAM：單包256B與
active config104B。可defer：256筆、crypto/DST引擎、跨重啟交易保證、多client及database。
嚴禁為簡化估算忽略Flash stall、功耗、磨耗、commit失敗與history metadata。

## 18. Differences vs Full v0.2 Draft

以下 **PROPOSED 比較**，full與minimal皆非已核准實作；估算不是量測節省。

| Topic | Full Draft | Minimal | 節省／優點 | 代價 |
| --- | --- | --- | --- | --- |
| session | session64/auth狀態機 | 無 | header18→6B | 單手機操作限制，無session隔離 |
| sequence | u16不wrap | u8＋TTL限制 | 少狀態 | wrap/重啟須人工reconcile |
| OperationId | 128bit＋revision | 無 | 無持久Op journal | 不保證跨重啟只執行一次 |
| retry | response256B＋4-entry cache256B | 12/18B＋既有config比較 | 少約494..500B cache | 只保最後一筆與短窗口 |
| history snapshot | TTL/pin/reserve | live cursor | 無跨頁buffer/pin | overwrite可有gap，非原子snapshot |
| history record | 10B | 8B，epoch在頁header | 每筆少2B | 必須處理ID/epoch rollover |
| authentication | CMAC/nonces/key流程 | error/cap預留 | 無crypto code/workspace | 工程模式不安全；量產另補 |
| NVM | data EEPROM約1152B候選 | 約352B | 約少800B/69% | history移到program Flash，非免費 |
| RAM | 約2..3KB含stack | core約304B；含管理stack約576..704B | 同口徑候選少約66..81% | 不含全產品ISR/framework |
| Flash code | 新增約8..16KB | 全部約2.14..4.19KB | 候選少約48..87% | 另扣log空間；保證較弱 |
| complexity | session/OpId/auth/journal/pin | 固定slot/一筆cache/cursor | 模組與狀態數大減 | 無法用單一百分比證明工程工時 |

## 19. Open Questions

全部 **OPEN**，未勾選不得當CONFIRMED：

| ID | Priority | 必須決定／驗證 |
| --- | --- | --- |
| M01 | P0 | 接受限時去重、跨reset結果不明與單手機限制？採18B精確比較而非僅CRC？ |
| M02 | P0 | 64/128保留目標、epoch及Flash頁header packing／link reserve是否成立？ |
| M03 | P0 | config兩slot與metadata實際layout、BOR/PVD/commit driver／fault規則 |
| M04 | P0 | target／LED公式、校時及掉電後counts/time invalid的產品語意 |
| M05 | P0 | 唯一02 wire、cap/error、MTU bounds及CRC golden vectors簽核 |
| M06 | P0 | 現板LPD/GPO/FTM供電與喚醒策略（不可改壞PA4低功耗規則） |
| M07 | P1 | 真機MTU/timeout、RF/I²C競爭、NFC/Flash功耗、耐久／掉電fault acceptance |
| M08 | P1 | 出貨前最小write protection；未完成禁止用工程安全性出貨 |
| M09 | P1 | firmware更新保留history、tag摘要map與公開資訊privacy |
| M10 | P2 | advanced security/account、多client、自動DST、大容量history |

## 20. Recommendation

**PROPOSED**：以一筆cache、two-slot config、live ID cursor、64筆Flash log為最小候選，
通過容量／掉電／電流測試再升128；不先採256或session/OpId架構。
只用RAM12B去重不能完全保證payload相同，推薦18B＋既有config比較；
若要求跨重啟exactly-once或完整當日復原，應回到需求審查，不偷偷加framework。
P0決策未簽前可繼續獨立硬體bring-up，但不能宣稱Minimal協定已可可靠整合。
Android crash修復與v0.2設計分離；未改v0.1 models、packet offsets、command或firmware。

### Sources / Evidence Boundary

- [STM32L031 DS10668 Rev6](https://www.st.com/resource/en/datasheet/stm32l031g6.pdf)，
  memories、Table46/47：容量、電氣條件、耐久；本輪直接讀取。
- [ST官方HAL Flash header](https://github.com/STMicroelectronics/stm32l0xx-hal-driver/blob/master/Inc/stm32l0xx_hal_flash.h)，
  FLASH_PAGE_SIZE/FLASH_TYPEPROGRAM_WORD；[erase driver](https://github.com/STMicroelectronics/stm32l0xx-hal-driver/blob/master/Src/stm32l0xx_hal_flash_ex.c)：page erase。
- [RM0377官方連結](https://www.st.com/resource/en/reference_manual/dm00108282.pdf)：
  未直接核對受限 PDF，限制詳§11；不將search摘要當完整原廠審核。
- [ST25DV K DS10925](https://www.st.com/resource/en/datasheet/st25dv64k.pdf)、
  [KC DS13519](https://www.st.com/resource/en/datasheet/st25dv04kc.pdf)：本輪直接讀取mailbox及記憶體描述。
- 實作基線為Android PROTOCOL-v0.1.md與Wire/Models/PayloadCodec；v0.1補充規則仍為暫定。
  新實機Read Status格式化crash及本輪修正不改其wire；其他缺口保留在canonical handoff。
  本文件的Flash/RAM/NVM/retention/CRC安全性均為提案分析，不是firmware build/HIL或電池壽命證明。
