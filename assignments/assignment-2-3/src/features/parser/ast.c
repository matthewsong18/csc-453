#include "ast.h"
#include "symbol_table.h"
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

// ASTnode *create_add_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(ADD, child0, child1);
// }

// ASTnode *create_and_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(AND, child0, child1);
// }

// ASTnode *create_assg_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(ASSG, child0, child1);
// }

// ASTnode *create_div_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(DIV, child0, child1);
// }

// ASTnode *create_eq_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(EQ, child0, child1);
// }

// // FIX how do I do this?
// ASTnode *create_expr_list_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(EXPR_LIST, child0, child1);
// }

// TODO
ASTnode *create_func_call_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(FUNC_CALL, child0, child1);
}

ASTnode *create_func_defn_node(Symbol *function_name, ASTnode *child0) {
  ASTnode *func_defn_node = create_one_child_node(FUNC_DEF, child0);
  func_defn_node->symbol = function_name;

  return func_defn_node;
}

// ASTnode *create_ge_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(GE, child0, child1);
// }

// ASTnode *create_gt_node(ASTnode *child0, ASTnode *child1) {
//   return create_two_child_node(GT, child0, child1);
// }

// ASTnode *create_identifier_node(Symbol *id_name) {
//   ASTnode *id_node = create_ast_node();
//   id_node->node_type = IDENTIFIER;
//   id_node->symbol = id_name;

//   return id_node;
// }

ASTnode *create_stmt_list_node(ASTnode *child0, ASTnode *child1) {
  return create_two_child_node(STMT_LIST, child0, child1);
}
