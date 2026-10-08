<!-- CONTRACT: what the system must DO. One row per requirement. IDs are permanent —
     never renumbered or reused. Dropped requirements get Status: Deprecated /
     Superseded, not deleted. Delete the _EXAMPLE_ row once you've seen the shape. -->

# Functional Requirements

Convention: `FR-###`, zero-padded, assigned sequentially by next-highest-number.
Priority: `Must` / `Should` / `Could` (MoSCoW). Status: `Proposed` / `Approved` /
`Implemented` / `Deprecated` / `Superseded by FR-0XX`.

| ID | Requirement | Priority | Related Use Cases | Status |
|---|---|---|---|---|
| FR-001 | Display the current time on four Nixie tubes, switchable between MM:SS and HH:MM modes via a button press | Must | UC-002 | Implemented |
| FR-002 | Maintain accurate time using an external RTC (DS3231 or DS1307) that survives power cycles | Must | — | Implemented |
| FR-003 | Allow brightness adjustment via a push-button that cycles through preset PWM brightness levels | Must | UC-001 | Implemented |
| FR-004 | Flash the colon/separator once per second as a visible seconds indicator | Must | — | Implemented |

<!-- Add new rows via /new-requirement, or by hand — keep the table format. Each
     requirement should be a single testable statement, not a paragraph. If it needs
     a paragraph, it's probably a use case (docs/requirements/use-cases.md) with this
     as one of its acceptance criteria. -->
