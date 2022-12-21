#ifndef SYMTAB_H

#include "../vm/weirdcpu.h"
#include "weirdasm.h"

int symbol_find(char *name, enum symbol_types type, addr_t *data);
int symbol_add(char *name, enum symbol_types type, addr_t data);
void symbol_dump(void);

#define SYMTAB_H
#endif