# Pebo NFC Protocol v0.1 — Android engineering implementation

Status: Android implementation + clearly labelled Sample transport. NOT evidence
of real MCU protocol compatibility. Original user requirement is retained in
protocol-user-request-v0.1.txt. User explicitly accepted provisional CRC/history
choices on 2026-10-15. These choices must be ratified before firmware integration.

## Architecture / source map

All within existing project/App ID com.pebo.nfcprobe20261015 (versionCode3).
- protocol/Wire.kt: pure Kotlin errors, LE, CRC profile, commands/status, packets.
- protocol/Models.kt: DeviceInfo, DeviceStatus, PeboConfig, ReminderPeriod,
  DrinkRecord, typed replies/requests, config validation. No Android/ST imports.
- protocol/PayloadCodec.kt: exact payload layouts, bounds and typed conversion.
- transport/PeboTransport.kt: full-packet interface, sequence/mapping checking,
  explicit unbound-real transport (no actual NFC packet traffic).
- transport/SamplePeboTransport.kt: in-memory simulator; all 8 commands pass
  through real encoder/decoder, same command/status/sequence/CRC checks.
- transport/St25NfcVBlockTransport.kt: legacy engineering ISO15693 block adapter.
- ProtocolActivity.kt: Device/Status/Config/History/Time/Debug engineering UI.
- MainActivity.kt: existing one-shot block reader/detection retained; button opens
  ProtocolActivity. Previous single-block source saved outside source sets.
- tests/protocol/ProtocolTest.kt:38 new tests; previous20 regressions retained.

Pebo application packets and ST ISO15693 frames/UID/registers are distinct layers.
No EEPROM/mailbox placement is invented. Real protocol mode always fails clearly
with missing-binding reason, not mock fallthrough. Existing engineering block
read is still the only actual NFC memory command (addressed Read Single Block).

## Agreed envelope / binary types

Header offsets:0..1=50 42(PB),2=01(version),3=command,4=sequence(uint8),
5=payload length(uint8);6..=payload;last2=CRC. Packet8..256bytes => payload0..248,
not255. Response command=request|80. Sequence checked and wraps modulo256.
All multi-byte payload numbers use LE. UTC timestamps uint32 use Kotlin Long
0..4294967295 (including beyond signed2038). Device ID is opaque8bytes, not NFC
UID. No displayed UID reversal is used in transport command construction.

## Provisional additions (NOT manufacturer/ST protocol facts)

1. CRC-16/CCITT-FALSE:poly1021,initFFFF,refin=false,refout=false,xorout0000;
   covers all header+payload bytes, excludes CRC; stored LE. Check123456789=29B1.
   PING request golden:50 42 01 08 00 00 7A F4, independently checked with
   Python binascii.crc_hqx(...,65535). Pebo application CRC is separate from
   ISO15693 RF CRC automatically handled by Android.
2. Status byte prefixes ALL responses, including GET_CONFIG/GET_HISTORY.
   Non-OK response contains exactly status only. SET_TIME/SET_CONFIG/
   CLEAR_HISTORY/PING OK replies are status-only ACK. PING request empty.
   Other read requests empty except GET_HISTORY. ACK/PING formats need ratification.
3. GET_HISTORY request:startIndex:u16LE,requestedCount:u8(1..255).
   Response payload:status[0],startIndex[1..2],totalRecords[3..4],returnedCount[5],
   records[6..],each6bytes. Maximum floor((248-6)/6)=40 records,254byte packet.
   Response start must match request; returned<=requested; start+count<=total;
   empty page allowed only at end, so pagination cannot silently stall.
   Sample85records exercise40/40/5 pages. No immutable-snapshot token yet.
4. Reminder periods use half-open[start,end),start0..1439,end1..1440,start<end,
   touching edges allowed,no crossing midnight/overlap. IDs unique0..15;count<=16.
   Zero periods and DailyTarget0..255 allowed. Reserved header=0;enable0/1.
   Period flag byte preserved because bit meanings not supplied. Drink record
   flags upper nibble reserved/rejected;PeriodID0..15 orFF accepted.
5. Battery Status,Capability Flags,Device Status Flags remain numeric/hex with
   undefined-semantics labels. No invented battery curve or status bit names.
6. App renders event count and completed-period count separately from wire;
   it never counts all drink records as completed periods. Sample demonstrates
   3 drinks/1 completed period. MCU must enforce per-period completion; Android
   does not infer history completeness/day boundaries or re-score events.

## Defined payloads implemented

GET_DEVICE_INFO OK:17bytes including status;8byteID,HW2,FW3,protocol1,flagsu16.
GET_STATUS OK:20bytes including status;UTCtimeu32,mVu16,percent0..100,
batteryStatusu8,eventCountu8,completedu8,total0..16,targetu8,LED0..8,
lastDrinku32,statusFlagsu16.Completed<=total but event count independent.
SET_TIME request:UTCtimeu32,timezoneMinutesi16 (current phone offset at sync time).
Config:target,count,enable,reserved,then count*(ID,startu16,endu16,flags).
History records:UTCtimeu32,periodIDu8,flagsu8. Exact lengths and no trailing
bytes enforced. Unknown command/status/version,magic,length,CRC,sequence,
direction,model validation each produce explicit ParseResult.Error.
No malformed packet is treated as success; parser fuzz tests cover random data.

## UI / source provenance

Main screen retains NFC discovery and Block0 engineering reader. New protocol
screen starts in SAMPLE and shows permanent Sample banners. Real UID/tech are
only from actual ReaderCallback,never generated from sample DeviceID. Mode switch
clears parsed values. Read Device Info/Status/Config/History and Next Page are
available;config rows edit ID/start/end/flags and daily target/enable. WriteConfig,
SyncTime and ClearHistory mutate ONLY Sample memory. No actual tag writes.
Debug shows actualUID/tech,full TX/RX application packets,command,status,CRC.
Offline pasted HEX parser is labelled offline and never updates device panels.
Packet operations use background executor;paused/cancelled generation does not
apply stale results. NFC reader mode disabled while paused. No new permissions
or dependencies (uses existingJUnit);no network/external storage.

Sample is a packet simulator,not a firmware behavioural model or validated
battery/count algorithm. Sample clears reset when Activity/process recreates;
real-mode controls are explicit unavailable calls,not silently successful UI.
No SyncTime success or Config write proves physical MCU operation.

## Validation / artifacts

58JUnit PASS=38protocol+14single-block+6read-gate;0failures/errors/skips.
Includes CRC golden,packet encode/decode/roundtrip,magic/version/length/CRC,
truncation/oversize,max248payload,unsigned/signedLE,model/config bounds,
history records/pagination,errorstatus,sequencewrap/mismatch,unbound transport,
Sample commands and2000random malformed buffers (multiple parsers per buffer).
Command:testDebugUnitTest assembleDebug --no-daemon --console=plain.
BUILD SUCCESSFUL19s,40tasks(17executed/23up-to-date). Signature verification PASS.
SDKXML/native-access/META-INF warnings unchanged and non-blocking.
No phone present at install attempt:APK built but version3 NOT installed/UI not
physically verified. Prior NFC discovery/Block0 PASS remains historical.
APK:app/build/outputs/apk/debug/app-debug.apk,916853bytes,SHA256
E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7.
Build log:protocol-build-test.log;test reports:app/build/reports/tests/testDebugUnitTest.
Same existingAGP9.1.1/Gradle9.3.1/Kotlin2.2.10/JBR25.0.3/SDK37.0/BT36.0.0.
Original process-only environment/rebuild instructions remain in README.

## Firmware integration blockers / questions

- Ratify CRC profile/coverage/storage order and all provisional response layouts.
- Define memory/mailbox mapping,ownership,size/alignment/padding,commit markers,
  request trigger,MCU wake,sequence/ACK,timeout,retry/idempotency,stale responses
  and coherent snapshot/CRC handling on interrupted concurrent RF/I2C accesses.
- Define authorization for SET_TIME/SET_CONFIG/CLEAR_HISTORY. CRC is integrity,
  not authentication;DeviceID/UID are not secrets or ownership proofs.
- History index/total can change while MCU appends/clears:define snapshot/cursor
  consistency,retention and index rollover. Current pagination validates one page,
  not multi-page snapshot freshness.
- Event count u8 overflows at255;define saturation/wrap or a versioned wider field.
  Clarify device-local day/reset rules,timezone/DST and invalid timestamp values.
- LED has8levels but configuration permits16periods and targetu8;define progress
  mapping,DailyTarget/PeriodCount relationship,BatteryStatus and flag bits.
- Clarify dailyTarget0,zero periods,End1440,reserved fields/error payload rules,
  periodID changes versus existing history and CLEAR_HISTORY impact on counts.
- Define protocol version/IC capability evolution and unsupported-command flow.
No application-specific choice above is claimed to be required by ST K/KC.