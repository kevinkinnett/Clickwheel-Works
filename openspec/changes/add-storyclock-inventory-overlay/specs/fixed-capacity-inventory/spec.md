## Purpose

Provides deterministic item and quantity storage for small C programs using only caller-owned fixed memory.

## ADDED Requirements

### Requirement: Caller-owned inventory storage
The inventory facility SHALL use slot storage supplied by the caller and SHALL NOT allocate memory, access files, or depend on a display API.

#### Scenario: Initialize bounded storage
- **WHEN** a caller initializes an inventory with a slot array and positive capacity
- **THEN** the inventory starts empty and can retain no more than that capacity

#### Scenario: Reject invalid storage
- **WHEN** a caller supplies a missing inventory, missing slot storage, or non-positive capacity
- **THEN** initialization fails without reading or writing invalid storage

### Requirement: Stable slot order
The facility SHALL keep retained item types in first-acquisition order. Adding quantity to an existing item SHALL preserve its slot, and removing an entire stack SHALL compact later slots without changing their relative order.

#### Scenario: Add different item types
- **WHEN** a caller adds several new item IDs
- **THEN** indexed iteration returns them in acquisition order

#### Scenario: Add an existing item
- **WHEN** a caller adds quantity to an item ID already present
- **THEN** the existing slot quantity increases and no new slot is created

#### Scenario: Remove a middle stack
- **WHEN** a caller removes the complete quantity of an item between two retained items
- **THEN** the later item moves into the empty position and the remaining order is unchanged

### Requirement: Atomic quantity changes
The facility SHALL accept only positive item IDs, positive quantity changes, and positive stack limits. Addition or removal SHALL either complete fully or leave the inventory unchanged.

#### Scenario: Add within a stack limit
- **WHEN** an addition fits within the supplied maximum stack quantity
- **THEN** the complete quantity is added

#### Scenario: Exceed a stack limit
- **WHEN** an addition would make an existing stack exceed the supplied maximum
- **THEN** the operation reports a stack-limit failure and preserves the previous quantity

#### Scenario: Remove available quantity
- **WHEN** a removal amount is less than the retained quantity
- **THEN** the complete amount is removed and the slot remains occupied

#### Scenario: Remove too much
- **WHEN** a removal amount exceeds the retained quantity or the item is absent
- **THEN** the operation reports failure and leaves every slot unchanged

### Requirement: Explicit capacity result
The facility SHALL report when adding a new item type would exceed slot capacity and SHALL preserve every retained item and quantity.

#### Scenario: Add a new type to a full inventory
- **WHEN** all slots are occupied and the caller adds an item ID not already present
- **THEN** the operation reports a capacity failure without changing the inventory

#### Scenario: Stack an existing type in a full inventory
- **WHEN** all slots are occupied and the caller adds quantity to an existing item within its stack limit
- **THEN** the quantity increases without requiring another slot

### Requirement: Inventory queries and reset
The facility SHALL let callers retrieve occupied slot count, indexed slot data, and quantity by item ID. Clearing an initialized inventory SHALL remove all retained items and quantities while preserving its storage and capacity.

#### Scenario: Query present and absent items
- **WHEN** a caller queries quantities for present and absent item IDs
- **THEN** the facility returns the retained quantities for present IDs and zero for absent IDs

#### Scenario: Clear retained items
- **WHEN** a caller clears a populated inventory
- **THEN** occupied count and all item queries return zero while the inventory can accept new additions
