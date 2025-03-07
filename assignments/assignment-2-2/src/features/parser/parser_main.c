#include "./symbol_table.h"
#include "parser_service.h"
#include "token_service.h"
#include <stdio.h>
#include <stdlib.h>

Scope *globalScope = NULL;
Scope *currentScope = NULL;

//void initSymbolTable(void) {
//  globalScope = malloc(sizeof(Scope));
//  if (!globalScope) {
//    fprintf(stderr, "ERROR: memory allocation failure in initSymbolTable\n");
//    exit(1);
//  }
//  globalScope->symbols = NULL;
//  globalScope->parent = NULL;
//  currentScope = globalScope;
//}
//
//int parse(void) {
//  initSymbolTable();
//  advanceToken();
//
//  if (parse_prog()) {
//    return 0;
//  } else {
//    return 1;
//  }
//}
