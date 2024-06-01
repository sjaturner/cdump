# Use DWARF Debug Records to Pretty Print C data Structures

Some programming languages (Rust, for example) provide support for pretty
printing data structures. C does not, which is a shame.

This is an attempt to use DWARF debug records to simplify structure
printing. The debug records are parsed from the output of objdump and you
will require an additional build step to incorporate this table into your
application. A small example has been provided which has been tested,
but only on Debian 12.

The output is nearly JSON, but I have not escaped the strings.

It looks as if the output works with jq and gron so you can drill into
the output using those tools.

There may be a better way of doing this, if you know of one then tell me!

## Implementation

The code is a bit over-fussy. There's a visitor function which walks
through the debug records - and at the same time through the memory of
your application.

Output to other formats would be nice. I'd originally intended that this
would C make initialisers for structures but that looks a bit more tricky.

The machinery tries to follow pointers so I guess linked structures
might get out of control, there's no depth stop.

If you generate a file with the debug tags in, then link it and dwarf.c
to your application then you should be able to dump structures. I used
weak linkage so if you omit the tags file the debug functions ought to
do nothing.

## Example

There's a test.c example in this directory, showing a couple of simple
cases.

```
:; make clean test
:; ./test
{"mxs":{"xa":1,"xb":2,"xc":[3,4,5],"xd":{"*":{"xa":0,"xb":3,"xc":[0,0,0],"xd":null,"xe":null,"pc":{"*":"foo"}}},"xe":{"*":7},"pc":null}}
{"debug":{"second":2,"foo":{"a":11,"b":22}}}
{"mi":7}
```

## Building

You will probably need build-essential at the very least. So ...

    sudo get update && apt-get install build-essential

Then try:

    make clean all

## Running

    ./test

## Limitations

Really just a proof of concept but I intend to use this code in real
applications so hopefully it'll improve with time. Send a PR if you have
any suggestions or improvements.

## Next

I think that I'll adapt this so that I can make programs which look at
global structures in running processes using process\_vm\_readv. The
easiest way to use that function on a modern Linux is to prefix the
program with "setarch -R" to sidestep the address space randomisation
(acceptable for debug). I might even try to turn this the other way
round and provide a mechanism for modifying global state.

# Additional Worked Example

Here's a small C program defining a none-trivial structure.

```C
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
```

Paste this into the command line to dump the foo variable:

```bash
make clean parse                            # This builds the tool which parses the output from objdump
gcc -Wall -g -c tiny.c                      # Compile the tiny.c file to get an object
objdump --dwarf tiny.o | ./parse > tags.c   # Make the tags used for debug
gcc tiny.o dwarf.c tags.c -o tiny           # Link it all together, the target file, the code to dump data structures and the tag information.

```

Now run the tiny example and pipe the output through jq, just to make it a bit simpler to read:

```bash
./tiny | jq
```

Which yields:

```JSON
{
  "foo": {
    "a": [
      1,
      2,
      3
    ],
    "p": {
      "*": "string"
    },
    "pp": {
      "*": {
        "*": 456
      }
    }
  }
}
```

