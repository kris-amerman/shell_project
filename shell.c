#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tokenize.c"

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
    if (scanf("%255[^\n]", buffer) != 1) {
      // no input read, consume newline and restart loop
      getchar(); 
      continue;
    }

    // ignore any remaining characters in stdin (if the user
    // entered more than 255)
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}

    // add a null terminator to the end of the buffer
    buffer[strlen(buffer)] = '\0';


    // ------- PROCESS USER INPUT -------
    
    // handle "exit" command
    if (strcmp(buffer, "exit") == 0) {
      printf("Bye bye.\n");
      break;
    }
    
    printf("BUFFER: %s\n", buffer);

    // tokenize
    strarr_t *tokens = tokenize(buffer);
    int i = 0;
    while (i < tokens->size) {
      printf("%s\n", tokens->data[i]);
      ++i;
    }

    printf("NUM TOKENS: %d\n", tokens->size);

    // ------------ CLEANUP -------------

    // clear buffer for next iteration
    memset(buffer, 0, sizeof(buffer));

    // clear the tokens for the next iteration
    strarr_delete(tokens);
  }



  return 0;
}
