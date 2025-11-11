/******************************
 * FILE: cmd.c
 * 
 * WHAT THIS FILE DOES:
 * This file handles all COMMAND PROCESSING for your CMS (Course Management System).
 * It takes user input like "OPEN" or "SHOW ALL" and executes the appropriate action.
 ******************************/

#include <stdio.h>   // For printf, puts, fprintf
#include <string.h>  // For string functions
#include <stdlib.h>  // For standard library functions
#include <ctype.h>   // For character checking
#include "cmd.h"     // Our header
#include "io.h"      // For cms_load, cms_save
#include "sort.h"    // For store_sort
#include "stats.h"   // For compute_stats
#include "util.h"    // For utility functions

/* 
 * INITIALIZE PATCH STRUCTURE
 * 
 * A "patch" is a partial Student object - only some fields are set.
 * We use -1 for ID and mark as "sentinel values" meaning "not set/unchanged".
 */
static void init_patch(Student *p) {
    memset(p, 0, sizeof *p);  // Set all bytes to 0
    p->id = -1;               // Sentinel: -1 means "not set"
    p->mark = -1.0f;          // Sentinel: -1.0 means "not set"
}

/* 
 * SHOW ALL STUDENTS - YOUR IMPLEMENTATION!
 * 
 * Displays all students in a table format.
 * This is one of the two main functions you need to implement.
 * 
 * Example output:
 * ID      Name    Programme       Mark
 * 2301234 John    CS              85.50
 */
static void show_all(const Store *s) {
    // Check if store is empty
    if (s->size == 0) { 
        puts("No student records to display."); 
        return; 
    }
    
    // Print table header
    puts("ID\tName\tProgramme\tMark"); 
    
    // Loop through all students and print each one
    for (size_t i = 0; i < s->size; ++i) {
        const Student *st = &s->data[i];
        printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
    }
}

/* 
 * PARSE KEY=VALUE PAIR
 * 
 * This function parses a token like "ID=1" or "Name=John"
 * and stores it into the patch Student structure.
 */
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

/* 
 * HANDLE INSERT COMMAND
 * 
 * Processes INSERT command: "INSERT ID=1 Name=John Programme=CS Mark=85.5"
 */
static bool handle_insert(char *args, Store *s) {
    Student patch; 
    init_patch(&patch);
    
    for (char *tok = strtok(args, " "); tok; tok = strtok(NULL, " ")) {
        if (!parse_kv(tok, &patch)) {
            fprintf(stderr, "Invalid token: %s\n", tok);
            return false;
        }
    }
    
    // INSERT requires ALL fields
    if (patch.id <= 0 || patch.name[0] == '\0' || patch.programme[0] == '\0' || patch.mark < 0.0f) {
        fprintf(stderr, "INSERT requires ID, Name, Programme, Mark.\n"); 
        return false;
    }
    
    if (!store_insert(s, patch)) { 
        puts("Insert failed (duplicate or invalid).");
        return false; 
    }
    
    puts("Record successfully inserted."); 
    return true;
}

/* 
 * HANDLE UPDATE COMMAND
 * 
 * Processes UPDATE command: "UPDATE ID=1 Name=Jane"
 */
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

/* 
 * HANDLE DELETE COMMAND
 * 
 * Processes DELETE command: "DELETE ID=1"
 */
static bool handle_delete(char *args, Store *s) {
    int id = -1;
    Student tmp; 
    init_patch(&tmp);
    
    for (char *tok = strtok(args, " "); tok; tok = strtok(NULL, " ")) {
        if (parse_kv(tok, &tmp)) { 
            if (tmp.id > 0) id = tmp.id;
        }
    }
    
    if (id <= 0) { 
        fputs("DELETE requires ID=...\n", stderr); 
        return false; 
    }
    
    if (store_find_index_by_id(s, id) < 0) { 
        puts("Record does not exist."); 
        return false; 
    }
    
    printf("Are you sure you want to delete ID %d? (Y/N): ", id); 
    fflush(stdout);
    
    char buf[16]; 
    if (!fgets(buf, sizeof buf, stdin)) return false;
    
    if (buf[0] != 'Y' && buf[0] != 'y') { 
        puts("Deletion cancelled."); 
        return false; 
    }
    
    if (!store_delete(s, id)) { 
        puts("Delete failed."); 
        return false; 
    }
    
    puts("Record successfully deleted."); 
    return true;
}

/* 
 * MAIN COMMAND PROCESSOR - THE HEART OF THIS FILE!
 * 
 * This function processes every line of user input.
 * It includes YOUR IMPLEMENTATIONS of OPEN and SHOW ALL.
 * 
 * Returns: true = continue running, false = exit program
 */
bool cmd_process_line(const char *line_in, Store *s, const char *db_path) {
    // Make a modifiable copy
    char line[512]; 
    strncpy(line, line_in, sizeof line);
    line[sizeof line - 1] = '\0';
    
    // Remove whitespace
    str_trim(line);
    if (line[0] == '\0') return true;  // Ignore empty lines
    
    // Split command and arguments
    char *p = line; 
    while (*p && !isspace((unsigned char)*p)) p++;
    char *cmd = line;
    char *args = NULL;
    if (*p) {
        *p = '\0';
        args = p + 1;
    }
    
    // Convert command to lowercase
    str_tolower(cmd);

    /* ==========================================
     * COMMAND: OPEN - YOUR FIRST IMPLEMENTATION!
     * ==========================================
     * Load database from file
     */
    if (strcmp(cmd, "open") == 0) {
        int skipped = 0;
        
        // Clear existing data and reinitialize store
        store_free(s);
        store_init(s);
        
        // Load from file
        if (cms_load(db_path, s, &skipped)) {
            printf("CMS: The database file \"%s\" is successfully opened.\n", db_path);
        } else {
            puts("No existing database found. A new one will be created on SAVE.");
        }
        return true;
    }
    
    // COMMAND: SAVE - Save database to file
    if (strcmp(cmd, "save") == 0) { 
        if (cms_save(db_path, s)) 
            puts("Database successfully saved."); 
        else 
            puts("Save failed."); 
        return true;
    }
    
    /* ==========================================
     * COMMAND: SHOW - YOUR SECOND IMPLEMENTATION!
     * ==========================================
     * Display students or statistics
     */
    if (strcmp(cmd, "show") == 0) {
        // Check if user wants statistics
        if (!args || strncasecmp(args, "summary", 7) != 0) {
            // Show all students (maybe sorted)
            
            bool sorted = false;
            bool asc = true;
            SortKey key = SORT_BY_ID;
            
            // Parse sorting options
            if (args && strcasestr(args, "sort by")) {
                sorted = true;
                
                if (strcasestr(args, "mark")) 
                    key = SORT_BY_MARK;
                
                if (strcasestr(args, "desc")) 
                    asc = false;
            }
            
            // Apply sorting if requested
            if (sorted) 
                store_sort(s, key, asc);
            
            // Display header message
            puts("CMS: Here are all the records found in the table \"StudentRecords\".");

            // Display all students using show_all function
            show_all(s);
            return true;
            
        } else {
            // Show statistics summary
            Stats st = compute_stats(s->data, s->size);
            
            printf("Total: %zu\nAverage: %.2f\nHighest: %.2f", 
                   st.count, st.average, st.max_mark);
            
            if (st.max_idx >= 0) 
                printf(" (%s)\n", s->data[st.max_idx].name); 
            else 
                puts("");
            
            printf("Lowest: %.2f", st.min_mark);
            
            if (st.min_idx >= 0) 
                printf(" (%s)\n", s->data[st.min_idx].name); 
            else 
                puts("");
            
            printf("Grade bands — A:%d B:%d C:%d D:%d F:%d\n", 
                   st.band_A, st.band_B, st.band_C, st.band_D, st.band_F);
            return true;
        }
    }
    
    // COMMAND: INSERT
    if (strcmp(cmd, "insert") == 0) 
        return handle_insert(args ? args : "", s);
    
    // COMMAND: UPDATE
    if (strcmp(cmd, "update") == 0) 
        return handle_update(args ? args : "", s);
    
    // COMMAND: DELETE
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
        
        const Student *st = &s->data[idx];
        printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
        return true;
    }
    
    // COMMAND: HELP
    if (strcmp(cmd, "help") == 0) {
        puts("Commands: OPEN | SAVE | SHOW [ALL] [SORT BY ID|MARK [ASC|DESC]] | SHOW SUMMARY | INSERT k=v... | UPDATE k=v... | DELETE ID=.. | QUERY ID=.. | HELP | EXIT");
        return true;
    }
    
    // COMMAND: EXIT/QUIT
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) { 
        return false;  // Signal to exit program
    }

    // Unknown command
    printf("Unknown command: %s (type HELP)\n", cmd);
    return true;
}

/* 
 * PRINT DECLARATION
 * 
 * Prints a declaration/header for the project
 */
void print_declaration(const char *team_name, const char *members_csv, const char *date_str) {
    puts("============================================");
    puts("We declare that this is our own work and ...");
    puts("(Place the exact provided declaration text here.)");
    printf("Team: %s\nMembers: %s\nDate: %s\n", team_name, members_csv, date_str);
    puts("============================================");
}

