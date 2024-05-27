#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "dwarf.h"
#include "tags.h"

#define BOILERPLATE(X)                                                              \
    void visit_##X(char *name, union tag *tag, struct link *link, unsigned char *p) \
    {                                                                               \
        assert(0);                                                                  \
    }                                                                               \
                                                                                    \
    struct op op_##X = {                                                            \
        visit_##X,                                                                  \
    };

BOILERPLATE(compile_unit);
BOILERPLATE(const_type);
BOILERPLATE(enumeration_type);
BOILERPLATE(enumerator);
BOILERPLATE(formal_parameter);
BOILERPLATE(lexical_block);
BOILERPLATE(member);
BOILERPLATE(subprogram);
BOILERPLATE(subrange_type);
BOILERPLATE(subroutine_type);
BOILERPLATE(union_type);

struct link make_link(union tag *tag, struct link *prev, struct link *this)
{
    struct link link = {
        .prev = prev,
        .tag = tag
    };

    if (prev) {
        prev->next = this;
    }

    return link;
}

void indent(int depth)
{
    while (depth-- > 0) {
        printf("   ");
    }
}

struct link *base(struct link *link, int *depth)
{
    struct link *scan = link;
    int count = 0;

    for (;;) {
        if (!scan->prev) {
            break;
        }

        scan = scan->prev;
        ++count;
    }

    if (depth) {
        *depth = count;
    }

    return scan;
}

void indent_link(struct link *link)
{
    int depth = 0;

    if (link) {
        base(link, &depth);
    }

    indent(depth);
}

void indent_printf(struct link *link, const char *fmt, ...)
{
    va_list ap;

    indent_link(link);

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void visit_array_type(char *name, union tag *tag, struct link *prev, unsigned char *p)
{
    struct link link = make_link(tag, prev, &link);
    struct tag_array_type *tag_array_type = (struct tag_array_type *)tag;
    union tag *scan = tag;
    struct tag_subrange_type *tag_subrange_type = 0;
    union tag *next = taglim.base + tag_array_type->type;

    printf("= {\n");

    do {
        ++scan;
        assert(scan < taglim.base + taglim.nelem);
    } while (!((struct tag_base *)scan)->op);

    assert(((struct tag_base *)scan)->op == &op_subrange_type);

    tag_subrange_type = (struct tag_subrange_type *)scan;

    for (link.val = 0; link.val <= tag_subrange_type->upper_bound; ++link.val) {
        indent_printf(&link, ".[%d] ", link.val);

        ((struct tag_base *)next)->op->visit(name, next, &link, p + link.val * ((struct tag_base *)next)->byte_size);
    }

    indent_printf(prev, "}\n");
}

struct op op_array_type = {
    visit_array_type,
    "array_type",
};

void visit_pointer_type(char *name, union tag *tag, struct link *prev, unsigned char *p)
{
    struct link link = make_link(tag, prev, &link);
    struct tag_pointer_type *tag_pointer_type = (struct tag_pointer_type *)tag;
    union tag *next = taglim.base + tag_pointer_type->type;

    if (p && *p) {
        printf("->");

        ((struct tag_base *)next)->op->visit(name, next, &link, *(unsigned char **)p);
    } else {
        printf("-> 0\n");
    }
}

struct op op_pointer_type = {
    visit_pointer_type,
    "pointer_type",
};

void visit_struct_type(char *name, union tag *tag, struct link *prev, unsigned char *p)
{
    struct link link = make_link(tag, prev, &link);
    union tag *scan = tag;

    printf("= {\n");

    for (;;) {
        do {
            ++scan;
            assert(scan < taglim.base + taglim.nelem);
        } while (!((struct tag_base *)scan)->op);

        if (((struct tag_base *)scan)->op == &op_member) {
            struct tag_member *tag_member = (struct tag_member *)scan;
            union tag *next = taglim.base + tag_member->type;

            indent_printf(&link, ".%s ", tag_member->tag_base.name);
            link.member = scan;
            ((struct tag_base *)next)->op->visit(name, next, &link, p + tag_member->data_member_location);
        } else {
            break;
        }
    }
    indent_printf(prev, "}\n");
}

struct op op_structure_type = {
    visit_struct_type,
    "struct_type",
};

void visit_typedef(char *name, union tag *tag, struct link *prev, unsigned char *p)
{
    struct link link = make_link(tag, prev, &link);
    struct tag_typedef *tag_typedef = (struct tag_typedef *)tag;
    union tag *next = taglim.base + tag_typedef->type;

    ((struct tag_base *)next)->op->visit(name, next, &link, p);
}

struct op op_typedef = {
    visit_typedef,
    "typedef",
};

void visit_variable(char *name, union tag *tag, struct link *prev, unsigned char *p)
{
    struct link link = make_link(tag, prev, &link);
    struct tag_variable *tag_variable = (struct tag_variable *)tag;
    union tag *next = taglim.base + tag_variable->type;

    indent_printf(&link, "%s ", name);
    ((struct tag_base *)next)->op->visit(name, next, &link, p);
}

struct op op_variable = {
    visit_variable,
};

#define RENDER2(FUNC, TYPE, FMT)                   \
    void FUNC(unsigned char *p, struct link *link) \
    {                                              \
        printf(FMT, *(TYPE *)p, *(TYPE *)p);       \
    }

void render_char(unsigned char *p, struct link *link)
{
    union tag *tag = link->prev->tag;

    if (!p) {
        printf("= 0\n");
    } else if (tag->tag_base.op == &op_pointer_type) /* Print it as a string if possible. */
    {
        printf("= \"%s\"\n", (char *)p);
    } else {
        printf("= '%c'\n", *(char *)p);
    }
}

RENDER2(render_unsigned_char, unsigned char, "= %hd 0x%hx\n");
RENDER2(render_short, short, "= %hd 0x%hx\n");
RENDER2(render_short_unsigned, short unsigned, "= %hu 0x%hx\n");
RENDER2(render_int, int, "= %d 0x%x\n");
RENDER2(render_unsigned_int, unsigned int, "= %u 0x%x\n");
RENDER2(render_long, long, "= %ld 0x%lx\n");
RENDER2(render_long_unsigned, long unsigned, "= %lu 0x%lx\n");
RENDER2(render_long_long, long long, "= %lld 0x%llx\n");
RENDER2(render_long_long_unsigned, long long unsigned, "= %lld 0x%llx\n");

#define RENDER1(FUNC, TYPE, FMT)                   \
    void FUNC(unsigned char *p, struct link *link) \
    {                                              \
        printf(FMT, *(TYPE *)p);                   \
    }

RENDER1(render_float, float, "= %f\n");
RENDER1(render_double, double, "= %lf\n");
RENDER1(render_long_double, long double, "= %Lf\n");

void visit_base_type(char *name, union tag *tag, struct link *prev, unsigned char *p)
{
    struct link link = make_link(tag, prev, &link);
    struct tag_base *tag_base = (struct tag_base *)tag;

    struct lookup {
        char *name;
        void (*render)(unsigned char *p, struct link *link);
    };
    struct lookup lookup[] = {
        { "char", render_char },
        { "signed char", render_char },
        { "unsigned char", render_unsigned_char },
        { "short", render_short },
        { "short int", render_short },
        { "short unsigned int", render_short_unsigned },
        { "int", render_int },
        { "unsigned int", render_unsigned_int },
        { "long", render_long },
        { "long int", render_long },
        { "long unsigned int", render_long_unsigned },
        { "long long", render_long_long },
        { "long long int", render_long_long },
        { "long long unsigned int", render_long_long_unsigned },
        { "float", render_float },
        { "double", render_double },
        { "long double", render_long_double },
    };

    for (int i = 0; i < sizeof(lookup) / sizeof(lookup[0]); ++i) {
        if (!strcmp(tag_base->name, lookup[i].name)) {
            lookup[i].render(p, &link);
            break;
        }
    }

    if (0) /* Could probably work out the type from here. */
    {
        int depth = 0;
        struct link *scan = base(&link, &depth);

        for (;;) {
            if (scan == &link) {
                break;
            }
            scan = scan->next;
        }
    }
}

struct op op_base_type = {
    visit_base_type,
    "base_type",
};

union tag *find(char *name)
{
    for (int i = 0; i < taglim.nelem; ++i) {
        struct tag_base *tag_base = (struct tag_base *)(taglim.base + i);
        if (tag_base && tag_base->name && !strcmp(tag_base->name, name)) {
            return tags + i;
        }
    }
    return 0;
}

void dump(union tag *tag, char *name, void *p)
{
    ((struct tag_base *)tag)->op->visit(name, tag, 0, (unsigned char *)p);
}
