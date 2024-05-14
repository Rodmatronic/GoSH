/*
    Goldie Shell, writtem by Rodmatronics, using the GNU GENERAL PUBLIC LICENSE.

    I wrote this shell to test my skills with C, and Unix.
    The whole goal of this is to by a usable shell, that I can trust being my default shell
    on macOS, Linux, Haiku, FreeBSD, OpenBSD, and maybe even Research Unix.

    Good luck porting to Windowos :3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h> // Include for file operations

#define MAX_COMMAND_LENGTH 167772
#define MAX_ARGUMENTS 256
#define COMNOTFOUND ": No such command"
#define DEBUG 0
#define STARTMSG 1
#define VERSION "0.21"
#define NAME "Goldie Shell (Go$H)"

int print_prompt = 1; // Flag to control printing of prompt
char input[MAX_COMMAND_LENGTH];
char * pwd;
char ps1[1024];

void sigint_handler(int);
void user_shell();
void exit_gracefully();

void print_environment_variables() {
    extern char **environ;
    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

void sigint_handler(int signum) {
    if (!isatty(STDIN_FILENO)) {
        // If not running in interactive mode (e.g., running a script), simply exit
        exit_gracefully();
    } else {
        if (print_prompt) {
            printf("\n%s>$ ", pwd);
            fflush(stdout); // Flush output buffer to ensure prompt is displayed
        }
    }
}

void exit_gracefully() {
    printf("exit\n");
    exit(EXIT_SUCCESS);
}

void execute_command(char *input) {
    // Set the flag to 0 to prevent printing the prompt
    print_prompt = 0;

    // Tokenize the input to separate commands
    char *command;
    char *commands[MAX_ARGUMENTS + 1];
    int command_count = 0;

    // Split input by '&&' to handle multiple commands
    command = strtok(input, "&&");
    while (command != NULL && command_count < MAX_ARGUMENTS) {
        commands[command_count++] = command;
        command = strtok(NULL, "&&");
    }
    commands[command_count] = NULL; // Null-terminate the array of commands

    int saved_stdout = dup(STDOUT_FILENO); // Save a copy of the original STDOUT

    // Check if the command is 'export'
    if (strcmp(commands[0], "export") == 0) {
        // If no variable is specified after 'export', print all environment variables
        if (command_count == 1) {
            print_environment_variables();
            return;
        }

        // If the export command is followed by a variable name
        char *variable;
        char *value;
        for (int i = 1; i < command_count; i++) {
            // Tokenize each argument by '=' to separate variable and value
            variable = strtok(commands[i], "=");
            value = strtok(NULL, "=");

            // Set the environment variable
            if (setenv(variable, value, 1) != 0) {
                perror("Error setting environment variable");
            }
        }
        return;
    }

    // Check if the command is 'export'
    if (strcmp(input, "export") == 0) {
        print_environment_variables();
        return;
    }

    if (strcmp(input, "exit") == 0 || (strcmp(input, "leave") == 0))
    {
        printf("exit\n");
        exit(EXIT_SUCCESS);
    }

    // Execute each command
    for (int i = 0; i < command_count; i++) {
        // Tokenize the command to separate command and arguments
        char *token;
        char *args[MAX_ARGUMENTS + 1];
        int arg_count = 0;

        // Split command by '|' to handle piping
        token = strtok(commands[i], "|");
        while (token != NULL && arg_count < MAX_ARGUMENTS) {
            args[arg_count++] = token;
            token = strtok(NULL, "|");
        }
        args[arg_count] = NULL; // Null-terminate the array of arguments

        // Create pipes for inter-process communication
        int pipe_fds[2];
        int prev_pipe_read_end = -1;

        // Iterate over each command in the pipe
        for (int j = 0; j < arg_count; j++) {
            // Tokenize the command and arguments
            char *sub_command;
            char *sub_args[MAX_ARGUMENTS + 1];
            int sub_arg_count = 0;

            sub_command = strtok(args[j], " ");
            while (sub_command != NULL && sub_arg_count < MAX_ARGUMENTS) {
                sub_args[sub_arg_count++] = sub_command;
                sub_command = strtok(NULL, " ");
            }
            sub_args[sub_arg_count] = NULL; // Null-terminate the array of arguments

            // Handle file redirections
            int fd;
            for (int k = 0; sub_args[k] != NULL; k++) {
                if (strcmp(sub_args[k], ">") == 0) {
                    // Output redirection (truncate)
                    fd = open(sub_args[k + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    sub_args[k] = NULL; // Remove ">" from arguments
                    sub_args[k + 1] = NULL; // Remove the filename from arguments
                    break;
                } else if (strcmp(sub_args[k], ">>") == 0) {
                    // Output redirection (append)
                    fd = open(sub_args[k + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    sub_args[k] = NULL; // Remove ">>" from arguments
                    sub_args[k + 1] = NULL; // Remove the filename from arguments
                    break;
                }
            }

            if (strcmp(sub_args[0], "cd") == 0) {
                if (sub_arg_count == 1) {
                    // Change to home directory
                    chdir(getenv("HOME"));
                } else if (sub_arg_count == 2) {
                    // Change to the specified directory
                    if (strcmp(sub_args[1], "-") == 0) {
                        // Change to the previous directory
                        char *prev_dir = getenv("OLDPWD");
                        if (prev_dir != NULL) {
                            chdir(prev_dir);
                        } else {
                            printf("cd: OLDPWD not set\n");
                        }
                    } else {
                        // Change to the specified directory
                        if (chdir(sub_args[1]) != 0) {
                            printf("cd: %s: No such file or directory\n", sub_args[1]);
                        }
                    }
                } else {
                    printf("cd: too many arguments\n");
                }

                // Update pwd to reflect the new current working directory
                pwd = getcwd(NULL, 0);
                if (pwd == NULL) {
                    perror("getcwd");
                    exit(EXIT_FAILURE);
                }

                continue; // Skip executing the command further
            }

            // Create pipe if not the last command
            if (j < arg_count - 1) {
                if (pipe(pipe_fds) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }

            // Fork a child process
            pid_t pid = fork();

            if (pid == -1) {
                // Error occurred
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process
                if (j > 0) {
                    // Redirect input from previous command
                    dup2(prev_pipe_read_end, STDIN_FILENO);
                    close(prev_pipe_read_end);
                }

                if (j < arg_count - 1) {
                    // Redirect output to the next command
                    dup2(pipe_fds[1], STDOUT_FILENO);
                    close(pipe_fds[1]);
                }

                // Execute the command
                execvp(sub_args[0], sub_args);

                // If execvp returns, it means it failed
                printf("%s%s\n", sub_args[0], COMNOTFOUND);
                exit(EXIT_FAILURE);
            } else {
                // Parent process
                if (j > 0) {
                    // Close the read end of the previous pipe
                    close(prev_pipe_read_end);
                }

                if (j < arg_count - 1) {
                    // Close the write end of the current pipe
                    close(pipe_fds[1]);
                    // Update the previous pipe read end for next command
                    prev_pipe_read_end = pipe_fds[0];
                }
            }
        }

        // Wait for all child processes to finish
        for (int j = 0; j < arg_count; j++) {
            wait(NULL);
        }
    }

    // After command execution, restore the original STDOUT
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    // Set the flag back to 1 to enable printing the prompt
    print_prompt = 1;
}

void user_shell() {
    while (1) {
        while (1) {
            snprintf(ps1, sizeof(ps1), "%s", getenv("PS1"));
            printf("%s>$ ", pwd);
            if (fgets(input, MAX_COMMAND_LENGTH, stdin) == NULL) {
                // Check for EOF (CTRL+D)
                exit_gracefully();
            }

            // Remove trailing newline character
            input[strcspn(input, "\n")] = 0;

            // If the command is just whitespace or empty, do nothing and return
            if (strlen(input) == 0 || strspn(input, " \t\n\r\v\f") == strlen(input)) {
                if (DEBUG == 1) {
                    printf("No command. (or just whitespace)\n");
                }
                continue;
            }

            execute_command(input);
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

    signal(SIGINT, sigint_handler);
    
    if (DEBUG == 1)
    {
        printf("DEBUG is on!\n");
        if (STARTMSG == 1)
        {
            printf("STARTMSG is on!\n");
        }
        printf("MAX_COMMAND_LENGTH is %d\n", MAX_COMMAND_LENGTH);
        printf("MAX_ARGUMENTS is %d\n", MAX_ARGUMENTS);
        printf("COMNOTFOUND is '%s'\n", COMNOTFOUND);
        printf("VERSION is %s\n", VERSION);
        printf("NAME is %s\n", NAME);
    }

    // Set PS1 environment variable
    if (setenv("PS1", "\\\\u@\\\\h:\\\\w\\\\$ ", 1) != 0) {
        perror("Error setting PS1");
        return 1;
    }

    pwd = getenv("PWD");

    if (STARTMSG == 1)
    {
        printf("       ___                    \n");
        printf("    __/_  `.  .-````-.        Welcome to\n");
        printf("<3  \\_,` \\| \\-'  /   )`-')    ┌─┐┌─┐┌┼┐┬ ┬\n");
        printf("      '') `'`    \\_ ((`''`    │ ┬│ │└┼┐├─┤\n");
        printf("     ___/  ,    .'/ /|        └─┘└─┘└┼┘┴ ┴\n");
        printf("    (_,___/...-` (_/_/        (v%s)     \n", VERSION);
        printf("═════════════════════════╣\n");
        if (DEBUG == 1)
        {
            printf("DEBUG build\n");
        }
    }

    if (DEBUG == 1)
    {
        printf("%s", ps1);
    }

    user_shell();

    return 0;
}