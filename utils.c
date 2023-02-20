#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Free an array of strings -- free each string (char*)
// and the string array itself (char**)
void free_str_array(char** arr) {
    int i = 0;
    while (arr[i] != NULL){
        free(arr[i]);
        i++;
    }
    free(arr);
}