#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cmd_internal.h"
#include "store.h"
#include "io.h"
#include "util.h"

/*
This file contains command handlers for INSERT, UPDATE, DELETE, QUERY, and FIND.
Each handler parses its arguments, performs necessary validations, and invokes
the appropriate store functions.
*/

// Initialize a patch Student structure with sentinels indicating "no change"
static void init_patch(Student *patch) {
    memset(patch, 0, sizeof(Student));
    patch->id = -1;
    patch->mark = -1.0f;
}

/*
INSERT handler

Expected args: key=value pairs for ID, Name, Programme, Mark. All fields are required for insertion.
parsing and validation are delegated to parse_args_to_patch.
*/
bool handle_insert(char *args, Store *s) {
    // Prepare a patch structure with sentinel values
    Student patch;
    init_patch(&patch);

    // Parse arguments and validate fields. Validated fields are stored in patch structure, unset fields remain as sentinels.
    if (!parse_args_to_patch(args, &patch)) {
        fprintf(stderr, "Error: No valid parameters found to insert.\n");
        return false;
    }

    // Fields remaining as sentinels indicate missing required data
    if (patch.id < 0 || patch.name[0] == '\0' || patch.programme[0] == '\0' || patch.mark < 0.0f) {
        fprintf(stderr, "INSERT requires ID, Name, Programme, Mark.\n");
        return false;
    }

    // Attempt to insert the new record
    if (!store_insert(s, patch)) {
        fprintf(stderr, "Failed to insert record. Possible duplicate ID or invalid data.\n");
        return false;
    }

    puts("Record successfully inserted.");
    return true;
}

/*
UPDATE handler

Expected args: key=value ... with ID required to identify the record.
Only provided fields are modified in the target record.
ID update is allowed but must not conflict with existing IDs.
*/
bool handle_update(char *args, Store *s) {
    // Prepare a patch structure with sentinel values
    Student patch;
    init_patch(&patch);

    //// Parse arguments and validate fields. Validated fields are stored in patch structure, unset fields remain as sentinels.
    if (!parse_args_to_patch(args, &patch)) {
        fprintf(stderr, "Error: No valid parameters found to insert.\n");
        return false;
    }

    // ID is required to identify the record to update
    if (patch.id < 0) {
        fprintf(stderr, "UPDATE requires existing ID to identify record.\n");
        return false;
    }

    // Only ID is provided without any fields to update
    if (patch.name[0] == '\0' && patch.programme[0] == '\0' && patch.mark < 0.0f) {
        printf("Warning: UPDATE command given with only an ID. No fields to update.\n");
    }

    // Attempt to update the record
    if (!store_update(s, patch.id, &patch)) {
        fprintf(stderr, "Failed to update record. Possible invalid data or ID not found.\n");
        return false;
    }

    puts("Record successfully updated.");
    return true;
}

/*
DELETE handler

Expected args: ID=<n>
Verifies existence, prompts for confirmation, and deletes on 'Y'.
*/
bool handle_delete(char *args, Store *s) {
    // Parse single ID argument
    int id;

    // Validate arguments strictly only consists of a single ID. Once verified, extract it into id.
    if (!parse_single_id_command(args, "DELETE", &id)) return false;

    // Validate existence of record with given ID, if ID is negative, not found
    if (store_find_index_by_id(s, id) < 0) {
        fprintf(stderr, "ID %d not found.\n", id);
        return false;
    }

    // Prompt for confirmation
    printf("Are you sure you want to delete ID %d? (Y/N): ", id);
    fflush(stdout); // Ensure prompt is displayed before input

    // Read user input
    char buf[16];
    if (!fgets(buf, sizeof buf, stdin)) return false;

    if (buf[0] != 'Y' && buf[0] != 'y') {
        puts("Delete operation cancelled.");
        return false;
    }

    // Proceed to delete the record
    if (!store_delete(s, id)) {
        fprintf(stderr, "Failed to delete record with ID %d.\n", id);
        return false;
    }

    puts("Record successfully deleted.");
    return true;
}

/*
QUERY handler

Expected args: ID=<n>
Prints the single record if present.
*/
bool handle_query(char *args, const Store *s) {
    // Parse single ID argument
    int id;

    // Validate arguments strictly only consists of a single ID. Once verified, extract it into id.
    if (!parse_single_id_command(args, "QUERY", &id)) return false;

    // If ID is zero or negative, invalid
    if (id <= 0) {
        fputs("QUERY requires ID=...\n", stderr);
        return false;
    }

    // Verify existence and retrieve record
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) {
        puts("Record does not exist.");
        return false;
    }

    // Retrieve record with ID and print
    const Student *st = &s->data[idx];
    printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
    return true;
}

/*
FIND handler

We need to extract three tokens (column, operator, value) where the operator 
may be a word (e.g. CONTAINS) or symbolic (>, >=, =, etc.). The value may also
be quoted. After parsing, each supported column (Name, Programme, Mark) applies the
appropriate comparison with operators and prints matches.
*/
bool handle_find(char *args, Store *s) {
    // Trim leading whitespace to locate the first token (column)
    char *p = args;
    while (p && isspace((unsigned char)*p)) p++;

    // Ensure we have arguments beyond whitespace
    if (!p || *p == '\0') {
        fprintf(stderr, "FIND command requires arguments. Syntax: FIND <Column> <Operator> <Value>\n");
        return false;
    }

    // Extract column token (first word)
    // Column is the first contiguous non-space token, stop at space or an operator symbol.
    char *col_start = p;
    while (*p && !isspace((unsigned char)*p) && *p != '=' && *p != '>' && *p != '<') p++;
    size_t col_len = p - col_start;
    char column[32];
    if (col_len >= sizeof(column)) col_len = sizeof(column) - 1;
    strncpy(column, col_start, col_len);
    column[col_len] = '\0';

    // Extract operator token (either a word like CONTAINS or symbolic like >=)
    // Skip any whitespace between column and operator
    while (isspace((unsigned char)*p)) p++;
    char *op_start = p;
    if (isalnum((unsigned char)*p)) {
        // operator is a word (e.g. CONTAINS)
        while (*p && !isspace((unsigned char)*p)) p++;
    } else {
        // operator uses symbols like >=, <=, >, <, =
        while (*p && (*p == '=' || *p == '>' || *p == '<')) p++;
    }
    size_t op_len = p - op_start;
    char op[16];
    if (op_len >= sizeof(op)) op_len = sizeof(op) - 1;
    strncpy(op, op_start, op_len);
    op[op_len] = '\0';

    // The remainder of the input is treated as the comparison value
    // Skip whitespace before the value; value may contain spaces (possibly quoted)
    while (p && isspace((unsigned char)*p)) p++;
    char *value = p;

    // Structural check, column, operator, and value must be present
    if (!column || !op || !value) {
        fprintf(stderr, "Error: FIND requires 3 arguments. Syntax: FIND <Column> <Operator> <Value>\n");
        fprintf(stderr, "Example: FIND Name CONTAINS \"Wang\"\n");
        fprintf(stderr, "Example: FIND Mark > 75\n");
        return false;
    }

    // Normalize tokens for case-insensitive matching and trim whitespace around value
    // Convert column and operator to lowercase to simplify comparisons.
    str_tolower(column);
    str_tolower(op);
    str_trim(value);

    // Remove surrounding double quotes from value if present (e.g. "Some Name")
    // Unmatch quotes are handled during parsing, so we can safely strip them here.
    size_t len = strlen(value);
    if (strcmp(column, "mark") == 0) {
        // Mark must be unquoted
        if (strchr(value, '"') || strchr(value, '\'')) {
            fprintf(stderr, "Error: Quotes are not supported for Mark values in FIND command.\n");
            return false;
        }
    } else if (strcmp(column, "name") == 0 || strcmp(column, "programme") == 0) {
        // For text columns, allow matched quotes or simply strip any leading/trailing quote
        if (len >= 2 && ((value[0] == '"' && value[len - 1] == '"') || (value[0] == '\'' && value[len - 1] == '\''))) {
            // Matched quotes: remove both
            value[len - 1] = '\0';
            memmove(value, value + 1, len - 1); // include terminating NUL
        } else {
            // Unmatched leading quote */
            if (len > 0 && (value[0] == '"' || value[0] == '\'')) {
                memmove(value, value + 1, len); // shift including NULL
            }
            // Recompute length and strip trailing quote if present
            len = strlen(value);
            if (len > 0 && (value[len - 1] == '"' || value[len - 1] == '\'')) {
                value[len - 1] = '\0';
            }
        }
    } else {
        // Unknown column will be handled later, but strip any surrounding quotes for safety
        if (len >= 2 && value[0] == '"' && value[len - 1] == '"') {
            value[len - 1] = '\0';
            memmove(value, value + 1, len - 1);
        } else {
            if (len > 0 && (value[0] == '"' || value[0] == '\'')) memmove(value, value + 1, len);
            len = strlen(value);
            if (len > 0 && (value[len - 1] == '"' || value[len - 1] == '\'')) value[len - 1] = '\0';
        }
    }

    // If searching by Mark, parse numeric comparison value
    float mark_value = 0.0f;
    if (strcmp(column, "mark") == 0) {
        if (!parse_float(value, &mark_value)) {
            fprintf(stderr, "Error: Invalid mark value for FIND command: %s\n", value);
            return false;
        }
    }

    // Iterate all records and apply the requested comparison
    int match_count = 0;
    for (size_t i = 0; i < s->size; i++) {
        const Student *st = &s->data[i];
        bool match = false;

        if (strcmp(column, "name") == 0) {
            // Text comparisons for Name: exact match or CONTAINS (case-insensitive)
            if (strcmp(op, "=") == 0) match = str_ieq(st->name, value);
            else if (strcmp(op, "contains") == 0) match = (str_icase_find(st->name, value) != NULL);
            else { fprintf(stderr, "Error: Unsupported operator for Name column: %s\n", op); return false; }
        } else if (strcmp(column, "programme") == 0) {
            // Text comparisons for Programme: exact match or CONTAINS (case-insensitive)
            if (strcmp(op, "=") == 0) match = str_ieq(st->programme, value);
            else if (strcmp(op, "contains") == 0) match = (str_icase_find(st->programme, value) != NULL);
            else { fprintf(stderr, "Error: Unsupported operator for Programme column: %s\n", op); return false; }
        } else if (strcmp(column, "mark") == 0) {
            // Numeric comparisons for Mark; allow small epsilon for equality check
            if (strcmp(op, "=") == 0) match = (st->mark >= mark_value - 0.01f && st->mark <= mark_value + 0.01f);
            else if (strcmp(op, ">") == 0) match = (st->mark > mark_value);
            else if (strcmp(op, "<") == 0) match = (st->mark < mark_value);
            else if (strcmp(op, ">=") == 0) match = (st->mark >= mark_value);
            else if (strcmp(op, "<=") == 0) match = (st->mark <= mark_value);
            else { fprintf(stderr, "Error: Unsupported operator for Mark column: %s\n", op); return false; }
        } else {
            // Unknown column specified by user
            fprintf(stderr, "Error: Unsupported column for FIND command: %s\nUse Name, Programme, Mark.\n", column);
            return false;
        }

        // Print each matching row; print header once
        if (match) {
            if (match_count == 0) printf("ID\tName\tProgramme\tMark\n");
            printf("%d\t%s\t%s\t%.2f\n", st->id, st->name, st->programme, st->mark);
            match_count++;
        }
    }

    // Report results summary
    if (match_count == 0) puts("No matching records found.");
    else printf("Total matches: %d\n", match_count);

    return true;
}
