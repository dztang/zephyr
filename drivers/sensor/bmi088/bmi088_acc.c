/* Bosch BMI088 inertial measurement unit driver
 * Note: This is for the Accelerometer part only
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Datasheet:
 * https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi088-ds001.pdf
 */

#define DT_DRV_COMPAT bosch_bmi088

#include "bmi088_acc.h"

#include <init.h>
#include <drivers/sensor.h>
#include <sys/byteorder.h>
#include <kernel.h>
#include <sys/__assert.h>
#include <devicetree.h>
#include <logging/log.h>
#include <math.h>

#include "../lib/libc/minimal/include/math.h"

LOG_MODULE_REGISTER(BMI088, CONFIG_SENSOR_LOG_LEVEL);

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#error "BMI088 driver enabled without any devices"
#endif

/**
 * Send/receive data to/from the BMI088
 *
 * @param dev Sensor device pointer
 * @param reg (Start) register address, for writing the MSb should be 0, for reading 1
 * @param write True for write-only, False for read-only
 * @param buf Out-parameter if reading, In-parameter if writing
 * @param length Length of buf
 * @return 0 on success
 */
 // TODO: Dummy byte in SPI read operations on acc (resolved?)
static int bmi088_acc_transceive(const struct device *dev, uint8_t reg, bool write, void *buf, size_t length) {
    const struct bmi088_acc_cfg *cfg = to_config(dev);
    const struct spi_buf tx_buf[3] = {
            {
                    // Always send dummy byte
                    .buf = NULL,
                    .len = 1
            },
            {
                    // Always send register address first
                    .buf = &reg,
                    .len = 1
            },
            {
                    // Then send whatever data there is to send
                    .buf = buf,
                    .len = length
            }
    };
    const struct spi_buf_set tx = {
            .buffers = tx_buf,
            // If the buffer is a null pointer, only use the first TX (register address)
            .count = buf ? 3 : 2        // If *buf points to null, only send dummy byte and register address
    };


    if (write) {
        // Write-only: No RX buffer necessary
        return spi_write_dt(&cfg->bus, &tx);
    } else {
        // If we want to read, use the same buffers for that purpose.
        // No useful data will be written to reg, as we dont receive anything while we still send the address
        // Whatever data is in the buffer will be sent out. The sensor will ignore that data if the address was a
        // read-request
        const struct spi_buf_set rx = {
                .buffers = tx_buf,
                .count = 3
        };

        return spi_transceive_dt(&cfg->bus, &tx, &rx);
    }
}

bool bmi088_acc_bus_ready_spi(const struct device *dev) {
    return spi_is_ready(&to_config(dev)->bus);
}

int bmi088_acc_read(const struct device *dev, uint8_t reg_addr, void *buf, uint8_t len) {
    return bmi088_acc_transceive(dev, reg_addr | BMI088_ACC_REG_READ, false, buf, len);
}

int bmi088_acc_byte_read(const struct device *dev, uint8_t reg_addr, uint8_t *byte) {
    return bmi088_acc_read(dev, reg_addr, byte, 1);
}

int bmi088_acc_write(const struct device *dev, uint8_t reg_addr, void *buf, uint8_t len) {

    return bmi088_acc_transceive(dev, reg_addr & BMI088_ACC_REG_MASK, true, buf, len);
}

int bmi088_acc_byte_write(const struct device *dev, uint8_t reg_addr,
                      uint8_t byte) {
    return bmi088_acc_write(dev, reg_addr & BMI088_ACC_REG_MASK, &byte, 1);
}

// TODO: Check the warning
int bmi088_acc_reg_field_update(const struct device *dev, uint8_t reg_addr,
                            uint8_t pos, uint8_t mask, uint8_t val) {

    uint8_t old_val;

    if (bmi088_acc_byte_read(dev, reg_addr, &old_val) < 0) {
        return -EIO;
    }

    return bmi088_acc_byte_write(dev, reg_addr,
                             (old_val & ~mask) | ((val << pos) & mask));
}

/**
 * Convert the raw value with a factor 'scale' and save the new values integer and fractional part in 'sensor_value'
 *
 * @param raw_val Raw sensor value
 * @param scale Value to scale the raw_val
 */
 // TODO: Evtl anpassen für scale (resolved?)
struct sensor_value bmi088_acc_to_fixed_point(int16_t raw_val, uint16_t scale) {
    int32_t converted_val = raw_val / scale;        // see bmi088_channel_get

    LOG_INF("Conversion: input %d scale %d converted %ld", raw_val, scale, converted_val);
    struct sensor_value val = {
            .val1=converted_val / 1000000,
            .val2=converted_val % 1000000};
    return val;
}

/**
 * Convert the corresponding value of the channel (X, Y or Z)
 *
 * @param chan Channel to read. Implemented: Acc X, Y, Z
 * @param scale Value to scale the raw_val
 * @param raw_xyz array for storing X, Y, Z channel raw value of the sensor
 * @return
 */
struct sensor_value bmi088_acc_channel_convert(enum sensor_channel chan, uint16_t scale, int16_t raw_xyz[3]) {
    switch (chan) {
        case SENSOR_CHAN_ACCEL_X:
            return bmi088_acc_to_fixed_point(raw_xyz[0], scale);
        case SENSOR_CHAN_ACCEL_Y:
            return bmi088_acc_to_fixed_point(raw_xyz[1], scale);
        case SENSOR_CHAN_ACCEL_Z:
            return bmi088_acc_to_fixed_point(raw_xyz[2], scale);
        default:
            LOG_ERR("Channels not supported !");
            struct sensor_value empty = {0};
            return empty;
    }
}

static int bmi088_acc_attr_set(const struct device *dev, enum sensor_channel chan, enum sensor_attribute attr,
                           const struct sensor_value *val) {
    return -ENOTSUP;
}

/**
 * API function to retrieve a measurement from the sensor.
 *
 * @param dev Sensor device pointer
 * @param chan Channel to fetch. Only SENSOR_CHAN_ALL and ACCEL_XYZ is supported.
 * @return 0 on success
 */
// TODO: check (5.3.3)
static int bmi088_acc_sample_fetch(const struct device *dev, enum sensor_channel chan) {
    struct bmi088_acc_data *data = to_data(dev);

    __ASSERT(chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_ACCEL_XYZ, "channel is not valid");

    uint8_t status = 0;
    while ((status & BMI088_ACC_STATUS_MASK) == 0) {

        if (bmi088_acc_byte_read(dev, BMI088_ACC_STATUS, &status) < 0) {
            return -EIO;
        }
    }

    if (bmi088_acc_read(dev, RATE_X_LSB, data->sample.acc, BMI088_SAMPLE_SIZE) < 0) {
        return -EIO;
    }

    // convert samples to cpu endianness
    for (size_t i = 0; i < BMI088_AXES; i++) {
        uint16_t *sample = &data->sample.acc[i];

        *sample = sys_le16_to_cpu(*sample);
    }

    LOG_INF("Fetched %d %d %d", data->sample.acc[0], data->sample.acc[1], data->sample.acc[2]);

    return 0;
}

/**
 * API function to get a cached sensor value that was previously fetched from the sensor
 *
 * @param dev Sensor device pointer
 * @param chan Channel to read. Implemented: Acc X, Y, Z
 * @param [out] val Sensor value
 * @return 0 on success, -ENOTSUP on unsupported channel
 */
static int bmi088_acc_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val) {

    uint8_t range_value = 0;
    // reading ACC_RANGE for calculation of the scale below
    if (bmi088_acc_byte_read(dev, ACC_RANGE, &range_value) < 1) {
        return -EIO;
    }
    // scale for converting sensor output to mg (Datasheet 5.3.4)
    const uint16_t scale = (32768 * 1000 * pow(2,(range_value + 1)) * 1.5);


    switch (chan) {
        case SENSOR_CHAN_ACCEL_X:
        case SENSOR_CHAN_ACCEL_Y:
        case SENSOR_CHAN_ACCEL_Z: {
            struct bmi088_acc_data *data = to_data(dev);
            *val = bmi088_acc_channel_convert(chan, scale, data->sample.acc);
            return 0;
        }
        case SENSOR_CHAN_ACCEL_XYZ: {
            struct bmi088_acc_data *data = to_data(dev);
            val[0] = bmi088_acc_channel_convert(SENSOR_CHAN_ACCEL_X, scale, data->sample.acc);
            val[1] = bmi088_acc_channel_convert(SENSOR_CHAN_ACCEL_Y, scale, data->sample.acc);
            val[2] = bmi088_acc_channel_convert(SENSOR_CHAN_ACCEL_Z, scale, data->sample.acc);
            return 0;
        }
        default:
            LOG_ERR("Channel not supported.");
            return -ENOTSUP;
    }
}

/**
 * Sensor device initialization
 *
 * @param dev Sensor device pointer
 * @return 0 on success
 */
 // TODO: Acc initialization (resolved?)
static int bmi088_acc_init(const struct device *dev) {
    LOG_DBG("Initializing BMI088 ACC device at %p", dev);

    if (!bmi088_acc_bus_ready_spi(dev)) {
        LOG_ERR("Bus not ready");
        return -EINVAL;
    }

    // reboot the chip: Softreset 0x7E
    if (bmi088_acc_byte_write(dev, BMI088_ACC_SOFTRESET, BMI088_ACC_SR_VAL) < 0) {
        LOG_DBG("Cannot reboot chip.");
        return -EIO;
    }

    k_busy_wait(1000);

    uint8_t val = 0U;
    // Dummy read to enable SPI
    if (bmi088_acc_byte_read(dev, BMI088_REG_CHIPID, &val) < 0) {
        LOG_DBG("Failed to read chip id.");
        return -EIO;
    }
    LOG_DBG("Acc in SPI mode");

    // Read chip ID value
    if (bmi088_acc_byte_read(dev, BMI088_REG_CHIPID, &val) < 0) {
        LOG_DBG("Failed to read chip id.");
        return -EIO;
    }
    // check if the chip ID is correct
    if (val != BMI088_ACC_CHIP_ID) {
        LOG_DBG("Unsupported chip detected (0x%x)!", val);
        return -ENODEV;
    }
    LOG_DBG("Chip successfully detected");

    // Set default acc range, For now: always use the largest range
    if (bmi088_acc_reg_field_update(dev, ACC_RANGE,0, BIT(0) | BIT(1) ,BMI088_ACC_DEFAULT_RANGE) < 0) {
        LOG_DBG("Cannot set default range for accelerometer.");
        return -EIO;
    }

    // Switch acc to normal mode
    if (bmi088_acc_byte_write(dev, ACC_PWR_CTRL, ACC_NORMAL_MODE) < 0) {
        LOG_DBG("Cannot switch power mode to normal");
        return -EIO;
    }

     k_busy_wait(50000);     // wait 50ms for acc to switch power mode

    return 0;
}

static const struct sensor_driver_api bmi088_acc_api = {
        .attr_set = bmi088_acc_attr_set,
        .sample_fetch = bmi088_acc_sample_fetch,
        .channel_get = bmi088_acc_channel_get,
};


#define BMI088_DEVICE_INIT(inst) \
    static struct bmi088_acc_data bmi088_acc_data_##inst;               \
    static const struct bmi088_acc_cfg bmi088_acc_cfg_##inst = {           \
        .bus = SPI_DT_SPEC_INST_GET(inst, SPI_WORD_SET(8), 0), \
    };                                   \
    DEVICE_DT_INST_DEFINE(inst, bmi088_acc_init, NULL,            \
                  &bmi088_acc_data_##inst, &bmi088_acc_cfg_##inst,    \
                  POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,    \
                  &bmi088_acc_api);


DT_INST_FOREACH_STATUS_OKAY(BMI088_DEVICE_INIT)
