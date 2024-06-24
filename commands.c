#include "commands.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

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

static void run_exec_child(int argc, char **argv) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }
    if (pid == 0) {
        execv(argv[0], &argv[0]);
        perror("execv failed");
        exit(0);
    }
    if (waitpid(pid, NULL, WUNTRACED) < 0) {
        perror("waitpid failed");
    }
    (void)argc;
}

static int isdelimiter(int c) {
    return c == ':';
}

static void query_path(int argc, char **argv) {
    char* path = getenv("PATH");
    int idx = 0;
    int start = -1;
    while (start != idx) {
        start = idx;
        int i = find_first_of(&path[idx], isdelimiter);
        idx += i + 1;

        char *dir = substr(path, start, i == NPOS ? strlen(path): idx - 1);


        // create file absolute path
        int dir_len = strlen(dir);
        char *file = (char *) malloc((dir_len + strlen(argv[0]) + 2) * sizeof(*file));
        strcpy(file, dir); file[dir_len] = '/'; strcpy(&file[dir_len + 1], argv[0]);

        struct stat stats;
        if (stat(file, &stats) == 0) {
            // file found
            free(argv[0]);
            argv[0] = file;
            run_exec_child(argc, argv);
            return;
        }
        free(dir);
    }
    printf("Unrecognized command: %s\n", argv[0]);
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
    if (argv[0][0] == '/' || argv[0][0] == '.') {
        run_exec_child(argc, argv);
        return;
    }
    query_path(argc, argv);


    for (int i = 0; i < argc; ++i) {
        free(argv[i]);
    }
    free(argv);
    (void)line_sz;
}
