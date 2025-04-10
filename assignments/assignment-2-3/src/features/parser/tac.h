#ifndef __TAC_H__
#define __TAC_H__

#include "ast.h"
#include "symbol_table.h"

typedef enum {
  TAC_ADD,
  TAC_SUB,
  TAC_MUL,
  TAC_DIV,

  TAC_ASSIGN,

  TAC_LABEL,

  TAC_PARAM,
  TAC_CALL,

  TAC_ENTER,
  TAC_LEAVE,
  TAC_RETURN,

  TAC_GOTO,
  TAC_IF_AND,
  TAC_IF_EQ,
  TAC_IF_GE,
  TAC_IF_GT,
  TAC_IF_LE,
  TAC_IF_LT,
  TAC_IF_NE,
  TAC_IF_OR,

  TAC_SET_RETVAL,
  TAC_RETRIEVE,

} OpType;

typedef enum {
  INTEGER_CONSTANT,
  SYM_TABLE_PTR,
} OperandType;

typedef struct {
  OperandType operand_type;
  union {
    int integer_const;
    Symbol *symbol_ptr;
    char *id_name;
  } val;
} Operand;

typedef struct tac {
  OpType op;     // The operation code.
  Operand *src1;   // First argument (e.g., variable name or constant as string).
  Operand *src2;   // Second argument (if needed).
  Operand *dest; // Result temporary variable or target variable.
  struct tac *next; // Link to the next instruction.
} Quad;

Symbol *make_TAC(ASTnode *node, Quad **code_list);
Quad *reverse_tac_list(Quad *head);
void print_quad(Quad *code_list);
char *quad_list_to_string(Quad *code_list);
void reset_temp_counter();

#endif
