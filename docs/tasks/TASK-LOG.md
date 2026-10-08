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
**Status:** Plan Ready
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

<!-- Add new tasks via /plan-task, or by hand — keep the section format: metadata
     lines, then Scope / Motivation. Unlike a bug, a task doesn't need a symptom or
     reproduction — it needs a clear boundary of what's included and what isn't,
     since "remove all the X" quietly becoming "remove most of the X" is the
     characteristic failure mode here. -->
