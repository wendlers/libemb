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

nrf_payload   ptx;
nrf_payload   prx;

SERIAL_RB_Q srx_buf[64];
SERIAL_RB_Q stx_buf[64];

serial_rb srx;
serial_rb stx;

#define PL_SIZE 8

#ifdef STM32_100

#define TX_FULL_LED     GPIO8
#define RX_FULL_LED     GPIO9


#else

#define TX_FULL_LED     GPIO11
#define RX_FULL_LED     GPIO12

#endif

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

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RX_FULL_LED);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, TX_FULL_LED);
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
    nvic_set_priority(NVIC_USART1_IRQ, 1);

	/* Enable USART1 Receive interrupt. */
	USART_CR1(USART1) |= USART_CR1_RXNEIE;
}

void nrf_configure_sb(void)
{

	// Set address for TX and receive on P0
 	static nrf_reg_buf addr;

	addr.data[0] = 1;
	addr.data[1] = 2;
	addr.data[2] = 3;
	addr.data[3] = 4;
	addr.data[4] = 5;

 	// set devicde into ESB mode as PTX, channel 40, 2 byte payload, 3 retrys, 500ms delay
// 	nrf_preset_sb(NRF_MODE_PRX, 40, PL_SIZE, &addr);

 	// set devicde into ESB mode as PRX, channel 40, 1 byte payload, 3 retrys, 250ms delay
	nrf_preset_esb(NRF_MODE_PRX, 40, PL_SIZE, 25, NRF_RT_DELAY_3000, &addr);


	// Wait for radio to power up (100000 is way to much time though ...)
	delay(50000);
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

void nrf_set_power(unsigned char mode)
{
    nrf_reg_buf buf;

    // Power up radio
	nrf_read_reg(NRF_REG_CONFIG, &buf);
	nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PWR_UP,  &buf, mode);
	nrf_write_reg(NRF_REG_CONFIG, &buf);

	delay(10000 * mode);
}

void nrf_set_mode_ptx()
{
    nrf_reg_buf buf;

    nrf_set_power(0);   // PWR OFF

    nrf_read_reg(NRF_REG_CONFIG, &buf);

    nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PRIM_RX, &buf, NRF_MODE_PTX);
	nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_TX_DS, &buf, 0);
	nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_RX_DR, &buf, 1);

	nrf_write_reg(NRF_REG_CONFIG, &buf);

	nrf_set_power(1);   // PWR ON
}

void nrf_set_mode_prx()
{
    nrf_reg_buf buf;

    nrf_set_power(0);   // PWR OFF

    nrf_read_reg(NRF_REG_CONFIG, &buf);

    nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PRIM_RX, &buf, NRF_MODE_PRX);
	nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_TX_DS, &buf, 1);
	nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_RX_DR, &buf, 0);

	nrf_write_reg(NRF_REG_CONFIG, &buf);

	nrf_set_power(1);   // PWR ON
}

void usart1_isr(void)
{
    /*
    static unsigned int irc = 0;

    cio_printf("%u\n\r", irc++);
    */
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
   	int s;
	int i;

	clock_init();
	gpio_init();
	serial_init(9600);
	serirq_init();

//    serial_rb_init(&srx);
//    serial_rb_init(&stx);

	nrf_init();

	nrf_configure_sb();

    nrf_dump_regs(&nrf_reg_def);

	prx.size 	= PL_SIZE;
	ptx.size 	= PL_SIZE;

	while (1) {
        ptx.data[0] = 0;

        while(!serial_rb_empty(&srx) && ptx.data[0] < (PL_SIZE - 1)) {
			// ptx.data[0] = PL_SET;
			ptx.data[ptx.data[0] + 1] = serial_rb_read(&srx);
			ptx.data[0]++;
        }

        if(ptx.data[0] > 0) {

            gpio_toggle(GPIOC, TX_FULL_LED);

            // cio_printf("switching to PTX to transfer %u bytes\n\r", ptx.data[0]);

            // switch to PTX for sending out data ...
            nrf_set_mode_ptx();

            s = nrf_send_blocking(&ptx);

/*
            if(s == NRF_ERR_TX_FULL) {
                gpio_set(GPIOC, TX_FULL_LED);
            }
            else {
                gpio_clear(GPIOC, TX_FULL_LED);
            }
*/

            // cio_printf("return to PRX for receiving data ...\n\r");
            nrf_set_mode_prx();
        }

        if((s = nrf_receive(&prx)) != 0) {
/*
            if(s == NRF_ERR_RX_FULL) {
                gpio_set(GPIOC, RX_FULL_LED);
            }
            else
*/
            if(prx.data[0] > 0) {
                gpio_toggle(GPIOC, RX_FULL_LED);
                // cio_printf("received %u bytes\n\r", ptx.data[0]);
                for(i = 0; i < prx.data[0]; i++) {
//                    serial_send_blocking(prx.data[i + 1]);

                    if(!serial_rb_full(&stx)) {
                        serial_rb_write(&stx, prx.data[i + 1]);
                        USART_CR1(USART1) |= USART_CR1_TXEIE;
                    }
                }
            }
        }

/*
        if(!serial_rb_empty(&stx)) {
            USART_CR1(USART1) |= USART_CR1_TXEIE;
        }
*/
	}

	return 0;
}
