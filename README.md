<div align="center">

# InterstellarOutpost.dx12

A C++23 real‑time strategy game prototype using a modular engine, Windows/MSVC toolchain, and CMake presets.

</div>

## Overview

InterstellarOutpost is a Windows‑only game project structured as a main application plus two engine libraries. It features a deterministic client/server simulation, a time‑sliced game loop, and a content pipeline that mirrors runtime assets into the build output automatically.

Key components:
- Main app (`src/`): rendering, input, game state, and the game loops (see `src/main.cpp`)
- Engine core (`libs/NeuronCore/`): shared systems and utilities (JSON, PIX markers)
- Client systems (`libs/NeuronClient/`): client‑side subsystems used by the app
- Game logic (`libs/GameLogic/`): entities, world, and gameplay rules

> [!NOTE]
> The runtime expects the working directory to be the configuration output folder (for example `.../out/build/windows-debug/bin/Debug/`). The build auto‑copies `gamedata/` there so assets resolve at runtime and in tests.

## Features

- C++23 targets with precompiled headers per target
- Deterministic client/server with rollback, 10Hz server tick, client prediction
- Time‑sliced simulation (up to 10 slices per frame) with built‑in profiler macros
- Modular libraries with clear alias targets: `InterstellarOutpost::NeuronCore`, `::NeuronClient`, `::GameLogic`
- Content mirroring: `gamedata/` auto‑copied to `bin/<Config>/gamedata`
- CTest integration with per‑target working directories

## Quickstart (Windows + Visual Studio 2022)

Use the provided CMake presets. x64 and ARM64 are supported.

```powershell
# Configure
cmake --preset windows-debug

# Build
cmake --build --preset windows-debug

# Run tests (Debug)
ctest --preset windows-debug
```

Run the game from the build output:

```powershell
# Debug config uses a postfix
./out/build/windows-debug/bin/Debug/InterstellarOutpost_d.exe

# Release config (no postfix)
./out/build/windows-release/bin/Release/InterstellarOutpost.exe
```

> [!TIP]
> If you prefer ARM64 on Windows, use the `windows-arm64-*` presets.

> [!WARNING]
> Presets reference a vcpkg toolchain file. If your local vcpkg path differs, edit `CMakePresets.json` or set the appropriate cache variables to point to your vcpkg installation.

## Architecture

- Game loop (`src/main.cpp`):
	- LocationGameLoop(): real‑time play with client/server sync, physics slices, rendering
	- GlobalWorldGameLoop(): world map/meta progression and menus
	- Built‑ins: profiler (`START_PROFILE/END_PROFILE`), input routing, UI windows
- Networking: deterministic client/server at 10Hz (`Server`, `NetworkClient`), sync validation via `GenerateSyncValue()`
- Rendering and camera: `Renderer`, `camera`, post‑effects and fade in/out handling
- Input system: layered drivers (Win32, XInput, aliases, chords, prefs) via `InputManager`
- Audio: 2D/3D libraries with ambient/event triggers through `SoundSystem`

> [!NOTE]
> Engine libraries propagate PIX GPU event marker support via vcpkg’s `winpixevent` and define `HAVE_PIX` for conditional compilation.

## Project layout

```
src/                 # Main application (game loops, rendering, input, state)
libs/
	NeuronCore/        # Core engine utilities (JSON, PIX, shared headers)
	NeuronClient/      # Client subsystems used by the app
	GameLogic/         # Entities, world objects, gameplay systems
gamedata/            # Assets and config; auto‑copied to bin/<Config>/gamedata
tests/               # CTest targets (unit + integration)
cmake/               # Shared CMake modules and install config
```

## Development

- Build options: control tests with `IO_BUILD_TESTS` (ON by default), toggle PCH via `IO_ENABLE_PCH`, and opt into warnings-as-errors with `IO_WARNINGS_AS_ERRORS`
- Targets use alias names; link against `InterstellarOutpost::NeuronCore`, `::NeuronClient`, `::GameLogic`
- Working directory is set per‑config to `bin/<Config>` for debugging in VS
- JSON via `nlohmann_json` (vcpkg); PIX via `winpixevent` (vcpkg)

> [!IMPORTANT]
> Always run from the configuration folder (`bin/Debug` or `bin/Release`) so the game can find `gamedata/`. Tests are also registered to run from their own output dirs and assume assets are present there.

## Testing

All tests are CTest‑driven with per‑target working directories. Typical flow:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Named targets include: `test_neuron_core`, `test_neuron_client`, `test_gamelogic`, and `test_integration`.

## Troubleshooting

- vcpkg toolchain not found: edit `CMakePresets.json` `toolchainFile`/`CMAKE_TOOLCHAIN_FILE` to your vcpkg path
- Missing assets at runtime: verify `gamedata/` exists under `out/build/<preset>/bin/<Config>/gamedata`
- Running from IDE: ensure the debugger working directory is `bin/<Config>` (set automatically per target)

---

If you’d like a quick tour, start at `src/main.cpp` for the loop, then browse `libs/GameLogic/` for entities and `libs/NeuronCore/` for engine utilities.

