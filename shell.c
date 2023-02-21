#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parse.h" // TODO fix the h files

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// ============================= CONSTANTS =============================

const int MAX_EXP_LEN = 255;

// ============================== HELPERS ==============================






// =============================== MAIN ===============================

int main(int argc, char **argv) {

  // --- SETUP ---  

  // input buffer (initialized to the max expression length plus 1  
  // to leave room for the null terminator if the user decides to use
  // all 255 characters)
  char buffer[MAX_EXP_LEN + 1];


  // --- CLEANUP ---
  // strarr_delete(tokens);

  
  printf("Welcome to mini-shell.\n");
  while (1) {
    // ------------- SETUP -------------

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

    // add a null terminator to the end of the buffer
    buffer[strlen(buffer) + 1] = '\0';


    // ------- PROCESS USER INPUT -------
    
    // handle "exit" command
    if (strcmp(buffer, "exit") == 0) {
      printf("Bye bye.\n");
      break;
    }
    
    // tokenize
    strarr_t *tokens = tokenize(buffer);

    // launch program with exec
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
      execvp(tokens->data[0], args);
      // if execvp returns, an error occurred
      perror("execvp");
      exit(1);
    }
    else {
      // parent process
      int status;
      if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        exit(1);
      }
    }

    // int i = 0;
    // while (i < tokens->size) {
    //   printf("%s\n", tokens->data[i]);
    //   ++i;
    // }




    // ------------ CLEANUP -------------

    // clear buffer for next iteration
    memset(buffer, 0, sizeof(buffer));

    // clear the tokens for the next iteration
    strarr_delete(tokens);
  }



  return 0;
}
