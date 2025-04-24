#include "simple-parse.h"
#include <stdio.h>   // Added for fprintf, stderr
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
parse_data *parse_return_stmt(parse_data *data);
parse_data *parse_stmt_after_id(parse_data *data);
parse_data *parse_opt_actuals(parse_data *data);
parse_data *parse_actuals_list(parse_data *data);
parse_data *parse_actuals_list_tail(parse_data *data);
parse_data *parse_arith_exp(parse_data *data);
parse_data *parse_term(parse_data *data);
parse_data *parse_factor(parse_data *data);
parse_data *parse_primary_exp(parse_data *data);
parse_data *parse_primary_exp_after_id(parse_data *data);
parse_data *parse_arith_exp_prime(parse_data *data);
parse_data *parse_term_prime(parse_data *data);
parse_data *parse_bool_exp(parse_data *data);
parse_data *parse_bool_exp_prime(parse_data *data);
parse_data *parse_and_exp(parse_data *data);
parse_data *parse_and_exp_prime(parse_data *data);
parse_data *parse_relational_exp(parse_data *data);
static parse_data *parse_type(parse_data *data);


static parse_data *create_parse_data(void) {
  parse_data *data = malloc(sizeof(parse_data));
  if (!data) exit(EXIT_FAILURE);
  data->current_token = next_token();
  data->exit_status = 0;
  return data;
}

void update_data_to_next_token(parse_data *data) {
  // Only advance if not already at EOF or in error state?
  // For now, assume scanner handles EOF gracefully.
  if (data && data->current_token.type != TOKEN_EOF) {
      data->current_token = next_token();
  }
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

static int is_first_of_arith_exp(const parse_data *data) {
    if (!data) return 0;
    return in_first_set(TOKEN_ID, data) ||
           in_first_set(TOKEN_INTCON, data) ||
           in_first_set(TOKEN_LPAREN, data) ||
           in_first_set(TOKEN_OPSUB, data);
}


int bad_exit(const parse_data *data) {
    if (!data) return 1;
    return data->exit_status == 1;
}

// --- NEW Error Reporting Function ---
static void report_syntax_error(parse_data *data, const char *message) {
    if (!data) return; // Safety check
    // Only report the first error encountered
    if (!bad_exit(data)) {
        int line = data->current_token.line > 0 ? data->current_token.line : 0; // Use 0 if line unknown
        const char* lexeme = data->current_token.lexeme ? data->current_token.lexeme : "";
        token_type type = data->current_token.type;

        fprintf(stderr, "ERROR: LINE %d: %s (found token type %d, lexeme '%s')\n",
                line, message, type, lexeme);
        data->exit_status = 1;
    }
}


static parse_data *parse_type(parse_data *data) {
  if (does_not_match(TOKEN_KWINT, data->current_token)) {
    report_syntax_error(data, "Expected 'int' type specifier");
    return data;
  }
  update_data_to_next_token(data);
  return data;
}


parse_data *parse_var_decl_list_tail(parse_data *data) {
  while (!bad_exit(data) && in_first_set(TOKEN_COMMA, data)) {
    update_data_to_next_token(data);

    if (does_not_match(TOKEN_ID, data->current_token)) {
      report_syntax_error(data, "Expected identifier after comma in declaration list");
      return data;
    }
    update_data_to_next_token(data);
  }
  return data;
}

parse_data *parse_decl_prime(parse_data *data) {
  data = parse_var_decl_list_tail(data);
  if (bad_exit(data)) { return data; }

  if (does_not_match(TOKEN_SEMI, data->current_token)) {
    report_syntax_error(data, "Expected semicolon ';' at end of declaration");
    return data;
  }
  update_data_to_next_token(data);
  return data;
}


parse_data *parse_opt_var_decls(parse_data *data) {
  while (!bad_exit(data) && in_first_set(TOKEN_KWINT, data)) {
    data = parse_decl(data);
    if (bad_exit(data)) { return data; }
  }
  return data;
}


parse_data *parse_formals_list_tail(parse_data *data) {
  while (!bad_exit(data) && in_first_set(TOKEN_COMMA, data)) {
    update_data_to_next_token(data);
    data = parse_type(data);
    if (bad_exit(data)) {
        // Error message already printed in parse_type
        return data;
    }
    if (does_not_match(TOKEN_ID, data->current_token)) {
      report_syntax_error(data, "Expected identifier after type in parameter list");
      return data;
    }
    update_data_to_next_token(data);
  }
  return data;
}

parse_data *parse_formals_list(parse_data *data) {
  data = parse_type(data);
  if (bad_exit(data)) { return data; }
  if (does_not_match(TOKEN_ID, data->current_token)) {
    report_syntax_error(data, "Expected identifier for parameter name");
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
    // This check might be redundant if caller ensures LPAREN
    report_syntax_error(data, "Expected '(' after function name");
    return data;
  }
  update_data_to_next_token(data);

  data = parse_opt_formals(data);
  if (bad_exit(data)) { return data; }

  if (does_not_match(TOKEN_RPAREN, data->current_token)) {
    report_syntax_error(data, "Expected ')' after function parameters");
    return data;
  }
  update_data_to_next_token(data);

  if (in_first_set(TOKEN_LBRACE, data)) {
    update_data_to_next_token(data);
    data = parse_opt_var_decls(data);
    if (bad_exit(data)) { return data; }
    data = parse_stmt_list(data);
    if (bad_exit(data)) { return data; }
    if (does_not_match(TOKEN_RBRACE, data->current_token)) {
      report_syntax_error(data, "Expected '}' to close function body");
      return data;
    }
    update_data_to_next_token(data);
  } else if (in_first_set(TOKEN_SEMI, data)) {
    update_data_to_next_token(data);
  } else {
    report_syntax_error(data, "Expected '{' to start function body or ';' for declaration");
    return data;
  }
  return data;
}


parse_data *parse_decl(parse_data *data) {
  data = parse_type(data);
  if (bad_exit(data)) { return data; }
  if (does_not_match(TOKEN_ID, data->current_token)) {
    report_syntax_error(data, "Expected identifier after type");
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
    if (bad_exit(data)) { return data; }
  }
  return data;
}


parse_data *parse_stmt_list(parse_data *data) {
  while (!bad_exit(data) &&
         (in_first_set(TOKEN_ID, data) ||
          in_first_set(TOKEN_SEMI, data) ||
          in_first_set(TOKEN_KWIF, data) ||
          in_first_set(TOKEN_KWWHILE, data) ||
          in_first_set(TOKEN_KWRETURN, data) ||
          in_first_set(TOKEN_LBRACE, data) )) {
    data = parse_stmt(data);
    if (bad_exit(data)) { return data; }
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
    else if (in_first_set(TOKEN_KWRETURN, data)) {
        data = parse_return_stmt(data);
    }
    else if (in_first_set(TOKEN_LBRACE, data)) {
        update_data_to_next_token(data);
        data = parse_opt_var_decls(data);
        if (bad_exit(data)) { return data; }
        data = parse_stmt_list(data);
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_RBRACE, data->current_token)) {
            report_syntax_error(data, "Expected '}' to close compound statement");
            return data;
        }
        update_data_to_next_token(data);
    }
     else if (in_first_set(TOKEN_SEMI, data)) {
         update_data_to_next_token(data);
    }
     else {
         report_syntax_error(data, "Unexpected token at start of statement");
     }
    if (bad_exit(data)) { return data; }
    return data;
}

parse_data *parse_stmt_after_id(parse_data *data) {
    if (in_first_set(TOKEN_OPASSG, data)) {
         update_data_to_next_token(data);
        data = parse_arith_exp(data);
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_SEMI, data->current_token)) {
            report_syntax_error(data, "Expected semicolon ';' after assignment expression");
            return data;
        }
        update_data_to_next_token(data);
    } else if (in_first_set(TOKEN_LPAREN, data)) {
         update_data_to_next_token(data);
        data = parse_opt_actuals(data);
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_RPAREN, data->current_token)) {
            report_syntax_error(data, "Expected ')' after function call arguments");
            return data;
        }
        update_data_to_next_token(data);
        if (does_not_match(TOKEN_SEMI, data->current_token)) {
            report_syntax_error(data, "Expected semicolon ';' after function call statement");
            return data;
        }
        update_data_to_next_token(data);
    } else {
         report_syntax_error(data, "Expected '=' for assignment or '(' for function call after identifier");
    }
    return data;
}

parse_data *parse_if_stmt(parse_data *data) {
     update_data_to_next_token(data);
    if (does_not_match(TOKEN_LPAREN, data->current_token)) {
         report_syntax_error(data, "Expected '(' after 'if'");
         return data;
     }
     update_data_to_next_token(data);
    data = parse_bool_exp(data);
    if (bad_exit(data)) { return data; }
    if (does_not_match(TOKEN_RPAREN, data->current_token)) {
         report_syntax_error(data, "Expected ')' after 'if' condition");
         return data;
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
        report_syntax_error(data, "Expected '(' after 'while'");
        return data;
    }
    update_data_to_next_token(data);
    data = parse_bool_exp(data);
    if (bad_exit(data)) { return data; }
    if (does_not_match(TOKEN_RPAREN, data->current_token)) {
        report_syntax_error(data, "Expected ')' after 'while' condition");
        return data;
    }
    update_data_to_next_token(data);
    data = parse_stmt(data);
    return data;
}

parse_data *parse_return_stmt(parse_data *data) {
    update_data_to_next_token(data);

    if (in_first_set(TOKEN_SEMI, data)) {
        update_data_to_next_token(data);
    } else if (is_first_of_arith_exp(data)) {
        data = parse_arith_exp(data);
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_SEMI, data->current_token)) {
            report_syntax_error(data, "Expected semicolon ';' after return expression");
            return data;
        }
        update_data_to_next_token(data);
    } else {
        report_syntax_error(data, "Expected semicolon ';' or expression after 'return'");
    }
    return data;
}


parse_data *parse_actuals_list_tail(parse_data *data) {
    while (!bad_exit(data) && in_first_set(TOKEN_COMMA, data)) {
        update_data_to_next_token(data);
        if (!is_first_of_arith_exp(data)) {
             report_syntax_error(data, "Expected expression after comma in argument list");
             return data;
        }
        data = parse_arith_exp(data);
        if (bad_exit(data)) { return data; }
    }
    if (!bad_exit(data) && in_first_set(TOKEN_COMMA, data)) {
        report_syntax_error(data, "Unexpected trailing comma in argument list");
    }
    return data;
}

parse_data *parse_actuals_list(parse_data *data) {
    data = parse_arith_exp(data);
    if (bad_exit(data)) { return data; }
    data = parse_actuals_list_tail(data);
    return data;
}

parse_data *parse_opt_actuals(parse_data *data) {
    if (is_first_of_arith_exp(data)) {
        data = parse_actuals_list(data);
    }
    return data;
}


parse_data *parse_primary_exp(parse_data *data) {
     if (in_first_set(TOKEN_ID, data)) {
         update_data_to_next_token(data);
         data = parse_primary_exp_after_id(data);
         if (bad_exit(data)) { return data; }
     } else if (in_first_set(TOKEN_INTCON, data)) {
         update_data_to_next_token(data);
     } else if (in_first_set(TOKEN_LPAREN, data)) {
         update_data_to_next_token(data);
         data = parse_bool_exp(data);
         if (bad_exit(data)) { return data; }
         if (does_not_match(TOKEN_RPAREN, data->current_token)) {
             report_syntax_error(data, "Expected ')' to close parenthesized expression");
             return data;
         }
         update_data_to_next_token(data);
     }
     else {
         report_syntax_error(data, "Expected identifier, integer constant, or '(' in expression");
     }
     return data;
 }

parse_data *parse_primary_exp_after_id(parse_data *data) {
    if (in_first_set(TOKEN_LPAREN, data)) {
        update_data_to_next_token(data);
        data = parse_opt_actuals(data);
        if (bad_exit(data)) { return data; }
        if (does_not_match(TOKEN_RPAREN, data->current_token)) {
            report_syntax_error(data, "Expected ')' after function call arguments");
            return data;
        }
        update_data_to_next_token(data);
    }
    return data;
}


parse_data *parse_factor(parse_data *data) {
    if (in_first_set(TOKEN_OPSUB, data)) {
        update_data_to_next_token(data);
        data = parse_factor(data);
        if (bad_exit(data)) { return data; }
    } else {
        data = parse_primary_exp(data);
        if (bad_exit(data)) { return data; }
    }
    return data;
}

parse_data *parse_term_prime(parse_data *data) {
     while (!bad_exit(data) && (in_first_set(TOKEN_OPMUL, data) || in_first_set(TOKEN_OPDIV, data))) {
         update_data_to_next_token(data);
         data = parse_factor(data);
         if (bad_exit(data)) { return data; }
     }
     return data;
 }

 parse_data *parse_term(parse_data *data) {
     data = parse_factor(data);
     if (bad_exit(data)) { return data; }
     data = parse_term_prime(data);
     if (bad_exit(data)) { return data; }
     return data;
 }

 parse_data *parse_arith_exp_prime(parse_data *data) {
     while (!bad_exit(data) && (in_first_set(TOKEN_OPADD, data) || in_first_set(TOKEN_OPSUB, data))) {
         update_data_to_next_token(data);
         data = parse_term(data);
         if (bad_exit(data)) { return data; }
     }
     return data;
 }

 parse_data *parse_arith_exp(parse_data *data) {
     data = parse_term(data);
     if (bad_exit(data)) { return data; }
     data = parse_arith_exp_prime(data);
     if (bad_exit(data)) { return data; }
     return data;
 }


 parse_data *parse_relational_exp(parse_data *data) {
     data = parse_arith_exp(data);
     if (bad_exit(data)) { return data; }

     int is_relop =
        in_first_set(TOKEN_OPLE, data) ||
        in_first_set(TOKEN_OPEQ, data) ||
        in_first_set(TOKEN_OPNE, data) ||
        in_first_set(TOKEN_OPLT, data) ||
        in_first_set(TOKEN_OPGE, data) ||
        in_first_set(TOKEN_OPGT, data);

      if (!bad_exit(data) && is_relop) {
          update_data_to_next_token(data);
          data = parse_arith_exp(data);
          if (bad_exit(data)) { return data; }
      }
     return data;
 }

 parse_data *parse_and_exp_prime(parse_data *data) {
     while (!bad_exit(data) && in_first_set(TOKEN_OPAND, data)) {
         update_data_to_next_token(data);
         data = parse_relational_exp(data);
         if (bad_exit(data)) { return data; }
     }
     return data;
 }

 parse_data *parse_and_exp(parse_data *data) {
     data = parse_relational_exp(data);
     if (bad_exit(data)) { return data; }
     data = parse_and_exp_prime(data);
     return data;
 }

 parse_data *parse_bool_exp_prime(parse_data *data) {
     while (!bad_exit(data) && in_first_set(TOKEN_OPOR, data)) {
         update_data_to_next_token(data);
         data = parse_and_exp(data);
         if (bad_exit(data)) { return data; }
     }
     return data;
 }

 parse_data *parse_bool_exp(parse_data *data) {
     data = parse_and_exp(data);
     if (bad_exit(data)) { return data; }
     data = parse_bool_exp_prime(data);
     return data;
 }


int parse_prog() {
  parse_data *data = create_parse_data();
  data = parse_decl_list(data);
  const int original_exit_status = data->exit_status;
  if (!bad_exit(data) && !in_first_set(TOKEN_EOF, data)) {
     // Report error for unexpected tokens after end of program
     report_syntax_error(data, "Unexpected token after end of program");
  }
  const int final_exit_status = data->exit_status;
  free(data);
  return final_exit_status;
}

int parse(void) {
    return parse_prog();
}