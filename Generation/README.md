# Generation/ — Shared Vendor SDK

This is ABOV Semiconductor's AUDK32 SDK for the A34G43x (CMSIS device
pack, HAL, linker script, and a bundled OpenOCD build for Linux, macOS,
and Windows — `AUDK32_A34xxxx-1.0.12/Tool/OpenOCD/`). It's the same
vendor tree the [reference firmware](../referance_only/Tiremo-Secure-Iot-Sw/README.md)
builds against — see that project's own README for what the full
production firmware layers on top of it.

## Why this is checked in once, here, shared

Every tutorial in this repo with a real, buildable `Sw/` solution folder
compiles against this one copy, referenced by relative path (typically
`../../../Generation/AUDK32_A34xxxx-1.0.12` from a
`<Topic>/<Section>/<NN-Tutorial>/Sw/Makefile`). It is **not** duplicated
into each tutorial — at ~107 MB (2000+ files, including three prebuilt
OpenOCD binaries), copying it into all 38 tutorial folders would balloon
this repo to several gigabytes for zero benefit, since every tutorial
targets the same board and MCU.

## License

ABOV's own [`AUDK32_A34xxxx-1.0.12/Document/LICENSE.md`](AUDK32_A34xxxx-1.0.12/Document/LICENSE.md)
covers this whole tree — BSD-3-Clause, redistribution in source and
binary form permitted with attribution retained. Individual source files
also carry ABOV's own copyright header pointing back to that same file.

## Regenerating / updating

If ABOV ships a newer AUDK32 SDK release, replace this directory wholesale
(matching the reference firmware's own "vendor glue stays thin" design
principle) rather than hand-editing files inside it — every tutorial's
`Sw/Makefile` only depends on stable relative paths within this tree, not
on anything specific to this exact SDK version.
