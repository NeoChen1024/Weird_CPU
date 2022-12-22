#include "../vm/weirdcpu.h"
#include "weirdasm.h"
#include "symtab.h"

mem_t mem[MEMSIZE];
FILE *srcfile;
FILE *corefile;
int debug = 0;
char line[PATH_MAX];

addr_t last_address = 0; // This tracks the last address used by the assembler

struct parsed_line_s
{
	enum line_types type;
	char op[MAX_SYM_LEN+1];
	char arg[MAX_SYM_LEN+1];
};

struct parsed_line_s *parsed_lines;
size_t parsed_lines_alloc = 5;
size_t parsed_lines_size = 0;

struct parsed_line_s parseline(char *text)
{
	struct parsed_line_s line;
	char t[MAX_SYM_LEN];
	sscanf(text, "%16s\t%16s\t%16s", t, line.op, line.arg);

	for(int i = 0; i < MAX_TYPES; i++)
	{
		if(strcmp(t, line_types_str[i]) == 0)
		{
			line.type = i;
			goto match;
		}

	}
	panic("Invalid Directive Type \"%s\"\n", t);

match:
	return line;
}

int ishex(char *str)
{
	for(unsigned int i = 0; i < strlen(str); i++)
	{
		if(!isxdigit(str[i]))
			return FALSE;
	}
	return TRUE;
}

int str2hexbyte(char * str, addr_t *data)
{
	if(str[0] != '$' || !ishex(str + 1))
	{
		return FALSE;
	}

	sscanf(str+1, "%hx", data);
	return TRUE;
}

int isprefixes(char c)
{
	return c == '$' || c == '@' || c == '%' || c == '&' || c == '=';
}

addr_t argparse(char *arg)
{
	addr_t ret = 0;

	// Try to parse as hex first
	if(str2hexbyte(arg, &ret))
	{
		return ret;
	}

	// Check if prefix exists
	if(isprefixes(arg[0]))
	{
		arg++;
	}
	if(symbol_find(arg, SYM_VALUE, &ret))
	{
		return ret;
	}
	else if(symbol_find(arg, SYM_LABEL, &ret))
	{
		return ret;
	}
	else
	{
		panic("Symbol \"%s\" not found", arg);
	}

	// unreachable
	assert(0);
	return 0;
}

void emit_inst(mem_t *mem, addr_t current_address, struct parsed_line_s *line)
{
	addr_t op;
	addr_t arg;

	if(!symbol_find(line->op, SYM_VALUE, &op))
	{
		panic("Invalid instruction \"%s\", %s\n", line->op, line->arg);
	}

	switch(line->arg[0])
	{
		case '$':
			break;
		case '@':
			op |= IND_MASK;
			break;
		case '%':
			op |= JCU_MASK;
			break;
		case '&':
			op |= JCU_MASK | IND_MASK;
			break;
		case '=':
			op |= IOMEM_MASK;
			break;
	}

	arg = argparse(line->arg);

	if(line->arg[0] == '$')
	{
		*(mem + 1) = arg;
	}
	else
	{
		if((current_address & 0xff00) != (arg & 0xff00)) // Page boundary crossed
		{
			if((arg >> 8) == 0 && (op & IOMEM_MASK) == 0)
			{
				op |= ZP_MASK;
				*(mem + 1) = arg & 0xff;
			}
			else
			{
				panic("Page boundary crossed");
			}
		}
		else
		{
			*(mem + 1) = arg & 0xff;
		}
	}
	
	*mem = op;
}

void asm_handler(struct parsed_line_s *line, addr_t *current_address)
{
	if(strcmp(line->op, "org") == 0)
	{
		*current_address = argparse(line->arg);
	}
	else
		panic("Invalid directive \"%s\"\n", line->op);
}

int first_pass(struct parsed_line_s *line)
{
	static addr_t current_address = 0;

	switch(line->type)
	{
		case TYPE_COMMENT:
			break;
		case TYPE_ASM:
			asm_handler(line, &current_address);
			break;
		case TYPE_INST:
			current_address+=2;
			break;
		case TYPE_LABEL:
			printf("Label: %#hx (%s)\n", current_address, line->op);
			symbol_add(line->op, SYM_LABEL, current_address);
			break;
		case TYPE_EQ:
			symbol_add(line->op, SYM_VALUE, argparse(line->arg));
			break;
		case TYPE_DATA:
			printf("Data: %#hx (%s)\n", current_address, line->op);
			symbol_add(line->op, SYM_LABEL, current_address);
			current_address+=1;
			break;
	}

	if(current_address >= last_address)
		last_address = current_address;

	return TRUE;
}

int second_pass(struct parsed_line_s *line)
{
	static addr_t current_address = 0;

	switch(line->type)
	{
		case TYPE_COMMENT:
			break;
		case TYPE_ASM:
			asm_handler(line, &current_address);
			break;
		case TYPE_INST:
			emit_inst(&mem[current_address], current_address, line);
			current_address+=2;
			break;
		case TYPE_LABEL:
			break;
		case TYPE_EQ:
			break;
		case TYPE_DATA:
			mem[current_address] = argparse(line->arg);
			current_address+=1;
			break;
	}

	if(current_address >= last_address)
		last_address = current_address;

	return TRUE;
}

void parsearg(int argc, char **argv)
{
	int opt=0;
	srcfile = stdin;
	corefile = stdout;
	while((opt = getopt(argc, argv, "hi:o:d")) != EOF)
	{
		switch(opt)
		{
			case 'h':
				fprintf(stderr, "%s [-h] [-d] [-i asm src] [-o corefile]\n", argv[0]);
				exit(0);
				break;
			case 'o':
				if(strcmp("-", optarg))
				{
					if((corefile = fopen(optarg, "wb")) == NULL)
					{
						perror(optarg);
						exit(8);
					}
				}
				else
					corefile=stdin;
				break;
			case 'i':
				if(strcmp("-", optarg))
				{
					if((srcfile = fopen(optarg, "r")) == NULL)
					{
						perror(optarg);
						exit(8);
					}
				}
				else
					srcfile = stdin;
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
	parsearg(argc, argv);

	parsed_lines = calloc(parsed_lines_alloc, sizeof(struct parsed_line_s));
	assert(parsed_lines != NULL);

	while(fgets(line, PATH_MAX, srcfile) != NULL)
	{
		parsed_lines[parsed_lines_size] = parseline(line);

		first_pass(&parsed_lines[parsed_lines_size]);

		parsed_lines_size++;

		if(parsed_lines_size >= parsed_lines_alloc)
		{
			parsed_lines = reallocarray(parsed_lines, parsed_lines_alloc *= 2, sizeof(struct parsed_line_s));
			assert(parsed_lines != NULL);
		}
	}

	for(size_t i = 0; i < parsed_lines_size; i++)
	{
		second_pass(&parsed_lines[i]);
	}

	dumpcore(mem, 0, last_address, corefile);

	puts("====== SYMBOLS ======");
	symbol_dump();

	return 0;
}
