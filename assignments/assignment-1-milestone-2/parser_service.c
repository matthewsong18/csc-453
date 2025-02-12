#include "./parser_service.h"
#include "./grammar_interface.h"
#include "./token_service.h"
#include <stdbool.h>
#include <stdio.h>

bool parse_prog(void) {
  // As long as the current token can start a function definition (i.e. kwINT),
  // parse a function definition. (Otherwise, we assume epsilon.)
  while (currentToken.type == TOKEN_KWINT) {
    if (!parse_func_defn())
      return false;
  }
  if (currentToken.type != TOKEN_EOF) {
    fprintf(stderr, "ERROR: LINE %d: unexpected token after program end\n",
            currentToken.line);
    return false;
  }
  return true;
}

bool parse_type(void) { return match(TOKEN_KWINT); }

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

  // func_defn -> type id LPAREN opt_formals RPAREN LBRACE opt_var_decls
  // opt_stmt_list RBRACE
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

bool parse_opt_formals(void) { return true; }

bool parse_opt_var_decls(void) { return true; }

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
