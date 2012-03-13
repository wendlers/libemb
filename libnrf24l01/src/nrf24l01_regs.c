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

#include "nrf24l01_regs.h"

#ifdef NRF_REG_DEF_META

#define REGF(FNAME, FSIZE, FRW)  	 { .name = FNAME, .size = FSIZE, .rw = FRW, }
#define REG(RNAME, RSIZE, RFIELDS)   { .name = RNAME, .size = RSIZE, .fields = RFIELDS, }

#else

#define REGF(FNAME, FSIZE, FRW)  	 { .size = FSIZE, }
#define REG(RNAME, RSIZE, RFIELDS)   { .size = RSIZE, .fields = RFIELDS, }

#endif

nrf_reg_fields nrf_reg_config_fields = {
     .count = 7,
     .data  = {
          REGF("PRIM_RX",		1, 1),
          REGF("PWR_UP",		1, 1),
          REGF("CRCO", 		1, 1),
          REGF("EN_CRC", 		1, 1),
          REGF("MASK_MAX_RT",	1, 1),
          REGF("MASK_TX_DS", 	1, 1),
          REGF("MASK_RX_DR", 	1, 1),
     },
};

nrf_reg_fields nrf_reg_enaa_fields = {
     .count = 6,
     .data  = {
          REGF("ENAA_P0",		1, 1),
          REGF("ENAA_P1", 	1, 1),
          REGF("ENAA_P2", 	1, 1),
          REGF("ENAA_P3", 	1, 1),
          REGF("ENAA_P4", 	1, 1),
          REGF("ENAA_P5", 	1, 1),
     },
};

nrf_reg_fields nrf_reg_enrxaddr_fields = {
     .count = 6,
     .data  = {
          REGF("ERX_P0", 		1, 1),
          REGF("ERX_P1",		1, 1),
          REGF("ERX_P2",		1, 1),
          REGF("ERX_P3",		1, 1),
          REGF("ERX_P4", 		1, 1),
          REGF("ERX_P5",		1, 1),
     },
};

nrf_reg_fields nrf_reg_setupaw_fields = {
     .count = 1,
     .data  = {
          REGF("AW",			2, 1),
     },
};

nrf_reg_fields nrf_reg_setupretr_fields = {
     .count = 2,
     .data  = {
          REGF("ARC", 		4, 1),
          REGF("ARD", 		4, 1),
     },
};

nrf_reg_fields nrf_reg_rfch_fields = {
     .count = 1,
     .data  = {
          REGF("RF_CH",		7, 1),
     },
};

nrf_reg_fields nrf_reg_rfsetup_fields = {
     .count = 4,
     .data  = {
          REGF("LNA_HCURR", 	1, 1),
          REGF("RF_PWR", 		2, 1),
          REGF("RF_DR", 		1, 1),
          REGF("PLL_LOCK", 	1, 1),
     },
};

nrf_reg_fields nrf_reg_status_fields = {
     .count = 5,
     .data  = {
          REGF("TX_FULL",		1, 0),
          REGF("RX_P_NO", 	3, 0),
          REGF("MAX_RT",		1, 1),
          REGF("TX_DS", 		1, 1),
          REGF("RX_DR", 		1, 1),
     },
};

nrf_reg_fields nrf_reg_observetx_fields = {
     .count = 2,
     .data  = {
          REGF("ARC_CNT", 	4, 0),
          REGF("PLOS_CNT",	4, 0),
     },
};

nrf_reg_fields nrf_reg_cd_fields = {
     .count = 1,
     .data  = {
          REGF("CD",			1, 0),
     },
};

nrf_reg_fields nrf_reg_addr_fields = {
     .count = 5,
     .data  = {
          REGF("A",			8, 1),
          REGF("B", 			8, 1),
          REGF("C", 			8, 1),
          REGF("D", 			8, 1),
          REGF("E", 			8, 1),
     },
};

nrf_reg_fields nrf_reg_rxpw_fields = {
     .count = 1,
     .data  = {
          REGF("RX_PW",		6, 1),
     },
};

nrf_reg_fields nrf_reg_fifostat_fields = {
     .count = 6,
     .data  = {
          REGF("RX_EMPTY",	1, 0),
          REGF("RX_FULL", 	1, 0),
          REGF("RESERVED", 	2, 0),
          REGF("TX_EMPTY",	1, 0),
          REGF("TX_FULL", 	1, 0),
          REGF("TX_REUSE",	1, 0),
     },
};

nrf_reg_fields nrf_reg_dynpd_fields = {
     .count = 6,
     .data  = {
          REGF("DPL_P0",		1, 1),
          REGF("DPL_P1",		1, 1),
          REGF("DPL_P2",		1, 1),
          REGF("DPL_P3",		1, 1),
          REGF("DPL_P4",		1, 1),
          REGF("DPL_P5",		1, 1),
     },
};

nrf_reg_fields nrf_reg_feature_fields = {
     .count = 3,
     .data  = {
          REGF("EN_DYN_ACK",	1, 1),
          REGF("EN_ACK_PAY",	1, 1),
          REGF("EN_DPL",		1, 1),
     },
};


nrf_regs nrf_reg_def = {
     .count = 30,
     .data  = {
          REG("CONFIG", 		1, &nrf_reg_config_fields		),
          REG("EN_AA", 			1, &nrf_reg_enaa_fields 		),
          REG("EN_RXADDR", 		1, &nrf_reg_enrxaddr_fields 	),
          REG("SETUP_AW", 		1, &nrf_reg_setupaw_fields 		),
          REG("SETUP_RETR", 	1, &nrf_reg_setupretr_fields 	),
          REG("RF_CH", 			1, &nrf_reg_rfch_fields 		),
          REG("RF_SETUP", 		1, &nrf_reg_rfsetup_fields	 	),
          REG("STATUS", 		1, &nrf_reg_status_fields 		),
          REG("OBSERVE_TX", 	1, &nrf_reg_observetx_fields 	),
          REG("CD", 			1, &nrf_reg_cd_fields 			),
          REG("RX_ADDR_P0", 	5, &nrf_reg_addr_fields 		),
          REG("RX_ADDR_P1", 	5, &nrf_reg_addr_fields 		),
          REG("RX_ADDR_P2", 	5, &nrf_reg_addr_fields 		),
          REG("RX_ADDR_P3", 	5, &nrf_reg_addr_fields 		),
          REG("RX_ADDR_P4", 	5, &nrf_reg_addr_fields 		),
          REG("RX_ADDR_P5", 	5, &nrf_reg_addr_fields 		),
          REG("TX_ADDR", 		5, &nrf_reg_addr_fields 		),
          REG("RX_PW_P0", 		1, &nrf_reg_rxpw_fields 		),
          REG("RX_PW_P1", 		1, &nrf_reg_rxpw_fields			),
          REG("RX_PW_P2", 		1, &nrf_reg_rxpw_fields			),
          REG("RX_PW_P3", 		1, &nrf_reg_rxpw_fields			),
          REG("RX_PW_P4", 		1, &nrf_reg_rxpw_fields			),
          REG("RX_PW_P5", 		1, &nrf_reg_rxpw_fields			),
          REG("FIFO_STATUS", 	1, &nrf_reg_fifostat_fields 	),
          REG("NA", 			0, 0 							),
          REG("NA", 			0, 0 							),
          REG("NA", 			0, 0 							),
          REG("NA", 			0, 0 							),
          REG("DYNDP", 			1, &nrf_reg_dynpd_fields 		),
          REG("FEATURE", 		1, &nrf_reg_feature_fields 		),
     },
};

unsigned char nrf_get_reg_field(unsigned char reg, unsigned char regf, nrf_reg_buf *buf)
{
     unsigned char i;
     unsigned char byte = 0;
     unsigned char bit  = 0;

     // determine register bit and byte
     for(i = 0; i < regf; i++) {
          bit += nrf_reg_def.data[reg].fields->data[i].size;
          if(bit >= 8) {
               byte++;
               bit -= 8;
          }
     }
     return (buf->data[byte] >> bit & (0b11111111 >> (8 - nrf_reg_def.data[reg].fields->data[regf].size)));
}

void nrf_set_reg_field(unsigned char reg, unsigned char regf, nrf_reg_buf *buf, unsigned char value)
{
     unsigned char i;
     unsigned char byte = 0;
     unsigned char bit  = 0;
     unsigned char mask;

     // dtermine register bit and byte
     for(i = 0; i < regf; i++) {
          bit += nrf_reg_def.data[reg].fields->data[i].size;
          if(bit >= 8) {
               byte++;
               bit -= 8;
          }
     }

     mask = (0b11111111 >> (8 - nrf_reg_def.data[reg].fields->data[regf].size)) << bit;

     buf->data[byte] &= ~mask;
     buf->data[byte] |= ((value << bit) & mask);
}
