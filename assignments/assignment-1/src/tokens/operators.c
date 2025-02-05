#include "../../include/scanner.h"
#include <ctype.h>
#include <string.h>

// ASSG token
static const char *match_op_assg(const char *input) {
  int compare = strncmp(input, "=", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opASSG = {
    .match = match_op_assg, .name = "opASSG", .value = opASSG};

// ADD token
static const char *match_op_add(const char *input) {
  int compare = strncmp(input, "+", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opADD = {
    .match = match_op_add, .name = "opADD", .value = opADD};

// SUB token
static const char *match_op_sub(const char *input) {
  int compare = strncmp(input, "-", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opSUB = {
    .match = match_op_sub, .name = "opSUB", .value = opSUB};

// MUL token
static const char *match_op_mul(const char *input) {
  int compare = strncmp(input, "*", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opMUL = {
    .match = match_op_mul, .name = "opMUL", .value = opMUL};

// DIV token
static const char *match_op_div(const char *input) {
  int compare = strncmp(input, "/", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opDIV = {
    .match = match_op_div, .name = "opDIV", .value = opDIV};

// EQ token
static const char *match_op_eq(const char *input) {
  int compare = strncmp(input, "==", 2) == 0;
  int end_token = !isalnum(input[2]);
  return (compare && end_token) ? input + 2 : NULL;
}

const TokenMatch M_opEQ = {.match = match_op_eq, .name = "opEQ", .value = opEQ};

// NE token
static const char *match_op_ne(const char *input) {
  int compare = strncmp(input, "!=", 2) == 0;
  int end_token = !isalnum(input[2]);
  return (compare && end_token) ? input + 2 : NULL;
}

const TokenMatch M_opNE = {.match = match_op_ne, .name = "opNE", .value = opNE};

// GT token
static const char *match_op_gt(const char *input) {
  int compare = strncmp(input, ">", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opGT = {.match = match_op_gt, .name = "opGT", .value = opGT};

// GE token
static const char *match_op_ge(const char *input) {
  int compare = strncmp(input, ">=", 2) == 0;
  int end_token = !isalnum(input[2]);
  return (compare && end_token) ? input + 2 : NULL;
}

const TokenMatch M_opGE = {.match = match_op_ge, .name = "opGE", .value = opGE};

// LT token
static const char *match_op_lt(const char *input) {
  int compare = strncmp(input, "<", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opLT = {.match = match_op_lt, .name = "opLT", .value = opLT};

// LE token
static const char *match_op_le(const char *input) {
  int compare = strncmp(input, "<=", 2) == 0;
  int end_token = !isalnum(input[2]);
  return (compare && end_token) ? input + 2 : NULL;
}

const TokenMatch M_opLE = {.match = match_op_le, .name = "opLE", .value = opLE};

// AND token
static const char *match_op_and(const char *input) {
  int compare = strncmp(input, "&&", 2) == 0;
  int end_token = !isalnum(input[2]);
  return (compare && end_token) ? input + 2 : NULL;
}

const TokenMatch M_opAND = {
    .match = match_op_and, .name = "opAND", .value = opAND};

// OR token
static const char *match_op_or(const char *input) {
  int compare = strncmp(input, "||", 2) == 0;
  int end_token = !isalnum(input[2]);
  return (compare && end_token) ? input + 2 : NULL;
}

const TokenMatch M_opOR = {.match = match_op_or, .name = "opOR", .value = opOR};

// NOT token
static const char *match_op_not(const char *input) {
  int compare = strncmp(input, "!", 1) == 0;
  int end_token = !isalnum(input[1]);
  return (compare && end_token) ? input + 1 : NULL;
}

const TokenMatch M_opNOT = {
    .match = match_op_not, .name = "opNOT", .value = opNOT};
