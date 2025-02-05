#include "../../include/scanner.h"
#include <ctype.h>
#include <string.h>

static const char *match_single_char(const char *input, const char expected) {
    if (!input || !*input) return NULL;
    return (*input == expected) ? input + 1 : NULL;
}

static const char *match_lparen(const char *input) {
    return match_single_char(input, '(');
}

static const char *match_rparen(const char *input) {
    return match_single_char(input, ')');
}

static const char *match_lbrace(const char *input) {
    return match_single_char(input, '{');
}

static const char *match_rbrace(const char *input) {
    return match_single_char(input, '}');
}

static const char *match_comma(const char *input) {
    return match_single_char(input, ',');
}

static const char *match_semi(const char *input) {
    return match_single_char(input, ';');
}

const TokenMatch M_LPAREN = {.match = match_lparen, .name = "LPAREN", .value = LPAREN};
const TokenMatch M_RPAREN = {.match = match_rparen, .name = "RPAREN", .value = RPAREN};
const TokenMatch M_LBRACE = {.match = match_lbrace, .name = "LBRACE", .value = LBRACE};
const TokenMatch M_RBRACE = {.match = match_rbrace, .name = "RBRACE", .value = RBRACE};
const TokenMatch M_COMMA = {.match = match_comma, .name = "COMMA", .value = COMMA};
const TokenMatch M_SEMI = {.match = match_semi, .name = "SEMI", .value = SEMI};
