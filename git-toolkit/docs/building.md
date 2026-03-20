# Building git-toolkit

## Prerequisites

| Tool | Minimum Version | Notes |
| ---- | --------------- | ----- |
| CMake | 3.20+ | Build system generator |
| Ninja | 1.10+ | Build tool (recommended over Make) |
| Clang | 16+ | C++20 with `std::format` support required |
| Git | 2.30+ | Required at runtime for git operations |

### Windows (Git Bash / MSYS2)

```bash
# Install via scoop (recommended)
scoop install cmake ninja llvm

# Or via winget
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install LLVM.LLVM
```

### macOS

```bash
brew install cmake ninja llvm
```

### Linux (Ubuntu/Debian)

```bash
sudo apt install cmake ninja-build clang-16 libsecret-1-dev
```

## Build

### Configure

```bash
cd git-toolkit

# Windows (Git Bash)
cmake -G Ninja \
  -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang++.exe" \
  -DCMAKE_RC_COMPILER="C:/Program Files/LLVM/bin/llvm-rc.exe" \
  -DCMAKE_MAKE_PROGRAM="$(which ninja)" \
  -B build

# macOS / Linux
cmake -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -B build
```

### Build

```bash
cmake --build build
```

### Run tests

```bash
cd build && ctest --output-on-failure
```

### Clean

```bash
rm -rf build
```

## Dependencies

Dependencies are fetched automatically via CMake FetchContent during configure:

| Dependency | Version | Purpose |
| ---------- | ------- | ------- |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | JSON parsing |
| [Catch2](https://github.com/catchorg/Catch2) | 3.7.1 | Unit testing |

No manual dependency installation required.

## Project structure

```text
git-toolkit/
  CMakeLists.txt        # Top-level: configures core + cli + gui
  core/
    CMakeLists.txt      # Core library (libgittoolkit)
    include/gittoolkit/ # Public headers
    src/                # Implementation
    tests/              # Unit tests
  cli/
    CMakeLists.txt      # CLI executable
    main.cpp
  gui/
    windows/            # Win32 GUI (Windows only)
    macos/              # SwiftUI GUI (macOS only)
    linux/              # GTK4 GUI (Linux only)
```

## Build targets

| Target | Description |
| ------ | ----------- |
| `gittoolkit_core` | Static library with all core logic |
| `gittoolkit_cli` | CLI executable |
| `gittoolkit_tests` | Unit test executable |
