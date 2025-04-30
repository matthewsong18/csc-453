#ifndef SYMBOLS_H
#define SYMBOLS_H

enum SymbolType {
  SYM_NULL,
  SYM_VARIABLE,
  SYM_FUNCTION,
};

typedef struct Symbol {
  char *name;
  enum SymbolType type;
  struct Symbol *next;
  struct Symbol *prev;
} Symbol;

typedef struct Scope {
  Symbol *head;
  Symbol *tail;
  struct Scope *next;
  struct Scope *prev;
} Scope;

typedef struct {
  Symbol *symbols;
  Scope *scopes;
  Scope *global_scope;
  Scope *current_scope;
} SymbolTable;

// Allocators
Symbol *allocate_symbol(void);
SymbolTable *allocate_symbol_table(void);

// Adds
SymbolTable *add_symbol(const char *name, enum SymbolType type,
                        SymbolTable *symbol_table);
SymbolTable *add_formal(const char *formal_name, SymbolTable *symbol_table);

// Scopes
void push_local_scope(SymbolTable *symbol_table);
void pop_local_scope(SymbolTable *symbol_table);

// Getters
Scope *get_global_scope(const SymbolTable *symbol_table);
Scope *get_current_scope(const SymbolTable *symbol_table);

// Free memory
void free_symbol_table(SymbolTable *symbol_table);

#endif // SYMBOLS_H
