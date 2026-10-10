<!-- CONTRACT: scoped engineering/maintenance work that isn't new capability (FR/NFR/
     UC) and isn't corrective (BUG) — dependency bumps, removing dead code or stale
     compile-time directives, retrofitting tests onto code that predates them,
     build/tooling changes. The common thread: the "right" outcome is usually
     mechanical and completeness matters (did every instance get handled?), not
     open-ended design. Delete the TASK-EXAMPLE section once you've seen the shape. -->

# Task Log

Convention: `TASK-###`, sequential, never renumbered. Status: `Open` → `Plan Ready` →
`In Progress` → `Done` (or `Won't Do` / `Superseded by TASK-0XX`). **Only the user sets
`Done`** — it's a claim about verified work, not something to self-report from having
written a plan. Category is one of: Dependency
Update, Code Cleanup, Test Coverage, Build/Tooling, Documentation (add categories as
needed).

---

## TASK-001 — Add native PlatformIO test environment, extracting button press-duration classification as a first unit-testable module

**Category:** Build/Tooling
**Status:** In Progress
**Created:** 2026-10-07
**Related requirements/constraints:** None found — stems from CLAUDE.md's testing
convention, not a tracked FR/NFR/IC. Underpins button behavior in FR-001, FR-003,
UC-001, UC-002 without changing it.
**Plan:** `plans/task-native-test-env-plan.md`

**Scope:** Add `[env:native]` to `platformio.ini`, standalone (not inheriting
`[env]`'s Arduino-only `lib_deps`/`build_flags`). Extract the press-duration
classification currently duplicated in `read_button_1()`/`read_button_2()`
(`src/mode_switch2.cc`) into `include/button_timing.h` + `src/button_timing.cc`,
with no `Arduino.h` dependency, and add a Unity test under
`test/test_button_timing/`. Does not extract any other logic (brightness
wraparound, digit-from-time computation, debounce timing itself), change button
behavior/thresholds, assign `read_button_2()`'s unused `medium_2s` case, or
introduce a `lib/` subfolder layout — each is out of scope for this task.

**Motivation:** No way currently exists to run any unit test without a board
attached; CLAUDE.md's testing convention calls for `env:native` but it was never
added. Flagged as a known gap during the `/init-planning` session rather than
invented independently.

---

## TASK-002 — Reassign button functions: button 1 = brightness, button 2 = display mode

**Category:** Code Cleanup
**Status:** In Progress
**Created:** 2026-10-08
**Related requirements/constraints:** FR-001 (mode switching via button press — wording
is button-agnostic, no change needed), FR-003 (brightness cycling via push-button —
wording is button-agnostic, no change needed). UC-001 and UC-002 explicitly document
the *current* button-to-function mapping and will be updated as part of this task to
match the new mapping.
**Plan:** `plans/task-button-function-reassignment-plan.md`

**Scope:** Rewire the button dispatch in `src/main.cpp`'s `loop()` so button 1's quick
press still drives brightness (unchanged) and button 2's quick press drives the
display-mode toggle (moved off button 1's medium ~2s press). Button 1's and button
2's medium (~2s) and long (~5s) presses become explicit no-ops. Update UC-001 and
UC-002 in `docs/requirements/use-cases.md` to describe the new mapping. Does not
touch `button_timing.cc/h`, press-duration thresholds, debounce timing, the ISR
handlers, `read_button_1()`/`read_button_2()`, or assign any behavior to the 2s/5s
presses themselves (reserved for a future task/feature) — each is out of scope.

**Motivation:** Today both buttons' quick presses do the same thing (brightness), and
mode-switching is buried behind a 2-second hold on button 1 — a less discoverable,
less evenly-distributed mapping. Reassigning gives each button a distinct quick-press
role and frees up both buttons' medium/long presses for future use, per user request.

---

## TASK-003 — Separate hardware-touching code from logic-only code for three more functions, extending TASK-001's pattern

**Category:** Code Cleanup
**Status:** Plan Ready
**Created:** 2026-10-09
**Related requirements/constraints:** None found — stems from CLAUDE.md's hardware-
adapter-layer convention, not a tracked FR/NFR/IC. The logic being extracted
underpins FR-001 (display mode), FR-003 (brightness), and FR-002/UC-002 (time
display) without changing any of their behavior.
**Plan:** `plans/task-hardware-logic-separation-plan.md`

**Scope:** Extract three more pieces of logic that TASK-001 explicitly deferred,
decoupling each from `Arduino.h` and (for the time-digit case) from RTClib's
`DateTime` type, and add a native Unity test for each: (1) the brightness-index
wraparound in `main.cpp`'s `input_switch_quick_press()`, (2) `main.cpp`'s already-pure
`toggle_display_mode()`, relocated into its own translation unit, and (3) the
digit-from-time/date decomposition in `RTC.cc`'s `update_display_with_time()` /
`update_display_with_date()`, rewritten to take plain integers instead of `DateTime`.
Does not touch debounce timing, the ISR handlers, `read_button_1()`/`read_button_2()`,
or remove `update_display_with_date()` despite it currently being unused — each is
out of scope.

**Motivation:** Sets the stage for more complete unit-test coverage by continuing the
hardware/logic separation TASK-001 started and explicitly deferred for these three
pieces, per user request.

---

<!-- Add new tasks via /plan-task, or by hand — keep the section format: metadata
     lines, then Scope / Motivation. Unlike a bug, a task doesn't need a symptom or
     reproduction — it needs a clear boundary of what's included and what isn't,
     since "remove all the X" quietly becoming "remove most of the X" is the
     characteristic failure mode here. -->
