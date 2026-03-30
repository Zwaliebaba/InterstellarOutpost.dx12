---
name: add-game-mode
description: Scaffold a new multiplayer game mode. Use when adding a new mode to the MMO (e.g. a new match type, objective mode, or PvP ruleset).
argument-hint: "<GameModeName>"
context: fork
agent: Plan
---

# Add New Game Mode: $ARGUMENTS

Scaffold a new multiplayer game mode called `$ARGUMENTS`.

## Steps

1. **Study existing modes** — Read `InterstellarOutpost/multiwinia.h` / `multiwinia.cpp` as the primary reference for how game modes are structured. Read `InterstellarOutpost/location.h` / `location.cpp` to understand how a location hosts a mode. Also review `GameLogic/GameMenuWindow.h` and surrounding files to see how modes are presented in the UI.

2. **Define the mode constant** — Add a new enum value or `#define` for `$ARGUMENTS` alongside existing mode constants in `multiwinia.h` (or wherever existing modes are enumerated).

3. **Implement the mode class**:
   - Create `InterstellarOutpost/$ARGUMENTS.h` and `InterstellarOutpost/$ARGUMENTS.cpp`
   - Inherit from (or follow the pattern of) the existing mode base
   - Implement `Update(float dt)`, `Render()`, `IsComplete()`, and win-condition logic
   - Handle team scoring via `InterstellarOutpost/team.h`

4. **Register in multiwinia dispatcher** — In `multiwinia.cpp`, add the new mode to the factory/switch that instantiates modes by type.

5. **Add UI entries**:
   - Add a button or entry in `GameLogic/GameMenuWindow.cpp` so the mode is selectable
   - Add any required display strings to `NeuronClient/Strings.h` / `Strings.cpp` or the language table in `NeuronClient/language_table.h`

6. **Map/level support** — If the mode requires specific level metadata, update `InterstellarOutpost/level_file.h` / `level_file.cpp` and `InterstellarOutpost/mapfile.h`.

7. **Network messages** — Add any mode-specific sync messages using the `add-network-message` skill.

8. **Add to project** — Add new `.h`/`.cpp` files to `InterstellarOutpost/InterstellarOutpost.vcxproj` and `.vcxproj.filters`.

## Design Guidelines
- All win/loss conditions must be evaluated server-side and broadcast to clients.
- Use `InterstellarOutpost/gametimer.cpp` for mode timers rather than raw frame counts.
- New modes should work with the existing spectator system (`GameLogic/CSpectatorButton.h`).
