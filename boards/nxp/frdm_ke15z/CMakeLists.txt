# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BOARD_FRDM_KE15Z_FLEXIO_CLKOUT)
  zephyr_library()
  zephyr_library_sources(pinmux.c)
endif()
