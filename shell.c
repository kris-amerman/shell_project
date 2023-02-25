#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>

#include "parse.h" 

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// ============================= CONSTANTS =============================

const int MAX_EXP_LEN = 255;


// ============================= PROTOTYPES ============================

int execute(strarr_t *tokens);


// ============================== HELPERS ==============================

// Changes the directory 
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

// open a file and execute the given commands
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

// print out built-in commands 
void help_command() {
  printf("\n*** Shell Built-in Commands ***\n\n");
  printf("  cd [directory]  Change the current working directory.\n");
  printf("  source [file]   Execute commands from a file in the current shell.\n");
  printf("  prev            Execute the previous command.\n");
  printf("  help            Display this help message.\n");
  printf("  exit            Terminate the shell.\n\n");
}


// ============================== EXECUTE ==============================

// handle a system call command
int execute_program(strarr_t *tokens) {
  if (tokens->size == 0) {
    return 1;
  }

  int exitStatus = 1;

  // ========= PROGRAM =========
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(1);
  }
  else if (pid == 0) {
    // child process 

    // check if > symbol exists in the arguments
    int fd;
    int index = strarr_index_of(tokens, ">");
    if (index != -1 && index < tokens->size - 1) {
      // open file for writing and truncate if it already exists
      fd = open(tokens->data[index + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
      if (fd == -1) {
          perror("open");
          exit(1);
      }

      // redirect stdout to file
      if (dup2(fd, STDOUT_FILENO) == -1) {
          perror("dup2");
          exit(1);
      }

      // remove arguments after > (including >)
      int numDelete = tokens->size - index;
      for (int k = 0; k < numDelete; k++) {
        strarr_remove_last(tokens);
      }
    }

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

  return exitStatus;
}


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

       // open the file for reading
       int fd = open(fileToken, O_RDONLY);
       if (fd == -1) {
         printf("Error trying to open %s\n", fileToken);
         return exitStatus;
       }
       char buffer[MAX_EXP_LEN + 1];
       int length = read(fd, buffer, MAX_EXP_LEN);
       if (length == -1) {
         printf("Error trying to open %s\n", fileToken);
         return exitStatus;
       }
       buffer[length] = '\0';
       strarr_t *args = tokenize(buffer);

       for (int k = 0; k < args->size; k++) {
         strarr_add(tokens, args->data[k]);
       }
       strarr_delete(args);

       if (close(fd) == -1) {
         printf("Error trying to close %s\n", fileToken);
         return exitStatus;
       }

      free(fileToken);
    }
  }

  // ========= EXIT =========
  if (strcmp(tokens->data[0], "exit") == 0) {
    printf("Bye bye.\n");
    return 0;
  }
  
  // ========= CD =========
  else if (strcmp(tokens->data[0], "cd") == 0) {
    cd_command(tokens);
    return 1;
  }

  // ========= SOURCE =========
  else if (strcmp(tokens->data[0], "source") == 0) {
    return source_command(tokens);
  }

  // ========= HELP =========
  else if (strcmp(tokens->data[0], "help") == 0) {
    help_command();
    return 1;
  }

  // ======== HANDLE PIPES ========
  else {
    int numPipes = 0;
    for (int i = 0; i < tokens->size; i++) {
      if (strcmp(tokens->data[i], "|") == 0) {
        numPipes++;
      }
    }
    
    if (numPipes > 0) {
      // allocate space for pipe file descriptors
      int pipefds[2 * numPipes];

      // initialize all pipe file descriptors
      for (int i = 0; i < numPipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
          perror("pipe");
          exit(1);
        }
      }
      
      // launch child processes for each command
      for (int i = 0; i <= numPipes; i++) {
        pid_t pid = fork();
        if (pid == -1) {
          perror("fork");
          exit(1);
        }
        else if (pid == 0) {
          // child process
          
          // determine the arguments for the current command
          int start = i;
          strarr_t *command = strarr_new(tokens->capacity);
          // create a subarray of tokens for the current command if this isn't
          // the last command
          if (i < numPipes) {
            for (int j = i; j < tokens->size; j++) {
              if (strcmp(tokens->data[j], "|") == 0) {
                break;
              }
              else {
                strarr_add(command, tokens->data[j]);
              }
            }
          }
          printf("COMMAND: %s\n", command->data[0]);

          // redirect input and output as necessary
          if (i > 0) {
            // redirect input to read end of previous pipe
            if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0) {
              perror("dup2");
              exit(1);
            }
          }
          if (i < numPipes) {
            // redirect output to write end of current pipe
            if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0) {
              perror("dup2");
              exit(1);
            }
          }

          // close all pipe file descriptors in the child process
          for (int j = 0; j < 2 * numPipes; j++) {
            close(pipefds[j]);
          }

          // execute the command
          exitStatus = execute_program(command);
          strarr_delete(command);
          exit(0);
        }
      }

      // close all pipe file descriptors in the parent process
      for (int i = 0; i < 2 * numPipes; i++) {
        close(pipefds[i]);
      }

      // wait for all child processes to finish
      for (int i = 0; i <= numPipes; i++) {
        wait(NULL);
      }
    }
    // no pipes, just execute the single command
    else {
      exitStatus = execute_program(tokens);
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
    while ((c = getchar()) != '\n' && c != EOF){}

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
    } 
    else {
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
