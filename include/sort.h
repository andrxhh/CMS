#ifndef SORT_H
#define SORT_H

#include <stdbool.h>
#include "store.h"

typedef enum {
    SORT_BY_ID,
    SORT_BY_MARK
} SortKey;

// Sort the store in-place by the specified key.
// if asc is false, we reverse after an ascending sort.
void store_sort(Store *s, SortKey key, bool asc);

#endif // SORT_H