
#include "simple-parse.h"
#include <stdlib.h>

static parse_data *create_parse_data(void) {
  parse_data *data = malloc(sizeof(parse_data));
  data->current_token = next_token();
  data->exit_status = 0;
  return data;
}

void update_data_to_next_token(parse_data *data) {
  data->current_token = next_token();
}

static int does_not_match(token_type expected_token,
                          token_data actual_token_data) {
  if (expected_token != actual_token_data.type) {
    return 1;
  }

  return 0;
}

static int in_first_set(token_type expected_token, parse_data *data) {
  if (does_not_match(expected_token, data->current_token)) {
    return 0;
  }

  return 1;
}

int bad_exit(const parse_data *data) { return data->exit_status == 1; }

static parse_data *parse_type(parse_data *data) {
  if (does_not_match(TOKEN_KWINT, data->current_token)) {
    data->exit_status = 1;
    return data;
  }
  update_data_to_next_token(data);

  return data;
}

parse_data *parse_var_decl_list_tail(parse_data *data) {
  // Loop as long as the next token is a comma
  while (!bad_exit(data) && in_first_set(TOKEN_COMMA, data)) {
    update_data_to_next_token(data); // Consume COMMA

    // After a comma, we expect a type (like kwINT)
    data = parse_type(data);
    if (bad_exit(data)) {
      // Error detected within parse_type (e.g., missing 'int')
      return data;
    }

    // After the type, we expect an ID
    if (does_not_match(TOKEN_ID, data->current_token)) {
      // Error: Missing ID after 'COMMA type' sequence
      // Handle the "int x, int ," case
      data->exit_status = 1;
      return data;
    }
    update_data_to_next_token(data); // Consume ID
  }

  // If the loop finishes (no more commas or an error occurred earlier),
  // return the current data state. This handles the epsilon case naturally.
  return data;
}

parse_data *parse_decl_prime(parse_data *data) {
  data = parse_var_decl_list_tail(data);
  if (bad_exit(data)) {
    return data;
  }

  if (does_not_match(TOKEN_SEMI, data->current_token)) {
    data->exit_status = 1;
    return data;
  }
  update_data_to_next_token(data);

  return data;
}
parse_data *parse_decl(parse_data *data) {
  data = parse_type(data);
  if (bad_exit(data)) {
    return data;
  }

  if (does_not_match(TOKEN_ID, data->current_token)) {
    data->exit_status = 1;
    return data;
  }
  update_data_to_next_token(data);

  data = parse_decl_prime(data);
  if (bad_exit(data)) {
    return data;
  }

  return data;
}

parse_data *parse_decl_list(parse_data *data) {
  // Check FIRST set for 'decl', which starts with 'type' (TOKEN_KWINT)
  while (!bad_exit(data) && in_first_set(TOKEN_KWINT, data)) {
    data = parse_decl(data);
  }
  return data;
}

int parse_prog() {
  parse_data *data = create_parse_data();
  return parse_decl_list(data)->exit_status;
}

int parse(void) { return parse_prog(); }