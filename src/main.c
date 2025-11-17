#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cmd.h"
#include "store.h"

#define DB_FILENAME "TeamName-CMS.txt"  // Change TeamName

int main(void) {
    char datebuf[32];
    time_t now = time(NULL); struct tm *lt = localtime(&now);
    strftime(datebuf, sizeof datebuf, "%Y-%m-%d", lt);
    print_declaration("<YourTeam>", "<Name1, Name2, Name3>", datebuf);

    Store store; store_init(&store);

    puts("Type HELP for commands. Start with OPEN.");
    char line[512];
    while (1) {
        printf("> "); fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) break; // EOF
        // strip trailing newline
        char *nl = strchr(line, '\n'); if (nl) *nl = '\0';
        if (!cmd_process_line(line, &store, DB_FILENAME)) break;
    }

    // On exit, you may prompt to save unsaved changes (TODO: track dirty flag)
    store_free(&store);
    puts("Goodbye.");
    return 0;
}
