#ifndef PARSER_SERVICE_H
#define PARSER_SERVICE_H

#include <stdbool.h>

// Function prototypes for parser functions.
bool parse_prog(void);
bool parse_decl_or_func(void);
bool parse_func_defn_rest(void);
bool parse_var_decl_rest(void);
bool parse_var_decl(void);
bool parse_id_list(void);
bool parse_type(void);
bool parse_func_defn(void);
bool parse_opt_formals(void);
bool parse_formals(void);
bool parse_opt_var_decls(void);
bool parse_opt_stmt_list(void);
bool parse_stmt(void);
bool parse_fn_call(void);
bool parse_opt_expr_list(void);

#endif
