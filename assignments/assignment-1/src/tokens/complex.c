#include "../../include/scanner.h"
#include <ctype.h>
#include <string.h>

// ID token
static const char *match_id(const char *input) {
  // TODO
  return 0;
}

const TokenMatch M_ID = {.match = match_id, .name = "ID", .value = ID};

// INTCON token
static const char *match_intcon(const char *input) {
  // TODO
  return 0;
}

const TokenMatch M_INTCON = {
    .match = match_intcon, .name = "INTCON", .value = LPAREN};
