#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cmd.h"
#include "io.h"
#include "stats.h"
#include "sort.h"
#include "util.h"

static void init_patch(Student *patch) {
    memset(patch, 0, sizeof(Student));
    patch->id = -1;        // Sentinel for no change
    patch->mark = -1.0f;   // Sentinel for no change
}

static void show_all(const Store *s){
    if (s->size == 0) {
        puts("No records.");
        return;
    }
    // Dynamically compute column widths based on data + header
    size_t id_w = strlen("ID");
    size_t name_w = strlen("Name");
    size_t prog_w = strlen("Programme");
    size_t mark_w = strlen("Mark");
    char tmp[64];

    for (size_t i = 0; i < s->size; ++i) {
        const Student *st = &s->data[i];
        // ID width
        int n = snprintf(tmp, sizeof tmp, "%d", st->id);
        if (n > 0 && (size_t)n > id_w) id_w = (size_t)n;
        // Name width
        size_t ln = strlen(st->name);
        if (ln > name_w) name_w = ln;
        // Programme width
        size_t lp = strlen(st->programme);
        if (lp > prog_w) prog_w = lp;
        // Mark width (one decimal place)
        n = snprintf(tmp, sizeof tmp, "%.1f", st->mark);
        if (n > 0 && (size_t)n > mark_w) mark_w = (size_t)n;
    }

    // Cast widths to int for printf field widths (safe for normal table sizes)
    int iw = (int)id_w;
    int nw = (int)name_w;
    int pw = (int)prog_w;
    int mw = (int)mark_w;

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

static bool parse_kv(char *token, Student *patch) {
    char *eq = strchr(token, '=');
    if (!eq) {
        return false;
    }
    *eq = '\0';

    char *key = token;
    char *value = eq + 1;
    str_trim(key);
    str_trim(value);

    if (value[0] == '"') {
        size_t len = strlen(value);
        if (len >= 2 && value[len - 1] == '"') {
            value[len-1] = '\0';
            memmove(value, value + 1, len - 1);
        }
    }

    if (str_ieq(key, "ID")) {
        int id;
        if(!parse_int(value, &id)) return false;
        patch->id = id;
        return true;
    } else if (str_ieq(key, "Name")) {
        strncpy(patch->name, value, sizeof(patch->name));
        patch->name[sizeof(patch->name) - 1] = '\0';
        return true;
    } else if (str_ieq(key, "Programme")) {
        strncpy(patch->programme, value, sizeof(patch->programme));
        patch->programme[sizeof(patch->programme) - 1] = '\0';
        return true;
    } else if (str_ieq(key, "Mark")) {
        float mark;
        if (!parse_float(value, &mark)) return false;
        patch->mark = mark;
        return true;
    }

    return false;
}

static bool handle_insert(char *args, Store *s) {
    Student patch;
    init_patch(&patch);

    for (char *token = strtok(args, " "); token; token = strtok(NULL, " ")) {
        if (!parse_kv(token, &patch)) {
            fprintf(stderr, "Invalid key-value pair: %s\n", token);
            return false;
        }
    }

    // Validate all fields are provided
    if (patch.id < 0 || patch.name[0] == '\0' || patch.programme[0] == '\0' || patch.mark < 0.0f) {
        fprintf(stderr, "INSERT requires ID, Name, Programme, Mark.\n");
        return false;
    }

    if (!store_insert(s, patch)) {
        fprintf(stderr, "Failed to insert record. Possible duplicate ID or invalid data.\n");
        return false;
    }

    puts("Record successfully inserted.");
    return true;
}

static bool handle_update(char *args, Store *s) {
    Student patch;
    init_patch(&patch);

    for (char *token = strtok(args, " "); token; token = strtok(NULL, " ")) {
        if (!parse_kv(token, &patch)) {
            fprintf(stderr, "Invalid key-value pair: %s\n", token);
            return false;
        }
    }

    if (patch.id < 0) {
        fprintf(stderr, "UPDATE requires existing ID to identify record.\n");
        return false;
    }

    if (!store_update(s, patch.id, &patch)) {
        fprintf(stderr, "Failed to update record. Possible invalid data or ID not found.\n");
        return false;
    }

    puts("Record successfully updated.");
    return true;
}

static bool handle_delete(char *args, Store *s) {
    int id = -1;
    Student patch;
    init_patch(&patch);
    for (char *token = strtok(args, " "); token; token = strtok(NULL, " '")) {
        if (parse_kv(token, &patch)) {
            if (patch.id > 0) id = patch.id;
        }
    }

    if (id < 0) {
        fprintf(stderr, "DELETE requires ID to identify record.\n");
        return false;
    }

    if (store_find_index_by_id(s, id) < 0) {
        fprintf(stderr, "ID %d not found.\n", id);
        return false;
    }

    printf("Are you sure you want to delete ID %d? (Y/N): ", id);
    fflush(stdout);

    char buf[16];
    if (!fgets(buf, sizeof buf, stdin)) {
        return false;
    }

    if (buf[0] != 'Y' && buf[0] != 'y') {
        puts("Delete operation cancelled.");
        return false;
    }

    if (!store_delete(s, id)) {
        fprintf(stderr, "Failed to delete record with ID %d.\n", id);
        return false;
    }

    puts("Record successfully deleted.");
    return true;
}

bool cmd_process_line(const char *line_in, Store *s, const char *db_path) {
    // Make a modifiable copy of the input line
    char line[512];
    strncpy(line, line_in, sizeof(line));
    line[sizeof(line) - 1] = '\0';

    //split commands and arguments
    char *p = line;
    while (*p && !isspace((unsigned char)*p)) p++;
    char *cmd = line;
    char *args = NULL;
    if (*p) {
        *p = '\0';
        args = p + 1;
    }
    str_tolower(cmd);

    if (strcmp(cmd, "open") == 0) {
        int skipped = 0;
        store_free(s);
        store_init(s);
        if (cms_load(db_path, s, &skipped)) {
            printf("Database loaded. Total %zu records, skipped %d line(s).\n", s->size, skipped);
        } else {
            fprintf(stderr, "Failed to load database from %s\n", db_path);
        }

        return true;
    }

    if (strcmp(cmd, "save") == 0) {
        if (cms_save(db_path, s)) {
            printf("Database saved to %s\n", db_path);
        } else {
            fprintf(stderr, "Failed to save database to %s\n", db_path);
        }

        return true;
    }

    if (strcmp(cmd, "show") == 0) {
        // SHOW [ALL] [SORT BY ID|MARK [ASC|DESC]] | SHOW SUMMARY
        if (!args || strncasecmp(args, "summary", 7) != 0) {
        // maybe has sorting clause
        bool sorted = false, asc = true; SortKey key = SORT_BY_ID;
        if (args && str_icontains(args, "sort by")) {
        sorted = true;
        if (str_icontains(args, "mark")) key = SORT_BY_MARK;
        if (str_icontains(args, "desc")) asc = false;
        }
        if (sorted) store_sort(s, key, asc);
        show_all(s);
        
        } else {
            Stats st = compute_stats(s->data, s->size);
            printf("Total: %zu\nAverage: %.2f\nHighest: %.2f", st.count, st.average, st.max_mark);
            if (st.max_idx >= 0) printf(" (%s)\n", s->data[st.max_idx].name); else puts("");
            printf("Lowest: %.2f", st.min_mark);
            if (st.min_idx >= 0) printf(" (%s)\n", s->data[st.min_idx].name); else puts("");
            printf("Grade bands - A:%d B:%d C:%d D:%d F:%d\n", st.band_A, st.band_B, st.band_C, st.band_D, st.band_F);
        }

        return true;
    }

    if (strcmp(cmd, "insert") == 0) {
        if (!handle_insert(args ? args : "", s)) {
            // Error printing handled in handler
        }

        return true;
    }
    if (strcmp(cmd, "update") == 0) {
        if (!handle_update(args ? args : "", s)) {
            // Error printing handled in handler
        }

        return true;
    }

    if (strcmp(cmd, "delete") == 0) {
        if (!handle_delete(args ? args : "", s)) {
            // Error printing handled in handler
        }

        return true;
    }

    if (strcmp(cmd, "query") == 0) {
    int id = -1; Student tmp; init_patch(&tmp);
    for (char *tok = strtok(args ? args : "", " "); tok; tok = strtok(NULL, " ")) {
        if (parse_kv(tok, &tmp)) { if (tmp.id > 0) id = tmp.id; }
    }

    if (id <= 0) {
        fputs("QUERY requires ID=...\n", stderr);
        return true; 
    }

    int idx = store_find_index_by_id(s, id);
    if (idx < 0) {
        puts("Record does not exist.");
        return true; 
    }
    
    const Student *st = &s->data[idx];
    printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
    return true;
    }

    if (strcmp(cmd, "help") == 0) {
        puts("Available commands:");
        puts("  OPEN                 - Load database from the configured file (unsaved changes will be lost).");
        puts("  SAVE                 - Save current database to the configured file.");
        puts("  SHOW [ALL] [SORT BY ID|MARK [ASC|DESC]]");
        puts("                       - Display records. Optional sort clause (default: ID ASC).");
        puts("  SHOW SUMMARY         - Display statistics: count, average, min/max (with names), grade bands.");
        puts("  INSERT k=v ...       - Add a new student. Required keys: ID, Name, Programme, Mark.");
        puts("                         Example: INSERT ID=1 Name=\"Jane Doe\" Programme=CS Mark=85.5");
        puts("  UPDATE k=v ...       - Update an existing student. ID is required to identify the record.");
        puts("                         Only provide keys you want to change (ID, Name, Programme, Mark).");
        puts("  DELETE ID=...        - Delete a student by ID (prompts for confirmation).");
        puts("                         Example: DELETE ID=1");
        puts("  QUERY ID=...         - Show a single record by ID.");
        puts("                         Example: QUERY ID=1");
        puts("  HELP                 - Show this help text.");
        puts("  EXIT | QUIT          - Exit the program (use SAVE to persist changes).");
        puts("");
        puts("Notes:");
        puts("  - Keys are case-insensitive (ID, Name, Programme, Mark).");
        puts("  - ID must be an integer; Mark is a floating point number.");
        puts("  - For multi-word values enclose them in double quotes: Name=\"John Smith\".");
        puts("  - When parsing key=value pairs, spaces separate tokens; quoted values may contain spaces.");
        puts("  - Use OPEN to reload the DB file; this will discard unsaved in-memory changes.");
        puts("  - Use SAVE to write current in-memory data to the DB file.");
        puts("");
        puts("Examples:");
        puts("  INSERT ID=2 Name=\"Alice Lee\" Programme=IT Mark=72.0");
        puts("  UPDATE ID=2 Mark=75.5");
        puts("  SHOW ALL SORT BY MARK DESC");
    return true;
    }
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) { return false; }


    printf("Unknown command: %s (type HELP)\n", cmd);
    return true;
}

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