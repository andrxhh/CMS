#ifndef STORE_H
#define STORE_H
#include <stddef.h>
#include <stdbool.h>
#include "student.h"

typedef struct {
    Student *data;
    size_t size;
    size_t cap;
} Store;

// Lifecycle
void store_init(Store *s);
void store_free(Store *s);

// Core ops
int store_find_index_by_id(const Store *s, int id);         // -1 if not found
bool store_insert(Store *s, Student st);                    // false if duplicate id or invalid
bool store_update(Store *s, int id, const Student *patch);  // patch uses sentinel values
bool store_delete(Store *s, int id);                        // false if id not found

#endif // STORE_H


// #ifndef STORE_H
// #define STORE_H

// #include <stdbool.h>
// #include <stddef.h>

// /* 
//  * CONFIGURATION: Choose storage model
//  * 
//  * Your uploaded code uses a fixed-size array (MAX_STUDENTS = 1000)
//  * The original template uses dynamic allocation
//  * 
//  * This version combines both approaches for flexibility
//  */

// // Use fixed-size array (set to 0 for dynamic allocation)
// #define USE_FIXED_SIZE_ARRAY 1

// #if USE_FIXED_SIZE_ARRAY
//     #define MAX_STUDENTS 1000
//     #define MAX_NAME_LEN 100
//     #define MAX_PROGRAMME_LEN 100
// #endif

// /* 
//  * STUDENT STRUCTURE
//  * Represents a single student record
//  */
// typedef struct {
//     int id;                                    // 6-8 digit positive ID
// #if USE_FIXED_SIZE_ARRAY
//     char name[MAX_NAME_LEN];                   // Student name
//     char programme[MAX_PROGRAMME_LEN];         // Programme name
// #else
//     char name[64];                             // Student name (original size)
//     char programme[64];                        // Programme name (original size)
// #endif
//     float mark;                                // Mark: 0.0-100.0
// } Student;

// /* 
//  * STORE STRUCTURE
//  * Container for all student records
//  */
// typedef struct {
// #if USE_FIXED_SIZE_ARRAY
//     Student students[MAX_STUDENTS];            // Fixed-size array
//     int count;                                 // Current number of students
// #else
//     Student *data;                             // Dynamic array (original)
//     size_t size;                               // Current number of students
//     size_t cap;                                // Allocated capacity
// #endif
// } Store;

// /* 
//  * LIFECYCLE FUNCTIONS
//  */
// void store_init(Store *s);
// void store_free(Store *s);

// /* 
//  * CORE OPERATIONS
//  */
// int store_find_index_by_id(const Store *s, int id);
// bool store_insert(Store *s, Student st);
// bool store_update(Store *s, int id, const Student *patch);
// bool store_delete(Store *s, int id);

// /* 
//  * VALIDATION FUNCTIONS
//  */
// bool valid_id(int id);
// bool valid_mark(float mark);
// bool valid_text(const char *text);

// #endif // STORE_H
