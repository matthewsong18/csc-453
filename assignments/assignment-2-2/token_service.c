#include "./token_service.h"
#include "./scanner.h"
#include <stdio.h>
#include <stdlib.h>

// Global current token.
TokenI currentToken;

// Call scanner
TokenI getNextToken(void) {
  int tok = get_token();
  TokenI token;
  token.line = currentLine;

  switch (tok) {
  case EOF:
    token.type = TOKEN_EOF;
    break;
  case ID:
    token.type = TOKEN_ID;
    break;
  case kwINT:
    token.type = TOKEN_KWINT;
    break;
  case LPAREN:
    token.type = TOKEN_LPAREN;
    break;
  case RPAREN:
    token.type = TOKEN_RPAREN;
    break;
  case LBRACE:
    token.type = TOKEN_LBRACE;
    break;
  case RBRACE:
    token.type = TOKEN_RBRACE;
    break;
  case SEMI:
    token.type = TOKEN_SEMI;
    break;
  case UNDEF:
    token.type = TOKEN_UNDEF;
    break;
  case COMMA:
    token.type = TOKEN_COMMA;
    break;
  case kwWHILE:
    token.type = TOKEN_KWWHILE;
  case kwIF:
    token.type = TOKEN_KWIF;
  case kwRETURN:
    token.type = TOKEN_KWRETURN;
  case opASSG:
    token.type = TOKEN_OPASSG;
  case INTCON:
    token.type = TOKEN_INTCON;
  case opEQ:
    token.type = TOKEN_OPEQ;
  case opNE:
    token.type = TOKEN_OPNE;
  case opLE:
    token.type = TOKEN_OPLE;
  case opLT:
    token.type = TOKEN_OPLT;
  case opGE:
    token.type = TOKEN_OPGE;
  case opGT:
    token.type = TOKEN_OPGT;

  default:
    token.type = TOKEN_UNDEF;
    break;
  }
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
    fprintf(stderr, "ERROR: LINE %d: unexpected token\n", currentToken.line);
    return false;
  }
}
