cmake_minimum_required(VERSION 3.20)
project(FleetProvisioningByClaim C)

# Shared vendor SDK, four directories up (repo root) -- see the repo
# root's Generation/ directory for why it's checked in once, shared,
# instead of duplicated per tutorial.
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Generation/AUDK32_A34xxxx-1.0.12)

# --- embed the shared claim credentials as C byte arrays ---------------
# certs/root_ca.pem is Amazon's own public Root CA -- safe to commit.
# certs/claim_cert.pem and certs/claim_key.pem are the shared fleet
# claim identity -- gitignored, never committed. Same pattern as
# Cloud Messaging 03's device_cert.pem / device_key.pem.
# See certs/README.md.
set(EMBED_PEM_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/embed_pem.cmake)
set(CERTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/certs)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

if(NOT EXISTS ${CERTS_DIR}/claim_cert.pem OR NOT EXISTS ${CERTS_DIR}/claim_key.pem)
    message(FATAL_ERROR
        "Missing claim credentials.\n"
        "  cp certs/claim_cert.pem.example certs/claim_cert.pem\n"
        "  cp certs/claim_key.pem.example certs/claim_key.pem\n"
        "then paste the real Tiremo Cloud claim certificate and private key. "
        "Those two files are gitignored; see certs/README.md.")
endif()

function(embed_pem name symbol)
    add_custom_command(
        OUTPUT ${GENERATED_DIR}/${name}.c
        COMMAND ${CMAKE_COMMAND}
                -DINPUT=${CERTS_DIR}/${name}.pem
                -DOUTPUT=${GENERATED_DIR}/${name}.c
                -DSYMBOL=${symbol}
                -P ${EMBED_PEM_SCRIPT}
        DEPENDS ${CERTS_DIR}/${name}.pem ${EMBED_PEM_SCRIPT}
        COMMENT "Embedding certs/${name}.pem as ${symbol}[]"
        VERBATIM
    )
endfunction()

embed_pem(root_ca g_rootCaPem)
embed_pem(claim_cert g_claimCertPem)
embed_pem(claim_key g_claimKeyPem)

add_executable(FleetProvisioningByClaim
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
    Tiremo_Hal/Src/tiremo_hal_gpio.c
    Common/Src/tiremo_systick.c
    Common/Src/tiremo_debug.c
    Common/Src/tiremo_identity.c
    Common/Src/tiremo_json.c
    Common/Src/tiremo_provision.c
    Drivers/esp32/Src/tiremo_esp32.c
    Drivers/button/Src/tiremo_button.c
    ${GENERATED_DIR}/root_ca.c
    ${GENERATED_DIR}/claim_cert.c
    ${GENERATED_DIR}/claim_key.c
)

target_include_directories(FleetProvisioningByClaim PRIVATE
    ${GEN_DIR}/Framework/CMSIS/Core/Include
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Include
    ${GEN_DIR}/ProductConfig/Config
    ${GEN_DIR}/Platform/HAL/Include
    ${GEN_DIR}/Platform/HAL/HPL/Include
    Board
    Tiremo_Hal/Include
    Common/Include
    Drivers/esp32/Include
    Drivers/button/Include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(FleetProvisioningByClaim PRIVATE
    EXTRN_SUBFAMILY_A34G43x
    EXTRN_ABOV_MODULE_CONFIG
)

# Log verbosity compile-time ceiling (0=NONE ... 4=DEBUG). Default INFO in
# tiremo_debug.h when unset. Override at configure time:
#   tdf.py build -D TIREMO_DEBUG_LEVEL=2   (WARN and below)
set(TIREMO_DEBUG_LEVEL "" CACHE STRING "Log verbosity compile-time ceiling (0=NONE, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG). Empty = tiremo_debug.h's own default (INFO).")
if(TIREMO_DEBUG_LEVEL)
    target_compile_definitions(FleetProvisioningByClaim PRIVATE TIREMO_DEBUG_LEVEL=${TIREMO_DEBUG_LEVEL})
endif()

set(LD_SCRIPT ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/GCC/gcc_a34g43x.ld)
target_link_options(FleetProvisioningByClaim PRIVATE
    -T${LD_SCRIPT}
    -Wl,-Map,$<TARGET_FILE_DIR:FleetProvisioningByClaim>/FleetProvisioningByClaim.map
)
set_target_properties(FleetProvisioningByClaim PROPERTIES LINK_DEPENDS ${LD_SCRIPT})

add_custom_command(TARGET FleetProvisioningByClaim POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:FleetProvisioningByClaim> $<TARGET_FILE_DIR:FleetProvisioningByClaim>/FleetProvisioningByClaim.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:FleetProvisioningByClaim>
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
                -c "program \"$<TARGET_FILE:FleetProvisioningByClaim>\" verify; reset run; shutdown"
        DEPENDS FleetProvisioningByClaim
        COMMENT "Flashing FleetProvisioningByClaim via bundled OpenOCD (CMSIS-DAP / SWD)"
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
