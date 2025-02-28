#include "./parser_service.h"
#include "./symbol_table.h"
#include "./token_service.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *currentType = NULL;

bool parse_prog(void) {
  while (currentToken.type == TOKEN_KWINT) {
    if (!parse_decl_or_func())
      return false;
  }
  if (currentToken.type != TOKEN_EOF) {
    fprintf(stderr, "ERROR: LINE %d: unexpected token after program end\n",
            currentToken.line);
    return false;
  }
  return true;
}

bool parse_type(void) {
  if (match(TOKEN_KWINT)) {
    currentType = "int";
    return true;
  }
  fprintf(stderr, "ERROR: LINE %d: expected type 'kwINT'\n", currentToken.line);
  return false;
}

// Captures the current identifier’s lexeme and advances the token.
char *captureID(void) {
  if (currentToken.type != TOKEN_ID) {
    fprintf(stderr, "ERROR: LINE %d: expected identifier\n", currentToken.line);
    return NULL;
  }
  char *idLex = strdup(lexeme);
  if (!idLex) {
    fprintf(stderr, "ERROR: memory allocation failure\n");
    return NULL;
  }
  advanceToken();
  return idLex;
}

// Chooses the branch based on the next token and handles it.
bool chooseDeclBranch(char *idLex) {
  if (currentToken.type == TOKEN_LPAREN)
    return parseFunctionDeclaration(idLex);
  else if (currentToken.type == TOKEN_COMMA || currentToken.type == TOKEN_SEMI)
    return parseVariableDeclaration(idLex);
  else {
    fprintf(stderr,
            "ERROR: LINE %d: expected '(' for function or ','/';' for variable "
            "declaration\n",
            currentToken.line);
    free(idLex);
    return false;
  }
}

// Branch for function declarations.
bool parseFunctionDeclaration(char *idLex) {
  if (!semanticCheckFunc(idLex)) {
    free(idLex);
    return false;
  }
  bool result = parse_func_defn_rest();
  free(idLex);
  return result;
}

// Branch for variable declarations.
bool parseVariableDeclaration(char *idLex) {
  if (!semanticCheckVar(idLex)) {
    free(idLex);
    return false;
  }
  if (!parse_id_list()) {
    free(idLex);
    return false;
  }
  if (!match(TOKEN_SEMI)) {
    free(idLex);
    return false;
  }
  free(idLex);
  return true;
}

bool semanticCheckVar(const char *idLex) {
  if (!chk_decl_flag)
    return true;
  if (lookupSymbol(idLex, currentScope)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate declaration of '%s'\n",
            currentToken.line, idLex);
    return false;
  }
  return addSymbol(idLex, currentScope, currentType,
                   false); // false for variables
}

bool semanticCheckFunc(const char *idLex) {
  if (!chk_decl_flag)
    return true;
  if (lookupSymbol(idLex, globalScope)) {
    fprintf(stderr, "ERROR: LINE %d: duplicate function declaration '%s'\n",
            currentToken.line, idLex);
    return false;
  }
  if (!addSymbol(idLex, globalScope, currentType, true)) // true for functions
    return false;
  pushScope();
  return true;
}

bool parse_decl_or_func(void) {
  if (!parse_type())
    return false;
  char *idLex = captureID();
  if (!idLex)
    return false;
  return chooseDeclBranch(idLex);
}

bool parse_id_list(void) {
  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;

    // Capture the identifier before advancing
    if (currentToken.type != TOKEN_ID) {
      fprintf(stderr,
              "ERROR: LINE %d: expected identifier after comma in id_list\n",
              currentToken.line);
      return false;
    }

    // Get the lexeme before advancing
    char *idLex = strdup(lexeme);
    if (!idLex) {
      fprintf(stderr, "ERROR: memory allocation failure\n");
      return false;
    }

    // Perform semantic check for duplicate declarations
    if (chk_decl_flag) {
      if (lookupSymbol(idLex, currentScope)) {
        fprintf(stderr, "ERROR: LINE %d: duplicate declaration of '%s'\n",
                currentToken.line, idLex);
        free(idLex);
        return false;
      }
      // Add the symbol to the current scope
      if (!addSymbol(idLex, currentScope, currentType, false)) {
        free(idLex);
        return false;
      }
    }

    free(idLex);
    advanceToken(); // Now advance the token after all processing
  }
  return true;
}

bool parse_func_defn_rest(void) {
  if (!match(TOKEN_LPAREN))
    return false;
  if (!parse_opt_formals())
    return false;
  if (!match(TOKEN_RPAREN))
    return false;
  if (!match(TOKEN_LBRACE))
    return false;
  if (!parse_opt_var_decls())
    return false;
  if (!parse_opt_stmt_list())
    return false;
  if (!match(TOKEN_RBRACE))
    return false;
  return true;
}

bool parse_var_decl(void) {
  if (!parse_type())
    return false;
  if (!match(TOKEN_ID))
    return false;
  if (!parse_id_list())
    return false;
  if (!match(TOKEN_SEMI))
    return false;
  return true;
}

bool parse_opt_formals(void) {
  if (currentToken.type == TOKEN_KWINT)
    return parse_formals();
  return true; // epsilon production
}

bool parse_formals(void) {
  if (!parse_type())
    return false;

  // Capture the parameter name
  char *paramName = captureID();
  if (!paramName)
    return false;

  // Add parameter to current scope
  if (chk_decl_flag) {
    if (lookupSymbol(paramName, currentScope)) {
      fprintf(stderr, "ERROR: LINE %d: duplicate parameter '%s'\n",
              currentToken.line, paramName);
      free(paramName);
      return false;
    }
    if (!addSymbol(paramName, currentScope, currentType,
                   false)) { // false because it's a variable
      free(paramName);
      return false;
    }
  }
  free(paramName);

  while (currentToken.type == TOKEN_COMMA) {
    if (!match(TOKEN_COMMA))
      return false;
    if (!parse_type())
      return false;

    // Capture the parameter name
    paramName = captureID();
    if (!paramName)
      return false;

    // Add parameter to current scope
    if (chk_decl_flag) {
      if (lookupSymbol(paramName, currentScope)) {
        fprintf(stderr, "ERROR: LINE %d: duplicate parameter '%s'\n",
                currentToken.line, paramName);
        free(paramName);
        return false;
      }
      if (!addSymbol(paramName, currentScope, currentType,
                     false)) { // false because it's a variable
        free(paramName);
        return false;
      }
    }
    free(paramName);
  }
  return true;
}

bool parse_opt_var_decls(void) {
  while (currentToken.type == TOKEN_KWINT) {
    if (!parse_var_decl())
      return false;
  }
  return true;
}

bool parse_opt_stmt_list(void) {
  while (currentToken.type == TOKEN_ID) {
    if (!parse_stmt())
      return false;
  }
  return true;
}

bool parse_stmt(void) {
  if (!parse_fn_call())
    return false;
  if (!match(TOKEN_SEMI))
    return false;
  return true;
}

bool parse_fn_call(void) {
  // Capture the function identifier before advancing the token
  char *funcName = strdup(lexeme);
  if (!funcName) {
    fprintf(stderr, "ERROR: memory allocation failure\n");
    return false;
  }

  // Check if function exists and is actually a function
  if (chk_decl_flag) {
    // First check if there's a local variable with this name
    Symbol *localSymbol = lookupSymbolInScope(funcName, currentScope);

    if (localSymbol) {
      // If there's a local symbol with this name and we're trying to call it
      // as a function, that's an error
      if (!localSymbol->isFunction) {
        fprintf(stderr, "ERROR: LINE %d: '%s' is not a function\n",
                currentToken.line, funcName);
        free(funcName);
        return false;
      }
    } else {
      // Check global scope for the function
      Symbol *globalSymbol = lookupSymbol(funcName, globalScope);
      if (!globalSymbol) {
        fprintf(stderr, "ERROR: LINE %d: call to undeclared function '%s'\n",
                currentToken.line, funcName);
        free(funcName);
        return false;
      }
      if (!globalSymbol->isFunction) {
        fprintf(stderr, "ERROR: LINE %d: '%s' is not a function\n",
                currentToken.line, funcName);
        free(funcName);
        return false;
      }
    }
  }

  free(funcName);

  // Now proceed with syntax checking
  if (!match(TOKEN_ID))
    return false;
  if (!match(TOKEN_LPAREN))
    return false;
  if (!parse_opt_expr_list())
    return false;
  if (!match(TOKEN_RPAREN))
    return false;
  return true;
}

bool parse_opt_expr_list(void) { return true; }
