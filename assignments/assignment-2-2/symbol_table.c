#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Symbol *lookupSymbol(const char *name, Scope *scope) {
  Symbol *symbol = scope->symbols;
  while (symbol != NULL) {
    if (strcmp(symbol->name, name) == 0)
      return symbol;
    symbol = symbol->next;
  }
  return NULL;
}

// Add a new symbolbol to the given scope. Caller should have already checked
// for duplicates.
bool addSymbol(const char *name, Scope *scope, const char *type) {
  Symbol *symbol = malloc(sizeof(Symbol));
  if (!symbol) {
    fprintf(stderr, "ERROR: memory allocation failure\n");
    return false;
  }
  symbol->name = strdup(name);
  if (!symbol->name) {
    free(symbol);
    fprintf(stderr, "ERROR: memory allocation failure for symbolbol name\n");
    return false;
  }
  symbol->type = strdup(type);
  if (!symbol->type) {
    free(symbol->name);
    free(symbol);
    fprintf(stderr, "ERROR: memory allocation failure for symbolbol type\n");
    return false;
  }
  symbol->next = scope->symbols;
  scope->symbols = symbol;
  return true;
}

// Push a new scope onto the scope stack.
void pushScope(void) {
  Scope *newScope = malloc(sizeof(Scope));
  if (!newScope) {
    fprintf(stderr, "ERROR: memory allocation failure in pushScope\n");
    exit(1);
  }
  newScope->symbols = NULL;
  newScope->parent = currentScope;
  currentScope = newScope;
}

// Pop the current scope off the scope stack, freeing its symbolbols.
void popScope(void) {
  if (currentScope == NULL) {
    fprintf(stderr, "ERROR: no scope to pop\n");
    exit(1);
  }
  Scope *temp = currentScope;
  currentScope = currentScope->parent;
  Symbol *symbol = temp->symbols;
  while (symbol != NULL) {
    Symbol *next = symbol->next;
    free(symbol->name);
    free(symbol->type);
    free(symbol);
    symbol = next;
  }
  free(temp);
}
