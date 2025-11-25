#include <stdlib.h>
#include "sort.h"


//cmp_id_asc() -> tells qsort how to compare students by ID in ascending order.
//cmp_mark_asc() -> tells qsort how to compare students by mark in ascending order.


static int cmp_id_asc(const void *a, const void *b) { // comparator for ID ascending
     const Student *x = (const Student*)a;
     const Student *y = (const Student*)b;
     return (x->id > y->id) - (x->id < y->id); // return -1/0/1 without overflow
}

static int cmp_mark_asc(const void *a, const void *b) { // comparator for mark ascending
     const Student *x = (const Student*)a;
     const Student *y = (const Student*)b;
     return (x->mark > y->mark) - (x->mark < y->mark); // return -1/0/1 without overflow
}



//This function sorts the students in the Store based on the specified key (ID or mark) and order (ascending or descending).
//Store *s → the list of students
//SortKey key → sort by ID or MARK
//bool asc → TRUE for ascending, FALSE for descending
void store_sort(Store *s, SortKey key, bool asc) {
     if (!s || s->size <= 1) return; // nothing to sort if null or size 0/1
     
     if (key == SORT_BY_ID) { // choose comparator by ID
          qsort(s->data, s->size, sizeof(Student), cmp_id_asc); // sort ascending by ID
     } else { // otherwise sort by mark
          qsort(s->data, s->size, sizeof(Student), cmp_mark_asc); // sort ascending by mark
     }

     // if descending requested, reverse the array in-place
     if (!asc) {
          for (size_t i = 0, j = s->size-1; i < j; i++, j--) { // two-pointer swap
                Student tmp = s->data[i];
                s->data[i] = s->data[j];
                s->data[j] = tmp;
          }
     }
}
