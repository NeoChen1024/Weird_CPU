/* ========= *\
|| NeoSUBLEQ ||
\* ========= */

/* subleq.c: Virtual Machine */

#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <weirdcpu.h>

mem_t *mem;
pc_t pc = 0;
int debug = 0;
FILE *corefile = NULL;

regs_t regs;

void parsearg(int argc, char **argv)
{
	int opt=0;
	corefile=stdin;
	while((opt = getopt(argc, argv, "hc:d")) != EOF)
	{
		switch(opt)
		{
			case 'h':
				fprintf(stderr, "%s [-h] [-d] [-c corefile]\n", argv[0]);
				exit(0);
				break;
			case 'c':
				if(strcmp("-", optarg))
				{
					if((corefile = fopen(optarg, "r")) == NULL)
					{
						perror(optarg);
						exit(8);
					}
				}
				else
					corefile=stdin;
				readcore(mem, MEMSIZE, corefile);
				break;
			case 'd':
				debug++;
				break;
			default:
				panic("?ARG\n");
				break;
		}
	}
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	mem = calloc(MEMSIZE, sizeof(mem_t));

	parsearg(argc, argv);

	if(debug)
		dumpcore(mem, PAGESIZE, stderr);

	vm_mainloop(&regs, mem, 0, debug, stdin, stdout);

	return 0;
}
