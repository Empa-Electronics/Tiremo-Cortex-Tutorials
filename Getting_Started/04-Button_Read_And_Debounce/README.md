<p class="sphinxhide" align="right"><a>English</a> | <a href="../../docs-tr/Getting_Started/04-Button_Read_And_Debounce/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Button Read And Debounce</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Button Read And Debounce

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Getting Started · Difficulty: Easy***

## Introduction

The USER button is only used for one thing in the production firmware — a 10-second factory-reset hold — but that means you need clean, debounced press/hold detection before you can build that feature later. This tutorial adds the USER button on top of Tutorial 03's UART logging: every press, release, and long-hold crossing gets its own log line, debounced so a single physical press can't be misread as several.

## You Will Learn

- Reading a button through `Tiremo_Hal_GPIO`, wrapped by the `Drivers/button` layer — this repo's first tutorial to touch `Drivers/`, not just `Tiremo_Hal`/`Common`
- Why the reference button driver itself has **no debounce logic at all**, and why that's a deliberate, defensible choice for its one real caller
- A small debounce state machine, built in application code on top of the raw driver, using the same non-blocking `TIREMO_SysTick_GetMs()` idiom as Tutorial 02
- Detecting a long-hold threshold the instant it's crossed, without waiting for release — the same shape the reference firmware's factory-reset check uses
- How `tdf monitor` works — the repo tool command that resets the board over SWD and streams the UART log, color-coded by level, so you can watch button events live

## Prerequisites

- [03-Debug_UART_Logging](../03-Debug_UART_Logging/) — this tutorial's LED loop and logging are exactly Tutorial 03's, with button handling added
- `arm-none-eabi-gcc`, `cmake` (3.20+), and `ninja` on `PATH` (see [01-Hello_Blink's Prerequisites](../01-Hello_Blink/#prerequisites))
- To see the press/release/hold log lines: either `tdf monitor` (see **Solution** below — needs `pyserial`, which `./install.sh` / `install.ps1` / `install.bat` sets up in a dedicated `tdf_venv/`), or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1. No separate USB-to-serial adapter is needed: the board's onboard CMSIS-DAP probe provides a serial port over the same USB cable (for example `USB Serial Device (COMx)` on Windows).
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../Hardware.md)) connected to your PC with a USB cable, with the USER button accessible. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## Understanding Debounce (and Why the Reference Driver Doesn't Have Any)

A mechanical button doesn't transition cleanly from open to closed. The
contacts physically bounce for a few milliseconds, so a single press can
read as several rapid `LOW`/`HIGH`/`LOW` transitions before settling.
Poll a raw GPIO level fast enough and you'll see that noise as multiple
"presses" for one finger press.

Look at this tutorial's
[`Sw/Drivers/button/Src/tiremo_button.c`](./Sw/Drivers/button/Src/tiremo_button.c)
(copied verbatim from the reference driver), though, and
`TIREMO_Button_IsPressed()` is just this:

```c
level = prvButtonReadLevel(buttonId);   /* one raw TIREMO_HAL_GPIO_ReadPin() */
#if (TIREMO_BOARD_BUTTON_ACTIVE_LOW == 1U)
return (level == TIREMO_HAL_GPIO_LEVEL_LOW);
#else
return (level == TIREMO_HAL_GPIO_LEVEL_HIGH);
#endif
```

No filtering, no debounce timer, nothing. That's not an oversight — it's
because the driver's only real caller in the production firmware,
`TIREMO_APP_ServiceFactoryResetHold()`, only cares about one thing: has
the button been *continuously* pressed for a full 10,000 ms. A few
milliseconds of contact bounce right at the start of a press shifts the
effective start time by an amount that's completely lost in the noise of
a 10-second threshold — it can never turn a real 10 s hold into a false
negative, or a real short press into a false positive, because the
threshold is four orders of magnitude larger than the bounce window.
Debouncing at the driver level would be effort spent solving a problem
that specific caller doesn't have.

This tutorial's own loop *does* have that problem, though — it logs
every press and release as a distinct, discrete event, and bounce noise
would turn one physical press into a burst of spurious "pressed" /
"released" pairs a few milliseconds apart. So the debounce logic lives
where it's actually needed: in `PollButton()` inside this tutorial's
[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c), one layer above the
driver, exactly where the reference architecture puts policy that's
specific to one caller rather than generic to the peripheral.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation. From the repo root:

```sh
cd Getting_Started/04-Button_Read_And_Debounce/Sw
tdf build
tdf flash
```

(See [01-Hello_Blink's Solution](../01-Hello_Blink/#solution) for what
`tdf` is and how to get it on `PATH` via `./install.sh` + `source
./export.sh` — on Windows, `install.ps1` + `. .\export.ps1`, or
`install.bat` + `export.bat`. On macOS/Linux, `tdf.py` also works as an
alias; on Windows, use `tdf`.)

Same shared vendor SDK and CMake build shape as every prior tutorial
([`Generation/`](../../Generation/) at the repo root). No new vendor
sources were needed — the button is pure GPIO, already covered by
`hal_pcu.c` since Tutorial 01 — so the only new build input is this
tutorial's own `Drivers/button/Src/tiremo_button.c`, added to
`CMakeLists.txt` alongside Tutorial 03's UART/debug sources.

**Confirmed on real hardware:** `tdf build` compiles and links without
errors on macOS and Windows 11 (`.text` about 13 KB; the exact size
varies slightly with the toolchain version, and the linker prints the
same expected, harmless `_close` / `_lseek` / `_read` / `_write is not
implemented` warnings explained in [01-Hello_Blink's Solution](../01-Hello_Blink/#solution)).
`tdf flash` completed a full program-and-verify cycle over SWD on a
real, connected Tiremo Cortex board (`** Verified OK **`). To watch the
log, run:

```sh
tdf monitor
```

This resets the board over SWD (the `reset-device` CMake target,
`init; reset run; shutdown` — same OpenOCD, no reprogramming) and then
streams the log, color-coded by level (`[ERROR]` red, `[WARN]` yellow,
`[INFO]` green, `[DEBUG]` cyan — on Windows `tdf monitor` turns on the
console's ANSI processing so the colors render). Press Ctrl+C to exit.
Only one program can open the serial port at a time, so close any other
terminal (Tera Term, PuTTY, …) on that port first. `monitor` needs
`pyserial`, installed into a dedicated `tdf_venv/` by the install
scripts rather than a plain `pip install`, because most current Python
installs (Homebrew, most Linux distros) refuse system-wide pip installs
(PEP 668); `export` activates that venv automatically. Full setup,
first time only:

```sh
# macOS / Linux
./install.sh          # also offers to create tdf_venv/ + install pyserial into it
source ./export.sh     # activates tdf_venv/ for this shell session
```

```powershell
# Windows (PowerShell)
.\install.ps1
. .\export.ps1
```

```bat
:: Windows (cmd.exe)
install.bat
export.bat
```

**What you should see:** the boot line and continuous `LED1`/`LED2
toggle @ ... ms` lines in green, matching Tutorial 02/03's blink
pattern. Press the USER button and you get exactly one yellow
`[WARN] USER button: pressed` line per physical press (not several),
then a yellow `[WARN] USER button: released after N ms` line on
release. Hold it past 10 s and a green `[INFO] USER button: long hold
reached (10000 ms)` line appears the moment the threshold is crossed;
the release line then ends with `(was a long hold)`. For example, two
short presses on a real board:

```text
[INFO] LED1 toggle @ 37003 ms
[WARN] USER button: pressed
[INFO] LED1 toggle @ 38003 ms
[WARN] USER button: released after 1821 ms
[INFO] LED1 toggle @ 39003 ms
```

Press/release are logged at `WARN` and the long-hold line at `INFO`:
the press/release edges stand out in yellow against the constant green
LED-toggle lines. Tip: to hide the LED lines entirely, build with
`tdf build -DTIREMO_DEBUG_LEVEL=2` — only the `WARN` press/release lines
remain (the `INFO` long-hold line is compiled out too at that level).

## Walkthrough

### 1. Board config — adds the USER button

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h) keeps Tutorial
03's LED1/LED2 macros and adds the button's pin map, copied from the
reference `board_config.h`:

```c
#define TIREMO_BOARD_BUTTON_ACTIVE_LOW     (1U)
#define TIREMO_BOARD_BUTTON_COUNT          (1U)

#define TIREMO_BOARD_USER_BUTTON_PORT      TIREMO_HAL_GPIO_PORT_C
#define TIREMO_BOARD_USER_BUTTON_PIN       TIREMO_HAL_GPIO_PIN_9
#define TIREMO_BOARD_USER_BUTTON_IRQ_PRIO  (3U)
```

`TIREMO_BOARD_USER_BUTTON_IRQ_PRIO` is only used by
`TIREMO_Button_IntrInit()`, which this tutorial never calls — it's kept
here anyway for parity with the verbatim driver file, which references
it whether or not a given caller uses interrupts.

### 2. A new layer: `Drivers/button`

Every tutorial so far has talked directly to `Tiremo_Hal` (GPIO,
SysTick, UART) or `Common` (SysTick service, debug logging). This is the
first one to bring in `Drivers/` — the layer that wraps a HAL into a
named, specific peripheral instead of a generic pin. `Sw/Drivers/button/`
holds `tiremo_button.h`/`.c`, copied verbatim from the reference. `main.c`
itself is **unchanged** from Tutorial 03 — the button needs no vendor
peripheral bring-up call, same as GPIO in Tutorial 01.

`CMakeLists.txt` adds the one new source file and its include directory:

```cmake
add_executable(ButtonReadAndDebounce
    ...                                     # everything Tutorial 03 had
    Drivers/button/Src/tiremo_button.c
)
target_include_directories(ButtonReadAndDebounce PRIVATE
    ...
    Drivers/button/Include
)
```

### 3. Debounce and hold detection

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) keeps Tutorial 03's
two-LED loop verbatim and adds one more independent, non-blocking thing
to the same `for(;;)`: a call to `PollButton(now)`.

```c
static void PollButton(uint32_t now)
{
    static bool stablePressed = false;
    static bool candidatePressed = false;
    static uint32_t candidateSinceMs = 0U;
    static uint32_t pressStartMs = 0U;
    static bool holdLogged = false;
    bool rawPressed = TIREMO_Button_IsPressed(TIREMO_BUTTON_USER);

    if (rawPressed != candidatePressed)
    {
        candidatePressed = rawPressed;   /* raw level changed -- restart the debounce timer */
        candidateSinceMs = now;
    }

    if ((candidatePressed != stablePressed) &&
        ((now - candidateSinceMs) >= BUTTON_DEBOUNCE_MS))
    {
        stablePressed = candidatePressed;   /* held steady long enough -- accept it */
        if (stablePressed) { pressStartMs = now; holdLogged = false; /* WARN-log "pressed" */ }
        else { /* WARN-log "released after N ms" */ }
    }

    if (stablePressed && !holdLogged && ((now - pressStartMs) >= BUTTON_HOLD_THRESHOLD_MS))
    {
        holdLogged = true;   /* fires once, the instant the threshold is crossed */
        /* log "long hold reached" */
    }
}
```

Two independent timers, both using the same unsigned-subtraction idiom
from Tutorial 02: `candidateSinceMs` gates how long a raw reading has to
hold steady before it's trusted (`BUTTON_DEBOUNCE_MS`, 30 ms — comfortably
above typical mechanical bounce, comfortably below any real press
duration), and `pressStartMs` gates the long-hold check
(`BUTTON_HOLD_THRESHOLD_MS`, 10000 ms, matching the reference's factory-reset
constant). The hold check runs on every loop iteration a button is held,
independent of the debounce state machine, so it fires the moment 10 s
is reached — it doesn't wait for the eventual release, the same shape
`TIREMO_APP_ServiceFactoryResetHold()` uses in the reference firmware.

### 4. Watching it live: `tdf monitor`

[`tools/tdf.py`](../../tools/tdf.py) has a `monitor` subcommand.
Unlike `build`/`flash`/`clean`/`clean-device`, it isn't pure CMake
delegation underneath — resetting the board still is (`cmake --build
build --target reset-device`, a target every tutorial's
`CMakeLists.txt` defines right next to `erase-device`), but reading the UART
afterwards needs an actual serial port, which CMake and OpenOCD have no
concept of. That part is `pyserial`: auto-detect the board's serial
port (falling back to `--port` if more than one candidate turns up),
open it at 115200 8N1, and print each line wrapped in an ANSI color
code picked from its `[LEVEL]` prefix:

```python
_LEVEL_COLORS = {
    "[ERROR]": "\033[31m",  # red
    "[WARN]": "\033[33m",   # yellow
    "[INFO]": "\033[32m",   # green
    "[DEBUG]": "\033[36m",  # cyan
}

def colorize(line):
    color = _LEVEL_COLORS.get(line.split(" ", 1)[0])
    return f"{color}{line}\033[0m" if color else line
```

The order is deliberate: `monitor` opens the serial port and discards
any stale bytes already buffered on the host, *then* resets the board.
That way every `tdf monitor` session starts from a fresh boot, and the
boot line isn't lost while the port is still opening. One edge case
worth knowing about: resetting a live UART connection can occasionally
tear a line in half right at the reset boundary (a stray, garbled
partial line, immediately followed by clean output starting back near
0 ms). It's harmless.

### 5. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) Same
OS-detection, same bundled-OpenOCD flow as every prior tutorial for
`build`/`flash` — see [01-Hello_Blink's Solution](../01-Hello_Blink/#solution).
See **Solution** above for what the output should look like.

## Done When

`tdf monitor` streams the boot line and continuous `LED1`/`LED2
toggle @ ... ms` lines in green. Pressing and holding the USER button
then adds: one yellow `USER button: pressed` line per physical press
(not several — proof the debounce is working), a green `USER button:
long hold reached (10000 ms)` line the instant a hold crosses 10 s, and
a yellow `USER button: released after N ms` line on release, with `N`
distinguishing a short press from a long hold.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
