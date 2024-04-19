# SPDX-License-Identifier: Apache-2.0

board_runner_args(jlink "--device=MKE15Z256xxx7")
board_runner_args(pyocd "--target=ke15z7")

include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/canopen.board.cmake)
