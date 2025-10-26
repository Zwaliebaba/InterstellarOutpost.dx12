# InterstellarOutpost

A C++20 interstellar outpost game

## Overview

InterstellarOutpost is a demonstration project showcasing modern C++20 features, modular architecture, and comprehensive testing. The system consists of:

- **InterstellarOutpost**: Main application coordinating outpost operations
- **Neuron Library**: Neural network processing system for advanced data analysis
- **GameLogic Library**: Resource management and game state processing

## Features

- **C++20 Standard**: Leverages modern C++ features including concepts, ranges, and improved template metaprogramming
- **Modular Architecture**: Clean separation between neural processing and game logic
- **CMake Build System**: Cross-platform build configuration with Visual Studio support
- **Comprehensive Testing**: Unit tests and integration tests using CTest framework
- **Resource Management**: Energy, minerals, food, and population tracking
- **Neural Processing**: Data processing simulation with configurable parameters

## Project Structure

```
InterstellarOutpost/
├── src/                     # Main application
│   ├── main.cpp
│   └── CMakeLists.txt
├── libs/                    # Custom libraries
│   ├── Neuron/             # Neural processing library
│   │   ├── include/Neuron/
│   │   │   └── Neuron.h
│   │   ├── src/
│   │   │   └── Neuron.cpp
│   │   └── CMakeLists.txt
│   └── GameLogic/          # Game logic library
│       ├── include/GameLogic/
│       │   └── GameLogic.h
│       ├── src/
│       │   └── GameLogic.cpp
│       └── CMakeLists.txt
├── tests/                   # Test suite
│   ├── test_neuron.cpp
│   ├── test_gamelogic.cpp
│   ├── test_integration.cpp
│   └── CMakeLists.txt
├── build/                   # Build output (generated)
├── .github/
│   └── copilot-instructions.md
├── CMakeLists.txt          # Root CMake configuration
└── README.md
```

## Prerequisites

- **CMake**: Version 3.20 or higher
- **C++ Compiler**: 
  - Visual Studio 2019/2022 with C++20 support (Windows)
  - GCC 10+ or Clang 11+ (Linux/macOS)
- **Operating System**: Windows 10/11, Linux, or macOS

## Building the Project

### Windows (Visual Studio)

1. **Clone and navigate to the project**:
   ```powershell
   cd path/to/InterstellarOutpost
   ```

2. **Create build directory**:
   ```powershell
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```powershell
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

4. **Build the project**:
   ```powershell
   cmake --build . --config Debug
   ```

### Linux/macOS

1. **Create build directory**:
   ```bash
   mkdir build && cd build
   ```

2. **Configure and build**:
   ```bash
   cmake ..
   cmake --build .
   ```

## Running the Application

After building, run the main executable:

### Windows
```powershell
.\build\bin\Debug\InterstellarOutpost.exe
```

### Linux/macOS
```bash
./build/bin/InterstellarOutpost
```

## Running Tests

Execute the test suite using CTest:

```bash
cd build
ctest --verbose
```

Individual tests can be run directly:
- `./tests/test_neuron` - Neuron library tests
- `./tests/test_gamelogic` - GameLogic library tests  
- `./tests/test_integration` - Integration tests

## Library Documentation

### Neuron Library

The Neuron library provides neural network processing capabilities:

```cpp
#include <Neuron/Neuron.h>

Neuron::NeuronSystem neuronSystem;
neuronSystem.initialize();

std::string result = neuronSystem.processData("input_data");
std::cout << result << std::endl;

neuronSystem.shutdown();
```

### GameLogic Library

The GameLogic library manages outpost resources and game state:

```cpp
#include <GameLogic/GameLogic.h>

GameLogic::GameSystem gameSystem;
gameSystem.initialize();

// Add resources
gameSystem.addResource(GameLogic::ResourceType::Energy, 50);

// Check resource levels
int energy = gameSystem.getResourceAmount(GameLogic::ResourceType::Energy);

// Update game state
gameSystem.update(1.0f); // 1 second delta time

gameSystem.shutdown();
```

## Development Guidelines

- **Code Style**: Follow modern C++20 best practices
- **Memory Management**: Use RAII and smart pointers
- **Error Handling**: Prefer exceptions for error conditions
- **Testing**: Write tests for all new functionality
- **Documentation**: Comment public APIs and complex algorithms

## Contributing

1. Ensure code compiles without warnings
2. Add appropriate tests for new features
3. Update documentation as needed
4. Follow the existing code style

## License

This project is created for educational and demonstration purposes.

## Version History

- **v1.0.0**: Initial release with basic neural processing and resource management

---

**🚀 Welcome to your InterstellarOutpost!**