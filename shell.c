#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

void redirect(char* array[], int size);

int main() {
    /*
      The while loop below continually prompts the user for input.
      Fork is called, and the child process executes the user input.

     */

    // env stores the home directory

    char* env;
    env = getenv("HOME");
// changes the pwd to home
    chdir(env);
    int hasHome = 0;
    char exitor[4] = "exit";
    int pid;
    while(1) {

        char cwd[256];
        // gets the cwd, stores in cwd array

        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");

        } // if
        // checks if the cwd has home in it
        for (int q = 0; q < strlen(env); q++) {
            if (cwd[q] == env[q]) {
                hasHome = 1;
            } else {
                hasHome = 0;
                break;
            } // else
        } // for
        int hasRedirect = 0;
        char buffer[4096];
        char* array[2000];

        setbuf(stdout, NULL);

        if (hasHome == 1) {
            printf("1730sh:~%s$ ", cwd + strlen(env));
        } else {
            printf("1730sh:%s$ ", cwd);
        } // else
        int bytes = read(STDIN_FILENO, buffer, sizeof(buffer));

        // we create a minibuffer to hold the values of user input


        char mini[bytes + 1];
        for (int i = 0; i < bytes; i++) {
            mini[i] = buffer[i];
        } // for

// we assign the last position in the buffer to a null terminator
        mini[bytes] = '\0';

        // we remove all the instances of \n
        for (int g = 0; g < bytes + 1; g++) {
            if (mini[g] == '\n') {
                mini[g] = ' ';
            } // if
        } // for

        // we use the strtok function to split the string by spaces
        char* token = strtok(mini, " ");
        int i = 0;
        while (token != NULL) {
            array[i] = token;
            token = strtok (NULL, " ");
            i++;
        } // while

// we copy the command into a new array and add NULL at the ending
        char* cmd[i + 1];
        cmd[i] = NULL;

        for (int d = 0; d < i; d++) {
            cmd[d] = array[d];
            if (cmd[d][0] == '<' || cmd[d][0] == '>') {
                // hasRedirect signals if input redirection is needed
                hasRedirect = 1;
            } // if
        } // for

        if (hasRedirect == 1) {
            // we pass in "i" as the size, to exclude the null terminator
            redirect(array, i);
            continue;
        } // if


        if (cmd[0] != NULL) {
            if (cmd[0][0] == 'c' && cmd[0][1] == 'd') {
                if (chdir(cmd[1]) < 0) perror("chdir");

                continue;
            } // if
        } // if

        // acts as a flag to determine whether exit command is found
        int s = 0;

        for (int x = 0; x < i; x++) {
            if (array[0][x] == exitor[x]) {
                s = 1;
                continue;
            } else {
                s = 0;
            } // else
        } // for

        if (s == 1) {
            exit(0);
        } else {
// we fork below

            if ((pid = fork()) < 0) perror("error");
            else if (pid == 0) { // child process
                if (execvp(cmd[0], cmd) == -1) {
                    perror("execvp");
                    return EXIT_FAILURE;
                } // if
            } else { // parent process
                int status;
                wait(&status);
            } // else

        } // else
    } // while
} // main

void redirect(char* array[], int size) {
    int type = 0;
    int commandCount = 0;
    int hasRight = 0;
    int hasLeft = 0;
    int fdIn;
    int fdOut;
    pid_t pid;
    char* input;
    char* output;

// counts the number of indices to traverse to get to the first redirect

    for (int i = 0; i < size; i++) {
        if (array[i][0] == '<' || array[i][0] == '>') {

            break;
        } // if
        commandCount += 1;
    } // for

    // stores strings required for execvp

    char* command[commandCount + 1];

    for (int q = 0; q < commandCount; q++) {
        command[q] = array[q];

    } // for

    // the last space in the array is set to NULL
    command[commandCount] = NULL;

// below are flags to store which type of redirect is present
    // the loop alse searches for the output file, which is after >

    for (int u = 0; u < size; u++) {
        if (array[u][0] == '>') {
            hasRight = 1;
            if (array[u][1] == '>') {
                type = 1;
            } // if
            output = array[u + 1];
            break;
        } // if
    } // for
// the loop searches for the input file, located after <
    // the hasLeft flag is turned on, indicating that the left < is used

    for (int h = 0; h < size; h++) {
        if (array[h][0] == '<') {
            hasLeft = 1;
            input = array[h + 1];
            break;
        } // if
    } // for

// opening the input file

    if (hasLeft == 1) {
        fdIn = open(input, O_RDONLY);
        if (fdIn == -1) perror("open");
// opens the output file in append if type = 1, else opens normally

    }
    if (hasRight == 1) {
        if (type == 0) {
            fdOut = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0744);
            if (fdOut == -1) perror("open");

        } else {
            fdOut = open(output, O_WRONLY | O_CREAT | O_APPEND, 0744);
            if (fdOut == -1) perror("open");

        } // else
    } // if

// forking into a child process for dup2 and execvp

if ((pid = fork()) < 0) perror("fork");
		else if (pid == 0) { // child process
            if (hasLeft == 1) {
                close(STDIN_FILENO);
                // redirect fdIn to STDIN_FILENO
                if (dup2(fdIn, STDIN_FILENO) < 0) perror("dup2: fdIn");
            } // if

            if (hasRight == 1) {
                close(STDOUT_FILENO);
                // redirect fdOut to STDOUT_FILENO
                if (dup2(fdOut, STDOUT_FILENO) < 0) perror("dup2: fdOut");
            } // if

            // executing commands below
			if (execvp(command[0], command) == -1) {
				perror("execvp");
				return;
			} // if

		} else {	// parent process
			int status;
			wait(&status);
//			if (WIFEXITED(status)) {
//				printf("child exited normally with status = %d\n", WEXITSTATUS(status));
//			} else if (WIFSIGNALED(status)) {
//				printf("child exited abnormally due to signal = %d\n", WTERMSIG(status));
//			} // if

		} // if



















} // else
