# MicroShell: A Minimalist Unix Shell

## Overview
MicroShell is a minimalist Unix-like shell implemented in C, designed to execute user commands within a custom terminal environment. The shell supports a variety of standard command operations, including conditional execution, piping, background processes, file redirection, and special character handling. MicroShell is built to offer a lightweight yet functional alternative to standard shells with specific constraints on command execution.

## Features
- **Basic Command Execution**: Supports commands with `argc` between 1 and 4, ensuring a simple yet versatile command structure.
- **Special Commands**:
  - **`dter`**: Terminates the current MicroShell session.
  - **`#`**: Counts the number of words in a specified `.txt` file.
  - **`~`**: Concatenates up to 4 `.txt` files in sequence, displaying the result.
  - **`+`**: Executes a command in the background, with the ability to bring it back to the foreground using the `fore` command.
  - **`|`**: Supports up to 4 piped commands, enabling complex command chains.
  - **`<`, `>`, `>>`**: Handles input and output redirection, including appending output to files.
  - **`;`**: Executes up to 4 commands sequentially.
  - **`&&`, `||`**: Conditional execution of commands with support for up to 4 operators in a combination of logical AND and OR.

## Compilation and Usage
### Compilation
To compile the MicroShell, use the following command:
```bash
gcc -o microshell microshell.c
```
### Running the Shell
```bash
./microshell
```
Once running, MicroShell will enter an infinite loop, waiting for user commands.

#### Special Commands
Terminate MicroShell:
```bash
microshell$ dter
```
#### Count Words in a File:
```bash
microshell$ # sample.txt
```
#### Concatenate Files:
```bash
microshell$ file1.txt ~ file2.txt ~ file3.txt ~ file4.txt
```
#### Run a Process in the Background:
```bash
microshell$ command arg1 arg2 +
```
#### Bring it back to the foreground:
```bash
microshell$ fore
```
#### Piping Commands:
```bash
microshell$ ls -l | grep txt | wc | wc -w
```
#### Redirection:
Input redirection:
```bash
microshell$ grep to < sample.txt
```
Output redirection:
```bash
microshell$ ls -l > dirlist.txt
```
Append output to a file:
```bash
microshell$ ls -l >> dirlist.txt
```
#### Sequential Command Execution:
```bash
microshell$ date ; pwd ; ls -l
```
#### Conditional Execution:
```bash
microshell$ command1 && command2 || command3 && command4
```


