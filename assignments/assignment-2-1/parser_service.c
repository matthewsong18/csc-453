#include "./parser_service.h"
#include "./symbol_table.h"
#include "./token_service.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *currentType = NULL;

bool parse_prog(void) {
  while (currentToken.type == TOKEN_KWINT) {
    if (!parse_decl_or_func())
      return false;
  }
  if (currentToken.type != TOKEN_EOF) {
    fprintf(stderr, "ERROR: LINE %d: unexpected token after program end\n",
            currentToken.line);
    return false;
  }
  return true;
}

bool parse_type(void) {
  if (match(TOKEN_KWINT)) {
    currentType = "int";
    return true;
  }
  fprintf(stderr, "ERROR: LINE %d: expected type 'kwINT'\n", currentToken.line);
  return false;
}

// Captures the current identifier’s lexeme and advances the token.
char *captureID(void) {
  if (currentToken.type != TOKEN_ID) {
    fprintf(stderr, "ERROR: LINE %d: expected identifier\n", currentToken.line);
    return NULL;
  }
  char *idLex = strdup(lexeme);
  if (!idLex) {
    fprintf(stderr, "ERROR: memory allocation failure\n");
    return NULL;
  }
  advanceToken();
  return idLex;
}

// Chooses the branch based on the next token and handles it.
bool chooseDeclBranch(char *idLex) {
  if (currentToken.type == TOKEN_LPAREN)
    return parseFunctionDeclaration(idLex);
  else if (currentToken.type == TOKEN_COMMA || currentToken.type == TOKEN_SEMI)
    return parseVariableDeclaration(idLex);
  else {
    fprintf(stderr,
            "ERROR: LINE %d: expected '(' for function or ','/';' for variable "
            "declaration\n",
            currentToken.line);
    free(idLex);
    return false;
  }
}

// Branch for function declarations.
bool parseFunctionDeclaration(char *idLex) {
  if (!semanticCheckFunc(idLex)) {
    free(idLex);
    return false;
  }
  bool result = parse_func_defn_rest();
  free(idLex);
  return result;
}

// Branch for variable declarations.
bool parseVariableDeclaration(char *idLex) {
  if (!semanticCheckVar(idLex)) {
    free(idLex);
    return false;
  }
  if (!parse_id_list()) {
    free(idLex);
    return false;
  }
  if (!match(TOKEN_SEMI)) {
    free(idLex);
    return false;
  }
  free(idLex);
  return true;
}

// Semantic check for a variable declaration.
bool semanticCheckVar(const char *idLex) {
  if (!chk_decl_flag)
    return true;
  if (lookupSymbol(idLex, currentScope)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate declaration of '%s'\n",
            currentToken.line, idLex);
    return false;
  }
  return addSymbol(idLex, currentScope, currentType);
}

// Semantic check for a function declaration.
bool semanticCheckFunc(const char *idLex) {
  if (!chk_decl_flag)
    return true;
  if (lookupSymbol(idLex, globalScope)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate function declaration '%s'\n",
            currentToken.line, idLex);
    return false;
  }
  if (!addSymbol(idLex, globalScope, currentType))
    return false;
  pushScope();
  return true;
}

bool parse_decl_or_func(void) {
  if (!parse_type())
    return false;
  char *idLex = captureID();
  if (!idLex)
    return false;
  return chooseDeclBranch(idLex);
}

bool parse_id_list(void) {
  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;
    if (!match(TOKEN_ID)) {
      fprintf(stderr,
              "ERROR: LINE %d: expected identifier after comma in id_list\n",
              currentToken.line);
      return false;
    }
  }
  return true;
}

bool parse_func_defn_rest(void) {
  if (!match(TOKEN_LPAREN))
    return false;
  if (!parse_opt_formals())
    return false;
  if (!match(TOKEN_RPAREN))
    return false;
  if (!match(TOKEN_LBRACE))
    return false;
  if (!parse_opt_var_decls())
    return false;
  if (!parse_opt_stmt_list())
    return false;
  if (!match(TOKEN_RBRACE))
    return false;
  return true;
}

bool parse_var_decl(void) {
  if (!parse_type())
    return false;
  if (!match(TOKEN_ID))
    return false;
  if (!parse_id_list())
    return false;
  if (!match(TOKEN_SEMI))
    return false;
  return true;
}

bool parse_opt_formals(void) {
  if (currentToken.type == TOKEN_KWINT)
    return parse_formals();
  return true; // epsilon production
}

bool parse_formals(void) {
  if (!parse_type())
    return false;
  if (!match(TOKEN_ID)) {
    fprintf(stderr, "ERROR: LINE %d: expected identifier in formals\n",
            currentToken.line);
    return false;
  }
  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;
    if (!parse_type())
      return false;
    if (!match(TOKEN_ID)) {
      fprintf(stderr,
              "ERROR: LINE %d: expected identifier after comma in formals\n",
              currentToken.line);
      return false;
    }
  }
  return true;
}

bool parse_opt_var_decls(void) {
  while (currentToken.type == TOKEN_KWINT) {
    if (!parse_var_decl())
      return false;
  }
  return true;
}

bool parse_opt_stmt_list(void) {
  while (currentToken.type == TOKEN_ID) {
    if (!parse_stmt())
      return false;
  }
  return true;
}

bool parse_stmt(void) {
  if (!parse_fn_call())
    return false;
  if (!match(TOKEN_SEMI))
    return false;
  return true;
}

bool parse_fn_call(void) {
  if (!match(TOKEN_ID))
    return false;
  if (!match(TOKEN_LPAREN))
    return false;
  if (!parse_opt_expr_list())
    return false;
  if (!match(TOKEN_RPAREN))
    return false;
  return true;
}

bool parse_opt_expr_list(void) { return true; }
