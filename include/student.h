#ifndef STUDENT_H
#define STUDENT_H
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    int id;                // 6–8 digit positive ID
    char name[64];         // non-empty, trimmed
    char programme[64];    // non-empty, trimmed
    float mark;            // 0.0–100.0
} Student;

#endif // STUDENT_H
