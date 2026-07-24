# High-Performance C++20 Minecraft Server

Un'implementazione custom e multithreaded di un server Minecraft Java Edition scritta in C++20 per Windows (Winsock2).

## Prerequisiti

* **OS:** Windows 11 / 10
* **Compiler:** MSVC (Visual Studio 2022 v17.0+) o GCC/Clang con supporto C++20
* **Build System:** CMake 3.20+

## Compilazione ed Esecuzione

```bash
# Generate build files
cmake -B build -S .

# Build executable
cmake --build build --config Release

# Run Unit Tests
ctest --test-dir build -C Release --output-on-failure

# Start Server
./build/Release/mc_server.exe