/* 
 * IO.C - Input/Output operations for loading and saving the database
 * 
 * This file handles reading from and writing to the database file
 */

#include <stdio.h>   // For FILE, fopen, fclose, fgets, fprintf
#include <string.h>  // For string functions
#include "io.h"      // Our header
#include "util.h"    // Utility functions

/* 
 * LOAD DATABASE FROM FILE
 * 
 * Reads student records from the database file and loads them into the store
 */
bool cms_load(const char *path, Store *s, int *skipped_lines) {
    FILE *f = fopen(path, "r");
    if (!f) return false;  // File doesn't exist or can't be opened
    
    *skipped_lines = 0;
    char line[512];
    bool in_data = false;  // Flag to track if we've found the data section
    
    // Read file line by line
    while (fgets(line, sizeof line, f)) {
        str_trim(line);  // Remove whitespace
        
        // Skip empty lines
        if (line[0] == '\0') continue;
        
        // Look for the table header line (ID Name Programme Mark)
        if (!in_data) {
            if (strstr(line, "ID") && strstr(line, "Name") && 
                strstr(line, "Programme") && strstr(line, "Mark")) {
                in_data = true;  // Found data section
            }
            continue;
        }
        
        // Parse data line: ID Name Programme Mark
        Student st;
        char name_buf[256], prog_buf[256];
        
        // Parse tab-separated values
        int fields = sscanf(line, "%d %[^\t] %[^\t] %f", 
                           &st.id, name_buf, prog_buf, &st.mark);
        
        if (fields == 4) {
            // Copy and trim strings
            strncpy(st.name, name_buf, sizeof st.name - 1);
            st.name[sizeof st.name - 1] = '\0';
            str_trim(st.name);
            
            strncpy(st.programme, prog_buf, sizeof st.programme - 1);
            st.programme[sizeof st.programme - 1] = '\0';
            str_trim(st.programme);
            
            // Insert student into store
            if (!store_insert(s, st)) {
                (*skipped_lines)++;
            }
        } else {
            (*skipped_lines)++;
        }
    }
    
    fclose(f);
    return s->size > 0 || *skipped_lines > 0;
}

/* 
 * SAVE DATABASE TO FILE
 * 
 * Writes all student records from the store to the database file
 */
bool cms_save(const char *path, const Store *s) {
    FILE *f = fopen(path, "w");
    if (!f) return false;
    
    // Write header
    fprintf(f, "Database Name: Sample-CMS\n");
    fprintf(f, "Authors: Student Project\n");
    fprintf(f, "\n");
    fprintf(f, "Table Name: StudentRecords\n");
    fprintf(f, "ID\tName\t\tProgramme\t\tMark\n");
    
    // Write all student records
    for (size_t i = 0; i < s->size; i++) {
        const Student *st = &s->data[i];
        fprintf(f, "%d\t%s\t%s\t%.2f\n", 
                st->id, st->name, st->programme, st->mark);
    }
    
    fclose(f);
    return true;
}

