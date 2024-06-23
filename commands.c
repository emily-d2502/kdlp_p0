#include "commands.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void run_cd(int argc, char **argv) {
    if (argc > 2) {
        printf("usage: cd [path to directory]");
        return;
    }
    if (chdir(argv[1]) != 0) {
        perror("cd failed");
    }
}

static void run_exit(int argc, char **argv) {
    if (argc > 1) {
        printf("error: arguments provided for exit");
    }
    for (int i = 0; i < argc; ++i) {
        free(argv[i]);
    }
    free(argv);
    exit(0);
}

static void run_exec(int argc, char **argv) {
    if (argc < 2) {
        printf("exec: arguments not provided for exec");
    }
    execv(argv[1], &argv[1]);
    perror("execv failed");
}

void run_command(const char* line, ssize_t line_sz) {
    char **argv;
    int argc = parse_command_line(line, &argv);


    if (strcmp(argv[0], "cd") == 0) {
        run_cd(argc, argv);
        return;
    }
    if (strcmp(argv[0], "exit") == 0) {
        run_exit(argc, argv);
        return;
    }
    if (strcmp(argv[0], "exec") == 0) {
        run_exec(argc, argv);
        return;
    }

    printf("Unrecognized command: %s\n", argv[0]);
    for (int i = 0; i < argc; ++i) {
        free(argv[i]);
    }
    free(argv);
    (void)line_sz;
}
