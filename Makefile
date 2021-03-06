CC=gcc
CFLAGS=-g -Wall
test.o:test.c
test.objdump:test.o
	objdump --dwarf test.o > test.objdump
parse: parse.c
tags.h: parse test.objdump
	./parse < test.objdump > $@
test: dwarf.c test.c tags.h 
	$(CC) $(CFLAGS) dwarf.c test.c -o $@
all: test
tags:
	ctags -R *
clean:
	@rm -f *.o parse tags.h test.o test.objdump core tags test
