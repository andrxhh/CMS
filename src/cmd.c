#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cmd.h"
#include "io.h"
#include "sort.h"
#include "stats.h"
#include "util.h"

// Simple key=value parser supporting quotes; stores into a patch Student
static void init_patch(Student *p) {
    memset(p, 0, sizeof *p);
    p->id = -1; // sentinel: unchanged
    p->mark = -1.0f; // sentinel: unchanged
}



static bool parse_kv(char *token, Student *patch) {
    char *eq = strchr(token, '='); if (!eq) return false; *eq = '\0';
    char *key = token; char *val = eq + 1;
    str_trim(key); str_trim(val);
    if (val[0] == '"') { // quoted value
        size_t L = strlen(val);
        if (L >= 2 && val[L-1] == '"') { val[L-1] = '\0'; memmove(val, val+1, L-1); }
    }
    if (str_ieq(key, "ID")) {
        int id; if (!parse_int(val, &id)) return false; patch->id = id; return true;
    } else if (str_ieq(key, "Name")) {
        strncpy(patch->name, val, sizeof patch->name); patch->name[sizeof patch->name - 1] = '\0'; return true;
    } else if (str_ieq(key, "Programme") || str_ieq(key, "Program") || str_ieq(key, "Prog")) {
        strncpy(patch->programme, val, sizeof patch->programme); patch->programme[sizeof patch->programme - 1] = '\0'; return true;
    } else if (str_ieq(key, "Mark")) {
        float m; if (!parse_float(val, &m)) return false; patch->mark = m; return true;
    }
    return false;
}



static bool handle_update(char *args, Store *s) {
    Student patch; init_patch(&patch);
    for (char *tok = strtok(args, " "); tok; tok = strtok(NULL, " ")) {
        if (!parse_kv(tok, &patch)) { fprintf(stderr, "Invalid token: %s\n", tok); return false; }
    }
    if (patch.id <= 0) { fputs("UPDATE requires ID=...\n", stderr); return false; }
    int target_id = patch.id; patch.id = -1; // prevent self-copy unless explicitly changing ID
    if (!store_update(s, target_id, &patch)) { puts("Update failed (not found/invalid)." ); return false; }
    puts("Record successfully updated."); return true;
}



bool cmd_process_line(const char *line_in, Store *s, const char *db_path) {
    // make a modifiable copy
    char line[512]; strncpy(line, line_in, sizeof line); line[sizeof line - 1] = '\0';
    str_trim(line); if (line[0] == '\0') return true; // ignore empty
    // split command and args
    char *p = line; while (*p && !isspace((unsigned char)*p)) p++; char *cmd = line; char *args = NULL; if (*p) { *p = '\0'; args = p + 1; }
    str_tolower(cmd);

    if (strcmp(cmd, "open") == 0) {
        int skipped = 0; store_free(s); store_init(s);
        if (cms_load(db_path, s, &skipped)) {
            printf("Database opened. (skipped %d bad line(s))\n", skipped);
        } else {
            puts("No existing database found. A new one will be created on SAVE.");
        }
        return true;
    }
    if (strcmp(cmd, "save") == 0) { if (cms_save(db_path, s)) puts("Database successfully saved."); else puts("Save failed."); return true; }
    if (strcmp(cmd, "show") == 0) {
        // SHOW [ALL] [SORT BY ID|MARK [ASC|DESC]] | SHOW SUMMARY
        if (!args || strncasecmp(args, "summary", 7) != 0) {
            // maybe has sorting clause
            bool sorted = false, asc = true; SortKey key = SORT_BY_ID;
            if (args && strcasestr(args, "sort by")) {
                sorted = true;
                if (strcasestr(args, "mark")) key = SORT_BY_MARK;
                if (strcasestr(args, "desc")) asc = false;
            }
            if (sorted) store_sort(s, key, asc);
            show_all(s);
            return true;
        } else {
            Stats st = compute_stats(s->data, s->size);
            printf("Total: %zu\nAverage: %.2f\nHighest: %.2f", st.count, st.average, st.max_mark);
            if (st.max_idx >= 0) printf(" (%s)\n", s->data[st.max_idx].name); else puts("");
            printf("Lowest: %.2f", st.min_mark);
            if (st.min_idx >= 0) printf(" (%s)\n", s->data[st.min_idx].name); else puts("");
            printf("Grade bands — A:%d B:%d C:%d D:%d F:%d\n", st.band_A, st.band_B, st.band_C, st.band_D, st.band_F);
            return true;
        }
    }
    if (strcmp(cmd, "insert") == 0) return handle_insert(args ? args : "", s);
    if (strcmp(cmd, "update") == 0) return handle_update(args ? args : "", s);
    if (strcmp(cmd, "delete") == 0) return handle_delete(args ? args : "", s);
    if (strcmp(cmd, "query") == 0) {
        int id = -1; Student tmp; init_patch(&tmp);
        for (char *tok = strtok(args ? args : "", " "); tok; tok = strtok(NULL, " ")) if (parse_kv(tok, &tmp)) { if (tmp.id > 0) id = tmp.id; }
        if (id <= 0) { fputs("QUERY requires ID=...\n", stderr); return true; }
        int idx = store_find_index_by_id(s, id);
        if (idx < 0) { puts("Record does not exist."); return true; }
        const Student *st = &s->data[idx];
        printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
        return true;
    }
    if (strcmp(cmd, "help") == 0) {
        puts("Commands: OPEN | SAVE | SHOW [ALL] [SORT BY ID|MARK [ASC|DESC]] | SHOW SUMMARY | INSERT k=v... | UPDATE k=v... | DELETE ID=.. | QUERY ID=.. | HELP | EXIT");
        return true;
    }
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) { return false; }

    printf("Unknown command: %s (type HELP)\n", cmd);
    return true;
}

void print_declaration(const char *team_name, const char *members_csv, const char *date_str) {
    puts("============================================");
    puts("We declare that this is our own work and ...");
    puts("(Place the exact provided declaration text here.)");
    printf("Team: %s\nMembers: %s\nDate: %s\n", team_name, members_csv, date_str);
    puts("============================================");
}
