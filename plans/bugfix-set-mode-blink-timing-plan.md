# Bugfix Plan: Set Time/Set Date selected-field blink timing

**Bug:** BUG-003
**Status:** Fixed
**Created:** 2026-10-10

## Summary

The field selected for editing in Set Time/Set Date mode is supposed to blink at
~1 Hz (NFR-001) but instead renders almost permanently dark. Fix the ISR/
`time_update_handler()` phase relationship so the display is actually rewritten on
both halves of the blink cycle.

## Root cause

`timer_2HZ_tick_ISR()` (`src/RTC.cc`) runs at 2 Hz (once per RTC square-wave edge)
and toggles `blink_phase` on every call, but only set `update_display = true` on the
call where `blink_phase` had just flipped to `false`. `update_display` is the only
flag that gates the shift-register rewrite in `main.cpp`'s `loop()`, so the physical
display only ever got rewritten once a second — and always at the instant
`blink_off_phase()` (`!blink_phase`) was `true`. The selected field was therefore
always written as blanked; the display never caught the "on" half of the cycle.

## Related requirements

- NFR-001 — violated: the selected field must blink at ~1 Hz; instead it renders
  blanked on effectively every write.

## Fix approach

Decouple "the display needs to be rewritten with the current blink phase" (should
happen at 2 Hz, matching the ISR's own rate) from "the RTC needs to be re-read and
digits recomputed" (only needs to happen at 1 Hz, and does an I2C transaction that
shouldn't double in frequency).

- `timer_2HZ_tick_ISR()` sets `update_display = true` unconditionally on every call.
- A new `refetch_time` flag is set only on the once-per-second edge (where the old
  code set `update_display`).
- `time_update_handler()` still returns `true` whenever `update_display` fires (now
  2 Hz), but only calls `rtc.now()` / `update_display_with_date()` /
  `update_display_with_time()` when `refetch_time` is also set, clearing both flags
  appropriately.

This is a narrow, targeted fix confined to `src/RTC.cc` — the bug is purely in the
hardware-adapter layer's timing, not in Phase 1's pure `clock_mode.h`/`.cc` logic
(`blank_if_selected()` itself was already correct and has passing unit tests), so no
changes are needed there.

## Regression risk

- `time_update_handler()`'s one call site (`main.cpp`'s `loop()`) is unaffected in
  shape — still `if (time_update_handler(the_operating_mode)) { ... }` — just called
  twice as often. In Running mode this means the shift register is now written at
  2 Hz instead of 1 Hz with (for the unselected, non-blinking case) the same bits
  each time — harmless, no visible behavior change since `blank_if_selected()` isn't
  in the `running` branch.
- `toggle`/`toggle_separator()` timing is untouched (still gated by the pre-existing
  `toggle` flag, set every ISR call as before).
- No changes to `clock_mode.h`/`.cc` (Phase 1) — existing `test_clock_mode` suite
  still passes unmodified (verified: `pio test -e native`, 33/33 passing).

## Verification

- `pio test -e native` — all 33 cases pass (confirmed).
- `pio run -e uno` — builds clean (confirmed).
- `pio check -e uno` — no new findings versus the pre-fix baseline (confirmed; only
  pre-existing low-severity `unusedFunction`/style notes and the unrelated BUG-002
  PCINT false positive remain).
- On-target check (still needed, per the original feature plan's Phase 4): enter Set
  Time mode and visually confirm the selected field now blinks at ~1 Hz (on ~0.5s,
  off ~0.5s) rather than appearing dark.

## Out of scope

- BUG-002 (`pio check` PCINT false positive) — unrelated, already tracked
  separately.
- Any change to the blink *rate* itself (NFR-001 only specifies ~1 Hz) — this fix
  corrects the display from never-rendering-on to the originally-intended ~1 Hz
  cycle, it doesn't retune the rate.
