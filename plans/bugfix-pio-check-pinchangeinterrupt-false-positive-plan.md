# Bugfix Plan: `pio check` reports a false-positive `preprocessorErrorDirective` from PinChangeInterrupt.h

**Bug:** BUG-002
**Status:** Done
**Created:** 2026-10-10

## Summary

`pio check -e uno` / `pio check -e pro16MHzatmega328` each report a high-severity
`preprocessorErrorDirective` defect twice, pointing at
`.pio/libdeps/<env>/PinChangeInterrupt/src/PinChangeInterrupt.h:68`'s
`#error Please enable at least one PCINT port and pin!`. The library compiles and
links fine with the real `avr-gcc` toolchain; cppcheck's isolated single-file
preprocessing just can't resolve the AVR register-bit macros the library's own
"which PCINT ports exist" detection depends on. The fix adds a narrowly-scoped,
documented cppcheck suppression for this exact defect ID and file — not a change to
our own source.

## Root cause

See `docs/bugs/BUG-LOG.md` BUG-002 for the full trace. In short:

- `PinChangeInterrupt.h:68`'s `#if !PCINT_NUM_USED_PORTS` depends on
  `PinChangeInterruptPins.h`'s `PCINT_HAS_PCINTn` macros, each gated on
  `#ifdef PCINTn` — register-bit names defined deep inside avr-libc's
  ATmega328P device header (reached via `<avr/io.h>`), not by this project or by
  the Arduino core.
- `pio run -e uno` / `pio run -e pro16MHzatmega328` both compile and link
  successfully (confirmed in this session), so the real `avr-gcc` preprocessor
  resolves these macros correctly and at least one PCINT port is detected as used.
- `pio check`'s cppcheck pass analyzes each translation unit in isolation
  (`src/main.cpp` and `src/mode_switch2.cc`, the two files that
  `#include <PinChangeInterrupt.h>` directly, hence the duplicate finding) and
  evidently fails to resolve that same macro chain, so every `PCINT_HAS_PCINTn`
  comes out false, `PCINT_NUM_USED_PORTS` is `0`, and the library's own
  internal safety-net `#error` fires.
- This is present on the pre-feature baseline (`c6d9693`, verified via a throwaway
  `git worktree`) — a pre-existing tooling artifact, not a regression.
- PlatformIO's own `cppcheck.py` (`platformio/check/tools/cppcheck.py`,
  `parse_defect()`) explicitly names `preprocessorErrorDirective` (with
  `syntaxError`) as a known "breaking defect" expected in third-party code analyzed
  in isolation, but only suppresses it for files under the **global**
  `~/.platformio/packages/` directory. `PinChangeInterrupt` is a `lib_deps`
  dependency fetched into the project-local `.pio/libdeps/<env>/`, which falls
  outside that check, so the exemption PlatformIO applies to its own bundled
  packages doesn't reach this library.
- Verified (read-only, via `pio check`'s `--flags` CLI override, no files changed)
  that scoping a suppression to this exact defect ID and filename eliminates both
  findings on both board environments while leaving every other finding (the
  pre-existing low-severity style notes) untouched.

## Related requirements

None — see BUG-002's "Related requirements" field. No FR/NFR/UC covers build-tool
output; CLAUDE.md's "`pio check` runs clean... or a new warning is justified inline
with a comment" convention is what this violates.

## Fix approach

Add a cppcheck suppression scoped to exactly this defect, in `platformio.ini`'s
`[env]` section (inherited by both `[env:uno]` and `[env:pro16MHzatmega328]` via
`${env.build_flags}`-style sharing, the same pattern already used for
`build_flags`):

```ini
check_flags =
    cppcheck: --suppress=preprocessorErrorDirective:*/PinChangeInterrupt.h
```

with a comment above it (matching CLAUDE.md's "don't silence a warning with a
pragma without a comment saying why" convention) explaining that this is a
confirmed cppcheck-only false positive in a vendored `lib_deps` library — real
builds (`pio run`) are unaffected — and pointing at BUG-002 for the full trace.

This is the right scope because:
- It targets the exact defect ID (`preprocessorErrorDirective`) and the exact
  file (`PinChangeInterrupt.h`), so it can't silently swallow an unrelated,
  genuine preprocessor error elsewhere in the project or in a different library.
- It doesn't touch `PinChangeInterrupt.h` itself (vendored third-party code
  re-fetched by `lib_deps` on every clean install — any local edit would be
  silently discarded and isn't ours to maintain).
- It doesn't reach for `--skip-packages` or `--suppress=preprocessorErrorDirective`
  unscoped: both are broader than necessary (`--skip-packages` doesn't even solve
  this, since it only skips the global `~/.platformio/packages/` tree, not
  `lib_deps`-fetched libraries like this one; an unscoped ID suppression would also
  hide a genuine preprocessor error anywhere else in the dependency tree).

## Regression risk

Very low. `check_flags` only affects `pio check`'s cppcheck invocation — it has no
effect on `pio run` or `pio test`, so firmware builds and the native unit test
suite are untouched. The suppression is scoped narrowly enough that it can't mask
a real defect introduced later in our own `src/`/`include/` files (those don't
match `*/PinChangeInterrupt.h`). The only thing that could regress this fix is a
future PinChangeInterrupt library upgrade that moves the `#error` to a different
line or file — `--suppress` doesn't pin a line number here, so a path-only match
is robust to that; it would only need revisiting if the library stops using this
filename entirely.

## Verification

1. Re-run the bug's reproduction steps: `pio check -e uno` and
   `pio check -e pro16MHzatmega328` each report zero high-severity findings and
   exit with `[PASSED]` (confirmed working via the ad-hoc `--flags` override during
   investigation; the committed fix moves that same flag into `platformio.ini` so
   it applies without needing to be typed on the command line).
2. The pre-existing low-severity `style`/`unusedFunction`/`noExplicitConstructor`
   findings are unchanged in count and content — confirms the suppression is
   scoped to only this one defect, not broader.
3. `pio run -e uno` and `pio run -e pro16MHzatmega328` still build clean (sanity
   check that `check_flags` doesn't affect the real build).
4. `pio test -e native` still passes all suites (sanity check that `env:native`,
   which doesn't use `check_flags`, is unaffected).
5. No regression test is added — this is a tool-configuration fix with no
   corresponding logic to unit-test; the verification above (defect-count
   comparison + successful builds/tests) is the complete check for a change of
   this kind.

## Out of scope

- Patching or forking the vendored `PinChangeInterrupt` library to make its own
  macro chain resolve under cppcheck's preprocessor — it's a `lib_deps` dependency,
  not code we own, and the library works correctly under the real toolchain.
- Filing the issue upstream with the `PinChangeInterrupt` or cppcheck maintainers —
  not something this project's fix depends on, though it may be worth doing
  separately.
- The pre-existing low-severity `pio check` findings (`unusedFunction`,
  `noExplicitConstructor`, `cstyleCast`) — all confirmed present on the baseline
  commit and are cppcheck single-TU-analysis noise unrelated to BUG-002; not
  addressed here.
- Extending `check_flags` to cover `[env:native]` or `[env:hv_ps_test]` — neither
  environment includes `PinChangeInterrupt.h` (confirmed: `env:native`'s
  `build_src_filter` excludes `mode_switch2.cc`/`main.cpp` entirely;
  `env:hv_ps_test` builds an unrelated source set), so neither can hit this defect.
