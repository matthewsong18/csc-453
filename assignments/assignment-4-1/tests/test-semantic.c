// Cmocka
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>

// Headers
#include "../src/features/semantic/symbols.h"

static void test_allocate_symbol(void **state) {
  (void) state;

  Symbol *symbol = allocate_symbol();
  assert_non_null(symbol);
  assert_null(symbol->name);
  assert_int_equal(SYM_NULL, symbol->type);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_allocate_symbol),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}