/*
 * Copyright 2018 - 2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __USB_DEVICE_CONFIG_H__
#define __USB_DEVICE_CONFIG_H__

#include <zephyr/devicetree.h>
#include "usb.h"

/******************************************************************************
 * Definitions
 *****************************************************************************/
#ifdef CONFIG_USB_DEVICE_DRIVER
/* EHCI instance count */
#ifdef CONFIG_USB_DC_NXP_EHCI
#define USB_DEVICE_CONFIG_EHCI (1U)
/* How many the DTD are supported. */
#define USB_DEVICE_CONFIG_EHCI_MAX_DTD (16U)
#endif /* CONFIG_USB_DC_NXP_EHCI */

#ifdef CONFIG_USB_DC_NXP_LPCIP3511

#ifdef USBHSD_BASE_ADDRS
#define USB_DEVICE_CONFIG_LPCIP3511HS (1U)
#else
#define USB_DEVICE_CONFIG_LPCIP3511HS (0U)
#endif

#ifdef USB_BASE_ADDRS
#define USB_DEVICE_CONFIG_LPCIP3511FS (1U)
#else
#define USB_DEVICE_CONFIG_LPCIP3511FS (0U)
#endif

#endif /* CONFIG_USB_DC_NXP_LPCIP3511 */

/* Whether device is self power. 1U supported, 0U not supported */
#define USB_DEVICE_CONFIG_SELF_POWER (1U)

#define NUM_INSTS DT_NUM_INST_STATUS_OKAY(nxp_ehci) + DT_NUM_INST_STATUS_OKAY(nxp_lpcip3511)
BUILD_ASSERT(NUM_INSTS <= 1, "Only one USB device supported");
#if DT_HAS_COMPAT_STATUS_OKAY(nxp_lpcip3511)
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT nxp_lpcip3511
#elif DT_HAS_COMPAT_STATUS_OKAY(nxp_ehci)
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT nxp_ehci
#endif

/* Number of endpoints supported */
#define USB_DEVICE_CONFIG_ENDPOINTS (DT_INST_PROP(0, num_bidir_endpoints))
#else

#ifdef CONFIG_USB_UDC_NXP_EHCI
#define USB_DEVICE_CONFIG_EHCI (DT_NUM_INST_STATUS_OKAY(nxp_ehci))
#endif

#ifdef CONFIG_USB_UDC_NXP_IP3511

#if defined(USBHSD_BASE_ADDRS) && defined(USB_BASE_ADDRS)
#define USB_DEVICE_CONFIG_LPCIP3511HS (1U)
#define USB_DEVICE_CONFIG_LPCIP3511FS (1U)

#else
#ifdef USBHSD_BASE_ADDRS
#define USB_DEVICE_CONFIG_LPCIP3511HS (DT_NUM_INST_STATUS_OKAY(nxp_lpcip3511))
#else
#define USB_DEVICE_CONFIG_LPCIP3511HS (0U)
#endif

#ifdef USB_BASE_ADDRS
#define USB_DEVICE_CONFIG_LPCIP3511FS (DT_NUM_INST_STATUS_OKAY(nxp_lpcip3511))
#else
#define USB_DEVICE_CONFIG_LPCIP3511FS (0U)
#endif

#endif
#endif

/* calculte the num of endponts.
 * mcux ip3511 driver doesn't use USB_DEVICE_CONFIG_ENDPOINTS,
 * so use ehci endpoint number if ehci is enabled.
 */
#if DT_HAS_COMPAT_STATUS_OKAY(nxp_ehci)
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT nxp_ehci
#elif DT_HAS_COMPAT_STATUS_OKAY(nxp_lpcip3511)
#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT nxp_lpcip3511
#endif

/* Number of endpoints supported */
#define USB_DEVICE_CONFIG_ENDPOINTS (DT_INST_PROP(0, num_bidir_endpoints))

#define USB_DEVICE_CONFIG_SELF_POWER (1U)

#if ((defined(USB_DEVICE_CONFIG_EHCI)) && (USB_DEVICE_CONFIG_EHCI > 0U))
/*! @brief How many the DTD are supported. */
#define USB_DEVICE_CONFIG_EHCI_MAX_DTD (16U)

/*! @brief Whether the EHCI ID pin detect feature enabled. */
#define USB_DEVICE_CONFIG_EHCI_ID_PIN_DETECT (0U)
#endif

#endif

#endif /* __USB_DEVICE_CONFIG_H__ */
