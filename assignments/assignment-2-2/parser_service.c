#include "./parser_service.h"
#include "./symbol_table.h"
#include "./token_service.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *currentType = NULL;

void error(char *error_msg) {
  fprintf(stderr, "ERROR: LINE %d: %s\n", currentToken.line, error_msg);
}

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
  char *idLex = strdup(currentToken.lexeme);
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
  if (chk_decl_flag) {
    popScope(); // Pop scope when exiting
  }
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

bool semanticCheckVar(const char *idLex) {
  if (!chk_decl_flag)
    return true;
  if (lookupSymbol(idLex, currentScope)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate declaration of '%s'\n",
            currentToken.line, idLex);
    return false;
  }
  return addSymbol(idLex, currentScope, currentType,
                   false); // false for variables
}

bool semanticCheckFunc(const char *idLex) {
  if (!chk_decl_flag)
    return true;
  if (lookupSymbol(idLex, globalScope)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate function declaration '%s'\n",
            currentToken.line, idLex);
    return false;
  }
  if (!addSymbol(idLex, globalScope, currentType, true)) // true for functions
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

    // Capture the identifier before advancing
    if (currentToken.type != TOKEN_ID) {
      fprintf(stderr,
              "ERROR: LINE %d: expected identifier after comma in id_list\n",
              currentToken.line);
      return false;
    }

    // Get the lexeme before advancing
    char *idLex = strdup(currentToken.lexeme);
    if (!idLex) {
      fprintf(stderr, "ERROR: memory allocation failure\n");
      return false;
    }

    // Perform semantic check for duplicate declarations
    if (chk_decl_flag) {
      if (lookupSymbol(idLex, currentScope)) {
        fprintf(stderr, "ERROR: LINE %d: duplicate declaration of '%s'\n",
                currentToken.line, idLex);
        free(idLex);
        return false;
      }
      // Add the symbol to the current scope
      if (!addSymbol(idLex, currentScope, currentType, false)) {
        free(idLex);
        return false;
      }
    }

    free(idLex);
    advanceToken(); // Now advance the token after all processing
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
  // Capture the identifier and perform semantic checking
  char *idLex = captureID();
  if (!idLex)
    return false;
  if (!semanticCheckVar(idLex)) {
    free(idLex);
    return false;
  }
  // Process any additional identifiers in the declaration
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

bool parse_opt_formals(void) {
  if (currentToken.type == TOKEN_KWINT)
    return parse_formals();
  return true; // epsilon production
}

bool parse_formals(void) {
  if (!parse_type())
    return false;

  // Capture the parameter name
  char *paramName = captureID();
  if (!paramName)
    return false;

  // Add parameter to current scope
  if (chk_decl_flag) {
    if (lookupSymbol(paramName, currentScope)) {
      fprintf(stderr, "ERROR: LINE %d: duplicate parameter '%s'\n",
              currentToken.line, paramName);
      free(paramName);
      return false;
    }
    if (!addSymbol(paramName, currentScope, currentType,
                   false)) { // false because it's a variable
      free(paramName);
      return false;
    }
  }
  free(paramName);

  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;
    if (!parse_type())
      return false;

    // Capture the parameter name
    paramName = captureID();
    if (!paramName)
      return false;

    // Add parameter to current scope
    if (chk_decl_flag) {
      if (lookupSymbol(paramName, currentScope)) {
        fprintf(stderr, "ERROR: LINE %d: duplicate parameter '%s'\n",
                currentToken.line, paramName);
        free(paramName);
        return false;
      }
      if (!addSymbol(paramName, currentScope, currentType,
                     false)) { // false because it's a variable
        free(paramName);
        return false;
      }
    }
    free(paramName);
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
  while (currentToken.type == TOKEN_ID || currentToken.type == TOKEN_KWIF ||
         currentToken.type == TOKEN_KWWHILE ||
         currentToken.type == TOKEN_KWRETURN ||
         currentToken.type == TOKEN_LBRACE || currentToken.type == TOKEN_SEMI) {
    if (!parse_stmt())
      return false;
  }
  return true;
}

bool parse_stmt(void) {
  switch (currentToken.type) {
  case TOKEN_ID: {
    // Peek at the next token to determine the production
    TokenI nextToken = peekToken();
    if (nextToken.type == TOKEN_LPAREN) {
      // It's a function call
      if (!parse_fn_call()) {
        error("Failed to parse fn call in parse stmt");
        return false;
      }
    } else {
      // It's an assignment
      if (!parse_assg_stmt()) {
        return false;
        error("Failed in parse assg stmt in parse stmt");
      }
    }
    return true;
  }
  case TOKEN_KWWHILE:
    return parse_while_stmt();
  case TOKEN_KWIF:
    return parse_if_stmt();
  case TOKEN_KWRETURN:
    return parse_return_stmt();
  case TOKEN_LBRACE: {
    if (!match(TOKEN_LBRACE))
      return false;
    if (!parse_opt_stmt_list())
      return false;
    return match(TOKEN_RBRACE);
  }
  case TOKEN_SEMI:
    return match(TOKEN_SEMI);
  default:
    fprintf(stderr, "ERROR: LINE %d: invalid statement\n", currentToken.line);
    return false;
  }
}

bool parse_while_stmt(void) {
  // Grammar rules
  if (!match(TOKEN_KWWHILE)) {
    return false;
  }
  if (!match(TOKEN_LPAREN)) {
    return false;
  }
  if (!parse_bool_exp()) {
    return false;
  }
  if (!match(TOKEN_RPAREN)) {
    return false;
  }
  if (!parse_stmt()) {
    return false;
  }
  return true;
}
bool parse_if_stmt(void) {
  if (!match(TOKEN_KWIF)) {
    error("Failed to match TOKEN_KWIF");
    return false;
  }
  if (!match(TOKEN_LPAREN)) {
    error("Failed to parse ( in if");
    return false;
  }
  if (!parse_bool_exp()) {
    error("Failed to parse bool exp in if");
    return false;
  }
  if (!match(TOKEN_RPAREN)) {
    error("Failed to parse ) in if");
    return false;
  }
  if (!parse_stmt()) {
    error("Failed to parse stmt in if");
    return false;
  }

  if (currentToken.type == TOKEN_KWELSE) {
    advanceToken();
    if (!parse_stmt()) {
      error("Failed to parse else clause in if statement");
      return false;
    }
  }

  return true;
}
bool parse_assg_stmt(void) {
  // grammar rules
  if (!match(TOKEN_ID)) {
    return false;
  }
  if (!match(TOKEN_OPASSG)) {
    return false;
  }
  if (!parse_arith_exp()) {
    return false;
  }
  if (!match(TOKEN_SEMI)) {
    return false;
  }
  return true;
}
bool parse_return_stmt(void) {
  // grammar rules
  if (!match(TOKEN_KWRETURN)) {
    return false;
  }

  if (currentToken.type == TOKEN_SEMI) {
    return true;
  } else if (parse_arith_exp()) {
    if (!match(TOKEN_SEMI)) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

bool parse_fn_call(void) {
  // Capture the function identifier before advancing the token
  char *funcName = strdup(currentToken.lexeme);
  if (!funcName) {
    fprintf(stderr, "ERROR: memory allocation failure\n");
    return false;
  }

  // Check if the name exists in the current scope chain
  if (chk_decl_flag) {
    // Start search from current scope and go up
    Scope *scopePtr = currentScope;
    Symbol *symbol = NULL;

    // Search up the scope chain for the first occurrence of the name
    while (scopePtr != NULL && symbol == NULL) {
      symbol = lookupSymbol(funcName, scopePtr);
      if (symbol != NULL) {
        // If we found the name and it's not a function, that's an error
        if (!symbol->isFunction) {
          fprintf(stderr, "ERROR: LINE %d: '%s' is not a function\n",
                  currentToken.line, funcName);
          free(funcName);
          return false;
        }
        // If it is a function, we're good!
        break;
      }
      scopePtr = scopePtr->parent;
    }

    // If we didn't find the name in any scope, it's undeclared
    if (symbol == NULL) {
      fprintf(stderr, "ERROR: LINE %d: call to undeclared function '%s'\n",
              currentToken.line, funcName);
      free(funcName);
      return false;
    }
  }

  free(funcName);

  advanceToken();

  if (!match(TOKEN_LPAREN)) {
    error("Failed ( in fn call");
    return false;
  }
  if (!parse_opt_expr_list()) {
    error("Failed opt expr list in fn call");
    return false;
  }
  if (!match(TOKEN_RPAREN)) {
    error("Failed ) in fn call");
    return false;
  }
  return true;
}

bool parse_opt_expr_list(void) {
  if (currentToken.type == TOKEN_ID || currentToken.type == TOKEN_INTCON) {
    if (!parse_expr_list()) {
      error("Failed expr list in opt expr list");
      return false;
    }
  }
  return true;
}

bool parse_expr_list(void) {
  if (!parse_arith_exp()) {
    error("Failed arith in expr list");
    return false;
  }

  if (currentToken.type == TOKEN_COMMA) {
    advanceToken();
    if (!parse_expr_list()) {
      error("Failed expr list in expr list");
      return false;
    }
  }

  return true;
}

bool parse_bool_exp(void) {
  if (!parse_arith_exp()) {
    error("Failed arith in bool exp");
    return false;
  }
  if (!parse_relop()) {
    error("Failed relop in bool exp");
    return false;
  }
  if (!parse_arith_exp()) {
    error("Failed arith in bool exp");
    return false;
  }
  return true;
}

bool parse_arith_exp(void) {
  if (currentToken.type != TOKEN_ID && currentToken.type != TOKEN_INTCON) {
    error("expected ID or INTCON");
    fprintf(stderr, "%d\n", currentToken.type);
    return false;
  }
  advanceToken();
  return true;
}

bool parse_relop(void) {
  switch (currentToken.type) {
  case TOKEN_OPEQ:
  case TOKEN_OPNE:
  case TOKEN_OPLE:
  case TOKEN_OPLT:
  case TOKEN_OPGE:
  case TOKEN_OPGT:
    advanceToken();
    return true;
  default:
    fprintf(stderr, "ERROR: LINE %d: expected relational operator\n",
            currentToken.line);
    return false;
  }
}
