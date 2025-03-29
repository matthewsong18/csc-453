#include "symbol_table.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Symbol *lookup_symbol_in_table(const char *name, const char *type) {
  const Scope *currentScopePtr = currentScope;
  while (currentScopePtr != NULL) {
    Symbol *symbol = lookup_symbol_in_scope(name, type, currentScopePtr);
    if (symbol != NULL) {
      return symbol;
    }
    currentScopePtr = currentScopePtr->parent;
  }
  return NULL;
}

Symbol *lookup_symbol_in_scope(const char *name, const char *type,
                               const Scope *scope) {
  Symbol *symbol = scope->symbols;
  while (symbol != NULL) {
    // Check matching name
    if (strcmp(symbol->name, name) != 0) {
      symbol = symbol->next;
      continue;
    }
    // Check matching type
    if (strcmp(symbol->type, type) != 0) {
      symbol = symbol->next;
      continue;
    }

    // Match found
    return symbol;
  }
  return NULL;
}

bool check_duplicate_symbol_in_scope(const char *name, const char *type,
                                     const Scope *scope) {
  assert(scope != NULL);
  assert(name != NULL);
  assert(type != NULL);

  Symbol *symbol = scope->symbols;
  while (symbol != NULL) {
    // Check matching name
    if (strcmp(symbol->name, name) != 0) {
      symbol = symbol->next;
      continue;
    }

    // Duplicate found
    return true;
  }
  return false;
}

Symbol *create_symbol(const char *name) {
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
  symbol->type = NULL;
  symbol->number_of_arguments = 0;
  symbol->arguments = NULL;
  symbol->next = NULL;

  return symbol;
}

// Add a new symbol to the given scope. Caller should have already checked
// for duplicates.
bool add_symbol(const char *name, const char *type) {
  Symbol *symbol = create_symbol(name);

  symbol->type = strdup(type);
  if (!symbol->type) {
    free(symbol->name);
    free(symbol);
    fprintf(stderr, "ERROR: memory allocation failure for symbol type\n");
    return false;
  }

  symbol->next = currentScope->symbols;
  currentScope->symbols = symbol;
  return true;
}

bool add_function_symbol(const char *name) {
  return add_symbol(name, "function");
}

bool add_variable_symbol(const char *name) {
  return add_symbol(name, "variable");
}

bool add_function_formal(const char *name) {
  if (currentScope->parent == NULL) {
    fprintf(stderr, "the current scope has no parents");
    exit(1);
  }

  Scope *parentScope = currentScope->parent;
  if (!parentScope) {
    fprintf(stderr, "mising parent scope");
    exit(1);
  }

  Symbol *function = parentScope->symbols;
  if (!function) {
    fprintf(stderr, "the global scope had no symbols");
    exit(1);
  }

  Symbol *argument_ptr = create_symbol(name);
  if (function->arguments == NULL) {
    function->arguments = argument_ptr;
    function->number_of_arguments = 1;
    return true;
  }

  Symbol *last_ptr = function->arguments;
  while (last_ptr->next != NULL) {
    last_ptr = last_ptr->next;
  }
  last_ptr->next = argument_ptr;
  function->number_of_arguments = function->number_of_arguments + 1;

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

// Pop the current scope off the scope stack, freeing its symbols.
void popScope(void) {
  if (currentScope == NULL) {
    fprintf(stderr, "ERROR: no scope to pop\n");
    return;
  }

  // Scope *temp = currentScope;
  currentScope = currentScope->parent;

  // Decided to just leak symbols + names for the ast and quad

  // Symbol *symbol = temp->symbols;
  // Symbol *next_symbol = NULL;

  // while (symbol != NULL) {
  //   next_symbol = symbol->next;
  //   free(symbol->name);
  //   free(symbol);
  //   symbol = next_symbol;
  // }
}

void initSymbolTable(void) {
  globalScope = malloc(sizeof(Scope));
  if (!globalScope) {
    fprintf(stderr, "ERROR: memory allocation failure in initSymbolTable\n");
    exit(1);
  }
  globalScope->symbols = NULL;
  globalScope->parent = NULL;
  currentScope = globalScope;
  Symbol *println_symbol = create_symbol("println");
  println_symbol->number_of_arguments = 1;
  println_symbol->type = strdup("function");
  globalScope->symbols = println_symbol;
}

void free_symbol(Symbol *symbol) {

  if (symbol == NULL) {
    return;
  }

  free(symbol->name);
  free(symbol->type);
  free_symbol(symbol->arguments);
  free_symbol(symbol->next);

  symbol->name = NULL;
  symbol->type = NULL;
  symbol->value = 0;
  symbol->number_of_arguments = 0;
  symbol->arguments = NULL;
  symbol->next = NULL;
}

void free_scope(Scope *scope) {
  if (scope == NULL) {
    return;
  }

  free_symbol(scope->symbols);
  free_scope(scope->parent);

  scope->symbols = NULL;
  scope->parent = NULL;
}

void free_symbol_table(void) {
  if (currentScope == NULL) {
    return;
  }

  free_scope(currentScope);

  currentScope = NULL;
  globalScope = NULL;
}
