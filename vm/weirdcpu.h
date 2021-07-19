/* ========= *\
|| NeoSUBLEQ ||
\* ========= */

/* subleq.h: Common Header */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define MEMSIZE	256

typedef uint8_t mem_t;
typedef uint8_t pc_t;

extern mem_t *mem;
extern pc_t pc;

/* Define instruction bitmasks */
#define IOMEM_MASK	0x80
#define HALT_MASK	0x40
#define JUMP_MASK	0x20
#define JC_MASK		0x10
#define IND_MASK	0x02
#define RW_MASK		0x01
#define GET_ALU(x)	((x & 0xC0) >> 2)
#define C_MASK		0x100

#define BITVAL(x)	(x ? 1 : 0)

struct instruction_s
{
	uint8_t iomem	:1;	/* Use I/O memory space */
	uint8_t halt	:1;	/* Halt execution */
	uint8_t jump	:1;	/* Perform jump */
	uint8_t jc	:1;	/* if 1, unconditionally */
	uint8_t alu	:2;	/* ALU functions */
	uint8_t ind	:1;	/* Indirect addressing */
	uint8_t rw	:1;	/* 0 -> Read / 1 -> Write */
};

typedef struct
{
	uint16_t a;
	struct instruction_s i;
	pc_t p;
	pc_t ea;
	size_t cycle; /* Total Cycle Count */
	FILE *in;
	FILE *out;
} regs_t;

enum cpu_cycle
{
	CYCL_LOAD_INST	= 0,
	CYCL_LOAD_EA	= 1,
	CYCL_IND_JMP	= 2,
	CYCL_RW_IO	= 3
};

void panic(char *msg);
void vm_mainloop(regs_t *regs, mem_t *mem, pc_t startpc, int debug, FILE *in, FILE *out);
pc_t readcore(mem_t *mem, size_t memsize, FILE *fd);
void dumpcore(mem_t *mem, size_t memsize, FILE *fd);
