/*
 * Copyright (c) 2024 Linumiz
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/posix/fcntl.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/posix/sys/stat.h>
#include "test_fs.h"

#define FILL_SIZE 128

#define TEST_FILE_SIZE     80

static void create_file(const char *filename, uint32_t size)
{
	int fh;

	fh = open(filename, O_CREAT | O_WRONLY);
	zassert(fh >= 0, "Failed creating test file");

	uint8_t filling[FILL_SIZE];

	while (size > FILL_SIZE) {
		zassert_equal(FILL_SIZE, write(fh, filling, FILL_SIZE));
		size -= FILL_SIZE;
	}

	zassert_equal(size, write(fh, filling, size));

	zassert_ok(close(fh));
}

static void before_fn(void *unused)
{
	ARG_UNUSED(unused);

	create_file(TEST_FILE, TEST_FILE_SIZE);
}

static void after_fn(void *unused)
{
	ARG_UNUSED(unused);

	zassert_ok(unlink(TEST_FILE));
}
ZTEST_SUITE(posix_fs_fstat_test, NULL, test_mount, before_fn, after_fn, test_unmount);

/**
 * @brief Test fstat command on file
 *
 */
ZTEST(posix_fs_fstat_test, test_fs_fstat_file)
{
	int fd = -1;
	struct stat buf;

	fd = open(TEST_FILE, O_CREAT | O_WRONLY);
	zassert(fd >= 0, "Failed creating test file");

	zassert_equal(0, fstat(fd, &buf));
	zassert_equal(TEST_FILE_SIZE, buf.st_size);
	zassert_equal(S_IFREG, buf.st_mode);
	zassert_ok(close(fd));
}
