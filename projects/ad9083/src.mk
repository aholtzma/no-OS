################################################################################
#									       #
#     Shared variables:							       #
#	- PROJECT							       #
#	- DRIVERS							       #
#	- INCLUDE							       #
#	- PLATFORM_DRIVERS						       #
#	- NO-OS								       #
#									       #
################################################################################

ifeq (y,$(strip $(TINYIIOD)))
SRCS += $(PROJECT)/src/app_iio.c
LIBRARIES += iio
endif

SRC_DIRS += $(PROJECT)/src/app
#SRC_DIRS += $(DRIVERS)/adc/ad9083/api_01
SRC_DIRS += $(DRIVERS)/adc/ad9083/api_02

SRCS += $(DRIVERS)/spi/spi.c						\
	$(DRIVERS)/gpio/gpio.c						\
	$(DRIVERS)/frequency/ad9528/ad9528.c				\
	$(DRIVERS)/adc/ad9083/ad9083.c
SRCS += $(DRIVERS)/axi_core/axi_adc_core/axi_adc_core.c			\
	$(DRIVERS)/axi_core/axi_dmac/axi_dmac.c				\
	$(DRIVERS)/axi_core/clk_axi_clkgen/clk_axi_clkgen.c		\
	$(DRIVERS)/axi_core/jesd204/axi_adxcvr.c			\
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_rx.c			\
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_tx.c			\
	$(DRIVERS)/axi_core/jesd204/xilinx_transceiver.c		\
	$(NO-OS)/util/clk.c						\
	$(NO-OS)/util/util.c
SRCS +=	$(PLATFORM_DRIVERS)/axi_io.c					\
	$(PLATFORM_DRIVERS)/xilinx_spi.c				\
	$(PLATFORM_DRIVERS)/xilinx_gpio.c				\
	$(PLATFORM_DRIVERS)/delay.c
ifeq (y,$(strip $(TINYIIOD)))
SRCS += $(NO-OS)/util/xml.c						\
	$(NO-OS)/util/fifo.c						\
	$(NO-OS)/iio/iio_axi_adc/iio_axi_adc.c				\
	$(NO-OS)/util/list.c						\
	$(PLATFORM_DRIVERS)/uart.c					\
	$(PLATFORM_DRIVERS)/irq.c
endif
INCS += $(PROJECT)/src/parameters.h
ifeq (y,$(strip $(TINYIIOD)))
INCS +=	$(PROJECT)/src/app_iio.h
endif
INCS += $(DRIVERS)/frequency/ad9528/ad9528.h				\
	$(DRIVERS)/adc/ad9083/ad9083.h
INCS += $(DRIVERS)/axi_core/axi_adc_core/axi_adc_core.h			\
	$(DRIVERS)/axi_core/axi_dmac/axi_dmac.h				\
	$(DRIVERS)/axi_core/clk_axi_clkgen/clk_axi_clkgen.h		\
	$(DRIVERS)/axi_core/jesd204/axi_adxcvr.h			\
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_rx.h			\
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_tx.h			\
	$(DRIVERS)/axi_core/jesd204/xilinx_transceiver.h
INCS +=	$(PLATFORM_DRIVERS)/spi_extra.h					\
	$(PLATFORM_DRIVERS)/gpio_extra.h
INCS +=	$(INCLUDE)/axi_io.h						\
	$(INCLUDE)/spi.h						\
	$(INCLUDE)/gpio.h						\
	$(INCLUDE)/error.h						\
	$(INCLUDE)/delay.h						\
	$(INCLUDE)/clk.h						\
	$(INCLUDE)/util.h
ifeq (y,$(strip $(TINYIIOD)))
INCS += $(INCLUDE)/xml.h						\
	$(INCLUDE)/fifo.h						\
	$(INCLUDE)/irq.h						\
	$(INCLUDE)/uart.h						\
	$(INCLUDE)/list.h						\
	$(PLATFORM_DRIVERS)/irq_extra.h					\
	$(PLATFORM_DRIVERS)/uart_extra.h                                \
	$(NO-OS)/iio/iio_axi_adc/iio_axi_adc.h
endif
