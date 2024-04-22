# Copyright (c) 2023 Cypress Semiconductor Corporation.
# SPDX-License-Identifier: Apache-2.0

zephyr_sources(soc.c)
zephyr_include_directories(.)

# Expedition family defines
zephyr_compile_definitions_ifdef(CONFIG_SOC_FAMILY_INFINEON_CAT1 CY_USING_HAL)
zephyr_compile_definitions_ifdef(CONFIG_SOC_FAMILY_INFINEON_CAT1B COMPONENT_CAT1B)
zephyr_compile_definitions(COMPONENT_CM33)

# Use custom linker script
set(SOC_LINKER_SCRIPT ${ZEPHYR_BASE}/soc/infineon/cat1b/cyw20829/linker.ld CACHE INTERNAL "")

# Generate Application header
find_program(SREC_CAT srec_cat)
if(NOT SREC_CAT)
  message(WARNING "srec_cat not found!")
else()
  set(generate_app_header ${ZEPHYR_BASE}/soc/infineon/cat1b/cyw20829/scripts/generate_app_header.py)
  math(EXPR bootstrap_dest "${CONFIG_SRAM_BASE_ADDRESS} + ${CONFIG_SRAM_SIZE}*1024")
  set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
    COMMAND ${PYTHON_EXECUTABLE}
      ${generate_app_header}
      ${ZEPHYR_BINARY_DIR}/app_header.bin
      0x2400
      ${bootstrap_dest}
    COMMAND ${SREC_CAT}
      ${ZEPHYR_BINARY_DIR}/app_header.bin -Binary -offset 0x60000000
      ${ZEPHYR_BINARY_DIR}/zephyr.bin -Binary -offset 0x60000050
      -o ${ZEPHYR_BINARY_DIR}/zephyr.hex -Intel -Output_Block_Size=16
  )
endif()
