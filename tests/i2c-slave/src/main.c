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
#define CMD_UNKNOWN 0xFF

/* Parameters */
#define PAR_LEDOFF  0x00
#define PAR_LEDON   0x01
#define PAR_UNKNOWN 0xFF

/* Responses */
#define RES_ERROR   0xFF

/* last command */
unsigned char cmd = CMD_UNKNOWN;

/* last parameter */
unsigned char par = PAR_UNKNOWN;

/* response to send out on read req. */
unsigned char res = RES_ERROR;

void process_cmd(unsigned char cmd, unsigned char par)
{
    res = RES_ERROR;

    switch(cmd) {
    case CMD_SETLED:
#ifdef MSP430
        if(par == PAR_LEDON) {
            P1OUT |= BIT0;
        } else if(par == PAR_LEDOFF) {
            P1OUT &= ~BIT0;
        }
#endif
        break;
    case CMD_GETBTN:
#ifdef MSP430
        if((P1IN & BIT3)) {
            res = 0x01;
        } else {
            res = 0x00;
        }
#endif
        break;
    }
}

void start_cb()
{
    cmd = CMD_UNKNOWN;
    par = PAR_UNKNOWN;
}

void receive_cb(unsigned char receive)
{
    if(cmd == CMD_UNKNOWN) {

        cmd = receive;

        if(cmd == CMD_GETBTN) {
            process_cmd(cmd, PAR_UNKNOWN);
        }
    } else {
        par = receive;
        process_cmd(cmd, par);
    }
}

void transmit_cb(unsigned char volatile *byte)
{
    *byte = res;
}

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

static i2c_cb callbacks = {
	.receive  = receive_cb,
	.transmit = transmit_cb,
	.start    = start_cb,
};

int main(void)
{
	clock_init();
	gpio_init();
	i2cslave_init(I2C_ADDR, &callbacks);
	
	while (1) {
		__asm__("nop");
	}

	return 0;
}
