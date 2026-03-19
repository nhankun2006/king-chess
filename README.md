# King Chess

A modern chess game built with raylib and C++.

## Build Dependencies

- C++17 compiler
- CMake 3.20+
- raylib

### Linux (system raylib)

This project is set up to use system-installed raylib on Linux.

Fedora example:

```bash
sudo dnf install raylib-devel
```

Configure and build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

Run:

```bash
./build/king-chess
```

### Windows (later / optional)

Windows support is prepared for `vcpkg` via `find_package(raylib CONFIG REQUIRED)`.
You can wire up a vcpkg toolchain later without changing Linux setup.

## License

[MIT License](LICENSE)
