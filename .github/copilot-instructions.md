# InterstellarOutpost C++20 Project

This is a C++20 project with the following structure:
- Main program: InterstellarOutpost
- Custom libraries: Neuron and GameLogic
- Build system: CMake with Visual Studio C++ compiler support
- Testing: CTest framework

## Development Guidelines
- Use C++20 standard features
- Follow modern C++ best practices
- Libraries should be modular and well-documented
- Use CMake for cross-platform builds
- Write unit tests for all components

## Project Structure
- `/src` - Main application source code
- `/libs/Neuron` - Neuron library source and headers
- `/libs/GameLogic` - GameLogic library source and headers
- `/tests` - Unit tests using CTest
- `/build` - Build output directory (not tracked)

## Build Instructions
1. Create build directory: `mkdir build && cd build`
2. Configure: `cmake ..`
3. Build: `cmake --build .`
4. Run tests: `ctest`