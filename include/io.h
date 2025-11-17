#ifndef IO_H
#define IO_H
#include "store.h"
#include <stdbool.h>

bool cms_load(const char *path, Store *s, int *skipped_lines);
bool cms_save(const char *path, const Store *s);

#endif // IO_H