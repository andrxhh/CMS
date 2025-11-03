#ifndef CMD_H
#define CMD_H
#include <stdbool.h>
#include "store.h"

// Process a single input line (without trailing \n). Returns false to request exit.
bool cmd_process_line(const char *line, Store *s, const char *db_path);

// Print the startup non-plagiarism declaration block.
void print_declaration(const char *team_name, const char *members_csv, const char *date_str);

#endif // CMD_H
