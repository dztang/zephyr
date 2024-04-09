/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

int dmm_dma_buffer_out_prepare(const struct device *dev, void const * user_buffer, size_t user_length, void ** buffer_out);
{
    // Get memory region that specified device can perform DMA transfers from, using devicetree

    // Check if:
    // - provided user buffer is already in correct memory region,
    // - provider user buffer is aligned and padded to cache line, if it is located in cacheable region.
    // If yes, assign buffer_out to user_buffer
    // If no:
    // - dynamically allocate buffer in correct memory region that respects cache line alignment and padding,
    //   using for example mem_attr_heap_alloc()
    // - copy user buffer contents into allocated buffer

    // Return error if dynamic allocation fails

    // Check if device memory region is cacheable
    // If yes, writeback all cache lines associated with output buffer (either user or allocated)
    // If no, no action is needed
}

int dmm_dma_buffer_out_release(void const * buffer_out)
{
    // Get memory region that specified device can perform DMA transfers from, using devicetree

    // Check if output buffer is contained within memory area managed by dynamic memory allocator
    // If yes, free the buffer
    // If no, no action is needed
}

int dmm_dma_buffer_in_prepare(const struct device *dev, void const * user_buffer, size_t user_length, void ** buffer_in);
{
    // Get memory region that specified device can perform DMA transfers to, using devicetree

    // Check if:
    // - provided user buffer is already in correct memory region,
    // - provider user buffer is aligned and padded to cache line, if it is located in cacheable region.
    // If yes, assign buffer_in to user_buffer
    // If no, dynamically allocate buffer in correct memory region that respects cache line alignment and padding,
    // using for example mem_attr_heap_alloc()

    // Return error if dynamic allocation fails
}

int dmm_dma_buffer_in_release(const struct device *dev, void const * user_buffer, size_t user_length, void const * buffer_in);
{
    // Get memory region that specified device can perform DMA transfers to, using devicetree

    // Check if device memory region is cacheable
    // If yes, invalidate all cache lines associated with input buffer (either user or allocated)
    // If no, no action is needed

    // Check if user buffer and allocated buffer points to the same memory location
    // If yes, no action is needed
    // If no, copy allocated buffer to the user buffer

    // Check if input buffer is contained within memory area managed by dynamic memory allocator
    // If yes, free the buffer
    // If no, no action is needed
}
