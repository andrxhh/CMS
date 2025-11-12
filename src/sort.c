/* 
 * SORT.C - Sorting functions for student data
 * 
 * Uses qsort() with comparison functions
 */

#include <stdlib.h>
#include "sort.h"

/* 
 * COMPARISON FUNCTIONS FOR SORTING
 */

static int cmp_by_id_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return sa->id - sb->id;
}

static int cmp_by_id_desc(const void *a, const void *b) {
    return cmp_by_id_asc(b, a);
}

static int cmp_by_mark_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    
    if (sa->mark < sb->mark) return -1;
    if (sa->mark > sb->mark) return 1;
    return 0;
}

static int cmp_by_mark_desc(const void *a, const void *b) {
    return cmp_by_mark_asc(b, a);
}

/* 
 * SORT THE STORE
 */
void store_sort(Store *s, SortKey key, bool ascending) {
#if USE_FIXED_SIZE_ARRAY
    if (s->count <= 1) return;
#else
    if (s->size <= 1) return;
#endif
    
    int (*cmp_func)(const void *, const void *);
    
    if (key == SORT_BY_ID) {
        cmp_func = ascending ? cmp_by_id_asc : cmp_by_id_desc;
    } else {
        cmp_func = ascending ? cmp_by_mark_asc : cmp_by_mark_desc;
    }
    
#if USE_FIXED_SIZE_ARRAY
    qsort(s->students, s->count, sizeof(Student), cmp_func);
#else
    qsort(s->data, s->size, sizeof(Student), cmp_func);
#endif
}
