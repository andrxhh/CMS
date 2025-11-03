#ifndef IO_H
#define IO_H
#include <stdbool.h>
#include "store.h"

// Load/save the single database file (TeamName-CMS.txt)
bool cms_load(const char *path, Store *s, int *skipped_lines);
bool cms_save(const char *path, const Store *s);

#endif // IO_H

