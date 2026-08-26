# Changelog

All notable changes to this tutorial repository are documented here. Format
loosely follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
entries are grouped under `Unreleased` until the curriculum's first tagged
release.

## [Unreleased]

### Added

- Initial repository scaffold: `Getting_Started`, `Sensors_And_Storage`,
  `ESP32_Connectivity`, `Cloud_Messaging`, `AWS_IoT_Core`,
  `Production_Firmware`, and `Developer_Contributed`, each with a README and
  numbered tutorial folders following the Feature/Design Tutorials split.
- Tutorial content is scaffolded (introduction, learning goals, prerequisites,
  reference-implementation links, step outline, done-when criteria) but not
  yet fully written up step by step — see each tutorial's `README.md`.
- Tiremo Cortex logo (light/dark) and brand banner strip on every README,
  under `images/`.
- Turkish translation of the top-level README (`docs-tr/README.md`), linked
  from a language switcher at the top of the English one.
- Shared vendor SDK at [`Generation/`](Generation/) (ABOV AUDK32 for the
  A34G43x — CMSIS, HAL, linker script, prebuilt OpenOCD for Linux/macOS/
  Windows; BSD-3-Clause, see `Generation/README.md`). Checked in once,
  ~107 MB, referenced by relative path from every tutorial's `Sw/`
  Makefile rather than duplicated per tutorial.
- `Getting_Started/01-Hello_Blink/Sw/`: this repo's first real, buildable
  tutorial solution — trimmed Makefile/build.sh/flash.sh (Linux/macOS/
  Windows-aware), minimal `main.c`/`prv_user_code.c`, and a trimmed
  `board_config.h` plus `Tiremo_Hal` GPIO subset. Verified to build and
  link clean; flashing not yet confirmed on real hardware (no board
  attached while writing it).
- `01-Hello_Blink`'s README expanded from a step outline into a full
  walkthrough (an "Understanding the GPIO HAL" section plus a 5-step,
  code-annotated build/flash walkthrough), with a matching Turkish
  translation at `docs-tr/Getting_Started/01-Hello_Blink/README.md` —
  the first per-tutorial (not just top-level) translation, linked via a
  language switcher on both pages.
- `Getting_Started/02-SysTick_Timing/Sw/`: second real, buildable
  solution — adds `Tiremo_Hal/tiremo_hal_systick` + `Common/tiremo_systick`
  on top of Tutorial 01's GPIO HAL (no new vendor sources needed), two
  LEDs blinking at independent, non-blocking rates from one loop. Full
  walkthrough + Turkish translation written alongside it, same pattern as
  Tutorial 01. The first tutorial in this repo confirmed end to end on
  real hardware: flashed to a connected ABOV A34G43A Starter Kit over
  SWD, both LEDs visually confirmed blinking at clearly different rates
  (`LED1_INTERVAL_MS`/`LED2_INTERVAL_MS` tuned to `1000`/`5000` on real
  hardware, docs updated to match).
- Windows `make` gap called out explicitly in Prerequisites (Git for
  Windows does not bundle `make`; needs a separate install via
  Chocolatey, MSYS2, or WSL's `apt`).
- `Getting_Started/03-Debug_UART_Logging/Sw/`: third real, buildable
  solution — adds `Tiremo_Hal/tiremo_hal_uart` + `Common/tiremo_debug` on
  top of Tutorial 02's set (main.c changes for the first time, to call
  the vendor's `PRV_UART_Init()`; new vendor deps `hal_uart.c`,
  `hpl_dma.c`, `hpl_nmi.c`). Full walkthrough + Turkish translation
  written alongside it. Flashed and verified on real hardware over SWD
  (`** Verified OK **`); actual UART output not yet confirmed over a
  serial terminal.
- Found and fixed a real bug in `tiremo_debug.h`'s compile-time log-level
  gating: the enum level constants were never visible to the
  preprocessor, so every `#if (TIREMO_DEBUG_LEVEL >= ...)` guard silently
  evaluated `0 >= 0` (always true) regardless of `-DTIREMO_DEBUG_LEVEL=N`
  — `TIREMO_LOG_DEBUG` compiled in unconditionally even at `WARN`.
  Verified broken via `arm-none-eabi-gcc -E`, fixed by re-declaring each
  level as a plain numeric macro after the enum, verified fixed via
  `strings`/binary size on the actual built `.elf`. Documented in
  `03-Debug_UART_Logging`'s README as "A Real Bug".
- `Getting_Started/04-Button_Read_And_Debounce/Sw/`: fourth real,
  buildable solution — adds `Drivers/button` (this repo's first tutorial
  to touch the `Drivers/` layer, not just `Tiremo_Hal`/`Common`) on top
  of Tutorial 03's set. No new vendor sources needed (the button is pure
  GPIO, already covered by `hal_pcu.c`); `main.c` unchanged from
  Tutorial 03. Adds a debounce + long-hold-threshold state machine in
  `prv_user_code.c`, built in application code on top of the reference
  driver's raw, undebounced `TIREMO_Button_IsPressed()` — documented in
  the README's "Understanding Debounce" section, including why the
  reference driver itself has no debounce logic at all (its one real
  caller, the factory-reset hold check, only cares about a 10-second
  sustained press, four orders of magnitude longer than any mechanical
  bounce window). Full walkthrough + Turkish translation written
  alongside it. Built clean and flashed + OpenOCD-verified on real
  hardware over SWD; the new button-press/debounce behavior itself is
  explicitly flagged as unconfirmed, since it needs a human finger on
  the board and a serial terminal — an attempt to capture UART output
  directly from this session (the board enumerates a USB serial port)
  didn't succeed in this sandboxed environment.
- Added `tdf.py monitor`: resets the connected board over SWD (a new
  `reset-device` CMake target, added to all four tutorials'
  `CMakeLists.txt` next to `erase-device` — `init; reset run; shutdown`,
  no reprogramming) and streams the UART log, color-coded by level
  (`[ERROR]` red, `[WARN]` yellow, `[INFO]` green, `[DEBUG]` cyan). This
  is the one `tdf.py` subcommand that isn't pure CMake delegation —
  reading a live UART needs `pyserial`, since CMake/OpenOCD have no
  concept of a serial port. `pyserial` goes into a new, dedicated
  `tdf_venv/` (created by `install.sh`) rather than a plain `pip
  install`, because most current Python installs (Homebrew, most Linux
  distros) now refuse system-wide pip installs outright (PEP 668,
  "externally-managed-environment" — hit for real while building this);
  `export.sh` activates that venv automatically so `tdf.py`'s own
  `#!/usr/bin/env python3` resolves to it. Also fixed a real, separate
  bug found while touching `install.sh`: every `confirm_and_run` call
  (declining a prompt returns 1) was invoked bare under `set -e`, so
  answering "no" to *any* install prompt silently killed the whole
  script instead of just skipping that one tool — now guarded with
  `|| true` at every call site. This closes a verification gap flagged
  since Tutorial 03: actual UART output is now genuinely captured and
  confirmed for the first time this session — `tdf.py monitor` was run
  for real against the connected board, showed the expected boot line
  and continuous `LED1`/`LED2 toggle @ ... ms` lines, and the
  reset-then-monitor ordering was independently verified by watching
  the logged timestamp restart near 0 ms across two consecutive runs.
  One honest, documented edge case: a reset can occasionally tear a
  UART line in half right at the reset boundary (observed once) —
  harmless, not engineered around. `03-Debug_UART_Logging`'s and
  `04-Button_Read_And_Debounce`'s READMEs (English + Turkish) updated to
  reflect the now-confirmed UART capture; button-press-specific behavior
  in Tutorial 04 remains explicitly flagged as unconfirmed, since that
  still needs a human finger on the board.
- `Getting_Started/05-Ten_LED_Pattern_Driver/Sw/`: fifth real, buildable
  solution — brings in the full 10-LED bank (`Drivers/led`, copied
  verbatim) and adds an original, non-reference `Common/tiremo_led_pattern.c`:
  a non-blocking, per-LED solid/blink/pulse state machine, ten
  independent instances of it running off one shared SysTick. No
  reference implementation exists for the pattern engine itself — the
  production driver only has raw on/off/toggle, LED patterns are
  actually driven by cloud-side shadow state out of this repo's scope —
  so the engine is original code, documented in the README as a direct
  architectural contrast with Tutorial 04's debounce logic: generic,
  reusable policy goes in `Common/` next to `tiremo_debug.c`, one-off
  policy specific to a single caller stays in application code. All ten
  LEDs cycle SOLID → BLINK → PULSE together, each on a staggered
  interval (300–1650 ms) so the cascade visibly proves independent
  addressability instead of looking like one LED wired ten times;
  cycling auto-advances every 8 s or immediately on a debounced USER
  button press (reusing Tutorial 04's exact debounce state machine,
  now returning a press-edge signal instead of only logging). No PWM
  on this HAL, so `PULSE` is a 20/80 duty-cycle heartbeat, not a
  brightness fade — called out explicitly rather than left implicit.
  Built clean, flashed and OpenOCD-verified over SWD, and — using
  `tdf.py monitor` — genuinely watched live: `scene -> SOLID @ 3 ms`,
  `scene -> BLINK (auto) @ 8003 ms`, `scene -> PULSE (auto) @ 16003 ms`,
  exactly matching the designed 8000 ms interval from real hardware
  timestamps. What's still unconfirmed: whether all ten LEDs are
  visibly staggered and correctly wired, and whether a real button
  press logs `(button)` and advances early — both need eyes on the
  board and a human finger. Full walkthrough + Turkish translation
  written alongside it.

### Changed

- Removed every doc reference to `referance_only/` (private, gitignored,
  never shipped to a clone of this repo). Each tutorial's former
  "Reference Implementation" section is now a "Solution" section, honestly
  stating that no solution has been written yet; the eventual plan is for
  each tutorial to carry its own local `Sw/` folder once one exists.
- Migrated all three existing `Sw/` solutions (01, 02, 03) from
  hand-written Makefile + `build.sh` + `flash.sh` to CMake + Ninja, with
  one shared [`cmake/toolchain-arm-none-eabi.cmake`](cmake/toolchain-arm-none-eabi.cmake)
  at the repo root. Two concrete wins: `cmake -G Ninja` runs natively in
  cmd.exe/PowerShell, removing the Git Bash/WSL requirement (and the
  Git-Bash-doesn't-bundle-`make` gotcha) documented for Windows users;
  and `add_executable()` sidesteps the `..`-path object-escaping Make bug
  entirely, since CMake manages object file placement for out-of-tree
  sources internally. Flashing is now `cmake --build build --target
  flash`, a custom target doing the same OS-detection `flash.sh` did, in
  pure CMake. Tutorial 03's `TIREMO_DEBUG_LEVEL` override moved from a
  one-shot `make`-time variable to a CMake cache variable set at
  configure time (`cmake -B build -DTIREMO_DEBUG_LEVEL=2`), which
  persists across rebuilds until reconfigured — a real behavioral
  difference, documented in that tutorial's README. All three re-verified
  after the migration: identical `.text`/`.data`/`.bss` sizes to the
  Make-based builds, and all three re-flashed and OpenOCD-verified on
  real hardware. Every tutorial's README updated to match, including a
  "Why CMake, not a hand-written Makefile" note in 01-Hello_Blink's
  Solution section.
- Added [`tools/tdf.py`](tools/tdf.py) ("Tiremo Development Framework"),
  a thin Python CLI wrapping the CMake commands above — same role
  `idf.py` plays for ESP-IDF projects. `tdf.py build` / `flash` / `clean`
  / `clean-device` (subcommands), plus [`install.sh`](install.sh) (checks
  for / offers to install `arm-none-eabi-gcc`/`cmake`/`ninja`, asks
  before running anything) and [`export.sh`](export.sh) (adds `tools/` to
  `PATH` for the current shell). `tdf.py` self-locates the repo from its
  own file path, so `python tools/tdf.py build` works with or without
  `export.sh` ever being run. Added a matching `erase-device` CMake
  target to all three tutorials' `CMakeLists.txt` (mass-erases the whole
  chip, both banks) for `tdf.py clean-device` to call. All four
  subcommands verified for real against 01-Hello_Blink's `Sw/` on a
  connected board, including a full `clean-device` → blank-flash (`pc:
  0xfffffffe`) → `flash` → working-again cycle. Every tutorial's Solution
  section and build/flash walkthrough steps updated to lead with `tdf.py`
  instead of raw `cmake` invocations, in both English and Turkish.
