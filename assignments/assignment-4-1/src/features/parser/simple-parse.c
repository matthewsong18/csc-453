#include "simple-parse.h"
#include <stdlib.h>

// Forward Declarations
parse_data *parse_decl(parse_data *data);
parse_data *parse_decl_prime(parse_data *data);
parse_data *parse_var_decl_list_tail(parse_data *data);
parse_data *parse_func_defn(parse_data *data);
parse_data *parse_opt_formals(parse_data *data);
parse_data *parse_formals_list(parse_data *data);
parse_data *parse_formals_list_tail(parse_data *data);
parse_data *parse_opt_var_decls(parse_data *data);
parse_data *parse_stmt_list(parse_data *data);
parse_data *parse_stmt(parse_data *data);
parse_data *parse_if_stmt(parse_data *data);
parse_data *parse_else_part(parse_data *data);
parse_data *parse_while_stmt(parse_data *data);
parse_data *parse_stmt_after_id(parse_data *data);
parse_data *parse_opt_actuals(parse_data *data);
parse_data *parse_arith_exp(parse_data *data);
parse_data *parse_term(parse_data *data);
parse_data *parse_factor(parse_data *data);
parse_data *parse_primary_exp(parse_data *data);
parse_data *parse_arith_exp_prime(parse_data *data);
parse_data *parse_term_prime(parse_data *data);
parse_data *parse_bool_exp(parse_data *data);         // Added
parse_data *parse_bool_exp_prime(parse_data *data);   // Added
parse_data *parse_and_exp(parse_data *data);          // Added
parse_data *parse_and_exp_prime(parse_data *data);    // Added
parse_data *parse_relational_exp(parse_data *data);   // Added
static parse_data *parse_type(parse_data *data);


static parse_data *create_parse_data(void) {
  parse_data *data = malloc(sizeof(parse_data));
  if (!data) exit(EXIT_FAILURE);
  data->current_token = next_token();
  data->exit_status = 0;
  return data;
}

void update_data_to_next_token(parse_data *data) {
  data->current_token = next_token();
}

static int does_not_match(const token_type expected_token,
                          const token_data actual_token_data) {
  if (expected_token != actual_token_data.type) {
    return 1;
  }
  return 0;
}

static int in_first_set(const token_type expected_token,
                        const parse_data *data) {
  if (!data) return 0;
  return !does_not_match(expected_token, data->current_token);
}

int bad_exit(const parse_data *data) {
    if (!data) return 1;
    return data->exit_status == 1;
}


static parse_data *parse_type(parse_data *data) {
  if (does_not_match(TOKEN_KWINT, data->current_token)) {
    data->exit_status = 1;
    return data;
  }
  update_data_to_next_token(data);
  return data;
}


parse_data *parse_var_decl_list_tail(parse_data *data) {
  while (!bad_exit(data) && in_first_set(TOKEN_COMMA, data)) {
    update_data_to_next_token(data);
    data = parse_type(data);
    if (bad_exit(data)) {
      return data;
    }
    if (does_not_match(TOKEN_ID, data->current_token)) {
      data->exit_status = 1;
      return data;
    }
    update_data_to_next_token(data);
  }
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


parse_data *parse_opt_var_decls(parse_data *data) {
  while (!bad_exit(data) && in_first_set(TOKEN_KWINT, data)) {
    data = parse_decl(data);
  }
  return data;
}


parse_data *parse_formals_list_tail(parse_data *data) {
  while (!bad_exit(data) && in_first_set(TOKEN_COMMA, data)) {
    update_data_to_next_token(data);
    data = parse_type(data);
    if (bad_exit(data)) {
      data->exit_status = 1;
      return data;
    }
    if (does_not_match(TOKEN_ID, data->current_token)) {
      data->exit_status = 1;
      return data;
    }
    update_data_to_next_token(data);
  }
  return data;
}

parse_data *parse_formals_list(parse_data *data) {
  data = parse_type(data);
  if (bad_exit(data)) {
    return data;
  }
  if (does_not_match(TOKEN_ID, data->current_token)) {
    data->exit_status = 1;
    return data;
  }
  update_data_to_next_token(data);
  data = parse_formals_list_tail(data);
  return data;
}

parse_data *parse_opt_formals(parse_data *data) {
  if (in_first_set(TOKEN_KWINT, data)) {
    data = parse_formals_list(data);
  }
  return data;
}


parse_data *parse_func_defn(parse_data *data) {
  if (does_not_match(TOKEN_LPAREN, data->current_token)) {
    data->exit_status = 1;
    return data;
  }
  update_data_to_next_token(data);

  data = parse_opt_formals(data);
  if (bad_exit(data)) {
    return data;
  }

  if (does_not_match(TOKEN_RPAREN, data->current_token)) {
    data->exit_status = 1;
    return data;
  }
  update_data_to_next_token(data);

  if (in_first_set(TOKEN_LBRACE, data)) {
    update_data_to_next_token(data);
    data = parse_opt_var_decls(data);
    if (bad_exit(data)) {
      return data;
    }
    data = parse_stmt_list(data);
    if (bad_exit(data)) {
      return data;
    }
    if (does_not_match(TOKEN_RBRACE, data->current_token)) {
      data->exit_status = 1;
      return data;
    }
    update_data_to_next_token(data);
  } else if (in_first_set(TOKEN_SEMI, data)) {
    update_data_to_next_token(data);
  } else {
    data->exit_status = 1;
    return data;
  }
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

  if (in_first_set(TOKEN_LPAREN, data)) {
    data = parse_func_defn(data);
  } else {
    data = parse_decl_prime(data);
  }
  return data;
}

parse_data *parse_decl_list(parse_data *data) {
  while (!bad_exit(data) && in_first_set(TOKEN_KWINT, data)) {
    data = parse_decl(data);
  }
  return data;
}


parse_data *parse_stmt_list(parse_data *data) {
  while (!bad_exit(data) &&
         (in_first_set(TOKEN_ID, data) ||
          in_first_set(TOKEN_SEMI, data) ||
          in_first_set(TOKEN_KWIF, data) ||
          in_first_set(TOKEN_KWWHILE, data) ||
          in_first_set(TOKEN_LBRACE, data)
          /* || in_first_set(TOKEN_KWRETURN, data) */ )) {
    data = parse_stmt(data);
  }
  return data;
}

parse_data *parse_stmt(parse_data *data) {
    if (in_first_set(TOKEN_ID, data)) {
        update_data_to_next_token(data);
        data = parse_stmt_after_id(data);
    }
    else if (in_first_set(TOKEN_KWIF, data)) {
        data = parse_if_stmt(data);
    }
    else if (in_first_set(TOKEN_KWWHILE, data)) {
        data = parse_while_stmt(data);
    }
    // else if (in_first_set(TOKEN_KWRETURN, data)) { /* TODO */ }
    else if (in_first_set(TOKEN_LBRACE, data)) {
        update_data_to_next_token(data);
        data = parse_opt_var_decls(data);
        if (bad_exit(data)) { return data; }
        data = parse_stmt_list(data);
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_RBRACE, data->current_token)) {
            data->exit_status = 1; return data;
        }
        update_data_to_next_token(data);
    }
     else if (in_first_set(TOKEN_SEMI, data)) {
         update_data_to_next_token(data);
    }
     else {
         data->exit_status = 1;
     }
    return data;
}

parse_data *parse_stmt_after_id(parse_data *data) {
    if (in_first_set(TOKEN_OPASSG, data)) {
         update_data_to_next_token(data);
        data = parse_arith_exp(data); // Use full arith exp parser now
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_SEMI, data->current_token)) {
            data->exit_status = 1; return data;
        }
        update_data_to_next_token(data);
    } else if (in_first_set(TOKEN_LPAREN, data)) {
         update_data_to_next_token(data);
        data = parse_opt_actuals(data);
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_RPAREN, data->current_token)) {
            data->exit_status = 1; return data;
        }
        update_data_to_next_token(data);
        if (does_not_match(TOKEN_SEMI, data->current_token)) {
            data->exit_status = 1; return data;
        }
        update_data_to_next_token(data);
    } else {
         data->exit_status = 1;
    }
    return data;
}

parse_data *parse_if_stmt(parse_data *data) {
     update_data_to_next_token(data);
    if (does_not_match(TOKEN_LPAREN, data->current_token)) {
         data->exit_status = 1; return data;
     }
     update_data_to_next_token(data);
    data = parse_bool_exp(data); // Use bool_exp for condition
    if (bad_exit(data)) { return data; }
    if (does_not_match(TOKEN_RPAREN, data->current_token)) {
         data->exit_status = 1; return data;
     }
     update_data_to_next_token(data);
    data = parse_stmt(data);
    if (bad_exit(data)) { return data; }
     data = parse_else_part(data);
     return data;
}

parse_data *parse_else_part(parse_data *data) {
     if (in_first_set(TOKEN_KWELSE, data)) {
         update_data_to_next_token(data);
         data = parse_stmt(data);
         if (bad_exit(data)) { return data; }
     }
     return data;
 }

parse_data *parse_while_stmt(parse_data *data) {
    update_data_to_next_token(data);
    if (does_not_match(TOKEN_LPAREN, data->current_token)) {
        data->exit_status = 1; return data;
    }
    update_data_to_next_token(data);
    data = parse_bool_exp(data); // Use bool_exp for condition
    if (bad_exit(data)) { return data; }
    if (does_not_match(TOKEN_RPAREN, data->current_token)) {
        data->exit_status = 1; return data;
    }
    update_data_to_next_token(data);
    data = parse_stmt(data);
    return data;
}


parse_data *parse_opt_actuals(parse_data *data) {
  return data;
}


parse_data *parse_primary_exp(parse_data *data) {
     if (in_first_set(TOKEN_ID, data)) {
         update_data_to_next_token(data);
     } else if (in_first_set(TOKEN_INTCON, data)) {
         update_data_to_next_token(data);
     } else if (in_first_set(TOKEN_LPAREN, data)) {
         update_data_to_next_token(data);
         data = parse_arith_exp(data);
         if (bad_exit(data)) { return data; }
         if (does_not_match(TOKEN_RPAREN, data->current_token)) {
             data->exit_status = 1; return data;
         }
         update_data_to_next_token(data);
     }
     else {
         data->exit_status = 1;
     }
     return data;
 }

parse_data *parse_factor(parse_data *data) {
    if (in_first_set(TOKEN_OPSUB, data)) {
        update_data_to_next_token(data);
        data = parse_factor(data);
    } else {
        data = parse_primary_exp(data);
    }
    return data;
}

parse_data *parse_term_prime(parse_data *data) {
     while (!bad_exit(data) && (in_first_set(TOKEN_OPMUL, data) || in_first_set(TOKEN_OPDIV, data))) {
         update_data_to_next_token(data);
         data = parse_factor(data);
     }
     return data;
 }

 parse_data *parse_term(parse_data *data) {
     data = parse_factor(data);
     if (bad_exit(data)) { return data; }
     data = parse_term_prime(data);
     return data;
 }

 parse_data *parse_arith_exp_prime(parse_data *data) {
     while (!bad_exit(data) && (in_first_set(TOKEN_OPADD, data) || in_first_set(TOKEN_OPSUB, data))) {
         update_data_to_next_token(data);
         data = parse_term(data);
     }
     return data;
 }

 parse_data *parse_arith_exp(parse_data *data) {
     data = parse_term(data);
     if (bad_exit(data)) { return data; }
     data = parse_arith_exp_prime(data);
     return data;
 }

 // --- Boolean Expression Parsing ---

 parse_data *parse_relational_exp(parse_data *data) {
     data = parse_arith_exp(data); // Parse the first arithmetic expression
     if (bad_exit(data)) { return data; }

     // Check for optional relational or assignment operator + second arith_exp
     int is_relop_or_assign =
        in_first_set(TOKEN_OPLE, data) ||
        in_first_set(TOKEN_OPEQ, data) ||
        in_first_set(TOKEN_OPNE, data) ||
        in_first_set(TOKEN_OPLT, data) ||
        in_first_set(TOKEN_OPGE, data) ||
        in_first_set(TOKEN_OPGT, data) ||
        in_first_set(TOKEN_OPASSG, data); // Keep OPASSG for if(x=z) test case

      if (is_relop_or_assign) {
          update_data_to_next_token(data); // Consume the operator
          data = parse_arith_exp(data); // Parse the second arithmetic expression
          if (bad_exit(data)) { return data; }
          // In a real AST builder, you'd combine the two arith_exp and the operator here
      }
      // else: If no operator follows, it's just a single arith_exp (e.g., if(x))
      // which is treated as potentially boolean contextually later.

     return data;
 }

 parse_data *parse_and_exp_prime(parse_data *data) {
     while (!bad_exit(data) && in_first_set(TOKEN_OPAND, data)) {
         update_data_to_next_token(data); // Consume &&
         data = parse_relational_exp(data); // Parse the following relational_exp
     }
     return data; // Epsilon case
 }

 parse_data *parse_and_exp(parse_data *data) {
     data = parse_relational_exp(data); // Parse the first relational_exp
     if (bad_exit(data)) { return data; }
     data = parse_and_exp_prime(data); // Parse the rest (... && relational_exp)*
     return data;
 }

 parse_data *parse_bool_exp_prime(parse_data *data) {
     while (!bad_exit(data) && in_first_set(TOKEN_OPOR, data)) {
         update_data_to_next_token(data); // Consume ||
         data = parse_and_exp(data); // Parse the following and_exp
     }
     return data; // Epsilon case
 }

 parse_data *parse_bool_exp(parse_data *data) {
     data = parse_and_exp(data); // Parse the first and_exp
     if (bad_exit(data)) { return data; }
     data = parse_bool_exp_prime(data); // Parse the rest (... || and_exp)*
     return data;
 }


int parse_prog() {
  parse_data *data = create_parse_data();
  data = parse_decl_list(data);
  const int exit_status = data->exit_status;
  if (!bad_exit(data) && !in_first_set(TOKEN_EOF, data)) {
     data->exit_status = 1;
  }
  free(data);
  return (exit_status != 0) ? exit_status : data->exit_status;
}

int parse(void) {
    return parse_prog();
}