#include "scanner.h"
#include <ctype.h>
#include <string.h>

static const char *match_single_char(const char *input, const char expected) {
  if (!input || !*input)
    return NULL;
  return (*input == expected) ? input + 1 : NULL;
}

static const char *match_two_char(const char *input, const char *expected) {
  if (!input || !*input)
    return NULL;
  int compare = strncmp(input, expected, 2) == 0;
  return (compare) ? input + 2 : NULL;
}

// ASSG token
static const char *match_op_assg(const char *input) {
  return match_single_char(input, '=');
}

// ADD token
static const char *match_op_add(const char *input) {
  return match_single_char(input, '+');
}

// SUB token
static const char *match_op_sub(const char *input) {
  return match_single_char(input, '-');
}

// MUL token
static const char *match_op_mul(const char *input) {
  return match_single_char(input, '*');
}

// DIV token
static const char *match_op_div(const char *input) {
  return match_single_char(input, '/');
}

// EQ token
static const char *match_op_eq(const char *input) {
  return match_two_char(input, "==");
}

// NE token
static const char *match_op_ne(const char *input) {
  return match_two_char(input, "!=");
}

// GT token
static const char *match_op_gt(const char *input) {
  return match_single_char(input, '>');
}

// GE token
static const char *match_op_ge(const char *input) {
  return match_two_char(input, ">=");
}

// LT token
static const char *match_op_lt(const char *input) {
  return match_single_char(input, '<');
}

// LE token
static const char *match_op_le(const char *input) {
  return match_two_char(input, "<=");
}

// AND token
static const char *match_op_and(const char *input) {
  return match_two_char(input, "&&");
}

// OR token
static const char *match_op_or(const char *input) {
  return match_two_char(input, "||");
}

// NOT token
static const char *match_op_not(const char *input) {
  return match_single_char(input, '!');
}

const TokenMatch M_opASSG = {
    .match = match_op_assg, .name = "opASSG", .value = opASSG};

const TokenMatch M_opADD = {
    .match = match_op_add, .name = "opADD", .value = opADD};

const TokenMatch M_opSUB = {
    .match = match_op_sub, .name = "opSUB", .value = opSUB};

const TokenMatch M_opMUL = {
    .match = match_op_mul, .name = "opMUL", .value = opMUL};

const TokenMatch M_opDIV = {
    .match = match_op_div, .name = "opDIV", .value = opDIV};

const TokenMatch M_opEQ = {.match = match_op_eq, .name = "opEQ", .value = opEQ};

const TokenMatch M_opNE = {.match = match_op_ne, .name = "opNE", .value = opNE};

const TokenMatch M_opGT = {.match = match_op_gt, .name = "opGT", .value = opGT};

const TokenMatch M_opGE = {.match = match_op_ge, .name = "opGE", .value = opGE};

const TokenMatch M_opLT = {.match = match_op_lt, .name = "opLT", .value = opLT};

const TokenMatch M_opLE = {.match = match_op_le, .name = "opLE", .value = opLE};

const TokenMatch M_opAND = {
    .match = match_op_and, .name = "opAND", .value = opAND};

const TokenMatch M_opOR = {.match = match_op_or, .name = "opOR", .value = opOR};

const TokenMatch M_opNOT = {
    .match = match_op_not, .name = "opNOT", .value = opNOT};
