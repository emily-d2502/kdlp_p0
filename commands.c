#include "commands.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>

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

static void handle_redirection(int argc, char **argv) {
    for (int i = argc - 1; i > 0; --i) {
        if (strcmp(argv[i], ">") == 0) {
            if (argv[i + 1] == NULL) {
                printf("filename not found");
                exit(0);
            }

            int file = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (file == -1) {
                perror("open failed");
                exit(0);
            }
            dup2(file, STDOUT_FILENO);
            break;
        }
    }
    for (int i = argc - 1; i > 0; --i) {
        if (strcmp(argv[i], "<") == 0) {
            if (argv[i + 1] == NULL) {
                printf("filename not found");
                exit(0);
            }

            int file = open(argv[i + 1], O_RDONLY, 0666);
            if (file == -1) {
                perror("open failed");
                exit(0);
            }
            dup2(file, STDIN_FILENO);
            break;
        }
    }
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0) {
            for (int k = i; k < argc; ++k) {
                argv[k] = NULL;
            }
            break;
        }
    }
}

static void run_exec_child(int argc, char **argv) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }
    if (pid == 0) {
        handle_redirection(argc, argv);
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

static void query_path(int argc, char **argv, int pipe_count) {
    if (argv[0][0] == '/' || argv[0][0] == '.') {
        return;
    }
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
            if (pipe_count == 0) {
                run_exec_child(argc, argv);
            }
            return;
        }
        free(dir);
    }
    printf("Unrecognized command: %s\n", argv[0]);
}

static int isuser(int c) {
    return c == '~';
}

static char *substitutes_user(char *arg) {
    int arg_len = strlen(arg);
    int user_place = find_first_of(arg, isuser);
    if (user_place == NPOS) {
        return arg;
    }

    int start = user_place + 1;
    int end = user_place + 1;
    for (;arg[end] != '\0' && arg[end] != '/'; ++end);

    if (start == end) {
        char *home = getenv("HOME");
        int home_len = strlen(home);
        char *new_arg = (char *) malloc((arg_len + home_len + 1) * sizeof(*new_arg));
        strcpy(new_arg, arg); // copy the whole thing
        strcpy(&new_arg[user_place], home); // copy the home path
        strcpy(&new_arg[user_place + home_len], &arg[end]); // copy the rest of the path
        free(arg);
        return new_arg;
    }

    struct passwd *pwd;
    char *username_str = substr(arg, start, end);
    pwd = getpwnam(username_str);
    if (pwd == NULL) {
        if (errno != 0) {
            perror("getpwnam failed");
            exit(0);
        }
        return arg;
    }
    free(username_str);

    int home_len = strlen(pwd->pw_dir);
    char *new_arg = (char *) malloc((arg_len + home_len - start + end) * sizeof(*new_arg));
    strcpy(new_arg, arg); // copy the whole thing
    strcpy(&new_arg[user_place], pwd->pw_dir); // copy the home path
    strcpy(&new_arg[user_place + home_len], &arg[end]); // copy the rest of the path
    free(arg);
    return new_arg;
}

static int ispipe(int c) {
    return c == '|';
}

static int spawn_proc(int in, int out, char **argv) {
    pid_t pid = fork();
    if (pid < 0) {
        return pid;
    }

    if (pid == 0) {
        if (in != STDIN_FILENO) {
            dup2(in, STDIN_FILENO);
            close(in);
        }
        if (out != STDOUT_FILENO) {
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        execv(argv[0], argv);
        perror("execv failed");
        exit(-1);
    }
    return pid;
}

static void fork_pipes(int n, char *argv[])
{
    // The first process should get its input from the original file descriptor 0.
    int fd[2];
    int start = 0;
    int end = 0;
    pid_t curr, prev = -1;
    int in = STDIN_FILENO;

    // Note the loop bound, we spawn here all, but the last stage of the pipeline.
    for (int i = 0; i < n - 1; ++i) {
        pipe(fd);

        // f[1] is the write end of the pipe, we carry `in` from the prev iteration.
        for (;strcmp(argv[end], "|") != 0; ++end);
        argv[end] = NULL;
        query_path(0, &argv[start], 1);
        curr = spawn_proc(in, fd[1], &argv[start]);
        if (curr < 0) {
            perror("fork failed");
            if (waitpid(prev, NULL, WUNTRACED) < 0) {
                perror("waitpid failed");
            }
            return;
        }
        ++end;
        start = end;

        // No need for the write end of the pipe, the child will write here.
        close(fd[1]);

        // Keep the read end of the pipe, the next child will read from there.
        in = fd[0];
        prev = curr;
    }

    // Last stage of the pipeline - set stdin be the read end of the previous pipe
    //     and output to the original file descriptor 1.
    query_path(0, &argv[start], 1);
    curr = spawn_proc(in, STDOUT_FILENO, &argv[start]);
    if (curr < 0) {
        perror("fork failed");
        if (waitpid(prev, NULL, WUNTRACED) < 0) {
            perror("waitpid failed");
        }
        return;
    }
    if (waitpid(curr, NULL, WUNTRACED) < 0) {
        perror("waitpid failed");
    }
}

void run_command(const char* line, ssize_t line_sz) {
    char **argv;
    int argc = parse_command_line(line, &argv);
    for (int i = 0; i < argc; ++i) {
        argv[i] = substitutes_user(argv[i]);
    }

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

    int pipe_count = find_count(line, ispipe);
    query_path(argc, argv, pipe_count);
    if (pipe_count > 0) {
        fork_pipes(pipe_count + 1, argv);
        return;
    }


    for (int i = 0; i < argc; ++i) {
        free(argv[i]);
    }
    free(argv);
    (void)line_sz;
}
