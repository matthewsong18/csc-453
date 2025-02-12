#include "./parser_service.h"
#include "./grammar_interface.h"
#include "./token_service.h"
#include <stdbool.h>
#include <stdio.h>

// --- prog: prog -> func_defn prog | epsilon ---
// FIRST(prog) is {kwINT, epsilon}; FOLLOW(prog) is {EOF}.
bool parse_prog(void) {
  // As long as the current token can start a function definition (i.e. kwINT),
  // parse a function definition. (Otherwise, we assume epsilon.)
  while (currentToken.type == TOKEN_KWINT) {
    if (!parse_func_defn())
      return false;
  }
  if (currentToken.type != TOKEN_EOF) {
    fprintf(stderr, "ERROR: Expected EOF at end of program at line %d\n",
            currentToken.line);
    return false;
  }
  return true;
}

// --- type: type -> kwINT ---
bool parse_type(void) { return match(TOKEN_KWINT); }

// --- func_defn: func_defn -> type id LPAREN opt_formals RPAREN LBRACE
// opt_var_decls opt_stmt_list RBRACE ---
bool parse_func_defn(void) {
  // For demonstration, we use an interface-like check for FIRST.
  // For func_defn, FIRST is {TOKEN_KWINT} (from type).
  TokenType funcDefnFirst[] = {TOKEN_KWINT};
  TokenType funcDefnFollow[] = {TOKEN_KWINT,
                                TOKEN_EOF}; // FOLLOW(func_defn) per spec.
  GrammarRule funcDefnRule = {
      .isFirst = isFirstImpl,
      .isFollow = isFollowImpl,
      .parse = NULL, // We’re directly in parse_func_defn.
      .firstSet = funcDefnFirst,
      .firstCount = sizeof(funcDefnFirst) / sizeof(TokenType),
      .followSet = funcDefnFollow,
      .followCount = sizeof(funcDefnFollow) / sizeof(TokenType),
      .name = "func_defn"};

  // Check FIRST condition.
  if (!funcDefnRule.isFirst(&funcDefnRule, currentToken)) {
    fprintf(stderr, "ERROR: func_defn: unexpected token %d at line %d\n",
            currentToken.type, currentToken.line);
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

// --- opt_formals: epsilon ---
bool parse_opt_formals(void) {
  // Nothing to do. In a full implementation you might check if the current
  // token is in FOLLOW(opt_formals).
  return true;
}

// --- opt_var_decls: epsilon ---
bool parse_opt_var_decls(void) { return true; }

// --- opt_stmt_list: opt_stmt_list -> stmt opt_stmt_list | epsilon ---
// FIRST(stmt) is {TOKEN_ID} (since stmt -> fn_call, and fn_call starts with
// id).
bool parse_opt_stmt_list(void) {
  while (currentToken.type == TOKEN_ID) {
    if (!parse_stmt())
      return false;
  }
  // Optionally, you could check that currentToken is in FOLLOW(opt_stmt_list)
  // (which is {TOKEN_RBRACE}).
  return true;
}

// --- stmt: stmt -> fn_call ---
bool parse_stmt(void) { return parse_fn_call(); }

// --- fn_call: fn_call -> id LPAREN opt_expr_list RPAREN SEMI ---
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

// --- opt_expr_list: epsilon ---
bool parse_opt_expr_list(void) { return true; }
