#ifndef CMD_H
#define CMD_H

#include <stdbool.h>
#include "store.h"

/* 
 * COMMAND PROCESSING
 */

// Process a single input line (without trailing \n)
// Returns false to request exit
bool cmd_process_line(const char *line, Store *s, const char *db_path);

// Print the startup non-plagiarism declaration block
void print_declaration(const char *team_name, const char *members_csv, const char *date_str);

/*
 * SPECIFIC COMMAND IMPLEMENTATIONS
 * These are exposed for use by other modules (like cms.c, cms_interactive.c)
 */

// INSERT command - exposed for external use
void cmd_insert(const char *line, Store *s);

// DELETE command - exposed for external use  
bool handle_delete(const char *args, Store *s);

#endif // CMD_H
