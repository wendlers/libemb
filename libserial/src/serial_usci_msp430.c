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

#include <msp430.h>
#include <legacymsp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "serial.h"

/**
 * RXD pin
 */
#define UART_RXD   		BIT1		

/**
 * TXD pin
 */
#define UART_TXD   		BIT2	

void serial_init(unsigned int baudrate)
{
	serial_clk_init(1000000L, baudrate);
}

void serial_clk_init(long clkspeed, unsigned int baudrate)
{
	P1SEL    |= UART_RXD + UART_TXD;                       
  	P1SEL2   |= UART_RXD + UART_TXD;                       
  	UCA0CTL1 |= UCSSEL_2; 

	switch(clkspeed) {
		case  1000000L:
			switch(baudrate) {
				case   9600: UCA0BR0 = 0x06; UCA0MCTL = 0x81; break;
				case  19200: UCA0BR0 = 0x03; UCA0MCTL = 0x41; break;
				case  57600: UCA0BR0 = 0x01; UCA0MCTL = 0x0F; break;
				default    : UCA0BR0 = 0x06; UCA0MCTL = 0x81; break;
			};
			break;
		case  8000000L:
			switch(baudrate) {
				case   9600: UCA0BR0 = 0x34; UCA0MCTL = 0x11; break;
				case  19200: UCA0BR0 = 0x1a; UCA0MCTL = 0x11; break;
				case  38400: UCA0BR0 = 0x0d; UCA0MCTL = 0x01; break;
				case  57600: UCA0BR0 = 0x08; UCA0MCTL = 0xb1; break;
				case 115200: UCA0BR0 = 0x04; UCA0MCTL = 0x3b; break;
				case 230400: UCA0BR0 = 0x02; UCA0MCTL = 0x27; break;	
				default    : UCA0BR0 = 0x34; UCA0MCTL = 0x11; break;
			};
			break;
		case 16000000L:
			switch(baudrate) {
				case   9600: UCA0BR0 = 0x68; UCA0MCTL = 0x31; break;
				case  19200: UCA0BR0 = 0x34; UCA0MCTL = 0x11; break;
				case  38400: UCA0BR0 = 0x1a; UCA0MCTL = 0x11; break;
				case  57600: UCA0BR0 = 0x11; UCA0MCTL = 0x61; break;
				case 115200: UCA0BR0 = 0x08; UCA0MCTL = 0xb1; break;
				case 230400: UCA0BR0 = 0x04; UCA0MCTL = 0x3b; break;	
				default    : UCA0BR0 = 0x68; UCA0MCTL = 0x31; break;
			};
			break;
		default:
			switch(baudrate) {
				case   9600: UCA0BR0 = 0x06; UCA0MCTL = 0x81; break;
				case  19200: UCA0BR0 = 0x03; UCA0MCTL = 0x41; break;
				case  57600: UCA0BR0 = 0x01; UCA0MCTL = 0x0F; break;
				default    : UCA0BR0 = 0x06; UCA0MCTL = 0x81; break;
			};
			break;
	}
 
  	UCA0BR1   = 0;
  	UCA0CTL1 &= ~UCSWRST; 
}

void serial_send(unsigned char data)
{
  	UCA0TXBUF = data;                 		
}

void serial_send_blocking(unsigned char data)
{
	while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  	UCA0TXBUF = data;                  
}

unsigned char serial_recv()
{
	return UCA0RXBUF;
}

unsigned char serial_recv_blocking()
{
    while (!(IFG2&UCA0RXIFG));         		// USCI_A0 RX buffer ready?
	return UCA0RXBUF;
}
