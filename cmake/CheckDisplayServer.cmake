# ─── CheckDisplayServer.cmake ────────────────────────────────────────────────
# Detects the Linux distro, checks for Wayland / X11 dev libraries, and
# optionally installs them automatically via the system package manager.
#
# Options consumed:
#   USE_WAYLAND          – look for Wayland dev libs  (default ON)
#   USE_X11              – look for X11 dev libs      (default OFF)
#   AUTO_INSTALL_DEPS    – run the package manager    (default OFF)
# ─────────────────────────────────────────────────────────────────────────────

# ── Distro detection ─────────────────────────────────────────────────────────
function(detect_linux_distro OUT_ID OUT_PKG_CMD)
    set(_id "unknown")
    set(_cmd "unknown")

    if(EXISTS "/etc/os-release")
        file(STRINGS "/etc/os-release" _lines)
        foreach(_line IN LISTS _lines)
            if(_line MATCHES "^ID=(.*)")
                string(STRIP "${CMAKE_MATCH_1}" _id)
                string(REPLACE "\"" "" _id "${_id}")
            endif()
        endforeach()
    endif()

    # Map distro ID → package-manager install command
    if(_id MATCHES "^(fedora|rhel|centos|rocky|alma)$")
        set(_cmd "dnf install -y")
    elseif(_id MATCHES "^(ubuntu|debian|linuxmint|pop|elementary|zorin)$")
        set(_cmd "apt-get install -y")
    elseif(_id MATCHES "^(arch|manjaro|endeavouros|garuda)$")
        set(_cmd "pacman -S --noconfirm")
    elseif(_id MATCHES "^(opensuse.*|sles|suse)$")
        set(_cmd "zypper install -y")
    endif()

    set(${OUT_ID}  "${_id}"  PARENT_SCOPE)
    set(${OUT_PKG_CMD} "${_cmd}" PARENT_SCOPE)
endfunction()

# ── Package lists per distro family ──────────────────────────────────────────
function(get_wayland_packages DISTRO_ID OUT_PACKAGES)
    if(DISTRO_ID MATCHES "^(fedora|rhel|centos|rocky|alma)$")
        set(_pkgs "wayland-devel;wayland-protocols-devel;libxkbcommon-devel;mesa-libEGL-devel;libdrm-devel;mesa-libgbm-devel")
    elseif(DISTRO_ID MATCHES "^(ubuntu|debian|linuxmint|pop|elementary|zorin)$")
        set(_pkgs "libwayland-dev;wayland-protocols;libxkbcommon-dev;libegl1-mesa-dev;libdrm-dev;libgbm-dev")
    elseif(DISTRO_ID MATCHES "^(arch|manjaro|endeavouros|garuda)$")
        set(_pkgs "wayland;wayland-protocols;libxkbcommon;mesa;libdrm")
    elseif(DISTRO_ID MATCHES "^(opensuse.*|sles|suse)$")
        set(_pkgs "wayland-devel;wayland-protocols-devel;libxkbcommon-devel;Mesa-libEGL-devel;libdrm-devel;Mesa-libgbm-devel")
    else()
        set(_pkgs "")
    endif()
    set(${OUT_PACKAGES} "${_pkgs}" PARENT_SCOPE)
endfunction()

function(get_x11_packages DISTRO_ID OUT_PACKAGES)
    if(DISTRO_ID MATCHES "^(fedora|rhel|centos|rocky|alma)$")
        set(_pkgs "libX11-devel;libXext-devel;libXrandr-devel;libXcursor-devel;libXi-devel;libXScrnSaver-devel")
    elseif(DISTRO_ID MATCHES "^(ubuntu|debian|linuxmint|pop|elementary|zorin)$")
        set(_pkgs "libx11-dev;libxext-dev;libxrandr-dev;libxcursor-dev;libxi-dev;libxss-dev")
    elseif(DISTRO_ID MATCHES "^(arch|manjaro|endeavouros|garuda)$")
        set(_pkgs "libx11;libxext;libxrandr;libxcursor;libxi;libxss")
    elseif(DISTRO_ID MATCHES "^(opensuse.*|sles|suse)$")
        set(_pkgs "libX11-devel;libXext-devel;libXrandr-devel;libXcursor-devel;libXi-devel;libXScrnSaver-devel")
    else()
        set(_pkgs "")
    endif()
    set(${OUT_PACKAGES} "${_pkgs}" PARENT_SCOPE)
endfunction()

# ── Main check ───────────────────────────────────────────────────────────────
function(check_display_server_deps)
    find_package(PkgConfig REQUIRED)

    set(_need_wayland OFF)
    set(_need_x11     OFF)
    set(_all_packages "")

    # ── Wayland ──
    if(USE_WAYLAND)
        pkg_check_modules(_WAY QUIET wayland-client)
        if(_WAY_FOUND)
            message(STATUS "✓ Wayland development libraries found")
        else()
            set(_need_wayland ON)
            message(STATUS "✗ Wayland development libraries NOT found")
        endif()
    endif()

    # ── X11 ──
    if(USE_X11)
        pkg_check_modules(_X11 QUIET x11)
        if(_X11_FOUND)
            message(STATUS "✓ X11 development libraries found")
        else()
            set(_need_x11 ON)
            message(STATUS "✗ X11 development libraries NOT found")
        endif()
    endif()

    # Nothing missing → all good
    if(NOT _need_wayland AND NOT _need_x11)
        return()
    endif()

    # ── Detect distro ──
    detect_linux_distro(_distro _pkg_cmd)

    if(_need_wayland)
        get_wayland_packages("${_distro}" _way_pkgs)
        list(APPEND _all_packages ${_way_pkgs})
    endif()
    if(_need_x11)
        get_x11_packages("${_distro}" _x11_pkgs)
        list(APPEND _all_packages ${_x11_pkgs})
    endif()

    # Turn the list into a space-separated string for the shell command
    list(JOIN _all_packages " " _pkg_str)
    set(_full_cmd "sudo ${_pkg_cmd} ${_pkg_str}")

    # ── AUTO_INSTALL_DEPS: run the package manager automatically ──
    if(AUTO_INSTALL_DEPS)
        message(STATUS "AUTO_INSTALL_DEPS is ON — installing missing packages …")
        message(STATUS "Running: ${_full_cmd}")

        execute_process(
            COMMAND sh -c "${_full_cmd}"
            RESULT_VARIABLE _rc
        )

        if(NOT _rc EQUAL 0)
            message(FATAL_ERROR
                "Package installation failed (exit code ${_rc}).\n"
                "Try running this manually:\n"
                "  ${_full_cmd}\n"
            )
        endif()

        message(STATUS "✓ Packages installed successfully — continuing build")
        return()
    endif()

    # ── Manual mode: tell the user exactly what to run ──
    if(_distro STREQUAL "unknown")
        message(FATAL_ERROR
            "\n"
            "══════════════════════════════════════════════════════════════\n"
            "  ERROR: Missing display-server development libraries!\n"
            "══════════════════════════════════════════════════════════════\n"
            "\n"
            "  Could not detect your Linux distribution.\n"
            "  Please install the equivalent of these packages:\n"
            "    ${_pkg_str}\n"
            "\n"
            "  Or re-run CMake with automatic installation:\n"
            "    cmake -B build -DAUTO_INSTALL_DEPS=ON\n"
            "══════════════════════════════════════════════════════════════\n"
        )
    else()
        message(FATAL_ERROR
            "\n"
            "══════════════════════════════════════════════════════════════\n"
            "  ERROR: Missing display-server development libraries!\n"
            "══════════════════════════════════════════════════════════════\n"
            "\n"
            "  Detected distro: ${_distro}\n"
            "\n"
            "  Option 1 — Install manually:\n"
            "    ${_full_cmd}\n"
            "\n"
            "  Option 2 — Let CMake handle it:\n"
            "    cmake -B build -DAUTO_INSTALL_DEPS=ON\n"
            "\n"
            "  Then re-run:  cmake -B build\n"
            "══════════════════════════════════════════════════════════════\n"
        )
    endif()
endfunction()
