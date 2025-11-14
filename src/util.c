#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "util.h"

// Helper function to convert ASCII character to lowercase without locale dependence
static inline int ascii_tolower_int(int c) {
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

void str_trim(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) { // Scan for leading whitespace
        start++;
    }

    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) { // Scan for trailing whitespace
        end--;
    }

    if (start > 0) {
        memmove(s, s + start, end - start); // Shift trimmed string to the front
    }

    s[end - start] = '\0';
}

void str_tolower(char *str) {
    if (!str) return;
    for (; *str; str++) {
        *str = (char)tolower((unsigned char)*str);
    }
}

bool str_ieq(const char *a, const char *b) {
    if (!a || !b) return false;
    
    while(*a && *b) { // Compare characters case-insensitively
        unsigned char ca = (unsigned char)tolower((unsigned char)*a);
        unsigned char cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) {
            return false;
        }
        a++, b++;
    }

    return *a == *b; // Both should be at the end for equality
}


// Case-insensitive substring find. Returns pointer into haystack, or NULL.
const char *str_icase_find(const char *hay, const char *needle) {
    if (!hay || !needle) return NULL;
    if (*needle == '\0') return hay; // empty needle: match at start
    for (const char *p = hay; *p; ++p) {
        const char *h = p, *n = needle;
        while (*h && *n) {
            int ch = ascii_tolower_int((unsigned char)*h);
            int cn = ascii_tolower_int((unsigned char)*n);
            if (ch != cn) break;
            ++h; ++n;
        }
        if (*n == '\0') return p; // matched full needle
    }
    return NULL;
}

bool str_icontains(const char *hay, const char *needle) {
    return str_icase_find(hay, needle) != NULL;
}

// char* smart_strtok(char **str, const char *delim, bool *in_quote_error) {
//     if (!str || !*str) {
//         return NULL;
//     }

//     char *p = *str;
//     *in_quote_error = false;

//     while (*p && strchr(delim, *p)) { // Skip leading delimiters
//         p++;
//     }

//     if (*p == '\0') {
//         *str = p;
//         return NULL; // At the end, no more tokens
//     }

//     char *token_start = p;
//     bool in_quote = false;

//     while (*p) {
//         if (*p == '"') {
//             in_quote = !in_quote; // Toggle quote state
//         }

//         if (!in_quote && strchr(delim, *p)) {
//             *p = '\0'; // Terminate token
//             *str = p + 1;
//             return token_start;
//         }
//         p++;
//     }

//     *str = p; // Set pointer to end
//     if (in_quote) {
//         *in_quote_error = true; // Unmatched quote detected
//         return NULL;
//     }

//     return token_start; // Return last token
// }

// Helper function to find the next key position in the string
char* find_next_key(char *p) {
    char *next_key = NULL;
    char *found;

    const char *keys[] = {"ID=", "Name=", "Programme=", "Mark="};
    size_t num_keys = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i<num_keys; i++) {
        found = (char*)str_icase_find(p, keys[i]);
        if (found && (next_key == NULL || found < next_key)) {
            next_key = found;
        }
    }

    return next_key;
}

bool parse_int(const char *s, int *out) {
    if (!s || !out) return false;
    if (*s == '\0') return false; // Empty string check

    char *end = NULL;
    long val = strtol(s, &end, 10);
    while (isspace((unsigned char)*end)) end++; // Skip trailing whitespace

    if (end == s || *end != '\0') {
        return false; // No digits parsed or extra characters
    }
    if (val < INT_MIN || val > INT_MAX) {
        return false; // Out of int range
    }
    *out = (int)val;
    return true;
}

bool parse_float(const char *s, float *out) {
    if (!s || !out) return false;
    if (*s == '\0') return false; // Empty string check

    char *end = NULL;
    float val = strtof(s, &end);
    while (isspace((unsigned char)*end)) end++; // Skip trailing whitespace

    if (end == s || *end != '\0') {
        return false; // No digits parsed or extra characters
    }

    *out = val;
    return true;
}

bool valid_id(int id) {
    return id >= 100000 && id <= 99999999; // 6 to 8 digit check
}

bool valid_mark(float m) {
    return m >= 0.0f && m <= 100.0f; // 0.0 to 100.0 check
}

bool valid_text(const char *s) {
    if (!s) return false;
    size_t len = strlen(s);
    return len > 0 && len < 64; // Non-empty and within length check
}