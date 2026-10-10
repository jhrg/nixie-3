# Bugfix Plan: `pio check`/`pio run`/`pio test` warn about unknown `lib_deps_builtin`/`lib_deps_external` options

**Bug:** BUG-001
**Status:** In Progress
**Created:** 2026-10-09

## Summary

`platformio.ini` defines `lib_deps_builtin`/`lib_deps_external` as custom keys inside
the standard `[env]` section, purely so `[env:uno]` and `[env:pro16MHzatmega328]` can
reuse their values via `${env.lib_deps_builtin}`/`${env.lib_deps_external}`.
PlatformIO validates `[env]`'s keys against a known schema and doesn't recognize
either name, so it prints an "Ignore unknown configuration option" warning for each
on every `pio check`/`run`/`test` invocation. The fix moves both keys into a new,
unvalidated custom section (conventionally named `[common]`) and updates the two
reference sites to read from it instead.

## Root cause

See `docs/bugs/BUG-LOG.md` BUG-001: PlatformIO only validates the option names of
recognized section types (`[platformio]`, `[env]`/`[env:*]`, etc.); a section with an
arbitrary, non-reserved name is never schema-checked, so any keys defined there are
accepted silently. `lib_deps_builtin`/`lib_deps_external` living inside `[env]`
(`platformio.ini:28-38`) is what triggers the warning — the interpolation mechanism
(`${section.key}`) itself works identically regardless of which section the key is
defined in.

## Related requirements

None — see BUG-001's "Related requirements" field. No FR/NFR/UC covers build-tool
output; this is a config-correctness fix, not a behavior change.

## Fix approach

Rename `[env]` to keep its real `[env]`-only keys (`platform`, `framework`,
`monitor_speed`, `build_flags`), and introduce a new `[common]` section holding only
`lib_deps_builtin` and `lib_deps_external`. Update the two consumers:

- `[env:uno]`'s `lib_deps` — `${env.lib_deps_builtin}` → `${common.lib_deps_builtin}`,
  `${env.lib_deps_external}` → `${common.lib_deps_external}`.
- `[env:pro16MHzatmega328]`'s `lib_deps` — same two substitutions.

This is the minimal fix: it removes the only two unrecognized keys from `[env]`
without touching `build_flags`, `build_src_filter`, the `native`/`hv_ps_test`
environments, or which libraries actually get pulled in — the literal values of
`lib_deps_builtin`/`lib_deps_external` are unchanged, only which section defines
them. A broader rewrite of `platformio.ini`'s structure is not warranted: the warning
has one specific, well-understood cause, and PlatformIO's own documentation uses
exactly this `[common]`-section pattern for sharing config across environments.

## Regression risk

Low. `platformio.ini` is the only file referencing these two keys (confirmed via
`grep -rn "lib_deps_builtin\|lib_deps_external"` across the repo — four lines total,
all in `platformio.ini`: the two definitions and the two `${env.*}` reads). No `.cc`/
`.cpp`/`.h` file reads PlatformIO config directly, and `[env:hv_ps_test]` and
`[env:native]` don't reference either key today and won't need to. The only way this
regresses is a typo in the renamed `${common.*}` references, which a build failure
would surface immediately (PlatformIO fails hard on an undefined interpolation
target, unlike the soft warning for an unrecognized key).

## Verification

1. Re-run the bug's reproduction steps: `pio check`, `pio run -e uno`,
   `pio run -e pro16MHzatmega328`, and `pio test -e native` each produce no
   `Ignore unknown configuration option` warnings in their output.
2. `pio run -e uno` and `pio run -e pro16MHzatmega328` both still succeed, and their
   reported Flash/RAM usage is unchanged from before the fix (confirms the same
   libraries — `SPI`, `Wire`, `PinChangeInterrupt`, `adafruit/RTCLib` — are still
   being pulled in, just via `${common.*}` instead of `${env.*}`).
3. `pio test -e native` still passes all existing test suites (unaffected by this
   change, but confirms the edit didn't break `[env:native]`'s unrelated
   `build_src_filter`).
4. No regression test is added — this is a config-file fix with no corresponding
   logic to unit-test; the verification above (warning absence + successful builds)
   is the complete check for a change of this kind.

## Out of scope

- Restructuring `platformio.ini` beyond the two renamed keys (e.g. consolidating
  `build_src_filter`'s repeated `; exclude hv_ps.cc` comment, or further DRYing
  `build_flags`) — unrelated to this warning and not touched here.
- `[env:hv_ps_test]`'s separate `lib_deps` (a GitHub URL, unrelated to
  `lib_deps_builtin`/`lib_deps_external`) — not implicated in this bug.
