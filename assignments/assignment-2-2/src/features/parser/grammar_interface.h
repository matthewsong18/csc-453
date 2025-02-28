#ifndef GRAMMAR_INTERFACE_H
#define GRAMMAR_INTERFACE_H

#include "./token_service.h"

typedef struct GrammarRule GrammarRule;

struct GrammarRule {
    // Function pointers
    bool (*isFirst)(GrammarRule *rule, TokenI token);
    bool (*isFollow)(GrammarRule *rule, TokenI token);
    bool (*parse)(GrammarRule *rule);
    // Data: the arrays for FIRST and FOLLOW sets and their counts.
    TokenType *firstSet;
    int firstCount;
    TokenType *followSet;
    int followCount;
    const char *name; // Rule name
};

// Function prototypes for FIRST/FOLLOW checking.
bool tokenInSet(TokenI token, TokenType *set, int count);
bool isFirstImpl(GrammarRule *rule, TokenI token);
bool isFollowImpl(GrammarRule *rule, TokenI token);

#endif
