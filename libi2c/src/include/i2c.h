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

#ifndef __I2CSLAVE_H_
#define __I2CSLAVE_H_

#define I2C_MAX_ARGS 	5

#define I2C_MAX_RES 	25	

typedef struct {
	/**
 	 * Callback when data is received 
 	 */
	void (*receive)(unsigned char data);

	/**
 	 * Callback when data is requested
 	 */
	void (*transmit)(unsigned char volatile *data);

	/**
 	 * Callback for I2C start condition
 	 */
	void (*start)(void);
} i2c_cb;

/**
 * All arguments from a single command 
 */
typedef struct {
     /**
      * Number of arguments
      */
     unsigned char	count;

     /**
      * The arguments
      */
     unsigned char	args[I2C_MAX_ARGS];
} i2c_cmd_args;

/**
 * Definition of a single command
 */
typedef struct {
     /**
      * Command id
      */
     unsigned char 	cmd;

	 /**
 	  * Number of arguments expecetd by this command
 	  */
	 unsigned char  args;

     /**
      * Function called when executing the commmand
      */
     void (*func)(i2c_cmd_args *args);
} i2c_cmd;

/**
 * All commands knwon
 */
typedef struct {
     /**
      * Number of commands
      */
     unsigned char		count;

     /**
      * The commands
      */
     i2c_cmd			cmds[];
} i2c_cmds;

/**
 * Response 
 */
typedef struct {
     /**
      * Number of data
      */
     unsigned char	count;

    /**
      * Number of data already transmitted
      */
     unsigned char	xmit_count;

     /**
      * Response 
      */
     unsigned char	data[I2C_MAX_RES];
} i2c_cmd_res;

/**
 *
 */
void i2cslave_init(unsigned int addr, i2c_cb *callbacks);

/**
 *
 */
void i2cslave_cmdproc_init(unsigned int add, i2c_cmds *cmds); 

/**
 *
 */
void i2cslave_cmdproc_clrres();

/**
 *
 */
int i2cslave_cmdproc_addres(unsigned char data);

#endif
