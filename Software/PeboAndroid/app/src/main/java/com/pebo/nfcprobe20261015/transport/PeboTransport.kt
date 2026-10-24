package com.pebo.nfcprobe20261015.transport

import com.pebo.nfcprobe20261015.protocol.*
import java.io.IOException

// Entire application packets, never ISO15693 flags, UID, block offsets or ST registers.
interface PeboPacketTransport {
 val sourceLabel: String
 @Throws(IOException::class) fun exchange(requestPacket: ByteArray): ByteArray
}
class UnboundNfcVPacketTransport: PeboPacketTransport {
 override val sourceLabel="真實裝置（尚未綁定）"
 override fun exchange(requestPacket: ByteArray): ByteArray = throw IOException(
  "尚缺 STM32 protocol、EEPROM/mailbox mapping、commit/ack/timeout 定義；未送出任何 NFC packet")
}
data class ExchangeReport(val source: String,val tx: ByteArray,val rx: ByteArray?,val crcValid: Boolean?,
 val result: ParseResult<PeboReply>,val error: String?=null)
class PeboClient(private val transport: PeboPacketTransport) {
 private var nextSequence=0
 @Synchronized fun execute(command: PeboCommand, body: RequestBody=RequestBody.Empty): ExchangeReport {
  val payload=try { PayloadCodec.encodeRequest(command,body) } catch(e: IllegalArgumentException) {
   return ExchangeReport(transport.sourceLabel,byteArrayOf(),null,null,ParseResult.Error(ErrorKind.VALIDATION,e.message ?: "Invalid request"))
  }
  val sequence=nextSequence;nextSequence=(nextSequence+1) and 255
  val tx=PacketCodec.encode(Packet(command,sequence,payload))
  val rx=try { transport.exchange(tx) } catch(e: IOException) {
   return ExchangeReport(transport.sourceLabel,tx,null,null,ParseResult.Error(ErrorKind.PAYLOAD,e.message ?: "Transport unavailable"),e.message)
  }
  val packet=PacketCodec.decode(rx,command,sequence,true)
  val result=when(packet) {
   is ParseResult.Error -> packet
   is ParseResult.Ok -> {
    val reply=PayloadCodec.decodeReply(command,packet.value.payload)
    if(command==PeboCommand.GET_HISTORY && body is RequestBody.History && reply is ParseResult.Ok && reply.value.body is HistoryPage) {
     val page=reply.value.body as HistoryPage
     if(page.startIndex!=body.startIndex || page.records.size>body.requestedCount)
      ParseResult.Error(ErrorKind.PAYLOAD,"History response does not match requested page") else reply
    } else reply
   }
  }
  return ExchangeReport(transport.sourceLabel,tx,rx,PacketCodec.crcMatches(rx),result)
 }
}