/*
 * Copyright (c) 2016 - 2023, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* This file is undergoing transition towards native Zephyr nrf USB driver. */

/** @cond INTERNAL_HIDDEN */

#ifndef NRF_USBD_COMMON_H__
#define NRF_USBD_COMMON_H__

#include <nrfx.h>
#include <hal/nrf_usbd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_usbd_common USBD driver
 * @{
 * @ingroup nrf_usbd
 * @brief   Universal Serial Bus Device (USBD) peripheral driver.
 */

/**
 * @brief Number of bytes in the endpoint.
 */
#define NRF_USBD_COMMON_EPSIZE 64

/**
 * @brief Number of bytes for isochronous endpoints.
 *
 * Number of bytes for isochronous endpoints in total.
 * This number would be shared between IN and OUT endpoint.
 * It may be also assigned totaly to one endpoint.
 * @sa nrf_usbd_isosplit_set
 * @sa nrf_usbd_isosplit_get
 */
#define NRF_USBD_COMMON_ISOSIZE 1023

/**
 * @brief The size of internal feeder buffer.
 *
 * @sa nrf_usbd_common_feeder_buffer_get
 */
#define NRF_USBD_COMMON_FEEDER_BUFFER_SIZE NRF_USBD_COMMON_EPSIZE

/**
 * @name Macros for creating endpoint identifiers.
 *
 * Auxiliary macros for creating endpoint identifiers compatible with the USB specification.
 * @{
 */

/**
 * @brief Create identifier for IN endpoint.
 *
 * Simple macro to create IN endpoint identifier for given endpoint number.
 *
 * @param[in] n Endpoint number.
 *
 * @return Endpoint identifier that connects endpoint number and endpoint direction.
 */
#define NRF_USBD_COMMON_EPIN(n)  ((nrf_usbd_common_ep_t)NRF_USBD_EPIN(n))
/**
 * @brief Create identifier for OUT endpoint.
 *
 * Simple macro to create OUT endpoint identifier for given endpoint number.
 *
 * @param[in] n Endpoint number.
 *
 * @return Endpoint identifier that connects endpoint number and endpoint direction.
 */
#define NRF_USBD_COMMON_EPOUT(n) ((nrf_usbd_common_ep_t)NRF_USBD_EPOUT(n))
/** @} */

/**
 * @brief Endpoint identifier.
 *
 * Endpoint identifier used in the driver.
 * This endpoint number is consistent with USB 2.0 specification.
 */
typedef enum {
	NRF_USBD_COMMON_EPOUT0 = NRF_USBD_EPOUT(0), /**< Endpoint OUT 0 */
	NRF_USBD_COMMON_EPOUT1 = NRF_USBD_EPOUT(1), /**< Endpoint OUT 1 */
	NRF_USBD_COMMON_EPOUT2 = NRF_USBD_EPOUT(2), /**< Endpoint OUT 2 */
	NRF_USBD_COMMON_EPOUT3 = NRF_USBD_EPOUT(3), /**< Endpoint OUT 3 */
	NRF_USBD_COMMON_EPOUT4 = NRF_USBD_EPOUT(4), /**< Endpoint OUT 4 */
	NRF_USBD_COMMON_EPOUT5 = NRF_USBD_EPOUT(5), /**< Endpoint OUT 5 */
	NRF_USBD_COMMON_EPOUT6 = NRF_USBD_EPOUT(6), /**< Endpoint OUT 6 */
	NRF_USBD_COMMON_EPOUT7 = NRF_USBD_EPOUT(7), /**< Endpoint OUT 7 */
	NRF_USBD_COMMON_EPOUT8 = NRF_USBD_EPOUT(8), /**< Endpoint OUT 8 */

	NRF_USBD_COMMON_EPIN0 = NRF_USBD_EPIN(0), /**< Endpoint IN 0 */
	NRF_USBD_COMMON_EPIN1 = NRF_USBD_EPIN(1), /**< Endpoint IN 1 */
	NRF_USBD_COMMON_EPIN2 = NRF_USBD_EPIN(2), /**< Endpoint IN 2 */
	NRF_USBD_COMMON_EPIN3 = NRF_USBD_EPIN(3), /**< Endpoint IN 3 */
	NRF_USBD_COMMON_EPIN4 = NRF_USBD_EPIN(4), /**< Endpoint IN 4 */
	NRF_USBD_COMMON_EPIN5 = NRF_USBD_EPIN(5), /**< Endpoint IN 5 */
	NRF_USBD_COMMON_EPIN6 = NRF_USBD_EPIN(6), /**< Endpoint IN 6 */
	NRF_USBD_COMMON_EPIN7 = NRF_USBD_EPIN(7), /**< Endpoint IN 7 */
	NRF_USBD_COMMON_EPIN8 = NRF_USBD_EPIN(8), /**< Endpoint IN 8 */
} nrf_usbd_common_ep_t;

/**
 * @brief Events generated by the driver.
 *
 * Enumeration of possible events that may be generated by the driver.
 */
typedef enum {
	NRF_USBD_COMMON_EVT_SOF,        /**< Start Of Frame event on USB bus detected. */
	NRF_USBD_COMMON_EVT_RESET,      /**< Reset condition on USB bus detected. */
	NRF_USBD_COMMON_EVT_SUSPEND,    /**< This device should go to suspend mode now. */
	NRF_USBD_COMMON_EVT_RESUME,     /**< This device should resume from suspend now. */
	/** Wakeup request - the USBD peripheral is ready to generate
	 * WAKEUP signal after exiting low power mode.
	 */
	NRF_USBD_COMMON_EVT_WUREQ,
	NRF_USBD_COMMON_EVT_SETUP,      /**< Setup frame received and decoded. */
	/** For Rx (OUT: Host->Device):
	 *   1. The packet has been received but there is no buffer
	 * prepared for transfer already.
	 *   2. Whole transfer has been finished.
	 *
	 *   For Tx (IN: Device->Host):
	 *   The last packet from requested transfer has been transferred
	 * over USB bus and acknowledged.
	 */
	NRF_USBD_COMMON_EVT_EPTRANSFER,
	NRF_USBD_COMMON_EVT_CNT         /**< Number of defined events. */
} nrf_usbd_common_event_type_t;

/**
 * @brief Endpoint status codes.
 *
 * Status codes that may be returned by @ref nrf_usbd_common_ep_status_get or, except for
 * @ref NRF_USBD_COMMON_EP_BUSY, reported together with @ref NRF_USBD_COMMON_EVT_EPTRANSFER.
 */
typedef enum {
	/** No error occurred. */
	NRF_USBD_COMMON_EP_OK,
	/** Data received, no buffer prepared already - waiting for configured transfer. */
	NRF_USBD_COMMON_EP_WAITING,
	/** Received number of bytes cannot fit given buffer.
	 * This error would also be returned when next_transfer function
	 * has been defined but currently received data cannot fit completely
	 * in current buffer. No data split from single endpoint transmission
	 * is supported.
	 *
	 *   When this error is reported - data is left inside endpoint
	 * buffer. Clear endpoint or prepare new buffer and read it.
	 */
	NRF_USBD_COMMON_EP_OVERLOAD,
	/** EP0 transfer can be aborted when new setup comes.
	 * Any other transfer can be aborted by USB reset or driver stopping.
	 */
	NRF_USBD_COMMON_EP_ABORTED,
	/** Transfer is in progress. */
	NRF_USBD_COMMON_EP_BUSY,
} nrf_usbd_common_ep_status_t;

/**
 * @brief Event structure.
 *
 * Structure passed to event handler.
 */
typedef struct {
	nrf_usbd_common_event_type_t type; /**< Event type. */
	union {
		struct {
			uint16_t framecnt; /**< Current value of frame counter. */
		} sof;                     /**< Data available for @ref NRF_USBD_COMMON_EVT_SOF. */
		struct {
			nrf_usbd_common_ep_t ep; /**< Endpoint number. */
		} isocrc;                  /**< Isochronouns channel endpoint number. */
		struct {
			nrf_usbd_common_ep_t ep;            /**< Endpoint number. */
			nrf_usbd_common_ep_status_t status; /**< Status for the endpoint. */
		} eptransfer;                         /**< Endpoint transfer status. */
	} data;                                       /**< Union to store event data. */
} nrf_usbd_common_evt_t;

/**
 * @brief USBD event callback function type.
 *
 * @param[in] p_event Event information structure.
 */
typedef void (*nrf_usbd_common_event_handler_t)(nrf_usbd_common_evt_t const *p_event);

/**
 * @brief Universal data pointer.
 *
 * Universal data pointer that can be used for any type of transfer.
 */
typedef union {
	void const *tx; /*!< Constant TX buffer pointer. */
	void *rx;       /*!< Writable RX buffer pointer. */
	uint32_t addr;  /*!< Numeric value used internally by the driver. */
} nrf_usbd_common_data_ptr_t;

/**
 * @brief Structure to be filled with information about the next transfer.
 *
 * This is used mainly for transfer feeders and consumers.
 * It describes a single endpoint transfer and therefore the size of the buffer
 * can never be higher than the endpoint size.
 */
typedef struct {
	/** Union with available data pointers used by the driver. */
	nrf_usbd_common_data_ptr_t p_data;
	/** Size of the requested transfer. */
	size_t size;
} nrf_usbd_common_ep_transfer_t;

/**
 * @brief Flags for the current transfer.
 *
 * Flags configured for the transfer that can be merged using the bitwise 'or' operator (|).
 */
typedef enum {
	NRF_USBD_COMMON_TRANSFER_ZLP_FLAG = 1U << 0, /*!< Add a zero-length packet. */
} nrf_usbd_common_transfer_flags_t;

/**
 * @brief Total transfer configuration.
 *
 * This structure is used to configure total transfer information.
 * It is used by internal built-in feeders and consumers.
 */
typedef struct {
	/** Union with available data pointers used by the driver. */
	nrf_usbd_common_data_ptr_t p_data;
	/** Total size of the requested transfer. */
	size_t size;
	/*!< Transfer flags. Use the @ref nrf_usbd_common_transfer_flags_t values. */
	uint32_t flags;
} nrf_usbd_common_transfer_t;

/**
 * @brief Auxiliary macro for declaring IN transfer description with optional flags.
 *
 * The base macro for creating transfers with any configuration option.
 *
 * @param name     Instance name.
 * @param tx_buff  Buffer to transfer.
 * @param tx_size  Transfer size.
 * @param tx_flags Flags for the transfer (see @ref nrf_usbd_common_transfer_flags_t).
 *
 * @return Configured variable with total transfer description.
 */
#define NRF_USBD_COMMON_TRANSFER_IN(name, tx_buff, tx_size, tx_flags)                 \
	const nrf_usbd_common_transfer_t name = {                                     \
		.p_data = {.tx = (tx_buff)}, .size = (tx_size), .flags = (tx_flags)}

/**
 * @brief Helper macro for declaring OUT transfer item (@ref nrf_usbd_common_transfer_t).
 *
 * @param name    Instance name.
 * @param rx_buff Buffer to transfer.
 * @param rx_size Transfer size.
 */
#define NRF_USBD_COMMON_TRANSFER_OUT(name, rx_buff, rx_size)                          \
	const nrf_usbd_common_transfer_t name = {                                     \
		.p_data = {.rx = (rx_buff)}, .size = (rx_size), .flags = 0}

/**
 * @brief USBD transfer feeder.
 *
 * Pointer for a transfer feeder.
 * Transfer feeder is a feedback function used to prepare a single
 * TX (Device->Host) endpoint transfer.
 *
 * The transfers provided by the feeder must be simple:
 * - The size of the transfer provided by this function is limited to a single endpoint buffer.
 *   Bigger transfers are not handled automatically in this case.
 * - Flash transfers are not automatically supported- you must copy them to the RAM buffer before.
 *
 * @note
 * This function may use @ref nrf_usbd_common_feeder_buffer_get to gain a temporary buffer
 * that can be used to prepare transfer.
 *
 * @param[out]    p_next    Structure with the data for the next transfer to be filled.
 *                          Required only if the function returns true.
 * @param[in,out] p_context Context variable configured with the transfer.
 * @param[in]     ep_size   The endpoint size.
 *
 * @retval false The current transfer is the last one - you do not need to call
 *               the function again.
 * @retval true  There is more data to be prepared and when the current transfer
 *               finishes, the feeder function is expected to be called again.
 */
typedef bool (*nrf_usbd_common_feeder_t)(nrf_usbd_common_ep_transfer_t *p_next, void *p_context,
				   size_t ep_size);

/**
 * @brief USBD transfer consumer.
 *
 * Pointer for a transfer consumer.
 * Transfer consumer is a feedback function used to prepare a single
 * RX (Host->Device) endpoint transfer.
 *
 * The transfer must provide a buffer big enough to fit the whole data from the endpoint.
 * Otherwise, the NRF_USBD_COMMON_EP_OVERLOAD event is generated.
 *
 * @param[out]    p_next    Structure with the data for the next transfer to be filled.
 *                          Required only if the function returns true.
 * @param[in,out] p_context Context variable configured with the transfer.
 * @param[in]     ep_size   The endpoint size.
 * @param[in]     data_size Number of received bytes in the endpoint buffer.
 *
 * @retval false Current transfer is the last one - you do not need to call
 *               the function again.
 * @retval true  There is more data to be prepared and when current transfer
 *               finishes, the feeder function is expected to be called again.
 */
typedef bool (*nrf_usbd_common_consumer_t)(nrf_usbd_common_ep_transfer_t *p_next, void *p_context,
				     size_t ep_size, size_t data_size);

/**
 * @brief Universal transfer handler.
 *
 * Union with feeder and consumer function pointer.
 */
typedef union {
	nrf_usbd_common_feeder_t feeder;     /*!< Feeder function pointer. */
	nrf_usbd_common_consumer_t consumer; /*!< Consumer function pointer. */
} nrf_usbd_common_handler_t;

/**
 * @brief USBD transfer descriptor.
 *
 * Universal structure that may hold the setup for callback configuration for
 * IN or OUT type of the transfer.
 */
typedef struct {
	/** Handler for the current transfer, function pointer. */
	nrf_usbd_common_handler_t handler;
	/** Context for the transfer handler. */
	void *p_context;
} nrf_usbd_common_handler_desc_t;

/**
 * @brief Setup packet structure.
 *
 * Structure that contains interpreted SETUP packet as described in USB specification.
 */
typedef struct {
	uint8_t bmRequestType; /*!< byte 0 */
	uint8_t bRequest;      /*!< byte 1 */
	uint16_t wValue;       /*!< byte 2, 3 */
	uint16_t wIndex;       /*!< byte 4, 5 */
	uint16_t wLength;      /*!< byte 6, 7 */
} nrf_usbd_common_setup_t;

/**
 * @brief Driver initialization.
 *
 * @param[in] event_handler Event handler provided by the user. Cannot be null.
 *
 * @retval NRFX_SUCCESS             Initialization successful.
 * @retval NRFX_ERROR_INVALID_STATE Driver was already initialized.
 */
nrfx_err_t nrf_usbd_common_init(nrf_usbd_common_event_handler_t event_handler);

/**
 * @brief Driver deinitialization.
 */
void nrf_usbd_common_uninit(void);

/**
 * @brief Enable the USBD port.
 *
 * After calling this function USBD peripheral would be enabled.
 * The USB LDO would be enabled.
 * Enabled USBD peripheral would request HFCLK.
 * This function does not enable external oscillator, so if it is not enabled by other part of the
 * program after enabling USBD driver HFINT would be used for the USBD peripheral.
 * It is perfectly fine until USBD is started. See @ref nrf_usbd_common_start.
 *
 * In normal situation this function should be called in reaction to USBDETECTED
 * event from POWER peripheral.
 *
 * Interrupts and USB pins pull-up would stay disabled until @ref nrf_usbd_common_start
 * function is called.
 */
void nrf_usbd_common_enable(void);

/**
 * @brief Disable the USBD port.
 *
 * After calling this function USBD peripheral would be disabled.
 * No events would be detected or processed by the driver.
 * Clock for the peripheral would be disconnected.
 */
void nrf_usbd_common_disable(void);

/**
 * @brief Start USB functionality.
 *
 * After calling this function USBD peripheral should be fully functional
 * and all new incoming events / interrupts would be processed by the driver.
 *
 * Also only after calling this function host sees new connected device.
 *
 * Call this function when USBD power LDO regulator is ready - on USBPWRRDY event
 * from POWER peripheral.
 *
 * Before USBD interrupts are enabled, external HFXO is requested.
 *
 * @param enable_sof The flag that is used to enable SOF processing.
 *                   If it is false, SOF interrupt is left disabled and will not be generated.
 *                   This improves power saving if SOF is not required.
 *
 * @note If the isochronous endpoints are going to be used,
 *       it is required to enable the SOF.
 *       In other case any isochronous endpoint would stay busy
 *       after first transmission.
 */
void nrf_usbd_common_start(bool enable_sof);

/**
 * @brief Check if driver is initialized.
 *
 * @retval false Driver is not initialized.
 * @retval true Driver is initialized.
 */
bool nrf_usbd_common_is_initialized(void);

/**
 * @brief Check if driver is enabled.
 *
 * @retval false Driver is disabled.
 * @retval true  Driver is enabled.
 */
bool nrf_usbd_common_is_enabled(void);

/**
 * @brief Check if driver is started.
 *
 * @retval false Driver is not started.
 * @retval true Driver is started (fully functional).
 * @note The USBD peripheral interrupt state is checked.
 */
bool nrf_usbd_common_is_started(void);

/**
 * @brief Suspend USBD operation.
 *
 * The USBD peripheral is forced to go into the low power mode.
 * The function has to be called in the reaction to @ref NRF_USBD_COMMON_EVT_SUSPEND event
 * when the firmware is ready.
 *
 * After successful call of this function most of the USBD registers would be unavailable.
 *
 * @note Check returned value for the feedback if suspending was successful.
 *
 * @retval true  USBD peripheral successfully suspended.
 * @retval false USBD peripheral was not suspended due to resume detection.
 */
bool nrf_usbd_common_suspend(void);

/**
 * @brief Start wake up procedure.
 *
 * The USBD peripheral is forced to quit the low power mode.
 * After calling this function all the USBD registers would be available.
 *
 * The hardware starts measuring time when wake up is possible.
 * This may take 0-5&nbsp;ms depending on how long the SUSPEND state was kept on the USB line.

 * When NRF_USBD_COMMON_EVT_WUREQ event is generated it means that Wake Up signaling has just been
 * started on the USB lines.
 *
 * @note Do not expect only @ref NRF_USBD_COMMON_EVT_WUREQ event.
 *       There always may appear @ref NRF_USBD_COMMON_EVT_RESUME event.
 * @note NRF_USBD_COMMON_EVT_WUREQ event means that Remote WakeUp signal
 *       has just begun to be generated.
 *       This may take up to 20&nbsp;ms for the bus to become active.
 *
 * @retval true WakeUp procedure started.
 * @retval false No WakeUp procedure started - bus is already active.
 */
bool nrf_usbd_common_wakeup_req(void);

/**
 * @brief Check if USBD is in SUSPEND mode.
 *
 * @note This is the information about peripheral itself, not about the bus state.
 *
 * @retval true  USBD peripheral is suspended.
 * @retval false USBD peripheral is active.
 */
bool nrf_usbd_common_suspend_check(void);

/**
 * @brief Check the bus state.
 *
 * This function checks if the bus state is suspended.
 *
 * @note The value returned by this function changes on SUSPEND and RESUME event processing.
 *
 * @retval true  USBD bus is suspended.
 * @retval false USBD bus is active.
 */
bool nrf_usbd_common_bus_suspend_check(void);

/**
 * @brief Force the bus state to active
 */
void nrf_usbd_common_force_bus_wakeup(void);

/**
 * @brief Configure packet size that should be supported by the endpoint.
 *
 * The real endpoint buffer size is always the same.
 * This value sets max packet size that would be transmitted over the endpoint.
 * This is required by the driver.
 *
 * @param[in] ep   Endpoint number.
 * @param[in] size Required maximum packet size.
 *
 * @note Endpoint size is always set to @ref NRF_USBD_COMMON_EPSIZE
 *       or @ref NRF_USBD_COMMON_ISOSIZE / 2
 *       when @ref nrf_usbd_common_ep_enable function is called.
 */
void nrf_usbd_common_ep_max_packet_size_set(nrf_usbd_common_ep_t ep, uint16_t size);

/**
 * @brief Get configured endpoint packet size.
 *
 * Function to get configured endpoint size on the buffer.
 *
 * @param[in] ep Endpoint number.
 *
 * @return Maximum pocket size configured on selected endpoint.
 */
uint16_t nrf_usbd_common_ep_max_packet_size_get(nrf_usbd_common_ep_t ep);

/**
 * @brief Check if the selected endpoint is enabled.
 *
 * @param[in] ep Endpoint number to check.
 *
 * @retval true  Endpoint is enabled.
 * @retval false Endpoint is disabled.
 */
bool nrf_usbd_common_ep_enable_check(nrf_usbd_common_ep_t ep);

/**
 * @brief Enable selected endpoint.
 *
 * This function enables endpoint itself and its interrupts.
 *
 * @param[in] ep Endpoint number to enable.
 *
 * @note
 * Max packet size is set to endpoint default maximum value.
 *
 * @sa nrf_usbd_common_ep_max_packet_size_set
 */
void nrf_usbd_common_ep_enable(nrf_usbd_common_ep_t ep);

/**
 * @brief Disable selected endpoint.
 *
 * This function disables endpoint itself and its interrupts.
 *
 * @param[in] ep Endpoint number to disable.
 */
void nrf_usbd_common_ep_disable(nrf_usbd_common_ep_t ep);

/**
 * @brief Disable all endpoints except for EP0.
 *
 * Disable all endpoints that can be disabled in USB device while it is still active.
 */
void nrf_usbd_common_ep_default_config(void);

/**
 * @brief Start sending data over endpoint.
 *
 * Function initializes endpoint transmission.
 * This is asynchronous function - it finishes immediately after configuration
 * for transmission is prepared.
 *
 * @note Data buffer pointed by p_data have to be kept active till
 *       @ref NRF_USBD_COMMON_EVT_EPTRANSFER event is generated.
 *
 * @param[in] ep         Endpoint number.
 *                       For IN endpoint sending would be initiated.
 *                       For OUT endpoint receiving would be initiated.
 * @param[in] p_transfer Transfer parameters.
 *
 * @retval NRFX_SUCCESS             Transfer queued or started.
 * @retval NRFX_ERROR_BUSY          Selected endpoint is pending.
 * @retval NRFX_ERROR_INVALID_ADDR  Unexpected transfer on EPIN0 or EPOUT0.
 */
nrfx_err_t nrf_usbd_common_ep_transfer(nrf_usbd_common_ep_t ep,
				       nrf_usbd_common_transfer_t const *p_transfer);

/**
 * @brief Start sending data over the endpoint using the transfer handler function.
 *
 * This function initializes an endpoint transmission.
 * Just before data is transmitted, the transfer handler
 * is called and it prepares a data chunk.
 *
 * @param[in] ep        Endpoint number.
 *                      For an IN endpoint, sending is initiated.
 *                      For an OUT endpoint, receiving is initiated.
 * @param[in] p_handler Transfer handler - feeder for IN direction and consumer for
 *                      OUT direction.
 *
 * @retval NRFX_SUCCESS             Transfer queued or started.
 * @retval NRFX_ERROR_BUSY          Selected endpoint is pending.
 * @retval NRFX_ERROR_INVALID_ADDR  Unexpected transfer on EPIN0 or EPOUT0.
 */
nrfx_err_t nrf_usbd_common_ep_handled_transfer(nrf_usbd_common_ep_t ep,
					       nrf_usbd_common_handler_desc_t const *p_handler);

/**
 * @brief Get the temporary buffer to be used by the feeder.
 *
 * This buffer is used for TX transfers and it can be reused automatically
 * when the transfer is finished.
 * Use it for transfer preparation.
 *
 * May be used inside the feeder configured in @ref nrf_usbd_common_ep_handled_transfer.
 *
 * @return Pointer to the buffer that can be used temporarily.
 *
 * @sa NRF_USBD_COMMON_FEEDER_BUFFER_SIZE
 */
void *nrf_usbd_common_feeder_buffer_get(void);

/**
 * @brief Get the information about last finished or current transfer.
 *
 * Function returns the status of the last buffer set for transfer on selected endpoint.
 * The status considers last buffer set by @ref nrf_usbd_common_ep_transfer function or
 * by transfer callback function.
 *
 * @param[in]  ep     Endpoint number.
 * @param[out] p_size Information about the current/last transfer size.
 *
 * @return Endpoint status.
 *
 * @sa nrf_usbd_common_ep_status_t
 */
nrf_usbd_common_ep_status_t nrf_usbd_common_ep_status_get(nrf_usbd_common_ep_t ep, size_t *p_size);

/**
 * @brief Get number of received bytes.
 *
 * Get the number of received bytes.
 * The function behavior is undefined when called on IN endpoint.
 *
 * @param[in] ep Endpoint number.
 *
 * @return Number of received bytes.
 */
size_t nrf_usbd_common_epout_size_get(nrf_usbd_common_ep_t ep);

/**
 * @brief Check if endpoint buffer is ready or is under USB IP control.
 *
 * Function to test if endpoint is busy.
 * Endpoint that is busy cannot be accessed by MCU.
 * It means that:
 * - OUT (TX) endpoint: Last uploaded data is still in endpoint and is waiting
 *                      to be received by the host.
 * - IN  (RX) endpoint: Endpoint is ready to receive data from the host
 *                      and the endpoint does not have any data.
 * When endpoint is not busy:
 * - OUT (TX) endpoint: New data can be uploaded.
 * - IN  (RX) endpoint: New data can be downloaded using @ref nrf_usbd_common_ep_transfer
 *                      function.
 *
 * @param[in] ep Endpoint number.
 *
 * @retval false Endpoint is not busy.
 * @retval true  Endpoint is busy.
 */
bool nrf_usbd_common_ep_is_busy(nrf_usbd_common_ep_t ep);

/**
 * @brief Stall endpoint
 *
 * Stall endpoit to send error information during next transfer request from
 * the host.
 *
 * @note To stall endpoint it is safer to use @ref nrf_usbd_common_setup_stall
 * @note Stalled endpoint would not be cleared when DMA transfer finishes.
 *
 * @param[in] ep Endpoint number to stall.
 */
void nrf_usbd_common_ep_stall(nrf_usbd_common_ep_t ep);

/**
 * @brief Clear stall flag on endpoint.
 *
 * This function clears endpoint that is stalled.
 * @note
 * If it is OUT endpoint (receiving) it would be also prepared for reception.
 * It means that busy flag would be set.
 * @note
 * In endpoint (transmitting) would not be cleared - it gives possibility to
 * write new data before transmitting.
 *
 * @param[in] ep Endpoint number.
 */
void nrf_usbd_common_ep_stall_clear(nrf_usbd_common_ep_t ep);

/**
 * @brief Check if endpoint is stalled.
 *
 * This function gets stall state of selected endpoint.
 *
 * @param[in] ep Endpoint number to check.
 *
 * @retval false Endpoint is not stalled.
 * @retval true  Endpoint is stalled.
 */
bool nrf_usbd_common_ep_stall_check(nrf_usbd_common_ep_t ep);

/**
 * @brief Clear current endpoint data toggle.
 *
 * @param[in] ep Endpoint number to clear.
 */
void nrf_usbd_common_ep_dtoggle_clear(nrf_usbd_common_ep_t ep);

/**
 * @brief Get parsed setup data.
 *
 * Function fills the parsed setup data structure.
 *
 * @param[out] p_setup Pointer to data structure that would be filled by
 *                     parsed data.
 */
void nrf_usbd_common_setup_get(nrf_usbd_common_setup_t *p_setup);

/**
 * @brief Clear the control endpoint for packet reception during DATA stage.
 *
 * This function may be called if any more data in control write transfer is expected.
 * Clears only OUT endpoint to be able to take another OUT data token.
 * It does not allow STATUS stage.
 * @sa nrf_usbd_common_setup_clear
 */
void nrf_usbd_common_setup_data_clear(void);

/**
 * @brief Clear setup endpoint.
 *
 * This function acknowledges setup when SETUP command was received and processed.
 * It has to be called if no data respond for the SETUP command is sent.
 */
void nrf_usbd_common_setup_clear(void);

/**
 * @brief Stall setup endpoint.
 *
 * Mark an error on setup endpoint.
 */
void nrf_usbd_common_setup_stall(void);

/**
 * @brief Abort pending transfer on selected endpoint.
 *
 * @param[in] ep Endpoint number.
 */
void nrf_usbd_common_ep_abort(nrf_usbd_common_ep_t ep);

/**
 * @brief Get the information about expected transfer SETUP data direction.
 *
 * Function returns the information about last expected transfer direction.
 *
 * @retval NRF_USBD_COMMON_EPOUT0 Expecting OUT (Host->Device) direction or no data.
 * @retval NRF_USBD_COMMON_EPIN0  Expecting IN (Device->Host) direction.
 */
nrf_usbd_common_ep_t nrf_usbd_common_last_setup_dir_get(void);

/**
 * @brief Drop transfer on OUT endpoint.
 *
 * @param[in] ep  OUT endpoint ID.
 */
void nrf_usbd_common_transfer_out_drop(nrf_usbd_common_ep_t ep);

/** @} */

void nrf_usbd_common_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* NRF_USBD_COMMON_H__ */

/** @endcond */
