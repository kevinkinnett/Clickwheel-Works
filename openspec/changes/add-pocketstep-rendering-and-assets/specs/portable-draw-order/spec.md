## Purpose

Provides graphics-API-independent ordering of small drawable sets so actors and
tall scenery overlap according to their visible foot positions.

## ADDED Requirements

### Requirement: Caller-owned draw storage
The draw-order facility SHALL use storage supplied by the caller and SHALL NOT
allocate memory or depend on a window, display, or graphics API.

#### Scenario: Initialize bounded storage
- **WHEN** a caller initializes a draw list with an array and positive capacity
- **THEN** the list accepts records up to that capacity without allocating memory

#### Scenario: Reject invalid storage
- **WHEN** a caller initializes a list with missing storage or non-positive capacity
- **THEN** initialization fails without reading or writing through the invalid storage

### Requirement: Stable foot-position ordering
The facility SHALL order records by ascending foot position and SHALL preserve
insertion order when two records have the same foot position.

#### Scenario: Insert mixed depths
- **WHEN** records with lower, higher, and equal foot positions are added
- **THEN** iteration returns them from back to front with equal-depth records in insertion order

### Requirement: Explicit capacity failure
The facility SHALL report when an insertion exceeds capacity and SHALL leave all
previously accepted records intact.

#### Scenario: Add beyond capacity
- **WHEN** a caller adds a record to a full draw list
- **THEN** the add operation reports failure and the existing ordered records remain unchanged

### Requirement: Opaque application payload
Each ordered record SHALL retain caller-provided type, identifier, position, and
foot-position values without interpreting their drawing meaning.

#### Scenario: Retrieve a record
- **WHEN** the caller iterates a successfully populated list
- **THEN** every returned record contains the same application payload supplied at insertion
