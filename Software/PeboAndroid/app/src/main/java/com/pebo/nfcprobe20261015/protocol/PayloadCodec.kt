package com.pebo.nfcprobe20261015.protocol

object PayloadCodec {
 private fun u8(value: Int): ByteArray { require(value in 0..255); return byteArrayOf(value.toByte()) }
 fun encodeConfig(config: PeboConfig): ByteArray {
  ConfigValidation.requireValid(config)
  return u8(config.dailyTarget)+u8(config.periods.size)+u8(if(config.reminderEnabled)1 else 0)+u8(config.reserved)+
   config.periods.fold(byteArrayOf()) { all,p -> all+u8(p.id)+LittleEndian.u16(p.startMinute)+LittleEndian.u16(p.endMinute)+u8(p.flags) }
 }
 fun decodeConfig(bytes: ByteArray): ParseResult<PeboConfig> = parseSafely {
  val r=Reader(bytes);val target=r.u8();val count=r.u8();val enable=r.u8();val reserved=r.u8()
  ensure(count<=16,ErrorKind.PAYLOAD,"Period count exceeds 16")
  ensure(enable in 0..1,ErrorKind.PAYLOAD,"Reminder Enable must be 0 or 1")
  val periods=List(count){ReminderPeriod(r.u8(),r.u16(),r.u16(),r.u8())};r.end()
  PeboConfig(target,enable==1,periods,reserved).also { ConfigValidation.requireValid(it) }
 }
 fun encodeDrink(record: DrinkRecord): ByteArray {
  ensure(record.periodId in 0..15 || record.periodId==255,ErrorKind.VALIDATION,"Invalid history Period ID")
  ensure(record.flags in 0..15,ErrorKind.VALIDATION,"History flags bits 4..7 must be zero")
  return LittleEndian.u32(record.unixTime)+u8(record.periodId)+u8(record.flags)
 }
 fun decodeDrink(bytes: ByteArray): ParseResult<DrinkRecord> = parseSafely {
  val r=Reader(bytes);val record=DrinkRecord(r.u32(),r.u8(),r.u8());r.end();encodeDrink(record);record
 }
 fun encodeRequest(command: PeboCommand, body: RequestBody=RequestBody.Empty): ByteArray = when(command) {
  PeboCommand.SET_TIME -> { require(body is RequestBody.SetTime);LittleEndian.u32(body.unixTime)+LittleEndian.i16(body.timezoneOffsetMinutes) }
  PeboCommand.SET_CONFIG -> { require(body is RequestBody.Config);encodeConfig(body.value) }
  PeboCommand.GET_HISTORY -> { require(body is RequestBody.History);require(body.requestedCount in 1..255);LittleEndian.u16(body.startIndex)+u8(body.requestedCount) }
  else -> { require(body==RequestBody.Empty);byteArrayOf() }
 }
 fun decodeRequest(command: PeboCommand, bytes: ByteArray): ParseResult<RequestBody> = parseSafely {
  val r=Reader(bytes)
  when(command) {
   PeboCommand.SET_TIME -> RequestBody.SetTime(r.u32(),r.i16()).also{r.end()}
   PeboCommand.SET_CONFIG -> RequestBody.Config(decodeConfig(bytes).required())
   PeboCommand.GET_HISTORY -> RequestBody.History(r.u16(),r.u8()).also{r.end();ensure(it.requestedCount>0,ErrorKind.PAYLOAD,"Requested count must be positive")}
   else -> RequestBody.Empty.also{r.end()}
  }
 }
 fun encodeReply(command: PeboCommand, reply: PeboReply): ByteArray {
  if(reply.status!=PeboStatusCode.OK) {require(reply.body==null);return u8(reply.status.code)}
  val body=reply.body
  val data=when(command) {
   PeboCommand.GET_DEVICE_INFO -> {
    require(body is DeviceInfo);require(body.deviceId.size==8 && body.deviceId.all{it in 0..255})
    body.deviceId.map{it.toByte()}.toByteArray()+u8(body.hwMajor)+u8(body.hwMinor)+u8(body.fwMajor)+u8(body.fwMinor)+u8(body.fwPatch)+u8(body.protocolVersion)+LittleEndian.u16(body.capabilityFlags)
   }
   PeboCommand.GET_STATUS -> {
    require(body is DeviceStatus);validateStatus(body)
    // Payload including status: time[1..4], mV[5..6], percent[7], status[8],
    // events[9],completed[10],total[11],target[12],LED[13],last[14..17],flags[18..19].
    LittleEndian.u32(body.deviceUnixTime)+LittleEndian.u16(body.batteryMillivolts)+u8(body.batteryPercent)+u8(body.batteryStatus)+
     u8(body.todayDrinkEventCount)+u8(body.completedReminderPeriodCount)+u8(body.totalReminderPeriodCount)+u8(body.dailyTarget)+u8(body.ledProgress)+
     LittleEndian.u32(body.lastDrinkUnixTime)+LittleEndian.u16(body.deviceStatusFlags)
   }
   PeboCommand.GET_CONFIG -> {require(body is PeboConfig);encodeConfig(body)}
   PeboCommand.GET_HISTORY -> {
    require(body is HistoryPage);validatePage(body)
    // PROVISIONAL response payload: status[0], start[1..2],total[3..4],count[5],records[6..].
    LittleEndian.u16(body.startIndex)+LittleEndian.u16(body.totalRecords)+u8(body.records.size)+body.records.fold(byteArrayOf()){all,r -> all+encodeDrink(r)}
   }
   else -> {require(body==Acknowledgement);byteArrayOf()} // PROVISIONAL status-only ACK incl PING.
  }
  return u8(0)+data
 }
 fun decodeReply(command: PeboCommand, bytes: ByteArray): ParseResult<PeboReply> = parseSafely {
  val r=Reader(bytes)
  val status=PeboStatusCode.fromCode(r.u8()) ?: throw ProtocolFailure(ErrorKind.PAYLOAD,"Unknown response status code")
  if(status!=PeboStatusCode.OK) {r.end();return@parseSafely PeboReply(status,null)}
  val body: ResponseBody=when(command) {
   PeboCommand.GET_DEVICE_INFO -> DeviceInfo(r.bytes(8).map{it.toInt() and 255},r.u8(),r.u8(),r.u8(),r.u8(),r.u8(),r.u8(),r.u16()).also {
    ensure(it.protocolVersion==1,ErrorKind.VERSION,"Device reports unsupported NFC protocol version")
   }
   PeboCommand.GET_STATUS -> DeviceStatus(r.u32(),r.u16(),r.u8(),r.u8(),r.u8(),r.u8(),r.u8(),r.u8(),r.u8(),r.u32(),r.u16()).also{validateStatus(it)}
   PeboCommand.GET_CONFIG -> decodeConfig(bytes.copyOfRange(1,bytes.size)).required().also{r.bytes(bytes.size-1)}
   PeboCommand.GET_HISTORY -> {
    val start=r.u16();val total=r.u16();val count=r.u8()
    ensure(count<=40,ErrorKind.PAYLOAD,"History page exceeds 40-record packet budget")
    HistoryPage(start,total,List(count){decodeDrink(r.bytes(6)).required()}).also{validatePage(it)}
   }
   else -> Acknowledgement
  }
  r.end();PeboReply(status,body)
 }
 private fun validateStatus(status: DeviceStatus) {
  ensure(status.batteryPercent in 0..100,ErrorKind.VALIDATION,"Battery Percent must be 0..100")
  ensure(status.ledProgress in 0..8,ErrorKind.VALIDATION,"LED Progress must be 0..8")
  ensure(status.totalReminderPeriodCount in 0..16 && status.completedReminderPeriodCount in 0..status.totalReminderPeriodCount,ErrorKind.VALIDATION,"Invalid completed/total period counts")
 }
 private fun validatePage(page: HistoryPage) {
  ensure(page.totalRecords in 0..65535 && page.startIndex in 0..page.totalRecords,ErrorKind.VALIDATION,"History index outside total")
  ensure(page.records.size<=40 && page.nextIndex<=page.totalRecords,ErrorKind.VALIDATION,"Invalid history page size")
  ensure(page.records.isNotEmpty() || !page.hasMore,ErrorKind.VALIDATION,"Empty history page cannot make forward progress")
 }
}