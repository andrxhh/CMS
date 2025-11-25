#ifndef CMS_CMD_INTERNAL_H
#define CMS_CMD_INTERNAL_H

#include <stdbool.h>
#include "student.h"
#include "store.h"

/* Parsing helpers */
bool parse_args_to_patch(char *args, Student *patch);
bool parse_single_id_command(char *args, const char *cmd_name, int *out_id);

/* Handlers */
bool handle_insert(char *args, Store *s);
bool handle_update(char *args, Store *s);
bool handle_delete(char *args, Store *s);
bool handle_query(char *args, const Store *s);
bool handle_find(char *args, Store *s);

/* Display helpers */
void show_all(const Store *s);
void print_declaration(const char *team_name, const char *members_csv, const char *date_str);

#endif /* CMS_CMD_INTERNAL_H */
