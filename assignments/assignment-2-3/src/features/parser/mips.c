// mips.c
#include "mips.h" // Your new header (assuming it defines MipsInstruction)
#include "symbol_table.h" // For Symbol definition [cite: uploaded:parser/symbol_table.h]
#include "tac.h" // Your existing TAC header [cite: uploaded:parser/tac.h]
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Helper functions: new_mips_instr, append_mips_instr assumed here ---
// (Include from previous response or implement as needed)
MipsInstruction *new_mips_instr(const char *instr_text) {
  MipsInstruction *new_instr =
      (MipsInstruction *)malloc(sizeof(MipsInstruction));
  if (!new_instr) {
    perror("Failed to allocate MipsInstruction");
    return NULL;
  }
  new_instr->instruction = strdup(instr_text);
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

// Main MIPS generation function - Rewritten to match 3addr2spim.pdf spec
MipsInstruction *generate_mips(Quad *tac_list) {
  MipsInstruction *mips_head = NULL;
  char buffer[256]; // Temporary buffer for formatting labels

  // Start with .text directive
  mips_head = append_mips_instr(mips_head, new_mips_instr(".text"));

  for (Quad *instruction = tac_list; instruction != NULL;
       instruction = instruction->next) {
    Operand *src1 = instruction->src1;
    // src2, dest not needed for ENTER/LEAVE/RETURN

    switch (instruction->op) {
    case TAC_ENTER:
      // Generate code exactly matching Section 3.8.1 [cite: 72]
      assert(src1 && src1->operand_type == SYM_TABLE_PTR &&
             src1->val.symbol_ptr);
      const char *func_name = src1->val.symbol_ptr->name;

      // 1. Function Label
      snprintf(buffer, sizeof(buffer), "%s:", func_name);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));

      // 2. Prologue from PDF (using 'la')
      // NOTE: Assumes n=0 (no locals/temps) for the last 'la $sp, n($sp)'
      // You will need to calculate 'n' for functions with locals/temps later.
      int n_local_bytes = 0; // For "int f() {}", n=0
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $sp, -8($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    sw $fp, 4($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    sw $ra, 0($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("    la $fp, 0($sp)"));
      snprintf(buffer, sizeof(buffer), "    la $sp, %d($sp)",
               -n_local_bytes); // Use negative offset for allocation if needed?
                                // PDF is ambiguous, use n=0 for empty func.
                                // Let's assume 'n' in PDF means space *below*
                                // saved regs, so -n adjusts sp down. For n=0,
                                // this is "la $sp, 0($sp)".
      mips_head = append_mips_instr(
          mips_head, new_mips_instr(buffer)); // "la $sp, 0($sp)" for n=0

      break;

    case TAC_LEAVE:
      // Generate code exactly matching Section 3.8.3 for 'leave f' [cite: 78]
      mips_head = append_mips_instr(
          mips_head,
          new_mips_instr("la $sp, 0($fp)")); // Note: space before instruction
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("lw $ra, 0($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("lw $fp, 4($sp)"));
      mips_head =
          append_mips_instr(mips_head, new_mips_instr("la $sp, 8($sp)"));
      break;

    case TAC_RETURN:
      // Generate code exactly matching Section 3.8.3 for 'return' [cite: 78]
      mips_head = append_mips_instr(mips_head, new_mips_instr("jr $ra"));
      break;

    // Add other cases (ASSIGN, ADD, etc.) here later,
    // translating them according to 3addr2spim.pdf rules.
    default:
      snprintf(buffer, sizeof(buffer), "# Unhandled TAC Op: %d",
               instruction->op);
      mips_head = append_mips_instr(mips_head, new_mips_instr(buffer));
      break;
    }
  }
  return mips_head;
}

// --- mips_list_to_string and free_mips_list functions remain the same ---
// (Include from previous response or implement as needed)
char *mips_list_to_string(MipsInstruction *mips_list) {
  if (!mips_list) {
    char *empty = malloc(1);
    if (empty)
      empty[0] = '\0';
    return empty;
  }
  size_t total_len = 0;
  for (MipsInstruction *curr = mips_list; curr != NULL; curr = curr->next) {
    if (curr->instruction) {
      total_len += strlen(curr->instruction) + 1; // +1 for newline
    }
  }
  total_len += 1; // +1 for final null terminator
  char *result_string = (char *)malloc(total_len);
  if (!result_string) {
    perror("Failed to allocate string for MIPS output");
    return NULL;
  }
  result_string[0] = '\0';
  for (MipsInstruction *curr = mips_list; curr != NULL; curr = curr->next) {
    if (curr->instruction) {
      strcat(result_string, curr->instruction);
      strcat(result_string, "\n");
    }
  }
  return result_string;
}

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
