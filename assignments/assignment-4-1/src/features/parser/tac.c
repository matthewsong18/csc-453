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
static int label_num = 0;

void reset_temp_counter() { temp_counter = 0; }
void reset_label_counter() { label_num = 0; }

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
  return prev;
}

void bool_helper(ASTnode *node, Quad *trueDest, Quad *falseDest, OpType op_type,
                 Quad **code_list) {
  Symbol *left = make_TAC(node->child0, code_list);
  Symbol *right = make_TAC(node->child1, code_list);

  Operand *src1 = new_operand(SYM_TABLE_PTR, left);
  Operand *src2 = new_operand(SYM_TABLE_PTR, right);
  Quad *instruction = new_instr(op_type, src1, src2, trueDest->src1);

  instruction->next = *code_list;
  *code_list = instruction;

  op_type = TAC_GOTO;
  instruction = NULL;
  instruction = new_instr(op_type, NULL, NULL, falseDest->src1);

  instruction->next = *code_list;
  *code_list = instruction;
}

void make_bool(ASTnode *node, Quad *trueDest, Quad *falseDest,
               Quad **code_list) {

  OpType op_type = TAC_IF_EQ;

  switch (node->node_type) {
  // case AND: {
  //   Quad *L_eval_B = new_label();
  //   make_bool(node->child0, L_eval_B, falseDest, code_list);
  //   L_eval_B->next = *code_list;
  //   *code_list = L_eval_B;
  //   make_bool(node->child1, trueDest, falseDest, code_list);
  //   break;
  // }
  // case OR: {
  //   Quad *L_eval_B = new_label();
  //   make_bool(node->child0, trueDest, L_eval_B, code_list);
  //   L_eval_B->next = *code_list;
  //   *code_list = L_eval_B;
  //   make_bool(node->child1, trueDest, falseDest, code_list);
  //   break;
  // }
  case EQ: {
    op_type = TAC_IF_EQ;
    bool_helper(node, trueDest, falseDest, op_type, code_list);
  } break;
  case GE: {
    op_type = TAC_IF_GE;
    bool_helper(node, trueDest, falseDest, op_type, code_list);
  } break;
  case GT: {
    op_type = TAC_IF_GT;
    bool_helper(node, trueDest, falseDest, op_type, code_list);
  } break;
  case LE: {
    op_type = TAC_IF_LE;
    bool_helper(node, trueDest, falseDest, op_type, code_list);
  } break;
  case LT: {
    op_type = TAC_IF_LT;
    bool_helper(node, trueDest, falseDest, op_type, code_list);
  } break;
  case NE: {
    op_type = TAC_IF_NE;
    bool_helper(node, trueDest, falseDest, op_type, code_list);
  } break;
  default:
    break;
  }
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

    // Generate TAC_ENTER for the function
    op_type = TAC_ENTER;
    src1 = new_operand(SYM_TABLE_PTR, left);
    instruction = new_instr(op_type, src1, NULL, NULL);
    instruction->next = *code_list;
    *code_list = instruction;

    reset_temp_counter(); // Reset temps for the new function

    Scope *function_body_scope = currentScope;

    debug_tac("Instruction Set");
    make_TAC(node->child0, code_list);

    int total_local_bytes = abs(function_body_scope->current_offset) - 8;
    if (total_local_bytes < 0)
      total_local_bytes = 0;

    // Store the calculated size in the function's symbol entry
    left->local_var_bytes = total_local_bytes;

    // Generate TAC_LEAVE
    op_type = TAC_LEAVE;
    instruction = new_instr(op_type, src1, NULL, NULL);
    instruction->next = *code_list;
    *code_list = instruction;

    // Generate TAC_RETURN
    op_type = TAC_RETURN;
    instruction = new_instr(op_type, NULL, NULL, NULL);
    instruction->next = *code_list;
    *code_list = instruction;

    return NULL;

  case FUNC_CALL: {
    debug_tac("FUNC_CALL");

    Symbol *func_symbol = node->symbol;
    Symbol *return_val_temp = NULL;
    Quad *call_instr = NULL;
    Quad *retrieve_instr = NULL;

    make_TAC(node->child0, code_list);

    bool returns_value = false;
    if (returns_value) {
      // E.place = newtemp(f.returnType);
      return_val_temp = new_temp("variable");
      assert(return_val_temp != NULL);
    }

    op_type = TAC_CALL;
    src1 = new_operand(SYM_TABLE_PTR, func_symbol);
    src2 = new_operand(INTEGER_CONSTANT, &(func_symbol->number_of_arguments));
    call_instr = new_instr(op_type, src1, src2, NULL);

    call_instr->next = *code_list;
    *code_list = call_instr;

    if (return_val_temp != NULL) {
      op_type = TAC_RETRIEVE;
      dest = new_operand(SYM_TABLE_PTR, return_val_temp);
      retrieve_instr = new_instr(op_type, NULL, NULL, dest);

      // Prepend RETRIEVE instruction (goes after CALL)
      retrieve_instr->next = *code_list;
      *code_list = retrieve_instr;
    }

    return return_val_temp;
  }
  case STMT_LIST:
    debug_tac("STMT_LIST");
    make_TAC(node->child0, code_list);
    make_TAC(node->child1, code_list);
    return NULL;

  case EXPR_LIST:
    debug_tac("EXPR_LIST");

    // We need to go through the params right to left

    // Recurse through right first
    make_TAC(node->child1, code_list);

    left = make_TAC(node->child0, code_list);

    bool needs_load = false;
    if (left && left->type && strcmp(left->type, "variable") == 0 &&
        left->name[0] != 't') {
      needs_load = true;
    }

    Symbol *param_symbol_to_use;

    if (needs_load) {
      Symbol *temp_for_load = new_temp("variable");

      Operand *dest_op = new_operand(SYM_TABLE_PTR, temp_for_load);
      Operand *src_op = new_operand(SYM_TABLE_PTR, left);
      Quad *load_instr = new_instr(TAC_ASSIGN, src_op, NULL, dest_op);

      load_instr->next = *code_list;
      *code_list = load_instr;

      param_symbol_to_use = temp_for_load; // param t1

    } else {
      param_symbol_to_use = left;
    }

    op_type = TAC_PARAM;
    src1 = new_operand(SYM_TABLE_PTR, param_symbol_to_use);
    instruction = new_instr(op_type, src1, NULL, NULL);

    instruction->next = *code_list;
    *code_list = instruction;

    return NULL;

  case IF: {
    // If statement
    // child0 -> bool expr
    // child1 -> true state
    // child2 -> false state

    Quad *Lthen = new_label();
    Quad *Lelse = NULL;
    Quad *Lafter = new_label();

    if (node->child2 != NULL) {
      Lelse = new_label();
      make_bool(node->child0, Lthen, Lelse, code_list);
    } else {
      make_bool(node->child0, Lthen, Lafter, code_list);
    }

    // Emit Lthen label
    Lthen->next = *code_list;
    *code_list = Lthen;

    // True state
    make_TAC(node->child1, code_list);

    // Handle the 'else' block if it exists
    if (node->child2 != NULL) {

      // Generate a jump to Lafter after the 'then' block so you don't enter
      // else block after completing true state
      Quad *gotoLafter = new_instr(TAC_GOTO, NULL, NULL, Lafter->src1);
      gotoLafter->next = *code_list;
      *code_list = gotoLafter;

      // Emit the Lelse label
      Lelse->next = *code_list;
      *code_list = Lelse;

      // False state
      make_TAC(node->child2, code_list);
    }

    // Emit the Lafter label
    Lafter->next = *code_list;
    *code_list = Lafter;

    return NULL;
  }
  case WHILE: {
    // While statement
    // child0 -> bool expr (condition)
    // child1 -> statement (body)

    Quad *Ltop = new_label();
    Quad *Lbody = new_label();
    Quad *Lafter = new_label();

    // Emit the Ltop label at the beginning of the loop
    Ltop->next = *code_list;
    *code_list = Ltop;

    // Generate code for the boolean condition. If true, jump to Lbody; if
    // false, jump to Lafter.
    make_bool(node->child0, Lbody, Lafter, code_list);

    // Emit the Lbody label before the loop body
    Lbody->next = *code_list;
    *code_list = Lbody;

    // Generate code for the body of the while loop
    make_TAC(node->child1, code_list);

    // Generate an unconditional jump back to the top of the loop (Ltop)
    Quad *gotoLtop = new_instr(TAC_GOTO, NULL, NULL, Ltop->src1);
    gotoLtop->next = *code_list;
    *code_list = gotoLtop;

    // Emit the Lafter label after the loop
    Lafter->next = *code_list;
    *code_list = Lafter;

    return NULL;
  }
  case RETURN: {
    debug_tac("RETURN");
    Symbol *return_val_place = NULL;
    Quad *set_retval_instr = NULL;

    if (node->child0 != NULL) {
      return_val_place = make_TAC(node->child0, code_list);

      assert(return_val_place != NULL);

      Operand *retval_op = new_operand(SYM_TABLE_PTR, return_val_place);
      set_retval_instr = new_instr(TAC_SET_RETVAL /* Add this OpType */,
                                   retval_op, NULL, NULL);

      set_retval_instr->next = *code_list;
      *code_list = set_retval_instr;
    }

    return NULL;
  }

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
  case TAC_PARAM:
  case TAC_ENTER:
  case TAC_LEAVE:
  case TAC_RETURN:
    break;
  default:
    break;
  }

  print_quad(code_list->next);
}

char *append_string(char *buffer, size_t *buffer_size, size_t *current_len,
                    const char *to_add) {

  size_t add_len = strlen(to_add);

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
    case TAC_GOTO:
    case TAC_IF_AND:
    case TAC_IF_EQ:
    case TAC_IF_GE:
    case TAC_IF_GT:
    case TAC_IF_LE:
    case TAC_IF_LT:
    case TAC_IF_NE:
    case TAC_IF_OR:
      break;
    default:
      break;
    }

    if (temp_instr_buffer[0] != '\0') {
      char *new_buffer_ptr = append_string(result_buffer, &buffer_size,
                                           &current_len, temp_instr_buffer);
      if (!new_buffer_ptr) {
        return NULL;
      }
      result_buffer = new_buffer_ptr;
    }

    current = current->next;
  }

  if (!result_buffer) {
    result_buffer = malloc(1);
    if (result_buffer)
      result_buffer[0] = '\0';
  }

  return result_buffer;
}
