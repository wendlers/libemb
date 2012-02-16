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

#include "serial.h"
#include "serial_rb.h"
#include "conio.h"
#include "nrf24l01.h"

// Payload to transmit as ACK payload
nrf_payload   ptx;

// Payload received from PTX
nrf_payload   prx;

SERIAL_RB_Q srx_buf[64];
SERIAL_RB_Q stx_buf[64];

serial_rb srx;
serial_rb stx;

#define PL_NOP	0
#define PL_SIZE 8

#define RX_LED     GPIO8
#define TX_LED     GPIO9

void clock_init(void)
{
#ifdef STM32_100
	rcc_clock_setup_in_hse_8mhz_out_24mhz();
#else
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
#endif
}

void gpio_init(void)
{
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RX_LED);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, TX_LED);
}

void delay(unsigned long n)
{
	unsigned long i;

	while(n--) {
		i = 2;
		while(i--) __asm__("nop");
	}
}

void serirq_init(void)
{
    serial_rb_init(&srx, &(srx_buf[0]), 64);
    serial_rb_init(&stx, &(stx_buf[0]), 64);

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
	nrf_preset_esbpl(NRF_MODE_PRX, 40, PL_SIZE + 1, 15, NRF_RT_DELAY_750, &addr);

	// Wait for radio to power up (100000 is way to much time though ...)
	delay(100000);
}

void usart1_isr(void)
{
    unsigned char c;

	/* Check if we were called because of RXNE. */
	if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_RXNE) != 0) &&
        (!serial_rb_full(&srx))) {
        c = serial_recv();
        serial_rb_write(&srx, c);
	}
	/* Check if we were called because of TXE. */
	else if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
             ((USART_SR(USART1) & USART_SR_TXE) != 0)) {

        if(!serial_rb_empty(&stx)) {
            // serial_send_blocking(serial_rb_read(&stx));
            serial_send(serial_rb_read(&stx));
        }
        else {
            /* Disable the TXE interrupt, it's no longer needed. */
            USART_CR1(USART1) &= ~USART_CR1_TXEIE;
        }
	}
	else {
        c = serial_recv();
	}
}

int main(void)
{
    unsigned char cnt = 0;

    int s;
    int i;

	clock_init();

	gpio_init();

	serial_init(9600);
	serirq_init();
	nrf_init();

	nrf_configure_esbpl_rx();

// 	cio_print("nRF2401 v0.1 - ser2air client\n\r");

	prx.size 	= PL_SIZE + 1;
	ptx.size 	= PL_SIZE + 1;

	while (1) {

        if(++cnt == 0xff) {
            gpio_clear(GPIOC, TX_LED);
            gpio_clear(GPIOC, RX_LED);
        }

        ptx.data[0] = PL_NOP;

        while(!serial_rb_empty(&srx) && ptx.data[0] < PL_SIZE) {
			ptx.data[ptx.data[0] + 1] = serial_rb_read(&srx);
			ptx.data[0]++;
        }

        if(ptx.data[0] > 0) {
            gpio_set(GPIOC, TX_LED);
        }

		s = nrf_write_ack_pl(&ptx, 0);
		s = nrf_receive_blocking(&prx);

		if(s != 0 && prx.data[0] != PL_NOP) {
		    if(!serial_rb_full(&stx)) {

		        gpio_set(GPIOC, RX_LED);

                for(i = 0; i < prx.data[0]; i++) serial_rb_write(&stx, prx.data[i + 1]);
                USART_CR1(USART1) |= USART_CR1_TXEIE;
            }
		}
	}

	return 0;
}
