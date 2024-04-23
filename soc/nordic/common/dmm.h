/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * nRF SoC specific public APIs for Device Memory Management (dmm) subsystem
 */

#ifndef SOC_NORDIC_COMMON_DMM_H_
#define SOC_NORDIC_COMMON_DMM_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @cond INTERNAL_HIDDEN */

/**
 * @brief Prepare a DMA output buffer for the specified device
 *
 * Allocate an output buffer in memory region that given device can perform DMA transfers from.
 * Copy @p user_buffer contents into it.
 * Writeback data cache lines associated with output buffer, if needed.
 *
 * @note Depending on provided user buffer parameters and SoC architecture,
 *       dynamic allocation and cache operations might be skipped.
 *
 * @warning It is prohibited to read or write @p user_buffer or @p buffer_out contents
 *          from the time this function is called until @ref dmm_dma_buffer_out_release()
 *          is called on the same buffer or until this function returns with an error.
 *
 * @param dev Device to prepare the buffer for.
 * @param user_buffer CPU address (virtual if applicable) of the buffer containing data
 *                    to be processed by the given device.
 * @param user_length Length of the buffer containing data to be processed by the given device.
 * @param buffer_out Pointer to a bus address of a buffer containing the prepared DMA buffer.
 *
 * @retval 0 If succeeded.
 * @retval -ENOMEM If output buffer could not be allocated.
 * @retval -errno Negative errno for other failures.
 */
int dmm_dma_buffer_out_prepare(const struct device *dev, void const * user_buffer, size_t user_length, void ** buffer_out);

/**
 * @brief Release the previously prepared DMA output buffer
 *
 * @param dev Device to release the buffer for.
 * @param buffer_out Bus address of the DMA output buffer previously prepared with @ref dmm_dma_buffer_out_prepare().
 *
 * @retval 0 If succeeded.
 * @retval -errno Negative errno code on failure.
 */
int dmm_dma_buffer_out_release(const struct device *dev, void const * buffer_out);

/**
 * @brief Prepare a DMA input buffer for the specified device
 *
 * Allocate an input buffer in memory region that given device can perform DMA transfers to.
 *
 * @note Depending on provided user buffer parameters and SoC architecture,
 *       dynamic allocation might be skipped.
 *
 * @warning It is prohibited to read or write @p user_buffer or @p buffer_in contents
 *          from the time this function is called until @ref dmm_dma_buffer_in_release()
 *          is called on the same buffer or until this function returns with an error.
 *
 * @param dev Device to prepare the buffer for.
 * @param user_buffer CPU address (virtual if applicable) of the buffer to be filled with data
 *                    from the given device.
 * @param user_length Length of the buffer to be filled with data from the given device..
 * @param buffer_in Pointer to a bus address of a buffer containing the prepared DMA buffer.
 *
 * @retval 0 If succeeded.
 * @retval -ENOMEM If input buffer could not be allocated.
 * @retval -errno Negative errno for other failures.
 */
int dmm_dma_buffer_in_prepare(const struct device *dev, void const * user_buffer, size_t user_length, void ** buffer_in);

/**
 * @brief Release the previously prepared DMA input buffer
 *
 * Invalidate data cache lines associated with input buffer, if needed.
 * Copy @p buffer_in contents into @p user_buffer, if needed.
 *
 * @param dev Device to release the buffer for.
 * @param user_buffer CPU address (virtual if applicable) of the buffer to be filled with data
 *                    from the given device.
 * @param user_length Length of the buffer to be filled with data from the given device..
 * @param buffer_in Bus address of the DMA input buffer previously prepared with @ref dmm_dma_buffer_in_prepare().
 *
 * @note @p user_buffer and @p buffer_in arguments pair provided in this function call must match
 *       the arguments pair provided in prior call to @ref dmm_dma_buffer_in_prepare().
 *
 * @retval 0 If succeeded.
 * @retval -errno Negative errno code on failure.
 */
int dmm_dma_buffer_in_release(const struct device *dev, void const * user_buffer, size_t user_length, void const * buffer_in);

/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* SOC_NORDIC_COMMON_DMM_H_ */
