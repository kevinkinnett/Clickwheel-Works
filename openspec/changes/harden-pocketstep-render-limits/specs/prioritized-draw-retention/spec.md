## Purpose

Protects essential actors and interactive objects when a caller-owned draw list runs out of fixed storage, without changing visible depth order.

## ADDED Requirements

### Requirement: Caller-assigned retention priority
The draw-list facility SHALL let a caller assign a retention priority to each drawable. Retention priority SHALL affect capacity handling only and SHALL NOT change the drawable's visible ordering.

#### Scenario: Different priorities at different depths
- **WHEN** a caller adds drawables with different retention priorities and foot positions
- **THEN** iteration orders the retained drawables by foot position rather than retention priority

#### Scenario: Existing positional initializer
- **WHEN** a caller omits the newly appended priority field from a positional drawable initializer
- **THEN** the drawable receives the documented optional default priority

### Requirement: Opt-in priority-aware insertion
The facility SHALL provide a priority-aware insertion operation in addition to the existing insertion operation. The existing operation SHALL retain its full-list behavior of rejecting the incoming record and preserving all accepted records.

#### Scenario: Existing insertion on a full list
- **WHEN** a caller uses the existing insertion operation on a full list
- **THEN** the operation reports failure and leaves the retained records unchanged

#### Scenario: Priority-aware insertion below capacity
- **WHEN** a caller uses priority-aware insertion on a list with free capacity
- **THEN** the operation accepts the record and preserves stable foot-position ordering

### Requirement: Deterministic retention under capacity pressure
When priority-aware insertion receives a record on a full list, the facility SHALL retain the incoming record only when its priority is higher than the lowest retained priority. It SHALL replace the last record in draw order among records at that lowest priority, then restore stable foot-position ordering.

#### Scenario: Required actor follows optional scenery
- **WHEN** optional scenery fills a list and a required actor is added through priority-aware insertion
- **THEN** the list discards the last optional record in draw order and retains the actor

#### Scenario: Incoming record has equal priority
- **WHEN** a full list receives a priority-aware record whose priority equals the lowest retained priority
- **THEN** the operation reports failure and leaves the retained records unchanged

#### Scenario: Multiple low-priority candidates
- **WHEN** more than one retained record shares the lowest priority and a higher-priority record arrives
- **THEN** repeated runs with the same inputs discard the same last low-priority record

### Requirement: Discard diagnostics
The draw list SHALL count each record discarded because the list was full. The count SHALL include rejected incoming records and retained records replaced by priority-aware insertion. Clearing the list SHALL reset the count.

#### Scenario: Reject an incoming record
- **WHEN** either insertion operation rejects a valid incoming record because the list is full
- **THEN** the discard count increases by one

#### Scenario: Replace a retained record
- **WHEN** priority-aware insertion replaces a lower-priority retained record
- **THEN** the discard count increases by one

#### Scenario: Start a new frame
- **WHEN** a caller clears a draw list after one or more capacity discards
- **THEN** the record count and discard count both become zero

### Requirement: Caller-owned fixed storage
Priority-aware retention and diagnostics SHALL use only the caller-supplied draw-list storage and SHALL NOT allocate memory or depend on a graphics API.

#### Scenario: Capacity overflow
- **WHEN** priority-aware insertion handles a full list
- **THEN** it completes using the initialized fixed-capacity storage without allocating memory
