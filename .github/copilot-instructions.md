# InterstellarOutpost AI Coding Instructions

## Architecture Overview

InterstellarOutpost is a C++23 real-time strategy game targeting DirectX 12 on Windows, built with a modular "Neuron" architecture:

### Core Architecture (Layered Design)
- **NeuronCore** (`libs/NeuronCore/`): Foundation layer - core utilities, shared systems, and PIX instrumentation
- **NeuronClient** (`libs/NeuronClient/`): Presentation layer - DirectX 12 rendering, audio, and input subsystems
- **NeuronServer** (`libs/NeuronServer/`): Server-side logic and networking (dedicated server support)
- **GameLogic** (`libs/GameLogic/`): Game-specific simulation and gameplay entities (depends on NeuronCore)
- **IOClient** (root): Main client executable linking NeuronCore + NeuronClient + GameLogic
- **IOServer** (root): Dedicated server executable linking NeuronCore + NeuronServer + GameLogic

**Dependency Flow**: `GameLogic` → `NeuronCore` ← `NeuronClient`/`NeuronServer` ← Applications

### Build System & Tooling

**CMake Structure** (C++23, MSVC required):
- Uses **CMake Presets** (`CMakePresets.json`) for configuration: `x64-debug`, `x64-release`, `x86-debug`, `x86-release`
- Custom modules in `cmake/`: `CompilerAndLinker.cmake`, `StandardProjectSettings.cmake`, `InstallConfig.cmake`
- Output directories: binaries → `build/bin/{Config}`, libraries → `build/libs/{Config}`
- **vcpkg** for dependencies (baseline pinned in `vcpkg.json`): currently only `winpixevent` for GPU profiling

**Critical Build Commands**:
```powershell
# Configure (first time or after CMakeLists.txt changes)
cmake --preset x64-debug

# Build (use VS Code task or command)
cmake --build build --config Debug
```

**VS Code Integration**:
- Build task defined in `.vscode/tasks.json`: "Build InterstellarOutpost (Debug)"
- Run via `run_task` tool or VS Code UI

### Code Standards & Conventions

**C++ Standards**:
- **C++23** (`CMAKE_CXX_STANDARD 23`) with standard library features
- MSVC-specific optimizations: `/Zc:__cplusplus`, `/Zc:inline`, `/fp:fast`, `/permissive-`
- Precompiled headers (`pch.h`) enabled per-target via `EnablePch()` function

**Formatting** (`.clang-format`):
- **Allman braces** (`BreakBeforeBraces: Allman`)
- **2-space indentation**, no tabs
- **Right-aligned pointers/references** (`int *ptr`, `Type &ref`)
- **Include order**: `pch.h` first (priority 0), then `<system>`, then `"local"`
- No column limit (`ColumnLimit: 0`)

**Linting** (`.clang-tidy`):
- Enabled checks: `clang-analyzer-*`, `bugprone-*`, `performance-*`, `modernize-*`, `cppcoreguidelines-*`
- Disabled: `modernize-use-trailing-return-type`, `readability-magic-numbers`, `hicpp-owning-memory`
- Naming: `camelBack` for variables
- Extra defines: `-D_USE_MATH_DEFINES -DNOMINMAX`

**Project-Specific CMake Helpers** (in `cmake/StandardProjectSettings.cmake`):
- `ApplyTargetDefaults(target)`: Stamps C++23, compiler defines, and switches onto any target
- `EnablePch(target header_path)`: Enables precompiled headers if `ENABLE_PCH=ON`

### Development Workflow

**Adding New Components**:
1. Create library subdirectory under `libs/` (e.g., `libs/MySystem/`)
2. Add `CMakeLists.txt` following pattern from existing libs:
   ```cmake
   file(GLOB sources CONFIGURE_DEPENDS "*.cpp")
   file(GLOB headers CONFIGURE_DEPENDS "*.h")
   add_library(MySystem ${sources} ${headers})
   add_library(InterstellarOutpost::MySystem ALIAS MySystem)
   target_include_directories(MySystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
   ApplyTargetDefaults(MySystem)
   EnablePch(MySystem "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
   ```
3. Add `add_subdirectory("libs/MySystem")` to root `CMakeLists.txt`

**PIX Integration**: NeuronCore links `Microsoft::WinPixEventRuntime` with `HAVE_PIX` define for GPU profiling

### Important Options & Features

**CMake Build Options** (set via `-D<OPTION>=ON`):
- `BUILD_TESTING=OFF`: Enable CTest and `tests/` subdirectory
- `ENABLE_SPECTRE_MITIGATION=OFF`: MSVC `/Qspectre` flag
- `DISABLE_MSVC_ITERATOR_DEBUGGING=OFF`: Disable iterator checks in Debug
- `ENABLE_CODE_ANALYSIS=OFF`: Static analysis
- `ENABLE_CODE_COVERAGE=OFF`: Coverage instrumentation
- `ENABLE_CODE_PROFILING=OFF`: Profiling builds
- `BUILD_FUZZING=OFF`: Fuzz testing mode

**Security Features** (MSVC):
- Control Flow Guard (CFG): `/guard:cf` in Release
- Enhanced CFG (EHCONT): `/guard:ehcont` on x64 with MSVC 19.28+
- SafeSEH for x86, CETCOMPAT for x64, ASLR/DEP via `/DYNAMICBASE /NXCOMPAT`

### File Organization Patterns

- Each library has its own `CMakeLists.txt`, `*.h`, `*.cpp`, and optional `pch.h`
- **No subdirectories** within library folders (flat structure) - use `file(GLOB CONFIGURE_DEPENDS)` pattern
- Applications (`IOClient`, `IOServer`) mirror library structure but produce `WIN32` executables
- Working directory for debugging set to output directory: `VS_DEBUGGER_WORKING_DIRECTORY`

### Testing & Quality

- Tests live in `tests/` (currently empty structure)
- CTest integration when `BUILD_TESTING=ON`
- Clang-tidy runs on headers matching `^(src|libs/(Neuron|GameLogic))/` (note: actual path is `libs/`, not `libs/`)

### Installation & Packaging

- CPack configured for Windows: NSIS installer + ZIP archive
- Components: Applications, Libraries, Headers, Data
- Desktop shortcut creation included
- Install prefix: `CMAKE_INSTALL_BINDIR`, `CMAKE_INSTALL_LIBDIR`, `CMAKE_INSTALL_DATADIR`

### Common Pitfalls

2. **Missing namespace alias**: Always use `InterstellarOutpost::ComponentName` alias pattern for internal linking
3. **Precompiled header requirement**: All libraries expect `pch.h` to exist (create empty if none needed)
4. **vcpkg integration**: Ensure `CMAKE_TOOLCHAIN_FILE` points to vcpkg toolchain (hardcoded in root CMakeLists.txt)

