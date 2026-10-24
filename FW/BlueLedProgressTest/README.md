# BlueLedProgressTest

最新2026-10-25 UART補驗：使用者自行供電，COM5/115200已收到7/7正確實際回覆。
先前PC RX未通的阻礙已解除；首個help回ERR too_long，換行同步後help、8、off、
show(CRLF)、invalid9、過長輸入與最後off全通過。未操作PPK2/重設/燒錄。
證據tests/results/uart-manual-power-20261025-02/session.json；舊partial紀錄保留作歷史。
最終LED全滅、保留level8，供電由使用者控制，串口已關閉。

獨立八階藍燈測試，STM32L031G6U6TR／STM32duino 3.0.0；不含NFC、RTC、
飲水辨識、比例計分、紅燈、蜂鳴器或STOP整合。開機level0，四顆全滅，不自動播放。

## 接線與排列依據

| 本測試LED | GPIO | 電位 | PWM能力（安裝核心PeripheralPins.c） |
| --- | --- | --- | --- |
| LED1 | PA5 | High亮、Low滅 | TIM2_CH1 AF5 |
| LED2 | PA6 | High亮、Low滅 | TIM22_CH1 AF5 |
| LED3 | PA7 | High亮、Low滅 | TIM22_CH2 AF5 |
| LED4 | PA8 | High亮、Low滅 | TIM2_CH1 AF5 |

GPIO/有效電位依docs/hardware_pinmap.md與HardwareTest；2026-10-25使用者指定
先以PA5→PA6→PA7→PA8測試；同日第二次播放後使用者目視確認零階雙閃、低高亮與
水滴累積順序「全都正確」。此順序現已得到本板目視確認，非單憑GPIO編號推測。
既有LedBrightnessTest僅為參考，舊五階demo不沿用。

UART：板PA9 TX→轉接器RX、PA10 RX←轉接器TX、GND共地，115200/8N1、3.3V邏輯。
SWD沿用PA13/PA14/NRST/VTref/GND。串口/探針每次重新辨識，不照抄COM號。
不要同時用CR2032與PPK2供電；不要讓外部訊號反向供電。本次使用者明確授權PPK2供電，
確認電池移除、唯一電源與共地；腳本不量電流，結束或失敗均嘗試OFF並關埠。

## 操作

命令以LF結束，亦接受CRLF；只接受以下小寫命令或單一數字：

| 命令 | 行為 |
| --- | --- |
| 0..8 | 儲存並顯示階數 |
| show | 再顯示目前階數，重新計時 |
| off | 立即熄滅，保留階數 |
| help | 中止效果、保留階數，列命令 |

1..8保持3000ms後全滅、保留進度；0的靜態圖案為全滅，查看時LED1低亮
120ms→滅120ms→低亮120ms→全滅。新有效命令取消舊效果；無效輸入不改狀態或計時。
輸入buffer16B，至多15B（含CR、不含LF）；過長整行丟至LF，僅回一次ERR too_long。
空行、負數、9、01、大寫、NUL等回ERR invalid。沒有動態String或blocking UART read。
回覆pattern用O/L/H表示四顆：OOOO、LOOO、HOOO、HLOO、HHOO、HHLO、HHHO、HHHL、HHHH。
0命令的LOOO是查看雙閃第一相，不是level0的靜態圖案。
UART只有命令回覆；256B固定TX queue配合availableForWrite，不因印字停住顯示狀態機。
loop每輪最多處理32個RX bytes，避免大量輸入餓死millis計時。

## PWM與低功耗注意

ProgressLogic.h集中參數：low25%、high100%、PWM1000Hz、hold3000ms、zero120/120ms。
這些是起始值；2026-10-25使用者已目視接受亮度差與零階雙閃，非光度／波形量測。
每階最多一顆低亮；該顆用TIM2或TIM22硬體PWM，其他高亮用GPIO High(100%)。
因此雖然PA5/PA8共用TIM2_CH1，仍能獨立顯示本規則；不使用analogWrite或PWM ISR。
high必須100%，有static_assert防止改成部分duty後產生共享通道衝突。
若未來要四顆任意獨立部分duty，須重評timer／軟體PWM；不能只刪static_assert。

每次開始PWM都強制setMode與指定pin AF，避免PA5↔PA8共用channel重啟時沿用舊pin。
allLedsOff停止TIM2/TIM22及通道，關閉PWM AF並把四腳改回Output Low。
無timer callback；不使用TIM21。TIM2與既有piezo test衝突，未來整合需另配。
本測試timer clock仍可開、UART/SysTick仍運作；不是低功耗或功耗驗收。
未來STOP前需檢查clock gating、NVIC/UART/back-power與時基恢復；PA4/ST25_LPD仍Output High。

## 建置與邏輯檢查

沿用HardwareTest板卡選項；ASCII build path避免工具路徑問題：

```powershell
$cli = 'G:\Google Drive\Application\Arduino-cli\arduino-cli.exe'
$fqbn = 'STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod'
& $cli compile --fqbn $fqbn --build-path "$env:TEMP\Pebo-BlueLedProgressTest-20261025" FW/BlueLedProgressTest
$gcc = "$env:LOCALAPPDATA\Arduino15\packages\STMicroelectronics\tools\xpack-arm-none-eabi-gcc\14.2.1-1.1\bin\arm-none-eabi-g++.exe"
& $gcc -std=gnu++17 -Wall -Wextra -Werror -fsyntax-only FW/BlueLedProgressTest/tests/LogicChecks.cpp
```

LogicChecks直接對正式ProgressLogic.h進行constexpr/static_assert求值，非另寫模擬版：
九種圖案、初始全滅、2999/3000ms、millis繞回、show延長、off保留、零階兩閃邊界、
新顯示取消、無效level、CRLF、分段輸入、NUL、過長丟棄與下一行恢復。
這些編譯期檢查不冒充目視亮度／真實PWM波形驗證。

## 實機驗證

tests/bench_verify.py沿用既有pyserial3.5／ppk2-api0.9.2及CheckedPPK2/OFF重試；
埠VID、PPK2 metadata及指定ST-LINK的DeviceID0x425先檢查，再燒錄新bin並verify/reset。
讀取STM32L031 GPIOA模式/ODR/AF、TIM2/TIM22 CEN/ARR/CCR，確認每階硬體路由、
25% duty、3秒後全滅與timer停止；另驗UART、off保留、錯誤／過長／分段輸入與取消。
ST-LINK讀取為hotplug，不量外部波形；每份register binary與完整命令回覆保存於results。

必須先確認本次探針、供電／電池／共地與UART接線，再填入實際參數：

```powershell
python FW/BlueLedProgressTest/tests/bench_verify.py --ppk-port COM7 --uart-port COM5 --stlink 56FF6C066688555210351387 --programmer 'C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe' --image "$env:TEMP\Pebo-BlueLedProgressTest-20261025\BlueLedProgressTest.ino.bin" --results FW/BlueLedProgressTest/tests/results/new-session --confirm-sole-supply --visual
```

命令會實際供電／燒錄，不能未辨識就照跑；results必須是新目錄，避免覆寫舊證據。
重測可加--verify-existing，比對Flash與bin完全相同後reset/run，不再燒錄。
若PC UART RX故障，診斷用--swd-replies搭配本機arm-none-eabi-nm.exe可從匹配ELF
找到MCU TX ring並核對命令回覆，同時繼續GPIO/timer驗證。這不是UART round-trip PASS，
結果明標PARTIAL_PC_UART_RX_UNVERIFIED；不能因MCU產生文字就宣稱電腦已收到。
診斷模式完成時exit2（非完整PASS）；正常模式需UART實際回覆全部通過才exit0。
--visual為PC依UART命令播放0..8、每階3.3秒，並非firmware開機自動播放。
最後送OFF不等於獨立確認物理VDD為0；本階段不量功耗。

目視驗收：確認0兩閃；1/2比較首顆低高；3..8確認只往下一顆累積、3秒熄滅；
中途輸入off後show應恢復階數，0雙閃中輸入8應立即改全高。
實際編譯／燒錄／測試結果以docs/handoff.md最新紀錄為準。

2026-10-25結果：build/constexpr檢查/flash verify PASS；17160B Flash、2352B static RAM。
bench-20261025-05以SWD核對全部八階GPIO、四個低亮位置25% duty、逾時timer停止，
以及42筆MCU命令回覆；PC UART RX始終未收到，所以整體為PARTIAL而非UART PASS。
第二次目視播放另存visual-repeat-20261025/session.json；使用者確認顯示全都正確。
現存問題集中在板PA9→轉接器RX→PC路徑，根因未定，不再以重插成功宣稱修復。
PPK2每個run/診斷/播放均finally送OFF並關埠，未測物理OFF電壓或功耗。
