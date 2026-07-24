#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

// Main loop for the Reader shell sub-system
int main() {
    char buffer[200];
    pid_t pid;

    // Display entry message
    printf("Entering Reader Shell...\n"); 

    while(1){
        // Print the sub-shell prompt
        printf("Reader$** ");
        if(fgets(buffer,200,stdin)==NULL) break;

        // Remove the newline character
        buffer[strcspn(buffer, "\n")] = 0;

        // Use strtok to extract the first word and handle leading spaces
        char *command = strtok(buffer, " ");

        // Error for empty input or just spaces
        if (command == NULL) {
            printf("Not Supported\n");
            continue;
        }

        // Return control back to LibShell
        if (strcmp(command, "Esc") == 0) {
            printf("Returning to LibShell...\n");
            break; 
        }

        // Check if input matches allowed commands
        if(strcmp(command, "date") == 0 || strcmp(command, "whoami") == 0 || 
           strcmp(command, "pwd") == 0 || strcmp(command, "uptime") == 0) {
            
            pid = fork(); // Create a new process
            if (pid < 0) {
                printf("Fork failed\n");
            } 
            else if (pid == 0) {
                // Child: execute the command
                char *args[] = {command, NULL};
                execvp(args[0], args); 
                exit(1);
            } 
            else {
                // Parent: wait for execution to finish
                wait(NULL);
            }
        }
        else {
            // Error for invalid commands
            printf("Not Supported\n");
        }
    }
}
