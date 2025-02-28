#ifndef PARSER_SERVICE_H
#define PARSER_SERVICE_H

#include <stdbool.h>

// External variables provided by other modules.
extern int chk_decl_flag;
extern char *lexeme; // Holds the lexeme for the last matched identifier.

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
char *captureID(void);
bool chooseDeclBranch(char *idLex);
bool parseFunctionDeclaration(char *idLex);
bool parseVariableDeclaration(char *idLex);
bool semanticCheckVar(const char *idLex);
bool semanticCheckFunc(const char *idLex);

#endif
