#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cmd_internal.h"
#include "util.h"
#include "student.h"

/*
 parse_args_to_patch
 
 This function scans an arguments string for key=value pairs and
 populates the provided `patch` Student structure. It supports
 quoted and unquoted values (e.g. Name="Alice Lee") and will validate types
 and lengths for each field. The function returns true when at
 least one valid key was parsed and returns false on any syntax
 or validation error.
 */
bool parse_args_to_patch(char *args, Student *patch) {
    /* p: current scanning pointer into args */
    char *p = args;
    /* any_parsed: becomes true if we successfully parse at least one key */
    bool any_parsed = false;

    /* Loop until we run out of input */
    while (p && *p) {
        const char *key_name = NULL;
        /* find_next_key locates the next occurrence of a known key
           (ID, Name, Programme, Mark) and returns its start. It
           ignores matches that appear inside quoted strings. */
        char *key_start = find_next_key(p, &key_name);
        if (!key_start) break; /* no more keys found */

        /* Look for '=' after the key text */
        char *eq = strchr(key_start + strlen(key_name), '=');

        /* Ensure only whitespace appears between the key token and '=' */
        char *check_ptr = key_start + strlen(key_name);
        while (eq && check_ptr < eq && isspace((unsigned char)*check_ptr)) check_ptr++;

        if (!eq || check_ptr != eq) {
            fprintf(stderr, "Malformed key-value pair: Missing or invalid '=' after key %s.\n", key_name);
            return false;
        }

        /* Position at the start of the value and skip leading whitespace */
        char *value_start = eq + 1;
        while (*value_start && isspace((unsigned char)*value_start)) value_start++;

        /* value_end is the start of the next key (if any), so the value
           spans [value_start, value_end) or until the end of the string. */
        const char *next_key = NULL;
        char *value_end = find_next_key(value_start, &next_key);

        /* Copy value into a temporary buffer and trim it. Use a fixed
           upper bound to avoid unbounded memory use. */
        char value_buf[256];
        size_t len;
        if (value_end == NULL) {
            len = strlen(value_start);
            p = NULL; /* no more keys after this */
        } else {
            len = value_end - value_start;
            p = value_end; /* continue scanning from next key */
        }

        if (len > sizeof(value_buf) - 1) len = sizeof(value_buf) - 1;
        strncpy(value_buf, value_start, len);
        value_buf[len] = '\0';
        str_trim(value_buf);

        /* If value is quoted, require a matching closing quote and strip it. */
        size_t value_len = strlen(value_buf);
        if (value_len > 0 && value_buf[0] == '"') {
            if (value_buf[value_len - 1] != '"') {
                fprintf(stderr, "Malformed quoted value for key %s: unmatched '\"'\n", key_name);
                return false;
            }
            /* Remove trailing quote and shift to remove leading quote */
            value_buf[value_len - 1] = '\0';
            memmove(value_buf, value_buf + 1, value_len - 1);
            /* Adjust length and ensure null termination */
            value_len -= 2;
            if ((int)value_len < 0) value_len = 0;
            value_buf[value_len] = '\0';
        }

        /* Dispatch and validate based on the key token */
        if (str_ieq(key_name, "ID")) {
            /* Parse integer ID */
            if (!parse_int(value_buf, &patch->id)) {
                fprintf(stderr, "Invalid ID value: %s\n", value_buf);
                return false;
            }
        } else if (str_ieq(key_name, "Name")) {
            /* Validate length and allowed characters */
            if (strlen(value_buf) >= sizeof(patch->name)) {
                fprintf(stderr, "Name too long (max %zu chars): %s\n", sizeof(patch->name)-1, value_buf);
                return false;
            }
            if (!valid_text(value_buf)) {
                fprintf(stderr, "Invalid Name: %s\n", value_buf);
                return false;
            }
            strcpy(patch->name, value_buf);
        } else if (str_ieq(key_name, "Programme")) {
            if (strlen(value_buf) >= sizeof(patch->programme)) {
                fprintf(stderr, "Programme too long (max %zu chars): %s\n", sizeof(patch->programme)-1, value_buf);
                return false;
            }
            if (!valid_text(value_buf)) {
                fprintf(stderr, "Invalid Programme: %s\n", value_buf);
                return false;
            }
            strcpy(patch->programme, value_buf);
        } else if (str_ieq(key_name, "Mark")) {
            if (!parse_float(value_buf, &patch->mark)) {
                fprintf(stderr, "Invalid Mark value: %s\n", value_buf);
                return false;
            }
        }

        any_parsed = true; /* successfully parsed one key=value */
    }

    return any_parsed;
}

/* Parse single ID argument in the form ID=<number> */
bool parse_single_id_command(char *args, const char *cmd_name, int *out_id) {
    if (!args) {
        fprintf(stderr, "%s command requires ID argument.\n", cmd_name);
        return false;
    }

    char *p = args;
    while (isspace((unsigned char)*p)) p++;

    if (strncasecmp(args, "ID", 2) != 0) {
        fprintf(stderr, "%s command requires ID argument in format ID=<value>.\n", cmd_name);
        return false;
    }

    p += 2;
    while (isspace((unsigned char)*p)) p++;
    if (*p != '=') {
        fprintf(stderr, "Expected '=' after ID in %s command.\n", cmd_name);
        return false;
    }

    p++;
    while (isspace((unsigned char)*p)) p++;
    if (*p == '\0') {
        fprintf(stderr, "No ID value provided for %s command.\n", cmd_name);
        return false;
    }

    char *endptr;
    long value = strtol(p, &endptr, 10);
    if (p == endptr) {
        fprintf(stderr, "Invalid numeric ID format for %s command.\n", cmd_name);
        return false;
    }

    while (*endptr && isspace((unsigned char)*endptr)) endptr++;
    if (*endptr != '\0') {
        fprintf(stderr, "Unexpected characters after ID value in %s command.\n", cmd_name);
        return false;
    }

    if (!valid_id((int)value)) {
        fprintf(stderr, "Invalid ID value for %s command. Must be 6-8 digits.\n", cmd_name);
        return false;
    }

    *out_id = (int)value;
    return true;
}
