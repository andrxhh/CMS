#ifndef STORE_H
#define STORE_H
#include <stddef.h>
#include <stdbool.h>
#include "student.h"

typedef struct {
    Student *data;
    size_t size;
    size_t cap;
} Store;

// Lifecycle
void store_init(Store *s);
void store_free(Store *s);

// Core ops
int  store_find_index_by_id(const Store *s, int id); // -1 if not found
bool store_insert(Store *s, Student st);             // false if dup/invalid
bool store_update(Store *s, int id, const Student *patch); // patch uses sentinel values
bool store_delete(Store *s, int id);

#endif // STORE_H

