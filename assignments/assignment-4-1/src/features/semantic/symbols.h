#ifndef SYMBOLS_H
#define SYMBOLS_H

enum SymbolType {
  SYM_NULL,
};

typedef struct {
  char *name;
  enum SymbolType type;
} Symbol;

Symbol *allocate_symbol(void);

#endif //SYMBOLS_H
