cmake_minimum_required(VERSION 3.20)
project(king-chess
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "A well-known chess game with bot integrated"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Raylib dependency resolution:
# - Windows: use CMake config package (vcpkg-friendly)
# - Non-Windows: prefer CMake config package, fallback to pkg-config
if(WIN32)
    find_package(raylib CONFIG REQUIRED)
    set(RAYLIB_TARGET raylib)
else()
    find_package(raylib QUIET)
    if(TARGET raylib)
        set(RAYLIB_TARGET raylib)
    elseif(TARGET raylib::raylib)
        set(RAYLIB_TARGET raylib::raylib)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)
        set(RAYLIB_TARGET PkgConfig::RAYLIB)
    endif()
endif()

# ─── Project sources ─────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

# On Windows, use WIN32 executable to avoid the console window in Release mode
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${RAYLIB_TARGET}
)

# ─── Copy assets to build directory ──────────────────────────────────────────
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets to output directory"
    )
endif()
