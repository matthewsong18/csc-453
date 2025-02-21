#include "parser_service.h"
#include "token_service.h"
#include <stdio.h>

int parse(void) {
  advanceToken();

  if (parse_prog()) {
    return 0;
  } else {
    return 1;
  }
}
