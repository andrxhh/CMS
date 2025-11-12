/* 
 * IO.C - Input/Output operations for loading and saving the database
 * 
 * Integrated version with your P6_5.txt file format support
 */

#include <stdio.h>
#include <string.h>
#include "io.h"
#include "util.h"
#include "cmd.h"  // For cmd_insert

/*******************************************************************************
 * LOAD DATABASE FROM FILE
 * 
 * Your implementation - reads P6_5.txt format with header lines
 ******************************************************************************/
bool cms_load(const char *path, Store *s, int *skipped_lines) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("INFO: Could not open %s (file may not exist yet)\n", path);
        return false;
    }
    
    char line[512];
    int line_num = 0;
    bool in_data = false;
    
    if (skipped_lines) *skipped_lines = 0;
    
    printf("Loading existing data from %s...\n", path);
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Remove newline
        line[strcspn(line, "\n")] = '\0';
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Look for the table header line
        if (!in_data) {
            if (strstr(line, "ID") && strstr(line, "Name") &&
                strstr(line, "Programme") && strstr(line, "Mark")) {
                in_data = true;
            }
            continue;
        }
        
        // Parse data line: ID, Name, Programme, Mark (tab-separated)
        int id;
        char name[256];
        char programme[256];
        float mark;
        
        // Try tab-separated format first
        int fields = sscanf(line, "%d\t%255[^\t]\t%255[^\t]\t%f",
                           &id, name, programme, &mark);
        
        if (fields == 4) {
            // Create INSERT command and use cmd_insert
            char insert_cmd[512];
            
            // Trim strings
            str_trim(name);
            str_trim(programme);
            
            snprintf(insert_cmd, sizeof(insert_cmd),
                    "INSERT ID=%d Name=\"%s\" Programme=\"%s\" Mark=%.1f",
                    id, name, programme, mark);
            
            cmd_insert(insert_cmd, s);
        } else {
            if (skipped_lines) (*skipped_lines)++;
        }
    }
    
    fclose(file);
    
#if USE_FIXED_SIZE_ARRAY
    printf("Loaded %d students from file.\n\n", s->count);
    return s->count > 0;
#else
    printf("Loaded %zu students from file.\n\n", s->size);
    return s->size > 0;
#endif
}

/*******************************************************************************
 * SAVE DATABASE TO FILE
 * 
 * Your implementation - writes P6_5.txt format
 ******************************************************************************/
bool cms_save(const char *path, const Store *s) {
    FILE *file = fopen(path, "w");
    if (!file) {
        printf("ERROR: Could not open %s for writing\n", path);
        return false;
    }
    
    // Write header
    fprintf(file, "Database Name: Sample-CMS\n");
    fprintf(file, "Authors: Assistant Prof Oran Zane Devilly\n");
    fprintf(file, "\n");
    fprintf(file, "Table Name: StudentRecords\n");
    fprintf(file, "ID\tName\t\tProgramme\t\tMark\n");
    
    // Write all students
#if USE_FIXED_SIZE_ARRAY
    for (int i = 0; i < s->count; i++) {
        const Student *st = &s->students[i];
#else
    for (size_t i = 0; i < s->size; i++) {
        const Student *st = &s->data[i];
#endif
        fprintf(file, "%d\t%s\t%s\t%.1f\n",
               st->id, st->name, st->programme, st->mark);
    }
    
    fclose(file);
    
#if USE_FIXED_SIZE_ARRAY
    printf("Saved %d students to %s\n", s->count, path);
#else
    printf("Saved %zu students to %s\n", s->size, path);
#endif
    
    return true;
}
