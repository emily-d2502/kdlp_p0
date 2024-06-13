#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {

    char *cwd = getcwd(NULL, 0);
    printf("%s$ ", cwd);

    // do {

    // } while (1);
    printf("\n");
    free(cwd);
    return 0;
}
