#!/usr/bin/env bash
# Copyright 2022 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../../_mesh_test.sh

# test IV index recovery procedure
RunTest mesh_ivi_recovery ivi_ivu_recovery

overlay=overlay_psa_conf
RunTest mesh_ivi_recovery_psa ivi_ivu_recovery
