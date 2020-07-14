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

#include <string.h>
#include <libdragon.h>
#include "gdbstub.h"

#ifdef __STRICT_ANSI__
#define asm __asm__
#endif

#define NUM_IDT_ENTRIES 32

/*****************************************************************************
 * BSS Data
 ****************************************************************************/
static struct dbg_state dbg_state;

extern volatile uint64_t __baseRegAddr;

static reg_block_t *regs = (reg_block_t*)&__baseRegAddr;

static struct {
	uintptr_t addr;
	uint32_t value;
} orig_op_codes[2];

/*****************************************************************************
 * Misc. Functions
 ****************************************************************************/
static void db_step_set(uint32_t index, uintptr_t addr, uint32_t value)
{
	orig_op_codes[index].addr = addr;
	orig_op_codes[index].value = value;
	*(uint32_t*)addr = value;

	data_cache_index_writeback_invalidate((void*)addr, 4);
	inst_cache_hit_invalidate((void*)addr, 4);
}


void *dbg_sys_memset(void *ptr, int data, size_t len)
{
	char *p = ptr;

	while (len--) {
		*p++ = (char)data;
	}

	return ptr;
}

/*****************************************************************************
 * Interrupt Management Functions
 ****************************************************************************/

void dbg_int_handler_libdragon(exception_t *exception)
{
	uint32_t badvaddr;
	asm volatile ("mfc0 %0, $8\nnop\nnop" : "=r"(badvaddr));

	dbg_state.signum = 5;
	for(int i = 0; i < 32; i++)
	{
		dbg_state.registers[i] = 0xFFFFFFFF00000000 | exception->regs->gpr[i];
	}

	dbg_state.registers[32] = 0xFFFFFFFF00000000 | exception->regs->sr;
	dbg_state.registers[33] = 0xFFFFFFFF00000000 | exception->regs->lo;
	dbg_state.registers[34] = 0xFFFFFFFF00000000 | exception->regs->hi;
	dbg_state.registers[35] = 0xFFFFFFFF00000000 | badvaddr;
	dbg_state.registers[36] = 0xFFFFFFFF00000000 | exception->regs->cr;
	dbg_state.registers[37] = 0xFFFFFFFF00000000 | exception->regs->epc;
	for(int i = 0; i < 32; i++)
	{
		dbg_state.registers[i+38] = exception->regs->fpr[i];
	}
	dbg_state.registers[73] = 0;


	for(int i = 0; i < sizeof(orig_op_codes)/sizeof(*orig_op_codes); i++) {
		if(orig_op_codes[i].addr != 0) {
			uint32_t *addr = (uint32_t*)orig_op_codes[i].addr;

			*addr = orig_op_codes[i].value;

			data_cache_index_writeback_invalidate((void*)addr, 4);
			inst_cache_hit_invalidate((void*)addr, 4);

			orig_op_codes[i].addr = 0;
		}
	}


	dbg_main(&dbg_state);

	for(int i = 0; i < 32; i++)
	{
		regs->gpr[i] = dbg_state.registers[i];
	}
	
	regs->sr = dbg_state.registers[32];
	regs->lo = dbg_state.registers[33];
	regs->hi = dbg_state.registers[34];
	regs->cr = dbg_state.registers[36];
	regs->epc = dbg_state.registers[37];

	asm volatile("move $t0, %0\nnop\nmtc0 $t0, $30\nnop\nmtc0 $t0, $14\nnop\nnop\n" :: "r"(dbg_state.registers[37]) : "%t0");
}

void dbg_init_exception(void)
{
	register_exception_handler(dbg_int_handler_libdragon);
}

/*****************************************************************************
 * Generic serial interface
 ****************************************************************************/

int dbg_serial_getc(void)
{
	/* Wait for data */
	return *((volatile uint32_t *) (0xA0000000 | 0x18000000));
}

int dbg_serial_putchar(int ch)
{
	*((volatile uint32_t *) (0xA0000000 | 0x18000000)) = ch;
	return ch;
}

/*****************************************************************************
 * Debugging System Functions
 ****************************************************************************/

/*
 * Write one character to the debugging stream.
 */
int dbg_sys_putchar(int ch)
{
	return dbg_serial_putchar(ch);
}

/*
 * Read one character from the debugging stream.
 */
int dbg_sys_getc(void)
{
	return dbg_serial_getc() & 0xff;
}

/*
 * Read one byte from memory.
 */
int dbg_sys_mem_readb(address addr, char *val)
{
	*val = *(volatile char *)addr;
	return 0;
}

/*
 * Write one byte to memory.
 */
int dbg_sys_mem_writeb(address addr, char val)
{
	*(volatile char *)addr = val;
	return 0;
}

/*
 * Continue program execution.
 */
int dbg_sys_continue(void)
{
	dbg_state.registers[DBG_REG_PC] += 4;
	return 0;
}

/*
 * Single step the next instruction.
 */
int dbg_sys_step(void)
{
	intptr_t pc = (intptr_t)dbg_state.registers[DBG_REG_PC];
	uint32_t insn = *(uint32_t*)pc;
	intptr_t tnext, fnext;

	tnext = pc + 4;
	fnext = -1;

	if((insn & 0xF8000000) == 0x08000000) { /* J/JAL */
		tnext = (tnext & 0xFFFFFFFFF0000000) | ((insn & 0x03FFFFFF) << 2);
	} else if(0
		|| ((insn & 0xFC0C0000) == 0x04000000) /* B{LT/GE}Z[AL][L] */
		|| ((insn & 0xF0000000) == 0x10000000) /* B{EQ/NE/LE/GT} */
		|| ((insn & 0xF3E00000) == 0x41000000) /* BCz{T/F}[L] */
		|| ((insn & 0xF0000000) == 0x50000000) /* B{EQ/NE/LE/GT}L */
		) {
		fnext = tnext + 4; /* next of branch-delay */
		tnext = tnext + ((intptr_t)(int16_t)insn << 2);
	} else if((insn & 0xFC00003E) == 0x00000008) { /* JR/JALR */
		uint32_t reg = (insn >> 21) & 0x1F;
		tnext = *((int64_t*)((uint32_t)dbg_state.registers[reg]));
	}

	if(fnext != -1) {
		db_step_set(1, fnext, 0x0000000D);
	}
	db_step_set(0, tnext, 0x0000000D);


	dbg_state.registers[DBG_REG_PC] += 4;

	return 0;
}

/*
 * Debugger init function.
 *
 * Installs the exception handler to enable debugging.
 */
void dbg_start(void)
{
	/* Initalize interrupt handler */
	dbg_init_exception();

	/* Interrupt to start debugging. */
	asm volatile ("break");
}
