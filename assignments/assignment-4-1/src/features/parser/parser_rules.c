// parser_rules.c
#include "ast.h"
#include "grammar_rule.h"
#include "mips.h"
#include "symbol_table.h"
#include "tac.h"
#include "token_service.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External variables
extern int chk_decl_flag;
extern int print_ast_flag;
extern int gen_code_flag;
extern int DEBUG_ON;
int PAR_DEBUG_ON = true;

// Forward declarations for all parse functions
ASTnode *parse_assg_or_fn_impl(const GrammarRule *rule);
ASTnode *parse_prog_impl(const GrammarRule *rule);
ASTnode *parse_func_defn_impl(const GrammarRule *rule);
ASTnode *parse_decl_or_func_impl(const GrammarRule *rule);
ASTnode *parse_var_decl_impl(const GrammarRule *rule);
ASTnode *parse_type_impl(const GrammarRule *rule);
ASTnode *parse_id_list_impl(const GrammarRule *rule);
ASTnode *parse_opt_formals_impl(const GrammarRule *rule);
ASTnode *parse_formals_impl(const GrammarRule *rule);
ASTnode *parse_opt_var_decls_impl(const GrammarRule *rule);
ASTnode *parse_opt_stmt_list_impl(const GrammarRule *rule);
ASTnode *parse_stmt_impl(const GrammarRule *rule);
ASTnode *parse_while_stmt_impl(const GrammarRule *rule);
ASTnode *parse_if_stmt_impl(const GrammarRule *rule);
ASTnode *parse_assg_stmt_impl(const GrammarRule *rule);
ASTnode *parse_return_stmt_impl(const GrammarRule *rule);
ASTnode *parse_fn_call_impl(const GrammarRule *rule);
ASTnode *parse_opt_expr_list_impl(const GrammarRule *rule,
                                  Symbol *function_symbol);
ASTnode *parse_expr_list_impl(const GrammarRule *rule, Symbol *function_symbol);
ASTnode *parse_bool_exp_impl(const GrammarRule *rule);
ASTnode *parse_arith_exp_impl(const GrammarRule *rule, Symbol *function_symbol);
ASTnode *parse_relop_impl(const GrammarRule *rule);

void debug(char *source) {
  if (DEBUG_ON && PAR_DEBUG_ON) {
    printf("%s\n", source);
    fflush(stdout);
  }
}

// Calls the symbol_table lookup function to search the entire table for the
// symbol
bool lookup(const char *name, const char *type) {
  if (!chk_decl_flag) {
    return true;
  }

  Symbol *symbol = lookup_symbol_in_table(name, type);
  if (symbol == NULL) {
    return NULL;
  }

  return true;
}

// Helper function to create and store a symbol
bool add_symbol_check(const char *name, const char *type) {
  if (!chk_decl_flag)
    return true;

  if (check_duplicate_symbol_in_scope(name, type, currentScope)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate %s declaration\n",
            currentToken.line, name);
    exit(1);
  }

  if (strcmp(type, "function") == 0) {
    debug("added function symbol");
    return add_function_symbol(name);
  } else {
    debug("added variable symbol");
    return add_variable_symbol(name);
  }
}

// Helper function to capture an identifier
char *capture_identifier() {
  if (currentToken.type != TOKEN_ID) {
    fprintf(stderr, "ERROR: LINE %d: expected identifier\n", currentToken.line);
    exit(1);
  }

  char *id = strdup(currentToken.lexeme);
  if (!id) {
    fprintf(stderr, "ERROR: memory allocation failure\n");
    exit(1);
  }

  advanceToken();
  return id;
}

// Implementation of all parse functions

// Program rule:
ASTnode *parse_prog_impl(const GrammarRule *rule) {
  debug("parse_prog_impl");

  ASTnode *func_node = NULL;
  Quad *code_list = NULL;

  // Check first
  while (rule->isFirst(rule, currentToken)) {
    // We need to parse type since both func and var have type and ID so we'll
    // be doing the first check in prog instead
    // Parsing type
    debug("prog calls type");
    const GrammarRule *type_rule = get_rule("type");
    type_rule->parse(type_rule);

    // Because we don't know whether the following rule will be a var_decl or
    // func_defn at this point, we don't want to use match(), because match()
    // will advance the token and leave us no way to access the current ID
    // unless we save it globally. We need to postpone the ID so we can save it
    // on the right scope in the symbol table.
    debug("prog checks ID");
    if (!type_rule->isFollow(type_rule, currentToken)) {
      report_error(rule->name, "expected ID after type");
      exit(1);
    }

    // Call decl_or_func rule
    debug("prog calls decl_or_func");
    const GrammarRule *decl_or_func = get_rule("decl_or_func");
    func_node = decl_or_func->parse(decl_or_func);

    if (gen_code_flag) {
      make_TAC(func_node, &code_list);
    }
  }

  // Check follow even if first is not matched because of epsilon
  // Matching EOF
  debug("prog checking for EOF");
  if (!rule->isFollow(rule, currentToken)) {
    report_error(rule->name, "unexpected follow token");
    exit(1);
  }

  if (gen_code_flag) {

    Quad *reversed_code_list = reverse_tac_list(code_list);

    MipsInstruction *mips_list = NULL;
    mips_list = generate_mips(reversed_code_list);

    char *output_string = NULL;
    output_string = mips_list_to_string(mips_list);

    printf("%s", output_string);
  }

  return func_node;
}

ASTnode *parse_decl_or_func_impl(const GrammarRule *rule) {

  token_data lookahead_token = peekToken();
  // Check next token is in FIRST set
  if (!rule->isFirst(rule, lookahead_token)) {
    report_error(rule->name, "lookahead token does not match first set");
    exit(1);
  }

  char *id_name = capture_identifier();
  // Check var_decl rule
  if (lookahead_token.type == TOKEN_COMMA) {
    if (add_symbol_check(id_name, "variable") == false) {
      report_error(rule->name, "failed to add variable id to symbol table");
      exit(1);
    }
    // Call var_decl
    debug("decl_or_func calls var_decl");
    const GrammarRule *var_decl = get_rule("var_decl");
    var_decl->parse(var_decl);
    return NULL;
  } else if (lookahead_token.type == TOKEN_LPAREN) {
    if (add_symbol_check(id_name, "function") == false) {
      report_error(rule->name, "failed to add variable id to symbol table");
      exit(1);
    }
    // Call func_defn
    debug("decl_or_func calls func_defn");
    const GrammarRule *func_defn = get_rule("func_defn");
    ASTnode *func_defn_node = func_defn->parse(func_defn);

    func_defn_node->symbol = lookup_symbol_in_table(id_name, "function");

    if (func_defn_node->symbol == NULL) {
      report_error(rule->name, "Could not find symbol");
      exit(1);
    }

    if (print_ast_flag) {
      print_ast(func_defn_node);
    }

    popScope();
    return func_defn_node;
  } else {
    // This case exists for when only a single variable is defined
    if (add_symbol_check(id_name, "variable") == false) {
      report_error(rule->name, "failed to add variable id to symbol table");
      exit(1);
    }
    if (!match(TOKEN_SEMI)) {
      report_error(rule->name,
                   "token didn't match decl or function grammar rules");
      exit(1);
    }
  }

  // Other ast's not implemented for now
  return NULL;
}

ASTnode *parse_func_defn_impl(const GrammarRule *rule) {
  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN token");
    exit(1);
  }

  // Everything from the LPAREN token to the RBRACE token in the func_defn is
  // part of the function's local scope
  pushScope();

  // Parse opt_formals
  debug("func_defn calls opt_formals");
  const GrammarRule *opt_formals = get_rule("opt_formals");
  opt_formals->parse(opt_formals);

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN token");
    exit(1);
  }

  // PARSE LBRACE
  if (!match(TOKEN_LBRACE)) {
    report_error(rule->name, "expected LBRACE token");
    exit(1);
  }

  // Parse opt_var_decls
  debug("func_defn calls opt_var_decls");
  const GrammarRule *opt_var_decls = get_rule("opt_var_decls");
  opt_var_decls->parse(opt_var_decls);

  // Parse opt_stmt_list
  debug("func_defn calls opt_stmt_list");
  const GrammarRule *opt_stmt_list = get_rule("opt_stmt_list");
  ASTnode *stmt_list_node = opt_stmt_list->parse(opt_stmt_list);

  // Parse RBRACE
  if (!match(TOKEN_RBRACE)) {
    report_error(rule->name, "expected RBRACE token");
    exit(1);
  }

  ASTnode *func_defn_node = create_func_defn_node(NULL, stmt_list_node);

  return func_defn_node;
}

ASTnode *parse_opt_formals_impl(const GrammarRule *rule) {
  const GrammarRule *opt_formals = get_rule("opt_formals");

  // Only parses if not epsilon and in first
  if (!opt_formals->isFirst(opt_formals, currentToken)) {
    return NULL; // Epsilon
  }

  // parse type
  debug("opt_formals calls type");
  const GrammarRule *type = get_rule("type");
  type->parse(type);

  // parse ID
  debug("checking id");
  char *id = capture_identifier();
  if (!id) {
    report_error(rule->name, "expected an ID but found NULL");
    exit(1);
  }
  if (chk_decl_flag) {
    debug("adding formal");
    if (!add_function_formal(id)) {
      report_error(rule->name, "failed to add formal to function");
      exit(1);
    }
  }
  debug("checking symbol");
  if (!add_symbol_check(id, "variable")) {
    report_error(rule->name, "failed to add formal to symbol table");
    exit(1);
  }

  // Parse formals
  debug("opt_formals calls formals");
  const GrammarRule *formals = get_rule("formals");
  formals->parse(formals);

  return NULL;
}

ASTnode *parse_formals_impl(const GrammarRule *rule) {
  const GrammarRule *formals = get_rule("formals");

  // Check first
  if (!formals->isFirst(formals, currentToken)) {
    return NULL; // Epsilon
  }

  // Parse COMMA
  if (!match(TOKEN_COMMA)) {
    report_error(rule->name, "expected a COMMA token");
    exit(1);
  }

  // Parse type
  debug("formals calls type");
  const GrammarRule *type = get_rule("type");
  type->parse(type);

  // Parse ID
  char *id = capture_identifier();
  if (chk_decl_flag) {
    if (!add_function_formal(id)) {
      report_error(rule->name, "failed to add formal to function");
      exit(1);
    }
  }
  if (!add_symbol_check(id, "variable")) {
    report_error(rule->name, "failed to add formal to symbol table");
    exit(1);
  }

  debug("formals calls formals");
  rule->parse(rule);

  return NULL;
}

ASTnode *parse_opt_var_decls_impl(const GrammarRule *rule) {
  const GrammarRule *opt_var_decls = get_rule("opt_var_decls");

  // check first
  if (!opt_var_decls->isFirst(opt_var_decls, currentToken)) {
    return NULL; // Epsilon
  }

  // parse type
  debug("opt_var_decls calls type");
  const GrammarRule *type = get_rule("type");
  type->parse(type);

  // parse ID
  char *id = capture_identifier();
  if (!add_symbol_check(id, "variable")) {
    report_error(rule->name, "failed to add id to symbol table");
    exit(1);
  }

  // parse var_decl
  debug("opt_var_decl calls var_decl");
  const GrammarRule *var_decl = get_rule("var_decl");
  var_decl->parse(var_decl);

  // Parse opt_var_decls
  rule->parse(rule);

  return NULL;
}

ASTnode *parse_opt_stmt_list_impl(const GrammarRule *rule) {
  if (!rule->isFirst(rule, currentToken)) {
    return NULL; // Epsilon
  }

  // Parse stmt
  debug("opt_stmt_list calls stmt");
  const GrammarRule *stmt = get_rule("stmt");
  ASTnode *stmt_node = stmt->parse(stmt);

  // Parse opt_stmt_list
  debug("opt_stmt_list calls opt_stmt_list");
  ASTnode *opt_stmt_list_node = rule->parse(rule);

  ASTnode *stmt_list_node =
      create_stmt_list_node(stmt_node, opt_stmt_list_node);

  return stmt_list_node;
}

ASTnode *parse_stmt_impl(const GrammarRule *rule) {
  const GrammarRule *stmt = get_rule("stmt");

  // check first
  if (!stmt->isFirst(stmt, currentToken)) {
    report_error(rule->name, "unexpected token in stmt");
    exit(1);
  }

  // Check assg_or_fn
  if (currentToken.type == TOKEN_ID) {
    const GrammarRule *assg_or_fn = get_rule("assg_or_fn");
    debug("stmt calls assg_or_fn");
    ASTnode *assg_or_fn_node = assg_or_fn->parse(assg_or_fn);
    return assg_or_fn_node;
  }

  // Check while_stmt
  const GrammarRule *while_stmt = get_rule("while_stmt");
  if (while_stmt->isFirst(while_stmt, currentToken)) {
    debug("stmt calls while_stmt");
    return while_stmt->parse(while_stmt);
  }

  // Check if_stmt
  const GrammarRule *if_stmt = get_rule("if_stmt");
  if (if_stmt->isFirst(if_stmt, currentToken)) {
    debug("stmt calls if_stmt");
    return if_stmt->parse(if_stmt);
  }

  // Check return_stmt
  const GrammarRule *return_stmt = get_rule("return_stmt");
  if (return_stmt->isFirst(return_stmt, currentToken)) {
    debug("stmt calls return_stmt");
    return return_stmt->parse(return_stmt);
  }

  // Match LBRACE
  if (match(TOKEN_LBRACE)) {
    // Parse opt_stmt_list
    const GrammarRule *opt_stmt_list = get_rule("opt_stmt_list");
    debug("stmt calls opt_stmt_list");
    ASTnode *opt_stmt_list_node = opt_stmt_list->parse(opt_stmt_list);

    // Parse RBRACE
    if (!match(TOKEN_RBRACE)) {
      report_error(rule->name, "expected RBRACE");
      exit(1);
    }
    return opt_stmt_list_node;
  }

  // Match SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected SEMI");
    exit(1);
  }

  return NULL;
}

ASTnode *parse_assg_or_fn_impl(const GrammarRule *rule) {
  token_data lookahead_token = peekToken();

  if (!rule->isFirst(rule, lookahead_token)) {
    report_error(rule->name, "token not part of assg_or_fn first set");
    exit(1);
  }

  if (lookahead_token.type == TOKEN_OPASSG) {
    debug("assg_or_fn calls assg_stmt");
    const GrammarRule *assg_stmt = get_rule("assg_stmt");
    return assg_stmt->parse(assg_stmt);
  }
  if (lookahead_token.type == TOKEN_LPAREN) {
    debug("assg_or_fn calls fn_call");
    const GrammarRule *fn_call = get_rule("fn_call");
    return fn_call->parse(fn_call);
  }

  exit(1);
}

ASTnode *parse_fn_call_impl(const GrammarRule *rule) {
  // Parse ID
  char *id = capture_identifier();
  Symbol *function_symbol = NULL;
  if (chk_decl_flag) {
    function_symbol = lookup_symbol_in_scope(id, "function", globalScope);
    if (!function_symbol) {
      report_error(rule->name, "ID does not exist");
      free(id);
      exit(1);
    }
  }

  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN");
    free(id);
  }

  int number_of_arguments = 0;
  if (chk_decl_flag) {
    number_of_arguments = function_symbol->number_of_arguments;
  }

  // Parse opt_expr_list
  debug("fn_call calls opt_expr_list");
  const GrammarRule *opt_expr_list = get_rule("opt_expr_list");
  ASTnode *expr_list_node =
      opt_expr_list->parseEx(opt_expr_list, function_symbol);

  if (chk_decl_flag) {
    if (strcmp(function_symbol->name, "println") == 0) {
      // continue
    } else if (function_symbol->number_of_arguments != 0) {
      report_error(rule->name, "wrong number of arguments provided");
      exit(1);
    }

    function_symbol->number_of_arguments = number_of_arguments;
  }

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN");
    free(id);
    exit(1);
  }

  // Parse SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected SEMI");
    free(id);
    exit(1);
  }

  ASTnode *fn_call_node =
      create_func_call_node(function_symbol, expr_list_node);

  free(id);
  return fn_call_node;
}

ASTnode *parse_opt_expr_list_impl(const GrammarRule *rule,
                                  Symbol *function_symbol) {

  if (!rule->isFirst(rule, currentToken)) {
    return NULL; // Epsilon
  }

  // Parse expr_list
  debug("opt_expr_list calls expr_list");
  const GrammarRule *expr_list = get_rule("expr_list");
  return expr_list->parseEx(expr_list, function_symbol);
}

ASTnode *parse_expr_list_impl(const GrammarRule *rule,
                              Symbol *function_symbol) {
  if (!rule->isFirst(rule, currentToken)) {
    report_error(rule->name, "unexpected token in expr_list");
    exit(1);
  }

  // Parse arith_exp
  debug("expr_list calls arith_exp");
  const GrammarRule *arith_exp = get_rule("arith_exp");
  ASTnode *arith_node = arith_exp->parseEx(arith_exp, function_symbol);

  ASTnode *opt_expr_list_node = NULL; // Parse optional COMMA
  if (match(TOKEN_COMMA)) {
    // Parse expr_list
    opt_expr_list_node = rule->parseEx(rule, function_symbol);
  }

  return create_expr_list_node(arith_node, opt_expr_list_node);
}

ASTnode *parse_arith_exp_impl(const GrammarRule *rule,
                              Symbol *callee_function_symbol) {
  if (!rule->isFirst(rule, currentToken)) {
    report_error(rule->name, "token not in arith_exp first set");
    exit(1);
  }

  // Check for ID
  if (currentToken.type == TOKEN_ID) {
    char *id = capture_identifier();
    Symbol *found_symbol = NULL;

    Symbol *current_defining_function_symbol = NULL;
    if (currentScope && currentScope->parent) {
      Symbol *sym_in_parent = currentScope->parent->symbols;
      while (sym_in_parent != NULL) {
        if (sym_in_parent->type &&
            strcmp(sym_in_parent->type, "function") == 0) {
          current_defining_function_symbol = sym_in_parent;
          break;
        }
        sym_in_parent = sym_in_parent->next;
      }
    }

    if (current_defining_function_symbol != NULL) {
      Symbol *formal = current_defining_function_symbol->arguments;
      while (formal != NULL) {
        if (strcmp(formal->name, id) == 0) {
          found_symbol = formal;
          break;
        }
        formal = formal->next;
      }
    }

    if (found_symbol == NULL) {
      found_symbol = lookup_symbol_in_table(id, "variable");
    }

    if (found_symbol == NULL) {
      report_error(rule->name, "could not find ID (parameter or variable)");
      free(id);
      exit(1);
    }

    if (callee_function_symbol != NULL) {
      int number_of_args = callee_function_symbol->number_of_arguments;
      if (chk_decl_flag && number_of_args <= 0 &&
          strcmp(callee_function_symbol->name, "println") != 0) {
        report_error(rule->name,
                     "too many arguments provided in function call");
        free(id);
        exit(1);
      }

      if (strcmp(callee_function_symbol->name, "println") != 0) {
        callee_function_symbol->number_of_arguments = number_of_args - 1;
      }
    }

    free(id);
    debug("return id node");
    return create_identifier_node(found_symbol);

  } else { // Handle Integer Constant
    if (currentToken.type != TOKEN_INTCON) {
      report_error(rule->name, "unexpected token in intcon");
      exit(1);
    }

    if (callee_function_symbol != NULL) {
      int number_of_args = callee_function_symbol->number_of_arguments;
      if (chk_decl_flag && number_of_args <= 0 &&
          strcmp(callee_function_symbol->name, "println") != 0) {
        report_error(rule->name, "too many arguments provided (constant)");
        exit(1);
      }
      if (strcmp(callee_function_symbol->name, "println") != 0) {
        callee_function_symbol->number_of_arguments = number_of_args - 1;
      }
    }

    debug("return intconst node");
    int number = 0;
    number = atoi(currentToken.lexeme);
    advanceToken();
    return create_intconst_node(number);
  }

  // Should not be reached
  exit(1);
}

ASTnode *parse_while_stmt_impl(const GrammarRule *rule) {
  if (!rule->isFirst(rule, currentToken)) {
    report_error(rule->name, "unexpected token in while_stmt");
    exit(1);
  }

  // Parse kwWHILE
  if (!match(TOKEN_KWWHILE)) {
    report_error(rule->name, "unexpected token in while_stmt");
    exit(1);
  }

  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN");
    exit(1);
  }

  // Parse bool_exp
  const GrammarRule *bool_exp = get_rule("bool_exp");
  ASTnode *bool_node = bool_exp->parse(bool_exp);

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN");
    exit(1);
  }

  // Parse stmt
  const GrammarRule *stmt = get_rule("stmt");
  ASTnode *stmt_node = stmt->parse(stmt);

  return create_while_node(bool_node, stmt_node);
}

ASTnode *parse_if_stmt_impl(const GrammarRule *rule) {
  const GrammarRule *if_stmt = get_rule("if_stmt");

  // Check first
  if (!if_stmt->isFirst(if_stmt, currentToken)) {
    report_error(rule->name, "unexpected token in if_stmt");
    exit(1);
  }

  // Parse if
  if (!match(TOKEN_KWIF)) {
    report_error(rule->name, "expected kwIF token");
    exit(1);
  }

  // Parse LPAREN
  if (!match(TOKEN_LPAREN)) {
    report_error(rule->name, "expected LPAREN");
    exit(1);
  }

  // Parse bool_exp
  debug("if_stmt calls bool_exp");
  const GrammarRule *bool_exp = get_rule("bool_exp");
  ASTnode *bool_node = bool_exp->parse(bool_exp);

  // Parse RPAREN
  if (!match(TOKEN_RPAREN)) {
    report_error(rule->name, "expected RPAREN");
    exit(1);
  }

  // Parse stmt
  const GrammarRule *stmt = get_rule("stmt");
  ASTnode *stmt_node = stmt->parse(stmt);

  // Parse optional else
  ASTnode *else_node = NULL;
  if (match(TOKEN_KWELSE)) {
    else_node = stmt->parse(stmt);
  }

  return create_if_node(bool_node, stmt_node, else_node);
}

ASTnode *parse_bool_exp_impl(const GrammarRule *rule) {
  // Check first
  if (!rule->isFirst(rule, currentToken)) {
    report_error(rule->name, "unexpected token in bool_exp");
    exit(1);
  }

  // Parse arith_exp
  debug("bool calling arith");
  const GrammarRule *arith_exp = get_rule("arith_exp");
  ASTnode *lhs_node = arith_exp->parseEx(arith_exp, NULL);

  // Parse relop
  debug("bool calling relop");
  const GrammarRule *relop = get_rule("relop");
  ASTnode *bool_node = relop->parse(relop);

  // Parse arith_exp
  debug("bool calling arith");
  ASTnode *rhs_node = arith_exp->parseEx(arith_exp, NULL);

  bool_node->child0 = lhs_node;
  bool_node->child1 = rhs_node;
  return bool_node;
}

ASTnode *parse_relop_impl(const GrammarRule *rule) {
  debug("parse_relop_impl");
  const GrammarRule *relop = get_rule("relop");
  // Check first
  if (!relop->isFirst(relop, currentToken)) {
    report_error(rule->name, "token not in relop first");
    exit(1);
  }

  if (match(TOKEN_OPEQ)) {
    debug("EQ");
    return create_eq_node(NULL, NULL);
  }

  if (match(TOKEN_OPNE)) {
    debug("NE");
    return create_ne_node(NULL, NULL);
  }

  if (match(TOKEN_OPLE)) {
    debug("LE");
    return create_le_node(NULL, NULL);
  }

  if (match(TOKEN_OPLT)) {
    debug("LT");
    return create_lt_node(NULL, NULL);
  }

  if (match(TOKEN_OPGE)) {
    debug("GE");
    return create_ge_node(NULL, NULL);
  }

  if (match(TOKEN_OPGT)) {
    debug("GT");
    return create_gt_node(NULL, NULL);
  }

  report_error(rule->name, "unexpected token in relop");
  exit(1);
}

ASTnode *parse_assg_stmt_impl(const GrammarRule *rule) {
  // Parse ID
  char *id = capture_identifier();

  // Lookup
  if (!lookup(id, "variable")) {
    report_error(rule->name, "ID does not exist");
    free(id);
    exit(1);
  }

  Symbol *id_name = lookup_symbol_in_table(id, "variable");

  if (id_name == NULL) {
    report_error(rule->name, "Could not find symbol");
    exit(1);
  }

  assert(strlen(id_name->name) != 0);

  ASTnode *identifier = create_identifier_node(id_name);

  // parse opASSG
  if (!match(TOKEN_OPASSG)) {
    report_error(rule->name, "expected opASSG");
    free(id);
    exit(1);
  }

  // parse arith_exp
  debug("assg_stmt calls arith_exp");
  const GrammarRule *arith_exp = get_rule("arith_exp");
  ASTnode *arith_node = arith_exp->parseEx(arith_exp, NULL);

  // parse SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected semi token");
    free(id);
    exit(1);
  }

  return create_assg_node(identifier, arith_node);
}

ASTnode *parse_return_stmt_impl(const GrammarRule *rule) {
  if (!rule->isFirst(rule, currentToken)) {
    report_error(rule->name, "unexpected token in return_stmt");
    exit(1);
  }

  // parse return
  if (!match(TOKEN_KWRETURN)) {
    report_error(rule->name, "unexpected token in return_stmt");
    exit(1);
  }

  // parse optional arith_exp
  ASTnode *arith_node = NULL;
  const GrammarRule *arith_exp = get_rule("arith_exp");
  if (arith_exp->isFirst(arith_exp, currentToken)) {
    // parse arith_exp
    debug("return calls arith_exp");
    arith_node = arith_exp->parseEx(arith_exp, NULL);
  }

  // parse semi
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected semi token");
    exit(1);
  }

  return create_return_node(arith_node);
}

// Variable declaration
ASTnode *parse_var_decl_impl(const GrammarRule *rule) {
  // Parsing id list
  debug("var_decl calls id_list");
  const GrammarRule *id_list = get_rule("id_list");
  id_list->parse(id_list);

  // Parse SEMI
  if (!match(TOKEN_SEMI)) {
    report_error(rule->name, "expected SEMI");
    exit(1);
  }

  return NULL;
}

// Type rule: type → 'int'
ASTnode *parse_type_impl(const GrammarRule *rule) {
  if (match(TOKEN_KWINT)) {
    return NULL;
  }

  report_error(rule->name, "expected type 'int'");
  exit(1);
}

// ID list rule: id_list → (',' ID)*
ASTnode *parse_id_list_impl(const GrammarRule *rule) {

  // Check first
  if (!rule->isFirst(rule, currentToken)) {
    return NULL; // Epsilon
  }

  // parse COMMA
  if (!match(TOKEN_COMMA)) {
    report_error(rule->name, "expected COMMA token");
    exit(1);
  }

  // parse ID
  char *id = capture_identifier();
  if (!add_symbol_check(id, "variable")) {
    report_error(rule->name, "token is ID but couldn't get name from lexeme");
    free(id);
    exit(1);
  }

  free(id);

  debug("id_list calls id_list");
  rule->parse(rule);

  return NULL;
}

// Initialize all grammar rules
void init_grammar_rules(void) {
  // Define FIRST and FOLLOW sets for rules

  // FIRST sets
  const token_type arith_exp_first[] = {TOKEN_ID, TOKEN_INTCON};
  const token_type assg_or_fn_first[] = {TOKEN_OPASSG, TOKEN_LPAREN};
  const token_type assg_stmt_first[] = {TOKEN_OPASSG};
  const token_type bool_exp_first[] = {TOKEN_ID, TOKEN_INTCON};
  const token_type decl_or_func_first[] = {TOKEN_COMMA, TOKEN_LPAREN,
                                          TOKEN_SEMI};
  const token_type expr_list_first[] = {TOKEN_ID, TOKEN_INTCON};
  const token_type fn_call_first[] = {TOKEN_LPAREN};
  const token_type formals_first[] = {TOKEN_COMMA};
  const token_type func_defn_first[] = {TOKEN_LPAREN};
  const token_type id_list_first[] = {TOKEN_COMMA};
  const token_type if_stmt_first[] = {TOKEN_KWIF};
  const token_type opt_expr_list_first[] = {TOKEN_ID, TOKEN_INTCON};
  const token_type opt_formals_first[] = {TOKEN_KWINT};
  const token_type opt_stmt_list_first[] = {TOKEN_ID,       TOKEN_KWIF,
                                           TOKEN_KWRETURN, TOKEN_LBRACE,
                                           TOKEN_SEMI,     TOKEN_KWWHILE};
  const token_type opt_var_decl_first[] = {TOKEN_KWINT};
  const token_type prog_first[] = {TOKEN_KWINT};
  const token_type relop_first[] = {TOKEN_OPEQ, TOKEN_OPNE, TOKEN_OPLE,
                                   TOKEN_OPLT, TOKEN_OPGE, TOKEN_OPGT};
  const token_type return_stmt_first[] = {TOKEN_KWRETURN};
  const token_type stmt_first[] = {TOKEN_ID,     TOKEN_KWIF, TOKEN_KWRETURN,
                                  TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE};
  const token_type type_first[] = {TOKEN_KWINT};
  const token_type var_decl_first[] = {TOKEN_COMMA, TOKEN_SEMI};
  const token_type while_stmt_first[] = {TOKEN_KWWHILE};

  // FOLLOW sets
  const token_type arith_exp_follow[] = {TOKEN_SEMI, TOKEN_OPEQ,  TOKEN_OPNE,
                                        TOKEN_OPLE, TOKEN_OPLT,  TOKEN_OPGE,
                                        TOKEN_OPGT, TOKEN_COMMA, TOKEN_RPAREN};
  const token_type assg_or_fn_follow[] = {
      TOKEN_KWELSE, TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN,
      TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};
  const token_type assg_stmt_follow[] = {
      TOKEN_KWELSE, TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN,
      TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};
  const token_type bool_exp_follow[] = {TOKEN_RPAREN};
  const token_type decl_or_func_follow[] = {TOKEN_KWINT, TOKEN_EOF};
  const token_type expr_list_follow[] = {TOKEN_RPAREN};
  const token_type fn_call_follow[] = {TOKEN_KWELSE,   TOKEN_ID,     TOKEN_KWIF,
                                      TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                      TOKEN_KWWHILE,  TOKEN_RBRACE};
  const token_type formals_follow[] = {TOKEN_RPAREN};
  const token_type func_defn_follow[] = {TOKEN_KWINT, TOKEN_EOF};
  const token_type id_list_follow[] = {TOKEN_SEMI};
  const token_type if_stmt_follow[] = {TOKEN_KWELSE,   TOKEN_ID,     TOKEN_KWIF,
                                      TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                      TOKEN_KWWHILE,  TOKEN_RBRACE};
  const token_type opt_expr_list_follow[] = {TOKEN_RPAREN};
  const token_type opt_formals_follow[] = {TOKEN_RPAREN};
  const token_type opt_stmt_list_follow[] = {TOKEN_RBRACE};
  const token_type opt_var_decls_follow[] = {
      TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN, TOKEN_LBRACE,
      TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};
  const token_type prog_follow[] = {TOKEN_EOF};
  const token_type relop_follow[] = {TOKEN_ID, TOKEN_INTCON};
  const token_type return_stmt_follow[] = {
      TOKEN_KWELSE, TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN,
      TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};
  const token_type stmt_follow[] = {TOKEN_KWELSE,   TOKEN_ID,     TOKEN_KWIF,
                                   TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                   TOKEN_KWWHILE,  TOKEN_RBRACE};
  const token_type type_follow[] = {TOKEN_ID};
  const token_type var_decl_follow[] = {TOKEN_KWINT,    TOKEN_ID,     TOKEN_KWIF,
                                       TOKEN_KWRETURN, TOKEN_LBRACE, TOKEN_SEMI,
                                       TOKEN_KWWHILE,  TOKEN_RBRACE, TOKEN_EOF};
  const token_type while_stmt_follow[] = {
      TOKEN_KWELSE, TOKEN_ID,   TOKEN_KWIF,    TOKEN_KWRETURN,
      TOKEN_LBRACE, TOKEN_SEMI, TOKEN_KWWHILE, TOKEN_RBRACE};

  // Create rules with their parsing functions
  create_rule("prog", prog_first, 1, prog_follow, 1, parse_prog_impl, false);
  create_rule("type", type_first, 1, type_follow, 1, parse_type_impl, false);
  create_rule("arith_exp", arith_exp_first, 2, arith_exp_follow, 9,
              parse_arith_exp_impl, true);
  create_rule("assg_or_fn", assg_or_fn_first, 2, assg_or_fn_follow, 8,
              parse_assg_or_fn_impl, false);
  create_rule("assg_stmt", assg_stmt_first, 1, assg_stmt_follow, 8,
              parse_assg_stmt_impl, false);
  create_rule("bool_exp", bool_exp_first, 2, bool_exp_follow, 1,
              parse_bool_exp_impl, false);
  create_rule("decl_or_func", decl_or_func_first, 3, decl_or_func_follow, 0,
              parse_decl_or_func_impl, false);
  create_rule("expr_list", expr_list_first, 2, expr_list_follow, 1,
              parse_expr_list_impl, true);
  create_rule("fn_call", fn_call_first, 1, fn_call_follow, 8,
              parse_fn_call_impl, false);
  create_rule("formals", formals_first, 1, formals_follow, 1,
              parse_formals_impl, false);
  create_rule("func_defn", func_defn_first, 1, func_defn_follow, 2,
              parse_func_defn_impl, false);
  create_rule("id_list", id_list_first, 1, id_list_follow, 1,
              parse_id_list_impl, false);
  create_rule("if_stmt", if_stmt_first, 1, if_stmt_follow, 8,
              parse_if_stmt_impl, false);
  create_rule("opt_expr_list", opt_expr_list_first, 2, opt_expr_list_follow, 1,
              parse_opt_expr_list_impl, true);
  create_rule("opt_formals", opt_formals_first, 1, opt_formals_follow, 1,
              parse_opt_formals_impl, false);
  create_rule("opt_stmt_list", opt_stmt_list_first, 6, opt_stmt_list_follow, 1,
              parse_opt_stmt_list_impl, false);
  create_rule("opt_var_decls", opt_var_decl_first, 1, opt_var_decls_follow, 7,
              parse_opt_var_decls_impl, false);
  create_rule("relop", relop_first, 6, relop_follow, 2, parse_relop_impl,
              false);
  create_rule("return_stmt", return_stmt_first, 1, return_stmt_follow, 8,
              parse_return_stmt_impl, false);
  create_rule("stmt", stmt_first, 6, stmt_follow, 8, parse_stmt_impl, false);
  create_rule("var_decl", var_decl_first, 1, var_decl_follow, 9,
              parse_var_decl_impl, false);
  create_rule("while_stmt", while_stmt_first, 1, while_stmt_follow, 8,
              parse_while_stmt_impl, false);
}
