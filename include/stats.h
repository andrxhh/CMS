#ifndef STATS_H
#define STATS_H

#include <stddef.h>
#include "student.h"

typedef struct {
    size_t count;
    float average;
    float min_mark, max_mark;
    int min_idx, max_idx;  // -1 if none
    int band_A, band_B, band_C, band_D, band_F;
} Stats;

Stats compute_stats(const Student *arr, size_t count);

#endif // STATS_H
