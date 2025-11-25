#include <stdio.h>
#include <string.h>
#include "cmd_internal.h"
#include "stats.h"

/*
This file contains display-related commands, such as showing all records and
printing the team declaration.
*/

// Print all records in a simple table. Column widths are computed dynamically
void show_all(const Store *s) {
    if (s->size == 0) {
        puts("No records in memory. (Type 'OPEN')");
        return;
    }

    // Computer length of headers
    size_t id_width = strlen("ID");
    size_t name_width = strlen("Name");
    size_t prog_width = strlen("Programme");
    size_t mark_width = strlen("Mark");
    char tmp[64];

    // Compute maximum width for each column by scanning all records and comparing lengths
    for (size_t i = 0; i < s->size; ++i) {
        const Student *st = &s->data[i];
        int n = snprintf(tmp, sizeof tmp, "%d", st->id);
        if (n > 0 && (size_t)n > id_width) id_width = (size_t)n;
        size_t ln = strlen(st->name);
        if (ln > name_width) name_width = ln;
        size_t lp = strlen(st->programme);
        if (lp > prog_width) prog_width = lp;
        n = snprintf(tmp, sizeof tmp, "%.1f", st->mark);
        if (n > 0 && (size_t)n > mark_width) mark_width = (size_t)n;
    }

    // Cast to int for printf
    int iw = (int)id_width;
    int nw = (int)name_width;
    int pw = (int)prog_width;
    int mw = (int)mark_width;

    printf("size=%zu cap=%zu\n", s->size, s->cap);
    printf("%-*s  %-*s  %-*s  %*s\n", iw, "ID", nw, "Name", pw, "Programme", mw, "Mark");
    for (size_t i = 0; i < s->size; ++i) {
        const Student *st = &s->data[i];
        printf("%*d  %-*s  %-*s  %*.1f\n",
               iw, st->id,
               nw, st->name,
               pw, st->programme,
               mw, st->mark);
    }
    puts("");
}

// Print the declaration required for the team project
void print_declaration(const char *team_name, const char *members_csv, const char *date_str) {
    puts("============================================");
    puts("We declare that this is our own work and ...");
    puts("SIT's policy on copying does not allow the students to copy source code as well as assessment solutions\n"
         "from another person, AI, or other places. It is the students' responsibility to guarantee that their\n"
         "assessment solutions are their own work. Meanwhile, the students must also ensure that their work is\n"
         "not accessible by others. Where such plagiarism is detected, both of the assessments involved will\n"
         "receive ZERO mark.");
    printf("Team: %s\n\nMembers: %s\nDate: %s\n", team_name, members_csv, date_str);
    puts("");
}
