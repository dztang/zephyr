/*
 * Copyright (c) 2017, 2020 Nordic Semiconductor ASA
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/storage/stream_flash.h>

#ifdef CONFIG_IMG_ERASE_PROGRESSIVELY
#include <bootutil/bootutil_public.h>
#include <zephyr/dfu/mcuboot.h>
#endif

#include <zephyr/devicetree.h>
#if defined(CONFIG_TRUSTED_EXECUTION_NONSECURE) && \
	!defined(CONFIG_IMG_IGNORE_NONSECURE_PARTITION)
	#define UPLOAD_FLASH_AREA_LABEL slot1_ns_partition
#else
#if FIXED_PARTITION_EXISTS(slot1_partition)
	#define UPLOAD_FLASH_AREA_LABEL slot1_partition
#else
	#define UPLOAD_FLASH_AREA_LABEL slot0_partition
#endif
#endif

/* FIXED_PARTITION_ID() values used below are auto-generated by DT */
#define UPLOAD_FLASH_AREA_ID FIXED_PARTITION_ID(UPLOAD_FLASH_AREA_LABEL)
#define UPLOAD_FLASH_AREA_CONTROLLER \
	DT_GPARENT(DT_NODELABEL(UPLOAD_FLASH_AREA_LABEL))

#if DT_NODE_HAS_PROP(UPLOAD_FLASH_AREA_CONTROLLER, write_block_size)
#define FLASH_WRITE_BLOCK_SIZE \
	DT_PROP(UPLOAD_FLASH_AREA_CONTROLLER, write_block_size)

BUILD_ASSERT((CONFIG_IMG_BLOCK_BUF_SIZE % FLASH_WRITE_BLOCK_SIZE == 0),
	     "CONFIG_IMG_BLOCK_BUF_SIZE is not a multiple of "
	     "FLASH_WRITE_BLOCK_SIZE");
#endif

int flash_img_buffered_write(struct flash_img_context *ctx, const uint8_t *data,
			     size_t len, bool flush)
{
	int rc;

	rc = stream_flash_buffered_write(&ctx->stream, data, len, flush);
	if (!flush) {
		return rc;
	}

#ifdef CONFIG_IMG_ERASE_PROGRESSIVELY
	ssize_t status_offset = boot_get_trailer_status_offset(
		ctx->flash_area->fa_size);
	rc = stream_flash_erase_page(&ctx->stream,
				ctx->flash_area->fa_off +
				status_offset);
	if (rc) {
		return rc;
	}
#endif

	flash_area_close(ctx->flash_area);
	ctx->flash_area = NULL;

	return rc;
}

size_t flash_img_bytes_written(struct flash_img_context *ctx)
{
	return stream_flash_bytes_written(&ctx->stream);
}

int flash_img_init_id(struct flash_img_context *ctx, uint8_t area_id)
{
	int rc;
	const struct device *flash_dev;

	rc = flash_area_open(area_id,
			       (const struct flash_area **)&(ctx->flash_area));
	if (rc) {
		return rc;
	}

	flash_dev = flash_area_get_device(ctx->flash_area);

	return stream_flash_init(&ctx->stream, flash_dev, ctx->buf,
			CONFIG_IMG_BLOCK_BUF_SIZE, ctx->flash_area->fa_off,
			ctx->flash_area->fa_size, NULL);
}

int flash_img_init(struct flash_img_context *ctx)
{
	return flash_img_init_id(ctx, UPLOAD_FLASH_AREA_ID);
}

#if defined(CONFIG_IMG_ENABLE_IMAGE_CHECK)
int flash_img_check(struct flash_img_context *ctx,
		    const struct flash_img_check *fic,
		    uint8_t area_id)
{
	struct flash_area_check fac;
	int rc;

	if (!ctx || !fic) {
		return -EINVAL;
	}

	rc = flash_area_open(area_id,
			     (const struct flash_area **)&(ctx->flash_area));
	if (rc) {
		return rc;
	}

	fac.match = fic->match;
	fac.clen = fic->clen;
	fac.off = 0;
	fac.rbuf = ctx->buf;
	fac.rblen = sizeof(ctx->buf);

	rc = flash_area_check_int_sha256(ctx->flash_area, &fac);

	flash_area_close(ctx->flash_area);
	ctx->flash_area = NULL;

	return rc;
}
#endif
