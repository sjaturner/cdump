#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "dwarf.h"

struct taglim *get_taglim(void) __attribute__((weak));
struct taglim *get_taglim(void)
{
    return 0;
}

#define BOILERPLATE(X)                                                                        \
    void visit_##X(char *name, union tag *tag, struct link *link, unsigned char *p, int more) \
    {                                                                                         \
        assert(0);                                                                            \
    }                                                                                         \
                                                                                              \
    struct op op_##X = {                                                                      \
        visit_##X,                                                                            \
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

struct link make_link(union tag *tag, struct link *prev)
{
    struct link link = {
        .prev = prev,
        .tag = tag
    };

    return link;
}

void visit_array_type(char *name, union tag *tag, struct link *prev, unsigned char *p, int more)
{
    struct taglim *taglim = get_taglim();
    if (!taglim) {
        return;
    }
    struct link link = make_link(tag, prev);
    struct tag_array_type *tag_array_type = (struct tag_array_type *)tag;
    union tag *scan = tag;
    struct tag_subrange_type *tag_subrange_type = 0;
    union tag *next = taglim->base + tag_array_type->type;

    printf("[");

    do {
        ++scan;
        assert(scan < taglim->base + taglim->nelem);
    } while (!((struct tag_base *)scan)->op);

    assert(((struct tag_base *)scan)->op == &op_subrange_type);

    tag_subrange_type = (struct tag_subrange_type *)scan;

    for (link.val = 0; link.val <= tag_subrange_type->upper_bound; ++link.val) {
        ((struct tag_base *)next)->op->visit(name, next, &link, p + link.val * ((struct tag_base *)next)->byte_size, link.val != tag_subrange_type->upper_bound);
    }

    printf("]%s", more ? "," : "");
}

struct op op_array_type = {
    visit_array_type,
    "array_type",
};

void visit_pointer_type(char *name, union tag *tag, struct link *prev, unsigned char *p, int more)
{
    struct taglim *taglim = get_taglim();
    if (!taglim) {
        return;
    }
    struct link link = make_link(tag, prev);
    struct tag_pointer_type *tag_pointer_type = (struct tag_pointer_type *)tag;
    union tag *next = taglim->base + tag_pointer_type->type;

    if (p && *p) {
        printf("{\"*\":");
        ((struct tag_base *)next)->op->visit(name, next, &link, *(unsigned char **)p, 0);
        printf("}%s", more ? "," : "");
    } else {
        printf("null%s", more ? "," : "");
    }
}

struct op op_pointer_type = {
    visit_pointer_type,
    "pointer_type",
};

void visit_struct_type(char *name, union tag *tag, struct link *prev, unsigned char *p, int more)
{
    struct taglim *taglim = get_taglim();
    if (!taglim) {
        return;
    }
    struct link link = make_link(tag, prev);
    union tag *scan = 0;
    unsigned int count = 0;

    printf("{");

    for (scan = tag;;) {
        do {
            ++scan;
            assert(scan < taglim->base + taglim->nelem);
        } while (!((struct tag_base *)scan)->op);

        if (((struct tag_base *)scan)->op == &op_member) {
            ++count;
        } else {
            break;
        }
    }

    for (scan = tag;;) {
        do {
            ++scan;
            assert(scan < taglim->base + taglim->nelem);
        } while (!((struct tag_base *)scan)->op);

        if (((struct tag_base *)scan)->op == &op_member) {
            struct tag_member *tag_member = (struct tag_member *)scan;
            union tag *next = taglim->base + tag_member->type;

            printf("\"%s\":", tag_member->tag_base.name);
            link.member = scan;
            ((struct tag_base *)next)->op->visit(name, next, &link, p + tag_member->data_member_location, !!--count);
        } else {
            break;
        }
    }
    printf("}%s", more ? "," : "");
}

struct op op_structure_type = {
    visit_struct_type,
    "struct_type",
};

void visit_typedef(char *name, union tag *tag, struct link *prev, unsigned char *p, int more)
{
    struct taglim *taglim = get_taglim();
    if (!taglim) {
        return;
    }
    struct link link = make_link(tag, prev);
    struct tag_typedef *tag_typedef = (struct tag_typedef *)tag;
    union tag *next = taglim->base + tag_typedef->type;

    ((struct tag_base *)next)->op->visit(name, next, &link, p, more);
}

struct op op_typedef = {
    visit_typedef,
    "typedef",
};

void visit_variable(char *name, union tag *tag, struct link *prev, unsigned char *p, int more)
{
    struct taglim *taglim = get_taglim();
    if (!taglim) {
        return;
    }
    struct link link = make_link(tag, prev);
    struct tag_variable *tag_variable = (struct tag_variable *)tag;
    union tag *next = taglim->base + tag_variable->type;

    printf("\"%s\":", name);
    ((struct tag_base *)next)->op->visit(name, next, &link, p, 1);
}

struct op op_variable = {
    visit_variable,
};

void render_char(unsigned char *p, struct link *link, int more)
{
    union tag *tag = link->prev->tag;

    if (!p) {
        printf("null%s", more ? "," : "");
    } else if (tag->tag_base.op == &op_pointer_type) {
        /* Print it as a string if possible. */
        printf("\"%s\"%s", (char *)p, more ? "," : "");
    } else {
        printf("%d%s", *(char *)p, more ? "," : "");
    }
}

#define RENDER(FUNC, TYPE, FMT)                              \
    void FUNC(unsigned char *p, struct link *link, int more) \
    {                                                        \
        printf(FMT, *(TYPE *)p, more ? "," : "");            \
    }

RENDER(render_unsigned_char, unsigned char, "%d%s");
RENDER(render_short, short, "%hd%s");
RENDER(render_short_unsigned, short unsigned, "%hu%s");
RENDER(render_int, int, "%d%s");
RENDER(render_unsigned_int, unsigned int, "%u%s");
RENDER(render_long, long, "%ld%s");
RENDER(render_long_unsigned, long unsigned, "%lu%s");
RENDER(render_long_long, long long, "%lld%s");
RENDER(render_long_long_unsigned, long long unsigned, "%llu%s");
RENDER(render_float, float, "%f%s");
RENDER(render_double, double, "%lf%s");
RENDER(render_long_double, long double, "%Lf%s");

void visit_base_type(char *name, union tag *tag, struct link *prev, unsigned char *p, int more)
{
    struct link link = make_link(tag, prev);
    struct tag_base *tag_base = (struct tag_base *)tag;

    struct lookup {
        char *name;
        void (*render)(unsigned char *p, struct link *link, int more);
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

    for (unsigned int i = 0; i < sizeof(lookup) / sizeof(lookup[0]); ++i) {
        if (!strcmp(tag_base->name, lookup[i].name)) {
            lookup[i].render(p, &link, more);
            break;
        }
    }
}

struct op op_base_type = {
    visit_base_type,
    "base_type",
};

union tag *find(char *name)
{
    struct taglim *taglim = get_taglim();
    if (!taglim) {
        return 0;
    }

    for (int i = 0; i < taglim->nelem; ++i) {
        struct tag_base *tag_base = (struct tag_base *)(taglim->base + i);
        if (tag_base && tag_base->name && !strcmp(tag_base->name, name)) {
            return taglim->base + i;
        }
    }
    return 0;
}

void dump(union tag *tag, char *name, void *p)
{
    struct taglim *taglim = get_taglim();
    if (!taglim || !tag) {
        return;
    }

    ((struct tag_base *)tag)->op->visit(name, tag, 0, (unsigned char *)p, 0);
    printf("\n");
}
