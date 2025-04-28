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

SymbolTable *allocate_symbol_table(void) {
  SymbolTable *symbol_table = malloc(sizeof(SymbolTable));
  if (symbol_table == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  return symbol_table;
}