/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>

#include "wifi_drv_api.h"
#include "internal.h"
#include "wifi_drv_priv.h"

LOG_MODULE_DECLARE(wifi_native_sim, CONFIG_WIFI_LOG_LEVEL);

void *wifi_drv_init(void *supp_drv_if_ctx, const char *iface_name,
		    struct zep_wpa_supp_dev_callbk_fns *callbacks)
{
	static bool started;
	struct wifi_context *ctx;
	const struct device *dev;
	struct net_if *iface;

	iface = net_if_get_by_index(net_if_get_by_name(iface_name));
	if (iface == NULL) {
		LOG_ERR("Network interface %s not found", iface_name);
		return NULL;
	}

	dev = net_if_get_device(iface);
	if (dev == NULL) {
		LOG_ERR("Device for %s not found", iface_name);
		return NULL;
	}

	ctx = dev->data;
	ctx->supplicant_drv_ctx = supp_drv_if_ctx;
	memcpy(&ctx->supplicant_callbacks, callbacks, sizeof(ctx->supplicant_callbacks));

	if (!started) {
		ctx->host_context = host_wifi_drv_init(ctx->supplicant_drv_ctx,
						       ctx->if_name_host, 0,
						       CONFIG_WIFI_NATIVE_SIM_SUPPLICANT_CONF_FILE,
						       CONFIG_WIFI_NATIVE_SIM_SUPPLICANT_LOG_FILE,
						       CONFIG_WIFI_NM_WPA_SUPPLICANT_DEBUG_LEVEL);
		if (ctx->host_context == NULL) {
			LOG_ERR("Cannot start host handler thread!");
			return NULL;
		}

		started = true;
	}


	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	return ctx;
}

void wifi_drv_deinit(void *if_priv)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	host_wifi_drv_deinit(ctx->host_context);

	ctx->supplicant_drv_ctx = NULL;
}

int wifi_drv_scan2(void *if_priv, struct wpa_driver_scan_params *params)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	/* Starting the host scan */
	int ret = host_wifi_drv_scan2(ctx->host_context, (void *)params);

	/* and telling Zephyr supplicant driver that scan has been started */
	ctx->supplicant_callbacks.scan_start(ctx->supplicant_drv_ctx);

	return ret;
}

int wifi_drv_scan_abort(void *if_priv)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return host_wifi_drv_scan_abort(ctx->host_context);
}

int wifi_drv_get_scan_results2(void *if_priv)
{
	struct wifi_context *ctx = if_priv;
	struct wpa_scan_results *results;
	int i;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	/* Get the scan results from nl80211 driver */
	results = (struct wpa_scan_results *)host_wifi_drv_get_scan_results(ctx->host_context);
	if (results == NULL) {
		LOG_DBG("No scan results.");
		return -ENOENT;
	}

	/* Convert the scan result from nl80211 driver to how zephyr uses them */
	for (i = 0; i < results->num; i++) {
		ctx->supplicant_callbacks.scan_res(ctx->supplicant_drv_ctx,
						   results->res[i], false);
	}

	ctx->my_scan_cb(ctx, results);

	host_wifi_drv_free_scan_results(results);

	return 0;
}

int wifi_drv_deauthenticate(void *if_priv, const char *addr, unsigned short reason_code)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return 0;
}

int wifi_drv_authenticate(void *if_priv, struct wpa_driver_auth_params *params,
			  struct wpa_bss *curr_bss)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return 0;
}

int wifi_drv_associate(void *if_priv, struct wpa_driver_associate_params *params)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return host_wifi_drv_associate(ctx->host_context, params);
}

int wifi_drv_set_key(void *if_priv, const unsigned char *ifname, enum wpa_alg alg,
		     const unsigned char *addr, int key_idx, int set_tx,
		     const unsigned char *seq, size_t seq_len,
		     const unsigned char *key, size_t key_len)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return 0;
}

int wifi_drv_set_supp_port(void *if_priv, int authorized, char *bssid)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return 0;
}

int wifi_drv_signal_poll(void *if_priv, struct wpa_signal_info *si, unsigned char *bssid)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return 0;
}

int wifi_drv_send_mlme(void *if_priv, const u8 *data, size_t data_len, int noack, unsigned int freq,
		       int no_cck, int offchanok, unsigned int wait_time, int cookie)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return 0;
}

int wifi_drv_get_wiphy(void *priv)
{
	struct wpa_supp_event_supported_band *band = NULL;
	struct wifi_context *ctx = priv;
	int ret, i, count;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	ret = host_wifi_drv_get_wiphy(ctx->host_context, (void **)&band, &count);
	if (ret < 0) {
		return ret;
	}

	if (band == NULL) {
		return -ENOENT;
	}

	for (i = 0; i < count; i++) {
		ctx->supplicant_callbacks.get_wiphy_res(ctx->supplicant_drv_ctx, &band[i]);
	}

	ctx->supplicant_callbacks.get_wiphy_res(ctx->supplicant_drv_ctx, NULL);

	host_wifi_drv_free_bands(band);

	return 0;
}

int wifi_drv_register_frame(void *priv, u16 type, const u8 *match, size_t match_len,
			    bool multicast)
{
	struct wifi_context *ctx = priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return host_wifi_drv_register_frame(ctx->host_context, type, match, match_len, multicast);
}

int wifi_drv_get_capa(void *if_priv, struct wpa_driver_capa *capa)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return host_wifi_drv_get_capa(ctx->host_context, capa);
}

int wifi_drv_get_conn_info(void *if_priv, struct wpa_conn_info *info)
{
	struct wifi_context *ctx = if_priv;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index,
		net_if_get_device(ctx->iface)->name, net_if_get_device(ctx->iface));

	return 0;
}

void wifi_drv_cb_scan_start(struct zep_drv_if_ctx *if_ctx)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_scan_done(struct zep_drv_if_ctx *if_ctx, union wpa_event_data *event)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_scan_res(struct zep_drv_if_ctx *if_ctx, struct wpa_scan_res *r, bool more_res)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_auth_resp(struct zep_drv_if_ctx *if_ctx, union wpa_event_data *event)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_assoc_resp(struct zep_drv_if_ctx *if_ctx, union wpa_event_data *event,
			    unsigned int status)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_deauth(struct zep_drv_if_ctx *if_ctx, union wpa_event_data *event)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_disassoc(struct zep_drv_if_ctx *if_ctx, union wpa_event_data *event)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_mgmt_tx_status(struct zep_drv_if_ctx *if_ctx, const u8 *frame, size_t len,
				bool ack)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_unprot_deauth(struct zep_drv_if_ctx *if_ctx, union wpa_event_data *event)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_unprot_disassoc(struct zep_drv_if_ctx *if_ctx, union wpa_event_data *event)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_get_wiphy_res(struct zep_drv_if_ctx *if_ctx, void *band)
{
	LOG_DBG("ctx %p", if_ctx);
}

void wifi_drv_cb_mgmt_rx(struct zep_drv_if_ctx *if_ctx, char *frame, int frame_len, int frequency,
			 int rx_signal_dbm)
{
	LOG_DBG("ctx %p", if_ctx);
}
