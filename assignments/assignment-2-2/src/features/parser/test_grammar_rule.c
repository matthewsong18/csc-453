#include "grammar_rule.h"
#include "token_service.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_create_rule() {
  // Define test data
  const char *ruleName = "testRule";
  const TokenType firstSet[] = {TOKEN_ID, TOKEN_EOF};
  const int firstCount = 2;
  const TokenType followSet[] = {TOKEN_KWIF, TOKEN_OPGE};
  const int followCount = 2;
  const ParseFn parseFn = NULL; // No parse for now

  // Create the rule
  GrammarRule *rule = create_rule(ruleName, firstSet, firstCount, followSet,
                                  followCount, parseFn);

  // Verify the rule was created successfully
  assert(rule != NULL);
  assert(strcmp(rule->name, ruleName) == 0);
  assert(rule->firstCount == firstCount);
  assert(rule->followCount == followCount);
  assert(rule->parse == parseFn);

  // Verify the FIRST set
  for (int i = 0; i < firstCount; i++) {
    assert(rule->firstSet[i] == firstSet[i]);
  }

  // Verify the FOLLOW set
  for (int i = 0; i < followCount; i++) {
    assert(rule->followSet[i] == followSet[i]);
  }

  // Clean up
  cleanup_grammar_rules();

  printf("test_create_rule passed\n");
}

int test_grammar_rule() {
  test_create_rule();
  return 0;
}