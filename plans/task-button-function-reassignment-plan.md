# Task Plan: Reassign button functions: button 1 = brightness, button 2 = display mode

**Task:** TASK-002
**Status:** In Progress
**Created:** 2026-10-08

## Summary

Swap which button drives which action: button 1's quick press keeps controlling
brightness, button 2's quick press takes over display-mode toggling (currently that's
a ~2s hold on button 1). Both buttons' medium (~2s) and long (~5s) presses become
explicit no-ops, reserved for future use. UC-001 and UC-002 are updated to match.

## Motivation

Today both buttons' quick presses do the identical thing (advance brightness), and
mode-switching is hidden behind a 2-second hold on button 1 that a user has to
discover or be told about. Splitting the two actions across the two buttons' quick
presses gives each button a distinct, easily-discoverable role, and frees up the
medium/long press duration on *both* buttons for a future task or feature (per user
request — out of scope here).

## Related requirements / constraints

- FR-001 — "switchable between MM:SS and HH:MM modes via a button press." Wording
  doesn't name a specific button, so no FR text change is needed; the use case that
  specializes it (UC-002) does need updating.
- FR-003 — "brightness adjustment via a push-button that cycles through preset PWM
  brightness levels." Same situation: button-agnostic wording, no FR change needed.
- UC-001 (Adjust display brightness) — currently documents the trigger as "quick
  press of button 1 or button 2." Updated by this task to "button 1" only.
- UC-002 (Switch between MM:SS and HH:MM) — currently documents the trigger as "medium
  press (~2s) of button 1." Updated by this task to "quick press of button 2."
- No IC/ADR found that bears on this change.

## Scope

**In scope:**
- `src/main.cpp`, `loop()`:
  - Button 1 (`read_button_1()`) switch: `quick` keeps calling
    `input_switch_quick_press()` (unchanged behavior). `medium_2s` and `long_5s`
    become explicit no-op cases (currently `medium_2s` calls
    `input_switch_medium_press()`; `long_5s` already falls through `default`).
  - Button 2 (`read_button_2()`) switch: `quick` calls the display-mode toggle
    (currently `input_switch_medium_press()`, invoked from button 1's `medium_2s`
    case) instead of `input_switch_quick_press()`. `medium_2s` and `long_5s` become
    explicit no-op cases (`medium_2s` is already a no-op today but implicit; make it
    explicit alongside `long_5s` for symmetry).
  - Rename `input_switch_medium_press()` to a name that doesn't reference "medium"
    duration, since it's no longer tied to a medium-length press (e.g.
    `toggle_display_mode()`). Logic inside the function is unchanged.
  - Remove the stale `// Replace this with ...` comment above button 2's switch,
    since this task is exactly that replacement.
  - Add a short comment on each new no-op case noting it's reserved for a future
    task/feature, so the empty branch doesn't read as an oversight.
- `docs/requirements/use-cases.md`:
  - UC-001: change Trigger to "Quick (momentary) press of button 1." Update the main
    flow's "either push button" wording to "button 1." Update/remove the alternate
    flow note that currently says "every quick press advances brightness regardless
    of current display mode" if it no longer applies, or reword to reflect
    single-button triggering.
  - UC-002: change Trigger to "Quick (momentary) press of button 2." Update the main
    flow's "holds button 1 for a medium press duration" wording to "gives a quick
    press on button 2." Replace the alternate-flow note about button 2's medium press
    having no assigned behavior — that's now moot — with a note that both buttons'
    medium (~2s) and long (~5s) presses are currently no-ops, reserved for future use.
- `docs/tasks/TASK-LOG.md`: already updated to `Plan Ready` with this plan linked.

**Out of scope:**
- `include/button_timing.h` / `src/button_timing.cc` — press-duration classification,
  thresholds (`SWITCH_PRESS_2S`/`SWITCH_PRESS_5S`), and debounce timing are untouched.
  TASK-001 (native test env + extraction of this module) is independent of this
  rewiring and currently `In Progress`; this task doesn't depend on it finishing.
- `read_button_1()` / `read_button_2()`, the ISR handlers, or `mode_switch_setup()` in
  `src/mode_switch2.cc` — press detection and debouncing logic is unchanged.
- Any assigned behavior for the medium (~2s) or long (~5s) presses on either button —
  explicitly deferred to a future task/feature per the user's request.
- FR-001 / FR-003 wording — already button-agnostic, no change needed.
- Adding a native unit test for the button-dispatch `switch` statements in
  `loop()` — this logic is tightly coupled to Arduino globals (`analogWrite`,
  `the_display_mode`, `brightness`) and isn't extracted into a testable module;
  extracting it would be new scope (and overlaps what TASK-001 is already doing for
  the press-duration side), not part of this reassignment.
- Any change to pin assignments, ISR attachment, or interrupt trigger types in
  `include/pins.h` or `mode_switch_setup()`.

## Approach

1. In `src/main.cpp`, rename `input_switch_medium_press()` to `toggle_display_mode()`
   (keep its signature/body as-is — it already takes and returns `enum
   display_mode`).
2. Rewrite button 1's `switch (read_button_1())`:
   - `case quick:` — unchanged, calls `input_switch_quick_press()`.
   - `case medium_2s:` — becomes `break;` with a one-line comment ("no-op; reserved
     for future task/feature").
   - `case long_5s:` — add explicit case with the same no-op + comment (previously
     implicit via `default`).
   - `default:` — unchanged (`break;`).
3. Rewrite button 2's `switch (read_button_2())`:
   - `case quick:` — now calls `the_display_mode =
     toggle_display_mode(the_display_mode);` instead of
     `input_switch_quick_press()`.
   - `case medium_2s:` — explicit no-op + comment (was already an empty case;
     keep it, just add the comment for symmetry with button 1).
   - `case long_5s:` — add explicit case, no-op + comment.
   - `default:` — unchanged.
   - Remove the `// Replace this with ...` comment above this switch.
4. Update `docs/requirements/use-cases.md` UC-001 and UC-002 per the Scope section
   above (Trigger, main flow wording, alternate-flow notes).
5. Build for the target env (`pio run`) to confirm it still compiles — no native
   test exists for this dispatch logic (see Out of scope), so this is a compile +
   manual/on-device check, not a unit-test pass.

Completeness check: since this only touches two `switch` statements in one function
plus one renamed function and two UC sections, completeness is verified by reading
the full diff of `src/main.cpp` and `docs/requirements/use-cases.md` against this
plan's enumerated changes above — no grep-count needed given the small, fixed set of
edit sites.

## Verification

- `pio run` builds clean for the target board environment (no new warnings beyond
  whatever baseline currently exists).
- `pio check` run against the touched file, per CLAUDE.md's static-analysis
  convention.
- Manual on-device check (per CLAUDE.md: anything that must touch real silicon is
  verified on-target, not unit tested): quick-press button 1 → brightness advances;
  quick-press button 2 → display mode toggles between MM:SS and HH:MM; hold either
  button ~2s or ~5s → nothing happens.
- Diff review of `src/main.cpp` and `docs/requirements/use-cases.md` against the
  Scope section to confirm nothing beyond the listed edit sites changed.

## Risks

- `toggle_display_mode()`'s rename could miss a call site if one exists outside
  `main.cpp` — mitigated by grepping for `input_switch_medium_press` before and after
  to confirm zero remaining references.
- Since button 2's quick press previously shared `input_switch_quick_press()` with
  button 1, removing that call means button 2 no longer touches `brightness`/
  `HV_PWM_CONTROL` at all — worth confirming no other code path assumed button 2
  could also drive brightness.

## Open questions

None — scope, category, and the UC-doc update were confirmed with the user before
drafting this plan.
