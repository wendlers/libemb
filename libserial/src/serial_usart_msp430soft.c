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
 * TXD on P1.1
 */
#define TXD BIT1

/**
 * RXD on P1.2
 */
#define RXD BIT2

/**
 * CPU freq.
 */
#define FCPU 			1000000

/**
 * Baudrate
 */
#define BAUDRATE 		9600

/**
 * Bit time
 */
#define BIT_TIME        (FCPU / BAUDRATE)

/**
 * Half bit time
 */
#define HALF_BIT_TIME   (BIT_TIME / 2)

/**
 * Bit count, used when transmitting byte
 */
static volatile uint8_t bitCount;

/**
 * Value sent over UART when uart_putc() is called
 */
static volatile unsigned int TXByte;

/**
 * Value recieved once hasRecieved is set
 */
static volatile unsigned int RXByte;

/**
 * Status for when the device is receiving
 */
static volatile bool isReceiving = false;

/**
 * Lets the program know when a byte is received
 */
static volatile bool hasReceived = false;


/**
 * NOTE: baudrate is fix at 9600 for MSP430 softuart. Passing in a
 * different baudrate here has NO EFFECT!
 */
void serial_init(unsigned int baudrate)
{
    P1SEL |= TXD;
    P1DIR |= TXD;

    P1IES |= RXD; 
    P1IFG &= ~RXD;
    P1IE  |= RXD;
}

void serial_send(unsigned char data)
{
     TXByte = (unsigned int)(data);

     while(isReceiving); 

     CCTL0 = OUT; 		
     TACTL = TASSEL_2 + MC_2; 		

     bitCount = 0xA; 
     CCR0 = TAR; 

     CCR0 += BIT_TIME; 
     TXByte |= 0x100; 
     TXByte = TXByte << 1;

     CCTL0 = CCIS_0 + OUTMOD_0 + CCIE + OUT;
}

void serial_send_blocking(unsigned char data)
{
	serial_send(data);
    while(CCTL0 & CCIE); 
}

unsigned char serial_recv()
{
	if (!hasReceived) {
    	return 0;
    }

	return (unsigned char)(RXByte);
}

unsigned char serial_recv_blocking()
{
	while(!hasReceived);
	return (unsigned char)(RXByte);
}

/**
 * ISR for RXD
 */
interrupt(PORT1_VECTOR) PORT1_ISR(void)
{
     isReceiving = true;

     P1IE &= ~RXD; 
     P1IFG &= ~RXD;

     TACTL = TASSEL_2 + MC_2;
     CCR0 = TAR; 
     CCR0 += HALF_BIT_TIME;
     CCTL0 = OUTMOD_1 + CCIE;

     RXByte = 0; 
     bitCount = 9;
}

/**
 * ISR for TXD and RXD
 */
interrupt(TIMERA0_VECTOR) TIMERA0_ISR(void)
{
     if(!isReceiving) {
          CCR0 += BIT_TIME; 						// Add Offset to CCR0
          if ( bitCount == 0) { 					// If all bits TXed
               TACTL = TASSEL_2; 					// SMCLK, timer off (for power consumption)
               CCTL0 &= ~ CCIE ; 					// Disable interrupt
          } else {
               if (TXByte & 0x01) {
                    CCTL0 = ((CCTL0 & ~OUTMOD_7 ) | OUTMOD_1);  //OUTMOD_7 defines the 'window' of the field.
               } else {
                    CCTL0 = ((CCTL0 & ~OUTMOD_7 ) | OUTMOD_5);  //OUTMOD_7 defines the 'window' of the field.
               }

               TXByte = TXByte >> 1;
               bitCount --;
          }
     } else {
          CCR0 += BIT_TIME; 						// Add Offset to CCR0

          if ( bitCount == 0) {

               TACTL = TASSEL_2; 					// SMCLK, timer off (for power consumption)
               CCTL0 &= ~ CCIE ; 					// Disable interrupt

               isReceiving = false;

               P1IFG &= ~RXD; 						// clear RXD IFG (interrupt flag)
               P1IE |= RXD; 						// enabled RXD interrupt

               if ( (RXByte & 0x201) == 0x200) { 	// Validate the start and stop bits are correct
                    RXByte = RXByte >> 1; 			// Remove start bit
                    RXByte &= 0xFF; 				// Remove stop bit
                    hasReceived = true;
               }
          } else {
               if ( (P1IN & RXD) == RXD) { 		// If bit is set?
                    RXByte |= 0x400; 				// Set the value in the RXByte
               }
               RXByte = RXByte >> 1; 				// Shift the bits down
               bitCount --;
          }
     }
}

