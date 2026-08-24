## Purpose

Defines the observable autonomous JRPG-style clock vignette for the iPod Color
and its deterministic Rockbox simulator review behavior.

## ADDED Requirements

### Requirement: Autonomous operation
The plugin SHALL perform its complete story loop without directional or
interaction input from the viewer.

#### Scenario: Viewer provides no input
- **WHEN** the plugin starts and the viewer does not touch the click wheel
- **THEN** the character moves, interacts, changes scenes, and restarts the story

### Requirement: Two fixed-screen scenes
The first release SHALL contain one 13 by 11 house scene and one 13 by 11
outdoor scene built from original 16-pixel artwork.

#### Scenario: Character leaves the house
- **WHEN** the story reaches the house exit
- **THEN** the outdoor scene replaces the house and the character appears at its entry spawn

#### Scenario: Character walks behind scenery
- **WHEN** the character passes behind a tall object such as a tree or furniture
- **THEN** drawing order uses the actor's feet so the overlap reads as depth

### Requirement: Visible autonomous story
The opening story SHALL show the character wake inside the house, collect an
item, speak with an NPC, leave the house, interact with an outdoor NPC or
object, and reach a visible ending before the loop resets.

#### Scenario: Item changes state
- **WHEN** the character completes the collection action
- **THEN** the world representation and inventory indicator both show that the item was collected

#### Scenario: NPC conversation
- **WHEN** the character reaches a conversation action
- **THEN** the actor faces the NPC and an on-screen dialogue box displays the exchange

### Requirement: Readable on-screen dialogue
The plugin SHALL render automatically advancing dialogue within the 220 by 176
display without covering the speaker name, text continuation cue, or current
line content.

#### Scenario: Two-line dialogue
- **WHEN** dialogue wraps onto a second line
- **THEN** both lines remain inside the dialogue box and visible for the full action

### Requirement: Clock presentation
The plugin SHALL display the iPod's current time throughout the story and SHALL
update it while movement or dialogue is active.

#### Scenario: Minute changes during dialogue
- **WHEN** the system minute changes while a dialogue box is visible
- **THEN** the displayed clock advances without resetting the dialogue action

### Requirement: Time-of-day appearance
The outdoor scene SHALL select day, evening, or night colors from the iPod
clock while preserving collision, route, and story behavior.

#### Scenario: Night launch
- **WHEN** the plugin starts during the configured night period
- **THEN** the outdoor scene uses its night palette

### Requirement: Screensaver controls
The plugin SHALL require no controls for story progression and SHALL provide a
Rockbox action that exits cleanly.

#### Scenario: Exit request
- **WHEN** the viewer invokes the exit action
- **THEN** the plugin restores Rockbox control without corrupting story or engine state

### Requirement: Simulator and device verification
The plugin SHALL build for the `ipodcolor` simulator and device target and
SHALL expose a deterministic capture mode for the complete opening story.

#### Scenario: Deterministic visual capture
- **WHEN** the capture script records the opening-story review scenario
- **THEN** the resulting video contains both scenes, item collection, and dialogue
