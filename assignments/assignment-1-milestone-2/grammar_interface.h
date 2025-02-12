#ifndef GRAMMAR_INTERFACE_H
#define GRAMMAR_INTERFACE_H

#include "./token_service.h"

// Forward declaration of GrammarRule.
typedef struct GrammarRule GrammarRule;

// Function pointer type for a parse function.
typedef bool (*ParseFunc)(GrammarRule *rule);

// GrammarRule "interface" structure.
struct GrammarRule {
    bool (*isFirst)(GrammarRule *rule, TokenI token);
    bool (*isFollow)(GrammarRule *rule, TokenI token);
    ParseFunc parse;  // Pointer to a parse function (if desired).
    TokenType *firstSet;
    int firstCount;
    TokenType *followSet;
    int followCount;
    const char *name; // For debugging purposes.
};

// Function prototypes for FIRST/FOLLOW checking.
bool tokenInSet(TokenI token, TokenType *set, int count);
bool isFirstImpl(GrammarRule *rule, TokenI token);
bool isFollowImpl(GrammarRule *rule, TokenI token);

#endif
