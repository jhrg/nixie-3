<!-- CONTRACT: actor-driven flows through the system. Each use case is what ties
     functional requirements together into something a real user actually does.
     Delete the _EXAMPLE_ use case once you've seen the shape. -->

# Use Cases

Convention: `UC-###`. Each use case follows the template below. Link related `FR-###`
/ `NFR-###` IDs at the bottom so `/trace` can find them in both directions.

---

## UC-001 — Adjust display brightness

**Actor(s):** Day-to-day user of the clock

**Trigger:** Quick (momentary) press of button 1

**Preconditions:** Clock is powered on and in Running mode (any display mode). Quick
press of button 1 has a different effect in Set Time or Set Date mode — see UC-003 /
UC-004.

**Main flow:**
1. User gives a quick press on button 1
2. System advances to the next brightness level in the preset sequence
   (`brightness_count`), wrapping back to the brightest level after the dimmest
3. System applies the new PWM level to the HV brightness control pin immediately

**Alternate / exception flows:**
- Every quick press of button 1 advances brightness regardless of current display
  mode (HH:MM/MM:SS/Date), as long as the clock is in Running mode
- While the clock is in Set Time or Set Date mode (UC-003/UC-004), a quick press of
  button 1 increments the field being edited instead of advancing brightness

**Postconditions:** Display brightness is at the newly selected preset level until
the next quick press

**Related requirements:** FR-003

---

## UC-002 — Cycle the display mode (HH:MM, MM:SS, date)

**Actor(s):** Day-to-day user of the clock

**Trigger:** Quick (momentary) press of button 2

**Preconditions:** Clock is powered on and in Running mode

**Main flow:**
1. User gives a quick press on button 2
2. System advances the display mode one step through the cycle
   `hh_mm` → `mm_ss` → date → `hh_mm`
3. System renders the four Nixie tubes for the newly selected mode on the next
   update tick. In date mode the system repeatedly shows the month and day as
   MM:DD (leading zeros, colon solid) for 2 s, then the year as 20YY (colon off) (FR-004)
   for 1 s, starting with MM:DD each time date mode is entered (FR-007)

**Alternate / exception flows:**
- On power-up the clock starts in `hh_mm` mode
- Long (~5s) presses of either button are currently no-ops, reserved for a future
  task/feature
- Button 1's medium (~2s) press is currently a no-op in Running mode, reserved for a
  future task/feature
- Button 2's medium (~2s) press enters Set Time mode instead of advancing the
  display mode, from any of the three display modes — see UC-003
- A quick press of button 2 while the clock is in Set Time or Set Date mode does not
  advance the display mode; instead it decrements the field currently selected for
  editing — see UC-003 / UC-004

**Postconditions:** Clock continues displaying in the newly selected mode until
the next quick press of button 2

**Related requirements:** FR-001, FR-007

---

## UC-003 — Set the clock's time (hour and minute)

**Actor(s):** Day-to-day user of the clock

**Trigger:** 2-second press of button 2 while the clock is in Running mode

**Preconditions:** Clock is powered on and in Running mode (any display mode)

**Main flow:**
1. User gives a 2-second press on button 2.
2. System enters Set Time mode with the hour field selected.
3. System displays the hour digit pair and the minute digit pair (regardless of
   which display mode — MM:SS or HH:MM — was active before entering Set Time mode).
   The currently selected field's digit pair blinks at ~1 Hz; the other pair stays
   lit continuously (NFR-001).
4. User gives a quick press on button 1 to increment the selected field's value by
   one, or a quick press on button 2 to decrement it by one, wrapping either way
   (hour: 0–23, minute: 0–59). System writes the updated hour/minute to the RTC
   immediately after either press, resetting seconds to `:00`.
5. User gives a 2-second press on button 1 to move the selected field to the other
   field (hour ↔ minute).
6. User repeats steps 4–5 as needed to set both fields.
7. User gives a 2-second press on button 2 to leave Set Time mode and enter Set Date
   mode (UC-004).

**Alternate / exception flows:**
- Decrementing below a field's minimum wraps to its maximum, mirroring the
  increment wraparound direction (hour: 0 → 23, minute: 0 → 59).
- A 2-second press of button 1 only changes which field is selected; it does not
  change the field's value.
- Long (~5s) presses of either button remain no-ops in Set Time mode, reserved for a
  future task/feature.
- Brightness cannot be adjusted while in Set Time mode — button 1's quick press is
  repurposed to increment the selected field (see UC-001's updated preconditions).
- Display mode cannot be toggled while in Set Time mode — button 2's quick press is
  repurposed to decrement the selected field (see UC-002's updated alternate flows).

**Postconditions:** The RTC reflects the newly set hour/minute with seconds at
`:00`. The clock is now in Set Date mode (UC-004), displaying the date for editing.

**Related requirements:** FR-005, NFR-001

---

## UC-004 — Set the clock's date (month, day, and year)

**Actor(s):** Day-to-day user of the clock

**Trigger:** 2-second press of button 2 while the clock is in Set Time mode (reached
via UC-003)

**Preconditions:** Clock is in Set Time mode (UC-003)

**Main flow:**
1. User gives a 2-second press on button 2 while in Set Time mode.
2. System enters Set Date mode with the month field selected.
3. System displays the month digit pair and the day digit pair. The currently
   selected field's digit pair blinks at ~1 Hz; the other pair stays lit
   continuously (NFR-001).
4. User gives a quick press on button 1 to increment the selected field's value by
   one, or a quick press on button 2 to decrement it by one, wrapping either way
   (month: 1–12, day: 1–31, year: last two digits, 00–99). System writes the updated
   month/day/year to the RTC immediately after either press.
5. User gives a 2-second press on button 1 to advance the selected field (month →
   day → year → month).
6. When the year field is selected, the system temporarily displays the full
   four-digit year — a fixed "20" century in the pair normally used for month, and
   the editable two-digit year in the pair normally used for day — both pairs
   blinking together at ~1 Hz; when selection moves off the year field, the display returns to showing
   month:day.
7. User repeats steps 4–6 as needed to set all three fields.
8. User gives a 2-second press on button 2 to leave Set Date mode and return to
   Running mode, resuming whichever display mode was active before
   Set Time mode was entered.

**Alternate / exception flows:**
- Decrementing below a field's minimum wraps to its maximum, mirroring the
  increment wraparound direction (month: 1 → 12, day: 1 → 31, year: 00 → 99).
- A 2-second press of button 1 only changes which field is selected; it does not
  change the field's value.
- Long (~5s) presses of either button remain no-ops in Set Date mode, reserved for a
  future task/feature.
- Brightness cannot be adjusted while in Set Date mode — button 1's quick press is
  repurposed to increment the selected field (see UC-001's updated preconditions).
- Display mode cannot be toggled while in Set Date mode — button 2's quick press is
  repurposed to decrement the selected field (see UC-002's updated alternate flows).
- No calendar validation is performed (e.g. day 30 is accepted for February),
  consistent with the existing digit-decomposition logic, which has never validated
  calendar correctness either.

**Postconditions:** The RTC reflects the newly set month/day/year. The clock returns
to Running mode, displaying time in its previously selected display mode.

**Related requirements:** FR-006, NFR-001

---

<!-- Copy the template above for each new use case. Sequential UC-### heading,
     same subsection order, so /trace can parse it. -->
