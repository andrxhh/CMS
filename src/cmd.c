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

static bool has_no_args(char *args, const char *cmd_name) {
    if (args) {
        str_trim(args);
        if (args[0] != '\0') {
            fprintf(stderr, "%s command does not take any arguments.\n", cmd_name);
            return false;
        }
    }
    return true;
}

static bool parse_single_id_command(char *args, const char *cmd_name, int *out_id) {
    if (!args) {
        fprintf(stderr, "%s command requires ID argument.\n", cmd_name);
        return false;
    }
    
    if (strncasecmp(args, "ID=", 3) != 0) {
        fprintf(stderr, "%s command requires ID argument in format ID=<value>.\n", cmd_name);
        return false;
    }

    char *value_str = args + 3;
    str_trim(value_str);

    char *endptr;
    long value = strtol(value_str, &endptr, 10);

    if (endptr == value_str) {
        fprintf(stderr, "No ID value provided for %s command.\n", cmd_name);
        return false;
    }

    while (*endptr && isspace((unsigned char)*endptr)) endptr++; // Skip trailing whitespace

    if (*endptr != '\0') {
        fprintf(stderr, "Unexpected characters after ID value in %s command.\n", cmd_name);
        return false;
    }

    if (!valid_id((int)value)) {
        fprintf(stderr, "Invalid ID value for %s command. Must be 6-8 digits.\n", cmd_name);
        return false;
    }

    *out_id = (int)value;
    return true;
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

static bool handle_find(char *args, Store *s) {
    char *column = strtok(args, " ");
    char *op = strtok(NULL, " ");
    char *value = strtok(NULL, "");

    if (!column || !op || !value) { // Missing arguments
        fprintf(stderr, "Error: FIND requires 3 arguments. Syntax: FIND <Column> <Operator> <Value>\n");
        fprintf(stderr, "Example: FIND Name CONTAINS \"Wang\"\n");
        fprintf(stderr, "Example: FIND Mark > 75\n");
        return false;
    }

    str_trim(column); str_tolower(column);
    str_trim(op); str_tolower(op);
    str_trim(value);

    size_t len = strlen(value);
    if (len >= 2 && value[0] == '"' && value[len - 1] == '"') {
        // Remove surrounding quotes
        value[len - 1] = '\0';
        memmove(value, value + 1, len - 1);
    }

    float mark_value = 0.0f;
    bool num_filter = false;
    if (strcmp(column, "mark") == 0) {
        if (!parse_float(value, &mark_value)) {
            fprintf(stderr, "Error: Invalid mark value for FIND command: %s\n", value);
            return false;
        }
        num_filter = true;
    }

    int match_count = 0;
    for (size_t i = 0; i<s->size; i++) {
        const Student *st = &s->data[i];
        bool match = false;

        if (strcmp(column, "name") == 0) {
            if (strcmp(op, "=") == 0) {
                match = str_ieq(st->name, value);
            } else if (strcmp(op, "contains") == 0) {
                match = (str_icase_find(st->name, value) != NULL);
            } else {
                fprintf(stderr, "Error: Unsupported operator for Name column: %s\n", op);
                return false;
            }
        } else if (strcmp(column, "programme") == 0) {
            if (strcmp(op, "=") == 0) {
                match = str_ieq(st->name, value);
            } else if (strcmp(op, "contains") == 0) {
                match = (str_icase_find(st->programme, value) != NULL);
            } else {
                fprintf(stderr, "Error: Unsupported operator for Programme column: %s\n", op);
                return false;
            }
        } else if (strcmp(column, "mark") == 0) {
            if (strcmp(op, "=") == 0) {
                match = (st->mark >= mark_value - 0.01f && st->mark <= mark_value + 0.01f);
            } else if (strcmp(op, ">") == 0) {
                match = (st->mark > mark_value);
            } else if (strcmp(op, "<") == 0) {
                match = (st->mark < mark_value);
            } else if (strcmp(op, ">=") == 0) {
                match = (st->mark >= mark_value);
            } else if (strcmp(op, "<=") == 0) {
                match = (st->mark <= mark_value);
            } else {
                fprintf(stderr, "Error: Unsupported operator for Mark column: %s\n", op);
                return false;
            }
        } else {
            fprintf(stderr, "Error: Unsupported column for FIND command: %s\nUse Name, Programme, Mark.\n", column);
        }

        if (match) {
            if (match_count == 0) {
                // Print header on first match
                printf("ID\tName\tProgramme\tMark\n");
            }
            printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
            match_count++;
        }
    }

    if (match_count == 0) {
        puts("No matching records found.");
    } else {
        printf("Total matches: %d\n", match_count);
    }
    
    return true;
}

// static bool parse_kv(char *token, Student *patch) {
//     char *eq = strchr(token, '=');
//     if (!eq) {
//         return false;
//     }
//     *eq = '\0';

//     char *key = token;
//     char *value = eq + 1;
//     str_trim(key);
//     str_trim(value);

//     if (value[0] == '"') {
//         size_t len = strlen(value);
//         if (len >= 2 && value[len - 1] == '"') {
//             value[len-1] = '\0';
//             memmove(value, value + 1, len - 1);
//         }
//     }

//     if (str_ieq(key, "ID")) {
//         int id;
//         if(!parse_int(value, &id)) return false;
//         patch->id = id;
//         return true;
//     } else if (str_ieq(key, "Name")) {
//         strncpy(patch->name, value, sizeof(patch->name));
//         patch->name[sizeof(patch->name) - 1] = '\0';
//         return true;
//     } else if (str_ieq(key, "Programme")) {
//         strncpy(patch->programme, value, sizeof(patch->programme));
//         patch->programme[sizeof(patch->programme) - 1] = '\0';
//         return true;
//     } else if (str_ieq(key, "Mark")) {
//         float mark;
//         if (!parse_float(value, &mark)) return false;
//         patch->mark = mark;
//         return true;
//     }

//     return false;
// }

static bool handle_insert(char *args, Store *s) {
    printf("Insert args: %s\n", args);
    Student patch;
    init_patch(&patch);

    char *p = args;
    while (p && *p) {
        const char *key_name= NULL;
        char *key_start = find_next_key(p, &key_name); // Find start of next key
        if (!key_start) {
            break;
        }

        char *eq = strchr(key_start + strlen(key_name), '=');

        char *check_ptr = key_start + strlen(key_name);
        while(check_ptr < eq && isspace((unsigned char)*check_ptr)) {
            check_ptr++;
        }

        if (!eq || check_ptr != eq) {
            fprintf(stderr, "Malformed key-value pair: Missing or invalid '=' after key %s.\n", key_start);
            return false;
        }

        char *value_start = eq + 1;
        while (*value_start && isspace((unsigned char)*value_start)) value_start++;

        const char *next_key = NULL;
        char *value_end = find_next_key(value_start, &next_key); // Find start of next key or the end of string

        char value_buf[256];
        size_t len;
        if (value_end == NULL) {
            len = strlen(value_start);
            p = NULL; // End of string
        } else {
            len = value_end - value_start;
            p = value_end;
        }

        if (len > sizeof(value_buf)-1) len = sizeof(value_buf)-1;
        strncpy(value_buf, value_start, len);
        value_buf[len] = '\0';
        str_trim(value_buf);

        size_t value_len = strlen(value_buf);
        if (value_len > 2 && value_buf[0] == '"' && value_buf[value_len - 1] == '"') {
            // Remove surrounding quotes
            value_buf[value_len - 1] = '\0';
            memmove(value_buf, value_buf + 1, value_len - 1);
        }

        if (str_ieq(key_name, "ID")) {
            if(!parse_int(value_buf, &patch.id)) {
                fprintf(stderr, "Invalid ID value: %s\n", value_buf);
                return false;
            }
        } else if (str_ieq(key_name, "Name")) {
            strncpy(patch.name, value_buf, sizeof(patch.name));
            patch.name[sizeof(patch.name) - 1] = '\0';
        } else if (str_ieq(key_name, "Programme")) {
            strncpy(patch.programme, value_buf, sizeof(patch.programme));
            patch.programme[sizeof(patch.programme) - 1] = '\0';
        } else if (str_ieq(key_name, "Mark")) {
            if (!parse_float(value_buf, &patch.mark)) {
                fprintf(stderr, "Invalid Mark value: %s\n", value_buf);
                return false;
            }
        }
    }

    printf("Parsed Insert - ID: %d, Name: %s, Programme: %s, Mark: %.2f\n",
           patch.id, patch.name, patch.programme, patch.mark);

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

    char *p = args;
    while (p && *p) {
        const char *key_name= NULL;
        char *key_start = find_next_key(p, &key_name); // Find start of next key
        if (!key_start) {
            break;
        }

        char *eq = strchr(key_start + strlen(key_name), '=');

        char *check_ptr = key_start + strlen(key_name);
        while(check_ptr < eq && isspace((unsigned char)*check_ptr)) {
            check_ptr++;
        }

        if (!eq || check_ptr != eq) {
            fprintf(stderr, "Malformed key-value pair: Missing or invalid '=' after key %s.\n", key_start);
            return false;
        }

        char *value_start = eq + 1;
        while (*value_start && isspace((unsigned char)*value_start)) value_start++;

        const char *next_key = NULL;
        char *value_end = find_next_key(value_start, &next_key); // Find start of next key or the end of string

        char value_buf[256];
        size_t len;
        if (value_end == NULL) {
            len = strlen(value_start);
            p = NULL; // End of string
        } else {
            len = value_end - value_start;
            p = value_end;
        }

        if (len > sizeof(value_buf)-1) len = sizeof(value_buf)-1;
        strncpy(value_buf, value_start, len);
        value_buf[len] = '\0';
        str_trim(value_buf);

        size_t value_len = strlen(value_buf);
        if (value_len > 2 && value_buf[0] == '"' && value_buf[value_len - 1] == '"') {
            // Remove surrounding quotes
            value_buf[value_len - 1] = '\0';
            memmove(value_buf, value_buf + 1, value_len - 1);
        }

        if (str_ieq(key_name, "ID")) {
            if(!parse_int(value_buf, &patch.id)) {
                fprintf(stderr, "Invalid ID value: %s\n", value_buf);
                return false;
            }
        } else if (str_ieq(key_name, "Name")) {
            strncpy(patch.name, value_buf, sizeof(patch.name));
            patch.name[sizeof(patch.name) - 1] = '\0';
        } else if (str_ieq(key_name, "Programme")) {
            strncpy(patch.programme, value_buf, sizeof(patch.programme));
            patch.programme[sizeof(patch.programme) - 1] = '\0';
        } else if (str_ieq(key_name, "Mark")) {
            if (!parse_float(value_buf, &patch.mark)) {
                fprintf(stderr, "Invalid Mark value: %s\n", value_buf);
                return false;
            }
        }
    }

    printf("Parsed Update - ID: %d, Name: %s, Programme: %s, Mark: %.2f\n",
           patch.id, patch.name, patch.programme, patch.mark);

    if (patch.id < 0) {
        fprintf(stderr, "UPDATE requires existing ID to identify record.\n");
        return false;
    }

    if (patch.name[0] == '\0' && patch.programme[0] == '\0' && patch.mark < 0.0f) {
        // Only an ID was provided.
        printf("Warning: UPDATE command given with only an ID. No fields to update.\n");
    }

    if (!store_update(s, patch.id, &patch)) {
        fprintf(stderr, "Failed to update record. Possible invalid data or ID not found.\n");
        return false;
    }

    puts("Record successfully updated.");
    return true;
}

static bool handle_delete(char *args, Store *s) {
    int id;
    if (!parse_single_id_command(args, "DELETE", &id)) {
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

static bool handle_query(char *args, const Store *s) {
    int id;
    if (!parse_single_id_command(args, "QUERY", &id)) {
        return false;
    }

    if (id <= 0) {
        fputs("QUERY requires ID=...\n", stderr);
        return false; 
    }

    int idx = store_find_index_by_id(s, id);
    if (idx < 0) {
        puts("Record does not exist.");
        return false; 
    }
    
    const Student *st = &s->data[idx];
    printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
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
        if (!has_no_args(args, "OPEN")) {
            return true;
        }

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
        if (!has_no_args(args, "SAVE")) {
            return true;
        }

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
        if (!handle_query(args ? args : "", s)) {
            // Error printing handled in handler
        }
        return true;
    }

    if (strcmp(cmd, "find") == 0) {
        if (!handle_find(args ? args : "", s)) {
            // Error printing handled in handler
        }
        return true;
    }

    if (strcmp(cmd, "help") == 0) {
        if (!has_no_args(args, "HELP")) {
            return true;
        }

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
        puts("  FIND <Column> <Op> <Value>");
        puts("                       - Search records. Columns: Name, Programme, Mark.");
        puts("                         Operators for Name/Programme: =, CONTAINS (case-insensitive).");
        puts("                         Operators for Mark: =, >, <, >=, <=.");
        puts("                         Value for strings may be quoted, e.g. FIND Name CONTAINS \"Wang\".");
        puts("                         Example: FIND Mark > 75");
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
        puts("  FIND Name CONTAINS \"Wang\"");
        puts("  FIND Mark >= 85");
        return true;
        
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