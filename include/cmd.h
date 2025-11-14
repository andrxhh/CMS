#ifndef CMD_H
#define CMD_H
#include "store.h"
#include <stdbool.h>


// Process single input line, returns false if user requested to exit.
bool cmd_process_line(const char* line, Store *s, const char *db_path);


// Print the declaration block with team and member names and date.
void print_declaration(const char *team_name, const char *member_names, const char *date_str);

#endif // CMD_H