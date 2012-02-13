/*
 * This file is part of the libemb project.
 *
 * Copyright (C) 2011 Stefan Wendler <sw@kaltpost.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>

#include "nrf24l01_hw.h"

#define SPI_CS_HIGH		gpio_set(GPIOB, GPIO_SPI2_NSS)
#define SPI_CS_LOW		gpio_clear(GPIOB, GPIO_SPI2_NSS)

void nrf_init(void)
{
     rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
     rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);

     /* Configure SCK and MOSI */
     gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                   GPIO_SPI2_SCK | GPIO_SPI2_MOSI);

     /* Configure MISO */
     gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT,
                   GPIO_SPI2_MISO);

     /* Configure CS pin on PB12. */
     gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_NSS);

     spi_set_unidirectional_mode(SPI2); 		/* We want to send only. */
     spi_set_dff_8bit(SPI2);
     spi_set_full_duplex_mode(SPI2);
     spi_send_msb_first(SPI2);

     /* Handle the CS signal in software. */
     spi_enable_software_slave_management(SPI2);
     spi_set_nss_high(SPI2);

     /* PCLOCK/8 as clock. */
     spi_set_baudrate_prescaler(SPI2, SPI_CR1_BR_FPCLK_DIV_8);

     /* We want to control everything and generate the clock -> master. */
     spi_set_master_mode(SPI2);
     spi_set_clock_polarity_0(SPI2); /* SCK idle state low. */

     /* Bit is taken on the second (falling edge) of SCK. */
     spi_set_clock_phase_0(SPI2);
     spi_enable_ss_output(SPI2);

     SPI_CS_HIGH;

     spi_enable(SPI2);
}

void nrf_spi_csh(void)
{
     SPI_CS_HIGH;
}

void nrf_spi_csl(void)
{
     SPI_CS_LOW;
}

unsigned char nrf_spi_xfer_byte(unsigned char data)
{
     return spi_xfer(SPI2, data);
}

