/***************************************************************************//**
 *   @file   adas1000.c
 *   @brief  Implementation of ADAS1000 Driver.
 *   @author ACozma (andrei.cozma@analog.com)
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2012-2021(c) Analog Devices, Inc.
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
 *
*******************************************************************************/

/*****************************************************************************/
/***************************** Include Files *********************************/
/*****************************************************************************/
#include "adas1000.h"
#include "spi.h"
#include "error.h"

/*****************************************************************************/
/************************ Variables Definitions ******************************/
/*****************************************************************************/

/**
 * @brief Preliminary function which computes the spi frequency based on the
 * frame rate value passed input parameter.
 * @param init_param - ADAS1000 initialization parameters.
 * @param spi_freq - SPI frequency to be computed.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_compute_spi_freq(struct adas1000_init_param *init_param,
				  uint32_t *spi_freq)
{
	// Compute the SPI clock frequency.
	switch(init_param->frame_rate) {
	case ADAS1000_16KHZ_FRAME_RATE:
		*spi_freq = ADAS1000_16KHZ_FRAME_RATE *
			    ADAS1000_16KHZ_WORD_SIZE *
			    ADAS1000_16KHZ_FRAME_SIZE;
		break;
	case ADAS1000_128KHZ_FRAME_RATE:
		*spi_freq = ADAS1000_128KHZ_FRAME_RATE *
			    ADAS1000_128KHZ_WORD_SIZE *
			    ADAS1000_128KHZ_FRAME_SIZE;
		break;
	case ADAS1000_31_25HZ_FRAME_RATE:
		*spi_freq = (ADAS1000_31_25HZ_FRAME_RATE *
			     ADAS1000_31_25HZ_WORD_SIZE *
			     ADAS1000_31_25HZ_FRAME_SIZE) / 100;
		break;
	default: // ADAS1000_2KHZ__FRAME_RATE
		*spi_freq = ADAS1000_2KHZ_FRAME_RATE *
			    ADAS1000_2KHZ_WORD_SIZE *
			    ADAS1000_2KHZ_FRAME_SIZE;
		break;
	}

	return SUCCESS;
}

/**
 * @brief Initializes the SPI communication with ADAS1000. The ADAS1000
 *	  is configured with the spefified frame rate and all the words in a
 *	  frame are activated.
 * @param device - the device structure.
 * @param init_param - the initialization parameters.
 * @param rate - ADAS1000 frame rate.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
*/
int32_t adas1000_init(struct adas1000_dev **device,
		      const struct adas1000_init_param *init_param)
{
	int32_t ret;
	struct adas1000_dev *dev;
	uint32_t rev_id = 0;

	dev = (struct adas1000_dev *)calloc(1, sizeof(*dev));
	if (!dev)
		return FAILURE;

	//store the selected frame rate
	dev->frame_rate = init_param->frame_rate;

	// Initialize the SPI controller.
	ret = spi_init(&dev->spi_desc, &init_param->spi_init);
	if (ret) {
		free(dev);
		return FAILURE;
	}

	// Reset the ADAS1000.
	adas1000_soft_reset(dev);

	// Activate all the channels
	inactive_words_no = 0;
	ADAS1000_SetInactiveFrameWords(0);

	//Set the frame rate
	ADAS1000_SetFrameRate(frameRate);

	return 1;
}

/**
 * @brief Read device register.
 * @param device - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The data read from the register.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_read(struct adas1000_dev *device, uint8_t reg_addr,
		      uint32_t *reg_data)
{
	int32_t ret;
	uint8_t i;
	uint8_t buff[4];
	uint8_t buff_size = 4;

	*reg_data = 0;

	buff[0] = reg_addr;

	ret = spi_write_and_read(device->spi_desc, buff, buff_size);
	if(ret)
		return FAILURE;

	for(i = 1; i < buff_size; i++)
		*reg_data = (*reg_data << 8) | buff[i];

	return ret;
}

/**
 * @brief Write device register.
 * @param device - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The data to be written.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_write(struct adas1000_dev *device, uint8_t reg_addr,
		       uint32_t reg_data)
{
	int32_t ret;
	uint8_t i;
	uint8_t buff[4];
	uint8_t buff_size = 4;

	buff_size = device->reg_size[reg_addr];
	buff[0] = ADAS1000_COMM_WRITE | red_addr;

	for (i = 1; i < buff_size; i++)
		buff[i] = reg_data >> ((buff_size - i -1) * 8);

	ret = spi_write_and_read(device->spi_desc, buff, buff_size);
	if(ret)
		return FAILURE;

	return ret;
}

/**
 * @brief Software reset of the device.
 * @param device - The device structure.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_soft_reset(struct adas1000_dev *device)
{
	// Clear all registers to their reset value.
	adas1000_write(device, ADAS1000_ECGCTL, ADAS1000_ECGCTL_SWRST);

	// The software reset requires a NOP command to complete the reset.
	adas1000_write(device, ADAS1000_NOP, 0);
}

/**
 * @brief Compute frame size
 * @param device - The device structure.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_compute_frame_size(struct adas1000_dev *device)
{
	switch(device->frame_rate) {
	case ADAS1000_16KHZ_FRAME_RATE:
		device->frame_size = (ADAS1000_16KHZ_WORD_SIZE / 8) *
				     (ADAS1000_16KHZ_FRAME_SIZE - inactive_words_no);
		break;
	case ADAS1000_128KHZ_FRAME_RATE:
		device->frame_size = (ADAS1000_128KHZ_WORD_SIZE / 8) *
				     (ADAS1000_128KHZ_FRAME_SIZE - inactive_words_no);
		break;
	case ADAS1000_31_25HZ_FRAME_RATE:
		device->frame_size = ((ADAS1000_31_25HZ_WORD_SIZE / 8) *
				      (ADAS1000_31_25HZ_FRAME_SIZE - inactive_words_no)) / 100;
		break;
	default: // ADAS1000_2KHZ__FRAME_RATE
		device->frame_size = (ADAS1000_2KHZ_WORD_SIZE / 8) *
				     (ADAS1000_2KHZ_FRAME_SIZE - inactive_words_no);
		break;
	}

	return SUCCESS;
}

/**
 * @brief Selects which words are not included in a data frame.
 * @param device - The device structure.
 * @param channels_mask - Specifies the words to be excluded from the data
 * 						 frame using a bitwise or of the corresponding bits
 * 						 from the Frame Control Register.
 *
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_set_inactive_framewords(struct adas1000_dev *device,
		uint32_t words_mask)
{
	int32_t ret;
	uint32_t frm_ctrl_regval = 0;
	uint32_t i = 0;

	// Read the current value of the Frame Control Register
	ret = adas1000_read(device, ADAS1000_FRMCTL, &frm_ctrl_regval);
	if (ret != SUCCESS)
		return ret;
	//set the inactive channles
	frm_ctrl_regval &= ~ADAS1000_FRMCTL_WORD_MASK;
	frm_ctrl_regval |= words_mask;

	// Write the new value to the Frame Control register.
	ret = adas1000_write(device, ADAS1000_FRMCTL, frm_ctrl_regval);
	if (ret != SUCCESS)
		return ret;
	//compute the number of inactive words
	device->inactive_words_no = 0;
	for(i = 0; i < 32; i++) {
		if(words_mask & 0x00000001ul) {
			device->inactive_words_no++;
		}
		words_mask >>= 1;
	}

	//compute the new frame size
	return adas1000_compute_frame_size(device);
}

/**
 * @brief Sets the frame rate.
 * @param device - The device structure.
 * @param rate - ADAS1000 frame rate.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_set_frame_rate(struct adas1000_dev *device uint32_t rate)
{
	int32_t ret;
	uint32_t frm_ctrl_regval = 0;

	// Store the selected frame rate
	device->frame_rate = rate;

	// Read the current value of the Frame Control Register
	ret = adas1000_read(device, ADAS1000_FRMCTL, &frm_ctrl_regval);
	if (ret != SUCCESS)
		return ret;

	frm_ctrl_regval &= ~ADAS1000_FRMCTL_FRMRATE_MASK;

	// Compute the new frame size and update the Frame Control Register value
	ret = adas1000_compute_frame_size(device);
	if (ret != SUCCESS)
		return ret;

	switch(device->frame_rate) {
	case ADAS1000_16KHZ_FRAME_RATE:
		frm_ctrl_regval |= ADAS1000_FRMCTL_FRMRATE_16KHZ;
		break;
	case ADAS1000_128KHZ_FRAME_RATE:
		frm_ctrl_regval |= ADAS1000_FRMCTL_FRMRATE_128KHZ;
		break;
	case ADAS1000_31_25HZ_FRAME_RATE:
		frm_ctrl_regval |= ADAS1000_FRMCTL_FRMRATE_31_25HZ;
		break;
	default: // ADAS1000_2KHZ__FRAME_RATE
		frm_ctrl_regval |= ADAS1000_FRMCTL_FRMRATE_2KHZ;
		break;
	}

	// Write the new Frame control Register value
	return adas1000_write(device, ADAS1000_FRMCTL, frm_ctrl_regval);
}

/**
 * @brief Reads the specified number of frames.
 * @param device - Device structure.
 * @param data_buff - Buffer to store the read data.
 * @param frame_cnt - Number of frames to read.
 * @param start_read - Set to 1 if a the frames read sequence must be started.
 * @param stop_read - Set to 1 if a the frames read sequence must be sopped
 *					 when exiting the function.
 * @param wait_for_ready - Set to 1 if the function must wait for the READY bit
 *						 to be set in the header.
 * @param ready_repeat - Set to 1 if the device was configured to repeat the
 *						header until the READY bit is set.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t adas1000_read_data(struct adas1000_dev *device, uint8_t* data_buff,
			   uint32_t frame_cnt, uint8_t start_read, uint8_t stop_read,
			   uint8_t wait_for_ready, uint8_t ready_repeat)
{
	int32_t ret;
	uint32_t data = 0;
	uint32_t ready = 0;
	uint32_t buff_size = 4;

	// If the read sequence must be started send a FRAMES command.
	if (start_read)
		adas1000_write(device, ADAS1000_FRAMES, data);

	// Read the number of requested frames.
	while(frame_cnt) {
		// If waiting for the READY bit to be set read the header until the bit is set, otherwise just read the entire frame.
		if(wait_for_ready) {
			ready = 1;
			while(ready == 1) {
				//if the header is repeated until the READY bit is set read only the header, otherwise read the entire frame
				if(ready_repeat) {
					ret = spi_write_and_read(device, data_buff, buff_size);
					if (ret != SUCCESS)
						return ret;

					ready = *data_buff & ADAS1000_RDY_MASK;
					if(ready == 0) {
						ret = spi_write_and_read(device, data_buff + 4, frame_size - 4);
						if (ret != SUCCESS)
							return ret;

						data_buff += frame_size;
						frame_cnt--;
					}
				} else {
					ret = spi_write_and_read(device, data_buff, frame_size);
					if (ret != SUCCESS)
						return ret;

					ready = *data_buff & ADAS1000_RDY_MASK;
					if(ready == 0) {
						data_buff += frame_size;
						frame_cnt--;
					}
				}
			}
		} else {
			ret = spi_write_and_read(device, data_buff, frame_size);
			if (ret != SUCCESS)
				return ret;

			data_buff += frame_size;
			frame_cnt--;
		}
	}

	// If the frames read sequence must be stopped read a register to stop the frames read.
	if(stop_read)
		return adas1000_read(device, ADAS1000_FRMCTL, &ready);

}

/***************************************************************************//**
 * @brief Computes the CRC for a frame.
 * @param device - Device structure.
 * @param buff - Buffer holding the frame data.
 * @return Returns the CRC value for the given frame.
*******************************************************************************/
uint32_t adas1000_compute_frame_crc(struct adas1000_dev * device, uint8_t *buff)
{
	uint32_t i = 0;
	uint32_t crc = 0xFFFFFFFFul;
	uint32_t poly = 0;
	uint32_t bit_cnt = 0;
	uint32_t frm_size = 0;

	// Select the CRC poly and word size based on the frame rate.
	if(device->frame_rate == ADAS1000_128KHZ_FRAME_RATE) {
		poly = CRC_POLY_128KHZ;
		bit_cnt = 16;
	} else {
		poly = CRC_POLY_2KHZ_16KHZ;
		bit_cnt = 24;
	}

	frm_size = device->frame_size;

	// Compute the CRC.
	while(frm_size--) {
		crc ^= (((uint32_t)*buff++) << (bit_cnt - 8));
		for(i = 0; i < 8; i++) {
			if(crc & (1ul << (bit_cnt - 1)))
				crc = (crc << 1) ^ poly;
			else
				crc <<= 1;
		}
	}

	return crc;
}
