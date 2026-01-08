#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 128

char last_command[MAX_LINE] = "";

/* ----------------- Tokenizer ----------------- */
char **parse_input(char *input) {
    char **args = malloc(MAX_ARGS * sizeof(char *));
    int idx = 0;

    char *token = strtok(input, " \t\n");
    while (token != NULL && idx < MAX_ARGS - 1) {
        args[idx++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[idx] = NULL;
    return args;
}

/* ----------------- Execute simple command ----------------- */
void execute_command(char **args, int background) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) { // child
        execvp(args[0], args);
        perror("Command not found");
        exit(1);
    } else { // parent
        if (!background)
            waitpid(pid, NULL, 0);
    }
}

/* ----------------- Pipe Execution: cmd1 | cmd2 ----------------- */
void execute_pipe(char *left, char *right) {
    char **args1 = parse_input(left);
    char **args2 = parse_input(right);

    int fd[2];
    pipe(fd);

    pid_t p1 = fork();
    if (p1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(args1[0], args1);
        perror("pipe cmd1 failed");
        exit(1);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execvp(args2[0], args2);
        perror("pipe cmd2 failed");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);

    free(args1);
    free(args2);
}

/* ----------------- Main Shell ----------------- */
int main() {
    char input[MAX_LINE];
    int running = 1;

    while (running) {
        printf("uinxsh> ");
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "!!") == 0) {
            if (strlen(last_command) == 0) {
                printf("No commands in history\n");
                continue;
            }
            printf("%s\n", last_command);
            strcpy(input, last_command);
        } else {
            strcpy(last_command, input);
        }

        /* check for pipe */
        char *pipe_pos = strchr(input, '|');
        if (pipe_pos) {
            *pipe_pos = '\0';
            char *left = input;
            char *right = pipe_pos + 1;
            execute_pipe(left, right);
            continue;
        }

        /* normal commands */
        char temp[MAX_LINE];
        strcpy(temp, input);
        char **args = parse_input(temp);

        if (args[0] == NULL) {
            free(args);
            continue;
        }

        /* built-in commands */
        if (strcmp(args[0], "exit") == 0) {
            running = 0;
            free(args);
            continue;
        }

        if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
            free(args);
            continue;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL)
                printf("cd: missing argument\n");
            else if (chdir(args[1]) != 0)
                perror("cd");
            free(args);
            continue;
        }

        if (strcmp(args[0], "help") == 0) {
            printf("Built-in commands:\n");
            printf("  exit\n  cd\n  pwd\n  help\n  history\n");
            free(args);
            continue;
        }

        if (strcmp(args[0], "history") == 0) {
            if (strlen(last_command) == 0)
                printf("No history available\n");
            else
                printf("Last command: %s\n", last_command);
            free(args);
            continue;
        }

        /* background check */
        int background = 0;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "&") == 0) {
                background = 1;
                args[i] = NULL;
                break;
            }
        }

        execute_command(args, background);

        while (waitpid(-1, NULL, WNOHANG) > 0);

        free(args);
    }

    return 0;
}
