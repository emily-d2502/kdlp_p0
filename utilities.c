#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Finds 1st appearance of a char that comp returns true to
int find_first_of(const char *str, int (*comp)(int)) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (comp(str[i])) {
            return i;
        }
    }
    return NPOS;
}

int find_first_not_of(const char *str, int (*comp)(int)) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!comp(str[i])) {
            return i;
        }
    }
    return NPOS;
}

// Finds last appearance of a char that comp returns true to
int find_last_not_of(const char *str, int (*comp)(int)) {
    int ret = NPOS;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!comp(str[i])) {
            ret = i;
        }
    }
    return ret;
}

// need to free output
char *substr(const char *str, int start, int end) {
    char *ret = (char *) malloc((end - start + 1) * sizeof(*ret));
    memcpy(ret, &str[start], end - start);
    ret[end - start] = '\0';
    return ret;
}

char *trim_whitespaces(const char *str) {
    int start = find_first_not_of(str, isspace);
    int end = find_last_not_of(str, isspace);
    if (start == NPOS || end == NPOS) {
        return substr(str, 0, 0);
    }
    return substr(str, start, end + 1);
}

unsigned int parse_command_line(const char *line, char*** argv_p) {
    unsigned int argc = 0;
    char *iss = trim_whitespaces(line);
    int idx = 0;
    while (iss[idx] != '\0') {
        ++argc;
        int i = find_first_of(&iss[idx], isspace);
        if (i == NPOS) {
            break;
        }
        idx += i;
        i = find_first_not_of(&iss[idx], isspace);
        if (i == NPOS) {
            break;
        }
        idx += i;
    }

    *argv_p = (char **) malloc((argc + 1) * sizeof(*argv_p));
    int arg = 0;
    idx = 0;
    while (iss[idx] != '\0') {
        int start = idx;
        int i = find_first_of(&iss[idx], isspace);
        if (i == NPOS) {
            (*argv_p)[arg++] = substr(iss, start, strlen(iss));
            break;
        }
        idx += i;
        (*argv_p)[arg++] = substr(iss, start, idx);
        i = find_first_not_of(&iss[idx], isspace);
        if (i == NPOS) {
            break;
        }
        idx += i;
    }
    (*argv_p)[argc] = NULL;
    free(iss);
    return argc;
}
