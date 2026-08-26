## Purpose

Defines repeatable selection of complete autonomous action scripts so a
screensaver can vary its route without accepting player input.

## ADDED Requirements

### Requirement: Itinerary collection
The story system SHALL let an application provide multiple caller-owned action
scripts with independent action counts.

#### Scenario: Select one script
- **WHEN** a story loop begins with two or more valid itineraries
- **THEN** the director executes one itinerary without reading beyond its action
  count

### Requirement: Deterministic itinerary selection
Itinerary selection SHALL return the same valid index for the same selection
seed, loop index, and itinerary count.

#### Scenario: Repeat a simulator run
- **WHEN** two simulator runs use the same review seed and loop index
- **THEN** both runs choose the same itinerary

#### Scenario: Advance the loop
- **WHEN** the loop index changes while multiple itineraries are available
- **THEN** selection can choose a different valid itinerary

### Requirement: Complete route execution
Once selected, an itinerary SHALL run to its declared end before another
itinerary can be selected.

#### Scenario: Route returns through Village Green
- **WHEN** an itinerary includes an outbound district and a return through the
  green
- **THEN** the director completes both legs before resetting or choosing another
  itinerary

### Requirement: Safe itinerary failure
The director SHALL enter its existing safe failure state if an itinerary names
an unreachable destination or invalid scene transition.

#### Scenario: Authored route is blocked
- **WHEN** collision data prevents a required walk action from reaching its
  destination
- **THEN** the actor does not cross blocked cells and the itinerary does not
  continue as if the walk succeeded

### Requirement: Forced review scenario
Simulator review mode SHALL allow each itinerary to be selected directly for
deterministic recording.

#### Scenario: Record the River Mill route
- **WHEN** the simulator starts with the River Mill review scenario
- **THEN** the director selects the River Mill itinerary regardless of normal
  loop selection

### Requirement: Repeatable reset
Completing an itinerary SHALL reset actor, inventory, dialogue, NPC facing,
ambient state, and selected-scene state before the next loop begins.

#### Scenario: Begin a second loop
- **WHEN** the selected itinerary reaches its end action
- **THEN** the next loop begins at the house with no state leaking from the
  previous itinerary
