#!/bin/bash
# Compilation script for LibShell project 

gcc libshell.c -o libshell
gcc reader_shell.c -o reader_shell
gcc catalog_shell.c -o catalog_shell
gcc archive_shell.c -o archive_shell

# Run the main program 
./libshell
