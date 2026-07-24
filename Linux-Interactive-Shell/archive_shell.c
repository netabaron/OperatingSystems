#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// Main loop for the Archive shell sub-system
int main() {
    char buffer[200];
    char *param[10];

    // Display entry message before the loop
    printf("Entering Archive Shell...\n");

    while (1) {
        // Print the sub-shell prompt
        printf("Archive$** ");
        if (fgets(buffer, 200, stdin) == NULL) break;

        // Remove the newline character
        buffer[strcspn(buffer, "\n")] = 0;

        // Parse input into tokens
        int i = 0;
        char *temp = strtok(buffer, " ");
        while (temp != NULL && i < 9) {
            param[i++] = temp;
            temp = strtok(NULL, " ");
        }
        param[i] = NULL;

        // Handle empty input
        if (param[0] == NULL) {
            printf("Not Supported\n");
            continue;
        }

        // Return to LibShell using the first token
        if (strcmp(param[0], "Esc") == 0) {
            printf("Returning to LibShell...\n");
            break;
        }

        // Merge command: append src to dst using system calls
        if (strcmp(param[0], "merge") == 0) {
            if (param[1] == NULL || param[2] == NULL) {
                printf("Missing parameters!!!\n");
            } else {
                int fd_src = open(param[1], O_RDONLY);
                if (fd_src < 0) {
                    printf("File not found\n");
                } else {
                    // Open destination for appending, create if it doesn't exist
                    int fd_dst = open(param[2], O_WRONLY | O_APPEND | O_CREAT, 0644);
                    char buf[1024];
                    ssize_t bytes;
                    while ((bytes = read(fd_src, buf, sizeof(buf))) > 0) {
                        write(fd_dst, buf, bytes);
                    }
                    close(fd_src);
                    close(fd_dst);
                    printf("Data from %s merged into %s\n", param[1], param[2]);
                }
            }
        }
        
        // Count command: count characters and lines manually
        else if (strcmp(param[0], "count") == 0) {
            if (param[1] == NULL) {
                printf("Missing parameters!!!\n");
            } else {
                int fd = open(param[1], O_RDONLY);
                if (fd < 0) {
                    printf("File not found\n");
                } else {
                    char ch;
                    long chars = 0, lines = 0;
                    while (read(fd, &ch, 1) > 0) {
                        chars++;
                        if (ch == '\n') lines++;
                    }
                    close(fd);
                    printf("Characters: %ld Lines: %ld\n", chars, lines); 
                }
            }
        }
        
        // Remove command: delete file using unlink system call
        else if (strcmp(param[0], "remove") == 0) {
            if (param[1] == NULL) {
                printf("Missing parameters!!!\n");
            } else {
                if (unlink(param[1]) != 0) {
                    printf("File not found\n"); 
                }
            }
        }
        
        // Protect command: change file permissions using octal mode
        else if (strcmp(param[0], "protect") == 0) {
            if (param[1] == NULL || param[2] == NULL) {
                printf("Missing parameters!!!\n");
            } else {
                // Convert string to octal long
                int mode = strtol(param[1], NULL, 8);
                if (mode < 0 || mode > 0777) {
                    printf("Invalid Mode!!\n"); 
                } else {
                    if (chmod(param[2], mode) == 0) {
                        printf("Permissions updated\n");
                    } else {
                        printf("File not found\n");
                    }
                }
            }
        }
        
        // Handle any other input
        else {
            printf("Not Supported\n");
        }
    }
    
}
