<p class="sphinxhide" align="right"><a>English</a> | <a href="../../docs-tr/Getting_Started/01-Hello_Blink/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Hello Blink</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Hello Blink

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

Before anything talks to WiFi or AWS, you need a build that flashes and a board that visibly does something. This tutorial blinks LED1 to prove the toolchain, OpenOCD wiring, and board bring-up all work.

## You Will Learn

- Active-low LED wiring and the `TIREMO_BOARD_LED*` pin macros
- The `Tiremo_Hal` GPIO wrapper (`tiremo_hal_gpio.c`)
- `tdf.py build` / `flash` / `clean-device` — this repo's CLI tool, and what it actually does underneath (CMake + Ninja + OpenOCD)

## Prerequisites

- `arm-none-eabi-gcc`, `cmake` (3.20+), and `ninja` on `PATH` — run [`./install.sh`](../../install.sh) (macOS/Linux), [`install.ps1`](../../install.ps1) (Windows PowerShell), or [`install.bat`](../../install.bat) (Windows cmd.exe) from the repo root to check for these and get install commands for what's missing (built and tested here against Arm GNU Toolchain 15.2.1, CMake 4.3, Ninja 1.13; anything reasonably recent should work)
- **Windows:** the commands below run identically in cmd.exe, PowerShell, bash, or zsh — no Git Bash or WSL required. (An earlier version of this repo used `build.sh`/`flash.sh` bash scripts and needed one of those; the switch to CMake was made specifically to remove that requirement — see [Solution](#solution) below.) The setup scripts come in native flavors now: `install.ps1` / `export.ps1` for PowerShell (dot-source the second: `. .\export.ps1`), `install.bat` / `export.bat` for cmd.exe. If PowerShell blocks `.ps1` scripts, run once: `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`.
- An ABOV A34G43A Starter Kit connected over its onboard SWD debugger

> **Windows: build verified, hardware flash still pending.** `tdf.py build` runs end-to-end on Windows 11 (Arm GNU Toolchain 14.2.1, CMake 4.4.0, Ninja 1.13.2, installed via `install.ps1` + winget) — every source compiles and `HelloBlink` links to a 5156-byte `.text` image, matching the macOS build. The Windows setup scripts (`install.ps1`/`.bat`, `export.ps1`/`.bat`) and the `tools/tdf.cmd` shim are in place. `tdf.py flash` picks the bundled `win32` OpenOCD via `CMAKE_HOST_SYSTEM_NAME` but hasn't been exercised against a real board on Windows yet — if you flash it, please report back.

## Understanding the GPIO HAL

Every peripheral driver in this codebase — this tutorial's LED, and later
ones' accelerometer interrupt pins, I2C bus, USER button — goes through
`Tiremo_Hal` instead of touching ABOV's vendor PCU API directly. That's
one file's worth of insulation: if this board ever moved to a different
MCU vendor, only `tiremo_hal_gpio.c` would need to change, not every
driver built on top of it.

**The API surface** ([`Tiremo_Hal/Include/tiremo_hal_gpio.h`](./Sw/Tiremo_Hal/Include/tiremo_hal_gpio.h)) is a
small enum-driven wrapper. Ports and pins are generic identifiers:

```c
typedef enum { TIREMO_HAL_GPIO_PORT_A = 0, ..., TIREMO_HAL_GPIO_PORT_G } TIREMO_HAL_GPIO_Port_t;
typedef enum { TIREMO_HAL_GPIO_PIN_0 = 0, ..., TIREMO_HAL_GPIO_PIN_15 } TIREMO_HAL_GPIO_Pin_t;
```

Hello Blink uses exactly three functions from it
([`Tiremo_Hal/Src/tiremo_hal_gpio.c`](./Sw/Tiremo_Hal/Src/tiremo_hal_gpio.c)):

```c
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetMode(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Mode_t mode);
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_WritePin(TIREMO_HAL_GPIO_Port_t port,
                                                  TIREMO_HAL_GPIO_Pin_t pin,
                                                  TIREMO_HAL_GPIO_Level_t level);
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_TogglePin(TIREMO_HAL_GPIO_Port_t port,
                                                   TIREMO_HAL_GPIO_Pin_t pin);
```

Inside, each one just translates the generic enum into ABOV's vendor PCU
type with a plain cast, then calls straight into the vendor HAL:

```c
vendorStatus = HAL_PCU_SetInOutMode((PCU_ID_e)port, (PCU_PIN_ID_e)pin, vendorMode);
```

That's the entire abstraction — no register access, no magic, just a
translation layer with a stable, vendor-independent name on one side.

**A gotcha worth knowing:** `TIREMO_HAL_GPIO_TogglePin()` is not an atomic
hardware toggle. Look at its implementation and it's just a read followed
by a write:

```c
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_TogglePin(...)
{
    TIREMO_HAL_GPIO_Level_t level;
    TIREMO_HAL_GPIO_ReadPin(port, pin, &level);
    level = (level == TIREMO_HAL_GPIO_LEVEL_HIGH) ? TIREMO_HAL_GPIO_LEVEL_LOW
                                                   : TIREMO_HAL_GPIO_LEVEL_HIGH;
    return TIREMO_HAL_GPIO_WritePin(port, pin, level);
}
```

Fine for a 1 Hz blink. Worth remembering later if a tutorial ever toggles
a pin from an interrupt handler, where a read-modify-write race is a real
bug, not a theoretical one.

**Active-low wiring:** LED1 is wired so `LEVEL_LOW` lights it and
`LEVEL_HIGH` turns it off — backwards from what "HIGH = on" intuition
suggests. [`Board/board_config.h`](./Sw/Board/board_config.h) documents this with
`TIREMO_BOARD_LED_ACTIVE_LOW`, but — worth being precise about this,
since it's a common source of confusion reading the code later — that
macro is descriptive only in this tutorial's code. Nothing reads its
value to invert logic; `prv_user_code.c` just hardcodes `LEVEL_HIGH`
directly as the "off" state, matching what the macro documents rather
than consulting it. (This mirrors how the full reference firmware's own
LED driver works, too.)

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation. First time only, from the repo root:

```sh
# macOS / Linux
./install.sh          # checks for / offers to install arm-none-eabi-gcc, cmake, ninja
source ./export.sh     # puts tdf on PATH for this shell session (optional but recommended)
```

```powershell
# Windows (PowerShell)
.\install.ps1
. .\export.ps1          # dot-sourced, so PATH / venv / completion changes stick
```

```bat
:: Windows (cmd.exe)
install.bat
export.bat
```

Then, from `Sw/`:

```sh
cd Sw
tdf build
tdf flash
```

(`tdf.py` still works as an alias.)

**What `tdf.py` actually is:** a thin Python wrapper at [`tools/tdf.py`](../../tools/tdf.py) — the same role `idf.py` plays for ESP-IDF. It owns no OpenOCD or compiler logic itself; `tdf build` is exactly `cmake -G Ninja -B build -DCMAKE_TOOLCHAIN_FILE=<repo>/cmake/toolchain-arm-none-eabi.cmake && cmake --build build`, and `tdf flash` adds `cmake --build build --target flash` after it. It self-locates the repo from its own file path, so `python tools/tdf.py build` (`python tools\tdf.py build` on Windows) works from anywhere, with or without `export.sh` — that script only adds the convenience of typing bare `tdf` (or `tdf.py`; on Windows, `tools/tdf.cmd`). There's also `tdf clean` (removes `build/`) and `tdf clean-device` (mass-erases the connected board's chip flash — asks first). `tdf monitor`, added once there was a UART to actually read (see [04-Button_Read_And_Debounce's Solution](../04-Button_Read_And_Debounce/#solution)), resets the board over SWD and streams the log — it's the one subcommand that isn't pure CMake delegation, since it also needs `pyserial`, installed into a dedicated `tdf_venv/` by `install.sh` / `install.ps1` / `install.bat` and activated by the matching `export.*`.

It builds against this repo's shared vendor SDK ([`Generation/`](../../Generation/) at the repo root — see that folder for why it's checked in once, shared, instead of copied per tutorial) and links a single ~5 KB image: no sensors, no radios, no debug UART, just port init, clock init, and GPIO.

**Why CMake, not a hand-written Makefile:** this tutorial was originally built with a plain Makefile — matching the reference firmware's own build system — and it worked. But it required Git Bash or WSL on Windows (plus a separate `make` install, since Git Bash doesn't bundle one), and hit a real bug: a vendor source path containing `..` components, fed through the naive `$(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(SRCS))` pattern every one of these tutorials' Makefiles used, let those `..` escape `build/obj/` and write object files into a stray directory tree outside the build folder entirely. The Make version worked around it with a `vpath` trick; CMake's `add_executable()` sidesteps the whole problem, since it manages object file placement for out-of-tree sources internally. Between removing the Windows shell dependency and removing a class of bug outright, CMake won — see [`Sw/CMakeLists.txt`](./Sw/CMakeLists.txt). Even CMake's own invocation has a learning curve, though (`cmake -G Ninja -B build -DCMAKE_TOOLCHAIN_FILE=...` is not something anyone should have to memorize or retype for every tutorial), which is what `tdf.py` is for.

**Honesty check:** `tdf.py build` compiles and links clean (verified on this machine, Arm GNU Toolchain 15.2.1, CMake 4.3.3, Ninja 1.13.2, macOS/arm64 — identical `.text`/`.data`/`.bss` sizes to the earlier Make-based build). `tdf.py flash` completed a full program-and-verify cycle over SWD against a real, connected ABOV A34G43A Starter Kit (`** Verified OK **`) — this tutorial is now genuinely flashed, not just linked. `tdf.py clean-device` was also exercised for real: erased the board (confirmed by the post-erase halt reading back `pc: 0xfffffffe`, the classic blank-flash signature), then `tdf.py flash` recovered it to a normal boot. `BLINK_DELAY_LOOPS` in `Sw/Src/prv_user_code.c` is a hand-picked estimate, not a calibrated time base — if the blink rate is visibly off on your board, adjust it up or down.

## Walkthrough

### 1. Board config — name the pin

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h) gives LED1's physical location a name the
rest of the code can use instead of a bare port/pin pair:

```c
#define TIREMO_BOARD_LED_ACTIVE_LOW    (1U)
#define TIREMO_BOARD_LED1_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED1_PIN          TIREMO_HAL_GPIO_PIN_4
```

Every later tutorial that adds a new piece of hardware follows this same
pattern: define what it's called and where it lives here, then reference
the name everywhere else — never a raw port/pin pair outside this file.

### 2. Entry point — what boots before your code runs

[`Sw/Src/main.c`](./Sw/Src/main.c) is a deliberately trimmed version of the vendor-generated
template entry point:

```c
int main(void)
{
    PRV_PORT_Init();      /* vendor-generated pin muxing, static inline in startup_a34g43x.h */
    PRV_SCU_CLK_Init();   /* vendor-generated clock config, unmodified from the template */
    PRV_USER_Code();      /* this tutorial's code -- see step 3 */
    return 0;
}
```

The full vendor template also calls `PRV_UART_Init()`, `PRV_ADC_Init()`,
`PRV_Timer1_Init()`, and `PRV_I2C_Init()` here, plus optional debug-UART
setup. This tutorial doesn't need any of that yet — UART logging is
[Tutorial 03](../03-Debug_UART_Logging/) — so `main.c` only keeps the two
calls a GPIO-only program actually needs: pin muxing and clock setup.
Leaving out unused peripheral init isn't just tidiness — it's also why
[the CMakeLists.txt below](#4-build--one-target-two-source-groups)
doesn't need to compile `hal_uart.c`, `hal_adc.c`, `hal_timer1.c`, or
`hal_i2c.c` at all.

### 3. The blink loop

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) is the whole program:

```c
#define BLINK_DELAY_LOOPS (600000UL)

static void prvDelay(void)
{
    volatile uint32_t i;
    for (i = 0UL; i < BLINK_DELAY_LOOPS; i++) { /* empty */ }
}

void PRV_USER_Code(void)
{
    TIREMO_HAL_GPIO_SetMode(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                            TIREMO_HAL_GPIO_MODE_OUTPUT_PP);
    TIREMO_HAL_GPIO_WritePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                             TIREMO_HAL_GPIO_LEVEL_HIGH);  /* start off */

    for (;;)
    {
        TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN);
        prvDelay();
    }
}
```

`volatile` on the loop counter is load-bearing, not decoration: at `-Og`
the compiler is otherwise free to notice the loop has no observable
effect and delete it entirely. This whole function is exactly what
[Tutorial 02](../02-SysTick_Timing/) replaces — same LED, same idea, but
timed off the SysTick hardware timer instead of a hand-tuned empty loop,
so two LEDs can blink at independent, accurate rates from one loop.

### 4. Build — one target, two source groups

[`Sw/CMakeLists.txt`](./Sw/CMakeLists.txt) declares one `add_executable(HelloBlink ...)`
listing this tutorial's own sources (`Src/`, `Tiremo_Hal/`) alongside six
vendor files pulled straight from the shared
[`Generation/`](../../Generation/) SDK three directories up
(`../../../Generation/AUDK32_A34xxxx-1.0.12`) — no separate source-group
bookkeeping needed, unlike this tutorial's original Make-based version
(see the **Why CMake** note in [Solution](#solution) above for exactly
what that involved). `target_include_directories()` and
`target_compile_definitions()` carry the same `-I`/`-D` flags the old
Makefile passed by hand; the cross-compiler itself (flags, CPU target,
linker specs) lives once in [`cmake/toolchain-arm-none-eabi.cmake`](../../cmake/toolchain-arm-none-eabi.cmake)
at the repo root, shared by every tutorial rather than repeated in each
`CMakeLists.txt`.

### 5. Flash

```sh
tdf.py build   # -> build/HelloBlink (.elf) + .bin, size report
tdf.py flash
```

The `flash` custom target in `CMakeLists.txt` (which `tdf.py flash` just
runs via `cmake --build build --target flash`) picks the matching
prebuilt OpenOCD binary for your OS (Linux/macOS/Windows all ship one in
the vendor SDK, detected via `CMAKE_HOST_SYSTEM_NAME`/`_PROCESSOR`) and
programs the single monolithic image at address 0. There's also a
`erase-device` target (`tdf.py clean-device`) that mass-erases the whole
chip instead — useful for proving to yourself that a board is genuinely
blank before flashing it fresh. See [Prerequisites](#prerequisites)
above for the Windows-specific setup, and the **Honesty check** in
[Solution](#solution) above for what has and hasn't actually been
verified on real hardware.

## Done When

LED1 blinks at 1 Hz after a fresh flash, with no debugger attached.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
