#include "symbols.h"
#include "../common/safe-memory.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Symbol *allocate_symbol(void) {
  Symbol *symbol = safe_malloc(sizeof(Symbol));

  symbol->name = NULL;
  symbol->type = SYM_NULL;
  symbol->num_of_formals = 0;
  symbol->formals = NULL;
  symbol->next = symbol;
  symbol->prev = symbol;
  symbol->free_prev = symbol;
  symbol->free_next = symbol;

  return symbol;
}

Scope *allocate_scope(void) {
  Scope *scope = safe_malloc(sizeof(Scope));

  scope->parent = NULL;
  scope->next = scope;
  scope->prev = scope;

  return scope;
}

SymbolTable *allocate_symbol_table(void) {
  SymbolTable *symbol_table = safe_malloc(sizeof(SymbolTable));
  symbol_table->symbols = allocate_symbol();
  Scope *scope = allocate_scope();
  symbol_table->global_scope = scope;
  symbol_table->current_scope = scope;

  symbol_table->scopes = allocate_scope();
  symbol_table->scopes->next = scope;

  // Adding println
  symbol_table = add_symbol("println", SYM_FUNCTION, symbol_table);

  return symbol_table;
}

SymbolTable *add_symbol(const char *name, const enum SymbolType type,
                        SymbolTable *symbol_table) {
  if (is_symbol_in_scope(get_current_scope(symbol_table), name)) {
    fprintf(stderr, "ERROR: symbol %s already defined in current scope\n",
            name);
    exit(EXIT_FAILURE);
  }

  Symbol *symbol = allocate_symbol();
  symbol->name = strdup(name);
  symbol->type = type;

  // Add a symbol to the end of a doubly linked list
  Symbol *tail_symbol = symbol_table->symbols->free_prev;
  symbol_table->symbols->free_prev = symbol;

  symbol->free_prev = tail_symbol;
  symbol->free_next = tail_symbol->free_next;
  tail_symbol->free_next = symbol;

  if (symbol_table->current_scope->head == NULL) {
    symbol_table->current_scope->head = symbol;
    symbol_table->current_scope->tail = symbol;
  } else {
    // Update symbol pointers
    symbol->prev = symbol_table->current_scope->tail;
    symbol->next = symbol_table->current_scope->head;
    // Update tail pointer
    symbol_table->current_scope->tail->next = symbol;
    symbol_table->current_scope->tail = symbol;
  }

  return symbol_table;
}

SymbolTable *add_formal(const char *formal_name, SymbolTable *symbol_table) {
  Symbol *symbol = allocate_symbol();
  symbol->name = strdup(formal_name);
  symbol->type = SYM_VARIABLE;

  // Add to the symbol list
  Symbol *tail_symbol = symbol_table->symbols->free_prev;
  symbol_table->symbols->free_prev = symbol;

  symbol->free_prev = tail_symbol;
  symbol->free_next = tail_symbol->free_next;
  tail_symbol->free_next = symbol;

  // Add to the function symbol
  Symbol *function_symbol = symbol_table->global_scope->tail;
  if (function_symbol == NULL) {
    // TODO
    fprintf(stderr, "ERROR: no function symbol found\n");
    exit(EXIT_FAILURE);
  }

  if (function_symbol->formals == NULL) {
    function_symbol->formals = symbol;
    function_symbol->num_of_formals = 1;
    return symbol_table;
  }

  // Append symbol to tail
  symbol->prev = function_symbol->formals->prev;
  symbol->next = function_symbol->formals;
  function_symbol->formals->prev->next = symbol;
  function_symbol->formals->prev = symbol;
  function_symbol->num_of_formals++;

  return symbol_table;
}

void push_local_scope(SymbolTable *symbol_table) {
  Scope *new_scope = allocate_scope();

  new_scope->parent = symbol_table->current_scope;
  symbol_table->current_scope = new_scope;

  Scope *last_scope = symbol_table->scopes->prev;

  // Insert new_scope into the list
  last_scope->next = new_scope;
  new_scope->prev = last_scope;
  new_scope->next = symbol_table->scopes;

  symbol_table->scopes->prev = new_scope;
}

void pop_local_scope(SymbolTable *symbol_table) {
  symbol_table->current_scope = symbol_table->current_scope->parent;
}

int is_symbol_in_scope(const Scope *scope, const char *symbol_name) {
  const Symbol *head = scope->head;
  const Symbol *symbol = head;
  int first = 0;
  while (symbol != NULL && (!first || symbol != head)) {
    if (strcmp(symbol->name, symbol_name) == 0) {
      return 1;
    }
    symbol = symbol->next;
    first = 1;
  }
  return 0;
}

Scope *get_global_scope(const SymbolTable *symbol_table) {
  return symbol_table->global_scope;
}

Scope *get_current_scope(const SymbolTable *symbol_table) {
  return symbol_table->current_scope;
}
Symbol *find_formal(const Symbol *function_symbol, const char *formal_name) {
  if (function_symbol->num_of_formals == 0) {
    fprintf(stderr, "ERROR: no formals found\n");
    exit(EXIT_FAILURE);
  }
  Symbol *formal = function_symbol->formals;
  for (int i = 0; i < function_symbol->num_of_formals; i++) {
    if (strcmp(formal->name, formal_name) == 0) {
      return formal;
    }
    formal = formal->next;
  }
  fprintf(stderr, "ERROR: formal not found\n");
  exit(EXIT_FAILURE);
}

void free_symbol_table(SymbolTable *symbol_table) {
  const Symbol *placeholder_symbol = symbol_table->symbols;
  Symbol *symbol = placeholder_symbol->free_next;
  while (symbol != placeholder_symbol) {
    Symbol *next_symbol = symbol->free_next;

    if (symbol->name != NULL) {
      free(symbol->name);
    }

    symbol->type = SYM_NULL;
    symbol->next = NULL;
    symbol->prev = NULL;
    symbol->free_prev = NULL;
    symbol->free_next = NULL;

    free(symbol);
    symbol = next_symbol;
  }

  const Scope *placeholder_scope = symbol_table->scopes;
  Scope *scope = placeholder_scope->next;
  while (scope != placeholder_scope) {
    Scope *next_scope = scope->next;
    free(scope);
    scope = next_scope;
  }

  symbol_table->symbols = NULL;
  symbol_table->scopes = NULL;
  symbol_table->global_scope = NULL;
  symbol_table->current_scope = NULL;
}