#define  _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "utilities.h"

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

        char *program = find_word(line, 0);
        if (line_sz > 1) {
            printf("Unrecognized command: %s\n", program);
        }
        free(program);

    } while (!feof(stdin));
    printf("\n");
    return 0;
}
