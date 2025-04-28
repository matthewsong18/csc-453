#include "symbols.h"

#include <stdio.h>
#include <stdlib.h>

Symbol *allocate_symbol(void) {
  Symbol *symbol = malloc(sizeof(Symbol));
  if (symbol == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  // symbol->name = NULL;
  // symbol->type = SYM_NULL;

  return symbol;
}