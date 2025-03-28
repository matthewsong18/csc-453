// parser_interface.c
#include "./grammar_rule.h"
#include "./symbol_table.h"
#include "./token_service.h"
#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// External flags from driver
extern int chk_decl_flag;
extern int print_ast_flag;
extern int gen_code_flag;

// Global variables
Scope *globalScope = NULL;
Scope *currentScope = NULL;

// Function to perform parsing with grammar rules
ASTnode *parse_with_grammar_rules() {
  initSymbolTable();
  init_grammar_rules();
  advanceToken();

  GrammarRule *prog = get_rule("prog");
  ASTnode *proj_node = prog->parse(prog);

  cleanup_grammar_rules();
  return proj_node;
}

int parse(void) {
  if (print_ast_flag) {
    chk_decl_flag = 1;
  }

  parse_with_grammar_rules();
  return 0; // Success
}
