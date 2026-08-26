## Purpose

Defines enterable, visually distinct village interiors and their autonomous story behavior, beginning with the South Gate smithy.

## ADDED Requirements

### Requirement: Reciprocal smithy entrance
Story Clock SHALL connect the existing South Gate smithy doorway to a dedicated smithy scene and SHALL return Luma through the same doorway relationship.

#### Scenario: Enter the smithy
- **WHEN** Luma reaches the South Gate smithy entrance during the smithy itinerary
- **THEN** the smithy scene loads with Luma near its bottom doorway facing into the room

#### Scenario: Leave the smithy
- **WHEN** Luma exits through the smithy's bottom doorway
- **THEN** the South Gate scene loads with Luma on the exterior threshold facing away from the building

#### Scenario: Approach the exterior doorway
- **WHEN** Luma walks from the South Gate road to the smithy entrance
- **THEN** the route stays below the facade collision footprint until Luma turns into the doorway threshold

### Requirement: Distinct smithy identity
The smithy SHALL use a sooted stone and timber treatment, a prominent active forge, metalworking furniture, and a room arrangement distinguishable from the opening house at native 220 by 176 resolution.

#### Scenario: Review the smithy beside the house
- **WHEN** native-resolution captures of the smithy and opening house are compared
- **THEN** their floor or wall treatment, dominant landmark, clutter family, and ambient effect are visibly different

#### Scenario: Review workshop density
- **WHEN** the smithy is held at native resolution
- **THEN** wall-edge beams, storage, fuel, and metalworking clutter fill dead areas without narrowing the entrance-to-Rowan aisle or hiding either actor

### Requirement: Navigable authored interior
The smithy SHALL provide passable routes between its entrance, Rowan's conversation position, the forge work area, and the exit while solid architecture and furniture remain blocked.

#### Scenario: Complete the smithy route
- **WHEN** the autonomous smithy itinerary runs from entrance through conversation and back to the exit
- **THEN** Luma reaches every destination without crossing a solid prop, clipping the room boundary, or entering a safe failure state

#### Scenario: Stand beside forge furniture
- **WHEN** Luma or Rowan stands near the forge, anvil, trough, or workbench
- **THEN** foot-position ordering keeps the actor in front of or behind the object according to world depth

### Requirement: Stateful Rowan interaction
Rowan SHALL face Luma while their smithy dialogue is visible. The first successful smithy visit in a plugin session SHALL award the existing Iron Charm once, while later visits SHALL acknowledge the completed exchange without replaying a new-item presentation.

#### Scenario: Receive the Iron Charm
- **WHEN** Luma completes Rowan's smithy conversation without already carrying the Iron Charm
- **THEN** one Iron Charm enters the session inventory and the inventory overlay presents it before the itinerary continues

#### Scenario: Revisit the smithy
- **WHEN** Luma completes Rowan's smithy conversation while already carrying the Iron Charm
- **THEN** Rowan uses repeat-visit dialogue, no duplicate charm is added, and the new-item overlay does not open

#### Scenario: Speak with Rowan
- **WHEN** the dialogue begins from any valid adjacent conversation cell
- **THEN** Rowan and Luma face one another for the duration of the dialogue

### Requirement: Non-disruptive smithy ambience
The smithy SHALL animate at least one forge-related element without changing collision, actor routes, or story timing.

#### Scenario: Observe the working forge
- **WHEN** the smithy remains visible through a dialogue or wait action
- **THEN** forge fire, sparks, bellows, or steam advances through a stable animation loop without flipping the whole object or moving its footprint

### Requirement: Reusable interior presentation
Story Clock SHALL describe indoor presentation separately from scene identity so future interiors can select their own architecture, landmark props, ambient animation, and palette treatment without inheriting opening-house-only drawing behavior.

#### Scenario: Render two indoor scenes
- **WHEN** Story Clock renders the opening house and smithy using their scene descriptions
- **THEN** both use the common indoor render order while selecting their own architecture, props, and ambience

### Requirement: Deterministic smithy review
The simulator tooling SHALL provide a forced smithy scenario that covers entry, Rowan's first-visit dialogue, Iron Charm presentation, ambient animation, and the return to South Gate in a repeatable order.

#### Scenario: Capture the smithy route twice
- **WHEN** the forced smithy scenario starts twice with the same review state
- **THEN** both runs use the same initial inventory, action sequence, actor positions, dialogue, animation timing, and return spawn
