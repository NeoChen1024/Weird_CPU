/* ====================== *\
|| Weird CPU ISA Emulator ||
\* ====================== */

/* vm.c: Virtual Machine Backend */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "weirdcpu.h"

io_handler_t io_handler[IOMEMSIZE];

void panic(char *fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

void debug_info(int debug, char *fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	if(debug)
		vfprintf(stderr, fmt, args);
	va_end(args);
}

addr_t readcore(mem_t *mem, size_t memsize, FILE *fd)
{
	size_t ptr=0;
	while(fscanf(fd, "%hhx ", mem + (ptr++)) > 0 && ptr < memsize);
	return (addr_t)ptr - 1;
}

void dumpcore(mem_t *mem, addr_t start_addr, addr_t size, FILE *fd)
{
	size_t ptr=0;
	fputs("================== COREDUMP ===================", fd);
	for(ptr = start_addr; ptr < start_addr + size; ptr++)
	{
		if(ptr % 16 == 0)
		{
			putc('\n', fd);
			fprintf(fd, "%04zx: ", ptr);
		}
		fprintf(fd, "%02hhx ", mem[ptr]);
	}
	putc('\n', fd);
	fputs("=============== END OF COREDUMP ===============\n", fd);
}

/* a should be A register value */
uint16_t alu_calc(uint8_t func, mem_t a, mem_t b)
{
	switch(func)
	{
		case 0:
			return b;
			break;
		case 1:
			return b + a;
			break;
		case 2:
			return ~(b & a);
			break;
		case 3:
			return ~b;
			break;
	}
	panic("?ALU");
	return 0;
}

/* MMIO handlers */

DEF_IO_HANDLER(halt)
{
	return IO_HALT;
}

DEF_IO_HANDLER(tty)
{
	int c;
	if(rw == IO_READ)
		c = getc(stdin);
	else if(rw == IO_WRITE)
		c = putc(data, stdout);

	if(c == EOF)
		return IO_NRDY;	/* I/O not ready */
	else
		return c;

	return IO_ERR;
}

io_handler_t io_handler[IOMEMSIZE] =
{
	_D_IO(halt, 0x00),
	_D_IO(tty, 0xFF)
};

int readmem(addr_t addr, mem_t *mem, int iomem)
{
	int ret = 0;

	if(iomem)
	{
		if(io_handler[IOMEM(addr)] != NULL)
		{
			ret = io_handler[IOMEM(addr)](IO_READ, IOMEM(addr), 0);
			if(ret == IO_ERR)
				panic("?IO_ERR: READ addr = %hhx", IOMEM(addr));
		}

		return ret;
	}
	else
	{
		return mem[addr];
	}
}

int writemem(addr_t addr, mem_t *mem, int iomem, mem_t data)
{
	int ret = 0;

	if(iomem)
	{
		if(io_handler[IOMEM(addr)] != NULL)
		{
			ret = io_handler[IOMEM(addr)](IO_WRITE, IOMEM(addr), data);
			if(ret == IO_ERR)
				panic("?IO_ERR: WRITE addr = %hhx, data = %hhx", IOMEM(addr), data);
			else if(ret == IO_NRDY)
			{
			}
		}

		return ret;
	}
	else
	{
		mem[addr] = data;
	}
	return 0;
}

void vm_mainloop(regs_t *regs, mem_t *mem, addr_t startpc, int debug, FILE *in, FILE *out)
{
	int halt = 0;
	int state = 0;

	regs->p = startpc;
	regs->in = in;
	regs->out = out;

	mem_t inst = 0;

	uint16_t alu_temp;

	while(halt == 0)
	{
		if(debug)
			usleep(20000);
		switch(regs->cycle % TOTAL_CYCLES)
		{
			case CYCL_LOAD_INST:
				inst = mem[regs->p];
				regs->i.iomem = BITVAL(mem[regs->p] & IOMEM_MASK);
				regs->i.zp    = BITVAL(mem[regs->p] &    ZP_MASK);
				regs->i.jump  = BITVAL(mem[regs->p] &  JUMP_MASK);
				regs->i.cu    = BITVAL(mem[regs->p] &	JCU_MASK);
				regs->i.alu   = GET_ALU(mem[regs->p]);
				regs->i.ind   = BITVAL(mem[regs->p] &   IND_MASK);
				regs->i.rw    = BITVAL(mem[regs->p++] &  RW_MASK);
				break;
			case CYCL_LOAD_EA:
				regs->ea = mem[regs->p];
				break;
			case CYCL_IND_JMP:
				regs->p++; /* advance P first */
				if(regs->i.jump)
				{
					if(regs->i.cu)
					{
						regs->p = regs->i.ind ? mem[regs->ea] : regs->ea;
					}
					else
					{
						if(!regs->c) /* Jump if C is not set */
							regs->p = regs->i.ind ? mem[regs->ea] : regs->ea;
					}
				}
				else if(regs->i.ind)
					regs->ea = mem[regs->ea];
				break;
			case CYCL_RW_IO:
				if(regs->i.jump == 0)
				{
					if(regs->i.rw == 0)
					{	/* Read */
						alu_temp = alu_calc(regs->i.alu, regs->a,
								(state = readmem(regs->ea, mem, regs->i.iomem)));
						regs->c = (state + regs->a) >> 8; /* Update C value */

						if(regs->i.cu)
							regs->u = (mem_t)alu_temp;
						else
							regs->a = (mem_t)alu_temp;
					}
					else
					{	/* Write */
						state = writemem(regs->ea, mem, regs->i.iomem, regs->a);
						if(debug)
							dumpcore(mem, (regs->ea & 0xFF00), PAGESIZE, stderr);
					}

					if(state == IO_HALT)
						halt = TRUE;

				}
				break;
		}
		
		if(debug)
		{
			fprintf(stderr, ">> cycle %zu (%zu), I=%02hhx: A=%02hhx, U=%02hhx, C=%01hhx, MEM[PC = %04hx]=%02hhx, MEM[EA = %04hx]=%02hhx\n",
					regs->cycle, regs->cycle % 4, inst, regs->a, regs->u, regs->c ? 1 : 0, regs->p, mem[regs->p], regs->ea, mem[regs->ea]);
		}

		regs->cycle++;
	}
}
