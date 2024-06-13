#define  _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    do {
        // Print current working directory
        char *cwd = getcwd(NULL, 0);
        printf("%s$ ", cwd);
        free(cwd);

        // Read command from user
        char*   line     = NULL;
        size_t  len      = 0;
        ssize_t line_sz  = getline(&line, &len, stdin);

        if (line_sz > 1) {
            puts("Unrecognized command");
        }

    } while (!feof(stdin));
    puts("\n");
    return 0;
}
