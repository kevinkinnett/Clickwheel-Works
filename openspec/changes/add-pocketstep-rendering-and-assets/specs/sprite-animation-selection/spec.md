## Purpose

Provides deterministic sprite-frame selection for small directional characters
without coupling animation state to a graphics system or device API.

## ADDED Requirements

### Requirement: Directional sheet description
The animation facility SHALL describe frame dimensions, frames per direction,
and the sprite-sheet row assigned to each supported facing direction.

#### Scenario: Resolve a directional frame
- **WHEN** a caller requests a frame for a valid facing direction and frame index
- **THEN** the facility returns the corresponding source rectangle within the described sheet

#### Scenario: Reject an invalid description
- **WHEN** frame dimensions, frame count, or a facing-row mapping is invalid
- **THEN** frame resolution reports failure rather than returning an out-of-range rectangle

### Requirement: Distance-driven walking frames
The facility SHALL select moving frames from accumulated whole-pixel travel
distance and a positive caller-selected distance per frame.

#### Scenario: Advance while moving
- **WHEN** accumulated travel crosses successive distance-per-frame boundaries
- **THEN** the selected frame advances and wraps within the direction's frame count

#### Scenario: Preserve frame during a pause
- **WHEN** no additional travel distance is reported
- **THEN** repeated selection returns the same moving frame

### Requirement: Explicit idle frame
The facility SHALL let a caller select a declared idle frame independently of
the last moving frame.

#### Scenario: Character stops
- **WHEN** the caller requests an idle selection
- **THEN** the facility returns the declared idle frame for the current direction

### Requirement: Portable deterministic behavior
The same animation description and movement inputs SHALL produce the same frame
selection on host tests, the Rockbox simulator, and the target device.

#### Scenario: Replay movement inputs
- **WHEN** two runs receive the same facing, movement-state, and distance sequence
- **THEN** both runs produce the same source-rectangle sequence
