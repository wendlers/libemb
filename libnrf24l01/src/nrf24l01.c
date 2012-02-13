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

#include "nrf24l01.h"

/*
 * NRF2401 Commands
 */

#define NRF_CMD_RREG		0b00000000
#define NRF_CMD_WREG		0b00100000
#define NRF_CMD_TX    		0b10100000
#define NRF_CMD_RX    		0b01100001
#define NRF_CMD_NOP			0b11111111
#define NRF_CMD_FLUSH_TX	0b11100001
#define NRF_CMD_FLUSH_RX	0b11100010
#define NRF_CMD_WACKPL		0b10101000

int nrf_read_reg(unsigned char reg, nrf_reg_buf *buf)
{
     int i;

     // get register payload size
     unsigned char s = nrf_reg_def.data[reg].size;

     nrf_spi_csl();

     // send command
     nrf_spi_xfer_byte(NRF_CMD_RREG | reg);

     // receive response
     for(i = 0; i < s; i++) {
          buf->data[i] = nrf_spi_xfer_byte(NRF_CMD_NOP);
     }

     nrf_spi_csh();

     buf->size = i;

     return i;
}

int nrf_write_reg(unsigned char reg, nrf_reg_buf *buf)
{
     int i;

     // get register payload size
     unsigned char s = nrf_reg_def.data[reg].size;

     nrf_spi_csl();

     // send command
     nrf_spi_xfer_byte(NRF_CMD_WREG | reg);

     // send payload
     for(i = 0; i < s; i++) {
          nrf_spi_xfer_byte(buf->data[i]);
     }

     nrf_spi_csh();

     return i;
}

int nrf_send(nrf_payload *payload)
{
     int i;

     nrf_reg_buf status;
     nrf_read_reg(NRF_REG_STATUS, &status);

     // if TX buffer is full, indicate no bytes where sent, flush TX buffer
     if(nrf_get_reg_field(NRF_REG_STATUS, NRF_REGF_TX_FULL, &status) == 1) {
          nrf_spi_csl();
          nrf_spi_xfer_byte(NRF_CMD_FLUSH_TX);
          nrf_spi_csh();
          return NRF_ERR_TX_FULL;
     }

     // send command
     nrf_spi_csl();
     nrf_spi_xfer_byte(NRF_CMD_TX);

     // send payload
     for(i = 0; i < payload->size; i++) {
          nrf_spi_xfer_byte(payload->data[i]);
     }

     nrf_spi_csh();

     return i;
}

int nrf_send_blocking(nrf_payload *payload)
{
     int i;

     nrf_reg_buf status;
     nrf_read_reg(NRF_REG_STATUS, &status);

     // if TX buffer is full, indicate no bytes where sent, flush TX buffer
     if(nrf_get_reg_field(NRF_REG_STATUS, NRF_REGF_TX_FULL, &status) == 1) {
          nrf_spi_csl();
          nrf_spi_xfer_byte(NRF_CMD_FLUSH_TX);
          nrf_spi_csh();
          i =  NRF_ERR_TX_FULL;
     }

     // send command
     nrf_spi_csl();
     nrf_spi_xfer_byte(NRF_CMD_TX);

     // send payload
     for(i = 0; i < payload->size; i++) {
          nrf_spi_xfer_byte(payload->data[i]);
     }
     nrf_spi_csh();

     // wait until payload passed TX FIFO
     do {
          nrf_read_reg(NRF_REG_STATUS, &status);

          // If MAX_RT is reached, indicate no bytes where sent ...
          if(nrf_get_reg_field(NRF_REG_STATUS, NRF_REGF_MAX_RT, &status) == 1) {
               i =  NRF_ERR_MAX_RT;
               break;
          }
     } while(nrf_get_reg_field(NRF_REG_STATUS, NRF_REGF_TX_DS , &status) != 1);

     // clear TX_DS/MAX_RT bits (by writing 1 or just writing back the status)
     nrf_write_reg(NRF_REG_STATUS, &status);

     return i;
}


int nrf_receive(nrf_payload *payload)
{
     unsigned char i = 0;

     nrf_reg_buf status;

     nrf_read_reg(NRF_REG_STATUS, &status);

     // receive payload
     if(nrf_get_reg_field(NRF_REG_STATUS, NRF_REGF_RX_DR, &status) == 1) {

          nrf_spi_csl();
          nrf_spi_xfer_byte(NRF_CMD_RX);

          for(i = 0; i < payload->size; i++) {
               payload->data[i] = nrf_spi_xfer_byte(NRF_CMD_RREG);
          }

          nrf_spi_csh();

          // write back status to clean RX_DR
          nrf_write_reg(NRF_REG_STATUS, &status);
     }

     nrf_read_reg(NRF_REG_FIFO_STATUS, &status);

     // if RX buffer is full, indicate no bytes where receifed, flush RX buffer
     if(nrf_get_reg_field(NRF_REG_FIFO_STATUS, NRF_REGF_FIFO_RX_FULL, &status) == 1) {
          nrf_spi_csl();
          nrf_spi_xfer_byte(NRF_CMD_FLUSH_RX);
          nrf_spi_csh();
          return NRF_ERR_RX_FULL;
     }

     return i;
}

int nrf_receive_blocking(nrf_payload *payload)
{
     unsigned char i = 0;

     nrf_reg_buf status;

     // wait until data arrives
     do {
          nrf_read_reg(NRF_REG_STATUS, &status);
     } while(nrf_get_reg_field(NRF_REG_STATUS, NRF_REGF_RX_DR, &status) != 1);

     // receive payload
     nrf_spi_csl();
     nrf_spi_xfer_byte(NRF_CMD_RX);

     for(i = 0; i < payload->size; i++) {
          payload->data[i] = nrf_spi_xfer_byte(NRF_CMD_RREG);
     }

     nrf_spi_csh();

     // write back status to clean RX_DR
     nrf_write_reg(NRF_REG_STATUS, &status);

     nrf_read_reg(NRF_REG_FIFO_STATUS, &status);

     // if RX buffer is full, indicate no bytes where receifed, flush RX buffer
     if(nrf_get_reg_field(NRF_REG_FIFO_STATUS, NRF_REGF_FIFO_RX_FULL, &status) == 1) {
          nrf_spi_csl();
          nrf_spi_xfer_byte(NRF_CMD_FLUSH_RX);
          nrf_spi_csh();
          return NRF_ERR_RX_FULL;
     }

     return i;
}

int nrf_write_ack_pl(nrf_payload *payload, unsigned char pipe)
{
     int i;

     nrf_reg_buf status;
     nrf_read_reg(NRF_REG_STATUS, &status);

     // if TX buffer is full, indicate no bytes where sent, flush TX buffer
     if(nrf_get_reg_field(NRF_REG_STATUS, NRF_REGF_TX_FULL, &status) == 1) {
          nrf_spi_csl();
          nrf_spi_xfer_byte(NRF_CMD_FLUSH_TX);
          nrf_spi_csh();
          return NRF_ERR_TX_FULL;
     }

     nrf_spi_csl();
     nrf_spi_xfer_byte(NRF_CMD_WACKPL | (0b00000111 & pipe));

     for(i = 0; i < payload->size; i++) {
          nrf_spi_xfer_byte(payload->data[i]);
     }

     nrf_spi_csh();

     return i;
}

int nrf_read_ack_pl(nrf_payload *payload)
{
     unsigned char i = 0;

     nrf_reg_buf status;
     nrf_read_reg(NRF_REG_FIFO_STATUS, &status);

     // if RX buffer is full, indicate no bytes where receifed, flush RX buffer
     if(nrf_get_reg_field(NRF_REG_FIFO_STATUS, NRF_REGF_FIFO_RX_FULL, &status) == 1) {
          nrf_spi_csl();
          nrf_spi_xfer_byte(NRF_CMD_FLUSH_RX);
          nrf_spi_csh();
          return NRF_ERR_RX_FULL;
     }

     nrf_spi_csl();
     nrf_spi_xfer_byte(NRF_CMD_RX);

     for(i = 0; i < payload->size; i++) {
          payload->data[i] = nrf_spi_xfer_byte(NRF_CMD_RREG);
     }

     nrf_spi_csh();

     return i;
}

void nrf_preset_sb(unsigned char mode, unsigned char rf_ch, unsigned char pw, nrf_reg_buf *addr)
{

     nrf_reg_buf buf;

     // Disable auto ACK on all pipes
     nrf_read_reg(NRF_REG_EN_AA, &buf);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P0,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P1,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P2,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P3,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P4,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P5,  &buf, 0);
     nrf_write_reg(NRF_REG_EN_AA, &buf);

     // Disable RX addresses, except PIPE0
     nrf_read_reg(NRF_REG_EN_RXADDR, &buf);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P0,  &buf, 1);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P1,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P2,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P3,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P4,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P5,  &buf, 0);
     nrf_write_reg(NRF_REG_EN_RXADDR, &buf);

     // CONFIG - CRC enable, 2-Bit CRC, RX/TX mode, disable MAX_RT_IRQ + TX_DS/RX_DR
     nrf_read_reg(NRF_REG_CONFIG, &buf);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_EN_CRC,  &buf, 1);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_CRCO,    &buf, 1);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PRIM_RX, &buf, mode);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_MAX_RT, &buf, 1);

     if(mode == NRF_MODE_PRX) {
          nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_TX_DS, &buf, 1);
     } else {
          nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_RX_DR, &buf, 1);
     }

     nrf_write_reg(NRF_REG_CONFIG, &buf);

     // Disable auto retry
     nrf_read_reg(NRF_REG_SETUP_RETR, &buf);
     nrf_set_reg_field(NRF_REG_SETUP_RETR, NRF_REGF_ARC,  &buf, 0);
     nrf_set_reg_field(NRF_REG_SETUP_RETR, NRF_REGF_ARD,  &buf, 0);
     nrf_write_reg(NRF_REG_SETUP_RETR, &buf);

     // Set address width to 5 bytes
     nrf_read_reg(NRF_REG_SETUP_AW, &buf);
     nrf_set_reg_field(NRF_REG_SETUP_AW, NRF_REGF_AW,  &buf, 0b11);
     nrf_write_reg(NRF_REG_SETUP_AW, &buf);

     // RX_ADDR_P0 - set receive address data pipe0
     nrf_write_reg(NRF_REG_RX_ADDR_P0, addr);

     // TX_ADDR - transmit address
     nrf_write_reg(NRF_REG_TX_ADDR, addr);

     if(mode == NRF_MODE_PRX) {
          // RX_PW_P0 - set number of bytes in RX payload in data PIPE0
          nrf_read_reg(NRF_REG_RX_PW_P0, &buf);
          nrf_set_reg_field(NRF_REG_RX_PW_P0, NRF_REGF_PW, &buf, pw);
          nrf_write_reg(NRF_REG_RX_PW_P0, &buf);
     }

     // Set RF-channel
     nrf_read_reg(NRF_REG_RF_CH, &buf);
     nrf_set_reg_field(NRF_REG_RF_CH, NRF_REGF_RF_CH, &buf, rf_ch);
     nrf_write_reg(NRF_REG_RF_CH, &buf);
	 
	 // Setup Data-Rate to 1MBit and RF power to 0db
     nrf_read_reg(NRF_REG_RF_SETUP, &buf);
     nrf_set_reg_field(NRF_REG_RF_SETUP, NRF_REGF_RF_DR , &buf, 0);
     nrf_set_reg_field(NRF_REG_RF_SETUP, NRF_REGF_RF_PWR, &buf, 3);
     nrf_write_reg(NRF_REG_RF_SETUP, &buf);


     // Power up radio
     nrf_read_reg(NRF_REG_CONFIG, &buf);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PWR_UP,  &buf, 1);
     nrf_write_reg(NRF_REG_CONFIG, &buf);
}

void nrf_preset_esb(
     unsigned char mode, unsigned char rf_ch, unsigned char pw,
     unsigned char retr, unsigned char delay, nrf_reg_buf *addr)
{

     nrf_reg_buf buf;

     // Disable auto ACK on all pipes, except PIPE0
     nrf_read_reg(NRF_REG_EN_AA, &buf);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P0,  &buf, 1);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P1,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P2,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P3,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P4,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P5,  &buf, 0);
     nrf_write_reg(NRF_REG_EN_AA, &buf);

     // Disable RX addresses, except PIPE0
     nrf_read_reg(NRF_REG_EN_RXADDR, &buf);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P0,  &buf, 1);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P1,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P2,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P3,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P4,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P5,  &buf, 0);
     nrf_write_reg(NRF_REG_EN_RXADDR, &buf);

     // CONFIG - CRC enable, 2-Bit CRC, RX/TX mode
     nrf_read_reg(NRF_REG_CONFIG, &buf);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_EN_CRC,  &buf, 1);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_CRCO,    &buf, 1);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PRIM_RX, &buf, mode);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_MAX_RT, &buf, 1);

     if(mode == NRF_MODE_PRX) {
          nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_TX_DS, &buf, 1);
     } else {
          nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_RX_DR, &buf, 1);
     }

     nrf_write_reg(NRF_REG_CONFIG, &buf);

     // Enable auto retry and delay
     nrf_read_reg(NRF_REG_SETUP_RETR, &buf);
     nrf_set_reg_field(NRF_REG_SETUP_RETR, NRF_REGF_ARC,  &buf, retr);
     nrf_set_reg_field(NRF_REG_SETUP_RETR, NRF_REGF_ARD,  &buf, delay);
     nrf_write_reg(NRF_REG_SETUP_RETR, &buf);

     // Set address width to 5 bytes
     nrf_read_reg(NRF_REG_SETUP_AW, &buf);
     nrf_set_reg_field(NRF_REG_SETUP_AW, NRF_REGF_AW,  &buf, 0b11);
     nrf_write_reg(NRF_REG_SETUP_AW, &buf);

     // RX_ADDR_P0 - set receive address data pipe0
     nrf_write_reg(NRF_REG_RX_ADDR_P0, addr);

     // TX_ADDR - transmit address
     nrf_write_reg(NRF_REG_TX_ADDR, addr);

     if(mode == NRF_MODE_PRX) {
          // RX_PW_P0 - set number of bytes in RX payload in data PIPE0
          nrf_read_reg(NRF_REG_RX_PW_P0, &buf);
          nrf_set_reg_field(NRF_REG_RX_PW_P0, NRF_REGF_PW, &buf, pw);
          nrf_write_reg(NRF_REG_RX_PW_P0, &buf);
     }

     // Set RF-channel
     nrf_read_reg(NRF_REG_RF_CH, &buf);
     nrf_set_reg_field(NRF_REG_RF_CH, NRF_REGF_RF_CH, &buf, rf_ch);
     nrf_write_reg(NRF_REG_RF_CH, &buf);
    
     // Setup Data-Rate to 1MBit and RF power to 0db
     nrf_read_reg(NRF_REG_RF_SETUP, &buf);
     nrf_set_reg_field(NRF_REG_RF_SETUP, NRF_REGF_RF_DR , &buf, 0);
     nrf_set_reg_field(NRF_REG_RF_SETUP, NRF_REGF_RF_PWR, &buf, 3);
     nrf_write_reg(NRF_REG_RF_SETUP, &buf);


     // Power up radio
     nrf_read_reg(NRF_REG_CONFIG, &buf);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PWR_UP,  &buf, 1);
     nrf_write_reg(NRF_REG_CONFIG, &buf);
}

void nrf_preset_esbpl(
     unsigned char mode, unsigned char rf_ch, unsigned char pw,
     unsigned char retr, unsigned char delay, nrf_reg_buf *addr)
{

     nrf_reg_buf buf;

     // Disable auto ACK on all pipes, except PIPE0
     nrf_read_reg(NRF_REG_EN_AA, &buf);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P0,  &buf, 1);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P1,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P2,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P3,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P4,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_AA, NRF_REGF_ENAA_P5,  &buf, 0);
     nrf_write_reg(NRF_REG_EN_AA, &buf);

     // Disable RX addresses, except PIPE0
     nrf_read_reg(NRF_REG_EN_RXADDR, &buf);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P0,  &buf, 1);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P1,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P2,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P3,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P4,  &buf, 0);
     nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P5,  &buf, 0);
     nrf_write_reg(NRF_REG_EN_RXADDR, &buf);

     // CONFIG - CRC enable, 2-Bit CRC, RX/TX mode
     nrf_read_reg(NRF_REG_CONFIG, &buf);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_EN_CRC,  &buf, 1);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_CRCO,    &buf, 1);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PRIM_RX, &buf, mode);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_MAX_RT, &buf, 1);

     if(mode == NRF_MODE_PRX) {
          nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_TX_DS, &buf, 1);
     } else {
          nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_MASK_RX_DR, &buf, 1);
     }

     nrf_write_reg(NRF_REG_CONFIG, &buf);

     // Enable auto retry and delay
     nrf_read_reg(NRF_REG_SETUP_RETR, &buf);
     nrf_set_reg_field(NRF_REG_SETUP_RETR, NRF_REGF_ARC,  &buf, retr);
     nrf_set_reg_field(NRF_REG_SETUP_RETR, NRF_REGF_ARD,  &buf, delay);
     nrf_write_reg(NRF_REG_SETUP_RETR, &buf);

     // Set address width to 5 bytes
     nrf_read_reg(NRF_REG_SETUP_AW, &buf);
     nrf_set_reg_field(NRF_REG_SETUP_AW, NRF_REGF_AW,  &buf, 0b11);
     nrf_write_reg(NRF_REG_SETUP_AW, &buf);

     // RX_ADDR_P0 - set receive address data pipe0
     nrf_write_reg(NRF_REG_RX_ADDR_P0, addr);

     // TX_ADDR - transmit address
     nrf_write_reg(NRF_REG_TX_ADDR, addr);

     // Set ACK PL + DYN PL
     nrf_read_reg(NRF_REG_FEATURE, &buf);
     nrf_set_reg_field(NRF_REG_FEATURE, NRF_REGF_EN_DPL, &buf, 1);
     nrf_set_reg_field(NRF_REG_FEATURE, NRF_REGF_EN_ACK_PAY, &buf, 1);
     nrf_write_reg(NRF_REG_FEATURE, &buf);

     // Enable dynamic payload width on PIPE0
     nrf_read_reg(NRF_REG_DYNPD, &buf);
     nrf_set_reg_field(NRF_REG_DYNPD, NRF_REGF_DPL_P0, &buf, 1);
     nrf_set_reg_field(NRF_REG_DYNPD, NRF_REGF_DPL_P1, &buf, 0);
     nrf_set_reg_field(NRF_REG_DYNPD, NRF_REGF_DPL_P2, &buf, 0);
     nrf_set_reg_field(NRF_REG_DYNPD, NRF_REGF_DPL_P3, &buf, 0);
     nrf_set_reg_field(NRF_REG_DYNPD, NRF_REGF_DPL_P4, &buf, 0);
     nrf_set_reg_field(NRF_REG_DYNPD, NRF_REGF_DPL_P5, &buf, 0);
     nrf_write_reg(NRF_REG_DYNPD, &buf);

     if(mode == NRF_MODE_PRX) {
          // RX_PW_P0 - set number of bytes in RX payload in data PIPE0
          nrf_read_reg(NRF_REG_RX_PW_P0, &buf);
          nrf_set_reg_field(NRF_REG_RX_PW_P0, NRF_REGF_PW, &buf, pw);
          nrf_write_reg(NRF_REG_RX_PW_P0, &buf);
     }

     // Set RF-channel
     nrf_read_reg(NRF_REG_RF_CH, &buf);
     nrf_set_reg_field(NRF_REG_RF_CH, NRF_REGF_RF_CH, &buf, rf_ch);
     nrf_write_reg(NRF_REG_RF_CH, &buf);

     // Setup Data-Rate to 1MBit and RF power to 0db
     nrf_read_reg(NRF_REG_RF_SETUP, &buf);
     nrf_set_reg_field(NRF_REG_RF_SETUP, NRF_REGF_RF_DR , &buf, 0);
     nrf_set_reg_field(NRF_REG_RF_SETUP, NRF_REGF_RF_PWR, &buf, 3);
     nrf_write_reg(NRF_REG_RF_SETUP, &buf);

     // Power up radio
     nrf_read_reg(NRF_REG_CONFIG, &buf);
     nrf_set_reg_field(NRF_REG_CONFIG, NRF_REGF_PWR_UP,  &buf, 1);
     nrf_write_reg(NRF_REG_CONFIG, &buf);
}
