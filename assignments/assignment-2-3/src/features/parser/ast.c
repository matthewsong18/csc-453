#include "ast.h"
#include "symbol_table.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

ASTnode *create_ast_node() {
  ASTnode *new_ast_node = malloc(sizeof(ASTnode));
  if (!new_ast_node) {
    fprintf(stderr, "ERROR: Memory allocation failed\n");
    exit(1);
  }

  new_ast_node->node_type = DUMMY;
  new_ast_node->symbol = NULL;
  new_ast_node->num = 0;
  new_ast_node->child0 = NULL;
  new_ast_node->child1 = NULL;
  new_ast_node->child2 = NULL;

  return new_ast_node;
}

ASTnode *create_one_child_node(NodeType node_type, ASTnode *child0) {
  ASTnode *node = create_ast_node();

  node->node_type = node_type;
  node->child0 = child0;

  return node;
}

ASTnode *create_two_child_node(NodeType node_type, ASTnode *child0,
                               ASTnode *child1) {
  ASTnode *node = create_one_child_node(node_type, child0);
  node->child1 = child1;

  return node;
}

ASTnode *create_three_child_node(NodeType node_type, ASTnode *child0,
                                 ASTnode *child1, ASTnode *child2) {
  ASTnode *node = create_two_child_node(node_type, child0, child1);
  node->child2 = child2;

  return node;
}

ASTnode *create_add_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(ADD, child0, child1);
}

ASTnode *create_and_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(AND, child0, child1);
}

ASTnode *create_assg_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(ASSG, child0, child1);
}

ASTnode *create_div_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(DIV, child0, child1);
}

ASTnode *create_eq_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(EQ, child0, child1);
}

ASTnode *create_expr_list_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(EXPR_LIST, child0, child1);
}

ASTnode *create_func_call_node(Symbol *function_name, ASTnode *child0) {
  ASTnode *func_call_node = create_one_child_node(FUNC_CALL, child0);
  func_call_node->symbol = function_name;
  return func_call_node;
}

ASTnode *create_func_defn_node(Symbol *function_name, ASTnode *child0) {
  ASTnode *func_defn_node = create_one_child_node(FUNC_DEF, child0);
  func_defn_node->symbol = function_name;

  return func_defn_node;
}

ASTnode *create_ge_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(GE, child0, child1);
}

ASTnode *create_gt_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(GT, child0, child1);
}

ASTnode *create_identifier_node(Symbol *id_name) {
  ASTnode *id_node = create_ast_node();
  id_node->node_type = IDENTIFIER;
  id_node->symbol = id_name;

  return id_node;
}

ASTnode *create_if_node(ASTnode *child0, ASTnode *child1, ASTnode *child2) {
  return create_three_child_node(IF, child0, child1, child2);
}

ASTnode *create_intconst_node(int num) {
  ASTnode *intconst_node = create_ast_node();
  intconst_node->node_type = INTCONST;
  intconst_node->num = num;
  return intconst_node;
}

ASTnode *create_le_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(LE, child0, child1);
}

ASTnode *create_lt_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(LT, child0, child1);
}

ASTnode *create_mul_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(MUL, child0, child1);
}

ASTnode *create_ne_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(NE, child0, child1);
}

ASTnode *create_or_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(OR, child0, child1);
}

ASTnode *create_return_node(ASTnode *child0) {
  return create_one_child_node(RETURN, child0);
}

ASTnode *create_stmt_list_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(STMT_LIST, child0, child1);
}

ASTnode *create_sub_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(SUB, child0, child1);
}

ASTnode *create_while_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(WHILE, child0, child1);
}

// Getter functions

/*
 * ptr: an arbitrary non-NULL AST pointer; ast_node_type() returns the node type
 * for the AST node ptr points to.
 */
NodeType ast_node_type(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->node_type;
}

/*
 * ptr: pointer to an AST for a function definition; func_def_name() returns
 * a pointer to the function name (a string) of the function definition AST that
 * ptr points to.
 */
char *func_def_name(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->symbol->name;
}

/*
 * ptr: pointer to an AST for a function definition; func_def_nargs() returns
 * the number of formal parameters for that function.
 */
int func_def_nargs(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->symbol->number_of_arguments;
}

/*
 * ptr: pointer to an AST for a function definition, n is an integer. If n > 0
 * and n <= no. of arguments for the function, then func_def_argname() returns
 * a pointer to the name (a string) of the nth formal parameter for that
 * function; the first formal parameter corresponds to n == 1.  If the value of
 * n is outside these parameters, the behavior of this function is undefined.
 */
char *func_def_argname(void *ptr, int n) {
  ASTnode *node = ptr;
  assert(node != NULL);
  if (node->node_type != FUNC_DEF) {
    exit(1);
  }

  if (n <= 0 || n > node->symbol->number_of_arguments) {
    exit(1);
  }

  Symbol *formal = node->symbol->arguments;
  for (int i = 1; i < n; i++) {
    formal = formal->next;
  }

  return formal->name;
}

/*
 * ptr: pointer to an AST for a function definition; func_def_body() returns
 * a pointer to the AST that is the function body of the function that ptr
 * points to.
 */
void *func_def_body(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}

/*
 * ptr: pointer to an AST node for a function call; func_call_callee() returns
 * a pointer to a string that is the name of the function being called.
 */
char *func_call_callee(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->symbol->name;
}

/*
 * ptr: pointer to an AST node for a function call; func_call_args() returns
 * a pointer to the AST that is the list of arguments to the call.
 */
void *func_call_args(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_head() returns
 * a pointer to the AST of the statement at the beginning of this list.
 */
void *stmt_list_head(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_rest() returns
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void *stmt_list_rest(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child1;
}

/*
 * ptr: pointer to an AST node for an expression list; expr_list_head() returns
 * a pointer to the AST of the expression at the beginning of this list.
 */
void *expr_list_head(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}

/*
 * ptr: pointer to an AST node for an expression list; expr_list_rest() returns
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void *expr_list_rest(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child1;
}

/*
 * ptr: pointer to an AST node for an IDENTIFIER; expr_id_name() returns a
 * pointer to the name of the identifier (a string).
 */
char *expr_id_name(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  assert(node->symbol != NULL);
  return node->symbol->name;
}

/*
 * ptr: pointer to an AST node for an INTCONST; expr_intconst_val() returns the
 * integer value of the constant.
 */
int expr_intconst_val(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->num;
}

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_1() returns a pointer to the AST of the first operand.
 */
void *expr_operand_1(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_2() returns a pointer to the AST of the second operand.
 */
void *expr_operand_2(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child1;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_expr() returns
 * a pointer to the AST for the expression tested by the if statement.
 */
void *stmt_if_expr(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_then() returns
 * a pointer to the AST for the then-part of the if statement, i.e., the
 * statement to be executed if the condition is true.
 */
void *stmt_if_then(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child1;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_else() returns
 * a pointer to the AST for the else-part of the if statement, i.e., the
 * statement to be executed if the condition is false.
 */
void *stmt_if_else(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child2;
}

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_lhs()
 * returns a pointer to the name of the identifier on the LHS of the
 * assignment.
 */
char *stmt_assg_lhs(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0->symbol->name;
}

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_rhs()
 * returns a pointer to the AST of the expression on the RHS of the assignment.
 */
void *stmt_assg_rhs(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child1;
}

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_expr()
 * returns a pointer to the AST of the expression tested by the while statement.
 */
void *stmt_while_expr(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_body()
 * returns a pointer to the AST of the body of the while statement.
 */
void *stmt_while_body(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child1;
}

/*
 * ptr: pointer to an AST node for a return statement.  stmt_return_expr()
 * returns a pointer to the AST of the expression whose value is returned.
 */
void *stmt_return_expr(void *ptr) {
  ASTnode *node = ptr;
  assert(node != NULL);
  return node->child0;
}
