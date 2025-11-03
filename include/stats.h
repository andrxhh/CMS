#ifndef STATS_H
#define STATS_H
#include "student.h"
#include <stddef.h>

typedef struct {
    size_t count;
    float average;
    float min_mark, max_mark;
    int min_idx, max_idx; // indices into the provided array, -1 if none
    // Optional extension: grade band counts
    int band_A, band_B, band_C, band_D, band_F;
} Stats;

Stats compute_stats(const Student *arr, size_t n);

#endif // STATS_H
