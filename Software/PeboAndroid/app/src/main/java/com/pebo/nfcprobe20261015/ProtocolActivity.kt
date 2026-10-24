package com.pebo.nfcprobe20261015

import android.app.Activity
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.os.Bundle
import android.text.InputType
import android.view.WindowManager
import android.widget.*
import com.pebo.nfcprobe20261015.protocol.*
import com.pebo.nfcprobe20261015.transport.*
import java.text.SimpleDateFormat
import java.util.*
import java.util.concurrent.Executors

class ProtocolActivity: Activity(),NfcAdapter.ReaderCallback {
 private lateinit var column: LinearLayout
 private lateinit var mode: Switch
 private lateinit var banner: TextView
 private lateinit var nfcView: TextView
 private lateinit var uidView: TextView
 private lateinit var infoView: TextView
 private lateinit var statusView: TextView
 private lateinit var configView: TextView
 private lateinit var historyView: TextView
 private lateinit var debugView: TextView
 private lateinit var message: TextView
 private lateinit var target: EditText
 private lateinit var reminder: Switch
 private lateinit var periods: LinearLayout
 private lateinit var historyStart: EditText
 private lateinit var historyCount: EditText
 private lateinit var next: Button
 private val periodRows=mutableListOf<List<EditText>>()
 private val controls=mutableListOf<Button>()
 private val executor=Executors.newSingleThreadExecutor()
 private var sample=SamplePeboTransport()
 private var client=PeboClient(sample)
 private var adapter: NfcAdapter?=null
 private var busy=false
 private var foreground=false
 private var generation=0
 private var nextIndex: Int?=null
 private var lastPageStart: Int?=null
 private var tagTech="尚未感應"
 private var uid="尚未感應；不使用 Sample UID"
 private var lastDebug="尚未收發；CRC profile 與分頁格式暫定"
 private val shownRecords=mutableListOf<DrinkRecord>()
 override fun onCreate(state: Bundle?) {
  super.onCreate(state)
  window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
  adapter=NfcAdapter.getDefaultAdapter(this)
  column=LinearLayout(this).apply {orientation=LinearLayout.VERTICAL;setPadding(24,52,24,48)}
  label("Pebo NFC Protocol v0.1").textSize=25f
  mode=Switch(this).apply {
   text="SAMPLE 模式（模擬，不代表真實裝置）";isChecked=true
   column.addView(this)
   setOnCheckedChangeListener { _,sampleMode ->
    client=PeboClient(if(sampleMode) sample else UnboundNfcVPacketTransport())
    clearDecoded();message.text="已切換來源；舊資料已清除，請重新讀取"
    lastDebug="尚未收發";renderDebug();refresh()
   }
  }
  banner=label("")
  label("暫定：CCITT-FALSE／CRC LE／History 回應與 ACK。Sample 僅在手機記憶體模擬；真實 MCU 協定及 EEPROM/mailbox 交接尚未完成。")
  nfcView=label("");uidView=label("")
  message=label("請選擇操作；資料來源會持續標示。")
  action("返回 NFC 偵測／單區塊唯讀"){finish()}
  section("Device Info")
  action("Read Device Info"){perform(PeboCommand.GET_DEVICE_INFO)}
  infoView=label("尚未讀取 Pebo Device ID／HW／FW／Protocol")
  section("Status")
  action("Read Status"){perform(PeboCommand.GET_STATUS)}
  statusView=label("尚未讀取狀態")
  section("Config")
  action("Read Config"){perform(PeboCommand.GET_CONFIG)}
  configView=label("編輯草稿；尚未讀取／寫入")
  label("Daily Target（0～255，暫不假設等於 Period Count）")
  target=number("8")
  reminder=Switch(this).apply{text="Reminder Enable";column.addView(this)}
  label("Reminder Periods：ID／Start minute／End minute／Flags\n暫定半開區間 [Start, End)，End=1440 是當日結束。")
  periods=LinearLayout(this).apply{orientation=LinearLayout.VERTICAL;column.addView(this)}
  action("新增 Reminder Period（最多 16）") {
   if(periodRows.size>=16) message.text="Period Count 不可超過 16" else addPeriod(ReminderPeriod(periodRows.size,540,600,0))
  }
  action("Write Config") {
   try {
    val config=editorConfig();ConfigValidation.requireValid(config)
    perform(PeboCommand.SET_CONFIG,RequestBody.Config(config))
   } catch(e:IllegalArgumentException){message.text="拒絕 Config：${e.message}"}
  }
  section("History")
  label("Start Index（0～65535）")
  historyStart=number("0")
  label("Requested Count（1～255；暫定每頁最多 40 筆）")
  historyCount=number("40")
  action("Read History") {readHistory(false)}
  next=action("下一頁") {readHistory(true)}
  action("CLEAR_HISTORY（僅 Sample）") {
   if(!mode.isChecked) message.text="真實 CLEAR_HISTORY 未啟用；未送出命令" else perform(PeboCommand.CLEAR_HISTORY)
  }
  historyView=label("尚未讀取 History")
  section("Time")
  label("UTC uint32 + 手機當下 timezone offset（分鐘）；顯示同時註明 UTC 與手機本地時間。")
  action("Sync Time") {
   val millis=System.currentTimeMillis()
   perform(PeboCommand.SET_TIME,RequestBody.SetTime(millis/1000,TimeZone.getDefault().getOffset(millis)/60000))
  }
  action("PING"){perform(PeboCommand.PING)}
  section("Debug Page")
  debugView=label(lastDebug)
  label("離線解析 RX Packet HEX（不觸發 NFC）")
  val raw=EditText(this).apply {hint="50 42 01 …";minLines=2;inputType=InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE;column.addView(this)}
  action("解析貼上的 RX") {
   try {
    val compact=raw.text.toString().replace(Regex("\\s+"),"")
    require(compact.length%2==0 && compact.length in 16..512 && compact.matches(Regex("[0-9a-fA-F]+"))) {"需輸入 8～256 bytes 的 HEX"}
    val bytes=compact.chunked(2).map{it.toInt(16).toByte()}.toByteArray()
    val packet=PacketCodec.decode(bytes,expectedResponse=true)
    val reply=when(packet){is ParseResult.Error -> packet;is ParseResult.Ok -> PayloadCodec.decodeReply(packet.value.command,packet.value.payload)}
    lastDebug="來源：離線貼上（非裝置讀值）\nNFC UID：$uid\nTag Technologies：$tagTech\nRaw TX：無\nRaw RX：${PacketCodec.hex(bytes)}\nCRC：${PacketCodec.crcMatches(bytes)}\nParsed Command：${if(packet is ParseResult.Ok)packet.value.command else "不可解析"}\nParsed Status／結果：${summary(reply)}"
    renderDebug();message.text="離線解析完成；未更新裝置資料區域"
   } catch(e:IllegalArgumentException){message.text="HEX 輸入錯誤：${e.message}"}
  }
  setContentView(ScrollView(this).apply{addView(column)})
  clearDecoded();refresh()
 }
 private fun label(text:String)=TextView(this).apply{this.text=text;textSize=17f;setTextIsSelectable(true);column.addView(this)}
 private fun section(title:String){label("\n$title").textSize=22f}
 private fun number(value:String)=EditText(this).apply{inputType=InputType.TYPE_CLASS_NUMBER;setSingleLine();setText(value);column.addView(this)}
 private fun action(text:String,run:()->Unit)=Button(this).apply{this.text=text;column.addView(this);controls.add(this);setOnClickListener{if(!busy)run()}}
 private fun addPeriod(period:ReminderPeriod) {
  val row=LinearLayout(this)
  val values=listOf(period.id,period.startMinute,period.endMinute,period.flags).map{value ->
   EditText(this).apply{inputType=InputType.TYPE_CLASS_NUMBER;setSingleLine();setText(value.toString());row.addView(this,LinearLayout.LayoutParams(0,LinearLayout.LayoutParams.WRAP_CONTENT,1f))}
  }
  row.addView(Button(this).apply{text="刪";setOnClickListener{if(!busy){periods.removeView(row);periodRows.remove(values)}}})
  periods.addView(row);periodRows.add(values)
 }
 private fun editorConfig():PeboConfig {
  fun int(edit:EditText)=edit.text.toString().toIntOrNull() ?: throw IllegalArgumentException("欄位必須是整數")
  return PeboConfig(int(target),reminder.isChecked,periodRows.map{ReminderPeriod(int(it[0]),int(it[1]),int(it[2]),int(it[3]))})
 }
 private fun readHistory(useNext:Boolean) {
  val start=if(useNext) nextIndex else historyStart.text.toString().toIntOrNull()
  val count=historyCount.text.toString().toIntOrNull()
  if(start==null || start !in 0..65535 || count==null || count !in 1..255){message.text="History index/count 無效";return}
  if(!useNext){shownRecords.clear();lastPageStart=null}
  perform(PeboCommand.GET_HISTORY,RequestBody.History(start,count))
 }
 private fun perform(command:PeboCommand,body:RequestBody=RequestBody.Empty) {
  if(busy || !foreground)return
  busy=true;refresh()
  val ticket=generation
  val activeClient=client
  message.text="${source()}：$command 處理中"
  executor.execute {
   val report=try{activeClient.execute(command,body)}catch(e:RuntimeException){
    ExchangeReport(sourceForWorker(activeClient),byteArrayOf(),null,null,ParseResult.Error(ErrorKind.PAYLOAD,"執行失敗：${e.message}"))
   }
   runOnUiThread {
    busy=false
    if(isDestroyed)return@runOnUiThread
    if(ticket!=generation || !foreground){refresh();return@runOnUiThread}
    lastDebug="來源：${report.source}\nNFC UID（實際）：$uid\nTag Technologies：$tagTech\nRaw TX Packet HEX：${PacketCodec.hex(report.tx)}\nRaw RX Packet HEX：${report.rx?.let{PacketCodec.hex(it)} ?: "無回應／未傳送至裝置"}\nParsed Command：$command → ${"%02X".format(command.responseCode)}\nParsed Status：${summary(report.result)}\nCRC Result：${when(report.crcValid){true->"PASS";false->"FAIL";null->"尚未驗證"}}\nProfile：暫定 CCITT-FALSE；非 ISO15693 RF CRC"
    renderDebug()
    val parsed=report.result
    if(parsed is ParseResult.Ok && parsed.value.status==PeboStatusCode.OK){
     message.text="${report.source}：$command OK；${if(mode.isChecked)"僅 Sample，沒有裝置寫入" else "已解析"}"
     showBody(parsed.value.body)
     if(command==PeboCommand.CLEAR_HISTORY){shownRecords.clear();nextIndex=null;historyView.text="SAMPLE History 已清除"}
    } else {
     clearDecoded();message.text="${report.source}：$command 失敗：${summary(parsed)}"
    }
    refresh()
   }
  }
 }
 private fun sourceForWorker(unused:PeboClient)="操作來源" // no UI access from worker on exceptional path
 private fun summary(result:ParseResult<PeboReply>):String=when(result){
  is ParseResult.Error -> "Parse/Transport Error [${result.kind}] ${result.detail}"
  is ParseResult.Ok -> "${result.value.status} (0x%02X)".format(result.value.status.code)
 }
 private fun showBody(body:ResponseBody?) {
  val prefix="${source()}\n"
  when(body){
   is DeviceInfo -> infoView.text=prefix+"NFC UID（實際）：$uid\nPebo Device ID：${body.deviceId.joinToString(" "){"%02X".format(it)}}\nHW ${body.hwMajor}.${body.hwMinor}\nFW ${body.fwMajor}.${body.fwMinor}.${body.fwPatch}\nNFC Protocol ${body.protocolVersion}\nCapability Flags：%04X（位元語意待定）".format(body.capabilityFlags)
   is DeviceStatus -> statusView.text=prefix+formatDeviceStatus(body,::time)
   is PeboConfig -> {
    target.setText(body.dailyTarget.toString());reminder.isChecked=body.reminderEnabled
    periods.removeAllViews();periodRows.clear();body.periods.forEach{addPeriod(it)}
    configView.text=prefix+"已解析 ${body.periods.size} 個 Reminder Period；編輯後需另按 Write Config"
   }
   is HistoryPage -> {
    if(lastPageStart==null || body.startIndex!=nextIndex)shownRecords.clear()
    shownRecords.addAll(body.records);lastPageStart=body.startIndex;nextIndex=if(body.hasMore)body.nextIndex else null
    historyStart.setText(body.nextIndex.toString())
    historyView.text=prefix+"本頁 start=${body.startIndex}，${body.records.size} 筆／總共 ${body.totalRecords}；累計顯示 ${shownRecords.size} 筆\n"+
     shownRecords.mapIndexed{i,r -> "${i+1}. ${time(r.unixTime)}\nPeriod ID：${if(r.periodId==255)"FF（不屬時段）" else r.periodId.toString()}；Flags：%02X".format(r.flags)+" [Valid=${r.validDrink}, Completion=${r.periodCompletionEvent}, Reminder=${r.reminderWasActive}, Test=${r.manualTestEvent}]"}.joinToString("\n")
   }
   Acknowledgement -> Unit
   null -> Unit
  }
 }
 private fun clearDecoded() {
  infoView.text="${source()}：尚未取得有效 Device Info"
  statusView.text="${source()}：尚未取得有效 Status"
  configView.text="${source()}：編輯草稿，尚未取得本次 Config"
  historyView.text="${source()}：尚未取得有效 History"
  shownRecords.clear();nextIndex=null;lastPageStart=null
 }
 private fun time(timestamp:Long):String {
  val utc=SimpleDateFormat("yyyy-MM-dd HH:mm:ss 'UTC'",Locale.US).apply{timeZone=TimeZone.getTimeZone("UTC")}
  val local=SimpleDateFormat("yyyy-MM-dd HH:mm:ss Z",Locale.US)
  return "${utc.format(Date(timestamp*1000))}（本地 ${local.format(Date(timestamp*1000))}）"
 }
 private fun source()=if(mode.isChecked)"SAMPLE 模擬資料（非真實裝置）" else "真實模式（尚未綁定 transport）"
 private fun renderDebug(){debugView.text=lastDebug}
 private fun refresh(){
  banner.text=source()
  nfcView.text="NFC 支援：${adapter!=null}／開啟：${adapter?.isEnabled==true}"
  uidView.text="NFC UID（Android 原始 byte order）：$uid\nTechnologies：$tagTech\nUID 與 Pebo Device ID 不同"
  mode.isEnabled=!busy
  controls.forEach{it.isEnabled=!busy}
  next.isEnabled=!busy && nextIndex!=null
  target.isEnabled=!busy;reminder.isEnabled=!busy
  periodRows.flatten().forEach{it.isEnabled=!busy}
 }
 override fun onResume(){
  super.onResume();foreground=true
  try{if(adapter?.isEnabled==true)adapter!!.enableReaderMode(this,this,NfcAdapter.FLAG_READER_NFC_V or NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK,null)}
  catch(e:RuntimeException){message.text="NFC Reader Mode：${e.javaClass.simpleName}"}
  refresh()
 }
 override fun onPause(){
  foreground=false;generation++
  if(busy)message.text="操作畫面已取消；Sample 可能已更新，請返回後重新讀取"
  try{adapter?.disableReaderMode(this)}catch(_:RuntimeException){}
  super.onPause()
 }
 override fun onDestroy(){executor.shutdown();super.onDestroy()}
 override fun onTagDiscovered(tag:Tag){runOnUiThread{
  if(!foreground || isDestroyed)return@runOnUiThread
  uid=PacketCodec.hex(tag.id);tagTech=tag.techList.joinToString(","){it.substringAfterLast('.')}
  refresh();message.text="實際 NFC 標籤已偵測；${source()}，未由標籤取得 protocol 資料"
 }}
}
