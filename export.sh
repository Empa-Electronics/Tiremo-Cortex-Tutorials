#!/usr/bin/env bash
# Adds tools/ to PATH so `tdf.py` can be run bare, from any directory, in
# this shell session, and activates tdf_venv/ (see install.sh) so
# `tdf.py monitor` can find pyserial -- mirrors ESP-IDF's export.sh,
# venv activation included.
#
# The PATH addition is a convenience, not a requirement: `python3
# <repo>/tools/tdf.py build` (or `python tools\tdf.py build` on Windows)
# works identically without ever running this script, since tdf.py
# locates the repo itself from its own file path. `monitor` specifically
# does need pyserial on the interpreter's path, though -- either source
# this script first, or run tdf.py with tdf_venv's own python3 directly.
#
# Must be SOURCED, not executed -- it edits the CURRENT shell's PATH.
# Running it as `./export.sh` instead of `. ./export.sh` only edits a
# child shell's environment, which disappears the moment that child exits.
#
#   source ./export.sh      # bash/zsh
#   . ./export.sh           # POSIX sh form, works in both too

# Detect direct execution (as opposed to sourcing) where the shell
# supports it; not all shells expose this the same way, so this is
# best-effort, not a hard guarantee.
if [ -n "${BASH_SOURCE:-}" ] && [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    echo "error: export.sh must be sourced, not executed:" >&2
    echo "         source ./export.sh" >&2
    echo "       or:" >&2
    echo "         . ./export.sh" >&2
    exit 1
fi

TDF_PATH="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
export TDF_PATH

case ":$PATH:" in
    *":$TDF_PATH/tools:"*)
        ;; # already on PATH -- re-sourcing export.sh is a no-op
    *)
        export PATH="$TDF_PATH/tools:$PATH"
        ;;
esac

echo "Tiremo Development Framework: tdf.py added to PATH."

# Activate tdf_venv/ (created by install.sh) so `python3`/`pip` -- and
# therefore tdf.py's own `#!/usr/bin/env python3` -- resolve to the venv
# with pyserial in it, not the system interpreter. VIRTUAL_ENV_DISABLE_PROMPT
# keeps this from rewriting a customized PS1; POSIX venvs use bin/activate,
# venvs made by a Windows-native Python use Scripts/activate.
if [ -f "$TDF_PATH/tdf_venv/bin/activate" ]; then
    VIRTUAL_ENV_DISABLE_PROMPT=1
    export VIRTUAL_ENV_DISABLE_PROMPT
    # shellcheck disable=SC1091
    . "$TDF_PATH/tdf_venv/bin/activate"
    echo "tdf_venv activated (pyserial available for 'tdf.py monitor')."
elif [ -f "$TDF_PATH/tdf_venv/Scripts/activate" ]; then
    VIRTUAL_ENV_DISABLE_PROMPT=1
    export VIRTUAL_ENV_DISABLE_PROMPT
    # shellcheck disable=SC1091
    . "$TDF_PATH/tdf_venv/Scripts/activate"
    echo "tdf_venv activated (pyserial available for 'tdf.py monitor')."
else
    echo "No tdf_venv/ found -- 'tdf.py monitor' needs pyserial. Run ./install.sh once to set it up."
fi

echo "Try: cd Getting_Started/01-Hello_Blink/Sw && tdf.py build"
