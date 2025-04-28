// grammar_rule.c
#include "grammar_rule.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum number of grammar rules
#define MAX_RULES 50

// Array to store all grammar rules
static GrammarRule **rules = NULL;
static int rule_count = 0;

bool token_in_set(const token_data token, const token_type *set, const int count) {
  for (int i = 0; i < count; i++) {
    if (token.type == set[i])
      return true;
  }
  return false;
}

bool is_first_impl(const GrammarRule *rule, const token_data token) {
  return token_in_set(token, rule->firstSet, rule->firstCount);
}

bool is_follow_impl(const GrammarRule *rule, const token_data token) {
  return token_in_set(token, rule->followSet, rule->followCount);
}

// Function to report parsing errors with context
void report_error(const char *ruleName, const char *message) {
  fprintf(stderr, "ERROR: LINE %d: %s in rule '%s' and current token '%d:%s'\n",
          currentToken.line, message, ruleName, currentToken.type,
          currentToken.lexeme);
}

// Helper function to allocate a token set
static token_type *allocate_token_set(const token_type *sourceSet,
                                     const int count) {
  if (count <= 0) {
    return NULL;
  }

  token_type *set = malloc(count * sizeof(token_type));
  if (!set) {
    fprintf(stderr, "ERROR: memory allocation failure for token set\n");
    return NULL;
  }

  for (int i = 0; i < count; i++) {
    set[i] = sourceSet[i];
  }
  return set;
}

// Helper function to initialize a rule structure
static GrammarRule *initialize_rule(const char *name, void *parseFn,
                                    bool hasExtraArg) {
  GrammarRule *rule = malloc(sizeof(GrammarRule));
  if (!rule) {
    fprintf(stderr, "ERROR: memory allocation failure for grammar rule\n");
    return NULL;
  }

  // Initialize function pointers
  rule->isFirst = is_first_impl;
  rule->isFollow = is_follow_impl;
  if (hasExtraArg) {
    rule->parseEx = parseFn;
  } else {
    rule->parse = parseFn;
  }

  // Initialize name
  rule->name = strdup(name);
  if (!rule->name) {
    fprintf(stderr, "ERROR: memory allocation failure for rule name\n");
    free(rule);
    return NULL;
  }

  // Initialize sets to NULL initially
  rule->firstSet = NULL;
  rule->followSet = NULL;
  rule->firstCount = 0;
  rule->followCount = 0;

  return rule;
}

// Helper function to add a rule to the global rules array
static bool add_rule_to_registry(GrammarRule *rule) {
  if (rules == NULL) {
    rules = (GrammarRule **)malloc(MAX_RULES * sizeof(GrammarRule *));
    if (!rules) {
      fprintf(stderr, "ERROR: memory allocation failure for rules array\n");
      return false;
    }
  }

  if (rule_count < MAX_RULES) {
    rules[rule_count++] = rule;
    return true;
  } else {
    fprintf(stderr, "ERROR: exceeded maximum number of grammar rules\n");
    return false;
  }
}

// Helper function to clean up a rule on failure
static void cleanup_rule(GrammarRule *rule) {
  if (rule) {
    free(rule->firstSet);
    free(rule->followSet);
    free((char *)rule->name);
    free(rule);
  }
}

// Function to create and initialize a grammar rule
GrammarRule *create_rule(const char *name, const token_type *firstSet,
                         const int firstCount, const token_type *followSet,
                         const int followCount, void *parseFn,
                         bool hasExtraArg) {
  // Initialize the basic rule structure
  GrammarRule *rule = initialize_rule(name, parseFn, hasExtraArg);
  if (!rule)
    return NULL;

  // Set up FIRST set
  rule->firstCount = firstCount;
  rule->firstSet = allocate_token_set(firstSet, firstCount);
  if (firstCount > 0 && !rule->firstSet) {
    cleanup_rule(rule);
    return NULL;
  }

  // Set up FOLLOW set
  rule->followCount = followCount;
  rule->followSet = allocate_token_set(followSet, followCount);
  if (followCount > 0 && !rule->followSet) {
    cleanup_rule(rule);
    return NULL;
  }

  // Add rule to registry
  if (!add_rule_to_registry(rule)) {
    cleanup_rule(rule);
    return NULL;
  }

  return rule;
}

// Function to get a rule by name
GrammarRule *get_rule(const char *name) {
  for (int i = 0; i < rule_count; i++) {
    if (strcmp(rules[i]->name, name) == 0)
      return rules[i];
  }
  fprintf(stderr, "ERROR: rule '%s' not found\n", name);
  return NULL;
}

// Function to clean up grammar rules
void cleanup_grammar_rules(void) {
  if (rules) {
    for (int i = 0; i < rule_count; i++) {
      if (rules[i]) {
        free(rules[i]->firstSet);
        free(rules[i]->followSet);
        free((char *)rules[i]->name);
        free(rules[i]);
      }
    }
    free(rules);
    rules = NULL;
    rule_count = 0;
  }
}
