/*
 * Copyright (c) 2016-2019 Matt Borgerson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _GDBSTUB_SYS_H_
#define _GDBSTUB_SYS_H_
#include <stdio.h>
#include <stdint.h>
#include <libdragon.h>

/*****************************************************************************
 * Defines
 ****************************************************************************/
#define DBG_MIPS_NUM_GPRS 32
#define DBG_MIPS_NUM_CP0 8
#define DBG_MIPS_NUM_REG_DUMP 90 
#define DBG_MIPS_NUM_REGISTER 90
#define DBG_MIPS_REGISTER_TYPE uint64_t

#define DBG_REG_PC 37

/*****************************************************************************
 * Types
 ****************************************************************************/
typedef unsigned int address;
typedef DBG_MIPS_REGISTER_TYPE reg;

struct dbg_state {
	int signum;
	reg registers[DBG_MIPS_NUM_REGISTER];
};

/*****************************************************************************
 * External Functions
 ****************************************************************************/
 
extern void dbg_int_handler_common(void);

/*****************************************************************************
 * Prototypes
 ****************************************************************************/
void dbg_start(void);
void *dbg_sys_memset(void *ptr, int data, size_t len);

#endif
