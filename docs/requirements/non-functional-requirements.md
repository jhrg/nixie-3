<!-- CONTRACT: quality attributes the system must meet, with a measurable target where
     possible. "Fast" and "user-friendly" are not requirements — a number or a
     testable condition is. Delete the _EXAMPLE_ row once you've seen the shape. -->

# Non-Functional Requirements

Convention: `NFR-###`. Category is one of: Performance, Security, Reliability,
Availability, Scalability, Usability, Maintainability, Compliance, Observability
(add categories as needed — keep the list here in sync). Priority: `Must` / `Should`
/ `Could`. Status as in functional-requirements.md.

| ID | Category | Requirement | Measurable Target | Priority | Status |
|---|---|---|---|---|---|
| NFR-001 | Usability | While the clock is in Set Time or Set Date mode, the digit pair currently selected for editing must be visually distinguishable from the digit pair(s) not selected | Selected digit pair blinks at approximately 1 Hz (on/off every ~0.5s); unselected pair(s) remain continuously lit | Must | Proposed |

_(no other hard NFR targets have been confirmed as of the initial planning
interview; add rows via `/new-requirement` as targets are decided — e.g. time drift/
accuracy, HV supply limits, brightness PWM frequency, power-on recovery time.)_

<!-- If you genuinely can't state a measurable target yet, write TBD in that column
     rather than a vague phrase — TBD is honest and searchable; "reasonably fast"
     just looks finished. -->
