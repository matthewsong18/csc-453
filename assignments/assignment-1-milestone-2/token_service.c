#include "./token_service.h"
#include "./scanner.h"
#include <stdio.h>
#include <stdlib.h>

// Global current token.
TokenI currentToken;

// Call scanner
TokenI getNextToken(void) {
  // For now, we return EOF immediately. Replace this with your scanner logic.
  TokenI t;
  t.type = TOKEN_EOF;
  t.lexeme = "";
  t.line = 1;
  return t;
}

// Advances to the next token.
void advanceToken(void) { currentToken = getNextToken(); }

// Checks that the current token matches the expected type; if so, advances.
// Otherwise, prints an error and returns false.
bool match(TokenType expected) {
  if (currentToken.type == expected) {
    advanceToken();
    return true;
  } else {
    fprintf(stderr, "ERROR: Expected token %d at line %d\n", expected,
            currentToken.line);
    return false;
  }
}
