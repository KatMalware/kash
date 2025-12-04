#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define KASH_RL_BUFFSIZE 1024
#define KASH_TOK_BUFFSIZE 64
#define KASH_TOK_DELIM " \t\r\n\a"

/*
    Built-in command: exit
*/
int kash_exit(char **args) {
    return 0;
}

/*
    Built-in command: cd
*/
int kash_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "kash: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("kash");
        }
    }
    return 1;
}

/*
    List of built-in commands
*/
char *builtin_str[] = {
    "cd",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &kash_cd,
    &kash_exit
};

int kash_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/*
    Launch external program
*/
int kash_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // child
        if (execvp(args[0], args) == -1) {
            perror("kash");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("kash");
    } else {
        // parent waits
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/*
    Execute command (builtin or external)
*/
int kash_execute(char **args) {
    if (args[0] == NULL) {
        return 1; // empty command
    }

    for (int i = 0; i < kash_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return kash_launch(args);
}

/*
    Read line
*/
char *kash_read_line(void) {
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

/*
    Split into tokens
*/
char **kash_split_line(char *line) {
    int bufsize = KASH_TOK_BUFFSIZE;
    int pos = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(line, KASH_TOK_DELIM);
    while (token != NULL) {
        tokens[pos] = token;
        pos++;

        if (pos >= bufsize) {
            bufsize += KASH_TOK_BUFFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
        }

        token = strtok(NULL, KASH_TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

/*
    Main loop
*/
void kash_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("kash> ");
        line = kash_read_line();
        args = kash_split_line(line);
        status = kash_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv) {
    // Run shell
    kash_loop();
    return EXIT_SUCCESS;
}
