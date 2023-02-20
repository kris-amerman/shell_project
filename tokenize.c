#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.c"
#include "vect.c"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// ============================= CONSTANTS =============================

const unsigned int MAX_EXPR_LEN = 256;

// ============================== HELPERS ==============================

// Is the given character a "special" character?
int is_special(char c) {
  return c == '(' || c == ')' || c == '<' || c == '>' || c == ';' || c == '|' || c == '\t' || c == ' ';
}

// Is the given character whitespace?
int is_whitespace(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

// Read a sequence of non-special characters from an input string,
// and write them to an output buffer
int read_word(const char *input, char *output) {
  int i = 0;
  // Copy the characters one at a time, as long as the character is non-special
  // and we haven't reached the end of the input
  while (input[i] != '\0' && !is_special(input[i]) && input[i] != '\n') {
    // overwrite buffer
    output[i] = input[i];
    ++i;
  }
  // Return the length of the word
  return i; 
}

// Read a sequence of characters bounded by double quotes from an input string,
// and write them to an output buffer (without the double quotes)
int read_sentence(const char *input, char *output) {
  // Initialize as 1 to ignore the first double quote
  int i = 1;
  // Copy the characters one at a time, as long as the character isn't a 
  // double quote and we haven't reached the end of the input
  while(input[i] != '\0' && input[i] != '"' && input[i] != '\n') {
    // overwrite buffer (and adjust buffer index to account for i offset)
    output[i - 1] = input[i];
    ++i;
  }
  // Return the length of the sentence (plus 1 to skip over the last double quote)
  return i + 1;
}

// Takes a string and decomposes it into a vector of string tokens.
// char* -> vect_t*
vect_t *tokenize(char expr[]) {
  assert(strlen(expr) < MAX_EXPR_LEN);

  // Temporary string buffer; we use this buffer to store words before allocating
  // memory for the word in the token array; no word is longer than 255 chars
  char temp_buffer[MAX_EXPR_LEN];

  // Allocate memory for the token array. We assume there can be
  // at most 255 unique tokens (each character in the expr string).
  vect_t *tokens = vect_new();

  int i = 0;

  // While we haven't reached the end of the expression 
  while (expr[i] != '\n' && expr[i] != '\0') {

    // CASE 1: special character
    if (is_special(expr[i])) {
      // SUB-CASE 1: not whitespace
      if (!is_whitespace(expr[i])) {
        char *special = (char *)malloc(2 * sizeof(char));
        special[0] = expr[i];
        special[1] = '\0';

        vect_add(tokens, special);
        
        ++i;
      }
      // SUB-CASE 1: whitespace
      else {
        ++i;
      }
    } 
    // CASE 2: sentence
    else if (expr[i] == '"') {
      // Read and write to a temporary buffer
      int len = read_sentence(&expr[i], temp_buffer);
      // Allocate some memory for the sentence based on the length
      char *sentence = (char *)malloc(len * sizeof(char));
      for (int j = 0; j < len; j++) {
        sentence[j] = temp_buffer[j]; // copy over chars to word
      }
      sentence[len] = '\0';
      
      vect_add(tokens, sentence);

      i += len;
    }
    // CASE 3: word
    else {
      // Read and write to a temporary buffer
      int len = read_word(&expr[i], temp_buffer);
      // Allocate some memory for the word based on the length
      char *word = (char *)malloc(len * sizeof(char));
      for (int j = 0; j < len; j++) {
        word[j] = temp_buffer[j]; // copy over chars to word
      }
      word[len] = '\0';
      
      vect_add(tokens, word);

      i += len;
    }
  }
  return tokens;
}


int main(int argc, char **argv) {
  
  char buffer[256];
  // int total_bytes = read(0, buffer, 255);  
  // buffer[total_bytes] = '\0';
  // close(0);

  // Takes buffer as expr
  vect_t *tokens = tokenize(buffer);

  // int i = 0;
  // while (i < tokens->size) {
  //   printf("%s\n", tokens->data[i]);
  //   ++i;
  // }

  vect_delete(tokens);
}
