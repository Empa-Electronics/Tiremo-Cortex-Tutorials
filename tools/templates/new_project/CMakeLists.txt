cmake_minimum_required(VERSION 3.20)
project(__PROJECT_NAME__ C)

# Shared vendor SDK -- see the repo root's Generation/ directory for why
# it's checked in once, shared, instead of duplicated per project. This
# relative path was computed by `tdf.py create-project` for wherever
# this file landed; if you move this folder, regenerate it (or fix it
# by hand) rather than assuming the usual three `../`.
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/__GEN_DIR_REL__/Generation/AUDK32_A34xxxx-1.0.12)

add_executable(__PROJECT_NAME__
    ${GEN_DIR}/Example/Source/TmplUserApp/prv_scu_clk.c
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/startup_a34xxxx.c
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/system_a34xxxx.c
    ${GEN_DIR}/Platform/HAL/PCU/hal_pcu.c
    ${GEN_DIR}/Platform/HAL/SCU/hal_scu.c
    ${GEN_DIR}/Platform/HAL/SCU/hal_scu_clk.c
    Src/main.c
    Src/prv_user_code.c
)

target_include_directories(__PROJECT_NAME__ PRIVATE
    ${GEN_DIR}/Framework/CMSIS/Core/Include
    ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Include
    ${GEN_DIR}/ProductConfig/Config
    ${GEN_DIR}/Platform/HAL/Include
    ${GEN_DIR}/Platform/HAL/HPL/Include
)

target_compile_definitions(__PROJECT_NAME__ PRIVATE
    EXTRN_SUBFAMILY_A34G43x
    EXTRN_ABOV_MODULE_CONFIG
)

set(LD_SCRIPT ${GEN_DIR}/Framework/CMSIS/Device/ABOV/Source/GCC/gcc_a34g43x.ld)
target_link_options(__PROJECT_NAME__ PRIVATE
    -T${LD_SCRIPT}
    -Wl,-Map,$<TARGET_FILE_DIR:__PROJECT_NAME__>/__PROJECT_NAME__.map
)
set_target_properties(__PROJECT_NAME__ PROPERTIES LINK_DEPENDS ${LD_SCRIPT})

# .bin + size report, matching every tutorial's CMakeLists.txt.
add_custom_command(TARGET __PROJECT_NAME__ POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:__PROJECT_NAME__> $<TARGET_FILE_DIR:__PROJECT_NAME__>/__PROJECT_NAME__.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:__PROJECT_NAME__>
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
                -c "program \"$<TARGET_FILE:__PROJECT_NAME__>\" verify; reset run; shutdown"
        DEPENDS __PROJECT_NAME__
        COMMENT "Flashing __PROJECT_NAME__ via bundled OpenOCD (CMSIS-DAP / SWD)"
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
