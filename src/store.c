/* 
 * STORE.C - Data storage and management functions
 * 
 * This file manages the in-memory storage of student records.
 * Integrated version supporting both fixed-size arrays and dynamic allocation.
 */

#include <stdlib.h>
#include <string.h>
#include "store.h"

/* 
 * VALIDATION FUNCTIONS
 */

bool valid_id(int id) {
    // ID must be 6-8 digits (100000 to 99999999)
    return (id >= 100000 && id <= 99999999);
}

bool valid_mark(float mark) {
    // Mark must be between 0.0 and 100.0
    return (mark >= 0.0f && mark <= 100.0f);
}

bool valid_text(const char *text) {
    // Text must be non-empty and not too long
    if (text == NULL || text[0] == '\0') return false;
    size_t len = strlen(text);
    return len > 0 && len < 64;
}

/* 
 * INITIALIZE STORE
 */
void store_init(Store *s) {
    if (!s) return;
    
#if USE_FIXED_SIZE_ARRAY
    // Fixed-size array: just set count to 0
    memset(s->students, 0, sizeof(s->students));
    s->count = 0;
#else
    // Dynamic allocation: initialize pointers
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
#endif
}

/* 
 * FREE STORE
 */
void store_free(Store *s) {
    if (!s) return;
    
#if USE_FIXED_SIZE_ARRAY
    // Fixed-size array: just reset count
    s->count = 0;
#else
    // Dynamic allocation: free memory
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
#endif
}

/* 
 * FIND INDEX BY ID
 * Returns index if found, -1 if not found
 */
int store_find_index_by_id(const Store *s, int id) {
    if (!s) return -1;
    
#if USE_FIXED_SIZE_ARRAY
    for (int i = 0; i < s->count; i++) {
        if (s->students[i].id == id) {
            return i;
        }
    }
#else
    for (size_t i = 0; i < s->size; i++) {
        if (s->data[i].id == id) {
            return (int)i;
        }
    }
#endif
    
    return -1;
}

/* 
 * INSERT STUDENT
 * Returns false if validation fails or duplicate ID
 */
bool store_insert(Store *s, Student st) {
    if (!s) return false;
    
    // Validate all fields
    if (!valid_id(st.id)) return false;
    if (!valid_mark(st.mark)) return false;
    if (!valid_text(st.name)) return false;
    if (!valid_text(st.programme)) return false;
    
    // Check for duplicate ID
    if (store_find_index_by_id(s, st.id) >= 0) {
        return false;
    }
    
#if USE_FIXED_SIZE_ARRAY
    // Check if store is full
    if (s->count >= MAX_STUDENTS) {
        return false;
    }
    
    // Insert the student
    s->students[s->count] = st;
    s->count++;
#else
    // Expand capacity if needed
    if (s->size >= s->cap) {
        size_t new_cap = s->cap == 0 ? 8 : s->cap * 2;
        Student *new_data = realloc(s->data, new_cap * sizeof(Student));
        if (!new_data) return false;
        
        s->data = new_data;
        s->cap = new_cap;
    }
    
    // Add student to end of array
    s->data[s->size++] = st;
#endif
    
    return true;
}

/* 
 * UPDATE STUDENT
 * patch = partial student data - only update fields that are set
 * Sentinel values: id=-1, mark=-1.0, empty strings = unchanged
 */
bool store_update(Store *s, int id, const Student *patch) {
    if (!s || !patch) return false;
    
    // Find student by ID
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;
    
#if USE_FIXED_SIZE_ARRAY
    Student *st = &s->students[idx];
#else
    Student *st = &s->data[idx];
#endif
    
    // Apply patch (only update fields that are provided)
    
    // Update ID if provided and different
    if (patch->id > 0 && patch->id != id) {
        // Changing ID - check for duplicates
        if (store_find_index_by_id(s, patch->id) >= 0) {
            return false;
        }
        if (!valid_id(patch->id)) return false;
        st->id = patch->id;
    }
    
    // Update name if provided (non-empty string)
    if (patch->name[0] != '\0') {
        if (!valid_text(patch->name)) return false;
        strncpy(st->name, patch->name, sizeof(st->name) - 1);
        st->name[sizeof(st->name) - 1] = '\0';
    }
    
    // Update programme if provided
    if (patch->programme[0] != '\0') {
        if (!valid_text(patch->programme)) return false;
        strncpy(st->programme, patch->programme, sizeof(st->programme) - 1);
        st->programme[sizeof(st->programme) - 1] = '\0';
    }
    
    // Update mark if provided (>= 0 means it was set)
    if (patch->mark >= 0.0f) {
        if (!valid_mark(patch->mark)) return false;
        st->mark = patch->mark;
    }
    
    return true;
}

/* 
 * DELETE STUDENT
 * Returns false if student not found
 */
bool store_delete(Store *s, int id) {
    if (!s) return false;
    
    // Find student by ID
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;
    
#if USE_FIXED_SIZE_ARRAY
    // Shift elements left to fill the gap
    for (int i = idx; i < s->count - 1; i++) {
        s->students[i] = s->students[i + 1];
    }
    s->count--;
#else
    // Shift elements down to fill the gap
    for (size_t i = idx; i < s->size - 1; i++) {
        s->data[i] = s->data[i + 1];
    }
    s->size--;
#endif
    
    return true;
}
