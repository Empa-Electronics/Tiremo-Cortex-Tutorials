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
- Added `tdf.py create-project <name> [--dir PARENT_DIR]`: scaffolds a
  new, empty, buildable `<name>/Sw/` from templates at
  `tools/templates/new_project/` (`CMakeLists.txt`, `Src/main.c`, an
  empty `Src/prv_user_code.c`) — deliberately minimal, no
  `board_config.h`/`Tiremo_Hal`, meant to be a blank starting point you
  copy tutorial modules into as needed. Unlike every tutorial's
  hand-written `CMakeLists.txt`, which hardcodes the fixed
  three-`../`-levels-deep path back to `Generation/` (every
  `Getting_Started/NN-Name/Sw/` sits at the same depth), this command
  computes that relative path at generation time for wherever the new
  project actually lands, so it works from anywhere inside the repo —
  verified for real by scaffolding a throwaway project both outside the
  repo (correctly rejected: `New projects need to stay inside the
  repo...`) and inside it at a non-standard depth, where the generated
  `CMakeLists.txt`'s `../../../Generation/...` path, `tdf.py build`
  (clean, `.text` 4636 bytes — smaller than every tutorial, since it
  skips the GPIO HAL entirely), and `tdf.py flash` (`** Verified OK
  **`) all worked against the connected board on the first try. Also
  validates the project name as a legal CMake target identifier and
  refuses to overwrite an existing folder. Root README and
  `docs-tr/README.md` updated with the command; noted as a natural fit
  for a future "Creating a New Project" tutorial, not yet written.
- Added [`Hardware.md`](Hardware.md) (+ Turkish translation): a
  board-level reference page, diagrams and a pinout table only, no
  firmware. Two Mermaid diagrams — a board block diagram (MCU, ten
  LEDs, USER button, the shared SHT40/LIS2DE12 I2C bus, the MP23ABS1
  microphone, the internal supply-voltage monitor, the ESP32 WiFi
  module, and the onboard SWD/UART0 debug probe) and a host-connection
  diagram (one USB cable carrying both the SWD and UART0 paths) — plus
  a full port/pin table. Every pin and address transcribed from the
  reference firmware's `board_config.h`, not guessed; cross-checked for
  internal consistency (no two peripherals claim the same pin) and
  against each sensor's own driver header for anything board_config.h
  might have missed. Also fixed a real, unrelated bug found along the
  way: `tools/__pycache__/tdf.cpython-314.pyc` had been committed by
  accident in an earlier commit — untracked it (`git rm --cached`,
  file kept on disk) and added `__pycache__/`/`*.pyc` to `.gitignore`.
- `Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/`: first
  real, buildable solution outside `Getting_Started`, and the first
  tutorial in `Sensors_And_Storage` — starts a fresh, minimal `Sw/`
  (SysTick + debug UART + the new I2C HAL only, no LED/button carried
  forward from `Getting_Started`, since this tutorial doesn't use them).
  Brings in `Tiremo_Hal/tiremo_hal_i2c` (copied verbatim) and the
  vendor's `PRV_I2C_Init()` (`user_i2c.c`/`user_i2c_isr.c`, scoped to
  exactly `I2C_ID_2`); new vendor dependency `hal_i2c.c` found by
  reading `tiremo_hal_i2c.c`'s own includes, not guessed. The HAL has
  no `Probe()` function, so this tutorial builds one from a one-byte
  `TIREMO_HAL_I2C_Receive()` — a read, deliberately, so the transaction
  can never write to (and potentially corrupt) a sensor's internal
  state — documented in the README's "Understanding the Probe" section
  alongside why a bare ACK/NACK is the right level of protocol
  knowledge for a *bus* bring-up tutorial specifically. `CMakeLists.txt`
  is the first to need `../../../../Generation/...` (four `../`, not
  three) since `Sensors_And_Storage/Feature_Tutorials/<name>/Sw/` sits
  one directory deeper than every `Getting_Started` tutorial. Built
  clean on the first try (no missing-dependency iteration needed),
  flashed and OpenOCD-verified over SWD, and — genuinely confirmed live
  via `tdf.py monitor` against the real connected board — both `SHT40
  (0x44): ACK` and `LIS2DE12 (0x19): ACK`, repeating every 5 s: this
  tutorial's entire "Done When" criterion, verified on real hardware,
  not simulated. Full walkthrough + Turkish translation written
  alongside it (the first Turkish translation under
  `docs-tr/Sensors_And_Storage/`).
- `Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw/`:
  first real I2C *device* driver in this repo (Tutorial 01 only probed
  the bus). Reuses 01's SysTick + debug UART + I2C HAL + `PRV_I2C_Init()`
  with no new vendor sources. Production firmware wraps Sensirion's C
  library; this tutorial implements the SHT4x protocol directly in
  `Drivers/sht40` so the command/response stays visible: write `0xFD`,
  STOP, wait 10 ms, read six bytes, CRC-8 (`0x31` / init `0xFF`) on
  each 2-byte word, integer milli-°C / milli-%RH conversion
  (`21875*ticks >> 13` / `15625*ticks >> 13`) with RH clamped to
  `[0, 100000]`. Documented in the README's "Understanding the SHT4x
  Protocol" section alongside why this is `Transmit`+`Receive`, not
  `WriteRead()` (repeated START would interrupt the conversion — that's
  the LIS2DE12 tutorial). 1 Hz sample loop in `prv_user_code.c` only;
  driver never formats strings. Built clean (`.text` 15660 bytes),
  flashed and OpenOCD-verified over SWD, and genuinely confirmed live
  via `tdf.py monitor`: repeating `SHT40 T: 33.282 C` / `SHT40 RH:
  52.431 %RH` (stable within sensor noise; ~33 °C is kit self-heat, not
  a conversion bug). Full walkthrough + Turkish translation written
  alongside it.
- `Sensors_And_Storage` feature tutorials 03–07 now have real `Sw/`
  solutions, using the gitignored reference firmware
  (`referance_only/Tiremo-Secure-Iot-Sw`) as the source for HAL wrappers
  and the production mic/battery/clock/flash drivers:
  - **03-LIS2DE12**: register-level driver (not ST's PID library) over
    `WriteRead()`; `Configure(2g, 100 Hz)`; mg conversion
    `(raw16 * sens) / 256`. Built (`.text` 15708), flashed, WHO_AM_I=0x33,
    rest gravity `Z=1036 mg`. Axis-by-axis tilt not confirmed.
  - **04-Battery ADC**: `tiremo_hal_adc` + reference `tiremo_battery`
    (`AVDD = 1000*4096/vcoreRaw`). Built (14436), flashed, live
    `VCORE raw=2032 AVDD=2015 mV`. Not meter-checked.
  - **05-Microphone**: reference mic BSP/app copied verbatim (vendor
    ADC/Timer1/DMA). Built (18316), flashed, DMA `blocks` climbing,
    quiet ~24 dB. Talk-vs-silence not confirmed.
  - **06-Flash config**: reference `tiremo_hal_flash` + tutorial-sized
    magic/CRC32 persist (`bootCount` + label). Built (16120), flashed,
    `bootCount` 4→5 across reflash, corrupt CRC rejected.
  - **07-Wall clock**: reference `tiremo_clock` unchanged. Built (12344),
    flashed, `GetEpoch` +1 s per second after `SetEpoch(1704067200)`.
  English walkthroughs + Turkish translations written alongside each.
- `Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/`:
  first Design Tutorial solution — composes feature tutorials 02–06
  (SHT40, LIS2DE12, battery, mic, flash) into one 3 s sample loop and an
  8-deep CRC32 ring in data-flash sector 0 (`ENV1`, overwrites tutorial
  06's bootCount record). Boot dumps oldest-first `hist` lines, then
  `live` appends. Built (`.text` 29752), flashed, live `flags=0x07`
  (T/RH + accel + AVDD + mic dB); `seq` continued across SWD reset.
  English + Turkish walkthroughs written alongside it.
- `tiremo_banner` (`Sw/Common/`): Tiremo Cortex wordmark printed over UART
  at boot, as the marker that a tutorial is an integrated application.
  Design Tutorials only — Feature Tutorials stay minimal. 100 columns
  wide, traced from `images/tiremo_sub_brand_1_mono_black.png`. Bypasses
  `TIREMO_LOG_INFO` so no `[INFO]` prefix breaks the artwork, and is
  suppressed below INFO verbosity. Verified on hardware.

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
