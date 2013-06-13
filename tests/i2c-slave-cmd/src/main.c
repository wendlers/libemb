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

/**
 * This firmware acts as an I2C slave on address 0x90(W)/0x91(R).
 * It receives a command / parameter pair from the I2C master and
 * sends out a response on a masters read request.
 *
 * The firmware knows two commands:
 *
 * CMD_SETLED: sets the build in LED (P1.0) of the launchpad depending
 * on the given parameter to HIGH (0x01) or LOW (0x00).
 *
 * CMD_GETBTN: copies the state of the build in push-button (P1.3) to
 * the response buffer and transmits it to the master on the next read
 * request.
 *
 * As a I2C master, the bus pirate could be used. To set the BP into
 * I2C mode use:
 *
 *    m4 3
 *
 * Bus-Piret I2C commands to
 *
 * - set LED HIGH (P1.0)
 *    [0x90 0x00 0x01
 *
 * - set LED LOW  (P1.0)
 *    [0x90 0x00 0x00
 *
 * - get BUTTON state (P1.3)
 *    [0x90 0x01 [0x91 r
 *
 * NOTE: 100k extrnal pull-ups are needed on SDA/SDC.
 */

#ifdef MSP430
#include <msp430.h>
#else
#include <libopencm3/stm32/f1/rcc.h>
#endif

#include "i2c.h"

/* I2C slave address (7-bit) */
#define I2C_ADDR	0x48

/* Commands */
#define CMD_SETLED  0x00
#define CMD_GETBTN  0x01
#define CMD_GETINF  0x02
#define CMD_ECHO3   0x03

/* Parameters */
#define PAR_LEDOFF  0x00
#define PAR_LEDON   0x01
#define PAR_INFVER	0x00
#define PAR_INFID	0x01

void cmd_setled(i2c_cmd_args *args) 
{
#ifdef MSP430
	if(args->args[0] == PAR_LEDON) {
    	P1OUT |= BIT0;
	}
	else if(args->args[0] == PAR_LEDOFF) {
        P1OUT &= ~BIT0;
	}
#endif
}

void cmd_getbtn(i2c_cmd_args *args) 
{
	i2cslave_cmdproc_clrres();

#ifdef MSP430
	if((P1IN & BIT3)) {
		i2cslave_cmdproc_addres(0x01);
    } else {
		i2cslave_cmdproc_addres(0x00);
    }
#endif
}

void cmd_getinf(i2c_cmd_args *args) 
{
	i2cslave_cmdproc_clrres();

	if(args->args[0] == PAR_INFVER) {
		i2cslave_cmdproc_addres(0x00);
		i2cslave_cmdproc_addres(0x01);
	}
	else if(args->args[0] == PAR_INFID) {
		i2cslave_cmdproc_addres('L');
		i2cslave_cmdproc_addres('I');
		i2cslave_cmdproc_addres('B');
		i2cslave_cmdproc_addres('E');
		i2cslave_cmdproc_addres('M');
		i2cslave_cmdproc_addres('B');
	}
}

void cmd_echo3(i2c_cmd_args *args) 
{
	i2cslave_cmdproc_clrres();

	i2cslave_cmdproc_addres(0xA0 + args->args[0]);
	i2cslave_cmdproc_addres(0xA0 + args->args[1]);
	i2cslave_cmdproc_addres(0xA0 + args->args[2]);
}

static i2c_cmds cmds = {
     .count = 4,
     .cmds	= {
          {
               .cmd		= CMD_SETLED,
			   .args	= 1,
               .func 	= cmd_setled,
          },
          {
               .cmd		= CMD_GETBTN,
			   .args	= 0,
               .func 	= cmd_getbtn,
          },
          {
               .cmd		= CMD_GETINF,
			   .args	= 1,
               .func 	= cmd_getinf,
          },
          {
               .cmd		= CMD_ECHO3,
			   .args	= 3,
               .func 	= cmd_echo3,
          },
	},
};

void clock_init(void)
{
#ifdef MSP430
    WDTCTL = WDTPW + WDTHOLD;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL  = CALDCO_16MHZ;

    __bis_SR_register(GIE);
#else
#ifdef STM32_100
	rcc_clock_setup_in_hse_8mhz_out_24mhz();
#else
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
#endif
#endif
}

void gpio_init(void)
{
#ifdef MSP430
    P1DIR |= BIT0; 	// Set P1.0 to output direction
    P1DIR &= ~BIT3; // Set P1.3 to input  direction
    P1OUT &= ~BIT0;
#endif
}

int main(void)
{
	clock_init();
	gpio_init();
	i2cslave_cmdproc_init(I2C_ADDR, &cmds);
	
	while (1) {
		__asm__("nop");
	}

	return 0;
}
