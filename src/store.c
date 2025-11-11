/* 
 * STORE.C - Data storage and management functions
 * 
 * This file manages the in-memory storage of student records
 * Like a dynamic array or list in Python
 */

#include <stdlib.h>  // For malloc, realloc, free
#include <string.h>  // For string functions
#include "store.h"   // Our header file

/* 
 * INITIALIZE STORE (like __init__ in Python)
 * 
 * In Python:
 * def __init__(self):
 *     self.data = []
 *     self.size = 0
 *     self.capacity = 0
 */
void store_init(Store *s) {
    s->data = NULL;   // No memory allocated yet
    s->size = 0;      // No items yet
    s->cap = 0;       // No capacity allocated yet
}

/* 
 * FREE STORE (like __del__ in Python, but MUST be called manually)
 * 
 * In Python, garbage collector handles this automatically
 * In C, you MUST free memory to prevent leaks
 */
void store_free(Store *s) {
    free(s->data);    // Release memory allocated by malloc/realloc
    s->data = NULL;   // Set to NULL to prevent using freed memory
    s->size = 0;
    s->cap = 0;
}

/* 
 * FIND INDEX BY ID (like list.index() in Python)
 * 
 * Returns index if found, -1 if not found
 */
int store_find_index_by_id(const Store *s, int id) {
    for (size_t i = 0; i < s->size; i++) {
        if (s->data[i].id == id) {
            return (int)i;  // Found it!
        }
    }
    return -1;  // Not found
}

/* 
 * INSERT STUDENT (like list.append in Python)
 * 
 * Returns false if duplicate ID or memory allocation fails
 */
bool store_insert(Store *s, Student st) {
    // Check for duplicate ID
    if (store_find_index_by_id(s, st.id) >= 0) {
        return false;  // Duplicate ID found
    }
    
    // Expand capacity if needed (Python lists do this automatically)
    if (s->size >= s->cap) {
        size_t new_cap = s->cap == 0 ? 8 : s->cap * 2;
        Student *new_data = realloc(s->data, new_cap * sizeof(Student));
        if (!new_data) return false;  // Memory allocation failed
        
        s->data = new_data;
        s->cap = new_cap;
    }
    
    // Add student to end of array
    s->data[s->size++] = st;
    return true;
}

/* 
 * UPDATE STUDENT (like modifying a dictionary in Python)
 * 
 * patch = partial student data - only update fields that are set
 */
bool store_update(Store *s, int id, const Student *patch) {
    // Find student by ID
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;  // Student not found
    
    Student *st = &s->data[idx];
    
    // Apply patch (only update fields that are provided)
    
    // Update ID if provided and different
    if (patch->id > 0 && patch->id != id) {
        // Changing ID - check for duplicates
        if (store_find_index_by_id(s, patch->id) >= 0) {
            return false;  // New ID already exists
        }
        st->id = patch->id;
    }
    
    // Update name if provided (non-empty string)
    if (patch->name[0] != '\0') {
        strncpy(st->name, patch->name, sizeof st->name - 1);
        st->name[sizeof st->name - 1] = '\0';
    }
    
    // Update programme if provided
    if (patch->programme[0] != '\0') {
        strncpy(st->programme, patch->programme, sizeof st->programme - 1);
        st->programme[sizeof st->programme - 1] = '\0';
    }
    
    // Update mark if provided (>= 0 means it was set)
    if (patch->mark >= 0.0f) {
        st->mark = patch->mark;
    }
    
    return true;
}

/* 
 * DELETE STUDENT (like list.remove in Python)
 * 
 * Returns false if student not found
 */
bool store_delete(Store *s, int id) {
    // Find student by ID
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;  // Student not found
    
    // Shift elements down to fill the gap
    for (size_t i = idx; i < s->size - 1; i++) {
        s->data[i] = s->data[i + 1];
    }
    
    s->size--;  // Decrement size
    return true;
}

