# SPDX-License-Identifier: Apache-2.0

list(APPEND LLEXT_EDK_REMOVE_FLAGS -mcpu=cortex-r5 -mfloat-abi=hard)
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

target_sources(app PRIVATE src/main.c src/pubsub.c)
zephyr_include_directories(include)
