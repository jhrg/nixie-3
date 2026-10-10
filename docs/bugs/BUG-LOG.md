<!-- CONTRACT: deviations from intended behavior, tracked from report through root
     cause to fix. Not a duplicate of docs/requirements/ — a bug means something
     already documented (or implicitly assumed) isn't holding true in practice.
     Delete the BUG-EXAMPLE section once you've seen the shape. -->

# Bug Log

Convention: `BUG-###`, sequential, never renumbered. Status: `Open` → `Investigating`
→ `Fix Planned` → `Fixed` → (or `Won't Fix` / `Duplicate of BUG-0XX`). **Only the user
sets a bug to `Fixed`** — that's a claim about verified deployed behavior, not
something to self-report from having written a fix plan.

---

## BUG-EXAMPLE — Password reset link accepted after expiry

*(delete this section once you've seen the shape)*

**Severity:** High
**Status:** Fix Planned
**Reported:** YYYY-MM-DD
**Related requirements:** FR-EXAMPLE, UC-EXAMPLE (violates the "time-limited" part of both)
**Fix plan:** `plans/bugfix-reset-link-expiry-plan.md`

**Reproduction steps:**
1. Request a password reset
2. Wait past the stated expiry window (currently 1 hour)
3. Follow the link anyway

**Expected behavior:** Link is rejected with an "expired, request a new one" message
(per FR-EXAMPLE).

**Actual behavior:** Link still succeeds and resets the password.

**Root cause:** Expiry is checked against token creation time using the token's own
embedded claim, but that claim is client-suppliable and never verified against issue
time recorded server-side. *(Fill this in during investigation — leave `TBD` until
actually found in the code, not guessed from symptoms.)*

---

## BUG-001 — `pio check`/`pio run`/`pio test` warn about unknown `lib_deps_builtin`/`lib_deps_external` options

**Severity:** Low
**Status:** Fix Planned
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

<!-- Add new bugs via /fix-bug, or by hand — keep the section format: metadata lines,
     then Reproduction / Expected / Actual / Root Cause. A bug with no reproduction
     steps yet is a symptom report, not a bug entry — get concrete steps before
     assigning an ID if at all possible. -->
