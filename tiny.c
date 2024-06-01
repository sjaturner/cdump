#include "dwarf.h"
#include <stdio.h>

struct bar {
    int a[3];
    char *p;
    int **pp;
};
int x = 456;
int *px = &x;
struct bar foo = {
    .a = { 1, 2, 3 },
    .p = "string",
    .pp = &px,
};

int main()
{
    dump(find("bar"), "foo", &foo);
    return 0;
}
