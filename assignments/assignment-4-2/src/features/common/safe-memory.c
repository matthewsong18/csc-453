#include "safe-memory.h"

#include <stdio.h>
#include <stdlib.h>

void *safe_malloc(const int size) {
  void *ptr = malloc(size);

  if (ptr == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  return ptr;
}