#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "io.h"
#include "store.h"
#include "student.h"

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

    while (fgets(line, sizeof line, fp)) {
        strip_eol(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue; // Skip empty lines and comments
        }

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

        int id;
        float mark;
        if (!parse_int(id_str, &id) || !parse_float(mark_str, &mark)) {
            skipped++;
            continue; // Parsing error
        }

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

    return true;
}

bool cms_save(const char *path, const Store *s) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        return false; // Unable to open file for writing
    }

    for (size_t i = 0; i < s->size; i++) {
        const Student *st = &s->data[i];
        fprintf(fp, "%d\t%s\t%s\t%.1f\n", st->id, st->name, st->programme, st->mark);
    }

    fclose(fp);
    return true;
}