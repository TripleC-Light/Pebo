# NFC data contract — draft requirements

This document defines desired phone-visible information, **not yet a final byte layout**. Confirm the selected ST25DV variant's usable memory and RF/I²C behavior against the manufacturer document and real board before freezing offsets.

## Phone read requirements

| Group | Fields | Status |
| --- | --- | --- |
| Identity | Device ID; protocol version; optionally HW/FW versions | Device ID required; representation TBD |
| Battery | Measured voltage (mV), estimated level/status, measurement age | Voltage required; percentage is an estimate |
| Drinking | Today's detected count, last detected event time, recent event history | Required; history depth TBD |
| Clock | Time validity, device time or last sync | Proposed for time-sync diagnosis |

The device detects likely drinking events; it does **not** measure water volume. Do not label event count as volume or a medically validated hydration goal. A “daily target” is a configurable product feature, not yet a fixed requirement.

## Format design decisions still needed

- NDEF payload for phone discovery and app handoff versus application-specific memory reads; test both iPhone and Android flows.
- Binary structure version, offsets, endianness, length, counter range, timestamp epoch, validity marker, and checksum/CRC if used.
- Memory allocation, write endurance, atomic snapshot/commit method, and behavior if phone reads during an MCU update.
- Whether extended history is on tag, in phone storage, or transferred over multiple reads. A generic C `struct` may include padding and must not define the wire format by itself.
- Authentication/access control for mutable commands. A public NFC Device ID should not be treated as a secret or proof of device ownership.
- Time-sync command sequence, stale-command detection, confirmation, and host/RTC clock tolerance.

## Candidate phone flow

1. Read a small, versioned status record with device ID, battery reading age/value, detected drink count, and last event.
2. Optionally read history using a documented layout.
3. If supported, write a versioned time/configuration request and read an acknowledgement after the MCU has processed it.

Avoid promising immediate fresh battery readings from a passive NFC read: the phone can read the latest MCU-published snapshot even while the MCU sleeps. The age field makes this visible.

## Validation

Use PN532 for repeatable reads, writes, corrupt input, interrupted writes, and version compatibility tests. Separately verify actual iPhone and Android apps and RF placement. Reader behavior and phone permissions differ; PN532 success alone does not establish mobile compatibility.

## 2026-10-15: Android discovery-only bench probe

Confirmed test scope: use an independent NFC-V ReaderCallback app before
implementing the still-draft Pebo payload. FLAG_READER_NFC_V with
FLAG_READER_SKIP_NDEF_CHECK polls ISO 15693 without NDEF probing. The probe
has no transceive, memory read/write, formatting, authentication or lock code.
It displays Tag.id bytes in Android-provided order and discovered technologies.
UID ordering here is a display convention, not the proposed device ID format.
NDEF is intentionally not enumerated; its absence does not show missing NDEF.
Discovery alone confirms an RF response, not chip variant, product identity,
user-memory readability, MCU freshness, clock sync or payload correctness.
Only the intended Pebo should be near the phone for physical attribution.
No application byte layout or NFC write/commit behavior is implemented here.

Independent local tool: C:/Users/echou/AppData/Local/Temp/Pebo-NfcProbe-20261015;
App ID com.pebo.nfcprobe20261015. Physical outcome recorded in handoff.md.
ST DS13519 Rev 8 (July 2024), pages 1 and 3 directly checked: ST25DVxxKC RF
uses ISO/IEC 15693 / NFC Forum Type 5. Actual fitted K/KC variant remains
unverified. Android docs checked for NfcV and ReaderMode/NDEF-skip behavior.
Sources: https://www.st.com/resource/en/datasheet/st25dv04kc.pdf
https://developer.android.com/reference/android/nfc/tech/NfcV
https://developer.android.com/reference/android/nfc/NfcAdapter

2026-10-15 physical result: PASS. Current-session NFC_V_DETECTED callback, 8-byte UID and NfcV technology recorded; user confirmed success on the selected Pebo target. Power/assembly conditions remain unverified; no data-memory operation. Evidence: temporary probe physical-detection.log; canonical details: docs/handoff.md.

## 2026-10-15: Superseding engineering scope — addressed Read Single Block

Existing temporary NFC probe now supports one user-armed Read Single Block.
This supersedes its earlier discovery-only implementation, not the draft Pebo
application data contract. No Pebo payload, status freshness or writes added.
Manufacturer documents directly checked: DS13519 Rev 8 (KC), p3 and
pp80-83/88-89; DS10925 Rev 11 (K family), p3 and pp75-78/83-84. Both 04K
variants use 128 user EEPROM blocks, 4 bytes each; engineering input is 0..127.
KC PDF: https://www.st.com/resource/en/datasheet/st25dv04kc.pdf
K family PDF: https://www.st.com/resource/en/datasheet/st25dv64k.pdf
The st25dv04k.pdf URL failed; the accessible official st25dv64k.pdf is the
same K-family DS10925 and explicitly includes ST25DV04K. No claim made from
the failed URL. Actual assembled K/KC variant remains unverified.

Command bytes without CRC: 22 20 [Android Tag.id, 8 raw bytes] [block, 1 byte].
Flags: Address=1, high data rate=1, Option/Select/Inventory/extension=0.
Display UID stays in raw Android order, never called Pebo Device ID.
Expected Android RX: 00 + exactly 4 data bytes; error: 01 + one error code.
Unexpected flags/empty/length mismatch are malformed, never success. App
shows entire application RX; stack handles CRC/framing. Zero/FF data valid.
Error code retained, known codes described; unknown code still TAG_ERROR.
No extra command, retry, NDEF check, password, lock or system/config access.
Source: https://developer.android.com/reference/android/nfc/tech/NfcV

Each Prepare arm is consumed by next callback before background I/O. Pause
cancels pending/running operation and closes connection; busy remains until
worker completes. Cancelled/stale completion cannot report success. Current
invalid/failure result is separated from explicitly historical last success.
Bounded 100 in-memory records with timestamp/block/TX/RX/result/error;
copy/clear; no new storage/network permissions. Formal data layout/commit
behavior is still TBD and outside this engineering memory read.

2026-10-15 final-build physical Block0 PASS: final APK update install Success; user-operated read, session1792039552199; full RX00 00 00 00 00, four data bytes00 00 00 00. readBusy=false,armedBlock=-1 afterwards. Evidence in original TEMP project/single-block-physical-result.log. Five repeats/interruption/invalid UI/background and Copy-Clear UI remain unverified;see handoff.

## 2026-10-15: User-specified Pebo Protocol v0.1 / provisional Android Sample

The user specified the PB/v01 binary envelope, eight commands, response OR80,
uint8 sequence/length, LE integers, UTC uint32, separate 8-byte Pebo Device ID,
and typed Device Info / Status / Config / History layouts. These replace the
older draft for those fields. Real transport binding remains undefined.
Full request and exact layouts: existing local Android project's
protocol-user-request-v0.1.txt and PROTOCOL-v0.1.md.

User explicitly accepted provisional choices for Android/Sample testing:
- CRC CCITT-FALSE: poly1021, initFFFF, non-reflected, xorout0000; covers header
  and payload, stored LE. Check123456789=29B1; independent PING vector:
  50 42 01 08 00 00 7A F4. This is separate from ISO15693 RF CRC.
- Status prefixes every response. Non-OK is status only; SET_TIME, SET_CONFIG,
  CLEAR_HISTORY and PING success are status-only ACKs. Empty PING request.
- History response: status[0], start u16[1..2], total u16[3..4], count[5],
  records[6..]. Packet max256 means payload max248 and at most40 records/page
  (254-byte packet). Request index/count and forward progress are checked.
- Periods are half-open [start,end), 0<=start<end<=1440; touching allowed;
  unique IDs0..15, count<=16. Zero periods/target permitted, reserved=0,
  enable0/1. Undefined status/capability/period flags stay numeric, not invented.
These are proposals pending firmware ratification, not ST manufacturer rules.

Pure Kotlin packet/model/payload package has explicit parse errors and no
Android/ST imports. Full-packet transport is separate from the ST block adapter.
Sample exercises all commands through real codecs; real protocol transport is
explicitly unbound and sends nothing. The engineering block reader is preserved.
UI labels Sample continuously, separates actual UID from Pebo ID, clears parsed
values on mode change, and includes Config/History/Sync Time/Debug/offline HEX.
Event count and completed-period count remain separate (Sample3 vs1).

Open: ratify CRC/ACK/History; define atomic EEPROM/mailbox placement, ownership,
commit/wake/ACK/timeout/retry/idempotency/freshness and write authorization;
History snapshots/index rollover/clear semantics; day/DST/time-validity rules;
uint8 count overflow; LED8 vs period16/target; flags/status meanings.
58 unit tests/build/signature PASS. Phone absent at install attempt; version3
not installed and no real MCU protocol data obtained. Canonical handoff has
artifact checksum, source paths and next steps.