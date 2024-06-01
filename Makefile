CC=gcc
CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
test.objdump:test.o
	objdump --dwarf test.o > test.objdump
parse: parse.c
tags.c: parse test.objdump
	echo "#include \"dwarf.h\"" > $@
	./parse < test.objdump >> $@
test: dwarf.c test.c tags.c
	$(CC) $(CFLAGS) dwarf.c test.c tags.c -o $@
all: test
tags:
	ctags -R *
clean:
	@rm -f *.o parse tags.h test.o test.objdump core tags test
indent:
	clang-format -i *.c *.h
