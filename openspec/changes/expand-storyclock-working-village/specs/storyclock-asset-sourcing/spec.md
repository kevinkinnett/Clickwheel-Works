## Purpose

Defines how Story Clock selects, records, converts, and reviews reusable open artwork and generated artwork for redistribution with the project.

## ADDED Requirements

### Requirement: Reuse-first asset selection
The project SHALL inventory suitable project-owned and already-imported artwork before acquiring open artwork, and SHALL evaluate compatible open artwork before generating a missing asset.

#### Scenario: Begin a district art pass
- **WHEN** a farm, garden, or smithy asset is needed
- **THEN** the asset notes identify the existing and open candidates considered before any generated replacement is accepted

### Requirement: Generated artwork as a recorded fallback
The project SHALL use generated artwork only when the reuse review finds no candidate that remains readable and stylistically coherent after native-resolution conversion.

#### Scenario: Generate a missing landmark
- **WHEN** the reuse review rejects every candidate for a required landmark
- **THEN** the project records the rejected candidates and the native-resolution reason before adding generated source art

#### Scenario: Open artwork passes review
- **WHEN** a reusable open asset fits the required scale, perspective, palette, and silhouette
- **THEN** the project uses that asset instead of generating a replacement for the same role

### Requirement: Redistributable licensing
Every third-party asset SHALL have terms compatible with repository redistribution and the project SHALL retain its license text beside the imported source.

#### Scenario: Import an open asset pack
- **WHEN** source artwork enters `assets/storyclock/third-party`
- **THEN** its local directory contains the original source file and license or attribution text

#### Scenario: License terms are unclear
- **WHEN** a candidate's redistribution or modification terms cannot be confirmed
- **THEN** the project rejects the candidate and does not add it to the repository

### Requirement: Complete provenance records
The Story Clock manifest and asset notes SHALL record each imported or generated source's creator, source location, license or usage basis, selected files, relevant edits, and attribution requirement.

#### Scenario: Audit an emitted sprite
- **WHEN** a maintainer traces an emitted C asset through the manifest
- **THEN** the source image and its provenance record can be identified without relying on conversation history

### Requirement: Pixel-preserving conversion
The asset pipeline SHALL convert accepted artwork using deterministic crops, palette adjustments, transparency handling, and nearest-neighbor scaling suitable for the 220 by 176 display.

#### Scenario: Recompile unchanged assets
- **WHEN** the compiler processes an unchanged manifest and unchanged sources twice
- **THEN** it emits byte-identical C arrays both times

#### Scenario: Review converted art
- **WHEN** a candidate is reduced to its in-game dimensions
- **THEN** the review uses native-scale output without smoothing or interpolation

### Requirement: Native-resolution acceptance gate
No new district asset SHALL be considered complete until its silhouette, palette, perspective, transparency, and collision alignment pass a native-resolution scene review.

#### Scenario: Candidate fails scene review
- **WHEN** an asset becomes muddy, floats above the ground, hides an entrance, or conflicts with its collision footprint at native scale
- **THEN** the project revises or replaces it before accepting the district art
