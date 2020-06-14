#include<stdio.h>
#include<string.h>
#include<stdlib.h>
struct entry
{
   char *name;
   char *val;
};

enum
{
   MAX_RECORDS = 0x20
};

enum
{
   MAX_LEN = 0x400
};

char tag[MAX_LEN];
int tag_index;
struct entry record[MAX_RECORDS];
int records;

char *rtrim(char *buf)
{
   char *p = buf + strlen(buf);

   while(p > buf && (!*p || *p == ' ' || *p == '\t'))
   {
      *p = 0;
      --p;
   }

   return buf;
}

void flush_record(void)
{
   int i = 0;

   if(!*tag)
   {
      return;
   }
   printf("    [0x%08x]={\n", tag_index);
   printf("        .tag_%s={\n", tag);
   printf("            .tag_base.op=&op_%s,\n", tag);
   for(i = 0; i < records; ++i)
   {
      if(!strcmp("name", record[i].name) || !strcmp("byte_size", record[i].name))
      {
         printf("            .tag_base.%s=%s,\n", record[i].name, rtrim(record[i].val));
      }
      else
      {
         printf("            .%s=%s,\n", record[i].name, rtrim(record[i].val));
      }
      free(record[i].name);
      free(record[i].val);
   }
   printf("        },\n");
   printf("    },\n");


   memset(record, 0, sizeof record);
   records = 0;
   tag_index = 0;
   tag[0] = 0;
}

char *tagdup(char *pc)
{
   char *DW_AT_ = "DW_AT_";
   int strlen_DW_AT_ = strlen(DW_AT_);
   char *b = strstr(pc, DW_AT_) + strlen_DW_AT_;
   char *e = strchr(b, ' ');
   char *tag = calloc(1, e - b + 1);
   char *t = tag;

   while(b != e)
   {
      *t++ = *b++;
   }

   return tag;
}

char *quotedup(char *pc)
{
   char *trimmed = rtrim(pc);
   int len = strlen(trimmed) + 3;
   char *ret = calloc(len, 1);

   snprintf(ret, len, "\"%s\"", trimmed);

   return ret;
}

int parse_line(char *line)
{
   {
      struct fmts
      {
         char *ofmt;
         char *ifmt;
      };
      struct fmts fmts[] = {
         {"%d", " <%x> DW_AT_artificial : %d",},
         {"%d", " <%x> DW_AT_bit_offset : %d",},
         {"%d", " <%x> DW_AT_data_member_location : %d",},
         {"%d", " <%x> DW_AT_bit_size : %d",},
         {"%d", " <%x> DW_AT_byte_size : %d",},
         {"%d", " <%x> DW_AT_const_value : %d",},
         {"%d", " <%x> DW_AT_decl_file : %d",},
         {"%d", " <%x> DW_AT_decl_line : %d",},
         {"%d", " <%x> DW_AT_external : %d",},
         {"%d", " <%x> DW_AT_prototyped : %d",},
         {"%d", " <%x> DW_AT_upper_bound : %d",},
         {"0x%08x", " <%x> DW_AT_high_pc : %x ",},
         {"0x%08x", " <%x> DW_AT_low_pc : %x ",},
         {"0x%08x", " <%x> DW_AT_stmt_list : %x ",},
         {"0x%08x", " <%x> DW_AT_sibling : <0x%x> ",},
         {"0x%08x", " <%x> DW_AT_type : <0x%x> ",},
      };

      int i = 0;

      for(i = 0; i < sizeof(fmts) / sizeof(fmts[0]); ++i)
      {
         unsigned int u = 0;
         unsigned int val = 0;
         char *ifmt = fmts[i].ifmt;
         char *ofmt = fmts[i].ofmt;

         if(2 == sscanf(line, ifmt, &u, &val))
         {
            char n[0x20] = "";
            struct entry *entry = record + records++;

            snprintf(n, (int)sizeof(n) - 1, ofmt, val);

            entry->val = strdup(n);
            entry->name = tagdup(ifmt);

            return 1;
         }
      }
   }

   {
      char buf[MAX_LEN] = "";
      unsigned int u0;
      unsigned int u1;
      char *fmts[] = {
         " <%x> DW_AT_comp_dir : (indirect string, offset: 0x%x): %[^\n]",
         " <%x> DW_AT_name : (indirect string, offset: 0x%x): %[^\n]",
      };
      int i = 0;

      for(i = 0; i < sizeof(fmts) / sizeof(fmts[0]); ++i)
      {
         char *fmt = fmts[i];

         if(3 == sscanf(line, fmt, &u0, &u1, buf))
         {
            struct entry *entry = record + records++;
            entry->val = quotedup(buf);
            entry->name = tagdup(fmt);
            return 1;
         }
      }
   }

   {
      unsigned int u;
      unsigned int val;
      char buf[MAX_LEN] = "";
      char *fmt = " <%x> DW_AT_encoding : %d (%s)";
      if(3 == sscanf(line, fmt, &u, &val, buf))
      {
         char n[0x20] = "";
         struct entry *entry = record + records++;

         snprintf(n, (int)sizeof(n) - 1, "%d", val);

         entry->val = strdup(n);
         entry->name = tagdup(fmt);
         return 1;
      }
   }

   {
      unsigned int u;
      char buf[MAX_LEN] = "";
      char *fmt = " <%x> DW_AT_name : %s";
      if(2 == sscanf(line, fmt, &u, buf))
      {
         struct entry *entry = record + records++;

         entry->val = quotedup(buf);
         entry->name = tagdup(fmt);
         return 1;
      }
   }
   return 0;
}

int main()
{
   int trig = 0;

   printf("union tag tags[]={\n");

   for(;;)
   {
      char line[MAX_LEN];
      memset(line, 0, sizeof line);

      if(!fgets(line, (int)sizeof(line) - 1, stdin))
      {
         break;
      }
      else if(!trig && strstr(line, "Contents of the .debug_info section"))
      {
         trig = 1;
         continue;
      }
      else if(!trig)
      {
         continue;
      }
      else if(trig && strstr(line, "Contents of the .debug_abbrev section"))
      {
         break;
      }
      else if(!strchr(line, '<'))
      {
         continue;
      }
      else
      {
         int found = 0;
         int i = 0;
         char *ignore[] = {
            "DW_AT_language",
            "DW_AT_location",
            "DW_AT_producer",
            "DW_AT_frame_base",
         };

         for(i = 0; i < sizeof(ignore) / sizeof(ignore[0]); ++i)
         {
            if(strstr(line, ignore[i]))
            {
               found = 1;
               break;
            }
         }

         if(found)
         {
            continue;
         }
      }

      if(parse_line(line))
      {
         continue;
      }

      {
         char buf[MAX_LEN] = "";
         unsigned int u0;
         unsigned int val;
         int i0;

         if(4 == sscanf(line, " <%x><%x>: Abbrev Number: %d (DW_TAG_%[^)]", &u0, &val, &i0, buf))
         {
            flush_record();
            tag_index = val;
            strcpy(tag, buf);
         }
      }
   }
   flush_record();
   printf("};\n");

   printf("struct taglim taglim = {\n");
   printf("    .base=tags,\n");
   printf("    .nelem=sizeof tags/sizeof tags[0],\n");
   printf("};\n");

   return 0;
}
