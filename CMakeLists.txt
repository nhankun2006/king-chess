cmake_minimum_required(VERSION 3.20)
project(king-chess
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "A well-known chess game with bot integrated"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ─── Options ─────────────────────────────────────────────────────────────────
option(USE_WAYLAND       "Build with Wayland support"                        ON)
option(USE_X11           "Build with X11 support"                           OFF)
option(AUTO_INSTALL_DEPS "Auto-install missing system deps via pkg manager" OFF)

# ─── Check / install display-server dev libraries ───────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CheckDisplayServer)
check_display_server_deps()

# Tell SDL3 which backends to build (only matters for FetchContent path)
set(SDL_WAYLAND ${USE_WAYLAND} CACHE BOOL "" FORCE)
set(SDL_X11     ${USE_X11}     CACHE BOOL "" FORCE)

# ─── SDL3: try system install first, fall back to FetchContent ───────────────
find_package(SDL3 QUIET)

if(SDL3_FOUND)
    message(STATUS "✓ Found system SDL3 (version ${SDL3_VERSION})")
else()
    message(STATUS "✗ System SDL3 not found — fetching from GitHub …")

    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        main              # or pin to a release tag, e.g. release-3.2.8
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )

    # Build SDL3 as a static library so the executable is self-contained
    set(SDL_STATIC  ON  CACHE BOOL "" FORCE)
    set(SDL_SHARED  OFF CACHE BOOL "" FORCE)
    set(SDL_TEST    OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL3)

    message(STATUS "✓ SDL3 fetched and built from source")
endif()

# ─── Project sources ─────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

# Link SDL3 — works for both system and FetchContent paths
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
