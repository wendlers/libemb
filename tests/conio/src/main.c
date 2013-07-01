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

#ifdef MSP430
#include <msp430.h>
#else
#include <libopencm3/stm32/f1/rcc.h>
#endif

#include "serial.h"
#include "conio.h"

void clock_init(void)
{
#ifdef MSP430
    WDTCTL = WDTPW + WDTHOLD;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL  = CALDCO_16MHZ;
	// 1MHz
    // BCSCTL1 = CALBC1_1MHZ;
    // DCOCTL  = CALDCO_1MHZ;
#else
#ifdef STM32_100
	rcc_clock_setup_in_hse_8mhz_out_24mhz();
#else
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
#endif
#endif
}

int main(void)
{
	clock_init();

#ifdef MSP430
	// 16Mhz
	serial_clk_init(16000000L, 9600);
	// 1MHz
	// serial_init(9600);
#else
	serial_init(9600);
#endif

	cio_print("conio\n\r");

	// Test simple print operations

	cio_print("printi\n\r");
	cio_print("'0': ");
	cio_printi(0);
	cio_print("\n\r");

#ifdef MSP430
	cio_print("'12345': ");
	cio_printi(12345);
#else
	cio_print("'1234567890': ");
	cio_printi(1234567890);
#endif
	cio_print("\n\r");

	cio_print("printb\n\r");
	cio_print("(8) '0' (00000000): ");
	cio_printb(0, 8);
	cio_print("\n\r");

	cio_print("(8) '1' (00000001): ");
	cio_printb(1, 8);
	cio_print("\n\r");

	cio_print("(8) '42' (00101010): ");
	cio_printb(42, 8);
	cio_print("\n\r");

	cio_print("(8) '151' (10010111): ");
	cio_printb(151, 8);
	cio_print("\n\r");

	cio_print("(8) '255' (11111111): ");
	cio_printb(255, 8);
	cio_print("\n\r");

	cio_print("(16) '255' (0000000011111111): ");
	cio_printb(255, 16);
	cio_print("\n\r");

	cio_print("(4) '151' (0111): ");
	cio_printb(151, 4);
	cio_print("\n\r");

	// Test printf operation
	//
    char*			s;
    char 			c;
    int 			i;
    unsigned int 	u;
    long int 		l;
    long unsigned 	n;
    unsigned int 	x;

    s = "test";
    c = 'X';
    i = -12345;
    u =  12345;
    l = -1234567890;
    n =  1234567890;
    x =  0xABCD;

	cio_printf("printf\n\r");

    cio_printf("s %s\n\r", s);
    cio_printf("c %c\n\r", c);
    cio_printf("i %i\n\r", i);
    cio_printf("u %u\n\r", u);
    cio_printf("l %l\n\r", l);
    cio_printf("n %n\n\r", n);
    cio_printf("x %x\n\r", x);

    cio_printf("all %s %c %i %u %l %n %x\n\r", s, c, i, u, l, n, x);

	cio_printf("DONE\n\r");


	while (1) {
		__asm__("nop");
	}
	return 0;
}
