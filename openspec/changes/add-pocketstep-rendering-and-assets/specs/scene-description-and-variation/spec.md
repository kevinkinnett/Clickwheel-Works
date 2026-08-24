## Purpose

Groups the portable data needed by a fixed-screen scene and supplies repeatable
visual variation without taking ownership of maps or application state.

## ADDED Requirements

### Requirement: Caller-owned scene description
A scene description SHALL reference a tile grid, passability grid, interaction
regions, and spawn cell supplied by the caller without copying or allocating
those collections.

#### Scenario: Read valid scene metadata
- **WHEN** a caller supplies valid map dimensions, arrays, regions, and spawn cell
- **THEN** the scene exposes the same data for navigation, interaction, and spawning

### Requirement: Scene validation
The scene facility SHALL reject inconsistent dimensions, missing required
arrays, invalid region counts, and spawn cells outside the passability grid or
on blocked cells.

#### Scenario: Spawn is blocked
- **WHEN** the declared spawn cell is marked blocked
- **THEN** scene validation reports failure

#### Scenario: Scene dimensions disagree
- **WHEN** the tile and passability descriptions do not cover the same dimensions
- **THEN** scene validation reports failure

### Requirement: Stateless tile variation
The facility SHALL select a variation index from tile coordinates, a caller
seed, and a positive variation count without storing mutable random state.

#### Scenario: Repeat a selection
- **WHEN** the same coordinates, seed, and variation count are supplied repeatedly
- **THEN** the same in-range variation index is returned every time

#### Scenario: Change the scene seed
- **WHEN** a caller changes the seed while keeping a map unchanged
- **THEN** at least some coordinate selections can change without modifying collision data

#### Scenario: Reject an empty variation set
- **WHEN** the caller supplies a variation count of zero
- **THEN** selection reports failure instead of producing an index

### Requirement: Compatibility with grid navigation
Scene descriptions SHALL use the existing PocketStep grid and region semantics
without changing their pathfinding or facing-query results.

#### Scenario: Navigate a described scene
- **WHEN** the existing pathfinder receives the passability grid from a valid scene
- **THEN** it returns the same route it would return for that grid supplied directly
