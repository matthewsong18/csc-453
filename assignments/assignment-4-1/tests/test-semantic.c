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

static void test_allocate_symbol_table(void **state) {
  (void) state;

  SymbolTable *symbol_table = allocate_symbol_table();

  assert_non_null(symbol_table);
  assert_null(symbol_table->symbols);
  assert_null(symbol_table->global_scope->head);
  assert_null(symbol_table->current_scope->head);
}

static void test_add_global_symbol(void **state) {
  (void) state;

  SymbolTable *symbol_table = allocate_symbol_table();
  char *name = "main";
  const enum SymbolType type = SYM_VARIABLE;
  symbol_table = add_symbol(name, type, symbol_table);

  assert_string_equal(name, symbol_table->global_scope->head->name);
  assert_string_equal(name, symbol_table->global_scope->tail->name);
  assert_string_equal(name, symbol_table->current_scope->head->name);
  assert_string_equal(name, symbol_table->current_scope->tail->name);
  assert_string_equal(name, symbol_table->symbols->name);
  assert_int_equal(type, symbol_table->global_scope->head->type);
  assert_int_equal(type, symbol_table->global_scope->tail->type);
  assert_int_equal(type, symbol_table->current_scope->head->type);
  assert_int_equal(type, symbol_table->current_scope->tail->type);
  assert_int_equal(type, symbol_table->symbols->type);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_allocate_symbol),
    cmocka_unit_test(test_allocate_symbol_table),
    cmocka_unit_test(test_add_global_symbol),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}