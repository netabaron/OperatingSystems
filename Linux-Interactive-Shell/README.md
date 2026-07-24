# LibShell - Interactive Linux Shell

## About The Project
This project is an interactive command-line shell implemented in C for Ubuntu/Linux environments. It simulates a digital library management system named "LibShell". 

The architecture is divided into multiple internal sub-shells, where each sub-shell specializes in a specific domain of the library management process.

## Key Technical Features
The project demonstrates direct interaction with the Linux Operating System using standard system calls:
* **Process Management:** Creating child processes using "fork()".
* **Program Execution:** Running external sub-programs and commands using "execvp()".
* **File I/O Operations:** Low-level file manipulation using "open()", "read()", "write()", and "close()".

## Project Structure
* "libshell.c" - The main shell program that orchestrates the system.
* "reader_shell.c" - Sub-shell wrapper for reader commands (commands without parameters).
* "catalog_shell.c" - Sub-shell wrapper for catalog search operations (commands with parameters).
* "archive_shell.c" - Sub-shell wrapper for archive file operations.
* "run_lib.sh" - A shell script provided for compiling and executing the project seamlessly.