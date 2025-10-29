# InterstellarOutpost AI Coding Instructions

## Architecture Overview

InterstellarOutpost is a C++20/23 real-time strategy game built with a modular architecture:

- **Main Application (`src/`)**: Game loop, rendering, input handling, and state management
- **Neuron Library (`libs/Neuron/`)**: Core engine systems - graphics, input, sound, networking
- **GameLogic Library (`libs/GameLogic/`)**: Game-specific logic, entities, and world management

Key architectural patterns:
- Entity-Component-System design with `Entity`, `Unit`, `Team`, and `WorldObject` hierarchies
- Client-server networking with `ClientToServer` and `Server` classes
- Time-sliced simulation: 10 slices per frame at 10Hz server tick rate (`globals.h`)
- Modular subsystems accessed via `g_app` singleton with subsystem managers

## Essential Build & Development Workflow

### CMake Build System
- Uses **CMake Presets** - always use presets, not manual cmake commands:
  ```powershell
  cmake --preset windows-debug
  cmake --build --preset windows-debug
  ```
- Build configurations: `windows-debug`, `windows-release`, `windows-relwithdebinfo`
- **Critical**: `gamedata/` directory auto-copies to build output - required for runtime

### Testing Strategy
- Unit tests per library: `test_neuron.cpp`, `test_gamelogic.cpp`  
- Integration test: `test_integration.cpp`
- Run via: `ctest --preset windows-debug` or VS Code task "Build InterstellarOutpost (Debug)"
- Tests expect working directory to be `build/bin/[Config]/`

### Project-Specific Conventions

#### Memory Management
- Uses **shared ownership** patterns - many objects stored in global managers
- Raw pointers are common and expected (legacy codebase pattern)
- RAII for resources, but manual cleanup in destructors is normal

#### Code Organization
- **Precompiled Headers**: `src/pch.h` included in all compilation units
- **Cross-library dependencies**: Neuron and GameLogic both access `src/` headers
- **Global Access**: `g_app`, `g_inputManager`, `g_prefsManager` - use these, don't create new globals

#### Naming Patterns
- Classes: `PascalCase` (e.g., `LocationInput`, `TaskManager`)
- Members: `m_camelCase` (e.g., `m_locationId`, `m_userInput`)
- Globals: `g_camelCase` (e.g., `g_app`, `g_gameTime`)
- File organization: `.h/.cpp` pairs, headers contain full class definitions

## Core Game Loop Architecture

The main loop in `main.cpp` follows this pattern:
1. **LocationGameLoop()**: Real-time gameplay with client-server sync
2. **GlobalWorldGameLoop()**: World map/meta-game navigation  
3. **MainMenuLoop()**: Frontend menus

Key timing variables in `main.cpp`:
- `g_gameTime`: Current simulation time
- `g_advanceTime`: Delta time for current frame
- `g_predictionTime`: Client prediction offset
- Time-sliced physics: advance 1-10 slices per frame based on network timing

## Critical Integration Points

### Input System (`src/user_input.cpp`, Neuron library)
- Multi-modal: keyboard/mouse + gamepad support
- Input preprocessing through driver chain in `InputManager`
- UI state affects input routing (windows open = block game input)

### Rendering Pipeline (`src/renderer.cpp`)
- Immediate-mode OpenGL rendering
- Camera system with multiple modes: `ModeFreeMovement`, `ModeSphereWorld`, `ModeMainMenu`
- Effect processing via `EffectProcessor` for post-processing

### Audio System (Neuron/Sound)
- 2D + 3D audio libraries (`g_soundLibrary2d`, `g_soundLibrary3d`)
- Event-driven audio triggers via `SoundSystem::TriggerOtherEvent()`
- Ambient audio tied to game state transitions

### Networking (Neuron/Network)
- Deterministic client-server with rollback (`ClientToServer`, `Server`)
- Fixed-timestep server at 10Hz with client prediction
- Sync validation via `GenerateSyncValue()` function

## Development Guidelines

### Adding New Features
1. **Entities**: Inherit from `Entity` or `WorldObject`, register in appropriate manager
2. **UI**: Use ECL window system (see `interface/` directories)
3. **Game Logic**: Add to location-based or global world systems
4. **Config**: Use `PrefsManager` for settings, store in `gamedata/default_preferences.txt`

### Debugging Patterns
- Use `DebugTrace()` for logging (not `printf` or `std::cout`)
- Camera debug modes available: `Camera::DebugModeAuto`, `DebugModeAlways`
- Profile with built-in profiler: `START_PROFILE()` / `END_PROFILE()` macros

### Visual Studio Integration
- Project uses folder structure for IDE organization
- Debugging working directory auto-configured to `build/bin/[Config]/`
- IntelliSense benefits from `CMAKE_EXPORT_COMPILE_COMMANDS=ON`

## Data Directories

- `gamedata/`: Runtime assets, scripts, preferences - **must be present in build output**
- `gamedata/shaders/`: GLSL shaders
- `gamedata/scripts/`: Game scripts and cutscenes  
- `gamedata/levels/`: Level definitions and mission files

When adding new content, ensure it's copied to build directory or add to `src/CMakeLists.txt` post-build commands.