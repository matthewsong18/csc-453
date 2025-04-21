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

// static void test_function_definition(void **state) {
//   (void)state;
//
//   const char *basic_ok_func = "int f() { }";
//   int parser_exit_result = test_parser(basic_ok_func);
//   assert_int_equal(0, parser_exit_result);
//
//   const char *basic_err_func = "int f) { }";
//   parser_exit_result = test_parser(basic_err_func);
//   assert_int_equal(1, parser_exit_result);
// }

bool DEBUG_ON = false;

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_global_variable),
    cmocka_unit_test(test_chained_global_variable),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}