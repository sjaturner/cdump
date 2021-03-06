#include "dwarf.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

char foo[] = "foo";

struct xs
{
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

void bigger()
{
   int array[4] = { 1, 2, 4, 8 };
   char *bar = "bar";
   float pi = 3.14;
   uint32_t ui = 23;

   DWARF(mxs);
   DWARF(array);
   DWARF(bar);
   DWARF(pi);
   DWARF(ui);
}

void simple()
{
   struct a
   {
      double d;
      short y;
   };
   struct a aa = { 3.14, 1 };
   struct b
   {
      struct a a;
      int er;
      struct a *p;
   } bb = {
      .a.d = 1.23,
      .a.y = 3,
      .er = 456,
      .p = &aa,
   };

   DWARF(bb);
}

int main()
{
   bigger();
   simple();
   return 0;
}
