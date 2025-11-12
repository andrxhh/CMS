/******************************
 * FILE: cmd.c
 * 
 * COMMAND PROCESSING for CMS (Course Management System)
 * Integrated version with INSERT and DELETE implementations
 ******************************/

#include <stdio.h>
#include <string.h>
#include <strings.h>  // For strncasecmp
#include <stdlib.h>
#include <ctype.h>
#include "cmd.h"
#include "io.h"
#include "sort.h"
#include "stats.h"
#include "util.h"

/*******************************************************************************
 * MENU / HELP PRINTER
 ******************************************************************************/
static void print_menu(void) {
    puts("------------------------------------------------------------");
    puts("Available Commands");
    puts("------------------------------------------------------------");
    puts("OPEN                      - Load database from P6_5.txt");
    puts("SAVE                      - Save database to P6_5.txt");
    puts("");
    puts("SHOW ALL                  - Display all students");
    puts("SHOW ALL SORT BY ID ASC   - Display sorted by ID ascending");
    puts("SHOW ALL SORT BY ID DESC  - Display sorted by ID descending");
    puts("SHOW ALL SORT BY MARK ASC - Display sorted by Mark ascending");
    puts("SHOW ALL SORT BY MARK DESC- Display sorted by Mark descending");
    puts("SHOW SUMMARY              - Show statistics");
    puts("");
    puts("INSERT ID=<id> Name=\"<name>\" Programme=\"<programme>\" Mark=<mark>");
    puts("  e.g. INSERT ID=2501234 Name=\"Alice Tan\" Programme=\"Computer Science\" Mark=85.5");
    puts("");
    puts("UPDATE ID=<id> [Name=\"<name>\"] [Programme=\"<programme>\"] [Mark=<mark>]");
    puts("  e.g. UPDATE ID=2301234 Mark=95.5");
    puts("");
    puts("DELETE ID=<id>            - Delete a record (prompts Y/N)");
    puts("QUERY ID=<id>             - Show a single record");
    puts("");
    puts("HELP                      - Show this menu");
    puts("EXIT or QUIT              - Exit the program");
    puts("------------------------------------------------------------");
}

/*******************************************************************************
 * HELPER FUNCTION: Extract quoted string value
 * 
 * Purpose: Parse values that are in quotes like Name="Alice Tan"
 ******************************************************************************/
static bool extract_quoted_value(const char *str, char *out, size_t out_size) {
    const char *start = strchr(str, '"');
    if (!start) return false;
    start++;
    
    const char *end = strchr(start, '"');
    if (!end) return false;
    
    size_t len = end - start;
    if (len >= out_size) return false;
    
    strncpy(out, start, len);
    out[len] = '\0';
    
    return true;
}

/*******************************************************************************
 * HELPER FUNCTION: Extract key-value pair
 * 
 * Purpose: Parse patterns like "ID=123456" or "Mark=85.5"
 ******************************************************************************/
static bool extract_value(const char *str, char *value, size_t value_size) {
    const char *eq = strchr(str, '=');
    if (!eq) return false;
    eq++;
    
    while (*eq == ' ' || *eq == '\t') eq++;
    
    size_t i = 0;
    while (eq[i] && eq[i] != ' ' && eq[i] != '\t' && i < value_size - 1) {
        value[i] = eq[i];
        i++;
    }
    value[i] = '\0';
    
    return i > 0;
}

/*******************************************************************************
 * INITIALIZE PATCH STRUCTURE
 * 
 * A "patch" is a partial Student object - only some fields are set
 ******************************************************************************/
static void init_patch(Student *p) {
    memset(p, 0, sizeof *p);
    p->id = -1;
    p->mark = -1.0f;
}

/*******************************************************************************
 * PARSE KEY=VALUE PAIR (Original template version)
 ******************************************************************************/
static bool parse_kv(char *token, Student *patch) {
    char *eq = strchr(token, '=');
    if (!eq) return false;
    
    *eq = '\0';
    char *key = token;
    char *val = eq + 1;
    
    str_trim(key);
    str_trim(val);
    
    // Handle quoted values
    if (val[0] == '"') {
        size_t L = strlen(val);
        if (L >= 2 && val[L-1] == '"') {
            val[L-1] = '\0';
            memmove(val, val+1, L-1);
        }
    }
    
    // Match key and set appropriate field
    if (str_ieq(key, "ID")) {
        int id;
        if (!parse_int(val, &id)) return false;
        patch->id = id;
        return true;
    } else if (str_ieq(key, "Name")) {
        strncpy(patch->name, val, sizeof patch->name);
        patch->name[sizeof patch->name - 1] = '\0';
        return true;
    } else if (str_ieq(key, "Programme") || str_ieq(key, "Program") || str_ieq(key, "Prog")) {
        strncpy(patch->programme, val, sizeof patch->programme);
        patch->programme[sizeof patch->programme - 1] = '\0';
        return true;
    } else if (str_ieq(key, "Mark")) {
        float m;
        if (!parse_float(val, &m)) return false;
        patch->mark = m;
        return true;
    }
    
    return false;
}

/*******************************************************************************
 * INSERT COMMAND - YOUR IMPLEMENTATION INTEGRATED!
 * 
 * Handles: INSERT ID=<id> Name="<name>" Programme="<programme>" Mark=<mark>
 ******************************************************************************/
void cmd_insert(const char *line, Store *s) {
    Student st = {0};
    st.id = 0;
    st.name[0] = '\0';
    st.programme[0] = '\0';
    st.mark = -1.0f;
    
    bool has_id = false;
    bool has_name = false;
    bool has_programme = false;
    bool has_mark = false;
    
    char buffer[512];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    // Find where parameters start
    char *params = strstr(buffer, "INSERT");
    if (!params) {
        printf("ERROR: Invalid INSERT command format.\n");
        return;
    }
    params += 6;
    
    // Skip whitespace
    while (*params == ' ' || *params == '\t') params++;
    
    // Parse each field
    char *current = params;
    while (*current) {
        while (*current == ' ' || *current == '\t') current++;
        if (*current == '\0') break;
        
        if (strncmp(current, "ID=", 3) == 0) {
            char id_str[32];
            if (extract_value(current, id_str, sizeof(id_str))) {
                if (parse_int(id_str, &st.id)) {
                    has_id = true;
                }
            }
            while (*current && *current != ' ' && *current != '\t') current++;
        }
        else if (strncmp(current, "Name=", 5) == 0) {
            if (extract_quoted_value(current, st.name, sizeof(st.name))) {
                str_trim(st.name);
                has_name = true;
            }
            char *quote_end = strchr(strchr(current, '"') + 1, '"');
            if (quote_end) current = quote_end + 1;
        }
        else if (strncmp(current, "Programme=", 10) == 0) {
            if (extract_quoted_value(current, st.programme, sizeof(st.programme))) {
                str_trim(st.programme);
                has_programme = true;
            }
            char *quote_end = strchr(strchr(current, '"') + 1, '"');
            if (quote_end) current = quote_end + 1;
        }
        else if (strncmp(current, "Mark=", 5) == 0) {
            char mark_str[32];
            if (extract_value(current, mark_str, sizeof(mark_str))) {
                if (parse_float(mark_str, &st.mark)) {
                    has_mark = true;
                }
            }
            while (*current && *current != ' ' && *current != '\t') current++;
        }
        else {
            while (*current && *current != ' ' && *current != '\t') current++;
        }
    }
    
    // Validate all required fields
    if (!has_id) {
        printf("ERROR: Missing or invalid ID field.\n");
        printf("Format: INSERT ID=<id> Name=\"<name>\" Programme=\"<programme>\" Mark=<mark>\n");
        return;
    }
    
    if (!has_name) {
        printf("ERROR: Missing or invalid Name field (must be in quotes).\n");
        printf("Format: INSERT ID=<id> Name=\"<name>\" Programme=\"<programme>\" Mark=<mark>\n");
        return;
    }
    
    if (!has_programme) {
        printf("ERROR: Missing or invalid Programme field (must be in quotes).\n");
        printf("Format: INSERT ID=<id> Name=\"<name>\" Programme=\"<programme>\" Mark=<mark>\n");
        return;
    }
    
    if (!has_mark) {
        printf("ERROR: Missing or invalid Mark field.\n");
        printf("Format: INSERT ID=<id> Name=\"<name>\" Programme=\"<programme>\" Mark=<mark>\n");
        return;
    }
    
    // Attempt to insert
    if (store_insert(s, st)) {
        printf("SUCCESS: Student inserted (ID=%d, Name=\"%s\", Programme=\"%s\", Mark=%.2f)\n",
               st.id, st.name, st.programme, st.mark);
    } else {
        if (!valid_id(st.id)) {
            printf("ERROR: ID must be 6-8 digits (got: %d)\n", st.id);
        } else if (!valid_mark(st.mark)) {
            printf("ERROR: Mark must be between 0.0 and 100.0 (got: %.2f)\n", st.mark);
        } else if (!valid_text(st.name)) {
            printf("ERROR: Name cannot be empty\n");
        } else if (!valid_text(st.programme)) {
            printf("ERROR: Programme cannot be empty\n");
        } else if (store_find_index_by_id(s, st.id) != -1) {
            printf("ERROR: Student with ID %d already exists\n", st.id);
        } else {
            printf("ERROR: Failed to insert student\n");
        }
    }
}

/*******************************************************************************
 * HELPER: Parse ID argument
 ******************************************************************************/
static int parse_id_arg(const char *args) {
    if (!args) return -1;
    
    char buf[256];
    strncpy(buf, args, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    for (char *tok = strtok(buf, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
        int id;
        if (sscanf(tok, "ID=%d", &id) == 1) return id;
    }
    return -1;
}

/*******************************************************************************
 * HELPER: Read Yes/No confirmation
 ******************************************************************************/
static char read_yes_no(void) {
    char line[32];
    if (!fgets(line, sizeof(line), stdin)) return 'N';
    return (line[0] == 'y' || line[0] == 'Y') ? 'Y' : 'N';
}

/*******************************************************************************
 * DELETE COMMAND - YOUR IMPLEMENTATION INTEGRATED!
 * 
 * Handles: DELETE ID=<id>
 ******************************************************************************/
bool handle_delete(const char *args, Store *s) {
    int id = parse_id_arg(args);
    if (id <= 0) {
        fputs("DELETE requires a valid student ID.\n", stderr);
        return false;
    }

    if (store_find_index_by_id(s, id) < 0) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return false;
    }

    printf("CMS: Are you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to cancel.\n", id);
    char yn = read_yes_no();
    
    if (yn == 'Y') {
        if (store_delete(s, id)) {
            printf("CMS: The record with ID=%d is successfully deleted.\n", id);
            return true;
        } else {
            printf("CMS: Deletion failed due to an internal error.\n");
            return false;
        }
    } else {
        puts("CMS: The deletion is cancelled.");
        return false;
    }
}

/*******************************************************************************
 * SHOW ALL STUDENTS
 ******************************************************************************/
static void show_all(const Store *s) {
#if USE_FIXED_SIZE_ARRAY
    if (s->count == 0) {
#else
    if (s->size == 0) {
#endif
        puts("No student records to display.");
        return;
    }
    
    puts("ID\tName\tProgramme\tMark");
    
#if USE_FIXED_SIZE_ARRAY
    for (int i = 0; i < s->count; i++) {
        const Student *st = &s->students[i];
#else
    for (size_t i = 0; i < s->size; i++) {
        const Student *st = &s->data[i];
#endif
        printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
    }
}

/*******************************************************************************
 * HANDLE UPDATE COMMAND
 ******************************************************************************/
static bool handle_update(char *args, Store *s) {
    Student patch;
    init_patch(&patch);
    
    for (char *tok = strtok(args, " "); tok; tok = strtok(NULL, " ")) {
        if (!parse_kv(tok, &patch)) {
            fprintf(stderr, "Invalid token: %s\n", tok);
            return false;
        }
    }
    
    if (patch.id <= 0) {
        fputs("UPDATE requires ID=...\n", stderr);
        return false;
    }
    
    int target_id = patch.id;
    patch.id = -1;
    
    if (!store_update(s, target_id, &patch)) {
        puts("Update failed (not found/invalid).");
        return false;
    }
    
    puts("Record successfully updated.");
    return true;
}

/*******************************************************************************
 * MAIN COMMAND PROCESSOR
 ******************************************************************************/
bool cmd_process_line(const char *line_in, Store *s, const char *db_path) {
    char line[512];
    strncpy(line, line_in, sizeof line);
    line[sizeof line - 1] = '\0';
    
    str_trim(line);
    if (line[0] == '\0') return true;
    
    char *p = line;
    while (*p && !isspace((unsigned char)*p)) p++;
    char *cmd = line;
    char *args = NULL;
    if (*p) {
        *p = '\0';
        args = p + 1;
    }
    
    str_tolower(cmd);

    // COMMAND: OPEN
    if (strcmp(cmd, "open") == 0) {
        int skipped = 0;
        
        store_free(s);
        store_init(s);
        
        if (cms_load(db_path, s, &skipped)) {
            printf("CMS: The database file \"%s\" is successfully opened.\n", db_path);
            print_menu();
        } else {
            puts("No existing database found. A new one will be created on SAVE.");
        }
        return true;
    }
    
    // COMMAND: SAVE
    if (strcmp(cmd, "save") == 0) {
        if (cms_save(db_path, s))
            puts("Database successfully saved.");
        else
            puts("Save failed.");
        return true;
    }
    
    // COMMAND: SHOW
    if (strcmp(cmd, "show") == 0) {
        if (!args || strncasecmp(args, "summary", 7) != 0) {
            bool sorted = false;
            bool asc = true;
            SortKey key = SORT_BY_ID;
            
            if (args && strcasestr(args, "sort by")) {
                sorted = true;
                if (strcasestr(args, "mark"))
                    key = SORT_BY_MARK;
                if (strcasestr(args, "desc"))
                    asc = false;
            }
            
            if (sorted)
                store_sort(s, key, asc);
            
            puts("CMS: Here are all the records found in the table \"StudentRecords\".");
            show_all(s);
            return true;
            
        } else {
#if USE_FIXED_SIZE_ARRAY
            Stats st = compute_stats(s->students, s->count);
            printf("Total: %d\nAverage: %.2f\nHighest: %.2f",
                   s->count, st.average, st.max_mark);
#else
            Stats st = compute_stats(s->data, s->size);
            printf("Total: %zu\nAverage: %.2f\nHighest: %.2f",
                   st.count, st.average, st.max_mark);
#endif
            
            if (st.max_idx >= 0) {
#if USE_FIXED_SIZE_ARRAY
                printf(" (%s)\n", s->students[st.max_idx].name);
#else
                printf(" (%s)\n", s->data[st.max_idx].name);
#endif
            } else {
                puts("");
            }
            
            printf("Lowest: %.2f", st.min_mark);
            
            if (st.min_idx >= 0) {
#if USE_FIXED_SIZE_ARRAY
                printf(" (%s)\n", s->students[st.min_idx].name);
#else
                printf(" (%s)\n", s->data[st.min_idx].name);
#endif
            } else {
                puts("");
            }
            
            printf("Grade bands — A:%d B:%d C:%d D:%d F:%d\n",
                   st.band_A, st.band_B, st.band_C, st.band_D, st.band_F);
            return true;
        }
    }
    
    // COMMAND: INSERT - Use new implementation
    if (strcmp(cmd, "insert") == 0) {
        cmd_insert(line, s);  // Pass full line to preserve quotes
        return true;
    }
    
    // COMMAND: UPDATE
    if (strcmp(cmd, "update") == 0)
        return handle_update(args ? args : "", s);
    
    // COMMAND: DELETE - Use new implementation
    if (strcmp(cmd, "delete") == 0)
        return handle_delete(args ? args : "", s);
    
    // COMMAND: QUERY
    if (strcmp(cmd, "query") == 0) {
        int id = -1;
        Student tmp;
        init_patch(&tmp);
        
        for (char *tok = strtok(args ? args : "", " "); tok; tok = strtok(NULL, " ")) {
            if (parse_kv(tok, &tmp)) {
                if (tmp.id > 0) id = tmp.id;
            }
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
        
#if USE_FIXED_SIZE_ARRAY
        const Student *st = &s->students[idx];
#else
        const Student *st = &s->data[idx];
#endif
        printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
        return true;
    }
    
    // COMMAND: HELP
    if (strcmp(cmd, "help") == 0) {
        print_menu();
        return true;
    }
    
    // COMMAND: EXIT/QUIT
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        return false;
    }

    printf("Unknown command: %s (type HELP)\n", cmd);
    return true;
}

/*******************************************************************************
 * PRINT DECLARATION
 ******************************************************************************/
void print_declaration(const char *team_name, const char *members_csv, const char *date_str) {
    puts("============================================");
    puts("We declare that this is our own work and ...");
    puts("(Place the exact provided declaration text here.)");
    printf("Team: %s\nMembers: %s\nDate: %s\n", team_name, members_csv, date_str);
    puts("============================================");
}
