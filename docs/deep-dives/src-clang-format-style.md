# What formatting conventions does `src/`/`include/` actually follow, and what `.clang-format` matches them?

**Date:** 2026-10-10
**Commit:** b1ad9d6

## Question

CLAUDE.md states "`clang-format` (config committed to the repo) is the formatting
source of truth" — but no `.clang-format` file existed in the repo. What style does
the existing hand-written code in `src/`/`include/` actually follow, and what config
reproduces it with minimal diff?

## Method

Read every file in `src/` and `include/` (1,117 lines total) for indentation, brace
placement, pointer/reference alignment, comment spacing, and line-length patterns.
Then iteratively drafted `.clang-format`, running `diff <(clang-format <file>) <file>`
per file and tuning options to minimize the diff, rather than guessing settings and
accepting whatever came out.

## Findings

- **Indentation:** 4 spaces, no tabs, everywhere (`grep -lP '\t'` found none).
- **Braces:** Attached (K&R/"Attach" style) for functions, control statements,
  structs, and enums — never on their own line. `else` stays cuddled with the
  preceding closing brace.
- **`switch`:** `case` labels are indented one level in from `switch` (not aligned
  with it) — e.g. `src/clock_mode.cc:4-14`, `src/display_mode.cc:5-14`.
- **Pointer/reference alignment:** Only one reference in the whole tree
  (`src/RTC.cc:72`, `const DateTime &dt`) — attached to the variable name, not the
  type (`Right`/`Pointer` alignment).
- **Trailing comments:** Overwhelmingly use **2 spaces** before `//` (not 1) — e.g.
  `extern volatile int brightness;  // defined in main.cpp` (`src/RTC.cc:21`). A
  handful of lines in `RTC.cc` and `mode_switch2.cc` use only 1 space; these read as
  inconsistency in the original hand-formatting, not a second convention — adopting
  the 2-space rule project-wide is a strict improvement on self-consistency.
- **Line length:** No single line in `src/`/`include/` exceeds 130 characters
  (confirmed by an explicit max-length scan); 99% of lines are 107 characters or
  under. A `ColumnLimit` of 130 reproduces the codebase with zero forced reflows from
  line length, while still being an enforced ceiling going forward.
- **Enums:** Short, comment-free enums are written on one line (`clock_mode.h`'s
  `operating_mode`/`time_field`/`date_field`/`field_adjust_direction`); one enum in
  `mode_switch2.h` (`display_mode`) is written across multiple lines for no apparent
  reason — an inconsistency, not a second convention (`AllowShortEnumsOnASingleLine:
  true`, clang-format's default, matches the dominant form).
- **Macro/`#define` blocks:** Consecutive `#define`s sometimes have their trailing
  comments manually aligned to a shared column (`include/pins.h:22-24`,
  `include/button_timing.h:11-13`). Tested `AlignConsecutiveMacros: true` to
  reproduce this — it over-corrects (also aligns the macro *values*, which the
  original code does not do) and nets *more* diff than leaving it off, so it's left
  off; the 2-space trailing-comment rule alone already reproduces most of these
  blocks exactly.

## Result

`.clang-format` (repo root) — see the file itself for the full option list. Applying
it (`clang-format <file>` without `-i`, no files actually reformatted by this
investigation) against every file in `src/`/`include/` produces a 94-line total
diff across 17 files, 9 of which are byte-for-byte already clang-format-clean:

| Diff lines | File |
|---|---|
| 0 | `brightness.h`, `button_timing.cc`, `button_timing.h`, `clock_mode.h`, `display_digits.cc`, `display_digits.h`, `brightness.cc`, `RTC.h` |
| 2–5 | `clock_mode.cc`, `display_mode.cc`, `pins.h` |
| 7–21 | `mode_switch2.h`, `main.cpp`, `RTC.cc`, `print.cc`, `print.h`, `mode_switch2.cc` |

The larger diffs are concentrated in `mode_switch2.cc` (an explicitly-labeled
imported tutorial example, per its header comment, with doubled spaces throughout
its prose comments) and in a few long lines that clang-format's line-breaking
algorithm wraps differently than the original hand-wrapping (same information,
different break points) — not systematic mismatches with the config.

## Open question for whoever applies this

The repo currently has no committed `.clang-format`, so CLAUDE.md's "`clang-format`
... is the formatting source of truth" convention wasn't actually enforceable before
now. Whether to run `clang-format -i` across `src/`/`include/` now (accepting the
94-line diff above) is a decision for whoever picks this up — this investigation
only produced and validated the config, it didn't apply it.
