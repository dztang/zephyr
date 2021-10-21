/*
 * Copyright (c) 2018-2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <soc.h>
#include <bluetooth/hci.h>
#include <sys/byteorder.h>

#include "util/util.h"
#include "util/memq.h"
#include "util/mem.h"
#include "util/mayfly.h"

#include "hal/cpu.h"
#include "hal/ccm.h"
#include "hal/radio.h"
#include "hal/ticker.h"

#include "ticker/ticker.h"

#include "pdu.h"

#include "lll.h"
#include "lll_clock.h"
#include "lll/lll_vendor.h"
#include "lll/lll_adv_types.h"
#include "lll_adv.h"
#include "lll/lll_adv_pdu.h"
#include "lll_chan.h"
#include "lll/lll_df_types.h"
#include "lll_conn.h"
#include "lll_peripheral.h"
#include "lll_filter.h"

#if !defined(CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY)
#include "ull_tx_queue.h"
#endif

#include "ull_adv_types.h"
#include "ull_conn_types.h"
#include "ull_filter.h"

#include "ull_internal.h"
#include "ull_adv_internal.h"
#include "ull_conn_internal.h"
#include "ull_periph_internal.h"

#include "ll.h"

#if (!defined(CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY))
#include "ll_sw/ull_llcp.h"
#endif

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_DEBUG_HCI_DRIVER)
#define LOG_MODULE_NAME bt_ctlr_ull_periph
#include "common/log.h"
#include "hal/debug.h"

static void invalid_release(struct ull_hdr *hdr, struct lll_conn *lll,
			    memq_link_t *link, struct node_rx_hdr *rx);
static void ticker_op_stop_adv_cb(uint32_t status, void *param);
static void ticker_op_cb(uint32_t status, void *param);
static void ticker_update_latency_cancel_op_cb(uint32_t ticker_status,
					       void *param);

void ull_periph_setup(struct node_rx_hdr *rx, struct node_rx_ftr *ftr,
		     struct lll_conn *lll)
{
	uint32_t conn_offset_us, conn_interval_us;
	uint8_t ticker_id_adv, ticker_id_conn;
	uint8_t peer_id_addr[BDADDR_SIZE];
	uint8_t peer_addr[BDADDR_SIZE];
	uint32_t ticks_slot_overhead;
	uint32_t ticks_slot_offset;
	uint32_t ready_delay_us;
	struct pdu_adv *pdu_adv;
	struct ll_adv_set *adv;
	uint32_t ticker_status;
	uint8_t peer_addr_type;
	uint16_t win_delay_us;
	struct node_rx_cc *cc;
	struct ll_conn *conn;
	uint16_t max_tx_time;
	uint16_t max_rx_time;
	uint16_t win_offset;
	memq_link_t *link;
	uint16_t timeout;
	uint8_t chan_sel;

	adv = ((struct lll_adv *)ftr->param)->hdr.parent;
	conn = lll->hdr.parent;

	/* Populate the peripheral context */
	pdu_adv = (void *)((struct node_rx_pdu *)rx)->pdu;

	peer_addr_type = pdu_adv->tx_addr;
	memcpy(peer_addr, pdu_adv->connect_ind.init_addr, BDADDR_SIZE);

#if defined(CONFIG_BT_CTLR_PRIVACY)
	uint8_t rl_idx = ftr->rl_idx;

	if (rl_idx != FILTER_IDX_NONE) {
		/* Get identity address */
		ll_rl_id_addr_get(rl_idx, &peer_addr_type, peer_id_addr);
		/* Mark it as identity address from RPA (0x02, 0x03) */
		peer_addr_type += 2;
	} else {
#else /* CONFIG_BT_CTLR_PRIVACY */
	if (1) {
#endif /* CONFIG_BT_CTLR_PRIVACY */
		memcpy(peer_id_addr, peer_addr, BDADDR_SIZE);
	}

	/* Use the link stored in the node rx to enqueue connection
	 * complete node rx towards LL context.
	 */
	link = rx->link;

#if defined(CONFIG_BT_CTLR_CHECK_SAME_PEER_CONN)
	const uint8_t peer_id_addr_type = (peer_addr_type & 0x01);
	const uint8_t own_id_addr_type = pdu_adv->rx_addr;
	const uint8_t *own_id_addr = adv->own_id_addr;

	/* Do not connect twice to the same peer */
	if (ull_conn_peer_connected(own_id_addr_type, own_id_addr,
				    peer_id_addr_type, peer_id_addr)) {
		invalid_release(&adv->ull, lll, link, rx);

		return;
	}

	/* Remember peer and own identity address */
	conn->peer_id_addr_type = peer_id_addr_type;
	(void)memcpy(conn->peer_id_addr, peer_id_addr,
		     sizeof(conn->peer_id_addr));
	conn->own_id_addr_type = own_id_addr_type;
	(void)memcpy(conn->own_id_addr, own_id_addr,
		     sizeof(conn->own_id_addr));
#endif /* CONFIG_BT_CTLR_CHECK_SAME_PEER_CONN */

	memcpy(&lll->crc_init[0], &pdu_adv->connect_ind.crc_init[0], 3);
	memcpy(&lll->access_addr[0], &pdu_adv->connect_ind.access_addr[0], 4);
	memcpy(&lll->data_chan_map[0], &pdu_adv->connect_ind.chan_map[0],
	       sizeof(lll->data_chan_map));
	lll->data_chan_count = util_ones_count_get(&lll->data_chan_map[0],
			       sizeof(lll->data_chan_map));
	lll->data_chan_hop = pdu_adv->connect_ind.hop;
	lll->interval = sys_le16_to_cpu(pdu_adv->connect_ind.interval);
	if ((lll->data_chan_count < 2) || (lll->data_chan_hop < 5) ||
	    (lll->data_chan_hop > 16) || !lll->interval) {
		invalid_release(&adv->ull, lll, link, rx);

		return;
	}

	((struct lll_adv *)ftr->param)->conn = NULL;

	lll->latency = sys_le16_to_cpu(pdu_adv->connect_ind.latency);

	win_offset = sys_le16_to_cpu(pdu_adv->connect_ind.win_offset);
	conn_interval_us = lll->interval * CONN_INT_UNIT_US;

	/* transmitWindowDelay to default calculated connection offset:
	 * 1.25ms for a legacy PDU, 2.5ms for an LE Uncoded PHY and 3.75ms for
	 * an LE Coded PHY.
	 */
	if (0) {
#if defined(CONFIG_BT_CTLR_ADV_EXT)
	} else if (adv->lll.aux) {
		if (adv->lll.phy_s & PHY_CODED) {
			win_delay_us = WIN_DELAY_CODED;
		} else {
			win_delay_us = WIN_DELAY_UNCODED;
		}
#endif
	} else {
		win_delay_us = WIN_DELAY_LEGACY;
	}

#if (!defined(CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY))
	/* Set LLCP as connection-wise connected */
	ull_cp_state_set(conn, ULL_CP_CONNECTED);
#endif /* CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY */

	/* calculate the window widening */
	conn->periph.sca = pdu_adv->connect_ind.sca;
	lll->periph.window_widening_periodic_us =
		(((lll_clock_ppm_local_get() +
		   lll_clock_ppm_get(conn->periph.sca)) *
		  conn_interval_us) + (1000000 - 1)) / 1000000U;
	lll->periph.window_widening_max_us = (conn_interval_us >> 1) -
					    EVENT_IFS_US;
	lll->periph.window_size_event_us = pdu_adv->connect_ind.win_size *
		CONN_INT_UNIT_US;

	/* procedure timeouts */
	timeout = sys_le16_to_cpu(pdu_adv->connect_ind.timeout);
	conn->supervision_reload =
		RADIO_CONN_EVENTS((timeout * 10U * 1000U), conn_interval_us);
	conn->procedure_reload =
		RADIO_CONN_EVENTS((40 * 1000 * 1000), conn_interval_us);

#if defined(CONFIG_BT_CTLR_LE_PING)
	/* APTO in no. of connection events */
	conn->apto_reload = RADIO_CONN_EVENTS((30 * 1000 * 1000),
					      conn_interval_us);
	/* Dispatch LE Ping PDU 6 connection events (that peer would
	 * listen to) before 30s timeout
	 * TODO: "peer listens to" is greater than 30s due to latency
	 */
	conn->appto_reload = (conn->apto_reload > (lll->latency + 6)) ?
			     (conn->apto_reload - (lll->latency + 6)) :
			     conn->apto_reload;
#endif /* CONFIG_BT_CTLR_LE_PING */

#if defined(CONFIG_BT_CTLR_CONN_RANDOM_FORCE)
	memcpy((void *)&conn->periph.force, &lll->access_addr[0],
	       sizeof(conn->periph.force));
#endif /* CONFIG_BT_CTLR_CONN_RANDOM_FORCE */

	if (0) {
#if defined(CONFIG_BT_CTLR_ADV_EXT)
	} else if (adv->lll.aux) {
		chan_sel = 1U;
#endif
	} else {
		chan_sel = pdu_adv->chan_sel;
	}

	cc = (void *)pdu_adv;
	cc->status = 0U;
	cc->role = 1U;

#if defined(CONFIG_BT_CTLR_PRIVACY)
	if (ull_filter_lll_lrpa_used(adv->lll.rl_idx)) {
		memcpy(&cc->local_rpa[0], &pdu_adv->connect_ind.adv_addr[0],
		       BDADDR_SIZE);
	} else {
		memset(&cc->local_rpa[0], 0x0, BDADDR_SIZE);
	}

	if (rl_idx != FILTER_IDX_NONE) {
		/* Store peer RPA */
		memcpy(cc->peer_rpa, peer_addr, BDADDR_SIZE);
	} else {
		memset(cc->peer_rpa, 0x0, BDADDR_SIZE);
	}
#endif /* CONFIG_BT_CTLR_PRIVACY */

	cc->peer_addr_type = peer_addr_type;
	memcpy(cc->peer_addr, peer_id_addr, BDADDR_SIZE);

	cc->interval = lll->interval;
	cc->latency = lll->latency;
	cc->timeout = timeout;
	cc->sca = conn->periph.sca;

	lll->handle = ll_conn_handle_get(conn);
	rx->handle = lll->handle;

#if defined(CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL)
	lll->tx_pwr_lvl = RADIO_TXP_DEFAULT;
#endif /* CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL */

	/* Use Channel Selection Algorithm #2 if peer too supports it */
	if (IS_ENABLED(CONFIG_BT_CTLR_CHAN_SEL_2)) {
		struct node_rx_pdu *rx_csa;
		struct node_rx_cs *cs;

		/* pick the rx node instance stored within the connection
		 * rx node.
		 */
		rx_csa = (void *)ftr->extra;

		/* Enqueue the connection event */
		ll_rx_put(link, rx);

		/* use the rx node for CSA event */
		rx = (void *)rx_csa;
		link = rx->link;

		rx->handle = lll->handle;
		rx->type = NODE_RX_TYPE_CHAN_SEL_ALGO;

		cs = (void *)rx_csa->pdu;

		if (chan_sel) {
			lll->data_chan_sel = 1;
			lll->data_chan_id = lll_chan_id(lll->access_addr);

			cs->csa = 0x01;
		} else {
			cs->csa = 0x00;
		}
	}

#if defined(CONFIG_BT_CTLR_ADV_EXT)
	if (ll_adv_cmds_is_ext()) {
		uint8_t handle;

		/* Enqueue connection or CSA event */
		ll_rx_put(link, rx);

		/* use reserved link and node_rx to prepare
		 * advertising terminate event
		 */
		rx = adv->lll.node_rx_adv_term;
		link = rx->link;

		handle = ull_adv_handle_get(adv);
		LL_ASSERT(handle < BT_CTLR_ADV_SET);

		rx->type = NODE_RX_TYPE_EXT_ADV_TERMINATE;
		rx->handle = handle;
		rx->rx_ftr.param_adv_term.status = 0U;
		rx->rx_ftr.param_adv_term.conn_handle = lll->handle;
		rx->rx_ftr.param_adv_term.num_events = 0U;
	}
#endif

	ll_rx_put(link, rx);
	ll_rx_sched();

#if defined(CONFIG_BT_CTLR_DATA_LENGTH)
#if defined(CONFIG_BT_CTLR_PHY)
#if defined(CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY)
	max_tx_time = lll->max_tx_time;
	max_rx_time = lll->max_rx_time;
#else
	max_tx_time = lll->dle.local.max_tx_time;
	max_rx_time = lll->dle.local.max_rx_time;
#endif
#else /* !CONFIG_BT_CTLR_PHY */
	max_tx_time = PDU_DC_MAX_US(PDU_DC_PAYLOAD_SIZE_MIN, PHY_1M);
	max_rx_time = PDU_DC_MAX_US(PDU_DC_PAYLOAD_SIZE_MIN, PHY_1M);
#endif /* !CONFIG_BT_CTLR_PHY */
#else /* !CONFIG_BT_CTLR_DATA_LENGTH */
	max_tx_time = PDU_DC_MAX_US(PDU_DC_PAYLOAD_SIZE_MIN, PHY_1M);
	max_rx_time = PDU_DC_MAX_US(PDU_DC_PAYLOAD_SIZE_MIN, PHY_1M);
#if defined(CONFIG_BT_CTLR_PHY)
	max_tx_time = MAX(max_tx_time,
			  PDU_DC_MAX_US(PDU_DC_PAYLOAD_SIZE_MIN, lll->phy_tx));
	max_rx_time = MAX(max_rx_time,
			  PDU_DC_MAX_US(PDU_DC_PAYLOAD_SIZE_MIN, lll->phy_rx));
#endif /* !CONFIG_BT_CTLR_PHY */
#endif /* !CONFIG_BT_CTLR_DATA_LENGTH */

#if defined(CONFIG_BT_CTLR_PHY)
	ready_delay_us = lll_radio_rx_ready_delay_get(lll->phy_rx, 1);
#else
	ready_delay_us = lll_radio_rx_ready_delay_get(0, 0);
#endif

	/* TODO: active_to_start feature port */
	conn->ull.ticks_active_to_start = 0U;
	conn->ull.ticks_prepare_to_start =
		HAL_TICKER_US_TO_TICKS(EVENT_OVERHEAD_XTAL_US);
	conn->ull.ticks_preempt_to_start =
		HAL_TICKER_US_TO_TICKS(EVENT_OVERHEAD_PREEMPT_MIN_US);
	conn->ull.ticks_slot =
		HAL_TICKER_US_TO_TICKS(EVENT_OVERHEAD_START_US +
				       ready_delay_us +
				       max_rx_time +
				       EVENT_IFS_US +
				       max_tx_time);

	ticks_slot_offset = MAX(conn->ull.ticks_active_to_start,
				conn->ull.ticks_prepare_to_start);
	if (IS_ENABLED(CONFIG_BT_CTLR_LOW_LAT)) {
		ticks_slot_overhead = ticks_slot_offset;
	} else {
		ticks_slot_overhead = 0U;
	}
	ticks_slot_offset += HAL_TICKER_US_TO_TICKS(EVENT_OVERHEAD_START_US);

	conn_interval_us -= lll->periph.window_widening_periodic_us;

	conn_offset_us = ftr->radio_end_us;
	conn_offset_us += win_offset * CONN_INT_UNIT_US;
	conn_offset_us += win_delay_us;
	conn_offset_us -= EVENT_TICKER_RES_MARGIN_US;
	conn_offset_us -= EVENT_JITTER_US;
	conn_offset_us -= ready_delay_us;

#if (CONFIG_BT_CTLR_ULL_HIGH_PRIO == CONFIG_BT_CTLR_ULL_LOW_PRIO)
	/* disable ticker job, in order to chain stop and start to avoid RTC
	 * being stopped if no tickers active.
	 */
	mayfly_enable(TICKER_USER_ID_ULL_HIGH, TICKER_USER_ID_ULL_LOW, 0);
#endif

#if defined(CONFIG_BT_CTLR_ADV_EXT) && (CONFIG_BT_CTLR_ADV_AUX_SET > 0)
	struct lll_adv_aux *lll_aux = adv->lll.aux;

	if (lll_aux) {
		struct ll_adv_aux_set *aux;

		aux = HDR_LLL2ULL(lll_aux);

		ticker_id_adv = TICKER_ID_ADV_AUX_BASE +
				ull_adv_aux_handle_get(aux);
		ticker_status = ticker_stop(TICKER_INSTANCE_ID_CTLR,
					    TICKER_USER_ID_ULL_HIGH,
					    ticker_id_adv,
					    ticker_op_stop_adv_cb, aux);
		ticker_op_stop_adv_cb(ticker_status, aux);

		aux->is_started = 0U;
	}
#endif

	/* Stop Advertiser */
	ticker_id_adv = TICKER_ID_ADV_BASE + ull_adv_handle_get(adv);
	ticker_status = ticker_stop(TICKER_INSTANCE_ID_CTLR,
				    TICKER_USER_ID_ULL_HIGH,
				    ticker_id_adv, ticker_op_stop_adv_cb, adv);
	ticker_op_stop_adv_cb(ticker_status, adv);

	/* Stop Direct Adv Stop */
	if (adv->lll.is_hdcd) {
		/* Advertiser stop can expire while here in this ISR.
		 * Deferred attempt to stop can fail as it would have
		 * expired, hence ignore failure.
		 */
		ticker_stop(TICKER_INSTANCE_ID_CTLR, TICKER_USER_ID_ULL_HIGH,
			    TICKER_ID_ADV_STOP, NULL, NULL);
	}

	/* Start Peripheral */
	ticker_id_conn = TICKER_ID_CONN_BASE + ll_conn_handle_get(conn);
	ticker_status = ticker_start(TICKER_INSTANCE_ID_CTLR,
				     TICKER_USER_ID_ULL_HIGH,
				     ticker_id_conn,
				     ftr->ticks_anchor - ticks_slot_offset,
				     HAL_TICKER_US_TO_TICKS(conn_offset_us),
				     HAL_TICKER_US_TO_TICKS(conn_interval_us),
				     HAL_TICKER_REMAINDER(conn_interval_us),
				     TICKER_NULL_LAZY,
				     (conn->ull.ticks_slot +
				      ticks_slot_overhead),
				     ull_periph_ticker_cb, conn, ticker_op_cb,
				     (void *)__LINE__);
	LL_ASSERT((ticker_status == TICKER_STATUS_SUCCESS) ||
		  (ticker_status == TICKER_STATUS_BUSY));

#if (CONFIG_BT_CTLR_ULL_HIGH_PRIO == CONFIG_BT_CTLR_ULL_LOW_PRIO)
	/* enable ticker job, irrespective of disabled in this function so
	 * first connection event can be scheduled as soon as possible.
	 */
	mayfly_enable(TICKER_USER_ID_ULL_HIGH, TICKER_USER_ID_ULL_LOW, 1);
#endif
}

void ull_periph_latency_cancel(struct ll_conn *conn, uint16_t handle)
{
	/* break peripheral latency */
	if (conn->lll.latency_event && !conn->periph.latency_cancel) {
		uint32_t ticker_status;

		conn->periph.latency_cancel = 1U;

		ticker_status =
			ticker_update(TICKER_INSTANCE_ID_CTLR,
				      TICKER_USER_ID_THREAD,
				      (TICKER_ID_CONN_BASE + handle),
				      0, 0, 0, 0, 1, 0,
				      ticker_update_latency_cancel_op_cb,
				      (void *)conn);
		LL_ASSERT((ticker_status == TICKER_STATUS_SUCCESS) ||
			  (ticker_status == TICKER_STATUS_BUSY));
	}
}

void ull_periph_ticker_cb(uint32_t ticks_at_expire, uint32_t ticks_drift,
			 uint32_t remainder, uint16_t lazy, uint8_t force,
			 void *param)
{
	static memq_link_t link;
	static struct mayfly mfy = {0, 0, &link, NULL, lll_periph_prepare};
	static struct lll_prepare_param p;
	struct ll_conn *conn;
	uint32_t err;
	uint8_t ref;

	DEBUG_RADIO_PREPARE_S(1);

	conn = param;

	/* Check if stopping ticker (on disconnection, race with ticker expiry)
	 */
	if (unlikely(conn->lll.handle == 0xFFFF)) {
		DEBUG_RADIO_CLOSE_S(0);
		return;
	}

#if defined(CONFIG_BT_CTLR_CONN_META)
	conn->common.is_must_expire = (lazy == TICKER_LAZY_MUST_EXPIRE);
#endif
	/* If this is a must-expire callback, LLCP state machine does not need
	 * to know. Will be called with lazy > 0 when scheduled in air.
	 */
	if (!IS_ENABLED(CONFIG_BT_CTLR_CONN_META) ||
	    (lazy != TICKER_LAZY_MUST_EXPIRE)) {
		int ret;

		/* Handle any LL Control Procedures */
		ret = ull_conn_llcp(conn, ticks_at_expire, lazy);
		if (ret) {
			/* NOTE: Under BT_CTLR_LOW_LAT, ULL_LOW context is
			 *       disabled inside radio events, hence, abort any
			 *       active radio event which will re-enable
			 *       ULL_LOW context that permits ticker job to run.
			 */
			if (IS_ENABLED(CONFIG_BT_CTLR_LOW_LAT) &&
			    (CONFIG_BT_CTLR_LLL_PRIO ==
			     CONFIG_BT_CTLR_ULL_LOW_PRIO)) {
				ll_radio_state_abort();
			}

			DEBUG_RADIO_CLOSE_S(0);
			return;
		}
	}

	/* Increment prepare reference count */
	ref = ull_ref_inc(&conn->ull);
	LL_ASSERT(ref);

	/* Append timing parameters */
	p.ticks_at_expire = ticks_at_expire;
	p.remainder = remainder;
	p.lazy = lazy;
	p.force = force;
	p.param = &conn->lll;
	mfy.param = &p;

	/* Kick LLL prepare */
	err = mayfly_enqueue(TICKER_USER_ID_ULL_HIGH, TICKER_USER_ID_LLL,
			     0, &mfy);
	LL_ASSERT(!err);

	/* De-mux remaining tx nodes from FIFO */
	ull_conn_tx_demux(UINT8_MAX);

	/* Enqueue towards LLL */
	ull_conn_tx_lll_enqueue(conn, UINT8_MAX);

	DEBUG_RADIO_PREPARE_S(1);
}

#if defined(CONFIG_BT_CTLR_LE_ENC)
uint8_t ll_start_enc_req_send(uint16_t handle, uint8_t error_code,
			    uint8_t const *const ltk)
{
	struct ll_conn *conn;

	conn = ll_connected_get(handle);
	if (!conn) {
		return BT_HCI_ERR_UNKNOWN_CONN_ID;
	}

#if defined(CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY)
	if (error_code) {
		if (conn->llcp_enc.refresh == 0U) {
			if ((conn->llcp_req == conn->llcp_ack) ||
			     (conn->llcp_type != LLCP_ENCRYPTION)) {
				return BT_HCI_ERR_CMD_DISALLOWED;
			}

			conn->llcp.encryption.error_code = error_code;
			conn->llcp.encryption.state = LLCP_ENC_STATE_INPROG;
		} else {
			if (conn->llcp_terminate.ack !=
			    conn->llcp_terminate.req) {
				return BT_HCI_ERR_CMD_DISALLOWED;
			}

			conn->llcp_terminate.reason_own = error_code;

			conn->llcp_terminate.req++;
		}
	} else {
		if ((conn->llcp_req == conn->llcp_ack) ||
		     (conn->llcp_type != LLCP_ENCRYPTION)) {
			return BT_HCI_ERR_CMD_DISALLOWED;
		}

		memcpy(&conn->llcp_enc.ltk[0], ltk,
		       sizeof(conn->llcp_enc.ltk));

		conn->llcp.encryption.error_code = 0U;
		conn->llcp.encryption.state = LLCP_ENC_STATE_INPROG;
	}
#else /* CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY */
	/*
	 * EGON TODO: add info to the conn-structure
	 * - refresh
	 * - no procedure in progress
	 * - procedure type
	 * and use that info to decide if the cmd is allowed
	 * or if we should terminate the connection
	 * see BT 5.2 Vol. 6 part B chapter 5.1.3
	 * see also ull_periph.c line 395-439
	 *
	 * EGON TODO: the ull_cp_ltx_req* functions should return success/fail status
	 */
	if (error_code) {
		ull_cp_ltk_req_neq_reply(conn);
	} else {
		ull_cp_ltk_req_reply(conn, ltk);
	}
#endif /* CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY */

	return 0;
}
#endif /* CONFIG_BT_CTLR_LE_ENC */

static void invalid_release(struct ull_hdr *hdr, struct lll_conn *lll,
			    memq_link_t *link, struct node_rx_hdr *rx)
{
	/* Reset the advertising disabled callback */
	hdr->disabled_cb = NULL;

	/* Let the advertiser continue with connectable advertising */
	lll->periph.initiated = 0U;

	/* Mark for buffer for release */
	rx->type = NODE_RX_TYPE_RELEASE;

	/* Release CSA#2 related node rx too */
	if (IS_ENABLED(CONFIG_BT_CTLR_CHAN_SEL_2)) {
		struct node_rx_pdu *rx_csa;

		/* pick the rx node instance stored within the
		 * connection rx node.
		 */
		rx_csa = rx->rx_ftr.extra;

		/* Enqueue the connection event to be release */
		ll_rx_put(link, rx);

		/* Use the rx node for CSA event */
		rx = (void *)rx_csa;
		link = rx->link;

		/* Mark for buffer for release */
		rx->type = NODE_RX_TYPE_RELEASE;
	}

	/* Enqueue connection or CSA event to be release */
	ll_rx_put(link, rx);
	ll_rx_sched();
}

static void ticker_op_stop_adv_cb(uint32_t status, void *param)
{
	LL_ASSERT(status != TICKER_STATUS_FAILURE ||
		  param == ull_disable_mark_get());
}

static void ticker_op_cb(uint32_t status, void *param)
{
	ARG_UNUSED(param);

	LL_ASSERT(status == TICKER_STATUS_SUCCESS);
}

static void ticker_update_latency_cancel_op_cb(uint32_t ticker_status,
					       void *param)
{
	struct ll_conn *conn = param;

	LL_ASSERT(ticker_status == TICKER_STATUS_SUCCESS);

	conn->periph.latency_cancel = 0U;
}

#if !defined(CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY)
#if defined(CONFIG_BT_CTLR_MIN_USED_CHAN)
uint8_t ll_set_min_used_chans(uint16_t handle, uint8_t const phys,
			      uint8_t const min_used_chans)
{
	struct ll_conn *conn;

	conn = ll_connected_get(handle);
	if (!conn) {
		return BT_HCI_ERR_UNKNOWN_CONN_ID;
	}

	if (!conn->lll.role) {
		return BT_HCI_ERR_CMD_DISALLOWED;
	}

	return ull_cp_min_used_chans(conn, phys, min_used_chans);
}
#endif /* CONFIG_BT_CTLR_MIN_USED_CHAN */
#endif /* !CONFIG_BT_LL_SW_SPLIT_LLCP_LEGACY */
