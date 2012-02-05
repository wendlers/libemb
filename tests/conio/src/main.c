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

#include "serial.h"
#include "conio.h"

void clock_init(void)
{
#ifdef STM32_100
	rcc_clock_setup_in_hse_8mhz_out_24mhz();
#else
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
#endif
}

void delay(unsigned long n)
{
	unsigned long i;

	while(n--) {
		i = 2;
		while(i--) __asm__("nop");
	}
}

int main(void)
{
	clock_init();
	serial_init(38400);

	cio_print("nRF2401 v0.1, ConioTest\n\r\n\r");

	// Test simple print operations
	//

	cio_print("---- TEST printi ----\n\r");
	cio_print("Print int '0': ");
	cio_printi(0);
	cio_print("\n\r");

	cio_print("Print int '1234567890': ");
	cio_printi(1234567890);
	cio_print("\n\r");

	cio_print("---- TEST printb ----\n\r");
	cio_print("Print intb(8) '0' (00000000): ");
	cio_printb(0, 8);
	cio_print("\n\r");

	cio_print("Print intb(8) '1' (00000001): ");
	cio_printb(1, 8);
	cio_print("\n\r");

	cio_print("Print intb(8) '42' (00101010): ");
	cio_printb(42, 8);
	cio_print("\n\r");

	cio_print("Print intb(8) '151' (10010111): ");
	cio_printb(151, 8);
	cio_print("\n\r");

	cio_print("Print intb(8) '255' (11111111): ");
	cio_printb(255, 8);
	cio_print("\n\r");

	cio_print("Print intb(16) '255' (0000000011111111): ");
	cio_printb(255, 16);
	cio_print("\n\r");

	cio_print("Print intb(4) '151' (0111): ");
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
    x = 0xABCD;

	cio_printf("---- TEST printf ----\n\r");

    cio_printf("String        %s\n\r", s);
    cio_printf("Char          %c\n\r", c);
    cio_printf("Integer       %i\n\r", i);
    cio_printf("Unsigned      %u\n\r", u);
    cio_printf("Long          %l\n\r", l);
    cio_printf("uNsigned loNg %n\n\r", n);
    cio_printf("heX           %x\n\r", x);

    cio_printf("multiple args %s %c %i %u %l %n %x\n\r", s, c, i, u, l, n, x);

	cio_printf("---- DONE ----\n\r");

	while (1) {
		__asm__("nop");
	}

	return 0;
}
