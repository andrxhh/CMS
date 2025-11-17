#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <stddef.h>

// String helpers
void str_trim(char *s);                       // Trim leading/trailing whitespace in-place
void str_tolower(char *s);                    // Convert string to lowercase in-place
bool str_ieq(const char *a, const char *b);   // Case-insensitive string equality check

// Case-insensitive substring search (like strstr)
const char *str_icase_find(const char *haystack, const char *needle);
bool str_icontains(const char *haystack, const char *needle);

// Quote aware string tokenization
char* smart_strtok(char **str, const char *delim, bool *in_quote_error);

// Find the next key position in a string
char* find_next_key(char *p, const char **found_keyname);

// Parsing helpers
bool parse_int(const char *s, int *out);      // Parse string to int with error checking
bool parse_float(const char *s, float *out);  // Parse string to float with error checking

// Validation helpers
bool valid_id(int id);                        // 6-8 digit ID check
bool valid_mark(float m);                     // 0.0 to 100.0 mark check        
bool valid_text(const char *s);               // Non-empty text and within length check

#endif // UTIL_H