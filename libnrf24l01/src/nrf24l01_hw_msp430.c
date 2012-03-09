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

#include <msp430.h>

#include "nrf24l01_hw.h"

/**
 * CS (Chip Select) at P1.4
 */
#define CS      BIT4

/**
 * SPI Clock at P1.5
 */
#define SCLK    BIT5

/**
 * SPI SOMI (Slave Out, Master In) at P1.6
 */
#define SOMI    BIT6

/**
 * SPI SIMO (Slave In, Master Out) at P1.7
 */
#define SIMO    BIT7

void nrf_init(void)
{
	UCB0CTL1 = UCSWRST;

    P1DIR  |= CS; 
    P1OUT  |= CS;
  	P1SEL  |= SOMI + SIMO + SCLK;
  	P1SEL2 |= SOMI + SIMO + SCLK;

    // 3-pin, 8-bit SPI master
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; 
	UCB0CTL1 |= UCSSEL_2;   // SMCLK

	UCB0CTL1 &= ~UCSWRST; 

    nrf_spi_csh();
}

void nrf_spi_csh(void)
{
    P1OUT |= CS;
}

void nrf_spi_csl(void)
{
    P1OUT &= ~CS;
}

unsigned char nrf_spi_xfer_byte(unsigned char data)
{
    UCB0TXBUF = data; 

	// wait for TX 
	while (!(IFG2 & UCB0TXIFG)); 	

	return UCB0RXBUF;
}

