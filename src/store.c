#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "store.h"
#include "util.h"

#define START_CAP 16

static bool ensure_cap(Store *s, size_t need) {
    if (s->cap >= need) {
        return true;
    }
    size_t new_cap = s->cap ? s->cap : START_CAP;
    while (new_cap < need) {
        // Check for overflow
        if (new_cap > SIZE_MAX / 2) {   
            fprintf(stderr, "Error: Store capacity limit reachewd.\n");
            return false;
        }
        new_cap *= 2;
    }

    // Check for overflow during byte size calculation
    if (new_cap > SIZE_MAX / sizeof(Student)) {
        fprintf(stderr, "Error: Store capacity limit reached.\n");
        return false;
    }

    Student *new_alloc = realloc(s->data, new_cap * sizeof(Student));
    if (!new_alloc) {
        return false;
    }
    s->data = new_alloc;
    s->cap = new_cap;
    return true;
}

void store_init(Store *s) {
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
    s->is_dirty = false;
    s->loaded = false;
}

void store_free(Store *s) {
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
    s->is_dirty = false;
    s->loaded = false;
}

int store_find_index_by_id(const Store *s, int id) {
    for (size_t i = 0; i < s->size; i++) {
        if (s->data[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

bool store_insert(Store *s, Student st) {
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

    if (store_find_index_by_id(s, st.id) != -1) {
        return false; // Duplicate ID
    }

    if (!ensure_cap(s, s->size + 1)) {
        return false; // Memory allocation failed
    }

    s->data[s->size++] = st;
    s->is_dirty = true;
    return true;
}



/*
   This function locates a student record by ID and updates specified fields with new values.
    
    Parameters:
    1. 's' -> pointer to the Store structure containing all student records
    2. 'id' -> the ID of the student record to update (used to locate the record)
    3. 'patch' -> pointer to a Student structure containing the new values to update
    
    Functionality:
    - Searches for the student record with the specified ID in the store
    - If ID field in patch is provided and different from current ID:
        * Validates the new ID format
        * Checks that the new ID doesn't already exist in the database
        * Updates the ID if valid
    - If Name/Programme field in patch is non-empty and Mark field in patch is non-negative:
        * Validates the name & programme text, & mark value
        * Updates the name/programme/mark field if valid
    - Only updates fields that are provided in the patch (partial updates supported)
    
    Returns:
    - true if the record was found and all provided fields were successfully updated
    - false if:
        * The ID is not found in the store
        * Any validation fails (invalid ID, duplicate ID, invalid text, invalid mark)
    
    Note: Empty strings in Name/Programme and negative Mark values are treated as "no update"
*/
bool store_update(Store *s, int id, const Student *patch) {
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;
    Student *cur = &s->data[idx];
    if (patch->id > 0 && patch->id != id) {
        if (!valid_id(patch->id)) return false;
        if (store_find_index_by_id(s, patch->id) != -1) return false;
        cur->id = patch->id;
    }

    if (patch->name[0] != '\0') {
        if (!valid_text(patch->name)) return false;
        strncpy(cur->name, patch->name, sizeof(cur->name));
        cur->name[sizeof(cur->name)-1] = '\0';
    }

    if (patch->programme[0] != '\0') {
        if (!valid_text(patch->programme)) return false;
        strncpy(cur->programme, patch->programme, sizeof(cur->programme));
        cur->programme[sizeof(cur->programme)-1] = '\0';
    }

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
    s->data[idx] = s->data[s->size - 1]; // Swap with last student record
    s->size--;
    s->is_dirty = true;
    return true;
}