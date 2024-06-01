CC=gcc
CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
test.objdump:test.o
	objdump --dwarf test.o > test.objdump
parse: parse.c
tags.c: parse test.objdump
	echo "#include \"dwarf.h\"" > $@
	./parse < test.objdump >> $@
test: test.o dwarf.c tags.c
	$(CC) $(CFLAGS) $^ -o $@
all: test
tags:
	ctags -R *
clean:
	@rm -f *.o parse test.objdump core tags test tags.c
indent:
	clang-format -i *.c *.h
