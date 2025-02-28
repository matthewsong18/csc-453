#include "grammar_interface.h"

bool tokenInSet(TokenI token, TokenType *set, int count) {
  for (int i = 0; i < count; i++) {
    if (token.type == set[i])
      return true;
  }
  return false;
}

bool isFirstImpl(GrammarRule *rule, TokenI token) {
  return tokenInSet(token, rule->firstSet, rule->firstCount);
}

bool isFollowImpl(GrammarRule *rule, TokenI token) {
  return tokenInSet(token, rule->followSet, rule->followCount);
}
