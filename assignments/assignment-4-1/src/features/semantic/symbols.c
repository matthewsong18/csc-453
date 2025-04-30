#include "symbols.h"
#include "../common/safe-memory.h"

#include <_string.h>

Symbol *allocate_symbol(void) {
  Symbol *symbol = safe_malloc(sizeof(Symbol));

  symbol->next = symbol;
  symbol->prev = symbol;

  return symbol;
}

Scope *allocate_scope(void) {
  Scope *scope = safe_malloc(sizeof(Scope));
  return scope;
}

SymbolTable *allocate_symbol_table(void) {
  SymbolTable *symbol_table = safe_malloc(sizeof(SymbolTable));
  symbol_table->symbols = allocate_symbol();
  symbol_table->global_scope = allocate_scope();
  symbol_table->current_scope = symbol_table->global_scope;

  return symbol_table;
}

SymbolTable *add_symbol(const char *name, enum SymbolType type,
                        SymbolTable *symbol_table) {
  Symbol *symbol = allocate_symbol();
  symbol->name = strdup(name);
  symbol->type = type;

  // Add a symbol to the end of a doubly linked list
  Symbol *tail_symbol = symbol_table->symbols->prev;
  tail_symbol->next = symbol;
  symbol->prev = tail_symbol;
  symbol->next = symbol_table->symbols;

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

void push_local_scope(SymbolTable *symbol_table) {
  symbol_table->current_scope = allocate_scope();
}

void pop_local_scope(SymbolTable *symbol_table) {
  // TODO
}

Scope *get_global_scope(const SymbolTable *symbol_table) {
  return symbol_table->global_scope;
}

Scope *get_current_scope(const SymbolTable *symbol_table) {
  return symbol_table->current_scope;
}