#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "strarr.c"

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
  while (!is_special(input[i]) && input[i] != '\0' && input[i] != '\n' && input[i] != '"') {
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
  int i = 0;
  // Copy the characters one at a time, as long as the character isn't a
  // double quote and we haven't reached the end of the input
  while (input[i] != '"' && input[i] != '\0' && input[i] != '\n') {
    // overwrite buffer
    output[i] = input[i];
    ++i;
  }
  // Return the length of the sentence
  return i;
}

// Takes a string and decomposes it into an array of string tokens.
// char* -> strarr_t*
strarr_t *tokenize(char expr[]) {
  assert(strlen(expr) < MAX_EXPR_LEN);

  // Temporary string buffer; we use this buffer to store words before allocating
  // memory for the word in the token array; no word is longer than 255 chars
  char temp_buffer[MAX_EXPR_LEN];

  // Allocate memory for the token array. We assume there can be
  // at most 255 unique tokens (each character in the expr string).
  strarr_t *tokens = strarr_new(MAX_EXPR_LEN);

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
        tokens->data[tokens->size] = special;
        ++tokens->size;
        ++i;
      }
      // SUB-CASE 1: whitespace
      else {
        ++i;
      }
    } 
    // CASE 2: sentence
    else if (expr[i] == '"') {
      // Read and write to a temporary buffer (and increment by 1 to skip
      // over the first double quote)
      int len = read_sentence(&expr[++i], temp_buffer);
      
      // Only add to tokens if sentence is not empty 
      if (len) {
        // Allocate some memory for the sentence based on the length plus the null terminator
        char *sentence = (char *)malloc((len + 1) * sizeof(char));
        for (int j = 0; j < len; j++) {
          sentence[j] = temp_buffer[j]; // copy over chars to word
        }
        sentence[len] = '\0';
        tokens->data[tokens->size] = sentence;
        ++tokens->size;
        i += len;
      }
      i++;
    }
    // CASE 3: word
    else {
      // Read and write to a temporary buffer
      int len = read_word(&expr[i], temp_buffer);
      // Allocate some memory for the word based on the length plus the null terminator
      char *word = (char *)malloc((len + 1) * sizeof(char));
      for (int j = 0; j < len; j++) {
        word[j] = temp_buffer[j]; // copy over chars to word
      }
      word[len] = '\0';
      tokens->data[tokens->size] = word;
      ++tokens->size;
      i += len; 
    }
  }
  return tokens;
}

// =============================== MAIN ==============================

// int main(int argc, char **argv) {
  
//   char buffer[256];
//   int total_bytes = read(0, buffer, 255);  
//   buffer[total_bytes] = '\0';
//   close(0);

//   // Takes buffer as expr
//   strarr_t *tokens = tokenize(buffer);

//   int i = 0;
//   while (i < tokens->size) {
//     printf("%s\n", tokens->data[i]);
//     ++i;
//   }

//   printf("\nNUM TOKENS: %d\n", tokens->size);

//   strarr_delete(tokens);

//   return 0;
// }
