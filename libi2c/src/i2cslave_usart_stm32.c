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

#include "i2c.h"

static i2c_cb *i2c_callbacks;

void i2cslave_init(unsigned int addr, i2c_cb *callbacks)
{
     i2c_callbacks = callbacks;
}

void i2cslave_cmdproc_init(unsigned int add, i2c_cmds *cmds) 
{
	/* TODO */
}

void i2cslave_cmdproc_clrres()
{
	/* TODO */
}

int i2cslave_cmdproc_addres(unsigned char data)
{
	/* TODO */

	return 0;
}
