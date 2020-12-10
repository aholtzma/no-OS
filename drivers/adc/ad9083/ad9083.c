/***************************************************************************//**
 *   @file   ad9083.c
 *   @brief  Implementation of ad9083 Driver.
 *   @author Stefan Popa (stefan.popa@analog.com)
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "ad9083.h"
#include "error.h"
#include <inttypes.h>

#define CHIPID_AD9083	0x9083
#define CHIPID_MASK	0xFFFF

/**
 * Spi write and read compatible with ad9083 API
 * @param user_data
 * @param wbuf Pointer to array with the data to be sent on the SPI
 * @param rbuf Pointer to array where the data to which the SPI will be written
 * @param len The size in bytes allocated for each of the indata and outdata arrays.
 * @return 0 for success, any non-zero value indicates an error
 */
static int32_t ad9083_spi_xfer(void *user_data, uint8_t *in_data,
			       uint8_t *out_data, uint32_t size_bytes)
{
	struct ad9083_phy *phy = user_data;
	uint8_t data[6];
	uint16_t bytes_number;
	int32_t ret;
	int32_t i;

	bytes_number = (size_bytes & 0xFF);

	for (i = 0; i < bytes_number; i++)
		data[i] =  in_data[i];

	ret = spi_write_and_read(phy->spi_desc, data, bytes_number);
	if (ret != SUCCESS)
		return FAILURE;

	for (i = 0; i < bytes_number; i++)
		out_data[i] =  data[i];

	return SUCCESS;
}

int32_t ad9083_log_write(void *user_data, int32_t log_type, const char *message,
			 va_list argp)
{
	char logMessage[160];

	vsnprintf(logMessage, sizeof(logMessage), message, argp);

	switch (log_type) {
	case ADI_CMS_LOG_NONE:
		break;
	case ADI_CMS_LOG_MSG:
		break;
	case ADI_CMS_LOG_WARN:
		printf("%s\n", logMessage);
		break;
	case ADI_CMS_LOG_ERR:
		printf("%s\n", logMessage);
		break;
	case ADI_CMS_LOG_SPI:
		break;
	case ADI_CMS_LOG_API:
		break;
	case ADI_CMS_LOG_ALL:
		printf(logMessage);
		break;
	}

	return 0;
}

/**
 * Set the test mode compatible with ad9083 API
 * @param st - The device structure
 * @param channel - Channel number
 * @param mode - Test mode. Accepted values:
 * 		 ad9083_TESTMODE_OFF
 * 		 ad9083_TESTMODE_MIDSCALE_SHORT
 * 		 ad9083_TESTMODE_POS_FULLSCALE
 * 		 ad9083_TESTMODE_NEG_FULLSCALE
 * 		 ad9083_TESTMODE_ALT_CHECKERBOARD
 * 		 ad9083_TESTMODE_PN23_SEQ
 * 		 ad9083_TESTMODE_PN9_SEQ
 * 		 ad9083_TESTMODE_ONE_ZERO_TOGGLE
 * 		 ad9083_TESTMODE_USER
 * 		 ad9083_TESTMODE_RAMP
 * @return 0 for success, any non-zero value indicates an error
 */
//static int ad9083_testmode_set(struct ad9083_state *st,
//			       unsigned int chan,
//			       unsigned int mode)
//{
//	ad9083_handle_t *ad9083_h = st->adc_h;
//	int ret;
//
//	ret = ad9083_adc_set_channel_select(ad9083_h, BIT(chan & 1));
//	if (ret < 0)
//		return ret;
//
//	ret = ad9083_register_write(ad9083_h, ad9083_REG_TEST_MODE, mode);
//	if (ret < 0)
//		return ret;
//
//	return ad9083_adc_set_channel_select(ad9083_h, ad9083_ADC_CH_ALL);
//}

/**
 * Setup the device.
 * @param st - The device structure.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
//static int32_t ad9083_setup(struct ad9083_state *st)
//{
//	ad9083_handle_t *ad9083_h = st->adc_h;
//	ad9083_adc_data_frmt_t input_fmt, output_fmt;
//	adi_chip_id_t chip_id;
//	uint64_t sample_rate, lane_rate_kbps;
//	uint8_t dcm, pll_stat;
//	int32_t timeout, i, ret;
//
//	ret = ad9083_init(ad9083_h);
//	if (ret < 0) {
//		printf("ad9083 init failed (%d)\n", ret);
//		return -ENODEV;
//	}
//
//	ret = ad9083_reset(ad9083_h, 0);
//	if (ret < 0) {
//		printf("ad9298 reset failed (%d)\n", ret);
//		ret = -ENODEV;
//		goto error;
//	}
//
//	ret = ad9083_get_chip_id(ad9083_h, &chip_id);
//	if (ret < 0) {
//		printf("ad9083_get_chip_id failed (%d)\n", ret);
//		ret = -ENODEV;
//		goto error;
//	}
//
//	if (chip_id.chip_type != ad9083_CHIP_TYPE) {
//		printf("Wrong chip type (%X)\n", chip_id.chip_type);
//		ret = -EINVAL;
//		goto error;
//	}
//
//	if (chip_id.prod_id != ad9083_CHIP_ID) {
//		printf("Unrecognized CHIP_ID (%X)\n", chip_id.prod_id);
//		ret = -ENODEV;
//		goto error;
//	}
//
//	ret = ad9083_adc_set_channel_select(ad9083_h, ad9083_ADC_CH_ALL);
//	if (ret < 0) {
//		printf( "Failed to select channels (%d)\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_set_pdn_pin_mode(ad9083_h, st->powerdown_pin_en,
//				      st->powerdown_mode);
//	if (ret < 0) {
//		printf("Failed to set PWDN pin mode (%d)\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_set_input_clk_duty_cycle_stabilizer(ad9083_h,
//			st->duty_cycle_stabilizer_en);
//	if (ret < 0) {
//		printf("Failed to set clk duty cycle stabilizer (%d)\n", ret);
//		goto error;
//	}
//
//	sample_rate = st->sampling_frequency_hz * st->input_div;
//
//	ret = ad9083_set_input_clk_cfg(ad9083_h, sample_rate,
//				       st->input_div);
//	if (ret < 0) {
//		printf("Failed to set input clk config (%d)\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_adc_set_input_cfg(ad9083_h,
//				       st->analog_input_mode ? COUPLING_DC : COUPLING_AC,
//				       st->ext_vref_en, st->current_scale);
//	if (ret < 0) {
//		printf("Failed to set adc input config: %d\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_adc_set_input_buffer_cfg(ad9083_h, st->buff_curr_n,
//					      st->buff_curr_p, ad9083_BUFF_CURR_600_UA);
//	if (ret < 0) {
//		printf("Failed to set input buffer config: %d\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_adc_set_fc_ch_mode(ad9083_h, st->fc_ch);
//	if (ret < 0) {
//		printf("Failed to set channel mode: %d\n", ret);
//		goto error;
//	}
//
//	if (st->fc_ch == ad9083_FULL_BANDWIDTH_MODE) {
//		dcm = 1; /* Full bandwidth */
//	} else {
//		dcm = st->ddc[0].decimation;
//		for (i = 1; i < st->ddc_cnt; i++)
//			dcm = min_t(uint8_t, dcm, st->ddc[i].decimation);
//	}
//
//	ret = ad9083_adc_set_dcm_mode(ad9083_h, dcm);
//	if (ret < 0) {
//		printf("Failed to set decimation mode: %d\n", ret);
//		goto error;
//	}
//
//	/* DDC Setup */
//	if (st->ddc_input_format_real_en)
//		input_fmt = ad9083_DATA_FRMT_REAL;
//	else
//		input_fmt = ad9083_DATA_FRMT_COMPLEX;
//
//	if (st->ddc_output_format_real_en)
//		output_fmt = ad9083_DATA_FRMT_REAL;
//	else
//		output_fmt = ad9083_DATA_FRMT_COMPLEX;
//
//	ret = ad9083_adc_set_data_format(ad9083_h, input_fmt, output_fmt);
//	if (ret < 0) {
//		printf("Failed to set data format: %d\n", ret);
//		goto error;
//	}
//
//	for (i = 0; i < st->ddc_cnt; i++) {
//		ret = ad9083_adc_set_ddc_gain(ad9083_h, i,
//					      st->ddc[i].gain_db ? 6 : 0);
//		if (ret < 0) {
//			printf("Failed to set ddc gain: %d\n", ret);
//			goto error;
//		}
//
//		ret = ad9083_adc_set_ddc_dcm(ad9083_h, i,
//					     st->ddc[i].decimation);
//		if (ret < 0) {
//			printf("Failed to set ddc decimation mode: %d\n", ret);
//			goto error;
//		}
//
//		ret = ad9083_adc_set_ddc_nco_mode(ad9083_h, i,
//						  st->ddc[i].nco_mode);
//		if (ret < 0) {
//			printf("Failed to set ddc nco mode: %d\n", ret);
//			goto error;
//		}
//
//		ret = ad9083_adc_set_ddc_nco(ad9083_h, i,
//					     st->ddc[i].carrier_freq_hz);
//		if (ret < 0) {
//			printf("Failed to set ddc nco frequency: %d\n", ret);
//			goto error;
//		}
//
//		ret = ad9083_adc_set_ddc_nco_phase(ad9083_h, i, st->ddc[i].po);
//		if (ret < 0) {
//			printf("Failed to set ddc nco phase: %d\n", ret);
//			goto error;
//		}
//	}
//
//	ret = ad9083_testmode_set(st, 0, st->test_mode_ch0);
//	if (ret < 0) {
//		printf("Failed to set test mode for ch 0: %d\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_testmode_set(st, 1, st->test_mode_ch1);
//	if (ret < 0) {
//		printf("Failed to set test mode for ch 1: %d\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_jesd_syref_lmfc_offset_set(ad9083_h,
//						st->sysref_lmfc_offset);
//	if (ret < 0) {
//		printf("Failed to set SYSREF lmfc offset: %d\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_jesd_syref_config_set(ad9083_h, st->sysref_edge_sel,
//					   st->sysref_clk_edge_sel,
//					   st->sysref_neg_window_skew,
//					   st->sysref_pos_window_skew);
//	if (ret < 0) {
//		printf("Failed to set SYSREF sig capture settings: %d\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_jesd_syref_mode_set(ad9083_h, st->sysref_mode,
//					 st->sysref_count);
//	if (ret < 0) {
//		printf("Failed to Set JESD SYNCHRONIZATION Mode: %d\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_jesd_set_if_config(ad9083_h, (jesd_param_t )*st->jesd_param,
//					&lane_rate_kbps);
//	if (ret < 0) {
//		printf("Failed to set JESD204 interface config (%d)\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_jesd_subclass_set(ad9083_h, st->jesd_subclass);
//	if (ret < 0) {
//		printf("Failed to set subclass (%d)\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_jesd_enable_scrambler(ad9083_h, 1);
//	if (ret < 0) {
//		printf("Failed to enable scrambler (%d)\n", ret);
//		goto error;
//	}
//
//	ret = ad9083_jesd_enable_link(ad9083_h, 1);
//	if (ret < 0) {
//		printf("Failed to enabled JESD204 link (%d)\n", ret);
//		goto error;
//	}
//
//	timeout = 10;
//
//	do {
//		mdelay(10);
//		ret = ad9083_jesd_get_pll_status(ad9083_h, &pll_stat);
//		if (ret < 0) {
//			printf("Failed to get pll status (%d)\n", ret);
//			goto error;
//		}
//	} while (!(pll_stat & ad9083_JESD_PLL_LOCK_STAT) && timeout--);
//
//	printf("ad9083 PLL %s\n", (pll_stat & ad9083_JESD_PLL_LOCK_STAT) ?
//	       "LOCKED" : "UNLOCKED");
//
//	return 0;
//
//error:
//	ad9083_deinit(ad9083_h);
//	return ret;
//}


/**
 * Delay microseconds, compatible with ad9083 API
 * Performs a blocking or sleep delay for the specified time in microseconds
 * @param user_data
 * @param us - time to delay/sleep in microseconds.
 */
static int ad9083_udelay(void *user_data, unsigned int us)
{
	udelay(us);

	return 0;
}

int32_t ad9083_reset_pin_ctrl(void *user_data, uint8_t enable)
{
	struct ad9083_phy *phy = user_data;

	return gpio_set_value(phy->gpio_reset, enable);
}

int32_t ad9083_parse_init_param(struct ad9083_phy *phy,
				const struct ad9083_init_param *init_param)
{
	return SUCCESS;
}

static int32_t ad9083_setup(struct ad9083_phy *phy)
{
	return SUCCESS;
}

/**
 * Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 * 		       parameters.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad9083_init(struct ad9083_phy **device, struct ad9083_init_param *init_param)
{
	adi_cms_chip_id_t chip_id;
	struct ad9083_phy *phy;
	uint8_t api_rev[3];
	int32_t ret = 0;

	phy = (struct ad9083_phy *)calloc(1, sizeof(*phy));
	if (!phy)
		return -ENOMEM;

	ret = gpio_get(&phy->gpio_reset, init_param->gpio_reset);
	if (ret < 0)
		goto error_1;

	/* SPI */
	ret = spi_init(&phy->spi_desc, init_param->spi_init);
	if (ret < 0)
		goto error_2;

//	phy->dev_clk = init_param->dev_clk;
//	phy->jesd_rx_clk = init_param->jesd_rx_clk;
//	phy->jesd_tx_clk = init_param->jesd_tx_clk;
//
//	ad9083_parse_init_param(phy, init_param);//todo
//
	phy->ad9083.hal_info.user_data = phy;
	phy->ad9083.hal_info.delay_us = ad9083_udelay;
	phy->ad9083.hal_info.reset_pin_ctrl = ad9083_reset_pin_ctrl;
	phy->ad9083.hal_info.sdo = SPI_SDO;
	phy->ad9083.hal_info.msb = SPI_MSB_FIRST;
	phy->ad9083.hal_info.addr_inc = SPI_ADDR_INC_AUTO;
	phy->ad9083.hal_info.spi_xfer = ad9083_spi_xfer;
	phy->ad9083.hal_info.log_write = ad9083_log_write;
//
//	ret = gpio_direction_output(phy->gpio_reset, 1);
//	if (ret < 0)
//		goto error_3;
//	adi_ad9083_device_reset(&phy->ad9083, AD9083_HARD_RESET_AND_INIT);
//	if (ret < 0) {
//		printf("%s: reset/init failed (%"PRId32")\n", __func__, ret);
//		goto error_3;
//	}
	uint8_t reg_val = 0x0, i = 0;
while (1) {
		for (i = 0; i < 15; i++) {

			adi_ad9083_hal_reg_set(&phy->ad9083, 0x00000B91, i);
			adi_ad9083_hal_reg_get(&phy->ad9083, 0x00000B91, &reg_val);
		}

		ret = adi_ad9083_device_chip_id_get(&phy->ad9083, &chip_id);
		if (ret < 0) {
			printf("%s: chip_id failed (%"PRId32")\n", __func__, ret);
	//		goto error_3;
	}
	else {
		printf("chip_id failed OK\n");
//		goto error_3;
	}
}
	if ((chip_id.prod_id & CHIPID_MASK) != CHIPID_AD9083) {
		printf("%s: Unrecognized CHIP_ID 0x%X\n", __func__,
		       chip_id.prod_id);
		ret = FAILURE;
		goto error_3;
	}

	ret = ad9083_setup(phy); //todo
	if (ret < 0) {
		printf("%s: ad9081_setup failed (%"PRId32")\n", __func__, ret);
		goto error_3;
	}

	adi_ad9083_device_api_revision_get(&phy->ad9083, &api_rev[0],
					   &api_rev[1], &api_rev[2]);

	printf("AD9083 Rev. %u Grade %u (API %u.%u.%u) probed\n",
	       chip_id.dev_revision, chip_id.prod_grade,
	       api_rev[0], api_rev[1], api_rev[2]);

error_3:
	spi_remove(phy->spi_desc);
error_2:
	gpio_remove(phy->gpio_reset);
error_1:
	if (phy)
		free(phy);

	return ret;
}

int32_t ad9083_remove(struct ad9083_phy *dev)
{
	int32_t ret = 0;

//	ret = gpio_remove(device->gpio_powerdown);
//	ret |= spi_remove(device->spi_desc);
//
//	if (device->st->adc_h)
//		free(device->st->adc_h);
//	if (device->st)
//		free(device->st);
	if (dev)
		free(dev);

	return ret;
}
