package com.pebo.nfcprobe20261015

import com.pebo.nfcprobe20261015.protocol.DeviceStatus

internal fun formatDeviceStatus(body:DeviceStatus,time:(Long)->String):String =
 "Battery：${body.batteryMillivolts} mV／${body.batteryPercent}%\n"+
 "Battery Status：${body.batteryStatus}（語意待定）\nDevice Time：${time(body.deviceUnixTime)}\n"+
 "Today Drink Events：${body.todayDrinkEventCount}\n"+
 "Completed Periods：${body.completedReminderPeriodCount} / ${body.totalReminderPeriodCount}\n"+
 "Daily Target：${body.dailyTarget}\nLED Progress：${body.ledProgress} / 8\n"+
 "Last Drink：${time(body.lastDrinkUnixTime)}\n"+
 "Device Status Flags：${"%04X".format(body.deviceStatusFlags)}（語意待定）"
