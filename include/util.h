#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

/* 
 * STRING PARSING FUNCTIONS
 */
bool parse_int(const char *str, int *out);
bool parse_float(const char *str, float *out);

/* 
 * STRING MANIPULATION
 */
void str_trim(char *str);
void str_tolower(char *str);

/* 
 * STRING COMPARISON
 */
bool str_ieq(const char *a, const char *b);

/* 
 * STRING SEARCH (case-insensitive)
 */
char *strcasestr(const char *haystack, const char *needle);

#endif // UTIL_H
