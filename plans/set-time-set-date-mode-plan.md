# Plan: Set Time / Set Date mode

**Status:** Draft
**Created:** 2026-10-10

## Summary

Add a Set Time mode and a Set Date mode to the clock, reached and operated entirely
through button 1's and button 2's existing quick-press and 2-second-press
signatures (no new hardware, no change to the 5-second press — that stays reserved
for a future task/feature per explicit instruction). Button 2's 2-second press
cycles the clock through Running → Set Time → Set Date → Running; within a set
mode, button 1's quick press increments the currently selected field and button
1's 2-second press moves selection to the next field. Edits write to the RTC
immediately, one field at a time.

## Requirements traced

- FR-005 — set the clock's time (hour/minute) via button presses
- FR-006 — set the clock's date (month/day/year) via button presses
- NFR-001 — the field selected for editing must be visually distinguishable
  (blinks at ~1 Hz) from the field(s) not selected
- UC-003 — set the clock's time
- UC-004 — set the clock's date
- FR-001, FR-003, UC-001, UC-002 are **not changed in behavior** while the clock is
  in Running mode, but UC-001/UC-002 were updated during this planning session to
  document the new contextual meaning of button 1's quick press and button 2's
  quick/2s press while a set mode is active — see the edits already made to
  `docs/requirements/use-cases.md`.

## Constraints considered

`docs/constraints/implementation-constraints.md` has no `IC-###` rows yet (confirmed
by reading it in full) — only the Arduino/C++ conventions baseline in `CLAUDE.md`
applies. This plan follows that baseline directly:

- Hardware/logic separation: all new field-selection, mode-cycling, and
  value-wraparound logic lives in a new header/source pair with no `Arduino.h`
  dependency (Phase 1), mirroring `brightness.h`/`button_timing.h`/
  `display_digits.h`. Only the RTC-commit step (Phase 2) and the button-dispatch/
  rendering wiring (Phase 3) touch `Arduino.h`/`RTClib`/hardware registers.
- Fixed-width integers (`uint8_t`) for all field values (hour, minute, month, day,
  year-offset), consistent with `next_brightness_index()`'s existing signature.
- No dynamic allocation, no blocking `delay()`, no new `String` usage.
- No new ISR-shared state: the new `operating_mode`/field-selection variables are
  read and written only from `loop()`, the same place `the_display_mode` already
  lives — none of them need to be `volatile`.
- Every new public function gets a one-line doc comment per CLAUDE.md's
  documentation convention.
- Every new logic-layer function gets a native Unity test under `test/`
  (`pio test -e native`), per CLAUDE.md's testing convention.

## Phases

### Phase 1 — Pure clock-mode logic

**Goal:** Represent the operating-mode state machine and per-field value
wraparound as plain, hardware-independent C++, unit-testable under `env:native`.
**Satisfies:** FR-005, FR-006 (foundation), NFR-001 (field-selection cycling)

Steps:
1. Add `include/clock_mode.h` / `src/clock_mode.cc` (no `Arduino.h`):
   - `enum operating_mode { running, set_time, set_date };`
   - `enum time_field { field_hour, field_minute };`
   - `enum date_field { field_month, field_day, field_year };`
   - `enum operating_mode next_operating_mode(enum operating_mode mode);` —
     `running → set_time → set_date → running`
   - `enum time_field next_time_field(enum time_field field);` — `hour ↔ minute`
   - `enum date_field next_date_field(enum date_field field);` —
     `month → day → year → month`
   - `uint8_t next_hour_value(uint8_t hour);` — wraps 0–23
   - `uint8_t next_minute_value(uint8_t minute);` — wraps 0–59
   - `uint8_t next_month_value(uint8_t month);` — wraps 1–12
   - `uint8_t next_day_value(uint8_t day);` — wraps 1–31 (no calendar/leap-year
     awareness — matches `digits_from_date()`'s existing lack of validation)
   - `uint8_t next_year_value(uint8_t year_last_two_digits);` — wraps 0–99
   - `constexpr uint8_t DIGIT_PAIR_BLANK = 0xFF;` — both nibbles `0xF`; confirmed
     hardware blanking value for a digit pair on this Nixie driver.
   - `uint8_t blank_if_selected(uint8_t digit_pair_bits, bool is_selected, bool
     blink_off);` — returns `DIGIT_PAIR_BLANK` when `is_selected && blink_off`,
     otherwise returns `digit_pair_bits` unchanged. Keeps the blink mechanics pure
     and testable, so Phase 3's rendering code is a thin call site rather than
     inline blanking logic.
2. Add `test/test_clock_mode/test_clock_mode.cpp` (Unity), covering: each mode/field
   cycle returns to its start after the expected number of steps, each value
   wraparound at its boundary (`hour` 23→0, `minute` 59→0, `month` 12→1, `day`
   31→1, `year` 99→0), and `blank_if_selected()`'s three cases (selected+blink_off →
   `0xFF`; selected+!blink_off → unchanged; !selected → unchanged regardless of
   `blink_off`).
3. Add `clock_mode.cc` to `[env:native]`'s `build_src_filter` in `platformio.ini`.

**Risks:** Low — purely additive, no existing code touched.

### Phase 2 — RTC adapter: field commit + date rendering + blink phase

**Goal:** Let the hardware-adapter layer (`RTC.cc`) apply a field edit to the RTC
and expose what the display rendering phase (Phase 3) needs: digits for the field
being edited, and a blink-phase signal.
**Satisfies:** FR-005, FR-006, NFR-001

Steps:
1. In `RTC.cc`, add `void commit_hour_or_minute(enum time_field field);` and
   `void commit_month_day_or_year(enum date_field field);`. Each reads the current
   `dt`, computes the next value via Phase 1's `next_*_value()`, calls
   `rtc.adjust(...)` with that one field changed (seconds forced to `0` for the
   time case, per the confirmed "reset to :00 on save" design decision), and
   updates the local `dt` from the result.
2. Wire up the already-defined-but-unused `update_display_with_date()`
   (`RTC.cc:50-59`) by having `time_update_handler()` take a parameter (e.g.
   `enum operating_mode mode`) and call `update_display_with_date()` instead of
   `update_display_with_time()` when `mode == set_date`. `set_time` and `running`
   both continue to use `update_display_with_time()` (set_time needs hour/minute/
   second digits the same way running does).
3. Add a blink-phase accessor, e.g. `bool blink_off_phase();`, exposing the
   existing 2 Hz `toggle` boolean already maintained by `timer_2HZ_tick_ISR()`, so
   `main.cpp` can decide whether to blank the selected digit pair on a given
   refresh.
4. Update `RTC.h`'s declarations to match (`commit_hour_or_minute`,
   `commit_month_day_or_year`, `blink_off_phase`, and `time_update_handler`'s new
   parameter). `RTC.h`/`RTC.cc` take on a dependency on `clock_mode.h` — acceptable
   since `RTC.cc` is already the hardware-adapter layer, not the pure-logic layer.

**Risks:**
- Changing `time_update_handler()`'s signature touches its one call site in
  `main.cpp`'s `loop()` — small, mechanical change, verify no other callers exist
  (`grep -rn "time_update_handler" src include`).
- `rtc.adjust()` requires a full `DateTime`; constructing one from the current `dt`
  with a single field changed needs care to not accidentally reset the other
  fields — reuse `dt.year()/month()/day()/hour()/minute()/second()` explicitly
  rather than assuming a partial-update constructor exists on `RTClib`'s
  `DateTime`.

### Phase 3 — `main.cpp` wiring: button dispatch + rendering

**Goal:** Make the button presses and display rendering in `loop()` context-aware
of the new `operating_mode`, implementing UC-003 and UC-004 end to end while
leaving Running-mode behavior (UC-001, UC-002) unchanged.
**Satisfies:** FR-005, FR-006, NFR-001, UC-003, UC-004

Steps:
1. Add `static enum operating_mode the_operating_mode = running;`,
   `static enum time_field the_time_field = field_hour;`, and
   `static enum date_field the_date_field = field_month;` as statics in `loop()`,
   alongside the existing `the_display_mode` static.
2. Button 1's `switch (read_button_1())`:
   - `quick`: if `the_operating_mode == running`, keep calling
     `input_switch_quick_press()` unchanged (FR-003/UC-001); otherwise call
     `commit_hour_or_minute(the_time_field)` or `commit_month_day_or_year(the_date_field)`
     depending on `the_operating_mode`.
   - `medium_2s`: no-op when `running` (unchanged); otherwise advance
     `the_time_field`/`the_date_field` via `next_time_field()`/`next_date_field()`.
   - `long_5s`: unchanged no-op in every mode (reserved).
3. Button 2's `switch (read_button_2())`:
   - `quick`: unchanged (`toggle_display_mode()`) when `running`; no-op in
     `set_time`/`set_date`.
   - `medium_2s`: `the_operating_mode = next_operating_mode(the_operating_mode)`;
     when the new mode is `set_time`, reset `the_time_field = field_hour`; when
     it's `set_date`, reset `the_date_field = field_month`.
   - `long_5s`: unchanged no-op in every mode (reserved).
4. Rendering block (currently `if (time_update_handler()) { switch
   (the_display_mode) ... }`): pass `the_operating_mode` into
   `time_update_handler()`. Add `set_time` and `set_date` branches alongside the
   existing `running` (`the_display_mode`-driven) branch:
   - `set_time`: `bits[0]` = `blank_if_selected(MSD[digit_5]|LSD[digit_4],
     the_time_field == field_hour, blink_off_phase())`; `bits[1]` =
     `blank_if_selected(MSD[digit_3]|LSD[digit_2], the_time_field == field_minute,
     blink_off_phase())`.
   - `set_date`: `bits[1]` = `blank_if_selected(MSD[digit_3]|LSD[digit_2],
     the_date_field == field_day, blink_off_phase())` always; the raw pair
     feeding `bits[0]` is `MSD[digit_5]|LSD[digit_4]` (month) unless
     `the_date_field == field_year`, in which case it's `MSD[digit_1]|LSD[digit_0]`
     (year) — either way, `bits[0]` = `blank_if_selected(<that raw pair>,
     the_date_field == field_month || the_date_field == field_year,
     blink_off_phase())`.

**Risks:**
- This is the phase most likely to introduce a regression in the unchanged
  Running-mode paths (UC-001/UC-002) since it edits the same `switch` statements
  those paths rely on — needs a careful before/after diff, not just a "it still
  compiles" check.

### Phase 4 — Verification

**Goal:** Confirm the new modes work as specified without regressing existing
behavior.
**Satisfies:** FR-005, FR-006, NFR-001, UC-003, UC-004 (verification), plus
regression coverage for FR-001, FR-003, UC-001, UC-002

Steps:
1. `pio test -e native` — all suites pass, including the new
   `test_clock_mode` suite.
2. `pio run -e uno` and `pio run -e pro16MHzatmega328` — both build clean.
3. On-target check (`pio test -e <board>` or manual hardware test): confirm
   `DIGIT_PAIR_BLANK` (`0xFF`) actually blanks the tube pair on real hardware, and
   walk through UC-003 and UC-004's main flows end to end, confirming the RTC
   retains the newly set time/date across a power cycle (DS3231/DS1307
   battery-backed, per FR-002).
4. Manually re-walk UC-001 and UC-002's flows on-target to confirm brightness
   cycling and display-mode toggling are unchanged in Running mode.
5. `pio check` and `clang-format` over every touched/added file.

## Open questions

- None blocking. (The blink-blanking value was the one open question at draft time;
  resolved as `0xFF` — see `DIGIT_PAIR_BLANK` in Phase 1.)

## Out of scope

- Adding a timeout that auto-returns the clock to Running mode if left idle
  mid-edit in Set Time/Set Date mode. Not requested; would need a new timer and
  adds a real design surface (what counts as "idle", does it discard or keep the
  last-saved field) that's better handled as its own follow-up if it turns out to
  matter in practice.
- Calendar-aware day validation (e.g. rejecting February 30) — matches the
  existing `digits_from_date()`/`digits_from_time()` behavior, which has never
  validated calendar correctness either.
- Any use of the 5-second press on either button — explicitly reserved for a
  future task/feature per the user's instruction.
- Making brightness adjustable while in Set Time/Set Date mode — button 1's quick
  press is fully repurposed to field-increment while a set mode is active.
- Year range beyond 2000–2099 (`next_year_value()` only wraps a two-digit
  offset) — matches the existing display's two-digit year limit
  (`digits_from_date()`'s `(year - 2000) / 10`).
