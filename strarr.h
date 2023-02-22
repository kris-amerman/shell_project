#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// String array 
typedef struct strarr {
  char **data;
  unsigned int size;
  unsigned int capacity;
} strarr_t;

/** Create a new empty string array with the given capacity */
strarr_t *strarr_new(unsigned int cap) {
  strarr_t *pa = (strarr_t *) malloc(sizeof(strarr_t));
  assert(pa != NULL);
  pa->size = 0;
  pa->capacity = cap;
  pa->data = (char **) malloc(cap * sizeof(char *));
  assert(pa->data != NULL);

  return pa;
}

/** Delete the string array and free its contents (including itself) */
void strarr_delete(strarr_t *pa) {
  if (pa == NULL) {
    return;
  }
  // First, we'll free the memory for each item allocated in data
  unsigned int i;
  for (i = 0; i < pa->size; i++) {
    free(pa->data[i]);
  }
  // Then, we'll free data itself
  free(pa->data);
  // Then, we'll free the vect struct
  free(pa);
}

/** Retrieve the element at the given index */
const char *strarr_get(strarr_t *pa, unsigned int idx) {
  assert(pa != NULL);
  assert(idx < pa->size);
  // To get the element at the given index, we simply access
  // that element of the data array (which should be a char *)
  return pa->data[idx];
}

/** Get a copy of the element at the given index. The caller is responsible
 *  for freeing the memory occupied by the copy. */
char *strarr_get_copy(strarr_t *pa, unsigned int idx) {
  assert(pa != NULL);
  assert(idx < pa->size);
  // In this case, we can copy the string and return a pointer to the copy
  char *pcopy = malloc(strlen(pa->data[idx]) + 1);
  strcpy(pcopy, pa->data[idx]);
  return pcopy;
}

/** Set the element at the given index. */
void strarr_set(strarr_t *pa, unsigned int idx, const char *elt) {
  assert(pa != NULL);
  assert(idx < pa->size);
  // To set an element, we simply reassign that element in data
  // to a copy of the given element

  // First, we'll free the existing memory location
  free(pa->data[idx]);
  // Then, we'll reallocate the memory location 
  pa->data[idx] = malloc(strlen(elt) + 1);
  // Then update the allocated memory with a copy of elt
  strcpy(pa->data[idx], elt);
}

/** Add an element to the back of the vector. */
void strarr_add(strarr_t *pa, const char *elt) {
  assert(pa != NULL);
  assert(pa->size != pa->capacity); // make sure we're not at capacity
 
  // Make a copy of elt and assign it to the end of the vector
  pa->data[pa->size] = malloc(strlen(elt) + 1);
  strcpy(pa->data[pa->size], elt);
  // Increment the size
  pa->size++;
}

/** Create a copy of the given string array. The caller is responsible
 *  for freeing the memory occupied by the copy. */
strarr_t *strarr_copy(strarr_t *src) {
  assert(src != NULL);
  strarr_t *copy = strarr_new(src->capacity);
  for (unsigned int i = 0; i < src->size; i++) {
    char *elt_copy = strarr_get_copy(src, i);
    strarr_add(copy, elt_copy);
    free(elt_copy);
  }
  return copy;
}

/** Remove the last element from the vector. */
void strarr_remove_last(strarr_t *pa) {
  assert(pa != NULL);
  // If it's empty, just return
  if (pa->size == 0) {
    return;
  }

  // To remove the last element in the vector, we can free up 
  // the memory space, NULL the memory space 
  // and decrement the size
  pa->size--;
  free(pa->data[pa->size]);
  pa->data[pa->size] = NULL;
}