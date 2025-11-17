#include "stats.h"
#include "student.h"

Stats compute_stats(const Student *arr, size_t size) {
    Stats stats = {0};
    stats.min_idx = -1;
    stats.max_idx = -1;
    if (size == 0) return stats;
    float sum = 0.0f;
    stats.min_mark = arr[0].mark;
    stats.max_mark = arr[0].mark;
    stats.min_idx, stats.max_idx = 0;

    for (size_t i = 0; i < size; i++) {
        float m = arr[i].mark;
        sum += m;

        if (m < stats.min_mark) {
            stats.min_mark = m;
            stats.min_idx = (int)i;
        }
        if (m > stats.max_mark) {
            stats.max_mark = m;
            stats.max_idx = (int)i;
        }

        // Grade bands: A>=85, B 75-84, C 65-74, D 50-64, F<50
        if (m >= 85) stats.band_A++;
        else if (m < 85 && m >= 75) stats.band_B++;
        else if (m < 75 && m >= 65) stats.band_C++;
        else if (m < 65 && m >= 50) stats.band_D++;
        else stats.band_F++;

    }

    stats.count = size;
    stats.average = sum / (float)size;

    return stats;
}