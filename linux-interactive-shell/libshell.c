#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


int main() {
    char buffer[200];
    pid_t pid;

    // Display entry message
    printf("Welcome to LibShell!\n");
    printf("Enter <reader> for reading room commands\n");
    printf("Enter <catalog> for catalog search\n");
    printf("Enter <archive> for file archive tools\n");

    while (1) {
        // Prefix for commands
        printf("Lib$** ");
        
        if (fgets(buffer, 200, stdin) == NULL) break;

        // Removing the \n 
        buffer[strcspn(buffer, "\n")] = 0;


        // Extract the first word to ignore leading spaces
        char *command = strtok(buffer, " ");

        // Handle empty input
        if (command == NULL) {
            printf("Not Supported\n");
            continue;
        }

        // Exit the entire system 
        if (strcmp(command, "exit") == 0) {
            printf("Goodbye from LibShell!\n"); 
            exit(0);
        }

        char *shell_path = NULL;

        // Map commands to sub-shell executables 
        if (strcmp(command, "reader") == 0) {
            shell_path = "./reader_shell";
        } else if (strcmp(command, "catalog") == 0) {
            shell_path = "./catalog_shell";
        } else if (strcmp(command, "archive") == 0) {
            shell_path = "./archive_shell";
        } else {
            // Command not recognized 
            printf("Not Supported\n");
            continue;
        }

        // Fork a child process to run the sub-shell 
        pid = fork();
        if (pid < 0) {
            printf("Fork failed\n");
        } 
        else if (pid == 0) {
            // Execute the sub-shell program 
            char *args[] = {shell_path, NULL};
            execvp(args[0], args);
            
            // Only runs if execvp fails
            printf("Execution failed\n");
            exit(1);
        } 
        else {
            // Wait for sub-shell to finish before returning to Lib$**
            wait(NULL);
        }
    }
}
