CC=	cc
CFLAGS=	-O3 -g3 -gdwarf-5 -pipe -Wall -Wextra -I. -std=c99 -pedantic -D_DEFAULT_SOURCE
VMOBJS=	vm/vm.o vm/main.o
EXEC=	weirdcpu
.PHONY: all clean syntax countline
all:	$(EXEC)
weirdcpu:	$(VMOBJS)
	$(CC) $(LDFLAGS) $^ $(.ALLSRC) -o weirdcpu

clean:
	rm -rf $(VMOBJS) $(EXEC)

syntax:
	$(CC) $(CFLAGS) -fsyntax-only *.c *.h

countline:
	wc -l *.c *.h *.l
