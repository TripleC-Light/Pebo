package com.pebo.nfcprobe20261015.protocol

sealed interface ResponseBody
// Pebo Device ID is opaque, 8 bytes. Never substitute NFC UID.
data class DeviceInfo(val deviceId: List<Int>, val hwMajor: Int,val hwMinor: Int,
 val fwMajor: Int,val fwMinor: Int,val fwPatch: Int,val protocolVersion: Int,val capabilityFlags: Int): ResponseBody
// Counts are intentionally distinct: 3 drink events in one period => 3 and 1.
data class DeviceStatus(val deviceUnixTime: Long,val batteryMillivolts: Int,val batteryPercent: Int,
 val batteryStatus: Int,val todayDrinkEventCount: Int,val completedReminderPeriodCount: Int,
 val totalReminderPeriodCount: Int,val dailyTarget: Int,val ledProgress: Int,
 val lastDrinkUnixTime: Long,val deviceStatusFlags: Int): ResponseBody
data class ReminderPeriod(val id: Int,val startMinute: Int,val endMinute: Int,val flags: Int)
data class PeboConfig(val dailyTarget: Int,val reminderEnabled: Boolean,val periods: List<ReminderPeriod>,val reserved: Int=0): ResponseBody
data class DrinkRecord(val unixTime: Long,val periodId: Int,val flags: Int) {
 val validDrink get()=flags and 1 != 0
 val periodCompletionEvent get()=flags and 2 != 0
 val reminderWasActive get()=flags and 4 != 0
 val manualTestEvent get()=flags and 8 != 0
}
data class HistoryPage(val startIndex: Int,val totalRecords: Int,val records: List<DrinkRecord>): ResponseBody {
 val nextIndex get()=startIndex+records.size
 val hasMore get()=nextIndex<totalRecords
}
data object Acknowledgement: ResponseBody
data class PeboReply(val status: PeboStatusCode,val body: ResponseBody?)
sealed interface RequestBody {
 data object Empty: RequestBody
 data class SetTime(val unixTime: Long,val timezoneOffsetMinutes: Int): RequestBody
 data class Config(val value: PeboConfig): RequestBody
 data class History(val startIndex: Int,val requestedCount: Int): RequestBody
}
object ConfigValidation {
 fun validate(config: PeboConfig): List<String> = buildList {
  if(config.dailyTarget !in 0..255) add("Daily Target 必須是 0～255")
  if(config.reserved!=0) add("v0.1 Reserved 必須為 0（暫定）")
  if(config.periods.size>16) add("Period Count 不可超過 16")
  if(config.periods.map{it.id}.distinct().size!=config.periods.size) add("Period ID 不可重複")
  for(p in config.periods) {
   if(p.id !in 0..15) add("Period ID 必須是 0～15")
   // PROVISIONAL half-open intervals [start,end), end=1440 means day boundary.
   if(p.startMinute !in 0..1439 || p.endMinute !in 1..1440 || p.startMinute>=p.endMinute) add("時段需符合 0≤Start<End≤1440，不可跨午夜")
   if(p.flags !in 0..255) add("Period Flags 必須是一個 byte")
  }
  val sorted=config.periods.sortedBy{it.startMinute}
  if(sorted.zipWithNext().any{(a,b)->a.endMinute>b.startMinute}) add("Reminder Period 不可重疊")
 }
 fun requireValid(config: PeboConfig) { val errors=validate(config);ensure(errors.isEmpty(),ErrorKind.VALIDATION,errors.joinToString("；")) }
}