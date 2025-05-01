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
  int num_of_formals;
  struct Symbol *formals;
  struct Symbol *next;
  struct Symbol *prev;
  struct Symbol *free_next;
  struct Symbol *free_prev;
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

// Finders
int is_symbol_in_scope(const Scope *scope, const char* symbol_name, const SymbolTable *symbol_table);

// Getters
Scope *get_global_scope(const SymbolTable *symbol_table);
Scope *get_current_scope(const SymbolTable *symbol_table);
Symbol *find_formal(const Symbol *function_symbol, const char *formal_name);

// Free memory
void free_symbol_table(SymbolTable *symbol_table);

#endif // SYMBOLS_H
