#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// A symbol represents an identifier along with its type (for our purposes, a string).
typedef struct Symbol {
    char *name;
    char *type;
    struct Symbol *next;
} Symbol;

// A scope is a linked list of symbols, with a pointer to its parent scope.
typedef struct Scope {
    Symbol *symbols;
    struct Scope *parent;
} Scope;

extern Scope *globalScope;
extern Scope *currentScope;

Symbol *lookupSymbol(const char *name, Scope *scope);
bool addSymbol(const char *name, Scope *scope, const char *type);
void pushScope(void);
void popScope(void);

#endif

