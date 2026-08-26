## Purpose

Defines fixed-memory links, entrances, and prop records for moving actors and
authored objects between connected top-down scenes.

## ADDED Requirements

### Requirement: Directional scene links
PocketStep SHALL let an application describe a directional connection from one
scene edge to another scene using caller-owned records and no heap allocation.

#### Scenario: Follow a linked edge
- **WHEN** an actor leaves a scene through an authored directional link
- **THEN** the link identifies the destination scene and destination entry edge

#### Scenario: Query an unlinked edge
- **WHEN** an application queries a scene edge with no authored link
- **THEN** PocketStep reports that no transition is available

### Requirement: Matching edge entry
A linked transition SHALL place the actor on a passable destination edge cell
nearest the preferred cross-axis offset and SHALL preserve the direction of
travel.

#### Scenario: Move south between scenes
- **WHEN** an actor leaves through a south link at column six
- **THEN** the actor enters the destination near its north edge at the nearest
  passable cell to column six while facing south

#### Scenario: Preferred entry is blocked
- **WHEN** the preferred destination edge cell is blocked
- **THEN** entry selection chooses the nearest passable cell without placing the
  actor inside collision geometry

### Requirement: Building entrance metadata
PocketStep SHALL let a scene associate an interaction region with a destination
scene, spawn cell, and facing direction, including a disabled destination for
an entrance whose interior does not exist yet.

#### Scenario: Future interior entrance
- **WHEN** an actor reaches a building entrance whose destination is disabled
- **THEN** the scene retains the entrance metadata but does not change scenes

#### Scenario: Enabled interior entrance
- **WHEN** an actor activates an entrance with a valid destination
- **THEN** the entrance reports the destination scene, spawn, and facing data

### Requirement: Static prop metadata
PocketStep SHALL support caller-owned prop records containing a scene position,
visible foot position, asset identifier, and collision flag.

#### Scenario: Depth-sort a prop
- **WHEN** an application submits a scene prop to its draw list
- **THEN** the prop's recorded foot position can be used with actor and NPC draw
  records

#### Scenario: Solid prop
- **WHEN** a prop is marked solid
- **THEN** the authored passability grid marks its occupied cell as blocked

### Requirement: Link and entrance validation
The scene-link facility SHALL reject invalid scene indices, directions, spawn
cells, region references, and enabled destinations.

#### Scenario: Destination scene is invalid
- **WHEN** a link or enabled entrance names a destination outside the scene set
- **THEN** validation reports failure

### Requirement: Portable fixed-memory operation
Scene links, entrances, and prop records SHALL remain C99, header-only,
allocator-free, and independent of Rockbox graphics or input APIs.

#### Scenario: Host test build
- **WHEN** a host test includes the scene-link API
- **THEN** it compiles and runs without Rockbox headers or libraries
