/* 
 * SORT.C - Sorting functions for student data
 * 
 * In Python: students.sort(key=lambda x: x.id) or students.sort(key=lambda x: x.mark)
 * In C: We use qsort() with comparison functions
 */

#include <stdlib.h>  // For qsort function
#include "sort.h"    // Our header file

/* 
 * COMPARISON FUNCTIONS FOR SORTING
 * 
 * qsort needs comparison functions that return:
 *   - negative if a < b
 *   - 0 if a == b
 *   - positive if a > b
 */

// Comparison function for sorting by ID (ascending)
static int cmp_by_id_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return sa->id - sb->id;
}

// Comparison function for sorting by ID (descending)
static int cmp_by_id_desc(const void *a, const void *b) {
    return cmp_by_id_asc(b, a);  // Swap a and b
}

// Comparison function for sorting by Mark (ascending)
static int cmp_by_mark_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    
    // For floats, we need explicit comparison (can't just subtract)
    if (sa->mark < sb->mark) return -1;
    if (sa->mark > sb->mark) return 1;
    return 0;
}

// Comparison function for sorting by Mark (descending)
static int cmp_by_mark_desc(const void *a, const void *b) {
    return cmp_by_mark_asc(b, a);  // Swap a and b
}

/* 
 * SORT THE STORE (like Python's list.sort())
 */
void store_sort(Store *s, SortKey key, bool ascending) {
    // No need to sort if 0 or 1 element
    if (s->size <= 1) return;
    
    // Function pointer - stores which comparison function to use
    int (*cmp_func)(const void *, const void *);
    
    // Choose comparison function based on key and order
    if (key == SORT_BY_ID) {
        cmp_func = ascending ? cmp_by_id_asc : cmp_by_id_desc;
    } else {
        cmp_func = ascending ? cmp_by_mark_asc : cmp_by_mark_desc;
    }
    
    // qsort = Quick Sort (built-in C function)
    qsort(s->data, s->size, sizeof(Student), cmp_func);
}

