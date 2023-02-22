#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>

#include "parse.h" // TODO fix the h files

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// ============================= CONSTANTS =============================

const int MAX_EXP_LEN = 255;

// ============================== HELPERS ==============================

void cd_command(strarr_t *tokens) {
  if (tokens->size == 1) {
    // no arguments, change to home directory
    if (chdir(getenv("HOME")) == -1) {
      perror("cd");
    }
  }
  else if (tokens->size == 2) {
    // one argument, change to specified directory
    if (chdir(tokens->data[1]) == -1) {
      perror("cd");
    }
  }
  else {
    // too many arguments
    printf("cd: too many arguments\n");
  }
}

// ============================== EXECUTE ==============================

// execute user input and return 0 or 1. 0 denotes that the program should 
// continue and 1 denotes that the program should exit.
int execute(strarr_t *tokens, char *buffer) {

  // ========= EXIT =========
  if (strcmp(buffer, "exit") == 0) {
    printf("Bye bye.\n");
    return 1;
  }
  
  // ========= CD =========
  else if (strcmp(tokens->data[0], "cd") == 0) {
    cd_command(tokens);
  }

  // ========= PROGRAM =========
  else {
    pid_t pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(1);
    }
    else if (pid == 0) {
      // child process
      char **args = (char **)malloc(sizeof(char *) * (tokens->size + 1));
      for (int i = 0; i < tokens->size; i++) {
        args[i] = tokens->data[i];
      }
      args[tokens->size] = NULL;

      // launch program with exec
      if (execvp(args[0], args) == -1) {
        printf("%s: command not found\n", args[0]);
      }
      free(args);
      exit(1);
    }
    else {
      // parent process
      wait(NULL);
    }
  }

  return 0;
}


// =============================== MAIN ===============================

int main(int argc, char **argv) {
  // input buffer (initialized to the max expression length plus 1  
  // to leave room for the null terminator if the user decides to use
  // all 255 characters)
  char buffer[MAX_EXP_LEN + 1];
  printf("Welcome to mini-shell.\n");

  while (1) {
    printf("shell $ ");
    fflush(stdout);

    // wait for user input
    int read_val = scanf("%255[^\n]", buffer);
    
    // handle ctrl-d (EOF)
    if (read_val == EOF) {
      // end-of-file, exit
      printf("Bye bye.\n");
      break;
    }
    // handle no input (newline)
    else if (read_val != 1) {
      // no input read, consume newline and restart loop
      getchar(); 
      continue;
    }

    // ignore any remaining characters in stdin (if the user
    // entered more than 255)
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}

    // remove trailing whitespace from buffer and add NULL terminator
    char *end = buffer + strlen(buffer) - 1;
    while (end > buffer && isspace(*end)) {
      *end-- = '\0';
    }

    // ------- PROCESS USER INPUT -------
    
    // tokenize -- MUST CLEANUP TO CONTINUE!
    strarr_t *tokens = tokenize(buffer);

    int shouldExit = execute(tokens, buffer);

    // ------------ CLEANUP -------------

    // clear the tokens for the next iteration
    strarr_delete(tokens);

    // clear buffer for next iteration
    memset(buffer, 0, sizeof(buffer));

    // exit if execute returned status 1
    if (shouldExit == 1) { 
      break; 
    }
  }

  return 0;
}
