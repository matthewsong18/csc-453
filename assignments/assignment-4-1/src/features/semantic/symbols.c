#include "symbols.h"
#include "../common/safe-memory.h"

#include <_string.h>

Symbol *allocate_symbol(void) {
  Symbol *symbol = safe_malloc(sizeof(Symbol));

  // symbol->name = NULL;
  // symbol->type = SYM_NULL;

  return symbol;
}

Scope *allocate_scope(void) {
  Scope *scope = safe_malloc(sizeof(Scope));
  return scope;
}

SymbolTable *allocate_symbol_table(void) {
  SymbolTable *symbol_table = safe_malloc(sizeof(SymbolTable));
  symbol_table->global_scope = allocate_scope();
  symbol_table->current_scope = symbol_table->global_scope;

  return symbol_table;
}

SymbolTable *add_symbol(char *name, enum SymbolType type, SymbolTable *symbol_table) {
  Symbol *symbol = allocate_symbol();
  symbol->name = strdup(name);
  symbol->type = type;

  symbol_table->symbols = symbol;
  symbol_table->global_scope->head = symbol;
  symbol_table->global_scope->tail = symbol;
  symbol_table->current_scope = symbol_table->global_scope;

  return symbol_table;
}