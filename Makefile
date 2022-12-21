CC=	cc
CFLAGS=	-O2 -g3 -pipe -Wall -Wextra -Ivm -Iasm -std=c11 -pedantic -D_DEFAULT_SOURCE -Wno-unused-parameter
COMMONS=	vm/vm.o asm/symtab.o
VMOBJS=		vm/main.o $(COMMONS)
ASMOBJS=	asm/asm.o $(COMMONS)
EXEC=	weirdcpu weirdasm
.PHONY: all clean syntax countline
all:	$(EXEC)
weirdcpu:	$(VMOBJS)
	$(CC) $(CFLAGS) $^ -o $@

weirdasm:	$(ASMOBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(VMOBJS) $(ASMOBJS) $(EXEC)

syntax:
	$(CC) $(CFLAGS) -fsyntax-only */*.c */*.h

countline:
	wc -l **/*.c **/*.h
