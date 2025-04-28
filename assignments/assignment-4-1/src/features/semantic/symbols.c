#include "symbols.h"
#include "../common/safe-memory.h"

Symbol *allocate_symbol(void) {
  Symbol *symbol = safe_malloc(sizeof(Symbol));

  // symbol->name = NULL;
  // symbol->type = SYM_NULL;

  return symbol;
}

SymbolTable *allocate_symbol_table(void) {
  SymbolTable *symbol_table = safe_malloc(sizeof(SymbolTable));

  return symbol_table;
}