/*
 * Copyright (c) 2018, Diego Sueiro <diego.sueiro@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include "device_imx.h"

static int colibri_imx7d_m4_pinmux_init(struct device *dev)
{
	ARG_UNUSED(dev);

#if DT_HAS_NODELABEL_STATUS_OKAY(gpio1)
	/* GPIO1_IO02 Mux Config */
	IOMUXC_LPSR_SW_MUX_CTL_PAD_GPIO1_IO02 = 0;
	IOMUXC_LPSR_SW_PAD_CTL_PAD_GPIO1_IO02 = 0;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(gpio2)
	/* GPIO2_IO26 Mux Config */
	IOMUXC_SW_MUX_CTL_PAD_EPDC_GDRL = 5;
	IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL =
		IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_PS(2) |
		IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_PE_MASK |
		IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(uart2)
	IOMUXC_SW_MUX_CTL_PAD_UART2_RX_DATA =
		IOMUXC_SW_MUX_CTL_PAD_UART2_RX_DATA_MUX_MODE(0);
	IOMUXC_SW_MUX_CTL_PAD_UART2_TX_DATA =
		IOMUXC_SW_MUX_CTL_PAD_UART2_TX_DATA_MUX_MODE(0);
	IOMUXC_SW_PAD_CTL_PAD_UART2_RX_DATA =
		IOMUXC_SW_PAD_CTL_PAD_UART2_RX_DATA_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_UART2_RX_DATA_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_UART2_RX_DATA_HYS_MASK |
		IOMUXC_SW_PAD_CTL_PAD_UART2_RX_DATA_DSE(0);

	IOMUXC_SW_PAD_CTL_PAD_UART2_TX_DATA =
		IOMUXC_SW_PAD_CTL_PAD_UART2_TX_DATA_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_UART2_TX_DATA_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_UART2_RX_DATA_HYS_MASK |
		IOMUXC_SW_PAD_CTL_PAD_UART2_TX_DATA_DSE(0);

	/* Select TX_PAD for RX data (DTE mode...) */
	IOMUXC_UART2_RX_DATA_SELECT_INPUT =
		IOMUXC_UART2_RX_DATA_SELECT_INPUT_DAISY(3);
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(i2c1)
	IOMUXC_SW_MUX_CTL_PAD_I2C1_SCL =
		IOMUXC_SW_MUX_CTL_PAD_I2C1_SCL_MUX_MODE(0) |
		IOMUXC_SW_MUX_CTL_PAD_I2C1_SCL_SION_MASK;
	IOMUXC_SW_MUX_CTL_PAD_I2C1_SDA =
		IOMUXC_SW_MUX_CTL_PAD_I2C1_SDA_MUX_MODE(0) |
		IOMUXC_SW_MUX_CTL_PAD_I2C1_SDA_SION_MASK;

	IOMUXC_I2C1_SCL_SELECT_INPUT = IOMUXC_I2C1_SCL_SELECT_INPUT_DAISY(1);
	IOMUXC_I2C1_SDA_SELECT_INPUT = IOMUXC_I2C1_SDA_SELECT_INPUT_DAISY(1);

	IOMUXC_SW_PAD_CTL_PAD_I2C1_SCL =
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SCL_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SCL_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SCL_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SCL_HYS_MASK;

	IOMUXC_SW_PAD_CTL_PAD_I2C1_SDA =
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SDA_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SDA_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SDA_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_I2C1_SDA_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(i2c2)
	IOMUXC_SW_MUX_CTL_PAD_I2C2_SCL =
		IOMUXC_SW_MUX_CTL_PAD_I2C2_SCL_MUX_MODE(0) |
		IOMUXC_SW_MUX_CTL_PAD_I2C2_SCL_SION_MASK;
	IOMUXC_SW_MUX_CTL_PAD_I2C2_SDA =
		IOMUXC_SW_MUX_CTL_PAD_I2C2_SDA_MUX_MODE(0) |
		IOMUXC_SW_MUX_CTL_PAD_I2C2_SDA_SION_MASK;

	IOMUXC_I2C2_SCL_SELECT_INPUT = IOMUXC_I2C2_SCL_SELECT_INPUT_DAISY(1);
	IOMUXC_I2C2_SDA_SELECT_INPUT = IOMUXC_I2C2_SDA_SELECT_INPUT_DAISY(1);

	IOMUXC_SW_PAD_CTL_PAD_I2C2_SCL =
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SCL_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SCL_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SCL_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SCL_HYS_MASK;

	IOMUXC_SW_PAD_CTL_PAD_I2C2_SDA =
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SDA_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SDA_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SDA_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_I2C2_SDA_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(i2c3)
	IOMUXC_SW_MUX_CTL_PAD_I2C3_SCL =
		IOMUXC_SW_MUX_CTL_PAD_I2C3_SCL_MUX_MODE(0) |
		IOMUXC_SW_MUX_CTL_PAD_I2C3_SCL_SION_MASK;
	IOMUXC_SW_MUX_CTL_PAD_I2C3_SDA =
		IOMUXC_SW_MUX_CTL_PAD_I2C3_SDA_MUX_MODE(0) |
		IOMUXC_SW_MUX_CTL_PAD_I2C3_SDA_SION_MASK;

	IOMUXC_I2C3_SCL_SELECT_INPUT = IOMUXC_I2C3_SCL_SELECT_INPUT_DAISY(2);
	IOMUXC_I2C3_SDA_SELECT_INPUT = IOMUXC_I2C3_SDA_SELECT_INPUT_DAISY(2);

	IOMUXC_SW_PAD_CTL_PAD_I2C3_SCL =
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SCL_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SCL_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SCL_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SCL_HYS_MASK;

	IOMUXC_SW_PAD_CTL_PAD_I2C3_SDA =
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SDA_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SDA_PS(3)    |
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SDA_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_I2C3_SDA_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(i2c4)
	IOMUXC_SW_MUX_CTL_PAD_ENET1_RGMII_TD2 =
		IOMUXC_SW_MUX_CTL_PAD_ENET1_RGMII_TD2_MUX_MODE(3) |
		IOMUXC_SW_MUX_CTL_PAD_ENET1_RGMII_TD2_SION_MASK;
	IOMUXC_SW_MUX_CTL_PAD_ENET1_RGMII_TD3 =
		IOMUXC_SW_MUX_CTL_PAD_ENET1_RGMII_TD3_MUX_MODE(3) |
		IOMUXC_SW_MUX_CTL_PAD_ENET1_RGMII_TD3_SION_MASK;

	IOMUXC_I2C4_SCL_SELECT_INPUT = IOMUXC_I2C4_SCL_SELECT_INPUT_DAISY(4);
	IOMUXC_I2C4_SDA_SELECT_INPUT = IOMUXC_I2C4_SDA_SELECT_INPUT_DAISY(4);

	IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD2 =
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD2_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD2_PS(1)    |
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD2_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD2_HYS_MASK;

	IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD3 =
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD3_PE_MASK  |
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD3_PS(1)    |
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD3_DSE(0)   |
		IOMUXC_SW_PAD_CTL_PAD_ENET1_RGMII_TD3_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(pwm1)
	IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO08 =
		IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO08_MUX_MODE(7);
	IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO08 =
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO08_PE_MASK |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO08_PS(3)   |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO08_DSE(0)  |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO08_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(pwm2)
	IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO09 =
		IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO09_MUX_MODE(7);
	IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO09 =
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO09_PE_MASK |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO09_PS(3)   |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO09_DSE(0)  |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO09_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(pwm3)
	IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO10 =
		IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO10_MUX_MODE(7);
	IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO10 =
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO10_PE_MASK |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO10_PS(3)   |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO10_DSE(0)  |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO10_HYS_MASK;
#endif

#if DT_HAS_NODELABEL_STATUS_OKAY(pwm4)
	IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO11 =
		IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO11_MUX_MODE(7);
	IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO11 =
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO11_PE_MASK |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO11_PS(3)   |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO11_DSE(0)  |
		IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO11_HYS_MASK;
#endif

	return 0;

}

SYS_INIT(colibri_imx7d_m4_pinmux_init, PRE_KERNEL_1, 0);
