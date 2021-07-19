/* ========= *\
|| NeoSUBLEQ ||
\* ========= */

/* vm.c: Virtual Machine Backend */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <weirdcpu.h>

void panic(char *msg)
{
	fputs(msg, stderr);
	exit(100);
}

pc_t readcore(mem_t *mem, size_t memsize, FILE *fd)
{
	size_t ptr=0;
	while(fscanf(fd, "%hhx ", mem + (ptr++)) > 0 && ptr < memsize);
	return (pc_t)ptr - 1;
}

void dumpcore(mem_t *mem, size_t memsize, FILE *fd)
{
	size_t ptr=0;
	fputs("================== COREDUMP ===================", fd);
	for(ptr=0; ptr < memsize; ptr++)
	{
		if(ptr % 16 == 0)
			putc('\n', fd);
		fprintf(fd, "%02hhx ", mem[ptr]);
	}
	putc('\n', fd);
	fputs("=============== END OF COREDUMP ===============\n", fd);
}

/* a should be A register value */
int alu_calc(uint8_t func, mem_t a, mem_t b)
{
	switch(func)
	{
		case 0:
			return a;
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
		default:
			return -1;
			break;
	}
}

int readmem(pc_t addr, mem_t *mem, int iomem)
{
	if(iomem)
	{
		return -1;
	}
	else
	{
		return mem[addr];
	}
}

int writemem(pc_t addr, mem_t *mem, int iomem, mem_t data)
{
	if(iomem)
	{
		return -1;
	}
	else
	{
		mem[addr] = data;
	}
	return 0;
}

void vm_mainloop(regs_t *regs, mem_t *mem, pc_t startpc, int debug, FILE *in, FILE *out)
{
	int halt = 0;
	int state = 0;

	regs->p = startpc;
	regs->in = in;
	regs->out = out;

	while(halt == 0)
	{
		switch(regs->cycle % 4)
		{
			case CYCL_LOAD_INST:
				regs->i.iomem = BITVAL(mem[regs->p] & IOMEM_MASK);
				regs->i.halt  = BITVAL(mem[regs->p] &  HALT_MASK);
				regs->i.jump  = BITVAL(mem[regs->p] &  JUMP_MASK);
				regs->i.jc    = BITVAL(mem[regs->p] &    JC_MASK);
				regs->i.ind   = BITVAL(mem[regs->p] &   IND_MASK);
				regs->i.rw    = BITVAL(mem[regs->p++] &  RW_MASK);
				if(regs->i.halt) halt = 1;
				break;
			case CYCL_LOAD_EA:
				regs->ea = mem[regs->p];
				break;
			case CYCL_IND_JMP:
				regs->p++; /* advance P first */
				if(regs->i.jump)
				{
					if(regs->i.jc)
					{
						regs->p = regs->i.ind ? mem[regs->ea] : regs->ea;
					}
					else
					{
						if(!(regs->a & C_MASK)) /* Jump if C is not set */
							regs->p = regs->i.ind ? mem[regs->ea] : regs->ea;
					}
				}
				else if(regs->i.ind)
					regs->ea = mem[regs->ea];
				break;
			case CYCL_RW_IO:
				if(regs->i.rw)
				{	/* Read */
					regs->a = (state = readmem(regs->ea, mem, regs->i.iomem));
					if(state < 0)
						panic("?READMEM");
				}
				else
				{	/* Write */
					state = writemem(regs->ea, mem, regs->i.iomem, regs->a);
					if(state < 0)
						panic("?WRITEMEM");
				}
				break;
		}
		
		if(debug)
		{
			fprintf(stderr, ">> cycle %zu -> %zu, A = %04hx, P = %02hhx, EA = %02hhx\n",
					regs->cycle, regs->cycle % 4, regs->a, regs->p, regs->ea);
		}
		regs->cycle++;
	}
}
