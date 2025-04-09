#include "../src/features/parser/ast.h"
#include "../src/features/parser/grammar_rule.h"
#include "../src/features/parser/mips.h"
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
  reset_temp_counter();

  scanner_init_with_string(test_src);

  advanceToken();

  GrammarRule *prog = get_rule("prog");
  ASTnode *proj_node = prog->parse(prog);

  cleanup_grammar_rules();

  return proj_node;
}

ASTnode *continue_ast(char *test_src) {
  init_grammar_rules();

  scanner_init_with_string(test_src);

  advanceToken();

  GrammarRule *rule_2 = get_rule("prog");
  ASTnode *ast_node_2 = rule_2->parse(rule_2);

  cleanup_grammar_rules();

  return ast_node_2;
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

void test_quad_assignment() {
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

void test_quad_one_func_call() {
  char *test_source_code_1 = "int f(int x) { }";
  char *test_src_2 = "int main() { int x; x = 10; f(x); }";
  ASTnode *ast_input_1 = build_ast_for_quad_test(test_source_code_1);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input_1, &actual_code_list);

  ASTnode *ast_input_2 = continue_ast(test_src_2);
  make_TAC(ast_input_2, &actual_code_list);

  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter f\n"
                                       "leave f\n"
                                       "return\n"
                                       "enter main\n"
                                       "t0 = 10\n"
                                       "x = t0\n"
                                       "param x\n"
                                       "call f, 1\n"
                                       "leave main\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_quad_println() {
  char *test_source_code = "int main() { int y; y = 20; println(y); }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter main\n"
                                       "t0 = 20\n"
                                       "y = t0\n"
                                       "param y\n"
                                       "call println, 1\n"
                                       "leave main\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_quad_3_formals() {
  char *test_source_code = "int f(int x, int y, int z) { { { f(0, z, 1); } } }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter f\n"
                                       "t0 = 1\n"
                                       "param t0\n"
                                       "param z\n"
                                       "t1 = 0\n"
                                       "param t1\n"
                                       "call f, 3\n"
                                       "leave f\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_quad_variable_references() {

  char *test_source_code = "int f() { int x; int y; x = 10; y = x; }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter f\n"
                                       "t0 = 10\n"
                                       "x = t0\n"
                                       "y = x\n"
                                       "leave f\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_quad_println_with_integer() {

  char *test_source_code = "int main() { println(34567); }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter main\n"
                                       "t0 = 34567\n"
                                       "param t0\n"
                                       "call println, 1\n"
                                       "leave main\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_quad_println_chained_function_calls() {

  char *test_src_1 = "int g(int x) { println(x); }";
  char *test_src_2 = "int f(int x) { g(x); }";
  char *test_src_3 = "int main() { f(34567); }";

  ASTnode *ast_input_1 = build_ast_for_quad_test(test_src_1);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input_1, &actual_code_list);

  ASTnode *ast_input_2 = continue_ast(test_src_2);
  make_TAC(ast_input_2, &actual_code_list);

  ASTnode *ast_input_3 = continue_ast(test_src_3);
  make_TAC(ast_input_3, &actual_code_list);

  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter g\n"
                                       "param x\n"
                                       "call println, 1\n"
                                       "leave g\n"
                                       "return\n"

                                       "enter f\n"
                                       "param x\n"
                                       "call g, 1\n"
                                       "leave f\n"
                                       "return\n"

                                       "enter main\n"
                                       "t0 = 34567\n"
                                       "param t0\n"
                                       "call f, 1\n"
                                       "leave main\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_quad_global_variable() {

  char *test_source_code = "int x; int main() { println(34567); }";
  ASTnode *ast_input = build_ast_for_quad_test(test_source_code);

  Quad *actual_code_list = NULL;
  make_TAC(ast_input, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = quad_list_to_string(actual_code_list);

  const char *expected_output_string = "enter main\n"
                                       "t0 = 34567\n"
                                       "param t0\n"
                                       "call println, 1\n"
                                       "leave main\n"
                                       "return\n";

  assert(strcmp(actual_output_string, expected_output_string) == 0);

  free(actual_output_string);
}

void test_mips_func_defn() {
  char *test_src = "int f() { }";

  ASTnode *actual_ast = build_ast_for_quad_test(test_src);

  Quad *actual_code_list = NULL;
  make_TAC(actual_ast, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  MipsInstruction *mips_list = NULL;
  mips_list = generate_mips(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = mips_list_to_string(mips_list);

  char *expected_output_string = ".text\n"
                                 "_f:\n"
                                 "    la $sp, -8($sp)\n"
                                 "    sw $fp, 4($sp)\n"
                                 "    sw $ra, 0($sp)\n"
                                 "    la $fp, 0($sp)\n"
                                 "    la $sp, 0($sp)\n"

                                 "    la $sp, 0($fp)\n"
                                 "    lw $ra, 0($sp)\n"
                                 "    lw $fp, 4($sp)\n"
                                 "    la $sp, 8($sp)\n"

                                 "    jr $ra\n";

  assert(strcmp(expected_output_string, actual_output_string) == 0);
}

void test_mips_println() {
  char *test_src = "int main() { println(34567); }";

  ASTnode *actual_ast = build_ast_for_quad_test(test_src);

  Quad *actual_code_list = NULL;
  make_TAC(actual_ast, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  MipsInstruction *mips_list = NULL;
  mips_list = generate_mips(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = mips_list_to_string(mips_list);

  char *expected_output_string = ".text\n"
                                 "_main:\n"
                                 "    la $sp, -8($sp)\n"
                                 "    sw $fp, 4($sp)\n"
                                 "    sw $ra, 0($sp)\n"
                                 "    la $fp, 0($sp)\n"
                                 "    la $sp, 0($sp)\n"
                                 "    li $t0, 34567\n"
                                 "    la $sp, -4($sp)\n"
                                 "    sw $t0, 0($sp)\n"
                                 "    jal _println\n"
                                 "    la $sp, 4($sp)\n"
                                 "    la $sp, 0($fp)\n"
                                 "    lw $ra, 0($sp)\n"
                                 "    lw $fp, 4($sp)\n"
                                 "    la $sp, 8($sp)\n"
                                 "    jr $ra\n"

                                 ".align 2\n"
                                 ".data\n"
                                 "_nl: .asciiz \"\\n\"\n"
                                 ".align 2\n"
                                 ".text\n"
                                 "_println:\n"
                                 "    li $v0, 1\n"
                                 "    lw $a0, 0($sp)\n"
                                 "    syscall\n"
                                 "    li $v0, 4\n"
                                 "    la $a0, _nl\n"
                                 "    syscall\n"
                                 "    jr $ra\n"

                                 "\nmain: j _main\n";

  assert(strcmp(expected_output_string, actual_output_string) == 0);
}

void test_mips_global_variables() {

  char *test_src = "int x; int main() { }";

  ASTnode *actual_ast = build_ast_for_quad_test(test_src);

  Quad *actual_code_list = NULL;
  make_TAC(actual_ast, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  MipsInstruction *mips_list = NULL;
  mips_list = generate_mips(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = mips_list_to_string(mips_list);

  char *expected_output_string = ".data\n"
                                 ".align 2\n"
                                 "_x: .space 4\n"
                                 ".text\n"
                                 "_main:\n"
                                 "    la $sp, -8($sp)\n"
                                 "    sw $fp, 4($sp)\n"
                                 "    sw $ra, 0($sp)\n"
                                 "    la $fp, 0($sp)\n"
                                 "    la $sp, 0($sp)\n"

                                 "    la $sp, 0($fp)\n"
                                 "    lw $ra, 0($sp)\n"
                                 "    lw $fp, 4($sp)\n"
                                 "    la $sp, 8($sp)\n"

                                 "    jr $ra\n"

                                 "\nmain: j _main\n";

  assert(strcmp(expected_output_string, actual_output_string) == 0);
}

void test_mips_assign_global_variable() {

  char *test_src = "int x; int main() { x = 10; } ";

  ASTnode *actual_ast = build_ast_for_quad_test(test_src);

  Quad *actual_code_list = NULL;
  make_TAC(actual_ast, &actual_code_list);
  actual_code_list = reverse_tac_list(actual_code_list);

  MipsInstruction *mips_list = NULL;
  mips_list = generate_mips(actual_code_list);

  char *actual_output_string = NULL;
  actual_output_string = mips_list_to_string(mips_list);

  char *expected_output_string = ".data\n"
                                 ".align 2\n"
                                 "_x: .space 4\n"
                                 ".text\n"
                                 "_main:\n"
                                 "    la $sp, -8($sp)\n"
                                 "    sw $fp, 4($sp)\n"
                                 "    sw $ra, 0($sp)\n"
                                 "    la $fp, 0($sp)\n"
                                 "    la $sp, 0($sp)\n"

                                 "    li $t0, 10\n"
                                 "    sw $t0, _x\n"

                                 "    la $sp, 0($fp)\n"
                                 "    lw $ra, 0($sp)\n"
                                 "    lw $fp, 4($sp)\n"
                                 "    la $sp, 8($sp)\n"

                                 "    jr $ra\n"

                                 "\nmain: j _main\n";

  assert(strcmp(expected_output_string, actual_output_string) == 0);
}

int main(void) {
  test_quad_func_defn();
  test_quad_assignment();
  test_quad_one_func_call();
  test_quad_println();
  test_quad_3_formals();
  test_quad_variable_references();
  test_quad_println_with_integer();
  test_quad_println_chained_function_calls();
  test_quad_global_variable();
  test_mips_func_defn();
  test_mips_println();
  test_mips_global_variables();
  test_mips_assign_global_variable();
}
