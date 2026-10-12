# Bugfix Plan: Stale digits shown for up to ~0.5 s on Set Time → Set Date and Set Date → Running transitions

**Bug:** BUG-004
**Status:** Done
**Created:** 2026-10-11

## Summary

After an operating-mode change, the first redraw can show digits that were computed
for the previous mode (time vs. date), laid out for the new mode, for up to ~0.5 s.
The fix: `time_update_handler()` re-reads the RTC immediately whenever the kind of
digits it's asked for differs from the kind it last computed.

**Implementation vehicle:** this fix is delivered as Phase 2, step 2 of
`plans/date-display-mode-plan.md`, not as a separate change. The new date display
mode needs the same mechanism. This plan exists so the fix traces to BUG-004 and
gets its own regression checks.

## Root cause

See BUG-004 in `docs/bugs/BUG-LOG.md`. In short:
- `time_update_handler()` (`src/RTC.cc:238-264`) signals "render" on every 2 Hz
  tick.
- It only recomputes `digit_0..5` on the 1 Hz `refetch_time` tick, and the kind it
  computes depends on the mode at that moment.
- A mode change between refetches leaves the old kind of digits in place, and
  `main.cpp` lays them out for the new mode.

This came in with the BUG-003 fix, which split redraw (2 Hz) from refetch (1 Hz).
It's established by reading the code and hasn't been seen on hardware.

## Related requirements

- UC-004 step 3: Set Date displays the month and day digit pairs. It shows
  hour:minute instead for up to ~0.5 s after entering.
- UC-004 step 8: leaving Set Date resumes the previous display mode. It shows date
  digits as time for up to ~0.5 s after leaving.

## Fix approach

In `src/RTC.cc`'s `time_update_handler()`:
1. Work out the requested digit kind, date or time, from the arguments. Today that
   is `mode == set_date`. Once the date display mode plan lands, it also includes
   `mode == running && display == date_display`.
2. Keep a `static bool` holding the kind last computed into `digit_0..5`.
3. On an `update_display` tick, refetch if `refetch_time` is set **or** the
   requested kind differs from the last-computed kind. Clear `refetch_time` only if
   it was set, so the regular 1 Hz cadence isn't disturbed. Record the kind just
   computed.
4. Put the step 3 decision in a pure function,
   `bool needs_refetch(bool refetch_tick, bool want_date, bool have_date)`, in an
   Arduino-free file built under `env:native`. Add a native Unity test covering:
   - the stale case (no refetch tick, kind changed) returns true;
   - the steady state (no refetch tick, same kind) returns false;
   - the refetch tick always returns true.
   This is the regression test CLAUDE.md requires: the stale-case assertion fails
   against today's logic and passes after the fix.

Why this scope:
- It fixes the cause, digits with no record of which kind they are, at the one
  function that computes them, so every caller and every current or future mode
  transition is covered.
- The narrower option is forcing a refetch from `main.cpp` at each mode-change site.
  That's more call sites, and a new mode (such as the date display mode) is easy to
  forget.
- The broader option is reverting to a refetch on every redraw. That undoes BUG-003's
  deliberate 1 Hz cap on I2C reads for no gain.

## Regression risk

- **Cost:** one extra `rtc.now()` I2C read (~1 ms, per `RTC.cc:251`) per digit-kind
  change. Mode changes are rare, so this is negligible.
- **Separator flash and NFR-001 blink:** both are untouched. `toggle_separator()`
  and `blink_phase` don't depend on the refetch path.
- **Per-second refresh in Running and Set modes:** the 1 Hz cadence is unchanged,
  because `refetch_time` is still honored and cleared as before.
- **Tests:** `RTC.cc` includes `Arduino.h` and RTClib, so it can't be built under
  `env:native` and has no native test coverage today. Fix approach step 4 moves the
  decision into a natively tested pure function. The rest of `time_update_handler()`
  (the RTC read and the digit recompute) is still covered only by on-target checks.

## Verification

- **Native:** `pio test -e native` with the new `needs_refetch()` test. Its
  stale-case assertion would fail against today's logic, which only refetches on a
  refetch tick.
- **Build and static checks:** `pio run -e uno` and `pio run -e pro16MHzatmega328`
  build clean. `pio check` reports no new findings: until BUG-002 (Fix Planned)
  lands, the run still shows BUG-002's two known `preprocessorErrorDirective` false
  positives from `PinChangeInterrupt.h` and exits `FAILED`, and nothing else is
  acceptable. clang-format applied to touched files.
- **On target:** re-run BUG-004's reproduction steps at least 10 times. Neither
  transition should ever show the previous mode's digits; the first redraw shows the
  right content. Once the date display mode lands, repeat this for HH:MM/MM:SS ↔
  date-mode transitions too.

## Out of scope

- The date display mode itself (FR-007), which is covered by
  `plans/date-display-mode-plan.md`.
- The up-to-0.5 s delay between a button release and the first redraw. That's
  existing behavior for all mode changes and isn't part of this bug. The bug is
  *wrong* content, not *late* content.
- Any change to the 2 Hz ISR or the blink mechanism. The fix keeps BUG-003's
  redraw/refetch split and its 1 Hz refetch cadence. It only adds an immediate
  refetch when the requested digit kind changes.
