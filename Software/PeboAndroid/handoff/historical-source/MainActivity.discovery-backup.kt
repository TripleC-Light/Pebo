package com.pebo.nfcprobe20261015

import android.app.Activity
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.os.Bundle
import android.util.Log
import android.view.Gravity
import android.view.WindowManager
import android.widget.TextView
import org.json.JSONObject
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainActivity : Activity(), NfcAdapter.ReaderCallback {
 private lateinit var view: TextView
 private var adapter: NfcAdapter? = null
 private var count = 0
 private var lastTag = "尚未感應"
 private val session = System.currentTimeMillis()
 override fun onCreate(savedInstanceState: Bundle?) {
  super.onCreate(savedInstanceState)
  window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
  view = TextView(this).apply {
   textSize = 21f
   gravity = Gravity.CENTER
   setPadding(28, 60, 28, 60)
  }
  setContentView(view)
  adapter = NfcAdapter.getDefaultAdapter(this)
  publish("CREATED")
 }
 override fun onResume() {
  super.onResume()
  val nfc = adapter
  if (nfc == null) { publish("NO_NFC"); return }
  if (!nfc.isEnabled) { publish("NFC_OFF"); return }
  try {
   nfc.enableReaderMode(this, this,
    NfcAdapter.FLAG_READER_NFC_V or NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK,
    Bundle().apply { putInt(NfcAdapter.EXTRA_READER_PRESENCE_CHECK_DELAY, 250) })
   publish("READY")
  } catch (e: RuntimeException) { publish("READER_ERROR_${e.javaClass.simpleName}") }
 }
 override fun onPause() {
  adapter?.disableReaderMode(this)
  super.onPause()
 }
 override fun onTagDiscovered(tag: Tag) {
  val tech = tag.techList.map { it.substringAfterLast('.') }.joinToString(",")
  val uid = tag.id.joinToString("") { "%02X".format(it.toInt() and 255) }
  val isV = tag.techList.contains("android.nfc.tech.NfcV")
  runOnUiThread {
   count++
   lastTag = "時間：${SimpleDateFormat("HH:mm:ss", Locale.US).format(Date())}\nUID（Android 原始順序）：$uid\n技術：$tech"
   val result = JSONObject().put("session", session).put("count", count)
    .put("timestampMs", System.currentTimeMillis()).put("nfcV", isV)
    .put("uidLength", tag.id.size).put("tech", tech)
   openFileOutput("last_detection.json", MODE_PRIVATE).use { it.write(result.toString().toByteArray(Charsets.UTF_8)) }
   Log.i("PeboNfcProbe", "NFC_DETECTED session=$session count=$count nfcV=$isV uidLength=${tag.id.size} tech=$tech")
   publish(if (isV) "NFC_V_DETECTED" else "OTHER_TAG_DETECTED")
  }
 }
 private fun publish(state: String) {
  val status = when(state) {
   "READY" -> "等待感應：請靠近 Pebo 天線"
   "NFC_V_DETECTED" -> "NFC-V 實際感應成功"
   "NFC_OFF" -> "請在手機設定開啟 NFC"
   "NO_NFC" -> "此手機沒有 NFC"
   else -> state
  }
  view.text = "Pebo NFC 唯讀測試\n\n$status\n\n感應次數：$count\n\n$lastTag\n\n保持解鎖並停留此畫面\n移開後再靠近可重複測試\n\n僅偵測標籤；不讀寫資料區\n尚未確認型號、電量或飲水資料"
  val result = JSONObject().put("session", session).put("state", state).put("count", count)
  openFileOutput("status.json", MODE_PRIVATE).use { it.write(result.toString().toByteArray(Charsets.UTF_8)) }
  Log.i("PeboNfcProbe", "NFC_STATE session=$session state=$state count=$count")
 }
}