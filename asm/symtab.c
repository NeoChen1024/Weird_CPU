#include "../vm/weirdcpu.h"
#include "weirdasm.h"
#include "symtab.h"

#define ARRAYSIZE(x) \
	(sizeof(x)/sizeof(x[0]))

const struct symbol_s reserved_symbols[] =
{
	{"HALT",	SYM_LABEL, 0x00},
	{"DISP",	SYM_LABEL, 0xFD},
	{"rd",		SYM_VALUE, 0x00},
	{"add",		SYM_VALUE, 0x04},
	{"nand",	SYM_VALUE, 0x08},
	{"inv",		SYM_VALUE, 0x0C},
	{"wr",		SYM_VALUE, 0x01},
	{"jp",		SYM_VALUE, 0x30},
	{"jc",		SYM_VALUE, 0x20}
};

static struct symbol_s *user_symbols;
static size_t user_symbols_size = 0;
static size_t user_symbols_alloc = 5;

int symbol_find(char *name, enum symbol_types type, addr_t *data)
{
	// Search for reserved symbols first
	for(size_t i = 0; i < ARRAYSIZE(reserved_symbols); i++)
	{
		if(strcmp(name, reserved_symbols[i].symbol) == 0 &&
			reserved_symbols[i].type == type)
		{
			if(data != NULL)
				*data = reserved_symbols[i].data;
			return TRUE;
		}
	}

	// Search in user defined symbols
	for(size_t i = 0; i < user_symbols_size; i++)
	{
		if(strcmp(name, user_symbols[i].symbol) == 0 &&
			reserved_symbols[i].type == type)
		{
			if(data != NULL)
				*data = user_symbols[i].data;
			return TRUE;
		}
	}

	return FALSE;
}

int symbol_add(char *name, enum symbol_types type, addr_t data)
{
	// First use initialization
	if(user_symbols == NULL)
	{
		user_symbols_alloc = 8;
		user_symbols_size = 0;
		user_symbols = calloc(user_symbols_alloc, sizeof(struct symbol_s));
		assert(user_symbols != NULL);
	}

	strcpy(user_symbols[user_symbols_size].symbol, name);
	user_symbols[user_symbols_size].type = type;
	user_symbols[user_symbols_size].data = data;

	user_symbols_size++;
	
	if(user_symbols_size >= user_symbols_alloc)
	{
		printf("Symbol table out of space, current size: %zd\n", user_symbols_size);
		user_symbols = reallocarray(user_symbols, user_symbols_alloc += 1024, sizeof(struct symbol_s));
		assert(user_symbols != NULL);
	}
	
	return TRUE;
}

void symbol_dump(void)
{
	puts("Symbols:");
	for(size_t i = 0; i < user_symbols_size; i++)
	{
		printf("%s\t== (%s)\t%#hx\n", user_symbols[i].symbol,
			user_symbols[i].type == SYM_LABEL ? "label" : "value",
			user_symbols[i].data);
	}
}