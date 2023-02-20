/**
 * Vector implementation.
 *
 * - Implement each of the functions to create a working growable array (vector).
 * - Do not change any of the structs
 * - When submitting, You should not have any 'printf' statements in your vector 
 *   functions.
 *
 * IMPORTANT: The initial capacity and the vector's growth factor should be 
 * expressed in terms of the configuration constants in vect.h
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vect.h"

/** Main data structure for the vector. */
struct vect {
  char **data;             /* Array containing the actual data. */
  unsigned int size;       /* Number of items currently in the vector. */
  unsigned int capacity;   /* Maximum number of items the vector can hold before growing. */
};

/** Construct a new empty vector. */
vect_t *vect_new() {
  // First, we have to allocate some memory for the vect struct:
  vect_t *pv = (vect_t *) malloc(sizeof(vect_t));
  // Check if memory allocation worked
  if (pv == NULL) {
    return NULL;
  }
  
  // Next, we have to set the initial capacity and size
  pv->capacity = VECT_INITIAL_CAPACITY;
  pv->size = 0;

  // Then, we have to allocate some memory for the data stored in vect:
  pv->data = (char **) malloc(VECT_INITIAL_CAPACITY * sizeof(char *));
  // Check if memory allocation worked
  if (pv->data == NULL){
    return NULL;
  }

  return pv;
}

/** Delete the vector, freeing all memory it occupies. */
void vect_delete(vect_t *pv) {
  if (pv == NULL) {
    return;
  }
  // First, we'll free the memory for each item allocated in data
  unsigned int i;
  for (i = 0; i < pv->size; i++) {
    free(pv->data[i]);
  }
  // Then, we'll free data itself
  free(pv->data);
  // Then, we'll free the vect struct
  free(pv);
  
}

/** Get the element at the given index. */
const char *vect_get(vect_t *pv, unsigned int idx) {
  assert(pv != NULL);
  assert(idx < pv->size);
  // To get the element at the given index, we simply access
  // that element of the data array (which should be a char *)
  return pv->data[idx];
}

/** Get a copy of the element at the given index. The caller is responsible
 *  for freeing the memory occupied by the copy. */
char *vect_get_copy(vect_t *pv, unsigned int idx) {
  assert(pv != NULL);
  assert(idx < pv->size);
  // In this case, we can copy the string and return a pointer to the copy
  char *pcopy = malloc(strlen(pv->data[idx]) +1);
  strcpy(pcopy, pv->data[idx]);
  return pcopy;
}

/** Set the element at the given index. */
void vect_set(vect_t *pv, unsigned int idx, const char *elt) {
  assert(pv != NULL);
  assert(idx < pv->size);
  // To set an element, we simply reassign that element in data
  // to a copy of the given element

  // First, we'll free the existing memory location
  free(pv->data[idx]);
  // Then, we'll reallocate the memory location 
  pv->data[idx] = malloc(strlen(elt) + 1);
  // Then update the allocated memory with a copy of elt
  strcpy(pv->data[idx], elt);
}

/** Add an element to the back of the vector. */
void vect_add(vect_t *pv, const char *elt) {
  assert(pv != NULL);
  // To add, we must check if we're at capacity:
  if (pv->size == pv->capacity) {
    // If we're at capacity, allocate some more memory according to the
    // growth factor 
    pv->capacity = pv->capacity * VECT_GROWTH_FACTOR;
    pv->data = (char **) realloc(pv->data, pv->capacity * sizeof(char *));
  }
  // Make a copy of elt and assign it to the end of the vector
  pv->data[pv->size] = malloc(strlen(elt) + 1);
  strcpy(pv->data[pv->size], elt);
  // Increment the size
  pv->size++;
}

/** Remove the last element from the vector. */
void vect_remove_last(vect_t *pv) {
  assert(pv != NULL);
  // If it's empty, just return
  if (pv->size == 0) {
    return;
  }

  // To remove the last element in the vector, we can free up 
  // the memory space,  NULL the memory space 
  // and decrement the size
  pv->size--;
  free(pv->data[pv->size]);
  pv->data[pv->size] = NULL;
}

/** The number of items currently in the vector. */
unsigned int vect_size(vect_t *pv) {
  assert(pv != NULL);
  // Just check the size of the vect struct:
  return pv->size;
}

/** The maximum number of items the vector can hold before it has to grow. */
unsigned int vect_current_capacity(vect_t *pv) {
  assert(pv != NULL);
  // Just check the capacity of the vect struct:
  return pv->capacity;
}

