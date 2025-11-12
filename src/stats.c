/* 
 * STATS.C - Statistics computation functions
 */

#include <float.h>
#include "stats.h"
#include "student.h"

/* 
 * COMPUTE STATISTICS
 */
Stats compute_stats(const void *data, size_t count) {
    const Student *students = (const Student *)data;
    Stats s = {0};
    
    if (count == 0) {
        s.max_idx = -1;
        s.min_idx = -1;
        return s;
    }
    
    s.count = count;
    s.max_mark = -FLT_MAX;
    s.min_mark = FLT_MAX;
    s.max_idx = -1;
    s.min_idx = -1;
    
    float sum = 0.0f;
    
    for (size_t i = 0; i < count; i++) {
        float mark = students[i].mark;
        sum += mark;
        
        if (mark > s.max_mark) {
            s.max_mark = mark;
            s.max_idx = (int)i;
        }
        
        if (mark < s.min_mark) {
            s.min_mark = mark;
            s.min_idx = (int)i;
        }
        
        // Grade bands: A: >= 70, B: 60-69, C: 50-59, D: 40-49, F: < 40
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
    
    s.average = sum / count;
    
    return s;
}
