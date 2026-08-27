<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Minimal JSON Helpers</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Minimal JSON Helpers

***Board: Tiremo Cortex (ESP32) · Section: Tiremo Cloud / Feature Tutorials · Difficulty: Medium***

## Introduction

Shadow documents, fleet-provisioning replies, and Jobs payloads are JSON. A general-purpose parser (cJSON, jsmn) would work and would also cost heap, recursion, and a dependency the rest of this MCU firmware does not need. This tutorial builds the four find helpers and the writer that later Tiremo Cloud tutorials actually call — scoped to known, bounded document shapes.

This `Sw/` does **not** talk to the ESP32 or to Tiremo Cloud. The tests run against a fixed sample shadow in flash and log pass/fail on UART0.

## You Will Learn

- Why a scan-at-current-depth approach is enough when you already know the schema
- Matching keys only as *siblings* of the object you were handed (so `enabled` inside `reporting` is not visible at the root)
- Skipping strings so a colon or brace inside `"note"` cannot look like structure
- A `JsonWriter` that can emit a nested `state.reported` object, then survive being parsed by the same Find* helpers

## Prerequisites

- [01-Device_Identity_From_MAC](../01-Device_Identity_From_MAC/) conceptually (Tiremo Cloud has started); this `Sw/` only needs debug UART from [Getting Started](../../../Getting_Started/)
- Toolchain and flashing set up per Getting Started

Fleet provisioning is [03](../03-Fleet_Provisioning_By_Claim/). It comes after this because `CreateKeysAndCertificate` replies are JSON too.

## Platform documentation

The documents these helpers parse are specified on the platform side:
[fleet provisioning](https://docs.tiremo.ai/device-provisioning/second-phase),
[Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction)
(classic Device Shadow), and
[Tiremo Jobs](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction).

## Understanding scan-at-this-depth

`TIREMO_Json_FindObject(doc, len, "state", ...)` looks at the members of `doc`, not at grandchildren. Nested fields are reached by feeding the slice back in:

```c
(void)TIREMO_Json_FindObject(doc, len, "state", &pState, &stateLen);
(void)TIREMO_Json_FindObject(pState, stateLen, "desired", &pDesired, &desiredLen);
(void)TIREMO_Json_FindObject(pDesired, desiredLen, "reporting", &pRep, &repLen);
(void)TIREMO_Json_FindBool(pRep, repLen, "enabled", &enabled);
```

That is the whole API. There is no JSON Pointer, no allocation, and no DOM. `FindBool("enabled")` on the root document **fails** even though the word appears deeper in — confirmed on hardware (`root has no sibling key enabled`).

Strings are skipped as atomic values. The sample includes `"note":"colon:not-a-key"` so a naive `strstr` for `:` would have already gone wrong.

The writer is the other half of a report: echo the *full* applied desired object (nested `reporting` and `led` included), not just the key that changed. Tutorial 04 will need that to clear a shadow delta.

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified on the connected board:** `.text` 16156 bytes, flash **Verified OK**. UART:

```text
[INFO] Minimal_JSON_Helpers: boot
[INFO] json: FindObject state
[INFO] json: FindObject desired
[INFO] json: FindObject reporting
[INFO] json: FindBool reporting.enabled == true
[INFO] json: FindUint reporting.intervalS == 60
[INFO] json: FindObject led
[INFO] json: FindBool led.on == false
[INFO] json: FindString note (colon inside string)
[INFO] json: FindUint version == 7
[INFO] json: root has no sibling key enabled
[INFO] json: writer finish
[INFO] json: wrote {"state":{"reported":{"reporting":{"enabled":true,"intervalS":60},"led":{"on":false},"note":"colon:not-a-key"}}}
[INFO] json: re-parse state
[INFO] json: re-parse reported
[INFO] json: re-parse reporting
[INFO] json: echo enabled == true
[INFO] json: echo intervalS == 60
[INFO] json: round-trip ok
```

## Walkthrough

1. [`tiremo_json.c`](./Sw/Common/Src/tiremo_json.c) — `FindObject` / `FindBool` / `FindUint` / `FindString` and `TIREMO_JsonWriter_*`
2. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — sample shadow, negative sibling test, write reported, re-parse

## Done When

All four Find* helpers extract nested fields from a realistic shadow document, a missing sibling key fails, and the writer produces JSON the same helpers accept. Confirmed on hardware as above.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
