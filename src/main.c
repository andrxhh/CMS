#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cmd.h"
#include "store.h"

#define DB_FILENAME "db/P6_5-CMS.txt" // Change TeamName


int main(void) {
    // Print the exact non-plagiarism declaration block per assignment
    char datebuf[32];
    time_t now = time(NULL); struct tm *lt = localtime(&now);
    strftime(datebuf, sizeof datebuf, "%Y-%m-%d", lt);
    print_declaration("LAB-P6-5", "CHESTON LEROY ONG (2502701)\nAARON ALISON SILVA (2500461)\nAFIQAH BINTE MOHAMED ADNAN (2503067)\nANDREW CHIA KAI XUN BEDINA (2501298)\nCHUA JIA JUN (2500533)\n", datebuf);

    // Create and initialize store
    Store store;
    store_init(&store);
    
    // Print program header
    printf("============================================\n");
    printf("Course Management System (CMS)\n");
    printf("============================================\n");
    printf("Type 'HELP' for available commands.\n");
    printf("Type 'EXIT' or 'QUIT' to exit.\n");
    printf("============================================\n\n");
    
    // Main command loop
    char line[512];
    for (;;) {
       printf("CMS> "); fflush(stdout);
       if (!fgets(line, sizeof line, stdin)) break; // EOF

       // strip trailing newline
       char *nl = strchr(line, '\n'); if (nl) *nl = '\0';
       if (!cmd_process_line(line, &store, DB_FILENAME)) break;
       }


       // On exit, you may prompt to save unsaved changes (TODO: track dirty flag)
       store_free(&store);
       puts("Thank you for using CMS.\nGoodbye.");
       return 0;
}
