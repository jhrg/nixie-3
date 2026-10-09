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

**Preconditions:** Clock is powered on and running (any display mode)

**Main flow:**
1. User gives a quick press on button 1
2. System advances to the next brightness level in the preset sequence
   (`brightness_count`), wrapping back to the brightest level after the dimmest
3. System applies the new PWM level to the HV brightness control pin immediately

**Alternate / exception flows:**
- None currently handled — every quick press of button 1 advances brightness
  regardless of current display mode

**Postconditions:** Display brightness is at the newly selected preset level until
the next quick press

**Related requirements:** FR-003

---

## UC-002 — Switch between MM:SS and HH:MM display mode

**Actor(s):** Day-to-day user of the clock

**Trigger:** Quick (momentary) press of button 2

**Preconditions:** Clock is powered on and running

**Main flow:**
1. User gives a quick press on button 2
2. System toggles the display mode between `mm_ss` and `hh_mm`
3. System renders the four Nixie tubes using the digits for the newly selected mode
   on the next time-update tick

**Alternate / exception flows:**
- None currently handled — medium (~2s) and long (~5s) presses of either button are
  currently no-ops, reserved for a future task/feature

**Postconditions:** Clock continues displaying time in the newly selected mode until
the next quick press of button 2

**Related requirements:** FR-001

---

<!-- Copy the template above for each new use case. Sequential UC-### heading,
     same subsection order, so /trace can parse it. -->
