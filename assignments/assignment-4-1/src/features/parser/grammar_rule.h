// grammar_rule.h
#ifndef GRAMMAR_RULE_H
#define GRAMMAR_RULE_H

#include "ast.h"
#include "token_service.h"
#include <stdbool.h>

// Forward declaration to avoid circular dependency
typedef struct GrammarRule GrammarRule;

// Define function pointer types for clarity
typedef bool (*IsSetFn)(const GrammarRule *rule, TokenI token);
typedef ASTnode *(*ParseFn)(const GrammarRule *rule);
typedef ASTnode *(*ParseFnExtra)(const GrammarRule *rule, void *extra);

struct GrammarRule {
  // Function pointers
  IsSetFn isFirst;  // Function to check if token is in FIRST set
  IsSetFn isFollow; // Function to check if token is in FOLLOW set

  // Needed an extra argument for some parsing so I made this union so that
  // I could swap which function type I'm using without having to change
  // everything
  union {
    ParseFn parse;        // Function to parse this rule
    ParseFnExtra parseEx; // Function to parse this rule with an extra argument
  };

  // Data for FIRST and FOLLOW sets
  TokenType *firstSet;  // Array of token types in FIRST set
  int firstCount;       // Size of FIRST set
  TokenType *followSet; // Array of token types in FOLLOW set
  int followCount;      // Size of FOLLOW set

  const char *name; // Rule name for error reporting
};

// Function to create and initialize a grammar rule
GrammarRule *create_rule(const char *name, const TokenType *firstSet,
                         int firstCount, const TokenType *followSet,
                         int followCount, void *parseFn, bool hasExtraArg);

// Implementation of FIRST and FOLLOW checking
bool is_first_impl(const GrammarRule *rule, TokenI token);
bool is_follow_impl(const GrammarRule *rule, TokenI token);

// Function to check if a token is in a set
bool token_in_set(TokenI token, const TokenType *set, int count);

// Function to get a rule by name (for rule dependencies)
GrammarRule *get_rule(const char *name);

// Function to report parsing errors with context
void report_error(const char *ruleName, const char *message);

// Function to initialize all grammar rules
void init_grammar_rules(void);

// Function to clean up grammar rules
void cleanup_grammar_rules(void);

#endif
