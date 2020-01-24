/* SPDX-License-Identifier: Apache-2.0 */

/* SoC level DTS fixup file */

#define DT_NUM_IRQ_PRIO_BITS	DT_ARM_V6M_NVIC_E000E100_ARM_NUM_IRQ_PRIORITY_BITS

#define DT_ADC_0_NAME			DT_NORDIC_NRF_ADC_ADC_0_LABEL

#define DT_UART_0_NAME			DT_NORDIC_NRF_UART_UART_0_LABEL

#define DT_FLASH_DEV_NAME		DT_INST_0_NORDIC_NRF51_FLASH_CONTROLLER_LABEL

#define DT_GPIO_P0_DEV_NAME		DT_NORDIC_NRF_GPIO_GPIO_0_LABEL

#define DT_I2C_0_NAME			DT_NORDIC_NRF_TWI_I2C_0_LABEL
#define DT_I2C_1_NAME			DT_NORDIC_NRF_TWI_I2C_1_LABEL

#define DT_SPI_0_NAME			DT_NORDIC_NRF_SPI_SPI_0_LABEL
#define DT_SPI_1_NAME			DT_NORDIC_NRF_SPI_SPI_1_LABEL

#define DT_WDT_0_NAME			DT_NORDIC_NRF_WATCHDOG_WDT_0_LABEL

/* End of SoC Level DTS fixup file */
