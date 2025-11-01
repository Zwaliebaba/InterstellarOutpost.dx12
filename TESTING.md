# Testing Guide for InterstellarOutpost.dx12

This document describes the testing strategy, infrastructure, and workflows for the InterstellarOutpost game project.

## Table of Contents

- [Overview](#overview)
- [Test Organization](#test-organization)
- [Building Tests](#building-tests)
- [Running Tests](#running-tests)
- [Writing Tests](#writing-tests)
- [CI Integration](#ci-integration)
- [Testability Guidelines](#testability-guidelines)

## Overview

InterstellarOutpost uses a **lightweight custom test framework** integrated with CTest for automated testing. The testing strategy separates fast unit tests from slower integration tests to optimize development workflows and CI efficiency.

### Test Framework

- **Test Harness**: Custom lightweight framework (`tests/common/TestHarness.h`)
- **Test Runner**: CTest (part of CMake)
- **Build Integration**: CMake targets with automatic working directory configuration
- **Asset Handling**: Tests run from `bin/<Config>/` where `gamedata/` is auto-mirrored

### Test Categories

| Category | Description | Speed | Run Frequency |
|----------|-------------|-------|---------------|
| **Unit** | Isolated tests of individual classes/functions | < 30s each | Every commit/PR |
| **Integration** | Multi-component tests with external resources | < 3m each | Nightly/manual |

## Test Organization

```
tests/
├── common/
│   ├── CMakeLists.txt
│   └── TestHarness.h          # Lightweight test framework
├── unit/
│   ├── CMakeLists.txt          # Unit test definitions
│   ├── neuron_core/            # NeuronCore library tests
│   ├── neuron_client/          # NeuronClient library tests
│   ├── gamelogic/              # GameLogic library tests
│   └── src/                    # Main app component tests
└── integration/
    ├── CMakeLists.txt          # Integration test definitions
    ├── test_renderer_init.cpp
    ├── test_asset_loading.cpp
    ├── test_location_loading.cpp
    ├── test_network_handshake.cpp
    └── test_simulation_tick.cpp
```

### Test Labels

Tests are tagged with CTest labels for selective execution:

- **unit**: Fast unit tests (run on every PR)
- **integration**: Slower integration tests (run on schedule)
- **neuroncore**, **neuronclient**, **gamelogic**: Module-specific labels
- **renderer**, **network**, **simulation**: Feature-specific labels

## Building Tests

### Prerequisites

- CMake 3.21+
- Visual Studio 2022 (MSVC)
- vcpkg (for dependencies: nlohmann_json, winpixevent)
- Windows SDK with DirectX 12

### Configuration

Tests are controlled by the `IO_BUILD_TESTS` CMake option (ON by default in debug builds):

```powershell
# Configure with tests enabled
cmake --preset windows-debug

# Or explicitly enable tests
cmake -B build -DIO_BUILD_TESTS=ON

# Disable tests (e.g., for release builds)
cmake -B build -DIO_BUILD_TESTS=OFF
```

### Build Commands

```powershell
# Build all tests
cmake --build --preset windows-debug

# Build specific test target
cmake --build --preset windows-debug --target unit_camera_basics
cmake --build --preset windows-debug --target integration_renderer_init

# Build only tests (skip main application)
cmake --build build --target <test_name>
```

## Running Tests

### Quick Start

```powershell
# Configure and build
cmake --preset windows-debug
cmake --build --preset windows-debug

# Run all tests
ctest --preset windows-debug

# Or from build directory
cd out\build\windows-debug
ctest --output-on-failure
```

### Selective Test Execution

```powershell
# Run only unit tests (fast)
ctest --preset windows-debug -L unit

# Run only integration tests
ctest --preset windows-debug -L integration

# Run tests for specific module
ctest --preset windows-debug -L neuroncore
ctest --preset windows-debug -L gamelogic

# Run specific test by name
ctest --preset windows-debug -R unit_camera_basics

# Run with verbose output
ctest --preset windows-debug --verbose

# Run with detailed failure output
ctest --preset windows-debug --output-on-failure

# Run tests in parallel (4 jobs)
ctest --preset windows-debug -j 4
```

### Test Output

Tests use a simple format:

```
[ RUN      ] TestSuiteName
[       OK ] TestSuiteName
```

Or on failure:

```
[ RUN      ] TestSuiteName
[  FAILED  ] TestSuiteName: condition (...location...)
[  FAILED  ] TestSuiteName with 2 failure(s).
```

### Common Test Scenarios

```powershell
# Quick smoke test (unit tests only)
ctest --preset windows-debug -L unit --output-on-failure

# Full test suite before committing
ctest --preset windows-debug --output-on-failure

# Debug specific failing test
ctest --preset windows-debug -R failing_test_name --verbose

# Integration tests for nightly build
ctest --preset windows-debug -L integration --timeout 300
```

## Writing Tests

### Test Harness API

Include the test harness header:

```cpp
#include "TestHarness.h"
```

Basic test structure:

```cpp
int main()
{
  io::tests::TestContext ctx("MySuite::TestName");

  // Test assertions
  IO_EXPECT_TRUE(ctx, someCondition);
  IO_EXPECT_NEAR(ctx, actualValue, expectedValue, epsilon);

  return ctx.Finalize();
}
```

### Adding a New Unit Test

1. **Create test source file** in appropriate directory:
   ```
   tests/unit/neuron_core/test_my_feature.cpp
   ```

2. **Write test using TestHarness**:
   ```cpp
   #include "TestHarness.h"
   #include "MyFeature.h"

   int main()
   {
     io::tests::TestContext ctx("MyFeature::BasicTest");

     MyFeature feature;
     IO_EXPECT_TRUE(ctx, feature.IsValid());

     return ctx.Finalize();
   }
   ```

3. **Register test in CMakeLists.txt**:
   ```cmake
   add_project_test(unit_my_feature
     SOURCES
       neuron_core/test_my_feature.cpp
     LIBRARIES
       InterstellarOutpost::NeuronCore
     LABELS unit neuroncore
     TIMEOUT ${TEST_TIMEOUT_SHORT}
   )
   ```

4. **Build and run**:
   ```powershell
   cmake --build --preset windows-debug --target unit_my_feature
   ctest --preset windows-debug -R unit_my_feature --verbose
   ```

### Adding a New Integration Test

Integration tests follow the same pattern but:

- Located in `tests/integration/`
- Use `LABELS integration <feature>`
- May have longer timeout: `TIMEOUT ${TEST_TIMEOUT_LONG}` or `${TEST_TIMEOUT_INTEGRATION}`
- Can access `gamedata/` assets (working directory is set automatically)

Example:

```cmake
add_project_test(integration_my_system
  SOURCES
    test_my_system.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS integration mysystem
  TIMEOUT ${TEST_TIMEOUT_INTEGRATION}
)
```

### Test Timeouts

Defined in `tests/CMakeLists.txt`:

- `TEST_TIMEOUT_SHORT`: 30 seconds (unit tests)
- `TEST_TIMEOUT_LONG`: 120 seconds (integration tests)
- `TEST_TIMEOUT_INTEGRATION`: 180 seconds (heavy integration tests)

## CI Integration

### GitHub Actions Workflow

Tests run automatically on push/PR via `.github/workflows/ci.yml`:

- **Unit tests**: Run on every push/PR (fast feedback)
- **Integration tests**: Run nightly or on manual trigger (avoid CI bottleneck)

### CI Commands

```yaml
# Configure
cmake --preset windows-debug -DIO_BUILD_TESTS=ON

# Build
cmake --build --preset windows-debug

# Run unit tests only (fast, every PR)
ctest --preset windows-debug -L unit --output-on-failure

# Run integration tests (nightly)
ctest --preset windows-debug -L integration --output-on-failure --timeout 300
```

### CI Environment Requirements

- Windows Server 2022 runner
- Visual Studio 2022
- vcpkg installed and configured
- DirectX 12 capable GPU (for renderer integration tests)

### Handling GPU-Dependent Tests

Integration tests that require GPU (e.g., `integration_renderer_init`) may fail in CI without proper hardware. Options:

1. **Skip in CI**: Use CTest's `DISABLED` property or label filtering
2. **Mock Backend**: Implement `NullRendererBackend` for headless testing
3. **GPU Runner**: Use self-hosted runner with GPU

## Testability Guidelines

### Current Testability Challenges

The codebase has some architectural patterns that make testing difficult. Here are the main issues and recommended refactorings:

#### 1. **Global State and Singletons**

❌ **Problem**:
```cpp
extern Location* g_location;
extern Renderer* g_renderer;
```

✅ **Solution**: Dependency injection
```cpp
class Location {
  Location(Renderer* renderer, SoundSystem* sound);
  // Pass dependencies explicitly
};
```

#### 2. **Tight Hardware Coupling**

❌ **Problem**: Constructor creates D3D12 device directly
```cpp
Renderer::Renderer() {
  D3D12CreateDevice(...);  // Can't test without GPU
}
```

✅ **Solution**: Abstract backend interface
```cpp
class IRendererBackend {
  virtual bool Initialize() = 0;
};

class Renderer {
  Renderer(IRendererBackend* backend);  // Inject backend
};

// Production: D3D12Backend
// Testing: NullBackend or MockBackend
```

#### 3. **File I/O in Constructors**

❌ **Problem**: Loading in constructor prevents testing
```cpp
AssetManager::AssetManager() {
  LoadTexture("gamedata/foo.dds");  // Requires filesystem
}
```

✅ **Solution**: Two-phase initialization or file system abstraction
```cpp
class IFileSystem {
  virtual std::vector<byte> ReadFile(path) = 0;
};

class AssetManager {
  AssetManager(IFileSystem* fs);  // Inject filesystem
  void LoadAssets();  // Separate loading phase
};

// Production: DiskFileSystem
// Testing: MemoryFileSystem
```

#### 4. **Mixed Responsibilities**

❌ **Problem**: Classes do too much
```cpp
class Location {
  void Render();      // Rendering
  void Update();      // Simulation
  void HandleInput(); // Input
  // All in one class!
};
```

✅ **Solution**: Separate concerns
```cpp
class LocationData {
  // Pure data, no rendering
};

class LocationRenderer {
  void Render(const LocationData& data);
};

class LocationSimulation {
  void Update(LocationData& data, float dt);
};
```

### Writing Testable Code

**Checklist for new code**:

- [ ] Use dependency injection (pass dependencies to constructor)
- [ ] Avoid global variables and singletons
- [ ] Separate data structures from rendering/I/O
- [ ] Use interfaces for external dependencies (GPU, file system, network)
- [ ] Two-phase initialization (construct, then initialize)
- [ ] Make RAII classes with clear ownership
- [ ] Avoid side effects in constructors
- [ ] Use const correctness to indicate read-only operations

### Mocking and Test Doubles

For integration tests, consider:

- **Mock Renderer**: Headless backend for testing without GPU
- **Mock File System**: In-memory files for testing without disk I/O
- **Mock Network**: In-process client/server for deterministic testing
- **Mock Time**: Controlled time source for deterministic simulation

## Troubleshooting

### Tests Can't Find gamedata/

**Symptom**: Test fails with "gamedata/ not found"

**Solution**: Tests must run from `bin/<Config>/` where `gamedata/` is mirrored. CTest automatically sets the working directory. If running manually:

```powershell
cd out\build\windows-debug\bin\Debug
.\unit_asset_loading.exe
```

### GPU-Dependent Tests Fail

**Symptom**: `integration_renderer_init` fails with D3D12 error

**Solution**: 
- Ensure GPU drivers are up to date
- Run on system with DirectX 12 capable hardware
- Consider implementing `NullRendererBackend` for CI

### Test Timeout

**Symptom**: Test killed after timeout

**Solution**: 
- Optimize test (remove long-running operations)
- Increase timeout in CMakeLists.txt
- Check for infinite loops or deadlocks

### vcpkg Dependencies Missing

**Symptom**: Test won't compile, missing headers

**Solution**:
```powershell
# Install dependencies
vcpkg install nlohmann-json winpixevent

# Reconfigure with vcpkg toolchain
cmake --preset windows-debug
```

## References

- [CMake CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Google Test Primer](https://google.github.io/googletest/primer.html) (concepts applicable)
- [Test-Driven Development](https://en.wikipedia.org/wiki/Test-driven_development)

---

## Quick Reference

```powershell
# Configure with tests
cmake --preset windows-debug

# Build all
cmake --build --preset windows-debug

# Run unit tests only (fast)
ctest --preset windows-debug -L unit --output-on-failure

# Run all tests
ctest --preset windows-debug --output-on-failure

# Run specific test
ctest --preset windows-debug -R unit_camera_basics --verbose

# Run integration tests
ctest --preset windows-debug -L integration --output-on-failure
```

---

**Questions or issues?** Check the [main README](README.md) or open an issue on GitHub.
