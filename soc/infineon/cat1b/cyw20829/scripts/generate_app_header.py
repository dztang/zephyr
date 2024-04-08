#
# Copyright (c) 2024 Cypress Semiconductor Corporation.
# SPDX-License-Identifier: Apache-2.0
#

import sys
import os

if len(sys.argv) < 4:
    print("Usage: python generate_app_header.py <path_to_binary> <BOOTSTRAP_SIZE> <BOOTSTRAP_DEST>")
    sys.exit(1)

if not os.path.exists(os.path.dirname(sys.argv[1])):
    print(f"The path '{os.path.dirname(sys.argv[1])}' does not exist.")
    sys.exit(1)

boot_strap_size = int(sys.argv[2], 16)
boot_dest = int(sys.argv[3])

with open(sys.argv[1], 'wb') as file:
    # TOC2_SIZE_HEX | L1_APP_DESCR_ADDR_HEX | SERVICE_APP_DESCR_ADDR_HEX | DEBUG_CERT_ADDR_HEX
    toc2_data = bytes([0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    # L1_APP_DESCR_SIZE_HEX | BOOT_STRAP_ADDR_HEX | BOOT_STRAP_DST_ADDR_HEX | BOOT_STRAP_SIZE_HEX
    l1_desc = bytearray([0x1c, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00])
    l1_desc.extend(boot_dest.to_bytes(4, 'little'))
    l1_desc.extend(boot_strap_size.to_bytes(4, 'little'))
    l1_desc.extend(bytearray(16))

    l1_usr_app_hdr = bytes([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])

    # Write the binary data to the file
    file.write(toc2_data)
    file.write(l1_desc)
    file.write(l1_usr_app_hdr)

    print("Application header generated successfully..")
