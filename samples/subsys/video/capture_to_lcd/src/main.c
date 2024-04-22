/*
 * Copyright (c) 2024 Charles Dias <charlesdias.cd@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/video.h>
#include <lvgl.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define VIDEO_DEV_SW "VIDEO_SW_GENERATOR"

void draw_border_and_label(void)
{
	/*Create an array for the border points */
	static lv_point_t line_points[] = { {4, 0}, {156, 0}, {156, 72}, {4, 72}, {4, 0} };

	/*Create style*/
	static lv_style_t style_line;

	lv_style_init(&style_line);
	lv_style_set_line_width(&style_line, 4);
	lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_line_rounded(&style_line, true);

	/*Create a line and apply the new style */
	lv_obj_t *border = lv_line_create(lv_scr_act());
	int quant_points = ARRAY_SIZE(line_points);

	lv_line_set_points(border, line_points, quant_points);
	lv_obj_add_style(border, &style_line, 0);
	lv_obj_align(border, LV_ALIGN_BOTTOM_LEFT, 0, 0);

	lv_obj_t *label = lv_label_create(border);

	lv_label_set_text(label, "LVGL on Zephyr!");
	lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);

	lv_task_handler();
}

void draw_lvgl_logo(void)
{
	LV_IMG_DECLARE(img_lvgl_logo);
	lv_obj_t *img1 = lv_img_create(lv_scr_act());

	lv_img_set_src(img1, &img_lvgl_logo);
	lv_obj_align(img1, LV_ALIGN_TOP_MID, 0, 10);

	lv_task_handler();
}

void display_splash_screen(void)
{
	draw_border_and_label();
	draw_lvgl_logo();

	k_sleep(K_MSEC(5000));
	lv_obj_clean(lv_scr_act());
}

int main(void)
{
	const struct device *display_dev;
	struct video_format fmt;
	struct video_caps caps;
	const struct device *video;
	unsigned int frame = 0;
	size_t bsize;
	int i = 0;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

	/* Default to software video pattern generator */
	video = device_get_binding(VIDEO_DEV_SW);
	if (video == NULL) {
		LOG_ERR("Video device %s not found", VIDEO_DEV_SW);
		return 0;
	}

	/* But would be better to use a real video device if any */
#if defined(CONFIG_VIDEO_STM32_DCMI)
	struct video_buffer *buffers[1], *vbuf;
	const struct device *const dev = DEVICE_DT_GET_ONE(st_stm32_dcmi);

	if (!device_is_ready(dev)) {
		LOG_ERR("%s: device not ready.\n", dev->name);
		return 0;
	}

	video = dev;
#endif

	printk("- Device name: %s\n", video->name);

	/* Get capabilities */
	if (video_get_caps(video, VIDEO_EP_OUT, &caps)) {
		LOG_ERR("Unable to retrieve video capabilities");
		return 0;
	}

	printk("- Capabilities:\n");
	while (caps.format_caps[i].pixelformat) {
		const struct video_format_cap *fcap = &caps.format_caps[i];
		/* fourcc to string */
		printk("  %c%c%c%c width [%u; %u; %u] height [%u; %u; %u]\n",
		       (char)fcap->pixelformat,
		       (char)(fcap->pixelformat >> 8),
		       (char)(fcap->pixelformat >> 16),
		       (char)(fcap->pixelformat >> 24),
		       fcap->width_min, fcap->width_max, fcap->width_step,
		       fcap->height_min, fcap->height_max, fcap->height_step);
		i++;
	}

	/* Get default/native format */
	if (video_get_format(video, VIDEO_EP_OUT, &fmt)) {
		LOG_ERR("Unable to retrieve video format");
		return 0;
	}

	printk("- Default format: %c%c%c%c %ux%u\n", (char)fmt.pixelformat,
	       (char)(fmt.pixelformat >> 8),
	       (char)(fmt.pixelformat >> 16),
	       (char)(fmt.pixelformat >> 24),
	       fmt.width, fmt.height);

#if defined(CONFIG_VIDEO_STM32_DCMI)
	const uint16_t WIDTH_VIDEO = 160;
	const uint16_t HEIGHT_VIDEO = 120;

	fmt.width = WIDTH_VIDEO;
	fmt.height = HEIGHT_VIDEO;
	fmt.pitch = fmt.width * 2;

	/* Set format */
	if (video_set_format(video, VIDEO_EP_OUT, &fmt)) {
		LOG_ERR("Unable to set up video format");
		return 0;
	}

	/* Get format */
	if (video_get_format(video, VIDEO_EP_OUT, &fmt)) {
		LOG_ERR("Unable to retrieve video format");
		return 0;
	}

	printk("- New format: %c%c%c%c %ux%u %u\n", (char)fmt.pixelformat,
		(char)(fmt.pixelformat >> 8),
		(char)(fmt.pixelformat >> 16),
		(char)(fmt.pixelformat >> 24),
		fmt.width, fmt.height, fmt.pitch);
#endif

	/* Size to allocate for each buffer */
	bsize = fmt.pitch * fmt.height;

	/* Alloc video buffers and enqueue for capture */
	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		buffers[i] = video_buffer_alloc(bsize);
		if (buffers[i] == NULL) {
			LOG_ERR("Unable to alloc video buffer");
			return 0;
		}

		video_enqueue(video, VIDEO_EP_OUT, buffers[i]);
	}

	/* Start video capture */
	if (video_stream_start(video)) {
		LOG_ERR("Unable to start capture (interface)");
		return 0;
	}

	display_splash_screen();

	display_blanking_off(display_dev);

	const lv_img_dsc_t video_img = {
		.header.always_zero = 0,
		.header.w = WIDTH_VIDEO,
		.header.h = HEIGHT_VIDEO,
		.data_size = WIDTH_VIDEO * HEIGHT_VIDEO * sizeof(lv_color_t),
		.header.cf = LV_IMG_CF_TRUE_COLOR,
		.data = (const uint8_t *) buffers[0]->buffer,
	};

	lv_obj_t *screen = lv_img_create(lv_scr_act());

	printk("Capture started\n");

	/* Grab video frames */
	while (1) {
		int err;

		err = video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_FOREVER);
		if (err) {
			LOG_ERR("Unable to dequeue video buf");
			return 0;
		}

		printk("\rGot frame %u! size: %u; timestamp %u ms",
		       frame++, vbuf->bytesused, vbuf->timestamp);

		lv_img_set_src(screen, &video_img);
		lv_obj_align(screen, LV_ALIGN_BOTTOM_LEFT, 0, 0);

		lv_task_handler();

		err = video_enqueue(video, VIDEO_EP_OUT, vbuf);
		if (err) {
			LOG_ERR("Unable to requeue video buf");
			return 0;
		}
	}
}
