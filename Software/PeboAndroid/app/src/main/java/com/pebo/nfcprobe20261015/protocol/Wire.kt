package com.pebo.nfcprobe20261015.protocol

// Pebo v0.1: agreed envelope; provisional CRC profile explicitly named below.
enum class ErrorKind { MAGIC, VERSION, LENGTH, CRC, COMMAND, SEQUENCE, PAYLOAD, VALIDATION }
class ProtocolFailure(val kind: ErrorKind, message: String): IllegalArgumentException(message)
sealed interface ParseResult<out T> {
 data class Ok<T>(val value: T): ParseResult<T>
 data class Error(val kind: ErrorKind, val detail: String): ParseResult<Nothing>
}
internal fun ensure(ok: Boolean, kind: ErrorKind, message: String) { if (!ok) throw ProtocolFailure(kind, message) }
internal fun <T> parseSafely(block: () -> T): ParseResult<T> = try { ParseResult.Ok(block()) }
 catch (e: ProtocolFailure) { ParseResult.Error(e.kind, e.message ?: e.kind.name) }
 catch (e: IllegalArgumentException) { ParseResult.Error(ErrorKind.VALIDATION, e.message ?: "Invalid value") }
internal fun <T> ParseResult<T>.required(): T = when(this) {
 is ParseResult.Ok -> value
 is ParseResult.Error -> throw ProtocolFailure(kind, detail)
}
object LittleEndian {
 fun u16(value: Int): ByteArray { require(value in 0..65535); return byteArrayOf(value.toByte(), (value ushr 8).toByte()) }
 fun i16(value: Int): ByteArray { require(value in -32768..32767); return u16(value and 65535) }
 fun u32(value: Long): ByteArray { require(value in 0..0xFFFFFFFFL); return ByteArray(4) { (value ushr (8*it)).toByte() } }
}
internal class Reader(private val bytes: ByteArray) {
 var offset=0; private set
 fun bytes(count: Int): ByteArray {
  ensure(count>=0 && offset+count<=bytes.size, ErrorKind.LENGTH, "Payload truncated at $offset, need $count bytes")
  return bytes.copyOfRange(offset, offset+count).also { offset+=count }
 }
 fun u8()=bytes(1)[0].toInt() and 255
 fun u16(): Int { val b=bytes(2); return (b[0].toInt() and 255) or ((b[1].toInt() and 255) shl 8) }
 fun i16(): Int = u16().toShort().toInt()
 fun u32(): Long { val b=bytes(4); return b.indices.fold(0L) { value,i -> value or ((b[i].toLong() and 255) shl (8*i)) } }
 fun end() { ensure(offset==bytes.size,ErrorKind.LENGTH,"Unexpected trailing payload bytes: ${bytes.size-offset}") }
}
object Crc16CcittFalse {
 // PROVISIONAL: poly 1021, init FFFF, refin/refout false, xorout 0000.
 // Check('123456789') = 29B1. Stored little-endian; covers header + payload.
 fun compute(bytes: ByteArray): Int {
  var crc=0xFFFF
  for(b in bytes) {
   crc=crc xor ((b.toInt() and 255) shl 8)
   repeat(8) { crc=if(crc and 0x8000 != 0) (crc shl 1) xor 0x1021 else crc shl 1; crc=crc and 65535 }
  }
  return crc
 }
}
enum class PeboCommand(val code: Int) {
 GET_DEVICE_INFO(1), GET_STATUS(2), SET_TIME(3), GET_CONFIG(4), SET_CONFIG(5), GET_HISTORY(6), CLEAR_HISTORY(7), PING(8);
 val responseCode get()=code or 0x80
 companion object { fun fromRequest(code: Int)=entries.firstOrNull { it.code==code } }
}
enum class PeboStatusCode(val code: Int) {
 OK(0), INVALID_COMMAND(1), INVALID_LENGTH(2), INVALID_DATA(3), BUSY(4), CRC_ERROR(5), INTERNAL_ERROR(6);
 companion object { fun fromCode(code: Int)=entries.firstOrNull{it.code==code} }
}
data class Packet(val command: PeboCommand, val sequence: Int, val payload: ByteArray, val response: Boolean=false)
object PacketCodec {
 const val VERSION=1
 const val MAX_PACKET=256
 const val MAX_PAYLOAD=248 // 6 header + N + 2 CRC, despite 1-byte length supporting 255.
 fun encode(packet: Packet): ByteArray {
  require(packet.sequence in 0..255)
  require(packet.payload.size<=MAX_PAYLOAD) { "Payload exceeds 248-byte limit" }
  // offsets 0..1 PB; 2 version; 3 command; 4 seq; 5 payload length.
  val body=byteArrayOf(0x50,0x42,VERSION.toByte(),
   (if(packet.response) packet.command.responseCode else packet.command.code).toByte(),
   packet.sequence.toByte(),packet.payload.size.toByte())+packet.payload
  return body+LittleEndian.u16(Crc16CcittFalse.compute(body))
 }
 fun decode(bytes: ByteArray, expectedCommand: PeboCommand?=null, expectedSequence: Int?=null, expectedResponse: Boolean?=null): ParseResult<Packet> = parseSafely {
  ensure(bytes.size in 8..MAX_PACKET,ErrorKind.LENGTH,"Packet must contain 8..256 bytes; got ${bytes.size}")
  ensure(bytes[0]==0x50.toByte() && bytes[1]==0x42.toByte(),ErrorKind.MAGIC,"Magic must be PB (50 42)")
  ensure((bytes[2].toInt() and 255)==VERSION,ErrorKind.VERSION,"Unsupported Pebo protocol version")
  val length=bytes[5].toInt() and 255
  ensure(length<=MAX_PAYLOAD && bytes.size==length+8,ErrorKind.LENGTH,"Payload length does not match packet")
  ensure(crcMatches(bytes)==true,ErrorKind.CRC,"Pebo CRC16 mismatch (provisional CCITT-FALSE)")
  val code=bytes[3].toInt() and 255
  val command=PeboCommand.fromRequest(code and 0x7F)
  ensure(command!=null,ErrorKind.COMMAND,"Unknown command 0x%02X".format(code))
  val response=code and 0x80 != 0
  val seq=bytes[4].toInt() and 255
  ensure(expectedCommand==null || expectedCommand==command,ErrorKind.COMMAND,"Response command does not match request")
  ensure(expectedSequence==null || expectedSequence==seq,ErrorKind.SEQUENCE,"Response sequence does not match request")
  ensure(expectedResponse==null || expectedResponse==response,ErrorKind.COMMAND,"Request/response direction mismatch")
  Packet(command!!,seq,bytes.copyOfRange(6,6+length),response)
 }
 fun crcMatches(bytes: ByteArray): Boolean? {
  if(bytes.size !in 8..MAX_PACKET || (bytes[5].toInt() and 255)+8!=bytes.size) return null
  val n=bytes.size
  val given=(bytes[n-2].toInt() and 255) or ((bytes[n-1].toInt() and 255) shl 8)
  return given==Crc16CcittFalse.compute(bytes.copyOfRange(0,n-2))
 }
 fun hex(bytes: ByteArray)=bytes.joinToString(" "){ "%02X".format(it.toInt() and 255) }
}