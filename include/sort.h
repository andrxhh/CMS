#ifndef SORT_H
#define SORT_H
#include <stdbool.h>
#include "store.h"

typedef enum { SORT_BY_ID, SORT_BY_MARK } SortKey;
void store_sort(Store *s, SortKey key, bool ascending);

#endif // SORT_H
