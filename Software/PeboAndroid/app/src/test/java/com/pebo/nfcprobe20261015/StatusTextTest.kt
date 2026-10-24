package com.pebo.nfcprobe20261015

import com.pebo.nfcprobe20261015.protocol.*
import com.pebo.nfcprobe20261015.transport.PeboClient
import com.pebo.nfcprobe20261015.transport.SamplePeboTransport
import org.junit.Assert.*
import org.junit.Test

class StatusTextTest {
 private fun status(percent:Int=65,flags:Int=0xBEEF)=DeviceStatus(
  1700000000L,2800,percent,0,3,1,8,8,1,1699999999L,flags)

 @Test fun rendersAllStatusFieldsWithoutFormattingLiteralPercent() {
  assertEquals(
   "Battery：2800 mV／65%\nBattery Status：0（語意待定）\n"+
   "Device Time：1700000000\nToday Drink Events：3\nCompleted Periods：1 / 8\n"+
   "Daily Target：8\nLED Progress：1 / 8\nLast Drink：1699999999\n"+
   "Device Status Flags：BEEF（語意待定）",
   formatDeviceStatus(status()){it.toString()})
 }

 @Test fun zeroAndFullBatteryKeepOneLiteralPercent() {
  for(percent in listOf(0,65,100)) {
   val text=formatDeviceStatus(status(percent)){it.toString()}
   assertEquals("Battery：2800 mV／$percent%",text.lineSequence().first())
   assertEquals(1,text.count{it=='%'})
  }
 }

 @Test fun dynamicTimeTextIsNeverTreatedAsFormatString() {
  val text=formatDeviceStatus(status(flags=1)){"time% %s %n"}
  assertTrue(text.contains("Device Time：time% %s %n\n"))
  assertTrue(text.contains("Last Drink：time% %s %n\n"))
  assertTrue(text.endsWith("Device Status Flags：0001（語意待定）"))
 }

 @Test fun sampleReadStatusCanBeRenderedThroughProductionFormatter() {
  val report=PeboClient(SamplePeboTransport()).execute(PeboCommand.GET_STATUS)
  val reply=(report.result as ParseResult.Ok).value
  assertEquals(PeboStatusCode.OK,reply.status)
  val body=reply.body as DeviceStatus
  val text=formatDeviceStatus(body){it.toString()}
  assertTrue(text.contains("${body.batteryPercent}%\n"))
  assertTrue(text.contains("Today Drink Events：3\n"))
  assertTrue(text.contains("Completed Periods：1 /"))
 }
}
