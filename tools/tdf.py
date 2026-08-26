#!/usr/bin/env python3
"""tdf.py -- Tiremo Development Framework CLI.

The single command-line entry point for building, flashing, and managing
Tiremo Cortex tutorial firmware -- replaces raw
`cmake -G Ninja -B build -DCMAKE_TOOLCHAIN_FILE=...` invocations with
short, memorable commands, the same role idf.py plays for ESP-IDF
projects. Every command underneath is still just CMake: tdf.py owns no
OpenOCD/compiler invocation logic of its own, it only decides which
`cmake --build build --target <x>` (or configure step) to run and
forwards to it -- see each tutorial's CMakeLists.txt for what actually
happens.

Run from inside a tutorial's Sw/ directory (wherever CMakeLists.txt
lives):

    tdf.py build                       configure (if needed) + build
    tdf.py build -DTIREMO_DEBUG_LEVEL=2   (extra CMake cache vars, forwarded as-is)
    tdf.py flash                       build (if needed), then flash over SWD
    tdf.py monitor                     reset the board over SWD, then stream
                                        color-coded UART log output (Ctrl+C to exit)
    tdf.py clean                       remove build/
    tdf.py clean-device                mass-erase the connected board's chip flash
    tdf.py create-project <name>       scaffold a new, empty buildable Sw/ project

`monitor` is the one command that isn't pure CMake delegation underneath
-- resetting the board is still just `cmake --build build --target
reset-device`, but reading the UART afterwards needs a serial port, and
CMake/OpenOCD has no such concept. That part uses `pyserial`
(`python3 -m pip install pyserial`, or `./install.sh`, which checks for
it alongside the compiler toolchain).

`create-project` doesn't need to run from inside an existing project --
run it from wherever the new project's parent directory should be. It
creates `<name>/Sw/` (CMakeLists.txt, Src/main.c, an empty
Src/prv_user_code.c) from the templates in `tools/templates/new_project/`,
computing the relative path back to this repo's shared `Generation/`
vendor SDK for wherever it lands, rather than assuming the fixed
three-levels-deep layout every Getting_Started tutorial uses. It's
deliberately minimal -- no board_config.h, no Tiremo_Hal -- copy in
whichever modules you need from an existing tutorial's `Sw/`.

Self-locating: this file lives at <repo>/tools/tdf.py, so it finds the
shared toolchain file from its own path -- no environment variables
required. Run `. ./export.sh` (from the repo root, once per shell
session) if you'd rather type bare `tdf.py` than the full path to this
script; it is a convenience, not a requirement -- `python tools/tdf.py
build` works identically without ever running export.sh.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLCHAIN_FILE = REPO_ROOT / "cmake" / "toolchain-arm-none-eabi.cmake"
BUILD_DIR = Path("build")
NEW_PROJECT_TEMPLATE_DIR = REPO_ROOT / "tools" / "templates" / "new_project"
_PROJECT_NAME_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

# TIREMO_Debug_Log()'s exact wire format is "[LEVEL] message\r\n" -- see
# Common/Src/tiremo_debug.c. Matched against the first whitespace-delimited
# token of each line; anything that doesn't match prints uncolored.
_LEVEL_COLORS = {
    "[ERROR]": "\033[31m",  # red
    "[WARN]": "\033[33m",   # yellow
    "[INFO]": "\033[32m",   # green
    "[DEBUG]": "\033[36m",  # cyan
}
_COLOR_RESET = "\033[0m"


def colorize(line):
    color = _LEVEL_COLORS.get(line.split(" ", 1)[0])
    return f"{color}{line}{_COLOR_RESET}" if color else line


def log(msg):
    # flush=True: without it, these interleave out of order with the
    # subprocess's own output, which writes directly to the inherited
    # stdout fd while print() stays Python-buffered until exit.
    print(f"==> {msg}", flush=True)


def die(msg):
    print(f"error: {msg}", file=sys.stderr, flush=True)
    sys.exit(1)


def require_project_dir():
    if not Path("CMakeLists.txt").exists():
        die(
            "no CMakeLists.txt in the current directory.\n"
            "       Run tdf.py from inside a tutorial's Sw/ directory, e.g.:\n"
            "         cd Getting_Started/01-Hello_Blink/Sw\n"
            "         tdf.py build"
        )
    if not TOOLCHAIN_FILE.exists():
        die(f"toolchain file not found: {TOOLCHAIN_FILE}\n"
            "       Is tools/tdf.py still inside the Tiremo-Cortex-Tutorials repo?")


def require_tool(name):
    if shutil.which(name) is None:
        die(
            f"'{name}' not found on PATH. Run "
            f"{REPO_ROOT / 'install.sh'} (or install it yourself), "
            "then open a new shell."
        )


def run(cmd):
    log(" ".join(str(c) for c in cmd))
    result = subprocess.run(cmd)
    if result.returncode != 0:
        sys.exit(result.returncode)


def configure(defines):
    require_tool("cmake")
    require_tool("ninja")
    require_tool("arm-none-eabi-gcc")
    cmd = [
        "cmake", "-G", "Ninja", "-B", str(BUILD_DIR),
        f"-DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN_FILE}",
    ]
    for define in defines:
        cmd.append(f"-D{define}")
    run(cmd)


def needs_configure(args):
    return args.reconfigure or args.define or not (BUILD_DIR / "CMakeCache.txt").exists()


def cmd_build(args):
    require_project_dir()
    if needs_configure(args):
        configure(args.define)
    run(["cmake", "--build", str(BUILD_DIR)])


def cmd_clean(args):
    require_project_dir()
    if BUILD_DIR.exists():
        log(f"removing {BUILD_DIR}/")
        shutil.rmtree(BUILD_DIR)
    else:
        log(f"{BUILD_DIR}/ does not exist, nothing to clean")


def cmd_flash(args):
    require_project_dir()
    cmd_build(args)
    run(["cmake", "--build", str(BUILD_DIR), "--target", "flash"])


def cmd_monitor(args):
    require_project_dir()
    require_tool("cmake")

    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        die(
            "the 'pyserial' Python package is required for 'tdf.py monitor'.\n"
            "       Run ./install.sh once -- it creates a venv at tdf_venv/ and\n"
            "       installs pyserial there (plain 'pip install pyserial' fails on\n"
            "       most current systems, PEP 668), then `source ./export.sh` to\n"
            "       activate it. Or run this command with tdf_venv's own\n"
            "       interpreter directly: tdf_venv/bin/python3 tools/tdf.py monitor"
        )

    if not (BUILD_DIR / "CMakeCache.txt").exists():
        configure([])

    log("resetting device over SWD")
    run(["cmake", "--build", str(BUILD_DIR), "--target", "reset-device"])

    port = args.port
    if port is None:
        candidates = list(list_ports.comports())
        cmsis = [p for p in candidates if "cmsis-dap" in (p.description or "").lower()]
        pool = cmsis if cmsis else candidates
        if len(pool) == 1:
            port = pool[0].device
        elif len(pool) > 1:
            log("multiple serial ports found -- pass one explicitly with --port:")
            for p in pool:
                log(f"  {p.device}  ({p.description})")
            sys.exit(1)
        else:
            die(
                "no serial port auto-detected. Pass one explicitly:\n"
                "       tdf.py monitor --port /dev/tty.usbmodemXXXX   "
                "(or COM3 on Windows, /dev/ttyACM0 on Linux)"
            )

    use_color = (not args.no_color) and sys.stdout.isatty()
    log(f"monitoring {port} @ {args.baud} 8N1 -- Ctrl+C to exit")

    try:
        with serial.Serial(port, args.baud, timeout=1) as ser:
            while True:
                raw = ser.readline()
                if not raw:
                    continue
                text = raw.decode("utf-8", errors="replace").rstrip("\r\n")
                if not text:
                    continue
                print(colorize(text) if use_color else text, flush=True)
    except KeyboardInterrupt:
        print()
        log("monitor stopped")
    except serial.SerialException as exc:
        die(f"serial port error: {exc}")


def cmd_clean_device(args):
    require_project_dir()
    require_tool("cmake")

    if not args.yes:
        reply = input(
            "This mass-erases the ENTIRE chip flash on the connected board "
            "(both banks) -- not just this project's image. The board will "
            "run nothing until you flash it again. Continue? [y/N] "
        )
        if reply.strip().lower() not in ("y", "yes"):
            log("cancelled")
            return

    if not (BUILD_DIR / "CMakeCache.txt").exists():
        configure([])
    run(["cmake", "--build", str(BUILD_DIR), "--target", "erase-device"])


def cmd_create_project(args):
    if not _PROJECT_NAME_RE.match(args.name):
        die(
            f"'{args.name}' isn't a valid project name -- it becomes a CMake "
            "target name, so it must start with a letter or underscore and "
            "contain only letters, digits, and underscores."
        )

    parent = Path(args.dir).resolve() if args.dir else Path.cwd().resolve()
    sw_dir = parent / args.name / "Sw"

    if sw_dir.exists():
        die(f"{sw_dir} already exists -- pick a different name or --dir, "
            "or remove it yourself first.")

    try:
        sw_dir.relative_to(REPO_ROOT)
    except ValueError:
        die(
            f"{sw_dir} would be outside this repo ({REPO_ROOT}).\n"
            "       New projects need to stay inside the repo so their "
            "CMakeLists.txt can reach the shared Generation/ vendor SDK "
            "by a relative path."
        )

    gen_dir_rel = os.path.relpath(REPO_ROOT, sw_dir)

    (sw_dir / "Src").mkdir(parents=True)

    cmakelists = (NEW_PROJECT_TEMPLATE_DIR / "CMakeLists.txt").read_text()
    cmakelists = cmakelists.replace("__PROJECT_NAME__", args.name)
    cmakelists = cmakelists.replace("__GEN_DIR_REL__", gen_dir_rel)
    (sw_dir / "CMakeLists.txt").write_text(cmakelists)

    shutil.copy(NEW_PROJECT_TEMPLATE_DIR / "Src" / "main.c", sw_dir / "Src" / "main.c")
    shutil.copy(NEW_PROJECT_TEMPLATE_DIR / "Src" / "prv_user_code.c", sw_dir / "Src" / "prv_user_code.c")

    log(f"created {sw_dir}")
    log(f"next: cd {sw_dir} && tdf.py build && tdf.py flash")


def main():
    parser = argparse.ArgumentParser(
        prog="tdf.py",
        description="Tiremo Development Framework -- build/flash tool for Tiremo Cortex tutorials.",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    def add_define_args(p):
        p.add_argument(
            "-D", dest="define", action="append", default=[], metavar="KEY=VALUE",
            help="Extra CMake cache variable, e.g. -DTIREMO_DEBUG_LEVEL=2 "
                 "(forwarded to cmake configure)",
        )
        p.add_argument(
            "--reconfigure", action="store_true",
            help="Force a fresh cmake configure step even if build/ already exists",
        )

    p_build = sub.add_parser("build", help="Configure (if needed) and build")
    add_define_args(p_build)
    p_build.set_defaults(func=cmd_build)

    p_clean = sub.add_parser("clean", help="Remove the build/ directory")
    p_clean.set_defaults(func=cmd_clean)

    p_flash = sub.add_parser("flash", help="Build (if needed), then flash over SWD")
    add_define_args(p_flash)
    p_flash.set_defaults(func=cmd_flash)

    p_monitor = sub.add_parser(
        "monitor",
        help="Reset the board over SWD, then stream color-coded UART log output",
    )
    p_monitor.add_argument(
        "--port", metavar="PORT",
        help="Serial port device, e.g. /dev/tty.usbmodemXXXX, /dev/ttyACM0, COM3 "
             "(auto-detected if omitted and exactly one candidate is found)",
    )
    p_monitor.add_argument(
        "--baud", type=int, default=115200,
        help="Baud rate (default: 115200, matching TIREMO_Debug_Init())",
    )
    p_monitor.add_argument(
        "--no-color", action="store_true",
        help="Disable ANSI color coding by log level",
    )
    p_monitor.set_defaults(func=cmd_monitor)

    p_erase = sub.add_parser(
        "clean-device",
        help="Mass-erase the connected board's chip flash (destructive; asks first)",
    )
    p_erase.add_argument("-y", "--yes", action="store_true", help="Skip the confirmation prompt")
    p_erase.set_defaults(func=cmd_clean_device)

    p_create = sub.add_parser(
        "create-project",
        help="Scaffold a new, empty buildable Sw/ project (CMakeLists.txt + main.c + empty prv_user_code.c)",
    )
    p_create.add_argument("name", help="Project name -- used as the CMake target name and the new folder's name")
    p_create.add_argument(
        "--dir", metavar="PARENT_DIR",
        help="Parent directory for the new <name>/ project folder (default: current directory)",
    )
    p_create.set_defaults(func=cmd_create_project)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
