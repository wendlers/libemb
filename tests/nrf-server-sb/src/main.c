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
#include "nrf24l01.h"

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

void nrf_dump_regs(nrf_regs *r) {

	int i;
	int j;

	cio_print("\n\r** START nRF2401 Register DUMP **\n\r");

	nrf_reg_buf buf;

	for(i = 0; i < r->count; i++) {

		nrf_read_reg(i, &buf);

		if(r->data[i].fields->count == 0) continue;

		cio_print(r->data[i].name);
		cio_print(": ");

		for(j = 0; j < buf.size; j++) {
			cio_printb(buf.data[j], 8);
			cio_printf(" (%x) ", buf.data[j]);
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

/**
 * Configure the NRF into ShockBurst without autoretry. Set device as PTX.
 */
void nrf_configure_sb_tx(void) {

	// Set address for TX and receive on P0
 	static nrf_reg_buf addr;

	addr.data[0] = 1;
	addr.data[1] = 2;
	addr.data[2] = 3;
	addr.data[3] = 4;
	addr.data[4] = 5;

	nrf_preset_sb(NRF_MODE_PTX, 40, 1, &addr);

	// Wait for radio to power up (100000 is way to much time though ...)
	delay(100000);
}

int main(void)
{
	clock_init();
	serial_init(38400);
	nrf_init();

	cio_print("nRF2401 v0.1 - TestServer\n\r");

	nrf_configure_sb_tx();
	nrf_dump_regs(&nrf_reg_def);

	int s;
	nrf_payload   p;

	p.size = 1;
	p.data[0] = 0;

	while (1) {
		cio_printf("Sending payload: %x ", p.data[0]);

		s = nrf_send_blocking(&p);

		cio_printf(" - done; bytes send: %u\n\r", s);

		delay(5000000);

		p.data[0]++;
	}

	return 0;
}
