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

#define SCLK    BIT5
#define SDI     BIT7
#define SDO     BIT6
#define CS      BIT4

void nrf_init(void)
{
    P1DIR |= SCLK | SDO | CS;
    P1DIR &= ~SDI;

    // enable SDI, SDO, SCLK, master mode, MSB, output enabled, hold in reset
    USICTL0 = USIPE7 | USIPE6 | USIPE5 | USIMST | USIOE | USISWRST;

    // SMCLK / 128
    USICKCTL = USIDIV_7 + USISSEL_2;

    // clock phase
    USICTL1 |= USICKPH;

    // release from reset
    USICTL0 &= ~USISWRST;

    nrf_spi_csh();
}

void nrf_spi_csh(void)
{
    // deassert CS
    P1OUT |= CS;
}

void nrf_spi_csl(void)
{
    // assert CS
    P1OUT &= ~CS;
}

unsigned char nrf_spi_xfer_byte(unsigned char data)
{
    USISRL = data;

    // clear interrupt flag
    USICTL1 &= ~USIIFG;
    // set number of bits to send, begins tx
    USICNT=8;

    // wait for tx
    while((USICTL1 & USIIFG) != 0x01);

    return (USISRL);
}

