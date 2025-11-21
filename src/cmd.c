#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cmd.h"
#include "io.h"
#include "stats.h"
#include "sort.h"
#include "util.h"
#include "cmd_internal.h"

/* Check that a command that expects no arguments was not given any.
   Trim whitespace and print an error message if unexpected args exist. */
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

/* Process a single input command line.
    Returns true to continue the main loop, false to exit program.
    Handles file ops, show, insert/update/delete/query/find/help/exit. */
bool cmd_process_line(const char *line_in, Store *s, const char *db_path) {
     /* Make a modifiable copy of the input line */
     char line[512];
     strncpy(line, line_in, sizeof(line));
     line[sizeof(line) - 1] = '\0';

     /* Ignore blank lines and comment lines starting with '#' or '--' */
     char *tmp = line;
     while (*tmp && isspace((unsigned char)*tmp)) tmp++;
     if (*tmp == '\0') return true; // blank
     if (*tmp == '#') return true; // comment
     if (*tmp == '-' && *(tmp+1) == '-') return true; // comment start with --

     /* Split command and arguments */
     char *p = line;
     while (*p && !isspace((unsigned char)*p)) p++;
     char *cmd = line;
     char *args = NULL;
     if (*p) {
          *p = '\0';
          args = p + 1;
     }
     str_tolower(cmd);

     if (strcmp(cmd, "open") == 0 || strcmp(cmd, "load") == 0) {
          if (!has_no_args(args, "OPEN")) {
                return true;
          }

          if (s->loaded) {
                printf("You have previously loaded the database from %s. Do you want to load again? (Y/N): ", db_path);
                char buf[16];
                fgets(buf, sizeof(buf), stdin);
                if (buf[0] != 'Y' && buf[0] != 'y') {
                     printf("Load cancelled...\n");
                     return true;
                }
          }

          int skipped = 0;
          store_free(s);
          store_init(s);
          if (cms_load(db_path, s, &skipped)) {
                if(s->size == 0) {
                     printf("File loaded successfully, no valid records found!\n");
                } else {
                     printf("Database loaded. Total %zu records, skipped %d line(s).\n", s->size, skipped);
                     s->loaded = true;
                }
          } else {
                fprintf(stderr, "Failed to load database from %s\n", db_path);
          }

          return true;
     }

     if (strcmp(cmd, "save") == 0) {
          /* SAVE [path] -> optional filename argument */
          if (args) {
                str_trim(args);
          }
          const char *save_path = db_path;
          if (args && args[0] != '\0') {
                save_path = args; // save to specified path
          }

          if (cms_save(save_path, s)) {
                printf("Database saved to %s\n", save_path);
          } else {
                fprintf(stderr, "Failed to save database to %s\n", save_path);
          }
          return true;
     }

     if (strcmp(cmd, "show") == 0) {
          /* SHOW ALL [SORT BY ...] or SHOW SUMMARY */
          if (!args || strncasecmp(args, "summary", 7) != 0) {
                /* maybe has sorting clause */
                bool sorted = false, asc = true; SortKey key = SORT_BY_ID;
                if (args && str_icontains(args, "sort by")) {
                     sorted = true;
                     if (str_icontains(args, "mark")) key = SORT_BY_MARK;
                     if (str_icontains(args, "desc")) asc = false;
                }
                if (sorted) store_sort(s, key, asc);
                     show_all(s);
          } else {
                     /* SHOW SUMMARY prints aggregated statistics */
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
                   /* Error messages already printed by handler */
              }

          return true;
     }
     if (strcmp(cmd, "update") == 0) {
             if (!handle_update(args ? args : "", s)) {
                   /* Error messages already printed by handler */
              }

          return true;
     }

     if (strcmp(cmd, "delete") == 0) {
             if (!handle_delete(args ? args : "", s)) {
                   /* Error messages already printed by handler */
              }

          return true;
     }

     if (strcmp(cmd, "query") == 0) {
             if (!handle_query(args ? args : "", s)) {
                   /* Error messages already printed by handler */
              }
          return true;
     }

     if (strcmp(cmd, "find") == 0) {
             if (!handle_find(args ? args : "", s)) {
                   /* Error messages already printed by handler */
              }
          return true;
     }

     if (strcmp(cmd, "help") == 0) {
          if (!has_no_args(args, "HELP")) return true;

          puts("Available commands:");
          printf("\n--- FILE OPERATIONS ---\n");
          printf("  OPEN                 : Load database from file (discards unsaved changes).\n");
          printf("  SAVE                 : Save current memory to file.\n");
          printf("  EXIT / QUIT          : Exit the program.\n");

          printf("\n--- VIEWING DATA ---\n");
          printf("  SHOW ALL [SORT BY..] : Display all records.\n");
          printf("                         Flags: SORT BY ID | SORT BY MARK [ASC|DESC]\n");
          printf("  SHOW SUMMARY         : Display statistics (Count, Avg, Min, Max, Grade Bands).\n");
          printf("  QUERY ID=<id>        : View a single student by ID.\n");
          printf("  FIND <Col> <Op> <Val>: Search filter.\n");
          printf("                         Cols: Name, Programme, Mark.\n");
          printf("                         Ops : =, CONTAINS, <, >, <=, >=.\n");

          printf("\n--- EDITING DATA ---\n");
          printf("  INSERT <k>=<v> ...   : Add a new student.\n");
          printf("                         Req: ID, Name, Programme, Mark.\n");
          printf("  UPDATE <k>=<v> ...   : Update a student. ID is required to match record.\n");
          printf("  DELETE ID=<id>       : Delete a student (prompts for confirmation).\n");

          printf("\n--- HELP ---\n");
          printf("  HELP                 : Show this help text.\n");

          printf("\n--- NOTES ---\n");
          puts("  - Keys are case-insensitive (ID, Name, Programme, Mark).");
          puts("  - ID must be an integer; Mark is a floating point number.");
          puts("  - For multi-word values enclose them in double quotes: Name=\"John Smith\".");
          puts("  - When parsing key=value pairs, spaces separate tokens; quoted values may contain spaces.");
          puts("  - Use OPEN to reload the DB file; this will discard unsaved in-memory changes.");
          puts("  - Use SAVE to write current in-memory data to the DB file.");

          printf("\n--- EXAMPLES ---\n");
          puts("  INSERT ID=2 Name=\"Alice Lee\" Programme=IT Mark=72.0");
          puts("  UPDATE ID=2 Mark=75.5");
          puts("  SHOW ALL SORT BY MARK DESC");
          puts("  FIND Name CONTAINS \"Wang\"");
          puts("  FIND Mark >= 85");
          return true;
     }
     if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
          if (s->is_dirty) {
                printf("You have unsaved changes. Do you want to save before exiting? (Y/N): ");
                char buf[16];
                fgets(buf, sizeof(buf), stdin);
                if (buf[0] == 'Y' || buf[0]=='y') {
                     if (cms_save(db_path, s)) {
                          printf("Database saved to %s\n", db_path);
                     } else {
                          fprintf(stderr, "Failed to save database to %s\n", db_path);
                          return true;
                     }
                } else {
                     printf("Exiting without saving...\n");
                }
          }
          return false; 
     }


     printf("Unknown command: %s (type HELP)\n", cmd);
     return true;
}

/* `print_declaration` moved to `cmd_display.c` to keep display logic separate. */
