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
#include <legacymsp430.h>
#else
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/f1/nvic.h>
#endif

#include "serial.h"
#include "serial_rb.h"
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

static nrf_payload   ptx;
static nrf_payload   prx;

#define RB_SIZE 32	

static SERIAL_RB_Q srx_buf[RB_SIZE];
static serial_rb srx;

static SERIAL_RB_Q stx_buf[RB_SIZE];
static serial_rb stx;

#define PL_NOP	0
#define PL_SIZE 8

#ifdef MSP430
#define RXTX_LED       	BIT0 
#else
#define TX_LED          GPIO8
#define RX_LED          GPIO9
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

void gpio_init(void)
{
#ifdef MSP430
	P1DIR  = RXTX_LED;
	P1OUT  = RXTX_LED;
#else
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RX_LED);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, TX_LED);
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

void serirq_init(void)
{
    serial_rb_init(&srx, &(srx_buf[0]), RB_SIZE);
    serial_rb_init(&stx, &(stx_buf[0]), RB_SIZE);

#ifdef MSP430
    IE2 |= UCA0RXIE; 
	__bis_SR_register(GIE);
#else

	/* Enable the USART1 interrupt. */
	nvic_enable_irq(NVIC_USART1_IRQ);

	/* Enable USART1 Receive interrupt. */
	USART_CR1(USART1) |= USART_CR1_RXNEIE;
#endif
}

void nrf_configure_esbpl_rx(void)
{
	// Set address for TX and receive on P0
 	nrf_reg_buf addr;

	addr.data[0] = 1;
	addr.data[1] = 2;
	addr.data[2] = 3;
	addr.data[3] = 4;
	addr.data[4] = 5;

 	// set devicde into ESB mode as PTX
	nrf_preset_esbpl(NRF_MODE_PRX, 40, PL_SIZE + 1, 15, NRF_RT_DELAY_750, &addr);

	// Wait for radio to power up
	delay(10000 * DF);
}

#ifdef MSP430
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
	if (!serial_rb_full(&srx)) {
        serial_rb_write(&srx, UCA0RXBUF);
	}
}

interrupt(USCIAB0TX_VECTOR) USCI0TX_ISR(void)
{
	if(!serial_rb_empty(&stx)) {
    	serial_send(serial_rb_read(&stx));
    }
    else {
    	/* Disable the TX interrupt, it's no longer needed. */
		IE2 &= ~UCA0TXIE; 
    }
}
#else
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
#endif

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

 	cio_print("nRF2401 v0.1 - ser2air client\n\r");

	prx.size 	= PL_SIZE + 1;
	ptx.size 	= PL_SIZE + 1;

	while (1) {

        if(++cnt == 0xff) {
#ifdef MSP430
			P1OUT &= ~RXTX_LED;
#else
            gpio_clear(GPIOC, TX_LED);
            gpio_clear(GPIOC, RX_LED);
#endif
        }

        ptx.data[0] = PL_NOP;

        while(!serial_rb_empty(&srx) && ptx.data[0] < PL_SIZE) {
			ptx.data[ptx.data[0] + 1] = serial_rb_read(&srx);
			ptx.data[0]++;
        }

        if(ptx.data[0] > 0) {
#ifdef MSP430
			P1OUT |= RXTX_LED;
#else
            gpio_set(GPIOC, TX_LED);
#endif
        }

		nrf_write_ack_pl(&ptx, 0);

		s = nrf_receive_blocking(&prx);

		if(s != 0 && prx.data[0] != PL_NOP) {
            if(!serial_rb_full(&stx)) {
                for(i = 0; i < prx.data[0]; i++) serial_rb_write(&stx, prx.data[i + 1]);
#ifdef MSP430
				P1OUT |= RXTX_LED;
				IE2 |= UCA0TXIE;
#else
             	gpio_set(GPIOC, RX_LED);
               	USART_CR1(USART1) |= USART_CR1_TXEIE;
#endif
            }
		}
	}

	return 0;
}
