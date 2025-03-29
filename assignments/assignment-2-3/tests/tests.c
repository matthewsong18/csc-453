#include "../src/features/parser/ast.h"
#include "../src/features/parser/grammar_rule.h"
#include "../src/features/parser/symbol_table.h"
#include "../src/features/parser/tac.h"
#include "../src/features/parser/token_service.h"
#include "../src/features/scanner/scanner.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int chk_decl_flag = 0;
int print_ast_flag = 0;
int gen_code_flag = 0;

ASTnode *build_ast_for_quad_test(char *test_src) {
  chk_decl_flag = 1;

  initSymbolTable();
  init_grammar_rules();

  scanner_init_with_string(test_src);

  advanceToken();

  GrammarRule *prog = get_rule("prog");
  ASTnode *proj_node = prog->parse(prog);

  cleanup_grammar_rules();

  return proj_node;
}

void test_quad_func_defn() {

  char *test_source_code = "int f() { }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter f\n"
                                       "leave f\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_assignment() {
  char *test_source_code = "int f() { int x; x = 10; }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter f\n"
                                       "t0 = 10\n"
                                       "x = t0\n"
                                       "leave f\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_three_address_code_generation() {
  char *test_source_code = "int f() { } int main() { int x; x = 10; f(x); }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter f\n"
                                       "leave f\n"
                                       "return\n"
                                       "enter main\n"
                                       "x = 10\n"
                                       "param x\n"
                                       "call f, 1\n"
                                       "leave main\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

int main(void) {
  test_quad_func_defn();
  test_assignment();
  test_three_address_code_generation();
}
