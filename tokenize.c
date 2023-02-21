#include "parse.h"

// =============================== MAIN ==============================

int main(int argc, char **argv) {
  
  char buffer[256];
  int total_bytes = read(0, buffer, 255);  
  buffer[total_bytes] = '\0';
  close(0);

  // Takes buffer as expr
  strarr_t *tokens = tokenize(buffer);

  int i = 0;
  while (i < tokens->size) {
    printf("%s\n", tokens->data[i]);
    ++i;
  }

  strarr_delete(tokens);

  return 0;
}
