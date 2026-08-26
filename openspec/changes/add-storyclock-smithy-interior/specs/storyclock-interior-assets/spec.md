## Purpose

Defines how Story Clock selects, adapts, records, and accepts artwork for visually distinct interiors on the 220 by 176 iPod display.

## ADDED Requirements

### Requirement: Interior asset audition
The project SHALL compare existing project assets and compatible open 16 by 16 artwork at native scene scale before generating a missing interior asset.

#### Scenario: Begin the smithy art pass
- **WHEN** candidate smithy artwork is reviewed
- **THEN** the review records the existing project art and open-library candidates considered for architecture, forge equipment, furniture, and ambience

#### Scenario: Compare viable directions
- **WHEN** more than one coherent source treatment remains viable
- **THEN** native 220 by 176 mockups expose the treatments in the same room layout before the final source is chosen

### Requirement: Generated signature art fallback
The project SHALL generate new interior artwork only for a required role whose reusable candidates fail scale, perspective, palette, silhouette, animation, or licensing review.

#### Scenario: Reusable forge passes review
- **WHEN** an open forge asset remains readable and coherent after deterministic conversion
- **THEN** the project uses that artwork instead of generating another forge for the same role

#### Scenario: Forge candidates fail review
- **WHEN** no reusable forge or ambient-animation candidate passes native-resolution review
- **THEN** the project records the rejection reasons, generates a focused replacement, and retains its final prompt beside the source image

### Requirement: Complete interior provenance
Every accepted third-party or generated interior source SHALL record its creator or generation mode, source location, license or usage basis, selected files or cells, transformations, and attribution requirement.

#### Scenario: Audit a compiled smithy sprite
- **WHEN** a maintainer traces a smithy sprite from the emitted C asset through the manifest
- **THEN** the exact source artwork, selection coordinates, transformations, and provenance record are available in the repository

#### Scenario: Candidate terms are incompatible or unclear
- **WHEN** an asset cannot be redistributed and modified under terms compatible with the project
- **THEN** the project rejects it and does not include it in source art or emitted assets

### Requirement: Interior visual-identity profile
Each accepted village interior SHALL define a floor or wall treatment, a dominant landmark, a room-specific clutter family, and an ambient effect that collectively distinguish it from other interiors.

#### Scenario: Add another interior
- **WHEN** a future inn, apothecary, shop, mill, farmhouse, or garden shed enters review
- **THEN** its asset notes identify those four identity elements and do not satisfy all four by copying the opening house or smithy treatment

### Requirement: Deterministic pixel conversion
The asset pipeline SHALL use recorded crops, transparency handling, palette operations, and nearest-neighbor scaling to compile accepted interior art into byte-identical fixed-size C assets.

#### Scenario: Recompile the smithy assets
- **WHEN** unchanged source art and manifest data are compiled twice
- **THEN** both runs emit byte-identical smithy arrays without smoothing or runtime image decoding

### Requirement: Native-resolution interior acceptance
No interior art pass SHALL be complete until its architecture, doorway, landmark, props, animation, collision alignment, actor contrast, and day, evening, and night treatment pass review at native 220 by 176 resolution.

#### Scenario: Candidate obscures play-space information
- **WHEN** a native capture shows a floating doorway, muddy landmark, unstable animation, hidden actor, blocked route, or collision mismatch
- **THEN** the project revises or rejects that candidate before accepting the interior
