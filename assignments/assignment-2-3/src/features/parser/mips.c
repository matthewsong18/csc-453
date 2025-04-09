// mips.c
#include "mips.h"
#include "symbol_table.h"
#include "tac.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Helper functions: new_mips_instr, append_mips_instr ---
// (Assume they exist as before)
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
// --- End Helpers ---

MipsInstruction *generate_mips(Quad *tac_list) {
  MipsInstruction *mips_head = NULL;
  char buffer[256];

  // --- First Pass: Check for main and println usage ---
  bool main_exists = false;
  bool println_used = false;
  for (Quad *q = tac_list; q != NULL; q = q->next) {
    if (q->op == TAC_ENTER && q->src1 &&
        q->src1->operand_type == SYM_TABLE_PTR && q->src1->val.symbol_ptr &&
        strcmp(q->src1->val.symbol_ptr->name, "main") == 0) {
      main_exists = true;
    }
    // Check for CALL to "println" (make_TAC now generates TAC_CALL)
    if (q->op == TAC_CALL && q->src1 &&
        q->src1->operand_type == SYM_TABLE_PTR && q->src1->val.symbol_ptr &&
        strcmp(q->src1->val.symbol_ptr->name, "println") ==
            0) { // <<< Check for TAC_CALL "println"
      println_used = true;
    }
  }

  // Start with .text directive
  mips_head = append_mips_instr(mips_head, new_mips_instr(".text"));
  // Do not add .globl main

  // --- Second Pass: Generate MIPS for TAC instructions ---
  for (Quad *instruction = tac_list; instruction != NULL;
       instruction = instruction->next) {
    Operand *src1 = instruction->src1;
    Operand *src2 = instruction->src2;
    Operand *dest = instruction->dest;

    switch (instruction->op) {
    case TAC_ENTER:
      assert(src1 && src1->operand_type == SYM_TABLE_PTR &&
             src1->val.symbol_ptr);
      const char *func_name = src1->val.symbol_ptr->name;
      // Always prepend underscore to function label now
      snprintf(buffer, sizeof(buffer),
               "_%s:", func_name); // <<< Reverted to always add "_"
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));

      // Prologue (using 'la' as per spec)
      int n_local_bytes = 0; // TODO: Calculate properly
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $sp, -8($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    sw $fp, 4($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    sw $ra, 0($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $fp, 0($sp)"));
      // Use user's fix for 0 offset
      if (n_local_bytes == 0) {
        snprintf(buffer, sizeof(buffer), "    la $sp, 0($sp)");
      } else {
        snprintf(buffer, sizeof(buffer), "    la $sp, -%d($sp)", n_local_bytes);
      }
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      break;

    case TAC_ASSIGN:
      assert(dest && dest->operand_type == SYM_TABLE_PTR);
      assert(src1);
      if (src1->operand_type == INTEGER_CONSTANT) {
        // Using $t0 for immediate -> param pattern
        snprintf(buffer, sizeof(buffer), "    li $t0, %d",
                 src1->val.integer_const);
        mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
        // Assuming $t0 is used by next PARAM, no store to dest needed YET
      } else { /* ... handle other assignments ... */
        snprintf(buffer, sizeof(buffer), "    # Unhandled TAC_ASSIGN Var");
        mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      }
      break;

    case TAC_PARAM:
      assert(src1);
      // Assuming value is in $t0 from preceding TAC_ASSIGN immediate
      // OR needs load_operand if src1 is a variable/temp
      // Simple version for immediate -> param:
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $sp, -4($sp)"));
      mips_head = append_mips_instr(
          mips_head,
          new_mips_instr("    sw $t0, 0($sp)")); // Push assumed $t0 value
      break;

    case TAC_CALL: // <<< Handles all calls now, including println
      assert(src1 && src1->operand_type == SYM_TABLE_PTR &&
             src1->val.symbol_ptr);
      int n_args = 0; // Default to 0 if src2 is missing/invalid
      if (src2 && src2->operand_type == INTEGER_CONSTANT) {
        n_args = src2->val.integer_const;
      }
      int k_arg_bytes = 4 * n_args;

      // Use underscore prefix for the function name in jal
      snprintf(buffer, sizeof(buffer), "    jal _%s",
               src1->val.symbol_ptr->name);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));

      // Deallocate parameters using 'la'
      if (k_arg_bytes > 0) { // Only add if args were passed
        snprintf(buffer, sizeof(buffer), "    la $sp, %d($sp)", k_arg_bytes);
        mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      }

      // Handle return value if needed
      // if (dest) { /* store $v0 to dest */ }
      break;

    case TAC_LEAVE:
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
      mips_head = append_mips_instr(mips_head, new_mips_instr("    jr $ra"));
      break;

      // case TAC_PRINTLN: // <<< This case should NOT be hit if using Option 1
      //     mips_head = append_mips_instr(mips_head, new_mips_instr("    #
      //     ERROR: TAC_PRINTLN should not be generated!")); break;

    default:
      snprintf(buffer, sizeof(buffer), "# Unhandled TAC Op: %d",
               instruction->op);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      break;
    }
  }

  // --- Conditionally Append Data Segment and Helpers ---

  if (println_used) {
    // (Append .data, _nl, _println definition as before)
    mips_head = append_mips_instr(mips_head, new_mips_instr(".align 2"));
    mips_head = append_mips_instr(mips_head, new_mips_instr(".data"));
    mips_head =
        append_mips_instr(mips_head, new_mips_instr("_nl: .asciiz \"\\n\""));
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
    // This provides the 'main' label SPIM looks for and jumps to your '_main'
    // code
    mips_head = append_mips_instr(
        mips_head, new_mips_instr("main: j _main")); // <<< FIX: Use this line
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
