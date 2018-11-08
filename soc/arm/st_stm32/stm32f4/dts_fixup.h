/* SoC level DTS fixup file */

#define CONFIG_NUM_IRQ_PRIO_BITS	        ARM_V7M_NVIC_E000E100_ARM_NUM_IRQ_PRIORITY_BITS

#define CONFIG_UART_STM32_USART_1_BASE_ADDRESS	ST_STM32_USART_40011000_BASE_ADDRESS
#define CONFIG_UART_STM32_USART_1_BAUD_RATE	ST_STM32_USART_40011000_CURRENT_SPEED
#define CONFIG_UART_STM32_USART_1_IRQ_PRI	ST_STM32_USART_40011000_IRQ_0_PRIORITY
#define CONFIG_UART_STM32_USART_1_NAME		ST_STM32_USART_40011000_LABEL
#define USART_1_IRQ				ST_STM32_USART_40011000_IRQ_0
#define CONFIG_UART_STM32_USART_1_CLOCK_BITS	ST_STM32_USART_40011000_CLOCK_BITS
#define CONFIG_UART_STM32_USART_1_CLOCK_BUS	ST_STM32_USART_40011000_CLOCK_BUS

#define CONFIG_UART_STM32_USART_2_BASE_ADDRESS	ST_STM32_USART_40004400_BASE_ADDRESS
#define CONFIG_UART_STM32_USART_2_BAUD_RATE	ST_STM32_USART_40004400_CURRENT_SPEED
#define CONFIG_UART_STM32_USART_2_IRQ_PRI	ST_STM32_USART_40004400_IRQ_0_PRIORITY
#define CONFIG_UART_STM32_USART_2_NAME		ST_STM32_USART_40004400_LABEL
#define USART_2_IRQ				ST_STM32_USART_40004400_IRQ_0
#define CONFIG_UART_STM32_USART_2_CLOCK_BITS	ST_STM32_USART_40004400_CLOCK_BITS
#define CONFIG_UART_STM32_USART_2_CLOCK_BUS	ST_STM32_USART_40004400_CLOCK_BUS

#define CONFIG_UART_STM32_USART_3_BASE_ADDRESS   ST_STM32_USART_40004800_BASE_ADDRESS
#define CONFIG_UART_STM32_USART_3_BAUD_RATE      ST_STM32_USART_40004800_CURRENT_SPEED
#define CONFIG_UART_STM32_USART_3_IRQ_PRI        ST_STM32_USART_40004800_IRQ_0_PRIORITY
#define CONFIG_UART_STM32_USART_3_NAME           ST_STM32_USART_40004800_LABEL
#define USART_3_IRQ                              ST_STM32_USART_40004800_IRQ_0
#define CONFIG_UART_STM32_USART_3_CLOCK_BITS     ST_STM32_USART_40004800_CLOCK_BITS
#define CONFIG_UART_STM32_USART_3_CLOCK_BUS      ST_STM32_USART_40004800_CLOCK_BUS

#define CONFIG_UART_STM32_USART_6_NAME           ST_STM32_USART_40011400_LABEL
#define CONFIG_UART_STM32_USART_6_BASE_ADDRESS   ST_STM32_USART_40011400_BASE_ADDRESS
#define CONFIG_UART_STM32_USART_6_BAUD_RATE      ST_STM32_USART_40011400_CURRENT_SPEED
#define CONFIG_UART_STM32_USART_6_IRQ_PRI        ST_STM32_USART_40011400_IRQ_0_PRIORITY
#define USART_6_IRQ                              ST_STM32_USART_40011400_IRQ_0
#define CONFIG_UART_STM32_USART_6_CLOCK_BITS     ST_STM32_USART_40011400_CLOCK_BITS
#define CONFIG_UART_STM32_USART_6_CLOCK_BUS      ST_STM32_USART_40011400_CLOCK_BUS

#define CONFIG_I2C_1_BASE_ADDRESS               ST_STM32_I2C_V1_40005400_BASE_ADDRESS
#define CONFIG_I2C_1_EVENT_IRQ_PRI              ST_STM32_I2C_V1_40005400_IRQ_EVENT_PRIORITY
#define CONFIG_I2C_1_ERROR_IRQ_PRI              ST_STM32_I2C_V1_40005400_IRQ_ERROR_PRIORITY
#define CONFIG_I2C_1_NAME                       ST_STM32_I2C_V1_40005400_LABEL
#define CONFIG_I2C_1_EVENT_IRQ                  ST_STM32_I2C_V1_40005400_IRQ_EVENT
#define CONFIG_I2C_1_ERROR_IRQ                  ST_STM32_I2C_V1_40005400_IRQ_ERROR
#define CONFIG_I2C_1_BITRATE                    ST_STM32_I2C_V1_40005400_CLOCK_FREQUENCY
#define CONFIG_I2C_1_CLOCK_BITS			ST_STM32_I2C_V1_40005400_CLOCK_BITS
#define CONFIG_I2C_1_CLOCK_BUS			ST_STM32_I2C_V1_40005400_CLOCK_BUS

#define CONFIG_I2C_2_BASE_ADDRESS               ST_STM32_I2C_V1_40005800_BASE_ADDRESS
#define CONFIG_I2C_2_EVENT_IRQ_PRI              ST_STM32_I2C_V1_40005800_IRQ_EVENT_PRIORITY
#define CONFIG_I2C_2_ERROR_IRQ_PRI              ST_STM32_I2C_V1_40005800_IRQ_ERROR_PRIORITY
#define CONFIG_I2C_2_NAME                       ST_STM32_I2C_V1_40005800_LABEL
#define CONFIG_I2C_2_EVENT_IRQ                  ST_STM32_I2C_V1_40005800_IRQ_EVENT
#define CONFIG_I2C_2_ERROR_IRQ                  ST_STM32_I2C_V1_40005800_IRQ_ERROR
#define CONFIG_I2C_2_BITRATE                    ST_STM32_I2C_V1_40005800_CLOCK_FREQUENCY
#define CONFIG_I2C_2_CLOCK_BITS			ST_STM32_I2C_V1_40005800_CLOCK_BITS
#define CONFIG_I2C_2_CLOCK_BUS			ST_STM32_I2C_V1_40005800_CLOCK_BUS

#define CONFIG_I2C_3_BASE_ADDRESS               ST_STM32_I2C_V1_40005C00_BASE_ADDRESS
#define CONFIG_I2C_3_EVENT_IRQ_PRI              ST_STM32_I2C_V1_40005C00_IRQ_EVENT_PRIORITY
#define CONFIG_I2C_3_ERROR_IRQ_PRI              ST_STM32_I2C_V1_40005C00_IRQ_ERROR_PRIORITY
#define CONFIG_I2C_3_NAME                       ST_STM32_I2C_V1_40005C00_LABEL
#define CONFIG_I2C_3_EVENT_IRQ                  ST_STM32_I2C_V1_40005C00_IRQ_EVENT
#define CONFIG_I2C_3_ERROR_IRQ                  ST_STM32_I2C_V1_40005C00_IRQ_ERROR
#define CONFIG_I2C_3_BITRATE                    ST_STM32_I2C_V1_40005C00_CLOCK_FREQUENCY
#define CONFIG_I2C_3_CLOCK_BITS			ST_STM32_I2C_V1_40005C00_CLOCK_BITS
#define CONFIG_I2C_3_CLOCK_BUS			ST_STM32_I2C_V1_40005C00_CLOCK_BUS

#define CONFIG_SPI_1_BASE_ADDRESS               ST_STM32_SPI_40013000_BASE_ADDRESS
#define CONFIG_SPI_1_IRQ_PRI                    ST_STM32_SPI_40013000_IRQ_0_PRIORITY
#define CONFIG_SPI_1_NAME                       ST_STM32_SPI_40013000_LABEL
#define CONFIG_SPI_1_IRQ                        ST_STM32_SPI_40013000_IRQ_0

#define CONFIG_SPI_2_BASE_ADDRESS               ST_STM32_SPI_40003800_BASE_ADDRESS
#define CONFIG_SPI_2_IRQ_PRI                    ST_STM32_SPI_40003800_IRQ_0_PRIORITY
#define CONFIG_SPI_2_NAME                       ST_STM32_SPI_40003800_LABEL
#define CONFIG_SPI_2_IRQ                        ST_STM32_SPI_40003800_IRQ_0

#define CONFIG_SPI_3_BASE_ADDRESS		ST_STM32_SPI_40003C00_BASE_ADDRESS
#define CONFIG_SPI_3_IRQ_PRI			ST_STM32_SPI_40003C00_IRQ_0_PRIORITY
#define CONFIG_SPI_3_NAME			ST_STM32_SPI_40003C00_LABEL
#define CONFIG_SPI_3_IRQ			ST_STM32_SPI_40003C00_IRQ_0

#define CONFIG_SPI_4_BASE_ADDRESS		ST_STM32_SPI_40013400_BASE_ADDRESS
#define CONFIG_SPI_4_IRQ_PRI			ST_STM32_SPI_40013400_IRQ_0_PRIORITY
#define CONFIG_SPI_4_NAME			ST_STM32_SPI_40013400_LABEL
#define CONFIG_SPI_4_IRQ			ST_STM32_SPI_40013400_IRQ_0

#define CONFIG_SPI_5_BASE_ADDRESS		ST_STM32_SPI_40015000_BASE_ADDRESS
#define CONFIG_SPI_5_IRQ_PRI			ST_STM32_SPI_40015000_IRQ_0_PRIORITY
#define CONFIG_SPI_5_NAME			ST_STM32_SPI_40015000_LABEL
#define CONFIG_SPI_5_IRQ			ST_STM32_SPI_40015000_IRQ_0

#define CONFIG_SPI_6_BASE_ADDRESS		ST_STM32_SPI_40015400_BASE_ADDRESS
#define CONFIG_SPI_6_IRQ_PRI			ST_STM32_SPI_40015400_IRQ_0_PRIORITY
#define CONFIG_SPI_6_NAME			ST_STM32_SPI_40015400_LABEL
#define CONFIG_SPI_6_IRQ			ST_STM32_SPI_40015400_IRQ_0

#define CONFIG_I2S_1_BASE_ADDRESS               ST_STM32_I2S_40013000_BASE_ADDRESS
#define CONFIG_I2S_1_IRQ_PRI                    ST_STM32_I2S_40013000_IRQ_0_PRIORITY
#define CONFIG_I2S_1_NAME                       ST_STM32_I2S_40013000_LABEL
#define CONFIG_I2S_1_IRQ                        ST_STM32_I2S_40013000_IRQ_0
#define CONFIG_I2S_1_CLOCK_BITS                 ST_STM32_I2S_40013000_CLOCK_BITS
#define CONFIG_I2S_1_CLOCK_BUS                  ST_STM32_I2S_40013000_CLOCK_BUS

#define CONFIG_I2S_2_BASE_ADDRESS               ST_STM32_I2S_40003800_BASE_ADDRESS
#define CONFIG_I2S_2_IRQ_PRI                    ST_STM32_I2S_40003800_IRQ_0_PRIORITY
#define CONFIG_I2S_2_NAME                       ST_STM32_I2S_40003800_LABEL
#define CONFIG_I2S_2_IRQ                        ST_STM32_I2S_40003800_IRQ_0
#define CONFIG_I2S_2_CLOCK_BITS                 ST_STM32_I2S_40003800_CLOCK_BITS
#define CONFIG_I2S_2_CLOCK_BUS                  ST_STM32_I2S_40003800_CLOCK_BUS

#define CONFIG_I2S_3_BASE_ADDRESS               ST_STM32_I2S_40003C00_BASE_ADDRESS
#define CONFIG_I2S_3_IRQ_PRI                    ST_STM32_I2S_40003C00_IRQ_0_PRIORITY
#define CONFIG_I2S_3_NAME                       ST_STM32_I2S_40003C00_LABEL
#define CONFIG_I2S_3_IRQ                        ST_STM32_I2S_40003C00_IRQ_0
#define CONFIG_I2S_3_CLOCK_BITS                 ST_STM32_I2S_40003C00_CLOCK_BITS
#define CONFIG_I2S_3_CLOCK_BUS                  ST_STM32_I2S_40003C00_CLOCK_BUS

#define CONFIG_I2S_4_BASE_ADDRESS               ST_STM32_I2S_40013400_BASE_ADDRESS
#define CONFIG_I2S_4_IRQ_PRI                    ST_STM32_I2S_40013400_IRQ_0_PRIORITY
#define CONFIG_I2S_4_NAME                       ST_STM32_I2S_40013400_LABEL
#define CONFIG_I2S_4_IRQ                        ST_STM32_I2S_40013400_IRQ_0
#define CONFIG_I2S_4_CLOCK_BITS                 ST_STM32_I2S_40013400_CLOCK_BITS
#define CONFIG_I2S_4_CLOCK_BUS                  ST_STM32_I2S_40013400_CLOCK_BUS

#define CONFIG_I2S_5_BASE_ADDRESS               ST_STM32_I2S_40015000_BASE_ADDRESS
#define CONFIG_I2S_5_IRQ_PRI                    ST_STM32_I2S_40015000_IRQ_0_PRIORITY
#define CONFIG_I2S_5_NAME                       ST_STM32_I2S_40015000_LABEL
#define CONFIG_I2S_5_IRQ                        ST_STM32_I2S_40015000_IRQ_0
#define CONFIG_I2S_5_CLOCK_BITS                 ST_STM32_I2S_40015000_CLOCK_BITS
#define CONFIG_I2S_5_CLOCK_BUS                  ST_STM32_I2S_40015000_CLOCK_BUS

#define CONFIG_I2S_6_BASE_ADDRESS               ST_STM32_I2S_40015400_BASE_ADDRESS
#define CONFIG_I2S_6_IRQ_PRI                    ST_STM32_I2S_40015400_IRQ_0_PRIORITY
#define CONFIG_I2S_6_NAME                       ST_STM32_I2S_40015400_LABEL
#define CONFIG_I2S_6_IRQ                        ST_STM32_I2S_40015400_IRQ_0
#define CONFIG_I2S_6_CLOCK_BITS                 ST_STM32_I2S_40015400_CLOCK_BITS
#define CONFIG_I2S_6_CLOCK_BUS                  ST_STM32_I2S_40015400_CLOCK_BUS

#define FLASH_DEV_BASE_ADDRESS		        ST_STM32F4_FLASH_CONTROLLER_40023C00_BASE_ADDRESS
#define FLASH_DEV_NAME			        ST_STM32F4_FLASH_CONTROLLER_40023C00_LABEL

#ifdef ST_STM32_OTGFS_50000000_BASE_ADDRESS
#define CONFIG_USB_BASE_ADDRESS			ST_STM32_OTGFS_50000000_BASE_ADDRESS
#define CONFIG_USB_IRQ				ST_STM32_OTGFS_50000000_IRQ_OTGFS
#define CONFIG_USB_IRQ_PRI			ST_STM32_OTGFS_50000000_IRQ_OTGFS_PRIORITY
#define CONFIG_USB_NUM_BIDIR_ENDPOINTS		ST_STM32_OTGFS_50000000_NUM_BIDIR_ENDPOINTS
#define CONFIG_USB_RAM_SIZE			ST_STM32_OTGFS_50000000_RAM_SIZE
#define CONFIG_USB_MAXIMUM_SPEED		ST_STM32_OTGFS_50000000_MAXIMUM_SPEED
#endif /*  ST_STM32_OTGFS_50000000_BASE_ADDRESS */

#ifdef ST_STM32_OTGHS_40040000_BASE_ADDRESS
#define CONFIG_USB_HS_BASE_ADDRESS		ST_STM32_OTGHS_40040000_BASE_ADDRESS
#define CONFIG_USB_IRQ				ST_STM32_OTGHS_40040000_IRQ_OTGHS
#define CONFIG_USB_IRQ_PRI			ST_STM32_OTGHS_40040000_IRQ_OTGHS_PRIORITY
#define CONFIG_USB_NUM_BIDIR_ENDPOINTS		ST_STM32_OTGHS_40040000_NUM_BIDIR_ENDPOINTS
#define CONFIG_USB_RAM_SIZE			ST_STM32_OTGHS_40040000_RAM_SIZE
#define CONFIG_USB_MAXIMUM_SPEED		ST_STM32_OTGHS_40040000_MAXIMUM_SPEED
#endif /* ST_STM32_OTGHS_40040000_BASE_ADDRESS */

#define CONFIG_PWM_STM32_1_DEV_NAME             ST_STM32_PWM_40010000_PWM_LABEL
#define CONFIG_PWM_STM32_1_PRESCALER            ST_STM32_PWM_40010000_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_2_DEV_NAME             ST_STM32_PWM_40000000_PWM_LABEL
#define CONFIG_PWM_STM32_2_PRESCALER            ST_STM32_PWM_40000000_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_3_DEV_NAME             ST_STM32_PWM_40000400_PWM_LABEL
#define CONFIG_PWM_STM32_3_PRESCALER            ST_STM32_PWM_40000400_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_4_DEV_NAME             ST_STM32_PWM_40000800_PWM_LABEL
#define CONFIG_PWM_STM32_4_PRESCALER            ST_STM32_PWM_40000800_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_5_DEV_NAME             ST_STM32_PWM_40000C00_PWM_LABEL
#define CONFIG_PWM_STM32_5_PRESCALER            ST_STM32_PWM_40000C00_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_6_DEV_NAME             ST_STM32_PWM_40001000_PWM_LABEL
#define CONFIG_PWM_STM32_6_PRESCALER            ST_STM32_PWM_40001000_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_7_DEV_NAME             ST_STM32_PWM_40001400_PWM_LABEL
#define CONFIG_PWM_STM32_7_PRESCALER            ST_STM32_PWM_40001400_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_8_DEV_NAME             ST_STM32_PWM_40010400_PWM_LABEL
#define CONFIG_PWM_STM32_8_PRESCALER            ST_STM32_PWM_40010400_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_9_DEV_NAME             ST_STM32_PWM_40014000_PWM_LABEL
#define CONFIG_PWM_STM32_9_PRESCALER            ST_STM32_PWM_40014000_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_10_DEV_NAME            ST_STM32_PWM_40014400_PWM_LABEL
#define CONFIG_PWM_STM32_10_PRESCALER           ST_STM32_PWM_40014400_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_11_DEV_NAME            ST_STM32_PWM_40014800_PWM_LABEL
#define CONFIG_PWM_STM32_11_PRESCALER           ST_STM32_PWM_40014800_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_12_DEV_NAME            ST_STM32_PWM_40001800_PWM_LABEL
#define CONFIG_PWM_STM32_12_PRESCALER           ST_STM32_PWM_40001800_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_13_DEV_NAME            ST_STM32_PWM_40001C00_PWM_LABEL
#define CONFIG_PWM_STM32_13_PRESCALER           ST_STM32_PWM_40001C00_PWM_ST_PRESCALER

#define CONFIG_PWM_STM32_14_DEV_NAME            ST_STM32_PWM_40002000_PWM_LABEL
#define CONFIG_PWM_STM32_14_PRESCALER           ST_STM32_PWM_40002000_PWM_ST_PRESCALER

#define CONFIG_RTC_0_BASE_ADDRESS               ST_STM32_RTC_40002800_BASE_ADDRESS
#define CONFIG_RTC_0_IRQ_PRI                    ST_STM32_RTC_40002800_IRQ_0_PRIORITY
#define CONFIG_RTC_0_IRQ                        ST_STM32_RTC_40002800_IRQ_0
#define CONFIG_RTC_0_NAME                       ST_STM32_RTC_40002800_LABEL
#define CONFIG_RTC_PRESCALER                    ST_STM32_RTC_40002800_PRESCALER

/* End of SoC Level DTS fixup file */
