## Purpose

Converts reusable PNG artwork into deterministic Rockbox bitmap headers through
a declared build manifest while retaining source and license provenance.

## ADDED Requirements

### Requirement: Manifest-declared assets
The compiler SHALL read a project manifest that names source PNG files, ordered
transform operations, emitted asset names, and output headers.

#### Scenario: Compile a valid manifest
- **WHEN** all declared sources and operations are valid
- **THEN** the compiler writes the declared header with every named output asset

#### Scenario: Missing source image
- **WHEN** a declared source file does not exist
- **THEN** compilation fails with the source path and does not replace a valid existing header

### Requirement: Required pixel-art transforms
The compiler SHALL support rectangular crops, multi-image assembly,
nearest-neighbor resize, alpha transparency, transparent color replacement,
time-of-day tint variants, and Rockbox RGB565-swapped output.

#### Scenario: Resize pixel art
- **WHEN** a manifest resizes a source image
- **THEN** the result uses nearest-neighbor sampling and introduces no blended pixels

#### Scenario: Emit transparent pixels
- **WHEN** a source or transform marks a pixel transparent
- **THEN** the generated Rockbox data uses the configured transparent color key

#### Scenario: Create palette variants
- **WHEN** a manifest requests day, evening, and night variants
- **THEN** the compiler emits three consistently named arrays with identical dimensions

### Requirement: Deterministic generated output
The compiler SHALL produce byte-identical header output when the manifest,
source images, compiler version, and options are unchanged.

#### Scenario: Repeat compilation
- **WHEN** an unchanged asset project is compiled twice
- **THEN** both generated headers have identical bytes

### Requirement: Manifest validation and atomic output
The compiler SHALL reject duplicate output names, invalid crop bounds,
inconsistent assembly dimensions, unsupported operations, and malformed color
values before replacing the output header.

#### Scenario: Duplicate asset name
- **WHEN** two manifest entries declare the same emitted asset name
- **THEN** compilation fails and leaves the previous output untouched

### Requirement: Asset provenance
The manifest SHALL allow each source collection to record its creator, source
location, and license, and the compiler SHALL emit a provenance summary next to
or within the generated output.

#### Scenario: Compile mixed-origin artwork
- **WHEN** a project uses generated artwork and CC0 artwork from another creator
- **THEN** the resulting provenance summary identifies both source collections and their licenses
