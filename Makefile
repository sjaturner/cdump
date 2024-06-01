CC=@gcc
CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
%.objdump:%.o
	@objdump --dwarf $^ > $@
parse: parse.c
tags.c: parse test.objdump
	@./parse < test.objdump >> $@
test: test.o dwarf.c tags.c
	@$(CC) $(CFLAGS) $^ -o $@
all: test
tags:
	@ctags -R *
clean:
	@rm -f *.o parse test.objdump core tags test tags.c
indent: # uses more or less the .clang-format from the Linux kernel, but an indent of four spaces
	@clang-format -i *.c *.h 
