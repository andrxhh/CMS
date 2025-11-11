/* 
 * UTIL.C - Utility functions for string manipulation
 * 
 * These are helper functions (like Python's string methods)
 */

#include <ctype.h>   // Character type functions (isspace, tolower)
#include <string.h>  // String functions (strlen, memmove)
#include <stdlib.h>  // Standard library (strtol, strtof)
#include "util.h"    // Our header file

/* 
 * TRIM WHITESPACE (like Python's str.strip())
 * 
 * In Python: line = line.strip()
 * In C: We modify the string in place (no return value)
 */
void str_trim(char *s) {
    if (!s) return;
    
    // Trim leading whitespace
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    
    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }
    
    // Trim trailing whitespace
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) end--;
    *end = '\0';
}

/* 
 * CONVERT TO LOWERCASE (like Python's str.lower())
 * 
 * Modifies string in place (Python creates new string)
 */
void str_tolower(char *s) {
    for (; *s; s++) {
        *s = tolower((unsigned char)*s);
    }
}

/* 
 * CASE-INSENSITIVE STRING COMPARISON (like Python's str.lower() == other.lower())
 * 
 * Returns true if strings are equal (ignoring case)
 */
bool str_ieq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return false;
        }
        a++;
        b++;
    }
    return *a == *b;
}

/* 
 * PARSE INTEGER (like Python's int(string))
 * 
 * Returns true if successful, false if invalid
 * out parameter stores the parsed value
 */
bool parse_int(const char *s, int *out) {
    char *end;
    long val = strtol(s, &end, 10);
    if (end == s || *end != '\0') return false;
    *out = (int)val;
    return true;
}

/* 
 * PARSE FLOAT (like Python's float(string))
 * 
 * Returns true if successful, false if invalid
 */
bool parse_float(const char *s, float *out) {
    char *end;
    float val = strtof(s, &end);
    if (end == s || *end != '\0') return false;
    *out = val;
    return true;
}

/* 
 * VALIDATION FUNCTIONS
 */
bool valid_id(int id) {
    return id >= 100000 && id <= 99999999;  // 6-8 digits
}

bool valid_mark(float m) {
    return m >= 0.0f && m <= 100.0f;
}

bool valid_text(const char *s) {
    return s && s[0] != '\0' && strlen(s) < 64;
}

/* 
 * CASE-INSENSITIVE SUBSTRING SEARCH (like Python's "needle" in "haystack".lower())
 * 
 * Returns pointer to found substring, or NULL if not found
 */
char *strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    
    size_t needle_len = strlen(needle);
    for (; *haystack; haystack++) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            size_t i;
            for (i = 1; i < needle_len; i++) {
                if (tolower((unsigned char)haystack[i]) != tolower((unsigned char)needle[i])) {
                    break;
                }
            }
            if (i == needle_len) {
                return (char *)haystack;
            }
        }
    }
    return NULL;
}

