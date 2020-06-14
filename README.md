# Use DWARF debug records to pretty print C data structures

Some programming languages provide support for pretty printing data
structures. C does not.

This is an attempt to use DWARF debug records to simplify structure
printing. The debug records are parsed from the output of objdump and
you will require an additional build step to incorporate this table into
your application. A small example has been provided which has been tested, 
but only on Debian 10.

The DWARF debug function tries hard to follow pointers in your structures,
unless those pointers are NULL. The '->' token indicates that a pointer
has been dereferenced and followed.

The output looks a bit like JSON.

There may be a better way of doing this, if so please let me know in a PR!

## Implementation

The code is a bit overfussy. There's a visitor function which walks
through the debug records - and at the same time through the memory of
your application.  Decorative indents and braces are inserted on the
way and values are printed out when a base type is reached. A linked
list is maintained so that the base type can figure out the path which
it took. I don't use that at present.  I expect that the entire thing
could be a lot simpler.

## Example 

Here's a code snippet and the resulting output:

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

Outputs:

    bb = {
       .a = {
          .d = 1.230000
          .y = 3 0x3
       }
       .er = 456 0x1c8
       .p ->= {
             .d = 3.140000
             .y = 1 0x1
          }
    }

## Building

You will probably need build-essential at the very least. So ...

    sudo get update && apt-get install build-essential

Then try:

    make clean all

## Running

    ./test

## Limitations

I've hardly tested it at all.
