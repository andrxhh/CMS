#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "io.h"
#include "store.h"
#include "student.h"

/*
This file handles loading and saving the student records to/from a text file.
*/

// Helper: strip trailing newline and optional carriage return
static void strip_eol(char *line) {
    char *nl = strchr(line, '\n');
    if (nl) *nl = '\0';
    size_t len = strlen(line);
    if (len && line[len - 1] == '\r') {
        line[len - 1] = '\0';
    }
}

// Expect tab-separated values: id, name, programme, mark
bool cms_load(const char *path, Store *s, int *skipped_lines) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return false; // File missing is not fatal, caller proceeds with empty store
    }

    char line[512];
    int skipped = 0;
    int line_num = 0;

    while (fgets(line, sizeof line, fp)) {
        line_num++;
        // Check if line was truncated (no newline found)
        size_t line_len = strlen(line);
        if (line_len == sizeof(line) - 1 && line[line_len - 1] != '\n') {
            fprintf(stderr, "Warning: Skipping overly long line in data file. <line %d>\n", line_num);
            // Line overflows buffer, skip rest of line
            int ch;
            while ((ch = fgetc(fp)) != '\n' && ch != EOF);
            skipped++;
            continue; // Skip this malformed line
        }

        strip_eol(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue; // Skip empty lines and comments
        }
        
        // Tokenize line
        char *id_str = strtok(line, "\t");
        char *name_str = strtok(NULL, "\t");
        char *programme_str = strtok(NULL, "\t");
        char *mark_str = strtok(NULL, "\t");

        if (!id_str || !name_str || !programme_str || !mark_str) {
            skipped++;
            continue; // Malformed line
        }

        // Trim whitespace
        str_trim(id_str); str_trim(name_str); str_trim(programme_str); str_trim(mark_str);
        
        // Parse and validate id and mark
        int id;
        float mark;
        if (!parse_int(id_str, &id) || !parse_float(mark_str, &mark)) {
            skipped++;
            continue;
        }

        // Validate fields are within expected lengths
        if (strlen(name_str) >= sizeof(((Student *)0)->name) || strlen(programme_str) >= sizeof(((Student *)0)->programme)) {
            fprintf(stderr, "Warning: Record skipped due to field being too long: ID %s.\n", id_str);
            skipped++;
            continue; 
        }

        // Create student record (temporary) and insert
        Student st = {0};
        st.id = id;
        st.mark = mark;
        strncpy(st.name, name_str, sizeof st.name); st.name[sizeof st.name - 1] = '\0';
        strncpy(st.programme, programme_str, sizeof st.programme); st.programme[sizeof st.programme - 1] = '\0';

        if(!store_insert(s, st)) {
            skipped++;
            continue; // Invalid data or duplicate
        }
    }

    fclose(fp);
    if (skipped_lines) {
        *skipped_lines = skipped;
    }
    
    // Reset dirty flag as we just loaded from file
    s->is_dirty = false;
    return true;
}

bool cms_save(const char *path, const Store *s) {
    char filename[512];
    const char *base = path;
    const char *p1 = strrchr(path, '/');
    const char *p2 = strrchr(path, '\\');
    if (p1 || p2) base = ((p1 > p2) ? p1 : p2) + 1;

    // Copy base name and ensure .txt extension
    strncpy(filename, base, sizeof filename - 1);
    filename[sizeof filename - 1] = '\0';
    char *ext = strrchr(filename, '.');
    if (!ext || (strcmp(ext, ".txt") != 0 && strcmp(ext, ".TXT") != 0)) {
        strncat(filename, ".txt", sizeof filename - strlen(filename) - 1);
    }

    // Build final path under db/ directory
    char final_path[768];
    snprintf(final_path, sizeof final_path, "db/%s", filename);

    FILE *fp = fopen(final_path, "w");
    if (!fp) {
        return false; // Unable to open file for writing
    }

    for (size_t i = 0; i < s->size; i++) {
        const Student *st = &s->data[i];
        fprintf(fp, "%d\t%s\t%s\t%.1f\n", st->id, st->name, st->programme, st->mark);
    }
    fclose(fp);

    // Reset dirty flag
    ((Store *)s)->is_dirty = false;
    return true;
}