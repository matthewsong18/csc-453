// Cmocka
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// Relies upon above
#include <cmocka.h>

// Headers
#include "../src/features/semantic/symbols.h"

static void test_allocate_symbol(void **state) {
  (void)state;

  Symbol *symbol = allocate_symbol();
  assert_non_null(symbol);
  assert_null(symbol->name);
  assert_int_equal(SYM_NULL, symbol->type);
}

static void test_allocate_symbol_table(void **state) {
  (void)state;

  SymbolTable *symbol_table = allocate_symbol_table();

  assert_non_null(symbol_table);
  assert_non_null(symbol_table->symbols);
  assert_null(symbol_table->global_scope->head);
  assert_null(symbol_table->current_scope->head);
}

static void test_add_global_symbol(void **state) {
  (void)state;

  SymbolTable *symbol_table = allocate_symbol_table();
  const char *name = "main";
  const enum SymbolType type = SYM_VARIABLE;
  symbol_table = add_symbol(name, type, symbol_table);

  assert_string_equal(name, symbol_table->global_scope->head->name);
  assert_string_equal(name, symbol_table->global_scope->tail->name);
  assert_string_equal(name, symbol_table->current_scope->head->name);
  assert_string_equal(name, symbol_table->current_scope->tail->name);
  assert_string_equal(name, symbol_table->symbols->next->name);
  assert_int_equal(type, symbol_table->global_scope->head->type);
  assert_int_equal(type, symbol_table->global_scope->tail->type);
  assert_int_equal(type, symbol_table->current_scope->head->type);
  assert_int_equal(type, symbol_table->current_scope->tail->type);
  assert_int_equal(type, symbol_table->symbols->next->type);

  const char *name_two = "symbol_two";
  symbol_table = add_symbol(name_two, type, symbol_table);

  assert_string_equal(name_two, symbol_table->symbols->next->next->name);
  assert_string_equal(name_two, symbol_table->global_scope->head->next->name);
}

static void test_add_local_symbol(void **state) {
  (void)state;

  SymbolTable *symbol_table = allocate_symbol_table();
  const char *local_name = "local";
  const enum SymbolType type = SYM_VARIABLE;
  push_local_scope(symbol_table);
  symbol_table = add_symbol(local_name, type, symbol_table);
  const Symbol *local_symbol = symbol_table->current_scope->head;

  assert_null(get_global_scope(symbol_table)->head);
  assert_non_null(get_current_scope(symbol_table)->head);
  assert_string_equal(local_name, local_symbol->name);
}

static void test_pop_scope(void **state) {
  (void)state;

  SymbolTable *symbol_table = allocate_symbol_table();
  symbol_table = add_symbol("global", SYM_VARIABLE, symbol_table);
  push_local_scope(symbol_table);
  symbol_table = add_symbol("local", SYM_VARIABLE, symbol_table);

  assert_string_equal("local", symbol_table->current_scope->head->name);
  pop_local_scope(symbol_table);
  assert_string_equal("global", symbol_table->current_scope->head->name);
}

static void test_free_symbol_table(void **state) {
  (void)state;

  SymbolTable *symbol_table = allocate_symbol_table();
  symbol_table = add_symbol("global", SYM_VARIABLE, symbol_table);
  push_local_scope(symbol_table);
  symbol_table = add_symbol("local", SYM_VARIABLE, symbol_table);

  free_symbol_table(symbol_table);
  assert_int_equal(SYM_NULL, symbol_table->symbols);
  assert_null(symbol_table->global_scope);
  assert_null(symbol_table->current_scope);
}

static void test_add_function_formals(void **state) {
  (void)state;

  SymbolTable *symbol_table = allocate_symbol_table();
  const char *function_name = "main";
  const char *formal_1_name = "x";
  const char *formal_2_name = "y";

  symbol_table = add_symbol(function_name, SYM_FUNCTION, symbol_table);
  push_local_scope(symbol_table);
  symbol_table = add_formal(formal_1_name, symbol_table);
  symbol_table = add_formal(formal_2_name, symbol_table);

  const Symbol *function_symbol = symbol_table->symbols->next;
  const Symbol *formal_1 = find_formal(function_symbol, formal_1_name);
  const Symbol *formal_2 = find_formal(function_symbol, formal_2_name);

  assert_string_equal(formal_1_name, formal_1->name);
  assert_string_equal(formal_2_name, formal_2->name);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_allocate_symbol),
      cmocka_unit_test(test_allocate_symbol_table),
      cmocka_unit_test(test_add_global_symbol),
      cmocka_unit_test(test_add_local_symbol),
      cmocka_unit_test(test_pop_scope),
      cmocka_unit_test(test_free_symbol_table),
      cmocka_unit_test(test_add_function_formals),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}