package com.pebo.nfcprobe20261015.transport

import com.pebo.nfcprobe20261015.protocol.*
import java.io.IOException

// Explicit in-memory simulator, no NFC calls, fixed sample identity distinct from UID.
class SamplePeboTransport: PeboPacketTransport {
 override val sourceLabel="SAMPLE 模擬資料（非真實 Pebo）"
 private var config=PeboConfig(8,true,listOf(ReminderPeriod(0,540,600,0),ReminderPeriod(1,600,660,0)))
 private var deviceTime=1700000000L
 private var timezoneMinutes=480
 private val records=MutableList(85){i -> DrinkRecord(1697500000L+(i/3)*86400+(i%3)*60,if(i%5==0)255 else 0,if(i%3==0)3 else 1)}
 @Synchronized override fun exchange(requestPacket: ByteArray): ByteArray {
  val decoded=PacketCodec.decode(requestPacket,expectedResponse=false)
  if(decoded is ParseResult.Error) throw IOException("Sample request rejected: ${decoded.detail}")
  val packet=(decoded as ParseResult.Ok).value
  val request=PayloadCodec.decodeRequest(packet.command,packet.payload)
  var status=PeboStatusCode.OK
  val body: ResponseBody?=if(request is ParseResult.Error) {status=PeboStatusCode.INVALID_DATA;null} else {
   val value=(request as ParseResult.Ok).value
   when(packet.command) {
    PeboCommand.GET_DEVICE_INFO -> DeviceInfo(listOf(0x53,0x41,0x4D,0x50,0x4C,0x45,0,1),0,1,0,0,1,1,0)
    PeboCommand.GET_STATUS -> DeviceStatus(deviceTime,2800,75,0,3,if(config.periods.isEmpty())0 else 1,config.periods.size,config.dailyTarget,1,deviceTime-60,0)
    PeboCommand.GET_CONFIG -> config
    PeboCommand.SET_CONFIG -> {config=(value as RequestBody.Config).value;Acknowledgement}
    PeboCommand.SET_TIME -> {value as RequestBody.SetTime;deviceTime=value.unixTime;timezoneMinutes=value.timezoneOffsetMinutes;Acknowledgement}
    PeboCommand.GET_HISTORY -> {
     value as RequestBody.History
     if(value.startIndex>records.size) {status=PeboStatusCode.INVALID_DATA;null}
     else HistoryPage(value.startIndex,records.size,records.drop(value.startIndex).take(minOf(value.requestedCount,40)))
    }
    PeboCommand.CLEAR_HISTORY -> {records.clear();Acknowledgement}
    PeboCommand.PING -> Acknowledgement
   }
  }
  return PacketCodec.encode(Packet(packet.command,packet.sequence,PayloadCodec.encodeReply(packet.command,PeboReply(status,body)),true))
 }
}