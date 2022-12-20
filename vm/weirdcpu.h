/* ====================== *\
|| Weird CPU ISA Emulator ||
\* ====================== */

/* subleq.h: Common Header */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>

#define IOMEMSIZE	(1<<8)
#define MEMSIZE		(1<<16)
#define PAGESIZE	(1<<8)

typedef uint8_t mem_t;
typedef uint16_t addr_t;

extern mem_t *mem;
extern addr_t pc;

#define TRUE	-1
#define FALSE	0

/* Define instruction bitmasks */
#define IOMEM_MASK	0x80
#define ZP_MASK		0x40
#define JUMP_MASK	0x20
#define JCU_MASK	0x10
#define IND_MASK	0x02
#define RW_MASK		0x01
#define GET_ALU(x)	(((x) & 0x0C) >> 2)
#define C_MASK		0x100

#define BITVAL(x)	((x) ? 1 : 0)
#define IOMEM(x)	((x) & 0xFF)

struct instruction_s
{
	uint8_t iomem	:1;	/* Use I/O memory space */
	uint8_t zp	:1;	/* Zero Page Addressing */
	uint8_t jump	:1;	/* Perform jump */
	uint8_t cu	:1;	/* Unconditional Jump or select U register */
	uint8_t alu	:2;	/* ALU functions */
	uint8_t ind	:1;	/* Indirect addressing */
	uint8_t rw	:1;	/* 0 -> Read / 1 -> Write */
};

typedef struct
{
	uint8_t a;
	uint8_t u;
	uint8_t c;	/* Only use 1 bit */
	struct instruction_s i;
	addr_t p;
	addr_t ea;
	size_t cycle; /* Total Cycle Count */
	FILE *in;
	FILE *out;
} regs_t;

enum cpu_cycle
{
	CYCL_LOAD_INST	= 0,
	CYCL_LOAD_EA	= 1,
	CYCL_IND_JMP	= 2,
	CYCL_RW_IO	= 3,
	TOTAL_CYCLES	= 4
};

enum io_return
{
	IO_NRDY	= -1,
	IO_HALT	= -2,
	IO_ERR	= -3
};

#define IO_READ  0
#define IO_WRITE 1

typedef int (*io_handler_t)(int rw, uint8_t addr, uint8_t data);

extern io_handler_t io_handler[IOMEMSIZE];

#define DEF_IO_HANDLER(x) \
	int io_ ## x(int rw, uint8_t addr, uint8_t data)

#define _D_IO(x, addr) [addr] = io_ ## x

void panic(char *fmt, ...);
void vm_mainloop(regs_t *regs, mem_t *mem, addr_t startpc, int debug, FILE *in, FILE *out);
addr_t readcore(mem_t *mem, size_t memsize, FILE *fd);
void dumpcore(mem_t *mem, addr_t start_addr, addr_t size, FILE *fd);
