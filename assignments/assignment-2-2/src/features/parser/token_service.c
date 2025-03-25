#include "./token_service.h"
#include "../scanner/scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *lexeme;
extern bool DEBUG_ON;
// Global current token.
TokenI currentToken;

static TokenI peekedToken;
static bool hasPeekedToken = false;

// Call scanner
TokenI getNextToken(void) {
  int tok = get_token();
  TokenI token;
  token.line = currentLine;
  token.lexeme = strdup(lexeme);

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
    break;
  case kwIF:
    token.type = TOKEN_KWIF;
    break;
  case kwELSE:
    token.type = TOKEN_KWELSE;
    break;
  case kwRETURN:
    token.type = TOKEN_KWRETURN;
    break;
  case opASSG:
    token.type = TOKEN_OPASSG;
    break;
  case INTCON:
    token.type = TOKEN_INTCON;
    break;
  case opEQ:
    token.type = TOKEN_OPEQ;
    break;
  case opNE:
    token.type = TOKEN_OPNE;
    break;
  case opLE:
    token.type = TOKEN_OPLE;
    break;
  case opLT:
    token.type = TOKEN_OPLT;
    break;
  case opGE:
    token.type = TOKEN_OPGE;
    break;
  case opGT:
    token.type = TOKEN_OPGT;
    break;
  default:
    token.type = TOKEN_UNDEF;
    break;
  }
  return token;
}

TokenI peekToken(void) {
  if (!hasPeekedToken) {
    peekedToken = getNextToken();
    hasPeekedToken = true;
  }
  return peekedToken;
}

// Advances to the next token and updates the currentToken
void advanceToken(void) {
  if (hasPeekedToken) {
    currentToken = peekedToken;
    hasPeekedToken = false;
  } else {
    currentToken = getNextToken();
  }
  if (DEBUG_ON) {
    printf("%s\n", currentToken.lexeme);
    fflush(stdout);
  }
}

// Checks that the current token matches the expected type; if so, advances.
bool match(const TokenType expected) {
  // printf("Trying to match token type %d with expected %d\n",
  // currentToken.type,
  //        expected);
  if (currentToken.type == expected) {
    advanceToken();
    return true;
  }
  return false;
}
