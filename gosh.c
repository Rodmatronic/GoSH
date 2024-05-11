#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 167772
#define MAX_ARGUMENTS 256
#define COMNOTFOUND ": No such command"
#define DEBUG 0
#define STARTMSG 1
#define VERSION "0.15"
#define NAME "Goldie Shell (Go$H)"

char input[MAX_COMMAND_LENGTH];
char * pwd;
char ps1[1024];

void print_environment_variables() {
    extern char **environ;
    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

void execute_command(char * input)
{
    // Tokenize the input to separate command and arguments
    char *token;
    char *args[MAX_ARGUMENTS + 1];
    int arg_count = 0;

    token = strtok(input, " ");
    while (token != NULL && arg_count < MAX_ARGUMENTS) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL; // Null-terminate the array of arguments

    // Check if the command is 'cd'
    if (strcmp(args[0], "cd") == 0) {
        if (arg_count == 1) {
            // Change to home directory
            chdir(getenv("HOME"));
        } else if (arg_count == 2) {
            // Change to the specified directory
            if (strcmp(args[1], "-") == 0) {
                // Change to the previous directory
                char *prev_dir = getenv("OLDPWD");
                if (prev_dir != NULL) {
                    chdir(prev_dir);
                } else {
                    printf("cd: OLDPWD not set\n");
                }
            } else {
                // Change to the specified directory
                if (chdir(args[1]) != 0) {
                    printf("cd: %s: No such file or directory\n", args[1]);
                }
            }
        } else {
            printf("cd: too many arguments\n");
        }

        // Set the OLDPWD environment variable before changing the directory
        setenv("OLDPWD", pwd, 1);

        // Update pwd to reflect the new current working directory
        pwd = getcwd(NULL, 0);
        if (pwd == NULL) {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }
        return;
    }

    // Fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        // Error occurred
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args);

        // If execvp returns, it means it failed
        printf("%s%s\n", args[0], COMNOTFOUND);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);

        // Check if the child process terminated normally
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                if (DEBUG == 1)
                {
                    printf("Child process exited with status %d\n", exit_status);
                }
            }
        } else {
            if (DEBUG == 1)
            {
                printf("Child process terminated abnormally\n");
            }
        }
    }
}


int main(int argc, char *argv[]) {

    // Check if the shell is invoked with a filename argument
    if (argc == 2) {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        
        char line[MAX_COMMAND_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
            // Remove trailing newline character
            line[strcspn(line, "\n")] = 0;

            // Skip lines starting with '#'
            if (line[0] == '#') {
                continue;
            }

            // Execute the command
            execute_command(line);
        }

        fclose(file);
        return 0;
    }

    // Set PS1 environment variable
    if (setenv("PS1", "\\u@\\h:\\w\\$ ", 1) != 0) {
        perror("Error setting PS1");
        return 1;
    }

    pwd = getenv("PWD");

    if (STARTMSG == 1)
    {
        printf(" Goldie___\n");
        printf("    __/_  `.  .-````-.\n");
        printf("    \\_,` \\| \\-'  /   )`-')\n");
        printf("      '') `'`    \\_ ((`''` SH\n");
        printf("     ___/  ,    .'/ /|\n");
        printf("    (_,___/...-` (_/_/   \n");

        printf("%s version %s, built with love <3\n", NAME, VERSION);
    }

    while (1) {
        while (1) {
            snprintf(ps1, sizeof(ps1), "%s", getenv("PS1"));
            //printf("%s", ps1);
            printf("$ ");
            fgets(input, MAX_COMMAND_LENGTH, stdin);

            // Remove trailing newline character
            input[strcspn(input, "\n")] = 0;

            // If the command is just whitespace or empty, do nothing and return
            if (strlen(input) == 0 || strspn(input, " \t\n\r\v\f") == strlen(input)) {
                continue;
            }

            if (strcmp(input, "exit") == 0 || (strcmp(input, "leave") == 0))
            {
                printf("exit\n");
                return 1;
            }

            // Check if the command is 'export'
            if (strcmp(input, "export") == 0) {
                print_environment_variables();
                continue;
            }

            // Check if the command is 'export'
            if (strncmp(input, "export ", 7) == 0) {
                char *variable = strtok(input + 7, "=");
                char *value = strtok(NULL, "");

                if (variable != NULL && value != NULL) {
                    // Set the environment variable
                    setenv(variable, value, 1);
                } else {
                    printf("Invalid export command\n");
                }

                continue;
            }

            execute_command(input);
        }
    }

    return 0;
}
