#!/usr/bin/env bash
# Checks for (and offers to install) everything tdf.py needs:
#
#   python3            -- runs tdf.py itself (every command, build included)
#   arm-none-eabi-gcc  -- the bare-metal compiler
#   cmake (3.20+)      -- the build system
#   ninja              -- the build backend
#   pyserial           -- only `tdf.py monitor` uses it; installed into a
#                         private venv at tdf_venv/, not system-wide (PEP 668)
#
# OpenOCD itself is bundled in
# Generation/AUDK32_A34xxxx-1.0.12/Tool/OpenOCD/ for Linux/macOS/Windows.
# The macOS binary is linked against Homebrew libftdi/hidapi/libusb/capstone,
# so those are checked (and offered) on Darwin.
#
# pyserial goes in a dedicated venv rather than a plain `pip install
# pyserial`, because most current Python installs (Homebrew, most Linux
# distros) refuse system-wide pip installs outright (PEP 668,
# "externally-managed-environment") -- a venv sidesteps that instead of
# fighting it. `source ./export.sh` activates it automatically.
#
# Safe by design: this script only ever *offers* an install command and
# asks before running it (or `-y` to skip asking) -- it never silently
# changes your system. Re-run any time; already-installed tools are just
# reported and skipped.
#
# Usage: ./install.sh [-y]
#   -y   Don't ask for confirmation before installing missing tools.
set -euo pipefail

ASSUME_YES=0
for arg in "$@"; do
    case "$arg" in
        -y|--yes) ASSUME_YES=1 ;;
        *)
            echo "error: unknown argument: $arg" >&2
            echo "usage: $0 [-y]" >&2
            exit 1
            ;;
    esac
done

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
VENV_DIR="$REPO_ROOT/tdf_venv"

# Prints the venv's own python3/python.exe path, or nothing if the venv
# doesn't exist yet. POSIX venvs use bin/; venvs made by a Windows-native
# Python (even when this script itself runs under Git Bash) use Scripts/.
venv_python() {
    if [ -x "$VENV_DIR/bin/python3" ]; then
        echo "$VENV_DIR/bin/python3"
    elif [ -x "$VENV_DIR/Scripts/python.exe" ]; then
        echo "$VENV_DIR/Scripts/python.exe"
    fi
}

# Host OS and the package manager we'll offer installs through.
HOST_OS="$(uname -s)"
PKG=""
case "$HOST_OS" in
    Darwin) command -v brew    >/dev/null 2>&1 && PKG=brew ;;
    Linux)  command -v apt-get >/dev/null 2>&1 && PKG=apt  ;;
esac

# A usable system Python 3: 'python3', else 'python' if that one is v3.
# Returns 0 either way (empty output == not found) so it's safe under set -e.
find_python() {
    if command -v python3 >/dev/null 2>&1; then
        echo python3; return 0
    fi
    if command -v python >/dev/null 2>&1 \
        && python -c 'import sys; sys.exit(sys.version_info[0] != 3)' 2>/dev/null; then
        echo python; return 0
    fi
    return 0
}
PYTHON="$(find_python)"

MISSING=()
[ -n "$PYTHON" ]                             || MISSING+=("python3")
command -v arm-none-eabi-gcc >/dev/null 2>&1 || MISSING+=("arm-none-eabi-gcc")
command -v cmake             >/dev/null 2>&1 || MISSING+=("cmake")
command -v ninja             >/dev/null 2>&1 || MISSING+=("ninja")

PYSERIAL_MISSING=1
VPY="$(venv_python)"
if [ -n "$VPY" ] && "$VPY" -c "import serial" >/dev/null 2>&1; then
    PYSERIAL_MISSING=0
fi

# Bundled openocd.darwin is linked to these Homebrew dylibs (absolute
# /opt/homebrew/opt/... install names). Linux/Windows OpenOCD ships its
# own libs; only Darwin needs this check.
OPENOCD_BREW_LIBS=(libftdi hidapi libusb capstone)
OPENOCD_LIBS_MISSING=()
if [ "$HOST_OS" = Darwin ]; then
    BREW_PREFIX=""
    [ "$PKG" = brew ] && BREW_PREFIX="$(brew --prefix 2>/dev/null)" || true
    for pkg in "${OPENOCD_BREW_LIBS[@]}"; do
        if [ -z "$BREW_PREFIX" ] || [ ! -e "$BREW_PREFIX/opt/$pkg" ]; then
            OPENOCD_LIBS_MISSING+=("$pkg")
        fi
    done
fi

echo "Checking for required tools..."
if [ -n "$PYTHON" ]; then
    pyver="$("$PYTHON" --version 2>&1)"
    echo "  [x] python3 -- $pyver"
else
    echo "  [ ] python3 -- not found (runs tdf.py itself)"
fi
for tool in arm-none-eabi-gcc cmake ninja; do
    if command -v "$tool" >/dev/null 2>&1; then
        version="$("$tool" --version 2>&1 | head -1)"
        echo "  [x] $tool -- $version"
    else
        echo "  [ ] $tool -- not found"
    fi
done
if [ "$PYSERIAL_MISSING" -eq 0 ]; then
    version="$("$VPY" -c 'import serial; print(serial.__version__)' 2>&1)"
    echo "  [x] pyserial -- $version, in tdf_venv/ (used by: tdf.py monitor)"
else
    echo "  [ ] pyserial -- not found in tdf_venv/ (used by: tdf.py monitor)"
fi
if [ "$HOST_OS" = Darwin ]; then
    if [ ${#OPENOCD_LIBS_MISSING[@]} -eq 0 ]; then
        echo "  [x] openocd runtime libs -- ${OPENOCD_BREW_LIBS[*]} (used by: tdf.py flash)"
    else
        echo "  [ ] openocd runtime libs -- missing: ${OPENOCD_LIBS_MISSING[*]} (used by: tdf.py flash)"
    fi
fi

confirm_and_run() {
    # $1 = human-readable description, remaining args = the command itself.
    # Returns 1 on decline/skip -- always call this guarded by `if` or
    # `||`/`&&`, never bare, or that 1 trips `set -e` and kills the whole
    # script instead of just skipping this one tool.
    local desc="$1"; shift
    if [ "$ASSUME_YES" -eq 0 ]; then
        read -r -p "Run: $desc ? [y/N] " reply
        case "$reply" in
            y|Y|yes|YES) ;;
            *) echo "  skipped"; return 1 ;;
        esac
    fi
    "$@"
}

# Offer to install one tool through $PKG. Never fatal: every branch ends
# with `|| true` (or a bare `case`) so a decline or a failed install just
# moves on to the next tool.
install_tool() {
    case "$PKG:$1" in
        brew:python3) confirm_and_run "brew install python3" brew install python3 || true ;;
        brew:cmake)   confirm_and_run "brew install cmake"   brew install cmake   || true ;;
        brew:ninja)   confirm_and_run "brew install ninja"   brew install ninja   || true ;;
        brew:arm-none-eabi-gcc)
            echo
            echo "arm-none-eabi-gcc: Homebrew's cask for this has changed names over the"
            echo "years, so this script won't guess which one is current. Two options:"
            echo "  1) brew install --cask gcc-arm-embedded   (commonly used cask name)"
            echo "  2) Official installer from Arm: search \"Arm GNU Toolchain Downloads\""
            echo "     on developer.arm.com and pick the AArch32 bare-metal (arm-none-eabi)"
            echo "     build for macOS."
            if [ "$ASSUME_YES" -eq 0 ]; then
                read -r -p "Try option 1 (brew install --cask gcc-arm-embedded) now? [y/N] " reply
                case "$reply" in
                    y|Y|yes|YES) brew install --cask gcc-arm-embedded || true ;;
                    *) echo "  skipped -- install manually, see above" ;;
                esac
            fi
            ;;
        apt:python3)
            confirm_and_run "sudo apt-get install -y python3 python3-venv python3-pip" \
                sudo apt-get install -y python3 python3-venv python3-pip || true ;;
        apt:cmake) confirm_and_run "sudo apt-get install -y cmake" sudo apt-get install -y cmake || true ;;
        apt:ninja) confirm_and_run "sudo apt-get install -y ninja-build" sudo apt-get install -y ninja-build || true ;;
        apt:arm-none-eabi-gcc)
            confirm_and_run "sudo apt-get install -y gcc-arm-none-eabi" \
                sudo apt-get install -y gcc-arm-none-eabi || true ;;
    esac
}

# --- install missing build tools / python3 (needs a package manager) ----
if [ ${#MISSING[@]} -gt 0 ]; then
    echo
    echo "Missing: ${MISSING[*]}"

    if [ -z "$PKG" ]; then
        case "$HOST_OS" in
            Darwin)
                echo "error: Homebrew not found. Install it from https://brew.sh, then re-run." >&2
                ;;
            Linux)
                echo "error: no apt-get found. Install these with your distro's package manager," >&2
                echo "       then re-run ./install.sh:  ${MISSING[*]}" >&2
                ;;
            MINGW*|MSYS*|CYGWIN*)
                echo "Windows shell detected. Use the native scripts instead:" >&2
                echo "  install.ps1  (PowerShell)   or   install.bat  (cmd.exe)" >&2
                ;;
            *)
                echo "error: unrecognized host OS ($HOST_OS). Install manually: ${MISSING[*]}" >&2
                ;;
        esac
        exit 1
    fi

    for tool in "${MISSING[@]}"; do
        install_tool "$tool"
    done

    # brew/apt symlink into a dir that's already on PATH, so a just-installed
    # python3 is usually visible right now -- re-resolve for the venv step.
    PYTHON="$(find_python)"
fi

# --- macOS OpenOCD Homebrew dylibs (needed to actually run tdf.py flash) -
if [ "$HOST_OS" = Darwin ] && [ ${#OPENOCD_LIBS_MISSING[@]} -gt 0 ]; then
    echo
    if [ "$PKG" != brew ]; then
        echo "error: Homebrew not found. Install it from https://brew.sh, then:" >&2
        echo "       brew install ${OPENOCD_BREW_LIBS[*]}" >&2
    else
        confirm_and_run "brew install ${OPENOCD_BREW_LIBS[*]}" \
            brew install "${OPENOCD_BREW_LIBS[@]}" || true
        OPENOCD_LIBS_MISSING=()
        BREW_PREFIX="$(brew --prefix 2>/dev/null)" || BREW_PREFIX=""
        for pkg in "${OPENOCD_BREW_LIBS[@]}"; do
            if [ -z "$BREW_PREFIX" ] || [ ! -e "$BREW_PREFIX/opt/$pkg" ]; then
                OPENOCD_LIBS_MISSING+=("$pkg")
            fi
        done
    fi
fi

# --- venv + pyserial (needs a working python3) --------------------------
if [ "$PYSERIAL_MISSING" -eq 1 ]; then
    echo
    if [ -z "$PYTHON" ] && [ -z "$(venv_python)" ]; then
        echo "python3 not available -- skipping tdf_venv/ + pyserial setup."
        echo "Install python3 (open a new shell if you just did), then re-run ./install.sh."
    else
        RUN_VENV_SETUP=$ASSUME_YES
        if [ "$ASSUME_YES" -eq 0 ]; then
            read -r -p "Create a venv at tdf_venv/ and install pyserial into it? [y/N] " reply
            case "$reply" in
                y|Y|yes|YES) RUN_VENV_SETUP=1 ;;
                *) echo "  skipped"; RUN_VENV_SETUP=0 ;;
            esac
        fi

        if [ "$RUN_VENV_SETUP" -eq 1 ]; then
            if [ -z "$(venv_python)" ]; then
                echo "  creating $VENV_DIR"
                if ! "$PYTHON" -m venv "$VENV_DIR"; then
                    echo "warning: 'python3 -m venv' failed." >&2
                    if [ "$PKG" = apt ]; then
                        echo "         On Debian/Ubuntu: sudo apt-get install -y python3-venv python3-pip" >&2
                    fi
                fi
            fi
            VPY="$(venv_python)"
            if [ -z "$VPY" ]; then
                echo "warning: venv not created -- 'tdf.py monitor' won't work." >&2
            elif "$VPY" -m pip install pyserial; then
                PYSERIAL_MISSING=0
            else
                echo "warning: 'pip install pyserial' failed -- 'tdf.py monitor' won't work." >&2
            fi
        fi
    fi
fi

# --- final status ------------------------------------------------------
echo
VPY="$(venv_python)"
if [ ${#MISSING[@]} -eq 0 ] && [ ${#OPENOCD_LIBS_MISSING[@]} -eq 0 ] \
    && [ -n "$VPY" ] && "$VPY" -c "import serial" >/dev/null 2>&1; then
    echo "All required tools are installed."
    echo "Next:  source ./export.sh   then   cd Getting_Started/01-Hello_Blink/Sw   then   tdf.py flash monitor"
else
    echo "Re-run ./install.sh to confirm everything is now present"
    echo "(open a new shell first if a package manager just changed your PATH)."
fi
