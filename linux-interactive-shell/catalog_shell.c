#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

// Main loop for the Catalog shell sub-system
int main() {
    char buffer[200];
    char *param[10];
    pid_t pid;

    // Display entry message
    printf("Entering Catalog Shell...\n"); 

    while(1){
        // Print the sub-shell prompt
        printf("Catalog$** ");
        if(fgets(buffer,200,stdin)==NULL) break;

        // Remove the newline character
        buffer[strcspn(buffer, "\n")] = 0;

        // Split the input into tokens by space
        int i = 0;
        char *temp = strtok(buffer, " ");
        while(temp != NULL && i < 9){
            param[i++]=temp;
            temp=strtok(NULL," ");
        }
        param[i] = NULL;

        // Check for empty input or only spaces
        if(param[0]==NULL){
            printf("Not Supported\n");
            continue;
        }

        // Return control back to LibShell - Fixed to use param[0]
        if (strcmp(param[0], "Esc") == 0) {
            printf("Returning to LibShell...\n");
            break; 
        }

        // Check if input matches allowed commands
        if(strcmp(param[0],"find")==0){
            if(param[1]==NULL || param[2] == NULL)
                printf("Missing parameters!!!\n");  
            else{
                pid = fork(); // Create a new process
                if (pid < 0) 
                    printf("Fork failed\n");
                else if (pid == 0) {
                    char *args[]= {"grep",param[1],param[2],NULL};
                    execvp(args[0], args);
                    exit(1);
                } else{
                    wait(NULL);
                }
            }
        } 

        // List catalog files
        else if (strcmp(param[0], "ls") == 0) {
            if (param[1] == NULL) {
                printf("Missing parameters!!!\n");
            } else {
                pid = fork();
                if (pid == 0) {
                    char *args[] = {"ls", param[1], NULL};
                    execvp(args[0], args);
                    exit(1);
                } else {
                    wait(NULL);
                }
            }
        }

        // Create a new catalog directory
        else if (strcmp(param[0], "newcat") == 0) {
            if (param[1] == NULL) {
                printf("Missing parameters!!!\n");
            } else {
                pid = fork();
                if (pid == 0) {
                    char *args[] = {"mkdir", param[1], NULL};
                    execvp(args[0], args);
                    exit(1);
                } else {
                    wait(NULL);
                }
            }
        }

        // Run an external program with one argument
        else if (strcmp(param[0], "run") == 0) {
            if (param[1] == NULL || param[2] == NULL) {
                printf("Missing parameters!!!\n");
            } else {
                pid = fork();
                if (pid == 0) {
                    char *args[] = {param[1], param[2], NULL};
                    execvp(args[0], args);
                    exit(1);
                } else {
                    wait(NULL);
                }
            }
        }

        // Handle invalid commands
        else {
            printf("Not Supported\n");
        }
    }
    
}
