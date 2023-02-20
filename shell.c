#include <stdio.h>
#include <string.h>

const MAX_EXP_LEN = 256;

// Takes a string and decomposes it into an array of string tokens.
// char* -> char** 
char** build_token_array(char expr[]) {
  assert(strlen(expr) < 256);
  // Allocate memory for the tokens. We assume there can be 
  // at most 255 unique tokens (each character in the expr string).
  char** tokens = (char**) malloc(sizeof(char*) * MAX_EXP_LEN);
  
}

int main(int argc, char **argv) {

  // TODO: Implement your shell's main
  return 0;
}
