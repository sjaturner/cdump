#include "dwarf.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

char foo[] = "foo";

struct xs {
    int xa;
    int xb;
    int xc[3];
    struct xs *xd;
    int *xe;
    char *pc;
};

int mi = 7;

struct xs nxs = {
    .xb = 3,
    .pc = foo,
};

struct xs mxs = {
    .xa = 1,
    .xb = 2,
    .xc = {
        3,
        4,
        5,
    },
    .xd = &nxs,
    .xe = &mi,
};

int main()
{
    dump(find("xs"), "mxs", &mxs);

    int first = 1;
    int second = 2;
    struct foo {
        int a;
        int b;
    } foo = {
        .a = 11,
        .b = 22,
    };

    struct debug {
        typeof(second) second;
        typeof(foo) foo;
    } debug = {
        .second = second,
        .foo = foo,
    };
    dump(find("debug"), "debug", &debug);
    dump(find("int"), "mi", &mi);
    return 0;
}
