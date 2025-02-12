#include "./token_service.h"
#include "./scanner.h"
#include <stdio.h>
#include <stdlib.h>

// Global current token.
TokenI currentToken;

// Call scanner
TokenI getNextToken(void) {
  int tok;
  TokenI token;
  if ((tok = get_token()) == EOF) {
    token.type = TOKEN_EOF;
  }

  if (tok == ID)
    token.type = TOKEN_ID;
  else if (tok == kwINT)
    token.type = TOKEN_KWINT;
  else if (tok == LPAREN)
    token.type = TOKEN_LPAREN;
  else if (tok == RPAREN)
    token.type = TOKEN_RPAREN;
  else if (tok == LBRACE)
    token.type = TOKEN_LBRACE;
  else if (tok == RBRACE)
    token.type = TOKEN_RBRACE;
  else if (tok == SEMI)
    token.type = TOKEN_SEMI;
  else if (tok == UNDEF)
    token.type = TOKEN_UNDEF;
  else {
    perror("ERROR: Invalid token");
  }

  token.line = currentLine;
  return token;
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
