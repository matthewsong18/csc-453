#ifndef MIPS_H
#define MIPS_H

#include <stdlib.h>
#include "tac.h"

// Structure to hold a single MIPS instruction line (as a string)
typedef struct MipsInstruction {
    char *instruction;
    struct MipsInstruction *next;
} MipsInstruction;

// Function prototypes
MipsInstruction* generate_mips(Quad *tac_list);
char* mips_list_to_string(MipsInstruction *mips_list);
void free_mips_list(MipsInstruction *mips_list); // Important for cleanup

#endif // MIPS_H
