# Toolchain file for MinGW-w64 (used when configuring on Windows with MinGW)
# This file runs early during configure and will try to locate gcc/g++ in PATH
# so users don't need to pass -DCMAKE_C_COMPILER/CMAKE_CXX_COMPILER manually.

# Prefer target-specific names (packagers may install prefixed toolchain)
find_program(MINGW_GCC
    NAMES x86_64-w64-mingw32-gcc x86_64-posix-seh-gcc gcc
    PATHS ENV PATH
)
find_program(MINGW_GXX
    NAMES x86_64-w64-mingw32-g++ x86_64-posix-seh-g++ g++
    PATHS ENV PATH
)

if(MINGW_GCC AND MINGW_GXX)
    # Set compilers (CACHE FILEPATH so CMake treats them as configured values)
    set(CMAKE_C_COMPILER   "${MINGW_GCC}"  CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER "${MINGW_GXX}"  CACHE FILEPATH "C++ compiler" FORCE)
else()
    message(FATAL_ERROR "Could not find MinGW gcc/g++ on PATH. Please install MinGW-w64 and ensure gcc/g++ are on PATH, or run CMake with -DCMAKE_C_COMPILER and -DCMAKE_CXX_COMPILER pointing to your compilers.")
endif()

# Ensure the make program (optional) — prefer mingw32-make if available
find_program(MINGW_MAKE mingw32-make PATHS ENV PATH)
if(MINGW_MAKE)
    set(CMAKE_MAKE_PROGRAM "${MINGW_MAKE}" CACHE FILEPATH "Make program" FORCE)
endif()

# Optionally set system name for cross-compiles (not required for native MinGW)
# set(CMAKE_SYSTEM_NAME Windows)

# Minimal helpful cache defaults
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compilation database" FORCE)
