/*
 * Copyright (c) 2019 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* SoC level DTS fixup file */

#define DT_NUM_IRQ_PRIO_BITS			DT_ARM_V6M_NVIC_E000E100_ARM_NUM_IRQ_PRIORITY_BITS

#define DT_FLASH_DEV_BASE_ADDRESS		DT_ST_STM32G0_FLASH_CONTROLLER_40022000_BASE_ADDRESS
#define DT_FLASH_DEV_NAME			DT_ST_STM32G0_FLASH_CONTROLLER_40022000_LABEL

#define DT_PWM_STM32_3_DEV_NAME			DT_ST_STM32_PWM_40000400_PWM_LABEL
#define DT_PWM_STM32_3_PRESCALER		DT_ST_STM32_PWM_40000400_PWM_ST_PRESCALER

/* there is no reference to GPIOE, GPIOG and GPIOH in the dts files */

#define DT_GPIO_STM32_GPIOA_BASE_ADDRESS	DT_ST_STM32_GPIO_50000000_BASE_ADDRESS
#define DT_GPIO_STM32_GPIOA_CLOCK_CONTROLLER	DT_ST_STM32_GPIO_50000000_CLOCKS_CONTROLLER
#define DT_GPIO_STM32_GPIOA_LABEL		DT_ST_STM32_GPIO_50000000_LABEL
#define DT_GPIO_STM32_GPIOA_SIZE		DT_ST_STM32_GPIO_50000000_SIZE
#define DT_GPIO_STM32_GPIOA_CLOCK_BITS		DT_ST_STM32_GPIO_50000000_CLOCKS_BITS
#define DT_GPIO_STM32_GPIOA_CLOCK_BUS		DT_ST_STM32_GPIO_50000000_CLOCKS_BUS

#define DT_GPIO_STM32_GPIOB_BASE_ADDRESS	DT_ST_STM32_GPIO_50000400_BASE_ADDRESS
#define DT_GPIO_STM32_GPIOB_CLOCK_CONTROLLER	DT_ST_STM32_GPIO_50000400_CLOCKS_CONTROLLER
#define DT_GPIO_STM32_GPIOB_LABEL		DT_ST_STM32_GPIO_50000400_LABEL
#define DT_GPIO_STM32_GPIOB_SIZE		DT_ST_STM32_GPIO_50000400_SIZE
#define DT_GPIO_STM32_GPIOB_CLOCK_BITS		DT_ST_STM32_GPIO_50000400_CLOCKS_BITS
#define DT_GPIO_STM32_GPIOB_CLOCK_BUS		DT_ST_STM32_GPIO_50000400_CLOCKS_BUS

#define DT_GPIO_STM32_GPIOC_BASE_ADDRESS	DT_ST_STM32_GPIO_50000800_BASE_ADDRESS
#define DT_GPIO_STM32_GPIOC_CLOCK_CONTROLLER	DT_ST_STM32_GPIO_50000800_CLOCKS_CONTROLLER
#define DT_GPIO_STM32_GPIOC_LABEL		DT_ST_STM32_GPIO_50000800_LABEL
#define DT_GPIO_STM32_GPIOC_SIZE		DT_ST_STM32_GPIO_50000800_SIZE
#define DT_GPIO_STM32_GPIOC_CLOCK_BITS		DT_ST_STM32_GPIO_50000800_CLOCKS_BITS
#define DT_GPIO_STM32_GPIOC_CLOCK_BUS		DT_ST_STM32_GPIO_50000800_CLOCKS_BUS

#define DT_GPIO_STM32_GPIOD_BASE_ADDRESS	DT_ST_STM32_GPIO_50000C00_BASE_ADDRESS
#define DT_GPIO_STM32_GPIOD_CLOCK_CONTROLLER	DT_ST_STM32_GPIO_50000C00_CLOCKS_CONTROLLER
#define DT_GPIO_STM32_GPIOD_LABEL		DT_ST_STM32_GPIO_50000C00_LABEL
#define DT_GPIO_STM32_GPIOD_SIZE		DT_ST_STM32_GPIO_50000C00_SIZE
#define DT_GPIO_STM32_GPIOD_CLOCK_BITS		DT_ST_STM32_GPIO_50000C00_CLOCKS_BITS
#define DT_GPIO_STM32_GPIOD_CLOCK_BUS		DT_ST_STM32_GPIO_50000C00_CLOCKS_BUS

#define DT_GPIO_STM32_GPIOF_BASE_ADDRESS	DT_ST_STM32_GPIO_50001400_BASE_ADDRESS
#define DT_GPIO_STM32_GPIOF_CLOCK_CONTROLLER	DT_ST_STM32_GPIO_50001400_CLOCKS_CONTROLLER
#define DT_GPIO_STM32_GPIOF_LABEL		DT_ST_STM32_GPIO_50001400_LABEL
#define DT_GPIO_STM32_GPIOF_SIZE		DT_ST_STM32_GPIO_50001400_SIZE
#define DT_GPIO_STM32_GPIOF_CLOCK_BITS		DT_ST_STM32_GPIO_50001400_CLOCKS_BITS
#define DT_GPIO_STM32_GPIOF_CLOCK_BUS		DT_ST_STM32_GPIO_50001400_CLOCKS_BUS

/* there is no reference to GPIOE, GPIOG and GPIOH in the dts files */

#define DT_UART_STM32_USART_1_BASE_ADDRESS	DT_ST_STM32_USART_40013800_BASE_ADDRESS
#define DT_UART_STM32_USART_1_BAUD_RATE		DT_ST_STM32_USART_40013800_CURRENT_SPEED
#define DT_UART_STM32_USART_1_IRQ_PRI		DT_ST_STM32_USART_40013800_IRQ_0_PRIORITY
#define DT_UART_STM32_USART_1_NAME		DT_ST_STM32_USART_40013800_LABEL
#define DT_USART_1_IRQ				DT_ST_STM32_USART_40013800_IRQ_0
#define DT_UART_STM32_USART_1_CLOCK_BITS	DT_ST_STM32_USART_40013800_CLOCKS_BITS
#define DT_UART_STM32_USART_1_CLOCK_BUS		DT_ST_STM32_USART_40013800_CLOCKS_BUS

#define DT_UART_STM32_USART_2_BASE_ADDRESS	DT_ST_STM32_USART_40004400_BASE_ADDRESS
#define DT_UART_STM32_USART_2_BAUD_RATE		DT_ST_STM32_USART_40004400_CURRENT_SPEED
#define DT_UART_STM32_USART_2_IRQ_PRI		DT_ST_STM32_USART_40004400_IRQ_0_PRIORITY
#define DT_UART_STM32_USART_2_NAME		DT_ST_STM32_USART_40004400_LABEL
#define DT_USART_2_IRQ				DT_ST_STM32_USART_40004400_IRQ_0
#define DT_UART_STM32_USART_2_CLOCK_BITS	DT_ST_STM32_USART_40004400_CLOCKS_BITS
#define DT_UART_STM32_USART_2_CLOCK_BUS		DT_ST_STM32_USART_40004400_CLOCKS_BUS
#define DT_UART_STM32_USART_2_HW_FLOW_CONTROL	DT_ST_STM32_USART_40004400_HW_FLOW_CONTROL

#define DT_WWDT_0_BASE_ADDRESS		DT_INST_0_ST_STM32_WINDOW_WATCHDOG_BASE_ADDRESS
#define DT_WWDT_0_NAME			DT_INST_0_ST_STM32_WINDOW_WATCHDOG_LABEL
#define DT_WWDT_0_IRQ			DT_INST_0_ST_STM32_WINDOW_WATCHDOG_IRQ_0
#define DT_WWDT_0_IRQ_PRI		DT_INST_0_ST_STM32_WINDOW_WATCHDOG_IRQ_0_PRIORITY
#define DT_WWDT_0_CLOCK_BITS		DT_INST_0_ST_STM32_WINDOW_WATCHDOG_CLOCKS_BITS
#define DT_WWDT_0_CLOCK_BUS		DT_INST_0_ST_STM32_WINDOW_WATCHDOG_CLOCKS_BUS

/* End of SoC Level DTS fixup file */
