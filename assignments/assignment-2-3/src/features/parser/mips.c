// mips.c
#include "mips.h"
#include "symbol_table.h"
#include "tac.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Scope *globalScope;
extern Scope *currentScope;

// --- Helper functions: new_mips_instr, append_mips_instr ---
MipsInstruction *new_mips_instr(const char *instr_text) {
  MipsInstruction *new_instr =
      (MipsInstruction *)malloc(sizeof(MipsInstruction));
  if (!new_instr) {
    perror("Failed to allocate MipsInstruction");
    return NULL;
  }
  new_instr->instruction = strdup(instr_text ? instr_text : "");
  if (!new_instr->instruction) {
    perror("Failed to duplicate instruction string");
    free(new_instr);
    return NULL;
  }
  new_instr->next = NULL;
  return new_instr;
}
MipsInstruction *append_mips_instr(MipsInstruction *list_head,
                                   MipsInstruction *new_instr) {
  if (!new_instr)
    return list_head;
  if (!list_head)
    return new_instr;
  MipsInstruction *current = list_head;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = new_instr;
  return list_head;
}

// ========================================================================
// CHANGE 1a: Add Helper to Reverse Symbol List
// ========================================================================
Symbol *reverse_symbol_list(Symbol *head) {
  Symbol *prev = NULL;
  Symbol *current = head;
  Symbol *next = NULL;
  while (current != NULL) {
    next = current->next;
    current->next = prev;
    prev = current;
    current = next;
  }
  return prev; // New head
}
// ========================================================================
// END CHANGE 1a
// ========================================================================

MipsInstruction *load_operand_for_branch(Operand *op, const char *target_reg,
                                         MipsInstruction *current_mips_head) {
  char buffer[256];
  if (!op)
    return current_mips_head;

  if (op->operand_type == INTEGER_CONSTANT) {
    snprintf(buffer, sizeof(buffer), "    li %s, %d", target_reg,
             op->val.integer_const);
    current_mips_head =
        append_mips_instr(current_mips_head, new_mips_instr(buffer));
  } else if (op->operand_type == SYM_TABLE_PTR) {
    Symbol *sym = op->val.symbol_ptr;
    char *sym_name = sym->name;
    bool is_temp = (sym_name[0] == 't' && isdigit(sym_name[1]));
    bool is_global = (!is_temp && sym->scope == globalScope);
    bool is_local_or_param = (!is_temp && !is_global);

    if (is_temp) {
      // Value is already in a temp register (e.g., $t2)
      char src_reg[10];
      snprintf(src_reg, sizeof(src_reg), "$%s", sym_name);
      // Move it to the target register ($t0 or $t1) if different
      if (strcmp(src_reg, target_reg) != 0) {
        snprintf(buffer, sizeof(buffer), "    move %s, %s", target_reg,
                 src_reg);
        current_mips_head =
            append_mips_instr(current_mips_head, new_mips_instr(buffer));
      }
    } else if (is_global) {
      // Load from global
      snprintf(buffer, sizeof(buffer), "    lw %s, _%s", target_reg, sym_name);
      current_mips_head =
          append_mips_instr(current_mips_head, new_mips_instr(buffer));
    } else if (is_local_or_param) {
      // Load from local/param via offset
      assert(sym->offset != 0);
      snprintf(buffer, sizeof(buffer), "    lw %s, %d($fp)", target_reg,
               sym->offset);
      current_mips_head =
          append_mips_instr(current_mips_head, new_mips_instr(buffer));
    } else { /* Unhandled */
    }
  } else { /* Unhandled */
  }
  return current_mips_head;
}

void get_label_str(Operand *label_op, char *label_buffer, size_t buffer_size) {
  if (label_op && label_op->operand_type == INTEGER_CONSTANT) {
    // Use "_" prefix for labels to avoid conflicts (as suggested in
    // 3addr2spim.pdf)
    snprintf(label_buffer, buffer_size, "_L%d", label_op->val.integer_const);
  } else {
    snprintf(label_buffer, buffer_size, "_INVALID_LABEL");
  }
}

MipsInstruction *generate_mips(Quad *tac_list) {
  MipsInstruction *mips_head = NULL;
  char buffer[256];
  char label_str[50];
  int param_load_temp_idx = 1; // Start at 1 for $t1

  // --- First Pass: Check for main and println usage ---
  bool main_exists = false;
  bool println_used = false;
  for (Quad *q = tac_list; q != NULL; q = q->next) {
    if (q->op == TAC_ENTER && q->src1 &&
        q->src1->operand_type == SYM_TABLE_PTR && q->src1->val.symbol_ptr &&
        strcmp(q->src1->val.symbol_ptr->name, "main") == 0) {
      main_exists = true;
    }
    if (q->op == TAC_CALL && q->src1 &&
        q->src1->operand_type == SYM_TABLE_PTR && q->src1->val.symbol_ptr &&
        strcmp(q->src1->val.symbol_ptr->name, "println") == 0) {
      println_used = true;
    }
  }

  // ========================================================================
  // CHANGE 1b: Fix Data Section Order (Reverse List before Iterating)
  // ========================================================================
  bool data_section_added = false;
  Symbol *original_global_head = globalScope->symbols; // Remember original head
  Symbol *reversed_globals =
      reverse_symbol_list(globalScope->symbols); // Reverse the list

  for (Symbol *sym = reversed_globals; sym != NULL;
       sym = sym->next) { // Iterate the REVERSED list (z -> y -> x)
    if (strcmp(sym->type, "variable") == 0 && sym->name[0] != 't') {
      if (!data_section_added) {
        mips_head = append_mips_instr(mips_head, new_mips_instr(".data"));
        mips_head = append_mips_instr(mips_head, new_mips_instr(".align 2"));
        data_section_added = true;
      }
      snprintf(buffer, sizeof(buffer), "_%s: .space 4", sym->name);
      // Append directives (appends _z, then _y, then _x)
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
    }
  }
  // IMPORTANT: Reverse the list back to original order if needed elsewhere
  globalScope->symbols = reverse_symbol_list(reversed_globals);
  // ========================================================================
  // END CHANGE 1b
  // ========================================================================

  // Add the .text section AFTER potential global variables
  mips_head = append_mips_instr(mips_head, new_mips_instr(".text"));

  // --- Second Pass: Generate MIPS for TAC instructions ---
  for (Quad *instruction = tac_list; instruction != NULL;
       instruction = instruction->next) {
    Operand *src1 = instruction->src1;
    Operand *src2 = instruction->src2;
    Operand *dest = instruction->dest;

    switch (instruction->op) {
    case TAC_ENTER: { // Use braces for scope
      assert(src1 && src1->operand_type == SYM_TABLE_PTR &&
             src1->val.symbol_ptr);
      Symbol *func_sym = src1->val.symbol_ptr; // Get the function symbol
      const char *func_name = func_sym->name;

      snprintf(buffer, sizeof(buffer), "_%s:", func_name);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));

      // --- Prologue ---
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $sp, -8($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    sw $fp, 4($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    sw $ra, 0($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $fp, 0($sp)"));

      // ============================================================
      // NEW: Allocate space for locals using calculated size
      // ============================================================
      int n_local_bytes = func_sym->local_var_bytes; // <<< Retrieve stored size

      if (n_local_bytes > 0) {
        snprintf(buffer, sizeof(buffer), "    la $sp, -%d($fp)",
                 n_local_bytes); // Adjust SP relative to new FP
        mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      }
      // If n_local_bytes is 0, SP remains correctly pointing below saved RA.
      // ============================================================
      // END NEW
      // ============================================================

    } break;

    // ========================================================================
    // CHANGE 3: Refine TAC_ASSIGN (Keep previous efficient version)
    // ========================================================================
    case TAC_ASSIGN: {
      assert(dest && dest->operand_type == SYM_TABLE_PTR);
      assert(src1);

      Symbol *dest_sym = dest->val.symbol_ptr;
      char *dest_name = dest_sym->name;

      bool is_dest_temp = (dest_name[0] == 't' && isdigit(dest_name[1]));
      bool is_global_dest = (!is_dest_temp && dest_sym->scope == globalScope);
      bool is_local_dest = (!is_dest_temp && !is_global_dest);

      if (is_dest_temp) {
        // === DESTINATION IS TEMP ($tN) ===
        char dest_reg_mips[10];
        snprintf(dest_reg_mips, sizeof(dest_reg_mips), "$%s",
                 dest_name); // e.g., $t2

        if (src1->operand_type == INTEGER_CONSTANT) {
          // Temp = Constant --> li $tN, const (Directly load into destination)
          snprintf(buffer, sizeof(buffer), "    li %s, %d", dest_reg_mips,
                   src1->val.integer_const);
          mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
        } else if (src1->operand_type == SYM_TABLE_PTR) {
          // Temp = Temp / Var ($tN = $tM / x / y)
          Symbol *src_sym = src1->val.symbol_ptr;
          char *src_name = src_sym->name;
          bool is_src_temp = (src_name[0] == 't' && isdigit(src_name[1]));
          bool is_src_global = (!is_src_temp && src_sym->scope == globalScope);
          bool is_src_local = (!is_src_temp && !is_src_global);

          if (is_src_temp) {
            // Temp = Temp ($tN = $tM) --> move $tN, $tM (if N != M)
            char src_reg_mips[10];
            snprintf(src_reg_mips, sizeof(src_reg_mips), "$%s", src_name);
            if (strcmp(dest_name, src_name) != 0) {
              snprintf(buffer, sizeof(buffer), "    move %s, %s", dest_reg_mips,
                       src_reg_mips);
              mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
            }
          } else if (is_src_global) {
            // Temp = Global ($tN = _x) --> lw $tN, _x
            snprintf(buffer, sizeof(buffer), "    lw %s, _%s", dest_reg_mips,
                     src_name);
            mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
          } else if (is_src_local) {
            // Temp = Local ($tN = offset($fp)) --> lw $tN, offset($fp)
            assert(src_sym->offset != 0);
            snprintf(buffer, sizeof(buffer), "    lw %s, %d($fp)",
                     dest_reg_mips, src_sym->offset);
            mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
          } else { /* Unhandled Source Symbol */
          }
        } else { /* Unhandled Source Operand Type */
        }

      } else {
        // === DESTINATION IS VAR (Global or Local) ===
        char temp_reg_for_store[10] =
            "$t0"; // Register holding the value to store

        // Load source value into temp_reg_for_store ($t0 or $tM)
        if (src1->operand_type == INTEGER_CONSTANT) {
          // Var = Constant --> li $t0, const
          snprintf(buffer, sizeof(buffer), "    li %s, %d", temp_reg_for_store,
                   src1->val.integer_const);
          mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
        } else if (src1->operand_type == SYM_TABLE_PTR) {
          Symbol *src_sym = src1->val.symbol_ptr;
          char *src_name = src_sym->name;
          bool is_src_temp = (src_name[0] == 't' && isdigit(src_name[1]));
          bool is_src_global = (!is_src_temp && src_sym->scope == globalScope);
          bool is_src_local = (!is_src_temp && !is_src_global);

          if (is_src_temp) {
            // Var = Temp --> Use $tM directly for store
            snprintf(temp_reg_for_store, sizeof(temp_reg_for_store), "$%s",
                     src_name);
          } else if (is_src_global) {
            // Var = Global --> lw $t0, _src; use $t0 for store
            snprintf(temp_reg_for_store, sizeof(temp_reg_for_store), "$t0");
            snprintf(buffer, sizeof(buffer), "    lw %s, _%s",
                     temp_reg_for_store, src_name);
            mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
          } else if (is_src_local) {
            // Var = Local --> lw $t0, offset($fp); use $t0 for store
            assert(src_sym->offset != 0);
            snprintf(temp_reg_for_store, sizeof(temp_reg_for_store), "$t0");
            snprintf(buffer, sizeof(buffer), "    lw %s, %d($fp)",
                     temp_reg_for_store, src_sym->offset);
            mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
          } else { /* Unhandled Source Symbol */
          }
        } else { /* Unhandled Source Operand Type */
        }

        // Store value from temp_reg_for_store ($t0 or $tM) into destination var
        if (is_global_dest) {
          snprintf(buffer, sizeof(buffer), "    sw %s, _%s", temp_reg_for_store,
                   dest_name);
          mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
        } else if (is_local_dest) {
          assert(dest_sym->offset != 0);
          snprintf(buffer, sizeof(buffer), "    sw %s, %d($fp)",
                   temp_reg_for_store, dest_sym->offset);
          mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
        } else { /* Should not happen */
        }
      }
    } break;
    // ========================================================================
    // END CHANGE 3
    // ========================================================================

    // ========================================================================
    // CHANGE 2b: Fix TAC_PARAM (Use counter for var loads -> $t1, $t3, $t5...)
    // ========================================================================
    case TAC_PARAM: {
      assert(instruction->src1);
      Operand *param_op = instruction->src1;
      char param_push_reg[10];

      if (param_op->operand_type == SYM_TABLE_PTR) {
        Symbol *param_sym = param_op->val.symbol_ptr;
        char *param_name = param_sym->name;
        bool is_param_temp = (param_name[0] == 't' && isdigit(param_name[1]));
        // --- Use scope field ---
        bool is_param_global =
            (!is_param_temp && param_sym->scope == globalScope);
        bool is_param_local = (!is_param_temp && !is_param_global);

        if (is_param_temp) {
          snprintf(param_push_reg, sizeof(param_push_reg), "$%s", param_name);
        } else if (is_param_local) {
          // Param is Local variable -> load into $tN using offset
          const char *load_reg = "$t0";

          snprintf(param_push_reg, sizeof(param_push_reg), "%s", load_reg);

          assert(param_sym->offset != 0 ||
                 (param_sym->type &&
                  strcmp(param_sym->type, "parameter") == 0 &&
                  param_sym->offset > 0));

          snprintf(buffer, sizeof(buffer), "    lw %s, %d($fp)", load_reg,
                   param_sym->offset); // Use offset
          mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));

        } else if (is_param_global) {
          // Param is Global variable -> load into $tN using label
          char load_reg[10];
          snprintf(load_reg, sizeof(load_reg), "$t%d", param_load_temp_idx);
          snprintf(param_push_reg, sizeof(param_push_reg), "%s", load_reg);
          snprintf(buffer, sizeof(buffer), "    lw %s, _%s", load_reg,
                   param_name); // Use label
          mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
          param_load_temp_idx += 2;
          if (param_load_temp_idx > 7)
            param_load_temp_idx = 1;
        } else { /* Should not happen */
        }
      } else if (param_op->operand_type == INTEGER_CONSTANT) {
        const char *load_reg = "$t0"; // Still use $t0 for constants
        snprintf(param_push_reg, sizeof(param_push_reg), "%s", load_reg);
        snprintf(buffer, sizeof(buffer), "    li %s, %d", load_reg,
                 param_op->val.integer_const);
        mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      } else { /* Unhandled param type */
      }

      // Push value
      snprintf(buffer, sizeof(buffer), "    la $sp, -4($sp)");
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      snprintf(buffer, sizeof(buffer), "    sw %s, 0($sp)", param_push_reg);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));

    } break;

    case TAC_CALL:
      // ... (Same as before) ...
      assert(src1 && src1->operand_type == SYM_TABLE_PTR &&
             src1->val.symbol_ptr);
      int n_args = 0;
      if (src2 && src2->operand_type == INTEGER_CONSTANT) {
        n_args = src2->val.integer_const;
      }
      int k_arg_bytes = 4 * n_args;
      snprintf(buffer, sizeof(buffer), "    jal _%s",
               src1->val.symbol_ptr->name);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      if (k_arg_bytes > 0) {
        snprintf(buffer, sizeof(buffer), "    la $sp, %d($sp)", k_arg_bytes);
        mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      }
      break;

    case TAC_LEAVE:
      // ... (Same as before) ...
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $sp, 0($fp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    lw $ra, 0($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    lw $fp, 4($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $sp, 8($sp)"));
      break;

    case TAC_RETURN:
      // ... (Same as before) ...
      mips_head = append_mips_instr(mips_head, new_mips_instr("    jr $ra"));
      break;
    case TAC_LABEL: {
      // TAC: label Lsrc1 (src1->val.integer_const holds label number)
      assert(src1 && src1->operand_type == INTEGER_CONSTANT);
      get_label_str(src1, label_str, sizeof(label_str));  // Use src1 now
      snprintf(buffer, sizeof(buffer), "%s:", label_str); // MIPS label: _Ln:
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
    } break;

    case TAC_GOTO: {
      // TAC: goto Ldest (dest->val.integer_const holds label number)
      assert(dest && dest->operand_type == INTEGER_CONSTANT);
      get_label_str(dest, label_str, sizeof(label_str));
      snprintf(buffer, sizeof(buffer), "    j %s",
               label_str); // MIPS jump: j _Ln
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
    } break;

    // Conditional Branches: if src1 relop src2 goto Ldest
    case TAC_IF_EQ: // if src1 == src2 goto Ldest -> beq $t0, $t1, _Ldest
    case TAC_IF_NE: // if src1 != src2 goto Ldest -> bne $t0, $t1, _Ldest
    case TAC_IF_LT: // if src1 <  src2 goto Ldest -> blt $t0, $t1, _Ldest
    case TAC_IF_LE: // if src1 <= src2 goto Ldest -> ble $t0, $t1, _Ldest
    case TAC_IF_GT: // if src1 >  src2 goto Ldest -> bgt $t0, $t1, _Ldest
    case TAC_IF_GE: // if src1 >= src2 goto Ldest -> bge $t0, $t1, _Ldest
    {
      assert(src1 && src2 && dest && dest->operand_type == INTEGER_CONSTANT);

      // 1. Load src1 into $t0 using minimal helper
      mips_head = load_operand_for_branch(src1, "$t0", mips_head);
      // 2. Load src2 into $t1 using minimal helper
      mips_head = load_operand_for_branch(src2, "$t1", mips_head);
      // 3. Get target label string
      get_label_str(dest, label_str, sizeof(label_str));
      // 4. Determine MIPS branch instruction
      char *branch_op;
      switch (instruction->op) {
      case TAC_IF_EQ:
        branch_op = "beq";
        break;
      case TAC_IF_NE:
        branch_op = "bne";
        break;
      case TAC_IF_LT:
        branch_op = "blt";
        break;
      case TAC_IF_LE:
        branch_op = "ble";
        break;
      case TAC_IF_GT:
        branch_op = "bgt";
        break;
      case TAC_IF_GE:
        branch_op = "bge";
        break;
      default:
        branch_op = "###";
        break; // Should not happen
      }
      // 5. Generate the MIPS branch instruction
      snprintf(buffer, sizeof(buffer), "    %s $t0, $t1, %s", branch_op,
               label_str);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
    } break;

    // Placeholder for AND/OR if implemented - requires specific logic
    case TAC_IF_AND:
    case TAC_IF_OR:
      snprintf(buffer, sizeof(buffer), "# Unhandled TAC Op: %d (AND/OR)",
               instruction->op);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      break;

    default:
      snprintf(buffer, sizeof(buffer), "# Unhandled TAC Op: %d",
               instruction->op);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      break;
    }
  }

  // --- Conditionally Append Data Segment and Helpers ---

  // ========================================================================
  // CHANGE 4: Fix Missing .data before _nl when Globals Exist
  // ========================================================================
  if (println_used) {
    // ========================================================================
    // CHANGE 4 Rev B: Fix Missing .data before _nl
    // ========================================================================
    // Always add .align 2 before _nl section
    mips_head = append_mips_instr(mips_head, new_mips_instr(".align 2"));

    // Add .data if it wasn't added for globals OR if it was, ensure we are in
    // data section The simplest is to just emit .data here always if println
    // used, as nested .data directives are usually handled fine by assemblers.
    // Alternatively, only add if !data_section_added, but that failed before.
    // Let's try always adding it here.
    mips_head = append_mips_instr(mips_head, new_mips_instr(".data"));

    // Now add _nl
    mips_head =
        append_mips_instr(mips_head, new_mips_instr("_nl: .asciiz \"\\n\""));
    // ========================================================================
    // END CHANGE 4 Rev B
    // ========================================================================

    // Add println code in .text section
    mips_head = append_mips_instr(mips_head, new_mips_instr(".align 2"));
    mips_head = append_mips_instr(mips_head, new_mips_instr(".text"));
    mips_head = append_mips_instr(mips_head, new_mips_instr("_println:"));
    mips_head = append_mips_instr(mips_head, new_mips_instr("    li $v0, 1"));
    mips_head =
        append_mips_instr(mips_head, new_mips_instr("    lw $a0, 0($sp)"));
    mips_head = append_mips_instr(mips_head, new_mips_instr("    syscall"));
    mips_head = append_mips_instr(mips_head, new_mips_instr("    li $v0, 4"));
    mips_head = append_mips_instr(mips_head, new_mips_instr("    la $a0, _nl"));
    mips_head = append_mips_instr(mips_head, new_mips_instr("    syscall"));
    mips_head = append_mips_instr(mips_head, new_mips_instr("    jr $ra"));
  }

  if (main_exists) {
    mips_head = append_mips_instr(mips_head, new_mips_instr("")); // Spacer
    mips_head = append_mips_instr(mips_head, new_mips_instr("main: j _main"));
  }

  return mips_head;
}

// --- mips_list_to_string (Simplified Newline Logic) ---
char *mips_list_to_string(MipsInstruction *mips_list) {
  if (!mips_list) {
    char *empty = malloc(1);
    if (empty)
      empty[0] = '\0';
    return empty; // Return empty, null-terminated string
  }

  // --- Calculate total length ---
  size_t total_len = 0;
  for (MipsInstruction *curr = mips_list; curr != NULL; curr = curr->next) {
    if (curr->instruction) {
      total_len += strlen(curr->instruction);
    }
    total_len += 1; // Add 1 for each newline '\n'
  }
  total_len += 1; // Add 1 for the final null terminator '\0'

  // --- Allocate buffer ---
  char *result_string = (char *)malloc(total_len);
  if (!result_string) {
    perror("Failed to allocate string for MIPS output");
    return NULL;
  }
  // Ensure buffer starts empty for strcat
  result_string[0] = '\0';

  // --- Concatenate instructions with consistent newlines ---
  for (MipsInstruction *curr = mips_list; curr != NULL; curr = curr->next) {
    if (curr->instruction) {
      strcat(result_string, curr->instruction); // Append instruction
    }
    strcat(result_string, "\n"); // ALWAYS append newline
  }

  // The final null terminator is implicitly handled by malloc size + strcat

  return result_string;
}

// --- free_mips_list (Remains the same) ---
void free_mips_list(MipsInstruction *mips_list) {
  MipsInstruction *current = mips_list;
  MipsInstruction *next = NULL;
  while (current != NULL) {
    next = current->next;
    free(current->instruction);
    free(current);
    current = next;
  }
}
