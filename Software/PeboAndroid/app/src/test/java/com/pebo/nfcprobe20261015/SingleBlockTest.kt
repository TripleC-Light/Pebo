package com.pebo.nfcprobe20261015
import org.junit.Assert.*
import org.junit.Test

class SingleBlockTest {
 private val uid = byteArrayOf(0x11,0x22,0x33,0x44,0x55,0x66,0x02,0xE0.toByte())
 @Test fun addressedCommandKeepsRawUidAndOmitsCrc() {
  val command = SingleBlock.command(uid, 0)
  assertArrayEquals(byteArrayOf(0x22,0x20,0x11,0x22,0x33,0x44,0x55,0x66,0x02,0xE0.toByte(),0), command)
  assertEquals(11, command.size)
  assertArrayEquals(uid, command.copyOfRange(2,10))
  command[2] = 0
  assertEquals(0x11, uid[0].toInt())
 }
 @Test fun upperBoundBlock127() { assertEquals(127, SingleBlock.command(uid,127).last().toInt()) }
 @Test fun rejectsInvalidBlockNumbers() { for (block in listOf(-1,128,256)) assertThrows(IllegalArgumentException::class.java) { SingleBlock.command(uid,block) } }
 @Test fun rejectsInvalidUidLengths() { for (length in listOf(0,7,9)) assertThrows(IllegalArgumentException::class.java) { SingleBlock.command(ByteArray(length),0) } }
 @Test fun decimalInputValidation() {
  for (text in listOf("-1","128","abc","1.0","0x10","","9999999999999999","+1")) assertNull(text, SingleBlock.blockNumber(text))
  assertEquals(0,SingleBlock.blockNumber("0")); assertEquals(127,SingleBlock.blockNumber(" 127 ")); assertEquals(7,SingleBlock.blockNumber("007"))
 }
 @Test fun successRequiresFlagsAndFourDataBytes() {
  val result=SingleBlock.parse(byteArrayOf(0,1,2,3,4)) as SingleBlock.Response.Success
  assertArrayEquals(byteArrayOf(1,2,3,4),result.data)
 }
 @Test fun zeroAndFfDataRemainValid() {
  for(value in listOf(0,255)) {
   val parsed=SingleBlock.parse(byteArrayOf(0)+ByteArray(4){value.toByte()}) as SingleBlock.Response.Success
   assertArrayEquals(ByteArray(4){value.toByte()}, parsed.data)
  }
 }
 @Test fun knownTagErrorsPreserveCode() {
  for (code in listOf(1,2,3,15,16,21)) {
   val result=SingleBlock.parse(byteArrayOf(1,code.toByte())) as SingleBlock.Response.TagError
   assertEquals(code,result.code); assertTrue(result.reason.isNotBlank())
  }
 }
 @Test fun unknownTagErrorRemainsTagError() {
  val result=SingleBlock.parse(byteArrayOf(1,0xA1.toByte())) as SingleBlock.Response.TagError
  assertEquals(0xA1,result.code); assertTrue(result.reason.contains("未識別"))
 }
 @Test fun emptyResponseIsMalformed() { assertTrue(SingleBlock.parse(byteArrayOf()) is SingleBlock.Response.Malformed) }
 @Test fun shortAndLongSuccessAreMalformed() {
  for (length in listOf(1,2,3,4,6,7,100)) assertTrue("length=$length",SingleBlock.parse(ByteArray(length)) is SingleBlock.Response.Malformed)
 }
 @Test fun malformedErrorLengthsRejected() {
  for (length in listOf(1,3,5)) assertTrue(SingleBlock.parse(ByteArray(length).apply{this[0]=1}) is SingleBlock.Response.Malformed)
 }
 @Test fun unsupportedFlagsRejectedEvenWithValidLength() {
  for (flag in listOf(2,8,16,128,255)) assertTrue(SingleBlock.parse(byteArrayOf(flag.toByte(),0,0,0,0)) is SingleBlock.Response.Malformed)
 }
 @Test fun hexIsUnsignedAndDoesNotMutateUid() {
  val before=uid.copyOf(); assertEquals("11 22 33 44 55 66 02 E0",SingleBlock.hex(uid)); assertArrayEquals(before,uid)
 }
}