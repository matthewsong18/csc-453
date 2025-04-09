#include "tac.h"
#include "ast.h"
#include "symbol_table.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int DEBUG_ON;
int TAC_DEBUG_ON = true;

void debug_tac(char *message) {
  if (!DEBUG_ON || !TAC_DEBUG_ON) {
    return;
  }

  printf("AST node type: %s\n", message);
  fflush(stdout);
}

Operand *new_operand(OperandType op_type, void *value) {
  Operand *operand = malloc(sizeof(Operand));

  if (!operand) {
    fprintf(stderr, "ERROR: Memory allocation failed\n");
    exit(1);
  }

  operand->operand_type = op_type;

  switch (op_type) {
  case INTEGER_CONSTANT:
    operand->val.integer_const = *(int *)value;
    break;

  case SYM_TABLE_PTR:
    operand->val.symbol_ptr = (Symbol *)value;
    break;
  }

  return operand;
}

int temp_counter = 0;

void reset_temp_counter() { temp_counter = 0; }

// Based on lecture slide 05
Symbol *new_temp(char *type) {
  char *temp_name = malloc(20);
  sprintf(temp_name, "t%d", temp_counter++);

  Symbol *new_temp = create_symbol(temp_name);

  new_temp->type = strdup(type);

  if (!new_temp->type) {
    free(new_temp->name);
    free(new_temp);
    fprintf(stderr, "ERROR: memory allocation failure for symbol type\n");
    exit(1);
  }

  new_temp->next = currentScope->symbols;
  currentScope->symbols = new_temp;

  return new_temp;
}

Quad *new_instr(OpType opType, Operand *src1, Operand *src2, Operand *dest) {
  Quad *new_instr = malloc(sizeof(Quad));
  new_instr->op = opType;
  new_instr->src1 = src1;
  new_instr->src2 = src2;
  new_instr->dest = dest;
  new_instr->next = NULL;

  return new_instr;
}

static int label_num = 0;
Quad *new_label() {
  Operand *src1 = new_operand(INTEGER_CONSTANT, &label_num);
  label_num++;
  return new_instr(TAC_LABEL, src1, NULL, NULL);
}

Quad *reverse_tac_list(Quad *head) {
  Quad *prev = NULL;
  Quad *current = head;
  Quad *next = NULL;

  while (current != NULL) {
    next = current->next;
    current->next = prev;
    prev = current;
    current = next;
  }
  return prev; // New head of the reversed list
}

Symbol *make_TAC(ASTnode *node, Quad **code_list) {
  Symbol *temp = NULL;
  Symbol *left = NULL;
  Symbol *right = NULL;

  OpType op_type;
  Operand *dest = NULL;
  Operand *src1 = NULL;
  Operand *src2 = NULL;

  Quad *instruction = NULL;

  if (!node)
    return NULL;

  switch (node->node_type) {
  case INTCONST:
    debug_tac("INTCONST");
    temp = new_temp("variable");

    dest = new_operand(SYM_TABLE_PTR, temp);
    src1 = new_operand(INTEGER_CONSTANT, &(node->num));
    instruction = new_instr(TAC_ASSIGN, src1, NULL, dest);

    instruction->next = *code_list;
    *code_list = instruction;
    return temp;

  case IDENTIFIER:
    debug_tac("IDENTIFIER");
    return node->symbol;

  case ADD:
  case SUB:
  case MUL:
  case DIV:
    left = make_TAC(node->child0, code_list);
    right = make_TAC(node->child1, code_list);

    temp = new_temp("variable");
    instruction = malloc(sizeof(Quad));

    op_type = (node->node_type == ADD)   ? TAC_ADD
              : (node->node_type == SUB) ? TAC_SUB
              : (node->node_type == MUL) ? TAC_MUL
                                         : TAC_DIV;

    src1 = new_operand(SYM_TABLE_PTR, left);
    src2 = new_operand(SYM_TABLE_PTR, right);
    dest = new_operand(SYM_TABLE_PTR, temp);

    instruction = new_instr(op_type, src1, src2, dest);

    instruction->next = *code_list;
    *code_list = instruction;

    return temp;

  case ASSG:
    left = node->child0->symbol;
    right = make_TAC(node->child1, code_list);

    op_type = TAC_ASSIGN;
    src1 = new_operand(SYM_TABLE_PTR, right);
    dest = new_operand(SYM_TABLE_PTR, left);

    instruction = new_instr(op_type, src1, src2, dest);

    instruction->next = *code_list;
    *code_list = instruction;

    return NULL;

  case FUNC_DEF:
    left = node->symbol;

    op_type = TAC_ENTER;

    src1 = new_operand(SYM_TABLE_PTR, left);

    instruction = new_instr(op_type, src1, src2, dest);

    instruction->next = *code_list;
    *code_list = instruction;

    debug_tac("Instruction Set");
    make_TAC(node->child0, code_list);

    op_type = TAC_LEAVE;

    instruction = new_instr(op_type, src1, src2, dest);

    instruction->next = *code_list;
    *code_list = instruction;

    op_type = TAC_RETURN;

    src1 = NULL;

    instruction = new_instr(op_type, src1, src2, dest);

    instruction->next = *code_list;
    *code_list = instruction;

    return NULL;

  case FUNC_CALL:
    debug_tac("FUNC_CALL");

    // PRINTLN
    if (strcmp(node->symbol->name, "println") == 0) {

      debug_tac("PRINTLN");

      left = make_TAC(node->child0, code_list);

      op_type = TAC_PRINTLN;

      src1 = new_operand(SYM_TABLE_PTR, left);

      instruction = new_instr(op_type, src1, src2, dest);

      instruction->next = *code_list;
      *code_list = instruction;
      return NULL;
    }

    // FUNC CALL

    // First do param

    right = make_TAC(node->child0, code_list);

    // Then do call

    right = NULL;

    left = node->symbol;

    op_type = TAC_CALL;

    src1 = new_operand(SYM_TABLE_PTR, left);
    src2 = new_operand(INTEGER_CONSTANT, &(left->number_of_arguments));

    instruction = new_instr(op_type, src1, src2, NULL);

    instruction->next = *code_list;
    *code_list = instruction;

    return NULL;

  case STMT_LIST:
    debug_tac("STMT_LIST");
    make_TAC(node->child0, code_list);
    make_TAC(node->child1, code_list);
    return NULL;

  case EXPR_LIST:
    debug_tac("EXPR_LIST");

    left = make_TAC(node->child0, code_list);

    op_type = TAC_PARAM;

    src1 = new_operand(SYM_TABLE_PTR, left);
    instruction = new_instr(op_type, src1, NULL, NULL);

    instruction->next = *code_list;
    *code_list = instruction;

    right = make_TAC(node->child1, code_list);

    // Optional Expr List
    if (right == NULL) {
      return NULL;
    }

    op_type = TAC_PARAM;

    src1 = new_operand(SYM_TABLE_PTR, right);
    instruction = new_instr(op_type, src1, NULL, NULL);

    instruction->next = *code_list;
    *code_list = instruction;

    return NULL;

  default:
    debug_tac("Did not match");
    return NULL;
  }
}

void print_quad(Quad *code_list) {
  if (code_list == NULL) {
    return;
  }

  Operand *src1 = NULL;
  Operand *src2 = NULL;
  Operand *dest = NULL;

  switch (code_list->op) {
  case TAC_ADD:
    src1 = code_list->src1;
    src2 = code_list->src2;

    if (src1->operand_type == SYM_TABLE_PTR) {
      printf("%s", src1->val.symbol_ptr->name);
    } else {
      printf("%d", src1->val.integer_const);
    }

    printf(" + ");

    if (src2->operand_type == SYM_TABLE_PTR) {
      printf("%s\n", src2->val.symbol_ptr->name);
    } else {
      printf("%d\n", src2->val.integer_const);
    }
    break;
  case TAC_SUB:
    src1 = code_list->src1;
    src2 = code_list->src2;

    if (src1->operand_type == SYM_TABLE_PTR) {
      printf("%s", src1->val.symbol_ptr->name);
    } else {
      printf("%d", src1->val.integer_const);
    }

    printf(" - ");

    if (src2->operand_type == SYM_TABLE_PTR) {
      printf("%s\n", src2->val.symbol_ptr->name);
    } else {
      printf("%d\n", src2->val.integer_const);
    }
    break;
  case TAC_MUL:
    src1 = code_list->src1;
    src2 = code_list->src2;

    if (src1->operand_type == SYM_TABLE_PTR) {
      printf("%s", src1->val.symbol_ptr->name);
    } else {
      printf("%d", src1->val.integer_const);
    }

    printf(" x ");

    if (src2->operand_type == SYM_TABLE_PTR) {
      printf("%s\n", src2->val.symbol_ptr->name);
    } else {
      printf("%d\n", src2->val.integer_const);
    }
    break;
  case TAC_DIV:
    src1 = code_list->src1;
    src2 = code_list->src2;

    if (src1->operand_type == SYM_TABLE_PTR) {
      printf("%s", src1->val.symbol_ptr->name);
    } else {
      printf("%d", src1->val.integer_const);
    }

    printf(" / ");

    if (src2->operand_type == SYM_TABLE_PTR) {
      printf("%s\n", src2->val.symbol_ptr->name);
    } else {
      printf("%d\n", src2->val.integer_const);
    }
    break;
  case TAC_ASSIGN:
    dest = code_list->dest;
    src1 = code_list->src1;

    printf("%s = ", dest->val.symbol_ptr->name);

    if (src1->operand_type == SYM_TABLE_PTR) {
      printf("%s\n", src1->val.symbol_ptr->name);
    } else {
      printf("%d\n", src1->val.integer_const);
    }

    break;
  case TAC_LABEL:
    dest = code_list->dest;

    printf("Label %s:\n", dest->val.symbol_ptr->name);
    break;
  case TAC_CALL:
    src1 = code_list->src1;
    src2 = code_list->src2;

    printf("call %s, %s\n", src1->val.symbol_ptr->name,
           src2->val.symbol_ptr->name);
    break;
  case TAC_PRINTLN:
    break;
  case TAC_PARAM:
  case TAC_ENTER:
  case TAC_LEAVE:
  case TAC_RETURN:
    break;
  }

  print_quad(code_list->next);
}

char *append_string(char *buffer, size_t *buffer_size, size_t *current_len,
                    const char *to_add) {

  size_t add_len = strlen(to_add);

  // Check if resize needed (+1 for null terminator)
  if (*current_len + add_len + 1 > *buffer_size) {
    size_t new_size = (*buffer_size == 0)
                          ? (add_len + 128)
                          : (*buffer_size * 2 + add_len); // Grow buffer
    char *new_buffer = realloc(buffer, new_size);
    if (!new_buffer) {
      fprintf(stderr, "ERROR: Failed to realloc buffer in append_string\n");
      free(buffer);
      return NULL;
    }
    buffer = new_buffer;
    *buffer_size = new_size;
  }

  // Append using strcpy for first element or strcat for subsequent ones
  if (*current_len == 0) {
    strcpy(buffer, to_add);
  } else {
    strcat(buffer, to_add);
  }
  *current_len += add_len;
  return buffer;
}

char *quad_list_to_string(Quad *code_list) {

  if (code_list == NULL) {
    // Return an empty, allocated string for consistency
    char *empty = malloc(1);
    if (empty)
      empty[0] = '\0';
    return empty;
  }

  char *result_buffer = NULL;
  size_t buffer_size = 0;
  size_t current_len = 0;
  char temp_instr_buffer[256];

  Quad *current = code_list;
  while (current != NULL) {
    Operand *src1 = current->src1;
    Operand *src2 = current->src2;
    Operand *dest = current->dest;

    temp_instr_buffer[0] = '\0'; // Clear temp buffer

    switch (current->op) {
    case TAC_ENTER:
      if (src1 && src1->operand_type == SYM_TABLE_PTR && src1->val.symbol_ptr &&
          src1->val.symbol_ptr->name) {
        snprintf(temp_instr_buffer, sizeof(temp_instr_buffer), "enter %s\n",
                 src1->val.symbol_ptr->name);
      } else {
        snprintf(temp_instr_buffer, sizeof(temp_instr_buffer),
                 "enter INVALID_OPERAND\n");
      }
      break;
    case TAC_LEAVE:
      if (src1 && src1->operand_type == SYM_TABLE_PTR && src1->val.symbol_ptr &&
          src1->val.symbol_ptr->name) {
        snprintf(temp_instr_buffer, sizeof(temp_instr_buffer), "leave %s\n",
                 src1->val.symbol_ptr->name);
      } else {
        snprintf(temp_instr_buffer, sizeof(temp_instr_buffer),
                 "leave INVALID_OPERAND\n");
      }
      break;
    case TAC_RETURN:
      snprintf(temp_instr_buffer, sizeof(temp_instr_buffer), "return\n");
      break;

    // TODO
    case TAC_ADD:
      break;
    case TAC_ASSIGN:
      if (src1->operand_type == SYM_TABLE_PTR) {
        snprintf(temp_instr_buffer, sizeof(temp_instr_buffer), "%s = %s\n",
                 dest->val.symbol_ptr->name, src1->val.symbol_ptr->name);
      } else {
        snprintf(temp_instr_buffer, sizeof(temp_instr_buffer), "%s = %d\n",
                 dest->val.symbol_ptr->name, src1->val.integer_const);
      }
      break;
    case TAC_LABEL:
    case TAC_SUB:
    case TAC_MUL:
    case TAC_DIV:
    case TAC_PARAM:
      snprintf(temp_instr_buffer, sizeof(temp_instr_buffer), "param %s\n",
               src1->val.symbol_ptr->name);
      break;
    case TAC_CALL:
      snprintf(temp_instr_buffer, sizeof(temp_instr_buffer), "call %s, %d\n",
               src1->val.symbol_ptr->name, src2->val.integer_const);
      break;
    case TAC_PRINTLN:
      break;
    }

    // Append the formatted instruction string to the main buffer
    if (temp_instr_buffer[0] != '\0') {
      char *new_buffer_ptr = append_string(result_buffer, &buffer_size,
                                           &current_len, temp_instr_buffer);
      if (!new_buffer_ptr) {
        return NULL; // Allocation failed
      }
      result_buffer = new_buffer_ptr;
    }

    current = current->next; // Move to the next instruction
  }

  // If the list was empty or only contained unhandled ops, ensure we return an
  // empty string
  if (!result_buffer) {
    result_buffer = malloc(1);
    if (result_buffer)
      result_buffer[0] = '\0';
  }

  return result_buffer; // Return the final, correctly sized buffer
}
