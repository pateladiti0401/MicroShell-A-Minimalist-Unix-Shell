#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT 1024
#define MAX_ARGS 4
#define MAX_COMMANDS 4
#define DELIMITERS " \t\n"

// Global variable to store the last background process PID
pid_t bg_pid = -1;

//function declarations
int split_input_string(char *input, char *args[], int *argc);
void execute_cmd(char *args[]);
void handle_hash(char *input);
void handle_tilde(char *input);
void handle_background(char *input);
void handle_foreground();
void handle_pipe(char *input);
void handle_redirection(char *input);
void handle_semicolon(char *input);
int handle_conditional_execution(char *input);

void execute_in_background(char *args[]);
void handle_special_chars(char *input);
void fork_and_execute_wc(const char *file_name);
void fork_and_execute_cmds(char *cmds[], int num_cmds, int pipe_fds[][2]);
void check_process_status(pid_t pid, int status);
int execute_single_command(char *cmd[]);
void print_error(const char *message);


int main() {
    char input[MAX_INPUT];  // Buffer to hold the user input
    int is_run = 1;        // Flag to control the loop

    // Start the command loop
    while (is_run) {
        printf("microshell$ ");  
        fflush(stdout);        // Ensure the prompt is printed immediately

        // Read a line of input from the user
     if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }
       
        input[strcspn(input, "\n")] = 0;  // Remove newline character from the input

        // Command handling
        if (strcmp(input, "dter") == 0) {
            printf("Killed\n"); // If the input command is "dter", print "Killed" and stop the loop
            is_run = 0;  // Set flag to 0 to exit the loop
        } else if (strcmp(input, "fore") == 0) {
            // If the input command is "fore", handle foreground process
            handle_foreground();
        } else {
            // For other commands, handle special characters in the input
            handle_special_chars(input);
        }
    }

    return 0;  
}

void execute_cmd(char *args[]) {
    int argc = 0;

    // Calculate the number of arguments
    while (args[argc] != NULL) {
        argc++;
    }

    // Check if the number of arguments is within the valid range
    if (argc < 1 || argc > MAX_ARGS) {
        print_error("Error: Number of arguments must be between 1 and 4.");
        return;
    }

    // Check the syntax for the 'grep' command
    if (strcmp(args[0], "grep") == 0) {
        if (argc > 3) {
            print_error("grep: wrong syntax");
            return;
        }
    }

    // Create a pipe for communication between processes
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    // Fork a child process
    pid_t pid = fork();

    if (pid == 0) {
        // child process
        close(pipefd[0]);  // Close the read end of the pipe
        dup2(pipefd[1], 1);  // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], 2);  // Redirect stderr to the write end of the pipe
        close(pipefd[1]);  // Close the write end of the pipe after duplicating

        // Execute the command
        execvp(args[0], args);

        // If execvp fails
        fprintf(stderr, "Failed to execute '%s': %s\n", args[0], strerror(errno));
        exit(1);  // Exit with error status
    } else if (pid > 0) {
        // parent process
        close(pipefd[1]);  // Close the write end of the pipe

        char buffer[4096];
        ssize_t bytes;
        int status;
        int get_op = 0;

        // Read the output from the pipe
        while ((bytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes] = '\0';  // Null-terminate the buffer
            printf("%s", buffer);  // Print the output
            get_op = 1;  // Mark that output has been received
        }
        printf("\n");

        close(pipefd[0]);  // Close the read end of the pipe

        // Wait for the child process to complete
        waitpid(pid, &status, 0);

        // Check if no output was received and the command was 'grep'
        if (!get_op && strcmp(args[0], "grep") == 0) {
            print_error("No matches found");
        }
    } else {
        // If fork fails
        perror("fork");
    }
}

void handle_hash(char *input){
char *file_name;

    // Skip the '#' character and extract the filename
    file_name = strtok(input + 1, DELIMITERS);

    // Ensure a filename was provided
    if (!file_name) {
        print_error("No filename provided after '#'.");
        return;
    }

    // Check for additional arguments (syntax error)
    if (strtok(NULL, DELIMITERS)) {
        print_error("Syntax error: Extra arguments found.");
        return;
    }

    // Fork and execute the command
    fork_and_execute_wc(file_name);
}
void fork_and_execute_wc(const char *file_name) {
    // Fork a new process
    pid_t child_pid = fork();

    if (child_pid == -1) {
        // Fork failed
        print_error("Failed to create child process.");
        return;
    } else if (child_pid == 0) {
        // In child process
        execlp("wc", "wc", "-w", file_name, NULL);

        // If execlp returns, it means execution failed
        fprintf(stderr, "Failed to execute 'wc': %s\n", strerror(errno));
        exit(1);
    } else {
        // In parent process
        int status;
        waitpid(child_pid, &status, 0);

        // Check if child exited normally and with success
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "wc command failed for file: %s\n", file_name);
        } else if (!WIFEXITED(status)) {
            print_error("wc command did not terminate normally.");
        }
    }
}

void handle_tilde(char *input) {
    char *files[MAX_COMMANDS]; // Array to store file names
    int file_count = 0;        // Number of valid files
    char *token;

    // Check if input starts with '~'
    if (input[0] == '~') {
        print_error("Syntax error: Input must not start with '~'");
        return;
    }

    // Split input by '~' and process each segment
    token = strtok(input, "~");
    while (token != NULL) {
        // Trim leading and trailing whitespace
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ') end--;
        *(end + 1) = '\0';

        // Check for empty segments and ensure file count limit
        if (strlen(token) > 0) {
            if (file_count < MAX_COMMANDS) {
                files[file_count++] = strdup(token); // Store a copy of the trimmed token
            } else {
                print_error("Too many files specified. Maximum allowed is 4");
                return;
            }
        }
        token = strtok(NULL, "~"); // Continue to the next segment
    }

    // Validate file count
    if (file_count == 0) {
        print_error("No files specified");
        return;
    }

    // Process each file
    for (int i = 0; i < file_count; i++) {
        FILE *file = fopen(files[i], "r"); // Open file for reading
        if (file == NULL) {
            fprintf(stderr, "Error opening file '%s': %s\n", files[i], strerror(errno));
            continue; // Continue with the next file
        }

        char buffer[1024];
        // Read and print file contents
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);
        }
        printf("\n"); 

        fclose(file); 
        free(files[i]); // Free allocated memory for the file name
    }
}

// Main function to handle the background command
void handle_background(char *input) {
    // Function to remove the trailing '+' from the input command
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '+') {
        input[len - 1] = '\0';
    } // Clean up the input
    char *args[MAX_ARGS + 1] = {0};  // Array to hold command arguments
    int arg_count = 0;              // Number of arguments
    split_input_string(input, args, &arg_count);  // Split the input into arguments
     if (arg_count > MAX_ARGS) {
        printf("Error: More than %d arguments are not allowed.\n", MAX_ARGS);
        return;
    }
    execute_in_background(args);  // Execute the command in background
}

// Function to execute a command in the background
void execute_in_background(char *args[]) {
    pid_t child_pid = fork();
    if (child_pid == 0) {
        // In child process
        setpgid(0, 0);  // Set process group ID to child's PID
        execvp(args[0], args);
        // If execvp fails, print an error message and exit
        perror("execvp");
        exit(1);
    } else if (child_pid > 0) {
        // In parent process
        printf("Background process started with PID: %d\n", child_pid);
        bg_pid = child_pid;
    } else {
        // Fork failed, print an error message
        perror("fork");
    }
}


// Main function to handle bringing the last background process to the foreground
void handle_foreground() {
    if (bg_pid == -1) {
        // No background process available
        printf("No background process available.\n");
        return;
    }

    // Ignore SIGTTOU and SIGTTIN signals to prevent stopping the shell
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    // Bring the background process to foreground
    printf("Bringing process %d to foreground\n", bg_pid);
    tcsetpgrp(0, bg_pid);  // Set the foreground process group to the background process' PID

    int status;
    waitpid(bg_pid, &status, WUNTRACED);  // Wait for process state change

    // Restore the shell as the foreground process group
    tcsetpgrp(0, getpgrp());

    // Check process status
    if (WIFSTOPPED(status)) {
        // Process is stopped (e.g., due to a signal)
        printf("Process %d stopped\n", bg_pid);
    } else {
        // Process has finished executing
        printf("Process %d finished\n", bg_pid);
        bg_pid = -1;  // Reset the last background PID
    }

    // Restore default signal handling for SIGTTOU and SIGTTIN
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
}

void handle_pipe(char *input) {
    char *cmds[MAX_COMMANDS]; // Array to store commands separated by pipes
    int num_cmds = 0;        // Number of commands
    char *token = strtok(input, "|"); // Tokenize input by '|'

    // Split input into individual commands
    while (token != NULL) {
        cmds[num_cmds++] = token; // Store command
        token = strtok(NULL, "|"); // Continue tokenizing
    }

    // Check for too many commands
    if (num_cmds > MAX_COMMANDS) {
        print_error("Error: Too many commands in piped command. Maximum allowed is 4.");
        return;
    }

    // Create pipes for inter-process communication
    int pipe_fds[MAX_COMMANDS - 1][2];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipe_fds[i]) == -1) {
            perror("pipe");
            return;
        }
    }

  // Fork and execute commands
    fork_and_execute_cmds(cmds, num_cmds, pipe_fds);

    // Parent process: close all pipe file descriptors
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }
}
// Function to fork and execute commands
void fork_and_execute_cmds(char *cmds[], int num_cmds, int pipe_fds[][2]) {
    for (int i = 0; i < num_cmds; i++) {
        char *args[MAX_ARGS + 1] = {0}; // Array to store arguments for execvp
        int arg_count = 0;             // Number of arguments

        // Split command into arguments
        if (split_input_string(cmds[i], args, &arg_count) != 0) {
            print_error("Error: Too many arguments in piped command. Maximum allowed is 4.");
            exit(1); // Exit if error occurs
        }

        pid_t pid = fork(); // Fork a new process
        if (pid == 0) {
            // Child process
            //Each process’s output is piped to the next process’s input
            if (i > 0) {
                dup2(pipe_fds[i - 1][0], 0); // Redirect input from previous pipe
            }
            if (i < num_cmds - 1) {
                dup2(pipe_fds[i][1], 1); // Redirect output to next pipe
            }
            // Close all pipe file descriptors
          for (int i = 0; i < num_cmds - 1; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }

            execvp(args[0], args); // Execute the command
            perror("execvp");      // Print error if execvp fails
            exit(1);
        } else if (pid < 0) {
            perror("fork"); // Handle fork failure
            exit(1);
        }
    }
}

void handle_redirection(char *input) {
    char *args[MAX_ARGS + 1] = {0};  
    int argc = 0;  
    char *input_file = NULL; 
    char *output_file = NULL;  
    int append = 0;  

    // Check for output redirection with append (">>")
    char *output_redirect = strstr(input, ">>");
    if (output_redirect != NULL) {
        *output_redirect = '\0';  // Null-terminate the input string before ">>"
        output_file = output_redirect + 2;  // Set the output_file pointer to the part after ">>"
        append = 1;  
    } else {
        // Check for output redirection without append (">")
        output_redirect = strstr(input, ">");
        if (output_redirect != NULL) {
            *output_redirect = '\0';  
            output_file = output_redirect + 1;  // Set the output_file pointer to the part after ">"
        }
    }

    // Check for input redirection ("<")
    char *input_redirect = strstr(input, "<");
    if (input_redirect != NULL) {
        *input_redirect = '\0';  
        input_file = input_redirect + 1;  // Set the input_file pointer to the part after "<"
    }

    // Trim leading/trailing whitespace from file names
    if (input_file) input_file = strtok(input_file, DELIMITERS);  
    if (output_file) output_file = strtok(output_file,DELIMITERS);  

    // Split command into arguments
    if (split_input_string(input, args, &argc) != 0) {
        print_error("Too many arguments in a command. Maximum allowed is 4.");  
        return;  
    }

    // Fork a child process to handle redirections and execute the command
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Input redirection
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);  // Open the input file for reading
            if (fd_in == -1) {
                perror("open"); 
                exit(1); 
            }
            if (dup2(fd_in, 0) == -1) { //duplicates its file descriptor 
                perror("dup2"); 
                exit(1);  
            }
            close(fd_in);  // Close the original file descriptor
        }

        // Output redirection
        if (output_file != NULL) {
            //It opens the output file with appropriate flags (create, append, or truncate)
            int flags = O_WRONLY | O_CREAT;  // Flags for opening the output file (write and create)
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;  // File permissions (read/write for user, read for group/others)
            if (append) {
                flags |= O_APPEND;  // Set append mode if needed
            } else {
                flags |= O_TRUNC;  // Truncate the file if not appending
            }
            int fd_out = open(output_file, flags, mode);  // Open the output file with the specified flags and mode
            if (fd_out == -1) {
                perror("open");  
                                exit(1);  
            }
            if (dup2(fd_out, 1) == -1) {// duplicates the output file descriptor
                perror("dup2"); 
                exit(1);  
            }
            close(fd_out);  
        }

        // Execute the command
        execvp(args[0], args); //replaces the child process with the new command
        perror("execvp");  
        exit(1);  
    } else if (pid < 0) {
        // Fork error
        perror("fork");  // Print error if fork fails
        exit(1);  
    }

    // Parent process waits for child to finish
    wait(NULL);  
}

void handle_semicolon(char *input)
{
    char *commands[MAX_COMMANDS]; 
    int num_commands = 0;         
    char *token = strtok(input, ";"); 

    // Split input into commands
    while (token != NULL )
    {
        commands[num_commands++] = token; // Store the token (command) in the array
        token = strtok(NULL, ";");      
    }

    // Check if the number of commands exceeds the maximum allowed
    if (num_commands > MAX_COMMANDS)
    {
        print_error("Too many commands. Maximum allowed is 4."); 
        return; 
    }

    // Process each command
    for (int i = 0; i < num_commands; i++)
    {
        char *args[MAX_ARGS + 1] = {0}; 
        int argc = 0; 

        // Trim leading whitespace
        char *trimmed_command = commands[i]; 
        while (*trimmed_command == ' ' || *trimmed_command == '\t')
            trimmed_command++; 

        // Trim trailing whitespace
        char *end = trimmed_command + strlen(trimmed_command) - 1; // Pointer to the end of the command
        while (end > trimmed_command && (*end == ' ' || *end == '\t'))
            end--; 

        *(end + 1) = '\0'; // Null-terminate the trimmed command string

        // Check if the trimmed command is empty
        if (strlen(trimmed_command) == 0)
        {
            print_error("Empty command."); 
            continue; 
        }

        // Split the command into arguments
        if (split_input_string(trimmed_command, args, &argc))
        {
            print_error("Too many arguments in a command. Maximum allowed is 4."); 
            continue; 
        }

        // Check if no command was provided
        if (argc < 1)
        {
            print_error("No command provided."); // Print error message
            continue; // Skip to the next command
        }

        // Execute the command with the parsed arguments
        execute_cmd(args);
    }
}

// Function to execute a single command
int execute_single_command(char **cmd) {
    pid_t pid = fork();
     // If fork fails
    if (pid == -1) {
        perror("fork");
        return 0;
    } else if (pid == 0) {// In the child process
        execvp(cmd[0], cmd); // Execute the command
        perror("execvp");
        exit(1);
    } else { // In the parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
        // Return whether the command executed successfully
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
}

// Function to handle conditional execution of commands
int handle_conditional_execution(char *input) {
    // Arrays to store commands and operators
    char *commands[MAX_COMMANDS][MAX_ARGS] = {0};
    char operators[MAX_COMMANDS - 1] = {0};
    int cmd_count = 0, op_count = 0;
// Tokenize the input string
    char *token = strtok(input, " ");
    int arg_count = 0;

    while (token != NULL && cmd_count < MAX_COMMANDS) {
         // If the token is a logical operator
        if (strcmp(token, "&&") == 0 || strcmp(token, "||") == 0) {
            // Check for argument count and operator count limits
            if (arg_count == 0 || op_count >= MAX_COMMANDS - 1) {
                fprintf(stderr, "Too many arguments in a command. Maximum allowed is 4.\n");
                return 0;
            }
            // Store the operator and reset argument count
            operators[op_count++] = token[0];
            cmd_count++;
            arg_count = 0;
        } else {
            // Store the command argument
            if (arg_count < MAX_ARGS - 1) {
                commands[cmd_count][arg_count++] = token;
            }
        }
        token = strtok(NULL, " ");
    }
    cmd_count++;

    if (cmd_count > MAX_COMMANDS) {
        fprintf(stderr, "Error: Too many commands (max %d)\n", MAX_COMMANDS - 1);
        return 0;
    }
// Execute the first command and handle the logic for conditional execution
    int result = execute_single_command(commands[0]);
    for (int i = 0; i < op_count; i++) {
        if (operators[i] == '&') {  // AND
            if (result) {
                result = execute_single_command(commands[i + 1]);
            }
        } else if (operators[i] == '|') {  // OR
            if (!result) {
                result = execute_single_command(commands[i + 1]);
            }
        }
    }

    return result;
}

// Function to handle special characters in input
void handle_special_chars(char *input)
{
    if (input[0] == '#')
    {
        handle_hash(input);
    }
    else if (strchr(input, '~'))
    {
        handle_tilde(input);
    }
     else if (strstr(input, "&&") || strstr(input, "||"))
    {
        handle_conditional_execution(input);
    }
    else if (strchr(input, '|'))
    {
        handle_pipe(input);
    }
       else if (strchr(input, '<') || strchr(input, '>'))
    {
        handle_redirection(input);
    }
    else if (strchr(input, '+'))
    {
        handle_background(input);
    }
    else if (strchr(input, ';'))
    {
        handle_semicolon(input);
    }
    else
    {
        char *args[MAX_ARGS + 1] = {0};
        int argc = 0;
         // Split the input string into arguments
         if (split_input_string(input, args, &argc) != 0)
        {
            print_error("Error: Invalid number of arguments. Must be between 1 and 4.");
            return;
        }
        else
        {
            execute_cmd(args);
        }
    }
}

// Function to split input string into arguments
int split_input_string(char *input, char **args, int *argc) {
    char *token = strtok(input, DELIMITERS);
    while (token != NULL && *argc < MAX_ARGS) {
        args[(*argc)++] = token; // Store each argument
        token = strtok(NULL, DELIMITERS);
    }
    if (*argc >= MAX_ARGS && token != NULL) {
        return -1; // Too many arguments
    }
    args[*argc] = NULL; // Null-terminate the argument list
    return 0;
}

void print_error(const char *message){
    fprintf(stderr, "%s\n", message);
}
