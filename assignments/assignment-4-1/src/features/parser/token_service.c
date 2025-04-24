#include "./token_service.h"
#include "../scanner/scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *lexeme;
extern bool DEBUG_ON;
// Global current token.
token_data currentToken;

static token_data peekedToken;
static bool hasPeekedToken = false;

// Call scanner
token_data getNextToken(void) {
  int tok = get_token();
  token_data token;

  switch (tok) {
  case EOF:
    token.type = TOKEN_EOF;
    token.lexeme = "";
    return token;
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
  case opADD:
    token.type = TOKEN_OPADD;
    break;
  case opSUB:
    token.type = TOKEN_OPSUB;
    break;
  case opMUL:
    token.type = TOKEN_OPMUL;
    break;
  case opDIV:
    token.type = TOKEN_OPDIV;
    break;
  default:
    token.type = TOKEN_UNDEF;
    break;
  }

  token.line = currentLine;
  token.lexeme = strdup(lexeme);
  return token;
}

token_data peekToken(void) {
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

token_data next_token(void) {
  return getNextToken();
}

// Checks that the current token matches the expected type; if so, advances.
bool match(const token_type expected) {
  // printf("Trying to match token type %d with expected %d\n",
  // currentToken.type,
  //        expected);
  if (currentToken.type == expected) {
    advanceToken();
    return true;
  }
  return false;
}
