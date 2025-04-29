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

SymbolTable *add_symbol(char *name, enum SymbolType type,
                        SymbolTable *symbol_table) {
  Symbol *symbol = allocate_symbol();
  symbol->name = strdup(name);
  symbol->type = type;

  // Add symbol to end of doubly-linked list
  Symbol *tail_symbol = symbol_table->symbols->prev;
  tail_symbol->next = symbol;
  symbol->prev = tail_symbol;
  symbol->next = symbol_table->symbols;

  if (symbol_table->global_scope->head == NULL) {
    symbol_table->global_scope->head = symbol;
    symbol_table->global_scope->tail = symbol;
    symbol_table->current_scope = symbol_table->global_scope;
  } else {
    // Update symbol pointers
    symbol->prev = symbol_table->global_scope->tail;
    symbol->next = symbol_table->global_scope->head;
    // Update tail pointer
    symbol_table->global_scope->tail->next = symbol;
    symbol_table->global_scope->tail = symbol;
  }

  return symbol_table;
}