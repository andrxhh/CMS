#include <stdlib.h>
#include "sort.h"

static int cmp_id_asc(const void *a, const void *b) {
    const Student *x = (const Student*)a;
    const Student *y = (const Student*)b;
    return (x->id > y->id) - (x->id < y->id); // Avoids integer overflow
}

static int cmp_mark_asc(const void *a, const void *b) {
    const Student *x = (const Student*)a;
    const Student *y = (const Student*)b;
    return (x->mark > y->mark) - (x->mark < y->mark);
}

void store_sort(Store *s, SortKey key, bool asc) {
    if (!s || s->size <= 1) return;
    
    if (key == SORT_BY_ID) {
        qsort(s->data, s->size, sizeof(Student), cmp_id_asc);
    } else {
        qsort(s->data, s->size, sizeof(Student), cmp_mark_asc);
    }

    if (!asc) {
        for (size_t i = 0, j = s->size-1; i<j; i++, j--) {
            Student tmp = s->data[i];
            s->data[i] = s->data[j];
            s->data[j] = tmp;
        }
    }
}