#include "stats.h"
#include "student.h"

// This function computes various statistics from an array of Student records.
// Takes in an array of Student and its size, returns a Stats struct with computed values.
// - minimum and maximum marks and their indices
// - average mark
// - total student count
// - count of students in each grade band (A, B, C, D, F

Stats compute_stats(const Student *arr, size_t size) {
    Stats stats = {0};
    stats.min_idx = -1; // Default to -1 indicating no students
    stats.max_idx = -1;
    if (size == 0) return stats; // If no students, return default stats. 
    float sum = 0.0f;
    stats.min_mark = arr[0].mark;
    stats.max_mark = arr[0].mark;
    stats.min_idx = 0; 
    stats.max_idx = 0;

    for (size_t i = 0; i < size; i++) {
        float m = arr[i].mark;
        sum += m; // Accumulate total marks for average calculation.

        if (m < stats.min_mark) {
            stats.min_mark = m;
            stats.min_idx = (int)i;
        }
        if (m > stats.max_mark) {
            stats.max_mark = m;
            stats.max_idx = (int)i;
        }

        // Grade bands classfication: A>=85, B 75-84, C 65-74, D 50-64, F<50
        if (m >= 85) stats.band_A++;
        else if (m < 85 && m >= 75) stats.band_B++;
        else if (m < 75 && m >= 65) stats.band_C++;
        else if (m < 65 && m >= 50) stats.band_D++;
        else stats.band_F++;

    }

    stats.count = size; // Total number of students
    stats.average = sum / (float)size; // Computes overall average for the students.

    return stats; // It returns the computed statistics.
}