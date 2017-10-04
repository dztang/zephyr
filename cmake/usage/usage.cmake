# TODO: Set to make when make is used as a generator
set(CMAKE_MAKE_PROGRAM ninja)
get_filename_component(generator ${CMAKE_MAKE_PROGRAM} NAME)

set(arch_list
  arc
  arm
  nios2
  riscv32
  x86
  xtensa
  )

set(board_dir $ENV{ZEPHYR_BASE}/boards)

foreach(arch ${arch_list})
  set(board_arch_dir ${board_dir}/${arch})

  # Match the *_defconfig files in the board directories to make sure
  # we are finding boards, e.g.  qemu_xtensa/qemu_xtensa_defconfig
  file(GLOB_RECURSE defconfigs_for_${arch}
    RELATIVE ${board_arch_dir}
    ${board_arch_dir}/*_defconfig
    )

  # The above gives a list like
  # nrf51_blenano/nrf51_blenano_defconfig;nrf51_pca10028/nrf51_pca10028_defconfig
  # we construct a list of board names by removing _defconfig suffix
  # and the path.
  set(boards_for_${arch} "")
  foreach(defconfig_path ${defconfigs_for_${arch}})
    get_filename_component(board_defconfig ${defconfig_path} NAME)

    string(REPLACE
      "_defconfig"       # <match_string>
      ""                 # <replace_string>
      board              # <output variable>
      ${board_defconfig} # <input>
      )

    list(APPEND boards_for_${arch} ${board})
  endforeach()
endforeach()

message("Cleaning targets:")
message("  clean     - Remove most generated files but keep configuration and backup files")
message("  pristine  - Remove all files in the build directory")
message("")
message("Configuration targets:")
message("")
message("  run <${generator} kconfig-help>")
message("")
message("Other generic targets:")
message("  all          - Build a zephyr application")
message("  run          - Build a zephyr application and run it if the board supports emulation")
message("  flash        - Build and flash an application")
message("  debug        - Build and debug an application using GDB")
message("  debugserver  - Build and start a GDB server (port 1234 for Qemu targets)")
message("  ram_report   - Build and create RAM usage report")
message("  rom_report   - Build and create ROM usage report")
message("  usage        - Display this text")
message("")
message("Supported Boards:")
message("")
message("  To generate project files for one of the supported boards below, run:")
message("")
message("  $ cmake -DBOARD=<BOARD NAME> -Bpath/to/build_dir -Hpath/to/source_dir")
message("")
message("  or")
message("")
message("  $ export BOARD=<BOARD NAME>")
message("  $ cmake -Bpath/to/build_dir -Hpath/to/source_dir")
message("")
foreach(arch ${arch_list})
  message(" ${arch}:")
  foreach(board ${boards_for_${arch}})
    message("   ${board}")
  endforeach()
endforeach()
message("")
message("Build flags:")
message("")
message("  ${generator} VERBOSE=1 [targets] verbose build")
message("  ${generator} C=1   [targets] Check all c source with $$CHECK (sparse by default)")
message("  ${generator} C=2   [targets] Force check of all c source with $$CHECK")
message("  ${generator} RECORDMCOUNT_WARN=1 [targets] Warn about ignored mcount sections")
message("  ${generator} W=n   [targets] Enable extra gcc checks, n=1,2,3 where")
message("		1: warnings which may be relevant and do not occur too often")
message("		2: warnings which occur quite often but may still be relevant")
message("		3: more obscure warnings, can most likely be ignored")
message("		Multiple levels can be combined with W=12 or W=123")
