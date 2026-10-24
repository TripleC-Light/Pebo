# Pebo NFC Protocol v0.2 — Decision Table

**狀態：Draft，人工 review；不是採用紀錄。**
主文件：[Pebo NFC Protocol v0.2 Draft](pebo-nfc-protocol-v0.2-draft.md)。
CONFIRMED只指已核對的目前基線；下表v0.2方案全為 **PROPOSED**，需決策處標 **OPEN**。
P0＝STM32 firmware開工前必須決定；P1＝真實NFC整合前必須決定；P2＝可延後。
尚無任何一列因文件存在就自動變成CONFIRMED。

| Topic | Current v0.1 | Risk | Proposed v0.2 | Priority |
| --- | --- | --- | --- | --- |
| 動態transport | interface+Sample；真實unbound | 容易把tag block read誤當MCU protocol | FTM收完整packet；EEPROM只做passive摘要；型號差異留adapter | P0 |
| LPD / wake / power | sleep PA4 High有板測依據；未驗證完整FTM session | active供電條件不成立，或常醒耗電 | 保留sleep規則；量測GPO wake→active→sleep；無ready只cache，OPEN | P0 |
| Tag memory map | 無正式產品配置；04K為512B | 雙256B packet已占滿，還沒metadata | descriptor64+snapshot96*2+reserved256；僅logical base，實際address/area protection OPEN | P0 |
| Internal persistent budget | 尚無產品NVM map | 1KB塞不下完整config/journal/day/history/keys | persistent候選約1152B超1KB；明選RAM-only A（不保拔電歷史）或精算/擴NVM的B，OPEN | P0 |
| Packet header | PB/version/seq8/length8，8B overhead | session與跨重啟身份不足 | profile02：18B header，seq16/session64/length16，完整新codec | P0 |
| MTU | application最大256，未做實體binding | WriteMessage overhead可能超手機單次上限 | adapter協商effective MTU<=256，建議先240；不先做fragmentation | P1 |
| CRC | 實作CCITT-FALSE但原始要求不完整 | STM32選不同init/coverage/endian | 採FALSE，header+payload+MAC，LE；ratify golden vectors | P0 |
| Atomic config handoff | 未實作 | 半包/掉電使配置損壞 | MCU單寫者、完整message驗證、inactive A/B+generation+CRC32+commit-last/readback | P0 |
| Commit ACK | Sample status-only成功 | received被當成persisted | mutation ACK帶OpId/revision，只在durable commit後回；RTC不確定另明示 | P0 |
| Sequence | 0..255循環，無持久去重 | retry造成重複副作用/誤配舊回覆 | session內seq16不wrap，1 outstanding；末包cache，舊窗外拒絕 | P0 |
| Mutation dedup | 沒有OperationId/CAS | 已執行但ACK遺失後再次SET/CLEAR | OpId128+digest+expectedRev，RAM4 compact結果，持久最後結果與QUERY_OPERATION | P0 |
| Error codes | 00..06 | 把duplicate/busy/格式/授權混在一起 | 保留00..06，新增UNAUTHORIZED/NOT_SUPPORTED/INVALID_STATE/OUT_OF_RANGE/STALE_SNAPSHOT/CONFLICT/STORAGE_FULL；不用DUPLICATE/RETRY | P0 |
| History record identity | 6B，timestamp不是唯一 | 同秒事件、校時倒退、重讀無法去重 | 10B record=RecordID4+UTC4+PeriodID1+Flags1；epoch在page，key=DeviceID/epoch/id | P0 |
| History pagination | start index/count；Sample前綴 | append/overwrite造成index移動 | 固定upper bound+RecordID cursor+30s pin+4 durable reserve；不足明確STALE | P0 |
| History retention | Sample85筆，不是NVM承諾 | 以為可離線存數週 | 12 retained+4 reserve；stock-hardware A為RAM，B為NVM但需解資源；不得混稱持久化，OPEN | P0 |
| Daily counts | events/completed等u8；兩個概念分開 | uint8 wrap、短history無法復原整日count | events升u16飽和；period/target/LED維持u8但非cumulative；day checkpoint/validity | P0 |
| Daily Target semantics | uint8、Sample允許0..255 | 喝水事件目標與時段目標混用 | 建議明定完成時段目標0..16，enabled時<=period count；產品確認OPEN | P0 |
| LED ownership | wire有0..8；Sample固定 | App不在就不能更新，邏輯分岔 | MCU按completed/goal計算，App只顯示；明定公式與invalid顯示 | P0 |
| UTC / timezone / DST | UTC+單一offset | 跨國/夏令時間使排程漂移 | UTC RTC/records，local minutes；最多4個轉換；原始local歷史若必需另加offset，OPEN；GET_TIME_POLICY | P0 |
| Day / time correction | 當日/重設/倒退未定 | 重複完成、歷史被改寫或重置 | 單調DayToken，travel/time-change部分日；UTC歷史不改，count validity明示 | P0 |
| RTC power-loss | 未實作 | 無法原子提交RTC與NVM | intent/journal+可驗證phase；不確定回INDETERMINATE，不重套過時timestamp | P0 |
| Write authorization | 無實機授權；Sample可改 | 陌生手機任意改瓶子 | 每台secret、QR受控交付、mutual proof/CMAC；ST password只輔助，無proof禁止量產寫入 | P0 |
| Ownership recovery / privacy | 無正式規則 | 丟手機、秘密外洩、公開兒童history | 定義claim/recovery/key保管；history需認證；MAC不等於加密，是否AEAD OPEN | P0 |
| Capabilities | uint16，bit未定 | 上層依賴KC寄存器 | 定義Pebo功能bits，runtime validity另列；不設KC-specific bit | P0 |
| Version compatibility | VERSION01嚴格拒絕新格式 | 改常數就破舊parser，或不安全降級 | 0.x明確breaking；major/minor nibble；新App雙codec，v1 optional TLV與critical拒絕 | P0 |
| Snapshot/public freshness | 沒有真正量測age/commit | 舊EEPROM被當即時MCU | A/B公開摘要帶generation/UTC/validity；節流、不每drink寫；cache明確非認證 | P1 |
| PeriodID歷史語意 | 沒有config revision關聯 | 改排程後舊ID被錯解 | baseline只顯示原ID；需完整舊排程解釋時另做context/archive，OPEN | P1 |
| Security/resource validation | JVM tests不驗證板級時序/密碼 | 高Flash成本、nonce或掉電漏洞 | golden vectors、故障注入、link/stack/功耗/NVMwear審核；不先上code | P1 |
| Lifetime counter | 未定義 | 挪用daily欄位造成溢位 | 如需另u32/u64與capability，非baseline | P2 |
| 家長帳戶/多key/撤銷 | 未實作 | 後續轉移權限困難 | authProfile/keyVersion留升級路；服務在App/backend，不塞TLS/JWT進L031 | P2 |
| 大容量歷史/高安全硬體 | prototype小容量 | 任意擴充超出硬體 | 另選NVM/MCU/secure element，保持上層model與transport分離 | P2 |

## Review record（全部待填，OPEN）

| Decision | 結果 Accept / Reject / Alternative | Owner | 驗收依據 |
| --- | --- | --- | --- |
| P0 packet/CRC/session/OpId | 待人工review | 未指定 | wire vectors與跨語言規格 |
| P0 memory/retention/NVM資源 | 待人工review | 未指定 | 真實BOM、NVM packing、link budget |
| P0 commit/RTC recovery | 待人工review | 未指定 | 斷電fault matrix與storage driver條件 |
| P0 timezone/day/LED/target | 待人工review | 未指定 | 產品情境與DST/travel案例 |
| P0 authorization/ownership/privacy | 待人工review | 未指定 | 製造/復原流程與security review |
| P0 sleep/wake/FTM | 待人工review | 未指定 | GPO/LPD/供電與電荷量測 |

此表只列建議與阻擋項，不授權修改v0.1或直接實作firmware。