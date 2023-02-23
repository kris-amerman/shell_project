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


// ============================= PROTOTYPES ============================
// TODO !! add these to a header file

int execute(strarr_t *tokens);

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

// returns 0 to exit. returns 1 to continue.
int source_command(strarr_t *tokens) {
  // check that there is exactly one argument
  if (tokens->size != 2) {
    printf("Usage: source <filename>\n");
    return 0;
  }

  // open the file for reading
  FILE *file = fopen(tokens->data[1], "r");
  if (file == NULL) {
    perror("fopen");
    return 0;
  }

  int exitStatus = 1;

  // read and execute each line of the file
  char line[MAX_EXP_LEN + 1];
  while (exitStatus == 1 && fgets(line, sizeof(line), file) != NULL) {
    // remove trailing newline (if any)
    char *nl = strchr(line, '\n');
    if (nl != NULL) {
      *nl = '\0';
    }

    // tokenize and execute the line as a command
    strarr_t *line_tokens = tokenize(line);
    exitStatus = execute(line_tokens);
    strarr_delete(line_tokens);
  }

  // close the file
  fclose(file);
  return exitStatus;
}

void help_command() {
  printf("\n*** Shell Built-in Commands ***\n\n");
  printf("  cd [directory]  Change the current working directory.\n");
  printf("  source [file]   Execute commands from a file in the current shell.\n");
  printf("  prev            Execute the previous command.\n");
  printf("  help            Display this help message.\n");
  printf("  exit            Terminate the shell.\n\n");
}

// ============================== EXECUTE ==============================

// execute user input.
// returns 0 to prompt the program to exit.
// returns 1 to prompt the program to continue.
int execute(strarr_t *tokens) {
  if (tokens->size == 0) {
    return 1;
  }

  int exitStatus = 1;

  // ====== INPUT REDIRECTION ======
  for (int i = 0; i < tokens->size; i++) {
    // if we ever encounter "<", we should use the contents of the file
    // as args to tokens->data[0]
    if (strcmp(tokens->data[i], "<") == 0) {
      // file token is index of "<" plus one; if no file is given, error
      if (i == tokens->size - 1) {
        printf("Input redirection expectes a file.\n");
        return exitStatus;
      }
      char *fileToken = strarr_get_copy(tokens, i + 1);
      // delete everything in tokens except first argument
      unsigned int j = i + 1;
      while (j > 0) {
        strarr_remove_last(tokens);
        j--;
      }

      // // open the file for reading
      // int fd = open(fileToken, O_RDONLY);
      // if (fd == -1) {
      //   printf("Error trying to open %s\n", fileToken);
      //   return exitStatus;
      // }
      // char buffer[MAX_EXP_LEN + 1];
      // int length = read(fd, buffer, MAX_EXP_LEN);
      // if (length == -1) {
      //   printf("Error trying to open %s\n", fileToken);
      //   return exitStatus;
      // }
      // buffer[length] = '\0';
      // strarr_t *args = tokenize(buffer);

      // for (int k = 0; k < args->size; k++) {
      //   strarr_add(tokens, strarr_get_copy(args, k));
      // }
      // strarr_delete(args);

      // if (close(fd) == -1) {
      //   printf("Error trying to close %s\n", fileToken);
      //   return exitStatus;
      // }
      
      // for (int k = 0; k < tokens->size; k++) {
      //   printf("TOKEN: %s\n", tokens->data[k]);
      // }

      free(fileToken);
    }
  }




  // ========= EXIT =========
  if (strcmp(tokens->data[0], "exit") == 0) {
    printf("Bye bye.\n");
    exitStatus = 0;
  }
  
  // ========= CD =========
  else if (strcmp(tokens->data[0], "cd") == 0) {
    cd_command(tokens);
  }

  // ========= SOURCE =========
  else if (strcmp(tokens->data[0], "source") == 0) {
    exitStatus = source_command(tokens);
  }

  // ========= HELP =========
  else if (strcmp(tokens->data[0], "help") == 0) {
    help_command();
  }

  // ========= PROGRAM =========
  else {
    pid_t pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(1);
    }
    else if (pid == 0) {
      // child process (copy args to avoid unexpected behavior)
      char **args = (char **)malloc(sizeof(char *) * (tokens->size + 1));
      for (int i = 0; i < tokens->size; i++) {
        args[i] = strarr_get_copy(tokens, i);
      }
      args[tokens->size] = NULL;

      // launch program with exec
      if (execvp(args[0], args) == -1) {
        printf("%s: command not found\n", args[0]);

        // free memory for each string
        for (int i = 0; i < tokens->size; i++) {
            free(args[i]);
        }

        // free args
        free(args);
        strarr_delete(tokens);
        exit(1);
      }
    }
    else {
      // parent process
      wait(NULL);
    }
  }
  return exitStatus;
}


// =============================== MAIN ===============================

int main(int argc, char **argv) {
  // input buffer (initialized to the max expression length plus 1  
  // to leave room for the null terminator if the user decides to use
  // all 255 characters)
  char buffer[MAX_EXP_LEN + 1];
  
  int exitStatus = 1;

  // store the previous buffer to redo the previous command
  char prev_buffer[MAX_EXP_LEN + 1];
  // initialize the entire array to 0  
  memset(prev_buffer, 0, sizeof(prev_buffer));

  printf("Welcome to mini-shell.\n");

  while (1) {
    if (!exitStatus) {
      break;
    }
    
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
    
    // tokenize 
    strarr_t *tokens = tokenize(buffer);

    // if prev, utilize the prev_buffer, otherwise continue with
    // the current set of commands in tokens
    if (strcmp(tokens->data[0], "prev") == 0) {
      if (strlen(prev_buffer) == 0) {
        printf("No previous command.\n");
        continue;
      }
      strarr_delete(tokens);
      tokens = tokenize(prev_buffer);
    } else {
      strcpy(prev_buffer, buffer);
    }

    // split the line into sequenced commands and execute in order as long as
    // the exit status is 1 (i.e., exiting in the middle of the sequence 
    // should stop the program)
    strarr_t *command = strarr_new(tokens->capacity);
    unsigned int i = 0;
    while (i < tokens->size && exitStatus == 1) {
      if (strcmp(tokens->data[i], ";") == 0) {
        // execute the command
        exitStatus = execute(command);

        // reset the command
        strarr_delete(command);
        command = strarr_new(tokens->capacity);
      }
      else {
        strarr_add(command, tokens->data[i]);
      }
      i++;
    }

    // execute the final command in the sequence (if one exits and the 
    // status code is 1)
    if (exitStatus == 1) {
      exitStatus = execute(command);
    }

    // ------------ CLEANUP -------------

    // clear the command
    strarr_delete(command);

    // clear the tokens for the next iteration
    strarr_delete(tokens);

    // clear buffer for next iteration
    memset(buffer, 0, sizeof(buffer));
  }

  return 0;
}
