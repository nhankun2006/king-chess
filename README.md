# King Chess

A modern chess game built with raylib and C++.

## Build Dependencies

- C++17 compiler
- CMake 3.20+
- make (Linux) or mingw32-make (Windows)
- pkg-config (Linux only)
- raylib

---

## Linux

Install the raylib development package for your distro:

```bash
# Fedora / RHEL / CentOS / Rocky / Alma
sudo dnf install raylib-devel

# Ubuntu / Debian / Linux Mint / Pop!_OS
sudo apt-get install libraylib-dev

# Arch / Manjaro / EndeavourOS
sudo pacman -S raylib

# openSUSE / SLES
sudo zypper install raylib-devel
```

Configure and build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

Or use presets:

```bash
cmake --preset linux-debug
cmake --build --preset linux-build
```
Run:

```bash
./build/king-chess
```

---

## Windows (MinGW-w64 + vcpkg)

### Prerequisites

1. Install [MinGW-w64](https://www.mingw-w64.org/) and ensure `gcc`, `g++`,
   and `mingw32-make` are on your `PATH`.
2. Install [vcpkg](https://github.com/microsoft/vcpkg) and set the
   `VCPKG_ROOT` environment variable to its install path.

### Build

vcpkg manifest mode will automatically fetch and build raylib on first
configure:

```powershell
cmake --preset mingw
cmake --build --preset mingw-build
```

For a release build:

```powershell
cmake --preset mingw-release
cmake --build --preset mingw-release-build
```

Run:

```powershell
.\build\king-chess.exe
```

---

## License

[MIT License](LICENSE)
