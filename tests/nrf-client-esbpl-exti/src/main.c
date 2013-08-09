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
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/exti.h>
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

static nrf_payload   ptx;
static nrf_payload   prx;

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

void exti_init()
{
#ifdef MSP430

	/*
 	 * Configure P2.0 as input, with IRQ triggered on falling edge
 	 */

    P1IES |=  BIT0;      // Hi/Lo edge interrupt
    P1IFG &= ~BIT0;      // Clear flag before enabling interrupt
    P1IE  |=  BIT0;      // Enable interrupt

	__bis_SR_register(GIE);
#else

	/*
 	 * Configure D2 as input, with IRQ triggered on falling edge 
 	 */

	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);

	nvic_enable_irq(NVIC_EXTI2_IRQ);

    gpio_set_mode(GPIOD, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO2);

	exti_select_source(EXTI2, GPIOD);

	exti_set_trigger(EXTI2, EXTI_TRIGGER_FALLING);

	exti_enable_request(EXTI2);
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
	nrf_preset_esbpl(NRF_MODE_PRX, 40, 1, 3, NRF_RT_DELAY_250, &addr);

	// Wait for radio to power up 
	delay(10000 * DF);
}

#ifdef MSP430
interrupt(PORT1_VECTOR) PORT1_ISR(void)
{
    P1IFG &= ~BIT0;                 // Clear interrupt flag
#else
void exti2_isr(void)
{
    exti_reset_request(EXTI2);		// Clear interrupt flag
#endif
	int s;

	// set ACK payload to next squence number
	ptx.data[0]++;
	s = nrf_write_ack_pl(&ptx, 0);

	if(s == NRF_ERR_TX_FULL) {
		cio_printf("Unable to send back ACK payload (TX_FULL)\n\r");
	}

	// receive non-blocking since IRQ already indicates that payload arived
	s = nrf_receive(&prx);

	if(s == NRF_ERR_RX_FULL) {
		cio_printf("Unable to receive any more (RX_FULL)\n\r");
	}
	else {
		cio_printf("Received payload: %c; sending back %u\n\r", prx.data[0], ptx.data[0]);
	}
}

int main(void)
{
	clock_init();
	serial_init(9600);
	exti_init();
	nrf_init();
	cio_print("nRF2401 v0.1 - TestClient ESBPL-exti\n\r");

	nrf_configure_esbpl_rx();

	// setup payload
	prx.size = 1;		// receive payload size 1Byte
	ptx.size = 1;		// send payload size 1Byte
	ptx.data[0] = 0;	// set payload to 0
	
	// Nothing to do here since ISR does all the work
	while (1) {
		__asm__("nop");
	}

	return 0;
}
