## Purpose

Defines fixed-memory top-down grid navigation and interaction queries that can
run in host tests, Rockbox plugins, and other small C99 programs.

## ADDED Requirements

### Requirement: Fixed-size passability grid
PocketStep SHALL represent a rectangular tile grid with caller-defined
dimensions and passable or blocked cells without heap allocation.

#### Scenario: Query a blocked tile
- **WHEN** a caller queries a cell marked as blocked
- **THEN** PocketStep reports that an actor cannot enter that cell

#### Scenario: Query outside the grid
- **WHEN** a caller queries a coordinate outside the declared grid
- **THEN** PocketStep treats the coordinate as blocked

### Requirement: Four-direction route finding
PocketStep SHALL find a shortest available route between two passable cells
using only up, down, left, and right steps and a caller-provided route buffer.

#### Scenario: Route around furniture
- **WHEN** blocked cells prevent a direct route but another route exists
- **THEN** PocketStep returns a shortest route that contains no blocked cell

#### Scenario: Destination cannot be reached
- **WHEN** blocked cells separate the start from the destination
- **THEN** PocketStep reports that no route exists and returns no partial route

#### Scenario: Route exceeds caller capacity
- **WHEN** a valid route requires more steps than the supplied route buffer
- **THEN** PocketStep reports insufficient capacity without writing past the buffer

### Requirement: Deterministic route selection
PocketStep SHALL return the same route for the same grid, start, destination,
and route capacity.

#### Scenario: Equivalent routes exist
- **WHEN** two or more shortest routes reach the destination
- **THEN** repeated route searches choose the same route

### Requirement: Interaction regions
PocketStep SHALL let an application associate numeric IDs with non-solid
rectangular regions and query them with a body-sized probe placed in front of
an actor.

#### Scenario: Actor faces an adjacent object
- **WHEN** the actor's facing probe overlaps an interaction region
- **THEN** the query returns that region's numeric ID

#### Scenario: Object is behind the actor
- **WHEN** an interaction region does not overlap the facing probe
- **THEN** the query does not report that region

### Requirement: Portable fixed-memory operation
The grid-navigation API SHALL remain C99, header-only, free of heap allocation,
and independent of Rockbox or any window system.

#### Scenario: Host-library build
- **WHEN** the PocketStep host test target includes the grid-navigation API
- **THEN** it compiles and runs without Rockbox headers or libraries
