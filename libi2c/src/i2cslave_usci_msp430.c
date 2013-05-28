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
#include <legacymsp430.h>

#include "i2c.h"

/**
 * SCL pin
 */
#define I2C_SCL   		BIT6

/**
 * SDA pin
 */
#define I2C_SDA   		BIT7

static i2c_cb *i2c_callbacks;

void i2cslave_init(unsigned int addr, i2c_cb *callbacks)
{
     i2c_callbacks = callbacks;

     P1SEL 		|= I2C_SDA + I2C_SCL;
     P1SEL2 	|= I2C_SDA + I2C_SCL;
     UCB0CTL1 	|= UCSWRST;
     UCB0CTL0 	 = UCMODE_3 + UCSYNC;
     UCB0I2COA 	 = addr;
     UCB0CTL1 	&= ~UCSWRST;
     IE2 		|= UCB0TXIE + UCB0RXIE;
     UCB0I2CIE 	|= UCSTTIE;
}

interrupt(USCIAB0TX_VECTOR) i2c_data_interrupt(void)
{
     if (IFG2 & UCB0TXIFG) {
          i2c_callbacks->transmit(&UCB0TXBUF);
     } else {
          i2c_callbacks->receive(UCB0RXBUF);
     }
}

interrupt(USCIAB0RX_VECTOR) i2c_state_interrupt(void)
{
     UCB0STAT &= ~UCSTTIFG;
     i2c_callbacks->start();
}
