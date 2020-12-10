/***************************************************************************//**
 *   @file   app_clocking.c
 *   @brief  Clock setup and initialization routines.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
// stdlibs
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_config.h"


#include "ad9528.h"

// platform drivers
#include "spi.h"
#include "spi_extra.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "error.h"
#include "delay.h"
#include "util.h"

// hal
#include "parameters.h"
//#include "adi_hal.h"

// header
#include "app_clocking.h"

#define SYSREF_FPGA_CLK 0
#define GLBL_CLK	1
#define REF_CLK		3
#define SYSREF_ADC_CLK	12

struct ad9528_dev* clkchip_device;

struct axi_clkgen *rx_clkgen;
struct axi_clkgen *tx_clkgen;
struct axi_clkgen *rx_os_clkgen;


int32_t app_clocking_init(uint32_t rx_div40_rate_hz,
			  uint32_t tx_div40_rate_hz,
			  uint32_t rx_os_div40_rate_hz,
			  uint32_t device_clock_khz,
			  uint32_t lmfc_rate_hz)
{
	int32_t status;
	uint64_t dev_clk, fmc_clk;
	uint64_t rate_dev = device_clock_khz * 1000;
	uint64_t rate_fmc = device_clock_khz * 1000;
	uint32_t n;
	int ret;

	struct ad9528_channel_spec ad9528_channels[14];
	struct ad9528_init_param ad9528_param;
	struct ad9528_platform_data ad9528_pdata;

	// ad9528 defaults
	ad9528_param.pdata = &ad9528_pdata;
	ad9528_param.pdata->num_channels = 14;
	ad9528_param.pdata->channels = &ad9528_channels[0];

	status = ad9528_init(&ad9528_param);
	if(status) {
		printf("error: ad9528_init() failed with %d\n", status);
		goto error_0;
	}

	// ad9528 channel defaults
	for(unsigned int ch = 0; ch < ad9528_param.pdata->num_channels; ch++) {
		ad9528_channels[ch].channel_num = ch;
		ad9528_channels[ch].output_dis = 1;
	}

	// ad9528 channel specifics

	// SYSREF to FPGA
	ad9528_channels[0].output_dis = 0;
	ad9528_channels[0].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[0].divider_phase = 0;
	ad9528_channels[0].signal_source = SOURCE_VCO;

	// GLBLCLK
	ad9528_channels[1].output_dis = 0;
	ad9528_channels[1].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[1].divider_phase = 0;
	ad9528_channels[1].signal_source = SOURCE_VCO;

	// REFCLK
	ad9528_channels[3].output_dis = 0;
	ad9528_channels[3].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[3].divider_phase = 0;
	ad9528_channels[3].signal_source = SOURCE_SYSREF_VCO;

	// SYSREF to ADC
	ad9528_channels[12].output_dis = 0;
	ad9528_channels[12].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[12].divider_phase = 0;
	ad9528_channels[12].signal_source = SOURCE_SYSREF_VCO;

	// ad9528 settings
	ad9528_param.pdata->spi3wire = 1;
	ad9528_param.pdata->vcxo_freq = 100000000;//122880000;
	ad9528_param.pdata->refa_en = 1;
	ad9528_param.pdata->refa_diff_rcv_en = 1;
	ad9528_param.pdata->refa_r_div = 1;
	ad9528_param.pdata->osc_in_cmos_neg_inp_en = 1;
	ad9528_param.pdata->pll1_feedback_div = 4;
	ad9528_param.pdata->pll1_feedback_src_vcxo = 0; /* VCO */
	ad9528_param.pdata->pll1_charge_pump_current_nA = 5000;
	ad9528_param.pdata->pll1_bypass_en = 0;
	ad9528_param.pdata->pll2_vco_div_m1 = 3;
	ad9528_param.pdata->pll2_n2_div = 10;
	ad9528_param.pdata->pll2_r1_div = 1;
	ad9528_param.pdata->pll2_charge_pump_current_nA = 805000;
	ad9528_param.pdata->pll2_bypass_en = false;
	ad9528_param.pdata->sysref_src = SYSREF_SRC_INTERNAL;
	ad9528_param.pdata->sysref_pattern_mode = SYSREF_PATTERN_CONTINUOUS;
	ad9528_param.pdata->sysref_req_en = true;
	ad9528_param.pdata->sysref_nshot_mode = SYSREF_NSHOT_4_PULSES;
	ad9528_param.pdata->sysref_req_trigger_mode = SYSREF_LEVEL_HIGH;
	ad9528_param.pdata->rpole2 = RPOLE2_900_OHM;
	ad9528_param.pdata->rzero = RZERO_1850_OHM;
	ad9528_param.pdata->cpole1 = CPOLE1_16_PF;
	ad9528_param.pdata->stat0_pin_func_sel = 0x1; /* PLL1 & PLL2 Locked */
	ad9528_param.pdata->stat1_pin_func_sel = 0x7; /* REFA Correct */


	struct xil_spi_init_param xil_spi_param = {
		.type = SPI_PS,
		.device_id = 0,
//		.flags = SPI_CS_DECODE
	};

	struct xil_gpio_init_param xil_gpio_param = {
		.type = GPIO_PS,
		.device_id = GPIO_DEVICE_ID,
	};


	// clock chip spi settings
	struct spi_init_param clkchip_spi_init_param = {
		.max_speed_hz = 10000000,
		.mode = SPI_MODE_0,
		.chip_select = CLK_AD9258_CS,

		.platform_ops = &xil_platform_ops,
		.extra = &xil_spi_param
	};


	ad9528_param.spi_init = clkchip_spi_init_param;

	/* Reset pin is tied up to P3V3_CLK_PLL trough a pull up resistor */
	ad9528_param.gpio_resetb = NULL;


	/** < Insert User System Clock(s) Initialization Code Here >
	* System Clock should provide a device clock and SYSREF signal
	* to the Talise device.
	**/
	status = ad9528_setup(&clkchip_device, ad9528_param);
	if(status < 0) {
		printf("error: ad9528_setup() failed with %d\n", status);
		goto error_1;
	}


	dev_clk = ad9528_clk_round_rate(clkchip_device, SYSREF_FPGA_CLK,
					device_clock_khz * 1000);
	dev_clk = ad9528_clk_round_rate(clkchip_device, GLBL_CLK,
					device_clock_khz * 1000);
	dev_clk = ad9528_clk_round_rate(clkchip_device, REF_CLK,
					device_clock_khz * 1000);
	dev_clk = ad9528_clk_round_rate(clkchip_device, SYSREF_ADC_CLK,
					device_clock_khz * 1000);

//	dev_clk = ad9528_clk_round_rate(clkchip_device, DEV_CLK,
//					device_clock_khz * 1000);
//
//	fmc_clk = ad9528_clk_round_rate(clkchip_device, FMC_CLK,
//					device_clock_khz * 1000);
//
//	if (dev_clk > 0 && fmc_clk > 0 && fmc_clk == dev_clk &&
//	    (dev_clk / 1000) == device_clock_khz) {
//		ad9528_clk_set_rate(clkchip_device, DEV_CLK, dev_clk);
//		ad9528_clk_set_rate(clkchip_device, FMC_CLK, fmc_clk);
//
//	} else {
//		printf("Requesting device clock %u failed got %u\n",
//		       device_clock_khz * 1000, dev_clk);
//		goto error_1;
//	}

//	/* If the current rate is not OK, change it */
//	if (!(adrv9009_check_sysref_rate(lmfc_rate_hz, rate_dev) && ///////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	      (rate_fmc == rate_dev))) {
//		/*
//		* Try to find a rate that integer divides the LMFC. Starting with a low
//		* rate is a good idea and then slowly go up in case the clock generator
//		* can't generate such slow rates.
//		*/
//		for (n = 64; n > 0; n--) {
//			rate_dev = ad9528_clk_round_rate(clkchip_device, DEV_SYSREF, lmfc_rate_hz / n);
//
//			if (adrv9009_check_sysref_rate(lmfc_rate_hz, rate_dev))
//				break;
//		}
//
//		if (n == 0) {
//			printf("Could not find suitable SYSREF rate for LMFC of %u\n", lmfc_rate_hz);
//			goto error_1;
//		}
//
//		ret = ad9528_clk_set_rate(clkchip_device, FMC_SYSREF, rate_fmc);
//		if (ret)
//			printf("Failed to set FMC SYSREF rate to %u Hz: %d\n",
//			       rate_fmc, ret);
//
//		ret = ad9528_clk_set_rate(clkchip_device, DEV_SYSREF, rate_dev);
//		if (ret)
//			printf("Failed to set DEV SYSREF rate to %u Hz: %d\n",
//			       rate_fmc, ret);
//	}

	return SUCCESS;

error_1:
	ad9528_remove(clkchip_device);
error_0:
	return FAILURE;
}

void app_clocking_deinit(void)
{
	ad9528_remove(clkchip_device);
}
