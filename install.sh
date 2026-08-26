#!/usr/bin/env bash
# Checks for (and offers to install) the tools tdf.py needs:
# arm-none-eabi-gcc, cmake (3.20+), ninja, and a private venv at
# tdf_venv/ with pyserial in it (only used by `tdf.py monitor`). OpenOCD
# is not checked here -- it's bundled in
# Generation/AUDK32_A34xxxx-1.0.12/Tool/OpenOCD/ for Linux/macOS/Windows
# and needs no separate install.
#
# pyserial goes in a dedicated venv rather than a plain `pip install
# pyserial`, because most current Python installs (Homebrew, most Linux
# distros) refuse system-wide pip installs outright (PEP 668,
# "externally-managed-environment") -- a venv sidesteps that instead of
# fighting it, and keeps this repo's one Python dependency out of your
# system Python entirely. `source ./export.sh` activates it automatically.
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

MISSING=()
command -v arm-none-eabi-gcc >/dev/null 2>&1 || MISSING+=("arm-none-eabi-gcc")
command -v cmake             >/dev/null 2>&1 || MISSING+=("cmake")
command -v ninja             >/dev/null 2>&1 || MISSING+=("ninja")

PYTHON=python3
command -v "$PYTHON" >/dev/null 2>&1 || PYTHON=python

PYSERIAL_MISSING=1
VPY="$(venv_python)"
if [ -n "$VPY" ] && "$VPY" -c "import serial" >/dev/null 2>&1; then
    PYSERIAL_MISSING=0
fi

echo "Checking for required tools..."
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

# A venv sidesteps pip's externally-managed-environment restriction
# entirely (it's always writable, regardless of the system Python's
# marker), so this doesn't need OS-specific branching -- handled once
# here rather than inside the OS-specific case below (which exits early
# on Windows before ever reaching that point).
if [ "$PYSERIAL_MISSING" -eq 1 ]; then
    echo
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
            "$PYTHON" -m venv "$VENV_DIR"
        fi
        VPY="$(venv_python)"
        if [ -z "$VPY" ]; then
            echo "warning: venv creation appears to have failed -- 'tdf.py monitor' won't work." >&2
        else
            "$VPY" -m pip install pyserial
        fi
    fi
fi

if [ ${#MISSING[@]} -eq 0 ]; then
    echo
    VPY="$(venv_python)"
    if [ -n "$VPY" ] && "$VPY" -c "import serial" >/dev/null 2>&1; then
        echo "All required tools are already installed."
    else
        echo "Required build/flash tools are installed. pyserial (in tdf_venv/) is"
        echo "still missing -- 'tdf.py monitor' won't work until it is (see above)."
    fi
    exit 0
fi

echo
echo "Missing: ${MISSING[*]}"

HOST_OS="$(uname -s)"
case "$HOST_OS" in
    Darwin)
        if ! command -v brew >/dev/null 2>&1; then
            echo "error: Homebrew not found. Install it from https://brew.sh, then re-run this script." >&2
            exit 1
        fi
        for tool in "${MISSING[@]}"; do
            case "$tool" in
                cmake) confirm_and_run "brew install cmake" brew install cmake || true ;;
                ninja) confirm_and_run "brew install ninja" brew install ninja || true ;;
                arm-none-eabi-gcc)
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
                            y|Y|yes|YES) brew install --cask gcc-arm-embedded ;;
                            *) echo "  skipped -- install manually, see above" ;;
                        esac
                    fi
                    ;;
            esac
        done
        ;;
    Linux)
        if command -v apt-get >/dev/null 2>&1; then
            for tool in "${MISSING[@]}"; do
                case "$tool" in
                    cmake) confirm_and_run "sudo apt-get install -y cmake" sudo apt-get install -y cmake || true ;;
                    ninja) confirm_and_run "sudo apt-get install -y ninja-build" sudo apt-get install -y ninja-build || true ;;
                    arm-none-eabi-gcc)
                        confirm_and_run "sudo apt-get install -y gcc-arm-none-eabi" sudo apt-get install -y gcc-arm-none-eabi || true
                        ;;
                esac
            done
        else
            echo "error: no apt-get found. Install manually with your distro's package manager:" >&2
            echo "       ${MISSING[*]}" >&2
            exit 1
        fi
        ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "Windows (Git Bash/MSYS2 detected). This script does not auto-install here --"
        echo "pick one:"
        echo "  choco install cmake ninja       (Chocolatey; https://chocolatey.org)"
        echo "  winget install Kitware.CMake Ninja-build.Ninja"
        echo "  arm-none-eabi-gcc: search \"Arm GNU Toolchain Downloads\" on developer.arm.com"
        echo "  and run the Windows installer (adds itself to PATH)."
        exit 1
        ;;
    *)
        echo "error: unrecognized host OS ($HOST_OS). Install manually: ${MISSING[*]}" >&2
        exit 1
        ;;
esac

echo
echo "Re-run ./install.sh to confirm everything is now present."
