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

static int i2cslave_cmdproc_last_cmd;

static i2c_cmd_args i2cslave_cmdproc_last_args;

static i2c_cmd_res i2cslave_cmdproc_res;

static void i2cslave_cmdproc_receive_cb(unsigned char data);

static void i2cslave_cmdproc_transmit_cb(unsigned char volatile *data);

static void i2cslave_cmdproc_start_cb();

static i2c_cb i2cslave_cmdproc_cbs = {
	.receive  = i2cslave_cmdproc_receive_cb,
	.transmit = i2cslave_cmdproc_transmit_cb,
	.start    = i2cslave_cmdproc_start_cb,
};

static i2c_cmds *i2cslave_cmdproc_cmds;

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

void i2cslave_cmdproc_init(unsigned int addr, i2c_cmds *cmds) 
{
	i2cslave_cmdproc_cmds = cmds;

	i2cslave_init(addr, &i2cslave_cmdproc_cbs); 
}

void i2cslave_cmdproc_clrres() 
{
	int i;

	i2cslave_cmdproc_res.count = 0;
	i2cslave_cmdproc_res.xmit_count = 0;

	for(i = 0; i < I2C_MAX_RES; i++) {
		i2cslave_cmdproc_res.data[i] = 0;
	}
}

int i2cslave_cmdproc_addres(unsigned char data) 
{
	if(i2cslave_cmdproc_res.count < I2C_MAX_RES) {
		i2cslave_cmdproc_res.data[i2cslave_cmdproc_res.count++] = data;	
		return 0;
	}

	return -1;
}

static void i2cslave_cmdproc_receive_cb(unsigned char data)
{
	int i;

	if(i2cslave_cmdproc_last_cmd == -1) {
		// not yet received command, see if data is known command
		for(i = 0; i < i2cslave_cmdproc_cmds->count; i++) {
			if(data == i2cslave_cmdproc_cmds->cmds[i].cmd) {
				i2cslave_cmdproc_last_cmd = i;
				if(i2cslave_cmdproc_cmds->cmds[i2cslave_cmdproc_last_cmd].args == 0) {
					i2cslave_cmdproc_cmds->cmds[i2cslave_cmdproc_last_cmd].func(&i2cslave_cmdproc_last_args);
				}
				break;
			}
		}
	}
	else {
		// already received command, see if data needs to be added to params
		if(i2cslave_cmdproc_last_args.count < i2cslave_cmdproc_cmds->cmds[i2cslave_cmdproc_last_cmd].args) {
			i2cslave_cmdproc_last_args.args[i2cslave_cmdproc_last_args.count++] = data;

			if(i2cslave_cmdproc_last_args.count == i2cslave_cmdproc_cmds->cmds[i2cslave_cmdproc_last_cmd].args) {
				i2cslave_cmdproc_cmds->cmds[i2cslave_cmdproc_last_cmd].func(&i2cslave_cmdproc_last_args);
			}
		}
	}
}

static void i2cslave_cmdproc_transmit_cb(unsigned char volatile *data)
{
	if(i2cslave_cmdproc_res.xmit_count < i2cslave_cmdproc_res.count) {
		*data = i2cslave_cmdproc_res.data[i2cslave_cmdproc_res.xmit_count++];
	}
	else {
		*data = 0xff;
	}
}

static void i2cslave_cmdproc_start_cb()
{
	int i; 

	i2cslave_cmdproc_last_cmd = -1;
	i2cslave_cmdproc_last_args.count = 0;

	for(i = 0; i < I2C_MAX_ARGS; i++) {
		i2cslave_cmdproc_last_args.args[i] = 0;
	}
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
