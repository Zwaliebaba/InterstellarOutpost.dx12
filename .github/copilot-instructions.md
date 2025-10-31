# InterstellarOutpost.dx12 – AI agent working notes

These instructions make AI coding agents productive in this Windows/MSVC C++23 game project. They distill the repo’s architecture, workflows, and conventions; stick to these patterns and file locations.

## Context
Project Type: Game
Project Name: Interstellar Outpost 
Language: C++
Framework / Libraries: STL / CMake / CTest
Architecture: Modular / RAII / OOP

## General guidelines
- Code Style: The project uses an .editorconfig file to enforce coding standards. Follow the rules defined in .editorconfig for indentation, line endings, and other formatting. Additional information can be found on the wiki at Implementation. The code requires C++23 features.
- Error Handling: Use C++ exceptions for error handling and uses RAII smart pointers to ensure resources are properly managed. For some functions that return HRESULT error codes, they are marked noexcept, use std::nothrow for memory allocation, and should not throw exceptions.

## Patterns

### Patterns to Follow
Use RAII for all resource ownership (memory, file handles, etc.).
Many classes utilize the pImpl idiom to hide implementation details, and to enable optimized memory alignment in the implementation.
Use std::unique_ptr for exclusive ownership and std::shared_ptr for shared ownership.
Use winrt::com_ptr for COM object management.
Make use of anonymous namespaces to limit scope of functions and variables.
Make use of DEBUG_ASSERT for debugging checks, but be sure to validate input parameters in release builds.
Make use of the DebugTrace helper to log diagnostic messages, particularly at the point of throwing an exception.

### Patterns to Avoid

- Don’t use raw pointers for ownership.
- Avoid macros for constants—prefer `constexpr` or `inline` `const`.
- Don’t put implementation logic in header files unless using templates.
- Avoid using `using namespace` in header files to prevent polluting the global namespace.

## No speculation

When creating documentation:

### Document Only What Exists

- Only document features, patterns, and decisions that are explicitly present in the source code.
- Only include configurations and requirements that are clearly specified.
- Do not make assumptions about implementation details.

### Handle Missing Information

- Ask the user questions to gather missing information.
- Document gaps in current implementation or specifications.
- List open questions that need to be addressed.

### Source Material

- Always cite the specific source file and line numbers for documented features.
- Link directly to relevant source code when possible.
- Indicate when information comes from requirements vs. implementation.

### Verification Process

- Review each documented item against source code whenever related to the task.
- Remove any speculative content.
- Ensure all documentation is verifiable against the current state of the codebase.

## Data and assets
- `gamedata/` is auto‑mirrored to `bin/<Config>/gamedata` by a root custom target (`copy_gamedata`). If assets are “missing,” check that mirror path exists.

## CMake patterns to follow
- Link only via alias targets. Example from `src/CMakeLists.txt`:
	- `target_link_libraries(InterstellarOutpost PRIVATE InterstellarOutpost::NeuronCore InterstellarOutpost::NeuronClient InterstellarOutpost::GameLogic)`
- Tests are simple executables registered with CTest (no external framework required). Use the helper in `tests/CMakeLists.txt`:
	- `add_project_test(test_name SOURCES test_xyz.cpp LIBRARIES InterstellarOutpost::GameLogic ... TIMEOUT 30)`
	- Working dir is forced to the test’s target output folder so `gamedata` resolves.

## Source layout highlights
- App: `src/` (entry in `main.cpp`, app wiring, rendering, input, state). Key files: `GameApp.cpp/.h`, `Renderer.*`, `camera.*`, `user_input.*`, `taskmanager*`, `location*`, `global_world*`.
- Engine libs: `libs/NeuronCore` (core utilities, JSON, PIX), `libs/NeuronClient` (client subsystems), `libs/GameLogic` (entities, world rules). Each has its own `CMakeLists.txt` and exposes a namespaced alias.

### Pointers to read first
- `README.md` (quickstart + architecture), `CMakePresets.json` (presets), root `CMakeLists.txt` (enforced policies and `copy_gamedata`).
- `src/main.cpp` for the game loops and timing; `tests/CMakeLists.txt` for adding tests; `libs/*/CMakeLists.txt` for alias/visibility patterns.

## Code Review Instructions

When reviewing code, focus on the following aspects:

- Adherence to coding standards defined in `.editorconfig`.
- Make coding recommendations based on the *C++ Core Guidelines*.
- Proper use of RAII and smart pointers.
- Correct error handling practices and C++ Exception safety.
- Clarity and maintainability of the code.
- Adequate comments where necessary.
- Compliance with the project's architecture and design patterns.
- Ensure that all public functions and classes are covered by unit tests located on tests\ where applicable. Report any gaps in test coverage.
- Check for performance implications, especially in geometry processing algorithms.
- Provide brutally honest feedback on code quality, design, and potential improvements as needed.