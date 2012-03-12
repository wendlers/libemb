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
#include "nrf24l01.h"

/**
 * Define delay factor based on target architecture
 */
#ifdef MSP430
#define DF 1
#else
#define DF 10
#endif

void clock_init(void)
{
#ifdef MSP430
    WDTCTL = WDTPW + WDTHOLD;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;
#else
#ifdef STM32_100
	rcc_clock_setup_in_hse_8mhz_out_24mhz();
#else
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
#endif
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

void nrf_dump_regs(nrf_regs *r) {

	int i;
	int j;

	cio_print("\n\r** START nRF2401 Register DUMP **\n\r");

	nrf_reg_buf buf;

	for(i = 0; i < r->count; i++) {

		nrf_read_reg(i, &buf);

		if(r->data[i].size == 0) continue;

		cio_printf("%s: ", r->data[i].name);

		for(j = 0; j < buf.size; j++) {
			cio_printb(buf.data[j], 8);
			cio_printf(" (%u) ", buf.data[j]);
		}

		cio_print("\n\r - ");

		for(j = 0; j < r->data[i].fields->count; j++) {
			cio_printf("%u[%u]:%s=%u ", j,
				r->data[i].fields->data[j].size,
				r->data[i].fields->data[j].name,
				nrf_get_reg_field(i, j, &buf));
		}

		cio_print("-\n\r");
	}

	cio_print("** END nRF2401 Register DUMP **\n\r");
}

void nrf_configure_esb_rx(void) {

	// Set address for TX and receive on P0
 	nrf_reg_buf addr;

	addr.data[0] = 1;
	addr.data[1] = 2;
	addr.data[2] = 3;
	addr.data[3] = 4;
	addr.data[4] = 5;

 	// set devicde into ESB mode as PRX
	nrf_preset_esb(NRF_MODE_PRX, 40, 1, 3, NRF_RT_DELAY_250, &addr);

	// Wait for radio to power up
	delay(10000 * DF);
}

int main(void)
{
	clock_init();
	serial_init(9600);
	nrf_init();

	cio_print("nRF2401 v0.1, TestClient\n\r");

	nrf_configure_esb_rx();
	nrf_dump_regs(&nrf_reg_def);

	static nrf_payload   p;

	int s;

	// set payload size according to the payload size configured for the RX channel
	p.size = 1;

	while (1) {
		s = nrf_receive_blocking(&p);

		cio_printf("Received payload: %x; bytes received: %u\n\r", p.data[0], s);
	}

	return 0;
}
