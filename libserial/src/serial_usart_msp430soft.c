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
 * TXD pin
 */
#define UART_TXD   		0x02 

/**
 * RXD pin
 */
#define UART_RXD   		0x04

/**
 * CPU freq.
 */
#define FCPU 			1000000

/**
 * Baudrate
 */
#define UART_BAUDRATE 		9600

/**
 * Half bit time
 */
#define UART_TBIT_DIV_2     (FCPU / (UART_BAUDRATE * 2))

/**
 * Bit time
 */
#define UART_TBIT           (FCPU /  UART_BAUDRATE)

/**
 * UART internal variable for TX
 */
unsigned int  txData;                      

/**
 * Received UART character
 */
unsigned char rxBuffer;                   

/**
 * Pointer to user defined ISR for receiving
 */
void (*uart_rx_isr_ptr)(unsigned char c);

/**
 * NOTE: baudrate is fix at 9600 for MSP430 softuart. Passing in a
 * different baudrate here has NO EFFECT!
 */
void serial_init(unsigned int baudrate)
{
	serial_msp430soft_recv_isr_ptr(0L);

	P1SEL = UART_TXD + UART_RXD;            // Timer function for TXD/RXD pins
    P1DIR |= UART_TXD;						// Set TXD to output
	TACCTL0 = OUT;                          // Set TXD Idle as Mark = '1'
    TACCTL1 = SCS + CM1 + CAP + CCIE;       // Sync, Neg Edge, Capture, Int
    TACTL   = TASSEL_2 + MC_2;              // SMCLK, start in continuous mode
}

void serial_msp430soft_recv_isr_ptr(void (*isr_ptr)(unsigned char c)) 
{
	uart_rx_isr_ptr = isr_ptr;	
}

void serial_send(unsigned char data)
{
    TACCR0 = TAR;                           // Current state of TA counter
    TACCR0 += UART_TBIT;                    // One bit time till first bit
    TACCTL0 = OUTMOD0 + CCIE;               // Set TXD on EQU0, Int
    txData = data;                          // Load global variable
    txData |= 0x100;                        // Add mark stop bit to TXData
    txData <<= 1;                           // Add space start bit
}

void serial_send_blocking(unsigned char data)
{
    while (TACCTL0 & CCIE);                 // Ensure last char got TX'd
	serial_send(data);
}

unsigned char serial_recv()
{
	return rxBuffer;
}

unsigned char serial_recv_blocking()
{
	// wait for incomming data
	__bis_SR_register(LPM0_bits);
	return serial_recv();
}

interrupt(TIMERA0_VECTOR) Timer_A0_ISR(void)
{
    static unsigned char txBitCnt = 10;
 
    TACCR0 += UART_TBIT;                    // Add Offset to CCRx
    if (txBitCnt == 0) {                    // All bits TXed?
        TACCTL0 &= ~CCIE;                   // All bits TXed, disable interrupt
        txBitCnt = 10;                      // Re-load bit counter
    }
    else {
        if (txData & 0x01) {
          TACCTL0 &= ~OUTMOD2;              // TX Mark '1'
        }
        else {
          TACCTL0 |= OUTMOD2;               // TX Space '0'
        }
        txData >>= 1;
        txBitCnt--;
    }
}      

interrupt(TIMERA1_VECTOR) Timer_A1_ISR(void)
{
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData   = 0;
 
    switch (TAIV) { 							 // Use calculated branching
        case TAIV_TACCR1:                        // TACCR1 CCIFG - UART RX
            TACCR1 += UART_TBIT;                 // Add Offset to CCRx
            if (TACCTL1 & CAP) {                 // Capture mode = start bit edge
                TACCTL1 &= ~CAP;                 // Switch capture to compare mode
                TACCR1 += UART_TBIT_DIV_2;       // Point CCRx to middle of D0
            }
            else {
                rxData >>= 1;
                if (TACCTL1 & SCCI) {            // Get bit waiting in receive latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {             // All bits RXed?
                    rxBuffer = rxData;           // Store in global variable
                    rxBitCnt = 8;                // Re-load bit counter
                    TACCTL1 |= CAP;              // Switch compare to capture mode

                    __bic_SR_register_on_exit(LPM0_bits);  // Clear LPM0 bits from 0(SR)

					if(uart_rx_isr_ptr != 0L) {
						(uart_rx_isr_ptr)(rxBuffer);
					}
                }
            }
            break;
    }
}
