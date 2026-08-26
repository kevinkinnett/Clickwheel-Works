## Purpose

Turns Story Clock item acquisition into a readable autonomous JRPG-style sequence with persistent session inventory and a pixel-art satchel screen.

## ADDED Requirements

### Requirement: Session inventory lifecycle
Story Clock SHALL initialize an empty 12-slot inventory when the plugin starts. District keepsakes SHALL persist across story-loop resets during that plugin session and SHALL clear when the plugin exits and starts again.

#### Scenario: Begin a plugin session
- **WHEN** Story Clock starts
- **THEN** the inventory has twelve available slots and no retained items

#### Scenario: Begin another story loop
- **WHEN** an autonomous itinerary finishes and Story Clock resets for the next loop
- **THEN** retained district keepsakes remain in their existing slot order

### Requirement: Quest item and district rewards
The Ember Key SHALL enter the inventory when Luma collects it and SHALL leave the inventory when the beacon consumes it. Market Row, River Mill, South Gate, South Fields, and Herb Garden SHALL each award one distinct keepsake that remains in the session inventory.

#### Scenario: Collect and use the Ember Key
- **WHEN** Luma collects the Ember Key and later activates the beacon
- **THEN** the key appears in the inventory after collection and is absent after beacon activation

#### Scenario: Complete a new district route
- **WHEN** an itinerary reaches its authored reward moment for the first time in a session
- **THEN** its distinct keepsake is added once and remains through later story loops

#### Scenario: Repeat a district route
- **WHEN** an itinerary reaches a keepsake already retained in the session inventory
- **THEN** Story Clock does not add a duplicate keepsake or replay a new-item presentation

#### Scenario: Inventory cannot accept a reward
- **WHEN** a new reward reaches a full inventory
- **THEN** Story Clock preserves all retained items and shows a visible capacity diagnostic instead of silently losing the reward

### Requirement: Compact world indicator
The normal world view SHALL show a small satchel indicator with occupied slots out of twelve and SHALL NOT show the former `KEY:YES` text.

#### Scenario: Inventory changes
- **WHEN** an item is added or completely removed
- **THEN** the indicator updates its occupied-slot count on the next rendered frame

### Requirement: JRPG-style item grid
The inventory overlay SHALL present a titled 4 by 3 grid containing every retained item in stable slot order. Each occupied cell SHALL show an icon, empty cells SHALL remain visibly distinct, and the focused item SHALL show a pixel highlight, quantity, name, and short description.

#### Scenario: Open a partially filled inventory
- **WHEN** the overlay opens with fewer than twelve occupied slots
- **THEN** retained item icons fill cells in slot order and every remaining grid cell uses the empty-slot treatment

#### Scenario: Focus an item stack
- **WHEN** an occupied cell receives focus
- **THEN** the detail area shows the matching name, quantity, and description

#### Scenario: Open an empty preview
- **WHEN** a deterministic simulator scenario opens an empty inventory
- **THEN** all twelve cells appear empty and the detail area identifies the satchel as empty

### Requirement: Autonomous open and close sequence
An authored inventory presentation SHALL pause story progression, open with a stepped pixel animation, focus the newly added item first, advance the focus through occupied slots at a fixed interval, and close before the next story action begins. Ambient world drawing MAY continue behind the overlay, but actor movement and dialogue SHALL NOT advance while it is open.

#### Scenario: Present a newly collected item
- **WHEN** a successful new-item action is followed by an inventory presentation
- **THEN** the overlay completes its opening, holding, focus-cycle, and closing phases before the story director advances

#### Scenario: No new item was added
- **WHEN** an authored new-item presentation follows a duplicate keepsake attempt
- **THEN** the presentation completes immediately without opening the overlay

#### Scenario: Exit while the overlay is open
- **WHEN** the existing plugin exit control is pressed during an inventory presentation
- **THEN** Story Clock exits normally without requiring the overlay sequence to finish

### Requirement: Item art provenance and readability
Every item and satchel icon SHALL have manifest-declared source provenance and SHALL remain legible at native 220 by 176 resolution in indoor, day, evening, and night palettes. Existing licensed project art SHALL be preferred when it produces a coherent icon; missing art MAY be generated and SHALL retain its prompt.

#### Scenario: Compile item assets
- **WHEN** the deterministic asset compiler processes the Story Clock manifest
- **THEN** it emits every referenced inventory icon with recorded source and transformation metadata

#### Scenario: Review the overlay
- **WHEN** the inventory review scenarios render at native resolution
- **THEN** grid borders, focused cells, item silhouettes, quantities, and descriptions remain distinguishable in each supported palette

### Requirement: Deterministic inventory review capture
Simulator tooling SHALL provide a scenario with a representative populated inventory and a complete autonomous open-and-close recording so visual changes can be reviewed without waiting for random itinerary order.

#### Scenario: Run the populated inventory scenario
- **WHEN** the inventory review scenario starts
- **THEN** it seeds the same item slots, opens the overlay on the same focused item, and follows the same timing on every run
