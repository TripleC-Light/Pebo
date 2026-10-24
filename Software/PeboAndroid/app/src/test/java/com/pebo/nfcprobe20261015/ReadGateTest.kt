package com.pebo.nfcprobe20261015
import org.junit.Assert.*
import org.junit.Test
class ReadGateTest {
 @Test fun idleDiscoveryDoesNotStartRead() { val gate=ReadGate();gate.resume();assertNull(gate.take()) }
 @Test fun onePressOnlyOneRead() {
  val gate=ReadGate();gate.resume();assertTrue(gate.arm(0));assertFalse(gate.arm(1))
  val request=gate.take()!!;assertEquals(0,request.block);assertNull(gate.take());assertFalse(gate.arm(1))
  assertTrue(gate.finish(request));assertNull(gate.take());assertTrue(gate.arm(1))
 }
 @Test fun pauseCancelsArmedAndDoesNotRearmOnResume() {
  val gate=ReadGate();gate.resume();gate.arm(0);val request=gate.pause()!!
  assertTrue(request.cancelled.get());gate.resume();assertNull(gate.take());assertTrue(gate.arm(0))
 }
 @Test fun pauseCancelsRunningAndKeepsBusyUntilWorkerReturns() {
  val gate=ReadGate();gate.resume();gate.arm(0);val request=gate.take()!!;gate.pause();gate.resume()
  assertTrue(request.cancelled.get());assertFalse(gate.arm(0));assertFalse(gate.finish(request));assertTrue(gate.arm(0))
 }
 @Test fun cannotArmInBackgroundOrOutsideBounds() {
  val gate=ReadGate();assertFalse(gate.arm(0));gate.resume();assertFalse(gate.arm(-1));assertFalse(gate.arm(128));assertTrue(gate.arm(127))
 }
 @Test fun staleCompletionCannotClearAnotherRequest() {
  val gate=ReadGate();gate.resume();gate.arm(0);val first=gate.take()!!;gate.finish(first)
  gate.arm(1);val second=gate.take()!!;assertFalse(gate.finish(first));assertSame(second,gate.running);assertTrue(gate.finish(second))
 }
}