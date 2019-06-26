# SPDX-License-Identifier: Apache-2.0

set(EMU_PLATFORM qemu)

if(NOT CONFIG_REBOOT)
  set(REBOOT_FLAG -no-reboot)
endif()

if(CONFIG_X86_LONGMODE)
  set_target_properties(${ZEPHYR_TARGET} PROPERTIES QEMU_binary_suffix x86_64)
endif()

if(CONFIG_X86_LONGMODE)
  set(QEMU_CPU_TYPE_${ARCH} qemu64,+x2apic)
else()
  set(QEMU_CPU_TYPE_${ARCH} qemu32,+nx,+pae)
endif()

set_target_properties(${ZEPHYR_TARGET} PROPERTIES QEMU_CPU_TYPE_${ARCH} ${QEMU_CPU_TYPE_${ARCH}})
set_property(TARGET   ${ZEPHYR_TARGET} PROPERTY   QEMU_FLAGS_${ARCH}
  -m 9
  -cpu ${QEMU_CPU_TYPE_${ARCH}}
  -device isa-debug-exit,iobase=0xf4,iosize=0x04
  ${REBOOT_FLAG}
  -nographic
  )

# TODO: Support debug
# board_set_debugger_ifnset(qemu)
# debugserver: QEMU_EXTRA_FLAGS += -s -S
# debugserver: qemu
