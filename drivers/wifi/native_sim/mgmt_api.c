/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi.h>

#include "mgmt_api.h"
#include "internal.h"

#include "common/ieee802_11_common.h"
#include "drivers/driver_zephyr.h"
#include "drivers/driver.h"
#include "wpa_supplicant/driver_i.h"
#include "wpa_supplicant/scan.h"
#include "rsn_supp/wpa.h"
#include "wifi_drv_api.h"
#include "wifi_drv_priv.h"

LOG_MODULE_DECLARE(wifi_native_sim, CONFIG_WIFI_LOG_LEVEL);

#define SCAN_TIMEOUT 3

static struct zep_wpa_supp_dev_ops *get_api(const struct device *dev)
{
	return ((struct net_wifi_mgmt_offload *)dev->api)->wifi_drv_ops;
}

static enum wifi_mfp_options get_mfp(struct wpa_scan_res *res)
{
	struct wpa_ie_data data;
	const uint8_t *ie;
	int ret;

	ie = wpa_scan_get_ie(res, WLAN_EID_RSN);
	if (ie == NULL) {
		return WIFI_MFP_DISABLE;
	}

	ret = wpa_parse_wpa_ie(ie, 2 + ie[1], &data);
	if (ret < 0) {
		return WIFI_MFP_UNKNOWN;
	}

	if (data.capabilities & WPA_CAPABILITY_MFPR) {
		return WIFI_MFP_REQUIRED;
	}

	if (data.capabilities & WPA_CAPABILITY_MFPC) {
		return WIFI_MFP_OPTIONAL;
	}

	return WIFI_MFP_DISABLE;
}

static int get_key_mgmt(struct wpa_scan_res *res)
{
	struct wpa_ie_data data;
	const uint8_t *ie;
	int ret;

	ie = wpa_scan_get_ie(res, WLAN_EID_RSN);
	if (ie == NULL) {
		return WPA_KEY_MGMT_NONE;
	}

	ret = wpa_parse_wpa_ie(ie, 2 + ie[1], &data);
	if (ret < 0) {
		return -EINVAL;
	}

	return data.key_mgmt;
}

static enum wifi_security_type get_security_type(struct wpa_scan_res *res)
{
	int key_mgmt;

	key_mgmt = get_key_mgmt(res);
	if (key_mgmt < 0) {
		LOG_DBG("Cannot get key mgmt (%d)", key_mgmt);
		return WIFI_SECURITY_TYPE_UNKNOWN;
	}

	if (wpa_key_mgmt_wpa_ieee8021x(key_mgmt)) {
		return WIFI_SECURITY_TYPE_EAP;

	} else if (key_mgmt & WPA_KEY_MGMT_PSK) {
		return WIFI_SECURITY_TYPE_PSK;

	} else if (key_mgmt & WPA_KEY_MGMT_NONE) {
		return WIFI_SECURITY_TYPE_NONE;

	} else if (key_mgmt & WPA_KEY_MGMT_IEEE8021X_NO_WPA) {
		return WIFI_SECURITY_TYPE_UNKNOWN;

	} else if (key_mgmt & WPA_KEY_MGMT_WPA_NONE) {
		return WIFI_SECURITY_TYPE_UNKNOWN;

	} else if (key_mgmt & WPA_KEY_MGMT_FT_PSK) {
		return WIFI_SECURITY_TYPE_UNKNOWN;

	} else if (key_mgmt & WPA_KEY_MGMT_PSK_SHA256) {
		return WIFI_SECURITY_TYPE_PSK_SHA256;

	} else if (key_mgmt & WPA_KEY_MGMT_WPS) {
		return WIFI_SECURITY_TYPE_UNKNOWN;

	} else if (key_mgmt & WPA_KEY_MGMT_SAE) {
		return WIFI_SECURITY_TYPE_SAE;

	} else if (key_mgmt & WPA_KEY_MGMT_FT_SAE) {
		return WIFI_SECURITY_TYPE_UNKNOWN;

	} else if (key_mgmt & WPA_KEY_MGMT_WAPI_PSK) {
		return WIFI_SECURITY_TYPE_WAPI;

	} else if (key_mgmt & WPA_KEY_MGMT_WAPI_CERT) {
		return WIFI_SECURITY_TYPE_WAPI;

	} else if (key_mgmt & WPA_KEY_MGMT_OWE) {
		return WIFI_SECURITY_TYPE_UNKNOWN;

	} else if (key_mgmt & WPA_KEY_MGMT_DPP) {
		return WIFI_SECURITY_TYPE_UNKNOWN;

	} else if (key_mgmt & WPA_KEY_MGMT_PASN) {
		return WIFI_SECURITY_TYPE_UNKNOWN;
	}

	return WIFI_SECURITY_TYPE_UNKNOWN;
}

static void my_scan_results(struct wifi_context *ctx,
			    struct wpa_scan_results *results)
{
	struct wifi_scan_result result;
	struct wpa_scan_res *res;
	int i;

	LOG_DBG("%d results", results->num);

	for (i = 0; i < results->num; i++) {
		const uint8_t *ssid;

		memset(&result, 0, sizeof(result));

		res = results->res[i];
		ssid = wpa_scan_get_ie(res, WLAN_EID_SSID);
		if (ssid == NULL) {
			continue;
		}

		result.ssid_length = MIN(sizeof(result.ssid), ssid[1]);
		memcpy(result.ssid, ssid + 2, result.ssid_length);

		memcpy(result.mac, res->bssid, sizeof(result.mac));
		result.mac_length = WIFI_MAC_ADDR_LEN;

		(void)ieee80211_freq_to_chan(res->freq, &result.channel);

		result.security = get_security_type(res);
		result.rssi = res->level;
		result.mfp = get_mfp(res);

		ctx->scan_cb(ctx->iface, 0, &result);
	}
}

static void scan_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct wifi_context *ctx = CONTAINER_OF(dwork,
						struct wifi_context,
						scan_work);
	union wpa_event_data event = {0};
	struct scan_info *info = &event.scan_info;

	/* Inform wpa_supplicant that scan is done */
	info->aborted = false;
	info->external_scan = 1;
	info->nl_scan_event = 1;

	ctx->supplicant_callbacks.scan_done(ctx->supplicant_drv_ctx, &event);
	ctx->scan_in_progress = false;
}

int wifi_scan(const struct device *dev, struct wifi_scan_params *params,
	      scan_result_cb_t cb)
{
	struct wifi_context *ctx = dev->data;
	struct zep_wpa_supp_dev_ops *drv = get_api(dev);
	struct wpa_driver_scan_params scan_params = {0};
	int ret;

	if (ctx->scan_in_progress) {
		LOG_DBG("Scan already in progress for %s", ctx->name);
		ret = -EBUSY;
		goto out;
	}

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* TODO: fill in the params */
	scan_params.num_ssids = 0;

	/* We could get the results pretty quick and call the scan result
	 * function here but simulate a way that a normal wifi driver
	 * would get the results.
	 */
	k_work_init_delayable(&ctx->scan_work, scan_handler);
	k_work_schedule(&ctx->scan_work, K_SECONDS(SCAN_TIMEOUT));

	(void)drv->scan2(ctx, &scan_params);

	ctx->scan_cb = cb;
	ctx->my_scan_cb = my_scan_results;
	ctx->scan_in_progress = true;

	ret = 0;
out:
	return ret;
}

int wifi_iface_status(const struct device *dev, struct wifi_iface_status *status)
{
	struct wifi_context *ctx = dev->data;
	int ret;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* TODO: add code here */

	ret = 0;

	return ret;
}

int wifi_filter(const struct device *dev, struct wifi_filter_info *filter)
{
	struct wifi_context *ctx = dev->data;
	int ret;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* TODO: add code here */

	ret = 0;

	return ret;
}

int wifi_mode(const struct device *dev, struct wifi_mode_info *mode)
{
	struct wifi_context *ctx = dev->data;
	int ret;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* TODO: add code here */

	ret = 0;

	return ret;
}

int wifi_channel(const struct device *dev, struct wifi_channel_info *channel)
{
	struct wifi_context *ctx = dev->data;
	int ret;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* TODO: add code here */

	ret = 0;

	return ret;
}

#ifdef CONFIG_NET_STATISTICS_WIFI
int wifi_get_stats(const struct device *dev, struct net_stats_wifi *stats)
{
	struct wifi_context *ctx = dev->data;
	int ret;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* Get the stats from the device */

	/* TODO: add code here, skipping this for now */
	ret = -ENOTSUP;
	goto out;

	stats->pkts.tx = 0;
	stats->pkts.rx = 0;
	stats->errors.tx = 0;
	stats->errors.rx = 0;
	stats->bytes.received = 0;
	stats->bytes.sent = 0;
	stats->sta_mgmt.beacons_rx = 0;
	stats->sta_mgmt.beacons_miss = 0;
	stats->broadcast.rx = 0;
	stats->broadcast.tx = 0;
	stats->multicast.rx = 0;
	stats->multicast.tx = 0;

	ret = 0;
out:
	return ret;
}
#endif /* CONFIG_NET_STATISTICS_WIFI */

int wifi_set_power_save(const struct device *dev,
			struct wifi_ps_params *params)
{
	struct wifi_context *ctx = dev->data;

	ARG_UNUSED(dev);
	ARG_UNUSED(params);

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	return 0;
}

int wifi_set_twt(const struct device *dev,
		 struct wifi_twt_params *twt_params)
{
	struct wifi_context *ctx = dev->data;
	int ret;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* Set target wait time */

	/* TODO: add code here, skipping this for now */
	ret = -ENOTSUP;
	goto out;

	ret = 0;
out:
	return ret;
}

int wifi_reg_domain(const struct device *dev,
		    struct wifi_reg_domain *reg_domain)
{
	struct wifi_context *ctx = dev->data;
	int ret;

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	/* Manage registration domain */

	/* TODO: add code here, skipping this for now */
	ret = -ENOTSUP;
	goto out;

	ret = 0;
out:
	return ret;
}

int wifi_get_power_save_config(const struct device *dev,
			       struct wifi_ps_config *ps_config)
{
	struct wifi_context *ctx = dev->data;

	ARG_UNUSED(dev);
	ARG_UNUSED(ps_config);

	LOG_DBG("iface %s [%d] dev %s (%p)", ctx->name, ctx->if_index, dev->name, dev);

	return 0;
}
