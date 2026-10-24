// Pebo v0.1: UART-driven eight-step blue LED display, no STOP integration.
#include <Arduino.h>
#include <HardwareTimer.h>
#include <stdio.h>
#include "ProgressLogic.h"

using namespace BlueProgress;

// User-authorized provisional fill order, 2026-10-25; visual verification pending.
static constexpr uint32_t kLedPins[4] = {PA5, PA6, PA7, PA8};
static constexpr uint32_t kUartBaud = 115200;
static constexpr uint16_t kTxBytes = 256;
static constexpr uint16_t kMaxReply = 96;
static constexpr uint8_t kRxBudget = 32;
static_assert(kHighDutyPercent == 100, "Shared TIM2_CH1 requires high LEDs to be GPIO high");
static_assert(kLowDutyPercent > 0 && kLowDutyPercent < kHighDutyPercent, "Invalid brightness");

static HardwareTimer LedTimer2(TIM2);
static HardwareTimer LedTimer22(TIM22);
static Uart DebugUart(PA10, PA9);
static Display display;
static LineParser parser;
static char tx[kTxBytes];
static uint16_t txRead = 0;
static uint16_t txWrite = 0;

static void stopLedOutputs() {
  LedTimer2.pauseChannel(1);
  LedTimer22.pauseChannel(1);
  LedTimer22.pauseChannel(2);
  LedTimer2.pause();
  LedTimer22.pause();
  LedTimer2.setMode(1, TIMER_OUTPUT_DISABLED);
  LedTimer22.setMode(1, TIMER_OUTPUT_DISABLED);
  LedTimer22.setMode(2, TIMER_OUTPUT_DISABLED);
  for (uint8_t i = 0; i < 4; ++i) {
    digitalWrite(kLedPins[i], LOW);
    pinMode(kLedPins[i], OUTPUT);
  }
}

static void render(uint8_t pattern) {
  stopLedOutputs();
  for (uint8_t i = 0; i < 4; ++i) {
    const uint8_t value = (pattern >> (2 * i)) & 3;
    if (value == 2) {
      digitalWrite(kLedPins[i], HIGH);
    } else if (value == 1) {
      HardwareTimer &timer = (i == 0 || i == 3) ? LedTimer2 : LedTimer22;
      const uint32_t channel = i == 2 ? 2 : 1;
      // Force the pin AF on every start: PA5 and PA8 share the same timer/channel.
      timer.setMode(channel, TIMER_OUTPUT_COMPARE_PWM1, kLedPins[i]);
      timer.setOverflow(kPwmHz, HERTZ_FORMAT);
      timer.setCaptureCompare(channel, kLowDutyPercent, PERCENT_COMPARE_FORMAT);
      timer.setCount(0);
      timer.refresh();
      timer.resume();
    }
  }
}

void allLedsOff() {
  display.off();
  stopLedOutputs();
}

void showProgress(uint8_t level) {
  if (display.show(level, static_cast<uint32_t>(millis()))) render(display.pattern);
}

static uint16_t txFree() {
  return (txRead + kTxBytes - txWrite - 1) % kTxBytes;
}

static void queueReply(const char *text) {
  while (*text) {
    tx[txWrite] = *text++;
    txWrite = (txWrite + 1) % kTxBytes;
  }
}

static void drainReplies() {
  while (txRead != txWrite && DebugUart.availableForWrite() > 0) {
    DebugUart.write(static_cast<uint8_t>(tx[txRead]));
    txRead = (txRead + 1) % kTxBytes;
  }
}

static void handleCommand(Command command) {
  const char *action = "level";
  switch (command.kind) {
    case Kind::Level: showProgress(command.level); break;
    case Kind::Show: showProgress(display.level); action = "show"; break;
    case Kind::Off: allLedsOff(); action = "off"; break;
    case Kind::Help:
      allLedsOff();
      queueReply("OK help commands=0..8,show,off,help\r\n"); return;
    case Kind::Invalid: queueReply("ERR invalid\r\n"); return;
    case Kind::TooLong: queueReply("ERR too_long\r\n"); return;
    case Kind::None: return;
  }
  char pattern[5] = {};
  for (uint8_t i = 0; i < 4; ++i) pattern[i] = "OLH"[(display.pattern >> (2 * i)) & 3];
  char reply[kMaxReply];
  snprintf(reply, sizeof(reply), "OK %s level=%u pattern=%s\r\n", action,
           static_cast<unsigned>(display.level), pattern);
  queueReply(reply);
}

void setup() {
  allLedsOff();
  pinMode(PA4, OUTPUT);
  digitalWrite(PA4, HIGH);
  DebugUart.begin(kUartBaud);
}

void loop() {
  drainReplies();
  for (uint8_t count = 0; count < kRxBudget && DebugUart.available() > 0 &&
       txFree() >= kMaxReply; ++count) {
    const int value = DebugUart.read();
    if (value >= 0) handleCommand(parser.push(static_cast<char>(value)));
  }
  const uint8_t previous = display.pattern;
  display.update(static_cast<uint32_t>(millis()));
  if (previous != display.pattern) render(display.pattern);
  drainReplies();
}
