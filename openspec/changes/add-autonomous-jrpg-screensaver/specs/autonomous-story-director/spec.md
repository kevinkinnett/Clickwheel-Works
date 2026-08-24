## Purpose

Defines a fixed-memory director that moves autonomous actors through visible,
completion-driven story actions without simulated player input.

## ADDED Requirements

### Requirement: Ordered action execution
The director SHALL execute one scripted action at a time and SHALL begin the
next action only after the current action reports completion.

#### Scenario: Walk followed by dialogue
- **WHEN** a walk action precedes a dialogue action
- **THEN** dialogue begins only after the actor reaches the requested destination

### Requirement: Supported screensaver actions
The director SHALL support walking to a grid cell, changing facing direction,
waiting for a fixed number of updates, displaying dialogue, collecting an
item, and changing scenes at a named spawn point.

#### Scenario: Item collection sequence
- **WHEN** the script reaches a collect-item action beside the matching object
- **THEN** the item becomes collected before the following action begins

#### Scenario: Scene change sequence
- **WHEN** the script completes a scene-change action
- **THEN** the destination scene loads and the actor appears at the requested spawn

### Requirement: Movement completion follows world state
Walking actions SHALL advance from the actor's current position through a valid
route and SHALL not infer completion from elapsed wall-clock time.

#### Scenario: Actor walks around an obstacle
- **WHEN** a route bends around blocked tiles
- **THEN** the actor follows the route and completes the action at the destination

#### Scenario: Route becomes unavailable
- **WHEN** no valid route reaches the requested destination
- **THEN** the director enters a safe failure state instead of moving through blocked cells

### Requirement: Automatic dialogue timing
Dialogue actions SHALL remain visible for a duration derived from the amount of
text and SHALL finish without user input.

#### Scenario: Longer dialogue remains longer
- **WHEN** one dialogue contains more visible characters than another
- **THEN** its automatic display duration is not shorter

### Requirement: Repeatable story reset
The director SHALL reset actor, item, dialogue, and scene state before starting
a new story loop.

#### Scenario: Story begins again
- **WHEN** the final action completes
- **THEN** the next loop starts from its declared opening scene and state

### Requirement: Deterministic review mode
The director SHALL support a deterministic simulator mode that produces the
same action order and initial state on repeated runs.

#### Scenario: Repeat simulator capture
- **WHEN** the same review scenario starts twice
- **THEN** both runs use the same story actions, spawns, and item state
