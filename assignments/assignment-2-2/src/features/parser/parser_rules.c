// parser_rules.c
#include "./grammar_rule.h"
#include "./symbol_table.h"
#include "scanner.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External variables
extern int chk_decl_flag;
extern char *currentType;

// Debug flag
bool DEBUG_ON = false;

// Forward declarations for all parse functions
bool parse_prog_impl(const GrammarRule *rule);
bool parse_func_defn_impl(const GrammarRule *rule);
bool parse_var_decl_impl(const GrammarRule *rule);
bool parse_type_impl(const GrammarRule *rule);
bool parse_id_list_impl(const GrammarRule *rule);
bool parse_func_defn_rest_impl(const GrammarRule *rule);
bool parse_opt_formals_impl(const GrammarRule *rule);
bool parse_formals_impl(const GrammarRule *rule);
bool parse_opt_var_decls_impl(const GrammarRule *rule);
bool parse_opt_stmt_list_impl(const GrammarRule *rule);
bool parse_stmt_impl(const GrammarRule *rule);
bool parse_while_stmt_impl(const GrammarRule *rule);
bool parse_if_stmt_impl(const GrammarRule *rule);
bool parse_assg_stmt_impl(const GrammarRule *rule);
bool parse_return_stmt_impl(const GrammarRule *rule);
bool parse_fn_call_impl(const GrammarRule *rule);
bool parse_opt_expr_list_impl(const GrammarRule *rule);
bool parse_expr_list_impl(const GrammarRule *rule);
bool parse_bool_exp_impl(const GrammarRule *rule);
bool parse_arith_exp_impl(const GrammarRule *rule);
bool parse_relop_impl(const GrammarRule *rule);

bool lookup(const char *name) {
  if (!chk_decl_flag) {
    return true;
  }

  if (!lookupSymbolInScope(name, currentScope)) {
    return false;
  }

  return true;
}

// Helper function to create and store a symbol
bool add_symbol_check(const char *name) {
  if (!chk_decl_flag)
    return true;

  if (lookup(name)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate %s declaration\n",
            currentToken.line, name);
    return false;
  }

  const bool isFunction = currentScope->parent != NULL;

  return addSymbol(name, currentScope, currentType, isFunction);
}

// Helper function to capture an identifier
char *capture_identifier() {
  if (currentToken.type != TOKEN_ID) {
    fprintf(stderr, "ERROR: LINE %d: expected identifier\n", currentToken.line);
    return NULL;
  }

  char *id = strdup(currentToken.lexeme);
  if (!id) {
    fprintf(stderr, "ERROR: memory allocation failure\n");
    return NULL;
  }

  advanceToken();
  return id;
}

void debug(char *source) {
  if (DEBUG_ON) {
    printf("%s\n", source);
    fflush(stdout);
  }
}

// Implementation of all parse functions

// Program rule:
bool parse_prog_impl(const GrammarRule *rule) {
  debug("parse_prog_impl");
  // Check first
  while (rule->isFirst(rule, currentToken)) {
    // Parse prog
    bool good_result;

    // We need to parse type since both func and var have type and ID so we'll
    // be doing the first check in prog instead
    // Parsing type
    const GrammarRule *type_rule = get_rule("type");
    if (!type_rule->parse(type_rule)) {
      report_error(rule->name, "expected a type");
      return false;
    }

    // Checking that ID follows type
    if (!type_rule->isFollow(type_rule, currentToken)) {
      report_error(rule->name, "expected ID after type");
      return false;
    }

    const TokenI peek_token = peekToken();
    if (peek_token.type == TOKEN_LPAREN) {
      const GrammarRule *function_rule = get_rule("func_defn");
      good_result = function_rule->parse(function_rule);
    } else {
      const GrammarRule *var_rule = get_rule("var_decl");
      good_result = var_rule->parse(var_rule);
    }

    // Checking result
    if (!good_result) {
      return false;
    }
  }

  // Check follow even if first is not matched because of epsilon
  if (!rule->isFollow(rule, currentToken)) {
    report_error(rule->name, "unexpected follow token");
    return false;
  }

  return true;
}

bool parse_func_defn_impl(const GrammarRule *rule) {
  debug("parse_func_defn_impl");
  // Type was already parsed in prog

  // Parse ID
  char *id = capture_identifier();
  if (!id) {
    report_error(rule->name, "expected identifier");
    return false;
  }

  // Adding symbol
  if (!add_symbol_check(id)) {
    report_error(rule->name, "adding symbol failed");
    free(id);
    return false;
  }

  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN token");
    free(id);
    return false;
  }

  // Parse opt_formals
  const GrammarRule *opt_formals = get_rule("opt_formals");
  if (!opt_formals->parse(opt_formals)) {
    report_error(rule->name, "unexpected token in opt_formals");
    free(id);
    return false;
  }

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN token");
    free(id);
    return false;
  }

  // PARSE LBRACE
  if (!match(TOKEN_LBRACE)) {
    report_error(rule->name, "expected LBRACE token");
    free(id);
    return false;
  }

  // Parse opt_var_decls
  const GrammarRule *opt_var_decls = get_rule("opt_var_decls");
  if (!opt_var_decls->parse(opt_var_decls)) {
    report_error(rule->name, "unexpected token in opt_var_decls");
    free(id);
    return false;
  }

  // Parse opt_stmt_list
  const GrammarRule *opt_stmt_list = get_rule("opt_stmt_list");
  if (!opt_stmt_list->parse(opt_stmt_list)) {
    report_error(rule->name, "unexpected token in opt_stmt_list");
    free(id);
    return false;
  }

  // Parse RBRACE
  if (!match(TOKEN_RBRACE)) {
    report_error(rule->name, "expected RBRACE token");
    free(id);
    return false;
  }

  return true;
}

bool parse_opt_formals_impl(const GrammarRule *rule) {
  debug("parse_opt_formals_impl");
  const GrammarRule *opt_formals = get_rule("opt_formals");

  // Only parses if not epsilon and in first
  if (opt_formals->isFirst(opt_formals, currentToken)) {
    // Parse formals
    const GrammarRule *formals = get_rule("formals");
    if (!formals->parse(formals)) {
      report_error(rule->name, "unexpected token in formals");
      return false;
    }
  }

  return true; // Epsilon
}

bool parse_formals_impl(const GrammarRule *rule) {
  debug("parse_formals_impl");
  const GrammarRule *formals = get_rule("formals");

  // Check first
  if (!formals->isFirst(formals, currentToken)) {
    report_error(rule->name, "unexpected token in formals");
    return false;
  }

  // Parse type
  const GrammarRule *type = get_rule("type");
  if (!type->parse(type)) {
    report_error(rule->name, "unexpected token in type");
    return false;
  }

  // Parse ID
  char *id = capture_identifier();
  if (!id) {
    report_error(rule->name, "expected identifier");
    return false;
  }

  // Adding symbol
  if (!add_symbol_check(id)) {
    report_error(rule->name, "adding symbol failed");
    free(id);
    return false;
  }

  // Parse optional comma
  if (match(TOKEN_COMMA)) {
    if (!formals->parse(formals)) {
      report_error(rule->name, "unexpected token in formals");
      free(id);
      return false;
    }
  }

  return true;
}

bool parse_opt_var_decls_impl(const GrammarRule *rule) {
  debug("parse_opt_var_decls_impl");
  const GrammarRule *opt_var_decls = get_rule("opt_var_decls");

  // check first
  if (!opt_var_decls->isFirst(opt_var_decls, currentToken)) {
    return true; // Epsilon
  }

  // Parse var_decl (Need to do type first)
  const GrammarRule *type = get_rule("type");
  if (!type->parse(type)) {
    report_error(rule->name, "unexpected token in type");
    return false;
  }

  const GrammarRule *var_decl = get_rule("var_decl");
  if (!var_decl->parse(var_decl)) {
    report_error(rule->name, "unexpected token in var_decl");
    return false;
  }

  // Parse opt_var_decls
  if (!opt_var_decls->parse(opt_var_decls)) {
    report_error(rule->name, "unexpected token in opt_var_decls");
    return false;
  }

  return true;
}

bool parse_opt_stmt_list_impl(const GrammarRule *rule) {
  debug("parse_opt_stmt_list_impl");
  const GrammarRule *opt_stmt_list = get_rule("opt_stmt_list");
  if (!opt_stmt_list->isFirst(opt_stmt_list, currentToken)) {
    return true; // Epsilon
  }

  // Parse stmt
  const GrammarRule *stmt = get_rule("stmt");
  if (!stmt->parse(stmt)) {
    report_error(rule->name, "unexpected token in stmt");
    return false;
  }

  // Parse opt_stmt_list
  if (!opt_stmt_list->parse(opt_stmt_list)) {
    report_error(rule->name, "unexpected token in opt_stmt_list");
    return false;
  }

  return true;
}

bool parse_stmt_impl(const GrammarRule *rule) {
  debug("parse_stmt_impl");
  const GrammarRule *stmt = get_rule("stmt");

  // check first
  if (!stmt->isFirst(stmt, currentToken)) {
    report_error(rule->name, "unexpected token in stmt");
    return false;
  }

  // Check fn_call
  const GrammarRule *fn_call = get_rule("fn_call");
  if (fn_call->isFirst(fn_call, currentToken)) {
    if (!fn_call->parse(fn_call)) {
      report_error(rule->name, "unexpected token in fn_call");
      return false;
    }
    return true;
  }

  // Check while_stmt
  const GrammarRule *while_stmt = get_rule("while_stmt");
  if (while_stmt->isFirst(while_stmt, currentToken)) {
    if (!while_stmt->parse(while_stmt)) {
      report_error(rule->name, "unexpected token in while_stmt");
      return false;
    }
    return true;
  }

  // Check if_stmt
  const GrammarRule *if_stmt = get_rule("if_stmt");
  if (if_stmt->isFirst(if_stmt, currentToken)) {
    if (!if_stmt->parse(if_stmt)) {
      report_error(rule->name, "unexpected token in if_stmt");
      return false;
    }
    return true;
  }

  // Check assg_stmt
  const GrammarRule *assg_stmt = get_rule("assg_stmt");
  if (assg_stmt->isFirst(assg_stmt, currentToken)) {
    if (!assg_stmt->parse(assg_stmt)) {
      report_error(rule->name, "unexpected token in assg_stmt");
      return false;
    }
    return true;
  }

  // Check return_stmt
  const GrammarRule *return_stmt = get_rule("return_stmt");
  if (return_stmt->isFirst(return_stmt, currentToken)) {
    if (!return_stmt->parse(return_stmt)) {
      report_error(rule->name, "unexpected token in return_stmt");
      return false;
    }
    return true;
  }

  // Match LBRACE
  if (match(TOKEN_LBRACE)) {
    // Parse opt_stmt_list
    const GrammarRule *opt_stmt_list = get_rule("opt_stmt_list");
    if (!opt_stmt_list->parse(opt_stmt_list)) {
      report_error(rule->name, "unexpected token in opt_stmt_list");
      return false;
    }

    // Parse RBRACE
    if (!match(TOKEN_RBRACE)) {
      report_error(rule->name, "expected RBRACE");
      return false;
    }
    return true;
  }

  // Match SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected SEMI");
    return false;
  }

  return true;
}

bool parse_fn_call_impl(const GrammarRule *rule) {
  debug("parse_fn_call_impl");
  const GrammarRule *fn_call = get_rule("fn_call");
  if (!fn_call->isFirst(fn_call, currentToken)) {
    report_error(rule->name, "unexpected token in fn_call");
    return false;
  }

  // Parse ID
  char *id = capture_identifier();
  if (!id) {
    report_error(rule->name, "unexpected token in id");
    free(id);
    return false;
  }

  if (!lookup(id)) {
    report_error(rule->name, "ID does not exist");
    free(id);
    return false;
  }

  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN");
    free(id);
  }

  // Parse opt_expr_list
  const GrammarRule *opt_expr_list = get_rule("opt_expr_list");
  if (!opt_expr_list->parse(opt_expr_list)) {
    report_error(rule->name, "unexpected token in opt_expr_list");
    free(id);
    return false;
  }

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN");
    free(id);
    return false;
  }

  // Parse SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected SEMI");
    free(id);
    return false;
  }

  return true;
}

bool parse_opt_expr_list_impl(const GrammarRule *rule) {
  debug("parse_opt_expr_list_impl");
  const GrammarRule *opt_expr_list = get_rule("opt_expr_list");

  if (!opt_expr_list->isFirst(opt_expr_list, currentToken)) {
    return true; // Epsilon
  }

  // Parse expr_list
  const GrammarRule *expr_list = get_rule("expr_list");
  if (!expr_list->parse(expr_list)) {
    report_error(rule->name, "unexpected token in expr_list");
    return false;
  }

  return true;
}

bool parse_expr_list_impl(const GrammarRule *rule) {
  debug("parse_expr_list_impl");
  const GrammarRule *expr_list = get_rule("expr_list");
  if (!expr_list->isFirst(expr_list, currentToken)) {
    report_error(rule->name, "unexpected token in expr_list");
    return false;
  }

  // Parse arith_exp
  const GrammarRule *arith_exp = get_rule("arith_exp");
  if (!arith_exp->parse(arith_exp)) {
    report_error(rule->name, "unexpected token in arith_exp");
    return false;
  }

  // Parse optional COMMA
  if (match(TOKEN_COMMA)) {
    // Parse expr_list
    if (!expr_list->parse(expr_list)) {
      report_error(rule->name, "unexpected token in expr_list");
      return false;
    }
    return true;
  }

  return true;
}

bool parse_arith_exp_impl(const GrammarRule *rule) {
  debug("parse_arith_exp_impl");
  const GrammarRule *arith_exp = get_rule("arith_exp");
  if (!arith_exp->isFirst(arith_exp, currentToken)) {
    report_error(rule->name, "token not in arith_exp first");
    return false;
  }

  // Check for ID
  if (currentToken.type == TOKEN_ID) {
    char *id = capture_identifier();
    if (!lookup(id)) {
      report_error(rule->name, "ID does not exist");
      free(id);
      return false;
    }
    free(id);
    return true;
  }

  if (!match(TOKEN_INTCON)) {
    report_error(rule->name, "unexpected token in intcon");
    return false;
  }

  return true;
}

bool parse_while_stmt_impl(const GrammarRule *rule) {
  debug("parse_while_stmt_impl");
  const GrammarRule *while_stmt = get_rule("while_stmt");
  if (!while_stmt->isFirst(while_stmt, currentToken)) {
    report_error(rule->name, "unexpected token in while_stmt");
    return false;
  }

  // Parse kwWHILE
  if (!match(TOKEN_KWWHILE)) {
    report_error(rule->name, "unexpected token in while_stmt");
    return false;
  }

  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN");
    return false;
  }

  // Parse bool_exp
  const GrammarRule *bool_exp = get_rule("bool_exp");
  if (!bool_exp->parse(bool_exp)) {
    report_error(rule->name, "unexpected token in bool_exp");
    return false;
  }

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN");
    return false;
  }

  // Parse stmt
  const GrammarRule *stmt = get_rule("stmt");
  if (!stmt->parse(stmt)) {
    report_error(rule->name, "unexpected token in stmt");
    return false;
  }

  return true;
}

bool parse_if_stmt_impl(const GrammarRule *rule) {
  debug("parse_if_stmt_impl");
  const GrammarRule *if_stmt = get_rule("if_stmt");

  // Check first
  if (!if_stmt->isFirst(if_stmt, currentToken)) {
    report_error(rule->name, "unexpected token in if_stmt");
    return false;
  }

  // Parse if
  if (!match(TOKEN_KWIF)) {
    report_error(rule->name, "expected kwIF token");
    return false;
  }

  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN");
    return false;
  }

  // Parse bool_exp
  const GrammarRule *bool_exp = get_rule("bool_exp");
  if (!bool_exp->parse(bool_exp)) {
    report_error(rule->name, "unexpected token in bool_exp");
    return false;
  }

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN");
    return false;
  }

  // Parse stmt
  const GrammarRule *stmt = get_rule("stmt");
  if (!stmt->parse(stmt)) {
    report_error(rule->name, "unexpected token in stmt");
    return false;
  }

  // Parse optional else
  if (match(TOKEN_KWELSE)) {
    if (!stmt->parse(stmt)) {
      report_error(rule->name, "unexpected token in stmt");
      return false;
    }
  }

  return true;
}

bool parse_bool_exp_impl(const GrammarRule *rule) {
  debug("parse_bool_exp_impl");
  const GrammarRule *bool_exp = get_rule("bool_exp");
  // Check first
  if (!bool_exp->isFirst(bool_exp, currentToken)) {
    report_error(rule->name, "unexpected token in bool_exp");
    return false;
  }

  // Parse arith_exp
  const GrammarRule *arith_exp = get_rule("arith_exp");
  if (!arith_exp->parse(arith_exp)) {
    report_error(rule->name, "unexpected token in arith_exp");
    return false;
  }

  // Parse relop
  const GrammarRule *relop = get_rule("relop");
  if (!relop->parse(relop)) {
    report_error(rule->name, "unexpected token in relop");
    return false;
  }

  // Parse arith_exp
  if (!arith_exp->parse(arith_exp)) {
    report_error(rule->name, "unexpected token in arith_exp");
    return false;
  }

  return true;
}

bool parse_relop_impl(const GrammarRule *rule) {
  debug("parse_relop_impl");
  const GrammarRule *relop = get_rule("relop");
  // Check first
  if (!relop->isFirst(relop, currentToken)) {
    report_error(rule->name, "token not in relop first");
    return false;
  }

  if (match(TOKEN_OPEQ)) {
    debug("EQ");
    return true;
  }

  if (match(TOKEN_OPNE)) {
    debug("NE");
    return true;
  }

  if (match(TOKEN_OPLE)) {
    debug("LE");
    return true;
  }

  if (match(TOKEN_OPLT)) {
    debug("LT");
    return true;
  }

  if (match(TOKEN_OPGE)) {
    debug("GE");
    return true;
  }

  if (match(TOKEN_OPGT)) {
    debug("GT");
    return true;
  }

  report_error(rule->name, "unexpected token in relop");
  return false;
}

bool parse_assg_stmt_impl(const GrammarRule *rule) {
  debug("parse_assg_stmt_impl");
  const GrammarRule *assg_stmt = get_rule("assg_stmt");
  if (!assg_stmt->isFirst(assg_stmt, currentToken)) {
    report_error(rule->name, "unexpected token in assg_stmt");
    return false;
  }

  // Parse ID
  char *id = capture_identifier();
  if (!id) {
    report_error(rule->name, "expected identifier");
    free(id);
    return false;
  }

  // Lookup
  if (!lookup(id)) {
    report_error(rule->name, "ID does not exist");
    free(id);
    return false;
  }

  // parse opASSG
  if (!match(TOKEN_OPASSG)) {
    report_error(rule->name, "expected opASSG");
    free(id);
    return false;
  }

  // parse arith_exp
  const GrammarRule *arith_exp = get_rule("arith_exp");
  if (!arith_exp->parse(arith_exp)) {
    report_error(rule->name, "unexpected token in arith_exp");
    free(id);
    return false;
  }

  // parse SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected semi token");
    free(id);
    return false;
  }

  return true;
}

bool parse_return_stmt_impl(const GrammarRule *rule) {
  debug("parse_return_stmt_impl");
  const GrammarRule *return_stmt = get_rule("return_stmt");
  if (!return_stmt->isFirst(return_stmt, currentToken)) {
    report_error(rule->name, "unexpected token in return_stmt");
    return false;
  }

  // parse return
  if (!match(TOKEN_KWRETURN)) {
    report_error(rule->name, "unexpected token in return_stmt");
    return false;
  }

  // parse optional arith_exp
  const GrammarRule *arith_exp = get_rule("arith_exp");
  if (arith_exp->isFirst(arith_exp, currentToken)) {
    // parse arith_exp
    if (!arith_exp->parse(arith_exp)) {
      report_error(rule->name, "unexpected token in arith_exp");
      return false;
    }
  }

  // parse semi
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected semi token");
    return false;
  }

  return true;
}

// Variable declaration rule: var_decl → type ID id_list ';'
bool parse_var_decl_impl(const GrammarRule *rule) {
  debug("parse_var_decl_impl");
  // Type was already parsed in prog

  // Parsing id list
  const GrammarRule *id_list = get_rule("id_list");
  if (!id_list->parse(id_list)) {
    return false;
  }

  // Parse SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected SEMI");
    return false;
  }

  advanceToken();
  return true;
}

// Type rule: type → 'int'
bool parse_type_impl(const GrammarRule *rule) {
  debug("parse_type_impl");
  if (currentToken.type == TOKEN_KWINT) {
    currentType = "int";
    advanceToken();
    return true;
  }

  report_error(rule->name, "expected type 'int'");
  return false;
}

// ID list rule: id_list → (',' ID)*
bool parse_id_list_impl(const GrammarRule *rule) {
  debug("parse_id_list_impl");
  const GrammarRule *id_list = get_rule("id_list");

  // Check first
  if (!id_list->isFirst(id_list, currentToken)) {
    report_error(rule->name, "token not in id_list first");
    return false;
  }

  // parse ID
  if (currentToken.type != TOKEN_ID) {
    report_error(rule->name, "expected identifier after comma");
    return false;
  }

  char *id = capture_identifier();
  if (!id) {
    report_error(rule->name, "expected identifier");
    return false;
  }

  add_symbol_check(id);

  // Parse optional COMMA
  if (match(TOKEN_COMMA)) {
    // Parse id_list
    if (!id_list->parse(id_list)) {
      report_error(rule->name, "unexpected token in id_list");
      free(id);
      return false;
    }
  }

  free(id);
  return true;
}

// Initialize all grammar rules
void init_grammar_rules(void) {
  // Define FIRST and FOLLOW sets for rules

  // FIRST sets
  const TokenType arith_exp_first[] = {TOKEN_ID, TOKEN_INTCON};
  const TokenType assg_stmt_first[] = {TOKEN_ID};
  const TokenType bool_exp_first[] = {TOKEN_ID, TOKEN_INTCON};
  const TokenType expr_list_first[] = {TOKEN_ID, TOKEN_INTCON};
  const TokenType fn_call_first[] = {TOKEN_ID};
  const TokenType formals_first[] = {TOKEN_KWINT};
  const TokenType func_defn_first[] = {TOKEN_KWINT};
  const TokenType id_list_first[] = {TOKEN_ID};
  const TokenType if_stmt_first[] = {TOKEN_KWIF};
  const TokenType opt_expr_list_first[] = {TOKEN_ID, TOKEN_INTCON};
  const TokenType opt_formals_first[] = {TOKEN_KWINT};
  const TokenType opt_stmt_list_first[] = {TOKEN_ID,       TOKEN_KWIF,
                                           TOKEN_KWRETURN, TOKEN_LBRACE,
                                           TOKEN_SEMI,     TOKEN_KWWHILE};
  const TokenType opt_var_decl_first[] = {TOKEN_KWINT};
  const TokenType prog_first[] = {TOKEN_KWINT};
  const TokenType relop_first[] = {TOKEN_OPEQ, TOKEN_OPNE, TOKEN_OPLE,
                                   TOKEN_OPLT, TOKEN_OPGE, TOKEN_OPGT};
  const TokenType return_stmt_first[] = {TOKEN_KWRETURN};
  const TokenType stmt_first[] = {TOKEN_ID,     TOKEN_KWIF, TOKEN_KWRETURN,
                                  TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE};
  const TokenType type_first[] = {TOKEN_KWINT};
  const TokenType var_decl_first[] = {TOKEN_KWINT};
  const TokenType while_stmt_first[] = {TOKEN_KWWHILE};

  // FOLLOW sets
  const TokenType arith_exp_follow[] = {TOKEN_SEMI, TOKEN_OPEQ,  TOKEN_OPNE,
                                        TOKEN_OPLE, TOKEN_OPLT,  TOKEN_OPGE,
                                        TOKEN_OPGT, TOKEN_COMMA, TOKEN_RPAREN};
  const TokenType assg_stmt_follow[] = {
      TOKEN_KWELSE, TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN,
      TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};
  const TokenType bool_exp_follow[] = {TOKEN_RPAREN};
  const TokenType expr_list_follow[] = {TOKEN_RPAREN};
  const TokenType fn_call_follow[] = {TOKEN_KWELSE,   TOKEN_ID,     TOKEN_KWIF,
                                      TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                      TOKEN_KWWHILE,  TOKEN_RBRACE};
  const TokenType formals_follow[] = {TOKEN_RPAREN};
  const TokenType func_defn_follow[] = {TOKEN_KWINT, TOKEN_EOF};
  const TokenType id_list_follow[] = {TOKEN_SEMI};
  const TokenType if_stmt_follow[] = {TOKEN_KWELSE,   TOKEN_ID,     TOKEN_KWIF,
                                      TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                      TOKEN_KWWHILE,  TOKEN_RBRACE};
  const TokenType opt_expr_list_follow[] = {TOKEN_RPAREN};
  const TokenType opt_formals_follow[] = {TOKEN_RPAREN};
  const TokenType opt_stmt_list_follow[] = {TOKEN_RBRACE};
  const TokenType opt_var_decls_follow[] = {
      TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN, TOKEN_LBRACE,
      TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};
  const TokenType prog_follow[] = {TOKEN_EOF};
  const TokenType relop_follow[] = {TOKEN_ID, TOKEN_INTCON};
  const TokenType return_stmt_follow[] = {
      TOKEN_KWELSE, TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN,
      TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};
  const TokenType stmt_follow[] = {TOKEN_KWELSE,   TOKEN_ID,     TOKEN_KWIF,
                                   TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                   TOKEN_KWWHILE,  TOKEN_RBRACE};
  const TokenType type_follow[] = {TOKEN_ID};
  const TokenType var_decl_follow[] = {TOKEN_KWINT,    TOKEN_ID,     TOKEN_KWIF,
                                       TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                       TOKEN_KWWHILE,  TOKEN_RBRACE, TOKEN_EOF};
  const TokenType while_stmt_follow[] = {
      TOKEN_KWELSE, TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN,
      TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};

  // Create rules with their parsing functions
  create_rule("prog", prog_first, 1, prog_follow, 1, parse_prog_impl);
  create_rule("type", type_first, 1, type_follow, 1, parse_type_impl);
  create_rule("arith_exp", arith_exp_first, 2, arith_exp_follow, 9,
              parse_arith_exp_impl);
  create_rule("assg_stmt", assg_stmt_first, 1, assg_stmt_follow, 8,
              parse_assg_stmt_impl);
  create_rule("bool_exp", bool_exp_first, 2, bool_exp_follow, 1,
              parse_bool_exp_impl);
  create_rule("expr_list", expr_list_first, 2, expr_list_follow, 1,
              parse_expr_list_impl);
  create_rule("fn_call", fn_call_first, 1, fn_call_follow, 8,
              parse_fn_call_impl);
  create_rule("formals", formals_first, 1, formals_follow, 1,
              parse_formals_impl);
  create_rule("func_defn", func_defn_first, 1, func_defn_follow, 2,
              parse_func_defn_impl);
  create_rule("id_list", id_list_first, 1, id_list_follow, 1,
              parse_id_list_impl);
  create_rule("if_stmt", if_stmt_first, 1, if_stmt_follow, 8,
              parse_if_stmt_impl);
  create_rule("opt_expr_list", opt_expr_list_first, 2, opt_expr_list_follow, 1,
              parse_opt_expr_list_impl);
  create_rule("opt_formals", opt_formals_first, 1, opt_formals_follow, 1,
              parse_opt_formals_impl);
  create_rule("opt_stmt_list", opt_stmt_list_first, 6, opt_stmt_list_follow, 1,
              parse_opt_stmt_list_impl);
  create_rule("opt_var_decls", opt_var_decl_first, 1, opt_var_decls_follow, 7,
              parse_opt_var_decls_impl);
  create_rule("relop", relop_first, 6, relop_follow, 2, parse_relop_impl);
  create_rule("return_stmt", return_stmt_first, 1, return_stmt_follow, 8,
              parse_return_stmt_impl);
  create_rule("stmt", stmt_first, 6, stmt_follow, 8, parse_stmt_impl);
  create_rule("var_decl", var_decl_first, 1, var_decl_follow, 9,
              parse_var_decl_impl);
  create_rule("while_stmt", while_stmt_first, 1, while_stmt_follow, 8,
              parse_while_stmt_impl);
}