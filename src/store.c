#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "store.h"
#include "util.h"

/*
This file implements the Store structure and its associated functions for managing
a dynamic array of Student records. It includes initialization, cleanup, insertion,
updating, deletion, and searching functionalities. The Store dynamically resizes
its internal array as needed, ensuring efficient memory usage and data integrity
through validation checks on Student fields.
*/

#define START_CAP 16

static bool ensure_cap(Store *s, size_t need) {
    if (s->cap >= need) {
        return true;
    }
    size_t new_cap = s->cap ? s->cap : START_CAP;
    while (new_cap < need) {
        // Grow capacity by doubling until it's >= need, checking for overflow
        if (new_cap > SIZE_MAX / 2) {   
            fprintf(stderr, "Error: Store capacity limit reachewd.\n");
            return false;
        }
        new_cap *= 2;
    }

    // Prevent overflow when calculating new_alloc byte size
    if (new_cap > SIZE_MAX / sizeof(Student)) {
        fprintf(stderr, "Error: Store capacity limit reached.\n");
        return false;
    }

    // Try to reallocate the underlying array to the new capacity
    Student *new_alloc = realloc(s->data, new_cap * sizeof(Student));
    if (!new_alloc) {
        return false;
    }
    s->data = new_alloc;
    s->cap = new_cap;
    return true;
}

void store_init(Store *s) {
    // Initialize store fields to empty/zero state
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
    s->is_dirty = false;
    s->loaded = false;
}

void store_free(Store *s) {
    // Free internal buffer and reset metadata
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
    s->is_dirty = false;
    s->loaded = false;
}

int store_find_index_by_id(const Store *s, int id) {
    // Linear search for student record with matching id
    for (size_t i = 0; i < s->size; i++) {
        if (s->data[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

bool store_insert(Store *s, Student st) {
    // Validate student fields before insertion to ensure data integrity
    if (!valid_id(st.id)) {
        fprintf(stderr, "Invalid ID: %d\n", st.id);
        return false;
    }
    if (!valid_text(st.name)) {
        fprintf(stderr, "Invalid Name: %s\n", st.name);
        return false;
    }
    if (!valid_text(st.programme)) {
        fprintf(stderr, "Invalid Programme: %s\n", st.programme);
        return false;
    }
    if (!valid_mark(st.mark)) {
        fprintf(stderr, "Invalid Mark: %.2f\n", st.mark);
        return false;
    }

    // Prevent inserting duplicate IDs
    if (store_find_index_by_id(s, st.id) != -1) {
        return false; // Duplicate ID
    }

    // Ensure there is enough capacity for one more student
    if (!ensure_cap(s, s->size + 1)) {
        return false; // Memory allocation failed
    }

    // Append student and mark store as modified
    s->data[s->size++] = st;
    s->is_dirty = true;
    return true;
}

bool store_update(Store *s, int id, const Student *patch) {
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;
    Student *cur = &s->data[idx];

    // If patch requests an id change, validate new id and prevent duplicates
    if (patch->id > 0 && patch->id != id) {
        if (!valid_id(patch->id)) return false;
        if (store_find_index_by_id(s, patch->id) != -1) return false;
        cur->id = patch->id;
    }

    // If name provided in patch, validate and copy safely
    if (patch->name[0] != '\0') {
        if (!valid_text(patch->name)) return false;
        strncpy(cur->name, patch->name, sizeof(cur->name));
        cur->name[sizeof(cur->name)-1] = '\0';
    }

    // If programme provided in patch, validate and copy safely
    if (patch->programme[0] != '\0') {
        if (!valid_text(patch->programme)) return false;
        strncpy(cur->programme, patch->programme, sizeof(cur->programme));
        cur->programme[sizeof(cur->programme)-1] = '\0';
    }

    // If mark provided in patch (non-negative), validate and update
    if (patch->mark >= 0.0f) {
        if (!valid_mark(patch->mark)) return false;
        cur->mark = patch->mark;
    }
    s->is_dirty = true;
    return true;
}

bool store_delete(Store *s, int id) {
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;
    // Remove by swapping with last element to keep array compact in O(1)
    s->data[idx] = s->data[s->size - 1];
    s->size--;
    s->is_dirty = true;
    return true;
}