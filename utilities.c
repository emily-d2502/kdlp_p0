#include "utilities.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *find_word(const char *str, int start) {
    int end = strlen(str);
    for (int i = start; str[i] != '\0'; ++i) {
        if (isspace(str[i])) {
            end = i;
            break;
        }
    }

    char *ret = (char *) malloc((end - start + 1) * sizeof(*ret));
    for (int i = start; i < end; ++i) {
        ret[i] = str[i];
    }
    ret[end] = '\0';
    return ret;
}
