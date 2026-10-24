package com.pebo.nfcprobe20261015.transport

import android.nfc.tech.NfcV
import com.pebo.nfcprobe20261015.SingleBlock

// ST/ISO15693 engineering adapter. No Pebo packet/EEPROM mapping is assumed.
// Lifecycle/connect/close ownership stays with the existing read Activity.
class St25NfcVBlockTransport(private val nfcV: NfcV) {
 fun readSingleBlock(rawAndroidUid: ByteArray,block: Int): ByteArray =
  nfcV.transceive(SingleBlock.command(rawAndroidUid,block))
}