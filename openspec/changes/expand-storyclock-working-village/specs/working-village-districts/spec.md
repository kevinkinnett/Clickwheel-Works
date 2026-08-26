## Purpose

Defines the connected farm and garden districts that extend Story Clock's autonomous village while keeping routes readable on the iPod Color display.

## ADDED Requirements

### Requirement: Connected working-village geography
Story Clock SHALL place South Fields south of South Gate and Herb Garden east of Market Row using reciprocal directional scene links.

#### Scenario: Enter South Fields
- **WHEN** Luma leaves South Gate through its south edge
- **THEN** Luma enters the north edge of South Fields near the matching path column while facing south

#### Scenario: Return from South Fields
- **WHEN** Luma leaves South Fields through its north edge
- **THEN** Luma enters the south edge of South Gate near the matching path column while facing north

#### Scenario: Enter and leave Herb Garden
- **WHEN** Luma crosses between the east edge of Market Row and the west edge of Herb Garden
- **THEN** Luma appears just inside the opposite edge near the matching path row while preserving travel direction

### Requirement: Distinct farm composition
South Fields SHALL present an open agricultural area with crop plots, a central travel route, a readable farm building or barn entrance, and a bounded animal area without blocking the required journey.

#### Scenario: Review South Fields at native resolution
- **WHEN** South Fields appears in a 220 by 176 review capture
- **THEN** the road, crop plots, farm building, chicken area, and Luma's walkable space remain visually distinct

### Requirement: Distinct garden composition
Herb Garden SHALL present a more enclosed planted area with shaped beds, a non-linear walkable path, a readable greenhouse or potting-shed entrance, and garden details that do not duplicate the farm composition.

#### Scenario: Review Herb Garden at native resolution
- **WHEN** Herb Garden appears in a 220 by 176 review capture
- **THEN** the path, planted beds, garden building, and Luma's walkable space remain visually distinct

### Requirement: Deferred district interiors
The farm building and garden building SHALL have visible, collision-aware doors recorded as inactive entrances until their interiors exist.

#### Scenario: Approach a deferred entrance
- **WHEN** an itinerary brings Luma beside either inactive door
- **THEN** Luma remains outside and does not cross the building collision footprint

### Requirement: Working smithy presentation
South Gate SHALL identify its existing smithy as a working blacksmith through exterior tools and at least one forge-related ambient action while retaining its current gate route and inactive door.

#### Scenario: Pause at South Gate
- **WHEN** Luma pauses near the smithy
- **THEN** a viewer can distinguish the smithy from the gate through its exterior props and smoke, sparks, firelight, or hammering motion

### Requirement: Autonomous district visits
Story Clock SHALL provide complete farm and garden itineraries that Luma can execute without player input and that the simulator can select directly.

#### Scenario: Run the farm itinerary
- **WHEN** the farm review scenario is selected
- **THEN** Luma travels from the existing village into South Fields, interacts with the farmer or farm environment, and completes the authored route

#### Scenario: Run the garden itinerary
- **WHEN** the garden review scenario is selected
- **THEN** Luma travels from the existing village into Herb Garden, interacts with the gardener or garden environment, and completes the authored route

#### Scenario: Reset after a district visit
- **WHEN** either new itinerary finishes
- **THEN** scene, actor, NPC facing, dialogue, and ambient state reset before the next story loop

### Requirement: District life and motion
South Fields, Herb Garden, and the South Gate smithy SHALL each include restrained ambient motion independent of Luma's walk animation.

#### Scenario: Observe ambient activity
- **WHEN** Luma stands still in one of the three working areas
- **THEN** at least one local element continues a deterministic animation without changing collision or route state

### Requirement: Collision-safe authored routes
All required entries, NPC conversations, environment interactions, and return paths SHALL remain reachable without crossing buildings, fences, crop beds, garden beds, animals, or smithy props.

#### Scenario: Validate every itinerary leg
- **WHEN** host tests run pathfinding across every new itinerary destination and reciprocal scene spawn
- **THEN** each leg reports a valid path and no path enters a blocked landmark footprint

### Requirement: Time-of-day readability
The farm, garden, smithy props, NPCs, and ambient frames SHALL remain readable in day, evening, and night presentation without changing collision data.

#### Scenario: Force each palette
- **WHEN** the simulator records each working area in day, evening, and night
- **THEN** landmarks, entrances, characters, and routes remain visible in all three captures

### Requirement: Review and device artifacts
The project SHALL produce deterministic captures and a loadable iPod Color Rockbox plugin containing the expanded working village.

#### Scenario: Verify the completed expansion
- **WHEN** implementation is complete
- **THEN** host tests, asset compilation, simulator build, forced itinerary recordings, native-scale review sheets, and device build finish successfully
