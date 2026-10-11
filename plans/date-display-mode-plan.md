# Plan: Date display mode (third Button-2 display mode)

**Status:** Approved
**Created:** 2026-10-11

## Summary

Add a third Running-mode display mode, reached by Button 2's quick press, that
repeatedly shows the date as MM:DD (colon solid) for 2 s and then the four-digit
year 20YY (colon off) for 1 s. The Button-2 cycle becomes `hh_mm` → `mm_ss` → date
→ `hh_mm`, and the clock now boots in `hh_mm`.

## Requirements traced

- FR-007 — the date display mode itself: MM:DD with leading zeros and a solid colon
  for 2 s, then 20YY with the colon off for 1 s, repeating. Status `Approved`.
- FR-001 — display switchable between HH:MM, MM:SS, and Date modes by a button
  press. The two time modes are unchanged; this plan adds Date mode to the cycle.
- FR-004 — the colon flashes once a second *except in date mode (FR-007)*. The
  exception is new and not implemented yet; this plan implements it.
- UC-002 — the three-way cycle, booting in `hh_mm`, MM:DD shown first each time
  date mode is entered, and Button 2's medium press entering Set Time from any of
  the three modes.
- UC-001 / FR-003 — brightness works in any display mode, date mode included. No
  code change is needed (button 1's quick-press path doesn't check display mode);
  the plan only verifies it, plus making the solid colon follow the current
  brightness.
- UC-003 — its precondition is now "any display mode". Already true in code:
  `next_operating_mode()` ignores display mode. The plan only verifies it.
- BUG-004 — stale digits on the Set Time → Set Date and Set Date → Running
  transitions. Phase 2, step 2 fixes it. The fix is specified in
  `plans/bugfix-stale-digits-on-mode-change-plan.md` and is delivered here because
  date mode needs the same mechanism.
- UC-004 — steps 3 and 8: Set Date shows month:day from the first redraw, and
  leaving Set Date resumes the previous display mode. Phase 2's BUG-004 fix covers
  both. When that mode is date mode, Phase 3, step 3 restarts it at MM:DD.
- NFR-001 — the Set-mode blink at ~1 Hz. No change is intended, but Phases 2 and 3
  modify `time_update_handler()`, which runs on the same 2 Hz tick path. Phase 4
  checks for regressions.

Related work (not satisfied by this plan): TASK-004, which shares the century
constants (Phase 3, step 6).

## Constraints considered

`docs/constraints/implementation-constraints.md` has no `IC-###` entries. It says
the Arduino/C++ conventions in `CLAUDE.md` are the binding constraints, so here is
how the plan meets each one that applies:

- **Logic separate from hardware:** the new cycle logic (`next_display_mode()`, the
  date-cycle tick helpers) goes in `src/display_mode.cc`, which is
  Arduino-independent and already builds under `env:native`. Separator pin writes
  stay in `src/RTC.cc`, and rendering stays in `src/main.cpp`.
- **No blocking `delay()`:** the 2 s / 1 s cycle counts the existing 2 Hz RTC tick
  (`timer_2HZ_tick_ISR`) in `loop()`. Nothing blocks or busy-waits.
- **Fixed-width types / no heap:** the tick counter is a `uint8_t` held in a
  `static`. No allocation and no `String`.
- **Unit tests for new logic:** the new pure functions get native Unity tests in
  `test/test_display_mode/`.
- **`pio check`, clang-format, one-line doc comments on public functions:** applied
  to every touched file.
- **ISR minimalism / `volatile`:** no new ISR work and no new state shared with an
  ISR. The counter only lives in `loop()`.

No ADRs exist apart from the placeholder, so the plan contradicts none.

## Phases

### Phase 1 — Three-way display-mode cycle (logic layer)

**Goal:** a pure, tested cycle function and date-cycle phase helpers.
**Satisfies:** UC-002, FR-007, FR-001

Steps:
1. `include/mode_switch2.h`: add a third value to `enum display_mode`
   (`date_display`), and replace `toggle_display_mode()` with
   `next_display_mode()`, documented as "Advance the display mode: hh_mm → mm_ss →
   date_display → hh_mm". The rename is deliberate: "toggle" is no longer accurate.
2. `src/display_mode.cc`: implement `next_display_mode()`.
3. `src/display_mode.cc` and its header: add two pure helpers for the date cycle,
   counted in 2 Hz half-second ticks:
   - `uint8_t next_date_cycle_tick(uint8_t tick)` returns `(tick + 1) % 6`.
   - `bool date_cycle_shows_year(uint8_t tick)` returns true for ticks 4–5 (1 s of
     year) and false for ticks 0–3 (2 s of MM:DD).
   Name the cycle lengths as `constexpr`s, e.g. `DATE_CYCLE_MONTH_DAY_TICKS = 4`
   and `DATE_CYCLE_TOTAL_TICKS = 6`.
4. `test/test_display_mode/test_display_mode.cpp`: replace the three toggle tests
   with tests for:
   - each of the three `next_display_mode()` transitions;
   - three applications returning to the start;
   - `date_cycle_shows_year()` for ticks 0–5;
   - `next_date_cycle_tick()` wrapping 5 → 0.

**Risks:** low. `toggle_display_mode()` has a single caller (`main.cpp:129`).
`mode_switch2.cc:50` has a `#if 0`'d copy of the display-mode variable, which is
dead code and left alone.

### Phase 2 — Render date digits in Running mode, and fix stale digits on a mode change

**Goal:** `time_update_handler()` supplies date digits while the date display mode
is active, and never draws digits computed for the other layout.
**Satisfies:** FR-007, UC-002, UC-004, BUG-004

Steps:
1. `include/RTC.h` and `src/RTC.cc`: give `time_update_handler()` a second
   parameter, `enum display_mode`. It renders the date when
   `mode == set_date || (mode == running && display == date_display)`, and the time
   otherwise. Update the doc comment to match.
2. In `time_update_handler()`, keep a `static bool` recording whether the last
   digits computed were date or time. When the requested kind differs, re-read the
   RTC and recompute the digits **on that half-tick**, instead of waiting up to
   0.5 s for the next `refetch_time` tick. The alternative is that switching into
   date mode briefly shows the time's digits laid out as MM:DD, and switching out
   briefly shows the date's digits as HH:MM.
   - This also fixes **BUG-004**, the same stale-digit window on Set Time → Set
     Date and Set Date → Running. Following
     `plans/bugfix-stale-digits-on-mode-change-plan.md`, put the decision in a pure
     `needs_refetch(refetch_tick, want_date, have_date)` function in an
     Arduino-free file. Add a native regression test covering the stale case, the
     steady state, and the refetch tick.
3. Update the caller in `main.cpp` to pass `the_display_mode`.

**Risks:** one extra I2C read (~1 ms, per the comment at `RTC.cc:251`) per mode
change. That's negligible.

### Phase 3 — Date rendering, colon control, and boot default (`main.cpp` / `RTC.cc`)

**Goal:** the visible behavior that FR-007 and UC-002 specify.
**Satisfies:** FR-007, FR-004, UC-002, UC-001, FR-003, UC-004

Steps:
1. `src/RTC.cc` and `include/RTC.h`: add `void set_separator(bool on)`. "On" uses
   `analogWrite(SEPARATOR, brightness_count[brightness])`, matching
   `toggle_separator()`, so the solid colon follows brightness (UC-001). "Off" uses
   `digitalWrite(SEPARATOR, LOW)`. In `time_update_handler()`, skip
   `toggle_separator()` while `mode == running && display == date_display` (FR-004's
   exception).
2. `src/main.cpp`: change the boot default at `main.cpp:99` from `mm_ss` to `hh_mm`
   (UC-002 alternate flow).
3. `src/main.cpp`: add `static uint8_t the_date_cycle_tick`. Reset it to 0 whenever
   date mode is entered: on a Button-2 quick press that lands on `date_display`, and
   on returning to `running` from Set Date with `the_display_mode == date_display`.
   Resetting on that second path follows UC-002 step 3, "starting with MM:DD each
   time date mode is entered". Confirmed with the user on 2026-10-11.
4. `src/main.cpp`: in the Button-2 quick-press handler, call `next_display_mode()`
   and update the `DPRINTV` at `main.cpp:130` to print all three modes.
5. `src/main.cpp`, rendering `switch`, `running` case: add a `date_display` arm:
   - If `!date_cycle_shows_year(tick)`, show MM:DD:
     `bits[0] = MSD[digit_5] | LSD[digit_4]` and
     `bits[1] = MSD[digit_3] | LSD[digit_2]`. Leading zeros come for free, since
     `MSD[0]`/`LSD[0]` drive a `0`. Then call `set_separator(true)`.
   - Otherwise show 20YY: `bits[0] = MSD[YEAR_CENTURY_TENS_DIGIT] |
     LSD[YEAR_CENTURY_ONES_DIGIT]` and `bits[1] = MSD[digit_1] | LSD[digit_0]`.
     Then call `set_separator(false)`.
   - Then `the_date_cycle_tick = next_date_cycle_tick(the_date_cycle_tick)`.
   Rendering happens once per 2 Hz tick, so the cycle works out to 4 × 0.5 s = 2 s
   of MM:DD and 2 × 0.5 s = 1 s of year.
6. **The century constants are shared with TASK-004.** `YEAR_CENTURY_TENS_DIGIT` /
   `YEAR_CENTURY_ONES_DIGIT` are specified in
   `plans/task-set-date-year-four-digit-display-plan.md` (Plan Ready, not yet
   implemented) for `include/clock_mode.h`. Whichever lands first adds them, and the
   other reuses them. Don't define them twice.

**Risks:**
- The first MM:DD period starts at the first render after the press, which comes
  ≤ 0.5 s later. The full 2 s is measured from that render, matching how the
  existing mode toggles appear.
- When leaving date mode, `toggle_separator()`'s internal `tick_tok` phase picks up
  wherever it left off. That's cosmetic: the colon flash may invert its phase
  relative to the seconds.

### Phase 4 — Verification

**Satisfies:** FR-007, FR-004, FR-001, FR-003, UC-001, UC-002, UC-003, UC-004,
NFR-001, BUG-004

1. `pio test -e native`: all suites pass, including the Phase 1 tests and the
   Phase 2 `needs_refetch()` regression test for BUG-004.
2. `pio run -e uno` and `pio run -e pro16MHzatmega328` build clean. `pio check`
   reports no new findings: until BUG-002 (Fix Planned) lands, the run still shows
   BUG-002's two known `preprocessorErrorDirective` false positives from
   `PinChangeInterrupt.h` and exits `FAILED`, and nothing else is acceptable.
   clang-format has been applied to the touched files.
3. On-target checks:
   - **Boot:** the clock comes up in HH:MM after the random-digit flash.
   - **Cycle:** quick presses of Button 2 go HH:MM → MM:SS → date → HH:MM.
   - **Date mode timing:** shows e.g. `10:11` (colon solid) for ~2 s, then `2026`
     (colon off) for ~1 s, repeating. Time it with a stopwatch over several cycles.
   - **Leading zeros:** confirmed on a single-digit month or day. Set the date via
     UC-004 if needed.
   - **No stale digits (date mode):** time digits never flash on entering date
     mode, and date digits never flash on leaving it.
   - **No stale digits (BUG-004):** re-run BUG-004's reproduction steps at least 10
     times. Entering Set Date from Set Time shows month:day from the first redraw,
     and returning to Running shows the resumed mode's digits from the first redraw.
   - **Set-mode blink unchanged (NFR-001):** in Set Time and Set Date, the selected
     pair blinks at ~1 Hz and the other pair stays lit. In Set Date, check each of
     month, day and year.
   - **Brightness:** Button 1's quick press changes brightness in date mode,
     including the solid colon (UC-001).
   - **Set Time from date mode:** Button 2's medium press enters Set Time (UC-003).
     After going through Set Date, the clock returns to date mode starting on MM:DD.
   - **Midnight:** if practical, set the time to 23:59:50 with date mode showing and
     confirm the date advances.

## Open questions

None remaining. Resolved on 2026-10-11:
- FR-007 is `Approved`.
- Returning from Set Date into date mode restarts at MM:DD (Phase 3, step 3).
- UC-001 and UC-004 step 8 wording was updated by the user and now covers three
  display modes.
- The Set Date stale-digit issue is logged as BUG-004 and fixed in Phase 2.
- TASK-004 ordering: whichever lands first adds the century constants, and the
  other reuses them and rebases.

## Out of scope

- Years outside 2000–2099. The century is fixed at "20", matching
  `adjust_year_value()` and `digits_from_date()`.
- Making the 2 s / 1 s durations configurable.
- Any change to Set Time / Set Date rendering, including TASK-004's four-digit year
  in Set Date.
- Removing the dead `#if 0` display-mode variable in `mode_switch2.cc`.
