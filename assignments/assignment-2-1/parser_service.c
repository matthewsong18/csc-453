#include "./parser_service.h"
#include "./grammar_interface.h"
#include "./symbol_table.h"
#include "./token_service.h"
#include <stdbool.h>
#include <stdio.h>

bool parse_prog(void) {
  // As long as the current token can start a function definition (i.e. kwINT),
  // parse a function definition. (Otherwise, we assume epsilon.)
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

extern int chk_decl_flag;
extern char *lexme;
char *currentType = NULL;

bool parse_decl_or_func(void) {
  if (!parse_type())
    return false;
  if (!match(TOKEN_ID))
    return false;

  // --- Semantic Action: Check declaration ---
  // If semantic checking is enabled, verify that this ID hasn't been declared
  // in the current scope.
  if (chk_decl_flag) {
    if (lookupSymbol(lexme, currentScope)) {
      // lookupSymbol is a function you’d write to check for duplicate
      // declarations.
      fprintf(stderr, "ERROR: LINE %d: duplicate declaration of '%s'\n",
              currentToken.line, lexme);
      return false;
    }
    // Add the identifier to the current scope's symbol table.
    addSymbol(lexme, currentScope, currentType);
  }
  // -------------------------------------------------

  // Lookahead: if LPAREN, then it's a function definition.
  if (currentToken.type == TOKEN_LPAREN) {
    if (chk_decl_flag) {
      // Semantic Action for function: add to global symbol table and push a new
      // scope.
      if (lookupSymbol(lexme, globalScope)) {
        fprintf(stderr, "ERROR: LINE %d: duplicate function declaration '%s'\n",
                currentToken.line, lexme);
        return false;
      }
      addSymbol(lexme, globalScope, currentType);
      pushScope(); // Enter new scope for function body.
    }
    return parse_func_defn_rest();
  } else if (currentToken.type == TOKEN_COMMA ||
             currentToken.type == TOKEN_SEMI) {
    // Otherwise, it's a variable declaration.
    if (!parse_id_list()) {
      return false;
    }
    if (!match(TOKEN_SEMI)) {
      return false;
    }
    return true;
  } else {
    fprintf(stderr,
            "ERROR: LINE %d: expected '(' for function or ','/';' for variable "
            "declaration\n",
            currentToken.line);
    return false;
  }
}

bool parse_var_decl_rest(void) { // Parse additional IDs if present (id_list)
  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;
    if (!match(TOKEN_ID))
      return false;
  }
  if (!match(TOKEN_SEMI))
    return false;
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

bool parse_id_list(void) {
  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;
    if (!match(TOKEN_ID))
      return false;
  }
  return true;
}

bool parse_type(void) {
  if (match(TOKEN_KWINT)) {
    char *currentType = NULL;
    return true;
  }
  fprintf(stderr, "ERROR: LINE %d: expected type 'kwINT'\n", currentToken.line);
  return false;
}

bool parse_func_defn(void) {
  // Setup first and follow sets
  TokenType funcDefnFirst[] = {TOKEN_KWINT};
  TokenType funcDefnFollow[] = {TOKEN_KWINT, TOKEN_EOF};

  // Setup grammar rule
  GrammarRule funcDefnRule = {
      .isFirst = isFirstImpl,
      .isFollow = isFollowImpl,
      .parse = NULL, // Currently inside parser
      .firstSet = funcDefnFirst,
      .firstCount = sizeof(funcDefnFirst) / sizeof(TokenType),
      .followSet = funcDefnFollow,
      .followCount = sizeof(funcDefnFollow) / sizeof(TokenType),
      .name = "func_defn"};

  // Check FIRST condition.
  if (!funcDefnRule.isFirst(&funcDefnRule, currentToken)) {
    fprintf(stderr, "ERROR: LINE %d: unexpected token in func_defn\n",
            currentToken.line);
    return false;
  }

  if (!parse_type())
    return false;
  if (!match(TOKEN_ID))
    return false;
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

bool parse_opt_formals(void) {
  if (currentToken.type == TOKEN_KWINT) {
    return parse_formals();
  }
  return true;
}

bool parse_formals(void) {
  if (!parse_type())
    return false;
  if (!match(TOKEN_ID))
    return false;
  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;
    if (!parse_type())
      return false;
    if (!match(TOKEN_ID))
      return false;
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

bool parse_stmt(void) { return parse_fn_call(); }

bool parse_fn_call(void) {
  if (!match(TOKEN_ID))
    return false;
  if (!match(TOKEN_LPAREN))
    return false;
  if (!parse_opt_expr_list())
    return false;
  if (!match(TOKEN_RPAREN))
    return false;
  if (!match(TOKEN_SEMI))
    return false;
  return true;
}

bool parse_opt_expr_list(void) { return true; }
