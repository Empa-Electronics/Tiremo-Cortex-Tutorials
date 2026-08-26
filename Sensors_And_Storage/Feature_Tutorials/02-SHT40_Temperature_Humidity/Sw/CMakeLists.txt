cmake_minimum_required(VERSION 3.20)
project(SHT40TemperatureHumidity C)

# Shared vendor SDK, four directories up (repo root) -- see the repo
# root's Generation/ directory for why it's checked in once, shared,
# instead of duplicated per tutorial.
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Generation/AUDK32_A34xxxx-1.0.12)

add_executable(SHT40TemperatureHumidity
    ${GEN_DIR}/Example/Source/TmplUserApp/prv_scu_clk.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_uart.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_uart_isr.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_i2c.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_i2c_isr.c
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/startup_a34xxxx.c
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/system_a34xxxx.c
    ${GEN_DIR}/Platform/HAL/PCU/hal_pcu.c
    ${GEN_DIR}/Platform/HAL/SCU/hal_scu.c
    ${GEN_DIR}/Platform/HAL/SCU/hal_scu_clk.c
    ${GEN_DIR}/Platform/HAL/UART/hal_uart.c
    ${GEN_DIR}/Platform/HAL/I2C/hal_i2c.c
    ${GEN_DIR}/Platform/HAL/HPL/DMA/hpl_dma.c
    ${GEN_DIR}/Platform/HAL/HPL/NMI/hpl_nmi.c
    Src/main.c
    Src/prv_user_code.c
    Tiremo_Hal/Src/tiremo_hal_systick.c
    Tiremo_Hal/Src/tiremo_hal_uart.c
    Tiremo_Hal/Src/tiremo_hal_i2c.c
    Common/Src/tiremo_systick.c
    Common/Src/tiremo_debug.c
    Drivers/sht40/Src/tiremo_sht40.c
)

target_include_directories(SHT40TemperatureHumidity PRIVATE
    ${GEN_DIR}/Framework/CMSIS/Core/Include
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Include
    ${GEN_DIR}/ProductConfig/Config
    ${GEN_DIR}/Platform/HAL/Include
    ${GEN_DIR}/Platform/HAL/HPL/Include
    ${GEN_DIR}/Platform/HAL/I2C
    Board
    Tiremo_Hal/Include
    Common/Include
    Drivers/sht40/Include
)

target_compile_definitions(SHT40TemperatureHumidity PRIVATE
    EXTRN_SUBFAMILY_A34G43x
    EXTRN_ABOV_MODULE_CONFIG
)

# Log verbosity compile-time ceiling (0=NONE ... 4=DEBUG). Default INFO in
# tiremo_debug.h when unset. Override at configure time:
#   tdf.py build -D TIREMO_DEBUG_LEVEL=2   (WARN and below)
set(TIREMO_DEBUG_LEVEL "" CACHE STRING "Log verbosity compile-time ceiling (0=NONE, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG). Empty = tiremo_debug.h's own default (INFO).")
if(TIREMO_DEBUG_LEVEL)
    target_compile_definitions(SHT40TemperatureHumidity PRIVATE TIREMO_DEBUG_LEVEL=${TIREMO_DEBUG_LEVEL})
endif()

set(LD_SCRIPT ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/GCC/gcc_a34g43x.ld)
target_link_options(SHT40TemperatureHumidity PRIVATE
    -T${LD_SCRIPT}
    -Wl,-Map,$<TARGET_FILE_DIR:SHT40TemperatureHumidity>/SHT40TemperatureHumidity.map
)
set_target_properties(SHT40TemperatureHumidity PROPERTIES LINK_DEPENDS ${LD_SCRIPT})

# .bin + size report, matching every tutorial's CMakeLists.txt.
add_custom_command(TARGET SHT40TemperatureHumidity POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:SHT40TemperatureHumidity> $<TARGET_FILE_DIR:SHT40TemperatureHumidity>/SHT40TemperatureHumidity.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:SHT40TemperatureHumidity>
    VERBATIM
)

# --- flash target -----------------------------------------------------
# Picks the matching bundled OpenOCD binary for the host (the vendor SDK
# ships Linux/x86_64, macOS/arm64, and Windows builds) and programs the
# single monolithic image over SWD. `tdf.py flash` runs identically in
# cmd.exe, PowerShell, bash, and zsh.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(TIREMO_OPENOCD_BIN ${GEN_DIR}/Tool/OpenOCD/win32/openocd.win32.exe)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(TIREMO_OPENOCD_BIN ${GEN_DIR}/Tool/OpenOCD/darwin/openocd.darwin)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(TIREMO_OPENOCD_BIN ${GEN_DIR}/Tool/OpenOCD/linux/openocd.linux.exe)
else()
    set(TIREMO_OPENOCD_BIN "")
    message(WARNING "No bundled OpenOCD for this host (${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR}) -- 'flash' target will not be available.")
endif()

if(TIREMO_OPENOCD_BIN)
    set(TIREMO_OPENOCD_SCRIPTS ${GEN_DIR}/Tool/OpenOCD/share/openocd/scripts)
    set(TIREMO_BOARD_CFG board/abov32/a34xxxx/a34g43x/starterkit-a34g43a-yln-a.cfg)

    add_custom_target(flash
        COMMAND ${TIREMO_OPENOCD_BIN}
                -s ${TIREMO_OPENOCD_SCRIPTS}
                -f ${TIREMO_BOARD_CFG}
                -c "cortex_m reset_config sysresetreq"
                -c "program \"$<TARGET_FILE:SHT40TemperatureHumidity>\" verify; reset run; shutdown"
        DEPENDS SHT40TemperatureHumidity
        COMMENT "Flashing SHT40TemperatureHumidity via bundled OpenOCD (CMSIS-DAP / SWD)"
        VERBATIM
        USES_TERMINAL
    )

    # Mass-erases the whole chip (both flash banks) -- not just this
    # project's image. Used by `tdf.py clean-device`; no per-tutorial
    # target needs this normally, `flash` already overwrites in place.
    add_custom_target(erase-device
        COMMAND ${TIREMO_OPENOCD_BIN}
                -s ${TIREMO_OPENOCD_SCRIPTS}
                -f ${TIREMO_BOARD_CFG}
                -c "cortex_m reset_config sysresetreq"
                -c "init; reset halt; flash probe 0; flash erase_address unlock 0x0 0x200000; shutdown"
        COMMENT "Mass-erasing entire chip flash (both banks) via bundled OpenOCD"
        VERBATIM
        USES_TERMINAL
    )

    # Resets and resumes the target over SWD without reprogramming --
    # used by `tdf.py monitor` right before it opens a serial terminal,
    # so log output always starts from a fresh boot.
    add_custom_target(reset-device
        COMMAND ${TIREMO_OPENOCD_BIN}
                -s ${TIREMO_OPENOCD_SCRIPTS}
                -f ${TIREMO_BOARD_CFG}
                -c "cortex_m reset_config sysresetreq"
                -c "init; reset run; shutdown"
        COMMENT "Resetting target over SWD via bundled OpenOCD (no reprogramming)"
        VERBATIM
        USES_TERMINAL
    )
endif()
