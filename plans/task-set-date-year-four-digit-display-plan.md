# Task Plan: Display the full four-digit year while editing the year field in Set Date mode

**Task:** TASK-004
**Status:** Done
**Created:** 2026-10-10

## Summary

While the year field is selected in Set Date mode, render the full four-digit year
("20 26") across both digit pairs instead of today's "year in the month pair's slot,
day left showing in the other pair" ("26 10").

## Motivation

The current combination reads as two unrelated two-digit values rather than one
four-digit year being edited — user-requested fix for clarity. `adjust_year_value()`
already only tracks a two-digit offset from a fixed century (2000–2099, an explicit
out-of-scope item in `plans/set-time-set-date-mode-plan.md`), so the century half of
the display ("20") is always a fixed literal, not a stored/edited value.

## Related requirements / constraints

- FR-006 — "set the clock's calendar date ... via button presses." Wording doesn't
  specify display mechanics; unaffected.
- NFR-001 — "the digit pair currently selected for editing must be visually
  distinguishable ... blinks at ~1 Hz." Status is `Implemented`. This task keeps the 
  ~1 Hz blink mechanism itself untouched; it only changes which raw digit pairs get blinked while the year field is selected (both pairs, together, instead of one).
- UC-004 — step 6 currently documents the exact behavior being replaced: "the
  system temporarily displays the year digit pair in place of the month digit pair
  (year blinking, day lit continuously)." Updating this step's wording is in scope
  for this task, the same way TASK-002 updated UC-001/UC-002 when it changed the
  behavior they documented.

## Scope

**In scope:**
- ~~`include/clock_mode.h`: add century-digit constants~~ — **already done**
  (2026-10-12 re-check): `YEAR_CENTURY_TENS_DIGIT` / `YEAR_CENTURY_ONES_DIGIT`
  were added to `include/clock_mode.h` by the FR-007 date-display work (commit
  2581e83) and are already used by the `date_display` rendering branch. This task
  reuses them; no header change is needed.
- `src/main.cpp`'s `loop()`, in the `set_date` case of the rendering `switch`:
  rewrite the `field_year` sub-case so:
  - `bits[0]` = `blank_if_selected(MSD[YEAR_CENTURY_TENS_DIGIT] |
    LSD[YEAR_CENTURY_ONES_DIGIT], true, blink_off_phase())`
  - `bits[1]` = `blank_if_selected(MSD[digit_1] | LSD[digit_0], true,
    blink_off_phase())`
  while `field_month` and `field_day` keep today's existing rendering (month pair in
  `bits[0]`, day pair in `bits[1]`, only the selected one of the two blinking).
- `docs/requirements/use-cases.md`: update UC-004 step 6 to describe the new
  rendering (full four-digit year, both pairs blinking together) instead of the old
  "year in place of month, day lit continuously" wording.

**Out of scope:**
- `field_month` / `field_day` rendering — unchanged (month pair / day pair, as
  today).
- `adjust_year_value()`, `digits_from_date()`, or any change to the year's
  underlying range (still a two-digit 0–99 offset from a fixed century 2000) —
  `digit_0`/`digit_1` are already computed correctly today; this task only changes
  which pairs are displayed and in which position while editing the year.
- `blank_if_selected()`, `blink_off_phase()`, or the blink rate/mechanism itself —
  reused as-is, just applied to a different pair of raw bits for the year case.
- `next_date_field()`'s field-cycling order (month → day → year → month) —
  unchanged.
- `clock_mode_h_`'s existing unit tests for `blank_if_selected()` / value
  wraparound — unaffected, no behavior change to the functions they cover.

## Approach

Steps:
1. (Already done by FR-007, commit 2581e83: `YEAR_CENTURY_TENS_DIGIT` /
   `YEAR_CENTURY_ONES_DIGIT` exist in `include/clock_mode.h`. Confirm they're still
   present; nothing to add.)
2. In `src/main.cpp`'s `set_date` rendering branch, split the `field_year` case out
   from the existing shared `month_or_year_bits` expression (which today feeds
   `bits[0]` for both `field_month` and `field_year`) into its own branch that sets
   both `bits[0]` and `bits[1]` as described above. Keep `field_month`/`field_day`
   using the existing shared logic for the other two cases.
3. Update UC-004 step 6 in `docs/requirements/use-cases.md`, e.g.: "When the year
   field is selected, the system temporarily displays the full four-digit year —
   a fixed '20' century in the pair normally used for month, and the editable
   two-digit year in the pair normally used for day — both pairs blinking together
   at ~1 Hz; when selection moves off the year field, the display returns to
   showing month:day."
4. Re-read the rest of UC-004 (main flow, alternate flows, postconditions) to
   confirm nothing else references the old per-pair description — only step 6 does.

Completeness check: `grep -n "month_or_year_bits\|field_year" src/main.cpp` before
and after to confirm the old shared-pair branch for `field_year` no longer exists
and the new explicit branch is the only place `field_year` is rendered.

## Verification

- `pio test -e native` — all existing suites (including `test_clock_mode`) still
  pass unmodified; this task adds no new logic-layer functions, so no new native
  test is required by CLAUDE.md's testing convention (`YEAR_CENTURY_*_DIGIT` are
  bare constants, not functions — consistent with `DIGIT_PAIR_BLANK` not having a
  dedicated test of its own).
- `pio run -e uno` and `pio run -e pro16MHzatmega328` — both build clean.
- `pio check` and `clang-format` over `src/main.cpp`.
- On-target check: enter Set Date mode, cycle to the year field, and visually
  confirm the display reads "20 26" (both pairs blinking together at ~1 Hz) rather
  than "26 10"; cycle to month and day and confirm their rendering is unchanged
  from before this task.

## Risks

- Low — confined to one rendering branch in `main.cpp` and one use-case
  paragraph; no change to field-cycling, value storage, RTC
  commit logic, or the blink primitives themselves.

## Open questions

- None blocking. (An earlier note here flagged NFR-001 as `Proposed`; as of
  2026-10-12 it is `Implemented`, so that note no longer applies.)
