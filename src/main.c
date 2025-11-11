/* 
 * MAIN.C - Main program entry point
 * 
 * This is the entry point for the Course Management System
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd.h"

/* 
 * MAIN FUNCTION - Entry point of the program
 */
int main(void) {
    // Create and initialize store
    Store store;
    store_init(&store);
    
    // Database file path
    const char *db_path = "sample-cms.txt";
    
    // Print program header
    printf("============================================\n");
    printf("Course Management System (CMS)\n");
    printf("============================================\n");
    printf("Type 'HELP' for available commands.\n");
    printf("Type 'EXIT' or 'QUIT' to exit.\n");
    printf("============================================\n\n");
    
    // Main command loop
    char input[512];
    bool running = true;
    
    while (running) {
        printf("CMS> ");
        fflush(stdout);
        
        // Read user input
        if (!fgets(input, sizeof input, stdin)) {
            break;
        }
        
        // Process the command
        running = cmd_process_line(input, &store, db_path);
    }
    
    printf("\nThank you for using CMS. Goodbye!\n");
    
    // Clean up memory
    store_free(&store);
    
    return 0;
}

