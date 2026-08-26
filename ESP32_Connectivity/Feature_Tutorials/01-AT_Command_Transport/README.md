<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>AT Command Transport</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# AT Command Transport

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

Every ESP32 feature from here on rides on this transport: power sequencing, a UART RX ring buffer, and an expect/timeout command primitive. Get this rock solid before building WiFi or MQTT on top of it. This tutorial does **not** copy the full production ESP32 driver — only Init / SendCommand / IsPresent / DisableEcho. Join, NTP, BluFi, and MQTT are functions added to the same file in later tutorials.

## You Will Learn

- The module power-cycle sequence (off → boot delay → AT probe)
- An expect-substring-with-timeout command primitive over an interrupt-driven RX ring buffer
- Why echo (`ATE0`) must be disabled before parsing later responses
- That debug logging and AT commands are two different UARTs (UART0 vs UART2)

## Prerequisites

- [Getting_Started](../../../Getting_Started/) complete — SysTick, debug UART, and `tdf.py` are assumed
- An ABOV A34G43A Starter Kit with the ESP32 module on UART2 / PA12

## Understanding the two UARTs

The MCU talks to *you* and to the *radio* on two different serial ports. Mixing them up is the usual first failure.

| Port | Pins | Who speaks | What you see |
|------|------|------------|----------------|
| **UART0** | the USB debug console | `TIREMO_LOG_INFO` / `tdf.py monitor` | `[INFO] AT_Command_Transport: boot` |
| **UART2** | PA8 RX, PA9 TX | ESP-AT firmware on the module | `AT\r\n` then `OK` |

[`PRV_UART_Init()`](../../../Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/user_uart.c) clocks **both** (and UART1, unused here). Then two Tiremo inits steal the IRQs:

- `TIREMO_Debug_Init()` re-arms **UART0** with the debug ring (Getting Started 03).
- `TIREMO_ESP32_Init()` re-arms **UART2** with this tutorial's AT ring.

If you type `AT` into `tdf.py monitor`, you are talking to the MCU log, not the ESP32. If you `TIREMO_Debug_Puts` onto UART2, you corrupt the AT parser. Keep the two pipes separate for the rest of this section.

## Understanding the power-cycle

The ESP32 is not powered just because the MCU is. PA12 is a **supply gate**, active-high: `LOW` = module off, `HIGH` = module on. A leftover WiFi session from the previous firmware (or a half-booted AT stack) will answer `AT` with garbage. The clean fix is always: turn it off, wait, turn it on, wait for boot, *then* probe.

From [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c):

```c
static void prvESP32PowerCycle(void)
{
    TIREMO_LOG_INFO("ESP32: power cycling module...");

    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_ESP32_PWR_PORT,
                                   TIREMO_ESP32_PWR_PIN,
                                   TIREMO_ESP32_PWR_OFF);
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_OFF_MS);

    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_ESP32_PWR_PORT,
                                   TIREMO_ESP32_PWR_PIN,
                                   TIREMO_ESP32_PWR_ON);
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_BOOT_MS);

    TIREMO_LOG_INFO("ESP32: power-on boot delay elapsed");
}
```

The delays live in [`board_config.h`](./Sw/Board/board_config.h): **500 ms off**, **3000 ms boot**. Those 3.5 seconds are why `tdf.py monitor` (reset the chip, *then* open the port) often misses the first log lines. The firmware still ran them; the terminal just wasn't listening yet.

`TIREMO_ESP32_Init()` also sets PA12 to push-pull output before that sequence, then registers the UART2 IRQ and arms a 1-byte receive. Until `prvESP32ArmRx()` runs, incoming AT bytes have nowhere to go.

## Understanding the RX ring

ESP-AT replies are a burst of bytes (`\r\nOK\r\n`, later a whole scan list). The ISR cannot parse them — it would hold off other interrupts for too long. So the ISR only **stores**, and the main loop **parses**.

1. `prvESP32ArmRx()` asks the UART HAL for **one** byte into `s_rxByte`, non-blocking.
2. When that byte arrives, `prvESP32UARTCallback` sees `RX_DONE`, pushes it into a 256-byte power-of-two ring, and arms the next byte.
3. A line error or a TX-done-while-RX-was-busy also re-arms, so a glitch does not stall the pipe.

```c
static void prvESP32UARTCallback(uint32_t event, void *pContext)
{
    (void)pContext;

    if (!s_initialized)
    {
        return;
    }

    if ((event & TIREMO_HAL_UART_EVENT_RX_DONE) != 0U)
    {
        prvESP32RxRingPush(s_rxByte);
        prvESP32ArmRx();
    }

    if ((event & TIREMO_HAL_UART_EVENT_LINE_ERROR) != 0U)
    {
        prvESP32ArmRx();
    }

    if (((event & TIREMO_HAL_UART_EVENT_TX_DONE) != 0U) && s_rxArmPending)
    {
        prvESP32ArmRx();
    }
}
```

The ring is 256 bytes because `AT` / `ATE0` replies are tiny. Tutorial 03's `CWLAP` scan and later MQTT certificate bursts will need a bigger ring — that change belongs there, not here.

## Understanding expect / timeout

[`TIREMO_ESP32_SendCommand()`](./Sw/Drivers/esp32/Include/tiremo_esp32.h) is the whole AT API for this tutorial. You pass:

- `pCommand` — bytes to transmit **as-is**. You must include `"\r\n"`; ESP-AT will not treat a bare `AT` as a command.
- `pExpected` — a **substring**, not a full line. `"OK"` matches both `\r\nOK\r\n` and a noisy `WIFI GOT IP\r\nOK\r\n`.
- `timeoutMs` — give up if the substring never appears.

The implementation is one blocking function; the core of it, with logging and the response copy elided:

```c
if (pCommand != NULL)
{
    prvESP32RxRingFlush();
    /* ... TIREMO_HAL_UART_Transmit(pCommand) on UART2 ... */
}

response[0] = '\0';
startMs = TIREMO_SysTick_GetMs();

while ((!matched) && (!errored) &&
       ((TIREMO_SysTick_GetMs() - startMs) < timeoutMs))
{
    while ((!matched) && (!errored) && prvESP32RxRingPop(&byte))
    {
        if (length < (sizeof(response) - 1U))
        {
            response[length++] = (char)byte;
            response[length] = '\0';
        }

        if (strstr(response, pExpected) != NULL)
        {
            matched = true;
        }
        else if ((strcmp(pExpected, "\r\nERROR\r\n") != 0) &&
                 (strstr(response, "\r\nERROR\r\n") != NULL))
        {
            errored = true;
        }
    }
}
```

Three details students trip on:

1. **Flush before TX.** Leftover boot chatter (`ready`, `WIFI DISCONNECT`) must not sit in the ring and look like the next command's answer.
2. **Substring match, not line match.** That is convenient for `OK`, and it is also a foot-gun: if you wait for `"OK"` while the module is still printing a payload that happens to contain those two letters, you return too early. Tutorial 02's `CWSTATE` gotcha is exactly this. For this tutorial `"OK"` is the whole payload, so it is safe.
3. **Framed `ERROR` aborts.** If the module replies `\r\nERROR\r\n`, waiting the rest of `timeoutMs` for an `OK` that will never come is wasted time. The `\r\n` framing avoids matching the word `ERROR` inside a later certificate payload.

`SendCommandCapture` is the same loop, plus a copy of the accumulated bytes so the app can *look* at them. `IsPresent()` is just `SendCommand("AT\r\n", "OK", 1000)`.

Blocking for up to a second is fine here: there is no MQTT to service yet. A production event loop will still use this same primitive, just with shorter timeouts on status queries.

## Understanding why `ATE0` comes first

ESP-AT **echoes** every command until you turn echo off. So the first `AT\r\n` comes back as something like:

```text
AT\r\n
\r\n
OK\r\n
```

After `ATE0`, the same probe is only:

```text
\r\n
OK\r\n
```

If you leave echo on, later parsers that hunt for `"OK"`, `"WIFI GOT IP"`, or `+SSID:` will also see the command you just sent sitting in the same buffer. `ResponseContainsEcho()` in `prv_user_code.c` skips leading CR/LF and checks whether the payload still starts with `AT` — that is the proof this tutorial exists to show, not a production API.

Call `TIREMO_ESP32_DisableEcho()` once after `IsPresent()`, then never again unless you factory-reset the module.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

Same CMake shape as Sensors & Local Storage; `GEN_DIR` is four levels up to [`Generation/`](../../../Generation/).

**Verified on the connected board:** `.text` 14624 bytes, flash **Verified OK**. Boot:

```text
[INFO] AT_Command_Transport: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: AT -> OK (module present)
[INFO] ESP32: AT (echo still on) capture starts with echo=yes
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: AT after ATE0 is echo-free (OK only)
[INFO] ESP32: heartbeat AT -> OK
```

Heartbeats continue every 5 s. Rebuild with `tdf.py build -D TIREMO_DEBUG_LEVEL=4` if you want the raw TX/RX JSON dumps.

## Walkthrough

Read the files in this order — each one adds one idea.

1. [`main.c`](./Sw/Src/main.c) — same shape as Debug UART Logging: port init, clocks, **`PRV_UART_Init()`**, then `PRV_USER_Code()`. Without `PRV_UART_Init()`, UART2 is not clocked and `TIREMO_ESP32_Init()` will fail the IRQ registration.

2. [`board_config.h`](./Sw/Board/board_config.h) — the only board knowledge the driver is allowed: UART2, PA12, on/off polarity, 500 ms / 3000 ms. If a later board spins the gate pin, this file changes and `tiremo_esp32.c` does not.

3. [`tiremo_esp32.h`](./Sw/Drivers/esp32/Include/tiremo_esp32.h) / [`.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — the transport. This is a **subset** of the production ESP32 driver: Init, SendCommand, IsPresent, DisableEcho. WiFi join, NTP, BluFi, and MQTT are later tutorials adding functions to this same file, not a second UART stack.

4. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — the lesson, not the driver:
   - SysTick + debug UART (so logs have a clock and a console)
   - `TIREMO_ESP32_Init()` (power-cycle + ring)
   - `IsPresent()` — if this fails, stop. There is nothing useful to do without `OK`.
   - Capture a second `AT` **before** `ATE0` and log `echo=yes`
   - `DisableEcho()`
   - Capture `AT` again and log `echo-free`
   - Forever: every 5 s, `IsPresent()` as a heartbeat so you can yank the antenna / power-cycle the kit and watch it fail

The `for (;;) { }` on init/probe failure is deliberate: a silent reboot loop would hide a wiring mistake. Fix the hardware (or the PA12 polarity), then reset.

## Done When

`AT` reliably returns `OK` after every power cycle, and echoed command text no longer appears in captured responses — **confirmed**.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
