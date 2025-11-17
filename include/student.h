#ifndef STUDENT_H
#define STUDENT_H

// A single student record structure
typedef struct {
    int id;
    char name[64];
    char programme[64];
    float mark;
} Student;

#endif // STUDENT_H
