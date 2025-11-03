#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <stddef.h>

// String helpers
void str_trim(char *s);
void str_tolower(char *s);
bool str_ieq(const char *a, const char *b);

// Parsing helpers
bool parse_int(const char *s, int *out);
bool parse_float(const char *s, float *out);

// Validation helpers
bool valid_id(int id);          // 6–8 digits
bool valid_mark(float m);       // 0..100 inclusive
bool valid_text(const char *s); // non-empty and within length

#endif // UTIL_H
