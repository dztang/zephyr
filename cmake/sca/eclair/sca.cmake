# SPDX-License-Identifier: Apache-2.0
#
# Copyright (c) 2023, BUGSENG Srl

find_program(ECLAIR_ENV eclair_env REQUIRED)
message(STATUS "Found eclair_env: ${ECLAIR_ENV}")

find_program(ECLAIR_REPORT eclair_report REQUIRED)
message(STATUS "Found eclair_report: ${ECLAIR_REPORT}")

# ECLAIR Settings
set(ECLAIR_PROJECT_NAME "Zephyr-${BOARD}")
set(ECLAIR_OUTPUT_DIR ${CMAKE_BINARY_DIR}/sca/eclair_out)
set(ECLAIR_ECL_DIR "${ZEPHYR_BASE}/cmake/sca/eclair/ECL")
set(ECLAIR_DIAGNOSTICS_OUTPUT "${ECLAIR_OUTPUT_DIR}/DIAGNOSTIC.txt")
set(ECLAIR_DATA_DIR "${ECLAIR_OUTPUT_DIR}/data")
set(ECLAIR_BUILD_DIR "${ECLAIR_OUTPUT_DIR}/build")
set(ECLAIR_PROJECT_ECD "${ECLAIR_OUTPUT_DIR}/PROJECT.ecd")
set(CC_ALIASES "${CMAKE_C_COMPILER}")
set(CXX_ALIASES "${CMAKE_CXX_COMPILER}")
set(AS_ALIASES "${CMAKE_AS}")
set(LD_ALIASES "${CMAKE_LINKER}")
set(AR_ALIASES "${CMAKE_ASM_COMPILER_AR} ${CMAKE_C_COMPILER_AR} ${CMAKE_CXX_COMPILER_AR}")

set(ECLAIR_ENV_ADDITIONAL_OPTIONS "")
set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "")

# Default value
set(ECLAIR_RULES_SET first_analysis)

if (ECLAIR_CONFIG)
  import_kconfig(CONFIG_ECLAIR ${ECLAIR_CONFIG})
  # ECLAIR env
  if (CONFIG_ECLAIR_RULES_SET_first_analysis)
    set(ECLAIR_RULES_SET first_analysis)
  endif()
  if (CONFIG_ECLAIR_RULES_SET_STU)
    set(ECLAIR_RULES_SET STU)
  endif()
  if (CONFIG_ECLAIR_RULES_SET_STU_heavy)
    set(ECLAIR_RULES_SET STU_heavy)
  endif()
  if (CONFIG_ECLAIR_RULES_SET_WP)
    set(ECLAIR_RULES_SET WP)
  endif()
  if (CONFIG_ECLAIR_RULES_SET_std_lib)
    set(ECLAIR_RULES_SET std_lib)
  endif()
  # ECLAIR report
  if (CONFIG_ECLAIR_metrics_tab)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-metrics_tab=${ECLAIR_OUTPUT_DIR}/metrics")
  endif()
  if (CONFIG_ECLAIR_reports_tab)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-reports_tab=${ECLAIR_OUTPUT_DIR}/reports")
  endif()
  if (CONFIG_ECLAIR_summary_txt)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-summary_txt=${ECLAIR_OUTPUT_DIR}/summary_txt")
  endif()
  if (CONFIG_ECLAIR_summary_doc)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-summary_doc=${ECLAIR_OUTPUT_DIR}/summary_doc")
  endif()
  if (CONFIG_ECLAIR_summary_odt)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-summary_odt=${ECLAIR_OUTPUT_DIR}/summary_odt")
  endif()
  if (CONFIG_ECLAIR_full_txt_areas_AREAS)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-setq=report_areas,areas")
  endif()
  if (CONFIG_ECLAIR_full_txt_areas_FIRST_AREA)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-setq=report_areas,first_area")
  endif()
  if (CONFIG_ECLAIR_full_txt)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-full_txt=${ECLAIR_OUTPUT_DIR}/txt")
  endif()
  if (CONFIG_ECLAIR_full_doc_areas_AREAS)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-setq=report_areas,areas")
  endif()
  if (CONFIG_ECLAIR_full_doc_areas_FIRST_AREA)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-setq=report_areas,first_area")
  endif()
  if (CONFIG_ECLAIR_full_doc)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-full_doc=${ECLAIR_OUTPUT_DIR}/doc")
  endif()
  if (CONFIG_ECLAIR_full_odt)
    set(ECLAIR_REPORT_ADDITIONAL_OPTIONS "${ECLAIR_REPORT_ADDITIONAL_OPTIONS}" "-full_odt=${ECLAIR_OUTPUT_DIR}/odt")
  endif()
endif()

message(STATUS "ECLAIR outputs have been written to: ${ECLAIR_OUTPUT_DIR}")
message(STATUS "ECLAIR ECB files have been written to: ${ECLAIR_DATA_DIR}")
message(STATUS "ECLAIR BUILD DIR is: ${ECLAIR_BUILD_DIR}")

add_custom_target(eclair ALL
  COMMAND ${CMAKE_COMMAND} -E
    remove_directory ${ECLAIR_OUTPUT_DIR}
  COMMAND ${CMAKE_COMMAND} -E
    make_directory ${ECLAIR_OUTPUT_DIR}
  COMMAND ${CMAKE_COMMAND} -E
    make_directory ${ECLAIR_DATA_DIR}
  COMMAND ${CMAKE_COMMAND} -E env
    ECLAIR_DIAGNOSTICS_OUTPUT=${ECLAIR_DIAGNOSTICS_OUTPUT}
    ECLAIR_DATA_DIR=${ECLAIR_DATA_DIR}
    CC_ALIASES=${CC_ALIASES}
    CXX_ALIASES=${CXX_ALIASES}
    AS_ALIASES=${AS_ALIASES}
    LD_ALIASES=${LD_ALIASES}
    AR_ALIASES=${AR_ALIASES}
    --
    ${ECLAIR_ENV}
    -verbose
    -project_name=${ECLAIR_PROJECT_NAME}
    -project_root=${ZEPHYR_BASE}
    -eval_file=${ECLAIR_ECL_DIR}/analysis.ecl
    -eval_file=${ECLAIR_ECL_DIR}/analysis_${ECLAIR_RULES_SET}.ecl
    ${ECLAIR_ENV_ADDITIONAL_OPTIONS}
    -- west build -p always
                  -b ${BOARD}
                  --build-dir ${ECLAIR_BUILD_DIR}
                  ${APPLICATION_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} -E env
    ECLAIR_DATA_DIR=${ECLAIR_DATA_DIR}
    ECLAIR_OUTPUT_DIR=${ECLAIR_OUTPUT_DIR}
    ECLAIR_PROJECT_ECD=${ECLAIR_PROJECT_ECD}
    --
    ${ECLAIR_REPORT}
    -quiet
    -eval_file=${ECLAIR_ECL_DIR}/db_generation.ecl
  COMMAND ${ECLAIR_REPORT}
    -db=${ECLAIR_PROJECT_ECD}
    ${ECLAIR_REPORT_ADDITIONAL_OPTIONS}
    -overall_txt=${ECLAIR_OUTPUT_DIR}/overall.txt
  COMMAND ${CMAKE_COMMAND} -E
    cat ${ECLAIR_OUTPUT_DIR}/overall.txt
  VERBATIM
  USES_TERMINAL
  COMMAND_EXPAND_LISTS
)

