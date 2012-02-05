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
#include <libopencm3/stm32/usart.h>

#include "serial.h"
#include "conio.h"
#include "nrf24l01.h"

nrf_payload   ptx;
nrf_payload   prx;

#define PL_SET	0x00
#define PL_NOP	0xFF

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
#ifdef NRF_REG_DEF_META
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
#endif
}

void nrf_configure_esbpl_tx(void) {

	// Set address for TX and receive on P0
 	static nrf_reg_buf addr;

	addr.data[0] = 1;
	addr.data[1] = 2;
	addr.data[2] = 3;
	addr.data[3] = 4;
	addr.data[4] = 5;

 	// set devicde into ESB mode as PTX, channel 40, 2 byte payload, 3 retrys, 500ms delay
	nrf_preset_esbpl(NRF_MODE_PTX, 40, 2, 10, NRF_RT_DELAY_500, &addr);

	// Wait for radio to power up (100000 is way to much time though ...)
	delay(100000);
}

int main(void)
{
	clock_init();
	serial_init(38400);
	nrf_init();

//	cio_print("nRF2401 v0.1 - ser2air server\n\r");

	nrf_configure_esbpl_tx();
//	nrf_dump_regs(&nrf_reg_def);

	int s;

	prx.size 	= 2;
	prx.data[0] = PL_NOP;
	prx.data[1] = 0;

	ptx.size 	= 2;
	ptx.data[0] = PL_NOP;
	ptx.data[1] = 0;

	while (1) {

		// TODO: wrap in "serial.h". something like "serial_rx_ready"
		if((USART_SR(USART1) & USART_SR_RXNE) != 0) {
			ptx.data[0] = PL_SET;
			ptx.data[1] = serial_recv();
//			cio_printf(" -> %x %x\n\r", ptx.data[0], ptx.data[1]);
		}

		s = nrf_send_blocking(&ptx);

		if(ptx.data[0] == PL_SET) {
			ptx.data[0] = PL_NOP;
			ptx.data[1] = 0;
		}

		if(s == NRF_ERR_MAX_RT) {
//			cio_printf(" <- NOK (MAX_RT)\n\r");

			// throttle down a little, client may be slow(er) ...
			delay(10000000);
		}
		else if(s == NRF_ERR_TX_FULL) {
//			cio_printf(" <- NOK (TX_FULL)\n\r");
			delay(10000000);
		}
		else {
			// see if ACK payload arived
			s = nrf_read_ack_pl(&prx);

			if(s == 0) {
//				cio_print(" <- NOK (NO_ACKPL)\n\r");
			}
			else {
//				if(prx.data[0] != PL_NOP) cio_printf(" <- %x %x\n\r", prx.data[0], prx.data[1]);
				if(prx.data[0] != PL_NOP) serial_send_blocking(prx.data[1]);
			}
		}

		// throttle down, client may be slow(er) ...
		delay(50000);
	}

	return 0;
}
