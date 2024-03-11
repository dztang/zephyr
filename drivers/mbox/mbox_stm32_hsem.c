/*
 * Copyright (c) 2024 Celina Sophie Kalus <hello@celinakalus.de>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/mbox.h>
#include <zephyr/drivers/clock_control/stm32_clock_control.h>

#include "stm32_hsem.h"

#define LOG_LEVEL CONFIG_MBOX_LOG_LEVEL
#include <zephyr/logging/log.h>
#include <zephyr/irq.h>
LOG_MODULE_REGISTER(mbox_stm32_hsem_ipc);

#define DT_DRV_COMPAT st_mbox_stm32_hsem

#define HSEM_CPU1                   1
#define HSEM_CPU2                   2

#if DT_NODE_EXISTS(DT_NODELABEL(cpu0))
#define HSEM_CPU_ID HSEM_CPU1
#elif DT_NODE_EXISTS(DT_NODELABEL(cpu1))
#define HSEM_CPU_ID HSEM_CPU2
#else
#error "Neither cpu0 nor cpu1 defined!"
#endif

#if HSEM_CPU_ID == HSEM_CPU1
#define ll_hsem_enableit_cier       LL_HSEM_EnableIT_C1IER
#define ll_hsem_disableit_cier      LL_HSEM_DisableIT_C1IER
#define ll_hsem_clearflag_cicr      LL_HSEM_ClearFlag_C1ICR
#define ll_hsem_isactiveflag_cmisr   LL_HSEM_IsActiveFlag_C1MISR
#else /* HSEM_CPU2 */
#define ll_hsem_enableit_cier       LL_HSEM_EnableIT_C2IER
#define ll_hsem_disableit_cier      LL_HSEM_DisableIT_C2IER
#define ll_hsem_clearflag_cicr      LL_HSEM_ClearFlag_C2ICR
#define ll_hsem_isactiveflag_cmisr   LL_HSEM_IsActiveFlag_C2MISR
#endif /* HSEM_CPU_ID */

#define MBOX_HSEM_TX_MASK(idx, _) BIT(DT_INST_PROP_BY_IDX(0, st_hsem_ids_tx, idx))
#define MBOX_HSEM_RX_MASK(idx, _) BIT(DT_INST_PROP_BY_IDX(0, st_hsem_ids_rx, idx))

#define MAX_TX_CHANNELS DT_INST_PROP_LEN(0, st_hsem_ids_tx)
#define MAX_RX_CHANNELS DT_INST_PROP_LEN(0, st_hsem_ids_rx)
#define MAX_CHANNELS (MAX_RX_CHANNELS + MAX_TX_CHANNELS)

struct mbox_stm32_hsem_data {
	const struct device *dev;
	mbox_callback_t cb[MAX_RX_CHANNELS];
	void *user_data[MAX_RX_CHANNELS];
};

static struct mbox_stm32_hsem_data stm32_hsem_mbox_data;

struct mbox_stm32_hsem_conf {
	struct stm32_pclken pclken;
	int rx_hsem_ids[MAX_RX_CHANNELS];
	int tx_ch_mask;
	int rx_ch_mask;
};

static const struct mbox_stm32_hsem_conf stm32_hsem_mbox_conf = {
	.tx_ch_mask = LISTIFY(MAX_TX_CHANNELS, MBOX_HSEM_TX_MASK, |),
	.rx_ch_mask = LISTIFY(MAX_RX_CHANNELS, MBOX_HSEM_RX_MASK, |),
	.rx_hsem_ids = DT_INST_PROP(0, st_hsem_ids_rx),
	.pclken = {
		.bus = DT_INST_CLOCKS_CELL(0, bus),
		.enr = DT_INST_CLOCKS_CELL(0, bits)
	},
};

static inline bool is_rx_channel_valid(const struct device *dev, uint32_t ch)
{
	const struct mbox_stm32_hsem_conf *const cfg = dev->config;

	return (cfg->rx_ch_mask & BIT(ch));
}

static inline bool is_tx_channel_valid(const struct device *dev, uint32_t ch)
{
	const struct mbox_stm32_hsem_conf *const cfg = dev->config;

	return (cfg->tx_ch_mask & BIT(ch));
}

static void mbox_dispatcher(const struct device *dev)
{
	struct mbox_stm32_hsem_data *data = dev->data;
	const struct mbox_stm32_hsem_conf *const cfg = dev->config;

	/* Check semaphore rx_semid interrupt status */
	for (int idx = 0; idx < MAX_RX_CHANNELS; idx++) {
		int channel = cfg->rx_hsem_ids[idx];

		if (!is_rx_channel_valid(dev, channel)) {
			continue;
		}

		if (!ll_hsem_isactiveflag_cmisr(HSEM, BIT(channel))) {
			continue;
		}

		if (data->cb[idx] != NULL) {
			data->cb[idx](dev, channel, data->user_data[idx], NULL);
		}

		/* Clear semaphore rx_semid interrupt status and masked status */
		ll_hsem_clearflag_cicr(HSEM, BIT(channel));
	}
}

static int mbox_stm32_hsem_send(const struct device *dev, uint32_t channel,
			 const struct mbox_msg *msg)
{
	if (msg) {
		LOG_ERR("Sending data not supported.");
		return -EINVAL;
	}

	if (!is_tx_channel_valid(dev, channel)) {
		return -EINVAL;
	}

	/*
	 * Locking and unlocking the hardware semaphore
	 * causes an interrupt on the receiving side.
	 */
	z_stm32_hsem_lock(channel, HSEM_LOCK_DEFAULT_RETRY);
	z_stm32_hsem_unlock(channel);

	return 0;
}

static int mbox_stm32_hsem_register_callback(const struct device *dev, uint32_t channel,
				      mbox_callback_t cb, void *user_data)
{
	struct mbox_stm32_hsem_data *data = dev->data;
	const struct mbox_stm32_hsem_conf *const cfg = dev->config;

	if (!(is_rx_channel_valid(dev, channel))) {
		return -EINVAL;
	}

	int idx = -1;

	for (int i = 0; i < MAX_RX_CHANNELS; i++) {
		if (cfg->rx_hsem_ids[i] == channel) {
			idx = i;
		}
	}

	__ASSERT_NO_MSG(idx != -1);

	data->cb[idx] = cb;
	data->user_data[idx] = user_data;

	return 0;
}

static int mbox_stm32_hsem_mtu_get(const struct device *dev)
{
	ARG_UNUSED(dev);

	/* We only support signalling */
	return 0;
}

static uint32_t mbox_stm32_hsem_max_channels_get(const struct device *dev)
{
	ARG_UNUSED(dev);

	return MAX_CHANNELS;
}

static int mbox_stm32_hsem_set_enabled(const struct device *dev, uint32_t channel, bool enable)
{
	if (!is_rx_channel_valid(dev, channel)) {
		return -EINVAL;
	}

	if (enable) {
		ll_hsem_enableit_cier(HSEM, BIT(channel));
	} else {
		ll_hsem_disableit_cier(HSEM, BIT(channel));
	}

	return 0;
}

#if HSEM_CPU_ID == HSEM_CPU1
static int mbox_stm32_clock_init(const struct device *dev)
{
	const struct mbox_stm32_hsem_conf *const cfg = dev->config;
	const struct device *const clk = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);

	if (!device_is_ready(clk)) {
		LOG_ERR("Clock control device not ready.");
		return -ENODEV;
	}

	if (clock_control_on(clk, (clock_control_subsys_t *)&cfg->pclken) != 0) {
		LOG_WRN("Failed to enable clock.");
		return -EIO;
	}

	return 0;
}
#endif /* HSEM_CPU_ID */

static int mbox_stm32_hsem_init(const struct device *dev)
{
	struct mbox_stm32_hsem_data *data = dev->data;
	const struct mbox_stm32_hsem_conf *const cfg = dev->config;
	int ret = 0;

	data->dev = dev;

#if HSEM_CPU_ID == HSEM_CPU1
	ret = mbox_stm32_clock_init(dev);

	if (ret != 0) {
		return ret;
	}
#endif /* HSEM_CPU_ID */

	ll_hsem_clearflag_cicr(HSEM, cfg->rx_ch_mask);

	/* Configure interrupt service routine */
	IRQ_CONNECT(DT_INST_IRQN(0),
		    DT_INST_IRQ(0, priority),
		    mbox_dispatcher, DEVICE_DT_INST_GET(0), 0);

	irq_enable(DT_INST_IRQN(0));

	return ret;
}

static const struct mbox_driver_api mbox_stm32_hsem_driver_api = {
	.send = mbox_stm32_hsem_send,
	.register_callback = mbox_stm32_hsem_register_callback,
	.mtu_get = mbox_stm32_hsem_mtu_get,
	.max_channels_get = mbox_stm32_hsem_max_channels_get,
	.set_enabled = mbox_stm32_hsem_set_enabled,
};

DEVICE_DT_INST_DEFINE(
	0,
	mbox_stm32_hsem_init,
	NULL,
	&stm32_hsem_mbox_data,
	&stm32_hsem_mbox_conf,
	POST_KERNEL,
	CONFIG_MBOX_INIT_PRIORITY,
	&mbox_stm32_hsem_driver_api);
