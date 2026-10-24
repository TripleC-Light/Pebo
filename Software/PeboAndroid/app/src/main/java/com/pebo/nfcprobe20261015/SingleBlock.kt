package com.pebo.nfcprobe20261015

import java.util.concurrent.atomic.AtomicBoolean

object SingleBlock {
 fun blockNumber(text: String): Int? = text.trim().takeIf { it.matches(Regex("[0-9]+")) }
  ?.toIntOrNull()?.takeIf { it in 0..127 }
 fun command(uid: ByteArray, block: Int): ByteArray {
  require(uid.size == 8) { "UID 必須是 8 bytes" }
  require(block in 0..127) { "block 必須在 0～127" }
  // High data rate + Address; Option/Select/Inventory are clear. No CRC.
  return byteArrayOf(0x22, 0x20) + uid.copyOf() + byteArrayOf(block.toByte())
 }
 fun hex(bytes: ByteArray): String = bytes.joinToString(" ") { "%02X".format(it.toInt() and 255) }
 sealed class Response {
  data class Success(val data: ByteArray): Response()
  data class TagError(val code: Int, val reason: String): Response()
  data class Malformed(val reason: String): Response()
 }
 fun parse(rx: ByteArray): Response {
  if (rx.isEmpty()) return Response.Malformed("空回應；缺少 response flags")
  val flags = rx[0].toInt() and 255
  if ((flags and 0xFE) != 0) return Response.Malformed("不支援的 response flags：%02X".format(flags))
  if ((flags and 1) != 0) {
   if (rx.size != 2) return Response.Malformed("錯誤回應應為 2 bytes，實際 ${rx.size}")
   val code = rx[1].toInt() and 255
   val reason = when(code) {
    0x01 -> "命令不支援"
    0x02 -> "命令無法識別／格式錯誤"
    0x03 -> "命令選項不支援"
    0x0F -> "標籤未提供詳細原因"
    0x10 -> "指定區塊不存在"
    0x15 -> "指定區塊有讀取保護"
    else -> "未識別的標籤錯誤碼"
   }
   return Response.TagError(code, reason)
  }
  if (rx.size != 5) return Response.Malformed("成功回應應為 flags + 4 bytes（共 5），實際 ${rx.size}")
  return Response.Success(rx.copyOfRange(1, 5))
 }
}

// Owned by UI thread. A running request stays busy until its worker finishes,
// even if pause invalidated it; this prevents overlapping NFC connections.
class ReadGate {
 class Request(val block: Int) { val cancelled = AtomicBoolean(false) }
 var foreground = false; private set
 var armed: Request? = null; private set
 var running: Request? = null; private set
 fun resume() { foreground = true }
 fun arm(block: Int): Boolean {
  if (!foreground || armed != null || running != null || block !in 0..127) return false
  armed = Request(block); return true
 }
 fun take(): Request? {
  if (!foreground || running != null) return null
  val request = armed ?: return null
  armed = null; running = request; return request
 }
 fun pause(): Request? {
  foreground = false
  val pending = armed
  pending?.cancelled?.set(true); armed = null
  running?.cancelled?.set(true)
  return pending
 }
 fun finish(request: Request): Boolean {
  if (running !== request) return false
  running = null
  return foreground && !request.cancelled.get()
 }
}