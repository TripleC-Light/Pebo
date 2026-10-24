#include "../ProgressLogic.h"
using namespace BlueProgress;

static_assert(patternFor(0) == 0x00 && patternFor(1) == 0x01 &&
              patternFor(2) == 0x02 && patternFor(3) == 0x06 &&
              patternFor(4) == 0x0A && patternFor(5) == 0x1A &&
              patternFor(6) == 0x2A && patternFor(7) == 0x6A &&
              patternFor(8) == 0xAA, "Nine progress patterns");

constexpr bool timingChecks() {
  Display state;
  if (state.level || state.pattern || state.phase != Phase::Off) return false;
  state.show(7, 0xFFFFFF00U);
  state.update(static_cast<uint32_t>(0xFFFFFF00U + 2999U));
  if (state.pattern != 0x6A) return false;
  state.update(static_cast<uint32_t>(0xFFFFFF00U + 3000U));
  if (state.pattern || state.level != 7) return false;
  state.show(state.level, 5000);
  state.off();
  if (state.level != 7 || state.pattern) return false;
  state.show(1, 6000);
  state.show(8, 8999);
  state.update(9000);
  if (state.pattern != 0xAA) return false;
  state.show(state.level, 10000);
  state.update(12999);
  if (state.pattern != 0xAA) return false;
  state.update(13000);
  return state.pattern == 0 && state.level == 8;
}
static_assert(timingChecks(), "Hold, wrap, interruption, off retention, show restart");

constexpr bool zeroChecks() {
  Display state;
  state.show(0, 0xFFFFFF80U);
  const uint32_t start = state.started;
  if (state.pattern != 1) return false;
  state.update(start + kZeroOnMs - 1);
  if (state.pattern != 1) return false;
  state.update(start + kZeroOnMs);
  if (state.pattern != 0) return false;
  state.update(start + kZeroOnMs + kZeroGapMs);
  if (state.pattern != 1) return false;
  state.update(start + 2 * kZeroOnMs + kZeroGapMs);
  if (state.pattern != 0 || state.phase != Phase::Off) return false;
  state.show(0, 500);
  state.show(4, 520);
  state.update(900);
  if (state.pattern != 0x0A || state.level != 4) return false;
  state.show(0, 1000);
  state.off();
  state.update(1240);
  return state.pattern == 0 && state.level == 0;
}
static_assert(zeroChecks(), "Zero double blink, wrap and cancellation");

constexpr Command feed(LineParser &parser, const char *text) {
  Command result{Kind::None, 0};
  while (*text) result = parser.push(*text++);
  return result;
}

constexpr bool parserChecks() {
  LineParser parser;
  if (feed(parser, "8\r\n").kind != Kind::Level) return false;
  if (feed(parser, "show\n").kind != Kind::Show) return false;
  if (feed(parser, "off\n").kind != Kind::Off) return false;
  if (feed(parser, "help\n").kind != Kind::Help) return false;
  const char *invalid[] = {"9\n", "-1\n", "1x\n", "01\n", "\n", "SHOW\n", "sh\row\n"};
  for (const char *bad : invalid)
    if (feed(parser, bad).kind != Kind::Invalid) return false;
  if (feed(parser, "12345678901234567890show\n").kind != Kind::TooLong) return false;
  if (feed(parser, "2\n").level != 2) return false;
  if (feed(parser, "sh").kind != Kind::None) return false;
  if (feed(parser, "ow\n").kind != Kind::Show) return false;
  parser.push('s'); parser.push('h'); parser.push('\0'); parser.push('w');
  return parser.push('\n').kind == Kind::Invalid;
}
static_assert(parserChecks(), "Bounded parser, CRLF, errors, overflow recovery, fragments, NUL");

constexpr bool invalidPreservesState() {
  Display state;
  state.show(6, 100);
  if (state.show(9, 200)) return false;
  return state.level == 6 && state.pattern == 0x2A && state.started == 100;
}
static_assert(invalidPreservesState(), "Invalid level must not restart or change display");
