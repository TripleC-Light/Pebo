#pragma once

#include <stddef.h>
#include <stdint.h>

namespace BlueProgress {
constexpr uint8_t kLowDutyPercent = 25;
constexpr uint8_t kHighDutyPercent = 100;
constexpr uint32_t kPwmHz = 1000;
constexpr uint32_t kHoldMs = 3000;
constexpr uint32_t kZeroOnMs = 120;
constexpr uint32_t kZeroGapMs = 120;
constexpr size_t kInputBytes = 16;

// Two bits per LED, starting at LED1: 0=off, 1=low, 2=high.
constexpr uint8_t patternFor(uint8_t level) {
  uint8_t pattern = 0;
  for (uint8_t led = 0; led < 4; ++led) {
    const uint8_t value = level >= 2 * led + 2 ? 2 :
                          level == 2 * led + 1 ? 1 : 0;
    pattern |= static_cast<uint8_t>(value << (2 * led));
  }
  return pattern;
}

enum class Phase : uint8_t { Off, Hold, ZeroBlink };

struct Display {
  uint8_t level = 0;
  uint8_t pattern = 0;
  Phase phase = Phase::Off;
  uint32_t started = 0;

  constexpr bool show(uint8_t next, uint32_t now) {
    if (next > 8) return false;
    level = next;
    started = now;
    phase = next == 0 ? Phase::ZeroBlink : Phase::Hold;
    pattern = next == 0 ? 1 : patternFor(next);
    return true;
  }

  constexpr void off() {
    pattern = 0;
    phase = Phase::Off;
  }

  constexpr void update(uint32_t now) {
    const uint32_t elapsed = now - started;
    if (phase == Phase::Hold) {
      if (elapsed >= kHoldMs) off();
    } else if (phase == Phase::ZeroBlink) {
      if (elapsed >= 2 * kZeroOnMs + kZeroGapMs) {
        off();
      } else {
        pattern = (elapsed < kZeroOnMs || elapsed >= kZeroOnMs + kZeroGapMs) ? 1 : 0;
      }
    }
  }
};

enum class Kind : uint8_t { None, Level, Show, Off, Help, Invalid, TooLong };
struct Command { Kind kind; uint8_t level; };

class LineParser {
 public:
  constexpr Command push(char value) {
    if (value != '\n') {
      if (length_ < kInputBytes - 1 && !discard_) line_[length_++] = value;
      else discard_ = true;
      return {Kind::None, 0};
    }
    Command result{Kind::Invalid, 0};
    if (discard_) {
      result.kind = Kind::TooLong;
    } else {
      if (length_ && line_[length_ - 1] == '\r') --length_;
      if (length_ == 1 && line_[0] >= '0' && line_[0] <= '8')
        result = {Kind::Level, static_cast<uint8_t>(line_[0] - '0')};
      else if (matches("show", 4)) result.kind = Kind::Show;
      else if (matches("off", 3)) result.kind = Kind::Off;
      else if (matches("help", 4)) result.kind = Kind::Help;
    }
    length_ = 0;
    discard_ = false;
    return result;
  }

 private:
  constexpr bool matches(const char *word, uint8_t length) const {
    if (length_ != length) return false;
    for (uint8_t i = 0; i < length; ++i) if (line_[i] != word[i]) return false;
    return true;
  }
  char line_[kInputBytes] = {};
  uint8_t length_ = 0;
  bool discard_ = false;
};
}
