#!/usr/bin/env python3
r"""tdf.py -- Tiremo Development Framework CLI.

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

    tdf build                       configure (if needed) + build
    tdf build -DTIREMO_DEBUG_LEVEL=2   (extra CMake cache vars, forwarded as-is)
    tdf flash                       build (if needed), then flash over SWD
    tdf monitor                     drain stale UART, reset over SWD, then
                                        stream color-coded log output (Ctrl+C to exit)
    tdf flash monitor               chain: flash, then monitor (idf.py-style)
    tdf build flash monitor         chain: build, flash, then monitor
    tdf clean                       remove build/
    tdf clean-device                mass-erase the connected board's chip flash
    tdf create-project <name>       scaffold a new, empty buildable Sw/ project
    tdf help                        this overview
    tdf help flash                  detailed help for one command

`tdf.py` is still accepted as an alias.

Tab-complete command names after `source ./export.sh` (bash or zsh), or
`. .\export.ps1` (PowerShell).

On Windows this runs the same in cmd.exe and PowerShell -- no Git Bash or
WSL needed. Use `install.ps1`/`export.ps1` (PowerShell) or
`install.bat`/`export.bat` (cmd.exe) instead of the `.sh` scripts;
`python tools\tdf.py build` works with no setup step at all.

`monitor` is the one command that isn't pure CMake delegation underneath
-- resetting the board is still just `cmake --build build --target
reset-device`, but reading the UART afterwards needs a serial port, and
CMake/OpenOCD has no such concept. That part uses `pyserial`
(`python3 -m pip install pyserial`, or `./install.sh` /
`.\install.ps1`, which checks for it alongside the compiler toolchain).

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
session) if you'd rather type bare `tdf` (or `tdf.py`) than the full
path to this script; it is a convenience, not a requirement -- `python
tools/tdf.py build` works identically without ever running export.sh.
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


def _prog():
    """Name the user typed: `tdf` via the shim, or `tdf.py` as an alias."""
    env = os.environ.get("TDF_PROG")
    if env:
        return env
    name = Path(sys.argv[0]).name
    if name.lower() in ("tdf", "tdf.cmd", "tdf.exe"):
        return "tdf"
    return name


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


def enable_windows_ansi():
    """Turn on ANSI escape processing for this console on Windows.

    Windows 10+ consoles understand the same "\\033[..m" SGR sequences
    used by _LEVEL_COLORS, but a freshly spawned python.exe doesn't always
    have the ENABLE_VIRTUAL_TERMINAL_PROCESSING console mode bit set, so
    the codes would otherwise print literally in `tdf.py monitor`. No-op
    off Windows, and harmless if stdout isn't a real console.
    """
    if os.name != "nt":
        return
    try:
        import ctypes

        kernel32 = ctypes.windll.kernel32
        STD_OUTPUT_HANDLE = -11
        ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004
        handle = kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
        mode = ctypes.c_uint32()
        if kernel32.GetConsoleMode(handle, ctypes.byref(mode)):
            kernel32.SetConsoleMode(
                handle, mode.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING
            )
    except Exception:
        pass  # best effort -- monitor just falls back to uncolored output


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
            "       Run tdf from inside a tutorial's Sw/ directory, e.g.:\n"
            "         cd Getting_Started/01-Hello_Blink/Sw\n"
            f"         {_prog()} build"
        )
    if not TOOLCHAIN_FILE.exists():
        die(f"toolchain file not found: {TOOLCHAIN_FILE}\n"
            "       Is tools/tdf.py still inside the Tiremo-Cortex-Tutorials repo?")


def require_tool(name):
    if shutil.which(name) is None:
        installer = "install.ps1" if os.name == "nt" else "install.sh"
        die(
            f"'{name}' not found on PATH. Run "
            f"{REPO_ROOT / installer} (or install it yourself), "
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
    args._built = True


def cmd_clean(args):
    require_project_dir()
    if BUILD_DIR.exists():
        log(f"removing {BUILD_DIR}/")
        shutil.rmtree(BUILD_DIR)
    else:
        log(f"{BUILD_DIR}/ does not exist, nothing to clean")


def cmd_flash(args):
    require_project_dir()
    if not getattr(args, "_built", False):
        cmd_build(args)
    run(["cmake", "--build", str(BUILD_DIR), "--target", "flash"])


def pick_serial_port(preferred, list_ports):
    if preferred is not None:
        return preferred

    candidates = list(list_ports.comports())
    cmsis = [p for p in candidates if "cmsis-dap" in (p.description or "").lower()]
    pool = cmsis if cmsis else candidates
    if len(pool) == 1:
        return pool[0].device
    if len(pool) > 1:
        log("multiple serial ports found -- pass one explicitly with --port:")
        for p in pool:
            log(f"  {p.device}  ({p.description})")
        sys.exit(1)
    die(
        "no serial port auto-detected. Pass one explicitly:\n"
        "       tdf.py monitor --port /dev/tty.usbmodemXXXX   "
        "(or COM3 on Windows, /dev/ttyACM0 on Linux)"
    )


def cmd_monitor(args):
    require_project_dir()
    require_tool("cmake")

    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        if os.name == "nt":
            die(
                "the 'pyserial' Python package is required for 'tdf.py monitor'.\n"
                "       Run .\\install.ps1 once -- it creates a venv at tdf_venv\\ and\n"
                "       installs pyserial there -- then `. .\\export.ps1` to activate\n"
                "       it. Or run this command with tdf_venv's own interpreter\n"
                "       directly: tdf_venv\\Scripts\\python.exe tools\\tdf.py monitor"
            )
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

    port = pick_serial_port(args.port, list_ports)
    use_color = (not args.no_color) and sys.stdout.isatty()
    if use_color:
        enable_windows_ansi()

    try:
        ser = serial.Serial(port, args.baud, timeout=1)
        # Host USB-CDC / tty still holds unread bytes from the previous
        # monitor session (or from the board logging while no one was
        # listening). Drop those before SWD reset so they are not mixed
        # into the new boot log.
        ser.reset_input_buffer()
        log("resetting device over SWD")
        run(["cmake", "--build", str(BUILD_DIR), "--target", "reset-device"])
        if not ser.is_open:
            ser.close()
            port = pick_serial_port(args.port, list_ports)
            ser = serial.Serial(port, args.baud, timeout=1)
        log(f"monitoring {port} @ {args.baud} 8N1 -- Ctrl+C to exit")
        try:
            while True:
                raw = ser.readline()
                if not raw:
                    continue
                text = raw.decode("utf-8", errors="replace").rstrip("\r\n")
                if not text:
                    continue
                print(colorize(text) if use_color else text, flush=True)
        finally:
            ser.close()
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
    log(f"next: cd {sw_dir} && {_prog()} flash monitor")


def print_overview():
    p = _prog()
    cmds = "\n".join(f"  {name:<18} {COMMANDS[name][0]}" for name in COMMANDS)
    print(
        f"""{p} -- Tiremo Development Framework

Build, flash, and monitor Tiremo Cortex tutorial firmware from a
tutorial Sw/ directory (where CMakeLists.txt lives).

Usage:
  {p} <command> [command ...] [options]
  {p} help [command]

Commands:
{cmds}

Pipeline (run left to right, idf.py-style):
  {p} flash monitor
  {p} build flash monitor
  {p} build -DTIREMO_DEBUG_LEVEL=2 flash monitor --port /dev/cu.usbmodemXXXX

Options (may appear before, between, or after commands):
  -h, --help             this page
  -D KEY=VALUE           extra CMake cache var (build / flash)
  --reconfigure          force a fresh cmake configure
  --port PORT            serial device for monitor
  --baud N               monitor baud (default 115200)
  --no-color             disable monitor log colors
  -y, --yes              skip clean-device confirmation
  --dir PARENT_DIR       create-project parent directory

Tab completion: source ./export.sh (bash/zsh) or ". ./export.ps1"
               (PowerShell), then  {p} <Tab>
More on one command: {p} help flash
`tdf.py` is still accepted as an alias.
""",
        end="",
    )


def print_command_help(topic):
    page = COMMAND_PAGES.get(topic)
    if page is None:
        die(
            f"no help for {topic!r}\n"
            "       commands: " + ", ".join(COMMANDS)
        )
    page = page.replace("tdf.py", _prog())
    print(page, end="" if page.endswith("\n") else "\n")


def cmd_help(args):
    topic = getattr(args, "help_topic", None)
    if topic:
        print_command_help(topic)
    else:
        print_overview()


# Name -> (one-liner, handler). Order is help listing and tab-complete.
COMMANDS = {
    "build": ("Configure (if needed) and build", cmd_build),
    "flash": ("Build (if needed), then flash over SWD", cmd_flash),
    "monitor": ("Reset over SWD, then stream color-coded UART log output", cmd_monitor),
    "clean": ("Remove the build/ directory", cmd_clean),
    "clean-device": ("Mass-erase the connected board's chip flash (asks first)", cmd_clean_device),
    "create-project": (
        "Scaffold a new empty Sw/ project (CMakeLists.txt + main.c + empty prv_user_code.c)",
        cmd_create_project,
    ),
    "help": ("Show this overview, or detailed help for one command", cmd_help),
}

COMMAND_PAGES = {
    "build": """tdf.py build

Configure (if needed) and compile the firmware in the current Sw/
directory. Extra -D flags are forwarded to cmake configure.

  tdf.py build
  tdf.py build -DTIREMO_DEBUG_LEVEL=2
  tdf.py build --reconfigure

Underneath: cmake -G Ninja -B build -DCMAKE_TOOLCHAIN_FILE=... &&
cmake --build build
""",
    "flash": """tdf.py flash

Build if needed, then program the board over SWD (cmake target flash).

  tdf.py flash
  tdf.py flash monitor
  tdf.py build flash monitor

In a pipeline, a preceding build is not repeated.
""",
    "monitor": """tdf.py monitor

Drain stale UART bytes, reset the board over SWD, then stream the
debug log (color-coded by [ERROR]/[WARN]/[INFO]/[DEBUG]). Ctrl+C exits.
Needs pyserial: run install.sh then "source ./export.sh" (macOS/Linux),
or install.ps1 then ". ./export.ps1" (Windows PowerShell).

  tdf.py monitor
  tdf.py flash monitor
  tdf.py monitor --port /dev/cu.usbmodemXXXX --baud 115200 --no-color
""",
    "clean": """tdf.py clean

Delete the current project's build/ directory. Does not touch the board.

  tdf.py clean
""",
    "clean-device": """tdf.py clean-device

Mass-erase both flash banks on the connected chip. The board will not
boot until you flash again. Asks for confirmation unless -y.

  tdf.py clean-device
  tdf.py clean-device -y
""",
    "create-project": """tdf.py create-project <name>

Scaffold <name>/Sw/ (CMakeLists.txt, Src/main.c, empty prv_user_code.c)
inside this repo. Cannot be chained with other commands.

  tdf.py create-project MyDemo
  tdf.py create-project MyDemo --dir Getting_Started
""",
    "help": """tdf.py help [command]

Overview of every command, or the detailed page for one of them.

  tdf.py help
  tdf.py help flash
  tdf.py --help
""",
}


def _take_value(argv, i, flag):
    """Read FLAG VALUE or FLAG=VALUE. Returns (value, next_index)."""
    arg = argv[i]
    prefix = flag + "="
    if arg.startswith(prefix):
        value = arg[len(prefix):]
        if not value:
            die(f"{flag} needs a value")
        return value, i + 1
    if arg != flag:
        die(f"internal: expected {flag}")
    if i + 1 >= len(argv):
        die(f"{flag} needs a value")
    return argv[i + 1], i + 2


def parse_pipeline(argv):
    """Split argv into an ordered command list plus a shared options namespace.

    Flags may appear before, between, or after commands:
      tdf.py flash monitor --port /dev/cu.usbmodemXXXX
      tdf.py build -DTIREMO_DEBUG_LEVEL=2 flash
    """
    args = argparse.Namespace(
        define=[],
        reconfigure=False,
        port=None,
        baud=115200,
        no_color=False,
        yes=False,
        name=None,
        dir=None,
        help_topic=None,
        _built=False,
    )
    commands = []
    i = 0
    while i < len(argv):
        arg = argv[i]
        if arg in ("-h", "--help"):
            print_overview()
            sys.exit(0)
        if arg in COMMANDS:
            if arg in commands:
                die(f"'{arg}' is already in the pipeline")
            commands.append(arg)
            i += 1
            if arg == "create-project":
                if (i < len(argv) and
                        not argv[i].startswith("-") and
                        argv[i] not in COMMANDS):
                    args.name = argv[i]
                    i += 1
            elif arg == "help":
                if (i < len(argv) and
                        not argv[i].startswith("-")):
                    args.help_topic = argv[i]
                    i += 1
            continue
        if arg == "-D":
            if i + 1 >= len(argv):
                die("-D needs KEY=VALUE")
            args.define.append(argv[i + 1])
            i += 2
            continue
        if arg.startswith("-D") and len(arg) > 2:
            args.define.append(arg[2:])
            i += 1
            continue
        if arg == "--reconfigure":
            args.reconfigure = True
            i += 1
            continue
        if arg == "--port" or arg.startswith("--port="):
            args.port, i = _take_value(argv, i, "--port")
            continue
        if arg == "--baud" or arg.startswith("--baud="):
            raw, i = _take_value(argv, i, "--baud")
            try:
                args.baud = int(raw)
            except ValueError:
                die(f"--baud needs an integer, not {raw!r}")
            continue
        if arg == "--no-color":
            args.no_color = True
            i += 1
            continue
        if arg in ("-y", "--yes"):
            args.yes = True
            i += 1
            continue
        if arg == "--dir" or arg.startswith("--dir="):
            args.dir, i = _take_value(argv, i, "--dir")
            continue
        die(
            f"unknown argument: {arg}\n"
            "       commands: " + ", ".join(COMMANDS) + "\n"
            "       try: tdf.py help"
        )

    if not commands:
        print_overview()
        sys.exit(2)
    if "help" in commands:
        if len(commands) != 1:
            die("help cannot be chained with other commands; use tdf.py help [command]")
        if args.help_topic is not None and args.help_topic not in COMMAND_PAGES:
            die(
                f"no help for {args.help_topic!r}\n"
                "       commands: " + ", ".join(COMMANDS)
            )
    if "create-project" in commands:
        if len(commands) != 1:
            die("create-project cannot be chained with other commands")
        if not args.name:
            die(f"create-project needs a project name, e.g. {_prog()} create-project MyDemo")
    return commands, args


def main():
    commands, args = parse_pipeline(sys.argv[1:])
    for name in commands:
        COMMANDS[name][1](args)


if __name__ == "__main__":
    main()
