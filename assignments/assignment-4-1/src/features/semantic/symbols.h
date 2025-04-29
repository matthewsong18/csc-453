#ifndef SYMBOLS_H
#define SYMBOLS_H

enum SymbolType {
  SYM_NULL,
  SYM_VARIABLE,
};

typedef struct Symbol {
  char *name;
  enum SymbolType type;
  struct Symbol *next;
  struct Symbol *prev;
} Symbol;

typedef struct {
  Symbol *head;
  Symbol *tail;
} Scope;

typedef struct {
  Symbol *symbols;
  Scope *global_scope;
  Scope *current_scope;
} SymbolTable;

Symbol *allocate_symbol(void);
SymbolTable *allocate_symbol_table(void);
SymbolTable *add_symbol(char *name, enum SymbolType type, SymbolTable *symbol_table);

#endif //SYMBOLS_H
