# What code in `src`/`include` lacks unit test coverage that could reasonably have it, and what code is unused?

**Date:** 2026-10-10
**Commit:** b1ad9d6

## Question

Analyze `src` (and its `include/` headers) for test coverage gaps — logic that could
plausibly be unit-tested under `env:native` but isn't — and for unused/dead code.

## Method

Read every file in `src/`/`include/`. Cross-referenced `platformio.ini`'s
`[env:native]` `build_src_filter` (which names the five files currently considered
Arduino-independent: `button_timing.cc`, `brightness.cc`, `display_mode.cc`,
`display_digits.cc`, `clock_mode.cc`) against `test/` to confirm what's actually
tested today. For "unused code," grepped every function/macro name across
`src/`, `include/`, and `test/` to find definitions with no call site anywhere
(`hv_ps_test/`, `retired/`, `PCB/` excluded per CLAUDE.md's ignore list).

## Findings — test coverage

**Already well covered:** all five files in `env:native`'s filter have a
corresponding `test/test_*/` suite — 33 Unity cases total (`pio test -e native`,
all passing as of this commit), covering boundary wraps in both directions for
`clock_mode.cc`'s adjust functions, mode/field cycling, and the three
`blank_if_selected()` cases, plus straightforward coverage for brightness,
display-mode toggling, button-duration classification, and digit decomposition.

**Gap — `src/RTC.cc`'s blink/redraw state machine (`timer_2HZ_tick_ISR()`,
`blink_phase`/`update_display`/`refetch_time`, `blink_off_phase()`):** This is
hardware-adapter code (includes `Arduino.h`, calls `attachInterrupt()`), so it was
never a target for `env:native` — but the actual *decision logic* (given the current
`blink_phase` and that an edge just fired, what's the next `blink_phase`, and should
`update_display`/`refetch_time` be set) has no Arduino dependency once isolated from
the `attachInterrupt()` call itself. **This is the exact logic where BUG-003 lived**
(`docs/bugs/BUG-LOG.md`) — the display-write cadence was phase-locked to always
render the selected field blanked. A native test asserting the phase/flag sequence
across several simulated ticks would plausibly have caught it before it shipped.
Worth flagging precisely because NFR-001 (the ~1 Hz blink requirement) depends on
this logic being right, and it's currently verified only by on-target observation.

**Smaller gap — `commit_hour_or_minute()` / `commit_month_day_or_year()`
(`RTC.cc:193-225`):** The field/direction dispatch (`if (field == field_hour) ...`)
is small, but it's fused with `DateTime` construction and `rtc.adjust()`, so today
it's exercised only by the original feature plan's Phase 4 on-target walkthrough,
not a native test.

**Smaller gap — `time_update_handler()`'s refetch/redraw decision tree
(`RTC.cc:238-...`):** Same kind of entanglement (flags mixed with `rtc.now()`),
lower stakes than the blink state machine above.

**Known, already-documented gap (not new):** debounce/press-duration detection in
`read_button_1()`/`read_button_2()` (`mode_switch2.cc`) remains untested — TASK-001
and TASK-003 both explicitly deferred extracting this, so this isn't a new finding.

## Findings — unused/dead code

- **`print_digits()`** (`src/RTC.cc:65-67`) — defined, not declared in `RTC.h`
  (not even exposed outside its translation unit), and has zero call sites anywhere
  in `src/`, `include/`, or `test/`. Its `newline` parameter is also unused inside
  the function body (the function always prints a trailing `\n` regardless).
- **`DPRINTF` macro** (`include/print.h:25` and `:30`, the `DEBUG`/non-`DEBUG`
  variants) — defined, zero call sites anywhere. Also carries a latent
  macro-hygiene issue: `Serial.print(fmt); Serial.println(f)` as a bare
  multi-statement body (no `do { } while (0)` wrapper) would silently execute only
  the first statement if ever used inside a brace-less `if`.
- **`frac(f)` macro** (`include/print.h:17`, plus its commented-out predecessor on
  line 16) — defined, zero call sites anywhere.
- **`SWITCH_PRESS_10S`** (`include/button_timing.h:13`) — defined, zero references;
  `classify_press_duration()` (`src/button_timing.cc:4-10`) only branches on
  `SWITCH_PRESS_2S`/`SWITCH_PRESS_5S`.
- **Three `#if 0`-disabled blocks:**
  - `src/main.cpp:62-66` — calls `input_switch_push`, which doesn't exist anywhere
    else in the tree; this block would fail to compile if re-enabled as-is.
  - `src/mode_switch2.cc:49-51` — a disabled file-scope `volatile enum display_mode
    the_display_mode = mm_ss;`, superseded by the `static enum display_mode
    the_display_mode` local in `main.cpp`'s `loop()`.
  - `src/mode_switch2.cc:188-199` — a disabled example `loop()` left over from the
    imported Arduino tutorial this file is based on (see the file's header
    comment). The only live reader of `led_status` (`mode_switch2.cc:40`) and the
    `switched` macro (`mode_switch2.cc:44`) is this same dead block — both are
    otherwise unused.

`pio check`'s cppcheck backend flags most public functions in `src/` as
`unusedFunction` — that's a single-translation-unit analysis artifact (it can't see
cross-file callers from `main.cpp`) for everything except `print_digits()`, which
really is unused.

## Cross-check against requirements

NFR-001 ("the digit pair currently selected for editing must be visually
distinguishable ... blinks at ~1 Hz") is the requirement most directly exposed by
the `RTC.cc` blink-state-machine coverage gap above — flagged as an observation to
verify (the requirement itself still reads correctly; it's the lack of a regression
test for the mechanism that satisfies it that stood out). No drift found between
FR-001/FR-003 and their corresponding tested logic (`toggle_display_mode()`,
`next_brightness_index()`).

## Suggested next steps (not taken here)

- Extracting the blink/redraw state machine out of `RTC.cc` into testable plain
  C++ (mirroring TASK-001/TASK-003's pattern) is a reasonable `/plan-task`
  candidate, given it's the exact logic that just regressed once.
- Removing `print_digits()`, the unused macros, and the three `#if 0` blocks is a
  reasonable `/plan-task` candidate (mechanical, low-risk cleanup) — not done here,
  per `/deep-dive`'s read-only scope.
