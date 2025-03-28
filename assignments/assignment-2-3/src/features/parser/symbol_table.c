#include "symbol_table.h"
#include "grammar_rule.h"
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
    if (strcmp(symbol->name, name) != 0) {
      symbol = symbol->next;
      continue;
    }
    if (strcmp(symbol->type, type) != 0) {
      fprintf(stderr, "ERROR: symbol already declared");
      exit(1);
    }
    return symbol;
  }
  return NULL;
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

bool add_function_formal(const char *name, const GrammarRule *rule) {
  if (currentScope->parent == NULL) {
    report_error(rule->name, "the current scope has no parents");
    return false;
  }

  Symbol *function = currentScope->parent->symbols;
  if (!function) {
    report_error(rule->name, "the global scope had no symbols");
    return false;
  }

  Symbol *argument_ptr = create_symbol(name);
  if (argument_ptr == NULL) {
    function->arguments = argument_ptr;
    function->number_of_arguments = 1;
    if (function->arguments == NULL) {
      report_error(rule->name, "failed to add initial formal");
      return false;
    }
    return true;
  }
  argument_ptr = create_symbol(name);
  argument_ptr->next = function->arguments;
  function->arguments = argument_ptr;
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

void initSymbolTable(void) {
  globalScope = malloc(sizeof(Scope));
  if (!globalScope) {
    fprintf(stderr, "ERROR: memory allocation failure in initSymbolTable\n");
    exit(1);
  }
  globalScope->symbols = NULL;
  globalScope->parent = NULL;
  currentScope = globalScope;
}
