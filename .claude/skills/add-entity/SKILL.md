---
name: add-entity
description: Add a new game entity type to InterstellarOutpost. Use when asked to create a new unit, object, or actor in the game world.
argument-hint: "<EntityTypeName>"
context: fork
agent: Plan
---

# Add New Entity: $ARGUMENTS

Add a new entity type called `$ARGUMENTS` to the InterstellarOutpost codebase.

## Steps

1. **Understand existing entities** — Read `InterstellarOutpost/entity_grid.h` and `InterstellarOutpost/entity_grid_cache.h` to understand entity registration and spatial tracking. Browse `InterstellarOutpost/` for an existing entity as a reference (e.g. `unit.h`/`unit.cpp` or `team.h`/`team.cpp`).

2. **Create the entity header** — Create `InterstellarOutpost/$ARGUMENTS.h`:
   - Class inheriting from the appropriate base entity class
   - Declare constructor, destructor, `Update(float dt)`, `Render()`, and any entity-specific methods
   - Include entity grid registration if the entity needs spatial queries

3. **Create the entity implementation** — Create `InterstellarOutpost/$ARGUMENTS.cpp`:
   - Implement all declared methods
   - Register/deregister from `entity_grid` in constructor/destructor
   - Hook into `global_world` or `location` update loop as needed

4. **Register the entity type** — Update `InterstellarOutpost/global_world.h` / `global_world.cpp` to include and instantiate the new type.

5. **Add to Visual Studio project** — Add the new `.h` and `.cpp` to `InterstellarOutpost/InterstellarOutpost.vcxproj` and `InterstellarOutpost/InterstellarOutpost.vcxproj.filters`.

6. **Networking (if applicable)** — If the entity state must be synced, add a network message using the `add-network-message` skill.

## Conventions
- Follow the existing code style in `.clang-format` (no trailing spaces, existing brace style).
- Use `NeuronCore` types for math (`vector3`, `matrix34`, `float_vector3`) rather than rolling new ones.
- Use `NeuronCore/Debug.h` macros for assertions and debug output.
