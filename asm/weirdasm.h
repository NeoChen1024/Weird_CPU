#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>

#ifndef WEIRDASM_H

#define MAX_SYM_LEN	16

enum line_types
{
	TYPE_COMMENT,
	TYPE_ASM,
	TYPE_INST,
	TYPE_LABEL,
	TYPE_EQ,
	TYPE_DATA,
	MAX_TYPES
};

static const char line_types_str[MAX_TYPES][MAX_SYM_LEN+1] =
{
	"c",
	"asm",
	"i",
	"lb",
	"eq",
	"db",
};

enum symbol_types
{
	SYM_ADDR_LABEL,
	SYM_VALUE
};

struct symbol_s
{
	char symbol[MAX_SYM_LEN+1];
	enum symbol_types type;

	addr_t data;
};

extern const struct symbol_s reserved_symbols[];

struct address_label_s
{
	char symbol[MAX_SYM_LEN+1];
	addr_t data;
};

#define WEIRDASM_H
#endif