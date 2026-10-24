package com.pebo.nfcprobe20261015

import android.app.Activity
import android.content.ClipData
import android.content.ClipboardManager
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.TagLostException
import android.nfc.tech.NfcV
import android.os.Bundle
import android.text.InputType
import android.util.Log
import android.view.WindowManager
import android.widget.*
import org.json.JSONObject
import java.io.IOException
import java.text.SimpleDateFormat
import java.util.*
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicReference

class MainActivity : Activity(), NfcAdapter.ReaderCallback {
 private lateinit var input: EditText
 private lateinit var prepare: Button
 private lateinit var nfcStatus: TextView
 private lateinit var discovery: TextView
 private lateinit var result: TextView
 private lateinit var historyView: TextView
 private lateinit var logView: TextView
 private var adapter: NfcAdapter? = null
 private val gate = ReadGate()
 private val worker = Executors.newSingleThreadExecutor()
 private val closer = Executors.newSingleThreadExecutor()
 // Serialize close across cancellation and worker cleanup. A delayed cancel
 // must never close an old tech after the next operation has already started.
 private class ActiveLink(val tech: NfcV) {
  private var closed = false
  @Synchronized fun closeOnce() {
   if (closed) return
   try { tech.close() } catch (_: IOException) {} finally { closed = true }
  }
 }
 private val link = AtomicReference<ActiveLink?>(null)
 private val records = ArrayDeque<String>()
 private var history = "歷史成功資料：無"
 private var count = 0
 private var readerReady = false
 private val session = System.currentTimeMillis()
 private var latestState = "IDLE"
 private var latestResult = "尚未執行本次讀取"
 override fun onCreate(savedInstanceState: Bundle?) {
  super.onCreate(savedInstanceState)
  window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
  adapter = NfcAdapter.getDefaultAdapter(this)
  val column = LinearLayout(this).apply {
   orientation = LinearLayout.VERTICAL
   setPadding(24, 48, 24, 48)
  }
  fun label(text: String) = TextView(this).apply { this.text = text; textSize = 17f; setTextIsSelectable(true); column.addView(this) }
  label("Pebo ST25DV 單一區塊唯讀測試").textSize = 23f
  nfcStatus = label("")
  column.addView(Button(this).apply {
   text = "Pebo Protocol v0.1／Sample 測試"
   setOnClickListener { startActivity(android.content.Intent(this@MainActivity, ProtocolActivity::class.java)) }
  })
  label("使用者 EEPROM：block 0～127，每區塊 4 bytes。資料不解讀為電量或飲水資訊。")
  input = EditText(this).apply {
   hint = "十進位 block number（0～127）"
   inputType = InputType.TYPE_CLASS_TEXT
   setSingleLine(true)
   setText(savedInstanceState?.getString("input") ?: "0")
   contentDescription = "十進位 block number"
   column.addView(this)
  }
  prepare = Button(this).apply { text = "準備讀取"; column.addView(this); setOnClickListener { arm() } }
  discovery = label("尚未感應標籤")
  result = label(savedInstanceState?.getString("result") ?: latestResult)
  history = savedInstanceState?.getString("history") ?: history
  historyView = label(history)
  label("UID 僅為 NFC UID，非 Pebo Device ID。顯示與 TX 都使用 Android Tag.id 原始 byte order，不反轉。")
  val buttons = LinearLayout(this)
  column.addView(buttons)
  buttons.addView(Button(this).apply {
   text = "複製紀錄"
   setOnClickListener {
    (getSystemService(CLIPBOARD_SERVICE) as ClipboardManager).setPrimaryClip(
     ClipData.newPlainText("Pebo NFC read log", records.joinToString("\n\n")))
    Toast.makeText(this@MainActivity, "已複製 ${records.size} 筆紀錄", Toast.LENGTH_SHORT).show()
   }
  })
  buttons.addView(Button(this).apply {
   text = "清除紀錄"
   setOnClickListener { records.clear(); renderLogs() }
  })
  label("測試紀錄（本機記憶體最多 100 筆；關閉程序後不保留）")
  logView = label("")
  savedInstanceState?.getStringArrayList("records")?.takeLast(100)?.forEach { records.addLast(it) }
  if (savedInstanceState != null) {
   result.text = "前次畫面（歷史結果，非本次讀取）：\n${result.text}\n請重新準備讀取。"
  }
  renderLogs()
  setContentView(ScrollView(this).apply { addView(column) })
 }
 override fun onResume() {
  super.onResume()
  gate.resume()
  readerReady = false
  try {
   if (adapter?.isEnabled == true) {
    adapter!!.enableReaderMode(this, this,
     NfcAdapter.FLAG_READER_NFC_V or NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK,
     Bundle().apply { putInt(NfcAdapter.EXTRA_READER_PRESENCE_CHECK_DELAY, 250) })
    readerReady = true
   }
  } catch (e: RuntimeException) { result.text = "讀取器啟用失敗：${e.javaClass.simpleName}" }
  refreshControls()
  publish("READY")
 }
 override fun onPause() {
  val pending = gate.pause()
  readerReady = false
  if (pending != null) {
   latestResult = "已取消：App 切到背景，待執行 block ${pending.block} 已取消；未送出 TX。"
   result.text = latestResult
   record(pending.block, "未送出", "未收到", "CANCELLED", "App 切到背景")
  }
  if (gate.running != null) result.text = "正在取消執行中的讀取；前次成功資料僅為歷史。"
  val connection = link.getAndSet(null)
  if (connection != null) closer.execute { connection.closeOnce() }
  try { adapter?.disableReaderMode(this) } catch (_: RuntimeException) {}
  refreshControls()
  publish("PAUSED")
  super.onPause()
 }
 override fun onDestroy() {
  worker.shutdown()
  closer.shutdown()
  super.onDestroy()
 }
 override fun onSaveInstanceState(outState: Bundle) {
  outState.putString("input", input.text.toString())
  outState.putString("result", result.text.toString())
  outState.putString("history", history)
  outState.putStringArrayList("records", ArrayList(records))
  super.onSaveInstanceState(outState)
 }
 private fun arm() {
  if (gate.armed != null || gate.running != null) return
  val block = SingleBlock.blockNumber(input.text.toString())
  if (block == null) {
   input.error = "請輸入十進位整數 0～127"
   result.text = "拒絕操作：block 輸入無效。未送出命令；下方成功資料僅為歷史。"
   publish("INVALID_INPUT"); return
  }
  if (!readerReady || adapter?.isEnabled != true) {
   result.text = "尚未準備：請確認 NFC 已開啟，切出再返回此 App。"
   refreshControls(); return
  }
  if (!gate.arm(block)) return
  input.error = null
  latestResult = "本次 block：$block\n等待下一次感應，只讀一次。請先移開標籤，再靠近。\nTX／RX／本次資料：尚無"
  result.text = latestResult
  refreshControls(); publish("ARMED")
 }
 override fun onTagDiscovered(tag: Tag) {
  runOnUiThread {
   if (!gate.foreground || isDestroyed) return@runOnUiThread
   count++
   val tech = tag.techList.joinToString(",") { it.substringAfterLast('.') }
   discovery.text = "NFC 感應次數：$count\nNFC UID（Android 原始順序）：${SingleBlock.hex(tag.id)}\n技術：$tech"
   Log.i("PeboNfcProbe", "NFC_DETECTED session=$session count=$count nfcV=${NfcV.get(tag) != null} uidLength=${tag.id.size} tech=$tech")
   val request = gate.take()
   if (request == null) { publish("DISCOVERED_NO_READ"); return@runOnUiThread }
   result.text = "本次 block：${request.block}\n讀取中；請保持天線位置。\n前次成功資料僅為歷史。"
   refreshControls(); publish("READING")
   worker.execute { readOnce(tag, request) }
  }
 }
 private fun readOnce(tag: Tag, request: ReadGate.Request) {
  var connection: NfcV? = null
  var active: ActiveLink? = null
  var tx: ByteArray? = null
  var rx: ByteArray? = null
  var sent = false
  var kind = "IO_ERROR"
  var reason = ""
  var data: ByteArray? = null
  try {
   if (request.cancelled.get()) throw IOException("操作已取消")
   tx = SingleBlock.command(tag.id, request.block)
   connection = NfcV.get(tag) ?: throw IllegalArgumentException("標籤不支援 NFC-V")
   active = ActiveLink(connection)
   link.set(active)
   if (request.cancelled.get()) throw IOException("操作已取消")
   connection.connect()
   if (request.cancelled.get()) throw IOException("操作已取消")
   sent = true
   rx = com.pebo.nfcprobe20261015.transport.St25NfcVBlockTransport(connection).readSingleBlock(tag.id, request.block)
   when (val parsed = SingleBlock.parse(rx)) {
    is SingleBlock.Response.Success -> { kind = "SUCCESS"; reason = "flags 與長度符合，取得 4 bytes"; data = parsed.data }
    is SingleBlock.Response.TagError -> { kind = "TAG_ERROR"; reason = "標籤錯誤 %02X：${parsed.reason}".format(parsed.code) }
    is SingleBlock.Response.Malformed -> { kind = "MALFORMED"; reason = parsed.reason }
   }
  } catch (e: TagLostException) { kind = "TAG_LOST"; reason = "TagLostException：標籤已離開，請重新準備並靠近" }
  catch (e: IOException) { kind = "IO_ERROR"; reason = "${e.javaClass.simpleName}：${e.message ?: "NFC I/O 失敗"}" }
  catch (e: IllegalArgumentException) { kind = "INVALID_TAG"; reason = e.message ?: "標籤／UID 無效" }
  catch (e: RuntimeException) { kind = "READER_ERROR"; reason = "${e.javaClass.simpleName}：${e.message ?: "NFC 服務錯誤"}" }
  finally {
   if (connection != null) {
    active?.closeOnce()
    link.compareAndSet(active, null)
   }
  }
  val txHex = tx?.let { SingleBlock.hex(it) } ?: "尚未組合"
  val rxHex = rx?.let { if (it.isEmpty()) "（空回應）" else SingleBlock.hex(it) } ?: "未收到"
  runOnUiThread {
   val valid = gate.finish(request)
   if (!valid) { kind = "CANCELLED"; reason = "App 切到背景／操作已取消；即使收到回應也不算本次成功"; data = null }
   if (!isDestroyed) {
    val success = kind == "SUCCESS"
    latestResult = "本次 block：${request.block}\n結果：${if (success) "成功" else "失敗／取消"} [$kind]\n原因：$reason\nTX HEX（${if (sent) "已呼叫 transceive" else "未送出"}）：$txHex\n完整 RX HEX：$rxHex\n本次 4-byte 資料：${data?.let { SingleBlock.hex(it) } ?: "無有效資料"}"
    result.text = latestResult
    if (success) history = "歷史最近成功（非本次狀態）：${now()}\nblock ${request.block}：${SingleBlock.hex(data!!)}"
    historyView.text = history
    record(request.block, "${if (sent) "送出" else "未送出"} $txHex", rxHex, kind, reason)
    refreshControls(); publish(kind)
   }
  }
 }
 private fun refreshControls() {
  val busy = gate.armed != null || gate.running != null
  prepare.isEnabled = gate.foreground && readerReady && adapter?.isEnabled == true && !busy
  input.isEnabled = !busy
  nfcStatus.text = "NFC 支援：${if (adapter != null) "是" else "否"}／開啟：${if (adapter?.isEnabled == true) "是" else "否"}\n狀態：${when { gate.running != null -> "處理中"; gate.armed != null -> "已準備，等待感應"; !gate.foreground -> "背景，操作已取消"; readerReady -> "可準備讀取；未準備時僅偵測"; else -> "讀取器未就緒" }}"
 }
 private fun now() = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS Z", Locale.US).format(Date())
 private fun record(block: Int, tx: String, rx: String, kind: String, error: String) {
  records.addLast("${now()} | block=$block | $kind\nTX=$tx\nRX=$rx\n${if (kind == "SUCCESS") "結果" else "錯誤"}=$error")
  while (records.size > 100) records.removeFirst()
  renderLogs()
  Log.i("PeboNfcProbe", "READ_RESULT session=$session block=$block result=$kind")
 }
 private fun renderLogs() { logView.text = if (records.isEmpty()) "尚無紀錄" else records.toList().asReversed().joinToString("\n\n") }
 private fun publish(state: String) {
  latestState = state
  // Existing private diagnostic status uses no storage permission; test logs stay in memory.
  val json = JSONObject().put("session", session).put("state", latestState).put("count", count)
   .put("readBusy", gate.running != null).put("armedBlock", gate.armed?.block ?: -1).put("logCount", records.size)
  openFileOutput("status.json", MODE_PRIVATE).use { it.write(json.toString().toByteArray(Charsets.UTF_8)) }
  Log.i("PeboNfcProbe", "NFC_STATE session=$session state=$state count=$count")
 }
}