/* 
 * STATS.C - Statistics computation functions
 * 
 * Computes statistics from student data (like Python's statistics module)
 */

#include <float.h>  // For FLT_MAX (maximum float value)
#include "stats.h"  // Our header file
#include "student.h"  // For Student type

/* 
 * COMPUTE STATISTICS (like Python's statistics.mean(), max(), min())
 */
Stats compute_stats(const void *data, size_t count) {
    const Student *students = (const Student *)data;
    Stats s = {0};  // Initialize all fields to 0
    
    // Handle empty data
    if (count == 0) {
        s.max_idx = -1;
        s.min_idx = -1;
        return s;
    }
    
    // Initialize statistics
    s.count = count;
    s.max_mark = -FLT_MAX;
    s.min_mark = FLT_MAX;
    s.max_idx = -1;
    s.min_idx = -1;
    
    float sum = 0.0f;
    
    // Loop through all students
    for (size_t i = 0; i < count; i++) {
        float mark = students[i].mark;
        sum += mark;
        
        // Track maximum mark
        if (mark > s.max_mark) {
            s.max_mark = mark;
            s.max_idx = (int)i;
        }
        
        // Track minimum mark
        if (mark < s.min_mark) {
            s.min_mark = mark;
            s.min_idx = (int)i;
        }
        
        // Count grade bands
        // A: >= 70, B: 60-69, C: 50-59, D: 40-49, F: < 40
        if (mark >= 70.0f) {
            s.band_A++;
        } else if (mark >= 60.0f) {
            s.band_B++;
        } else if (mark >= 50.0f) {
            s.band_C++;
        } else if (mark >= 40.0f) {
            s.band_D++;
        } else {
            s.band_F++;
        }
    }
    
    // Calculate average
    s.average = sum / count;
    
    return s;
}

