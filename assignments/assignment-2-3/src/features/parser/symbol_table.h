#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// A symbol represents an identifier along with its type (for our purposes, a string).
typedef struct Symbol {
    char *name;
    char *type;
    int number_of_arguments;
    struct Symbol *arguments;
    struct Symbol *next;
} Symbol;

// A scope is a linked list of symbols, with a pointer to its parent scope.
typedef struct Scope {
    Symbol *symbols;
    struct Scope *parent;
} Scope;

extern Scope *globalScope;
extern Scope *currentScope;

Symbol *lookup_symbol_in_scope(const char *name, const char *type, const Scope *scope);
Symbol *lookup_symbol_in_table(const char *name, const char *type);
bool add_symbol(const char *name, const char *type);
bool add_function_symbol(const char *name);
bool add_variable_symbol(const char *name);
bool add_function_formal(const char *name);
void pushScope(void);
void popScope(void);
void initSymbolTable(void);

#endif

