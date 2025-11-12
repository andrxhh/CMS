/* 
 * UTIL.C - Utility functions for string manipulation
 * 
 * Integrated version combining your implementation with the original template
 */

#include "util.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* 
 * PARSE INTEGER (like Python's int(string))
 */
bool parse_int(const char *str, int *out) {
    if (str == NULL || out == NULL) return false;
    
    char *endptr;
    long val = strtol(str, &endptr, 10);
    
    // Check if conversion was successful
    if (*endptr != '\0' && !isspace((unsigned char)*endptr)) {
        return false;
    }
    
    // Check for overflow (int range: -2147483648 to 2147483647)
    if (val < -2147483648L || val > 2147483647L) {
        return false;
    }
    
    *out = (int)val;
    return true;
}

/* 
 * PARSE FLOAT (like Python's float(string))
 */
bool parse_float(const char *str, float *out) {
    if (str == NULL || out == NULL) return false;
    
    char *endptr;
    double val = strtod(str, &endptr);
    
    // Check if conversion was successful
    if (*endptr != '\0' && !isspace((unsigned char)*endptr)) {
        return false;
    }
    
    *out = (float)val;
    return true;
}

/* 
 * TRIM WHITESPACE (like Python's str.strip())
 * Removes leading and trailing whitespace in place
 */
void str_trim(char *str) {
    if (str == NULL) return;
    
    // Trim leading whitespace
    char *start = str;
    while (*start && (*start == ' ' || *start == '\t' || 
           *start == '\n' || *start == '\r')) {
        start++;
    }
    
    // If string was all whitespace
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }
    
    // Move trimmed string to beginning
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || 
           *end == '\n' || *end == '\r')) {
        end--;
    }
    end[1] = '\0';
}

/* 
 * CONVERT TO LOWERCASE (like Python's str.lower())
 * Modifies string in place
 */
void str_tolower(char *str) {
    if (!str) return;
    for (; *str; str++) {
        *str = tolower((unsigned char)*str);
    }
}

/* 
 * CASE-INSENSITIVE STRING COMPARISON (like Python's str.lower() == other.lower())
 * Returns true if strings are equal (ignoring case)
 */
bool str_ieq(const char *a, const char *b) {
    if (!a || !b) return false;
    
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
 * CASE-INSENSITIVE SUBSTRING SEARCH
 * Returns pointer to found substring, or NULL if not found
 */
char *strcasestr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
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
