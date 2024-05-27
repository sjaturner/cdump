#ifndef _DWARF_H_
#define _DWARF_H_
struct link
{
   struct link *prev;
   struct link *next;
   union tag *tag;
   unsigned int val;
   union tag *member;
};

struct taglim
{
   union tag *base;
   int nelem;
};

struct op
{
   void (*visit)(char *name, union tag * tag, struct link * link, unsigned char *p);
   char *name;
};

extern struct op end;

struct tag_base
{
   struct op *op;
   char *name;
   int byte_size;
};

extern struct op op_array_type;
struct tag_array_type
{
   struct tag_base tag_base;
   unsigned int sibling;
   unsigned int type;
};

extern struct op op_base_type;
struct tag_base_type
{
   struct tag_base tag_base;
   int encoding;
};

extern struct op op_compile_unit;
struct tag_compile_unit
{
   struct tag_base tag_base;
   char *comp_dir;
   unsigned int high_pc;
   int language;
   unsigned int low_pc;
   char *producer;
   unsigned int stmt_list;
};

extern struct op op_const_type;
struct tag_const_type
{
   struct tag_base tag_base;
   unsigned int type;
};

extern struct op op_enumeration_type;
struct tag_enumeration_type
{
   struct tag_base tag_base;
   int decl_file;
   int decl_line;
   unsigned int sibling;
};

extern struct op op_enumerator;
struct tag_enumerator
{
   struct tag_base tag_base;
   int const_value;
};

extern struct op op_formal_parameter;
struct tag_formal_parameter
{
   struct tag_base tag_base;
   int decl_file;
   int decl_line;
   int location; /* leave */
   unsigned int type;
};

extern struct op op_lexical_block;
struct tag_lexical_block
{
   struct tag_base tag_base;
   unsigned int high_pc;
   unsigned int low_pc;
   unsigned int sibling;
};

extern struct op op_member;
struct tag_member
{
   struct tag_base tag_base;
   int bit_offset;
   int bit_size;
   int data_member_location; /* leave */
   int decl_file;
   int decl_line;
   unsigned int type;
};

extern struct op op_pointer_type;
struct tag_pointer_type
{
   struct tag_base tag_base;
   unsigned int type;
};

extern struct op op_structure_type;
struct tag_structure_type
{
   struct tag_base tag_base;
   int decl_file;
   int decl_line;
   unsigned int sibling;
};

extern struct op op_subprogram;
struct tag_subprogram
{
   struct tag_base tag_base;
   int decl_file;
   int decl_line;
   int external;
   unsigned int frame_base;
   unsigned int high_pc;
   unsigned int low_pc;
   int prototyped;
   unsigned int sibling;
   unsigned int type;
};

extern struct op op_subrange_type;
struct tag_subrange_type
{
   struct tag_base tag_base;
   unsigned int type;
   int upper_bound;
};

extern struct op op_subroutine_type;
struct tag_subroutine_type
{
   struct tag_base tag_base;
   int prototyped;
   unsigned int sibling;
   unsigned int type;
};

extern struct op op_typedef;
struct tag_typedef
{
   struct tag_base tag_base;
   int decl_file;
   int decl_line;
   unsigned int type;
};

extern struct op op_union_type;
struct tag_union_type
{
   struct tag_base tag_base;
   int decl_file;
   int decl_line;
   unsigned int sibling;
};

extern struct op op_variable;
struct tag_variable
{
   struct tag_base tag_base;
   int decl_file;
   int decl_line;
   int external;
   int location; /* leave */
   unsigned int type;
   int artificial;
};

union tag
{
   struct tag_base tag_base;
   struct tag_array_type tag_array_type;
   struct tag_base_type tag_base_type;
   struct tag_compile_unit tag_compile_unit;
   struct tag_const_type tag_const_type;
   struct tag_enumeration_type tag_enumeration_type;
   struct tag_enumerator tag_enumerator;
   struct tag_formal_parameter tag_formal_parameter;
   struct tag_lexical_block tag_lexical_block;
   struct tag_member tag_member;
   struct tag_pointer_type tag_pointer_type;
   struct tag_structure_type tag_structure_type;
   struct tag_subprogram tag_subprogram;
   struct tag_subrange_type tag_subrange_type;
   struct tag_subroutine_type tag_subroutine_type;
   struct tag_typedef tag_typedef;
   struct tag_union_type tag_union_type;
   struct tag_variable tag_variable;
};

union tag *find(char *name);
void dump(union tag *tag, char *name, void *p);

extern struct taglim taglim;

#define PASTE(x,y) x ## _ ## y
#define EVAL(x,y)  PASTE(x,y)
#define NAME(fun) EVAL(fun, __LINE__)
#define DWARF(THING) {typeof(THING) *NAME(debug)= &THING; union tag *tag = find(#THING); ((struct tag_base *)tag)->op->visit(#THING, tag, 0, (unsigned char *)NAME(debug));}

#endif
