# Shared CMake toolchain file for every tutorial's Sw/ project in this
# repo. Cross-compiles for the ABOV A34G43x (Cortex-M4) using
# arm-none-eabi-gcc -- the same compiler every tutorial's old Makefile
# invoked directly, just now reached through CMake instead of raw make.
#
# One file, not duplicated per tutorial: CPU flags, compiler names, and
# linker specs are identical across every tutorial in this repo (same
# MCU), so there is nothing tutorial-specific to teach by repeating them.
# What *is* tutorial-specific (source list, include paths, linker script
# path) stays in each tutorial's own CMakeLists.txt.
#
# Usage, from a tutorial's Sw/ directory:
#   cmake -G Ninja -B build -DCMAKE_TOOLCHAIN_FILE=../../../cmake/toolchain-arm-none-eabi.cmake
#   cmake --build build
#
# Works identically in cmd.exe, PowerShell, bash, and zsh -- no Git Bash
# or WSL required on Windows, unlike this repo's earlier build.sh/
# flash.sh scripts.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy CACHE FILEPATH "arm-none-eabi objcopy")
set(CMAKE_SIZE arm-none-eabi-size CACHE FILEPATH "arm-none-eabi size")

# A bare-metal target has no OS to run CMake's compiler-detection test
# programs on; building a static library instead of a full executable is
# CMake's documented way to make that detection succeed anyway.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TIREMO_CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mlittle-endian")

set(CMAKE_C_FLAGS_INIT "${TIREMO_CPU_FLAGS} -Og -g3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${TIREMO_CPU_FLAGS} --specs=nano.specs --specs=nosys.specs -Wl,--no-warn-rwx-segment -Wl,--gc-sections")
