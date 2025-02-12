#ifndef TOKEN_SERVICE_H
#define TOKEN_SERVICE_H

#include <stdbool.h>

typedef enum {
  TOKEN_ID,
  TOKEN_KWINT,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_SEMI,
  TOKEN_EOF,
  TOKEN_UNDEF
} TokenType;

typedef struct {
  TokenType type;
  const char *lexeme; // For simplicity, not used in this skeleton.
  int line;           // Line number (for error reporting)
} TokenI;

// Scanner function declarations.
TokenI getNextToken(void);
void advanceToken(void);
bool match(TokenType expected);

// Global current token.
extern TokenI currentToken;

#endif
