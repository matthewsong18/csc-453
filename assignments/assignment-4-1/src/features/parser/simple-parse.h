
#ifndef SIMPLE_PARSE_H
#define SIMPLE_PARSE_H

#include "token_service.h"

typedef struct {
  token_data current_token;
  int exit_status;
} parse_data;

int parse(void);

#endif //SIMPLE_PARSE_H
