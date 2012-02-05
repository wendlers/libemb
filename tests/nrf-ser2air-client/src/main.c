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
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/nvic.h>
#include <libopencm3/stm32/exti.h>

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

// Payload to transmit as ACK payload
nrf_payload   ptx;

// Payload received from PTX
nrf_payload   prx;

#define PL_SET	0x00
#define PL_NOP	0xFF

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

void exti_init(void)
{
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);

	nvic_enable_irq(NVIC_EXTI2_IRQ);

    gpio_set_mode(GPIOD, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO2);

	exti_select_source(EXTI2, GPIOD);

	exti_set_trigger(EXTI2, EXTI_TRIGGER_FALLING);

	exti_enable_request(EXTI2);
}

void serirq_init(void)
{
	/* Enable the USART1 interrupt. */
	nvic_enable_irq(NVIC_USART1_IRQ);

	/* Enable USART1 Receive interrupt. */
	USART_CR1(USART1) |= USART_CR1_RXNEIE;
}

void nrf_configure_esbpl_rx(void)
{
	// Set address for TX and receive on P0
 	static nrf_reg_buf addr;

	addr.data[0] = 1;
	addr.data[1] = 2;
	addr.data[2] = 3;
	addr.data[3] = 4;
	addr.data[4] = 5;

 	// set devicde into ESB mode as PTX, channel 40, 2 byte payload, 3 retrys, 250ms delay
	nrf_preset_esbpl(NRF_MODE_PRX, 40, 2, 3, NRF_RT_DELAY_250, &addr);

	// Wait for radio to power up (100000 is way to much time though ...)
	delay(100000);
}

unsigned int n = 0;

void exti2_isr(void)
{
    exti_reset_request(EXTI2);

	int s;

	s = nrf_write_ack_pl(&ptx, 0);

	cio_printf("[%u] -> %x %x", n++, ptx.data[0], ptx.data[1]);

	if(s == NRF_ERR_TX_FULL) {
		cio_printf(" <- NOK (TX_FULL)\n\r");
	}

	// receive non-blocking since IRQ already indicates that payload arived
	s = nrf_receive_blocking(&prx);

	if(s == NRF_ERR_RX_FULL) {
		cio_printf(" <- NOK (RX_FULL)\n\r");
	}
	else {
		cio_printf(" <- %x %x\n\r", prx.data[0], prx.data[1]);
	}

	if(ptx.data[0] == PL_SET) {
		ptx.data[0] = PL_NOP;
		ptx.data[1] = 0;
	}

/*
	nrf_spi_csl();
	nrf_spi_xfer_byte(0b11100010);
	nrf_spi_csh();
*/
}

void usart1_isr(void)
{
    /* Check if we were called because of RXNE. */
    if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
            ((USART_SR(USART1) & USART_SR_RXNE) != 0) && ptx.data[0] == PL_NOP) {
		ptx.data[0] = PL_SET;
		ptx.data[1] = cio_getc();
		cio_printf("\n\rSetting payload to %x %x\n\r", ptx.data[0], ptx.data[1]);
	}
}

int main(void)
{
	clock_init();
	serial_init(38400);
//	serirq_init();
//	exti_init();
	nrf_init();

//	cio_print("nRF2401 v0.1 - ser2air client\n\r");

	nrf_configure_esbpl_rx();
//	nrf_dump_regs(&nrf_reg_def);

	prx.size 	= 2;
	prx.data[0] = PL_NOP;
	prx.data[1] = 0;

	ptx.size 	= 2;
	ptx.data[0] = PL_NOP;
	ptx.data[1] = 0;

	nrf_reg_buf status;

	while (1) {
		int s;

		if((USART_SR(USART1) & USART_SR_RXNE) != 0) {
			ptx.data[0] = PL_SET;
			ptx.data[1] = serial_recv();
//			cio_printf(" -> %x %x\n\r", ptx.data[0], ptx.data[1]);
		}

		s = nrf_write_ack_pl(&ptx, 0);

		if(ptx.data[0] == PL_SET) {
			ptx.data[0] = PL_NOP;
			ptx.data[1] = 0;
		}


		if(s == NRF_ERR_TX_FULL) {
//			cio_printf(" <- NOK (TX_FULL)\n\r");
			delay(10000000);
		}

		s = nrf_receive_blocking(&prx);

		if(s == NRF_ERR_RX_FULL) {
//			cio_printf(" <- NOK (RX_FULL)\n\r");
		}
		else {
//			if(prx.data[0] != PL_NOP) cio_printf(" <- %x %x\n\r", prx.data[0], prx.data[1]);
			if(prx.data[0] != PL_NOP) serial_send_blocking(prx.data[1]);
		}
	}

	return 0;
}
