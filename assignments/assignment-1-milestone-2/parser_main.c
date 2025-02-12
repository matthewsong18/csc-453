#include "parser_service.h"
#include "token_service.h"
#include <stdio.h>

int main(void) {
  advanceToken();

  if (parse_prog()) {
    printf("Parsing succeeded.\n");
    return 0;
  } else {
    printf("Parsing failed.\n");
    return 1;
  }
}
