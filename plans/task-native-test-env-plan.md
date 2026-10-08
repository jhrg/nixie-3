# Task Plan: Add native PlatformIO test environment, extracting button press-duration classification as a first unit-testable module

**Task:** TASK-001
**Status:** Draft
**Created:** 2026-10-07

## Summary

Add an `[env:native]` PlatformIO environment so hardware-independent logic can be
unit-tested on the host, and prove it out by extracting the button
press-duration classification (elapsed ms → `quick` / `medium_2s` / `long_5s`) out
from behind `Arduino.h` into a plain C++ function with a real Unity test.

## Motivation

CLAUDE.md's testing convention requires an `env:native` target so logic-layer code
gets host-run unit tests, but `platformio.ini` currently only defines `uno`,
`pro16MHzatmega328`, and `hv_ps_test` — there is no way to run any test without a
board attached. Flagged as a gap during the `/init-planning` session
(2026-10-07) rather than invented independently.

Scaffolding alone (an empty `env:native` with no real test) would prove the
toolchain works but not that it's useful, so this task also extracts one
self-contained piece of logic to validate the pattern end-to-end.

## Related requirements / constraints

None found in `docs/requirements/` or `docs/constraints/` — this task stems from
the testing convention in `CLAUDE.md`, not a tracked FR/NFR/IC. The logic being
extracted underpins button-driven behavior described in FR-001, FR-003, UC-001,
and UC-002, but this task changes nothing about that behavior — it only relocates
and tests existing logic.

## Scope

**In scope:**
- Add `[env:native]` to `platformio.ini` (`platform = native`, no `board =`),
  defined standalone rather than inheriting `${env.build_flags}` /
  `${env.lib_deps_builtin}` / `${env.lib_deps_external}` from `[env]`, since those
  pull in Arduino-only libraries (`SPI`, `Wire`, `PinChangeInterrupt`,
  `adafruit/RTCLib`) that don't exist for the native platform.
- Extract the press-duration classification logic currently duplicated in
  `read_button_1()` and `read_button_2()` (`src/mode_switch2.cc:128-134` and
  `:169-175`) into a new pure function:
  - `include/button_timing.h` — declares
    `enum switch_press_duration classify_press_duration(long elapsed_ms)`, plus the
    `SWITCH_PRESS_2S` / `SWITCH_PRESS_5S` threshold constants (currently `#define`d
    privately in `mode_switch2.cc`).
  - `src/button_timing.cc` — implementation. No `Arduino.h` include, no
    hardware calls — pure arithmetic over its input.
- Update `mode_switch2.cc` to call `classify_press_duration(elapsed)` in both
  `read_button_1()` and `read_button_2()` instead of the duplicated inline
  if/else chain. Thresholds and return values must be unchanged.
- Configure `env:native`'s `build_src_filter` (or equivalent) so the native test
  build compiles only `button_timing.cc` — excluding `main.cpp`,
  `mode_switch2.cc`, `RTC.cc`, and `print.cc`, all of which include `Arduino.h`
  and would fail to compile natively.
- Add `test/test_button_timing/test_button_timing.cpp` (PlatformIO/Unity
  convention) covering: a value just under 2000ms → `quick`; exactly at and just
  over the 2000ms boundary → `medium_2s`; just under and at/over the 5000ms
  boundary → `long_5s`; a 0ms / negative value if the function's contract allows
  it (decide during implementation whether negative elapsed is a defined input or
  a precondition violation, and document whichever is chosen).

**Out of scope:**
- Extracting any other logic (brightness-index wraparound in `main.cpp`,
  digit-from-time computation in `RTC.cc`, debounce timing itself) — each is a
  separate candidate for a future task, not folded into this one.
- Changing button behavior, thresholds, or adding the currently-unassigned
  `read_button_2()` `medium_2s` behavior (`main.cpp`'s `// Replace this with ...`
  comment) — unrelated to test infrastructure.
- Introducing a `lib/` subfolder structure for this or future modules — this task
  keeps the project's existing flat `include/` + `src/` convention rather than
  introducing a new layout choice as a side effect.
- Enabling `-Werror` or resolving any existing `pio check` findings outside the
  files this task touches.

## Approach

1. Add `include/button_timing.h` and `src/button_timing.cc` with
   `classify_press_duration()`, moving the `SWITCH_PRESS_2S` / `SWITCH_PRESS_5S`
   constants there from `mode_switch2.cc`.
2. Update both call sites in `src/mode_switch2.cc` to use the new function;
   remove the now-duplicated inline logic. `grep -n "SWITCH_PRESS_"
   src/mode_switch2.cc` should return zero matches afterward (the constants now
   live only in `button_timing.h`).
3. Add `[env:native]` to `platformio.ini` per the Scope section above.
4. Add the Unity test file under `test/test_button_timing/`.
5. Run `pio test -e native` and confirm it passes.
6. Run `pio run -e uno` and `pio run -e pro16MHzatmega328` and confirm both still
   build clean — the extraction must not change the board builds' behavior.
7. Run `pio check` and `clang-format` (per CLAUDE.md) over every touched/added
   file.

## Verification

- `pio test -e native` passes, covering all boundary cases listed above.
- `pio run -e uno` and `pio run -e pro16MHzatmega328` both build with no new
  warnings beyond whatever the codebase already emits.
- `grep -n "SWITCH_PRESS_" src/mode_switch2.cc` returns nothing (constants fully
  relocated, not duplicated).
- Manual read-through of `classify_press_duration()` against the original inline
  logic in both `read_button_1()` and `read_button_2()` confirms identical
  threshold values and boundary behavior (`>` vs `>=`) — a behavior change here
  would silently alter UC-001/UC-002 press-duration detection.
- `pio check` run clean (or any new finding justified inline per CLAUDE.md).

## Risks

- `env:native` accidentally inheriting `[env]`'s `lib_deps`/`build_flags` would
  try to fetch or compile Arduino-only libraries for the native platform and
  fail the build — mitigated by defining it standalone (see Scope).
- If PlatformIO's native test runner pulls in more of `src/` by default than
  intended (test builds can implicitly include library-resolved sources),
  `build_src_filter`/`test_filter` must be checked carefully, not assumed from
  the other `[env:...]` sections' filters.
- Low overall risk: the extracted function is small, pure, and has no behavior
  change intended — the main failure mode is a subtle `>` vs `>=` boundary
  mismatch between old and new code, which the Verification step's manual
  read-through and the explicit boundary test cases are meant to catch.

## Open questions

- None blocking. One implementation-time decision, not a blocker: whether
  `classify_press_duration()` treats a negative `elapsed_ms` as a defined input
  (e.g. always `quick`) or as a precondition the caller must never violate —
  pick whichever during implementation and note it in the function's one-line
  doc comment per CLAUDE.md's documentation convention.
