#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include "store.h"

/* 
 * FILE I/O OPERATIONS
 */

// Load/save the single database file (P6_5.txt format)
bool cms_load(const char *path, Store *s, int *skipped_lines);
bool cms_save(const char *path, const Store *s);

#endif // IO_H
