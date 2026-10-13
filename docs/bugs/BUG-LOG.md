<!-- CONTRACT: deviations from intended behavior, tracked from report through root
     cause to fix. Not a duplicate of docs/requirements/ — a bug means something
     already documented (or implicitly assumed) isn't holding true in practice. -->

# Bug Log

Convention: `BUG-###`, sequential, never renumbered. Status: `Open` → `Investigating`
→ `Fix Planned` → `Fixed` → (or `Won't Fix` / `Duplicate of BUG-0XX`). **Only the user
sets a bug to `Fixed`** — that's a claim about verified deployed behavior, not
something to self-report from having written a fix plan.

---

## BUG-001 — `pio check`/`pio run`/`pio test` warn about unknown `lib_deps_builtin`/`lib_deps_external` options

**Severity:** Low
**Status:** Fixed
**Reported:** 2026-10-09
**Related requirements:** None found. This is a build-tool configuration correctness
issue, not a user-facing functional behavior deviation — it violates the implicit
assumption that running the project's standard PlatformIO commands produces no
unexplained warnings, not any documented FR/NFR/UC.
**Fix plan:** `plans/bugfix-platformio-ini-unknown-options-plan.md`

**Reproduction steps:**
1. Run `pio check` (also reproducible with `pio run` and `pio test`) from the
   project root.
2. Observe the warnings printed before the rest of the command's output:
   ```
   Warning! Ignore unknown configuration option `lib_deps_builtin` in section [env]
   Warning! Ignore unknown configuration option `lib_deps_external` in section [env]
   ```

**Expected behavior:** These commands run without emitting warnings about
unrecognized configuration keys.

**Actual behavior:** Both warnings print on every invocation of `pio check`,
`pio run`, and `pio test`.

**Root cause:** `platformio.ini`'s `[env]` section (`platformio.ini:28-38`) defines
`lib_deps_builtin` and `lib_deps_external` as custom keys purely so `[env:uno]` and
`[env:pro16MHzatmega328]` can interpolate them into their own `lib_deps` via
`${env.lib_deps_builtin}` / `${env.lib_deps_external}` (`platformio.ini:48-50,
61-63`) — a DRY mechanism to share the library list across both board environments.
PlatformIO validates `[env]`/`[env:*]` sections against a known schema of option
names; `lib_deps_builtin` and `lib_deps_external` aren't recognized options for that
section type, so PlatformIO logs a warning and ignores them as configuration (while
still permitting `${env.*}` variable interpolation to read their literal text, which
is why the build itself isn't broken — only the warning is spurious).

---

## BUG-002 — `pio check` reports a false-positive `preprocessorErrorDirective` from PinChangeInterrupt.h

**Severity:** Low
**Status:** Fixed
**Reported:** 2026-10-10
**Related requirements:** None found. Like BUG-001, this is a build-tool
configuration/correctness issue, not a user-facing functional behavior deviation —
it violates CLAUDE.md's "`pio check` runs clean... before a change is considered
finished" convention, not any documented FR/NFR/UC/IC.
**Fix plan:** `plans/bugfix-pio-check-pinchangeinterrupt-false-positive-plan.md`

**Reproduction steps:**
1. Run `pio check -e uno` (also reproducible with `pio check -e pro16MHzatmega328`)
   from the project root.
2. Observe two identical high-severity findings in the output:
   ```
   .pio/libdeps/uno/PinChangeInterrupt/src/PinChangeInterrupt.h:68: [high:error] #error Please enable at least one PCINT port and pin! [preprocessorErrorDirective]
   .pio/libdeps/uno/PinChangeInterrupt/src/PinChangeInterrupt.h:68: [high:error] #error Please enable at least one PCINT port and pin! [preprocessorErrorDirective]
   ```
3. `pio check` exits non-zero (`FAILED`) because of these two high-severity
   defects, even though the project itself builds and tests cleanly.

**Expected behavior:** `pio check` reports no defects for code that compiles and
links successfully with the real toolchain (or, if a tool limitation is unavoidable,
the defect is suppressed with a documented reason rather than surfacing as a
user-facing high-severity error).

**Actual behavior:** `pio check` reports the same `preprocessorErrorDirective`
finding twice (once per translation unit that includes the header —
`src/main.cpp` and `src/mode_switch2.cc` each `#include <PinChangeInterrupt.h>`
directly), and fails the check run.

**Root cause:** Confirmed not a real build defect — `pio run -e uno` and
`pio run -e pro16MHzatmega328` both compile and link successfully (verified in this
session), and the same two findings are present on the pre-feature baseline commit
(`c6d9693`, verified via a throwaway `git worktree`), so this is a pre-existing
tooling artifact, not a regression from recent feature work.

The finding is a cppcheck-specific false positive, not something in our own code.
`PinChangeInterrupt.h:68` (`#if !PCINT_NUM_USED_PORTS`) depends on a macro chain
inside the vendored `PinChangeInterrupt` library (fetched as a `lib_deps` dependency
into `.pio/libdeps/<env>/PinChangeInterrupt/`) that ultimately checks
`#ifdef PCINT0` / `PCINT1` / ... (`PinChangeInterruptPins.h`, the
`PCINT_HAS_PCINTn` block) — AVR register-bit macros defined deep inside avr-libc's
device header (`<avr/io.h>` → the ATmega328P-specific sub-header), not by the
Arduino core or by this project. When the real `avr-gcc` toolchain compiles the
project (`pio run`), those macros resolve correctly and at least one PCINT port is
detected as used, so the library's own `#error` guard never fires. When `pio check`
invokes cppcheck as an isolated single-translation-unit static analyzer, cppcheck's
own preprocessor evidently fails to resolve that same macro chain from the system
headers, so every `PCINT_HAS_PCINTn` macro evaluates false, `PCINT_NUM_USED_PORTS`
comes out `0`, and the library's own internal safety-net `#error` fires — a
limitation of cppcheck's preprocessing of this library's header against the AVR
system headers, not a defect in this project's code.

This also isn't filtered by PlatformIO's own built-in handling for exactly this
class of defect: `cppcheck.py`'s `parse_defect()` explicitly names
`preprocessorErrorDirective` (alongside `syntaxError`) as a known "breaking defect"
that is expected to occur in third-party code analyzed in isolation, but that
suppression only applies to files under the **global**
`~/.platformio/packages/` directory (centrally-installed toolchain/framework/platform
packages). `PinChangeInterrupt` is fetched per-project via `lib_deps` into
`.pio/libdeps/<env>/PinChangeInterrupt/`, which doesn't start with that global
packages path, so the exemption PlatformIO applies to its own bundled packages
doesn't extend to this project-local library dependency, and the known-breaking
defect surfaces as a normal high-severity finding instead of being suppressed.

---

## BUG-003 — Selected field in Set Time/Set Date mode blinks almost entirely off instead of ~1 Hz

**Severity:** Medium
**Status:** Fixed
**Reported:** 2026-10-10
**Related requirements:** NFR-001 — "the field selected for editing must be visually
distinguishable (blinks at ~1 Hz) from the field(s) not selected."
**Fix plan:** `plans/bugfix-set-mode-blink-timing-plan.md`

**Reproduction steps:**
1. Enter Set Time mode (button 2, 2-second press, from Running).
2. Observe the selected field (hour, by default).

**Expected behavior:** The selected field's tubes blink at roughly 1 Hz (on ~0.5s,
off ~0.5s) per NFR-001.

**Actual behavior:** The selected field appears lit only very briefly (on the order
of the next `loop()` iteration after entering the mode) and then stays dark —
reported as "on for about 1/100th of a second, off otherwise."

**Root cause:** `src/RTC.cc`'s `timer_2HZ_tick_ISR()` fires at 2 Hz (once per edge
of the RTC's 1 Hz square wave) and toggles `blink_phase` every call, but it only set
`update_display = true` — the sole flag that gates the shift-register rewrite in
`main.cpp`'s `loop()` — on alternating calls, specifically the ones where
`blink_phase` had just been set to `false`. Since `blink_off_phase()` returns
`!blink_phase`, every actual hardware write therefore happened at the exact instant
`blink_off_phase()` was `true`: the physical display was only ever rewritten once a
second, and always with the selected field blanked. The one-time "briefly on" flash
was just the leftover Running-mode display persisting on the tubes until the first
post-mode-switch write blanked it for good; after that it never recovered, because
`update_display` and the "off" half of the blink cycle were phase-locked together.

**Fix:** `timer_2HZ_tick_ISR()` now sets `update_display = true` on every tick (2 Hz),
so `main.cpp` rewrites the shift register on both halves of the blink cycle and
actually observes `blink_off_phase()` alternate. A new `refetch_time` flag, set only
on the same once-per-second edge the old code used, still gates the RTC read
(`rtc.now()`) and digit recompute in `time_update_handler()` so that expensive I2C
work doesn't double in frequency — only the cheap re-render does.

---

## BUG-004 — Stale digits shown for up to ~0.5 s on Set Time → Set Date and Set Date → Running transitions

**Severity:** Medium
**Status:** Fixed
**Reported:** 2026-10-11
**Related requirements:** UC-004 step 3 (Set Date mode displays the month and day
digit pairs) and step 8 (leaving Set Date resumes the previously active display
mode). Found by code reading while planning `plans/date-display-mode-plan.md`; not
yet observed on hardware.
**Fix plan:** `plans/bugfix-stale-digits-on-mode-change-plan.md` (implemented as part of
`plans/date-display-mode-plan.md` Phase 2)

**Reproduction steps:** (derived from the code; intermittent — about a 50% chance
per transition, depending on which half-second tick follows the button release)
1. In Running mode, at a time whose HH:MM differs visibly from today's MM:DD (e.g.
   14:35 on 10/11), give button 2 a 2-second press to enter Set Time.
2. Give button 2 another 2-second press to enter Set Date.
3. Watch the tubes for the first half-second after the release.
4. Give button 2 a 2-second press to leave Set Date and return to Running (HH:MM
   display mode); watch the first half-second again.
5. Repeat steps 1–4 several times.

**Expected behavior:** On entering Set Date the tubes show month:day (`10 11`, month
blinking) from the first redraw. On returning to Running they show the current
HH:MM from the first redraw.

**Actual behavior (predicted):** On some transitions the first redraw shows the
previous mode's digits in the new layout for ~0.5 s: entering Set Date shows the
time's hour:minute (`14 35`) as month:day, and returning to Running shows the date's
month:day (`10 11`) as HH:MM.

**Root cause:** `src/RTC.cc`'s `time_update_handler()` returns `true` (meaning
"render now") on every 2 Hz `update_display` tick, but only re-reads the RTC and
recomputes `digit_0..5` when `refetch_time` is also set. The ISR sets
`refetch_time` on every other tick (`RTC.cc:115-122`). Which kind of digits gets
computed (date or time) depends on the `mode` argument at the moment of that refetch
(`RTC.cc:255-258`). `digit_0..5` hold no record of which kind they are.

A Button-2 medium press changes `the_operating_mode` in `loop()` (`main.cpp:137-143`)
straight away. If the next tick is a non-refetch tick, `time_update_handler()`
returns `true` with the old mode's digits unchanged. `main.cpp`'s render `switch`
(`main.cpp:150-180`) then lays them out for the new mode:
- the `set_date` case reads hour/minute from `digit_5..2` as month/day;
- the `running` case reads month/day (or day/year, in `mm_ss`) as time.

The wrong digits last until the next refetch tick, at most ~0.5 s later.
Running ↔ Set Time is unaffected because both modes compute time digits.

This came in with the BUG-003 fix. Per that entry's Fix note, `refetch_time` was
added so the RTC read stays at 1 Hz while redraws went to 2 Hz. Before that, every
redraw also refetched, so the digits were always the right kind for the mode.

---

<!-- Add new bugs via /fix-bug, or by hand — keep the section format: metadata lines,
     then Reproduction / Expected / Actual / Root Cause. A bug with no reproduction
     steps yet is a symptom report, not a bug entry — get concrete steps before
     assigning an ID if at all possible. -->
