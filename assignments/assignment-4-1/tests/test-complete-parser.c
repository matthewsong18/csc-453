#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>

#include "../src/features/scanner/scanner.h"
#include "../src/features/parser/simple-parse.h"

static int test_parser(const char * test_input_string) {
  scanner_init_with_string(test_input_string);

  return parse();
}

static void test_global_variable(void **state) {
  (void) state;

  const char *basic_ok_global_var = "int x;";
  int parser_exit_result = test_parser(basic_ok_global_var);
  assert_int_equal(0, parser_exit_result);

  const char *missing_id = "int ;";
  parser_exit_result = test_parser(missing_id);
  assert_int_equal(1, parser_exit_result);

  const char *missing_semi = "int x";
  parser_exit_result = test_parser(missing_semi);
  assert_int_equal(1, parser_exit_result);

  const char *change_variable_name = "int y;";
  parser_exit_result = test_parser(change_variable_name);
  assert_int_equal(0, parser_exit_result);

  const char *not_semi = "int x$";
  parser_exit_result = test_parser(not_semi);
  assert_int_equal(1, parser_exit_result);

  const char *multiple_valid_declarations = "int x; int y; int z;";
  parser_exit_result = test_parser(multiple_valid_declarations);
  assert_int_equal(0, parser_exit_result);

  const char *multiple_invalid_declarations = "int x int y int z;";
  parser_exit_result = test_parser(multiple_invalid_declarations);
  assert_int_equal(1, parser_exit_result);
}

static void test_chained_global_variable(void **state) {
  (void) state;

  const char *basic_valid = "int x, int y, int z;";
  int parser_exit_result = test_parser(basic_valid);
  assert_int_equal(0, parser_exit_result);

  const char *missing_id = "int x, int , int z;";
  parser_exit_result = test_parser(missing_id);
  assert_int_equal(1, parser_exit_result);

  const char *missing_semi = "int x, int y, int z";
  parser_exit_result = test_parser(missing_semi);
  assert_int_equal(1, parser_exit_result);

  const char *random_function_defn = "int x, int y, int f();";
  parser_exit_result = test_parser(random_function_defn);
  assert_int_equal(1, parser_exit_result);
}

static void test_function_definition(void **state) {
  (void)state;

  const char *basic_valid = "int f() { }";
  int parser_exit_result = test_parser(basic_valid);
  assert_int_equal(0, parser_exit_result);

  const char *different_id = "int main() { }";
  parser_exit_result = test_parser(different_id);
  assert_int_equal(0, parser_exit_result);

  const char *missing_type = "f() { }";
  parser_exit_result = test_parser(missing_type);
  assert_int_equal(0, parser_exit_result);

  const char *missing_id = "int () { }";
  parser_exit_result = test_parser(missing_id);
  assert_int_equal(1, parser_exit_result);

  const char *missing_l_paren = "int f) { }";
  parser_exit_result = test_parser(missing_l_paren);
  assert_int_equal(1, parser_exit_result);

  const char *missing_r_paren = "int f( { }";
  parser_exit_result = test_parser(missing_r_paren);
  assert_int_equal(1, parser_exit_result);

  const char *missing_l_brace = "int f()  }";
  parser_exit_result = test_parser(missing_l_brace);
  assert_int_equal(1, parser_exit_result);

  const char *missing_r_brace = "int f() { ";
  parser_exit_result = test_parser(missing_r_brace);
  assert_int_equal(1, parser_exit_result);
}

static void test_function_call(void **state) {
  (void) state;

  const char *basic_valid = "int f();";
  int parser_exit_result = test_parser(basic_valid);
  assert_int_equal(0, parser_exit_result);

  const char *missing_id = "int ();";
  parser_exit_result = test_parser(missing_id);
  assert_int_equal(1, parser_exit_result);

  const char *missing_semi = "int f()";
  parser_exit_result = test_parser(missing_semi);
  assert_int_equal(1, parser_exit_result);

  const char *different_id = "int main();";
  parser_exit_result = test_parser(different_id);
  assert_int_equal(0, parser_exit_result);
}

static void test_formals(void **state) {
  (void) state;

  const char *basic_valid = "int f(int x) { }";
  int parser_exit_result = test_parser(basic_valid);
  assert_int_equal(0, parser_exit_result);

  const char *chained_formals = "int f(int x, int y, int z) { }";
  parser_exit_result = test_parser(chained_formals);
  assert_int_equal(0, parser_exit_result);

  const char *function_call_formals = "int f(int x, int y, int z);";
  parser_exit_result = test_parser(function_call_formals);
  assert_int_equal(0, parser_exit_result);

  const char *missing_id = "int f(int ) { }";
  parser_exit_result = test_parser(missing_id);
  assert_int_equal(1, parser_exit_result);

  const char *missing_commas = "int f(int x int y int z) { }";
  parser_exit_result = test_parser(missing_commas);
  assert_int_equal(1, parser_exit_result);
}

static void test_assignment(void **state) {
  (void) state;

  const char *basic_valid = "int f() { x = 1; }";
  int parser_exit_result = test_parser(basic_valid);
  assert_int_equal(0, parser_exit_result);

  const char *different_id = "int f() { y = 1; }";
  parser_exit_result = test_parser(different_id);
  assert_int_equal(0, parser_exit_result);

  const char *different_const = "int f() { y = 3; }";
  parser_exit_result = test_parser(different_const);
  assert_int_equal(0, parser_exit_result);

  const char *missing_id = "int f() {  = 1; }";
  parser_exit_result = test_parser(missing_id);
  assert_int_equal(1, parser_exit_result);

  const char *missing_semi = "int f() { y = 1 }";
  parser_exit_result = test_parser(missing_semi);
  assert_int_equal(1, parser_exit_result);
}

static void test_if_statement(void **state) {
  (void) state;

  const char *less_equal = "int f() { if (y <= x) { } }";
  int parser_exit_result = test_parser(less_equal);
  assert_int_equal(0, parser_exit_result);

  const char *less_than = "int f() { if (y < x) { } }";
  parser_exit_result = test_parser(less_than);
  assert_int_equal(0, parser_exit_result);

  const char *greater_than = "int f() { if (y > x) { } }";
  parser_exit_result = test_parser(greater_than);
  assert_int_equal(0, parser_exit_result);

  const char *greater_equal = "int f() { if (y >= x) { } }";
  parser_exit_result = test_parser(greater_equal);
  assert_int_equal(0, parser_exit_result);

  const char *equal = "int f() { if (y == x) { } }";
  parser_exit_result = test_parser(equal);
  assert_int_equal(0, parser_exit_result);

  const char *not_equal = "int f() { if (x != z) { } }";
  parser_exit_result = test_parser(not_equal);
  assert_int_equal(0, parser_exit_result);

  const char *missing_parens = "int f() { if x == z { } }";
  parser_exit_result = test_parser(missing_parens);
  assert_int_equal(1, parser_exit_result);

  const char *invalid_operator = "int f() { if (x = z) { } }";
  parser_exit_result = test_parser(invalid_operator);
  assert_int_equal(0, parser_exit_result);

  const char *nested_if_statements = "int f() { if (x == z) { if (z != y) { } } }";
  parser_exit_result = test_parser(nested_if_statements);
  assert_int_equal(0, parser_exit_result);
}

static void test_while_statement(void **state) {
  (void) state;

  const char *valid = "int f() { while (z > y) { } }";
  int parser_exit_result = test_parser(valid);
  assert_int_equal(0, parser_exit_result);

  const char *bad_operator = "int f() { while (y === z) { } }";
  parser_exit_result = test_parser(bad_operator);
  assert_int_equal(1, parser_exit_result);
}

static void test_else_statement(void **state) {
  (void) state;

  const char *valid = "int f() { if (x > y) { } else { } }";
  int parser_exit_result = test_parser(valid);
  assert_int_equal(0, parser_exit_result);
}

static void test_local_variable(void **state) {
  (void) state;

  const char *valid = "int f() { int x; int y; int a, int b, int z; }";
  int parser_exit_result = test_parser(valid);
  assert_int_equal(0, parser_exit_result);

  const char *inside_if = "int f() { if (x > y) { int x; int y; } }";
  parser_exit_result = test_parser(inside_if);
  assert_int_equal(0, parser_exit_result);

  const char *inside_else = "int f() { if (x > y) { } else { int x; }}";
  parser_exit_result = test_parser(inside_else);
  assert_int_equal(0, parser_exit_result);

  const char *inside_while = "int f() { while (x > y) { int x; } }";
  parser_exit_result = test_parser(inside_while);
  assert_int_equal(0, parser_exit_result);
}

static void test_arithmetic_expressions(void **state) {
  (void) state;
  int parser_exit_result;

  // --- Basic Binary Operations ---
  const char *add = "int f() { x = a + 1; }";
  parser_exit_result = test_parser(add);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *sub = "int f() { x = a - b; }";
  parser_exit_result = test_parser(sub);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *mul = "int f() { x = 2 * c; }";
  parser_exit_result = test_parser(mul);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *div = "int f() { x = d / 3; }";
  parser_exit_result = test_parser(div);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Precedence (* / before + -) ---
  const char *prec1 = "int f() { x = a + b * c; }"; // Should parse as a + (b * c)
  parser_exit_result = test_parser(prec1);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *prec2 = "int f() { x = a * b - c; }"; // Should parse as (a * b) - c
  parser_exit_result = test_parser(prec2);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *prec3 = "int f() { x = a / b + c * d; }"; // Should parse as (a / b) + (c * d)
  parser_exit_result = test_parser(prec3);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Associativity (Left-to-right for +, -, *, /) ---
  const char *assoc1 = "int f() { x = a - b + c; }"; // Should parse as (a - b) + c
  parser_exit_result = test_parser(assoc1);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *assoc2 = "int f() { x = a / b * c; }"; // Should parse as (a / b) * c
  parser_exit_result = test_parser(assoc2);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Parentheses ---
  const char *paren1 = "int f() { x = (a + b) * c; }";
  parser_exit_result = test_parser(paren1);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *paren2 = "int f() { x = a * (b - c); }";
  parser_exit_result = test_parser(paren2);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *paren3 = "int f() { x = a / (b + c * d); }";
  parser_exit_result = test_parser(paren3);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Unary Minus (Needs Factor rule update) ---
  const char *unary1 = "int f() { x = -a; }";
  parser_exit_result = test_parser(unary1);
  assert_int_equal(0, parser_exit_result); // Expect success (Will fail until factor rule is updated)

  const char *unary2 = "int f() { x = b * -c; }"; // Needs precedence handling for unary vs binary
  parser_exit_result = test_parser(unary2);
  assert_int_equal(0, parser_exit_result); // Expect success (Will fail until factor rule is updated)

  // --- Error Cases ---
  const char *err1 = "int f() { x = a + ; }"; // Missing operand
  parser_exit_result = test_parser(err1);
  assert_int_equal(1, parser_exit_result); // Expect failure

  const char *err2 = "int f() { x = * b; }"; // Missing operand
  parser_exit_result = test_parser(err2);
  assert_int_equal(1, parser_exit_result); // Expect failure

  const char *err3 = "int f() { x = a + * b; }"; // Adjacent operators
  parser_exit_result = test_parser(err3);
  assert_int_equal(1, parser_exit_result); // Expect failure

  const char *err4 = "int f() { x = (a + b; }"; // Missing closing parenthesis
  parser_exit_result = test_parser(err4);
  assert_int_equal(1, parser_exit_result); // Expect failure

  const char *err5 = "int f() { x = a (+) b; }"; // Invalid operator placement
  parser_exit_result = test_parser(err5);
  assert_int_equal(1, parser_exit_result); // Expect failure
}

static void test_boolean_logic(void **state) {
  (void) state;
  int parser_exit_result;

  // --- Basic Logical Operations (Assumes Relational Ops Work) ---
  const char *and_op = "int f() { if (a < b && c > d) { } }";
  parser_exit_result = test_parser(and_op);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *or_op = "int f() { if (a == b || c != d) { } }";
  parser_exit_result = test_parser(or_op);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Precedence (&& before ||) ---
  const char *prec1 = "int f() { if (a || b && c) { } }"; // Should parse as a || (b && c)
  parser_exit_result = test_parser(prec1);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *prec2 = "int f() { if (a && b || c && d) { } }"; // Should parse as (a && b) || (c && d)
  parser_exit_result = test_parser(prec2);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Associativity (Left-to-right for &&, ||) ---
  const char *assoc1 = "int f() { if (a || b || c) { } }"; // Should parse as (a || b) || c
  parser_exit_result = test_parser(assoc1);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *assoc2 = "int f() { if (a && b && c) { } }"; // Should parse as (a && b) && c
  parser_exit_result = test_parser(assoc2);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Parentheses ---
  const char *paren1 = "int f() { if ((a || b) && c) { } }";
  parser_exit_result = test_parser(paren1);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *paren2 = "int f() { if (a && (b || c)) { } }";
  parser_exit_result = test_parser(paren2);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Mixed Relational / Logical ---
  const char *mixed1 = "int f() { if (x > y && y <= z) { } }";
  parser_exit_result = test_parser(mixed1);
  assert_int_equal(0, parser_exit_result); // Expect success

  const char *mixed2 = "int f() { if (x == y + 1 || z * 2 > 5) { } }"; // Requires working arith exp too
  parser_exit_result = test_parser(mixed2);
  assert_int_equal(0, parser_exit_result); // Expect success

  // --- Error Cases ---
  const char *err1 = "int f() { if (a && ) { } }"; // Missing operand
  parser_exit_result = test_parser(err1);
  assert_int_equal(1, parser_exit_result); // Expect failure

  const char *err2 = "int f() { if ( || b) { } }"; // Missing operand
  parser_exit_result = test_parser(err2);
  assert_int_equal(1, parser_exit_result); // Expect failure

  const char *err3 = "int f() { if (a && || b) { } }"; // Adjacent logical operators
  parser_exit_result = test_parser(err3);
  assert_int_equal(1, parser_exit_result); // Expect failure

  const char *err4 = "int f() { if ((a && b) { } }"; // Missing closing parenthesis
  parser_exit_result = test_parser(err4);
  assert_int_equal(1, parser_exit_result); // Expect failure
}

bool DEBUG_ON = false;

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_global_variable),
    cmocka_unit_test(test_chained_global_variable),
    cmocka_unit_test(test_function_definition),
    cmocka_unit_test(test_function_call),
    cmocka_unit_test(test_formals),
    cmocka_unit_test(test_assignment),
    cmocka_unit_test(test_if_statement),
    cmocka_unit_test(test_while_statement),
    cmocka_unit_test(test_else_statement),
    cmocka_unit_test(test_local_variable),
    cmocka_unit_test(test_arithmetic_expressions),
    cmocka_unit_test(test_boolean_logic),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}