# Task Plan: Separate hardware-touching code from logic-only code for three more functions

**Task:** TASK-003
**Status:** Draft
**Created:** 2026-10-09

## Summary

Extract three pieces of logic that TASK-001 identified but explicitly deferred —
brightness-index wraparound, display-mode toggle, and digit-from-time/date
decomposition — out from behind `Arduino.h` (and, for the digit decomposition, out
from behind RTClib's `DateTime` type) into plain C++ functions, each with a native
Unity test, following the pattern TASK-001 established for button press-duration
classification.

## Motivation

CLAUDE.md's hardware-adapter-layer convention calls for algorithmic logic to live in
plain C++ that doesn't include `Arduino.h`, confined to a thin adapter layer
otherwise. TASK-001 built the `env:native` test environment and proved the pattern
with one extraction (`classify_press_duration()`), but its Scope section explicitly
deferred "brightness-index wraparound ... digit-from-time computation ... debounce
timing itself" as separate future candidates. This task picks up the first two of
those three and adds a related third (the display-mode toggle), per user request, to
set the stage for more complete unit-test coverage.

## Related requirements / constraints

None found in `docs/requirements/` or `docs/constraints/` — this task stems from
CLAUDE.md's testing/hardware-adapter convention, not a tracked FR/NFR/IC. The logic
being extracted underpins:
- FR-003 (brightness adjustment via push-button) — brightness-index wraparound.
- FR-001 (display mode switchable between MM:SS/HH:MM) — display-mode toggle.
- FR-002 (accurate time via RTC) and UC-002 — digit-from-time/date decomposition.

This task changes none of that behavior; it only relocates and tests existing logic.

## Scope

**In scope:**

1. **Brightness-index wraparound** (`src/main.cpp:89-93`,
   `input_switch_quick_press()`):
   - New `include/brightness.h` / `src/brightness.cc`, no `Arduino.h` include.
   - `uint8_t next_brightness_index(uint8_t current_index, uint8_t level_count)` —
     returns `0` if `current_index == level_count - 1`, else `current_index + 1`.
     Matches the existing inline ternary exactly; `level_count` is computed at the
     call site the same way it is today
     (`sizeof(brightness_count)/sizeof(brightness_count[0])`).
   - `input_switch_quick_press()` becomes: compute the new index via
     `next_brightness_index()`, assign it to `brightness`, then keep the existing
     `DPRINTV("brightness: %d\n", brightness)` and `analogWrite(...)` calls at the
     call site (logging and hardware I/O are not part of the pure function).

2. **Display-mode toggle** (`src/main.cpp:95-109`, `toggle_display_mode()`):
   - Declare `enum display_mode toggle_display_mode(enum display_mode display_mode)`
     in `include/mode_switch2.h` (which already defines `enum display_mode` and has
     no `Arduino.h` dependency — no new header needed).
   - Move the implementation to a new `src/display_mode.cc`, with the function body
     unchanged except dropping the `DPRINTV(...)` call (which depends on
     `print.h`'s `F()`/`Arduino.h`-coupled macro). The equivalent log line moves to
     the call site in `main.cpp`'s `loop()`, immediately after the call to
     `toggle_display_mode()`.

3. **Digit-from-time/date decomposition** (`src/RTC.cc:37-58`,
   `update_display_with_time()` / `update_display_with_date()`):
   - New `include/display_digits.h` / `src/display_digits.cc`, no `Arduino.h` or
     RTClib include.
   - A plain struct `display_digits { int d0, d1, d2, d3, d4, d5; };` and two pure
     functions:
     - `display_digits digits_from_time(int hour, int minute, int second)`
     - `display_digits digits_from_date(int year, int month, int day)`
     reproducing the existing `%10` / `/10` arithmetic exactly (including the
     existing `(year - 2000) / 10` behavior for the two-digit year).
   - `update_display_with_time()` / `update_display_with_date()` in `RTC.cc` become
     thin wrappers: call the new function with `dt.hour()`, `dt.minute()`, etc.,
     and assign the result's fields to the six `volatile int digit_N` globals.
     `DateTime` itself stays confined to `RTC.cc`.

4. **`platformio.ini` / test infrastructure:**
   - Add `brightness.cc`, `display_mode.cc`, and `display_digits.cc` to
     `[env:native]`'s `build_src_filter` (alongside the existing
     `+<button_timing.cc>`).
   - Add `test/test_brightness/test_brightness.cpp`,
     `test/test_display_mode/test_display_mode.cpp`, and
     `test/test_display_digits/test_display_digits.cpp` (Unity, following
     `test/test_button_timing/test_button_timing.cpp`'s structure).

**Out of scope:**
- Debounce timing and the ISR handlers in `mode_switch2.cc`
  (`button_1_interrupt_handler()`, `button_2_interrupt_handler()`,
  `read_button_1()`, `read_button_2()`) — TASK-001 already deferred debounce timing
  itself, and this task doesn't revisit that call.
- `toggle_separator()` in `RTC.cc` — its only non-hardware content is a single
  boolean flip, not substantial enough to extract on its own.
- Removing `update_display_with_date()` despite it currently having no call site
  (confirmed via `grep -rn "update_display_with_date" src include` — only the
  definition at `RTC.cc:49` and no callers). It's wired up the same as
  `update_display_with_time()` by this task, but whether the function itself is
  dead code is a separate question from hardware/logic separation — flagged here
  as a candidate for a future Code Cleanup task, not addressed in this one.
- Changing `brightness`'s or `digit_0`..`digit_5`'s types away from `volatile int`
  — out of scope; this task only changes how their new values are computed.
- Enabling `-Werror` or resolving any existing `pio check` findings outside the
  files this task touches.
- Introducing a `lib/` subfolder layout (same exclusion TASK-001 made).

## Approach

1. Add `include/brightness.h` / `src/brightness.cc` with `next_brightness_index()`.
   Update `main.cpp`'s `input_switch_quick_press()` to call it.
2. Add `toggle_display_mode()`'s declaration to `include/mode_switch2.h`; move its
   implementation to `src/display_mode.cc`, dropping the internal `DPRINTV` call;
   add the equivalent log line at the call site in `main.cpp`'s `loop()`. Remove the
   old definition from `main.cpp`.
3. Add `include/display_digits.h` / `src/display_digits.cc` with `display_digits`,
   `digits_from_time()`, `digits_from_date()`. Rewrite `RTC.cc`'s
   `update_display_with_time()` / `update_display_with_date()` to call them.
4. Update `[env:native]`'s `build_src_filter` in `platformio.ini` to include the
   three new `.cc` files.
5. Add the three new Unity test files under `test/`, each covering: a mid-range
   value, the wraparound/rollover boundary (e.g. `next_brightness_index()` at
   `level_count - 1`), and the two-digit split behavior (e.g. `digits_from_time()`
   at `hour=0`, `hour=23`, `minute=0`, `second=59`; `digits_from_date()` at a
   two-digit-year boundary like `year=2000` and `year=2099`).
6. Enumerate the three extractions as a checklist and confirm each is complete:
   - [ ] `next_brightness_index()` extracted, `main.cpp` has no inline wraparound
     arithmetic left (`grep -n "brightness ==" src/main.cpp` returns nothing).
   - [ ] `toggle_display_mode()` moved out of `main.cpp`
     (`grep -n "^enum display_mode toggle_display_mode" src/main.cpp` returns
     nothing; the declaration lives only in `mode_switch2.h`).
   - [ ] `digits_from_time()` / `digits_from_date()` extracted, `RTC.cc`'s two
     `update_display_with_*` functions contain no `%10`/`/10` arithmetic directly
     (`grep -n "% 10\|/ 10" src/RTC.cc` returns nothing outside the new file).
7. Run `pio test -e native` and confirm all four test suites
   (`button_timing` + the three new ones) pass.
8. Run `pio run -e uno` and `pio run -e pro16MHzatmega328` and confirm both still
   build clean.
9. Run `pio check` and `clang-format` over every touched/added file.

## Verification

- `pio test -e native` passes, covering all cases listed in Approach step 5.
- `pio run -e uno` and `pio run -e pro16MHzatmega328` both build with no new
  warnings beyond whatever the codebase already emits.
- All three checklist greps in Approach step 6 come back empty, confirming the
  logic was relocated rather than duplicated.
- Manual read-through of each new pure function against the code it replaced
  confirms identical arithmetic and boundary behavior — a behavior change here
  would silently alter FR-001/FR-003 behavior or UC-002's displayed time.
- `pio check` runs clean (or any new finding justified inline per CLAUDE.md).

## Risks

- Dropping the `DPRINTV` call from inside `toggle_display_mode()` and
  `input_switch_quick_press()`'s pure half, and re-adding equivalent logging at the
  call site, is a small structural change to *where* a debug line is emitted
  (same information, same logical point in `loop()`) — low risk, but worth a
  deliberate diff check rather than assuming it's a no-op.
- `digits_from_time()`/`digits_from_date()` replacing direct `DateTime` field
  access with explicit `int` parameters relies on `DateTime::hour()`/`minute()`/
  etc. all returning types that convert losslessly to `int` — true today, but worth
  confirming during implementation rather than assumed.
- Low overall risk otherwise: all three extractions are small, pure, and intended
  to be behavior-preserving; the main failure mode is a subtle arithmetic slip
  between old and new code, which the checklist greps and manual read-through are
  meant to catch.

## Open questions

- None blocking.
