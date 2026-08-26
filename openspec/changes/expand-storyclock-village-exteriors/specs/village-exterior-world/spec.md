## Purpose

Defines the connected Story Clock village that Luma explores autonomously on
the 220 by 176 iPod Color display.

## ADDED Requirements

### Requirement: Five-screen village geography
Story Clock SHALL contain Cottage Rise, Village Green, River Mill, Market Row,
and South Gate exterior scenes arranged with Village Green as the district hub.

#### Scenario: Enter the village
- **WHEN** Luma walks south from Cottage Rise
- **THEN** Luma enters the north edge of Village Green at the matching path
  column

#### Scenario: Reach each district
- **WHEN** Luma leaves Village Green west, east, or south
- **THEN** Luma enters River Mill, Market Row, or South Gate respectively at a
  matching passable edge position

### Requirement: Distinct district landmarks
Each exterior scene SHALL have a readable landmark, a clear route through the
screen, and a composition distinguishable at native iPod resolution.

#### Scenario: Review the contact sheet
- **WHEN** the five exterior scenes appear in a native-scale review sheet
- **THEN** the cottage, village well, mill and waterwheel, market facades, and
  south gate are visually distinguishable without labels

### Requirement: Future building entrances
Village Green, River Mill, Market Row, and South Gate SHALL include visible,
collision-aware doors for future interiors while this change keeps those doors
inactive.

#### Scenario: Luma approaches an inactive door
- **WHEN** an itinerary brings Luma beside a future building entrance
- **THEN** Luma remains outside and does not walk through the facade

### Requirement: Autonomous district itineraries
After the existing cottage and beacon sequence, Story Clock SHALL run at least
one complete itinerary through each new district without player input.

#### Scenario: Market itinerary
- **WHEN** the Market Row itinerary is selected
- **THEN** Luma travels through Village Green, visits Market Row, performs its
  authored interaction, and reaches the itinerary's declared ending

#### Scenario: River and gate itineraries
- **WHEN** either the River Mill or South Gate itinerary is selected
- **THEN** Luma reaches that district through connected scene edges and performs
  a district-specific interaction

### Requirement: District life and motion
Each new district SHALL include at least one ambient animated element or moving
character that is independent of Luma's walk animation.

#### Scenario: Observe a district
- **WHEN** Luma pauses in a new exterior scene
- **THEN** water, a wheel, smoke, lantern light, an animal, foliage, or an NPC
  continues to animate

### Requirement: Time-of-day presentation
All exterior terrain, architecture, props, and ambient elements SHALL remain
readable in day, evening, and night palettes without changing collision data.

#### Scenario: Force each palette
- **WHEN** the simulator records the same district in day, evening, and night
- **THEN** the scene uses the requested palette and preserves the same routes

### Requirement: Uncluttered navigation
Every authored itinerary destination and scene entry SHALL remain reachable,
and no screen SHALL fill the route corridor with decorative collision.

#### Scenario: Validate authored routes
- **WHEN** host tests evaluate all itinerary legs and reciprocal scene entries
- **THEN** every leg reports a valid collision-free route

### Requirement: Review and device artifacts
The project SHALL produce deterministic simulator recordings and a loadable
iPod Color Rockbox plugin containing the expanded village.

#### Scenario: Build and record
- **WHEN** the expanded village change is complete
- **THEN** host tests, the simulator build, itinerary recordings, visual review
  sheets, and the device build complete successfully
