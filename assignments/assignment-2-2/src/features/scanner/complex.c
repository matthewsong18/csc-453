#include "scanner.h"
#include <ctype.h>
#include <string.h>

// ID token
static const char *match_id(const char *input) {
  if (!isalpha(input[0])) {
    return NULL; // Must start with a letter
  }

  int i = 1;
  while (isalnum(input[i])) {
    i++;
  }

  return input + i; // Return pointer to next unmatched character
}

// INTCON token
static const char *match_intcon(const char *input) {
  if (!isdigit(input[0])) {
    return NULL; // Must start with a digit
  }

  int i = 1;
  while (isdigit(input[i])) {
    i++;
  }

  return input + i; // Return pointer to next unmatched character
}

const TokenMatch M_ID = {.match = match_id, .name = "ID", .value = ID};
const TokenMatch M_INTCON = {
    .match = match_intcon, .name = "INTCON", .value = INTCON};
