cmake_minimum_required(VERSION 3.20)
project(StandaloneEnvironmentalLogger C)

set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Generation/AUDK32_A34xxxx-1.0.12)

add_executable(StandaloneEnvironmentalLogger
    ${GEN_DIR}/Example/Source/TmplUserApp/prv_scu_clk.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_uart.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_uart_isr.c
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/startup_a34xxxx.c
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/system_a34xxxx.c
    ${GEN_DIR}/Platform/HAL/PCU/hal_pcu.c
    ${GEN_DIR}/Platform/HAL/SCU/hal_scu.c
    ${GEN_DIR}/Platform/HAL/SCU/hal_scu_clk.c
    ${GEN_DIR}/Platform/HAL/UART/hal_uart.c
    ${GEN_DIR}/Platform/HAL/HPL/DMA/hpl_dma.c
    ${GEN_DIR}/Platform/HAL/HPL/NMI/hpl_nmi.c
    Src/main.c
    Src/prv_user_code.c
    Tiremo_Hal/Src/tiremo_hal_systick.c
    Tiremo_Hal/Src/tiremo_hal_uart.c
    Common/Src/tiremo_systick.c
    Common/Src/tiremo_debug.c
    Common/Src/tiremo_banner.c
    Common/Src/tiremo_envlog.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_i2c.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_i2c_isr.c
    ${GEN_DIR}/Platform/HAL/I2C/hal_i2c.c
    Tiremo_Hal/Src/tiremo_hal_i2c.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_adc.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_adc_isr.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_timer1.c
    ${GEN_DIR}/Example/Source/TmplUserApp/user_timer1_isr.c
    ${GEN_DIR}/Platform/HAL/ADC/hal_adc.c
    ${GEN_DIR}/Platform/HAL/Timer1/hal_timer1.c
    ${GEN_DIR}/Platform/HAL/FMC/hal_fmc.c
    Tiremo_Hal/Src/tiremo_hal_adc.c
    Tiremo_Hal/Src/tiremo_hal_flash.c
    Drivers/sht40/Src/tiremo_sht40.c
    Drivers/lis2de12/Src/tiremo_lis2de12.c
    Drivers/battery/Src/tiremo_battery.c
    Drivers/mic/Src/tiremo_mic_bsp.c
    Drivers/mic/Src/tiremo_mic_app.c
)

target_include_directories(StandaloneEnvironmentalLogger PRIVATE
    ${GEN_DIR}/Framework/CMSIS/Core/Include
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Include
    ${GEN_DIR}/ProductConfig/Config
    ${GEN_DIR}/Platform/HAL/Include
    ${GEN_DIR}/Platform/HAL/HPL/Include
    Board
    Tiremo_Hal/Include
    Common/Include
    ${GEN_DIR}/Platform/HAL/I2C
    ${GEN_DIR}/Platform/HAL/ADC
    ${GEN_DIR}/Platform/HAL/Timer1
    ${GEN_DIR}/Platform/HAL/FMC
    Drivers/sht40/Include
    Drivers/lis2de12/Include
    Drivers/battery/Include
    Drivers/mic/Include
    Drivers/mic/Src
)

target_compile_definitions(StandaloneEnvironmentalLogger PRIVATE
    EXTRN_SUBFAMILY_A34G43x
    EXTRN_ABOV_MODULE_CONFIG
)

# Log verbosity compile-time ceiling (0=NONE ... 4=DEBUG). Default INFO in
# tiremo_debug.h when unset. Override at configure time:
#   tdf.py build -D TIREMO_DEBUG_LEVEL=2   (WARN and below)

set(TIREMO_DEBUG_LEVEL "" CACHE STRING "Log verbosity compile-time ceiling (0=NONE, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG). Empty = tiremo_debug.h's own default (INFO).")
if(TIREMO_DEBUG_LEVEL)
    target_compile_definitions(StandaloneEnvironmentalLogger PRIVATE TIREMO_DEBUG_LEVEL=${TIREMO_DEBUG_LEVEL})
endif()

set(LD_SCRIPT ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/GCC/gcc_a34g43x.ld)
target_link_options(StandaloneEnvironmentalLogger PRIVATE
    -T${LD_SCRIPT}
    -Wl,-Map,$<TARGET_FILE_DIR:StandaloneEnvironmentalLogger>/StandaloneEnvironmentalLogger.map
)
set_target_properties(StandaloneEnvironmentalLogger PROPERTIES LINK_DEPENDS ${LD_SCRIPT})

add_custom_command(TARGET StandaloneEnvironmentalLogger POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:StandaloneEnvironmentalLogger> $<TARGET_FILE_DIR:StandaloneEnvironmentalLogger>/StandaloneEnvironmentalLogger.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:StandaloneEnvironmentalLogger>
    VERBATIM
)

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
                -c "program \"$<TARGET_FILE:StandaloneEnvironmentalLogger>\" verify; reset run; shutdown"
        DEPENDS StandaloneEnvironmentalLogger
        COMMENT "Flashing StandaloneEnvironmentalLogger via bundled OpenOCD (CMSIS-DAP / SWD)"
        VERBATIM
        USES_TERMINAL
    )

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
