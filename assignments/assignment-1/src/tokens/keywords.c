#include "../../include/scanner.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// INT token
static const char *match_kw_int(const char *input) {
  int compare = strncmp(input, "int", 3) == 0;
  int end_token = !isalnum(input[3]);
  return (compare && end_token) ? input + 3 : NULL;
}

const TokenMatch M_kwINT = {
    .match = match_kw_int, .name = "kwINT", .value = kwINT};

// IF token
static const char *match_kw_if(const char *input) {
  int compare = strncmp(input, "if", 2) == 0;
  int end_token = !isalnum(input[2]);
  return (compare && end_token) ? input + 2 : NULL;
}

const TokenMatch M_kwIF = {.match = match_kw_if, .name = "kwIF", .value = kwIF};

// ELSE token
static const char *match_kw_else(const char *input) {
  int compare = strncmp(input, "else", 4) == 0;
  int end_token = !isalnum(input[4]);
  return (compare && end_token) ? input + 4 : NULL;
}

const TokenMatch M_kwELSE = {
    .match = match_kw_else, .name = "kwELSE", .value = kwELSE};

// WHILE token
static const char *match_kw_while(const char *input) {
  int compare = strncmp(input, "while", 5) == 0;
  int end_token = !isalnum(input[5]);
  return (compare && end_token) ? input + 5 : NULL;
}

const TokenMatch M_kwWHILE = {
    .match = match_kw_while, .name = "kwWHILE", .value = kwWHILE};

// RETURN token
static const char *match_kw_return(const char *input) {
  int compare = strncmp(input, "return", 6) == 0;
  int end_token = !isalnum(input[6]);
  return (compare && end_token) ? input + 6 : NULL;
}

const TokenMatch M_kwRETURN = {
    .match = match_kw_return, .name = "kwRETURN", .value = kwRETURN};
