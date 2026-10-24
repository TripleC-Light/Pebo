# Pebo NFC Protocol v0.2 Minimal - Decision Table

日期：2026-10-25。**Draft；未批准、未實作。**
主文件：[Minimal Draft](pebo-nfc-protocol-v0.2-minimal-draft.md)。
CONFIRMED只用於本輪限制／已核對基線；下列新協定規則均PROPOSED，決策仍OPEN。
P0＝產品protocol firmware開始前必須決定；P1＝真實NFC整合／出貨前；P2＝可延後。
P0不阻擋與協定無關的獨立硬體bring-up。

| Topic | Recommendation | Priority | Status | Reason |
| --- | --- | --- | --- | --- |
| 本輪scope | 只修Status crash；Minimal文件獨立、不改01/firmware/dependencies | P0 | CONFIRMED（使用者限制） | 避免草案變成未批准實作 |
| Transport | FTM動態命令、EEPROM低頻摘要；型號只在adapter | P0 | PROPOSED / OPEN | 省寫入與packet EEPROM競爭 |
| LPD/wake | 保留PA4 High深睡規則，先確認GPO/FTM供電喚醒 | P0 | OPEN（既有深睡規則CONFIRMED） | 不以被動RF成功推定MCU可交換 |
| Packet/CRC | 6B header＋CRC16，profile02，CCITT-FALSE/LE候選 | P0 | PROPOSED / OPEN | 無session/多層header；正式02僅能選一種layout |
| Sequence | 保留u8，一次一筆，TTL內不重用last mutation seq | P0 | PROPOSED / OPEN | 無需u16，但不能保證跨重啟識別 |
| Retry | 一筆8B資料＋4B deadline；建議另6B SET_TIME精確比較、config逐byte核對 | P0 | PROPOSED / OPEN | CRC16有碰撞，18B仍是幾bytes等級 |
| Retry boundary | <=5s有限retry、10s cache候選；expired/reset結果不明需readback/新意圖 | P0 | PROPOSED / OPEN | 不承諾跨重啟exactly-once，CLEAR不得盲重放 |
| Atomic config | 固定112B two-slot，共224B | P0 | PROPOSED / OPEN | 單slot雖省112B，不能保留被覆寫舊版 |
| NVM | identity32/config224/meta64/security reserve32＝352B | P0 | PROPOSED / OPEN | 幾百bytes；需核定marker/BOR、counter預留與磨耗 |
| History ID | wire8B含u16 ID，page/response帶u16 epoch | P0 | PROPOSED / OPEN | 純ID16無法區分手機離線超過半range/一整圈 |
| Cursor | afterRecordId＋epoch、START bit；無snapshot/pin | P0 | PROPOSED / OPEN | 無額外snapshot buffer，但覆寫須報gap |
| Retention | 先驗64；128候選；256 defer | P0 | PROPOSED / OPEN | raw512/1024/2048與physical1536/2688/4992不同 |
| Flash circular log | 有條件採用，先核定16B頁標頭packing/slotCRC/commit/link reserve | P0 | PROPOSED / OPEN | 128B erase頁、程式空間與供電限制不能省略 |
| Time | UTC RTC/record、local periods、offset配置；DST由App靠近時更新 | P0 | PROPOSED / OPEN | 簡單但離線跨DST可能差1小時 |
| Reset/count validity | 不能證明完整恢復時counts/time invalid，下一正常日恢復 | P0 | PROPOSED / OPEN | 不增加持久transaction journal；須產品接受 |
| LED | MCU owns，App只顯示 | P0 | PROPOSED → 建議升CONFIRMED，待簽 | 離線工作且只有一份計算邏輯 |
| DailyTarget/width | events u16，其餘u8；target完成時段目標0..16與LED floor公式 | P0 | PROPOSED / OPEN | v0.1仍0..255，不在本輪改validation |
| Security表示 | 07 UNAUTHORIZED、SECURE_WRITE_SUPPORTED；明示INSECURE ENGINEERING MODE | P0 | PROPOSED / OPEN | 不把CRC/UID當授權，也不加入crypto |
| Error/capability/version | 只加07..0A；u16功能bits；單byte01/02 | P0 | PROPOSED / OPEN | 最少分支，不猜未知payload |
| MTU/timeout | 單packet<=256，手機實測上限優先；無fragmentation | P1 | OPEN | FTM buffer不等於手機可傳256B整包 |
| Power-fail/wear | slot/erase逐步fault注入、BOR/壓降/磨耗驗收 | P1 | OPEN | 還不能把commit marker視為任意掉電原子 |
| 最小write protection | 量產前按實際mailbox路徑驗證，否則不得出貨 | P1 | OPEN | ST EEPROM password不自動等於FTM命令授權 |
| Passive summary/privacy | logical32＋48*2＋reserved384；實際區域／讀權限待核 | P1 | OPEN | 不公開完整兒童history、不宣稱即時 |
| Firmware update | 明定保留／清除log與epoch策略 | P1 | OPEN | program Flash資料會受erase/upload流程影響 |
| Advanced features | account、多key、crypto架構、DST engine、256筆、多client | P2 | DEFERRED / PROPOSED | 不阻塞目前工程bring-up |

## Review Record

| Group | Accept / Reject / Alternative | Owner | Evidence |
| --- | --- | --- | --- |
| 去重／單手機／reset邊界 | OPEN | 未指定 | seq wrap、CRC碰撞、丟ACK、掉電案例 |
| History epoch／retention／Flash | OPEN | 未指定 | header packing、link map、page fault與wear |
| Config／time／daily／LED | OPEN | 未指定 | NVM格式、掉電矩陣、產品情境 |
| Wire／CRC／error／cap | OPEN | 未指定 | 跨語言golden vectors、長度與bounds |
| Power／FTM／安全 | OPEN | 未指定 | 現板喚醒電荷、手機MTU、出貨寫保護 |

## Current Android Evidence

Read Status crash已在來源以獨立顯示formatter修正；62 JVM tests（原58＋4新測試）
及assembleDebug PASS，詳Android handoff/evidence/pc02-20261025-status-fix。
這些是v0.1 Android修正證據，不是Minimal v0.2 firmware或RF驗收。
原Full Draft／決策表保留，無任何方案由此自動升為CONFIRMED。
