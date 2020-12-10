/**
 * \file adrv9009/src/app/headless.c
 *
 * \brief Contains example code for user integration with their application
 *
 * Copyright 2015-2017 Analog Devices Inc.
 * Released under the AD9378-AD9379 API license, for more information see the "LICENSE.txt" file in this zip file.
 *
 */

/****< Insert User Includes Here >***/

#include <stdio.h>
#include "ad9083.h"
//#include "adi_hal.h"
#include "inttypes.h"
#include "spi.h"
#include "spi_extra.h"
#include "gpio_extra.h"
#include "error.h"
#include "delay.h"
#include "parameters.h"
#include "util.h"
//#include "axi_dac_core.h"
#include "axi_adc_core.h"
#include "axi_dmac.h"
#ifndef ALTERA_PLATFORM
#include "xil_cache.h"
#endif
//#include "talise.h"
//#include "talise_config.h"
#include "app_config.h"
#include "app_clocking.h"
#include "app_jesd.h"
#include "app_transceiver.h"
#include "app.h"
#include "ad9528.h"

#ifdef IIO_SUPPORT

#include "iio.h"
#include "iio_axi_adc.h"
#include "iio_axi_dac.h"
#include "irq.h"
#include "irq_extra.h"
#include "uart.h"
#include "uart_extra.h"

static struct uart_desc *uart_desc;

/**
 * iio_uart_write() - Write data to UART device wrapper.
 * @buf - Pointer to buffer containing data.
 * @len - Number of bytes to write.
 * @Return: SUCCESS in case of success, FAILURE otherwise.
 */
static ssize_t iio_uart_write(const char *buf, size_t len)
{
	return uart_write(uart_desc, (const uint8_t *)buf, len);
}

/**
 * iio_uart_read() - Read data from UART device wrapper.
 * @buf - Pointer to buffer containing data.
 * @len - Number of bytes to read.
 * @Return: SUCCESS in case of success, FAILURE otherwise.
 */
static ssize_t iio_uart_read(char *buf, size_t len)
{
	return uart_read(uart_desc, (uint8_t *)buf, len);
}

#endif // IIO_SUPPORT

/**********************************************************/
/**********************************************************/
/********** Talise Data Structure Initializations ********/
/**********************************************************/
/**********************************************************/

int main(void)
{
//	adiHalErr_t err;
	int status;
	struct ad9083_phy *ad9083_phy;

	struct xil_spi_init_param xil_spi_param = {
		.type = SPI_PS,
		.device_id = 0,
//		.flags = SPI_CS_DECODE
	};

	// clock chip spi settings
	struct spi_init_param ad9083_spi_init_param = {
		.max_speed_hz = 10000000,
		.mode = SPI_MODE_0,
		.chip_select = SPI_AD9083_CS,

		.platform_ops = &xil_platform_ops,
		.extra = &xil_spi_param
	};
	struct xil_gpio_init_param  xil_gpio_param = {
		.type = GPIO_PS,
		.device_id = GPIO_DEVICE_ID
	};
	struct gpio_init_param	gpio_phy_resetb = {
		.number = AD9528_RSTB,
		.platform_ops = &xil_gpio_platform_ops,
		.extra = &xil_gpio_param
	};

	struct ad9083_init_param ad9083_init_param = {
		.spi_init = &ad9083_spi_init_param,
		.gpio_reset = &gpio_phy_resetb,
	};

	printf("Hello\n");
	status = app_clocking_init(500000, 250000, 250000, 10000000, 10000000);
	if (status != SUCCESS)
		printf("app_clock_init() error: %" PRId32 "\n", status);

//	status = ad9083_init(&ad9083_phy, &ad9083_init_param);
//	if (status != SUCCESS)
//		printf("ad9083_initialize() error: %" PRId32 "\n", status);


//	status = app_jesd_init(jesd_clk,
//			       500000, 250000, 250000, 10000000, 10000000);
//	if (status != SUCCESS)
//		printf("app_jesd_init() error: %" PRId32 "\n", status);

	printf("Bye\n");
	return SUCCESS;
}
