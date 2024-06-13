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
        char*  line = NULL;
        size_t len  = 0;
        getline(&line, &len, stdin);

    } while (!feof(stdin));
    printf("\n");
    return 0;
}
