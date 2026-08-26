# macOS OpenOCD build (arm64)

openocd.darwin was built locally from ABOV's OpenOCD fork (the same source
the vendor's linux/win32 builds in ../ come from), since the vendor package
doesn't ship a macOS binary and mainline OpenOCD (e.g. Homebrew's) lacks the
abov32v3 flash driver these MCUs need.

## Rebuilding

Dependencies (Homebrew): autoconf automake libtool pkg-config libusb hidapi

    git clone --branch abov32 https://git.code.sf.net/u/abovsemic/openocd abov-openocd
    cd abov-openocd
    git submodule update --init --recursive
    ./bootstrap nosubmodule
    ./configure --enable-cmsis-dap --enable-cmsis-dap-v2 --disable-werror
    make -j"$(sysctl -n hw.ncpu)"
    cp src/openocd <path-to-repo>/Generation/AUDK32_A34xxxx-1.0.12/Tool/OpenOCD/darwin/openocd.darwin

Built 2026-07-29 against OpenOCD 0.12.0 (commit a4e8f32, abov32 branch).
Uses the shared ../share/openocd/scripts tree already in this package
(the abov32v3 driver is compiled into the binary, not the tcl scripts).
